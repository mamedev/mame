// license:BSD-3-Clause
// copyright-holders:Curt Coder
#include "emu.h"
#include "includes/tmc600.h"

WRITE8_MEMBER( tmc600_state::vismac_register_w )
{
	m_vismac_reg_latch = data >> 4;
}

WRITE8_MEMBER( tmc600_state::vismac_data_w )
{
	uint16_t ma = m_maincpu->get_memory_address();

	switch (m_vismac_reg_latch & 0x07)
	{
	case 2: m_vismac_color_latch = data & 0x0f; break;
	case 3: m_vis->out3_w(space, ma, data); break;
	case 4: m_vis->out4_w(space, ma, data); break;
	case 5: m_vis->out5_w(space, ma, data); break;
	case 6: m_vis->out6_w(space, ma, data); break;
	case 7: m_vis->out7_w(space, ma, data); break;
	}
}

uint8_t tmc600_state::get_color(uint16_t pma)
{
	uint16_t pageaddr = pma & TMC600_PAGE_RAM_MASK;
	uint8_t color = m_color_ram[pageaddr];

	if (BIT(color, 3) && m_blink)
	{
		color ^= 0x07;
	}

	return color;
}

WRITE8_MEMBER( tmc600_state::page_ram_w )
{
	m_page_ram[offset] = data;
	m_color_ram[offset] = m_vismac_color_latch;
}

ADDRESS_MAP_START(tmc600_state::cdp1869_page_ram)
	AM_RANGE(0x000, 0x3ff) AM_MIRROR(0x400) AM_RAM AM_SHARE("page_ram") AM_WRITE(page_ram_w)
ADDRESS_MAP_END

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

WRITE_LINE_MEMBER( tmc600_state::prd_w )
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
	// allocate memory
	m_color_ram.allocate(TMC600_PAGE_RAM_SIZE);

	// state saving
	save_item(NAME(m_vismac_reg_latch));
	save_item(NAME(m_vismac_color_latch));
	save_item(NAME(m_frame));
	save_item(NAME(m_blink));
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

static GFXDECODE_START( tmc600 )
	GFXDECODE_ENTRY( "chargen", 0x0000, tmc600_charlayout, 0, 36 )
GFXDECODE_END

MACHINE_CONFIG_START(tmc600_state::tmc600_video)
	// video hardware
	MCFG_CDP1869_SCREEN_PAL_ADD(CDP1869_TAG, SCREEN_TAG, cdp1869_device::DOT_CLK_PAL)

	MCFG_GFXDECODE_ADD("gfxdecode", CDP1869_TAG":palette", tmc600)

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_CDP1869_ADD(CDP1869_TAG, cdp1869_device::DOT_CLK_PAL, cdp1869_page_ram)
	MCFG_CDP1869_COLOR_CLOCK(cdp1869_device::COLOR_CLK_PAL)
	MCFG_CDP1869_CHAR_PCB_READ_OWNER(tmc600_state, tmc600_pcb_r)
	MCFG_CDP1869_CHAR_RAM_READ_OWNER(tmc600_state, tmc600_char_ram_r)
	MCFG_CDP1869_PAL_NTSC_CALLBACK(VCC)
	MCFG_CDP1869_PRD_CALLBACK(WRITELINE(tmc600_state, prd_w))
	MCFG_VIDEO_SET_SCREEN(SCREEN_TAG)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END
