/***************************************************************************

    Cyberball 68000 sound simulator

****************************************************************************/

#include "emu.h"
#include "sound/dac.h"
#include "sound/2151intf.h"
#include "machine/atarigen.h"
#include "includes/cyberbal.h"


static void update_sound_68k_interrupts(running_machine *machine);



void cyberbal_sound_reset(running_machine *machine)
{
	cyberbal_state *state = (cyberbal_state *)machine->driver_data;

	/* reset the sound system */
	state->bank_base = &memory_region(machine, "audiocpu")[0x10000];
	memory_set_bankptr(machine, "soundbank", &state->bank_base[0x0000]);
	state->fast_68k_int = state->io_68k_int = 0;
	state->sound_data_from_68k = state->sound_data_from_6502 = 0;
	state->sound_data_from_68k_ready = state->sound_data_from_6502_ready = 0;
}



/*************************************
 *
 *  6502 Sound Interface
 *
 *************************************/

READ8_HANDLER( cyberbal_special_port3_r )
{
	cyberbal_state *state = (cyberbal_state *)space->machine->driver_data;
	int temp = input_port_read(space->machine, "JSAII");
	if (!(input_port_read(space->machine, "IN0") & 0x8000)) temp ^= 0x80;
	if (state->atarigen.cpu_to_sound_ready) temp ^= 0x40;
	if (state->atarigen.sound_to_cpu_ready) temp ^= 0x20;
	return temp;
}


READ8_HANDLER( cyberbal_sound_6502_stat_r )
{
	cyberbal_state *state = (cyberbal_state *)space->machine->driver_data;
	int temp = 0xff;
	if (state->sound_data_from_6502_ready) temp ^= 0x80;
	if (state->sound_data_from_68k_ready) temp ^= 0x40;
	return temp;
}


WRITE8_HANDLER( cyberbal_sound_bank_select_w )
{
	cyberbal_state *state = (cyberbal_state *)space->machine->driver_data;
	memory_set_bankptr(space->machine, "soundbank", &state->bank_base[0x1000 * ((data >> 6) & 3)]);
	coin_counter_w(space->machine, 1, (data >> 5) & 1);
	coin_counter_w(space->machine, 0, (data >> 4) & 1);
	cputag_set_input_line(space->machine, "dac", INPUT_LINE_RESET, (data & 0x08) ? CLEAR_LINE : ASSERT_LINE);
	if (!(data & 0x01)) devtag_reset(space->machine, "ymsnd");
}


READ8_HANDLER( cyberbal_sound_68k_6502_r )
{
	cyberbal_state *state = (cyberbal_state *)space->machine->driver_data;
	state->sound_data_from_68k_ready = 0;
	return state->sound_data_from_68k;
}


WRITE8_HANDLER( cyberbal_sound_68k_6502_w )
{
	cyberbal_state *state = (cyberbal_state *)space->machine->driver_data;

	state->sound_data_from_6502 = data;
	state->sound_data_from_6502_ready = 1;

	if (!state->io_68k_int)
	{
		state->io_68k_int = 1;
		update_sound_68k_interrupts(space->machine);
	}
}



/*************************************
 *
 *  68000 Sound Interface
 *
 *************************************/

static void update_sound_68k_interrupts(running_machine *machine)
{
	cyberbal_state *state = (cyberbal_state *)machine->driver_data;
	cputag_set_input_line(machine, "dac", 6, state->fast_68k_int ? ASSERT_LINE : CLEAR_LINE);
	cputag_set_input_line(machine, "dac", 2, state->io_68k_int   ? ASSERT_LINE : CLEAR_LINE);
}


INTERRUPT_GEN( cyberbal_sound_68k_irq_gen )
{
	cyberbal_state *state = (cyberbal_state *)device->machine->driver_data;
	if (!state->fast_68k_int)
	{
		state->fast_68k_int = 1;
		update_sound_68k_interrupts(device->machine);
	}
}


WRITE16_HANDLER( cyberbal_io_68k_irq_ack_w )
{
	cyberbal_state *state = (cyberbal_state *)space->machine->driver_data;
	if (state->io_68k_int)
	{
		state->io_68k_int = 0;
		update_sound_68k_interrupts(space->machine);
	}
}


READ16_HANDLER( cyberbal_sound_68k_r )
{
	cyberbal_state *state = (cyberbal_state *)space->machine->driver_data;
	int temp = (state->sound_data_from_6502 << 8) | 0xff;

	state->sound_data_from_6502_ready = 0;

	if (state->sound_data_from_6502_ready) temp ^= 0x08;
	if (state->sound_data_from_68k_ready) temp ^= 0x04;
	return temp;
}


WRITE16_HANDLER( cyberbal_sound_68k_w )
{
	cyberbal_state *state = (cyberbal_state *)space->machine->driver_data;
	if (ACCESSING_BITS_8_15)
	{
		state->sound_data_from_68k = (data >> 8) & 0xff;
		state->sound_data_from_68k_ready = 1;
	}
}


WRITE16_HANDLER( cyberbal_sound_68k_dac_w )
{
	cyberbal_state *state = (cyberbal_state *)space->machine->driver_data;
	const device_config *dac = devtag_get_device(space->machine, (offset & 8) ? "dac2" : "dac1");
	dac_data_16_w(dac, (((data >> 3) & 0x800) | ((data >> 2) & 0x7ff)) << 4);

	if (state->fast_68k_int)
	{
		state->fast_68k_int = 0;
		update_sound_68k_interrupts(space->machine);
	}
}
