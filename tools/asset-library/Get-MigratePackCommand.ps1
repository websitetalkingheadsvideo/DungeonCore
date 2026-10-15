#Requires -Version 5.1
<#
.SYNOPSIS
  Resolve a catalog pack id via pack-roots.json and print (or run) migrate-pack-assets.ps1.

.EXAMPLE
  .\Get-MigratePackCommand.ps1 -ListPacks

.EXAMPLE
  .\Get-MigratePackCommand.ps1 -PackId MedievalFantasyDungeon_5.4

.EXAMPLE
  .\Get-MigratePackCommand.ps1 -PackId Crystal_Cave -Seeds @('/Game/Crystal_Cave/Meshes/SM_Crystal_D_01') -Execute -DryRun
#>
param(
    [string]$PackId,
    [string[]]$Seeds = @(),
    [string]$DestContentRoot = 'E:\Dungeon Core\DungeonCore\Content',
    [string]$PackRootsFile = (Join-Path $PSScriptRoot 'pack-roots.json'),
    [switch]$ListPacks,
    [switch]$Execute,
    [switch]$DryRun
)

$ErrorActionPreference = 'Stop'

function Normalize-StringArray {
    param($Value)
    if ($null -eq $Value) { return @() }
    if ($Value -is [System.Array]) { return @($Value) }
    return @([string]$Value)
}

function Resolve-PackEntry {
    param($Packs, [string]$Id)
    $exact = @($Packs | Where-Object { $_.id -eq $Id })
    if ($exact.Count -eq 1) { return $exact[0] }
    $ci = @($Packs | Where-Object { $_.id -eq $Id -or $_.id -like "*$Id*" })
    if ($ci.Count -eq 1) { return $ci[0] }
    if ($ci.Count -gt 1) {
        Write-Error "Ambiguous pack id '$Id'. Matches: $($ci.id -join ', ')"
    }
    return $null
}

if (-not (Test-Path -LiteralPath $PackRootsFile)) {
    Write-Error "Missing $PackRootsFile — run Sync-PackRoots.ps1 first."
}

$roots = Get-Content -LiteralPath $PackRootsFile -Raw | ConvertFrom-Json
$packList = @($roots.packs)

if ($ListPacks) {
    foreach ($p in ($packList | Sort-Object { $_.id })) {
        $flag = if ($p.hasUproject) { 'ue' } else { 'no-uproject' }
        $mounts = (Normalize-StringArray $p.contentMounts) -join ', '
        Write-Output ("{0,-45} [{1}] mounts: {2}" -f $p.id, $flag, $mounts)
    }
    exit 0
}

if ([string]::IsNullOrWhiteSpace($PackId)) {
    Write-Error 'Pass -PackId <catalog folder name> or -ListPacks.'
}

$pack = Resolve-PackEntry -Packs $packList -Id $PackId
if ($null -eq $pack) {
    Write-Error "No pack matching '$PackId'. Use -ListPacks."
}

$mounts = Normalize-StringArray $pack.contentMounts
Write-Output "# Catalog id: $($pack.id)"
Write-Output "# projectRel: $($pack.projectRel)"
Write-Output "# uproject:   $($pack.uproject)"
if ($mounts.Count -gt 0) {
    Write-Output ("# /Game mounts: {0}" -f ($mounts -join ', '))
}

if (-not $pack.hasUproject) {
    Write-Error "Pack '$($pack.id)' has no .uproject (e.g. raw WAV). Cannot migrate via migrate-pack-assets.ps1."
}

if ($Seeds.Count -eq 0) {
    Write-Output '# Pass -Seeds @("/Game/<mount>/...") — example prefixes:'
    foreach ($m in $mounts) {
        Write-Output "#   /Game/$m/"
    }
    Write-Output ''
}

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$migrateScript = Join-Path $repoRoot '.cursor\skills\fab-asset-migration\scripts\migrate-pack-assets.ps1'
if (-not (Test-Path -LiteralPath $migrateScript)) {
    Write-Error "Migrate script not found: $migrateScript"
}

$seedLiteral = if ($Seeds.Count -eq 0) {
    '@("/Game/<mount>/path/to/asset")'
} else {
    '@(' + (($Seeds | ForEach-Object { "'$_'" }) -join ', ') + ')'
}

$dryRunFlag = if ($DryRun -or -not $Execute) { ' -DryRun' } else { '' }

$lines = @(
    "& `"$migrateScript`" ``"
    "    -PackContentRoot `"$($pack.contentPath)`" ``"
    "    -Seeds $seedLiteral ``"
    "    -DestContentRoot `"$DestContentRoot`"$dryRunFlag"
)
$commandText = $lines -join [Environment]::NewLine
Write-Output $commandText

if (-not $Execute) {
    Write-Output ''
    Write-Output '# Add -Execute -DryRun to run dry-run; -Execute without -DryRun copies files.'
    exit 0
}

if ($Seeds.Count -eq 0) {
    Write-Error 'Cannot -Execute without -Seeds.'
}

$params = @{
    PackContentRoot  = $pack.contentPath
    Seeds            = $Seeds
    DestContentRoot  = $DestContentRoot
}
if ($DryRun) { $params['DryRun'] = $true }

& $migrateScript @params
