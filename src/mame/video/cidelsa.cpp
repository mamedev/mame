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

MACHINE_CONFIG_START(cidelsa_state::destryer_video)
	MCFG_CDP1869_SCREEN_PAL_ADD(CDP1869_TAG, SCREEN_TAG, DESTRYER_CHR2)
	MCFG_SCREEN_DEFAULT_POSITION(1.226, 0.012, 1.4, 0.044)

	SPEAKER(config, "mono").front_center();
	MCFG_CDP1869_ADD(CDP1869_TAG, DESTRYER_CHR2, cidelsa_page_ram)
	MCFG_CDP1869_CHAR_PCB_READ_OWNER(cidelsa_state, cidelsa_pcb_r)
	MCFG_CDP1869_CHAR_RAM_READ_OWNER(cidelsa_state, cidelsa_charram_r)
	MCFG_CDP1869_CHAR_RAM_WRITE_OWNER(cidelsa_state, cidelsa_charram_w)
	MCFG_CDP1869_PAL_NTSC_CALLBACK(CONSTANT(1))
	MCFG_CDP1869_PRD_CALLBACK(WRITELINE(*this, cidelsa_state, prd_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END

MACHINE_CONFIG_START(cidelsa_state::altair_video)
	MCFG_CDP1869_SCREEN_PAL_ADD(CDP1869_TAG, SCREEN_TAG, ALTAIR_CHR2)
	MCFG_SCREEN_DEFAULT_POSITION(1.226, 0.012, 1.4, 0.044)

	SPEAKER(config, "mono").front_center();
	MCFG_CDP1869_ADD(CDP1869_TAG, ALTAIR_CHR2, cidelsa_page_ram)
	MCFG_CDP1869_CHAR_PCB_READ_OWNER(cidelsa_state, cidelsa_pcb_r)
	MCFG_CDP1869_CHAR_RAM_READ_OWNER(cidelsa_state, cidelsa_charram_r)
	MCFG_CDP1869_CHAR_RAM_WRITE_OWNER(cidelsa_state, cidelsa_charram_w)
	MCFG_CDP1869_PAL_NTSC_CALLBACK(CONSTANT(1))
	MCFG_CDP1869_PRD_CALLBACK(WRITELINE(*this, cidelsa_state, prd_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END

MACHINE_CONFIG_START(draco_state::draco_video)
	MCFG_CDP1869_SCREEN_PAL_ADD(CDP1869_TAG, SCREEN_TAG, DRACO_CHR2)
	MCFG_SCREEN_DEFAULT_POSITION(1.226, 0.012, 1.360, 0.024)

	SPEAKER(config, "mono").front_center();
	MCFG_CDP1869_ADD(CDP1869_TAG, DRACO_CHR2, draco_page_ram)
	MCFG_CDP1869_CHAR_PCB_READ_OWNER(draco_state, draco_pcb_r)
	MCFG_CDP1869_CHAR_RAM_READ_OWNER(draco_state, draco_charram_r)
	MCFG_CDP1869_CHAR_RAM_WRITE_OWNER(draco_state, draco_charram_w)
	MCFG_CDP1869_PAL_NTSC_CALLBACK(CONSTANT(1))
	MCFG_CDP1869_PRD_CALLBACK(INPUTLINE(CDP1802_TAG, COSMAC_INPUT_LINE_EF1))
	MCFG_DEVICE_ADD(AY8910_TAG, AY8910, DRACO_SND_CHR1)
	MCFG_AY8910_OUTPUT_TYPE(AY8910_SINGLE_OUTPUT)
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(*this, draco_state, psg_pb_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END
