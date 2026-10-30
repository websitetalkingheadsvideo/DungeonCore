#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture2D.h"
#include "Fonts/CompositeFont.h"
#include "Fonts/SlateFontInfo.h"
#include "Misc/Paths.h"
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

    /**
     * Typeface names inside the fantasy composite font. Tiers follow Design/fonts.md:
     * titles (Cinzel), telemetry (Orbitron digits, Rajdhani labels), body copy (Sorts Mill Goudy).
     */
    inline FName FantasyRegular() { return FName(TEXT("Regular")); }
    inline FName FantasyBold() { return FName(TEXT("Bold")); }
    /** Cinzel Decorative: titles and headings only. */
    inline FName FantasyDisplay() { return FName(TEXT("Display")); }
    /** Orbitron: resource counters, costs, coordinates. */
    inline FName Telemetry() { return FName(TEXT("Telemetry")); }
    inline FName TelemetryBold() { return FName(TEXT("TelemetryBold")); }
    /** Rajdhani: compact panel labels, kickers, readouts. */
    inline FName Panel() { return FName(TEXT("Panel")); }
    inline FName PanelBold() { return FName(TEXT("PanelBold")); }
    /** Sorts Mill Goudy: descriptions, tooltips, lore. */
    inline FName Body() { return FName(TEXT("Body")); }
    inline FName BodyItalic() { return FName(TEXT("BodyItalic")); }

    /**
     * All faces are SIL OFL TTFs shipped raw under Content/UI/Fonts and staged loose via DefaultGame.ini,
     * so no font .uasset is required. Built once; Slate caches the FreeType faces.
     */
    inline TSharedPtr<const FCompositeFont> FantasyCompositeFont()
    {
        static TSharedPtr<const FCompositeFont> CachedFont;
        if (CachedFont.IsValid())
        {
            return CachedFont;
        }

        struct FFace
        {
            FName Typeface;
            const TCHAR* FileName;
        };
        const FFace Faces[] = {
            {FantasyRegular(), TEXT("Cinzel-Regular.ttf")},
            {FantasyBold(), TEXT("Cinzel-Bold.ttf")},
            {FantasyDisplay(), TEXT("CinzelDecorative-Bold.ttf")},
            {Telemetry(), TEXT("Orbitron-Regular.ttf")},
            {TelemetryBold(), TEXT("Orbitron-Bold.ttf")},
            {Panel(), TEXT("Rajdhani-Regular.ttf")},
            {PanelBold(), TEXT("Rajdhani-SemiBold.ttf")},
            {Body(), TEXT("SortsMillGoudy-Regular.ttf")},
            {BodyItalic(), TEXT("SortsMillGoudy-Italic.ttf")},
        };

        const FString FontDirectory = FPaths::ProjectContentDir() / TEXT("UI/Fonts");
        TSharedPtr<FStandaloneCompositeFont> Font;
        for (const FFace& Face : Faces)
        {
            const FString FontPath = FontDirectory / Face.FileName;
            if (!FPaths::FileExists(FontPath))
            {
                UE_LOG(LogTemp, Error, TEXT("Fantasy UI font missing on disk: %s"), *FontPath);
            }
            if (!Font.IsValid())
            {
                Font = MakeShared<FStandaloneCompositeFont>(Face.Typeface, FontPath, EFontHinting::Default, EFontLoadingPolicy::LazyLoad);
                continue;
            }
            Font->DefaultTypeface.AppendFont(Face.Typeface, FontPath, EFontHinting::Default, EFontLoadingPolicy::LazyLoad);
        }
        CachedFont = Font;
        return CachedFont;
    }

    inline FSlateFontInfo MakeFantasyFont(const FName Typeface, const int32 Size, const int32 LetterSpacing)
    {
        FSlateFontInfo Font(FantasyCompositeFont(), static_cast<float>(Size), Typeface);
        Font.LetterSpacing = LetterSpacing;
        return Font;
    }
}
