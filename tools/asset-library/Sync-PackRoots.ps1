#Requires -Version 5.1
<#
.SYNOPSIS
  Scan H:\Unreal Assets and write pack-roots.json (library id -> UE project root -> Content).

  Library catalog rows use the TOP-LEVEL folder name under the assets root.
  The .uproject is often nested and its file name rarely matches that folder.
#>
param(
    [string]$AssetsRoot = 'H:\Unreal Assets',
    [string]$OutFile = (Join-Path $PSScriptRoot 'pack-roots.json'),
    [string[]]$ExcludeDirs = @('.cursor', 'reference', 'Licenses', 'Unreal Assets')
)

$ErrorActionPreference = 'Stop'

function Get-ContentMountNames {
    param([string]$ContentDir)
    if (-not (Test-Path -LiteralPath $ContentDir)) {
        return @()
    }
    $skip = @('__ExternalActors__', '__ExternalObjects__', 'Collections', 'Developers', 'Splash')
    return @(Get-ChildItem -LiteralPath $ContentDir -Directory |
        Where-Object { $skip -notcontains $_.Name } |
        ForEach-Object { $_.Name })
}

$packs = @()
Get-ChildItem -LiteralPath $AssetsRoot -Directory |
    Where-Object { $ExcludeDirs -notcontains $_.Name } |
    ForEach-Object {
        $id = $_.Name
        $topFull = $_.FullName
        $uprojects = @(Get-ChildItem -LiteralPath $topFull -Filter '*.uproject' -Recurse -ErrorAction SilentlyContinue)
        if ($uprojects.Count -eq 0) {
            $packs += [ordered]@{
                id              = $id
                hasUproject     = $false
                projectRel      = $null
                uproject        = $null
                contentPath     = $null
                contentMounts   = @()
                uprojectCount   = 0
            }
            return
        }
        if ($uprojects.Count -gt 1) {
            Write-Warning "Pack '$id' has $($uprojects.Count) .uproject files; using shallowest."
        }
        $uprojectFile = $uprojects | Sort-Object { $_.FullName.Length } | Select-Object -First 1
        $projectDir = $uprojectFile.DirectoryName
        $projectRel = $projectDir.Substring($AssetsRoot.Length).TrimStart('\')
        $contentPath = Join-Path $projectDir 'Content'
        $mounts = Get-ContentMountNames -ContentDir $contentPath
        $packs += [ordered]@{
            id            = $id
            hasUproject   = $true
            projectRel    = $projectRel
            uproject      = $uprojectFile.Name
            contentPath   = $contentPath
            contentMounts = $mounts
            uprojectCount = $uprojects.Count
        }
    }

$payload = [ordered]@{
    generatedAt = (Get-Date).ToString('o')
    assetsRoot  = $AssetsRoot
    excludeDirs = $ExcludeDirs
    packs       = @($packs | Sort-Object { $_.id })
}

$json = $payload | ConvertTo-Json -Depth 6
Set-Content -LiteralPath $OutFile -Value $json -Encoding UTF8
Write-Host "Wrote $($packs.Count) pack entries to $OutFile"
