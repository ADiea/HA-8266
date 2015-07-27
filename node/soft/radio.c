#include "radio.h"

#define MAX_TRANSMIT_TIMEOUT 200

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
uint16_t _kbps;
uint16_t _packageSign;

// sets SPI and pins ready and boot the radio
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
	_kbps = 100;
	_packageSign = 0xDEAD;
}

// switches to Tx mode and sends the package, then optionally receives response package
bool sendPacketSimple(uint8_t length, const byte* data)
{
	return sendPacket(length, data, false, 100, 0, 0);
}



