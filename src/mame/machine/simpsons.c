#include "emu.h"
#include "video/konicdev.h"
#include "cpu/konami/konami.h"
#include "machine/eeprom.h"
#include "sound/k053260.h"
#include "includes/simpsons.h"

/***************************************************************************

  EEPROM

***************************************************************************/

WRITE8_MEMBER(simpsons_state::simpsons_eeprom_w)
{

	if (data == 0xff)
		return;

	input_port_write(machine(), "EEPROMOUT", data, 0xff);

	m_video_bank = data & 0x03;
	simpsons_video_banking(machine(), m_video_bank);

	m_firq_enabled = data & 0x04;
}

/***************************************************************************

  Coin Counters, Sound Interface

***************************************************************************/

WRITE8_MEMBER(simpsons_state::simpsons_coin_counter_w)
{

	/* bit 0,1 coin counters */
	coin_counter_w(machine(), 0, data & 0x01);
	coin_counter_w(machine(), 1, data & 0x02);
	/* bit 2 selects mono or stereo sound */
	/* bit 3 = enable char ROM reading through the video RAM */
	k052109_set_rmrd_line(m_k052109, (data & 0x08) ? ASSERT_LINE : CLEAR_LINE);
	/* bit 4 = INIT (unknown) */
	/* bit 5 = enable sprite ROM reading */
	k053246_set_objcha_line(m_k053246, (~data & 0x20) ? ASSERT_LINE : CLEAR_LINE);
}

READ8_MEMBER(simpsons_state::simpsons_sound_interrupt_r)
{
	device_set_input_line_and_vector(m_audiocpu, 0, HOLD_LINE, 0xff );
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
	memory_set_bank(device->machine(), "bank1", lines & 0x3f);
}

static void simpsons_postload(running_machine &machine)
{
	simpsons_state *state = machine.driver_data<simpsons_state>();

	simpsons_video_banking(machine, state->m_video_bank);
}

MACHINE_START( simpsons )
{
	simpsons_state *state = machine.driver_data<simpsons_state>();

	state->m_generic_paletteram_8.allocate(0x1000);
	state->m_xtraram = auto_alloc_array_clear(machine, UINT8, 0x1000);
	state->m_spriteram = auto_alloc_array_clear(machine, UINT16, 0x1000 / 2);

	state->m_maincpu = machine.device("maincpu");
	state->m_audiocpu = machine.device("audiocpu");
	state->m_k053260 = machine.device("k053260");
	state->m_k052109 = machine.device("k052109");
	state->m_k053246 = machine.device("k053246");
	state->m_k053251 = machine.device("k053251");

	state->save_item(NAME(state->m_firq_enabled));
	state->save_item(NAME(state->m_video_bank));
	state->save_item(NAME(state->m_sprite_colorbase));
	state->save_item(NAME(state->m_layer_colorbase));
	state->save_item(NAME(state->m_layerpri));
	state->save_pointer(NAME(state->m_xtraram), 0x1000);
	state->save_pointer(NAME(state->m_spriteram), 0x1000 / 2);
	machine.save().register_postload(save_prepost_delegate(FUNC(simpsons_postload), &machine));
}

MACHINE_RESET( simpsons )
{
	simpsons_state *state = machine.driver_data<simpsons_state>();
	int i;

	konami_configure_set_lines(machine.device("maincpu"), simpsons_banking);

	for (i = 0; i < 3; i++)
	{
		state->m_layerpri[i] = 0;
		state->m_layer_colorbase[i] = 0;
	}

	state->m_sprite_colorbase = 0;
	state->m_firq_enabled = 0;
	state->m_video_bank = 0;

	/* init the default banks */
	memory_configure_bank(machine, "bank1", 0, 64, machine.region("maincpu")->base() + 0x10000, 0x2000);
	memory_set_bank(machine, "bank1", 0);

	memory_configure_bank(machine, "bank2", 0, 2, machine.region("audiocpu")->base() + 0x10000, 0);
	memory_configure_bank(machine, "bank2", 2, 6, machine.region("audiocpu")->base() + 0x10000, 0x4000);
	memory_set_bank(machine, "bank2", 0);

	simpsons_video_banking(machine, 0);
}
