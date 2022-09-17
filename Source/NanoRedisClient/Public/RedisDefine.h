// Copyright 2022 Danyang Chen https://github.com/DAN-AND-DNA

#pragma once

#include "CoreMinimal.h"
#include "RedisDefine.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class EReplyValueType : uint8 {
	ENil,
	EInt,
	EString,
	EList,
	EStatus,
};