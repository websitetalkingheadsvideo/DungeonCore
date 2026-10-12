#include "Game/DungeonGameMode.h"

#include "Engine/World.h"
#include "EngineUtils.h"
#include "Player/DungeonCameraPawn.h"
#include "Player/DungeonPlayerController.h"
#include "World/DungeonChunkActor.h"

ADungeonGameMode::ADungeonGameMode()
{
    DefaultPawnClass = ADungeonCameraPawn::StaticClass();
    PlayerControllerClass = ADungeonPlayerController::StaticClass();
}

void ADungeonGameMode::BeginPlay()
{
    Super::BeginPlay();

    UWorld* World = GetWorld();
    if (World == nullptr)
    {
        return;
    }

    for (TActorIterator<ADungeonChunkActor> It(World); It; ++It)
    {
        return;
    }

    World->SpawnActor<ADungeonChunkActor>(ADungeonChunkActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
}
