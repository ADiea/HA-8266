#include "radio.h"

//TODO: 
//verify 0x75,76,77 -> freq select
//verify  0x7a channel spet size: currently 1mhz

#define MAX_TRANSMIT_TIMEOUT 200

#define BAUD_RATE_REGS 12
typedef struct _baudRateCfg
{
	uint8_t reg[BAUD_RATE_REGS];
} baudRateCfg;

baudRateCfg BaudRates[] = 
{
	//0x1C  0x20  0x21  0x22   0x23  0x24  0x25  0x6E  0x6F  0x70  0x71  0x72	
	{{ 0x02, 0x68, 0x01, 0x3A, 0x93, 0x04, 0xEE, 0x09, 0xD5, 0x0C, 0x23, 0x1F}}, //38k4 -> 38k
	{{ 0x82, 0x68, 0x01, 0x3A, 0x93, 0x04, 0xEE, 0x1D, 0x7E, 0x0C, 0x23, 0x5C}}, //115k2 -> 115k
	{{ 0x8B, 0x34, 0x02, 0x75, 0x25, 0x07, 0xFF, 0x3A, 0xFB, 0x0C, 0x23, 0xB8}} //230k4 -> 230k

/*
calc by esp	=> veriify with integer baud rate values in excel file
	{ 0x89, 0x3c, 0x20, 0x67, 0xc4, 0x00, 0x36, 0x09, 0xBA, 0x0C, 0x23, 0xF0}, //38k
	{ 0x8A, 0x68, 0x01, 0x3A, 0x07, 0x01, 0xE5, 0x1D, 0x71, 0x0C, 0x23, 0xF0}, //115k
	{ 0x8D, 0x34, 0x02, 0x74, 0x0E, 0x07, 0x8E, 0x3A, 0xE1, 0x0C, 0x23, 0xF0}, //230k
*/
};

typedef enum _AntennaMode 
{
	RXMode = 0x04, TXMode = 0x08, Ready = 0x01, TuneMode = 0x02
} AntennaMode;

typedef enum _Registers 
{
	REG_DEV_TYPE = 0x00,
	REG_DEV_VERSION = 0x01,
	REG_DEV_STATUS = 0x02,

	REG_INT_STATUS1 = 0x03,
	REG_INT_STATUS2 = 0x04,
	REG_INT_ENABLE1 = 0x05,
	REG_INT_ENABLE2 = 0x06,
	REG_STATE = 0x07,
	REG_OPERATION_CONTROL = 0x08,

	REG_MCU_OUT_CLK = 0x0A,

	REG_GPIO0_CONF = 0x0B,
	REG_GPIO1_CONF = 0x0C,
	REG_GPIO2_CONF = 0x0D,
	REG_IOPORT_CONF = 0x0E,

	REG_IF_FILTER_BW = 0x1C,
	REG_AFC_LOOP_GEARSHIFT_OVERRIDE = 0x1D,
	REG_AFC_TIMING_CONTROL = 0x1E,
	REG_CLOCK_RECOVERY_GEARSHIFT = 0x1F,
	REG_CLOCK_RECOVERY_OVERSAMPLING = 0x20,
	REG_CLOCK_RECOVERY_OFFSET2 = 0x21,
	REG_CLOCK_RECOVERY_OFFSET1 = 0x22,
	REG_CLOCK_RECOVERY_OFFSET0 = 0x23,
	REG_CLOCK_RECOVERY_TIMING_GAIN1 = 0x24,
	REG_CLOCK_RECOVERY_TIMING_GAIN0 = 0x25,
	REG_RSSI = 0x26,
	REG_RSSI_THRESHOLD = 0x27,

	REG_AFC_LIMITER = 0x2A,
	REG_AFC_CORRECTION_READ = 0x2B,

	REG_DATAACCESS_CONTROL = 0x30,
	REG_EZMAC_STATUS = 0x31,
	REG_HEADER_CONTROL1 = 0x32,
	REG_HEADER_CONTROL2 = 0x33,
	REG_PREAMBLE_LENGTH = 0x34,
	REG_PREAMBLE_DETECTION = 0x35,
	REG_SYNC_WORD3 = 0x36,
	REG_SYNC_WORD2 = 0x37,
	REG_SYNC_WORD1 = 0x38,
	REG_SYNC_WORD0 = 0x39,
	REG_TRANSMIT_HEADER3 = 0x3A,
	REG_TRANSMIT_HEADER2 = 0x3B,
	REG_TRANSMIT_HEADER1 = 0x3C,
	REG_TRANSMIT_HEADER0 = 0x3D,

	REG_PKG_LEN = 0x3E,

	REG_CHECK_HEADER3 = 0x3F,
	REG_CHECK_HEADER2 = 0x40,
	REG_CHECK_HEADER1 = 0x41,
	REG_CHECK_HEADER0 = 0x42,

	REG_RECEIVED_HEADER3 = 0x47,
	REG_RECEIVED_HEADER2 = 0x48,
	REG_RECEIVED_HEADER1 = 0x49,
	REG_RECEIVED_HEADER0 = 0x4A,

	REG_RECEIVED_LENGTH = 0x4B,

	REG_CHARGEPUMP_OVERRIDE = 0x58,
	REG_DIVIDER_CURRENT_TRIM = 0x59,
	REG_VCO_CURRENT_TRIM = 0x5A,

	REG_AGC_OVERRIDE = 0x69,

	REG_TX_POWER = 0x6D,
	REG_TX_DATARATE1 = 0x6E,
	REG_TX_DATARATE0 = 0x6F,

	REG_MODULATION_MODE1 = 0x70,
	REG_MODULATION_MODE2 = 0x71,

	REG_FREQ_DEVIATION = 0x72,
	REG_FREQ_OFFSET1 = 0x73,
	REG_FREQ_OFFSET2 = 0x74,
	REG_FREQBAND = 0x75,
	REG_FREQCARRIER_H = 0x76,
	REG_FREQCARRIER_L = 0x77,

	REG_FREQCHANNEL = 0x79,
	REG_CHANNEL_STEPSIZE = 0x7A,

	REG_FIFO = 0x7F,
} Registers;

//values here are kept in khz x 10 format (for not to deal with decimals) - look at AN440 page 26 for whole table
const uint16_t IFFilterTable[][2] = { { 322, 0x26 }, { 3355, 0x88 }, { 3618, 0x89 }, { 4202, 0x8A }, { 4684, 0x8B }, {
		5188, 0x8C }, { 5770, 0x8D }, { 6207, 0x8E } };
		
uint64_t _freqCarrier;
uint8_t _freqChannel;
eBaudRate _kbps;
uint16_t _packageSign;

uint8_t _intPin = 0;

/************* STATIC FCTS ****************/
static void boot(); 
static void switchMode(byte mode);
static void ChangeRegister(Registers reg, byte value);
static byte ReadRegister(Registers reg);
static void BurstWrite(Registers startReg, const byte value[], uint8_t length);
static void BurstRead(Registers startReg, byte value[], uint8_t length);

/*$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/

void radio_init(void)
{
	_freqCarrier = 433000000;
	_freqChannel = 0;
	_kbps = eBaud_38k4;
	_packageSign = 0xDEAD;

	radio_hardReset();
}

// switches to Tx mode and sends the package, then optionally receives response package
bool radio_sendPacketSimple(uint8_t length, const byte* data)
{
	return radio_sendPacket(length, data, false, 100, 0, 0);
}


void radio_setFrequency(unsigned long baseFrequencyMhz) {

	if ((baseFrequencyMhz < 240) || (baseFrequencyMhz > 930))
		return; // invalid frequency

	_freqCarrier = baseFrequencyMhz;
	byte highBand = 0;
	if (baseFrequencyMhz >= 480) {
		highBand = 1;
	}

	double fPart = (baseFrequencyMhz / (10 * (highBand + 1))) - 24;

	uint8_t freqband = (uint8_t) fPart; // truncate the int

	uint16_t freqcarrier = (fPart - freqband) * 64000;

	// sideband is always on (0x40) :
	byte vals[3] = { (byte)(0x40 | (highBand << 5) | (freqband & 0x3F)),
					(byte)(freqcarrier >> 8),
					(byte)(freqcarrier & 0xFF) };

	BurstWrite(REG_FREQBAND, vals, 3);

}

void radio_setCommsSignature(uint16_t signature) {
	_packageSign = signature;

	ChangeRegister(REG_TRANSMIT_HEADER3, _packageSign >> 8); // header (signature) byte 3 val
	ChangeRegister(REG_TRANSMIT_HEADER2, (_packageSign & 0xFF)); // header (signature) byte 2 val

	ChangeRegister(REG_CHECK_HEADER3, _packageSign >> 8); // header (signature) byte 3 val for receive checks
	ChangeRegister(REG_CHECK_HEADER2, (_packageSign & 0xFF)); // header (signature) byte 2 val for receive checks

#if DEBUG_SI4432
	debugf("Package signature is set!");
#endif
}


bool radio_sendPacket(uint8_t length, const byte* data, bool waitResponse, uint32_t ackTimeout,
		uint8_t* responseLength, byte* responseBuffer) {

	radio_clearTxFIFO();
	ChangeRegister(REG_PKG_LEN, length);

	BurstWrite(REG_FIFO, data, length);

	ChangeRegister(REG_INT_ENABLE1, 0x04); // set interrupts on for package sent
	ChangeRegister(REG_INT_ENABLE2, 0x00); // set interrupts off for anything else
	//read interrupt registers to clean them
	ReadRegister(REG_INT_STATUS1);
	ReadRegister(REG_INT_STATUS2);

	switchMode(TXMode | Ready);

	uint64_t enterMillis = millis();

	while (millis() - enterMillis < MAX_TRANSMIT_TIMEOUT) 
	{

		if ((_intPin != 0) /*&& (digitalRead(_intPin) != 0)*/) {
			continue;
		}

		byte intStatus = ReadRegister(REG_INT_STATUS1);
		ReadRegister(REG_INT_STATUS2);

		if (intStatus & 0x04) {
			switchMode(Ready | TuneMode);
#if DEBUG_SI4432
			debugf("Pkg sent--%x\n", intStatus);
#endif
			// package sent. now, return true if not to wait ack, or wait ack (wait for packet only for 'remaining' amount of time)
			if (waitResponse) {
				if (radio_waitForPacket(ackTimeout)) {
					radio_getPacketReceived(responseLength, responseBuffer);
					return true;
				} else {
					return false;
				}
			} else {
				return true;
			}
		}
	}

	//timeout occured.
#if DEBUG_SI4432
	debugf("TX timeout\n");
#endif
	switchMode(Ready);

	if (ReadRegister(REG_DEV_STATUS) & 0x80) {
		radio_clearFIFO();
	}

	return false;

}

bool radio_waitForPacket(uint64_t waitMs) {

	radio_startListening();

	uint64_t enterMillis = millis();
	while (millis() - enterMillis < waitMs) {

		if (!radio_isPacketReceived()) {
			continue;
		} else {
			return true;
		}

	}
	//timeout occured.

	debugf("RX timeout\n");

	switchMode(Ready);
	radio_clearRxFIFO();

	return false;
}

void radio_getPacketReceived(uint8_t* length, byte* readData) {

	*length = ReadRegister(REG_RECEIVED_LENGTH);

	if(!(*length) && *length <= 64)
	{
		BurstRead(REG_FIFO, readData, *length);
	}
	else *length = 0;

	radio_clearRxFIFO(); // which will also clear the interrupts
}

void radio_setChannel(byte channel)
{
	ChangeRegister(REG_FREQCHANNEL, channel);
}

void radio_setBaudRateFast(eBaudRate baud)
{
	BurstWrite(REG_IF_FILTER_BW, &(BaudRates[baud].reg[0]), 1);

	BurstWrite(REG_CLOCK_RECOVERY_OVERSAMPLING, &(BaudRates[baud].reg[1]), 6);

	BurstWrite(REG_TX_DATARATE1, &(BaudRates[baud].reg[7]), 5);
}

void radio_readAll() {

	byte allValues[0x7F];
	byte i;

	BurstRead(REG_DEV_TYPE, allValues, 0x7F);

	debugf("\n\nREGS  00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\r\n");

	for ( i = 0; i < 0x7f; i+=16)
	{
		debugf("(%02x): %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", i,
				(int ) allValues[i+0], (int ) allValues[i+1], (int ) allValues[i+2], (int ) allValues[i+3],
				(int ) allValues[i+4], (int ) allValues[i+5], (int ) allValues[i+6], (int ) allValues[i+7],
				(int ) allValues[i+8], (int ) allValues[i+9], (int ) allValues[i+10], (int ) allValues[i+11],
				(int ) allValues[i+12], (int ) allValues[i+13], (int ) allValues[i+14], (int ) allValues[i+15]
				);
	}
}

void radio_clearTxFIFO() {
	ChangeRegister(REG_OPERATION_CONTROL, 0x01);
	ChangeRegister(REG_OPERATION_CONTROL, 0x00);

}

void radio_clearRxFIFO() {
	ChangeRegister(REG_OPERATION_CONTROL, 0x02);
	ChangeRegister(REG_OPERATION_CONTROL, 0x00);
}

void radio_clearFIFO() {
	ChangeRegister(REG_OPERATION_CONTROL, 0x03);
	ChangeRegister(REG_OPERATION_CONTROL, 0x00);
}

void radio_softReset() {

	switchMode(0x80);//chip reset
	_delay_ms(1);

	byte reg = ReadRegister(REG_INT_STATUS2);
	while ((reg & 0x02) != 0x02) {
		_delay_ms(1);
		reg = ReadRegister(REG_INT_STATUS2);
	}

	boot();

}

void radio_hardReset() {


	switchMode(0x80);//chip reset
	_delay_ms(1);

	byte reg = ReadRegister(REG_INT_STATUS2);
	uint8_t count=10;
	while ((reg & 0x02) != 0x02 && reg != 0xFF && --count) {

		debugf("POR: %x ", reg);
		_delay_ms(100);
		reg = ReadRegister(REG_INT_STATUS2);
	}

	boot();
}

void radio_startListening() {

	radio_clearRxFIFO(); // clear first, so it doesn't overflow if packet is big

	ChangeRegister(REG_INT_ENABLE1, 0x03); // set interrupts on for package received and CRC error

#if DEBUG_SI4432
	ChangeRegister(REG_INT_ENABLE2, 0xC0);
#else
	ChangeRegister(REG_INT_ENABLE2, 0x00); // set other interrupts off
#endif
	//read interrupt registers to clean them
	ReadRegister(REG_INT_STATUS1);
	ReadRegister(REG_INT_STATUS2);

	switchMode(RXMode | Ready);
}

bool radio_isPacketReceived() {

	if ((_intPin != 0) /*&& (digitalRead(_intPin) != 0)*/) {
		return false; // if no interrupt occured, no packet received is assumed (since startListening will be called prior, this assumption is enough)
	}
	// check for package received status interrupt register
	byte intStat = ReadRegister(REG_INT_STATUS1);

#if DEBUG_SI4432
	byte intStat2 = ReadRegister(REG_INT_STATUS2);

	if (intStat2 & 0x40) { //interrupt occured, check it && read the Interrupt Status1 register for 'preamble '

		debugf("Valid Preamb %x\n", intStat2);

	}

	if (intStat2 & 0x80) { //interrupt occured, check it && read the Interrupt Status1 register for 'preamble '

		debugf("Valid sync %x\n", intStat2);

	}
#else
	ReadRegister(REG_INT_STATUS2);
#endif

	if (intStat & 0x02) { //interrupt occured, check it && read the Interrupt Status1 register for 'valid packet'

		switchMode(Ready | TuneMode); // if packet came, get out of Rx mode till the packet is read out. Keep PLL on for fast reaction
#if DEBUG_SI4432
				debugf("Pkg detect %x\n", intStat);
#endif
		return true;
	} else if (intStat & 0x01) { // packet crc error

		switchMode(Ready); // get out of Rx mode till buffers are cleared
#if DEBUG_SI4432
		debugf("CRC Error %x\n", intStat);
#endif
		radio_clearRxFIFO();
		switchMode(RXMode | Ready); // get back to work
		return false;
	}

	//no relevant interrupt? no packet!

	return false;
}


/************* STATIC FCTS ****************/


void static boot() {
	/*
	 byte currentFix[] = { 0x80, 0x40, 0x7F };
	 BurstWrite(REG_CHARGEPUMP_OVERRIDE, currentFix, 3); // refer to AN440 for reasons

	 ChangeRegister(REG_GPIO0_CONF, 0x0F); // tx/rx data clk pin
	 ChangeRegister(REG_GPIO1_CONF, 0x00); // POR inverted pin
	 ChangeRegister(REG_GPIO2_CONF, 0x1C); // clear channel pin
	 */
	ChangeRegister(REG_AFC_TIMING_CONTROL, 0x02); // refer to AN440 for reasons
	ChangeRegister(REG_AFC_LIMITER, 0xFF); // write max value - excel file did that.
	ChangeRegister(REG_AGC_OVERRIDE, 0x60); // max gain control
	ChangeRegister(REG_AFC_LOOP_GEARSHIFT_OVERRIDE, 0x3C); // turn off AFC
	ChangeRegister(REG_DATAACCESS_CONTROL, 0xAD); // enable rx packet handling, enable tx packet handling, enable CRC, use CRC-IBM
	ChangeRegister(REG_HEADER_CONTROL1, 0x0C); // no broadcast address control, enable check headers for bytes 3 & 2
	ChangeRegister(REG_HEADER_CONTROL2, 0x22);  // enable headers byte 3 & 2, no fixed package length, sync word 3 & 2
	ChangeRegister(REG_PREAMBLE_LENGTH, 0x08); // 8 * 4 bits = 32 bits (4 bytes) preamble length
	ChangeRegister(REG_PREAMBLE_DETECTION, 0x3A); // validate 7 * 4 bits of preamble  in a package
	ChangeRegister(REG_SYNC_WORD3, 0x2D); // sync byte 3 val
	ChangeRegister(REG_SYNC_WORD2, 0xD4); // sync byte 2 val

	ChangeRegister(REG_TX_POWER, 0x1F); // max power

	ChangeRegister(REG_CHANNEL_STEPSIZE, 0x64); // each channel is of 1 Mhz interval

	//radio_setFrequency(_freqCarrier); // default freq
	
	//set frequency to 433Mhz
	ChangeRegister(REG_FREQBAND, 0x53);
	ChangeRegister(REG_FREQCARRIER_H, 0x0);
	ChangeRegister(REG_FREQCARRIER_L, 0x0);
	
	//set external clk speed to 10Mhz
	/*
	000: 30 MHz
	001: 15 MHz
	010: 10 MHz
	011: 4 MHz
	100: 3 MHz
	101: 2 MHz
	110: 1 MHz *default
	*/
	
	ChangeRegister(REG_MCU_OUT_CLK, 0x03);

	radio_setBaudRateFast(_kbps); // default baud rate is 100kpbs
	radio_setChannel(_freqChannel); // default channel is 0
	radio_setCommsSignature(_packageSign); // default signature

	switchMode(Ready);
}

void static switchMode(byte mode) {

	ChangeRegister(REG_STATE, mode); // receive mode
	//_delay_ms(20);
#if DEBUG_SI4432
	_delay_ms(1);
	byte val = ReadRegister(REG_DEV_STATUS);
	debugf("DEV STAT:%x\n", val);

#endif
}

void static ChangeRegister(Registers reg, byte value) {
	BurstWrite(reg, &value, 1);
}

byte static ReadRegister(Registers reg) {
	byte val = 0xFF;
	BurstRead(reg, &val, 1);
	return val;
}

void static BurstWrite(Registers startReg, const byte value[], uint8_t length) {

	byte regVal = (byte) startReg | 0x80; // set MSB

	spi_enable();
	asm volatile("nop");
	spi_send(&regVal, 1);

#if DEBUG_SI4432
	#if DEBUG_SI4432_VERBOSE
		debugf("Writing: %x | %x ... %x (%d bytes)", (regVal != 0xFF ? (regVal) & 0x7F : 0x7F),
				value[0], value[length-1], length);
	#endif
#endif

	spi_send(value, length);

	spi_disable();
}

void static BurstRead(Registers startReg, byte value[], uint8_t length) {

	byte regVal = (byte) startReg & 0x7F; // set MSB

	spi_enable();
	asm volatile("nop");
	spi_send(&regVal, 1);

	spi_recv(value, length);

#if DEBUG_SI4432
	#if DEBUG_SI4432_VERBOSE
		debugf("Reading: %x  | %x..%x (%d bytes)", (regVal != 0x7F ? (regVal) & 0x7F : 0x7F),
				value[0], value[length-1], length);
	#endif		
#endif
	spi_disable();
}