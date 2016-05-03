#ifndef DRV_RADIO
#define DRV_RADIO

#include "radio_protocol.h"

#include "device.h"
#include <SmingCore/SmingCore.h>
#include <Libraries/si4432/si4432.h>

#define RADIO_WAIT_ACK_MS 150

uint8_t init_DEV_RADIO(uint8_t operation);

uint8_t RadioNextSeqID();

uint8_t checkRadioChecksum(byte *pkg, uint8_t length);

extern Si4432 *Radio;
bool RadioSend(byte *pkg, uint8_t length, uint8_t *outLen, uint32_t waitMs);
void releaseRadio();
bool getRadio(uint32_t waitMs);

#endif /*DRV_RADIO*/
