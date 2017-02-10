// Copyright 2015-2017 Directive Games Limited - All Rights Reserved

#include "DriftAnalyticsPrivatePCH.h"

#include "DriftAnalyticsProvider.h"
#include "IDriftProvider.h"
#include "DriftAPI.h"

#include "Engine.h"


FDriftAnalyticsProvider::FDriftAnalyticsProvider()
{
}


FDriftAnalyticsProvider::~FDriftAnalyticsProvider()
{
}


bool FDriftAnalyticsProvider::StartSession(const TArray<FAnalyticsEventAttribute>& Attributes)
{
    return true;
}


void FDriftAnalyticsProvider::EndSession()
{

}


void FDriftAnalyticsProvider::FlushEvents()
{
    auto drift = GetDrift();
    if (drift)
    {
        drift->FlushEvents();
    }
}


void FDriftAnalyticsProvider::SetUserID(const FString& InUserID)
{

}


FString FDriftAnalyticsProvider::GetUserID() const
{
    return TEXT("");
}


void FDriftAnalyticsProvider::SetBuildInfo(const FString& InBuildInfo)
{
    RecordEvent(TEXT("game.build_info"), TEXT("build_info"), InBuildInfo);
}


void FDriftAnalyticsProvider::SetGender(const FString& InGender)
{
    RecordEvent(TEXT("player.gender"), TEXT("gender"), InGender);
}


void FDriftAnalyticsProvider::SetLocation(const FString& InLocation)
{
    RecordEvent(TEXT("player.location"), TEXT("location"), InLocation);
}


void FDriftAnalyticsProvider::SetAge(const int32 InAge)
{
    RecordEvent(TEXT("player.age"), TEXT("age"), *TTypeToString<int32>::ToString(InAge));
}


FString FDriftAnalyticsProvider::GetSessionID() const
{
    return TEXT("");
}


bool FDriftAnalyticsProvider::SetSessionID(const FString& InSessionID)
{
    return false;
}


void FDriftAnalyticsProvider::RecordEvent(const FString& EventName, const TArray<FAnalyticsEventAttribute>& Attributes)
{
    auto drift = GetDrift();
    if (drift)
    {
        drift->AddAnalyticsEvent(EventName, Attributes);
    }
}


void FDriftAnalyticsProvider::RecordItemPurchase(const FString& ItemId, const FString& Currency, int PerItemCost, int ItemQuantity)
{
    TArray<FAnalyticsEventAttribute> params;
    params.Add(FAnalyticsEventAttribute(TEXT("currency"), Currency));
    params.Add(FAnalyticsEventAttribute(TEXT("per_item_cost"), PerItemCost));
    params.Add(FAnalyticsEventAttribute(TEXT("total_cost"), PerItemCost * ItemQuantity));
    RecordItemPurchase(ItemId, ItemQuantity, params);
}


void FDriftAnalyticsProvider::RecordItemPurchase(const FString& ItemId, int ItemQuantity, const TArray<FAnalyticsEventAttribute>& EventAttrs)
{
    TArray<FAnalyticsEventAttribute> params(EventAttrs);
    params.Add(FAnalyticsEventAttribute(TEXT("item_id"), ItemId));
    params.Add(FAnalyticsEventAttribute(TEXT("item_quantity"), ItemQuantity));
    RecordEvent(TEXT("economy.item_purchase"), params);
}


void FDriftAnalyticsProvider::RecordCurrencyPurchase(const FString& GameCurrencyType, int GameCurrencyAmount, const FString& RealCurrencyType, float RealMoneyCost, const FString& PaymentProvider)
{
    TArray<FAnalyticsEventAttribute> params;
    params.Add(FAnalyticsEventAttribute(TEXT("real_currency_type"), RealCurrencyType));
    params.Add(FAnalyticsEventAttribute(TEXT("real_money_cost"), RealMoneyCost));
    params.Add(FAnalyticsEventAttribute(TEXT("payment_provider"), PaymentProvider));
    RecordCurrencyPurchase(GameCurrencyType, GameCurrencyAmount, params);
}


void FDriftAnalyticsProvider::RecordCurrencyPurchase(const FString& GameCurrencyType, int GameCurrencyAmount, const TArray<FAnalyticsEventAttribute>& EventAttrs)
{
    TArray<FAnalyticsEventAttribute> params(EventAttrs);
    params.Add(FAnalyticsEventAttribute(TEXT("game_currency_type"), GameCurrencyType));
    params.Add(FAnalyticsEventAttribute(TEXT("game_currency_amount"), GameCurrencyAmount));
    RecordEvent(TEXT("economy.currency_purchase"), params);
}


void FDriftAnalyticsProvider::RecordCurrencyGiven(const FString& GameCurrencyType, int GameCurrencyAmount, const TArray<FAnalyticsEventAttribute>& EventAttrs)
{
    TArray<FAnalyticsEventAttribute> params(EventAttrs);
    params.Add(FAnalyticsEventAttribute(TEXT("game_currency_type"), GameCurrencyType));
    params.Add(FAnalyticsEventAttribute(TEXT("game_currency_amount"), GameCurrencyAmount));
    RecordEvent(TEXT("economy.currency_given"), params);
}


void FDriftAnalyticsProvider::RecordError(const FString& Error, const TArray<FAnalyticsEventAttribute>& EventAttrs)
{
    // TODO: Pass this to /clientlogs instead?
    TArray<FAnalyticsEventAttribute> params(EventAttrs);
    params.Add(FAnalyticsEventAttribute(TEXT("message"), Error));
    RecordEvent(TEXT("game.error"), params);
}


void FDriftAnalyticsProvider::RecordProgress(const FString& ProgressType, const TArray<FString>& ProgressHierarchy, const TArray<FAnalyticsEventAttribute>& EventAttrs)
{
    TArray<FAnalyticsEventAttribute> params(EventAttrs);
    params.Add(FAnalyticsEventAttribute(TEXT("progress_type"), ProgressType));
    FString hierarchy;
    // Build a dotted hierarchy string from the list of hierarchy progress
    for (int32 Index = 0; Index < ProgressHierarchy.Num(); Index++)
    {
        hierarchy += ProgressHierarchy[Index];
        if (Index + 1 < ProgressHierarchy.Num())
        {
            hierarchy += TEXT(".");
        }
    }
    params.Add(FAnalyticsEventAttribute(TEXT("progress_hierarchy"), hierarchy));
    RecordEvent(TEXT("player.progression"), params);
}


const FName DriftProviderName = TEXT("Drift");


IDriftAPI* FDriftAnalyticsProvider::GetDrift()
{
    FName identifier = NAME_None;

#if UE_EDITOR
    /**
     * In PIE, just get any PIE world.
     */
    const FWorldContext* contextToUse = nullptr;
    
    for (const FWorldContext& worldContext : GEngine->GetWorldContexts())
    {
        if (worldContext.WorldType == EWorldType::PIE && contextToUse == nullptr)
        {
            contextToUse = &worldContext;
            break;
        }
    }
    
    if (contextToUse != nullptr)
    {
        identifier = contextToUse->ContextHandle;
    }
#endif // UE_EDITOR
 
    auto& provider = IModularFeatures::Get().GetModularFeature<IDriftProvider>(DriftProviderName);
    return provider.GetInstance(identifier);
}
