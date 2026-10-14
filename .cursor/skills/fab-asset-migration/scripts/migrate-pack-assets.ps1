param(
    [Parameter(Mandatory = $true)][string]$PackContentRoot,
    [Parameter(Mandatory = $true)][string[]]$Seeds,
    [Parameter(Mandatory = $true)][string]$DestContentRoot,
    [switch]$DryRun
)

# Resolve a /Game/Path/Asset package name to a source file in the pack.
function Resolve-PackageFile([string]$PackageName) {
    $Relative = $PackageName.Substring("/Game/".Length).Replace('/', '\')
    foreach ($Ext in @(".uasset", ".umap")) {
        $Candidate = Join-Path $PackContentRoot ($Relative + $Ext)
        if (Test-Path $Candidate) { return $Candidate }
    }
    return $null
}

# Pull every /Game/... package reference out of a package's name table.
function Get-PackageReferences([string]$FilePath) {
    $Bytes = [System.IO.File]::ReadAllBytes($FilePath)
    $Text = [System.Text.Encoding]::ASCII.GetString($Bytes)
    $Matches = [regex]::Matches($Text, '/Game/[A-Za-z0-9_\-/]+')
    $Set = New-Object System.Collections.Generic.HashSet[string]
    foreach ($M in $Matches) { [void]$Set.Add($M.Value) }
    return $Set
}

$Visited = New-Object System.Collections.Generic.HashSet[string]
$Queue = New-Object System.Collections.Generic.Queue[string]
foreach ($Seed in $Seeds) { $Queue.Enqueue($Seed) }

$Missing = @()
$Plan = @()
while ($Queue.Count -gt 0) {
    $Package = $Queue.Dequeue()
    if (-not $Visited.Add($Package)) { continue }
    if ($Package -match '^/Game/(Developers|Collections|__ExternalActors__|__ExternalObjects__)') { continue }

    $Source = Resolve-PackageFile $Package
    if ($null -eq $Source) { $Missing += $Package; continue }
    if ($Source.EndsWith(".umap")) { continue }

    $Plan += [pscustomobject]@{ Package = $Package; Source = $Source; Size = (Get-Item $Source).Length }
    foreach ($Ref in (Get-PackageReferences $Source)) {
        if (-not $Visited.Contains($Ref)) { $Queue.Enqueue($Ref) }
    }
}

$TotalMB = [math]::Round(($Plan | Measure-Object Size -Sum).Sum / 1MB, 1)
Write-Output ("{0} packages, {1} MB" -f $Plan.Count, $TotalMB)
foreach ($Item in ($Plan | Sort-Object Package)) {
    Write-Output ("{0,9:N0} KB  {1}" -f ($Item.Size / 1KB), $Item.Package)
}
if ($Missing.Count -gt 0) {
    Write-Output "--- unresolved (not in this pack; engine/plugin or other pack):"
    $Missing | Sort-Object | ForEach-Object { Write-Output "  $_" }
}

if ($DryRun) { exit 0 }

$Conflicts = @()
foreach ($Item in $Plan) {
    $Dest = Join-Path $DestContentRoot ($Item.Package.Substring("/Game/".Length).Replace('/', '\') + [System.IO.Path]::GetExtension($Item.Source))
    if (Test-Path $Dest) {
        $Existing = Get-Item $Dest
        if ($Existing.Length -ne $Item.Size) { $Conflicts += $Item.Package }
        continue
    }
    New-Item -ItemType Directory -Force -Path (Split-Path $Dest) | Out-Null
    Copy-Item $Item.Source $Dest
}
Write-Output ("Copied into {0}" -f $DestContentRoot)
if ($Conflicts.Count -gt 0) {
    Write-Output "--- CONFLICTS (destination exists with different size; NOT overwritten):"
    $Conflicts | ForEach-Object { Write-Output "  $_" }
    exit 2
}
