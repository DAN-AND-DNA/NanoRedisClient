// Copyright 2022 Danyang Chen https://github.com/DAN-AND-DNA

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
