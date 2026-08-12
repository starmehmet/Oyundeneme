using UnrealBuildTool;

public class SurvivalGame : ModuleRules
{
	public SurvivalGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"NavigationSystem",
			"GameplayTasks",
			"UMG",
			"DeveloperSettings"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Slate",
			"SlateCore",
			"Json",
			"JsonUtilities",
			"AssetRegistry"
		});

		// Landscape yalnizca editor-only arazi ureticisi (LandscapeBuilder.cpp) tarafindan
		// kullanilir — ALandscapeProxy::Import() WITH_EDITOR icindedir. Paketlenmis oyun
		// hedefi bu bagimliligi tasimaz; arazi zaten .umap'e gomulu veri olarak gider.
		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.Add("Landscape");
		}
	}
}
