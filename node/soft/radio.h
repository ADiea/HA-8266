//Modified by ADiea: rewrite for C

/*
 * SI4432 library for Arduino - v0.1
 *
 * Please note that Library uses standart SS pin for NSEL pin on the chip. This is 53 for Mega, 10 for Uno.
 * NOTES:
 *
 * V0.1
 * * Library supports no `custom' changes and usages of GPIO pin. Modify/add/remove your changes if necessary
 * * Radio use variable packet field format with 4 byte address header, first data field as length. Change if necessary
 *
 * made by Ahmet (theGanymedes) Ipkin
 *
 * 2014
 */
 
#ifndef RADIO_H
#define RADIO_H

#include "main.h"

#define DEBUG_SI4432 1
#define DEBUG_SI4432_VERBOSE 0

typedef unsigned char byte;
typedef unsigned char bool;

#ifndef false
#define false 0
#endif

#ifndef true
#define true 1
#endif

typedef enum _eBaudRate
{
	eBaud_38k4 = 0,
	eBaud_115k2,
	eBaud_230k4,
	e_Baud_numBauds
} eBaudRate;


/* Now, according to the this design, you must
 * 1- Create an instance
 * 2- Call init()
 *
 * According to the data sheet, you can change any register value and most will get to work after going into IDLE state and back (to RX/TX)
 * (some are hot - changes right away) I didn't test this - but why not? :)
 * */

void radio_setFrequency(unsigned long baseFrequency); // sets the freq. call before boot
void radio_setChannel(byte channel); // sets the channel. call before switching to tx or rx mode
void radio_setBaudRateFast(eBaudRate baud);
void radio_init(void);
void radio_setCommsSignature(uint16_t signature); // used to 'sign' packets with a predetermined signature - call before boot

 // switches to Tx mode and sends the package, then optionally receives response package
bool radio_sendPacketSimple(uint8_t length, const byte* data);


bool radio_sendPacket(uint8_t length, const byte* data, bool waitResponse, uint32_t ackTimeout,
		uint8_t *responseLength, byte* responseBuffer); // switches to Tx mode and sends the package, then optionally receives response package

void radio_startListening(void); // switch to Rx mode (don't block)

bool radio_isPacketReceived(void); // check for the packet received flags

bool radio_waitForPacket(uint64_t waitMs); // switch to Rx mode and wait until timeout or 'valid' package to arrive
void radio_getPacketReceived(uint8_t* length, byte* readData); // read from FIFO

void radio_readAll(void);

void radio_clearTxFIFO(void);
void radio_clearRxFIFO(void);

void radio_clearFIFO(void);

void radio_softReset(void);

void radio_hardReset(void);


#endif
