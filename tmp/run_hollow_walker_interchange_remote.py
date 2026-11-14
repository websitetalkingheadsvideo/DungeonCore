"""Discover open UE editor and run Interchange GLB import via Python remote execution."""
from __future__ import annotations

import json
import sys
import time
from pathlib import Path

REMOTE = Path(
    r"C:\Program Files\Epic Games\UE_5.8\Engine\Plugins\Experimental\PythonScriptPlugin\Content\Python"
)
sys.path.insert(0, str(REMOTE))

from remote_execution import (  # noqa: E402
    MODE_EXEC_FILE,
    RemoteExecution,
    RemoteExecutionConfig,
)

SCRIPT = Path(r"e:\Dungeon Core\tmp\import_hollow_walker_interchange.py")
MANIFEST = Path(r"e:\Dungeon Core\tmp\hollow-walker-interchange-manifest.json")


def main() -> None:
    if not SCRIPT.is_file():
        raise SystemExit(f"Missing {SCRIPT}")
    remote = RemoteExecution(RemoteExecutionConfig())
    remote.start()
    deadline = time.time() + 15.0
    nodes: list = []
    while time.time() < deadline and not nodes:
        nodes = remote.remote_nodes
        time.sleep(0.3)
    if not nodes:
        remote.stop()
        raise SystemExit(
            "No Unreal Editor Python remote node discovered. "
            "Enable Python Editor Script Plugin + remote execution in the open editor, then retry."
        )
    node_id = nodes[0]["node_id"]
    remote.open_command_connection(node_id)
    output = remote.run_command(
        str(SCRIPT),
        unattended=True,
        exec_mode=MODE_EXEC_FILE,
        raise_on_failure=True,
    )
    remote.stop()
    manifest = {"node_id": node_id, "output": output}
    MANIFEST.write_text(json.dumps(manifest, indent=2), encoding="utf-8")
    print(json.dumps(manifest, indent=2))


if __name__ == "__main__":
    main()