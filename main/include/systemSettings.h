#ifndef SYSTEM_SETTINGS_H
#define SYSTEM_SETTINGS_H

#include <user_config.h>

//TODO move to firmwareUpdate.h
#define FIRMWARE_VER_MAJOR 1
#define FIRMWARE_VER_MINOR 0


#define SYS_SETTINGS_FILE "sysCfg"


struct SystemSettings
{
	SystemSettings():fwVerMinor(0), fwVerMajor(0), wifiStationIsConfigured(0),
			wifiSSID(NULL), wifiPwd(NULL), dbgNumRestarts(0),
			lastKnownTimeStamp(0)
	{}
	uint32_t fwVerMinor;
	uint32_t fwVerMajor;
	uint8_t wifiStationIsConfigured;
	char* wifiSSID; //max size 32
	char* wifiPwd; //max size 64
	//TODO backup wifi ssid/pwd
	uint32_t dbgNumRestarts;
	uint32_t lastKnownTimeStamp;
	int8_t timeZone;
};

extern SystemSettings gSysCfg;

// Restores settings to default. Prevents accidental calling
//usage: First call with no parameter, then call with returned code
//       resetSystemSettings(resetSystemSettings());
uint32_t resetSystemSettings(uint32_t code = 0);

bool loadSystemSettings();
bool saveSystemSettings();

#endif /*SYSTEM_SETTINGS_H*/
