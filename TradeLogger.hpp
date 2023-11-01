#pragma once
#pragma comment(lib, "pluginsdk.lib")
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "Wrappers/GuidWrapper.hpp"

class TradeId
{
private:
	GuidWrapper _guid;
	EGuidFormats _format;

public:
	TradeId();
	TradeId(const TradeId& tradeId);
	~TradeId();

public:
	GuidWrapper GetGuid() const;
	std::string GetGuidStr() const;
	EGuidFormats GetFormat() const;
	std::string GetFormatStr() const;
	void SetGuid(const GuidWrapper& tradeGuid);
	void SetFormat(EGuidFormats newFormat);
	bool IsValid() const;

public:
	TradeId& operator=(const TradeId& other);
};

class InventoryInfo
{
public:
	std::string Names;
	std::string Instances;
	int32_t CurrencyId;
	int32_t CurrencyAmount;

public:
	InventoryInfo();
	InventoryInfo(const InventoryInfo& inventoryInfo);
	~InventoryInfo();

public:
	InventoryInfo& operator=(const InventoryInfo& other);
};

class TradeInfo
{
public:
	UniqueIDWrapper LocalPlayer;
	UniqueIDWrapper RemotePlayer;
	InventoryInfo LocalData;
	InventoryInfo RemoteData;
	TradeId Id;
	uint64_t StartEpoch;
	uint64_t EndEpoch;

public:
	TradeInfo();
	TradeInfo(const TradeInfo& tradeInfo);
	~TradeInfo();

public:
	void Reset();

public:
	TradeInfo& operator=(const TradeInfo& other);
};

class TradeLogger : public BakkesMod::Plugin::BakkesModPlugin
{
private:
	std::filesystem::path _dataFolder;
	TradeInfo _activeTrade;
	bool _isTrading = false;
	uint64_t _killStamp = 1701860766; // Wed Dec 06 2023 11:06:06 GMT+0000

public:
	virtual void onLoad();
	virtual void onUnload();
	bool CanTrade() const;
	void LogTrade(const TradeInfo& tradeInfo);
	void TradeAccept(ActorWrapper caller, void* params, const std::string& functionName);
	void TradeCancel(ActorWrapper caller, void* params, const std::string& functionName);
	void TradeUpdate(ActorWrapper caller, void* params, const std::string& functionName);
	void TradeComplete(ActorWrapper caller, void* params, const std::string& functionName);
};