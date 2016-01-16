// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi, Nicola Salmoria
#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/buggychl.h"


const device_type BUGGYCHL_MCU = &device_creator<buggychl_mcu_device>;

buggychl_mcu_device::buggychl_mcu_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, BUGGYCHL_MCU, "M68705 MCU Simulation (Buggy Challenge)", tag, owner, clock, "buggychl_mcu", __FILE__),
	m_port_a_in(0),
	m_port_a_out(0),
	m_ddr_a(0),
	m_port_b_in(0),
	m_port_b_out(0),
	m_ddr_b(0),
	m_port_c_in(0),
	m_port_c_out(0),
	m_ddr_c(0),
	m_from_main(0),
	m_from_mcu(0),
	m_mcu_sent(0),
	m_main_sent(0)
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void buggychl_mcu_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void buggychl_mcu_device::device_start()
{
	m_mcu = machine().device("mcu");

	save_item(NAME(m_from_main));
	save_item(NAME(m_from_mcu));
	save_item(NAME(m_mcu_sent));
	save_item(NAME(m_main_sent));
	save_item(NAME(m_port_a_in));
	save_item(NAME(m_port_a_out));
	save_item(NAME(m_ddr_a));
	save_item(NAME(m_port_b_in));
	save_item(NAME(m_port_b_out));
	save_item(NAME(m_ddr_b));
	save_item(NAME(m_port_c_in));
	save_item(NAME(m_port_c_out));
	save_item(NAME(m_ddr_c));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void buggychl_mcu_device::device_reset()
{
	m_mcu_sent = 0;
	m_main_sent = 0;
	m_from_main = 0;
	m_from_mcu = 0;
	m_port_a_in = 0;
	m_port_a_out = 0;
	m_ddr_a = 0;
	m_port_b_in = 0;
	m_port_b_out = 0;
	m_ddr_b = 0;
	m_port_c_in = 0;
	m_port_c_out = 0;
	m_ddr_c = 0;
}


/***************************************************************************

 Buggy Challenge 68705 protection interface

 This is accurate. FairyLand Story seems to be identical.

***************************************************************************/

READ8_MEMBER( buggychl_mcu_device::buggychl_68705_port_a_r )
{
	//logerror("%04x: 68705 port A read %02x\n", m_mcu->safe_pc(), m_port_a_in);
	return (m_port_a_out & m_ddr_a) | (m_port_a_in & ~m_ddr_a);
}

WRITE8_MEMBER( buggychl_mcu_device::buggychl_68705_port_a_w )
{
	//logerror("%04x: 68705 port A write %02x\n", m_mcu->safe_pc(), data);
	m_port_a_out = data;
}

WRITE8_MEMBER( buggychl_mcu_device::buggychl_68705_ddr_a_w )
{
	m_ddr_a = data;
}



/*
 *  Port B connections:
 *  parts in [ ] are optional (not used by buggychl)
 *
 *  all bits are logical 1 when read (+5V pullup)
 *
 *  0   n.c.
 *  1   W  IRQ ack and enable latch which holds data from main Z80 memory
 *  2   W  loads latch to Z80
 *  3   W  to Z80 BUSRQ (put it on hold?)
 *  4   W  n.c.
 *  5   W  [selects Z80 memory access direction (0 = write 1 = read)]
 *  6   W  [loads the latch which holds the low 8 bits of the address of
 *               the main Z80 memory location to access]
 *  7   W  [loads the latch which holds the high 8 bits of the address of
 *               the main Z80 memory location to access]
 */


READ8_MEMBER( buggychl_mcu_device::buggychl_68705_port_b_r )
{
	return (m_port_b_out & m_ddr_b) | (m_port_b_in & ~m_ddr_b);
}

WRITE8_MEMBER( buggychl_mcu_device::buggychl_68705_port_b_w )
{
	logerror("%04x: 68705 port B write %02x\n", m_mcu->safe_pc(), data);

	if ((m_ddr_b & 0x02) && (~data & 0x02) && (m_port_b_out & 0x02))
	{
		m_port_a_in = m_from_main;
		if (m_main_sent)
			m_mcu->execute().set_input_line(0, CLEAR_LINE);
		m_main_sent = 0;
		logerror("read command %02x from main cpu\n", m_port_a_in);
	}
	if ((m_ddr_b & 0x04) && (data & 0x04) && (~m_port_b_out & 0x04))
	{
		logerror("send command %02x to main cpu\n", m_port_a_out);
		m_from_mcu = m_port_a_out;
		m_mcu_sent = 1;
	}

	m_port_b_out = data;
}

WRITE8_MEMBER( buggychl_mcu_device::buggychl_68705_ddr_b_w )
{
	m_ddr_b = data;
}


/*
 *  Port C connections:
 *
 *  all bits are logical 1 when read (+5V pullup)
 *
 *  0   R  1 when pending command Z80->68705
 *  1   R  0 when pending command 68705->Z80
 */

READ8_MEMBER( buggychl_mcu_device::buggychl_68705_port_c_r )
{
	m_port_c_in = 0;
	if (m_main_sent)
		m_port_c_in |= 0x01;
	if (!m_mcu_sent)
		m_port_c_in |= 0x02;
	logerror("%04x: 68705 port C read %02x\n", m_mcu->safe_pc(), m_port_c_in);
	return (m_port_c_out & m_ddr_c) | (m_port_c_in & ~m_ddr_c);
}

WRITE8_MEMBER( buggychl_mcu_device::buggychl_68705_port_c_w )
{
	logerror("%04x: 68705 port C write %02x\n", m_mcu->safe_pc(), data);
	m_port_c_out = data;
}

WRITE8_MEMBER( buggychl_mcu_device::buggychl_68705_ddr_c_w )
{
	m_ddr_c = data;
}


WRITE8_MEMBER( buggychl_mcu_device::buggychl_mcu_w )
{
	logerror("%04x: mcu_w %02x\n", m_mcu->safe_pc(), data);
	m_from_main = data;
	m_main_sent = 1;
	m_mcu->execute().set_input_line(0, ASSERT_LINE);
}

READ8_MEMBER( buggychl_mcu_device::buggychl_mcu_r )
{
	logerror("%04x: mcu_r %02x\n", m_mcu->safe_pc(), m_from_mcu);
	m_mcu_sent = 0;
	return m_from_mcu;
}

READ8_MEMBER( buggychl_mcu_device::buggychl_mcu_status_r )
{
	int res = 0;

	/* bit 0 = when 1, mcu is ready to receive data from main cpu */
	/* bit 1 = when 1, mcu has sent data to the main cpu */
	//logerror("%04x: mcu_status_r\n",m_mcu->safe_pc());
	if (!m_main_sent)
		res |= 0x01;
	if (m_mcu_sent)
		res |= 0x02;

	return res;
}

ADDRESS_MAP_START( buggychl_mcu_map, AS_PROGRAM, 8, buggychl_mcu_device )
	ADDRESS_MAP_GLOBAL_MASK(0x7ff)
	AM_RANGE(0x0000, 0x0000) AM_DEVREADWRITE("bmcu", buggychl_mcu_device, buggychl_68705_port_a_r, buggychl_68705_port_a_w)
	AM_RANGE(0x0001, 0x0001) AM_DEVREADWRITE("bmcu", buggychl_mcu_device, buggychl_68705_port_b_r, buggychl_68705_port_b_w)
	AM_RANGE(0x0002, 0x0002) AM_DEVREADWRITE("bmcu", buggychl_mcu_device, buggychl_68705_port_c_r, buggychl_68705_port_c_w)
	AM_RANGE(0x0004, 0x0004) AM_DEVWRITE("bmcu", buggychl_mcu_device, buggychl_68705_ddr_a_w)
	AM_RANGE(0x0005, 0x0005) AM_DEVWRITE("bmcu", buggychl_mcu_device, buggychl_68705_ddr_b_w)
	AM_RANGE(0x0006, 0x0006) AM_DEVWRITE("bmcu", buggychl_mcu_device, buggychl_68705_ddr_c_w)
	AM_RANGE(0x0010, 0x007f) AM_RAM
	AM_RANGE(0x0080, 0x07ff) AM_ROM
ADDRESS_MAP_END
