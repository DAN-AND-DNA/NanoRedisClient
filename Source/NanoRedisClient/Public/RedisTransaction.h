// Copyright 2022 Danyang Chen https://github.com/DAN-AND-DNA

#pragma once

#include "CoreMinimal.h"
#include "RedisTaskData.h"
#include "UObject/Object.h"
#include "Runtime/Core/Public/HAL/Runnable.h"
#include "RedisTransaction.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class NANOREDISCLIENT_API URedisTransaction : public UObject {
	GENERATED_BODY()
	 
public:
	URedisTransaction();

	UFUNCTION(BlueprintCallable, Category = "NanoRedisClient")
	void Start(FRedisTaskDoneDelegate OnDone);

	UFUNCTION(BlueprintCallable, Category = "NanoRedisClient")
	void Queue(const FString& Command, FRedisTaskDoneDelegate OnDone);

	UFUNCTION(BlueprintCallable, Category = "NanoRedisClient")
	void Commit(FRedisTaskDoneDelegate OnDone);

	//UFUNCTION(BlueprintCallable)
	//void CleanUp(FRedisTaskDoneDelegate OnDone);
	
	TWeakObjectPtr<class URedisClient> RedisClient;

	TWeakPtr<class FRedisTaskPool> RedisTaskPool;
private:
	TSharedPtr<class FRedisTransactionJob> Job;
};

class FRedisTransactionJob: public FRunnable {
	friend URedisTransaction;
public:
	FRedisTransactionJob(const TWeakObjectPtr<URedisTransaction>& InOwnerTransaction, const TWeakObjectPtr<class URedisClient>& InWeakRedisClient);
	
	virtual ~FRedisTransactionJob() override;

	static TSharedPtr<FRedisTransactionJob> RunNewJob(const TWeakObjectPtr<URedisTransaction>& InOwnerTransaction, const TWeakObjectPtr<class URedisClient>& InWeakRedisClient);

	virtual bool Init() override;
	
	virtual uint32 Run() override;
	
	virtual void Stop() override;

	virtual void Exit() override;

	void AddTask(const TSharedPtr<class FRedisTask>& Task);

	void ProcessTask(const TSharedPtr<class FRedisTask>& Task);
private:
	bool bRun;
	
	FRedisTaskDoneDelegate OnDone;
	
	static int32 BaseId;
	
	class FRunnableThread* WorkerThread;

	TWeakObjectPtr<class URedisClient> WeakRedisClient;

	TWeakObjectPtr<URedisTransaction> OwnerTransaction;

	TArray<TSharedPtr<class FRedisTask>> TaskQueue;

	class FEvent* Event;

	FCriticalSection Mutex;
	
	TSharedPtr<class FHiRedisCpp> RedisCon;
};
