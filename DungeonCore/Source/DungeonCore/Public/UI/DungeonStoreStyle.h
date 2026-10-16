#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture2D.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateBrush.h"
#include "Styling/SlateTypes.h"

/** Shared vault colors and rounded brushes. One palette so the HUD chip and the store match. */
namespace DungeonStoreStyle
{
    inline FLinearColor Ink()
    {
        return FLinearColor(0.04f, 0.032f, 0.045f, 0.96f);
    }

    inline FLinearColor InkRaised()
    {
        return FLinearColor(0.09f, 0.07f, 0.1f, 0.94f);
    }

    inline FLinearColor CardFill()
    {
        return FLinearColor(0.11f, 0.085f, 0.125f, 1.0f);
    }

    inline FLinearColor CardHover()
    {
        return FLinearColor(0.16f, 0.12f, 0.18f, 1.0f);
    }

    inline FLinearColor CardSelected()
    {
        return FLinearColor(0.18f, 0.12f, 0.24f, 1.0f);
    }

    inline FLinearColor ScrimTint()
    {
        return FLinearColor(0.012f, 0.008f, 0.02f, 0.78f);
    }

    inline FLinearColor Ivory()
    {
        return FLinearColor(0.95f, 0.92f, 0.86f, 1.0f);
    }

    inline FLinearColor Muted()
    {
        return FLinearColor(0.62f, 0.57f, 0.52f, 1.0f);
    }

    inline FLinearColor Gold()
    {
        return FLinearColor(0.93f, 0.76f, 0.42f, 1.0f);
    }

    inline FLinearColor GoldDim()
    {
        return FLinearColor(0.93f, 0.76f, 0.42f, 0.4f);
    }

    inline FLinearColor Violet()
    {
        return FLinearColor(0.74f, 0.48f, 1.0f, 1.0f);
    }

    inline FLinearColor Danger()
    {
        return FLinearColor(0.93f, 0.38f, 0.32f, 1.0f);
    }

    inline FLinearColor OwnedGreen()
    {
        return FLinearColor(0.48f, 0.84f, 0.6f, 1.0f);
    }

    inline FLinearColor InkText()
    {
        return FLinearColor(0.1f, 0.07f, 0.04f, 1.0f);
    }

    inline UTexture2D* WhiteSquare()
    {
        static TWeakObjectPtr<UTexture2D> CachedTexture;
        if (!CachedTexture.IsValid())
        {
            CachedTexture = LoadObject<UTexture2D>(nullptr, TEXT("/Engine/EngineResources/WhiteSquareTexture.WhiteSquareTexture"));
            if (!CachedTexture.IsValid())
            {
                UE_LOG(LogTemp, Error, TEXT("Vault UI missing /Engine/EngineResources/WhiteSquareTexture; rounded plates will not fill."));
            }
        }
        return CachedTexture.Get();
    }

    inline FSlateBrush MakeRoundedBrush(
        const FLinearColor& Fill,
        const FLinearColor& Outline,
        const float Radius,
        const float OutlineWidth
    )
    {
        FSlateBrush Brush;
        Brush.DrawAs = ESlateBrushDrawType::RoundedBox;
        Brush.ImageType = ESlateBrushImageType::FullColor;
        Brush.TintColor = FSlateColor(Fill);
        Brush.ImageSize = FVector2D(32.0, 32.0);
        Brush.OutlineSettings = FSlateBrushOutlineSettings(Radius, FSlateColor(Outline), OutlineWidth);
        Brush.SetResourceObject(WhiteSquare());
        return Brush;
    }

    inline FButtonStyle MakeButtonStyle(
        const FLinearColor& Fill,
        const FLinearColor& HoveredFill,
        const FLinearColor& PressedFill,
        const FLinearColor& Outline,
        const float Radius,
        const float OutlineWidth
    )
    {
        FButtonStyle Style;
        Style.Normal = MakeRoundedBrush(Fill, Outline, Radius, OutlineWidth);
        Style.Hovered = MakeRoundedBrush(HoveredFill, Outline, Radius, OutlineWidth);
        Style.Pressed = MakeRoundedBrush(PressedFill, Outline, Radius, OutlineWidth);
        const FLinearColor DisabledFill(Fill.R * 0.6f, Fill.G * 0.6f, Fill.B * 0.6f, Fill.A);
        Style.Disabled = MakeRoundedBrush(DisabledFill, Outline, Radius, OutlineWidth);
        Style.NormalPadding = FMargin(0.0f);
        Style.PressedPadding = FMargin(0.0f);
        return Style;
    }

    inline FSlateFontInfo MakeFont(const FName Typeface, const int32 Size, const int32 LetterSpacing)
    {
        FSlateFontInfo Font = FCoreStyle::GetDefaultFontStyle(Typeface, Size);
        Font.LetterSpacing = LetterSpacing;
        return Font;
    }
}
