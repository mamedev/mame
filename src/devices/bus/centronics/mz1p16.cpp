// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    Sharp MZ-1P16 4-color plot printer (skeleton)

    This printer was sold for use with the Sharp MZ-800.

**********************************************************************/

#include "emu.h"
#include "mz1p16.h"

#include "cpu/mcs48/mcs48.h"


//**************************************************************************
//  DEVICE DEFINITION
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(MZ1P16, mz1p16_device, "mz1p16", "Sharp MZ-1P16 Plotter Printer")


//-------------------------------------------------
//  mz1p16_device - constructor
//-------------------------------------------------

mz1p16_device::mz1p16_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MZ1P16, tag, owner, clock)
	, device_centronics_peripheral_interface(mconfig, *this)
	, m_mcu(*this, "mcu")
	, m_buffer(*this, "buffer")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mz1p16_device::device_start()
{
}


//-------------------------------------------------
//  input_data0 - RD1 line handler
//-------------------------------------------------

void mz1p16_device::input_data0(int state)
{
	m_buffer->write_bit0(state);
}

//-------------------------------------------------
//  input_data1 - RD2 line handler
//-------------------------------------------------

void mz1p16_device::input_data1(int state)
{
	m_buffer->write_bit1(state);
}

//-------------------------------------------------
//  input_data2 - RD3 line handler
//-------------------------------------------------

void mz1p16_device::input_data2(int state)
{
	m_buffer->write_bit2(state);
}

//-------------------------------------------------
//  input_data3 - RD4 line handler
//-------------------------------------------------

void mz1p16_device::input_data3(int state)
{
	m_buffer->write_bit3(state);
}

//-------------------------------------------------
//  input_data4 - RD5 line handler
//-------------------------------------------------

void mz1p16_device::input_data4(int state)
{
	m_buffer->write_bit4(state);
}

//-------------------------------------------------
//  input_data5 - RD6 line handler
//-------------------------------------------------

void mz1p16_device::input_data5(int state)
{
	m_buffer->write_bit5(state);
}

//-------------------------------------------------
//  input_data6 - RD7 line handler
//-------------------------------------------------

void mz1p16_device::input_data6(int state)
{
	m_buffer->write_bit6(state);
}

//-------------------------------------------------
//  input_data7 - RD8 line handler
//-------------------------------------------------

void mz1p16_device::input_data7(int state)
{
	m_buffer->write_bit7(state);
}

//-------------------------------------------------
//  input_strobe - RDP line handler
//-------------------------------------------------

void mz1p16_device::input_strobe(int state)
{
	m_mcu->set_input_line(MCS48_INPUT_IRQ, state ? ASSERT_LINE : CLEAR_LINE);
}

//-------------------------------------------------
//  input_init - IRT line handler
//-------------------------------------------------

void mz1p16_device::input_init(int state)
{
	m_mcu->set_input_line(INPUT_LINE_RESET, state ? ASSERT_LINE : CLEAR_LINE);
}


//-------------------------------------------------
//  xy_step_w - set X and Y stepper motor outputs
//-------------------------------------------------

void mz1p16_device::xy_step_w(u8 data)
{
	// TODO
}


//-------------------------------------------------
//  control_w - set miscellaneous control outputs
//-------------------------------------------------

void mz1p16_device::control_w(u8 data)
{
	// P14 = /RDA (ready to receive)
	output_busy(BIT(data, 4));

	// P17 = /STA (status check)
	output_busy(BIT(data, 7));
}


//-------------------------------------------------
//  device_add_mconfig - device-specific config
//-------------------------------------------------

void mz1p16_device::device_add_mconfig(machine_config &config)
{
	mcs48_cpu_device &mcu(I8050(config, m_mcu, 6_MHz_XTAL));
	mcu.bus_out_cb().set(FUNC(mz1p16_device::xy_step_w));
	mcu.p1_out_cb().set(FUNC(mz1p16_device::control_w));
	mcu.p2_in_cb().set(m_buffer, FUNC(input_buffer_device::read));
	mcu.t0_in_cb().set_ioport("KEYSW").bit(0);
	mcu.t1_in_cb().set_ioport("KEYSW").bit(1);

	INPUT_BUFFER(config, m_buffer);
}


//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  reset_sw_w - handle local reset button
//-------------------------------------------------

void mz1p16_device::reset_sw_w(int state)
{
	// TODO
}


static INPUT_PORTS_START(mz1p16)
	PORT_START("KEYSW")
	PORT_BIT(1, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Pen Change")
	PORT_BIT(2, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Paper Feed")
	PORT_BIT(4, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Reset") PORT_WRITE_LINE_MEMBER(FUNC(mz1p16_device::reset_sw_w))
INPUT_PORTS_END

//-------------------------------------------------
//  device_input_ports - device-specific ports
//-------------------------------------------------

ioport_constructor mz1p16_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(mz1p16);
}


//**************************************************************************
//  ROM DEFINITION
//**************************************************************************

ROM_START(mz1p16)
	ROM_REGION(0x1000, "mcu", 0)
	ROM_LOAD("m5m8050h-059p.ic1", 0x0000, 0x1000, CRC(beb12a84) SHA1(2e059c2f7d653294c77fe8e1a37436b4c028e122))
ROM_END

//-------------------------------------------------
//  device_rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *mz1p16_device::device_rom_region() const
{
	return ROM_NAME(mz1p16);
}
