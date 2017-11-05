// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Bellfruit Common Hardware / Functions */
#include "emu.h"


static const uint16_t AddressDecode[]=
{
	0x0800,0x1000,0x0001,0x0004,0x0008,0x0020,0x0080,0x0200,
	0x0100,0x0040,0x0002,0x0010,0x0400,0x2000,0x4000,0x8000,
	0
};

static const uint8_t DataDecode[]=
{
	0x02,0x08,0x20,0x40,0x10,0x04,0x01,0x80,
	0
};


///////////////////////////////////////////////////////////////////////////
void bfm_decode_mainrom(running_machine &machine, const char *rom_region, uint8_t* codec_data)
{
	uint8_t *rom;

	rom = machine.root_device().memregion(rom_region)->base();

	{
		std::vector<uint8_t> tmp(0x10000);
		int i;

		memcpy(&tmp[0], rom, 0x10000);

		for ( i = 0; i < 256; i++ )
		{
			uint8_t data,pattern,newdata,*tab;
			data    = i;

			tab     = (uint8_t*)DataDecode;
			pattern = 0x01;
			newdata = 0;

			do
			{
				newdata |= data & pattern ? *tab : 0;
				pattern <<= 1;
			} while ( *(++tab) );

			codec_data[i] = newdata;
		}

		for ( int address = 0; address < 0x10000; address++)
		{
			int newaddress,pattern;
			uint16_t *tab;

			tab      = (uint16_t*)AddressDecode;
			pattern  = 0x0001;
			newaddress = 0;
			do
			{
				newaddress |= address & pattern ? *tab : 0;
				pattern <<= 1;
			} while ( *(++tab) );

			rom[newaddress] = codec_data[ tmp[address] ];
		}
	}
}
