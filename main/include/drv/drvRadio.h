#ifndef DRV_RADIO
#define DRV_RADIO

#include "radio_protocol.h"

#include "driver.h"
#include <Libraries/si4432/si4432.h>

#define RADIO_WAIT_SPI_BUS 30
#define RADIO_WAIT_ACK_MS 150

class CDrvRadio : public CGenericDriver
{
public:
	CDrvRadio():m_theChip(NULL), pRadioSPI(NULL), mRadioSendFaults(0),
	mSeqID(0){}

	virtual eDriverError setup(eDriverOp op = drvEnable);
	virtual ~CDrvRadio(){setup(drvDisable);}

	uint8_t RadioNextSeqID()
	{
		return mSeqID++;
	}

	uint8_t checkRadioChecksum(byte *pkg, uint8_t length);

	eRadioError RadioSend(byte *pkg, uint8_t length, uint8_t *outLen, uint32_t waitMs);

	bool DelegateCS(eSPIChipSelect op);

	Si4432* get(){return m_theChip;}

private:
	bool initRadio();

	Si4432 *m_theChip;
	uint8_t mRadioSendFaults = 0;
	uint8_t mSeqID = 0;
};

extern CDrvRadio DrvRadio;

#endif /*DRV_RADIO*/
