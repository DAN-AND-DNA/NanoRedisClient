// Copyright 2022 Danyang Chen https://github.com/DAN-AND-DNA

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FNanoRedisClientModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
