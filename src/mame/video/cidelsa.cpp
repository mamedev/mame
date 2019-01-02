// license:BSD-3-Clause
// copyright-holders:Curt Coder
#include "emu.h"
#include "includes/cidelsa.h"
#include "speaker.h"


/* Register Access */

WRITE8_MEMBER( cidelsa_state::cdp1869_w )
{
	uint16_t ma = m_maincpu->get_memory_address();

	switch (offset + 3)
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

/* Character RAM Access */

CDP1869_CHAR_RAM_READ_MEMBER( cidelsa_state::cidelsa_charram_r )
{
	uint8_t column = BIT(pma, 10) ? 0xff : pmd;
	uint16_t addr = ((column << 3) | (cma & 0x07)) & CIDELSA_CHARRAM_MASK;

	uint8_t data = m_charram[addr];
	m_cdp1869_pcb = m_pcbram[addr];

	return data;
}

CDP1869_CHAR_RAM_WRITE_MEMBER( cidelsa_state::cidelsa_charram_w )
{
	uint8_t column = BIT(pma, 10) ? 0xff : pmd;
	uint16_t addr = ((column << 3) | (cma & 0x07)) & CIDELSA_CHARRAM_MASK;

	m_charram[addr] = data;
	m_pcbram[addr] = m_cdp1802_q;
}

CDP1869_CHAR_RAM_READ_MEMBER( draco_state::draco_charram_r )
{
	uint16_t addr = ((pmd << 3) | (cma & 0x07)) & CIDELSA_CHARRAM_MASK;

	uint8_t data = m_charram[addr];
	m_cdp1869_pcb = m_pcbram[addr];

	return data;
}

CDP1869_CHAR_RAM_WRITE_MEMBER( draco_state::draco_charram_w )
{
	uint16_t addr = ((pmd << 3) | (cma & 0x07)) & CIDELSA_CHARRAM_MASK;

	m_charram[addr] = data;
	m_pcbram[addr] = m_cdp1802_q;
}

/* Page Color Bit Access */

CDP1869_PCB_READ_MEMBER( cidelsa_state::cidelsa_pcb_r )
{
	uint16_t addr = ((pmd << 3) | (cma & 0x07)) & CIDELSA_CHARRAM_MASK;

	return m_pcbram[addr];
}

CDP1869_PCB_READ_MEMBER( draco_state::draco_pcb_r )
{
	uint16_t addr = ((pmd << 3) | (cma & 0x07)) & CIDELSA_CHARRAM_MASK;

	return m_pcbram[addr];
}

/* Predisplay Changed Handler */

WRITE_LINE_MEMBER( cidelsa_state::prd_w )
{
	/* invert PRD signal */
	m_maincpu->set_input_line(COSMAC_INPUT_LINE_INT, state ? CLEAR_LINE : ASSERT_LINE);
	m_maincpu->set_input_line(COSMAC_INPUT_LINE_EF1, state ? CLEAR_LINE : ASSERT_LINE);
}

/* Page RAM */

void cidelsa_state::cidelsa_page_ram(address_map &map)
{
	map.unmap_value_high();
	map(0x000, 0x3ff).ram();
}

void draco_state::draco_page_ram(address_map &map)
{
	map.unmap_value_high();
	map(0x000, 0x7ff).ram();
}

/* Video Start */

void cidelsa_state::video_start()
{
	// allocate memory
	m_pcbram = make_unique_clear<uint8_t[]>(CIDELSA_CHARRAM_SIZE);
	m_charram = make_unique_clear<uint8_t[]>(CIDELSA_CHARRAM_SIZE);

	// register for state saving
	save_item(NAME(m_cdp1869_pcb));
	save_pointer(NAME(m_pcbram), CIDELSA_CHARRAM_SIZE);
	save_pointer(NAME(m_charram), CIDELSA_CHARRAM_SIZE);
}

/* AY-3-8910 */

WRITE8_MEMBER( draco_state::psg_pb_w )
{
	/*

	  bit   description

	    0   RELE0
	    1   RELE1
	    2   sound output -> * -> 22K capacitor -> GND
	    3   sound output -> * -> 220K capacitor -> GND
	    4   5V -> 1K resistor -> * -> 10uF capacitor -> GND (volume pot voltage adjustment)
	    5   not connected
	    6   not connected
	    7   not connected

	*/
}

/* Machine Drivers */

void cidelsa_state::destryer_video(machine_config &config)
{
	SPEAKER(config, "mono").front_center();
	CDP1869(config, m_vis, DESTRYER_CHR2, &cidelsa_state::cidelsa_page_ram);
	screen_device &screen(m_vis->add_pal_screen(config, SCREEN_TAG, DESTRYER_CHR2));
	screen.set_default_position(1.226, 0.012, 1.4, 0.044);
	m_vis->set_pcb_read_callback(FUNC(cidelsa_state::cidelsa_pcb_r));
	m_vis->set_char_ram_read_callback(FUNC(cidelsa_state::cidelsa_charram_r));
	m_vis->set_char_ram_write_callback(FUNC(cidelsa_state::cidelsa_charram_w));
	m_vis->pal_ntsc_callback().set_constant(1);
	m_vis->prd_callback().set(FUNC(cidelsa_state::prd_w));
	m_vis->add_route(ALL_OUTPUTS, "mono", 0.25);
}

void cidelsa_state::altair_video(machine_config &config)
{
	SPEAKER(config, "mono").front_center();
	CDP1869(config, m_vis, ALTAIR_CHR2, &cidelsa_state::cidelsa_page_ram);
	screen_device &screen(m_vis->add_pal_screen(config, SCREEN_TAG, ALTAIR_CHR2));
	screen.set_default_position(1.226, 0.012, 1.4, 0.044);
	m_vis->set_pcb_read_callback(FUNC(cidelsa_state::cidelsa_pcb_r));
	m_vis->set_char_ram_read_callback(FUNC(cidelsa_state::cidelsa_charram_r));
	m_vis->set_char_ram_write_callback(FUNC(cidelsa_state::cidelsa_charram_w));
	m_vis->pal_ntsc_callback().set_constant(1);
	m_vis->prd_callback().set(FUNC(cidelsa_state::prd_w));
	m_vis->add_route(ALL_OUTPUTS, "mono", 0.25);
}

void draco_state::draco_video(machine_config &config)
{
	SPEAKER(config, "mono").front_center();
	CDP1869(config, m_vis, DRACO_CHR2, &draco_state::draco_page_ram);
	screen_device &screen(m_vis->add_pal_screen(config, SCREEN_TAG, DRACO_CHR2));
	screen.set_default_position(1.226, 0.012, 1.360, 0.024);
	m_vis->set_pcb_read_callback(FUNC(draco_state::draco_pcb_r));
	m_vis->set_char_ram_read_callback(FUNC(draco_state::draco_charram_r));
	m_vis->set_char_ram_write_callback(FUNC(draco_state::draco_charram_w));
	m_vis->pal_ntsc_callback().set_constant(1);
	m_vis->prd_callback().set_inputline(CDP1802_TAG, COSMAC_INPUT_LINE_EF1);

	AY8910(config, m_psg, DRACO_SND_CHR1);
	m_psg->set_flags(AY8910_SINGLE_OUTPUT);
	m_psg->port_b_write_callback().set(FUNC(draco_state::psg_pb_w));
	m_psg->add_route(ALL_OUTPUTS, "mono", 0.25);
}
