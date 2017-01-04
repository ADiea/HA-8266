#include "heater.h"

volatile uint16_t g_heaterLastGasReading = 0;

uint16_t g_heaterLowGasThresh = 1;

uint16_t g_heaterMedGasThresh = 1;

uint16_t g_heaterHighGasThresh = 1;

uint8_t g_heaterStatus = HEATER_STATUS_OFF;

uint8_t g_heaterFault = HEATER_FAULT_NONE;

uint8_t g_heaterPkgSequence = 0;

uint32_t g_lastHeaterDataTimestamp = 0;
uint8_t g_numFailedRetries = 0;

uint32_t g_lastHeaterReadGasLevel = 0;

#define GAS_SENSOR_SIZE 4
uint16_t g_gasBuffer[GAS_SENSOR_SIZE] = {0};
uint8_t g_gasBufferIdx = 0;
uint8_t g_gasSensorFilledBuff=0;

uint16_t feedGasBuffer(uint16_t sample)
{
	uint32_t acc=0;
	uint8_t i;
	
	if(!g_gasSensorFilledBuff && g_gasBufferIdx == GAS_SENSOR_SIZE - 1)
	{
		g_gasSensorFilledBuff = 1;
	}
	
	g_gasBuffer[(g_gasBufferIdx++)%GAS_SENSOR_SIZE] = sample;
	
	if(g_gasSensorFilledBuff)
	{
		for(i=0;i<GAS_SENSOR_SIZE;i++)
		{
			acc += g_gasBuffer[i];
		}
		return acc / GAS_SENSOR_SIZE;
	}
	else return sample;	
}

void (*app_start)(void) = 0x0000;

void sendHeaterStatusPkg(uint8_t seq);
void sendHeaterRequestPkg(uint8_t seq);

void heater_init(void)
{
	//heater relay output
	DDRB |= 1<<2;
	 //turn heater off
	PORTB &= ~(1<<2);
	
	//MQ9 snsor heat
	DDRB |= 1<<1;
	 //turn snsor on
	PORTB |= (1<<1);
	
	//ADC
	ADMUX = 0;//channel 0
	ADCSRA = 1<<ADEN | 1<<ADSC | 1<<ADPS2 | 1<<ADPS1; //prescaler 64=> 125KHz
	DIDR0 = 1<<ADC0D; //disable digital logic for pin 0
}

void heater_loop()
{
	//read ADC / feed moving average filter etc
	// => g_heaterLastGasReading

	uint16_t adcValue;
	
	if(millis() - g_lastHeaterDataTimestamp > 1000)
	{
		g_lastHeaterDataTimestamp = millis();
		g_numFailedRetries++;
		
		if(g_numFailedRetries > 75)
		{
			debugf("Radio LOST;RESET\n");
			app_start();
		}
		
		if(g_numFailedRetries > 30)
		{
			if(g_numFailedRetries % 3 == 0)
			{
				debugf("Radio LOST:%d\n", (g_numFailedRetries-30)/3);
				sendHeaterRequestPkg(g_heaterPkgSequence++);
			}
			g_LedStateInterval = 200;
		}
	}
	
	if(millis() - g_lastHeaterReadGasLevel > 250)
	{
		g_lastHeaterReadGasLevel = millis();
		
		ADCSRA |= 1<<ADSC;
		
		while(ADCSRA & 1<<ADSC);

		adcValue = ADCL;
		adcValue |= ((uint16_t)ADCH)<<8;
		
		g_heaterLastGasReading = feedGasBuffer(adcValue);
	
		debugf("ADC adc:%d mean:%d\n", adcValue, g_heaterLastGasReading);
		
		if(g_heaterLastGasReading >= g_heaterMedGasThresh && 
			(g_heaterStatus & HEATER_STATUS_ON))
		{
			g_heaterStatus = HEATER_STATUS_FAULT | HEATER_STATUS_OFF;
			g_heaterFault = HEATER_FAULT_GAS_HIGH;
			PORTB &= ~(1<<2); //turn heater off // todo: maintain heater state, ignore changes
			
			sendHeaterStatusPkg(g_heaterPkgSequence++);
		}
	}
}

void sendHeaterRequestPkg(uint8_t seq)
{
	uint8_t pkg[PKG_HEATER_REQUEST_LEN];
	
	debugf("RQ HEAT");
	
	pkg[0] = GATEWAY_ID;
	pkg[1] = MY_ID;
	pkg[2] = PKG_TYPE_HEATER_REQUEST;		
	pkg[3] = 0;
	pkg[0x4] = seq;
	
	pkg[0x5] = 0;
	
	for(seq=0; seq<PKG_HEATER_REQUEST_LEN - 1; seq++)
		pkg[0x5] += pkg[seq];

	if(!radio_sendPacketSimple(PKG_HEATER_REQUEST_LEN, pkg))
	{
		debugf("ER\n");
	}
	else
	{
		debugf("OK\n");
	}
	
	radio_startListening();
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
	
	pkg[5] = 0xFF & g_heaterLastGasReading;
	pkg[6] = g_heaterLastGasReading >> 8;
	
	pkg[7] = 0xFF & g_heaterLowGasThresh;
	pkg[8] = g_heaterLowGasThresh >> 8;
	
	pkg[9] = 0xFF & g_heaterMedGasThresh;
	pkg[0xa] = g_heaterMedGasThresh >> 8;
	
	pkg[0xb] = 0xFF & g_heaterHighGasThresh;
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
	uint8_t err = 0;
	do
	{
		if(len != PKG_HEATER_LEN)
		{
			err = 1;
			break;
		}

		if(pkg[0] != MY_ID)
		{
			err = 2;
			break;
		}
			
		if(pkg[2] != PKG_TYPE_HEATER)
		{
			err = 3;
			break;
		}
		
		if(((pkg[0] + pkg[1] + pkg[2] + pkg[3] + pkg[4] + 
			 pkg[5] + pkg[6] + pkg[7] + pkg[8] + pkg[9] + 
			 pkg[0xA]  ) & 0xFF ) != pkg[0xB])
		{
			err = 4;		
			break;
		}

		seq = pkg[0xa];
		
		g_heaterStatus &= ~(HEATER_STATUS_ON  | HEATER_STATUS_OFF | HEATER_STATUS_FAULT);
		
		g_heaterLowGasThresh = ((uint16_t)pkg[5] << 8) | pkg[4];
		g_heaterMedGasThresh = ((uint16_t)pkg[7] << 8) | pkg[6];
		g_heaterHighGasThresh = ((uint16_t)pkg[9] << 8) | pkg[8];
		
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
	
	if(err != 0)
	{
		debugf("RX HEAT ERR: %u\r\n", err);
	}
	else
	{
		g_lastHeaterDataTimestamp = millis();
		g_numFailedRetries = 0;
		g_LedStateInterval = 1000;
	}
}

