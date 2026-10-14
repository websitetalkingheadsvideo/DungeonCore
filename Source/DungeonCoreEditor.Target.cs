using UnrealBuildTool;
using System.Collections.Generic;

public class DungeonCoreEditorTarget : TargetRules
{
    public DungeonCoreEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V7;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        ExtraModuleNames.AddRange(new string[] { "DungeonCore" });
    }
}
