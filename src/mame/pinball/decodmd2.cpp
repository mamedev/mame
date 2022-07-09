// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 *   Data East Pinball Dot Matrix Display
 *
 *    Type 2: 128x32
 *    68B09E @ 2MHz
 *    68B45 CRTC
 */

#include "emu.h"
#include "decodmd2.h"
#include "screen.h"

DEFINE_DEVICE_TYPE(DECODMD2, decodmd_type2_device, "decodmd2", "Data East Pinball Dot Matrix Display Type 2")

void decodmd_type2_device::bank_w(uint8_t data)
{
	m_rombank1->set_entry(data & 0x1f);
}

void decodmd_type2_device::crtc_address_w(uint8_t data)
{
	m_mc6845->address_w(data);
	m_crtc_index = data;
}

uint8_t decodmd_type2_device::crtc_status_r()
{
	return m_mc6845->register_r();
}

void decodmd_type2_device::crtc_register_w(uint8_t data)
{
	m_mc6845->register_w(data);
	m_crtc_reg[m_crtc_index] = data;
}

uint8_t decodmd_type2_device::latch_r()
{
	// clear IRQ?
	m_cpu->set_input_line(M6809_IRQ_LINE,CLEAR_LINE);
	m_busy = false;
	return m_command;
}

void decodmd_type2_device::data_w(uint8_t data)
{
	// set IRQ?
	m_latch = data;
}

uint8_t decodmd_type2_device::busy_r()
{
	uint8_t ret = 0x00;

	ret = (m_status & 0x0f) << 3;

	if(m_busy)
		return 0x80 | ret;
	else
		return 0x00 | ret;
}


void decodmd_type2_device::ctrl_w(uint8_t data)
{
	if(!(m_ctrl & 0x01) && (data & 0x01))
	{
		m_cpu->set_input_line(M6809_IRQ_LINE,ASSERT_LINE);
		m_busy = true;
		m_command = m_latch;
	}
	if((m_ctrl & 0x02) && !(data & 0x02))
	{
		m_cpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
		m_rombank1->set_entry(0);
		logerror("DMD2: Reset\n");
	}
	m_ctrl = data;
}

uint8_t decodmd_type2_device::ctrl_r()
{
	return m_ctrl;
}

uint8_t decodmd_type2_device::status_r()
{
	return m_status;
}

void decodmd_type2_device::status_w(uint8_t data)
{
	m_status = data & 0x0f;
}

TIMER_DEVICE_CALLBACK_MEMBER(decodmd_type2_device::dmd_firq)
{
	m_cpu->set_input_line(M6809_FIRQ_LINE, HOLD_LINE);
}

MC6845_UPDATE_ROW( decodmd_type2_device::crtc_update_row )
{
	uint16_t addr = (ma & 0xfc00) + ((ma & 0x100)<<2) + (ra << 4);

	for (int x = 0; x < 128; x += 8)
	{
		for (int dot = 0; dot < 8; dot++)
		{
			uint8_t intensity = ((m_ram[addr] >> (7-dot) & 0x01) << 1) | (m_ram[addr + 0x200] >> (7-dot) & 0x01);
			bitmap.pix(y, x + dot) = rgb_t(0x3f * intensity, 0x2a * intensity, 0x00);
		}
		addr++;
	}
}

void decodmd_type2_device::decodmd2_map(address_map &map)
{
	map(0x0000, 0x2fff).ram().share("dmdram");
	map(0x3000, 0x3000).rw(FUNC(decodmd_type2_device::crtc_status_r), FUNC(decodmd_type2_device::crtc_address_w));
	map(0x3001, 0x3001).w(FUNC(decodmd_type2_device::crtc_register_w));
	map(0x3002, 0x3002).w(FUNC(decodmd_type2_device::bank_w));
	map(0x3003, 0x3003).r(FUNC(decodmd_type2_device::latch_r));
	map(0x4000, 0x7fff).bankr("dmdbank1").w(FUNC(decodmd_type2_device::status_w));
	map(0x8000, 0xffff).bankr("dmdbank2"); // last 32k of ROM
}

void decodmd_type2_device::device_add_mconfig(machine_config &config)
{
	/* basic machine hardware */
	MC6809E(config, m_cpu, XTAL(8'000'000) / 4);
	m_cpu->set_addrmap(AS_PROGRAM, &decodmd_type2_device::decodmd2_map);

	config.set_maximum_quantum(attotime::from_hz(60));

	TIMER(config, "firq_timer", 0).configure_periodic(FUNC(decodmd_type2_device::dmd_firq), attotime::from_hz(80));

	MC6845(config, m_mc6845, XTAL(8'000'000) / 8);  // TODO: confirm clock speed
	m_mc6845->set_screen(nullptr);
	m_mc6845->set_show_border_area(false);
	m_mc6845->set_char_width(8);
	m_mc6845->set_update_row_callback(FUNC(decodmd_type2_device::crtc_update_row));

	screen_device &screen(SCREEN(config, "dmd", SCREEN_TYPE_RASTER));
	screen.set_native_aspect();
	screen.set_size(128, 32);
	screen.set_visarea(0, 128-1, 0, 32-1);
	screen.set_screen_update("dmd6845", FUNC(mc6845_device::screen_update));
	screen.set_refresh_hz(60);
}


decodmd_type2_device::decodmd_type2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, DECODMD2, tag, owner, clock)
	, m_cpu(*this, "dmdcpu")
	, m_mc6845(*this, "dmd6845")
	, m_rombank1(*this, "dmdbank1")
	, m_rombank2(*this, "dmdbank2")
	, m_ram(*this, "dmdram")
	, m_rom(*this, finder_base::DUMMY_TAG)
{
}

void decodmd_type2_device::device_start()
{
}

void decodmd_type2_device::device_reset()
{
	m_rombank1->configure_entries(0, 32, &m_rom[0x0000], 0x4000);
	m_rombank2->configure_entry(0, &m_rom[0x78000]);
	m_rombank1->set_entry(0);
	m_rombank2->set_entry(0);
	m_busy = false;
}
