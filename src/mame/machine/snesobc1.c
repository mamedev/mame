/***************************************************************************

  snesobc1.c

  File to handle emulation of the SNES "OBC-1" add-on chip.

  Original C++ code by byuu

  MAME/MESS C conversion by etabeta

***************************************************************************/

#include "includes/snes.h"

int obc1_address;
int obc1_offset;
int obc1_shift;

READ8_HANDLER( obc1_read )
{
	UINT16 address = offset & 0x1fff;
	UINT8 value;

	switch (address)
	{
		case 0x1ff0:
			value = snes_ram[obc1_offset + (obc1_address << 2) + 0];
			break;

		case 0x1ff1:
			value = snes_ram[obc1_offset + (obc1_address << 2) + 1];
			break;

		case 0x1ff2:
			value = snes_ram[obc1_offset + (obc1_address << 2) + 2];
			break;

		case 0x1ff3:
			value = snes_ram[obc1_offset + (obc1_address << 2) + 3];
			break;

		case 0x1ff4:
			value = snes_ram[obc1_offset + (obc1_address >> 2) + 0x200];
			break;

		default:
			value = snes_ram[address];
			break;
	}

	return value;
}


WRITE8_HANDLER( obc1_write )
{
	UINT16 address = offset & 0x1fff;
	UINT8 temp;

	switch(address) 
	{
		case 0x1ff0:
			snes_ram[obc1_offset + (obc1_address << 2) + 0] = data;
			break;

		case 0x1ff1:
			snes_ram[obc1_offset + (obc1_address << 2) + 1] = data;
			break;

		case 0x1ff2:
			snes_ram[obc1_offset + (obc1_address << 2) + 2] = data;
			break;

		case 0x1ff3:
			snes_ram[obc1_offset + (obc1_address << 2) + 3] = data;
			break;

		case 0x1ff4:
			temp = snes_ram[obc1_offset + (obc1_address >> 2) + 0x200];
			temp = (temp & ~(3 << obc1_shift)) | ((data & 0x03) << obc1_shift);
			snes_ram[obc1_offset + (obc1_address >> 2) + 0x200] = temp;
			break;

		case 0x1ff5:
			obc1_offset = (data & 0x01) ? 0x1800 : 0x1c00;
			snes_ram[address & 0x1fff] = data;
			break;

		case 0x1ff6:
			obc1_address = data & 0x7f;
			obc1_shift = (data & 0x03) << 1;
			snes_ram[address & 0x1fff] = data;
			break;

		default:
			snes_ram[address & 0x1fff] = data;
			break;
	}
}

void obc1_init( void )
{
	obc1_offset  = (snes_ram[0x1ff5] & 0x01) ? 0x1800 : 0x1c00;
	obc1_address = (snes_ram[0x1ff6] & 0x7f);
	obc1_shift   = (snes_ram[0x1ff6] & 0x03) << 1;
}
