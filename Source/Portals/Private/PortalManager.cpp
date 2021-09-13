#include "PortalManager.h"

#include <Runtime/Engine/Public/EngineUtils.h>
#include <Camera/CameraComponent.h>
#include <Engine/World.h>
#include <Kismet/GameplayStatics.h>

#include "PortalCharacter.h"
#include "PortalTools.h"
#include "Portal.h"
#include "TeleporterPortal.h"


// Sets default values
APortalManager::APortalManager(const FObjectInitializer& ObjectInitializer) :
   Super(ObjectInitializer)
{
   PrimaryActorTick.bCanEverTick = true;
}


void APortalManager::BeginPlay()
{
   Super::BeginPlay();

   m_controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
   AttachToActor(m_controller, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
}


void APortalManager::Init()
{
   for (TActorIterator<APortal> portals_it(GetWorld()); portals_it; ++portals_it)
   {
      APortal* portal = *portals_it;
      portal->SetSceneCaptures();
   }
}


void APortalManager::Tick(float DeltaSeconds)
{
   Super::Tick(DeltaSeconds);

   // Find portals in the level and update them
   UpdateVisiblePortals();
}


void APortalManager::RequestTeleportByPortal(ATeleporterPortal* portal, AActor* target_to_teleport) const
{
   if (portal && target_to_teleport && m_controller)
   {
      portal->TeleportActor(target_to_teleport);

      UpdateVisiblePortals();
   }
}


void APortalManager::UpdateVisiblePortals() const
{
   if (!m_controller)
      return;

   const APortalCharacter* character = Cast<APortalCharacter>(m_controller->GetCharacter());
   if (!character)
      return;

   FVector player_location = character->GetActorLocation();

   ClearAllPortals();

   TArray<APortal*> visible_portals;

   for (TActorIterator<APortal> portals_it(GetWorld()); portals_it; ++portals_it)
   {
      APortal* portal = *portals_it;
      UCameraComponent* camera = character->GetPlayerCamera();

      // If the portal is on screen, render it
      if (Tools::isPortalVisibleToCamera(portal, camera))
         portal->Render(camera->GetComponentTransform(), GetCameraProjectionMatrix(), 0);
   }
}


void APortalManager::ClearAllPortals() const
{
   for (TActorIterator<APortal> portals_it(GetWorld()); portals_it; ++portals_it)
   {
      APortal* portal = *portals_it;

      portal->SetActive(false);
      portal->SetIsRendererButNotCaptured(false);
   }
}


FMatrix APortalManager::GetCameraProjectionMatrix() const 
{
   FMatrix projection_matrix;
   ULocalPlayer* local_player = m_controller->GetLocalPlayer();

   if (local_player)
   {
      FSceneViewProjectionData player_projection_data;
      local_player->GetProjectionData(local_player->ViewportClient->Viewport, EStereoscopicPass::eSSP_FULL, player_projection_data);

      projection_matrix = player_projection_data.ProjectionMatrix;
   }

   return projection_matrix;
}
