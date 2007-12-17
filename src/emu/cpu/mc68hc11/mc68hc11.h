#ifndef _MC68HC11_H
#define _MC68HC11_H

#include "cpuintrf.h"

#ifdef MAME_DEBUG
offs_t hc11_disasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram);
#endif

void mc68hc11_get_info(UINT32 state, cpuinfo *info);


#define MC68HC11_IO_PORTA			0x00
#define MC68HC11_IO_PORTB			0x01
#define MC68HC11_IO_PORTC			0x02
#define MC68HC11_IO_PORTD			0x03
#define MC68HC11_IO_PORTE			0x04
#define MC68HC11_IO_PORTF			0x05
#define MC68HC11_IO_PORTG			0x06
#define MC68HC11_IO_PORTH			0x07
#define MC68HC11_IO_SPI1_DATA		0x08
#define MC68HC11_IO_SPI2_DATA		0x09
#define MC68HC11_IO_AD0				0x10
#define MC68HC11_IO_AD1				0x11
#define MC68HC11_IO_AD2				0x12
#define MC68HC11_IO_AD3				0x13
#define MC68HC11_IO_AD4				0x14
#define MC68HC11_IO_AD5				0x15
#define MC68HC11_IO_AD6				0x16
#define MC68HC11_IO_AD7				0x17

#endif
