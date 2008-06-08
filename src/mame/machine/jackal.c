/***************************************************************************

  machine.c

  Written by Kenneth Lin (kenneth_lin@ai.vancouver.bc.ca)

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

***************************************************************************/

#include "driver.h"
#include "cpu/m6809/m6809.h"

extern UINT8 jackal_interrupt_enable;
extern void jackal_mark_tile_dirty(int offset);

static UINT8 *jackal_rambank = 0;
static UINT8 *jackal_spritebank = 0;


MACHINE_RESET( jackal )
{
	// HACK: running at the nominal clock rate, music stops working
	// at the beginning of the game. This fixes it.
	cpunum_set_clockscale(machine, 1, 1.2f);

	memory_set_bankptr(1,&((memory_region(REGION_CPU1))[0x4000]));
 	jackal_rambank = &((memory_region(REGION_CPU1))[0]);
	jackal_spritebank = &((memory_region(REGION_CPU1))[0]);
}



READ8_HANDLER( jackal_zram_r )
{
	return jackal_rambank[0x0020+offset];
}


READ8_HANDLER( jackal_voram_r )
{
	return jackal_rambank[0x2000+offset];
}


READ8_HANDLER( jackal_spriteram_r )
{
	return jackal_spritebank[0x3000+offset];
}


WRITE8_HANDLER( jackal_rambank_w )
{
if (data & 0x04) popmessage("jackal_rambank_w %02x",data);
	coin_counter_w(0,data & 0x01);
	coin_counter_w(1,data & 0x02);
	jackal_spritebank = &((memory_region(REGION_CPU1))[((data & 0x08) << 13)]);
	jackal_rambank = &((memory_region(REGION_CPU1))[((data & 0x10) << 12)]);
	memory_set_bankptr(1,&((memory_region(REGION_CPU1))[((data & 0x20) << 11) + 0x4000]));
}


WRITE8_HANDLER( jackal_zram_w )
{
	jackal_rambank[0x0020+offset] = data;
}


WRITE8_HANDLER( jackal_voram_w )
{
	if ((offset & 0xF800) == 0)
	{
		jackal_mark_tile_dirty(offset & 0x3ff);
	}
	jackal_rambank[0x2000+offset] = data;
}


WRITE8_HANDLER( jackal_spriteram_w )
{
	jackal_spritebank[0x3000+offset] = data;
}
