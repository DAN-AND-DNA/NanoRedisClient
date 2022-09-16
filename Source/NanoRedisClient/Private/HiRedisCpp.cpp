#include "HiRedisCpp.h"
#include "hiredis.h"
#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
//#include "Windows/AllowWindowsPlatformAtomics.h"
THIRD_PARTY_INCLUDES_START
#include "win32.h"
THIRD_PARTY_INCLUDES_END
//#include "Windows/HideWindowsPlatformAtomics.h"
//#include "Windows/HideWindowsPlatformTypes.h"
#endif


FHiRedisCpp::FHiRedisCpp()
	: bConnected(false)
	, RedisCtx (nullptr) {
}

FHiRedisCpp::~FHiRedisCpp() {
	this->Disconnect();
}

bool FHiRedisCpp::Connect(const FString& IP, int Port, FString& Err) {
	this->Disconnect();
	redisContext* Ctx = redisConnectWithTimeout(TCHAR_TO_ANSI(*IP), Port, timeval{ 1, 500000 });	// 1.5 seconds

	if (Ctx == nullptr) {
		Err = FString(TEXT("redis context is nil"));
		return false;
	}
	
	this->RedisCtx = Ctx;
	this->bConnected = true;
	
	if(this->GetError(Err)) {
		this->Disconnect();
		return false;
	}
	
	return true;
}

void FHiRedisCpp::Disconnect() {
	if (this->RedisCtx != nullptr) {
		redisFree(this->RedisCtx);
		this->RedisCtx = nullptr;
		this->bConnected = false;
	}
}

bool FHiRedisCpp::ExecCommand(const FString& Command, FReplyValue& Value, FString& Err) {
	if (!this->IsConnected()) {
		Err = FString(TEXT("bad connection"));
		return false;
	}

	redisReply* Reply = static_cast<redisReply*> (redisCommand(this->RedisCtx, TCHAR_TO_ANSI(*Command)));
	if (this->IsServerClosed()) {
		this->GetError(Err, Reply);
		freeReplyObject(Reply);
		this->Disconnect();
		return false;
	}
	
	if (this->GetError(Err, Reply)) {
		freeReplyObject(Reply);
		return false;
	}
	
	Value.ParserReply(Reply);
	freeReplyObject(Reply);
	return true;
}

bool FHiRedisCpp::ExecPipelineCommands(const TArray<FString>& PipelineCommands, TArray<FReplyValue>& Values, FString& Err) {
	if (!this->IsConnected()) {
		Err = FString(TEXT("bad connection"));
		return false;
	}
	
	int PipelineCommandNum = 0;
	for (const FString& Command: PipelineCommands) {
		redisAppendCommand(this->RedisCtx, TCHAR_TO_ANSI(*Command));
		PipelineCommandNum++;
	}
	
	for (int i = 0; i < PipelineCommandNum; i++) {
		redisReply* Reply;
		FReplyValue Value;
		redisGetReply(this->RedisCtx, reinterpret_cast<void**>(&Reply));
		if (this->IsServerClosed()) {
			this->GetError(Err, Reply);
			freeReplyObject(Reply);
			this->Disconnect();
			return false;
		}
	
		if (this->GetError(Err, Reply)) {
			freeReplyObject(Reply);
			return false;
		}
		Value.ParserReply(Reply);
		Values.Add(MoveTemp(Value));
		freeReplyObject(Reply);
	}
	
	return true;
}

bool FHiRedisCpp::Ping(FString& Err) {
	FReplyValue ReplyValue;
	if (this->RedisCtx) {
		redisSetTimeout(this->RedisCtx, timeval{1, 0});
	}
	
	if (!ExecCommand("PING",ReplyValue,Err)) {
		return false;
	}
	return true;
}

bool FHiRedisCpp::Auth(const FString& Password, const int MaxSecond, FString& Err) {
	FReplyValue ReplyValue;
	if (this->RedisCtx) {
		redisSetTimeout(this->RedisCtx, timeval{MaxSecond, 0});
	}
	
	const FString StrCommand = FString::Printf(TEXT("AUTH %s"), *Password);
	if (!ExecCommand(StrCommand,ReplyValue,Err)) {
		return false;
	}
	
	return true;
}

bool FHiRedisCpp::Set(const FString& Key, const FString& Value, const int MaxSecond, FString& Err) {
	FReplyValue ReplyValue;
	if (this->RedisCtx) {
		redisSetTimeout(this->RedisCtx, timeval{MaxSecond, 0});
	}
	
	const FString StrCommand = FString::Printf(TEXT("SET %s %s"), *Key, *Value);
	if (!ExecCommand(StrCommand,ReplyValue,Err)) {
		return false;
	}
	return true;
}

bool FHiRedisCpp::Get(const FString& Key, const int MaxSecond, FReplyValue& Value, FString& Err) {
	if (this->RedisCtx) {
		redisSetTimeout(this->RedisCtx, timeval{MaxSecond, 0});
	}
	
	const FString StrCommand = FString::Printf(TEXT("GET %s"), *Key);
	if (!ExecCommand(StrCommand,Value,Err)) {
		return false;
	}
	return true;
}

bool FHiRedisCpp::LPush(const FString& Key, const TArray<FString>& Values, const int MaxSecond, FString& Err) {
	FReplyValue ReplyValue;
	if (this->RedisCtx) {
		redisSetTimeout(this->RedisCtx, timeval{MaxSecond, 0});
	}
	
	FString StrCommand = FString::Printf(TEXT("LPUSH %s "), *Key);
	for (const FString& Tmp : Values) {
		StrCommand += FString::Printf(TEXT("%s "), *Tmp);
	}
	
	if (!ExecCommand(StrCommand,ReplyValue,Err)) {
		return false;
	}
	return true;
}

bool FHiRedisCpp::LRange(const FString& Key, const int Start, const int End, const int MaxSecond, FReplyValue& Value, FString& Err) {
	if (this->RedisCtx) {
		redisSetTimeout(this->RedisCtx, timeval{MaxSecond, 0});
	}
	
	const FString StrCommand = FString::Printf(TEXT("LRANGE %s %d %d"), *Key, Start, End);
	if (!ExecCommand(StrCommand,Value,Err)) {
		return false;
	}
	return true;
}

bool FHiRedisCpp::Incr(const FString& Key, const int MaxSecond, FReplyValue& Value, FString& Err) {
	if (this->RedisCtx) {
		redisSetTimeout(this->RedisCtx, timeval{MaxSecond, 0});
	}
	
	const FString StrCommand = FString::Printf(TEXT("INCR %s"), *Key);
	if (!ExecCommand(StrCommand,Value,Err)) {
		return false;
	}
	return true;
}

bool FHiRedisCpp::HSet(const FString& Key, const TMap<FString, FString>& MapValues, const int MaxSecond, FString& Err) {
	FReplyValue ReplyValue;
	if (this->RedisCtx) {
		redisSetTimeout(this->RedisCtx, timeval{MaxSecond, 0});
	}
	
	FString StrCommand = FString::Printf(TEXT("HSET %s "), *Key);
	for (const TPair<FString, FString>& Pair : MapValues) {
		StrCommand += FString::Printf(TEXT("%s %s "), *(Pair.Key), *(Pair.Value));
	}
	
	if (!ExecCommand(StrCommand,ReplyValue,Err)) {
		return false;
	}
	return true;
}

bool FHiRedisCpp::HGet(const FString& Key, const FString& MapKey, const int MaxSecond, FReplyValue& Value, FString& Err) {
	if (this->RedisCtx) {
		redisSetTimeout(this->RedisCtx, timeval{MaxSecond, 0});
	}
	
	const FString StrCommand = FString::Printf(TEXT("HGET %s %s"), *Key, *MapKey);
	if (!ExecCommand(StrCommand,Value,Err)) {
		return false;
	}
	return true;
}

bool FHiRedisCpp::SAdd(const FString& Key, const TArray<FString>& Values, const int MaxSecond, FString& Err) {
	FReplyValue ReplyValue;
	if (this->RedisCtx) {
		redisSetTimeout(this->RedisCtx, timeval{MaxSecond, 0});
	}
	
	FString StrCommand = FString::Printf(TEXT("SADD %s "), *Key);
	for (const FString& Tmp : Values) {
		StrCommand += FString::Printf(TEXT("%s "), *Tmp);
	}
	
	if (!ExecCommand(StrCommand,ReplyValue,Err)) {
		return false;
	}
	return true;
}

bool FHiRedisCpp::IsConnected() {
	return this->bConnected;
}


bool FHiRedisCpp::IsServerClosed() const {
	if (this->RedisCtx == nullptr) {
		return false;
	}
	
	return this->RedisCtx->err == REDIS_ERR_EOF;
}

bool FHiRedisCpp::GetError(FString& Err, struct redisReply* Reply) const {
	if (this->RedisCtx == nullptr) {
		Err = FString(TEXT("bad connection"));
		return true;
	}
	
	// server closed
	if (this->IsServerClosed()) {
		Err = FString(TEXT("server closes this connection"));
		return true;
	}
	
	// network error
	if (this->RedisCtx->err != 0) {
		Err = FString(ANSI_TO_TCHAR(this->RedisCtx->errstr));
		return true;
	}

	// api error
	if (Reply != nullptr && Reply->type == REDIS_REPLY_ERROR ) {
		Err = FString(ANSI_TO_TCHAR(Reply->str));
		return true;
	}
	
	Err = FString("");
	return false;
}
