// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ReplyValue.h"
#include "RedisTaskData.generated.h"

DECLARE_DYNAMIC_DELEGATE_ThreeParams(FRedisTaskDoneDelegate, bool, Ok, const FString&, Err, const FReplyValue&, Value);
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FRedisTaskDonePipelineDelegate, bool, Ok, const FString&, Err, const TArray<FReplyValue>&, Values);
/**
 * 
 */
USTRUCT()
struct NANOREDISCLIENT_API FRedisTaskData {
	GENERATED_BODY()

	bool bInit;
	bool bPipeline;
	bool bTransaction;
 
	TWeakObjectPtr<class URedisClient> WeakRedisClient;
	TWeakPtr<class FRedisTaskPool> WeakOwnerTaskPool;
	FString Command;
	FRedisTaskDoneDelegate TaskDoneDelegate;
	TArray<FString> PipelineCommands;
	FRedisTaskDonePipelineDelegate TaskDonePipelineDelegate;
};
