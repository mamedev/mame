#include "emu.h"
#include "rendlay.h"
#include "includes/comx35.h"
#include "cpu/cosmac/cosmac.h"
#include "sound/cdp1869.h"
#include "sound/wave.h"
#include "video/mc6845.h"

WRITE8_MEMBER( comx35_state::cdp1869_w )
{
	UINT16 ma = m_maincpu->get_memory_address();

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

static ADDRESS_MAP_START( cdp1869_page_ram, AS_0, 8, driver_device )
	AM_RANGE(0x000, 0x7ff) AM_RAM
ADDRESS_MAP_END

static CDP1869_CHAR_RAM_READ( comx35_charram_r )
{
	comx35_state *state = device->machine().driver_data<comx35_state>();

	UINT8 column = pmd & 0x7f;
	UINT16 charaddr = (column << 4) | cma;

	return state->m_charram[charaddr];
}

static CDP1869_CHAR_RAM_WRITE( comx35_charram_w )
{
	comx35_state *state = device->machine().driver_data<comx35_state>();

	UINT8 column = pmd & 0x7f;
	UINT16 charaddr = (column << 4) | cma;

	state->m_charram[charaddr] = data;
}

static CDP1869_PCB_READ( comx35_pcb_r )
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

static CDP1869_INTERFACE( pal_cdp1869_intf )
{
	SCREEN_TAG,
	CDP1869_COLOR_CLK_PAL,
	CDP1869_PAL,
	comx35_pcb_r,
	comx35_charram_r,
	comx35_charram_w,
	DEVCB_DRIVER_LINE_MEMBER(comx35_state, prd_w)
};

static CDP1869_INTERFACE( ntsc_cdp1869_intf )
{
	SCREEN_TAG,
	CDP1869_COLOR_CLK_NTSC,
	CDP1869_NTSC,
	comx35_pcb_r,
	comx35_charram_r,
	comx35_charram_w,
	DEVCB_DRIVER_LINE_MEMBER(comx35_state, prd_w)
};

void comx35_state::video_start()
{
	// allocate memory
	m_charram = auto_alloc_array(machine(), UINT8, COMX35_CHARRAM_SIZE);

	// register for save state
	save_pointer(NAME(m_charram), COMX35_CHARRAM_SIZE);
}

UINT32 comx35_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (screen.width() == CDP1869_SCREEN_WIDTH)
	{
		m_vis->screen_update(screen, bitmap, cliprect);
	}
	else
	{
		m_expansion->screen_update(screen, bitmap, cliprect);
	}

	return 0;
}

/* Machine Drivers */

MACHINE_CONFIG_FRAGMENT( comx35_pal_video )
	MCFG_CDP1869_SCREEN_PAL_ADD(CDP1869_TAG, SCREEN_TAG, CDP1869_DOT_CLK_PAL)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_CDP1869_ADD(CDP1869_TAG, CDP1869_DOT_CLK_PAL, pal_cdp1869_intf, cdp1869_page_ram)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_SOUND_WAVE_ADD(WAVE_TAG, CASSETTE_TAG)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END

MACHINE_CONFIG_FRAGMENT( comx35_ntsc_video )
	MCFG_CDP1869_SCREEN_NTSC_ADD(CDP1869_TAG, SCREEN_TAG, CDP1869_DOT_CLK_NTSC)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_CDP1869_ADD(CDP1869_TAG, CDP1869_DOT_CLK_NTSC, ntsc_cdp1869_intf, cdp1869_page_ram)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_SOUND_WAVE_ADD(WAVE_TAG, CASSETTE_TAG)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END
