/***************************************************************************

  snesobc1.c

  File to handle emulation of the SNES "OBC-1" add-on chip.

  Original C++ code by byuu.
  Byuu's code is released under GNU General Public License
  version 2 as published by the Free Software Foundation.
  The implementation below is released under the MAME license
  for use in MAME, MESS and derivatives by permission of the author.

***************************************************************************/

struct snes_obc1_state
{
	int address;
	int offset;
	int shift;
};

static snes_obc1_state obc1_state;


static READ8_HANDLER( obc1_read )
{
	UINT16 address = offset & 0x1fff;
	UINT8 value;

	switch (address)
	{
		case 0x1ff0:
			value = snes_ram[obc1_state.offset + (obc1_state.address << 2) + 0];
			break;

		case 0x1ff1:
			value = snes_ram[obc1_state.offset + (obc1_state.address << 2) + 1];
			break;

		case 0x1ff2:
			value = snes_ram[obc1_state.offset + (obc1_state.address << 2) + 2];
			break;

		case 0x1ff3:
			value = snes_ram[obc1_state.offset + (obc1_state.address << 2) + 3];
			break;

		case 0x1ff4:
			value = snes_ram[obc1_state.offset + (obc1_state.address >> 2) + 0x200];
			break;

		default:
			value = snes_ram[address];
			break;
	}

	return value;
}


static WRITE8_HANDLER( obc1_write )
{
	UINT16 address = offset & 0x1fff;
	UINT8 temp;

	switch(address)
	{
		case 0x1ff0:
			snes_ram[obc1_state.offset + (obc1_state.address << 2) + 0] = data;
			break;

		case 0x1ff1:
			snes_ram[obc1_state.offset + (obc1_state.address << 2) + 1] = data;
			break;

		case 0x1ff2:
			snes_ram[obc1_state.offset + (obc1_state.address << 2) + 2] = data;
			break;

		case 0x1ff3:
			snes_ram[obc1_state.offset + (obc1_state.address << 2) + 3] = data;
			break;

		case 0x1ff4:
			temp = snes_ram[obc1_state.offset + (obc1_state.address >> 2) + 0x200];
			temp = (temp & ~(3 << obc1_state.shift)) | ((data & 0x03) << obc1_state.shift);
			snes_ram[obc1_state.offset + (obc1_state.address >> 2) + 0x200] = temp;
			break;

		case 0x1ff5:
			obc1_state.offset = (data & 0x01) ? 0x1800 : 0x1c00;
			snes_ram[address & 0x1fff] = data;
			break;

		case 0x1ff6:
			obc1_state.address = data & 0x7f;
			obc1_state.shift = (data & 0x03) << 1;
			snes_ram[address & 0x1fff] = data;
			break;

		default:
			snes_ram[address & 0x1fff] = data;
			break;
	}
}

static void obc1_init( running_machine &machine )
{
	obc1_state.offset  = (snes_ram[0x1ff5] & 0x01) ? 0x1800 : 0x1c00;
	obc1_state.address = (snes_ram[0x1ff6] & 0x7f);
	obc1_state.shift   = (snes_ram[0x1ff6] & 0x03) << 1;

	state_save_register_global(machine, obc1_state.offset);
	state_save_register_global(machine, obc1_state.address);
	state_save_register_global(machine, obc1_state.shift);
}
