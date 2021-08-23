// Copyright Epic Games, Inc. All Rights Reserved.

#include "PlayGroundCppGameMode.h"
#include "PlayGroundCppHUD.h"
#include "PlayGroundCppCharacter.h"
#include "UObject/ConstructorHelpers.h"

APlayGroundCppGameMode::APlayGroundCppGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = APlayGroundCppHUD::StaticClass();
}
