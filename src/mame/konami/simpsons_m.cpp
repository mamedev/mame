// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
#include "emu.h"

#include "cpu/m6809/konami.h"
#include "machine/eepromser.h"
#include "simpsons.h"

/***************************************************************************

  EEPROM

***************************************************************************/

void simpsons_state::eeprom_w(uint8_t data)
{
	if (data == 0xff)
		return;

	ioport("EEPROMOUT")->write(data, 0xff);

	video_bank_select(data & 0x03);

	m_firq_enabled = data & 0x04;
	if (!m_firq_enabled)
		m_maincpu->set_input_line(KONAMI_FIRQ_LINE, CLEAR_LINE);
}

/***************************************************************************

  Coin Counters, Sound Interface

***************************************************************************/

void simpsons_state::coin_counter_w(uint8_t data)
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

uint8_t simpsons_state::sound_interrupt_r()
{
	if (!machine().side_effects_disabled())
		m_audiocpu->set_input_line_and_vector(0, HOLD_LINE, 0xff); // Z80

	return 0x00;
}


/***************************************************************************

  Banking, initialization

***************************************************************************/

void simpsons_state::banking_callback(u8 data)
{
	membank("bank1")->set_entry(data & 0x3f);
}

void simpsons_state::machine_start()
{
	m_spriteram = make_unique_clear<uint16_t[]>(0x1000 / 2);

	membank("bank1")->configure_entries(0, 64, memregion("maincpu")->base(), 0x2000);

	membank("bank2")->configure_entries(0, 2, memregion("audiocpu")->base() + 0x10000, 0);
	membank("bank2")->configure_entries(2, 6, memregion("audiocpu")->base() + 0x10000, 0x4000);

	save_item(NAME(m_firq_enabled));
	save_item(NAME(m_sprite_colorbase));
	save_item(NAME(m_layer_colorbase));
	save_item(NAME(m_layerpri));
	save_pointer(NAME(m_spriteram), 0x1000 / 2);

	m_dma_start_timer = timer_alloc(FUNC(simpsons_state::dma_start), this);
	m_dma_end_timer = timer_alloc(FUNC(simpsons_state::dma_end), this);
	m_nmi_blocked = timer_alloc(timer_expired_delegate());
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
	video_bank_select(0);

	m_dma_start_timer->adjust(attotime::never);
	m_dma_end_timer->adjust(attotime::never);

	// Z80 _NMI goes low at same time as reset
	m_audiocpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	m_audiocpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
}
