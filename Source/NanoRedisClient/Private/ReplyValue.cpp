// Fill out your copyright notice in the Description page of Project Settings.


#include "ReplyValue.h"
#include "hiredis.h"


FListReplyValue::FListReplyValue()
	: Type(EReplyValueType::ENil)
	, Integer(0) {
}

void FListReplyValue::ParserReply(const redisReply* Reply) {
	switch (Reply->type)
	{
	case REDIS_REPLY_STRING:
		{
			this->String = FString(ANSI_TO_TCHAR(Reply->str));
			this->Type = EReplyValueType::EString;	// get返回的值都是字符串
			break;
		}
	case REDIS_REPLY_INTEGER:
		{
			this->Integer = Reply->integer;		// incr dbsize等 返回的值为int和double
			this->Type = EReplyValueType::EInt;
			break;
		}
	case REDIS_REPLY_ARRAY:
		{
			for (size_t i = 0; i < Reply->elements; i ++) {
				FString Tmp = FString(ANSI_TO_TCHAR(Reply->element[static_cast<int>(i)]->str));
				this->List.Add(MoveTemp(Tmp));
			}
			this->Type = EReplyValueType::EList;
			break;
		}
	case REDIS_REPLY_STATUS:
		{
			this->Status = FString(ANSI_TO_TCHAR(Reply->str));
			this->Type = EReplyValueType::EStatus;
			break;
		}
	default:
		{
			this->Type = EReplyValueType::ENil;
			break;
		}
	}
}


FReplyValue::FReplyValue()
	: Type(EReplyValueType::ENil)
	, Integer(0) {
}

void FReplyValue::ParserReply(const redisReply* Reply) {
	switch (Reply->type)
	{
	case REDIS_REPLY_STRING:
		{
			this->String = FString(ANSI_TO_TCHAR(Reply->str));
			this->Type = EReplyValueType::EString;	// get返回的值都是字符串
			break;
		}
	case REDIS_REPLY_INTEGER:
		{
			this->Integer = Reply->integer;		// incr dbsize等 返回的值为int和double
			this->Type = EReplyValueType::EInt;
			break;
		}
	case REDIS_REPLY_ARRAY:
		{
			for (size_t i = 0; i < Reply->elements; i ++) {
				FListReplyValue Tmp;
				Tmp.ParserReply(Reply->element[static_cast<int>(i)]);
				this->List.Add(MoveTemp(Tmp));
			}
			this->Type = EReplyValueType::EList;
			break;
		}
	case REDIS_REPLY_STATUS:
		{
			this->Status = FString(ANSI_TO_TCHAR(Reply->str));
			this->Type = EReplyValueType::EStatus;
			break;
		}
	default:
		{
			this->Type = EReplyValueType::ENil;
			break;
		}
	}
}
