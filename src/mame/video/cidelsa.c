#include "emu.h"
#include "cpu/cosmac/cosmac.h"
#include "sound/cdp1869.h"
#include "sound/ay8910.h"
#include "includes/cidelsa.h"

/* Register Access */

WRITE8_MEMBER( cidelsa_state::cdp1869_w )
{
	UINT16 ma = m_maincpu->get_memory_address();

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

static CDP1869_CHAR_RAM_READ( cidelsa_charram_r )
{
	cidelsa_state *state = device->machine().driver_data<cidelsa_state>();

	UINT8 column = BIT(pma, 10) ? 0xff : pmd;
	UINT16 addr = ((column << 3) | (cma & 0x07)) & CIDELSA_CHARRAM_MASK;

	UINT8 data = state->m_charram[addr];
	state->m_cdp1869_pcb = state->m_pcbram[addr];

	return data;
}

static CDP1869_CHAR_RAM_WRITE( cidelsa_charram_w )
{
	cidelsa_state *state = device->machine().driver_data<cidelsa_state>();

	UINT8 column = BIT(pma, 10) ? 0xff : pmd;
	UINT16 addr = ((column << 3) | (cma & 0x07)) & CIDELSA_CHARRAM_MASK;

	state->m_charram[addr] = data;
	state->m_pcbram[addr] = state->m_cdp1802_q;
}

static CDP1869_CHAR_RAM_READ( draco_charram_r )
{
	cidelsa_state *state = device->machine().driver_data<cidelsa_state>();

	UINT16 addr = ((pmd << 3) | (cma & 0x07)) & CIDELSA_CHARRAM_MASK;

	UINT8 data = state->m_charram[addr];
	state->m_cdp1869_pcb = state->m_pcbram[addr];

	return data;
}

static CDP1869_CHAR_RAM_WRITE( draco_charram_w )
{
	cidelsa_state *state = device->machine().driver_data<cidelsa_state>();

	UINT16 addr = ((pmd << 3) | (cma & 0x07)) & CIDELSA_CHARRAM_MASK;

	state->m_charram[addr] = data;
	state->m_pcbram[addr] = state->m_cdp1802_q;
}

/* Page Color Bit Access */

static CDP1869_PCB_READ( cidelsa_pcb_r )
{
	cidelsa_state *state = device->machine().driver_data<cidelsa_state>();

	UINT16 addr = ((pmd << 3) | (cma & 0x07)) & CIDELSA_CHARRAM_MASK;

	return state->m_pcbram[addr];
}

static CDP1869_PCB_READ( draco_pcb_r )
{
	cidelsa_state *state = device->machine().driver_data<cidelsa_state>();

	UINT16 addr = ((pmd << 3) | (cma & 0x07)) & CIDELSA_CHARRAM_MASK;

	return state->m_pcbram[addr];
}

/* Predisplay Changed Handler */

WRITE_LINE_MEMBER( cidelsa_state::prd_w )
{
	/* invert PRD signal */
	m_maincpu->set_input_line(COSMAC_INPUT_LINE_INT, state ? CLEAR_LINE : ASSERT_LINE);
	m_maincpu->set_input_line(COSMAC_INPUT_LINE_EF1, state ? CLEAR_LINE : ASSERT_LINE);
}

/* Page RAM */

static ADDRESS_MAP_START( cidelsa_page_ram, AS_0, 8, driver_device )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000, 0x3ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( draco_page_ram, AS_0, 8, driver_device )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000, 0x7ff) AM_RAM
ADDRESS_MAP_END

/* CDP1869 Interface */

static CDP1869_INTERFACE( destryer_vis_intf )
{
	SCREEN_TAG,
	0,
	CDP1869_PAL,
	cidelsa_pcb_r,
	cidelsa_charram_r,
	cidelsa_charram_w,
	DEVCB_DRIVER_LINE_MEMBER(cidelsa_state, prd_w)
};

static CDP1869_INTERFACE( altair_vis_intf )
{
	SCREEN_TAG,
	0,
	CDP1869_PAL,
	cidelsa_pcb_r,
	cidelsa_charram_r,
	cidelsa_charram_w,
	DEVCB_DRIVER_LINE_MEMBER(cidelsa_state, prd_w)
};

static CDP1869_INTERFACE( draco_vis_intf )
{
	SCREEN_TAG,
	0,
	CDP1869_PAL,
	draco_pcb_r,
	draco_charram_r,
	draco_charram_w,
	DEVCB_CPU_INPUT_LINE(CDP1802_TAG, COSMAC_INPUT_LINE_EF1)
};

/* Video Start */

void cidelsa_state::video_start()
{
	// allocate memory
	m_pcbram = auto_alloc_array(machine(), UINT8, CIDELSA_CHARRAM_SIZE);
	m_charram = auto_alloc_array(machine(), UINT8, CIDELSA_CHARRAM_SIZE);

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

static const ay8910_interface psg_intf =
{
	AY8910_SINGLE_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(draco_state, psg_pb_w)
};

/* Machine Drivers */

MACHINE_CONFIG_FRAGMENT( destryer_video )
	MCFG_CDP1869_SCREEN_PAL_ADD(CDP1869_TAG, SCREEN_TAG, DESTRYER_CHR2)
	MCFG_SCREEN_DEFAULT_POSITION(1.226, 0.012, 1.4, 0.044)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_CDP1869_ADD(CDP1869_TAG, DESTRYER_CHR2, destryer_vis_intf, cidelsa_page_ram)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END

MACHINE_CONFIG_FRAGMENT( altair_video )
	MCFG_CDP1869_SCREEN_PAL_ADD(CDP1869_TAG, SCREEN_TAG, ALTAIR_CHR2)
	MCFG_SCREEN_DEFAULT_POSITION(1.226, 0.012, 1.4, 0.044)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_CDP1869_ADD(CDP1869_TAG, ALTAIR_CHR2, altair_vis_intf, cidelsa_page_ram)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END

MACHINE_CONFIG_FRAGMENT( draco_video )
	MCFG_CDP1869_SCREEN_PAL_ADD(CDP1869_TAG, SCREEN_TAG, DRACO_CHR2)
	MCFG_SCREEN_DEFAULT_POSITION(1.226, 0.012, 1.360, 0.024)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_CDP1869_ADD(CDP1869_TAG, DRACO_CHR2, draco_vis_intf, draco_page_ram)
	MCFG_SOUND_ADD(AY8910_TAG, AY8910, DRACO_SND_CHR1)
	MCFG_SOUND_CONFIG(psg_intf)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END
