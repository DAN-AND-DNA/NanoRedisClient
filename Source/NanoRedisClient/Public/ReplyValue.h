// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RedisDefine.h"
#include "ReplyValue.generated.h"


USTRUCT(BlueprintType)
struct NANOREDISCLIENT_API FListReplyValue {
	GENERATED_BODY()
	
	FListReplyValue();
	
	void ParserReply(const struct redisReply* Reply);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NanoRedisClient")
	EReplyValueType Type;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NanoRedisClient")
	int Integer;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NanoRedisClient")
	FString String;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NanoRedisClient")
	TArray<FString> List;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NanoRedisClient")
	FString Status;
};

/**
 * 
 */
USTRUCT(BlueprintType)
struct NANOREDISCLIENT_API FReplyValue {
	GENERATED_BODY()
	
	FReplyValue();
	
	void ParserReply(const struct redisReply* Reply);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NanoRedisClient")
	EReplyValueType Type;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NanoRedisClient")
	int Integer;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NanoRedisClient")
	FString String;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NanoRedisClient")
	TArray<FListReplyValue> List;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NanoRedisClient")
	FString Status;
};





