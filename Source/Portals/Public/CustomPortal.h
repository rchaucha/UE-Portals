// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Portal.h"
#include "CustomPortal.generated.h"

class UPortalSceneCapture;

UCLASS()
class PORTALS_API ACustomPortal : public APortal
{
	GENERATED_UCLASS_BODY()

public:
	virtual void SetSceneCaptures() override;
	
	void AddSceneCapture(UPortalSceneCapture* scene_capture_actor);
};
