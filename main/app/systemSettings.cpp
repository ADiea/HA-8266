#include "appMain.h"

/*
struct SystemSettings
{
	SystemSettings():fwVerMinor(0), fwVerMajor(0), wifiStationIsConfigured(0),
			wifiSSID(NULL), wifiPwd(NULL), dbgNumRestarts(0),
			lastKnownTimeStamp(0)
	{}
	uint16_t fwVerMinor;
	uint16_t fwVerMajor;
	uint8_t wifiStationIsConfigured;
	char* wifiSSID; //max size 32
	char* wifiPwd; //max size 64
	//TODO backup wifi ssid/pwd
	uint32_t dbgNumRestarts;
	uint32_t lastKnownTimeStamp;
};
*/

SystemSettings gSysCfg;

// Restores settings to default. Prevents accidental calling
//usage: First call with no parameter, then call with returned code
//       resetSystemSettings(resetSystemSettings());
uint32_t resetSystemSettings(uint32_t code/* = 0*/)
{
	static uint32_t challenge=0;
	const char* naStr = "na";

	if(code == 0)
	{
		challenge = phy_get_rand();
	}
	else if(code == challenge)
	{
		if(FIRMWARE_VER_MAJOR == 1)
		{
			if(FIRMWARE_VER_MINOR == 0) /* for 01.00 */
			{
				gSysCfg.fwVerMinor = FIRMWARE_VER_MINOR;
				gSysCfg.fwVerMajor = FIRMWARE_VER_MAJOR;
				gSysCfg.wifiStationIsConfigured = 0;

				if(gSysCfg.wifiSSID)
					free(gSysCfg.wifiSSID);
				gSysCfg.wifiSSID = new char[sizeof(naStr)+1];
				memcpy(gSysCfg.wifiSSID, naStr, sizeof(naStr)+1);

				if(gSysCfg.wifiPwd)
					free(gSysCfg.wifiPwd);
				gSysCfg.wifiPwd = new char[sizeof(naStr)+1];
				memcpy(gSysCfg.wifiPwd, naStr, sizeof(naStr)+1);

				gSysCfg.dbgNumRestarts = 0;
				gSysCfg.lastKnownTimeStamp = 0;
				gSysCfg.timeZone = 2;
			}
		}
	}
	return challenge;
}

bool loadSystemSettings()
{
	bool bRet = false;
	char *sysCfgStr = NULL, *saveSysCfgStr;

	int iValue;
	uint32_t uValue;
	char sBuffer[128];

	if(readFileFull(SYS_SETTINGS_FILE, &sysCfgStr))
	{
		saveSysCfgStr = sysCfgStr;
		if(FIRMWARE_VER_MAJOR == 1)
		{
			if(FIRMWARE_VER_MINOR == 0) /* for 01.00 */
			{
				do
				{
					if(!skipUint((const char**)&sysCfgStr, &gSysCfg.fwVerMinor ))
						break;

					if(!skipUint((const char**)&sysCfgStr, &gSysCfg.fwVerMajor))
						break;

					if(!skipInt((const char**)&sysCfgStr, (int*)&gSysCfg.wifiStationIsConfigured))
						break;
					gSysCfg.wifiStationIsConfigured = iValue;

					if(!skipString((const char**)&sysCfgStr, sBuffer, sizeof(sBuffer)))
						break;
					iValue = strlen(sBuffer);
					if(iValue < sizeof(sBuffer))
					{
						if(gSysCfg.wifiSSID)
							delete gSysCfg.wifiSSID;
						gSysCfg.wifiSSID = new char[iValue];
						memcpy(gSysCfg.wifiSSID, sBuffer, iValue);
						gSysCfg.wifiSSID[iValue - 1] = 0;
					}

					if(!skipString((const char**)&sysCfgStr, sBuffer, sizeof(sBuffer)))
						break;
					iValue = strlen(sBuffer);
					if(iValue < sizeof(sBuffer))
					{
						if(gSysCfg.wifiPwd)
							delete gSysCfg.wifiPwd;
						gSysCfg.wifiPwd = new char[iValue];
						memcpy(gSysCfg.wifiPwd, sBuffer, iValue);
						gSysCfg.wifiPwd[iValue - 1] = 0;
					}

					if(!skipUint((const char**)&sysCfgStr, &gSysCfg.dbgNumRestarts))
						break;

					if(!skipUint((const char**)&sysCfgStr, &gSysCfg.lastKnownTimeStamp))
						break;

					if(!skipInt((const char**)&sysCfgStr, (int*)&gSysCfg.timeZone))
						break;

					bRet = true;
				}while(0);
			}
		}
		if(saveSysCfgStr)
			delete saveSysCfgStr;
	}
	return bRet;
}
bool saveSystemSettings()
{
	bool bRet = false;
	uint32_t actual, size;

	WDT.alive();

	do
	{
		if(FIRMWARE_VER_MAJOR == 1)
		{
			if(FIRMWARE_VER_MINOR == 0) /* for 01.00 */
			{
				size = m_snprintf(g_devScrapBuffer, MAXDEVSZ, "%u;%u;%u;%s;%s;%u;%u;%d;",
						gSysCfg.fwVerMinor, gSysCfg.fwVerMajor, gSysCfg.wifiStationIsConfigured,
						gSysCfg.wifiSSID, gSysCfg.wifiPwd, gSysCfg.dbgNumRestarts,
						gSysCfg.lastKnownTimeStamp, gSysCfg.timeZone);

			}
		}

		if(size == MAXDEVSZ)
		{
			LOG_E("settings too large");

			break;
		}

		actual = writeFileFull(SYS_SETTINGS_FILE, g_devScrapBuffer, size);

		if (actual == size)
		{
			LOG_E(" SAVED");
			bRet = true;
		}
	}
	while(0);

	return bRet;
}
