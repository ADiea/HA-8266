#include "main.h"

#define SPI_PORT PORTB
#define SPI_PIN PINB
#define SPI_DDR DDRB

#define SPI_MOSI 3
#define SPI_MISO 4
#define SPI_SCK 5
#define SPI_TRUE_SS 2

#define SPI_PORT_SS PORTD
#define SPI_DDR_SS DDRD
#define SPI_SS 7

void spi_begin()
{
	SPI_DDR |= 1<<SPI_MOSI | 1<<SPI_SCK | 1<<SPI_TRUE_SS;
	SPI_PORT |= 1<<SPI_SS | 1<<SPI_MISO;
	
	SPI_DDR_SS |= 1<<SPI_SS;
	spi_disable();
	
	SPCR = (1<<SPE)|(1<<MSTR);
}

void spi_enable()
{
	SPI_PORT_SS &= ~(1<<SPI_SS);
}

void spi_disable()
{
	SPI_PORT_SS |= 1<<SPI_SS;
}

void spi_send(const uint8_t* buffer, uint32_t size)
{
	do 
	{
		SPDR = *buffer++;
		while(!(SPSR & (1<<SPIF) ));
	} 
	while (--size);
}

void spi_recv(uint8_t* buffer, uint32_t size)
{
	do
	{
		SPDR = 0xFF;
		while(!(SPSR & (1<<SPIF) ));
		*buffer++ = SPDR;
	}
	while (--size);
}