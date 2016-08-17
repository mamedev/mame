// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Ondra driver by Miodrag Milanovic

        08/09/2008 Preliminary driver.

****************************************************************************/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "includes/ondra.h"


READ8_MEMBER(ondra_state::ondra_keyboard_r)
{
	UINT8 retVal = 0x00;
	double valcas = m_cassette->input();

	if ( valcas < 0.00) {
		retVal = 0x80;
	}

	switch ( offset & 0x0f )
	{
		case 0: retVal |= m_line0->read(); break;
		case 1: retVal |= m_line1->read(); break;
		case 2: retVal |= m_line2->read(); break;
		case 3: retVal |= m_line3->read(); break;
		case 4: retVal |= m_line4->read(); break;
		case 5: retVal |= m_line5->read(); break;
		case 6: retVal |= m_line6->read(); break;
		case 7: retVal |= m_line7->read(); break;
		case 8: retVal |= m_line8->read(); break;
		case 9: retVal |= m_line9->read(); break;
		default: retVal |= 0x1f; break;
	}

	return retVal;
}

void ondra_state::ondra_update_banks()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	UINT8 *mem = m_region_maincpu->base();

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
		m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
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
	machine().scheduler().timer_pulse(attotime::from_hz(10), timer_expired_delegate(FUNC(ondra_state::nmi_check_callback),this));
}
