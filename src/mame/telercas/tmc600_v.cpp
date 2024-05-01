// license:BSD-3-Clause
// copyright-holders:Curt Coder
#include "emu.h"
#include "tmc600.h"

void tmc600_state::vismac_register_w(uint8_t data)
{
	m_vismac_reg_latch = data >> 4;
}

void tmc600_state::vismac_data_w(uint8_t data)
{
	uint16_t ma = m_maincpu->get_memory_address();

	switch (m_vismac_reg_latch & 0x07)
	{
	case 2: m_vismac_color_latch = data & 0x0f; break;
	case 3: m_vis->out3_w(data); break;
	case 4: m_vis->out4_w(ma); break;
	case 5: m_vis->out5_w(ma); break;
	case 6: m_vis->out6_w(ma); break;
	case 7: m_vis->out7_w(ma); break;
	}
}

uint8_t tmc600_state::get_color(uint16_t pma)
{
	uint16_t pageaddr = pma & TMC600_PAGE_RAM_MASK;
	uint8_t color = m_color_ram[pageaddr];

	if (BIT(color, 3) && m_blink)
	{
		color = 0;
	}

	return color;
}

void tmc600_state::page_ram_w(offs_t offset, uint8_t data)
{
	m_page_ram[offset] = data;
	m_color_ram[offset] = m_vismac_color_latch;
}

void tmc600_state::cdp1869_page_ram(address_map &map)
{
	map(0x000, 0x3ff).mirror(0x400).ram().share("page_ram").w(FUNC(tmc600_state::page_ram_w));
}

CDP1869_CHAR_RAM_READ_MEMBER( tmc600_state::tmc600_char_ram_r )
{
	uint16_t pageaddr = pma & TMC600_PAGE_RAM_MASK;
	uint8_t color = get_color(pageaddr);
	uint16_t charaddr = ((cma & 0x08) << 8) | (pmd << 3) | (cma & 0x07);
	uint8_t cdb = m_char_rom[charaddr] & 0x3f;

	int ccb0 = BIT(color, 2);
	int ccb1 = BIT(color, 1);

	return (ccb1 << 7) | (ccb0 << 6) | cdb;
}

CDP1869_PCB_READ_MEMBER( tmc600_state::tmc600_pcb_r )
{
	uint16_t pageaddr = pma & TMC600_PAGE_RAM_MASK;
	uint8_t color = get_color(pageaddr);

	return BIT(color, 0);
}

void tmc600_state::prd_w(int state)
{
	if (!state) {
		m_frame++;

		switch (m_frame) {
		case 8:
			m_maincpu->int_w(CLEAR_LINE);
			break;

		case 16:
			m_maincpu->int_w(m_rtc_int);
			m_blink = !m_blink;
			m_frame = 0;
			break;
		}
	}
}

void tmc600_state::video_start()
{
	// state saving
	save_item(NAME(m_vismac_reg_latch));
	save_item(NAME(m_vismac_color_latch));
	save_item(NAME(m_frame));
	save_item(NAME(m_blink));
	save_item(NAME(m_out3));
}

static const gfx_layout tmc600_charlayout =
{
	6, 9,                   // 6 x 9 characters
	256,                    // 256 characters
	1,                      // 1 bits per pixel
	{ 0 },                  // no bitplanes
	// x offsets
	{ 2, 3, 4, 5, 6, 7 },
	// y offsets
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 2048*8 },
	8*8                     // every char takes 8 x 8 bytes
};

static GFXDECODE_START( gfx_tmc600 )
	GFXDECODE_ENTRY( "chargen", 0x0000, tmc600_charlayout, 0, 36 )
GFXDECODE_END

void tmc600_state::tmc600_video(machine_config &config)
{
	// video hardware
	GFXDECODE(config, "gfxdecode", CDP1869_TAG":palette", gfx_tmc600);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	CDP1869(config, m_vis, 3.57_MHz_XTAL, &tmc600_state::cdp1869_page_ram);
	m_vis->add_pal_screen(config, SCREEN_TAG, cdp1869_device::DOT_CLK_PAL);
	m_vis->set_color_clock(cdp1869_device::COLOR_CLK_PAL);
	m_vis->set_pcb_read_callback(FUNC(tmc600_state::tmc600_pcb_r));
	m_vis->set_char_ram_read_callback(FUNC(tmc600_state::tmc600_char_ram_r));
	m_vis->pal_ntsc_callback().set_constant(1);
	m_vis->prd_callback().set(FUNC(tmc600_state::prd_w));
	m_vis->set_screen(SCREEN_TAG);
	m_vis->add_route(ALL_OUTPUTS, "mono", 0.1);
}
