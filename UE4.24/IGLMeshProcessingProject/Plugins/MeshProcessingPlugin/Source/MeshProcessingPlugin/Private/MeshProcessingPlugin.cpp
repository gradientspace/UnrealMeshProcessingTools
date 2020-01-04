#include "MeshProcessingPlugin.h"
#include "MeshProcessingPluginEdMode.h"
#include "MeshProcessingPluginCommands.h"
#include "MeshProcessingPluginStyle.h"

#define LOCTEXT_NAMESPACE "FMeshProcessingPluginModule"

void FMeshProcessingPluginModule::StartupModule()
{
	FCoreDelegates::OnPostEngineInit.AddRaw(this, &FMeshProcessingPluginModule::OnPostEngineInit);
}

void FMeshProcessingPluginModule::ShutdownModule()
{
	FCoreDelegates::OnPostEngineInit.RemoveAll(this);

	FMeshProcessingPluginCommands::Unregister();
	FMeshProcessingPluginStyle::Shutdown();

	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FEditorModeRegistry::Get().UnregisterMode(FMeshProcessingPluginEdMode::EM_MeshProcessingPluginEdModeId);
}


void FMeshProcessingPluginModule::OnPostEngineInit()
{
	FMeshProcessingPluginStyle::Initialize();

	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	FEditorModeRegistry::Get().RegisterMode<FMeshProcessingPluginEdMode>(FMeshProcessingPluginEdMode::EM_MeshProcessingPluginEdModeId, LOCTEXT("MeshProcessingPluginEdModeName", "MeshProcessingPluginEdMode"), FSlateIcon(), true);

	FMeshProcessingPluginCommands::Register();
}


#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FMeshProcessingPluginModule, MeshProcessingPlugin)