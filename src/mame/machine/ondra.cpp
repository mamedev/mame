// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Ondra driver by Miodrag Milanovic

        08/09/2008 Preliminary driver.

****************************************************************************/


#include "emu.h"
#include "includes/ondra.h"

#include "cpu/z80/z80.h"


READ8_MEMBER(ondra_state::ondra_keyboard_r)
{
	uint8_t retVal = 0x00;

	double const valcas = m_cassette->input();
	if (valcas < 0.00)
		retVal |= 0x80;

	if ((offset & 0x0f) < m_lines.size())
		retVal |= m_lines[offset & 0x0f]->read();
	else
		retVal |= 0x1f;

	return retVal;
}

void ondra_state::ondra_update_banks()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	uint8_t *mem = m_region_maincpu->base();

	if (m_bank1_status==0) {
		space.unmap_write(0x0000, 0x3fff);
		m_bank1->set_base(mem + 0x010000);
	} else {
		space.install_write_bank(0x0000, 0x3fff, "bank1");
		m_bank1->set_base(m_ram->pointer() + 0x0000);
	}
	m_bank2->set_base(m_ram->pointer() + 0x4000);
	if (m_bank2_status==0) {
		space.install_readwrite_bank(0xe000, 0xffff, "bank3");
		m_bank3->set_base(m_ram->pointer() + 0xe000);
	} else {
		space.unmap_write(0xe000, 0xffff);
		space.install_read_handler (0xe000, 0xffff, read8_delegate(FUNC(ondra_state::ondra_keyboard_r),this));
	}
}

WRITE8_MEMBER(ondra_state::ondra_port_03_w)
{
	m_video_enable = data & 1;
	m_bank1_status = (data >> 1) & 1;
	m_bank2_status = (data >> 2) & 1;
	ondra_update_banks();
	m_cassette->output(((data >> 3) & 1) ? -1.0 : +1.0);
}

WRITE8_MEMBER(ondra_state::ondra_port_09_w)
{
}

WRITE8_MEMBER(ondra_state::ondra_port_0a_w)
{
}

TIMER_CALLBACK_MEMBER(ondra_state::nmi_check_callback)
{
	if ((m_nmi->read() & 1) == 1)
	{
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
	}
}

void ondra_state::machine_reset()
{
	m_bank1_status = 0;
	m_bank2_status = 0;
	ondra_update_banks();
}

void ondra_state::machine_start()
{
	m_nmi_check_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(ondra_state::nmi_check_callback), this));
	m_nmi_check_timer->adjust(attotime::from_hz(10), 0, attotime::from_hz(10));
}
