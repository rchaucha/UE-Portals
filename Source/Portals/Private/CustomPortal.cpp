// Fill out your copyright notice in the Description page of Project Settings.

#include "CustomPortal.h"

#include "PortalSceneCapture.h"


ACustomPortal::ACustomPortal(const FObjectInitializer& ObjectInitializer) :
   Super(ObjectInitializer)
{
   PrimaryActorTick.bCanEverTick = true;
}


void ACustomPortal::SetSceneCaptures()
{
   // We retrieve all SC associated to the portal
   GetComponents<UPortalSceneCapture>(m_scene_captures);

   for (auto* scene_capture : m_scene_captures)
      SetDefaultSceneCaptureParameters(scene_capture);
}


void ACustomPortal::AddSceneCapture(UPortalSceneCapture* scene_capture)
{
   // Maximum 5 scene captures supported
   if (m_scene_captures.Num() < 5)
      m_scene_captures.Add(scene_capture);
}
