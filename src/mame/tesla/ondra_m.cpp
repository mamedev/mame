// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Ondra driver by Miodrag Milanovic

        08/09/2008 Preliminary driver.

****************************************************************************/


#include "emu.h"
#include "ondra.h"


u8 ondra_state::keyboard_r(offs_t offset)
{
	u8 data = 0x60;
	offset &= 15;

	data |= (m_cassette->input() < 0.00) ? 0x80: 0;

	if (offset < m_io_keyboard.size())
		data |= (m_io_keyboard[offset]->read() & 0x1f);
	else
		data |= 0x1f;

	return data;
}

void ondra_state::update_banks()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	u8 *m = m_rom->base();
	u8 *r = m_ram->pointer();

	if (BIT(m_bank_status, 0) != BIT(m_bank_old, 0))
	{
		if (BIT(m_bank_status, 0))
		{
			space.install_write_bank(0x0000, 0x3fff, m_bank1);
			m_bank1->set_base(r);
		}
		else
		{
			space.unmap_write(0x0000, 0x3fff);
			m_bank1->set_base(m);
		}
	}

	if (BIT(m_bank_status, 1) != BIT(m_bank_old, 1))
	{
		if (BIT(m_bank_status, 1))
		{
			space.unmap_write(0xe000, 0xffff);
			space.install_read_handler (0xe000, 0xffff, read8sm_delegate(*this, FUNC(ondra_state::keyboard_r)));
		}
		else
		{
			space.install_readwrite_bank(0xe000, 0xffff, m_bank3);
			m_bank3->set_base(r + 0xe000);
		}
	}

	m_bank_old = m_bank_status;
}

/*
0 - video on/off
1 - banking
2 - banking
3 - cassette out
4 - A0 on pits
5 - A1 on pits */
void ondra_state::port03_w(u8 data)
{
	if (BIT(data, 1, 2) != m_bank_status)
	{
		m_bank_status = BIT(data, 1, 2);
		update_banks();
	}

	m_video_enable = BIT(data, 0);
	m_cassette->output(BIT(data, 3) ? -1.0 : +1.0);
}


// external connection
u8 ondra_state::port09_r()
{
	return 0xff;
}

/*
0 - a LED next to keyboard
1 - a LED next to keyboard
2 - external
3 - external
4 - cassette relay
5 - speaker
6 - speaker
7 - speaker */
void ondra_state::port0a_w(u8 data)
{
	static u16 tones[8] = { 0, 110, 156, 220, 311, 440, 622, 880 };  // a guess

	m_cassette->change_state(BIT(data,4) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);
	u16 tone = tones[BIT(data, 5, 3)];
	m_beep->set_state(tone? 1 : 0);
	if (tone)
		m_beep->set_clock(tone);
}

void ondra_state::machine_reset()
{
	m_beep->set_state(0);
	m_video_enable = 0;
	m_bank_status = 0;
	m_bank_old = 0xff;
	update_banks();
}

void ondra_state::machine_start()
{
	save_item(NAME(m_video_enable));
	save_item(NAME(m_bank_status));
	save_item(NAME(m_bank_old));
	membank("bank2")->set_base(m_ram->pointer() + 0x4000);
}
