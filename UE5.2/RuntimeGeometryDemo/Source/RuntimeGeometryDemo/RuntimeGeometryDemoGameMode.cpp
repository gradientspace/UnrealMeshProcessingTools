// Copyright Epic Games, Inc. All Rights Reserved.

#include "RuntimeGeometryDemoGameMode.h"
#include "RuntimeGeometryDemoCharacter.h"
#include "UObject/ConstructorHelpers.h"

ARuntimeGeometryDemoGameMode::ARuntimeGeometryDemoGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
