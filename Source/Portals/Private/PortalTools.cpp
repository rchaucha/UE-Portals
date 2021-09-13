#include "PortalTools.h"

#include <GameFramework/Actor.h>
#include <Components/BoxComponent.h>
#include <Math/UnrealMathUtility.h> 
#include <Editor/DetailCustomizations/Private/SceneCaptureDetails.h>
#include <Kismet/GameplayStatics.h>
#include <Kismet/KismetMathLibrary.h> 
#include <Camera/CameraComponent.h>

#include "PortalSceneCapture.h"
#include "Portal.h"


FTransform Tools::ComputeNewTransform(const FTransform& watched_actor_transfo, const APortal* reference, const UPortalSceneCapture* scene_capture)
{
   FTransform new_transform;

   if (!reference || !scene_capture)
      return new_transform;

   switch (scene_capture->getType())
   {
   case ECameraType::Portal:
   {
      new_transform.SetLocation(ComputeNewPortalLocation(watched_actor_transfo.GetLocation(), reference, scene_capture));
      new_transform.SetRotation(FQuat(ComputeNewPortalRotation(watched_actor_transfo.GetRotation(), reference, scene_capture)));
   } break;

   case ECameraType::Hole:
      new_transform = watched_actor_transfo;
      break;

   case ECameraType::Mirror:
   {
      FPlane portal_plane = reference->GetPortalPlane();

      // Mirror location
      FMirrorMatrix mirror_position(portal_plane);
      new_transform.SetLocation(mirror_position.TransformPosition(watched_actor_transfo.GetLocation()));

      // Center the plane on the mirrored WA
      portal_plane.TransformBy(FTranslationMatrix(new_transform.GetLocation() - reference->GetActorLocation()));

      // Mirror forward vector
      FMirrorMatrix mirror_rotation(portal_plane);
      FVector new_forward = (mirror_rotation.TransformVector(watched_actor_transfo.GetRotation().GetForwardVector()));
      new_transform.SetRotation(FRotationMatrix::MakeFromX(new_forward).ToQuat());

   } break;
   }

   return new_transform;
}


FVector Tools::ComputeNewPortalLocation(const FVector& position, const APortal* reference, const UPortalSceneCapture* scene_capture)
{
   const APortal* target = scene_capture->GetLinkedPortal();

   if (!target)
      return FVector::ZeroVector;

   FVector direction = position - reference->GetActorLocation();
   FVector target_location = target->GetActorLocation();

   // Projection of direction vector in local space of reference
   FVector local_coords;
   local_coords.X = FVector::DotProduct(direction, reference->GetActorForwardVector());
   local_coords.Y = FVector::DotProduct(direction, reference->GetActorRightVector());
   local_coords.Z = FVector::DotProduct(direction, reference->GetActorUpVector());

   // Compute direction vector in local space of target
   FVector new_direction = (local_coords.X * target->GetActorForwardVector()
      + local_coords.Y * target->GetActorRightVector()) * (scene_capture->IsExitInFront() ? -1 : 1)
      + local_coords.Z * target->GetActorUpVector();

   // Returning object position in this new space
   return target_location + new_direction;
}


FRotator Tools::ComputeNewPortalRotation(const FQuat& rotation, const APortal* reference, const UPortalSceneCapture* scene_capture)
{
   const APortal* target = scene_capture->GetLinkedPortal();

   if (!reference || !target)
      return FRotator::ZeroRotator;

   FQuat source_rotation = reference->GetActorTransform().GetRotation();
   FQuat target_rotation = target->GetActorTransform().GetRotation();

   // Compute local rotation in reference portal space
   FQuat local_quat = source_rotation.Inverse() * rotation;

   // Compute rotation in target portal space
   FRotator new_world_rot = (target_rotation * local_quat).Rotator();

   // Mirror if exit in front
   if (scene_capture->IsExitInFront())
      new_world_rot.Yaw = fmod(new_world_rot.Yaw + 180, 360);

   return new_world_rot;
}


bool Tools::isPortalVisibleToCamera(APortal* portal, USceneComponent* camera, float near_plane_distance)
{
   float distance = FMath::Abs(FVector::Dist(camera->GetComponentLocation(), portal->GetActorLocation()));
   if (distance > APortal::GetActivePortalDistance())
      return false;

   bool is_camera_in_front_of_portal = FVector::DotProduct(portal->GetActorForwardVector(), camera->GetComponentLocation() - portal->GetActorLocation()) > 0;

   // If the portal is behind the camera is not in the camera frustum, we don't render it
   if (is_camera_in_front_of_portal && isActorInCameraViewFrustum(portal, camera, near_plane_distance))
   {
      for (const FVector& vertex : *(portal->GetMeshVertices()))
      {
         FCollisionQueryParams params(FName("Vertex visibility from camera"), true, portal);
         FHitResult hit_result;
         // If one vertex of the portal is directly visible (not hidden) to the camera, we render it
         if (!IsVertexHidden(hit_result, camera, vertex, params))
            return true;
         else
         {
            float distance_to_portal = FVector::Distance(hit_result.Location, vertex);

            // If the impact is near the portal, it may be because the portal is integrated in a wall (thus the vertices are hidden)
            // If so, we render it anyway
            if (distance_to_portal < 150)
               return true;
         }
      }
   }

   return false;
}


bool Tools::isActorInCameraViewFrustum(AActor* actor, USceneComponent* camera, float near_plane_distance)
{
   FMinimalViewInfo view_info;

   // Get the MinimalViewInfo of the camera component
   if (camera->IsA(UCameraComponent::StaticClass()))
      Cast<UCameraComponent>(camera)->GetCameraView(0, view_info);

   else if (camera->IsA(USceneCaptureComponent2D::StaticClass()))
   {
      Cast<USceneCaptureComponent2D>(camera)->GetCameraView(0, view_info);
      view_info.OrthoNearClipPlane = near_plane_distance;
   }
   
   else
      return false;

   // Get the View Projection matrix of the camera component
   FMatrix view_matrix, projection_matrix, view_projection_matrix;
   UGameplayStatics::GetViewProjectionMatrix(view_info, view_matrix, projection_matrix, view_projection_matrix);

   // Get the camera frustum
   FConvexVolume frustum;
   GetViewFrustumBounds(frustum, view_projection_matrix, true);

   // Get the bounding box of the actor
   FVector portal_origin, portal_extent;
   actor->GetActorBounds(true, portal_origin, portal_extent, false);

   return frustum.IntersectBox(portal_origin, portal_extent);
}


bool Tools::IsPointInsideBox(FVector point, UBoxComponent* box)
{
   if (box)
   {
      // From : https://stackoverflow.com/questions/52673935/check-if-3d-point-inside-a-box/52674010

      FVector center = box->GetComponentLocation();
      FVector half = box->GetScaledBoxExtent();
      FVector direction_x = box->GetForwardVector();
      FVector direction_y = box->GetRightVector();
      FVector direction_z = box->GetUpVector();

      FVector direction = point - center;

      bool IsInside = FMath::Abs(FVector::DotProduct(direction, direction_x)) <= half.X 
                   && FMath::Abs(FVector::DotProduct(direction, direction_y)) <= half.Y 
                   && FMath::Abs(FVector::DotProduct(direction, direction_z)) <= half.Z;

      return IsInside;
   }

   return false;
}


float Tools::ComputeIncidenceAngle(const FVector& cam_pos, const FVector& portal_mid, const FVector& portal_normal)
{
   FVector incident_ray = portal_mid - cam_pos;

   return AngleBetweenVectors(incident_ray, portal_normal);
}


float Tools::ComputeRefractionAngle(float incidence_angle, float n1, float n2)
{
   float sin_angle = FMath::Abs(n1 / n2 * FMath::Sin(FMath::DegreesToRadians(incidence_angle)));

   // If there is total reflection, returns -1
   if (sin_angle > 1)
      return -1.f;

   return FMath::RadiansToDegrees(FMath::Asin(sin_angle));
}


bool Tools::IsVertexHidden(FHitResult& OUT_hit_result, USceneComponent* camera, const FVector& vertex, FCollisionQueryParams& params)
{
   ECollisionChannel collision_channel = ECollisionChannel::ECC_Camera;

   // If it's the player camera, we do a simple ray cast test
   if (camera->IsA(UCameraComponent::StaticClass()))
      return camera->GetWorld()->LineTraceSingleByChannel(OUT_hit_result, camera->GetComponentLocation(), vertex, collision_channel, params);

   // If it's a scene capture
   else if (camera->IsA(UPortalSceneCapture::StaticClass()))
   {
      UPortalSceneCapture* scene_capture = Cast<UPortalSceneCapture>(camera);
      APortal* linked_portal = scene_capture->GetLinkedPortal();
      FVector start, end;

      // We have to cast the ray in front of the portal for the collision to happen
      if (scene_capture->IsExitInFront() || scene_capture->getType() == ECameraType::Mirror)
      {
         start = vertex;
         end = camera->GetComponentLocation();
      }
      else
      {
         start = camera->GetComponentLocation();
         end = vertex;
      }

      bool is_vertex_visible_inside_linked_portal = linked_portal->ActorLineTraceSingle(OUT_hit_result, start, end, collision_channel, params);

      // If the vertex is not visible inside linked portal, it's hidden
      if (!is_vertex_visible_inside_linked_portal)
         return true;

      // Otherwise, we check if it's not hidden behind something behind linked_portal
      else
      {
         params.AddIgnoredActor(linked_portal);
         return camera->GetWorld()->LineTraceSingleByChannel(OUT_hit_result, OUT_hit_result.Location, vertex, collision_channel, params);
      }
   }

   return true;
}


float Tools::AngleBetweenVectors(const FVector& A, const FVector& B)
{
   return FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(A, B) / (A.Size() * B.Size())));
}
