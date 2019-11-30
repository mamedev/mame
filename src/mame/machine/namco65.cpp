// license:BSD-3-Clause
// copyright-holders:David Haywood, K.Wilkins

/*
TODO:
output support, Golly Ghost is currently hacking this based on DPRAM in the namcos2.cpp driver side!
some of this can likely be moved into the actual MCU core too

*/

#include "emu.h"
#include "machine/namco65.h"

DEFINE_DEVICE_TYPE(NAMCOC65, namcoc65_device, "namcoc65", "Namco C65 I/O")

namcoc65_device::namcoc65_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, NAMCOC65, tag, owner, clock),
	m_mcu(*this, "mcu"),
	m_in_pb_cb(*this),
	m_in_pc_cb(*this),
	m_in_ph_cb(*this),
	m_in_pdsw_cb(*this),
	m_port_analog_in_cb{{*this}, {*this}, {*this}, {*this}, {*this}, {*this}, {*this}, {*this}},
	m_port_dial_in_cb{{*this}, {*this}, {*this}, {*this}},
	m_dp_in(*this),
	m_dp_out(*this)
{
}

ROM_START( namcoc65 )
	ROM_REGION( 0x2000, "mcu", 0 )
	ROM_LOAD( "sys2mcpu.bin",  0x000000, 0x002000, CRC(a342a97e) SHA1(2c420d34dba21e409bf78ddca710fc7de65a6642) )
ROM_END


WRITE8_MEMBER( namcoc65_device::namcos2_mcu_port_d_w )
{
	/* Undefined operation on write */
}

READ8_MEMBER(namcoc65_device::namcos2_mcu_port_d_r)
{
	/* Provides a digital version of the analog ports */
	int threshold = 0x7f;
	int data = 0;

	/* Read/convert the bits one at a time */
	if (m_port_analog_in_cb[0]() > threshold) data |= 0x01;
	if (m_port_analog_in_cb[1]() > threshold) data |= 0x02;
	if (m_port_analog_in_cb[2]() > threshold) data |= 0x04;
	if (m_port_analog_in_cb[3]() > threshold) data |= 0x08;
	if (m_port_analog_in_cb[4]() > threshold) data |= 0x10;
	if (m_port_analog_in_cb[5]() > threshold) data |= 0x20;
	if (m_port_analog_in_cb[6]() > threshold) data |= 0x40;
	if (m_port_analog_in_cb[7]() > threshold) data |= 0x80;

	/* Return the result */
	return data;
}


READ8_MEMBER(namcoc65_device::namcos2_mcu_analog_ctrl_r)
{
	int data = 0;

	/* ADEF flag is only cleared AFTER a read from control THEN a read from DATA */
	if (m_mcu_analog_complete == 2) m_mcu_analog_complete = 1;
	if (m_mcu_analog_complete) data |= 0x80;

	/* Mask on the lower 6 register bits, Irq EN/Channel/Clock */
	data |= m_mcu_analog_ctrl & 0x3f;
	/* Return the value */
	return data;
}

WRITE8_MEMBER( namcoc65_device::namcos2_mcu_analog_port_w )
{
}

READ8_MEMBER(namcoc65_device::namcos2_mcu_analog_port_r)
{
	if (m_mcu_analog_complete == 1) m_mcu_analog_complete = 0;
	return m_mcu_analog_data;
}


WRITE8_MEMBER(namcoc65_device::namcos2_mcu_analog_ctrl_w)
{
	m_mcu_analog_ctrl = data & 0xff;

	/* Check if this is a start of conversion */
	/* Input ports 2 through 9 are the analog channels */

	if (data & 0x40)
	{
		/* Set the conversion complete flag */
		m_mcu_analog_complete = 2;
		/* We convert instantly, good eh! (not really) */
		switch ((data >> 2) & 0x07)
		{
		case 0:
			m_mcu_analog_data = m_port_analog_in_cb[0]();
			break;
		case 1:
			m_mcu_analog_data = m_port_analog_in_cb[1]();
			break;
		case 2:
			m_mcu_analog_data = m_port_analog_in_cb[2]();
			break;
		case 3:
			m_mcu_analog_data = m_port_analog_in_cb[3]();
			break;
		case 4:
			m_mcu_analog_data = m_port_analog_in_cb[4]();
			break;
		case 5:
			m_mcu_analog_data = m_port_analog_in_cb[5]();
			break;
		case 6:
			m_mcu_analog_data = m_port_analog_in_cb[6]();
			break;
		case 7:
			m_mcu_analog_data = m_port_analog_in_cb[7]();
			break;
		default:
			//output().set_value("anunk",data);
			break;
		}

		/* If the interrupt enable bit is set trigger an A/D IRQ */
		if (data & 0x20)
		{
			m_mcu->pulse_input_line(HD63705_INT_ADCONV, m_mcu->minimum_quantum_time());
		}
	}
}

READ8_MEMBER(namcoc65_device::dpram_byte_r)
{
	return m_dp_in(offset);
}

WRITE8_MEMBER(namcoc65_device::dpram_byte_w)
{
	m_dp_out(offset,data);
}

void namcoc65_device::mcu_map(address_map &map)
{
	map(0x0000, 0x003f).ram(); /* Fill in register to stop logging */
	map(0x0000, 0x0000).nopr(); /* Keep logging quiet */
	map(0x0001, 0x0001).r(FUNC(namcoc65_device::mcub_r)); /* Usually P1/P2 direction inputs (UDL) + start buttons */
	map(0x0002, 0x0002).r(FUNC(namcoc65_device::mcuc_r)); /* Usually coins + start */
	map(0x0003, 0x0003).rw(FUNC(namcoc65_device::namcos2_mcu_port_d_r), FUNC(namcoc65_device::namcos2_mcu_port_d_w));
	map(0x0007, 0x0007).r(FUNC(namcoc65_device::mcuh_r)); /* Usually P1/P2 direction input (R) + Buttons 1,2,3 */
	map(0x0010, 0x0010).rw(FUNC(namcoc65_device::namcos2_mcu_analog_ctrl_r), FUNC(namcoc65_device::namcos2_mcu_analog_ctrl_w));
	map(0x0011, 0x0011).rw(FUNC(namcoc65_device::namcos2_mcu_analog_port_r), FUNC(namcoc65_device::namcos2_mcu_analog_port_w));
	map(0x0040, 0x01bf).ram();
	map(0x01c0, 0x1fff).rom(); /* internal ROM */
	map(0x2000, 0x2000).r(FUNC(namcoc65_device::mcudsw_r)); /* Dipswitch, including service mode */
	map(0x3000, 0x3000).r(FUNC(namcoc65_device::mcudi0_r));
	map(0x3001, 0x3001).r(FUNC(namcoc65_device::mcudi1_r));
	map(0x3002, 0x3002).r(FUNC(namcoc65_device::mcudi2_r));
	map(0x3003, 0x3003).r(FUNC(namcoc65_device::mcudi3_r));
	map(0x5000, 0x57ff).rw(FUNC(namcoc65_device::dpram_byte_r), FUNC(namcoc65_device::dpram_byte_w));
	map(0x6000, 0x6fff).nopr(); /* watchdog */
	map(0x8000, 0xffff).rom().region("external", 0);; /* external ROM socket */
}


void namcoc65_device::device_add_mconfig(machine_config &config)
{
	HD63705(config, m_mcu, DERIVED_CLOCK(1, 1));
	m_mcu->set_addrmap(AS_PROGRAM, &namcoc65_device::mcu_map);
}

void namcoc65_device::device_resolve_objects()
{
	m_in_pb_cb.resolve_safe(0xff);
	m_in_pc_cb.resolve_safe(0xff);
	m_in_ph_cb.resolve_safe(0xff);
	m_in_pdsw_cb.resolve_safe(0xff);

	for (auto &cb : m_port_analog_in_cb)
		cb.resolve_safe(0xff);

	for (auto &cb : m_port_dial_in_cb)
		cb.resolve_safe(0xff);

	m_dp_in.resolve_safe(0xff);
	m_dp_out.resolve_safe();
}

void namcoc65_device::device_start()
{
}

void namcoc65_device::device_reset()
{
	m_mcu_analog_ctrl = 0;
	m_mcu_analog_data = 0xaa;
	m_mcu_analog_complete = 0;
}

const tiny_rom_entry *namcoc65_device::device_rom_region() const
{
	return ROM_NAME(namcoc65);
}
