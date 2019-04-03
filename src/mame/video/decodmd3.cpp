// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * Data East Pinball / Sega Pinball Dot Matrix Display
 *
 * Type 3: 192x64
 * 68000 @ 12MHz
 * 68B45 CRTC
 */

#include "emu.h"
#include "decodmd3.h"
#include "screen.h"

DEFINE_DEVICE_TYPE(DECODMD3, decodmd_type3_device, "decodmd3", "Data East Pinball Dot Matrix Display Type 3")

WRITE8_MEMBER( decodmd_type3_device::data_w )
{
	m_latch = data;
}

READ8_MEMBER( decodmd_type3_device::busy_r )
{
	uint8_t ret = 0x00;

	ret = (m_status & 0x0f) << 3;

	if(m_busy)
		return 0x80 | ret;
	else
		return 0x00 | ret;
}

WRITE8_MEMBER( decodmd_type3_device::ctrl_w )
{
	if(!(m_ctrl & 0x01) && (data & 0x01))
	{
		m_cpu->set_input_line(M68K_IRQ_1,ASSERT_LINE);
		m_busy = true;
		m_command = m_latch;
	}
	if((m_ctrl & 0x02) && !(data & 0x02))
	{
		m_cpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
		logerror("DMD3: Reset\n");
	}
	m_ctrl = data;
}

READ16_MEMBER( decodmd_type3_device::status_r )
{
	return m_status;
}

WRITE16_MEMBER( decodmd_type3_device::status_w )
{
	m_status = data & 0x0f;
}

READ16_MEMBER( decodmd_type3_device::latch_r )
{
	// clear IRQ?
	m_cpu->set_input_line(M68K_IRQ_1,CLEAR_LINE);
	m_busy = false;
	return m_command;
}

TIMER_DEVICE_CALLBACK_MEMBER(decodmd_type3_device::dmd_irq)
{
	m_cpu->set_input_line(M68K_IRQ_2, HOLD_LINE);
}

WRITE16_MEMBER( decodmd_type3_device::crtc_address_w )
{
	if(ACCESSING_BITS_8_15)
	{
		m_mc6845->address_w(data >> 8);
		m_crtc_index = data >> 8;
	}
}

READ16_MEMBER( decodmd_type3_device::crtc_status_r )
{
	if(ACCESSING_BITS_8_15)
		return m_mc6845->register_r();
	else
		return 0xff;
}

WRITE16_MEMBER( decodmd_type3_device::crtc_register_w )
{
	if(ACCESSING_BITS_8_15)
	{
		if(m_crtc_index == 9)  // hack!!
			data -= 0x100;
		m_mc6845->register_w(data >> 8);
		m_crtc_reg[m_crtc_index] = data >> 8;
	}
}

MC6845_UPDATE_ROW( decodmd_type3_device::crtc_update_row )
{
	uint8_t *RAM = m_ram->pointer();
	uint8_t intensity;
	uint16_t addr = ((ma & 0x7ff) << 2) | ((ra & 0x02) << 12);
	addr += ((ra & 0x01) * 24);

	for (int x = 0; x < 192; x += 16)
	{
		for (int dot = 0; dot < 8; dot++)
		{
			intensity = ((RAM[addr + 1] >> (7-dot) & 0x01) << 1) | (RAM[addr + 0x801] >> (7-dot) & 0x01);
			bitmap.pix32(y, x + dot) = rgb_t(0x3f * intensity, 0x2a * intensity, 0x00);
		}
		for (int dot = 8; dot < 16; dot++)
		{
			intensity = ((RAM[addr] >> (15-dot) & 0x01) << 1) | (RAM[addr + 0x800] >> (15-dot) & 0x01);
			bitmap.pix32(y, x + dot) = rgb_t(0x3f * intensity, 0x2a * intensity, 0x00);
		}
		addr += 2;
	}
}

void decodmd_type3_device::decodmd3_map(address_map &map)
{
	map(0x00000000, 0x000fffff).bankr("dmdrom");
	map(0x00800000, 0x0080ffff).bankrw("dmdram");
	map(0x00c00010, 0x00c00011).rw(FUNC(decodmd_type3_device::crtc_status_r), FUNC(decodmd_type3_device::crtc_address_w));
	map(0x00c00012, 0x00c00013).w(FUNC(decodmd_type3_device::crtc_register_w));
	map(0x00c00020, 0x00c00021).rw(FUNC(decodmd_type3_device::latch_r), FUNC(decodmd_type3_device::status_w));
}

void decodmd_type3_device::device_add_mconfig(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_cpu, XTAL(12'000'000));
	m_cpu->set_addrmap(AS_PROGRAM, &decodmd_type3_device::decodmd3_map);

	config.m_minimum_quantum = attotime::from_hz(60);

	TIMER(config, "irq_timer", 0).configure_periodic(timer_device::expired_delegate(FUNC(decodmd_type3_device::dmd_irq), this), attotime::from_hz(150));

	MC6845(config, m_mc6845, XTAL(12'000'000) / 4);  // TODO: confirm clock speed
	m_mc6845->set_screen(nullptr);
	m_mc6845->set_show_border_area(false);
	m_mc6845->set_char_width(16);
	m_mc6845->set_update_row_callback(FUNC(decodmd_type3_device::crtc_update_row), this);

	screen_device &screen(SCREEN(config, "dmd", SCREEN_TYPE_RASTER));
	screen.set_native_aspect();
	screen.set_size(192, 64);
	screen.set_visarea(0, 192-1, 0, 64-1);
	screen.set_screen_update("dmd6845", FUNC(mc6845_device::screen_update));
	screen.set_refresh_hz(60);

	RAM(config, RAM_TAG).set_default_size("64K");
}


decodmd_type3_device::decodmd_type3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, DECODMD3, tag, owner, clock)
	, m_cpu(*this,"dmdcpu")
	, m_mc6845(*this,"dmd6845")
	, m_ram(*this,RAM_TAG)
	, m_rambank(*this,"dmdram")
	, m_rombank(*this,"dmdrom")
	, m_rom(*this, finder_base::DUMMY_TAG)
{
}

void decodmd_type3_device::device_start()
{
}

void decodmd_type3_device::device_reset()
{
	uint8_t* RAM = m_ram->pointer();

	memset(RAM,0,0x10000);
	m_rambank->configure_entry(0, &RAM[0]);
	m_rambank->set_entry(0);
	m_rombank->configure_entry(0, &m_rom[0]);
	m_rombank->set_entry(0);
}
