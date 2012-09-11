/*********************************************************************

    ace_ace.c

    Format code for Jupiter Ace snapshot files

*********************************************************************/

#include "emu.h"
#include "ace_ace.h"
#include "cpu/z80/z80.h"
#include "machine/ram.h"

/* Load in .ace files. These are memory images of 0x2000 to 0x7fff
   and compressed as follows:

   ED 00        : End marker
   ED <cnt> <byt>   : repeat <byt> count <cnt:1-240> times
   <byt>        : <byt>
*/

/******************************************************************************
 Snapshot Handling
******************************************************************************/

SNAPSHOT_LOAD( ace )
{
	cpu_device *cpu = image.device().machine().firstcpu;
	UINT8 *RAM = image.device().machine().root_device().memregion(cpu->tag())->base();
	address_space *space = cpu->space(AS_PROGRAM);
	unsigned char ace_repeat, ace_byte, loop;
	int done=0, ace_index=0x2000;

	if (image.device().machine().device<ram_device>(RAM_TAG)->size() < 16*1024)
	{
		image.seterror(IMAGE_ERROR_INVALIDIMAGE, "At least 16KB RAM expansion required");
		image.message("At least 16KB RAM expansion required");
		return IMAGE_INIT_FAIL;
	}

	logerror("Loading file %s.\r\n", image.filename());
	while (!done && (ace_index < 0x8001))
	{
		image.fread( &ace_byte, 1);
		if (ace_byte == 0xed)
		{
			image.fread(&ace_byte, 1);
			switch (ace_byte)
			{
			case 0x00:
					logerror("File loaded!\r\n");
					done = 1;
					break;
			case 0x01:
					image.fread(&ace_byte, 1);
					RAM[ace_index++] = ace_byte;
					break;
			default:
					image.fread(&ace_repeat, 1);
					for (loop = 0; loop < ace_byte; loop++)
						RAM[ace_index++] = ace_repeat;
					break;
			}
		}
		else
			RAM[ace_index++] = ace_byte;
	}

	logerror("Decoded %X bytes.\r\n", ace_index-0x2000);

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

	ace_index = RAM[0x2080] | (RAM[0x2081] << 8);

	if ((ace_index & 0x3FFF)==0)
	{
		cpu->set_state_int(Z80_AF, RAM[0x2100] | (RAM[0x2101] << 8));
		cpu->set_state_int(Z80_BC, RAM[0x2104] | (RAM[0x2105] << 8));
		cpu->set_state_int(Z80_DE, RAM[0x2108] | (RAM[0x2109] << 8));
		cpu->set_state_int(Z80_HL, RAM[0x210c] | (RAM[0x210d] << 8));
		cpu->set_state_int(Z80_IX, RAM[0x2110] | (RAM[0x2111] << 8));
		cpu->set_state_int(Z80_IY, RAM[0x2114] | (RAM[0x2115] << 8));
		cpu->set_pc(RAM[0x211c] | (RAM[0x211d] << 8));
		cpu->set_state_int(Z80_AF2, RAM[0x2120] | (RAM[0x2121] << 8));
		cpu->set_state_int(Z80_BC2, RAM[0x2124] | (RAM[0x2125] << 8));
		cpu->set_state_int(Z80_DE2, RAM[0x2128] | (RAM[0x2129] << 8));
		cpu->set_state_int(Z80_HL2, RAM[0x212c] | (RAM[0x212d] << 8));
		cpu->set_state_int(Z80_IM, RAM[0x2130]);
		cpu->set_state_int(Z80_IFF1, RAM[0x2134]);
		cpu->set_state_int(Z80_IFF2, RAM[0x2138]);
		cpu->set_state_int(Z80_I, RAM[0x213c]);
		cpu->set_state_int(Z80_R, RAM[0x2140]);

		if ((RAM[0x2119] < 0x80) || !ace_index)
			cpu->set_state_int(STATE_GENSP, RAM[0x2118] | (RAM[0x2119] << 8));
	}

	/* Copy data to the address space */
	for (ace_index = 0x2000; ace_index < 0x8000; ace_index++)
		space->write_byte(ace_index, RAM[ace_index]);

	return IMAGE_INIT_PASS;
}
