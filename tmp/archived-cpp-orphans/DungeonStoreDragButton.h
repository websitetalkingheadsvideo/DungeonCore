#pragma once

#include "CoreMinimal.h"
#include "Components/Button.h"
#include "DungeonStoreDragButton.generated.h"

class UDungeonLegacyStoreHudWidget;

UCLASS()
class DUNGEONCORE_API UDungeonStoreDragButton : public UButton
{
    GENERATED_BODY()

public:
    void InitStoreDrag(UDungeonLegacyStoreHudWidget* InHud, const int32 InSlotIndex);

protected:
    virtual FReply NativeOnMouseButtonDown(
        const FGeometry& InGeometry,
        const FPointerEvent& InMouseEvent) override;

    virtual void NativeOnDragDetected(
        const FGeometry& InGeometry,
        const FPointerEvent& InMouseEvent,
        UDragDropOperation*& OutOperation) override;

private:
    UPROPERTY()
    TObjectPtr<UDungeonLegacyStoreHudWidget> OwningHud;

    int32 StoreSlotIndex = -1;
};