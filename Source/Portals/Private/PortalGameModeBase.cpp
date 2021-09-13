// Fill out your copyright notice in the Description page of Project Settings.

#include "PortalGameModeBase.h"

#include "PortalManager.h"


void APortalGameModeBase::BeginPlay()
{
   Super::BeginPlay();

   FActorSpawnParameters SpawnParams;

   portal_manager = GetWorld()->SpawnActor<APortalManager>(APortalManager::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
   portal_manager->AttachToActor(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
   portal_manager->Init();
}
