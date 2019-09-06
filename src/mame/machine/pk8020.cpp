// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, AJR
/***************************************************************************

        PK-8020 driver by Miodrag Milanovic
            based on work of Sergey Erokhin from pk8020.narod.ru

        18/07/2008 Preliminary driver.

****************************************************************************/


#include "emu.h"
#include "includes/pk8020.h"

#define DESCRIBE_DECPLM 1


uint8_t pk8020_state::keyboard_r(offs_t offset)
{
	uint8_t result = 0xff;

	if (BIT(offset, 8))
	{
		for (uint8_t line = 8; line < 16; line++)
			if (BIT(offset, line - 8))
				result &= m_io_port[line]->read();
	}
	else
	{
		for (uint8_t line = 0; line < 8; line++)
			if (BIT(offset, line))
				result &= m_io_port[line]->read();
	}

	return result ^ 0xff;
}

void pk8020_state::sysreg_w(offs_t offset, uint8_t data)
{
	if (!BIT(offset, 7))
	{
		m_bank_select = data & 0xfc;
		logerror("%s: Bank select = %02X\n", machine().describe_context(), (data >> 2) & 0x1f);
	}

	if (!BIT(offset, 6))
		color_w(data);
	if (!BIT(offset, 2))
		palette_w(data);
}

uint8_t pk8020_state::memory_r(offs_t offset)
{
	uint8_t select = m_decplm->read(bitswap<13>((offset & 0xff00) | m_bank_select, 2, 5, 6, 4, 12, 3, 13, 8, 9, 11, 15, 10, 14) | 0x8000);
	uint8_t result = 0xff;

	if (!BIT(select, 2))
		result &= m_region_maincpu->base()[offset & 0x1fff];
	if (!BIT(select, 3))
		result &= m_region_maincpu->base()[(offset & 0x1fff) | 0x2000];
	if (!BIT(select, 0))
		result &= m_region_maincpu->base()[(offset & 0x1fff) | 0x4000];
	if (!BIT(select, 1))
		result &= keyboard_r(offset);
	if (!BIT(select, 4))
		result &= m_devbank->read8(offset);

	if ((select & 0xc0) == 0x00)
		result &= m_ram->pointer()[offset];
	else if ((select & 0xc0) == 0x40)
		result &= gzu_r(offset & 0x3fff);
	else if ((select & 0xc0) == 0x80)
		result &= text_r(offset & 0x3ff);

	return result;
}

void pk8020_state::memory_w(offs_t offset, uint8_t data)
{
	uint8_t select = m_decplm->read(bitswap<13>((offset & 0xff00) | m_bank_select, 2, 5, 6, 4, 12, 3, 13, 8, 9, 11, 15, 10, 14) | 0x2000);

	if (!BIT(select, 5))
		sysreg_w(offset, data);
	if (!BIT(select, 4))
		m_devbank->write8(offset, data);

	if ((select & 0xc0) == 0x00)
		m_ram->pointer()[offset] = data;
	else if ((select & 0xc0) == 0x40)
		gzu_w(offset & 0x3fff, data);
	else if ((select & 0xc0) == 0x80)
		text_w(offset & 0x3ff, data);
}

uint8_t pk8020_state::ppi_porta_r()
{
	return 0xf0 | (m_takt <<1) | (m_text_attr<<3) | ((m_cass->input() > +0.04) ? 1 : 0);
}

void pk8020_state::floppy_control_w(uint8_t data)
{
	floppy_image_device *floppy = nullptr;

	// Turn all motors off
	for (int n = 0; n < 4; n++)
		if (m_floppy[n]->get_device())
			m_floppy[n]->get_device()->mon_w(1);

	for (int n = 0; n < 4; n++)
		if (BIT(data, n))
			floppy = m_floppy[n]->get_device();

	m_fdc->set_floppy(floppy);

	if (floppy)
	{
		floppy->mon_w(0);
		floppy->ss_w(BIT(data, 4));
	}

	// todo: at least bit 5 and bit 7 is connected to something too...
}


void pk8020_state::ppi_2_portc_w(uint8_t data)
{
	static const double levels[4] = { 0.0, 1.0, -1.0, 0.0 };
	m_cass->output(levels[data & 3]);

	m_sound_gate = BIT(data,3);
	m_speaker->level_w(m_sound_gate ? m_sound_level : 0);

	m_printer->write_select(!BIT(data, 4));
	m_printer->write_strobe(!BIT(data, 5));
}

WRITE_LINE_MEMBER(pk8020_state::pit_out0)
{
	m_sound_level = state;

	m_speaker->level_w(m_sound_gate ? m_sound_level : 0);
}


const char *pk8020_state::plm_select_name(uint8_t data)
{
	switch (data)
	{
	case 0xfb:
	case 0xf7:
	case 0xfe:
		return "ROM";

	case 0x3f:
		return "RAM";

	case 0xfd:
		return "keyboard";

	case 0xdf:
		return "system reg";

	case 0xef:
		return "devices";

	case 0xbf:
		return "text video memory";

	case 0x7f:
		return "video RAM";

	default:
		return "multiple/unknown";
	}
}

void pk8020_state::log_bank_select(uint8_t bank, offs_t start, offs_t end, uint8_t rdecplm, uint8_t wdecplm)
{
	if (rdecplm == wdecplm)
		logerror("Bank select %02X, %04X-%04Xh: read/write %s (%02X)\n", bank, start, end, plm_select_name(rdecplm), rdecplm);
	else
		logerror("Bank select %02X, %04X-%04Xh: read %s (%02X), write %s (%02X)\n", bank, start, end, plm_select_name(rdecplm), rdecplm, plm_select_name(wdecplm), wdecplm);
}

void pk8020_state::machine_start()
{
	m_ios[0]->write_cts(0);
	m_ios[1]->write_cts(0);
	m_ios[1]->write_dsr(0);
	m_takt = 0;

	save_item(NAME(m_bank_select));
	save_item(NAME(m_takt));
	save_item(NAME(m_sound_gate));
	save_item(NAME(m_sound_level));

	if (DESCRIBE_DECPLM)
	{
		for (uint8_t bank = 0; bank < 0x20; bank++)
		{
			uint8_t rdecplm = 0;
			uint8_t wdecplm = 0;
			offs_t start = 0x0000;

			for (offs_t offset = 0x0000; offset < 0x10000; offset += 0x100)
			{
				uint8_t rdecplm_test = m_decplm->read(bitswap<13>((offset & 0xff00) | (bank << 2), 2, 5, 6, 4, 12, 3, 13, 8, 9, 11, 15, 10, 14) | 0x8000);
				uint8_t wdecplm_test = m_decplm->read(bitswap<13>((offset & 0xff00) | (bank << 2), 2, 5, 6, 4, 12, 3, 13, 8, 9, 11, 15, 10, 14) | 0x2000);

				if (rdecplm != rdecplm_test || wdecplm != wdecplm_test)
				{
					if (offset != 0x0000)
						log_bank_select(bank, start, offset - 1, rdecplm, wdecplm);
					rdecplm = rdecplm_test;
					wdecplm = wdecplm_test;
					start = offset;
				}
			}
			log_bank_select(bank, start, 0xffff, rdecplm, wdecplm);
		}
	}
}

void pk8020_state::machine_reset()
{
	m_bank_select = 0;

	m_sound_gate = 0;
	m_sound_level = 0;
}

INTERRUPT_GEN_MEMBER(pk8020_state::pk8020_interrupt)
{
	m_takt ^= 1;
	m_inr->ir4_w(1);
}
