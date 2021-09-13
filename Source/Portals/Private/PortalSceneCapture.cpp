// Fill out your copyright notice in the Description page of Project Settings.

#include "PortalSceneCapture.h"

#include <Engine/TextureRenderTarget2D.h>
#include <Kismet/KismetMathLibrary.h> 

#include "Portal.h"
#include "PortalTools.h"


UPortalSceneCapture::UPortalSceneCapture(const FObjectInitializer& ObjectInitializer) :
   Super(ObjectInitializer)
{}


void UPortalSceneCapture::BeginPlay()
{
   Super::BeginPlay();

   GenerateDefaultTexture();

   SetOwnerIfAvailable();

   if (m_type != ECameraType::Portal)
      m_exit_in_front = false;

   if (m_weight < 0)
      m_weight = 0;

   if (!m_linked_portal)
      m_linked_portal = m_owner;
}


bool UPortalSceneCapture::SetOwnerIfAvailable()
{
   AActor* owner_actor = GetOwner();

   if (owner_actor->IsA(APortal::StaticClass()))
   {
      m_owner = Cast<APortal>(owner_actor);
      return true;
   }

   return false;
}


bool UPortalSceneCapture::IsOwnerValid()
{
   return m_owner || SetOwnerIfAvailable();
}


void UPortalSceneCapture::Init(ECameraType type, APortal* linked_portal, bool exit_in_front, float weight)
{
   m_type = type;
   m_linked_portal = linked_portal;
   m_exit_in_front = exit_in_front;
   m_weight = weight;
}


void UPortalSceneCapture::Update(const FTransform& watched_actor_transfo, const FMatrix& projection_matrix)
{
   if (!m_render_target || m_render_target->GetFName().IsNone())
      GenerateDefaultTexture();

   if (IsOwnerValid())
   {
      // Set the SceneCapture position and rotation
      if (m_linked_portal)
      {
         // Transformation of the watched actor in the linked portal space
         UpdateTransformation(watched_actor_transfo);

         UpdateNearClipPlane();
      }

      TextureTarget = m_render_target;

      CustomProjectionMatrix = projection_matrix;
      
      CaptureScene();
   }
}


void UPortalSceneCapture::UpdateTransformation_Implementation(const FTransform& watched_actor_transfo)
{
   if (IsOwnerValid())
   {
      FTransform modified_wa_transfo = watched_actor_transfo;

      // If n1 != n2, compute refraction with n1*sin(theta1) = n2*sin(theta2)
      if (GetTrueType() != ECameraType::Mirror && m_refractive_ind_1 != m_refractive_ind_2)
      {
         FVector portal_mid = m_owner->GetMiddlePoint();
         FVector portal_normal = m_owner->GetActorForwardVector();
         FVector wa_pos = modified_wa_transfo.GetLocation();

         float incidence_angle = Tools::ComputeIncidenceAngle(wa_pos, portal_mid, portal_normal);
         float refraction_angle = Tools::ComputeRefractionAngle(incidence_angle, m_refractive_ind_1, m_refractive_ind_2);
         
         // If total reflection, consider the scene_capture as a mirror
         if (refraction_angle == -1.f)
            m_is_total_reflection = true;

         else
         {
            m_is_total_reflection = false;
            // Rotation of watched actor to simulate refraction
            FPlane incidence_plane(wa_pos, portal_mid, portal_mid + portal_normal);
            FVector incidence_plane_normal = incidence_plane.GetNormal();
            incidence_plane_normal.Normalize();

            FRotator rotation = FQuat(incidence_plane_normal, FMath::DegreesToRadians(incidence_angle - refraction_angle)).Rotator();
            FRotationAboutPointMatrix rotation_matrix(rotation, portal_mid);
            modified_wa_transfo.SetLocation(rotation_matrix.TransformPosition(modified_wa_transfo.GetLocation()));

            // We only want the camera to rotate with Yaw
            rotation.Pitch = 0;
            rotation.Roll = 0;

            modified_wa_transfo.SetRotation(FQuat(rotation) * modified_wa_transfo.GetRotation());
         }
      }

      SetWorldTransform(Tools::ComputeNewTransform(modified_wa_transfo, m_owner, this));
   }
}


void UPortalSceneCapture::UpdateNearClipPlane_Implementation()
{
   if (IsOwnerValid())
   {
      bool is_mirror = (getType() == ECameraType::Mirror);

      APortal* target_portal = (getType() == ECameraType::Portal) ? m_linked_portal : m_owner;

      // Clip Plane : to ignore objects between the SceneCapture and the Target of the portal
      ClipPlaneNormal = - target_portal->GetActorForwardVector() * ((is_mirror || m_exit_in_front) ? -1 : 1);
      ClipPlaneBase = target_portal->GetActorLocation()
                    + 0.3 * ClipPlaneNormal; // Offset to avoid visible pixel border
   }
}


void UPortalSceneCapture::GenerateDefaultTexture()
{
   int32 current_size_x = 1920;
   int32 current_size_y = 1080;

   // Use a smaller size than the current screen to reduce the performance impact
   current_size_x = FMath::Clamp(int(current_size_x / 1.7), 128, 1920);
   current_size_y = FMath::Clamp(int(current_size_y / 1.7), 128, 1080);

   // Create new RTT
   m_render_target = NewObject<UTextureRenderTarget2D>(this, UTextureRenderTarget2D::StaticClass(), FName("Default texture"));

   m_render_target->RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGBA16f;
   m_render_target->Filter = TextureFilter::TF_Bilinear;
   m_render_target->SizeX = current_size_x;
   m_render_target->SizeY = current_size_y;
   m_render_target->ClearColor = FLinearColor::Blue;
   m_render_target->TargetGamma = 2.2f;
   m_render_target->bNeedsTwoCopies = false;
   m_render_target->AddressX = TextureAddress::TA_Clamp;
   m_render_target->AddressY = TextureAddress::TA_Clamp;

   // Not needed since the texture is displayed on screen directly
   // in some engine versions this can even lead to crashes (notably 4.24/4.25)
   m_render_target->bAutoGenerateMips = false;

   // This force the engine to create the render target 
   // with the parameters we defined just above
   m_render_target->UpdateResource();
}
