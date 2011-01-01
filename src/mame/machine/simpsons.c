#include "emu.h"
#include "video/konicdev.h"
#include "cpu/konami/konami.h"
#include "machine/eeprom.h"
#include "sound/k053260.h"
#include "includes/simpsons.h"

/***************************************************************************

  EEPROM

***************************************************************************/

WRITE8_HANDLER( simpsons_eeprom_w )
{
	simpsons_state *state = space->machine->driver_data<simpsons_state>();

	if (data == 0xff)
		return;

	input_port_write(space->machine, "EEPROMOUT", data, 0xff);

	state->video_bank = data & 0x03;
	simpsons_video_banking(space->machine, state->video_bank);

	state->firq_enabled = data & 0x04;
}

/***************************************************************************

  Coin Counters, Sound Interface

***************************************************************************/

WRITE8_HANDLER( simpsons_coin_counter_w )
{
	simpsons_state *state = space->machine->driver_data<simpsons_state>();

	/* bit 0,1 coin counters */
	coin_counter_w(space->machine, 0, data & 0x01);
	coin_counter_w(space->machine, 1, data & 0x02);
	/* bit 2 selects mono or stereo sound */
	/* bit 3 = enable char ROM reading through the video RAM */
	k052109_set_rmrd_line(state->k052109, (data & 0x08) ? ASSERT_LINE : CLEAR_LINE);
	/* bit 4 = INIT (unknown) */
	/* bit 5 = enable sprite ROM reading */
	k053246_set_objcha_line(state->k053246, (~data & 0x20) ? ASSERT_LINE : CLEAR_LINE);
}

READ8_HANDLER( simpsons_sound_interrupt_r )
{
	simpsons_state *state = space->machine->driver_data<simpsons_state>();
	cpu_set_input_line_and_vector(state->audiocpu, 0, HOLD_LINE, 0xff );
	return 0x00;
}

READ8_DEVICE_HANDLER( simpsons_sound_r )
{
	return k053260_r(device, 2 + offset);
}


/***************************************************************************

  Banking, initialization

***************************************************************************/

static KONAMI_SETLINES_CALLBACK( simpsons_banking )
{
	memory_set_bank(device->machine, "bank1", lines & 0x3f);
}

static STATE_POSTLOAD( simpsons_postload )
{
	simpsons_state *state = machine->driver_data<simpsons_state>();

	simpsons_video_banking(machine, state->video_bank);
}

MACHINE_START( simpsons )
{
	simpsons_state *state = machine->driver_data<simpsons_state>();

	machine->generic.paletteram.u8 = auto_alloc_array_clear(machine, UINT8, 0x1000);
	state->xtraram = auto_alloc_array_clear(machine, UINT8, 0x1000);
	state->spriteram = auto_alloc_array_clear(machine, UINT16, 0x1000 / 2);

	state->maincpu = machine->device("maincpu");
	state->audiocpu = machine->device("audiocpu");
	state->k053260 = machine->device("k053260");
	state->k052109 = machine->device("k052109");
	state->k053246 = machine->device("k053246");
	state->k053251 = machine->device("k053251");

	state_save_register_global(machine, state->firq_enabled);
	state_save_register_global(machine, state->video_bank);
	state_save_register_global(machine, state->sprite_colorbase);
	state_save_register_global_array(machine, state->layer_colorbase);
	state_save_register_global_array(machine, state->layerpri);
	state_save_register_global_pointer(machine, machine->generic.paletteram.u8, 0x1000);
	state_save_register_global_pointer(machine, state->xtraram, 0x1000);
	state_save_register_global_pointer(machine, state->spriteram, 0x1000 / 2);
	state_save_register_postload(machine, simpsons_postload, NULL);
}

MACHINE_RESET( simpsons )
{
	simpsons_state *state = machine->driver_data<simpsons_state>();
	int i;

	konami_configure_set_lines(machine->device("maincpu"), simpsons_banking);

	for (i = 0; i < 3; i++)
	{
		state->layerpri[i] = 0;
		state->layer_colorbase[i] = 0;
	}

	state->sprite_colorbase = 0;
	state->firq_enabled = 0;
	state->video_bank = 0;

	/* init the default banks */
	memory_configure_bank(machine, "bank1", 0, 64, machine->region("maincpu")->base() + 0x10000, 0x2000);
	memory_set_bank(machine, "bank1", 0);

	memory_configure_bank(machine, "bank2", 0, 2, machine->region("audiocpu")->base() + 0x10000, 0);
	memory_configure_bank(machine, "bank2", 2, 6, machine->region("audiocpu")->base() + 0x10000, 0x4000);
	memory_set_bank(machine, "bank2", 0);

	simpsons_video_banking(machine, 0);
}
