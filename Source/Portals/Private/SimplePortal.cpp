// Fill out your copyright notice in the Description page of Project Settings.

#include "SimplePortal.h"

#include "PortalSceneCapture.h"


ASimplePortal::ASimplePortal(const FObjectInitializer& ObjectInitializer) :
   Super(ObjectInitializer)
{
   PrimaryActorTick.bCanEverTick = true;
}


void ASimplePortal::BeginPlay()
{
   Super::BeginPlay();

   if (m_type != ECameraType::Portal || !m_linked_portal)
   {
      m_linked_portal = this;
      m_exit_in_front = false;
   }
}


void ASimplePortal::SetSceneCaptures()
{
   UPortalSceneCapture* scene_capture = CreateDefaultSceneCapture();

   scene_capture->Init(m_type, m_linked_portal, m_exit_in_front);

   m_scene_captures.Add(scene_capture);
}

