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

void comx35_state::cdp1869_w(offs_t offset, uint8_t data)
{
	uint16_t ma = m_maincpu->get_memory_address();

	switch (offset)
	{
	case 3:
		m_vis->out3_w(data);
		break;

	case 4:
		m_vis->out4_w(ma);
		break;

	case 5:
		m_vis->out5_w(ma);
		break;

	case 6:
		m_vis->out6_w(ma);
		break;

	case 7:
		m_vis->out7_w(ma);
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
}

/* Machine Drivers */

void comx35_state::comx35_pal_video(machine_config &config)
{
	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	CDP1869(config, m_vis, cdp1869_device::DOT_CLK_PAL, &comx35_state::cdp1869_page_ram);
	m_vis->add_pal_screen(config, SCREEN_TAG, cdp1869_device::DOT_CLK_PAL);
	m_vis->set_color_clock(cdp1869_device::COLOR_CLK_PAL);
	m_vis->set_pcb_read_callback(FUNC(comx35_state::comx35_pcb_r));
	m_vis->set_char_ram_read_callback(FUNC(comx35_state::comx35_charram_r));
	m_vis->set_char_ram_write_callback(FUNC(comx35_state::comx35_charram_w));
	m_vis->pal_ntsc_callback().set_constant(1);
	m_vis->prd_callback().set(FUNC(comx35_state::prd_w));
	m_vis->set_screen(SCREEN_TAG);
	m_vis->add_route(ALL_OUTPUTS, "mono", 0.25);

	WAVE(config, "wave", "cassette").add_route(ALL_OUTPUTS, "mono", 0.25);
}

void comx35_state::comx35_ntsc_video(machine_config &config)
{
	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	CDP1869(config, m_vis, cdp1869_device::DOT_CLK_NTSC, &comx35_state::cdp1869_page_ram);
	m_vis->add_ntsc_screen(config, SCREEN_TAG, cdp1869_device::DOT_CLK_NTSC);
	m_vis->set_color_clock(cdp1869_device::COLOR_CLK_NTSC);
	m_vis->set_pcb_read_callback(FUNC(comx35_state::comx35_pcb_r));
	m_vis->set_char_ram_read_callback(FUNC(comx35_state::comx35_charram_r));
	m_vis->set_char_ram_write_callback(FUNC(comx35_state::comx35_charram_w));
	m_vis->pal_ntsc_callback().set_constant(0);
	m_vis->prd_callback().set(FUNC(comx35_state::prd_w));
	m_vis->set_screen(SCREEN_TAG);
	m_vis->add_route(ALL_OUTPUTS, "mono", 0.25);

	WAVE(config, "wave", "cassette").add_route(ALL_OUTPUTS, "mono", 0.25);
}
