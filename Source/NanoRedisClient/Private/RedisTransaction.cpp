// Copyright 2022 Danyang Chen https://github.com/DAN-AND-DNA


#include "RedisTransaction.h"
#include "HiRedisCpp.h"
#include "RedisClient.h"
#include "RedisTask.h"
#include "HAL/RunnableThread.h"

URedisTransaction::URedisTransaction() {
}

void URedisTransaction::Start( FRedisTaskDoneDelegate OnDone) {
	// 启动工作线程
	if (!this->Job.IsValid()) {
		this->Job = FRedisTransactionJob::RunNewJob(this, this->RedisClient);	
	}
	
	// 事务关联连接
	const TSharedPtr<FRedisTask> Task1 = MakeShared<FRedisTask> (this->RedisClient, FString("CONNECT"), OnDone, true);
	this->Job->AddTask(Task1);
	
	const TSharedPtr<FRedisTask> Task2 = MakeShared<FRedisTask> (this->RedisClient, FString("MULTI"), OnDone, true);
	this->Job->AddTask(Task2);
}

void URedisTransaction::Queue(const FString& Command, FRedisTaskDoneDelegate OnDone) {
	const TSharedPtr<FRedisTask> Task = MakeShared<FRedisTask> (this->RedisClient, Command, OnDone, true);
	this->Job->AddTask(Task);
}


void URedisTransaction::Commit(FRedisTaskDoneDelegate OnDone) {
	const TSharedPtr<FRedisTask> Task = MakeShared<FRedisTask> (this->RedisClient, FString(TEXT("EXEC")), OnDone, true);
	this->Job->AddTask(Task);
}


int32 FRedisTransactionJob::BaseId = 1000;

FRedisTransactionJob::FRedisTransactionJob(const TWeakObjectPtr<URedisTransaction>& InOwnerTransaction, const TWeakObjectPtr<URedisClient>& InWeakRedisClient)
	: bRun(true)
	, WorkerThread(nullptr)
	, WeakRedisClient(InWeakRedisClient)
	, OwnerTransaction(InOwnerTransaction) {
}

FRedisTransactionJob::~FRedisTransactionJob() {
	if(this->WorkerThread != nullptr) {
		this->WorkerThread->Kill(true);
		delete this->WorkerThread;
		this->WorkerThread = nullptr;
	}

	if (this->Event != nullptr) {
		FPlatformProcess::ReturnSynchEventToPool(this->Event);
		this->Event = nullptr;
	}
}

TSharedPtr<FRedisTransactionJob> FRedisTransactionJob::RunNewJob(const TWeakObjectPtr<URedisTransaction>& InOwnerTransaction, const TWeakObjectPtr<URedisClient>& InWeakRedisClient) {
	FRedisTransactionJob* RawPtr = new FRedisTransactionJob(InOwnerTransaction, InWeakRedisClient);
	TSharedPtr<FRedisTransactionJob> NewJob (RawPtr);
	
	FRedisTransactionJob::BaseId++;
	NewJob->RedisCon = MakeShared<FHiRedisCpp>();
	NewJob->Event = FPlatformProcess::GetSynchEventFromPool();
	NewJob->WeakRedisClient = InWeakRedisClient;
	NewJob->WorkerThread = FRunnableThread::Create(RawPtr, *FString::Printf(TEXT("Redis_Transaction_Job_%d"), FRedisTransactionJob::BaseId));
	
	return NewJob;
}

bool FRedisTransactionJob::Init() {
	return true;
}

uint32 FRedisTransactionJob::Run() {
	FString Err;
			
	
	TArray<TSharedPtr<FRedisTask>> LocalTaskQueue;
	while (this->bRun) {
		
		// block at here
		this->Event->Wait();
		if (!this->bRun) {
			break;
		}
		
		{
			FScopeLock Lock(&this->Mutex);
			LocalTaskQueue = MoveTemp(this->TaskQueue);
			this->TaskQueue.Empty();
		}

		for (const TSharedPtr<FRedisTask>& Task : LocalTaskQueue) {
			// 处理任务
			ProcessTask(Task);
		}

		LocalTaskQueue.Empty();
	}

	return 0;
}

void FRedisTransactionJob::Stop() {
	this->bRun = false;
	if (this->Event != nullptr) {
		this->Event->Trigger();
	}
}

void FRedisTransactionJob::Exit() {
}


void FRedisTransactionJob::AddTask(const TSharedPtr<FRedisTask>& Task) {
	{
		FScopeLock Lock(&this->Mutex);
		this->TaskQueue.Add(Task);
	}
	
	this->Event->Trigger();
}

void FRedisTransactionJob::ProcessTask(const TSharedPtr<FRedisTask>& Task) {
	if (!Task.IsValid() || !Task->Data.bInit || !Task->Data.bTransaction) {
		return;
	}
	
	FReplyValue ReplyValue;
	FString Err;

	if (Task->Data.Command == FString(TEXT("CONNECT"))) {
		this->RedisCon = MakeShared<FHiRedisCpp>();
		
		// 建立连接
		const URedisClient* RedisClient = this->WeakRedisClient.Get();
		if (RedisClient == nullptr) {
			return;
		}
			
		this->RedisCon->Connect(RedisClient->IP, RedisClient->Port, Err);
		if (!RedisClient->Password.IsEmpty()) {
			this->RedisCon->Auth(RedisClient->Password, 2,Err);	
		}
	} else {
		bool Ok = this->RedisCon->ExecCommand(Task->Data.Command, ReplyValue, Err);
		const TSharedPtr<FRedisTask> LocalTask = Task;
		AsyncTask(ENamedThreads::GameThread, [LocalTask, Ok, ReplyValue, Err]
		{
			if (!LocalTask->Data.WeakRedisClient.IsValid()) {
				return;
			}
		
			// 调用任务关联的代理
			LocalTask->Data.TaskDoneDelegate.ExecuteIfBound(Ok, Err, ReplyValue);
		});

		if (!Ok || Task->Data.Command == FString(TEXT("EXEC"))) {
			this->RedisCon = MakeShared<FHiRedisCpp>();
		}
	}
}
