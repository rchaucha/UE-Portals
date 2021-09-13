// Fill out your copyright notice in the Description page of Project Settings.

#include "PortalCharacter.h"

#include "Camera/CameraComponent.h" 
#include "Components/CapsuleComponent.h" 


APortalCharacter::APortalCharacter(const FObjectInitializer& ObjectInitializer) :
   Super(ObjectInitializer)
{
   m_camera = CreateDefaultSubobject<UCameraComponent>(TEXT("PlayerCamera"));
   m_camera->AttachToComponent(RootComponent, FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));
   m_camera->Mobility = EComponentMobility::Movable;
   m_camera->bUsePawnControlRotation = true;

   GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
}


UCameraComponent* APortalCharacter::GetPlayerCamera() const
{
   return m_camera;
}