// Copyright 2022 Danyang Chen https://github.com/DAN-AND-DNA

#include "RedisClient.h"
#include "RedisPipeline.h"
#include "RedisTask.h"
#include "RedisTaskPool.h"
#include "RedisTransaction.h"


URedisClient::URedisClient() {
}

URedisClient* URedisClient::NewRedisClient(int MaxNum, const FString& InIP, int InPort, const FString& InPassword) {
	URedisClient* NewClient = NewObject<URedisClient>();
	
	NewClient->IP = InIP;
	NewClient->Port = InPort;
	NewClient->Password = InPassword;
	NewClient->RedisTaskPool = MakeShared<FRedisTaskPool>(InIP, InPort, InPassword, MaxNum);
	
	return NewClient;
}

URedisPipeline* URedisClient::NewPipeline() {
	URedisPipeline* NewPipeline = NewObject<URedisPipeline>();
	NewPipeline->RedisTaskPool = this->RedisTaskPool;
	NewPipeline->RedisClient = this;

	return NewPipeline;
}

URedisTransaction* URedisClient::NewTransaction() {
	URedisTransaction* NewTransaction = NewObject<URedisTransaction>();
	NewTransaction->RedisTaskPool = this->RedisTaskPool;
	NewTransaction->RedisClient = this;
	
	return NewTransaction;
}


void URedisClient::Ping(FRedisTaskDoneDelegate OnDone) {
	//const uint64 TaskId = this->NextTaskId();
	FRedisTask* NewTask = new FRedisTask(this, FString(TEXT("PING")), MoveTemp(OnDone));
	this->RedisTaskPool->Dispatch(NewTask);
}


void URedisClient::Raw(const FString& Command, FRedisTaskDoneDelegate OnDone) {
	//const uint64 TaskId = this->NextTaskId();
	FRedisTask* NewTask = new FRedisTask(this, Command,  MoveTemp(OnDone));
	this->RedisTaskPool->Dispatch(NewTask);
}