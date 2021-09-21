#include "TradeLogger.hpp"
#include <fstream>
#include <iostream>

BAKKESMOD_PLUGIN(TradeLogger, "ItsBranK's Trade Logger", "1.2", PLUGINTYPE_THREADED)

std::string TradeLogger::GuidFormatStr(EGuidFormats format)
{
	switch (format)
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
	//case EGuidFormats::Short:
	//	return "Short";
	//case EGuidFormats::Base36Encoded:
	//	return "Base36Encoded";
	}

	return "Unknown";
}

void TradeLogger::onLoad()
{
	DataFolder = gameWrapper->GetBakkesModPath() / "data" / "TradeLogger";

	gameWrapper->HookEventWithCaller<ActorWrapper>("Function TAGame.GFxData_TradeLobby_TA.HandleAcceptedInviteToTrade", std::bind(&TradeLogger::TradeAccept, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	gameWrapper->HookEventWithCaller<ActorWrapper>("Function TAGame.GFxData_Party_TA.HandleCloseInviteToTrade", std::bind(&TradeLogger::TradeCancel, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	gameWrapper->HookEventWithCaller<ActorWrapper>("Function TAGame.GFxData_TradeLobby_TA.HandleTradeProductUpdate", std::bind(&TradeLogger::TradeUpdate, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	gameWrapper->HookEventWithCaller<ActorWrapper>("Function TAGame.GFxData_TradeLobby_TA.HandleTradeCurrencyUpdate", std::bind(&TradeLogger::TradeUpdate, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	gameWrapper->HookEventWithCaller<ActorWrapper>("Function TAGame.GFxData_TradeLobby_TA.SetTransactionQuantity", std::bind(&TradeLogger::TradeUpdate, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	gameWrapper->HookEventWithCaller<ActorWrapper>("Function TAGame.GFxData_TradeLobby_TA.HandleTradePlayerError", std::bind(&TradeLogger::TradeCancel, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	gameWrapper->HookEventWithCaller<ActorWrapper>("Function TAGame.GFxData_TradeLobby_TA.HandleTradePlayerComplete", std::bind(&TradeLogger::TradeComplete, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	gameWrapper->HookEventWithCaller<ActorWrapper>("Function TAGame.OnlineGameParty_TA.SendTradeToBackEnd", std::bind(&TradeLogger::TradeVerify, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

void TradeLogger::onUnload()
{
	gameWrapper->UnhookEvent("Function TAGame.GFxData_TradeLobby_TA.HandleAcceptedInviteToTrade");
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
	if (!tradeInfo.TradeId.Guid.IsValid())
	{
		cvarManager->log("(LogTrade) Warning: TradeId is invalid! Please contact ItsBranK with any information about your trade!");
	}

	std::string fileName = "TradeDate_" + std::to_string(std::time(nullptr)) + ".json";

	if (!std::filesystem::exists(DataFolder))
	{
		std::filesystem::create_directory(DataFolder);
	}

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

	logFile << "\t\t\"TradeGuid\": \"" + tradeInfo.TradeId.Guid.ToString(tradeInfo.TradeId.Format) + "\"," << std::endl;
	logFile << "\t\t\"TradeGuidFormat\": \"EGuidFormats::" + GuidFormatStr(tradeInfo.TradeId.Format) + "\"" << std::endl;
	logFile << "\t\t\"TradeStartEpoch\": \"" + std::to_string(tradeInfo.StartEpoch) + "\"," << std::endl;
	logFile << "\t\t\"TradeEndEpoch\": \"" + std::to_string(tradeInfo.EndEpoch) + "\"" << std::endl;

	logFile << "\t}" << std::endl;
	logFile << "]" << std::endl;

	logFile.close();

	cvarManager->log("(LogTrade) Logged to file: " + fileName);
}

void TradeLogger::TradeAccept(ActorWrapper caller, void* params, const std::string& functionName)
{
	ActiveTrade.StartEpoch = std::time(nullptr);
	cvarManager->log("(TradeAccept) Monitoring trade!");
}

void TradeLogger::TradeCancel(ActorWrapper caller, void* params, const std::string& functionName)
{
	ActiveTrade.Clear();
	CurrentTrade = NULL;
}

void TradeLogger::TradeUpdate(ActorWrapper caller, void* params, const std::string& functionName)
{
	if (caller)
	{
		CurrentTrade = caller.memory_address;
		TradeWrapper tradeWrapper(caller.memory_address);

		if (tradeWrapper)
		{
			GUIDWrapper brokenWrapper = tradeWrapper.GetTradeGuid();
			ActiveTrade.TradeId.Guid = { brokenWrapper.A, brokenWrapper.B, brokenWrapper.C, brokenWrapper.D };
		}
	}
}

void TradeLogger::TradeVerify(ActorWrapper caller, void* params, const std::string& functionName)
{
	if (CurrentTrade)
	{
		TradeWrapper wrappedTrade(CurrentTrade);

		if (wrappedTrade)
		{
			ArrayWrapper<OnlineProductWrapper> localProducts = wrappedTrade.GetSendingProducts();
			ArrayWrapper<OnlineProductWrapper> remoteProducts = wrappedTrade.GetReceivingProducts();

			for (OnlineProductWrapper localProduct : localProducts)
			{
				if (localProduct)
				{
					ActiveTrade.LocalData.Names += "\"" + localProduct.GetLongLabel().ToString() + "\", ";
					ActiveTrade.LocalData.Instances += "\"" + std::to_string(localProduct.GetInstanceID()) + "\", ";
				}
			}

			for (OnlineProductWrapper remoteProduct : remoteProducts)
			{
				if (remoteProduct)
				{
					ActiveTrade.RemoteData.Names += "\"" + remoteProduct.GetLongLabel().ToString() + "\", ";
					ActiveTrade.RemoteData.Instances += "\"" + std::to_string(remoteProduct.GetInstanceID()) + "\", ";
				}
			}

			std::vector<TradeWrapper::Currency> localCurrency = wrappedTrade.GetSendingCurrency();
			std::vector<TradeWrapper::Currency> remoteCurrency = wrappedTrade.GetReceivingCurrency();

			if (localCurrency.size() > 0)
			{
				ActiveTrade.LocalData.CurrencyId = localCurrency[0].currency_id;
				ActiveTrade.LocalData.CurrencyAmount = localCurrency[0].quantity;
			}

			if (remoteCurrency.size() > 0)
			{
				ActiveTrade.RemoteData.CurrencyId = remoteCurrency[0].currency_id;
				ActiveTrade.RemoteData.CurrencyAmount = remoteCurrency[0].quantity;
			}
		}
		else
		{
			cvarManager->log("(TradeVerify) Error: Invalid wrapper! Please contact ItsBranK with any information about your trade!");
		}
	}
	else
	{
		cvarManager->log("(TradeVerify) Error: Invalid pointer! Please contact ItsBranK with any information about your trade!");
	}
}

void TradeLogger::TradeComplete(ActorWrapper caller, void* params, const std::string& functionName)
{
	ActiveTrade.EndEpoch = std::time(nullptr);
	ActiveTrade.TradeId.Format = EGuidFormats::UniqueObjectGuid;

	LogTrade(ActiveTrade);

	ActiveTrade.Clear();
	CurrentTrade = NULL;
}