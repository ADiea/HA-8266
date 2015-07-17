#include "drv/drvSDCard.h"
#include <SmingCore/SmingCore.h>
#include <Libraries/SDCard/SDCard.h>


void devSDCard_benchmark()
{
	FIL Fil;
	unsigned int bw;
	uint8_t buf[1024];

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
		LOG(INFO, "fopen FAIL %d \n", fRes);
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

uint8_t devSDCard_init(uint8_t operation)
{
	uint8_t retVal = DEV_ERR_OK;
	do
	{
		if(operation & ENABLE)
		{
			//init GPIO, enable device
			
			//configure device
			if(operation & CONFIG)
			{
				SDCard.begin();
			}
		}
		else
		{
			//deinit GPIO
		}
	}
	while(0);

	return retVal;
}




