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
	m_lastreadtime = 0;
}

float DHT::readSensor(bool bReadTemperature, bool bFarenheit/* = false*/)
{
	float f = NAN;

	if (read())
	{
		switch (m_sensorType)
		{
		case DHT11:
			if(bReadTemperature)
			{
				f = m_data[2];
			}
			else /* Read humidity */
			{
				f = m_data[0];
			}
			break;

		case DHT22:
		case DHT21:
			if(bReadTemperature)
			{
				f = m_data[2] & 0x7F;
				f *= 256;
				f += m_data[3];
				f /= 10;
				if (m_data[2] & 0x80)
					f *= -1;
			}
			else /* Read humidity */
			{
				f = m_data[0];
				f *= 256;
				f += m_data[1];
				f /= 10;
			}
			break;

		default:
			debugf("DHT Sensor type not implemented");
			return f;
		}

		if (bFarenheit)
			f = convertCtoF(f);
	}

	return f;
}

float DHT::readTemperature(bool bFarenheit/* = false*/)
{
	float f = NAN;

	if (read())
	{
		switch (m_sensorType)
		{
		case DHT11:
			f = m_data[2];
			break;

		case DHT22:
		case DHT21:
			f = m_data[2] & 0x7F;
			f *= 256;
			f += m_data[3];
			f /= 10;
			if (m_data[2] & 0x80)
				f *= -1;
			break;

		default:
			debugf("DHT Sensor type not implemented");
			return f;
		}

		if (bFarenheit)
			f = convertCtoF(f);
	}

	return f;
}

float DHT::convertCtoF(float c)
{
	return c * 9 / 5 + 32;
}

float DHT::readHumidity(void)
{
	float f;
	if (read())
	{
		switch (m_sensorType)
		{
		case DHT11:

			return f;
		case DHT22:
		case DHT21:

			return f;
		}
	}
	debugf("DHT Read fail Humid");
	return NAN;
}

bool DHT::readTempAndHumidity(float* temp, float* humid, bool bFarenheit/* = false*/)
{
	bool bSuccess = false;
	float f;
	uint8_t readResult = read();

	if ()
	{
		bSuccess = true;

		switch (m_kSensorType)
		{
		case DHT11:
			if(temp)
			{
				*temp = m_data[2];
				m_lastTemp = m_data[2];
			}
			if(humid)
			{
				*humid = m_data[0];
				m_lastHumid = m_data[0];
			}

			break;

		case DHT22:
		case DHT21:
			if(temp)
			{
				f = m_data[2] & 0x7F;
				f *= 256;
				f += m_data[3];
				f /= 10;
				if (m_data[2] & 0x80)
					f *= -1;
				*temp = f;
				m_lastTemp = f;
			}
			if(humid)
			{
				f = m_data[0];
				f *= 256;
				f += m_data[1];
				f /= 10;
				*humid = f;
				m_lastHumid = f;
			}

			break;

		default:
			debugf("DHT Sensor type not implemented");
			bSuccess = false;
			break;
		}

		if (temp && bFarenheit && bSuccess)
			*temp = convertCtoF(*temp);
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

void updateInternalCache()
{
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

uint8_t DHT::read(void)
{
	uint8_t laststate = HIGH;
	uint8_t counter = 0;
	uint8_t j = 0, i;
	unsigned long currenttime;
	float f;

	// pull the pin high and wait 250 milliseconds
	digitalWrite(m_kSensorPin, HIGH);
	delay(250);

	currenttime = millis();
	if (currenttime < m_lastreadtime)
	{
		// ie there was a rollover
		m_lastreadtime = 0;
	}
	if ((currenttime - m_lastreadtime) < 2500)
	{
		return READ_CACHE; // return last correct measurement
	}

	/*
	 Serial.print("Currtime: "); Serial.print(currenttime);
	 Serial.print(" Lasttime: "); Serial.print(m_lastreadtime);
	 */
	m_lastreadtime = millis();

	m_data[0] = m_data[1] = m_data[2] = m_data[3] = m_data[4] = 0;

	// now pull it low for ~20 milliseconds
	digitalWrite(m_kSensorPin, LOW);
	delayMicroseconds(20000);

	// make pin input and activate pullup
	pinMode(m_kSensorPin, INPUT);

	if (m_bPllupEnabled)
	{
		pullup(m_kSensorPin);
	}
	else
	{
		digitalWrite(m_kSensorPin, HIGH);
	}
	delayMicroseconds(10);

	cli();

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
			// shove each bit into the storage bytes
			m_data[j / 8] <<= 1;
			if (counter > m_count)
				m_data[j / 8] |= 1;
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

	// check we read 40 bits and that the checksum matches
	if ((j >= 40)
			&& (m_data[4] == ((m_data[0] + m_data[1] + m_data[2] + m_data[3]) & 0xFF)))
	{
		updateInternalCache();
		return READ_OK;
	}

	return READ_FAIL;

}
