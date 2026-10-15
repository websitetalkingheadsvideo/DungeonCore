#Requires -Version 5.1
param([switch]$Check)
$ErrorActionPreference = 'Stop'
$py = Join-Path $PSScriptRoot 'update_library_table.py'
$args = @($py)
if ($Check) { $args += '--check' }
& python @args
exit $LASTEXITCODE
