; ArmStoreDrag(ItemIndex) -> bool
(fn ArmStoreDrag (ItemIndex)
  (bind _itemcosts (Variables|Default|GetItemCosts))
  (bind _output (Utilities|Array|Get(acopy) _itemcosts ItemIndex))
  (if (and (>= ItemIndex 0) (>= (Variables|Default|GetMana) _output))
    (Variables|Default|SetSelectedItem ItemIndex)
    (return true)
    (else
      (Variables|Default|SetStatusMsg "Not enough mana for this item.")
      (Variables|Default|SetMsgTimer 3.0)
      (return false))))

(fn CancelStoreDrag ()
  (Variables|Default|SetSelectedItem -1)
  (Transformation|SetRelativeLocation (Variables|Default|GetGhost) (Math|Vector|MakeVector 0.0 0.0 -5000.0)))

(fn PreviewStoreDrag ()
  (CallFunction|UpdateCursorCell)
  (bind _selecteditem (Variables|Default|GetSelectedItem))
  (if (>= _selecteditem 0)
    (Transformation|SetRelativeLocation (Variables|Default|GetGhost) (Math|Vector|MakeVector (* (- (Math|Conversions|ToFloat(Integer) (Variables|Default|GetCursorX)) (* (Math|Conversions|ToFloat(Integer) (- (Variables|Default|GetGridSize) 1)) 0.5)) (Variables|Default|GetCellSize)) (* (- (Math|Conversions|ToFloat(Integer) (Variables|Default|GetCursorY)) (* (Math|Conversions|ToFloat(Integer) (- (Variables|Default|GetGridSize) 1)) 0.5)) (Variables|Default|GetCellSize)) (select (Variables|Default|GetCursorOk) 3.0 -5000.0)))))

(fn CommitStoreDrag ()
  (bind _selecteditem (Variables|Default|GetSelectedItem))
  (Transformation|SetRelativeLocation (Variables|Default|GetGhost) (Math|Vector|MakeVector 0.0 0.0 -5000.0))
  (if (and (>= _selecteditem 0) (Variables|Default|GetCursorOk))
    (CallFunction|TryPlace :X (Variables|Default|GetCursorX) :Y (Variables|Default|GetCursorY))
    (Variables|Default|SetSelectedItem -1)
    (return true)
    (else
      (Variables|Default|SetSelectedItem -1)
      (return false))))