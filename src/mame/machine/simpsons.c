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

	ioport("EEPROMOUT")->write(data, 0xff);

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
	m_audiocpu->set_input_line_and_vector(0, HOLD_LINE, 0xff );
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
	device->machine().root_device().membank("bank1")->set_entry(lines & 0x3f);
}

static void simpsons_postload(running_machine &machine)
{
	simpsons_state *state = machine.driver_data<simpsons_state>();

	simpsons_video_banking(machine, state->m_video_bank);
}

void simpsons_state::machine_start()
{

	m_generic_paletteram_8.allocate(0x1000);
	m_xtraram = auto_alloc_array_clear(machine(), UINT8, 0x1000);
	m_spriteram = auto_alloc_array_clear(machine(), UINT16, 0x1000 / 2);

	m_maincpu = machine().device<cpu_device>("maincpu");
	m_audiocpu = machine().device<cpu_device>("audiocpu");
	m_k053260 = machine().device("k053260");
	m_k052109 = machine().device("k052109");
	m_k053246 = machine().device("k053246");
	m_k053251 = machine().device("k053251");

	save_item(NAME(m_firq_enabled));
	save_item(NAME(m_video_bank));
	save_item(NAME(m_sprite_colorbase));
	save_item(NAME(m_layer_colorbase));
	save_item(NAME(m_layerpri));
	save_pointer(NAME(m_xtraram), 0x1000);
	save_pointer(NAME(m_spriteram), 0x1000 / 2);
	machine().save().register_postload(save_prepost_delegate(FUNC(simpsons_postload), &machine()));
}

void simpsons_state::machine_reset()
{
	int i;

	konami_configure_set_lines(machine().device("maincpu"), simpsons_banking);

	for (i = 0; i < 3; i++)
	{
		m_layerpri[i] = 0;
		m_layer_colorbase[i] = 0;
	}

	m_sprite_colorbase = 0;
	m_firq_enabled = 0;
	m_video_bank = 0;

	/* init the default banks */
	membank("bank1")->configure_entries(0, 64, machine().root_device().memregion("maincpu")->base() + 0x10000, 0x2000);
	membank("bank1")->set_entry(0);

	membank("bank2")->configure_entries(0, 2, machine().root_device().memregion("audiocpu")->base() + 0x10000, 0);
	membank("bank2")->configure_entries(2, 6, machine().root_device().memregion("audiocpu")->base() + 0x10000, 0x4000);
	membank("bank2")->set_entry(0);

	simpsons_video_banking(machine(), 0);
}
