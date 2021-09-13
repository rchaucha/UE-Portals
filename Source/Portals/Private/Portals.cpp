// Copyright Epic Games, Inc. All Rights Reserved.

#include "Portals.h"
#include "GameFramework/InputSettings.h" 

#define LOCTEXT_NAMESPACE "FPortalsModule"

void FPortalsModule::StartupModule()
{
   SetKeybindings();
}


void FPortalsModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}


void FPortalsModule::SetKeybindings()
{
   UInputSettings* Settings = UInputSettings::GetInputSettings();

   FInputActionKeyMapping jump_action = FInputActionKeyMapping(FName(TEXT("Jump")), EKeys::SpaceBar);
   Settings->RemoveActionMapping(jump_action);
   Settings->AddActionMapping(jump_action);

   setAxisMapping(FName(TEXT("LookUp")), EKeys::MouseY, -1.f);

   setAxisMapping(FName(TEXT("MoveForward")), EKeys::S, -1.f);
   setAxisMapping(FName(TEXT("MoveForward")), EKeys::Z,  1.f);

   setAxisMapping(FName(TEXT("MoveRight")), EKeys::Q, -1.f);
   setAxisMapping(FName(TEXT("MoveRight")), EKeys::D, 1.f);

   setAxisMapping(FName(TEXT("Turn")), EKeys::MouseX, 1.f);

   Settings->SaveKeyMappings();
}


void FPortalsModule::setAxisMapping(FName name, FKey key, float scale)
{
   UInputSettings* Settings = UInputSettings::GetInputSettings();
   
   FInputAxisKeyMapping axis_mapping(name, key, scale);

   Settings->RemoveAxisMapping(axis_mapping);
   Settings->AddAxisMapping(axis_mapping);
}


#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FPortalsModule, Portals)