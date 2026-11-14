#include "UI/DungeonStoreDragButton.h"

#include "InputCoreTypes.h"
#include "UI/DungeonLegacyStoreHudWidget.h"

void UDungeonStoreDragButton::InitStoreDrag(UDungeonLegacyStoreHudWidget* InHud, const int32 InSlotIndex)
{
    OwningHud = InHud;
    StoreSlotIndex = InSlotIndex;
}

FReply UDungeonStoreDragButton::NativeOnMouseButtonDown(
    const FGeometry& InGeometry,
    const FPointerEvent& InMouseEvent)
{
    const FReply Reply = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
    if (Reply.IsEventHandled())
    {
        return DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton);
    }

    return Reply;
}

void UDungeonStoreDragButton::NativeOnDragDetected(
    const FGeometry& InGeometry,
    const FPointerEvent& InMouseEvent,
    UDragDropOperation*& OutOperation)
{
    Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

    if (OwningHud == nullptr || StoreSlotIndex < 0)
    {
        return;
    }

    OutOperation = OwningHud->CreateStoreDragOperation(StoreSlotIndex);
}