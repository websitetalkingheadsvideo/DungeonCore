#pragma once

#include "CoreMinimal.h"
#include "Components/Button.h"

#include "DungeonShopItemButton.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDungeonShopItemClickedSignature, FName, BuildableId);

/** UButton that remembers which catalog item it sells, so one HUD handler can serve every shop row. */
UCLASS()
class DUNGEONCORE_API UDungeonShopItemButton : public UButton
{
    GENERATED_BODY()

public:
    UDungeonShopItemButton();

    void SetBuildableId(const FName InBuildableId);

    FName GetBuildableId() const;

    UPROPERTY(BlueprintAssignable, Category = "Dungeon")
    FDungeonShopItemClickedSignature OnShopItemClicked;

private:
    UFUNCTION()
    void HandleClicked();

    UPROPERTY()
    FName BuildableId;
};
