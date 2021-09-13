// Fill out your copyright notice in the Description page of Project Settings.

#include "TeleporterPortal.h"

#include <GameFramework/CharacterMovementComponent.h>
#include <GameFramework/Character.h>

#include "PortalSceneCapture.h"


ATeleporterPortal::ATeleporterPortal(const FObjectInitializer& ObjectInitializer) :
   Super(ObjectInitializer),
   m_last_in_front(false)
{
   PrimaryActorTick.bCanEverTick = true;
}


void ATeleporterPortal::BeginPlay()
{
   Super::BeginPlay();

   if (!m_linked_portal)
      m_linked_portal = this;
}


void ATeleporterPortal::SetSceneCaptures()
{
   UPortalSceneCapture* scene_capture = CreateDefaultSceneCapture();

   scene_capture->Init(ECameraType::Portal, m_linked_portal, m_exit_in_front);

   m_scene_captures.Add(scene_capture);
}


bool ATeleporterPortal::IsPointInFrontOfPortal(FVector point) const
{
   float point_distance_to_plane = GetPortalPlane().PlaneDot(point);

   return (point_distance_to_plane >= 0);
}


bool ATeleporterPortal::IsCrossingPortal(FVector position)
{
   bool is_in_front = IsPointInFrontOfPortal(position);

   // Did we cross the portal in the right direction (we're not in front of it anymore)?
   bool is_crossing = !is_in_front && m_last_in_front;

   m_last_in_front = is_in_front;

   return is_crossing;
}


void ATeleporterPortal::TeleportActor(AActor* actor_to_teleport)
{
   if (actor_to_teleport == nullptr || m_linked_portal == nullptr)
      return;

   actor_to_teleport->SetActorTransform(Tools::ComputeNewTransform(actor_to_teleport->GetTransform(), this, m_scene_captures[0]));

   // If we are teleporting a character we need to update its controller and reapply its velocity
   if (actor_to_teleport->IsA(APortalCharacter::StaticClass()))
   {
      APortalCharacter* player = Cast<APortalCharacter>(actor_to_teleport);
      FVector saved_velocity = player->GetVelocity();

      // Update Controller
      AController* player_controller = player->GetController();

      if (player_controller)
         player_controller->SetControlRotation(Tools::ComputeNewPortalRotation(FQuat(player_controller->GetControlRotation()), this, this->m_scene_captures[0]));

      // Reapply Velocity
      FVector local_velocity;
      local_velocity.X = FVector::DotProduct(saved_velocity, GetActorForwardVector());
      local_velocity.Y = FVector::DotProduct(saved_velocity, GetActorRightVector());
      local_velocity.Z = FVector::DotProduct(saved_velocity, GetActorUpVector());

      FVector NewVelocity = (local_velocity.X * m_linked_portal->GetActorForwardVector()
                           + local_velocity.Y * m_linked_portal->GetActorRightVector()) * (m_exit_in_front ? -1 : 1)
                           + local_velocity.Z * m_linked_portal->GetActorUpVector();

      player->GetCharacterMovement()->Velocity = NewVelocity;
   }
}