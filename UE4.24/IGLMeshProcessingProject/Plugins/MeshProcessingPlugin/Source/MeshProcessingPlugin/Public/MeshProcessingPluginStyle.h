#pragma once

#include "CoreMinimal.h"
#include "Styling/ISlateStyle.h"
#include "Styling/SlateStyle.h"

//=======================================
// EVERYTHING BELOW HERE IS BOILERPLATE!
//=======================================

class FMeshProcessingPluginStyle
{
public:
	static void Initialize();

	static void Shutdown();

	static TSharedPtr<class ISlateStyle> Get() { return StyleSet; }

	static FName GetStyleSetName();
private:
	static FString InContent(const FString& RelativePath, const ANSICHAR* Extension);

private:
	static TSharedPtr< class FSlateStyleSet > StyleSet;
};
