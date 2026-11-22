# Ping Blender MCP add-on on localhost (no python -c quoting issues).
$ErrorActionPreference = "Stop"

$TmpDir = $PSScriptRoot
$env:BLENDER_MCP_HOST = "127.0.0.1"
$env:BLENDER_MCP_PORT = "9876"
$env:BLENDER_MCP_TIMEOUT = "30"

$PingScript = Join-Path $TmpDir "blender-mcp-ping.py"
$UvDir = Join-Path $TmpDir "blender-mcpb-1.0.3"

Push-Location $UvDir
try {
    uv run python $PingScript
    exit $LASTEXITCODE
}
finally {
    Pop-Location
}