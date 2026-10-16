#pragma once

#include "CoreMinimal.h"

#ifndef DUNGEONCORE_LOG_CATEGORY_DEFINED
#define DUNGEONCORE_LOG_CATEGORY_DEFINED

struct FLogCategoryLogDungeonCore : public FLogCategory<ELogVerbosity::Log, ELogVerbosity::All>
{
    UE_FORCEINLINE_HINT FLogCategoryLogDungeonCore()
        : FLogCategory(TEXT("LogDungeonCore"))
    {
    }
};

extern FLogCategoryLogDungeonCore LogDungeonCore;

#endif
