#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Sim/DungeonSimSubsystem.h"

#include "DungeonResourceWidgetBridge.generated.h"

class UDungeonSimSubsystem;

/**
 * Forwards dungeon sim multicast delegates to a single owner widget so UMG stays event-driven
 * without polling authoritative state.
 */
UCLASS()
class DUNGEONCORE_API UDungeonResourceWidgetBridge : public UObject
{
    GENERATED_BODY()

public:
    void BindToSimulation(UDungeonSimSubsystem* DungeonSimSubsystem, UObject* InOwner);

    void UnbindFromSimulation();

    UPROPERTY(BlueprintAssignable, Category = "Dungeon")
    FDungeonResourceChangedSignature OnResourceChanged;

    UPROPERTY(BlueprintAssignable, Category = "Dungeon")
    FDungeonBreachStateChangedSignature OnBreachStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Dungeon")
    FDungeonWaveChangedSignature OnWaveChanged;

    UPROPERTY(BlueprintAssignable, Category = "Dungeon")
    FDungeonCoreHealthChangedSignature OnCoreHealthChanged;

    UPROPERTY(BlueprintAssignable, Category = "Dungeon")
    FDungeonRunOutcomeChangedSignature OnRunOutcomeChanged;

private:
    UFUNCTION()
    void ForwardResourceChanged(FResourceState ResourceState);

    UFUNCTION()
    void ForwardBreachStateChanged(EBreachState BreachState);

    UFUNCTION()
    void ForwardWaveChanged(int32 CurrentWaveNumber);

    UFUNCTION()
    void ForwardCoreHealthChanged(int32 CurrentCoreHealth);

    UFUNCTION()
    void ForwardRunOutcomeChanged(ERunOutcome RunOutcome);

    UPROPERTY()
    TWeakObjectPtr<UDungeonSimSubsystem> BoundSim;

    UPROPERTY()
    TWeakObjectPtr<UObject> Owner;
};
