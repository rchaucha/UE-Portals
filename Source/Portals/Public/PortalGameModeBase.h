// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PortalGameModeBase.generated.h"


class APortalManager;

UCLASS()
class PORTALS_API APortalGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
   UFUNCTION(BlueprintCallable)
   APortalManager* GetPortalManager() { return portal_manager; }

private:
   // A PortalManager is linked to the game mode
   APortalManager* portal_manager;
   
protected:
   void BeginPlay() override;

};
