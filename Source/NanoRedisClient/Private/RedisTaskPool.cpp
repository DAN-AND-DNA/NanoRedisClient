// Copyright 2022 Danyang Chen https://github.com/DAN-AND-DNA


#include "RedisTaskPool.h"
#include "RedisTask.h"
#include "Misc/QueuedThreadPool.h"


FRedisTaskPool::FRedisTaskPool(const FString& InIP, int InPort, const FString& InPassword, int MaxNum)
	: IP(InIP)
	, Port(InPort)
	, Password(InPassword) {
	
	this->ThreadPool = FQueuedThreadPool::Allocate();
	this->ThreadPool->Create(MaxNum, 32 * 1024, TPri_Normal, TEXT("RedisTaskPool"));
}

FRedisTaskPool::~FRedisTaskPool() {
	this->ThreadPool->Destroy();
	delete this->ThreadPool;
	this->ThreadPool = nullptr;
	RedisCons.Empty();
}

void FRedisTaskPool::Dispatch(FRedisTask* Task) {
	if (this->ThreadPool == nullptr || Task == nullptr) {
		return;
	}

	this->ThreadPool->AddQueuedWork(Task);
}
