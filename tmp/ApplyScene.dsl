(fn ApplyScene ()
  (bind _monsterindex (Variables|Default|GetMonsterIndex))
  (bind _themeindex (Variables|Default|GetThemeIndex))
  (CallFunction|ApplyDressing)
  (bind _outactors (Actor|GetAllActorsWithTag "Creature_0"))
  (for _array_element _outactors
    (Rendering|SetActorHiddenInGame _array_element (!= _monsterindex 0)))
  (bind _outactors_1 (Actor|GetAllActorsWithTag "Creature_1"))
  (for _array_element_1 _outactors_1
    (Rendering|SetActorHiddenInGame _array_element_1 (!= _monsterindex 1)))
  (bind _outactors_2 (Actor|GetAllActorsWithTag "Creature_2"))
  (for _array_element_2 _outactors_2
    (Rendering|SetActorHiddenInGame _array_element_2 (!= _monsterindex 2)))
  (bind _outactors_3 (Actor|GetAllActorsWithTag "Creature_3"))
  (for _array_element_3 _outactors_3
    (Rendering|SetActorHiddenInGame _array_element_3 (!= _monsterindex 3)))
  (bind _outactors_4 (Actor|GetAllActorsWithTag "Creature_4"))
  (for _array_element_4 _outactors_4
    (Rendering|SetActorHiddenInGame _array_element_4 (!= _monsterindex 4)))
  (bind _outactors_5 (Actor|GetAllActorsWithTag "Creature_5"))
  (for _array_element_5 _outactors_5
    (Rendering|SetActorHiddenInGame _array_element_5 (!= _monsterindex 5)))
  (bind _outactors_6 (Actor|GetAllActorsWithTag "ThemeLight"))
  (for _array_element_6 _outactors_6
    (Rendering|Components|Light|SetLightColor (Actor|GetComponentByClass _array_element_6 "/Script/Engine.PointLightComponent") (select (== _themeindex 0) (Utilities|Struct|MakeLinearColor 1.0 0.78 0.45 1.0) (select (== _themeindex 1) (Utilities|Struct|MakeLinearColor 1.0 0.55 0.2 1.0) (select (== _themeindex 2) (Utilities|Struct|MakeLinearColor 0.35 1.0 0.45 1.0) (Utilities|Struct|MakeLinearColor 0.2 0.9 0.8 1.0)))) false))
  (bind _outactors_7 (Actor|GetAllActorsWithTag "ThemeWall"))
  (bind _outactors_8 (Actor|GetAllActorsWithTag "ThemeFloor"))
  (bind _outactors_9 (Actor|GetAllActorsWithTag "ThemeCeiling"))
  (switch Utilities|FlowControl|Switch|SwitchonInt _themeindex
    (:Default
      (for _array_element_7 _outactors_7
        (Rendering|Material|SetMaterial (Actor|GetComponentByClass _array_element_7 "/Script/Engine.StaticMeshComponent") 0 "/Game/Environments/AncientCrypt/Materials/MI_blind_Arch_1.MI_blind_Arch_1"))
      (for _array_element_8 _outactors_8
        (Rendering|Material|SetMaterial (Actor|GetComponentByClass _array_element_8 "/Script/Engine.StaticMeshComponent") 0 "/Game/Environments/AncientCrypt/Materials/MI_Water_Inst.MI_Water_Inst"))
      (for _array_element_9 _outactors_9
        (Rendering|Material|SetMaterial (Actor|GetComponentByClass _array_element_9 "/Script/Engine.StaticMeshComponent") 0 "/Game/Environments/AncientCrypt/Materials/M_WA_Inst.M_WA_Inst")))
    (:0
      (for _array_element_7 _outactors_7
        (Rendering|Material|SetMaterial (Actor|GetComponentByClass _array_element_7 "/Script/Engine.StaticMeshComponent") 0 "/Game/Maps/Materials/MI_NG_Wall.MI_NG_Wall"))
      (for _array_element_8 _outactors_8
        (Rendering|Material|SetMaterial (Actor|GetComponentByClass _array_element_8 "/Script/Engine.StaticMeshComponent") 0 "/Game/Maps/Materials/MI_NG_Floor.MI_NG_Floor"))
      (for _array_element_9 _outactors_9
        (Rendering|Material|SetMaterial (Actor|GetComponentByClass _array_element_9 "/Script/Engine.StaticMeshComponent") 0 "/Game/Maps/Materials/MI_NG_Wall.MI_NG_Wall")))
    (:1
      (for _array_element_7 _outactors_7
        (Rendering|Material|SetMaterial (Actor|GetComponentByClass _array_element_7 "/Script/Engine.StaticMeshComponent") 0 "/Game/Environments/Scene_OldMine/Assets/Megascans/Surfaces/Wall_Mine_Rock_01/MI_Wall_Mine_Rock_01_A.MI_Wall_Mine_Rock_01_A"))
      (for _array_element_8 _outactors_8
        (Rendering|Material|SetMaterial (Actor|GetComponentByClass _array_element_8 "/Script/Engine.StaticMeshComponent") 0 "/Game/Environments/Scene_OldMine/Assets/Megascans/Surfaces/Floor_Mine_Coal_01/MI_Floor_Mine_Coal_01.MI_Floor_Mine_Coal_01"))
      (for _array_element_9 _outactors_9
        (Rendering|Material|SetMaterial (Actor|GetComponentByClass _array_element_9 "/Script/Engine.StaticMeshComponent") 0 "/Game/Environments/Scene_OldMine/Assets/Megascans/Surfaces/Wall_Mine_Rock_01/MI_Wall_Mine_Rock_01_B.MI_Wall_Mine_Rock_01_B")))
    (:2
      (for _array_element_7 _outactors_7
        (Rendering|Material|SetMaterial (Actor|GetComponentByClass _array_element_7 "/Script/Engine.StaticMeshComponent") 0 "/Game/Environments/AncientCrypt/Materials/M_WA_Inst2.M_WA_Inst2"))
      (for _array_element_8 _outactors_8
        (Rendering|Material|SetMaterial (Actor|GetComponentByClass _array_element_8 "/Script/Engine.StaticMeshComponent") 0 "/Game/Environments/AncientCrypt/Materials/M_TileableAsset_Irregularstone.M_TileableAsset_Irregularstone"))
      (for _array_element_9 _outactors_9
        (Rendering|Material|SetMaterial (Actor|GetComponentByClass _array_element_9 "/Script/Engine.StaticMeshComponent") 0 "/Game/Environments/AncientCrypt/Materials/M_TileableAsset_Inst7.M_TileableAsset_Inst7")))))
