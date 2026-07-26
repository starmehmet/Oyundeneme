// Sistem #24 (Content pipeline) — MIMARI.md'nin bu sistem icin hic C++ sinif taslagi YOKTU
// (butun diger sistemlerin aksine, dogrudan "Uygulama Surumu" notuna gecmis), yalnizca
// isimlendirme kurallari + "asset validasyon" ihtiyaci. FBX/PSD import-otomasyonu BILINCLI
// olarak YAZILMADI — projede gercek 3D model/doku/animasyon icerigi yok (Content/ Sistem
// #19/#20'nin de dokumante ettigi gibi buyuk olcude bos), otomatiklestirilecek gercek bir
// "artist export" sureci henuz yok. Bunun yerine: MEVCUT varliklari CLAUDE.md'nin isimlendirme
// kurallarina (BP_/WBP_/DT_/DA_/SM_/SK_/M_/MI_/T_/A_ + projenin zaten kullandigi IA_/IMC_,
// MIMARI'nin listesinde yok ama Content/Blueprints/_Core/Input altinda zaten bu adlandirmayla
// var) gore denetleyen bir konsol komutu.
//
// Inceleme bulgusu (kritik, motor kaynagindan dogrulandi): WITH_EDITOR ile sarmalanmasi GEREKIR.
// Ilk yazimda "AssetRegistry modulu runtime'da da linklenir" gerekcesiyle bu guard atlanmisti —
// bu DOGRU ama YANLIS soruyu cevapliyordu. Asil sorun modulun linklenip linklenmedigi degil,
// COOKLENMIS bir pakette Blueprint/WidgetBlueprint varliklarinin sinif-adi anlaminin
// DEGISMESI: cooking, bir Blueprint paketindeki editor-tarafi UBlueprint/UWidgetBlueprint
// nesnesini (NeedsLoadForClient/Server=false, bkz. Blueprint.cpp) atip yalnizca derlenmis
// Generated Class'i birakir — cooklenmis bir calistirmada AssetRegistry bu varliklari
// "Blueprint"/"WidgetBlueprint" DEGIL "BlueprintGeneratedClass"/"WidgetBlueprintGeneratedClass"
// olarak raporlar. Rules tablosu bunlari eslestiremez, ihlal SAYILMAZ (sessizce atlanir) ve
// arac yine de "temiz" bir ozet basardi — tam da bu projenin en cok kullandigi iki kuralda
// (BP_/WBP_) yanlis bir guven verirdi. Bu, yalnizca editor icindeki (cooklenmemis) icerik icin
// anlamli bir denetim — bu yuzden WITH_EDITOR ile sinirlandirildi.

#include "SurvivalGame.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"

#if WITH_EDITOR
#include "AssetRegistry/IAssetRegistry.h"
#include "AssetRegistry/AssetData.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "Harvesting/HarvestNodeDefinition.h"

namespace
{
	const TMap<FName, FString>& GetPrefixRules()
	{
		static const TMap<FName, FString> Rules = {
			{ FName(TEXT("Blueprint")), TEXT("BP_") },
			{ FName(TEXT("WidgetBlueprint")), TEXT("WBP_") },
			{ FName(TEXT("DataTable")), TEXT("DT_") },
			{ FName(TEXT("StaticMesh")), TEXT("SM_") },
			{ FName(TEXT("SkeletalMesh")), TEXT("SK_") },
			{ FName(TEXT("Material")), TEXT("M_") },
			{ FName(TEXT("MaterialInstanceConstant")), TEXT("MI_") },
			{ FName(TEXT("Texture2D")), TEXT("T_") },
			{ FName(TEXT("AnimSequence")), TEXT("A_") },
			{ FName(TEXT("InputAction")), TEXT("IA_") },
			{ FName(TEXT("InputMappingContext")), TEXT("IMC_") },
		};
		return Rules;
	}

	FAutoConsoleCommandWithWorldAndArgs GCmdContentValidate(
		TEXT("content_validate"),
		TEXT("/Game altindaki varliklari CLAUDE.md isimlendirme kurallarina gore denetler (yalniz editor)"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
			[](const TArray<FString>& Args, UWorld* World)
			{
				IAssetRegistry& Registry = IAssetRegistry::GetChecked();
				if (Registry.IsLoadingAssets())
				{
					Registry.WaitForCompletion();
				}

				// Inceleme bulgusu (majör): CLAUDE.md'nin zorunlu DA_ onegi Rules tablosunda YOKTU
				// (unutulmus). Ayrica DataAsset'in KENDISI genelde dogrudan kullanilmaz, alt
				// siniflanir (ör. planlanan DA_GameConfig) — AssetClassPath tam olarak "DataAsset"
				// DEGIL alt-sinifin adini dondurur, bu yuzden basit bir TMap girdisi yetmez.
				// GetDerivedClassNames ile UDataAsset'ten tureyen TUM sinif adlarini onceden
				// topluyoruz (motor kaynagindan dogrulanan API, IAssetRegistry.h).
				TSet<FTopLevelAssetPath> DataAssetDerivedClasses;
				{
					const FTopLevelAssetPath DataAssetClassPath(UDataAsset::StaticClass()->GetPathName());
					Registry.GetDerivedClassNames({ DataAssetClassPath }, {}, DataAssetDerivedClasses);
					DataAssetDerivedClasses.Add(DataAssetClassPath);
				}

				TArray<FAssetData> Assets;
				Registry.GetAssetsByPath(FName(TEXT("/Game")), Assets, /*bRecursive=*/true);

				const TMap<FName, FString>& Rules = GetPrefixRules();
				int32 Checked = 0;
				int32 Violations = 0;

				for (const FAssetData& Asset : Assets)
				{
					const FName ClassName = Asset.AssetClassPath.GetAssetName();
					const FString AssetName = Asset.AssetName.ToString();

					// PIE'de CANLI bulunan bulgu: InputAction/InputMappingContext GERCEKTEN
					// UDataAsset'ten tureiyor (motor kaynagindan dogrulandi) — bu yuzden DataAsset
					// soyagaci kontrolu TMap'teki DAHA OZEL IA_/IMC_ kuralindan ONCE calisirsa
					// bu iki turu yanlislikla "DA_" bekler hale getirir. TMap (ozel/tam-eslesme
					// kurallari) HER ZAMAN once kontrol edilir; DataAsset soyagaci kontrolu yalnizca
					// TMap'te ozel bir kural TANIMLI DEGILSE devreye girer.
					const FString* ExpectedPrefix = Rules.Find(ClassName);
					if (ExpectedPrefix)
					{
						++Checked;
						if (!AssetName.StartsWith(*ExpectedPrefix))
						{
							++Violations;
							UE_LOG(LogSurvival, Warning,
								TEXT("content_validate: '%s' (%s) beklenen on-ek '%s' ile baslamiyor — %s"),
								*AssetName, *ClassName.ToString(), **ExpectedPrefix, *Asset.PackageName.ToString());
						}
						continue;
					}

					if (DataAssetDerivedClasses.Contains(Asset.AssetClassPath))
					{
						++Checked;
						if (!AssetName.StartsWith(TEXT("DA_")))
						{
							++Violations;
							UE_LOG(LogSurvival, Warning,
								TEXT("content_validate: '%s' (%s) beklenen on-ek 'DA_' ile baslamiyor — %s"),
								*AssetName, *ClassName.ToString(), *Asset.PackageName.ToString());
						}
						continue;
					}

					// bu varlik turu icin (ne TMap'te ne DataAsset soyagacinda) kural tanimli degil
					// — ihlal SAYILMAZ, sessizce atlanir (bkz. dosya basi yorumu).
				}

				UE_LOG(LogSurvival, Log,
					TEXT("content_validate: %d varlik tarandi, %d/%d kural-tanimli varlik uygun (%d ihlal, %d varlik icin kural yok)"),
					Assets.Num(), Checked - Violations, Checked, Violations, Assets.Num() - Checked);

				// Sistem #29 (Hasat Dugumleri) — Docs/MIMARI.md #29'da vaat edilen entegrasyon: DT_HarvestNodes'un
				// her satirinin YieldItemID'si DT_Items'ta gercekten var mi (Recipes/Buildings/ProductionRecipes
				// icin zaten elle yapilan capraz kontrolun otomatiklestirilmis hali). Ikisi de /Game altinda zaten
				// taranmis Assets listesinden bulunur — ayrica bir AssetRegistry sorgusu gerekmez.
				const UDataTable* HarvestTable = nullptr;
				const UDataTable* ItemTable = nullptr;
				for (const FAssetData& Asset : Assets)
				{
					if (Asset.AssetClassPath.GetAssetName() != FName(TEXT("DataTable")))
					{
						continue;
					}
					if (Asset.AssetName == FName(TEXT("DT_HarvestNodes")))
					{
						HarvestTable = Cast<UDataTable>(Asset.GetAsset());
					}
					else if (Asset.AssetName == FName(TEXT("DT_Items")))
					{
						ItemTable = Cast<UDataTable>(Asset.GetAsset());
					}
				}

				if (HarvestTable && ItemTable)
				{
					int32 RefChecked = 0;
					int32 RefViolations = 0;
					HarvestTable->ForeachRow<FHarvestNodeDefinition>(TEXT("content_validate::HarvestRef"),
						[&](const FName& RowName, const FHarvestNodeDefinition& Row)
						{
							++RefChecked;
							if (!ItemTable->GetRowMap().Contains(Row.YieldItemID))
							{
								++RefViolations;
								UE_LOG(LogSurvival, Warning,
									TEXT("content_validate: DT_HarvestNodes['%s'].YieldItemID '%s' DT_Items'ta bulunamadi"),
									*RowName.ToString(), *Row.YieldItemID.ToString());
							}
						});
					UE_LOG(LogSurvival, Log,
						TEXT("content_validate: DT_HarvestNodes referans butunlugu: %d satir tarandi, %d ihlal"),
						RefChecked, RefViolations);
				}
			}));
}
#endif // WITH_EDITOR
