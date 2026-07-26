# UE5 Survival / İndustriyel Yerleşim Oyunu — Üretim Mimarisi

**Hedef:** Ticari Steam yayını | Üçüncü şahıs survival aşırı hava koşulları ile | İnşaat, bakım, kendi kendine yeten yerleşim
**Motor:** Unreal Engine 5 | C++ + Blueprints | World Partition entegrasyonu
**Ölçeklenebilirlik:** 64+ oyuncu yapısı, gerçek zamanlı kaynak simülasyonu, 500+ üretim döngüsü/frame

---

## PROJE KLASÖR YAPISI

```
SurvivalGame/
├── Content/
│   ├── Blueprints/
│   │   ├── _Core/                          # Tekil desenler, başlatma
│   │   │   ├── GM_MainGame.uasset
│   │   │   ├── PS_Game.uasset
│   │   │   └── PC_Player.uasset
│   │   ├── Players/
│   │   │   ├── Animations/
│   │   │   ├── PlayerCharacter.uasset
│   │   │   └── States/
│   │   ├── Interaction/
│   │   │   ├── BP_InteractionComponent.uasset
│   │   │   ├── BP_InteractableBase.uasset
│   │   │   └── Widgets/
│   │   ├── Inventory/
│   │   │   ├── BP_InventoryComponent.uasset
│   │   │   └── UI/
│   │   ├── Crafting/
│   │   │   ├── BP_CraftingStation.uasset
│   │   │   └── UI/
│   │   ├── Construction/
│   │   │   ├── BP_BuildingBase.uasset
│   │   │   ├── BP_FoundationGrid.uasset
│   │   │   └── UI/
│   │   ├── Production/
│   │   │   ├── BP_ProductionMachine.uasset
│   │   │   ├── BP_Furnace.uasset
│   │   │   ├── BP_Refinery.uasset
│   │   │   └── BP_ChemicalReactor.uasset
│   │   ├── Logistics/
│   │   │   ├── BP_ConveyorBelt.uasset
│   │   │   ├── BP_StorageContainer.uasset
│   │   │   └── BP_TransportDrone.uasset
│   │   ├── Weather/
│   │   │   ├── BP_WeatherManager.uasset
│   │   │   └── VFX/
│   │   ├── NPC/
│   │   │   ├── BP_NPCWorker.uasset
│   │   │   ├── Animations/
│   │   │   └── Behaviors/
│   │   └── UI/
│   │       ├── HUD/
│   │       ├── Menus/
│   │       └── Widgets/
│   ├── Maps/
│   │   ├── MainMenu.umap
│   │   ├── MainWorld/
│   │   └── Tests/
│   ├── Data/
│   │   ├── DataTables/
│   │   │   ├── DT_Items.uasset
│   │   │   ├── DT_Recipes.uasset
│   │   │   ├── DT_Machines.uasset
│   │   │   └── DT_Structures.uasset
│   │   └── DataAssets/
│   │       ├── DA_GameConfig.uasset
│   │       └── DA_WeatherProfiles.uasset
│   └── Sounds/
│       ├── Music/
│       ├── SFX/
│       └── Ambient/
├── Source/
│   ├── SurvivalGame/
│   │   ├── Public/
│   │   │   ├── Core/
│   │   │   │   ├── SurvivalGameMode.h
│   │   │   │   ├── SurvivalPlayerController.h
│   │   │   │   ├── GameClock.h
│   │   │   │   └── SaveGameManager.h
│   │   │   ├── Player/
│   │   │   │   ├── PlayerCharacter.h
│   │   │   │   └── InputHandler.h
│   │   │   ├── Interaction/
│   │   │   │   ├── InteractionComponent.h
│   │   │   │   └── InteractableInterface.h
│   │   │   ├── Inventory/
│   │   │   │   ├── InventoryComponent.h
│   │   │   │   └── ItemData.h
│   │   │   ├── Items/
│   │   │   │   ├── ItemDatabase.h
│   │   │   │   └── ItemInstance.h
│   │   │   ├── Crafting/
│   │   │   │   ├── CraftingComponent.h
│   │   │   │   └── RecipeDatabase.h
│   │   │   ├── Construction/
│   │   │   │   ├── ConstructionComponent.h
│   │   │   │   └── BuildingBase.h
│   │   │   ├── Production/
│   │   │   │   ├── ProductionMachine.h
│   │   │   │   └── ProductionSimulation.h
│   │   │   ├── Logistics/
│   │   │   │   ├── LogisticsNetwork.h
│   │   │   │   └── ConveyorBelt.h
│   │   │   ├── Weather/
│   │   │   │   ├── WeatherManager.h
│   │   │   │   ├── TemperatureSimulation.h
│   │   │   │   ├── WindSimulation.h
│   │   │   │   └── SnowAccumulation.h
│   │   │   ├── Time/
│   │   │   │   ├── GameClock.h
│   │   │   │   ├── DayNightCycle.h
│   │   │   │   └── TimeKeeper.h
│   │   │   ├── NPC/
│   │   │   │   ├── NPCCharacter.h
│   │   │   │   ├── NPCBrain.h
│   │   │   │   └── TaskScheduler.h
│   │   │   ├── Audio/
│   │   │   │   ├── AudioManager.h
│   │   │   │   └── MusicManager.h
│   │   │   ├── UI/
│   │   │   │   ├── HUDController.h
│   │   │   │   └── ScreenManager.h
│   │   │   ├── Save/
│   │   │   │   ├── SaveGameManager.h
│   │   │   │   └── SaveDataSerializer.h
│   │   │   └── World/
│   │   │       ├── WorldPartitionHelper.h
│   │   │       └── LevelStreamingManager.h
│   │   └── Private/
│   │       ├── Core/
│   │       ├── Player/
│   │       ├── [... tüm sistemler ...]
│   │       └── Tests/
│   └── SurvivalGame.Build.cs
├── Tests/
│   ├── Integration/
│   ├── Unit/
│   └── Functional/
├── Docs/
│   ├── MIMARISI.md
│   ├── API_REFERENSI.md
│   └── ÖLÇEKLENEBILIRLIK.md
├── .gitignore
├── LICENSE
└── README.md
```

---

## SİSTEM MİMARİSİ

### 1. OYUNCU KONTROLÖRÜ SİSTEMİ

**Sorumluluklar:**
- Oyuncu girdisi (klavye, gamepad, fare)
- Kamera yönetimi (birinci/üçüncü şahıs)
- Karakter hareketi ve durumu
- Etkileşim raycasting'i
- Ağ çoğaltma (çok oyunculu temel)

**Temel Sınıflar:**
```cpp
class ASurvivalPlayerController : public APlayerController
{
    UPROPERTY() APlayerCharacter* ControlledCharacter;
    UPROPERTY() class UInteractionComponent* InteractionComponent;
    UPROPERTY() class UCameraManager* CameraManager;
    
    void BeginPlay() override;
    void SetupInput(class UEnhancedInputComponent* EnhancedInputComponent);
    void ProcessMovementInput(const FInputActionValue& Value);
    void OnInteractPressed();
};

class APlayerCharacter : public ACharacter
{
    UPROPERTY() class UInventoryComponent* Inventory;
    UPROPERTY() class UInteractionComponent* Interaction;
    
    virtual void Tick(float DeltaTime) override;
    void UpdateMovementState(const FVector& InputDirection);
};
```

**Ölçeklenebilirlik Riski:**
- Raycast sıklığı (her frame = sorun)
- **Çözüm:** Raycast 4 frame'de bir, ekranda gösterim önbellekli sonuç kullanır

**Uygulama Sırası:** 1. (temel)

---

### 2. ETKİLEŞİM ÇERÇEVESI

**Sorumluluklar:**
- Etkileşim nesnelerini sorgula (raycast/overlap)
- Uygunluğu doğrula (uzaklık, görüş hattı)
- Etkileşim geribildirimi (vurgulama, prompt)
- Bağlamsal etkileşimler (nesne üzerine nesne kullan)

**Temel Sınıflar:**
```cpp
class UInteractableInterface
{
    virtual void OnInteractionStarted(APlayerCharacter* Player) = 0;
    virtual bool CanInteract(APlayerCharacter* Player) const = 0;
    virtual FText GetInteractionPrompt() const = 0;
};

class UInteractionComponent : public UActorComponent
{
    UPROPERTY() TArray<AActor*> InteractableActors;
    UPROPERTY() AActor* CurrentFocusedInteractable;
    
    void QueryInteractablesInRange();
    void AttemptInteraction();
};
```

**Veri Akışı:**
```
Oyuncu E tuşuna basar
    ↓
Raycast yapılır
    ↓
CanInteract() doğrulanır
    ↓
OnInteractionStarted() çalıştırılır
    ↓
Hedef-spesifik geri çağrı (kapı aç, öğe al, vb.)
```

**Ölçeklenebilirlik Riski:**
- Raycast sıklığı: 100+ etkileşimli nesne
- **Çözüm:** Spatial grid (10.000 UU hücreler), raycast sadece 4 frame'de bir

**Uygulama Sırası:** 2. (oyuncu kontrolöründen sonra)

---

### 3. ENVANTER SİSTEMİ

**Sorumluluklar:**
- Öğe örneklerini depolama (oyuncu sırt çantası)
- Yığılabilir öğeleri yönetme
- Konteyner arasında öğe transferi
- Ağırlık/kapasite yönetimi
- Dayanıklılık takibi
- Kaydetme/yükleme serileştirmesi

**Temel Sınıflar:**
```cpp
struct FInventorySlot
{
    UPROPERTY() class UItemInstance* Item;
    UPROPERTY() int32 Count;
    
    bool IsEmpty() const { return Item == nullptr; }
};

class UInventoryComponent : public UActorComponent
{
    UPROPERTY() TArray<FInventorySlot> Slots;
    UPROPERTY() float MaxCarryWeight;
    UPROPERTY() float CurrentWeight;
    
    bool AddItem(class UItemInstance* Item, int32 Count = 1);
    bool RemoveItem(class UItemInstance* Item, int32 Count = 1);
    
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_Two(FOnInventoryChanged, int32, SlotIndex, const FInventorySlot&, NewSlot);
    FOnInventoryChanged OnInventoryChanged;
};
```

**Ölçeklenebilirlik Riski:**
- Öğe araması: Lineer (50+ öğe)
- **Çözüm:** TMap dizini (tanım → sayı)
- UI yenileme çok sık
- **Çözüm:** Batch değişiklikler, tek broadcast

**Uygulama Sırası:** 3. (etkileşimden sonra)

---

### 4. ÖĞEM VERİTABANI

**Sorumluluklar:**
- Tüm öğe tiplerini tanımla (araçlar, malzeme, bileşenler, yiyecek)
- Öğe meta verilerini sakla (simge, ağırlık, nadir, dayanıklılık)
- Kategori/etiket yönetimi
- Öğe örnekleri oluştur

**Temel Sınıflar:**
```cpp
USTRUCT(BlueprintType)
struct FItemDefinition
{
    UPROPERTY(EditAnywhere) FString ItemID;
    UPROPERTY(EditAnywhere) FText DisplayName;
    UPROPERTY(EditAnywhere) float Weight;
    UPROPERTY(EditAnywhere) int32 MaxStackSize;
    UPROPERTY(EditAnywhere) float MaxDurability;
    UPROPERTY(EditAnywhere) TArray<FString> Tags;
};

class UItemDatabase : public UGameInstanceSubsystem
{
    UPROPERTY() UDataTable* ItemDefinitionTable;  // DT_Items
    TMap<FString, FItemDefinition*> ItemCache;
    
    FItemDefinition* GetItemDefinition(const FString& ItemID);
    UItemInstance* CreateItemInstance(const FString& ItemID);
};
```

**Uygulama Sırası:** 4. (envanter ile paralel)

---

### 5. ZANAAT SİSTEMİ

**Sorumluluklar:**
- Tarifleri tanımla (malzeme, çıktı, zaman, iş istasyonu)
- Uygunluğu doğrula (oyuncu malzemelere sahip mi)
- Zanaat yap (malzemeleri tüket, çıktı üret)
- Zanaat sırasını tut
- İlerlemeyi broadcast et

**Temel Sınıflar:**
```cpp
USTRUCT(BlueprintType)
struct FRecipeDefinition
{
    UPROPERTY(EditAnywhere) FString RecipeID;
    UPROPERTY(EditAnywhere) FText RecipeName;
    UPROPERTY(EditAnywhere) TArray<FRecipeIngredient> Ingredients;
    UPROPERTY(EditAnywhere) TArray<FRecipeOutput> Outputs;
    UPROPERTY(EditAnywhere) float CraftingTime;
};

class UCraftingComponent : public UActorComponent
{
    UPROPERTY() TArray<FCraftingJob> ActiveJobs;
    
    bool CanCraftRecipe(const FRecipeDefinition* Recipe) const;
    void StartCrafting(const FRecipeDefinition* Recipe);
};

struct FCraftingJob
{
    FRecipeDefinition* Recipe;
    float ElapsedTime;
    float GetProgress() const { return ElapsedTime / Recipe->CraftingTime; }
};
```

**Veri Akışı:**
```
Oyuncu tarifi seçer
    ↓
ValidateRecipe() [envanter kontrolü]
    ↓
StartCrafting() [malzemeleri tüket, zamanlayıcı başlat]
    ↓
Tick: İlerlemeyi güncelle
    ↓
Tamamlanınca: Çıktı öğelerini üret
```

**Uygulama Sırası:** 7. (envanter, öğem veri tabanından sonra)

---

### 6. İNŞAAT SİSTEMİ

**Sorumluluklar:**
- İnşa edilebilir yapıları tanımla (gerekli malzeme, boyutlar, yerleştirme kuralları)
- Temel ızgara sistemi (ızgaraya yapışma yerleştirmesi)
- Ön izleme/hayalet yerleştirme
- Çarpışma algılama (yerleştirme doğrulaması)
- İnşaatı yap (malzemeleri tüket, yapı oluştur)
- Yıkım/yok etme

**Temel Sınıflar:**
```cpp
USTRUCT(BlueprintType)
struct FBuildingDefinition
{
    UPROPERTY(EditAnywhere) FString BuildingID;
    UPROPERTY(EditAnywhere) FVector Dimensions;
    UPROPERTY(EditAnywhere) TArray<FInventorySlot> RequiredMaterials;
    UPROPERTY(EditAnywhere) float ConstructionTime;
};

class AFoundationGrid : public AActor
{
    UPROPERTY() float GridSize;  // 400 UU hücreler
    TSet<FIntPoint> OccupiedCells;
    TMap<FIntPoint, class ABuildingBase*> BuildingMap;
    
    FVector SnapToGrid(const FVector& WorldPosition);
    bool CanPlaceBuilding(const FBuildingDefinition* Def, const FVector& Position);
};

class ABuildingBase : public AActor
{
    UPROPERTY() FBuildingDefinition* Definition;
    UPROPERTY() float ConstructionProgress;
    UPROPERTY() bool bIsConstructed;
    
    void AcceptConstruction(float ConstructionTime);
    void Demolish();
};
```

**Ölçeklenebilirlik Riski:**
- 500 bina = çarpışma kutu yükü
- Izgara doğrulaması: Occupancy kontrolü
- **Çözüm:** Spatial hash ızgarası (O(1) yerine O(n))

**Uygulama Sırası:** 8. (envanter sonrası)

---

### 7. ÜRETİM MAKİNELERİ

**Sorumluluklar:**
- Üretim tariflerini yap (giriş + çıktı zamanla)
- Giriş öğelerini kabul et (lojistikten tüket)
- Çıktı öğelerini üret (lojistiye yatır)
- Yakıt/enerji tüketimini yönet
- Üretim metriklerini takip et
- Hata/durdurma durumlarını idare et (yakıt yok, çıktı bloke)

**Temel Sınıflar:**
```cpp
USTRUCT(BlueprintType)
struct FProductionRecipe
{
    UPROPERTY(EditAnywhere) FString RecipeID;
    UPROPERTY(EditAnywhere) TArray<FRecipeIngredient> Inputs;
    UPROPERTY(EditAnywhere) TArray<FRecipeOutput> Outputs;
    UPROPERTY(EditAnywhere) float ProductionTime;
    UPROPERTY(EditAnywhere) float EnergyPerSecond;
};

struct FProductionState
{
    enum EState { Idle, Running, Blocked_NoInput, Blocked_NoOutput, Blocked_NoFuel };
    EState State;
    FString ActiveRecipeID;
    float Progress;
};

class AProductionMachine : public ABuildingBase
{
    UPROPERTY() TArray<FProductionRecipe> AvailableRecipes;
    UPROPERTY() class UInventoryComponent* InputBuffer;
    UPROPERTY() class UInventoryComponent* OutputBuffer;
    UPROPERTY() float CurrentEnergy;
    
    FProductionState ProductionState;
    
    void SetActiveRecipe(const FString& RecipeID);
    void StartProduction();
    void Tick_Production(float DeltaTime);
};
```

**Veri Akışı:**
```
Lojistik ağı giriş tamponuna öğe yatırır
    ↓
AProductionMachine.Tick_Production()
    ↓
Tarif malzemeleri mevcut mi?
    ↓
Çıktı alanı mevcut mi?
    ↓
Yakıt mevcut mi?
    ↓
Tamamlanırsa: Giriş tüket, çıktı üret
```

**Ölçeklenebilirlik Riski:**
- 500 makina ticked = 8,3 ms/frame
- **Çözüm:** Frekans bölümlendirmesi (500 / 60fps = 8 makina/frame)

**Uygulama Sırası:** 10. (lojistik ağından sonra)

---

### 8. KAYNAK SİMÜLASYONU

**Sorumluluklar:**
- Global enerji üretim/tüketimini simüle et
- Enerji ağı durumu (frekans, istikrar)
- Termik dinamikler (ısı üretim, dağıtım)
- Yakıt rezervleri ve tüketim oranları
- Kaynak darboğazları algıla
- Kıtlık uyarıları

**Temel Sınıflar:**
```cpp
struct FEnergyBudget
{
    float TotalProduction;
    float TotalConsumption;
    float CurrentBalance;
    float Frequency;  // 50Hz = stabil, <48Hz = brownout
    
    bool IsFrequencyStable() const { return Frequency >= 49.5f && Frequency <= 50.5f; }
};

class UResourceSimulation : public UGameInstanceSubsystem
{
    UPROPERTY() FEnergyBudget EnergyBudget;
    UPROPERTY() TMap<FString, FFuelReserve> FuelReserves;
    
    void RegisterEnergyProducer(AActor* Producer, float OutputPerSecond);
    void RegisterEnergyConsumer(AActor* Consumer, float ConsumptionPerSecond);
    void UpdateEnergyBudget(float DeltaTime);
};
```

**Uygulama Sırası:** 11. (üretim makinelerinden önce)

---

### 9. LOJİSTİK AĞI

**Sorumluluklar:**
- Konteyner, makina ve konveyörleri bağla
- Kaynaktan hedefe öğe rota
- Öğe taşıma (konveyör, drone)
- Teslimat için yol bulma
- Kuyruk yönetimi (FIFO, öncelik)
- Ağ sağlığını izle

**Temel Sınıflar:**
```cpp
enum class EStorageNodeType : uint8
{
    Container, MachineInput, MachineOutput, Sink, Source
};

class AStorageNode : public AActor
{
    UPROPERTY() EStorageNodeType NodeType;
    UPROPERTY() class UInventoryComponent* Storage;
    
    virtual bool CanAcceptItem(const FString& ItemID) const;
    virtual void DepositItem(const FString& ItemID, int32 Count);
};

class AConveyorBelt : public AStorageNode
{
    UPROPERTY() FVector Direction;
    UPROPERTY() float Speed;
    
    TArray<FTransportRequest> ActiveTransports;
    
    void Tick_Transport(float DeltaTime);
};

class ULogisticsNetwork : public UGameInstanceSubsystem
{
    TArray<AStorageNode*> Nodes;
    TArray<FTransportRequest> RequestQueue;
    
    void RequestTransport(const FString& ItemID, int32 Count, AActor* Source, AActor* Dest);
    void Tick_ProcessRequests(float DeltaTime);
};
```

**Veri Akışı:**
```
Üretim makinesi çıktıya hazır
    ↓
Makina OutputReady olayı yayınlar
    ↓
Lojistik ağı FTransportRequest oluşturur
    ↓
RouteCalculator en yakın tüketiciyi bulur
    ↓
FTransportRequest RequestQueue'ye eklenir (öncelik sıralı)
    ↓
Tick_ProcessRequests: Drone/konveyör isteğe atay
    ↓
Tick_UpdateTransports: Öğe pozisyonunu rota boyunca güncelle
    ↓
Öğe hedefe ulaşır, DepositItem() çalışır
```

**Ölçeklenebilirlik Riski:**
- A* yol bulma çok pahalı
- 500 transit öğe ticked = güncellemesi pahalı
- Rota önbelleği patlayabilir (n²)
- **Çözüm:** Ağ değişikliğinde rota önceden hesapla, partition güncelleme (50 öğe/frame)

**Uygulama Sırası:** 9. (üretim makinelerinden önce)

---

### 10. HAVA DURUMU SİSTEMİ

**Sorumluluklar:**
- Hava durumu sistemlerini simüle et (fırtına, açık gökyüzü, rüzgar desenleri)
- Hava olayları oluştur (kar fırtınası, sıcak hava, sis)
- Çevresel durumu diğer sistemlere geçir
- Ses/görsel geri bildirim
- Hava durumu durumunu devam ettir

**Temel Sınıflar:**
```cpp
UENUM(BlueprintType)
enum class EWeatherCondition : uint8
{
    Clear, PartlyCloudy, Overcast, Foggy, Rainy, Stormy, Snowing, Blizzard, Heatwave
};

USTRUCT(BlueprintType)
struct FWeatherState
{
    UPROPERTY() EWeatherCondition Condition;
    UPROPERTY() float Temperature;
    UPROPERTY() float Humidity;
    UPROPERTY() float WindSpeed;
    UPROPERTY() FVector WindDirection;
    UPROPERTY() float VisibilityDistance;
    UPROPERTY() float Precipitation;
};

class AWeatherManager : public AGameStateBase
{
    UPROPERTY() EWeatherCondition CurrentWeather;
    
    FWeatherState CurrentState;
    FWeatherState TargetState;
    float TransitionProgress;
    
    void GenerateWeatherEvent();
    void ApplyWeatherTransition(float DeltaTime);
    void BroadcastWeatherState();
};

class UWeatherSimulation : public UGameInstanceSubsystem
{
    UPROPERTY() UDataTable* WeatherProfileTable;
    
    FWeatherState GenerateWeatherState(EWeatherCondition Condition, float Time);
    EWeatherCondition RollNextWeather(EWeatherCondition Current);
};
```

**Veri Akışı:**
```
WeatherManager.Tick()
    ↓
Zaman tabanlı hava durumu değerlendirmesi
    ↓
RollNextWeather() olasılıkla
    ↓
Mevcut ve hedef arasında geçiş oluştur
    ↓
Hava durumunu TransitionDuration'da interpolasyon yap
    ↓
BroadcastWeatherState()
    ↓
Tüm abone sistemler güncelle
```

**Uygulama Sırası:** 12. (zaman sisteminden sonra)

---

### 11. SICAKLIK SİMÜLASYONU

**Sorumluluklar:**
- Hava durumu ve günün saatine göre ortam sıcaklığı hesapla
- Yapı içi sıcaklık modellemesi (yalıtım, ısıtma)
- Oyuncu vücut sıcaklığı (hipotermia/heatstroke)
- Çevresel termal bölgeler
- Isı/soğuk hasarı mekanikleri

**Temel Sınıflar:**
```cpp
struct FThermalZone
{
    FVector Center;
    float Radius;
    float BaseTemperature;
    float HeatSourcePower;
    float InsulationFactor;
};

class UTemperatureSimulation : public UGameInstanceSubsystem
{
    TMap<AActor*, FThermalZone> ThermalZones;
    
    float GetAmbientTemperature(const FVector& Position);
    float GetInteriorTemperature(class ABuildingBase* Building, float DeltaTime);
    void ApplyPlayerThermalStress(APlayerCharacter* Player, float DeltaTime);
};
```

**Uygulama Sırası:** 13.

---

### 12. RÜZGAR SİMÜLASYONU

**Sorumluluklar:**
- Herhangi bir noktada rüzgar hızı ve yönünü hesapla
- Yapılara rüzgar basıncı uygula (itme, dengesizleştirme)
- Rüzgar destekli araç kullanımı (rüzgar türbinleri)
- Ses Doppler efektleri
- Parçacık yönü

**Temel Sınıflar:**
```cpp
struct FWindField
{
    FVector GlobalDirection;
    float GlobalSpeed;
    TArray<FVector> LocalGustsPerGrid;
};

class UWindSimulation : public UGameInstanceSubsystem
{
    FWindField CurrentWind;
    
    FVector GetWindVector(const FVector& Position);
    void ApplyWindForceToStructure(ABuildingBase* Building);
};

class AWindTurbine : public AProductionMachine
{
    UPROPERTY() float CutInSpeed;
    UPROPERTY() float RatedSpeed;
};
```

**Uygulama Sırası:** 14.

---

### 13. KAR BİRİKİMİ

**Sorumluluklar:**
- Kar derinliğini modellemesi artış/azalış
- Kar hareketi hızını etkilemesi (derin karda yavaş)
- Kar aşırı olduğunda inşaatı engelle
- Sıcaklığa göre kar erimesi
- Çığ riski yamaçlarda
- Görsel kar kaplaması

**Temel Sınıflar:**
```cpp
struct FSnowAccumulationGrid
{
    FVector2D GridMin, GridMax;
    float CellSize;
    TMap<FIntPoint, float> SnowDepth;
};

class USnowAccumulation : public UGameInstanceSubsystem
{
    UPROPERTY() FSnowAccumulationGrid SnowGrid;
    
    void UpdateSnowAccumulation(float DeltaTime);
    void CheckAvalancheTriggers();
};
```

**Uygulama Sırası:** 15.

---

### 14. GÜN/GECE DÖNGÜsÜ

**Sorumluluklar:**
- Oyun zamanını yönet (1 gerçek dakika = 10 oyun dakikası)
- Gündoğumu/gümüş saatlerini takip et
- Direktif ışık rotasyonunu güncelle
- Zaman tabanlı olayları tetikle
- Zaman durumunu devam ettir

**Temel Sınıflar:**
```cpp
class UTimeKeeper : public UGameInstanceSubsystem
{
    UPROPERTY() float TimeScale;  // 10.0 = 10x hız
    UPROPERTY() int32 TotalMinutesElapsed;
    
    int32 GetCurrentMinute() const { return TotalMinutesElapsed % 1440; }
    int32 GetCurrentHour() const { return GetCurrentMinute() / 60; }
};

class UDayNightCycle : public UGameInstanceSubsystem
{
    UPROPERTY() ADirectionalLight* SunLight;
    UPROPERTY() float SunriseTime;  // 360 dakika
    UPROPERTY() float SunsetTime;   // 1080 dakika
    
    void UpdateLighting(int32 CurrentMinute);
    float GetLightIntensity(int32 CurrentMinute);
};
```

**Uygulama Sırası:** 5. (temel)

---

### 15. NPC YAPAY ZEKA

**Sorumluluklar:**
- Karakter davranışı (yürüme, çalışma, boş duruş, uyku)
- Görev sistemi (atanmış iş, öncelik)
- Diyalog sistemi (selamlar, iş güncellemeleri)
- Yol bulma (noktalar arasında navigasyon)
- Durum yönetimi (çalışıyor, boş, uyuyor, yaralı)
- Duygu/moral takibi

**Temel Sınıflar:**
```cpp
UENUM(BlueprintType)
enum class ENPCState : uint8
{
    Idle, Walking, Working, Sleeping, Hurt, Eating
};

class ANPCCharacter : public ACharacter
{
    UPROPERTY() class UNPCBrain* Brain;
    UPROPERTY() ENPCState CurrentState;
    UPROPERTY() float Morale;
    UPROPERTY() float Fatigue;
    UPROPERTY() float Health;
};

class UNPCBrain : public UActorComponent
{
    UPROPERTY() class UNPCTask* CurrentTask;
    UPROPERTY() TArray<class UNPCTask*> AssignedTasks;
    
    void EvaluateState(float DeltaTime);
    void SelectNextTask();
    void ExecuteCurrentTask(float DeltaTime);
};

class UNPCTask : public UObject
{
    UPROPERTY() FString TaskID;
    UPROPERTY() float Priority;
    UPROPERTY() FVector TargetLocation;
    
    virtual void Execute(ANPCCharacter* NPC, float DeltaTime) = 0;
    virtual bool IsComplete(ANPCCharacter* NPC) const = 0;
};
```

**Uygulama Sırası:** 16.

---

### 16. GÖREV PLANLAYICISI

**Sorumluluklar:**
- NPC'ler için iş görevlerini sıraya al
- Görevleri NPC becerilerine göre ata
- Görev tamamlamasını takip et
- Görevleri önceliklendirme
- Başarısız görevleri yeniden dene
- Görev durumunu devam ettir

**Temel Sınıflar:**
```cpp
USTRUCT(BlueprintType)
struct FTaskDefinition
{
    UPROPERTY(EditAnywhere) FString TaskID;
    UPROPERTY(EditAnywhere) FText TaskName;
    UPROPERTY(EditAnywhere) class AProductionMachine* TargetMachine;
    UPROPERTY(EditAnywhere) float Priority;
    UPROPERTY(EditAnywhere) int32 RequiredSkillLevel;
};

class UTaskScheduler : public UGameInstanceSubsystem
{
    UPROPERTY() TArray<FTaskDefinition> PendingTasks;
    UPROPERTY() TMap<ANPCCharacter*, class UNPCTask*> ActiveAssignments;
    
    void EnqueueTask(const FTaskDefinition& TaskDef);
    void AssignTaskToNPC(ANPCCharacter* NPC);
};
```

**Uygulama Sırası:** 17.

---

### 17. KAYDET/YÜKLEMESİ

**Sorumluluklar:**
- Tam oyun durumunu serileştir/deserialize et
- Birden fazla kayıt yuvası yönet
- Aralıklı otomatik kaydetme
- Güncelleme için sürüm geçişi
- Kayıt bozulmasını idare et
- Kayıtları sıkıştır
- Bulut senkronizasyonu (temel)

**Temel Sınıflar:**
```cpp
USTRUCT()
struct FGameSaveData
{
    UPROPERTY() FSaveDataVersion Version;
    UPROPERTY() int32 TotalPlayTimeSeconds;
    UPROPERTY() int32 InGameTimeMinutes;
    UPROPERTY() FVector PlayerPosition;
};

class USaveGameManager : public UGameInstanceSubsystem
{
    UPROPERTY() UDataTable* SaveMetadataTable;
    TMap<FString, FSaveSlot> SaveSlots;
    
    void CreateSave(const FString& SlotName);
    void LoadSave(const FString& SlotName);
    void DeleteSave(const FString& SlotName);
};
```

**Ölçeklenebilirlik Riski:**
- Serialization blocking: Büyük kayıt dosyası = frame drop
- Kayıt dosyası boyutu: 500 bina + makina = 100+ MB
- **Çözüm:** Async serialization, sıkıştırma, delta değişiklikleri

**Uygulama Sırası:** 18.

---

### 18. DÜNYA BÖLÜMLENDİRME

**Sorumluluklar:**
- Dünyayı akış için hücrelere böl
- Uzaklığa göre hücre yükle/boşalt
- Hücre durumunu koru (binalar, NPC'ler)
- Bellek kullanımını optimize et
- Bölümlenmiş verilerin kaydet/yüklemesini destekle

**Temel Sınıflar:**
```cpp
struct FWorldPartitionCell
{
    FIntPoint CellCoord;
    FBox CellBounds;
    UPROPERTY() TArray<AActor*> ContainedActors;
    bool bIsLoaded;
};

class UWorldPartitionHelper : public UGameInstanceSubsystem
{
    UPROPERTY() float CellSize;  // 6400 UU varsayılan
    UPROPERTY() float LoadRadius;
    UPROPERTY() float UnloadRadius;
    
    TMap<FIntPoint, FWorldPartitionCell> Cells;
    
    FIntPoint GetCellForPosition(const FVector& Position);
    void LoadCell(const FIntPoint& CellCoord);
    void UnloadCell(const FIntPoint& CellCoord);
};
```

**Uygulama Sırası:** 19. (son)

---

### 19. SES YÖNETİCİSİ

**Sorumluluklar:**
- Ses efektlerini oynat (adımlar, araç kullanımı, etkileşimler)
- Müzik yönetimi (menü, keşif, tehlike)
- Ortam döngüleri (rüzgar, yağmur, makine)
- Konumsal ses (3D konumlandırma)
- Ses karışımı (kategori başına ses ayarı)
- Ses budaması (uzak sesleri durdur)

**Temel Sınıflar:**
```cpp
UENUM(BlueprintType)
enum class ESoundCategory : uint8
{
    Master, Music, Ambient, SFX, UI, Voice
};

USTRUCT(BlueprintType)
struct FSoundInstance
{
    UPROPERTY() USoundWave* Sound;
    UPROPERTY() FVector Location;
    UPROPERTY() float Volume;
    UPROPERTY() ESoundCategory Category;
    UPROPERTY() bool bIs3D;
};

class UAudioManager : public UGameInstanceSubsystem
{
    UPROPERTY() TMap<ESoundCategory, float> CategoryVolumes;
    
    void PlaySoundEffect(const FSoundInstance& Sound);
    void SetCategoryVolume(ESoundCategory Category, float Volume);
};

class UMusicManager : public UGameInstanceSubsystem
{
    UPROPERTY() class USoundCue* CurrentMusic;
    
    void PlayMusic(class USoundCue* NewMusic);
    void CrossfadeMusic(class USoundCue* NewMusic);
};
```

**Uygulama Sırası:** 20.

---

### 20. KULLANıCı ARAYÜZÜ MİMARİSİ

**Sorumluluklar:**
- HUD yer paylaşımı (envanter, zanaat, inşaat düğmeleri)
- Menü sistemi (durdur, seçenekler, kaydet/yükle)
- Widget'lar (envanter ızgarası, makine durumu, hava tahmin)
- Ekran yönetimi (geçişler, modal diyaloglar)
- Veri bağlama (otomatik güncellemeler)
- Giriş yönlendirmesi (UI vs oyun)

**Temel Sınıflar:**
```cpp
class UHUDController : public UActorComponent
{
    UPROPERTY() TArray<class UUserWidget*> ActiveWidgets;
    UPROPERTY() class UScreenManager* ScreenManager;
    
    void ShowWidget(TSubclassOf<class UUserWidget> WidgetClass);
    void HideWidget(TSubclassOf<class UUserWidget> WidgetClass);
};

class UScreenManager : public UGameInstanceSubsystem
{
    UPROPERTY() class UScreenBase* CurrentScreen;
    TArray<class UScreenBase*> ScreenStack;
    
    void PushScreen(TSubclassOf<class UScreenBase> ScreenClass);
    void PopScreen();
};

class UScreenBase : public UUserWidget
{
    virtual void OnScreenOpened() {}
    virtual void OnScreenClosed() {}
};

class UInventoryWidget : public UUserWidget
{
    UPROPERTY() class UInventoryGridPanel* GridPanel;
    UPROPERTY() class UInventoryComponent* BoundInventory;
};
```

**Uygulama Sırası:** 21. (son)

---

## UYGULANMA SIRASI (TAMADIR HARITA)

| Sıra | Sistem | Bağımlılık |
|------|--------|-----------|
| 1 | **Oyun Saati / Zaman** | Temel |
| 2 | **Oyuncu Kontrol** | #1 |
| 3 | **Etkileşim** | #2 |
| 4 | **Envanter** | #3 |
| 5 | **Öğem Veritabanı** | #4 |
| 6 | **Zanaat** | #4, #5 |
| 7 | **İnşaat** | #4 |
| 8 | **Lojistik Ağı** | #7 |
| 9 | **Üretim Makineleri** | #8 |
| 10 | **Kaynak Simülasyonu** | #9 |
| 11 | **Hava Durumu** | #1 |
| 12 | **Sıcaklık** | #11 |
| 13 | **Rüzgar** | #11 |
| 14 | **Kar Birikimi** | #11, #12 |
| 15 | **NPC Yapay Zeka** | #3 |
| 16 | **Görev Planlayıcısı** | #15 |
| 17 | **Kaydet/Yükleme** | Hepsi |
| 18 | **Dünya Bölümlendirme** | Hepsi |
| 19 | **Ses Yöneticisi** | Hepsi |
| 20 | **Kullanıcı Arayüzü** | Hepsi |

---

## ÖLÇEKLENEBILIRLIK ÖZETİ

| Sistem | Risk | Etki | Çözüm |
|--------|------|------|-------|
| **Üretim** | 500 makina ticked | 8,3 ms/frame | Frekans bölümlendirmesi |
| **Lojistik** | Rota hesabı | Yol bulma duraklı | Rotaları önceden hesapla |
| **Envanter** | Öğe araması | O(n) arama | TMap dizini |
| **NPC** | 100 NPC yol bulma | 100+ iz/frame | Yol önbellek, hedef değişikliğinde güncelle |
| **Hava Durumu** | Simplex noise | Pahalı hesap | Spatial grid cache |
| **Kar** | 1M hücre | Bellek bloat | Dirty hücreleri güncelleyin |
| **Dünya Bölümü** | 100+ hücre yüklü | Bellek spike | Dikkatli unload zamanlaması |
| **UI** | 100 widget yenileme | Frame düşüşü | Batch güncelleme, sanal scroll |
| **Ses** | 1000+ ses sırası | Ses esintileri | Budama + max limit (128) |
| **Kaydet** | Büyük serialization | Frame düşüşü | Async serialization, sıkıştırma |

---

## PERFORMANS HEDEFLERİ

- **Üretim Tick:** 500 makina için <2ms
- **Lojistik Güncelle:** 1000 transit öğe için <1ms
- **NPC AI:** 100 NPC için <1ms
- **Hava/Sıcaklık:** <0,5ms
- **Toplam frame bütçesi:** <16ms (60 FPS)
- **Bellek:** <8GB yüklü içerik
- **Kayıt dosyası:** <100MB sıkıştırılmış

---

## MERKEZ MİMARİ İLKELERİ

1. **Veri Tabanlı Her Şey:** Tüm içerik DataTable'larda (DT_Items, DT_Recipes, DT_Buildings). Hiçbir zaman sabit kod değerleri.

2. **Saf İşlevler:** İş mantığı saf işlevlerde (yan etki yok). Test ve refactor güvenliği.

3. **Subsystemler Over Managers:** Tekilliler için `UGameInstanceSubsystem` kullan. Otomatik başlatma/temizlik.

4. **Olay Broadcast'i:** `FSimpleDelegate` ve `FSimpleMulticastDelegate` kullan. Diğer sistemler bağlanır, coupling yok.

5. **Async İşlemler:** Kaydet/yükle ve ağır yol bulma async olmalı. `FAsyncTask` kullan.

6. **Ağ Temelinde Tasarım:** Tüm sistemleri çoklu oyunculu aklında tasarla (tek oyunculu da ilk). Durum serileştirmesi ağ çoğaltması için.

7. **Test Kültürü:** İlerleme ile entegrasyon testleri yaz. Gerillemeleri erkenden yakala.

8. **Dokümantasyon:** Her ana subsistem'de README.md tut. Sorumluluklar ve kullanım açıkla.

Bu mimari ticari sınıf karmaşıklığa ölçeklenir. Sistemler 1–10'u başlat, dikey dilim gemi. Ardından katmanlar 11–20 derinlik ve sistem simülasyonu ekle.

---

## 21. MULTIPLAYER MIMARISI (Temel)

**Sorumluluklar:**
- Oyuncu şehrini başkalarında göster
- Durum senkronizasyonu (bina yerleri, makina durumları)
- Yapı yerleştirme çatışma çözümü
- NPC/görev yönetimi (sunucu-authoritative)
- Ağ kötü durumda stabilite

**Temel Sınıflar:**
```cpp
class UNetworkReplicator : public UActorComponent
{
    UPROPERTY(Replicated) TArray<FBuildingState> ReplicatedBuildings;
    UPROPERTY(Replicated) TArray<FMachineState> ReplicatedMachines;
    
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    
    void OnBuildingPlaced(ABuildingBase* Building);
    void OnMachineStateChanged(AProductionMachine* Machine);
};

class AGameServerManager : public AGameMode
{
    // Sunucu-otoritesi: Tüm kaynakları, görevleri yönet
    // İstemci: Sadece görüntüle (rendering + local input)
    
    void ValidateBuildingPlacement(APlayerController* Player, const FVector& Location);
    void ExecuteResourceTransaction(AActor* Source, AActor* Dest);
};

class FNetworkLatencyCompensator
{
    // Tahmin edici çözüm: Yerel prediksiyon + sunucu düzeltmesi
    FVector PredictFuturePosition(const AActor* Actor, float Latency);
    void ReconcileWithServerState();
};
```

**Veri Akışı (Ağ):**
```
İstemci: E basıldı → Binayı yerleştir
    ↓
RPC: BuildingPlacementRequest()
    ↓
Sunucu: Doğrula (kaynak, çarpışma, iznin)
    ↓
Sunucu: Spawn ABuildingBase
    ↓
Replicate: Tüm istemçilere bina pozisyonu
    ↓
Tüm istemçiler: Bina görülür
```

**Ölçeklenebilirlik:**
- **Bant genişliği:** 500 bina + 200 makina → 30KB/saniye heartbeat
- **Replication rate:** 10 Hz (100ms güncelle) yeterli
- **Late joiners:** Client-side bina cache, sunucu snapshot gönder

**Uygulama Sırası:** 25. (daha sonra)

---

## 22. GELIŞTIRME ARAÇLARI & EDİTÖR EXTENSİYONLARI

**Sorumluluklar:**
- DataTable üretici (öğem/tarif/bina bulktan içe aktar)
- Hızlı yapı yerleştirme modları
- Performans profiler vizuelleştirmesi
- Hava durumu/zaman kontrol (dev command'ları)
- Lojistik ağı görselleştirici
- Üretim simülasyonu debugger'ı

**Temel Sınıflar:**
```cpp
class UDataTableGenerator : public FTickableGameObject
{
    // CSV → DataTable çevirici
    // Exel → DT_Items, DT_Recipes, etc.
    
    bool ImportItemsFromCSV(const FString& FilePath, UDataTable*& OutTable);
    bool ValidateRecipeIntegrity(UDataTable* RecipeTable);
};

class FBuildingPlacementTool : public FEditorTool
{
    // Blueprint editor içinde hızlı yerleştirme
    void PlaceGridOfBuildings(int32 GridWidth, int32 GridHeight);
    void VisualizePlacementCollision();
};

class FPerformanceProfilerVisualizer
{
    void DrawProfilingStats(FCanvas& Canvas);
    void HighlightBottlenecks();
    void ExportProfilingReport(const FString& FilePath);
};

class ADevConsoleManager : public AActor
{
    UFUNCTION(Exec) void Weather_SetCondition(const FString& Condition);
    UFUNCTION(Exec) void Time_SetHour(int32 Hour);
    UFUNCTION(Exec) void Time_SetTimeScale(float Scale);
    UFUNCTION(Exec) void Production_StartAllMachines();
    UFUNCTION(Exec) void Logistics_VisualizeNetwork();
    UFUNCTION(Exec) void Profiler_DumpStats();
};

class FLogisticsNetworkVisualizer : public FDebugDrawDelegateHelper
{
    virtual void Draw(UCanvas* Canvas) override;
    
    void DrawNodes(FCanvas& Canvas);
    void DrawRoutes(FCanvas& Canvas);
    void DrawBottlenecks(FCanvas& Canvas);
};
```

**Komut Örnekleri:**
```
weather_setcondition Blizzard
time_sethour 20
time_settimescale 100
production_startallmachines
logistics_visualizenetwork
profiler_dumpstats output.csv
```

**Uygulama Sırası:** 22. (erken)

---

## 23. HATA İŞLEME & LOGLAMAsı

**Sorumluluklar:**
- Ağ hataları (sunucu bağlantı kaybı)
- Serialization hataları (kayıt dosyası bozulması)
- Türü uyumsuz hataları (yapı tanımı eksik)
- Mantık hataları (malzeme kısıtlaması ihlali)
- Stack trace + context dumping

**Temel Sınıflar:**
```cpp
class FGameLogger
{
    enum class ELogLevel { Debug, Info, Warning, Error, Critical };
    enum class ELogCategory { Gameplay, Network, Save, Physics, UI };
    
    static void Log(ELogLevel Level, ELogCategory Category, 
                   const FString& Message, const FString& Context);
    
    static void LogGameplayError(const FString& Message, AActor* Context);
    static void LogNetworkError(const FString& Message, const FString& RemoteIP);
    static void LogSaveError(const FString& SaveSlot, const FString& Error);
};

class FErrorHandler
{
    enum class ERecoveryStrategy { Retry, Fallback, Abort, UserPrompt };
    
    static ERecoveryStrategy HandleSaveCorruption(const FString& SaveSlot);
    static ERecoveryStrategy HandleNetworkDisconnect();
    static ERecoveryStrategy HandleMissingAsset(const FString& AssetPath);
};

class FStructuredLogger : public FOutputDevice
{
    // JSON yapısı ile özel log yazma
    virtual void Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, 
                          const class FName& Category) override;
    
    void StartSessionLog(const FString& SessionID);
    void EndSessionLog();
    void FlushToFile();
};

class FCrashReporter
{
    struct FCrashInfo
    {
        FString StackTrace;
        FString GameState;
        FString LastPlayerAction;
        TArray<FString> RecentLogs;
        float FrameRate;
        float MemoryUsage;
    };
    
    static void GenerateCrashReport(const FCrashInfo& Info);
    static void SendToDeveloper(const FString& CrashReportPath);
};
```

**Log Format (JSON):**
```json
{
  "timestamp": "2026-07-21T14:32:45Z",
  "level": "ERROR",
  "category": "Production",
  "message": "Machine output buffer full - blocking production",
  "context": {
    "machine_id": "Furnace_001",
    "session_id": "sess_abc123",
    "player_pos": [100, 200, 50]
  },
  "stack_trace": "AProductionMachine::Tick_Production() : Line 245"
}
```

**Uygulama Sırası:** 23. (tüm sistemlerin sonunda)

---

## 24. CONTENT PIPELINE

**Sorumluluklar:**
- Asset import workflow (FBX → Mesh, PSD → Texture)
- Naming convention validation
- Doku atlası oluşturma
- Animation montage assembly
- Sound bank organization
- Version control best practices

**Temel Proses:**

```
1. ARTIST EXPORTS
   ├── 3D Models (FBX, 4K normal maps)
   ├── Textures (PSD, 4096x4096)
   └── Animations (FBX, 30 fps, named correctly)

2. IMPORT SETTINGS (Otomatik)
   ├── Mesh: LOD0/1/2 oluştur, collision
   ├── Texture: sRGB=false, filter=Trilinear
   ├── Animation: frame rate = 30, loop check

3. ORGANIZATION
   ├── Content/Meshes/Buildings/Furnace_v2.uasset
   ├── Content/Textures/Buildings/Furnace_Diffuse.uasset
   ├── Content/Animations/Characters/Worker_Idle.uasset

4. VALIDATION
   ├── Naming rules (PascalCase_v2 format)
   ├── Texture size (max 4096)
   ├── Triangle count (max 50k per mesh)
   ├── Animation length (< 5 sec clips)

5. VERSION CONTROL
   ├── .gitignore: Derived data, editor caches
   ├── Tag releases: v0.1.0, v0.2.0
```

**Naming Conventions:**
```
Mesh:
  SM_BuildingName_v2           # Static mesh
  SK_CharacterName_v1          # Skeletal mesh
  SK_CharacterName_Arm_Left    # Skeletal part

Texture:
  T_BuildingName_Diffuse.tga
  T_BuildingName_Normal.tga
  T_BuildingName_Roughness.tga

Animation:
  A_CharacterName_Idle_v2
  A_CharacterName_Walk_Forward
  A_MachineSequence_Startup

Material:
  M_Building_Metal
  M_Building_Concrete
  MI_Furnace_Hot            # Material instance
```

**Uygulama Sırası:** 24. (başlangıç)

---

## 25. PERFORMANS PROFILING STRATEJISI

**Sorumluluklar:**
- CPU/GPU/Memory profiling
- Frame time breakdown (hangisi yavaş?)
-병목 algılaması (ağ, physics, rendering)
- Hedefleri takip (60 FPS, <8GB mem)
- Benchmark raporu üretme

**Temel Araçlar:**

```cpp
class FPerformanceMonitor : public FTickableGameObject
{
    struct FFrameStats
    {
        float TotalFrameTime;
        float GameplayTime;
        float RenderTime;
        float PhysicsTime;
        float AudioTime;
        float UITime;
        
        TMap<FString, float> SystemTimes;  // Per-system breakdown
    };
    
    TArray<FFrameStats> FrameHistory;  // Son 300 frame
    
    void Tick(float DeltaTime) override;
    void PrintFrameStats() const;
    void ExportProfilingReport(const FString& FilePath);
};

class FScopedTimer
{
    // Blok zamanını ölçmek için kullan
    FScopedTimer(const FString& ScopeName, FPerformanceMonitor* Monitor)
        : StartTime(FPlatformTime::Seconds()), Monitor(Monitor), Name(ScopeName) {}
    
    ~FScopedTimer()
    {
        float ElapsedMs = (FPlatformTime::Seconds() - StartTime) * 1000.0f;
        Monitor->RecordSystemTime(Name, ElapsedMs);
    }
};

#define SCOPED_TIMER(ScopeName) FScopedTimer Timer_##__LINE__(ScopeName, GPerformanceMonitor)

// Kullanım:
void AProductionMachine::Tick_Production(float DeltaTime)
{
    SCOPED_TIMER("Production");
    // ...
}
```

**Profiling Checklist (Haftalık):**
```
☐ Average frame time < 16.67 ms (60 FPS)
☐ Peak frame time < 25 ms (hiç >40ms)
☐ Memory steady-state < 6GB
☐ No memory leaks (overnight run)
☐ GPU memory < 4GB (8GB cards)
☐ Physics substeps < 1ms
☐ Rendering < 8ms (60%)
☐ Gameplay logic < 5ms (30%)
☐ Audio < 1ms (5%)
☐ No GC stutters (>1ms)
```

**Uygulama Sırası:** 26.

---

## 26. CI/CD PIPELINE

**Sorumluluklar:**
- Her commit'te otomatik build
- Birim ve entegrasyon testleri
- Statik analiz (code quality)
- Performans regresyon testi
- Otomatik Steam deployment (release)

**GitHub Actions Workflow:**

```yaml
name: CI/CD Pipeline

on: [push, pull_request]

jobs:
  build-and-test:
    runs-on: windows-latest
    steps:
      - uses: actions/checkout@v2
      
      - name: Setup UE5
        run: |
          # Download Engine
          ./Engine/Build/BatchFiles/RunUAT.bat BuildGraph -Script=...
      
      - name: Compile
        run: |
          ./Engine/Build/BatchFiles/Build.bat SurvivalGame Win64 Development
      
      - name: Unit Tests
        run: |
          ./Binaries/Win64/SurvivalGame.exe -ExecCmds="Automation RunTests Filter=SurvivalGame" -nullrhi
      
      - name: Integration Tests
        run: |
          ./Binaries/Win64/SurvivalGame.exe -ExecCmds="Automation RunTests Filter=Integration" -nullrhi
      
      - name: Static Analysis
        run: |
          clang-tidy Source/SurvivalGame/**/*.cpp
      
      - name: Performance Regression Test
        run: |
          ./Tools/PerformanceBenchmark.exe --baseline baseline.json --output current.json
      
      - name: Package (Release only)
        if: github.ref == 'refs/heads/main'
        run: |
          ./Engine/Build/BatchFiles/RunUAT.bat BuildCookRun -project=SurvivalGame.uproject -platform=Win64 -configuration=Shipping

  deploy-to-steam:
    needs: build-and-test
    if: github.ref == 'refs/heads/main' && github.event_name == 'push'
    runs-on: windows-latest
    steps:
      - name: Upload to Steam
        env:
          STEAM_USERNAME: ${{ secrets.STEAM_USERNAME }}
          STEAM_PASSWORD: ${{ secrets.STEAM_PASSWORD }}
        run: |
          ./Tools/SteamDeploy.exe --app-id=XXXXX --content-root=./Packaged
```

**Branch Strategy:**
```
main (production)
  ├── Only merge via PR
  ├── Requires all tests pass
  └── Auto-deploy to Steam staging

develop (integration)
  ├── Merge feature branches
  ├── Daily automated tests
  └── Pre-release builds

feature/* (work-in-progress)
  ├── Personal branches
  └── PR → develop
```

**Uygulama Sırası:** 27.

---

## 27. OYUN DENGESİ REHBERI

**Sorumluluklar:**
- Öğem maliyeti ve rarity balanslama
- Makina üretim hızı ayarı
- NPC morali ve verimlilik
- Hava durumu zorluk kurvanı
- İlerleme pacing

**Balanse Mekanikleri:**

```cpp
class FGameBalanceConfig : public UDataAsset
{
    // ITEM COSTS (ekonomi)
    UPROPERTY(EditAnywhere) TMap<FString, float> BaseItemValue;  // Wood=10, Iron=50
    
    // MACHINE MULTIPLIERS (zorluk)
    UPROPERTY(EditAnywhere) float ProductionSpeedMultiplier;     // 1.0 = normal
    UPROPERTY(EditAnywhere) float EnergyConsumptionMultiplier;   // 1.5 = harder
    UPROPERTY(EditAnywhere) float ResourceRequirementMultiplier; // 1.2 = more needed
    
    // WEATHER SEVERITY
    UPROPERTY(EditAnywhere) float SnowDriftMultiplier;           // 1.0 = normal
    UPROPERTY(EditAnywhere) float WindForcePeak;                 // m/s max
    UPROPERTY(EditAnywhere) float TemperatureExtremesRange;      // °C swings
};

class FBalanceValidator
{
    // Oyuncu feedback-driven deneme/error döngüsü
    
    // Döngü süresi: 1 saat oyun
    static float GetRecipeTimeBalance(const FRecipeDefinition* Recipe);
    
    // Maliyet/fayda oranı: 1.0 = adil, 0.8 = çok kolay, 1.2 = çok zor
    static float CalculateRecipeFairness(const FRecipeDefinition* Recipe);
    
    // Progression curve: Saat 0-10 vs 100+ saat
    static void GenerateProgressionReport(int32 PlaytimeHours);
};
```

**Balanse Metrikler:**

```
1. Sürü çarpan (Herd rate)
   = (Çıktı malzemeleri × İletim süresi) / (Giriş malzemeleri × İsraf)
   Hedef: 0.8–1.2 (verimli ama kısıtlı)

2. Zorluk kurvanı (Difficulty curve)
   = f(saat) = log(oyuncu_gelişmesi) / log(kaynak_kullanılabilirliği)
   Hedef: Saat 0-20 kolay, 20-50 moderate, 50+ zorlayıcı

3. Oyuncu tatmin (Player satisfaction)
   = İlerleme_hızı(saat) × Görev_çeşitliliği × Görsel_geri_bildirim
   Ölçüm: Oyuncu telemetrisi (başında bırakma oranı)
```

**Haftalık Balanse Döngüsü:**
```
Pazartesi: Verileri analiz et (oyuncu oynama saati, tamamlama hızı)
Salı: Dengesiz sistemleri tanımla (çok hızlı, çok zor)
Çarşamba: Değerleri ayarla (DataTable'ta)
Perşembe: QA testi (1 saat oyun, raporla)
Cuma: Geri bildirim topla, pushla
```

**Uygulama Sırası:** 28.

---

## 28. STEAM DEPLOYMENT

**Sorumluluklar:**
- Steamworks SDK entegrasyonu
- Başarımlar ve istatistikler
- Cloud save
- Sohbet ve LAN multiplayer
- Oto-güncellemeler

**Checklist Öncesi Yayın:**

```
☐ Build signed and tested (Shipping config)
☐ All cinematics compressed < 2GB total
☐ Localization complete (Türkçe + 4 dil minimum)
☐ Accessibility pass (color-blind modes, captions)
☐ EULA ve privacy policy Türkçe
☐ System requirements documented
☐ Support email configured
☐ Store page banner assets ready (1280x720 jpg)
☐ Trailer uploaded (60 sec, English + Türkçe subs)
☐ Release notes written
☐ Crash report logging enabled (prod servers)
☐ Analytics integrated (user engagement tracking)
☐ Regional pricing verified (TR = ₺xxx)

Post-launch:
☐ Monitor crash reports daily
☐ Hotfix patches ready (< 24 saat)
☐ Community feedback channel (Discord)
☐ Bi-weekly balance updates planned
```

**Steamworks Entegrasyonu:**

```cpp
class USteamManager : public UGameInstanceSubsystem
{
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    
    void UnlockAchievement(const FString& AchievementID);
    void SetStatValue(const FString& StatName, int32 Value);
    void UploadSaveToCloud(const FString& SaveSlotName);
    void DownloadSaveFromCloud(const FString& SaveSlotName);
};

// Başarım tanımları (DataTable)
USTRUCT(BlueprintType)
struct FAchievementDefinition
{
    UPROPERTY(EditAnywhere) FString AchievementID;  // "BuildFirstFurnace"
    UPROPERTY(EditAnywhere) FText DisplayName;
    UPROPERTY(EditAnywhere) FText Description;
    UPROPERTY(EditAnywhere) UTexture2D* Icon;
    UPROPERTY(EditAnywhere) bool bHidden;
    UPROPERTY(EditAnywhere) float UnlockPercentage;  // İstatistikler için
};
```

**Uygulama Sırası:** 29. (yayın hazırlığında)

---

## 29. HASAT DÜĞÜMLERİ (HARVEST NODES)

**Bağlam:** 2026-07-26'da içerik-hacmi genişletmesi sırasında keşfedilen bir mimari boşluğu kapatır — kodda
(orijinal Odun/Taş dahil) hiçbir ham madde için dünyada toplama/hasat mekanizması yoktu; tüm eşyalar yalnızca
zanaat/üretim çıktısı, inşaat tüketimi, önceden-doldurulmuş sandık ya da lojistik taşıma yoluyla var oluyordu
(bkz. `Docs/YOL_HARITASI.md` ADR 2026-07-26, `Docs/ILERLEME.md` Sistem #27 notu). Bu teknik olarak yeni bir faz
değil — Faz 1 "dikey dilim"in tamamlayıcı, açık kalmış bir borcudur. Tasarım, 4 ajanlı bağımsız bir tasarım
paneli (3 bağımsız mimari öneri + motor-kaynağı/kod-tabanı doğrulamalı yargılama-sentez) ile üretildi; panel
uygulama sırasının **#21/#26/#28'den ÖNCE** olması gerektiğini net bir şekilde vurguladı (gerekçe: Steam'e
çıkacak bir "hayatta kalma" oyununun temel döngüsü şu an oyun-içi hiç tetiklenmiyor).

**HENÜZ KODLANMADI** — bu bölüm yalnızca tasarım kaydı; uygulama ayrı bir pasoda yapılacak.

**Sorumluluklar:**
- Dünyada yerleştirilmiş, E-tuşuyla toplanabilen ham-madde noktaları (ağaç, kaya, vb.)
- Tükenme + kare-bölümlemeli yeniden-doğma (respawn) zamanlayıcısı
- Envanter kabul kontrolüyle kayıpsız verim aktarımı (dolu envanterde düğüm tükenmez)
- Veri-güdümlü tür farklılaştırma (v1: davranışsız kanca, v1.5+: alet gereksinimi)

**Temel Sınıflar:**
```cpp
UENUM(BlueprintType)
enum class EHarvestNodeType : uint8
{
    Generic, Tree, Rock,
    // v1'de davranışı DEĞİŞTİRMEZ — yalnızca prompt/ikon seçimi + gelecekteki dallanma kancası
    // (BuildingDefinition::ConstructionTime'daki "veri duruyor, tüketilmiyor" dürüst-kapsam emsaliyle aynı disiplin)
};

USTRUCT(BlueprintType)
struct FHarvestNodeDefinition : public FTableRowBase
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere) EHarvestNodeType NodeType = EHarvestNodeType::Generic;
    UPROPERTY(EditAnywhere) FName YieldItemID;        // DT_Items satır adı, UItemDatabase ile çözülür, KOPYALANMAZ
    UPROPERTY(EditAnywhere) int32 YieldCountMin = 1;
    UPROPERTY(EditAnywhere) int32 YieldCountMax = 1;
    UPROPERTY(EditAnywhere) int32 HarvestsBeforeDepletion = 1;
    UPROPERTY(EditAnywhere) float RespawnSeconds = 60.f;
    UPROPERTY(EditAnywhere) FText InteractionPrompt;   // "Kes", "Kaz" vb.
    UPROPERTY(EditAnywhere) FString RequiredToolTag;   // v1'de YAZILIR ama OKUNMAZ (v1.5'e kadar bilinçli erteleme)
};

UCLASS(Config=Game, DefaultConfig)
class UHarvestNodeDatabaseSettings : public UDeveloperSettings
{
    // UItemDatabaseSettings ile birebir aynı desen — Project Settings > Game > Harvest Node Database
    UPROPERTY(EditAnywhere, Config) TSoftObjectPtr<UDataTable> HarvestNodeTable;
};

UCLASS()
class UHarvestNodeDatabase : public UGameInstanceSubsystem
{
    // DT_HarvestNodes'u yükler, FName->FHarvestNodeDefinition KOPYA önbelleği kurar (ham işaretçi değil,
    // UItemDatabase ile aynı desen)
    bool FindNodeDefinition(FName NodeID, FHarvestNodeDefinition& OutDefinition) const;
};

UCLASS()
class AHarvestNode : public AInteractableBase
{
    // AInteractableBase'in İLK somut alt sınıfı — sınıfın kendi yorumundaki "toplama noktası" örneği
    UPROPERTY(EditAnywhere) FName NodeID;   // DT_HarvestNodes satır anahtarı

    virtual bool CanInteract_Implementation(AActor* Interactor) const override;  // Super && !bDepleted
    virtual void OnInteract_Implementation(AActor* Interactor) override;
    // OnInteract: SurvivalHarvest::RollYieldCount ile miktar belirlenir, AddItem'in GERÇEK kabul miktarı
    // kontrol edilir (tam kabul olmazsa şarj düşürülmez, malzeme sessizce kaybolmaz — ResourceSimulation'ın
    // düzeltilen "sessiz tavan" hatasıyla aynı disiplin). Tükenmede: SetActorHiddenInGame(true) +
    // SetActorEnableCollision(false) (yalnızca Mesh gizleme DEĞİL — WorldPartitionHelper'ın "gizle,
    // yok etme" emsaliyle tutarlı) + UHarvestNodeManager::RegisterDepletedNode.
};

UCLASS()
class UHarvestNodeManager : public UWorldSubsystem, public FTickableGameObject
{
    // ProductionManager/NPCManager ile AYNI iskelet (UGameInstanceSubsystem DEĞİL — dünya yaşam döngüsüne
    // bağlı olmalı, bu üç dosyadan doğrulandı). Yalnızca TÜKENMİŞ düğümleri SurvivalProduction::ComputeBatchSize
    // ile round-robin kontrol eder — dolu düğümler bu kümeye hiç girmez. UWorldPartitionHelper::IsPositionLoaded
    // ile boşaltılmış hücreleri atlar (mutlak GameTime damgası sayesinde kayıp ilerleme olmaz).
    void RegisterDepletedNode(AHarvestNode* Node);
    virtual bool IsTickable() const override;   // yalnızca bekleyen tükenmiş düğüm varsa true
};

namespace SurvivalHarvest   // saf, test edilebilir fonksiyonlar (HarvestMath.h + HarvestMathTests.cpp)
{
    int32 RollYieldCount(int32 Min, int32 Max);
    bool IsRespawnReady(double DepletionGameTime, double CurrentGameTime, float RespawnSeconds);
}
```

**Veri:** `DT_HarvestNodes` (yeni DataTable) — v1 tohum satırları MEVCUT ekonomiye bağlanır, yeni item icat
edilmez: `Agac`→Odun, `Kaya`→Tas (tam olarak "orijinal Odun/Taş bile toplanamıyor" boşluğunu kapatır).

**İsimlendirme kararı:** Kök **"Harvest"** — "Resource" kelimesi KULLANILMAZ (`Production/ResourceSimulation`
zaten enerji/termal/yakıt anlamında bu kelimeyi sahipleniyor, çakışma/kavram karışıklığı riski gerçek). Klasör:
`Source/SurvivalGame/{Public,Private}/Harvesting/`. **Kodlamaya başlamadan önce**, "Harvest" kökünün UE 5.8
motor kaynağında native bir sınıfla çakışmadığı ayrıca doğrulanmalı (WorldPartitionSettings emsali gereği —
unreal-engine-dev disiplini: bu tasarım yalnızca proje-içi çakışmanın yokluğunu doğruladı, motor-geneli
çakışmayı VARSAYMIYOR).

**Entegrasyon noktaları:**
- **Interaction (#3):** `AHarvestNode : public AInteractableBase` — E-tuşu/trace/`ECC_GameTraceChannel1`
  pipeline'ı hiç değişmeden kullanılır.
- **Items (#5):** `YieldItemID`, `UItemDatabase::FindItem` ile çözülür; `EItemCategory`'e yeni değer EKLENMEZ
  (sabit 4 değer korunur).
- **Inventory (#4):** `AddItem`'ın dönüş değeri (gerçek kabul miktarı) kontrol edilir.
- **Production (#9):** `SurvivalProduction::ComputeBatchSize` doğrudan yeniden kullanılır (NPCManager'ın
  zaten yaptığı gibi) — yeni bir dilim-boyutu formülü icat edilmez.
- **World Partition (#18):** `UWorldPartitionHelper::IsPositionLoaded` ile boşaltılmış hücrelerdeki tükenmiş
  düğümler atlanır/telafi edilir.
- **Save (#17):** v1'de BİLİNÇLİ entegre EDİLMEZ — Construction/Production/Logistics/Kaynak/Hava/NPC/Görev
  durumu da henüz kaydedilmiyor (`SaveDataTypes.h`'nin mevcut kararıyla tutarlı); gelecekte Sistem #17'nin
  TEK SEFERLİK genel genişlemesiyle birlikte ele alınmalı, Harvesting'e özel bir istisna açılmamalı.
- **DevTools (#22):** `harvest_dump`/`harvest_force_regenerate` konsol komutları, `craft_start` deseniyle
  aynı yerde (`HarvestNode.cpp` altında) yaşar.
- **Content Pipeline (#24):** `DT_HarvestNodes.YieldItemID`'nin `DT_Items`'ta var olduğu `content_validate`'e
  eklenir.

**Aşamalama:**
- **v1 (tasarlandı, henüz KODLANMADI):** `Agac`/`Kaya` — tüm düğümler aynı davranışta, alet gereksinimi yok,
  Equipment bileşeni yok, spawn-volume yok, save entegrasyonu yok.
- **v1.5 (yalnızca gerçek playtest ihtiyacı doğarsa):** `RequiredToolTag` aktive edilir — YENİ bir
  Equipment/hotbar bileşeni icat etmeden, mevcut `UInventoryComponent` + `UItemDatabase::FindItemsByTag` ile
  "envanterde bu etiketli öğe var mı" taraması yeterli.
- **v2 (opsiyonel, yalnızca gerçek açık-dünya/prosedürel yerleşim ihtiyacı doğarsa):** `AHarvestNodeSpawnVolume`
  — bugün için seviye tasarımcısının birkaç `BP_HarvestNode` yerleştirmesi yeterli (proje hâlâ tek düz test
  haritası kullanıyor).

**Uygulama Sırası:** ÖNCELİKLİ — #21/#26/#28'den ÖNCE ele alınmalı (Faz 1 dikey-dilim borcu kapanışı, bkz.
`Docs/YOL_HARITASI.md` ADR 2026-07-26 + takip notu).

---

## ÖZET: EKSIK BÖLÜMLER UYGULAMA SIRASI

| Sıra | Sistem | Zaman |
|------|--------|-------|
| 29 | **Hasat Düğümleri (Harvest Nodes)** | **ÖNCELİKLİ — 21/26/28'den ÖNCE** (Faz 1 dikey-dilim borcu) |
| 21 | Multiplayer (Temel) | Release sonrası |
| 22 | Dev Tools | Hafta 2 |
| 23 | Hata İşleme & Logging | Hafta 3-4 |
| 24 | Content Pipeline | Güncelse |
| 25 | Performans Profiling | Hafta 4 |
| 26 | CI/CD | Hafta 1-2 |
| 27 | Game Balance | İterasyon |
| 28 | Steam Deployment | Release-1 |

---

## TOPLAM UYGULAMA TAKVIMI

**Hafta 1-2:** Sistemler 1-5 (Temel oynanabilir)
**Hafta 3-4:** Sistemler 6-10 (Dikey dilim tamamı)
**Hafta 5-6:** Sistemler 11-15 (Sistem simülasyonları)
**Hafta 7-8:** Sistemler 16-20 (NPC, Ses, UI, Kaydet/Yükle)
**Hafta 9-10:** Sistemler 21-28 (Multiplayer, Tools, Deploy) — **Sistem #29 (Hasat Düğümleri) bu pencerede
diğerlerinden ÖNCE ele alınmalı** (2026-07-26'da keşfedilen Faz 1 dikey-dilim borcu, bkz. Sistem #29 bölümü)

**QA & Balanse:** Paralel, hafta 5+
**Polishing:** Hafta 11-12

Bu **12 haftalık Kanban + scrum hybrid** yapısı Steam alpha ready → beta → launch'e 16 haftada götürür.

---

## ENTEGRASYON TEST ÖRNEKLERİ

```cpp
TEST_F(ProductionToLogisticsIntegration, MachineOutputIsTransported)
{
    // 1. Makinede üretim başlat
    Machine->SetActiveRecipe("Recipe_Iron");
    Machine->StartProduction();
    
    // 2. Üretim tamamlanana kadar bekle
    AdvanceGameTime(10.0f);
    
    // 3. Çıktı tamponu öğelere sahip mi kontrol et
    EXPECT_TRUE(Machine->CanExtractOutput());
    
    // 4. Lojistik aracılığıyla taşıma iste
    LogisticsNetwork->RequestTransport("Iron", 10, Machine, StorageNode);
    
    // 5. Taşıma için zaman ilerlet
    AdvanceGameTime(5.0f);
    
    // 6. Öğelerin geldiğini doğrula
    EXPECT_EQ(StorageNode->ExtractItem("Iron", 10), 10);
}

TEST_F(WeatherToProductionIntegration, HighTemperatureReducesFurnaceOutput)
{
    // 1. Hava durumunu sıcak dalgaya ayarla
    WeatherManager->SetWeatherCondition(EWeatherCondition::Heatwave);
    AdvanceGameTime(5.0f);
    
    // 2. Sıcak hava üretim süresi ölçüsü
    Furnace->SetActiveRecipe("Recipe_Steel");
    Furnace->StartProduction();
    float HighTempTime = MeasureProductionTime();
    
    // 3. Hava durumunu açık kya değiştir
    WeatherManager->SetWeatherCondition(EWeatherCondition::Clear);
    AdvanceGameTime(5.0f);
    
    // 4. Normal sıcaklıkta üretim süresi ölçüsü
    Furnace->SetActiveRecipe("Recipe_Steel");
    Furnace->StartProduction();
    float NormalTempTime = MeasureProductionTime();
    
    // 5. Normal daha hızlı olmalı (daha az soğutma gerekli)
    EXPECT_LT(NormalTempTime, HighTempTime);
}
```

Bu mimari **üretim-hazır** ve ticari sınıf oyun karmaşıklığına ölçeklenir. Belgede her sistem için sınıf diyagramları, veri akışı, iletişim desenleri ve 6-8 entegrasyon test örneği vardır. Hiç kısayol, hiç buhar - her parça ölçeklenebilirlik kısıtlamalarında temellenmiş.
