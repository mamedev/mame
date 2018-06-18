// license:BSD-3-Clause
// copyright-holders:Curt Coder
#include "emu.h"
#include "includes/comx35.h"

#include "cpu/cosmac/cosmac.h"
#include "sound/cdp1869.h"
#include "sound/wave.h"
#include "video/mc6845.h"
#include "rendlay.h"
#include "screen.h"
#include "speaker.h"

WRITE8_MEMBER( comx35_state::cdp1869_w )
{
	uint16_t ma = m_maincpu->get_memory_address();

	switch (offset)
	{
	case 3:
		m_vis->out3_w(space, ma, data);
		break;

	case 4:
		m_vis->out4_w(space, ma, data);
		break;

	case 5:
		m_vis->out5_w(space, ma, data);
		break;

	case 6:
		m_vis->out6_w(space, ma, data);
		break;

	case 7:
		m_vis->out7_w(space, ma, data);
		break;
	}
}

/* CDP1869 */

void comx35_state::cdp1869_page_ram(address_map &map)
{
	map(0x000, 0x7ff).ram();
}

CDP1869_CHAR_RAM_READ_MEMBER( comx35_state::comx35_charram_r )
{
	uint8_t column = pmd & 0x7f;
	uint16_t charaddr = (column << 4) | cma;

	return m_char_ram[charaddr];
}

CDP1869_CHAR_RAM_WRITE_MEMBER( comx35_state::comx35_charram_w )
{
	uint8_t column = pmd & 0x7f;
	uint16_t charaddr = (column << 4) | cma;

	m_char_ram[charaddr] = data;
}

CDP1869_PCB_READ_MEMBER( comx35_state::comx35_pcb_r )
{
	return BIT(pmd, 7);
}

WRITE_LINE_MEMBER( comx35_state::prd_w )
{
	if ((m_prd == CLEAR_LINE) && (state == ASSERT_LINE))
	{
		m_cr1 = m_iden ? CLEAR_LINE : ASSERT_LINE;
		check_interrupt();
	}

	m_prd = state;

	m_maincpu->set_input_line(COSMAC_INPUT_LINE_EF1, state);
}


void comx35_state::video_start()
{
	// allocate memory
	m_char_ram.allocate(COMX35_CHARRAM_SIZE);
}

/* Machine Drivers */

MACHINE_CONFIG_START(comx35_state::comx35_pal_video)
	MCFG_CDP1869_SCREEN_PAL_ADD(CDP1869_TAG, SCREEN_TAG, cdp1869_device::DOT_CLK_PAL)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	MCFG_CDP1869_ADD(CDP1869_TAG, cdp1869_device::DOT_CLK_PAL, cdp1869_page_ram)
	MCFG_CDP1869_COLOR_CLOCK(cdp1869_device::COLOR_CLK_PAL)
	MCFG_CDP1869_CHAR_PCB_READ_OWNER(comx35_state, comx35_pcb_r)
	MCFG_CDP1869_CHAR_RAM_READ_OWNER(comx35_state, comx35_charram_r)
	MCFG_CDP1869_CHAR_RAM_WRITE_OWNER(comx35_state, comx35_charram_w)
	MCFG_CDP1869_PAL_NTSC_CALLBACK(VCC)
	MCFG_CDP1869_PRD_CALLBACK(WRITELINE(*this, comx35_state, prd_w))
	MCFG_CDP1869_SET_SCREEN(SCREEN_TAG)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	WAVE(config, "wave", "cassette").add_route(ALL_OUTPUTS, "mono", 0.25);
MACHINE_CONFIG_END

MACHINE_CONFIG_START(comx35_state::comx35_ntsc_video)
	MCFG_CDP1869_SCREEN_NTSC_ADD(CDP1869_TAG, SCREEN_TAG, cdp1869_device::DOT_CLK_NTSC)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	MCFG_CDP1869_ADD(CDP1869_TAG, cdp1869_device::DOT_CLK_NTSC, cdp1869_page_ram)
	MCFG_CDP1869_COLOR_CLOCK(cdp1869_device::COLOR_CLK_NTSC)
	MCFG_CDP1869_CHAR_PCB_READ_OWNER(comx35_state, comx35_pcb_r)
	MCFG_CDP1869_CHAR_RAM_READ_OWNER(comx35_state, comx35_charram_r)
	MCFG_CDP1869_CHAR_RAM_WRITE_OWNER(comx35_state, comx35_charram_w)
	MCFG_CDP1869_PAL_NTSC_CALLBACK(GND)
	MCFG_CDP1869_PRD_CALLBACK(WRITELINE(*this, comx35_state, prd_w))
	MCFG_CDP1869_SET_SCREEN(SCREEN_TAG)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	WAVE(config, "wave", "cassette").add_route(ALL_OUTPUTS, "mono", 0.25);
MACHINE_CONFIG_END
