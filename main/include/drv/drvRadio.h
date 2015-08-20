#ifndef DRV_RADIO
#define DRV_RADIO

#include "device.h"
#include <SmingCore/SmingCore.h>
#include <Libraries/si4432/si4432.h>

#define RADIO_WAIT_ACK_MS 100

uint8_t devRadio_init(uint8_t operation);


extern Si4432 *radio;


#endif /*DRV_RADIO*/
