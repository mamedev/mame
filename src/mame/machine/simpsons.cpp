// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
#include "emu.h"

#include "cpu/m6809/konami.h"
#include "machine/eepromser.h"
#include "includes/simpsons.h"

/***************************************************************************

  EEPROM

***************************************************************************/

WRITE8_MEMBER(simpsons_state::simpsons_eeprom_w)
{
	if (data == 0xff)
		return;

	ioport("EEPROMOUT")->write(data, 0xff);

	simpsons_video_banking(data & 0x03);

	m_firq_enabled = data & 0x04;
}

/***************************************************************************

  Coin Counters, Sound Interface

***************************************************************************/

WRITE8_MEMBER(simpsons_state::simpsons_coin_counter_w)
{
	/* bit 0,1 coin counters */
	machine().bookkeeping().coin_counter_w(0, data & 0x01);
	machine().bookkeeping().coin_counter_w(1, data & 0x02);
	/* bit 2 selects mono or stereo sound */
	/* bit 3 = enable char ROM reading through the video RAM */
	m_k052109->set_rmrd_line((data & 0x08) ? ASSERT_LINE : CLEAR_LINE);
	/* bit 4 = INIT (unknown) */
	/* bit 5 = enable sprite ROM reading */
	m_k053246->k053246_set_objcha_line((~data & 0x20) ? ASSERT_LINE : CLEAR_LINE);
}

READ8_MEMBER(simpsons_state::simpsons_sound_interrupt_r)
{
	m_audiocpu->set_input_line_and_vector(0, HOLD_LINE, 0xff );
	return 0x00;
}


/***************************************************************************

  Banking, initialization

***************************************************************************/

WRITE8_MEMBER( simpsons_state::banking_callback )
{
	membank("bank1")->set_entry(data & 0x3f);
}

void simpsons_state::machine_start()
{
	m_spriteram = make_unique_clear<UINT16[]>(0x1000 / 2);

	membank("bank1")->configure_entries(0, 64, memregion("maincpu")->base(), 0x2000);

	membank("bank2")->configure_entries(0, 2, memregion("audiocpu")->base() + 0x10000, 0);
	membank("bank2")->configure_entries(2, 6, memregion("audiocpu")->base() + 0x10000, 0x4000);

	save_item(NAME(m_firq_enabled));
	save_item(NAME(m_sprite_colorbase));
	save_item(NAME(m_layer_colorbase));
	save_item(NAME(m_layerpri));
	save_pointer(NAME(m_spriteram.get()), 0x1000 / 2);
}

void simpsons_state::machine_reset()
{
	for (int i = 0; i < 3; i++)
	{
		m_layerpri[i] = 0;
		m_layer_colorbase[i] = 0;
	}

	m_sprite_colorbase = 0;
	m_firq_enabled = 0;

	/* init the default banks */
	membank("bank1")->set_entry(0);
	membank("bank2")->set_entry(0);
	simpsons_video_banking(0);
}
