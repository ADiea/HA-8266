#include "appMain.h"

CLogin Login;

// Will be called when WiFi station network scan was completed
void listWiFiNetworks(bool succeeded, BssList list)
{
	if (!succeeded)
	{
		LOG_E("Failed to scan networks");
		return;
	}

	for (int i = 0; i < list.count(); i++)
	{
		LOG_I("\tWiFi: %s, %s, %s", list[i].ssid, list[i].getAuthorizationMethodName(),
				list[i].hidden ? "(bcast)":"(hidden)");
	}

	Login.announceScanCompleted(list);
}

eLoginError CLogin::startWiFiScan(CAbstractPeer* requester)
{
	if(bWiFiScanInProgress)
	{
		return eLoginBusy;
	}
	lastScanRequester = requester;
	return WifiStation.startScan(listWiFiNetworks)?eLoginOK:eLoginFailed;
}

void CLogin::announceScanCompleted(BssList& list)
{
	if(bWiFiScanInProgress && lastScanRequester)
	{
		sendWiFiAPListToPeer(lastScanRequester, list);
	}
	bWiFiScanInProgress = false;
}
