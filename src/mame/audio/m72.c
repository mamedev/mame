/***************************************************************************

IREM "M72" sound hardware

All games have a YM2151 for music, and most of them also samples. Samples
are not handled consistently by all the games, some use a high frequency NMI
handler to push them through a DAC, others use external hardware.
In the following table, the NMI column indicates with a No the games whose
NMI handler only consists of RETN. R-Type is an exception, it doesn't have
a valid NMI handler at all.

Game                                    Year  ID string     NMI
--------------------------------------  ----  ------------  ---
R-Type                                  1987  - (earlier version, no samples)
Battle Chopper / Mr. Heli               1987  Rev 2.20      Yes
Vigilante                               1988  Rev 2.20      Yes
Ninja Spirit                            1988  Rev 2.20      Yes
Image Fight                             1988  Rev 2.20      Yes
Legend of Hero Tonma                    1989  Rev 2.20      Yes
X Multiply                              1989  Rev 2.20      Yes
Dragon Breed                            1989  Rev 2.20      Yes
Kickle Cubicle                          1988  Rev 2.21      Yes
Shisensho                               1989  Rev 2.21      Yes
R-Type II                               1989  Rev 2.21      Yes
Major Title                             1990  Rev 2.21      Yes
Air Duel                                1990  Rev 3.14 M72   No
Daiku no Gensan                         1990  Rev 3.14 M81  Yes
Daiku no Gensan (M72)                   1990  Rev 3.15 M72   No
Hammerin' Harry                         1990  Rev 3.15 M81  Yes
Ken-Go                                  1991  Rev 3.15 M81  Yes
Pound for Pound                         1990  Rev 3.15 M83   No
Cosmic Cop                              1991  Rev 3.15 M81  Yes
Gallop - Armed Police Unit              1991  Rev 3.15 M72   No
Hasamu                                  1991  Rev 3.15 M81  Yes
Bomber Man                              1991  Rev 3.15 M81  Yes
Bomber Man World (Japan)                1992  Rev 3.31 M81  Yes
Bomber Man World (World) / Atomic Punk  1992  Rev 3.31 M99   No
Quiz F-1 1,2finish                      1992  Rev 3.33 M81  Yes
Risky Challenge                         1993  Rev 3.34 M81  Yes
Shisensho II                            1993  Rev 3.34 M81  Yes

***************************************************************************/

#include "driver.h"
#include "sound/dac.h"
#include "m72.h"


/*

  The sound CPU runs in interrup mode 0. IRQ is shared by two sources: the
  YM2151 (bit 4 of the vector), and the main CPU (bit 5).
  Since the vector can be changed from different contexts (the YM2151 timer
  callback, the main CPU context, and the sound CPU context), it's important
  to accurately arbitrate the changes to avoid out-of-order execution. We do
  that by handling all vector changes in a single timer callback.

*/


enum
{
	VECTOR_INIT,
	YM2151_ASSERT,
	YM2151_CLEAR,
	Z80_ASSERT,
	Z80_CLEAR
};

static UINT8 irqvector;
static UINT32 sample_addr;

static TIMER_CALLBACK( setvector_callback )
{
	switch(param)
	{
		case VECTOR_INIT:
			irqvector = 0xff;
			break;

		case YM2151_ASSERT:
			irqvector &= 0xef;
			break;

		case YM2151_CLEAR:
			irqvector |= 0x10;
			break;

		case Z80_ASSERT:
			irqvector &= 0xdf;
			break;

		case Z80_CLEAR:
			irqvector |= 0x20;
			break;
	}

	if (irqvector == 0)
		logerror("You didn't call m72_init_sound()\n");

	cpunum_set_input_line_vector(1,0,irqvector);
	if (irqvector == 0xff)	/* no IRQs pending */
		cpunum_set_input_line(1,0,CLEAR_LINE);
	else	/* IRQ pending */
		cpunum_set_input_line(1,0,ASSERT_LINE);
}

MACHINE_RESET( m72_sound )
{
	setvector_callback(machine, VECTOR_INIT);

	state_save_register_global(irqvector);
	state_save_register_global(sample_addr);
}

void m72_ym2151_irq_handler(int irq)
{
	if (irq)
		timer_call_after_resynch(YM2151_ASSERT,setvector_callback);
	else
		timer_call_after_resynch(YM2151_CLEAR,setvector_callback);
}

WRITE16_HANDLER( m72_sound_command_w )
{
	if (ACCESSING_LSB)
	{
		soundlatch_w(offset,data);
		timer_call_after_resynch(Z80_ASSERT,setvector_callback);
	}
}

WRITE8_HANDLER( m72_sound_command_byte_w )
{
	soundlatch_w(offset,data);
	timer_call_after_resynch(Z80_ASSERT,setvector_callback);
}

WRITE8_HANDLER( m72_sound_irq_ack_w )
{
	timer_call_after_resynch(Z80_CLEAR,setvector_callback);
}



void m72_set_sample_start(int start)
{
	sample_addr = start;
}

WRITE8_HANDLER( vigilant_sample_addr_w )
{
	if (offset == 1)
		sample_addr = (sample_addr & 0x00ff) | ((data << 8) & 0xff00);
	else
		sample_addr = (sample_addr & 0xff00) | ((data << 0) & 0x00ff);
}

WRITE8_HANDLER( shisen_sample_addr_w )
{
	sample_addr >>= 2;

	if (offset == 1)
		sample_addr = (sample_addr & 0x00ff) | ((data << 8) & 0xff00);
	else
		sample_addr = (sample_addr & 0xff00) | ((data << 0) & 0x00ff);

	sample_addr <<= 2;
}

WRITE8_HANDLER( rtype2_sample_addr_w )
{
	sample_addr >>= 5;

	if (offset == 1)
		sample_addr = (sample_addr & 0x00ff) | ((data << 8) & 0xff00);
	else
		sample_addr = (sample_addr & 0xff00) | ((data << 0) & 0x00ff);

	sample_addr <<= 5;
}

WRITE8_HANDLER( poundfor_sample_addr_w )
{
	/* poundfor writes both sample start and sample END - a first for Irem...
       we don't handle the end written here, 00 marks the sample end as usual. */
	if (offset > 1) return;

	sample_addr >>= 4;

	if (offset == 1)
		sample_addr = (sample_addr & 0x00ff) | ((data << 8) & 0xff00);
	else
		sample_addr = (sample_addr & 0xff00) | ((data << 0) & 0x00ff);

	sample_addr <<= 4;
}

READ8_HANDLER( m72_sample_r )
{
	return memory_region(REGION_SOUND1)[sample_addr];
}

WRITE8_HANDLER( m72_sample_w )
{
	DAC_signed_data_w(0,data);
	sample_addr = (sample_addr + 1) & (memory_region_length(REGION_SOUND1) - 1);
}
