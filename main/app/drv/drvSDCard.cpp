/*------------------------------------------------------------------------/
/  Foolproof MMCv3/SDv1/SDv2 (in SPI mode) control module
/-------------------------------------------------------------------------/
/
/  Copyright (C) 2013, ChaN, all right reserved.
/
/ * This software is a free software and there is NO WARRANTY.
/ * No restriction on use. You can use, modify and redistribute it for
/   personal, non-profit or commercial products UNDER YOUR RESPONSIBILITY.
/ * Redistributions of source code must retain the above copyright notice.
/
/-------------------------------------------------------------------------/
  Features and Limitations:

  * Easy to Port Bit-banging SPI
    It uses only four GPIO pins. No complex peripheral needs to be used.

  * Platform Independent
    You need to modify only a few macros to control the GPIO port.

  * Low Speed
    The data transfer rate will be several times slower than hardware SPI.

  * No Media Change Detection
    Application program needs to perform a f_mount() after media change.

/-------------------------------------------------------------------------*/
#include "drv/diskio.h"		/* Common include file for FatFs and disk I/O layer */
#include "drv/drvSDCard.h"
#include <SmingCore/SmingCore.h>

#define PIN_CARD_DO 5
#define PIN_CARD_DI 4
#define PIN_CARD_CK 15
#define PIN_CARD_SS 12

FATFS FatFs;		/* FatFs work area needed for each volume */
FIL Fil;

#define SCK_SLOW_INIT 4
#define SCK_NORMAL 0

uchar gUsDelay = SCK_SLOW_INIT;

/*-------------------------------------------------------------------------*/
/* Platform dependent macros and functions needed to be modified           */
/*-------------------------------------------------------------------------*/

#define DO_INIT()	do{pinMode(PIN_CARD_DO, INPUT);pullup(PIN_CARD_DO);}while(0)			/* Initialize port for MMC DO as input */
#define DO			digitalRead(PIN_CARD_DO)	/* Test for MMC DO ('H':true, 'L':false) */

#define DI_INIT()	pinMode(PIN_CARD_DI, OUTPUT)	/* Initialize port for MMC DI as output */
#define DI_H()		digitalWrite(PIN_CARD_DI, HIGH)	/* Set MMC DI "high" */
#define DI_L()		digitalWrite(PIN_CARD_DI, LOW)	/* Set MMC DI "low" */

#define CK_INIT()	pinMode(PIN_CARD_CK, OUTPUT)	/* Initialize port for MMC SCLK as output */
#define CK_H()		do{digitalWrite(PIN_CARD_CK, HIGH);delayMicroseconds(gUsDelay);}while(0)	/* Set MMC SCLK "high" */
#define	CK_L()		do{digitalWrite(PIN_CARD_CK, LOW);delayMicroseconds(gUsDelay);}while(0)	/* Set MMC SCLK "low" */

#define CS_INIT()	pinMode(PIN_CARD_SS, OUTPUT)	/* Initialize port for MMC CS as output */
#define	CS_H()		digitalWrite(PIN_CARD_SS, HIGH)	/* Set MMC CS "high" */
#define CS_L()		digitalWrite(PIN_CARD_SS, LOW)	/* Set MMC CS "low" */


static
void dly_us (UINT n)	/* Delay n microseconds (avr-gcc -Os) */
{
	delayMicroseconds(n);
}

void devSDCard_benchmark()
{
	unsigned int bw;
	uchar buf[1024];

	unsigned int i;

	for(i=0; i<1024; i++)
		buf[i] = '0'+(i%10);
	FRESULT fRes;
	uint32_t t1, t2, td;

	t1 = system_get_time();
	LOG(INFO, "Write 1K in 1K increment\n");
	fRes = f_open(&Fil, "b1k.txt", FA_WRITE | FA_CREATE_ALWAYS);
	if (fRes == FR_OK)
	{
		f_write(&Fil, buf, 1024, &bw);	/* Write data to the file */

		f_close(&Fil);								/* Close the file */

		if (bw != 1024) /* Lights green LED if data written well */
		{
			LOG(INFO, "Write to file FAIL\n");
		}
	}
	else
	{
		LOG(INFO, "fopen FAIL %d", fRes);
	}
	t2 = system_get_time();
	LOG(INFO, "Test end: %lu\n", t2 - t1);

	t1 = system_get_time();
	LOG(INFO, "(2) Write 1K in 1K increment\n");
	fRes = f_open(&Fil, "b1k.txt", FA_WRITE | FA_CREATE_ALWAYS);
	if (fRes == FR_OK)
	{
		f_write(&Fil, buf, 1024, &bw);	/* Write data to the file */

		f_close(&Fil);								/* Close the file */

		if (bw != 1024) /* Lights green LED if data written well */
		{
			LOG(INFO, "Write to file FAIL\n");
		}
	}
	else
	{
		LOG(INFO, "fopen FAIL %d", fRes);
	}
	t2 = system_get_time();
	LOG(INFO, "Test end: %lu = ", t2 - t1); //Serial.print(1000000./(t2-t1)); LOG(INFO, "kBps\n");


	t1 = system_get_time();
	LOG(INFO, "Write 1K in 4 bytes increment\n");
	fRes = f_open(&Fil, "b1k4.txt", FA_WRITE | FA_CREATE_ALWAYS);
	if (fRes == FR_OK)
	{
		for(i=0; i<1024/4; i++)
		{
			f_write(&Fil, buf, 4, &bw);	/* Write data to the file */

			if (bw != 4) /* Lights green LED if data written well */
			{
				LOG(INFO, "Write to file FAIL: %d\n", i);
				break;
			}
		}

		f_close(&Fil);								/* Close the file */
	}
	else
	{
		LOG(INFO, "fopen FAIL %d", fRes);
	}
	t2 = system_get_time();
	LOG(INFO, "Test end: %lu = ", t2 - t1); //Serial.print(1000000./(t2-t1)); LOG(INFO, "kBps\n");

	t1 = system_get_time();
	LOG(INFO, "Write 1k in 64 bytes increment\n");
	fRes = f_open(&Fil, "b1k64.txt", FA_WRITE | FA_CREATE_ALWAYS);
	if (fRes == FR_OK)
	{
		for(i=0; i<1024/64; i++)
		{
			f_write(&Fil, buf, 64, &bw);	/* Write data to the file */

			if (bw != 64) /* Lights green LED if data written well */
			{
				LOG(INFO, "Write to file FAIL: %d\n", i);
				break;
			}
		}

		f_close(&Fil); /* Close the file */
	}
	else
	{
		LOG(INFO, "fopen FAIL %d", fRes);
	}
	t2 = system_get_time();
	LOG(INFO, "Test end: %lu = ", t2 - t1); //Serial.print(1000000./(t2-t1)); LOG(INFO, "kBps\n");

	t1 = system_get_time();
	LOG(INFO, "Write 8k in 256 bytes increment\n");
	fRes = f_open(&Fil, "b8k128.txt", FA_WRITE | FA_CREATE_ALWAYS);
	if (fRes == FR_OK)
	{
		for(i=0; i<32; i++)
		{
			f_write(&Fil, buf, 256, &bw);	/* Write data to the file */

			if (bw != 256) /* Lights green LED if data written well */
			{
				LOG(INFO, "Write to file FAIL: %d\n", i);
				break;
			}
		}

		f_close(&Fil); /* Close the file */
	}
	else
	{
		LOG(INFO, "fopen FAIL %d", fRes);
	}
	t2 = system_get_time();
	LOG(INFO, "Test end: %lu = ", t2 - t1); //Serial.print(8000000./(t2-t1)); LOG(INFO, "kBps\n");

	t1 = system_get_time();
	LOG(INFO, "Write 8k in 512 bytes increment\n");
	fRes = f_open(&Fil, "b8k512.txt", FA_WRITE | FA_CREATE_ALWAYS);
	if (fRes == FR_OK)
	{
		for(i=0; i<16; i++)
		{
			f_write(&Fil, buf, 512, &bw);	/* Write data to the file */


			if (bw != 512) /* Lights green LED if data written well */
			{
				LOG(INFO, "Write to file FAIL: %d\n", i);
				break;
			}
		}

		f_close(&Fil); /* Close the file */

	}
	else
	{
		LOG(INFO, "fopen FAIL %d", fRes);
	}
	t2 = system_get_time();
	LOG(INFO, "Test end: %lu = ", t2 - t1); //Serial.print(8000000./(t2-t1)); LOG(INFO, "kBps\n");
#if 0
	t1 = system_get_time();
	LOG(INFO, "Write 32k in 1024 bytes increment\n");
	fRes = f_open(&Fil, "b8k512.txt", FA_WRITE | FA_CREATE_ALWAYS);
	if (fRes == FR_OK)
	{
		for(i=0; i<32; i++)
		{
			f_write(&Fil, buf, 1024, &bw);	/* Write data to the file */


			if (bw != 1024) /* Lights green LED if data written well */
			{
				LOG(INFO, "Write to file FAIL: %d\n", i);
				break;
			}
		}

		f_close(&Fil); /* Close the file */

	}
	else
	{
		LOG(INFO, "fopen FAIL %d", fRes);
	}
	t2 = system_get_time();
	LOG(INFO, "Test end: %lu = ", t2 - t1); //Serial.print(32000000./(t2-t1)); LOG(INFO, "kBps\n");
#endif
}

uchar devSDCard_init(uchar operation)
{
	uchar retVal = DEV_ERR_OK;
	do
	{
		if(operation & ENABLE)
		{
			//init GPIO, enable device
			
			//configure device
			if(operation & CONFIG)
			{
				FRESULT mountRes = f_mount(&FatFs, "", 0);		/* Give a work area to the default drive */
				if(FR_OK != mountRes)
				{
					LOG(INFO, "f_mount: FAIL %d\n", mountRes);
				}
				else
				{
					LOG(INFO, "f_mount: SUCCESS %d\n", mountRes);
				}
			}
		}
		else
		{
		//Therefore to make MMC/SDC release DO signal, the master device must send a byte after CS signal is deasserted.
			//deinit GPIO
		}
	}
	while(0);

	return retVal;
}


/*--------------------------------------------------------------------------

   Module Private Functions

---------------------------------------------------------------------------*/

/* MMC/SD command (SPI mode) */
#define CMD0	(0)			/* GO_IDLE_STATE */
#define CMD1	(1)			/* SEND_OP_COND */
#define	ACMD41	(0x80+41)	/* SEND_OP_COND (SDC) */
#define CMD8	(8)			/* SEND_IF_COND */
#define CMD9	(9)			/* SEND_CSD */
#define CMD10	(10)		/* SEND_CID */
#define CMD12	(12)		/* STOP_TRANSMISSION */
#define CMD13	(13)		/* SEND_STATUS */
#define ACMD13	(0x80+13)	/* SD_STATUS (SDC) */
#define CMD16	(16)		/* SET_BLOCKLEN */
#define CMD17	(17)		/* READ_SINGLE_BLOCK */
#define CMD18	(18)		/* READ_MULTIPLE_BLOCK */
#define CMD23	(23)		/* SET_BLOCK_COUNT */
#define	ACMD23	(0x80+23)	/* SET_WR_BLK_ERASE_COUNT (SDC) */
#define CMD24	(24)		/* WRITE_BLOCK */
#define CMD25	(25)		/* WRITE_MULTIPLE_BLOCK */
#define CMD32	(32)		/* ERASE_ER_BLK_START */
#define CMD33	(33)		/* ERASE_ER_BLK_END */
#define CMD38	(38)		/* ERASE */
#define CMD55	(55)		/* APP_CMD */
#define CMD58	(58)		/* READ_OCR */


static
DSTATUS Stat = STA_NOINIT;	/* Disk status */

static
BYTE CardType;			/* b0:MMC, b1:SDv1, b2:SDv2, b3:Block addressing */



/*-----------------------------------------------------------------------*/
/* Transmit bytes to the card (bitbanging)                               */
/*-----------------------------------------------------------------------*/

static
void xmit_mmc (
	const BYTE* buff,	/* Data to be sent */
	UINT bc				/* Number of bytes to send */
)
{
	BYTE d;


	do {
		d = *buff++;	/* Get a byte to be sent */
		if (d & 0x80) DI_H(); else DI_L();	/* bit7 */
		CK_H(); CK_L();
		if (d & 0x40) DI_H(); else DI_L();	/* bit6 */
		CK_H(); CK_L();
		if (d & 0x20) DI_H(); else DI_L();	/* bit5 */
		CK_H(); CK_L();
		if (d & 0x10) DI_H(); else DI_L();	/* bit4 */
		CK_H(); CK_L();
		if (d & 0x08) DI_H(); else DI_L();	/* bit3 */
		CK_H(); CK_L();
		if (d & 0x04) DI_H(); else DI_L();	/* bit2 */
		CK_H(); CK_L();
		if (d & 0x02) DI_H(); else DI_L();	/* bit1 */
		CK_H(); CK_L();
		if (d & 0x01) DI_H(); else DI_L();	/* bit0 */
		CK_H(); CK_L();
	} while (--bc);
}



/*-----------------------------------------------------------------------*/
/* Receive bytes from the card (bitbanging)                              */
/*-----------------------------------------------------------------------*/

static
void rcvr_mmc (
	BYTE *buff,	/* Pointer to read buffer */
	UINT bc		/* Number of bytes to receive */
)
{
	BYTE r;


	DI_H();	/* Send 0xFF */

	do {
		r = 0;	 if (DO) r++;	/* bit7 */
		CK_H(); CK_L();
		r <<= 1; if (DO) r++;	/* bit6 */
		CK_H(); CK_L();
		r <<= 1; if (DO) r++;	/* bit5 */
		CK_H(); CK_L();
		r <<= 1; if (DO) r++;	/* bit4 */
		CK_H(); CK_L();
		r <<= 1; if (DO) r++;	/* bit3 */
		CK_H(); CK_L();
		r <<= 1; if (DO) r++;	/* bit2 */
		CK_H(); CK_L();
		r <<= 1; if (DO) r++;	/* bit1 */
		CK_H(); CK_L();
		r <<= 1; if (DO) r++;	/* bit0 */
		CK_H(); CK_L();
		*buff++ = r;			/* Store a received byte */
	} while (--bc);
}



/*-----------------------------------------------------------------------*/
/* Wait for card ready                                                   */
/*-----------------------------------------------------------------------*/

static
int wait_ready (void)	/* 1:OK, 0:Timeout */
{
	BYTE d;
	UINT tmr;


	for (tmr = 5000; tmr; tmr--) {	/* Wait for ready in timeout of 500ms */
		rcvr_mmc(&d, 1);
		if (d == 0xFF)
		{
		//LOG(INFO, "SD wait:%d00us\n", tmr);
		break;
		}


		dly_us(100);
	}

	return tmr ? 1 : 0;
}



/*-----------------------------------------------------------------------*/
/* Deselect the card and release SPI bus                                 */
/*-----------------------------------------------------------------------*/

static
void deselect (void)
{
	BYTE d;

	CS_H();				/* Set CS# high */
	rcvr_mmc(&d, 1);	/* Dummy clock (force DO hi-z for multiple slave SPI) */
}



/*-----------------------------------------------------------------------*/
/* Select the card and wait for ready                                    */
/*-----------------------------------------------------------------------*/

static
int select (void)	/* 1:OK, 0:Timeout */
{
	BYTE d;

	CS_L();				/* Set CS# low */
	rcvr_mmc(&d, 1);	/* Dummy clock (force DO enabled) */
	if (wait_ready()) return 1;	/* Wait for card ready */

	LOG(INFO, "SDCard select() failed\n");
	deselect();
	return 0;			/* Failed */
}



/*-----------------------------------------------------------------------*/
/* Receive a data packet from the card                                   */
/*-----------------------------------------------------------------------*/

static
int rcvr_datablock (	/* 1:OK, 0:Failed */
	BYTE *buff,			/* Data buffer to store received data */
	UINT btr			/* Byte count */
)
{
	BYTE d[2];
	UINT tmr;


	for (tmr = 1000; tmr; tmr--) {	/* Wait for data packet in timeout of 100ms */
		rcvr_mmc(d, 1);
		if (d[0] != 0xFF) break;
		dly_us(100);
	}
	if (d[0] != 0xFE) return 0;		/* If not valid data token, return with error */

	rcvr_mmc(buff, btr);			/* Receive the data block into buffer */
	rcvr_mmc(d, 2);					/* Discard CRC */

	return 1;						/* Return with success */
}



/*-----------------------------------------------------------------------*/
/* Send a data packet to the card                                        */
/*-----------------------------------------------------------------------*/

static
int xmit_datablock (	/* 1:OK, 0:Failed */
	const BYTE *buff,	/* 512 byte data block to be transmitted */
	BYTE token			/* Data/Stop token */
)
{
	BYTE d[2];


	if (!wait_ready()) return 0;

	d[0] = token;
	xmit_mmc(d, 1);				/* Xmit a token */
	if (token != 0xFD) {		/* Is it data token? */
		xmit_mmc(buff, 512);	/* Xmit the 512 byte data block to MMC */
		rcvr_mmc(d, 2);			/* Xmit dummy CRC (0xFF,0xFF) */
		rcvr_mmc(d, 1);			/* Receive data response */
		if ((d[0] & 0x1F) != 0x05)	/* If not accepted, return with error */
			return 0;
	}

	return 1;
}



/*-----------------------------------------------------------------------*/
/* Send a command packet to the card                                     */
/*-----------------------------------------------------------------------*/

static
BYTE send_cmd (		/* Returns command response (bit7==1:Send failed)*/
	BYTE cmd,		/* Command byte */
	DWORD arg		/* Argument */
)
{
	BYTE n, d, buf[6];


	if (cmd & 0x80) {	/* ACMD<n> is the command sequense of CMD55-CMD<n> */
		cmd &= 0x7F;
		n = send_cmd(CMD55, 0);
		if (n > 1) return n;
	}

	/* Select the card and wait for ready except to stop multiple block read */
	if (cmd != CMD12) {
		deselect();
		if (!select()) return 0xFF;
	}

	/* Send a command packet */
	buf[0] = 0x40 | cmd;			/* Start + Command index */
	buf[1] = (BYTE)(arg >> 24);		/* Argument[31..24] */
	buf[2] = (BYTE)(arg >> 16);		/* Argument[23..16] */
	buf[3] = (BYTE)(arg >> 8);		/* Argument[15..8] */
	buf[4] = (BYTE)arg;				/* Argument[7..0] */
	n = 0x01;						/* Dummy CRC + Stop */
	if (cmd == CMD0) n = 0x95;		/* (valid CRC for CMD0(0)) */
	if (cmd == CMD8) n = 0x87;		/* (valid CRC for CMD8(0x1AA)) */
	buf[5] = n;
	xmit_mmc(buf, 6);

	/* Receive command response */
	if (cmd == CMD12) rcvr_mmc(&d, 1);	/* Skip a stuff byte when stop reading */
	n = 10;								/* Wait for a valid response in timeout of 10 attempts */
	do
		rcvr_mmc(&d, 1);
	while ((d & 0x80) && --n);
	//LOG(INFO, "SDcard send_cmd %d (%d try)\n", d, n);
	return d;			/* Return with the response value */
}



/*--------------------------------------------------------------------------

   Public Functions

---------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE drv			/* Drive number (always 0) */
)
{
	if (drv) return STA_NOINIT;

	return Stat;
}



/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE drv		/* Physical drive nmuber (0) */
)
{
	BYTE n, ty, cmd, buf[4];
	UINT tmr;
	DSTATUS s;


	if (drv) return RES_NOTRDY;

	gUsDelay = SCK_SLOW_INIT;

	dly_us(10000);			/* 10ms */
	CS_INIT(); CS_H();		/* Initialize port pin tied to CS */
	CK_INIT(); CK_L();		/* Initialize port pin tied to SCLK */
	DI_INIT(); DI_H();				/* Initialize port pin tied to DI */
	DO_INIT();				/* Initialize port pin tied to DO */

	for (n = 10; n; n--) rcvr_mmc(buf, 1);	/* Apply 80 dummy clocks and the card gets ready to receive command */

	ty = 0;

	BYTE retCmd;

	n=5;
	do
	{
		retCmd = send_cmd(CMD0, 0);
		n--;
	}
	while(n && retCmd != 1);

	if (retCmd == 1)
	{
		/* Enter Idle state */
		if (send_cmd(CMD8, 0x1AA) == 1) {	/* SDv2? */
			rcvr_mmc(buf, 4);							/* Get trailing return value of R7 resp */
			if (buf[2] == 0x01 && buf[3] == 0xAA) {		/* The card can work at vdd range of 2.7-3.6V */
				for (tmr = 1000; tmr; tmr--) {			/* Wait for leaving idle state (ACMD41 with HCS bit) */
					if (send_cmd(ACMD41, 1UL << 30) == 0) break;
					dly_us(1000);
				}
				if (tmr && send_cmd(CMD58, 0) == 0) {	/* Check CCS bit in the OCR */
					rcvr_mmc(buf, 4);
					ty = (buf[0] & 0x40) ? CT_SD2 | CT_BLOCK : CT_SD2;	/* SDv2 */
				}
			}
		} else {							/* SDv1 or MMCv3 */
			if (send_cmd(ACMD41, 0) <= 1) 	{
				ty = CT_SD1; cmd = ACMD41;	/* SDv1 */
			} else {

				ty = CT_MMC; cmd = CMD1;	/* MMCv3 */
			}
			for (tmr = 1000; tmr; tmr--) {			/* Wait for leaving idle state */
				if (send_cmd(cmd, 0) == 0) break;
				dly_us(1000);
			}
			if (!tmr || send_cmd(CMD16, 512) != 0)	/* Set R/W block length to 512 */
			{
				ty = 0;
			}
		}
	}
	else
	{
		LOG(INFO, "SD_! %x", retCmd);
	}
	CardType = ty;
	s = ty ? 0 : STA_NOINIT;
	Stat = s;

	deselect();
	LOG(INFO, "SD init TYPE %d STAT %d\n", ty, s);

	gUsDelay = SCK_NORMAL;

	return s;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE drv,			/* Physical drive nmuber (0) */
	BYTE *buff,			/* Pointer to the data buffer to store read data */
	DWORD sector,		/* Start sector number (LBA) */
	UINT count			/* Sector count (1..128) */
)
{
	BYTE cmd;


	if (disk_status(drv) & STA_NOINIT) return RES_NOTRDY;
	if (!(CardType & CT_BLOCK)) sector *= 512;	/* Convert LBA to byte address if needed */

	cmd = count > 1 ? CMD18 : CMD17;			/*  READ_MULTIPLE_BLOCK : READ_SINGLE_BLOCK */
	if (send_cmd(cmd, sector) == 0) {
		do {
			if (!rcvr_datablock(buff, 512)) break;
			buff += 512;
		} while (--count);
		if (cmd == CMD18) send_cmd(CMD12, 0);	/* STOP_TRANSMISSION */
	}
	deselect();

	return count ? RES_ERROR : RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT disk_write (
	BYTE drv,			/* Physical drive nmuber (0) */
	const BYTE *buff,	/* Pointer to the data to be written */
	DWORD sector,		/* Start sector number (LBA) */
	UINT count			/* Sector count (1..128) */
)
{
	if (disk_status(drv) & STA_NOINIT) return RES_NOTRDY;
	if (!(CardType & CT_BLOCK)) sector *= 512;	/* Convert LBA to byte address if needed */

	if (count == 1) {	/* Single block write */
		if ((send_cmd(CMD24, sector) == 0)	/* WRITE_BLOCK */
			&& xmit_datablock(buff, 0xFE))
			count = 0;
	}
	else {				/* Multiple block write */
		if (CardType & CT_SDC) send_cmd(ACMD23, count);
		if (send_cmd(CMD25, sector) == 0) {	/* WRITE_MULTIPLE_BLOCK */
			do {
				if (!xmit_datablock(buff, 0xFC)) break;
				buff += 512;
			} while (--count);
			if (!xmit_datablock(0, 0xFD))	/* STOP_TRAN token */
				count = 1;
		}
	}
	deselect();

	return count ? RES_ERROR : RES_OK;
}


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE drv,		/* Physical drive nmuber (0) */
	BYTE ctrl,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;
	BYTE n, csd[16];
	DWORD cs;


	if (disk_status(drv) & STA_NOINIT) return RES_NOTRDY;	/* Check if card is in the socket */

	res = RES_ERROR;
	switch (ctrl) {
		case CTRL_SYNC :		/* Make sure that no pending write process */
			if (select()) res = RES_OK;
			break;

		case GET_SECTOR_COUNT :	/* Get number of sectors on the disk (DWORD) */
			if ((send_cmd(CMD9, 0) == 0) && rcvr_datablock(csd, 16)) {
				if ((csd[0] >> 6) == 1) {	/* SDC ver 2.00 */
					cs = csd[9] + ((WORD)csd[8] << 8) + ((DWORD)(csd[7] & 63) << 16) + 1;
					*(DWORD*)buff = cs << 10;
				} else {					/* SDC ver 1.XX or MMC */
					n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
					cs = (csd[8] >> 6) + ((WORD)csd[7] << 2) + ((WORD)(csd[6] & 3) << 10) + 1;
					*(DWORD*)buff = cs << (n - 9);
				}
				res = RES_OK;
			}
			break;

		case GET_BLOCK_SIZE :	/* Get erase block size in unit of sector (DWORD) */
			*(DWORD*)buff = 128;
			res = RES_OK;
			break;

		default:
			res = RES_PARERR;
	}

	deselect();

	return res;
}



