// Fill out your copyright notice in the Description page of Project Settings.

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