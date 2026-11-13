(event UserInterface|EventPreConstruct (IsDesignTime))

(event UserInterface|EventConstruct
  (Variables|Default|SetRandomIndex 1)
  (CallFunction|RefreshSelection)
  (CallFunction|ApplyScene))

(event UserInterface|EventTick (MyGeometry InDeltaTime)
  (Layout|CanvasSlot|SetOffsets (Slot|SlotAsCanvasSlot (Widget|GetParent (Widget|GetParent (Variables|WBP_NewGame|GetBtn_StartGame)))) (Utilities|Struct|MakeMargin (Math|Float|Max(Float) 0.0 (/ (- (/ (Math|Vector2D|BreakVector2D (Viewport|GetViewportSize)) (Viewport|GetViewportScale)) (* (/ (Math|Vector2D|BreakVector2D (Viewport|GetViewportSize)) (Viewport|GetViewportScale)) 1.7777778)) 2.0)) (Math|Float|Max(Float) 0.0 (/ (- (/ (Math|Vector2D|BreakVector2D (Viewport|GetViewportSize)) (Viewport|GetViewportScale)) (/ (/ (Math|Vector2D|BreakVector2D (Viewport|GetViewportSize)) (Viewport|GetViewportScale)) 1.7777778)) 2.0)) (Math|Float|Max(Float) 0.0 (/ (- (/ (Math|Vector2D|BreakVector2D (Viewport|GetViewportSize)) (Viewport|GetViewportScale)) (* (/ (Math|Vector2D|BreakVector2D (Viewport|GetViewportSize)) (Viewport|GetViewportScale)) 1.7777778)) 2.0)) (Math|Float|Max(Float) 0.0 (/ (- (/ (Math|Vector2D|BreakVector2D (Viewport|GetViewportSize)) (Viewport|GetViewportScale)) (/ (/ (Math|Vector2D|BreakVector2D (Viewport|GetViewportSize)) (Viewport|GetViewportScale)) 1.7777778)) 2.0)))))

(event OnClicked(Btn_Theme_0)
  (Variables|Default|SetThemeIndex 0)
  (CallFunction|RefreshSelection)
  (CallFunction|ApplyScene))

(event OnClicked(Btn_Theme_1)
  (Variables|Default|SetThemeIndex 1)
  (CallFunction|RefreshSelection)
  (CallFunction|ApplyScene))

(event OnClicked(Btn_Theme_2)
  (Variables|Default|SetThemeIndex 2)
  (CallFunction|RefreshSelection)
  (CallFunction|ApplyScene))

(event OnClicked(Btn_Theme_3)
  (Variables|Default|SetThemeIndex 3)
  (CallFunction|RefreshSelection)
  (CallFunction|ApplyScene))

(event OnClicked(Btn_Monsters_0)
  (Variables|Default|SetMonsterIndex 0)
  (CallFunction|RefreshSelection)
  (CallFunction|ApplyScene))

(event OnClicked(Btn_Monsters_1)
  (Variables|Default|SetMonsterIndex 1)
  (CallFunction|RefreshSelection)
  (CallFunction|ApplyScene))

(event OnClicked(Btn_Monsters_2)
  (Variables|Default|SetMonsterIndex 2)
  (CallFunction|RefreshSelection)
  (CallFunction|ApplyScene))

(event OnClicked(Btn_Monsters_3)
  (Variables|Default|SetMonsterIndex 3)
  (CallFunction|RefreshSelection)
  (CallFunction|ApplyScene))

(event OnClicked(Btn_Monsters_4)
  (Variables|Default|SetMonsterIndex 4)
  (CallFunction|RefreshSelection)
  (CallFunction|ApplyScene))

(event OnClicked(Btn_Monsters_5)
  (Variables|Default|SetMonsterIndex 5)
  (CallFunction|RefreshSelection)
  (CallFunction|ApplyScene))

(event OnClicked(Btn_Randomness_0)
  (Variables|Default|SetRandomIndex 0)
  (CallFunction|RefreshSelection)
  (CallFunction|ApplyScene))

(event OnClicked(Btn_Randomness_1)
  (Variables|Default|SetRandomIndex 1)
  (CallFunction|RefreshSelection)
  (CallFunction|ApplyScene))

(event OnClicked(Btn_Randomness_2)
  (Variables|Default|SetRandomIndex 2)
  (CallFunction|RefreshSelection)
  (CallFunction|ApplyScene))

(event OnClicked(Btn_StartGame)
  (Development|PrintString (Utilities|String|Append "Start Game - Theme " (Utilities|String|Append (Utilities|String|ToString(Integer) (Variables|Default|GetThemeIndex)) (Utilities|String|Append ", Monsters " (Utilities|String|Append (Utilities|String|ToString(Integer) (Variables|Default|GetMonsterIndex)) (Utilities|String|Append ", Randomness " (Utilities|String|ToString(Integer) (Variables|Default|GetRandomIndex))))))) true true "(R=0.000000,G=0.660000,B=1.000000,A=1.000000)" 5.0))
