#include "TradeLogger.hpp"
#include <fstream>
#include <iostream>

BAKKESMOD_PLUGIN(TradeLogger, "ItsBranK's Trade Logger", "1.7", PLUGINTYPE_FREEPLAY)

TradeId::TradeId() : Guid(0), Format(EGuidFormats::Digits) {}

TradeId::TradeId(const TradeId& tradeId) : Guid(tradeId.Guid), Format(tradeId.Format) {}

TradeId::~TradeId() {}

GuidWrapper TradeId::GetGuid() const
{
	return Guid;
}

std::string TradeId::GetGuidStr() const
{
	return Guid.ToString(Format);
}

EGuidFormats TradeId::GetFormat() const
{
	return Format;
}

std::string TradeId::GetFormatStr() const
{
	switch (Format)
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
	}

	return "Unknown";
}

void TradeId::SetGuid(const GuidWrapper& tradeGuid)
{
	Guid = tradeGuid;
}

void TradeId::SetFormat(EGuidFormats newFormat)
{
	Format = newFormat;
}

bool TradeId::IsValid() const
{
	return Guid.IsValid();
}

TradeId& TradeId::operator=(const TradeId& other)
{
	Guid = other.Guid;
	Format = other.Format;
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
	DataFolder = (gameWrapper->GetBakkesModPath() / "data" / "TradeLogger");
	gameWrapper->HookEventWithCaller<ActorWrapper>("Function TAGame.GFxData_TradeLobby_TA.GetRemotePlayerName", std::bind(&TradeLogger::TradeAccept, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	gameWrapper->HookEventWithCaller<ActorWrapper>("Function TAGame.GFxData_PartyMemberProfile_TA.OnRemoved", std::bind(&TradeLogger::TradeCancel, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	gameWrapper->HookEventWithCaller<ActorWrapper>("Function TAGame.GFxData_TradeLobby_TA.HandleTradeProductUpdate", std::bind(&TradeLogger::TradeUpdate, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	gameWrapper->HookEventWithCaller<ActorWrapper>("Function TAGame.GFxData_TradeLobby_TA.HandleTradeCurrencyUpdate", std::bind(&TradeLogger::TradeUpdate, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	gameWrapper->HookEventWithCaller<ActorWrapper>("Function TAGame.GFxData_TradeLobby_TA.SetTransactionQuantity", std::bind(&TradeLogger::TradeUpdate, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	gameWrapper->HookEventWithCaller<ActorWrapper>("Function TAGame.GFxData_TradeLobby_TA.HandleTradePlayerError", std::bind(&TradeLogger::TradeCancel, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	gameWrapper->HookEventWithCaller<ActorWrapper>("Function TAGame.__OnlineGameParty_TA__SendTradeToBackEnd_0x1.__OnlineGameParty_TA__SendTradeToBackEnd_0x1", std::bind(&TradeLogger::TradeComplete, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

void TradeLogger::onUnload()
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

void TradeLogger::LogTrade(const TradeInfo& tradeInfo)
{
	if (!tradeInfo.Id.IsValid())
	{
		cvarManager->log("(LogTrade) Warning: TradeId is invalid! Please contact ItsBranK with any information about your trade!");
		return;
	}

	if (!std::filesystem::exists(DataFolder))
	{
		std::filesystem::create_directory(DataFolder);
	}

	if (std::filesystem::exists(DataFolder))
	{
		std::string fileName = "TradeDate_" + std::to_string(std::time(nullptr)) + ".json";
		std::ofstream logFile(DataFolder / fileName);

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
	if (!IsTrading)
	{
		IsTrading = true;
		ActiveTrade.StartEpoch = std::time(nullptr);
		cvarManager->log("(TradeAccept) Monitoring trade!");
	}
}

void TradeLogger::TradeCancel(ActorWrapper caller, void* params, const std::string& functionName)
{
	ActiveTrade.Reset();
	IsTrading = false;
}

void TradeLogger::TradeUpdate(ActorWrapper caller, void* params, const std::string& functionName)
{
	if (caller)
	{
		TradeWrapper tradeWrapper(caller.memory_address);

		if (tradeWrapper)
		{
			GUIDWrapper brokenWrapper = tradeWrapper.GetTradeGuid();
			ActiveTrade.Id.SetGuid(GuidWrapper(brokenWrapper.A, brokenWrapper.B, brokenWrapper.C, brokenWrapper.D));
		}
	}
}

void TradeLogger::TradeComplete(ActorWrapper caller, void* params, const std::string& functionName)
{
	TradeWrapper wrappedTrade = gameWrapper->GetItemsWrapper().GetTradeWrapper();

	if (wrappedTrade)
	{
		ActiveTrade.RemotePlayer = wrappedTrade.GetTradingPlayer();
		ArrayWrapper<OnlineProductWrapper> localProducts = wrappedTrade.GetSendingProducts();
		ArrayWrapper<OnlineProductWrapper> remoteProducts = wrappedTrade.GetReceivingProducts();

		for (OnlineProductWrapper localProduct : localProducts)
		{
			if (localProduct)
			{
				ProductInstanceID localId = localProduct.GetInstanceIDV2();
				ActiveTrade.LocalData.Names += ("\"" + localProduct.GetLongLabel().ToString() + "\", ");
				ActiveTrade.LocalData.Instances += ("\"" + std::to_string(localId.upper_bits) + "-" + std::to_string(localId.lower_bits) + "\", ");
			}
		}

		for (OnlineProductWrapper remoteProduct : remoteProducts)
		{
			if (remoteProduct)
			{
				ProductInstanceID remoteId = remoteProduct.GetInstanceIDV2();
				ActiveTrade.RemoteData.Names += ("\"" + remoteProduct.GetLongLabel().ToString() + "\", ");
				ActiveTrade.RemoteData.Instances += ("\"" + std::to_string(remoteId.upper_bits) + "-" + std::to_string(remoteId.lower_bits) + "\", ");
			}
		}

		std::vector<TradeWrapper::Currency> localCurrency = wrappedTrade.GetSendingCurrency();
		std::vector<TradeWrapper::Currency> remoteCurrency = wrappedTrade.GetReceivingCurrency();

		if (!localCurrency.empty())
		{
			ActiveTrade.LocalData.CurrencyId = localCurrency[0].currency_id;
			ActiveTrade.LocalData.CurrencyAmount = localCurrency[0].quantity;
		}

		if (!remoteCurrency.empty())
		{
			ActiveTrade.RemoteData.CurrencyId = remoteCurrency[0].currency_id;
			ActiveTrade.RemoteData.CurrencyAmount = remoteCurrency[0].quantity;
		}

		ActiveTrade.EndEpoch = std::time(nullptr);
		ActiveTrade.Id.SetFormat(EGuidFormats::UniqueObjectGuid);
		LogTrade(ActiveTrade);
		ActiveTrade.Reset();
		IsTrading = false;
	}
	else
	{
		cvarManager->log("(TradeVerify) Error: Invalid wrapper! Please contact ItsBranK with any information about your trade!");
	}
}