#ifndef DRV_RADIO
#define DRV_RADIO

#include "radio_protocol.h"

#include "driver.h"
#include <Libraries/si4432/si4432.h>

/*(!) Warning on some hardware versions (ESP07, maybe ESP12)
 * 		pins GPIO4 and GPIO5 are swapped !*/
#define PIN_RADIO_DO 5	/* Master In Slave Out */
#define PIN_RADIO_DI 4	/* Master Out Slave In */
#define PIN_RADIO_CK 15	/* Serial Clock */
#define PIN_RADIO_SS 13	/* Slave Select */

#define RADIO_WAIT_ACK_MS 150

class CDrvRadio : public CGenericDriver
{
public:
	CDrvRadio():m_theChip(NULL), pRadioSPI(NULL), mRadioSendFaults(0),
	mSeqID(0){}

	virtual eDriverError setup(eDriverOp op = drvEnable);
	virtual ~CDrvRadio();

	uint8_t init_DEV_RADIO(uint8_t operation);

	uint8_t RadioNextSeqID();

	uint8_t checkRadioChecksum(byte *pkg, uint8_t length);

	eRadioError RadioSend(byte *pkg, uint8_t length, uint8_t *outLen, uint32_t waitMs)
	{
		return mSeqID++;
	}

private:
	Si4432 *m_theChip;
	SPISoft *pRadioSPI;
	uint8_t mRadioSendFaults = 0;
	uint8_t mSeqID = 0;
};

extern CDrvRadio DrvRadio;

#endif /*DRV_RADIO*/
