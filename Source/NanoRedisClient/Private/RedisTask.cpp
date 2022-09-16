// Fill out your copyright notice in the Description page of Project Settings.


#include "RedisTask.h"
#include "HiRedisCpp.h"
#include "RedisClient.h"
#include "RedisTaskPool.h"
#include "RedisTransaction.h"
#include "Async/Async.h"


FRedisTask::FRedisTask(TWeakObjectPtr<URedisClient> InRedisClient, const FString& InCommand, FRedisTaskDoneDelegate InTaskDoneDelegate,  bool bTransaction) {
	const URedisClient* RedisClient = InRedisClient.Get();
	if (!IsValid(RedisClient)) {
		Data.bInit = false;
		return;
	}
	
	Data.bInit = true;
	Data.bPipeline = false;
	Data.bTransaction = bTransaction;
	Data.WeakOwnerTaskPool = RedisClient->RedisTaskPool;
	Data.WeakRedisClient = InRedisClient;
	Data.Command = InCommand;
	Data.TaskDoneDelegate = MoveTemp(InTaskDoneDelegate);
}

FRedisTask::FRedisTask(TWeakObjectPtr<class URedisClient> InRedisClient, const TArray<FString>& InCommands, FRedisTaskDonePipelineDelegate InTaskDonePipelineDelegate) {
	const URedisClient* RedisClient = InRedisClient.Get();
	if (!IsValid(RedisClient)) {
		Data.bInit = false;
		return;
	}
	
	Data.bPipeline = true;
	Data.bTransaction = false;
	Data.WeakOwnerTaskPool = RedisClient->RedisTaskPool;
	Data.WeakRedisClient = InRedisClient;
	Data.PipelineCommands = InCommands;
	Data.TaskDonePipelineDelegate = MoveTemp(InTaskDonePipelineDelegate);
	
	Data.bInit = true;
}

FRedisTask::~FRedisTask() {
}

void FRedisTask::DoThreadedWork() {
	if (!this->Data.bInit) {
		return;
	}
	
	const TSharedPtr<FRedisTaskPool> OwnerTaskPool = this->Data.WeakOwnerTaskPool.Pin();
	if (!OwnerTaskPool.IsValid()) {
		 return;
	}
	
	bool bGetFreeConn = false;
	TSharedPtr<FHiRedisCpp> HiRedisCpp;
	FString Err;
	TSharedPtr<FRedisTransactionJob> TransactionJob;

	if (this->Data.bTransaction) {
		
	} else {
		// 管道和普通
		{
			// 从池里拿连接
			FScopeLock ScopeLock(&OwnerTaskPool->Mutex);
			if (OwnerTaskPool->RedisCons.Num() > 0) {
				HiRedisCpp = OwnerTaskPool->RedisCons.Pop();
				bGetFreeConn = true;
			}
		}
	
		// 测试连接
		if (bGetFreeConn && !HiRedisCpp->Ping(Err)) {
			bGetFreeConn = false;
		}	
	}
	
	bool Ok = false;
	if (!bGetFreeConn) {
		
		// CONNECT
		HiRedisCpp = MakeShared<FHiRedisCpp>();
		Ok = HiRedisCpp->Connect(OwnerTaskPool->IP, OwnerTaskPool->Port, Err);
		if (!Ok) {
			AsyncTask(ENamedThreads::GameThread, [this, Ok, Err]{
				if (!this->Data.WeakRedisClient.IsValid()) {
					delete this;
					return;
				}
				// 调用任务关联的代理
				if (this->Data.bPipeline) {
					this->Data.TaskDonePipelineDelegate.ExecuteIfBound(Ok, Err, TArray<FReplyValue>());
				} else {
					this->Data.TaskDoneDelegate.ExecuteIfBound(Ok, Err, FReplyValue());	
				}
		
				// 最后删除任务自己
				delete this;
			});
			return;
		}

		// AUTH
		if (!OwnerTaskPool->Password.IsEmpty()) {
			Ok = HiRedisCpp->Auth(OwnerTaskPool->Password, 2, Err);
			if (!Ok) {
				AsyncTask(ENamedThreads::GameThread, [this, Ok, Err]{
					if (!this->Data.WeakRedisClient.IsValid()) {
						delete this;
						return;
					}
					// 调用任务关联的代理
					if (this->Data.bPipeline) {
						this->Data.TaskDonePipelineDelegate.ExecuteIfBound(Ok, Err, TArray<FReplyValue>());
					} else {
						this->Data.TaskDoneDelegate.ExecuteIfBound(Ok, Err, FReplyValue());	
					}
		
					// 最后删除任务自己
					delete this;
				});
				return;
			}	
		}
		
	}
	
	
	FReplyValue ReplyValue;
	TArray<FReplyValue> ReplyValues;
	
	if (this->Data.bPipeline) {
		// pipeline
		Ok = HiRedisCpp->ExecPipelineCommands(this->Data.PipelineCommands, ReplyValues, Err);
	} else if (this->Data.bTransaction) {
		
	} else {
		// normal
		Ok = HiRedisCpp->ExecCommand(this->Data.Command, ReplyValue, Err);
	}
	
	AsyncTask(ENamedThreads::GameThread, [this, Ok, ReplyValue, ReplyValues, Err]
	{
		if (!this->Data.WeakRedisClient.IsValid()) {
			delete this;
			return;
		}
		
		// 调用任务关联的代理
		if (this->Data.bPipeline) {
			this->Data.TaskDonePipelineDelegate.ExecuteIfBound(Ok, Err, ReplyValues);
		} else {
			this->Data.TaskDoneDelegate.ExecuteIfBound(Ok, Err, ReplyValue);	
		}
		
		// 最后删除任务自己
		delete this;
	});

	if (!this->Data.bTransaction) {
		// 放回池里
		FScopeLock ScopeLock(&OwnerTaskPool->Mutex);
		OwnerTaskPool->RedisCons.Add(HiRedisCpp);
	}
}

void FRedisTask::Abandon() {
	delete this;
}