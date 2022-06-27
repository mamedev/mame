// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Pecom driver by Miodrag Milanovic

        08/11/2008 Preliminary driver.

****************************************************************************/

#include "emu.h"
#include "pecom.h"


TIMER_CALLBACK_MEMBER(pecom_state::reset_tick)
{
	m_reset = 1;
}

void pecom_state::machine_reset()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	m_bank1->set_entry(1);

	space.unmap_write(0xf000, 0xffff);
	space.install_read_bank (0xf000, 0xf7ff, m_bank3);
	space.install_read_bank (0xf800, 0xffff, m_bank4);
	m_bank3->set_base(m_rom + 0x7000);
	m_bank4->set_base(m_rom + 0x7800);

	m_reset = 0;
	m_dma = 0;
	m_reset_timer->adjust(attotime::from_msec(5));
}

uint8_t pecom_state::cdp1869_charram_r(offs_t offset)
{
	return m_cdp1869->char_ram_r(offset);
}

void pecom_state::cdp1869_charram_w(offs_t offset, uint8_t data)
{
	return m_cdp1869->char_ram_w(offset, data);
}

uint8_t pecom_state::cdp1869_pageram_r(offs_t offset)
{
	return m_cdp1869->page_ram_r(offset);
}

void pecom_state::cdp1869_pageram_w(offs_t offset, uint8_t data)
{
	return m_cdp1869->page_ram_w(offset, data);
}

void pecom_state::bank_w(uint8_t data)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	m_bank1->set_entry(0);

	if (data==2)
	{
		space.install_read_handler (0xf000, 0xf7ff, read8sm_delegate(*this, FUNC(pecom_state::cdp1869_charram_r)));
		space.install_write_handler(0xf000, 0xf7ff, write8sm_delegate(*this, FUNC(pecom_state::cdp1869_charram_w)));
		space.install_read_handler (0xf800, 0xffff, read8sm_delegate(*this, FUNC(pecom_state::cdp1869_pageram_r)));
		space.install_write_handler(0xf800, 0xffff, write8sm_delegate(*this, FUNC(pecom_state::cdp1869_pageram_w)));
	}
	else
	{
		space.unmap_write(0xf000, 0xffff);
		space.install_read_bank (0xf000, 0xf7ff, m_bank3);
		space.install_read_bank (0xf800, 0xffff, m_bank4);
		m_bank3->set_base(m_rom + 0x7000);
		m_bank4->set_base(m_rom + 0x7800);
	}
}

uint8_t pecom_state::keyboard_r()
{
	/*
	   INP command BUS -> M(R(X)) BUS -> D
	   so on each input, address is also set, 8 lower bits are used as input for keyboard
	   Address is available on address bus during reading of value from port, and that is
	   used to determine keyboard line reading
	*/
	uint16_t addr = m_maincpu->state_int(cosmac_device::COSMAC_R0 + m_maincpu->state_int(cosmac_device::COSMAC_X));
	/* just in case someone is reading non existing ports */
	if (addr<0x7cca || addr>0x7ce3) return 0;
	return m_io_keyboard[addr - 0x7cca]->read();
}

/* CDP1802 Interface */

READ_LINE_MEMBER(pecom_state::clear_r)
{
	return m_reset;
}

READ_LINE_MEMBER(pecom_state::ef2_r)
{
	bool shift = BIT(m_io_cnt->read(), 1);
	double cas = m_cassette->input();

	return (cas < -0.02) | shift; // touching shift kills cassette load
}

WRITE_LINE_MEMBER(pecom_state::q_w)
{
	m_cassette->output(state ? -1.0 : +1.0);
}

void pecom_state::sc_w(uint8_t data)
{
	switch (data)
	{
		case COSMAC_STATE_CODE_S2_DMA:
			// DMA acknowledge clears the DMAOUT request
			m_maincpu->set_input_line(COSMAC_INPUT_LINE_DMAOUT, CLEAR_LINE);
			break;

		default:
			break;
	}
}

/*
static COSMAC_EF_READ( pecom64_ef_r )
{
    int flags = 0x0f;
    double valcas = (cassette_device_image(device->machine()));->input()
    uint8_t val = device->machine().root_device().ioport("CNT")->read();

    if ((val & 0x04)==0x04 && pecom_prev_caps_state==0)
    {
        pecom_caps_state = (pecom_caps_state==4) ? 0 : 4; // Change CAPS state
    }
    pecom_prev_caps_state = val & 0x04;
    if (valcas!=0.0)
    { // If there is any cassette signal
        val = (val & 0x0D); // remove EF2 from SHIFT
        flags -= EF2;
        if ( valcas > 0.00) flags += EF2;
    }
    flags -= (val & 0x0b) + pecom_caps_state;
    return flags;
}
*/

