// Fill out your copyright notice in the Description page of Project Settings.


#include "RedisPipeline.h"

#include "RedisTask.h"
#include "RedisTaskPool.h"

URedisPipeline::URedisPipeline()
	: bStartPipeline(false) {
}

void URedisPipeline::Start() {
	this->bStartPipeline = true;
	PipelineCommands.Empty();
}

bool URedisPipeline::Exec(const FString& Command, FString& Err) {
	if (!this->bStartPipeline) {
		Err = FString("need start pipeline");
		return false;
	}
	
	PipelineCommands.Add(Command);
	Err = FString("");
	return true;
}

 void URedisPipeline::Commit(FRedisTaskDonePipelineDelegate OnDone) {
	FString Err;
	if (!this->bStartPipeline) {
		Err = FString("need start pipeline");
		OnDone.ExecuteIfBound(false, Err, TArray<FReplyValue>());
		return;
	}
	
	const TSharedPtr<FRedisTaskPool> Pool = this->RedisTaskPool.Pin();
	if (!Pool.IsValid()) {
		Err = FString("bad task pool or bad redis client");
		OnDone.ExecuteIfBound(false, Err, TArray<FReplyValue>());
		return;
	}
	
	FRedisTask* NewTask = new FRedisTask(this->RedisClient, this->PipelineCommands, MoveTemp(OnDone));
	Pool->Dispatch(NewTask);

	Err = FString("");
	this->bStartPipeline = false;
	PipelineCommands.Empty();
}
