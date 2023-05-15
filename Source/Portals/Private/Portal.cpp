// Fill out your copyright notice in the Description page of Project Settings.

#include "Portal.h"

#include <utility> /* std::move */
#include <Components/ActorComponent.h>
#include <Engine/TextureRenderTarget2D.h>
#include <Runtime/Engine/Public/EngineUtils.h>

#include "PortalTools.h"
#include "PortalSceneCapture.h"


FTextureToRender::FTextureToRender(FTextureToRender&& other) noexcept
{
   texture = other.texture;
   is_mirror = other.is_mirror;
   weight = other.weight;

   other.texture = nullptr;
   other.is_mirror = false;
   other.weight = 0.f;
}

FTextureToRender::FTextureToRender(const FTextureToRender& other) noexcept
{
   texture = other.texture;
   is_mirror = other.is_mirror;
   weight = other.weight;
}

FTextureToRender& FTextureToRender::operator=(FTextureToRender&& other) noexcept
{
   if (this != &other)
   {
      delete texture;

      texture = other.texture;
      is_mirror = other.is_mirror;
      weight = other.weight;

      other.texture = nullptr;
      other.is_mirror = false;
      other.weight = 0.f;
   }

   return *this;
}

FTextureToRender& FTextureToRender::operator=(const FTextureToRender& other) noexcept
{
   if (this != &other)
   {
      delete texture;

      texture = other.texture;
      is_mirror = other.is_mirror;
      weight = other.weight;
   }

   return *this;
}


// Sets default values
APortal::APortal(const FObjectInitializer& ObjectInitializer) :
   Super(ObjectInitializer),
   m_is_active(false)
{
   PrimaryActorTick.bCanEverTick = true;

   m_is_renderer_but_not_captured = false;

   RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
   RootComponent->Mobility = EComponentMobility::Static;

   m_portal_mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PortalMesh"));
   m_portal_mesh->AttachToComponent(RootComponent, FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));
}


void APortal::BeginPlay()
{
   Super::BeginPlay();

   LoadMeshVertices();
}


void APortal::LoadMeshVertices() const
{
   m_vertices.Empty(4);

   m_middle_point = FVector::ZeroVector;

   // Vertex Buffer
   if (!IsValidLowLevel()) return;
   if (!m_portal_mesh) return;
   if (!m_portal_mesh->GetStaticMesh()) return;
   if (!m_portal_mesh->GetStaticMesh()->GetRenderData()) return;

   FStaticMeshLODResources& LOD_model = m_portal_mesh->GetStaticMesh()->GetRenderData()->LODResources[0];
   int32 nb_vertices = LOD_model.VertexBuffers.StaticMeshVertexBuffer.GetNumVertices();

   for (int32 vertex_index = 0; vertex_index < nb_vertices; ++vertex_index)
   {
      const FVector3f& local_position = LOD_model.VertexBuffers.PositionVertexBuffer.VertexPosition(vertex_index);
      const FVector world_position = m_portal_mesh->GetComponentTransform().TransformPosition(UE::Math::TVector<double>(local_position));
      m_vertices.Add(world_position);

      m_middle_point += world_position;
   }

   m_middle_point /= nb_vertices;
}


const TArray<FVector>* APortal::GetMeshVertices() const
{
   if (m_vertices.Num() == 0)
      LoadMeshVertices();

   return &m_vertices;
}


// Recursive function
void APortal::Render(const FTransform watched_actor_transform, const FMatrix& projection_matrix, unsigned int depth)
{
   // In case we have to make copies to restore them afterwards
   TMap<APortal*, TArray<UTextureRenderTarget2D*>> portal_to_textures_copies;

   UpdateCaptures(watched_actor_transform, projection_matrix);

   // If we reached the max depth, we directly capture the scene
   if (depth < m_MAX_RENDER_DEPTH)
   {
      TMap<APortal*, FTransform> visible_portals_to_watched_actor;

      // For each portal, if it's visible from the current one, store it
      for (TActorIterator<APortal> portals_it(GetWorld()); portals_it; ++portals_it)
      {
         APortal* portal = *portals_it;

         for (auto* scene_capture : m_scene_captures)
         {
            // If the portal checked is the one linked to the current SC, we ignore it
            if (portal != scene_capture->GetLinkedPortal())
            {
               // Distance between the camera and the linked portal
               float near_plane_distance = FMath::Abs(FVector::Dist(scene_capture->GetComponentLocation(), scene_capture->GetLinkedPortal()->GetActorLocation()));

               if (Tools::isPortalVisibleToCamera(portal, scene_capture, near_plane_distance))
               {
                  // If the portal is waiting to be captured, we create a temporary copy of its textures
                  if (portal->IsRendererButNotCaptured())
                  {
                     TArray<UTextureRenderTarget2D*> texture_copies;
                     for (auto* scene_capture_to_copy : portal->m_scene_captures)
                     {
                        auto* texture_original = scene_capture_to_copy->GetRenderTarget();
                        auto* texture_copy = DuplicateObject<UTextureRenderTarget2D>(texture_original, texture_original->GetOuter(), "Texture copy");
                        scene_capture_to_copy->SetRenderTarget(texture_copy);

                        texture_copies.Add(texture_original);
                     }

                     portal_to_textures_copies.Add(portal, std::move(texture_copies));
                  }

                  visible_portals_to_watched_actor.Add(portal, scene_capture->GetComponentTransform());
               }
            }
         }
      }

      // Rendering every visible portal
      for (auto& visible_portal : visible_portals_to_watched_actor)
      {
         APortal* portal = visible_portal.Key;
         FTransform wa_transform = visible_portal.Value;

         portal->Render(wa_transform, projection_matrix, depth + 1);
      }

      for (auto& visible_portal : visible_portals_to_watched_actor)
         visible_portal.Key->SetIsRendererButNotCaptured(false);
   }

   UpdateCaptures(watched_actor_transform, projection_matrix);
   UpdatePortalTexture();

   // We restore all texture copies
   for (auto& copied_portal : portal_to_textures_copies)
   {
      APortal* portal = copied_portal.Key;
      for (int i = 0; i < portal->m_scene_captures.Num(); i++)
         portal->m_scene_captures[i]->SetRenderTarget(copied_portal.Value[i]);

      portal->UpdatePortalTexture();
   }

   // Prevent to override the texture rendered in the next iterations
   SetIsRendererButNotCaptured(true);

   SetActive(true);
}


void APortal::UpdateCaptures(const FTransform& watched_actor_transform, const FMatrix& projection_matrix)
{
   for (UPortalSceneCapture* scene_capture : m_scene_captures)
      scene_capture->Update(watched_actor_transform, projection_matrix);
}


void APortal::UpdatePortalTexture()
{
   TArray<FTextureToRender> portal_textures;

   for (UPortalSceneCapture* scene_capture : m_scene_captures)
   {
      FTextureToRender texture_to_render;
      texture_to_render.is_mirror = scene_capture->getType() == ECameraType::Mirror;
      texture_to_render.weight = scene_capture->getWeight();
      texture_to_render.texture = scene_capture->GetRenderTarget();

      portal_textures.Add(std::move(texture_to_render));
   }

   SetRTT(portal_textures);
}


UPortalSceneCapture* APortal::CreateDefaultSceneCapture()
{
   UPortalSceneCapture* scene_capture = NewObject<UPortalSceneCapture>(this, UPortalSceneCapture::StaticClass());

   scene_capture->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
   scene_capture->RegisterComponent();

   SetDefaultSceneCaptureParameters(scene_capture);

   return scene_capture;
}


void APortal::SetDefaultSceneCaptureParameters(UPortalSceneCapture* inout_scene_capture)
{   
   inout_scene_capture->bCaptureEveryFrame = false;
   inout_scene_capture->bCaptureOnMovement = false;
   inout_scene_capture->LODDistanceFactor = 3;          //Force bigger LODs for faster computations
   inout_scene_capture->TextureTarget = nullptr;
   inout_scene_capture->bEnableClipPlane = true;
   inout_scene_capture->bUseCustomProjectionMatrix = true;
   inout_scene_capture->CaptureSource = ESceneCaptureSource::SCS_SceneColorHDRNoAlpha;


   FPostProcessSettings& capture_settings = inout_scene_capture->PostProcessSettings;

   //Setup Post-Process of SceneCapture (optimization : disable Motion Blur, etc)
   capture_settings.bOverride_AmbientOcclusionQuality = true;
   capture_settings.bOverride_MotionBlurAmount = true;
   capture_settings.bOverride_MotionBlurMax = true;
   capture_settings.bOverride_SceneFringeIntensity = true;
   capture_settings.bOverride_FilmGrainIntensity = true;
   capture_settings.bOverride_ScreenSpaceReflectionQuality = true;
   

   capture_settings.AmbientOcclusionQuality = 100.0f; // 100 = maximum quality
   capture_settings.MotionBlurAmount = 0.0f;   // 0 = disabled
   capture_settings.MotionBlurMax = 0.0f;   // 0 = disabled
   capture_settings.SceneFringeIntensity = 0.0f;   // 0 = disabled
   capture_settings.FilmGrainIntensity = 0.0f;   // 0 = disabled
   capture_settings.ScreenSpaceReflectionQuality = 50.0f;  // 50 = default
}

bool APortal::IsPointInsideBox(FVector point, UBoxComponent* box)
{
   return Tools::IsPointInsideBox(point, box); 
}
