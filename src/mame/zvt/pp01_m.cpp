// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        PP-01 driver by Miodrag Milanovic

        08/09/2008 Preliminary driver.

****************************************************************************/


#include "emu.h"
#include "pp01.h"


void pp01_state::video_write_mode_w(u8 data)
{
	m_video_write_mode = data & 0x0f;
}

void pp01_state::video_w(u8 block, u16 offset, u8 data, bool part)
{
	u16 const rgb_offset[3] = { 0x6000, 0xa000, 0xe000 };
	offset += part ? 0x1000 : 0x0000;
	u8 *const ram = m_ram->pointer();

	if (BIT(m_video_write_mode, 3))
	{
		// Copy mode
		for (int i = 0; i < 3; i++)
		{
			if (BIT(m_video_write_mode, i))
				ram[rgb_offset[i] + offset] = data;
			else
				ram[rgb_offset[i] + offset] = 0;
		}
	}
	else
	{
		ram[rgb_offset[block] + offset] = data;
	}
}

void pp01_state::video_r_1_w(offs_t offset, u8 data)
{
	video_w(0, offset, data, false);
}
void pp01_state::video_g_1_w(offs_t offset, u8 data)
{
	video_w(1, offset, data, false);
}
void pp01_state::video_b_1_w(offs_t offset, u8 data)
{
	video_w(2, offset, data, false);
}

void pp01_state::video_r_2_w(offs_t offset, u8 data)
{
	video_w(0, offset, data, true);
}
void pp01_state::video_g_2_w(offs_t offset, u8 data)
{
	video_w(1, offset, data, true);
}
void pp01_state::video_b_2_w(offs_t offset, u8 data)
{
	video_w(2, offset, data, true);
}


void pp01_state::set_memory(u8 block, u8 data)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	u16 const startaddr = block * 0x1000;
	u16 const endaddr   = startaddr + 0xfff;

	if (data >= 0xe0 && data <= 0xef)
	{
		// This is RAM
		space.install_read_bank(startaddr, endaddr, m_bank[block]);
		switch (data)
		{
		case 0xe6:
			space.install_write_handler(startaddr, endaddr, write8sm_delegate(*this, FUNC(pp01_state::video_r_1_w)));
			break;
		case 0xe7:
			space.install_write_handler(startaddr, endaddr, write8sm_delegate(*this, FUNC(pp01_state::video_r_2_w)));
			break;
		case 0xea:
			space.install_write_handler(startaddr, endaddr, write8sm_delegate(*this, FUNC(pp01_state::video_g_1_w)));
			break;
		case 0xeb:
			space.install_write_handler(startaddr, endaddr, write8sm_delegate(*this, FUNC(pp01_state::video_g_2_w)));
			break;
		case 0xee:
			space.install_write_handler(startaddr, endaddr, write8sm_delegate(*this, FUNC(pp01_state::video_b_1_w)));
			break;
		case 0xef:
			space.install_write_handler(startaddr, endaddr, write8sm_delegate(*this, FUNC(pp01_state::video_b_2_w)));
			break;
		default:
			space.install_write_bank(startaddr, endaddr, m_bank[block]);
			break;
		}
		m_bank[block]->set_base(m_ram->pointer() + (data & 0x0f) * 0x1000);
	}
	else if (data >= 0xfc)
	{
		space.install_read_bank(startaddr, endaddr, m_bank[block]);
		space.unmap_write(startaddr, endaddr);
		m_bank[block]->set_base(&m_mainrom[(data & 0x03) * 0x1000]);
	}
	else
	{
		logerror("set_memory %02x = %02x\n", block, data);
		space.unmap_readwrite(startaddr, endaddr);
	}
}

void pp01_state::mem_block_w(offs_t offset, u8 data)
{
	m_memory_block[offset] = data;
	set_memory(offset, data);
}

u8 pp01_state::mem_block_r(offs_t offset)
{
	return m_memory_block[offset];
}

void pp01_state::device_post_load()
{
	for (int i = 0; i < 16; i++)
	{
		set_memory(i, m_memory_block[i]);
	}
}

void pp01_state::machine_start()
{
	save_item(NAME(m_video_scroll));
	save_item(NAME(m_memory_block));
	save_item(NAME(m_video_write_mode));
	save_item(NAME(m_key_line));
	save_item(NAME(m_txe));
	save_item(NAME(m_txd));
	save_item(NAME(m_rts));
	save_item(NAME(m_casspol));
	save_item(NAME(m_cass_data));
}

void pp01_state::machine_reset()
{
	for (int i = 0; i < 16; i++)
	{
		m_memory_block[i] = 0xff;
		set_memory(i, 0xff);
	}
	m_uart->write_cts(0);
}

TIMER_DEVICE_CALLBACK_MEMBER(pp01_state::kansas_r)
{
	if (m_rts)
	{
		m_cass_data[1] = m_cass_data[2] = m_cass_data[3] = 0;
		m_casspol = 1;
		return;
	}

	m_cass_data[1]++;
	m_cass_data[2]++;

	u8 const cass_ws = (m_cass->input() > +0.04) ? 1 : 0;

	if (cass_ws != m_cass_data[0])
	{
		m_cass_data[0] = cass_ws;
		if (m_cass_data[1] > 13)
			m_casspol ^= 1;
		m_cass_data[1] = 0;
		m_cass_data[2] = 0;
		m_uart->write_rxd(m_casspol);
	}
	if ((m_cass_data[2] & 7) == 2)
	{
		m_cass_data[3]++;
		m_uart->write_rxc(BIT(m_cass_data[3], 0));
	}
}

void pp01_state::z2_w(int state)
{
	// incoming 1200Hz
	m_uart->write_txc(state);
	if (!m_txe)
		m_cass->output((m_txd ^ state) ? -1.0 : 1.0);
}

u8 pp01_state::ppi1_porta_r()
{
	return m_video_scroll;
}
void pp01_state::ppi1_porta_w(u8 data)
{
	m_video_scroll = data;
}

u8 pp01_state::ppi1_portb_r()
{
	return m_io_keyboard[m_key_line]->read() | m_io_keyboard[16]->read();
}
void pp01_state::ppi1_portb_w(u8 data)
{
	//logerror("pp01_8255_portb_w %02x\n", data);

}

void pp01_state::ppi1_portc_w(u8 data)
{
	if (BIT(data, 4))
		m_key_line = data & 0x0f;
	else
		m_speaker->level_w(BIT(data, 0));
}

u8 pp01_state::ppi1_portc_r()
{
	return 0xff;
}
