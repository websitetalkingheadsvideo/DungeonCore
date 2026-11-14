(fn TickStore ()
  (bind _hud (Variables|Default|GetHUD))
  (bind _selecteditem (Variables|Default|GetSelectedItem))
  (bind _index (Class|WBPDungeonHUD|GetHoveredItem _hud))
  (Variables|Default|SetHoveredItem _index)
  (Variables|Default|SetMsgTimer (- (Variables|Default|GetMsgTimer) (Utilities|Time|GetWorldDeltaSeconds)))
  (Class|WBPDungeonHUD|Highlight _hud _index _selecteditem)
  (Class|WBPDungeonHUD|SetStatus _hud
    (select (> (Variables|Default|GetMsgTimer) 0.0)
      (Variables|Default|GetStatusMsg)
      (select (>= _selecteditem 0)
        (Utilities|String|Append "Placing: "
          (Utilities|String|Append
            (Utilities|Array|Get(acopy) (Variables|Default|GetItemNames)
              (Math|Integer|Max(Integer) _selecteditem))
            " - drop on a room square"))
        "Drag a store item onto a room square")))
  (CallFunction|UpdateCursorCell)
  (if (>= _selecteditem 0)
    (CallFunction|PreviewStoreDrag)
    (else
      (CallFunction|OnLeftClick))))