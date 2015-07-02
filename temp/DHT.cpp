/*
 * Name: DHT library
 * License: MIT license
 *
 * 7/02/15 ADiea:	dew point algorithms
 * 6/25/15 ADiea: 	pullup option
 *  	 	 	 	heat index functions
 *  	 	 	 	read temp and humidity in one function call
 *  	 	 	 	cache converted value for last temp and humid
 *
 * -/-/-	written by Adafruit Industries
 */

#include "DHT.h"

void DHT::begin(void)
{
	// set up the pins!
	pinMode(m_kSensorPin, INPUT);
	if (m_bPllupEnabled)
	{
		pullup(m_kSensorPin);
	}
	else
	{
		digitalWrite(m_kSensorPin, HIGH);
	}
}



float DHT::readTemperature(bool bFarenheit/* = false*/)
{
	float f = NAN;
	readTempAndHumidity(&f, NULL, bFarenheit);
	return f;
}

float DHT::convertCtoF(float c)
{
	return c * 1.8f + 32;
}

float DHT::convertFtoC(float f)
{
	return (f-32)*0.555555f;
}

float DHT::readHumidity(void)
{
	float f = NAN;
	readTempAndHumidity(NULL, &f);
	return f;
}

bool DHT::readTempAndHumidity(float* temp, float* humid, bool bFarenheit/* = false*/)
{
	bool bSuccess = false;

	if (read())
	{
		bSuccess = true;
		if(temp)
		{
			*temp = m_lastTemp;
			if (bFarenheit)
				*temp = convertCtoF(*temp);
		}
		if(humid)
		{
			*humid = m_lastHumid;
		}
	}
	return bSuccess;
}

float DHT::computeHeatIndexF(float tempFahrenheit, float percentHumidity)
{
// Adapted from equation at: https://github.com/adafruit/DHT-sensor-library/issues/9 and
// Wikipedia: http://en.wikipedia.org/wiki/Heat_index
	float t2F = tempFahrenheit * tempFahrenheit;
	float h2 = percentHumidity * percentHumidity;

	return -42.379 + 2.04901523 * tempFahrenheit + 10.14333127 * percentHumidity
			+ -0.22475541 * tempFahrenheit * percentHumidity + -0.00683783 * t2F
			+ -0.05481717 * h2 + 0.00122874 * t2F * percentHumidity
			+ 0.00085282 * tempFahrenheit * h2 + -0.00000199 * t2F * h2;
}

float DHT::computeHeatIndexC(float tempCelsius, float percentHumidity)
{
// Adapted from equation at: https://github.com/adafruit/DHT-sensor-library/issues/9 and
// Wikipedia: http://en.wikipedia.org/wiki/Heat_index
	float t2C = tempCelsius * tempCelsius;
	float h2 = percentHumidity * percentHumidity;

	return -8.784695 + 1.61139411 * tempCelsius + 2.33854900 * percentHumidity
			+ -0.14611605 * tempCelsius * percentHumidity + -0.01230809 * t2C
			+ -0.01642482 * h2 + 0.00221173 * t2C * percentHumidity
			+ 0.00072546 * tempCelsius * h2 + -0.00000358 * t2C * h2;
}

double DHT::computeDewPoint(float tempCelsius, float percentHumidity, uint8_t algType /*= DEW_ACCURATE*/)
{
	double result = NAN;
	percentHumidity = percentHumidity * 0.01;

	switch(algType)
	{
		case DEW_ACCURATE: /* 6.040ms @ 80Mhz Accuracy +0.00 */
		{
			/* 01/JUL/2015 ADiea: ported from FORTRAN http://wahiduddin.net/calc/density_algorithms.htm */
			/*
			FUNCTION ESGG(T)
			Baker, Schlatter  17-MAY-1982     Original version.
			THIS FUNCTION RETURNS THE SATURATION VAPOR PRESSURE OVER LIQUID
			WATER ESGG (MILLIBARS) GIVEN THE TEMPERATURE T (CELSIUS). THE
			FORMULA USED, DUE TO GOFF AND GRATCH, APPEARS ON P. 350 OF THE
			SMITHSONIAN METEOROLOGICAL TABLES, SIXTH REVISED EDITION, 1963,
			BY ROLAND LIST.
			*/
			double CTA = 273.15,  // DIFFERENCE BETWEEN KELVIN AND CELSIUS TEMPERATURES
				   EWS = 3.00571489795, // log10 of SATURATION VAPOR PRESSURE (MB) OVER LIQUID WATER AT 100C
				   TS = 373.15; // BOILING POINT OF WATER (K)

			double  C1 = -7.90298, C2 = 5.02808, C3 = 1.3816E-7, C4 = 11.344, C5 = 8.1328E-3,  C6 = -3.49149;
			tempCelsius = tempCelsius + CTA;
			result = (TS / tempCelsius) - 1;

			//   GOFF-GRATCH FORMULA

			result = pow(10, (C1 * result + C2 * log10(result + 1) -
						  C3 * (pow(10, (C4 * (1. - tempCelsius / TS))) - 1.) +
						  C5 * (pow(10, (C6 * result)) - 1.) + EWS));
			if(result < 0)
				result = 0;
			//ESGG now holds the saturation vapor pressure https://en.wikipedia.org/wiki/Vapor_pressure
			//debugf("Sat Water Vapor(mBar)=");
			//Serial.print(result);

			//Convert from mBar to kPa (1mBar = 0.1 kPa) and divide by 0.61078 constant
			//Determine vapor pressure (takes the RH into account)
			result = percentHumidity * result / (10 * 0.61078);
			//debugf("Water Vapor(K)=", result);
			//Serial.print(result);

			result = log(result);
			//http://www.colorado.edu/geography/weather_station/Geog_site/about.htm
			result =(241.88 * result) / (17.558 - result);
		}
		break;

		case DEW_ACCURATE_FAST: /* 5.725ms @ 80Mhz Accuracy +0.01 */
		{
			/*Saturation vapor pressure is calculated by the datalogger
			 * with the following approximating polynomial
			 * (see Lowe, P.R. 1930. J. Appl. Meteor., 16:100-103):
			 * http://www.colorado.edu/geography/weather_station/Geog_site/about.htm
			 */
			result = 6.107799961 +
						  tempCelsius * (0.4436518521 +
						  tempCelsius * (0.01428945805 +
						  tempCelsius * (2.650648471e-4 +
						  tempCelsius * (3.031240396e-6 +
						  tempCelsius * (2.034080948e-8 +
						  tempCelsius * 6.136820929e-11)))));
	        //debugf("Sat Water Vapor(kPa)=");
	        //Serial.print(result);
	        //Convert from mBar to kPa (1mBar = 0.1 kPa) and divide by 0.61078 constant
	        //Determine vapor pressure (takes the RH into account)
	        result = percentHumidity * result / (10 * 0.61078);
			//debugf("Water Vapor(K)=");
			//Serial.print(result);
			result = log(result);
			result = (241.88 * result) / (17.558 - result);
		}
		break;

		case DEW_FAST: /* 1.479 ms Accuracy -0.04 */
		{
			/* http://en.wikipedia.org/wiki/Dew_point */
			double a = 17.271;
			double b = 237.7;
			result = (a * tempCelsius) / (b + tempCelsius) + log(percentHumidity);
			result = (b * result) / (a - result);
		}
		break;

		case DEW_FASTEST: /* 1.415 ms @ 80Mhz Accuracy +0.10 */
		{
		/* 01/JUL/2015 ADiea: ported from FORTRAN http://wahiduddin.net/calc/density_algorithms.htm */
		/*
			Baker, Schlatter  17-MAY-1982     Original version.
			THIS FUNCTION RETURNS THE DEW POINT (CELSIUS) GIVEN THE TEMPERATURE
			(CELSIUS) AND RELATIVE HUMIDITY (%). THE FORMULA IS USED IN THE
			PROCESSING OF U.S. RAWINSONDE DATA AND IS REFERENCED IN PARRY, H.
			DEAN, 1969: "THE SEMIAUTOMATIC COMPUTATION OF RAWINSONDES,"
			TECHNICAL MEMORANDUM WBTM EDL 10, U.S. DEPARTMENT OF COMMERCE,
			ENVIRONMENTAL SCIENCE SERVICES ADMINISTRATION, WEATHER BUREAU,
			OFFICE OF SYSTEMS DEVELOPMENT, EQUIPMENT DEVELOPMENT LABORATORY,
			SILVER SPRING, MD (OCTOBER), PAGE 9 AND PAGE II-4, LINE 460.
		*/
			result = 1. - percentHumidity;

		/*  COMPUTE DEW POINT DEPRESSION. */
			result = (14.55 + 0.114 * tempCelsius)*result +
						 pow((2.5 + 0.007 * tempCelsius)*result, 3) +
						 (15.9 + 0.117 * tempCelsius)*pow(result, 14);

			result = tempCelsius - result;
		}
		break;
	};

	return result;
}
void DHT::updateInternalCache()
{
	float f;
	/*Compute and write temp and humid to internal cache*/
	switch (m_kSensorType)
	{
		case DHT11:
			m_lastTemp = m_data[2];
			m_lastHumid = m_data[0];
			break;
		case DHT22:
		case DHT21:
			/*Temp*/
			f = m_data[2] & 0x7F;
			f *= 256;
			f += m_data[3];
			f /= 10;
			if (m_data[2] & 0x80)
				f *= -1;
			m_lastTemp = f;

			/*Humidity*/
			f = m_data[0];
			f *= 256;
			f += m_data[1];
			f /= 10;
			m_lastHumid = f;
			break;
		default:
			debugf("DHT Sensor type not implemented");
			break;
	}
}

boolean DHT::read(void)
{
	uint8_t laststate = HIGH;
	uint8_t counter = 0;
	uint8_t j = 0, i;
	unsigned long currenttime;

	// pull the pin high and wait 250 milliseconds
	pinMode(m_kSensorPin, OUTPUT);
	digitalWrite(m_kSensorPin, HIGH);

	if(m_firstRead)
		delay(250);

	currenttime = millis();

	/*subtraction of 2 unsigned values yields correct result no matter if there was one overflow*/
	if ((currenttime - m_lastreadtime) < m_maxIntervalRead && !m_firstRead)
	{
		return true; // return last correct measurement
	}

	m_firstRead = false;

	/*
	 Serial.print("Currtime: "); Serial.print(currenttime);
	 Serial.print(" Lasttime: "); Serial.print(m_lastreadtime);
	 */
	m_lastreadtime = millis();

	m_data[0] = m_data[1] = m_data[2] = m_data[3] = m_data[4] = 0;

	// now pull it low for ~20 milliseconds

	digitalWrite(m_kSensorPin, LOW);
	delayMicroseconds(20000);
	cli();
	// make pin input and activate pullup
	pinMode(m_kSensorPin, INPUT);
	digitalWrite(m_kSensorPin, HIGH);
	if (m_bPllupEnabled)
	{
		pullup(m_kSensorPin);
	}
	else
	{
		digitalWrite(m_kSensorPin, HIGH);
	}

	delayMicroseconds(10);

	// read in timings
	for (i = 0; i < MAXTIMINGS || j >= 40; i++)
	{
		counter = 0;
		while (digitalRead(m_kSensorPin) == laststate)
		{
			counter++;
			delayMicroseconds(1);
			if (counter == 255)
			{
				break;
			}
		}
		laststate = digitalRead(m_kSensorPin);

		if (counter == 255)
			break;

		// ignore first 3 transitions
		if ((i >= 4) && (i % 2 == 0))
		{
			//Serial.print(i, DEC);

			// shove each bit into the storage bytes
			m_data[j / 8] <<= 1;

			if (counter > m_kThreshOne)
			{
				//Serial.print("#");
				m_data[j / 8] |= 1;
			}
			//Serial.print(" ");
			j++;
		}
	}

	sei();

/*
	 Serial.println(j, DEC);
	 Serial.print(m_data[0], HEX); Serial.print(", ");
	 Serial.print(m_data[1], HEX); Serial.print(", ");
	 Serial.print(m_data[2], HEX); Serial.print(", ");
	 Serial.print(m_data[3], HEX); Serial.print(", ");
	 Serial.print(m_data[4], HEX); Serial.print(" =? ");
	 Serial.println(m_data[0] + m_data[1] + m_data[2] + m_data[3], HEX);
*/

	// pull the pin high at the end(will stay high at least 250ms(actually >2s) for the next reading)
	pinMode(m_kSensorPin, OUTPUT);
	digitalWrite(m_kSensorPin, HIGH);

	// check we read 40 bits and that the checksum matches
	if ((j >= 40)
			&& (m_data[4] == ((m_data[0] + m_data[1] + m_data[2] + m_data[3]) & 0xFF)))
	{
		updateInternalCache();
		return true;
	}

	return false;

}
