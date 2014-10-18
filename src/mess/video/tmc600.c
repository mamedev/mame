// license:BSD-3-Clause
// copyright-holders:Curt Coder
#include "emu.h"
#include "sound/cdp1869.h"
#include "includes/tmc600.h"

WRITE8_MEMBER( tmc600_state::vismac_register_w )
{
	m_vismac_reg_latch = data;
}

WRITE8_MEMBER( tmc600_state::vismac_data_w )
{
	UINT16 ma = m_maincpu->get_memory_address();

	switch (m_vismac_reg_latch)
	{
	case 0x20:
		// character color latch
		m_vismac_color_latch = data;
		break;

	case 0x30:
		// background color latch
		m_vismac_bkg_latch = data & 0x07;

		m_vis->out3_w(space, ma, data);
		break;

	case 0x40:
		m_vis->out4_w(space, ma, data);
		break;

	case 0x50:
		m_vis->out5_w(space, ma, data);
		break;

	case 0x60:
		m_vis->out6_w(space, ma, data);
		break;

	case 0x70:
		m_vis->out7_w(space, ma, data);
		break;
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(tmc600_state::blink_tick)
{
	m_blink = !m_blink;
}

UINT8 tmc600_state::get_color(UINT16 pma)
{
	UINT16 pageaddr = pma & TMC600_PAGE_RAM_MASK;
	UINT8 color = m_color_ram[pageaddr];

	if (BIT(color, 3) && m_blink)
	{
		color = m_vismac_bkg_latch;
	}

	return color;
}

WRITE8_MEMBER( tmc600_state::page_ram_w )
{
	m_page_ram[offset] = data;
	m_color_ram[offset] = m_vismac_color_latch;
}

static ADDRESS_MAP_START( cdp1869_page_ram, AS_0, 8, tmc600_state )
	AM_RANGE(0x000, 0x3ff) AM_MIRROR(0x400) AM_RAM AM_SHARE("page_ram") AM_WRITE(page_ram_w)
ADDRESS_MAP_END

CDP1869_CHAR_RAM_READ_MEMBER( tmc600_state::tmc600_char_ram_r )
{
	UINT16 pageaddr = pma & TMC600_PAGE_RAM_MASK;
	UINT8 color = get_color(pageaddr);
	UINT16 charaddr = ((cma & 0x08) << 8) | (pmd << 3) | (cma & 0x07);
	UINT8 cdb = m_char_rom[charaddr] & 0x3f;

	int ccb0 = BIT(color, 2);
	int ccb1 = BIT(color, 1);

	return (ccb1 << 7) | (ccb0 << 6) | cdb;
}

CDP1869_PCB_READ_MEMBER( tmc600_state::tmc600_pcb_r )
{
	UINT16 pageaddr = pma & TMC600_PAGE_RAM_MASK;
	UINT8 color = get_color(pageaddr);

	return BIT(color, 0);
}

void tmc600_state::video_start()
{
	// allocate memory
	m_color_ram.allocate(TMC600_PAGE_RAM_SIZE);

	// register for state saving
	save_item(NAME(m_vismac_reg_latch));
	save_item(NAME(m_vismac_color_latch));
	save_item(NAME(m_vismac_bkg_latch));
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

MACHINE_CONFIG_FRAGMENT( tmc600_video )
	// video hardware
	MCFG_CDP1869_SCREEN_PAL_ADD(CDP1869_TAG, SCREEN_TAG, CDP1869_DOT_CLK_PAL)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("blink", tmc600_state, blink_tick, attotime::from_hz(2))

	MCFG_GFXDECODE_ADD("gfxdecode", CDP1869_TAG":palette", tmc600)

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_CDP1869_ADD(CDP1869_TAG, CDP1869_DOT_CLK_PAL, cdp1869_page_ram)
	MCFG_CDP1869_COLOR_CLOCK(CDP1869_COLOR_CLK_PAL)
	MCFG_CDP1869_CHAR_PCB_READ_OWNER(tmc600_state, tmc600_pcb_r)
	MCFG_CDP1869_CHAR_RAM_READ_OWNER(tmc600_state, tmc600_char_ram_r)
	MCFG_CDP1869_PAL_NTSC_CALLBACK(VCC)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END
