/***************************************************************************

  machine.c

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "imagedev/snapquik.h"
#include "includes/jupiter.h"



/* Load in .ace files. These are memory images of 0x2000 to 0x7fff
   and compressed as follows:

   ED 00        : End marker
   ED <cnt> <byt>   : repeat <byt> count <cnt:1-240> times
   <byt>        : <byt>
*/


/******************************************************************************
 Snapshot Handling
******************************************************************************/

SNAPSHOT_LOAD(jupiter)
{
	UINT8 *RAM = image.device().machine().root_device().memregion("maincpu")->base();
	device_t *cpu = image.device().machine().device("maincpu");
	address_space *space = image.device().machine().device("maincpu")->memory().space(AS_PROGRAM);
	unsigned char jupiter_repeat, jupiter_byte, loop;
	int done=0, jupiter_index=0x2000;

	if (space->machine().root_device().ioport("CFG")->read()==0)
	{
		image.seterror(IMAGE_ERROR_INVALIDIMAGE, "At least 16KB RAM expansion required");
		image.message("At least 16KB RAM expansion required");
		return IMAGE_INIT_FAIL;
	}

	logerror("Loading file %s.\r\n", image.filename());
	while (!done && (jupiter_index < 0x8001))
	{
		image.fread( &jupiter_byte, 1);
		if (jupiter_byte == 0xed)
		{
			image.fread(&jupiter_byte, 1);
			switch (jupiter_byte)
			{
			case 0x00:
					logerror("File loaded!\r\n");
					done = 1;
					break;
			case 0x01:
					image.fread(&jupiter_byte, 1);
					RAM[jupiter_index++] = jupiter_byte;
					break;
			default:
					image.fread(&jupiter_repeat, 1);
					for (loop = 0; loop < jupiter_byte; loop++)
						RAM[jupiter_index++] = jupiter_repeat;
					break;
			}
		}
		else
			RAM[jupiter_index++] = jupiter_byte;
	}

	logerror("Decoded %X bytes.\r\n", jupiter_index-0x2000);

	if (!done)
	{
		image.seterror(IMAGE_ERROR_INVALIDIMAGE, "EOF marker not found");
		image.message("EOF marker not found");
		return IMAGE_INIT_FAIL;
	}

        // patch CPU registers
        // Some games do not follow the standard, and have rubbish in the CPU area. So,
        // we check that some other bytes are correct.
        // 2080 = memory size of original machine, should be 0000 or 8000 or C000.
        // 2118 = new stack pointer, do not use if between 8000 and FF00.

	jupiter_index = RAM[0x2080] | (RAM[0x2081] << 8);

	if ((jupiter_index & 0x3FFF)==0)
	{
		cpu_set_reg(cpu, Z80_AF, RAM[0x2100] | (RAM[0x2101] << 8));
		cpu_set_reg(cpu, Z80_BC, RAM[0x2104] | (RAM[0x2105] << 8));
		cpu_set_reg(cpu, Z80_DE, RAM[0x2108] | (RAM[0x2109] << 8));
		cpu_set_reg(cpu, Z80_HL, RAM[0x210c] | (RAM[0x210d] << 8));
		cpu_set_reg(cpu, Z80_IX, RAM[0x2110] | (RAM[0x2111] << 8));
		cpu_set_reg(cpu, Z80_IY, RAM[0x2114] | (RAM[0x2115] << 8));
		cpu_set_reg(cpu, STATE_GENPC, RAM[0x211c] | (RAM[0x211d] << 8));
		cpu_set_reg(cpu, Z80_AF2, RAM[0x2120] | (RAM[0x2121] << 8));
		cpu_set_reg(cpu, Z80_BC2, RAM[0x2124] | (RAM[0x2125] << 8));
		cpu_set_reg(cpu, Z80_DE2, RAM[0x2128] | (RAM[0x2129] << 8));
		cpu_set_reg(cpu, Z80_HL2, RAM[0x212c] | (RAM[0x212d] << 8));
		cpu_set_reg(cpu, Z80_IM, RAM[0x2130]);
		cpu_set_reg(cpu, Z80_IFF1, RAM[0x2134]);
		cpu_set_reg(cpu, Z80_IFF2, RAM[0x2138]);
		cpu_set_reg(cpu, Z80_I, RAM[0x213c]);
		cpu_set_reg(cpu, Z80_R, RAM[0x2140]);

		if ((RAM[0x2119] < 0x80) || !jupiter_index)
			cpu_set_reg(cpu, STATE_GENSP, RAM[0x2118] | (RAM[0x2119] << 8));
	}

	/* Copy data to the address space */
	for (jupiter_index = 0x2000; jupiter_index < 0x8000; jupiter_index++)
		space->write_byte(jupiter_index, RAM[jupiter_index]);

	return IMAGE_INIT_PASS;
}

