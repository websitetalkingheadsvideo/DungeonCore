#include "UI/DungeonShopItemButton.h"

UDungeonShopItemButton::UDungeonShopItemButton()
{
}

void UDungeonShopItemButton::SetBuildableId(const FName InBuildableId)
{
    BuildableId = InBuildableId;

    // Bound here rather than in the constructor so the CDO never ends up as the delegate target.
    if (!OnClicked.IsAlreadyBound(this, &UDungeonShopItemButton::HandleClicked))
    {
        OnClicked.AddDynamic(this, &UDungeonShopItemButton::HandleClicked);
    }
}

FName UDungeonShopItemButton::GetBuildableId() const
{
    return BuildableId;
}

void UDungeonShopItemButton::HandleClicked()
{
    OnShopItemClicked.Broadcast(BuildableId);
}
