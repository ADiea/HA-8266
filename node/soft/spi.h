#ifndef _SPI_H
#define _SPI_H

void spi_begin();
void spi_enable();
void spi_disable();
void spi_send(const uint8_t* buffer, uint32_t size);
void spi_recv(uint8_t* buffer, uint32_t size);

#endif /*_SPI_H*/