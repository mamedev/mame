/***************************************************************************

    Cyberball 68000 sound simulator

****************************************************************************/

#include "driver.h"
#include "machine/atarigen.h"
#include "sound/dac.h"
#include "includes/cyberbal.h"
#include <math.h>


static UINT8 *bank_base;
static UINT8 fast_68k_int, io_68k_int;
static UINT8 sound_data_from_68k, sound_data_from_6502;
static UINT8 sound_data_from_68k_ready, sound_data_from_6502_ready;


static void update_sound_68k_interrupts(void);



void cyberbal_sound_reset(void)
{
	/* reset the sound system */
	bank_base = &memory_region(REGION_CPU2)[0x10000];
	memory_set_bankptr(8, &bank_base[0x0000]);
	fast_68k_int = io_68k_int = 0;
	sound_data_from_68k = sound_data_from_6502 = 0;
	sound_data_from_68k_ready = sound_data_from_6502_ready = 0;
}



/*************************************
 *
 *  6502 Sound Interface
 *
 *************************************/

READ8_HANDLER( cyberbal_special_port3_r )
{
	int temp = readinputport(3);
	if (!(readinputport(0) & 0x8000)) temp ^= 0x80;
	if (atarigen_cpu_to_sound_ready) temp ^= 0x40;
	if (atarigen_sound_to_cpu_ready) temp ^= 0x20;
	return temp;
}


READ8_HANDLER( cyberbal_sound_6502_stat_r )
{
	int temp = 0xff;
	if (sound_data_from_6502_ready) temp ^= 0x80;
	if (sound_data_from_68k_ready) temp ^= 0x40;
	return temp;
}


WRITE8_HANDLER( cyberbal_sound_bank_select_w )
{
	memory_set_bankptr(8, &bank_base[0x1000 * ((data >> 6) & 3)]);
	coin_counter_w(1, (data >> 5) & 1);
	coin_counter_w(0, (data >> 4) & 1);
	cpunum_set_input_line(3, INPUT_LINE_RESET, (data & 0x08) ? CLEAR_LINE : ASSERT_LINE);
	if (!(data & 0x01)) sndti_reset(SOUND_YM2151, 0);

}


READ8_HANDLER( cyberbal_sound_68k_6502_r )
{
	sound_data_from_68k_ready = 0;
	return sound_data_from_68k;
}


WRITE8_HANDLER( cyberbal_sound_68k_6502_w )
{
	sound_data_from_6502 = data;
	sound_data_from_6502_ready = 1;

	if (!io_68k_int)
	{
		io_68k_int = 1;
		update_sound_68k_interrupts();
	}
}



/*************************************
 *
 *  68000 Sound Interface
 *
 *************************************/

static void update_sound_68k_interrupts(void)
{
	int newstate = 0;

	if (fast_68k_int)
		newstate |= 6;
	if (io_68k_int)
		newstate |= 2;

	if (newstate)
		cpunum_set_input_line(3, newstate, ASSERT_LINE);
	else
		cpunum_set_input_line(3, 7, CLEAR_LINE);
}


INTERRUPT_GEN( cyberbal_sound_68k_irq_gen )
{
	if (!fast_68k_int)
	{
		fast_68k_int = 1;
		update_sound_68k_interrupts();
	}
}


WRITE16_HANDLER( cyberbal_io_68k_irq_ack_w )
{
	if (io_68k_int)
	{
		io_68k_int = 0;
		update_sound_68k_interrupts();
	}
}


READ16_HANDLER( cyberbal_sound_68k_r )
{
	int temp = (sound_data_from_6502 << 8) | 0xff;

	sound_data_from_6502_ready = 0;

	if (sound_data_from_6502_ready) temp ^= 0x08;
	if (sound_data_from_68k_ready) temp ^= 0x04;
	return temp;
}


WRITE16_HANDLER( cyberbal_sound_68k_w )
{
	if (ACCESSING_MSB)
	{
		sound_data_from_68k = (data >> 8) & 0xff;
		sound_data_from_68k_ready = 1;
	}
}


WRITE16_HANDLER( cyberbal_sound_68k_dac_w )
{
	DAC_data_16_w((offset >> 3) & 1, (((data >> 3) & 0x800) | ((data >> 2) & 0x7ff)) << 4);

	if (fast_68k_int)
	{
		fast_68k_int = 0;
		update_sound_68k_interrupts();
	}
}
