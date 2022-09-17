// Copyright 2022 Danyang Chen https://github.com/DAN-AND-DNA

#pragma once

#include "CoreMinimal.h"
#include "RedisTaskData.h"
#include "UObject/Object.h"
#include "RedisClient.generated.h"


/**
 * 
 */
UCLASS(BlueprintType)
class NANOREDISCLIENT_API URedisClient : public UObject {
	GENERATED_BODY()

public:
	URedisClient();

	UFUNCTION(BlueprintCallable, Category = "NanoRedisClient")
	static UPARAM(DisplayName = "RedisClient") URedisClient* NewRedisClient(int MaxNum = 3, const FString& InIP = FString(TEXT("127.0.0.1")), int InPort = 6379, const FString& InPassword = FString(TEXT("")));
	
	UFUNCTION(BlueprintCallable, Category = "NanoRedisClient")
	UPARAM(DisplayName = "Redis Pipeline") class URedisPipeline* NewPipeline();

	UFUNCTION(BlueprintCallable, Category = "NanoRedisClient")
	UPARAM(DisplayName = "Redis Transaction") class URedisTransaction* NewTransaction();

	UFUNCTION(BlueprintCallable, Category = "NanoRedisClient")
	void Ping (FRedisTaskDoneDelegate OnDone);

	UFUNCTION(BlueprintCallable, Category = "NanoRedisClient")
	void Raw (const FString& Command, FRedisTaskDoneDelegate OnDone);
	
	TSharedPtr<class FRedisTaskPool> RedisTaskPool;

	FString IP;

	int Port;

	FString Password;
};
