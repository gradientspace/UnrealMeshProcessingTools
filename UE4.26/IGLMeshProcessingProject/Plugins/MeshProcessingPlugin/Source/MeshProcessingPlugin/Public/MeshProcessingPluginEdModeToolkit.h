#pragma once
#include "CoreMinimal.h"
#include "Toolkits/BaseToolkit.h"

//=======================================
// EVERYTHING BELOW HERE IS BOILERPLATE!
//=======================================

class FMeshProcessingPluginEdMode;

class FMeshProcessingPluginEdModeToolkit : public FModeToolkit
{
public:
	FMeshProcessingPluginEdModeToolkit();
	
	/** FModeToolkit interface */
	virtual void Init(const TSharedPtr<IToolkitHost>& InitToolkitHost) override;

	/** IToolkit interface */
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual class FEdMode* GetEditorMode() const override;
	virtual TSharedPtr<class SWidget> GetInlineContent() const override { return ToolkitWidget; }

	/** Returns the Mode specific tabs in the mode toolbar **/
	virtual void GetToolPaletteNames(TArray<FName>& InPaletteName) const;
	virtual FText GetToolPaletteDisplayName(FName PaletteName) const;
	virtual void BuildToolPalette(FName PaletteName, class FToolBarBuilder& ToolbarBuilder);

	virtual FMeshProcessingPluginEdMode* GetMeshProcessingPluginEdMode() const;

private:

	TSharedPtr<SWidget> ToolkitWidget;

	TSharedPtr<IDetailsView> DetailsView;
	TSharedPtr<STextBlock> ToolNameLabel;

	const static TArray<FName> ToolbarPaletteNames;
};
