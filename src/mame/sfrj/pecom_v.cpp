// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Pecom driver by Miodrag Milanovic

        08/11/2008 Preliminary driver.

****************************************************************************/

#include "emu.h"
#include "pecom.h"


void pecom_state::cdp1869_w(offs_t offset, uint8_t data)
{
	uint16_t ma = m_maincpu->get_memory_address();

	switch (offset + 3)
	{
	case 3:
		m_cdp1869->out3_w(data);
		break;

	case 4:
		m_cdp1869->out4_w(ma);
		break;

	case 5:
		m_cdp1869->out5_w(ma);
		break;

	case 6:
		m_cdp1869->out6_w(ma);
		break;

	case 7:
		m_cdp1869->out7_w(ma);
		break;
	}
}

CDP1869_CHAR_RAM_READ_MEMBER(pecom_state::char_ram_r )
{
	uint8_t column = pmd & 0x7f;
	uint16_t charaddr = (column << 4) | cma;

	return m_charram[charaddr];
}

CDP1869_CHAR_RAM_WRITE_MEMBER(pecom_state::char_ram_w )
{
	uint8_t column = pmd & 0x7f;
	uint16_t charaddr = (column << 4) | cma;

	m_charram[charaddr] = data;
}

CDP1869_PCB_READ_MEMBER(pecom_state::pcb_r )
{
	return BIT(pmd, 7);
}

WRITE_LINE_MEMBER(pecom_state::prd_w)
{
	// every other PRD triggers a DMAOUT request
	if (m_dma)
		m_maincpu->set_input_line(COSMAC_INPUT_LINE_DMAOUT, HOLD_LINE);

	m_dma = !m_dma;
}

void pecom_state::machine_start()
{
	m_bank1->configure_entry(0, m_ram);
	m_bank1->configure_entry(1, m_rom);

	/* allocate memory */
	m_charram = std::make_unique<uint8_t[]>(0x0800);

	/* register for state saving */
	save_item(NAME(m_reset));
	save_item(NAME(m_dma));
	save_pointer(NAME(m_charram), 0x0800);
	m_reset_timer = timer_alloc(FUNC(pecom_state::reset_tick), this);
}

