#include "drv/drvIO.h"

#define IO_CHIP_ADDR 0x20

CDrvIO DrvIO;

bool CDrvIO::setPin(uint8_t pin, uint8_t val)
{
	if(isPinValid(pin))
	{
		if(val)
		{
			mPort |= (1<<pin);
		}
		else
		{
			mPort &= ~(1<<pin);
		}

		return writeIO();
	}
	return false;
}

uint8_t CDrvIO::testPin(uint8_t pin)
{
	if(isPinValid(pin))
	{
		if(readIO())
		{
			if(mPin & (1<<pin))
			{
				return PIN_SET;
			}
			else
			{
				return PIN_CLEAR;
			}
		}
	}
	return PIN_ERR;
}

void CDrvIO::setDirection(uint8_t pin, eIODirection dir)
{
	if(isPinValid(pin))
	{
		if(dir == eIO_In)
		{
			mDdr &= ~(1<<pin);
		}
		else
		{
			mDdr |= (1<<pin);
		}
	}
}

uint8_t CDrvIO::safePort()
{
	return (mPort | ~mDdr);
}

bool CDrvIO::readIO()
{
	CBusAutoRelease bus(devI2C_IO);

	if(bus.getBus())
	{
		mPin = m_theChip->read();
	}
	else return false;

	return true;
}

bool CDrvIO::writeIO()
{
	CBusAutoRelease bus(devI2C_IO);

	if(bus.getBus())
	{
		m_theChip->write(safePort());
	}
	else return false;

	return true;
}


eDriverError CDrvIO::setup(eDriverOp op)
{
	eDriverError retErr = drvErrOther;
	do
	{
		if(drvEnable == op)
		{
			if(m_theChip)
			{
				delete m_theChip;
				m_theChip = NULL;
			}

			m_theChip = new PCF8574();

			if(m_theChip)
			{
				CBusAutoRelease bus(devI2C_IO);

				if(bus.getBus())
				{
					m_theChip->begin(IO_CHIP_ADDR);
						setDirection(IO_PIN_CS_SD, eIO_Out);
						setDirection(IO_PIN_ENA_RST, eIO_Out);
						setDirection(IO_PIN_CS_RF, eIO_Out);
						setDirection(IO_PIN_OUT_PREHEAT, eIO_Out);
						setDirection(IO_PIN_OUT_HAPTIC, eIO_Out);
						setDirection(IO_PIN_OUT_LCD_PWR, eIO_Out);

						retErr = drvErrOK;
						m_State = drvEnabled;

				}
				else
				{
					retErr = drvErrBus;
				}
			}
			else
			{
				retErr = drvErrMalloc;
			}

		}
		else if (drvDisable == op)
		{
			if(m_theChip)
			{
				delete m_theChip;
				m_theChip = NULL;
			}

			retErr = drvErrOK;
			m_State = drvDisabled;
		}
	} while(0);
	m_lastError = retErr;
	return retErr;
}





