// Copyright 2022 Danyang Chen https://github.com/DAN-AND-DNA

#pragma once

#include "CoreMinimal.h"
#include "ReplyValue.h"


class FHiRedisCpp {
public:
	explicit FHiRedisCpp();
	~FHiRedisCpp();

	// connect to redis server
	bool Connect(const FString& IP, int Port, FString& Err);

	void Disconnect();

	bool ExecCommand(const FString& Command, FReplyValue& Value, FString& Err);
	
	bool ExecPipelineCommands(const TArray<FString>& PipelineCommands, TArray<FReplyValue>& Values, FString& Err);

	// ping
	bool Ping(FString& Err);

	bool Auth(const FString& Password, const int MaxSecond, FString& Err);

	// set
	bool Set(const FString& Key, const FString& Value, const int MaxSecond, FString& Err);

	// get
	bool Get(const FString& Key, const int MaxSecond, FReplyValue& Value, FString& Err);

	// lpush
	bool LPush(const FString& Key, const TArray<FString>& Values, const int MaxSecond, FString& Err);

	// lrange
	bool LRange(const FString& Key, const int Start, const int End, const int MaxSecond, FReplyValue& Value, FString& Err);

	// incr
	bool Incr(const FString& Key, const int MaxSecond, FReplyValue& Value, FString& Err);

	// hset
	bool HSet(const FString& Key, const TMap<FString, FString>& MapValues, const int MaxSecond, FString& Err);

	// hget
	bool HGet(const FString& Key, const FString& MapKey, const int MaxSecond, FReplyValue& Value, FString& Err);

	// hgetall
	bool HGetAll(const FString& Key, const int MaxSecond, FReplyValue& Value, FString& Err);

	// sadd
	bool SAdd(const FString& Key, const TArray<FString>& Values, const int MaxSecond, FString& Err);

	bool IsConnected();
	
	bool IsServerClosed() const;

	bool GetError(FString& Err, struct redisReply* Reply = nullptr) const;
private:
	bool bConnected;
	
	struct redisContext* RedisCtx;
};


