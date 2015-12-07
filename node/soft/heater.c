#include "heater.h"

uint16_t g_heaterLastGasReading = 0;

uint16_t g_heaterLowGasThresh = 0;

uint16_t g_heaterMedGasThresh = 0;

uint16_t g_heaterHighGasThresh = 0;

uint8_t g_heaterStatus = HEATER_STATUS_OFF;

uint8_t g_heaterFault = HEATER_FAULT_NONE;

uint8_t g_heaterPkgSequence = 0;

uint8_t g_heaterWarningLoopCounter=0;

void sendHeaterStatusPkg(uint8_t seq);

void heater_init(void)
{
	//heater relay output
	DDRB |= 1<<2;
	PORTB &= ~(1<<2);
}

void heater_loop()
{
	//read ADC / feed moving average filter etc
	// => g_heaterLastGasReading

	if((g_heaterLastGasReading >= g_heaterMedGasThresh) && 
		((g_heaterStatus & HEATER_STATUS_ON) || !(g_heaterWarningLoopCounter++)))
	{
		g_heaterStatus = HEATER_STATUS_FAULT | HEATER_STATUS_OFF;
		g_heaterFault = HEATER_FAULT_GAS_HIGH;
		PORTB &= ~(1<<2); //turn heater off
		
		sendHeaterStatusPkg(g_heaterPkgSequence++);
	}

}

void sendHeaterStatusPkg(uint8_t seq)
{
	uint8_t pkg[PKG_HEATER_STATUS_LEN];
	
	debugf("TX HEAT");
	
	pkg[0] = GATEWAY_ID;
	pkg[1] = MY_ID;
	pkg[2] = PKG_TYPE_HEATER_STATUS;		
	pkg[3] = g_heaterStatus;
	pkg[4] = g_heaterFault;
	
	pkg[5] = 0xFF && g_heaterLastGasReading;
	pkg[6] = g_heaterLastGasReading >> 8;
	
	pkg[7] = 0xFF && g_heaterLowGasThresh;
	pkg[8] = g_heaterLowGasThresh >> 8;
	
	pkg[9] = 0xFF && g_heaterMedGasThresh;
	pkg[0xa] = g_heaterMedGasThresh >> 8;
	
	pkg[0xb] = 0xFF && g_heaterHighGasThresh;
	pkg[0xc] = g_heaterHighGasThresh >> 8;
	
	pkg[0xd] = seq;
	
	pkg[0xe] = 0;
	
	for(seq=0; seq<PKG_HEATER_STATUS_LEN - 1; seq++)
		pkg[0xe] += pkg[seq];


	if(!radio_sendPacketSimple(PKG_HEATER_STATUS_LEN, pkg))
	{
		debugf("ER\n");
	}
	else
	{
		debugf("OK\n");
	}
	
	radio_startListening();
}

void heater_processPkg(uint8_t* pkg, uint8_t len)
{
	uint8_t seq;
	do
	{
		if(len != PKG_HEATER_LEN)
			break;

		if(pkg[0] != MY_ID)
			break;
			
		if(pkg[1] != PKG_TYPE_HEATER)
			break;
		
		if(((pkg[0] + pkg[1] + pkg[2] + pkg[3] + pkg[4] + 
			 pkg[5] + pkg[6] + pkg[7] + pkg[8] + pkg[9] + 
			 pkg[0xA]  ) & 0xFF ) != pkg[0xB])
			break;

		seq = pkg[0xa];
		
		g_heaterStatus &= ~(HEATER_STATUS_ON  | HEATER_STATUS_OFF | HEATER_STATUS_FAULT);
		
		g_heaterLowGasThresh = ((uint16_t)pkg[5]) << 8 | pkg[4];
		g_heaterMedGasThresh = ((uint16_t)pkg[7]) << 8 | pkg[6];
		g_heaterHighGasThresh = ((uint16_t)pkg[9]) << 8 | pkg[8];
		
		debugf("RX HEAT: %u low:%u med:%u hi:%d\n", 
				pkg[3], g_heaterLowGasThresh, g_heaterMedGasThresh, g_heaterHighGasThresh);
		
		g_heaterFault = HEATER_FAULT_NONE;
		
		if(g_heaterLastGasReading >= g_heaterMedGasThresh)
		{
			g_heaterStatus |= HEATER_STATUS_FAULT;
			g_heaterFault = HEATER_FAULT_GAS_HIGH;
		}
		
		if(pkg[3] & HEATER_REQ_ON)
		{
			if(!(g_heaterStatus & HEATER_STATUS_FAULT))
			{
				PORTB |= (1<<2); //turn heater on
				g_heaterStatus |= HEATER_STATUS_ON;
				
			}
		}
		else
		{
			PORTB &= ~(1<<2); //turn heater off
			g_heaterStatus |= HEATER_STATUS_OFF;			
		}

		sendHeaterStatusPkg(seq);

	} while(0);
}

