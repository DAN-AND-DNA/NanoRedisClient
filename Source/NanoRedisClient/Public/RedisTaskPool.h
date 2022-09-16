﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"


/**
 * 
 */
class FRedisTaskPool {
public:
	FRedisTaskPool(const FString& InIP, int InPort, const FString& InPassword, int MaxNum = 3);
	
	~FRedisTaskPool();

	void Dispatch(class FRedisTask* Task);
	
	FCriticalSection Mutex;
	
	TArray<TSharedPtr<class FHiRedisCpp>> RedisCons;

	FString IP;
	
	int Port;

	FString Password;
private:
	class FQueuedThreadPool* ThreadPool;
};
