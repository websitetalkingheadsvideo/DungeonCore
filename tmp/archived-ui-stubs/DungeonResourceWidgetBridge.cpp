#include "UI/DungeonResourceWidgetBridge.h"

#include "Sim/DungeonSimSubsystem.h"

void UDungeonResourceWidgetBridge::BindToSimulation(UDungeonSimSubsystem* DungeonSimSubsystem, UObject* InOwner)
{
    UnbindFromSimulation();

    if (DungeonSimSubsystem == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("DungeonResourceWidgetBridge requires a sim subsystem."));
        return;
    }

    Owner = InOwner;
    BoundSim = DungeonSimSubsystem;

    DungeonSimSubsystem->OnResourceChanged.AddDynamic(this, &UDungeonResourceWidgetBridge::ForwardResourceChanged);
    DungeonSimSubsystem->OnBreachStateChanged.AddDynamic(this, &UDungeonResourceWidgetBridge::ForwardBreachStateChanged);
    DungeonSimSubsystem->OnWaveChanged.AddDynamic(this, &UDungeonResourceWidgetBridge::ForwardWaveChanged);
    DungeonSimSubsystem->OnCoreHealthChanged.AddDynamic(this, &UDungeonResourceWidgetBridge::ForwardCoreHealthChanged);
    DungeonSimSubsystem->OnRunOutcomeChanged.AddDynamic(this, &UDungeonResourceWidgetBridge::ForwardRunOutcomeChanged);
}

void UDungeonResourceWidgetBridge::UnbindFromSimulation()
{
    if (UDungeonSimSubsystem* DungeonSimSubsystem = BoundSim.Get())
    {
        DungeonSimSubsystem->OnResourceChanged.RemoveDynamic(this, &UDungeonResourceWidgetBridge::ForwardResourceChanged);
        DungeonSimSubsystem->OnBreachStateChanged.RemoveDynamic(this, &UDungeonResourceWidgetBridge::ForwardBreachStateChanged);
        DungeonSimSubsystem->OnWaveChanged.RemoveDynamic(this, &UDungeonResourceWidgetBridge::ForwardWaveChanged);
        DungeonSimSubsystem->OnCoreHealthChanged.RemoveDynamic(this, &UDungeonResourceWidgetBridge::ForwardCoreHealthChanged);
        DungeonSimSubsystem->OnRunOutcomeChanged.RemoveDynamic(this, &UDungeonResourceWidgetBridge::ForwardRunOutcomeChanged);
    }

    BoundSim = nullptr;
    Owner = nullptr;
}

void UDungeonResourceWidgetBridge::ForwardResourceChanged(const FResourceState ResourceState)
{
    OnResourceChanged.Broadcast(ResourceState);
}

void UDungeonResourceWidgetBridge::ForwardBreachStateChanged(const EBreachState BreachState)
{
    OnBreachStateChanged.Broadcast(BreachState);
}

void UDungeonResourceWidgetBridge::ForwardWaveChanged(const int32 CurrentWaveNumber)
{
    OnWaveChanged.Broadcast(CurrentWaveNumber);
}

void UDungeonResourceWidgetBridge::ForwardCoreHealthChanged(const int32 CurrentCoreHealth)
{
    OnCoreHealthChanged.Broadcast(CurrentCoreHealth);
}

void UDungeonResourceWidgetBridge::ForwardRunOutcomeChanged(const ERunOutcome RunOutcome)
{
    OnRunOutcomeChanged.Broadcast(RunOutcome);
}
