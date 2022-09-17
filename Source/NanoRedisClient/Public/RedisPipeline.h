// Copyright 2022 Danyang Chen https://github.com/DAN-AND-DNA

#pragma once

#include "CoreMinimal.h"
#include "RedisTaskData.h"
#include "UObject/Object.h"
#include "RedisPipeline.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class NANOREDISCLIENT_API URedisPipeline : public UObject {
	GENERATED_BODY()
public:
	URedisPipeline();

	UFUNCTION(BlueprintCallable, Category = "NanoRedisClient")
	void Start();

	UFUNCTION(BlueprintCallable, Category = "NanoRedisClient")
	UPARAM(DisplayName = "Ok") bool Exec(const FString& Command, FString& Err);

	UFUNCTION(BlueprintCallable, Category = "NanoRedisClient")
	void Commit(FRedisTaskDonePipelineDelegate OnDone);
	
	TWeakPtr<class FRedisTaskPool> RedisTaskPool;
	
	TWeakObjectPtr<class URedisClient> RedisClient;
private:
	bool bStartPipeline;
	
	TArray<FString> PipelineCommands;
};
