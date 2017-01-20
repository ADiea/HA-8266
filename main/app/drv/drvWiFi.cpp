#include "drv/drvWiFi.h"

#include "appMain.h"

CDrvWiFi DrvWiFi;

eDriverError CDrvWiFi::setup(eDriverOp op)
{
	eDriverError retErr = drvErrOther;
	do
	{
		if(drvEnable == op)
		{
			//the ESP8266 station will not try to connect
			//to the router automatically when power on until wifi_station_connect is called.
			wifi_station_set_auto_connect(false);

			retErr = drvErrOK;
			m_State = drvEnabled;
		}
		else if (drvDisable == op)
		{
			retErr = drvErrOK;
			m_State = drvDisabled;
		}
	} while(0);
	m_lastError = retErr;
	return retErr;
}


void CDrvWiFi::startWiFi(SystemSettings& sysSettings)
{
	if(sysSettings.wifiStationIsConfigured)
		{
			LOG_I("WiFi is configured, start STA");
			WifiAccessPoint.enable(false);
			WifiStation.enable(true);
			WifiStation.config(sysSettings.wifiSSID, sysSettings.wifiPwd);
			
			wifi_station_set_reconnect_policy(true);

			// Run our method when station was connected to AP
			WifiStation.waitConnection(connectOk, 30, connectFail);
		}
		else
		{
			LOG_I("WiFi is _not_ configured");

			/*
			PHY_MODE_11B	= 1,
			PHY_MODE_11G	= 2,
			PHY_MODE_11N    = 3
			*/
			if(wifi_get_phy_mode() != PHY_MODE_11N)
			{
				wifi_set_phy_mode(PHY_MODE_11N);
				LOG_I("Set WiFi PHY mode to N");
			}

			WifiAccessPoint.enable(true);
			WifiAccessPoint.config("Casa_1254", "abcd1234", AUTH_WPA2_PSK, false, 8, 300);

			WifiStation.enable(true);
			wifi_station_set_reconnect_policy(false);
			WifiStation.disconnect();


			WifiAccessPoint.setIP(IPAddress(192, 168, 1, 1),
								  IPAddress(192, 168, 1, 3),
								  IPAddress(192, 168, 1, 6));

			// Set system ready callback method
			System.onReady(startAPScan);

			WifiEvents.onAccessPointProbeReqRecved(probeRcved);
		}
}

///////////

void scanAPFinished(bool succeeded, BssList list);

void setupAPMode()
{
	LOG_E("WiFi AP not set, switch on SoftAP");
	//WifiStation.enable(false);
	//WifiAccessPoint.enable(true);

	//startWebServers();
/*
	softap_config config = {0};
	wifi_softap_get_config(&config);
	LOG_I("AP is set to ch%d maxc%d beacon%d", config.channel,
			config.max_connection, config.beacon_interval);

	WifiAccessPoint.config("Casa_1254", "", AUTH_OPEN, false, 8, 300);

	wifi_softap_get_config(&config);
	LOG_I("(2)AP is set to ch%d maxc%d beacon%d", config.channel,
			config.max_connection, config.beacon_interval);
*/
	debugf("AP. ip: %s mac: %s", WifiAccessPoint.getIP().toString().c_str(), WifiAccessPoint.getMAC().c_str());
	tmrHalfSecond.initializeUs(ONE_SECOND, startWebServers).start(false);
}

void startAPScan()
{
	LOG_I("start ap scan");
	WifiStation.startScan(scanAPFinished);
}

void scanAPFinished(bool succeeded, BssList list)
{
	if (!succeeded)
	{
		LOG_E("Failed to scan networks");
		startAPScan();
	}

	LOG_I("WiFi: found %d networks", list.count());
	for (int i = 0; i < list.count(); i++)
	{
		LOG_I("WiFi: %s, %s, %s ch:%d", list[i].ssid.c_str(), list[i].getAuthorizationMethodName(),
				list[i].hidden ? "(bcast)":"(hidden)", list[i].channel);
	}

	//wifi_station_set_reconnect_policy(false);
	//wifi_station_set_auto_connect(false);
	//wifi_station_disconnect();

	//Login.announceScanCompleted(list);
	tmrHalfSecond.initializeUs(ONE_SECOND, setupAPMode).start(false);
}

void probeRcved(int16_t rssi, uint8_t* mac)
{
	//LOG_I("probe %d", rssi);
}

