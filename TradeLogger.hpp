#pragma once
#pragma comment(lib, "pluginsdk.lib")
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "Wrappers/GuidWrapper.hpp"

class TradeIdInfo
{
public:
	GuidWrapper Guid;
	EGuidFormats Format;

public:
	TradeIdInfo()
	{
		Format = EGuidFormats::Digits;
	}

	~TradeIdInfo() { }

public:
	TradeIdInfo operator=(const TradeIdInfo& other)
	{
		Guid = other.Guid;
		Format = other.Format;
		return *this;
	}
};

class InventoryInfo
{
public:
	std::string Names;
	std::string Instances;
	int32_t CurrencyId;
	int32_t CurrencyAmount;

public:
	InventoryInfo()
	{
		CurrencyId = -1;
		CurrencyAmount = 0;
	}

	~InventoryInfo() { }

public:
	InventoryInfo operator=(const InventoryInfo& other)
	{
		Names = other.Names;
		Instances = other.Instances;
		CurrencyId = other.CurrencyId;
		CurrencyAmount = other.CurrencyAmount;
		return *this;
	}
};

class TradeInfo
{
public:
	UniqueIDWrapper LocalPlayer;
	UniqueIDWrapper RemotePlayer;
	TradeIdInfo TradeId;
	InventoryInfo LocalData;
	InventoryInfo RemoteData;
	uint64_t StartEpoch;
	uint64_t EndEpoch;

public:
	TradeInfo()
	{
		StartEpoch = 0;
		EndEpoch = 0;
	}

	~TradeInfo() { }

public:
	void Clear()
	{
		LocalPlayer = UniqueIDWrapper();
		RemotePlayer = UniqueIDWrapper();
		TradeId = TradeIdInfo{};
		LocalData = InventoryInfo{};
		RemoteData = InventoryInfo{};
		StartEpoch = 0;
		EndEpoch = 0;
	}

public:
	TradeInfo operator=(const TradeInfo& other)
	{
		LocalPlayer = other.LocalPlayer;
		RemotePlayer = other.RemotePlayer;
		TradeId = other.TradeId;
		LocalData = other.LocalData;
		RemoteData = other.RemoteData;
		StartEpoch = other.StartEpoch;
		EndEpoch = other.EndEpoch;
		return *this;
	}
};

class TradeLogger : public BakkesMod::Plugin::BakkesModPlugin
{
private:
	std::filesystem::path DataFolder;
	TradeInfo ActiveTrade;
	uintptr_t CurrentTrade;

public:
	std::string GuidFormatStr(EGuidFormats format);
	virtual void onLoad();
	virtual void onUnload();
	void LogTrade(const TradeInfo& tradeInfo);
	void TradeAccept(ActorWrapper caller, void* params, const std::string& functionName);
	void TradeCancel(ActorWrapper caller, void* params, const std::string& functionName);
	void TradeUpdate(ActorWrapper caller, void* params, const std::string& functionName);
	void TradeVerify(ActorWrapper caller, void* params, const std::string& functionName);
	void TradeComplete(ActorWrapper caller, void* params, const std::string& functionName);
};