#include "MeshProcessingPluginEdModeToolkit.h"
#include "MeshProcessingPluginEdMode.h"
#include "Engine/Selection.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "EditorModeManager.h"

#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"
#include "IDetailRootObjectCustomization.h"
#include "MeshProcessingPluginEdMode.h"
#include "InteractiveTool.h"
#include "InteractiveToolManager.h"
#include "MeshProcessingPluginCommands.h"


#define LOCTEXT_NAMESPACE "FMeshProcessingPluginEdModeToolkit"


// [1] 
// This function adds Toolbar buttons for Commands. If you want to add a new command,
// first set it up in MeshProcessingPluginCommands.h/cpp, then add a button for it here,
// then register the tool in MeshProcessingPluginEdMode.cpp:RegisterModeTools()
//
void FMeshProcessingPluginEdModeToolkit::BuildToolPalette(FName PaletteIndex, class FToolBarBuilder& ToolbarBuilder)
{
	const FMeshProcessingPluginCommands& Commands = FMeshProcessingPluginCommands::Get();

	ToolbarBuilder.AddToolBarButton(Commands.AcceptActiveTool);
	ToolbarBuilder.AddToolBarButton(Commands.CancelActiveTool);
	ToolbarBuilder.AddToolBarButton(Commands.CompleteActiveTool);

	ToolbarBuilder.AddSeparator();

	ToolbarBuilder.AddToolBarButton(Commands.BeginIGLSmoothingTool);
	ToolbarBuilder.AddToolBarButton(Commands.BeginMeshExportTool);
}




//=======================================
// EVERYTHING BELOW HERE IS BOILERPLATE!
//=======================================



class FHideObjectNamesDetailRootObjectCustomization : public IDetailRootObjectCustomization
{
public:
	FHideObjectNamesDetailRootObjectCustomization()
	{
	}

	virtual TSharedPtr<SWidget> CustomizeObjectHeader(const UObject* InRootObject) override
	{
		return SNew(STextBlock).Text(FText::FromString(InRootObject->GetName()));
	}

	virtual bool IsObjectVisible(const UObject* InRootObject) const override
	{
		return true;
	}

	virtual bool ShouldDisplayHeader(const UObject* InRootObject) const override
	{
		return false;
	}
};



FMeshProcessingPluginEdModeToolkit::FMeshProcessingPluginEdModeToolkit()
{
}

void FMeshProcessingPluginEdModeToolkit::Init(const TSharedPtr<IToolkitHost>& InitToolkitHost)
{
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");

	FDetailsViewArgs DetailsViewArgs(false, false, false, FDetailsViewArgs::HideNameArea, true, nullptr, false, NAME_None);
	DetailsViewArgs.DefaultsOnlyVisibility = EEditDefaultsOnlyNodeVisibility::Automatic;
	DetailsViewArgs.bShowOptions = false;
	DetailsViewArgs.bAllowMultipleTopLevelObjects = true;
	DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);

	// add customization that hides header labels
	TSharedPtr<FHideObjectNamesDetailRootObjectCustomization> RootObjectCustomization
		= MakeShared<FHideObjectNamesDetailRootObjectCustomization>();
	DetailsView->SetRootObjectCustomizationInstance(RootObjectCustomization);

	ToolNameLabel = SNew(STextBlock).AutoWrapText(true);
	ToolNameLabel->SetJustification(ETextJustify::Center);

	SAssignNew(ToolkitWidget, SBorder).HAlign(HAlign_Fill).Padding(4)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Fill).Padding(5)
				[ ToolNameLabel->AsShared() ]
			+ SVerticalBox::Slot().HAlign(HAlign_Fill).FillHeight(1.f)
				[ DetailsView->AsShared() ]
		];
		
	FModeToolkit::Init(InitToolkitHost);


	GetMeshProcessingPluginEdMode()->GetToolManager()->OnToolStarted.AddLambda([this](UInteractiveToolManager* Manager, UInteractiveTool* Tool)
	{
		UInteractiveTool* CurTool = GetMeshProcessingPluginEdMode()->GetToolManager()->GetActiveTool(EToolSide::Left);
		DetailsView->SetObjects(CurTool->GetToolProperties());
		ToolNameLabel->SetText(CurTool->GetClass()->GetDisplayNameText());
		//ToolHeaderLabel->SetText(FString("(Tool Name Here)"));
	});
	GetMeshProcessingPluginEdMode()->GetToolManager()->OnToolEnded.AddLambda([this](UInteractiveToolManager* Manager, UInteractiveTool* Tool)
	{
		DetailsView->SetObject(nullptr);
		ToolNameLabel->SetText(LOCTEXT("SelectToolLabel", "Select a Tool"));
	});


}

FName FMeshProcessingPluginEdModeToolkit::GetToolkitFName() const
{
	return FName("MeshProcessingPluginEdMode");
}

FText FMeshProcessingPluginEdModeToolkit::GetBaseToolkitName() const
{
	return NSLOCTEXT("MeshProcessingPluginEdModeToolkit", "DisplayName", "MeshProcessingPluginEdMode Tool");
}

class FEdMode* FMeshProcessingPluginEdModeToolkit::GetEditorMode() const
{
	return GLevelEditorModeTools().GetActiveMode(FMeshProcessingPluginEdMode::EM_MeshProcessingPluginEdModeId);
}

FMeshProcessingPluginEdMode* FMeshProcessingPluginEdModeToolkit::GetMeshProcessingPluginEdMode() const
{
	return (FMeshProcessingPluginEdMode*)GetEditorMode();
}




const TArray<FName> FMeshProcessingPluginEdModeToolkit::ToolbarPaletteNames = { FName(TEXT("Tools")) };

void FMeshProcessingPluginEdModeToolkit::GetToolPaletteNames(TArray<FName>& InPaletteName) const
{
	InPaletteName = ToolbarPaletteNames;
}

FText FMeshProcessingPluginEdModeToolkit::GetToolPaletteDisplayName(FName Palette)
{
	return FText::FromName(Palette);
}


#undef LOCTEXT_NAMESPACE
