// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once 

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "PlayGroundCppHUD.generated.h"

UCLASS()
class APlayGroundCppHUD : public AHUD
{
	GENERATED_BODY()

public:
	APlayGroundCppHUD();

	/** Primary draw call for the HUD */
	virtual void DrawHUD() override;

private:
	/** Crosshair asset pointer */
	class UTexture2D* CrosshairTex;

};

