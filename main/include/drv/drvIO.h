#ifndef DRV_PORTEXPANDER
#define DRV_PORTEXPANDER

#include "driver.h"
#include <Libraries/PCF8574/PCF8574.h>

#define IO_PIN_INT_PIR 0
#define IO_PIN_CS_SD 1
#define IO_PIN_ENA_RST 2
#define IO_PIN_CS_RF 3
#define IO_PIN_OUT_PREHEAT 4
#define IO_PIN_OUT_HAPTIC 5
#define IO_PIN_INT_MISC 6
#define IO_PIN_OUT_LCD_PWR 7

#define IO_PIN_MAX IO_PIN_OUT_LCD_PWR

#define PIN_SET 1
#define PIN_CLEAR 0
#define PIN_ERR 0xFF

enum eIODirection
{
	eIO_In,
	eIO_Out
};

class CDrvIO : public CGenericDriver
{
public:
	CDrvIO():m_theChip(NULL), mPin(0xFF), mPort(0xFF), mDdr(0x00){}

	bool setPin(uint8_t pin, uint8_t val);
	uint8_t testPin(uint8_T pin);

	void setDirection(uint8_t pin, eIODirection dir);

	virtual eDriverError setup(eDriverOp op = drvEnable);
	virtual ~CDrvIO(){{setup(drvDisable);}}
private:
	uint8_t safePort();//always write 1 to an input pin to enable
						//weak pullup rather than strong pulldown

	bool isPinValid(uint8_t pin){return pin <= IO_PIN_MAX;}

	bool readIO();
	bool writeIO();

	PCF8574 *m_theChip;
	uint8_t mPin, mPort, mDdr;
};

extern CDrvIO DrvIO;

#endif /*DRV_PORTEXPANDER*/
