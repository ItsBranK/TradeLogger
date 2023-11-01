#include "TradeLogger.hpp"
#include <fstream>
#include <iostream>

BAKKESMOD_PLUGIN(TradeLogger, "ItsBranK's Trade Logger", "1.9", PLUGINTYPE_FREEPLAY)

TradeId::TradeId() : _guid(0), _format(EGuidFormats::Digits) {}

TradeId::TradeId(const TradeId& tradeId) : _guid(tradeId._guid), _format(tradeId._format) {}

TradeId::~TradeId() {}

GuidWrapper TradeId::GetGuid() const
{
	return _guid;
}

std::string TradeId::GetGuidStr() const
{
	return _guid.ToString(_format);
}

EGuidFormats TradeId::GetFormat() const
{
	return _format;
}

std::string TradeId::GetFormatStr() const
{
	switch (_format)
	{
	case EGuidFormats::Digits:
		return "Digits";
	case EGuidFormats::DigitsWithHyphens:
		return "DigitsWithHyphens";
	case EGuidFormats::DigitsWithHyphensInBraces:
		return "DigitsWithHyphensInBraces";
	case EGuidFormats::DigitsWithHyphensInParentheses:
		return "DigitsWithHyphensInParentheses";
	case EGuidFormats::HexValuesInBraces:
		return "HexValuesInBraces";
	case EGuidFormats::UniqueObjectGuid:
		return "UniqueObjectGuid";
	default:
		return "Unknown";
	}
}

void TradeId::SetGuid(const GuidWrapper& tradeGuid)
{
	_guid = tradeGuid;
}

void TradeId::SetFormat(EGuidFormats newFormat)
{
	_format = newFormat;
}

bool TradeId::IsValid() const
{
	return _guid.IsValid();
}

TradeId& TradeId::operator=(const TradeId& other)
{
	_guid = other._guid;
	_format = other._format;
	return *this;
}

InventoryInfo::InventoryInfo() : CurrencyId(-1), CurrencyAmount(0) {}

InventoryInfo::InventoryInfo(const InventoryInfo& inventoryInfo) : CurrencyId(inventoryInfo.CurrencyId), CurrencyAmount(inventoryInfo.CurrencyAmount) {}

InventoryInfo::~InventoryInfo() {}

InventoryInfo& InventoryInfo::operator=(const InventoryInfo& other)
{
	Names = other.Names;
	Instances = other.Instances;
	CurrencyId = other.CurrencyId;
	CurrencyAmount = other.CurrencyAmount;
	return *this;
}

TradeInfo::TradeInfo()
{
	Reset();
}

TradeInfo::TradeInfo(const TradeInfo& tradeInfo) :
	LocalPlayer(tradeInfo.LocalPlayer),
	RemotePlayer(tradeInfo.RemotePlayer),
	LocalData(tradeInfo.LocalData),
	RemoteData(tradeInfo.RemoteData),
	Id(tradeInfo.Id),
	StartEpoch(tradeInfo.StartEpoch),
	EndEpoch(tradeInfo.EndEpoch)
{

}

TradeInfo::~TradeInfo() {}

void TradeInfo::Reset()
{
	LocalPlayer = UniqueIDWrapper();
	RemotePlayer = UniqueIDWrapper();
	LocalData = InventoryInfo();
	RemoteData = InventoryInfo();
	Id = TradeId();
	StartEpoch = 0;
	EndEpoch = 0;
}

TradeInfo& TradeInfo::operator=(const TradeInfo& other)
{
	LocalPlayer = other.LocalPlayer;
	RemotePlayer = other.RemotePlayer;
	Id = other.Id;
	LocalData = other.LocalData;
	RemoteData = other.RemoteData;
	StartEpoch = other.StartEpoch;
	EndEpoch = other.EndEpoch;
	return *this;
}

void TradeLogger::onLoad()
{
	if (CanTrade())
	{
		_dataFolder = (gameWrapper->GetBakkesModPath() / "data" / "TradeLogger");
		gameWrapper->HookEventWithCaller<ActorWrapper>("Function TAGame.GFxData_TradeLobby_TA.GetRemotePlayerName", std::bind(&TradeLogger::TradeAccept, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
		gameWrapper->HookEventWithCaller<ActorWrapper>("Function TAGame.GFxData_PartyMemberProfile_TA.OnRemoved", std::bind(&TradeLogger::TradeCancel, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
		gameWrapper->HookEventWithCaller<ActorWrapper>("Function TAGame.GFxData_TradeLobby_TA.HandleTradeProductUpdate", std::bind(&TradeLogger::TradeUpdate, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
		gameWrapper->HookEventWithCaller<ActorWrapper>("Function TAGame.GFxData_TradeLobby_TA.HandleTradeCurrencyUpdate", std::bind(&TradeLogger::TradeUpdate, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
		gameWrapper->HookEventWithCaller<ActorWrapper>("Function TAGame.GFxData_TradeLobby_TA.SetTransactionQuantity", std::bind(&TradeLogger::TradeUpdate, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
		gameWrapper->HookEventWithCaller<ActorWrapper>("Function TAGame.GFxData_TradeLobby_TA.HandleTradePlayerError", std::bind(&TradeLogger::TradeCancel, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
		gameWrapper->HookEventWithCaller<ActorWrapper>("Function TAGame.__OnlineGameParty_TA__SendTradeToBackEnd_0x1.__OnlineGameParty_TA__SendTradeToBackEnd_0x1", std::bind(&TradeLogger::TradeComplete, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	}
	else
	{
		cvarManager->log("(onLoad) Error: This plugin is no longer functional due to Epic Games removing player to player trading!");
	}
}

void TradeLogger::onUnload()
{
	if (CanTrade())
	{
		gameWrapper->UnhookEvent("Function ProjectX.OnlineGameParty_X.HandleAcceptInviteToTrade");
		gameWrapper->UnhookEvent("Function TAGame.GFxData_Party_TA.HandleCloseInviteToTrade");
		gameWrapper->UnhookEvent("Function TAGame.GFxData_TradeLobby_TA.HandleTradeProductUpdate");
		gameWrapper->UnhookEvent("Function TAGame.GFxData_TradeLobby_TA.HandleTradeCurrencyUpdate");
		gameWrapper->UnhookEvent("Function TAGame.GFxData_TradeLobby_TA.SetTransactionQuantity");
		gameWrapper->UnhookEvent("Function TAGame.GFxData_TradeLobby_TA.HandleTradePlayerError");
		gameWrapper->UnhookEvent("Function TAGame.GFxData_TradeLobby_TA.HandleTradePlayerComplete");
		gameWrapper->UnhookEvent("Function TAGame.OnlineGameParty_TA.SendTradeToBackEnd");
	}
}

bool TradeLogger::CanTrade() const
{
	uint64_t currentTime = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock().now().time_since_epoch()).count();
	return (currentTime < _killStamp);
}

void TradeLogger::LogTrade(const TradeInfo& tradeInfo)
{
	if (!tradeInfo.Id.IsValid())
	{
		cvarManager->log("(LogTrade) Warning: TradeId is invalid! Please contact ItsBranK with any information about your trade!");
		return;
	}

	if (!std::filesystem::exists(_dataFolder))
	{
		std::filesystem::create_directory(_dataFolder);
	}

	if (std::filesystem::exists(_dataFolder))
	{
		std::string fileName = "TradeDate_" + std::to_string(std::time(nullptr)) + ".json";
		std::ofstream logFile(_dataFolder / fileName);

		logFile << "[" << std::endl;
		logFile << "\t{" << std::endl;
		logFile << "\t\t\"LocalPlayer\": \"" + gameWrapper->GetUniqueID().GetIdString() + "\"," << std::endl;

		if (tradeInfo.LocalData.Names.length() > 3)
		{
			std::string formattedNames = tradeInfo.LocalData.Names;
			formattedNames.erase(formattedNames.length() - 2, 2);
			logFile << "\t\t\"LocalProducts\": [" << formattedNames << "]," << std::endl;
		}
		else
		{
			logFile << "\t\t\"LocalProducts\": [\"\"]," << std::endl;
		}

		if (tradeInfo.LocalData.Instances.length() > 3)
		{
			std::string formattedProducts = tradeInfo.LocalData.Instances;
			formattedProducts.erase(formattedProducts.length() - 2, 2);
			logFile << "\t\t\"LocalInstances\": [" << formattedProducts << "]," << std::endl;
		}
		else
		{
			logFile << "\t\t\"LocalInstances\": [\"\"]," << std::endl;
		}

		logFile << "\t\t\"LocalCurrencyId\": " + std::to_string(tradeInfo.LocalData.CurrencyId) + "," << std::endl;
		logFile << "\t\t\"LocalCurrencyAmount\": " + std::to_string(tradeInfo.LocalData.CurrencyAmount) + "," << std::endl;

		logFile << "\t\t\"RemotePlayer\": \"" + tradeInfo.RemotePlayer.str() + "\"," << std::endl;

		if (tradeInfo.RemoteData.Names.length() > 3)
		{
			std::string formattedNames = tradeInfo.RemoteData.Names;
			formattedNames.erase(formattedNames.length() - 2, 2);
			logFile << "\t\t\"RemoteProducts\": [" << formattedNames << "]," << std::endl;
		}
		else
		{
			logFile << "\t\t\"RemoteProducts\": [\"\"]," << std::endl;
		}

		if (tradeInfo.RemoteData.Instances.length() > 3)
		{
			std::string formattedProducts = tradeInfo.RemoteData.Instances;
			formattedProducts.erase(formattedProducts.length() - 2, 2);
			logFile << "\t\t\"RemoteInstances\": [" << formattedProducts << "]," << std::endl;
		}
		else
		{
			logFile << "\t\t\"RemoteInstances\": [\"\"]," << std::endl;
		}

		logFile << "\t\t\"RemoteCurrencyId\": " + std::to_string(tradeInfo.RemoteData.CurrencyId) + "," << std::endl;
		logFile << "\t\t\"RemoteCurrencyAmount\": " + std::to_string(tradeInfo.RemoteData.CurrencyAmount) + "," << std::endl;

		logFile << "\t\t\"TradeGuid\": \"" + tradeInfo.Id.GetGuidStr() + "\"," << std::endl;
		logFile << "\t\t\"TradeGuidFormat\": \"EGuidFormats::" + tradeInfo.Id.GetFormatStr() + "\"," << std::endl;
		logFile << "\t\t\"TradeStartEpoch\": \"" + std::to_string(tradeInfo.StartEpoch) + "\"," << std::endl;
		logFile << "\t\t\"TradeEndEpoch\": \"" + std::to_string(tradeInfo.EndEpoch) + "\"" << std::endl;

		logFile << "\t}" << std::endl;
		logFile << "]" << std::endl;

		logFile.close();

		cvarManager->log("(LogTrade) Logged to file: " + fileName);
	}
	else
	{
		cvarManager->log("(LogTrade) Error: Failed to log trade, data folder does not exist!");
	}
}

void TradeLogger::TradeAccept(ActorWrapper caller, void* params, const std::string& functionName)
{
	if (!_isTrading)
	{
		_isTrading = true;
		_activeTrade.StartEpoch = std::time(nullptr);
		cvarManager->log("(TradeAccept) Monitoring trade with player \"" + TradeWrapper(caller.memory_address).GetTradingPlayer().GetIdString() + "\"!");
	}
}

void TradeLogger::TradeCancel(ActorWrapper caller, void* params, const std::string& functionName)
{
	_activeTrade.Reset();
	_isTrading = false;
}

void TradeLogger::TradeUpdate(ActorWrapper caller, void* params, const std::string& functionName)
{
	if (caller)
	{
		TradeWrapper tradeWrapper(caller.memory_address);

		if (tradeWrapper)
		{
			GUIDWrapper brokenWrapper = tradeWrapper.GetTradeGuid();
			_activeTrade.Id.SetGuid(GuidWrapper(brokenWrapper.A, brokenWrapper.B, brokenWrapper.C, brokenWrapper.D));
		}
	}
}

void TradeLogger::TradeComplete(ActorWrapper caller, void* params, const std::string& functionName)
{
	TradeWrapper wrappedTrade = gameWrapper->GetItemsWrapper().GetTradeWrapper();

	if (wrappedTrade)
	{
		_activeTrade.RemotePlayer = wrappedTrade.GetTradingPlayer();
		ArrayWrapper<OnlineProductWrapper> localProducts = wrappedTrade.GetSendingProducts();
		ArrayWrapper<OnlineProductWrapper> remoteProducts = wrappedTrade.GetReceivingProducts();

		for (OnlineProductWrapper localProduct : localProducts)
		{
			if (localProduct)
			{
				ProductInstanceID localId = localProduct.GetInstanceIDV2();
				_activeTrade.LocalData.Names += ("\"" + localProduct.GetLongLabel().ToString() + "\", ");
				_activeTrade.LocalData.Instances += ("\"" + std::to_string(localId.upper_bits) + "-" + std::to_string(localId.lower_bits) + "\", ");
			}
		}

		for (OnlineProductWrapper remoteProduct : remoteProducts)
		{
			if (remoteProduct)
			{
				ProductInstanceID remoteId = remoteProduct.GetInstanceIDV2();
				_activeTrade.RemoteData.Names += ("\"" + remoteProduct.GetLongLabel().ToString() + "\", ");
				_activeTrade.RemoteData.Instances += ("\"" + std::to_string(remoteId.upper_bits) + "-" + std::to_string(remoteId.lower_bits) + "\", ");
			}
		}

		std::vector<TradeWrapper::Currency> localCurrency = wrappedTrade.GetSendingCurrency();
		std::vector<TradeWrapper::Currency> remoteCurrency = wrappedTrade.GetReceivingCurrency();

		if (!localCurrency.empty())
		{
			_activeTrade.LocalData.CurrencyId = localCurrency[0].currency_id;
			_activeTrade.LocalData.CurrencyAmount = localCurrency[0].quantity;
		}

		if (!remoteCurrency.empty())
		{
			_activeTrade.RemoteData.CurrencyId = remoteCurrency[0].currency_id;
			_activeTrade.RemoteData.CurrencyAmount = remoteCurrency[0].quantity;
		}

		_activeTrade.EndEpoch = std::time(nullptr);
		_activeTrade.Id.SetFormat(EGuidFormats::UniqueObjectGuid);
		LogTrade(_activeTrade);
		TradeCancel(ActorWrapper(0), nullptr, "");
	}
	else
	{
		cvarManager->log("(TradeVerify) Error: Invalid wrapper! Please contact ItsBranK with any information about your trade!");
	}
}