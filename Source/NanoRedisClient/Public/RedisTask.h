﻿// Copyright 2022 Danyang Chen https://github.com/DAN-AND-DNA

#pragma once

#include "CoreMinimal.h"
#include "RedisTaskData.h"
#include "Misc/IQueuedWork.h"
/**
 * 
 */
class FRedisTask : public IQueuedWork {
	
public:
	// for normal and transaction
	explicit FRedisTask(TWeakObjectPtr<class URedisClient> InRedisClient, const FString& InCommand, FRedisTaskDoneDelegate InTaskDoneDelegate, bool bTransaction = false);

	// for pipeline
	explicit FRedisTask(TWeakObjectPtr<class URedisClient> InRedisClient, const TArray<FString>& InCommands, FRedisTaskDonePipelineDelegate InTaskDonePipelineDelegate);
	
	virtual ~FRedisTask() override;
	
	virtual void DoThreadedWork() override;
	
	virtual void Abandon() override;

	FRedisTaskData Data;
};