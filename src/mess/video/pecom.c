/***************************************************************************

        Pecom driver by Miodrag Milanovic

        08/11/2008 Preliminary driver.

****************************************************************************/

#include "emu.h"
#include "sound/cdp1869.h"
#include "sound/wave.h"
#include "cpu/cosmac/cosmac.h"
#include "includes/pecom.h"

WRITE8_MEMBER(pecom_state::pecom_cdp1869_w)
{

	UINT16 ma = m_cdp1802->get_memory_address();

	switch (offset + 3)
	{
	case 3:
		m_cdp1869->out3_w(space, ma, data);
		break;

	case 4:
		m_cdp1869->out4_w(space, ma, data);
		break;

	case 5:
		m_cdp1869->out5_w(space, ma, data);
		break;

	case 6:
		m_cdp1869->out6_w(space, ma, data);
		break;

	case 7:
		m_cdp1869->out7_w(space, ma, data);
		break;
	}
}

static ADDRESS_MAP_START( cdp1869_page_ram, AS_0, 8, driver_device )
	AM_RANGE(0x000, 0x3ff) AM_MIRROR(0x400) AM_RAM
ADDRESS_MAP_END

static CDP1869_CHAR_RAM_READ( pecom_char_ram_r )
{
	pecom_state *state = device->machine().driver_data<pecom_state>();

	UINT8 column = pmd & 0x7f;
	UINT16 charaddr = (column << 4) | cma;

	return state->m_charram[charaddr];
}

static CDP1869_CHAR_RAM_WRITE( pecom_char_ram_w )
{
	pecom_state *state = device->machine().driver_data<pecom_state>();

	UINT8 column = pmd & 0x7f;
	UINT16 charaddr = (column << 4) | cma;

	state->m_charram[charaddr] = data;
}

static CDP1869_PCB_READ( pecom_pcb_r )
{
	return BIT(pmd, 7);
}

WRITE_LINE_MEMBER(pecom_state::pecom_prd_w)
{
	// every other PRD triggers a DMAOUT request
	if (m_dma)
	{
		machine().device(CDP1802_TAG)->execute().set_input_line(COSMAC_INPUT_LINE_DMAOUT, HOLD_LINE);
	}

	m_dma = !m_dma;
}

static CDP1869_INTERFACE( pecom_cdp1869_intf )
{
	SCREEN_TAG,
	CDP1869_COLOR_CLK_PAL,
	CDP1869_PAL,
	pecom_pcb_r,
	pecom_char_ram_r,
	pecom_char_ram_w,
	DEVCB_DRIVER_LINE_MEMBER(pecom_state,pecom_prd_w)
};

VIDEO_START_MEMBER(pecom_state,pecom)
{

	/* allocate memory */
	m_charram = auto_alloc_array(machine(), UINT8, PECOM_CHAR_RAM_SIZE);

	/* register for state saving */
	save_item(NAME(m_reset));
	save_item(NAME(m_dma));
	save_pointer(NAME(m_charram), PECOM_CHAR_RAM_SIZE);
}

MACHINE_CONFIG_FRAGMENT( pecom_video )
	MCFG_CDP1869_SCREEN_PAL_ADD(CDP1869_TAG, SCREEN_TAG, CDP1869_DOT_CLK_PAL)

	MCFG_VIDEO_START_OVERRIDE(pecom_state,pecom)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_CDP1869_ADD(CDP1869_TAG, CDP1869_DOT_CLK_PAL, pecom_cdp1869_intf, cdp1869_page_ram)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, CASSETTE_TAG)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END
