#include "CDeviceLight.h"

bool CDeviceLight::deserialize(const char *devicesString)
{
	int devID;
	#define MAX_FRIENDLY_NAME 64
	char friendlyName[MAX_FRIENDLY_NAME];

	tLightState lightState;

	if(!skipInt(&devicesString, &devID))return false;
	if(!skipString(&devicesString, (char*)friendlyName, MAX_FRIENDLY_NAME))return false;

	LOG_I("LIGHT device ID:%d NAME: %s", devID, friendlyName);

	//skipInt minIntensity , ...
	String name(friendlyName);
	initLight((uint32_t)devID, name, lightState);

	return true;
}

uint32_t CDeviceLight::serialize(char* buffer, uint32_t size)
{

}
