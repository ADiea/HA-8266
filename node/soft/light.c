#include "light.h"

#include<avr/eeprom.h>



typedef struct __attribute__((packed))_eepromData
{
	uint8_t dimMax, dimMin, speedDim, dataChecksum;
	unsigned long onDuration;
} t_eepromData;

t_eepromData EEMEM g_eepromData;
t_eepromData g_LightData;

uint8_t g_dimFactor, g_dimTarget, g_lightState;

uint8_t g_movementPkg[PKG_MOVEMENT_LEN];

unsigned long lastOnTime;

void light_init(void)
{
	eeprom_read_block((void*)&g_LightData, (const void*)&g_eepromData, sizeof(t_eepromData));
	
	uint8_t sum = g_LightData.dimMax + g_LightData.dimMin +
					g_LightData.speedDim + g_LightData.onDuration;
	if((sum & 0xFF) != g_LightData.dataChecksum)
	{
		debugf("Light: Inv chksum %u != %u\n", sum, g_LightData.dataChecksum);
		
		g_LightData.dimMax = 255;
		g_LightData.dimMin = 50;
		g_LightData.speedDim = 5;
		g_LightData.onDuration = 30 * 1000;	
	}

	debugf("Light: max:%u min:%u speed:%u durH:%x durL:%x\n", 
			g_LightData.dimMax,
			g_LightData.dimMin,
			g_LightData.speedDim,
			(unsigned int)(g_LightData.onDuration >> 16),
			(unsigned int)g_LightData.onDuration);
			
	g_dimFactor = g_LightData.dimMin;
	g_dimTarget = g_LightData.dimMax;
	g_lightState = LIGHT_STATE_ON;
	lastOnTime = millis();
	
	g_movementPkg[0] = GATEWAY_ID;
	g_movementPkg[1] = MY_ID;
	g_movementPkg[2] = PKG_TYPE_MOVEMENT;
	g_movementPkg[3] = MOVEMENT_OFF;
}

void light_loop(void)
{
	uint8_t len = g_LightData.speedDim;

	do
	{
		if(g_dimFactor < g_dimTarget)
		{
			++g_dimFactor;
		}
		else if(g_dimFactor > g_dimTarget)
		{
			--g_dimFactor;
		}
		
		if(g_dimFactor < g_LightData.dimMin)
		{
			if(g_lightState == LIGHT_STATE_OFF)
				g_dimFactor = 0;
			else
				g_dimFactor = g_LightData.dimMin;
		}
	}
	while(--len);
	
	relay_setDim(g_dimFactor);
	
	if(PINC & 1<<2)
	{
		if(g_movementPkg[3] != MOVEMENT_ON)
		{
			debugf("MOVEMENT ON\n");
			g_movementPkg[3] = MOVEMENT_ON;
			g_movementPkg[4] = (CHECKSUM_MOVEMENT_ON(MY_ID) + g_movementPkg[1]) & 0xFF;
			radio_sendPacketSimple(PKG_MOVEMENT_LEN, g_movementPkg);
			radio_startListening();
		}
		
		lastOnTime = millis();
		if(g_lightState == LIGHT_STATE_OFF)
		{
			debugf("LIGHT_ON %u -> %u\n", g_dimTarget, g_LightData.dimMax);
			g_lightState = LIGHT_STATE_ON;
			g_dimTarget = g_LightData.dimMax;
		}
	}
	else
	{
		if(g_movementPkg[3] != MOVEMENT_OFF)
		{
			debugf("MOVEMENT OFF\n");
			g_movementPkg[3] = MOVEMENT_OFF;
			g_movementPkg[4] = (CHECKSUM_MOVEMENT_OFF(MY_ID) + g_movementPkg[1]) & 0xFF;
			radio_sendPacketSimple(PKG_MOVEMENT_LEN, g_movementPkg);
			radio_startListening();
		}
	
		if(g_lightState == LIGHT_STATE_ON && (millis() - lastOnTime > g_LightData.onDuration))
		{
			debugf("LIGHT_OFF %u -> 0\n", g_dimTarget);
			g_lightState = LIGHT_STATE_OFF;
			g_dimTarget = 0;
		}
	}
}


void light_processPkg(uint8_t* pkg, uint8_t len)
{
	do
	{

		if(len != PKG_INTENSITY_LEN)
			break;

		if(pkg[0] != MY_ID)
			break;
			
		if(pkg[1] != PKG_TYPE_INTENSITY)
			break;
		
		if(((pkg[0] + pkg[1] + pkg[2] + pkg[3] + pkg[4]  + pkg[5]+ pkg[6]) & 0xFF ) != pkg[7])
			break;
		
		g_LightData.dimMax = pkg[2];
		
		g_LightData.onDuration = (((unsigned long)(pkg[3] >> 4) * 60) + ((pkg[3] & 0xF) << 2)) * 1000;
		
		g_LightData.speedDim = pkg[4] & 0xF;
		
		if(pkg[4] & PKG_MANUAL_FLAG)
		{
			g_lightState = LIGHT_STATE_MANUAL;
		}
		else
		{
			g_lightState = LIGHT_STATE_ON;
		}
		
		g_dimTarget = g_LightData.dimMax;
		
		g_LightData.dimMin = pkg[5];
		
		if(g_lightState != LIGHT_STATE_MANUAL)
		{
			g_LightData.dataChecksum = g_LightData.dimMax + g_LightData.dimMin +
					g_LightData.speedDim + g_LightData.onDuration;
					
			eeprom_write_block((const void*)&g_LightData, (void*)&g_eepromData, sizeof(t_eepromData));
		}

		debugf("INTY: max%u dH%u dL:%u sp%u st%u min%u\n", 
			g_LightData.dimMax, (unsigned int)(g_LightData.onDuration >> 16), (unsigned int)g_LightData.onDuration, 
			g_LightData.speedDim, g_lightState, g_LightData.dimMin);
		
		debugf("RPL ");
		
		pkg[0] = GATEWAY_ID;
		pkg[1] = MY_ID;
		pkg[2] = PKG_TYPE_ACK;		
		pkg[3] = pkg[6]; //sequence
		pkg[4] = pkg[0] + pkg[1] + pkg[2] + pkg[3];

		if(!radio_sendPacketSimple(5, pkg))
		{
			debugf("ER\n");
		}
		else
		{
			debugf("OK\n");
		}
	
	} while(0);
}
