// license:LGPL-2.1+
// copyright-holders:Tomasz Slanina, Pierpaolo Prazzoli
/***************************************************************************

    Pit&Run

    Based on TaitsoSJ driver
    68705 has access to Z80 memory space

***************************************************************************/

#include "emu.h"
#include "cpu/m6805/m6805.h"
#include "includes/pitnrun.h"


void pitnrun_state::machine_start()
{
	save_item(NAME(m_nmi));
	save_item(NAME(m_fromz80));
	save_item(NAME(m_toz80));
	save_item(NAME(m_zaccept));
	save_item(NAME(m_zready));
	save_item(NAME(m_portA_in));
	save_item(NAME(m_portA_out));
	save_item(NAME(m_address));
}

void pitnrun_state::machine_reset()
{
	m_zaccept = 1;
	m_zready = 0;
	m_mcu->set_input_line(0, CLEAR_LINE);
}

TIMER_CALLBACK_MEMBER(pitnrun_state::mcu_real_data_r)
{
	m_zaccept = 1;
}

READ8_MEMBER(pitnrun_state::mcu_data_r)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(pitnrun_state::mcu_real_data_r),this));
	return m_toz80;
}

TIMER_CALLBACK_MEMBER(pitnrun_state::mcu_real_data_w)
{
	m_zready = 1;
	m_mcu->set_input_line(0, ASSERT_LINE);
	m_fromz80 = param;
}

WRITE8_MEMBER(pitnrun_state::mcu_data_w)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(pitnrun_state::mcu_real_data_w),this), data);
	machine().scheduler().boost_interleave(attotime::zero, attotime::from_usec(5));
}

READ8_MEMBER(pitnrun_state::mcu_status_r)
{
	/* mcu synchronization */
	/* bit 0 = the 68705 has read data from the Z80 */
	/* bit 1 = the 68705 has written data for the Z80 */
	return ~((m_zready << 1) | (m_zaccept << 0));
}


READ8_MEMBER(pitnrun_state::m68705_portA_r)
{
	return m_portA_in;
}

WRITE8_MEMBER(pitnrun_state::m68705_portA_w)
{
	m_portA_out = data;
}



/*
 *  Port B connections:
 *
 *  all bits are logical 1 when read (+5V pullup)
 *
 *  0   W  !68INTRQ
 *  1   W  !68LRD (enables latch which holds command from the Z80)
 *  2   W  !68LWR (loads the latch which holds data for the Z80, and sets a
 *                 status bit so the Z80 knows there's data waiting)
 *  3   W  to Z80 !BUSRQ (aka !WAIT) pin
 *  4   W  !68WRITE (triggers write to main Z80 memory area )
 *  5   W  !68READ (triggers read from main Z80 memory area )
 *  6   W  !LAL (loads the latch which holds the low 8 bits of the address of
 *               the main Z80 memory location to access)
 *  7   W  !UAL (loads the latch which holds the high 8 bits of the address of
 *               the main Z80 memory location to access)
 */

READ8_MEMBER(pitnrun_state::m68705_portB_r)
{
	return 0xff;
}


TIMER_CALLBACK_MEMBER(pitnrun_state::mcu_data_real_r)
{
	m_zready = 0;
}

TIMER_CALLBACK_MEMBER(pitnrun_state::mcu_status_real_w)
{
	m_toz80 = param;
	m_zaccept = 0;
}

WRITE8_MEMBER(pitnrun_state::m68705_portB_w)
{
	address_space &cpu0space = m_maincpu->space(AS_PROGRAM);
	if (~data & 0x02)
	{
		/* 68705 is going to read data from the Z80 */
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(pitnrun_state::mcu_data_real_r),this));
		m_mcu->set_input_line(0,CLEAR_LINE);
		m_portA_in = m_fromz80;
	}
	if (~data & 0x04)
	{
		/* 68705 is writing data for the Z80 */
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(pitnrun_state::mcu_status_real_w),this), m_portA_out);
	}
	if (~data & 0x10)
	{
		cpu0space.write_byte(m_address, m_portA_out);
	}
	if (~data & 0x20)
	{
		m_portA_in = cpu0space.read_byte(m_address);
	}
	if (~data & 0x40)
	{
		m_address = (m_address & 0xff00) | m_portA_out;
	}
	if (~data & 0x80)
	{
		m_address = (m_address & 0x00ff) | (m_portA_out << 8);
	}
}

/*
 *  Port C connections:
 *
 *  0   R  ZREADY (1 when the Z80 has written a command in the latch)
 *  1   R  ZACCEPT (1 when the Z80 has read data from the latch)
 *  2   R  from Z80 !BUSAK pin
 *  3   R  68INTAK (goes 0 when the interrupt request done with 68INTRQ
 *                  passes through)
 */

READ8_MEMBER(pitnrun_state::m68705_portC_r)
{
	return (m_zready << 0) | (m_zaccept << 1);
}
