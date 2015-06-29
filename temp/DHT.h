#ifndef DHT_H
#define DHT_H
#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

#define ONE_DURATION_THRESH_US 30 //From datasheet: '0' if HIGH lasts 26-28us,
								  //				'1' if HIGH lasts 70us

/*
 * Name: DHT library
 * License: MIT license
 *
 * 6/25/15 ADiea: 	pullup option
 *  	 	 	 	heat index functions
 *  	 	 	 	read temp and humidity in one function call
 *
 * -/-/-	written by Adafruit Industries
 */

// how many timing transitions we need to keep track of. 2 * number bits + extra
#define MAXTIMINGS 85

#define DHT11 11
#define DHT22 22
#define DHT21 21
#define AM2301 21


class DHT
{
public:
	DHT(uint8_t pin, uint8_t type, boolean pullup = false, uint8_t count = ONE_DURATION_THRESH_US)
		: m_kSensorPin(pin), m_kSensorType(type), m_kThreshOne(count),
		  m_bPllupEnabled(pullup), m_firstRead(true), m_lastreadtime(0){};

	void begin(void);

	float readSensor(bool bReadTemperature, bool bFarenheit = false);
	float readTemperature(bool bFarenheit = false);
	float readHumidity(void);

	bool readTempAndHumidity(float* temp, float* humid, bool bFarenheit = false);

	float convertCtoF(float);
	float computeHeatIndexC(float tempCelsius, float percentHumidity); //TODO:test accuracy against computeHeatIndexF
	float computeHeatIndexF(float tempFahrenheit, float percentHumidity);
private:
	boolean read(void);
	void updateInternalCache();

	boolean m_firstRead;
	uint8_t m_kSensorPin, m_kSensorType, m_kThreshOne;
	uint8_t m_data[6];
	unsigned long m_lastreadtime;
	boolean m_bPllupEnabled;

	float m_lastTemp, m_lastHumid;
};
#endif
