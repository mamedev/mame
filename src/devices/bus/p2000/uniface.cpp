// license:BSD-3-Clause
// copyright-holders:Bart Eversdijk
/**********************************************************************

    P2000 PTCC Universal IO (UNIFACE) Cartridge

**********************************************************************/

#include "emu.h"
#include "uniface.h"

//#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(P2000_UNIFACE, p2000_uniface_device, "p2000_uniface", "P2000 Universal I/O Interface (uniface)")

//-------------------------------------------------
//  p2000_uniface_device - constructor
//-------------------------------------------------
p2000_uniface_device::p2000_uniface_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, P2000_UNIFACE, tag, owner, clock)
    , device_p2000_expansion_slot_card_interface(mconfig, *this)
    , m_address_bus(*this, "address")
    , m_data_out(*this, "dataout")
    , m_data_in(*this, "datain") 
    , m_status_bus(*this, "status")
{
}

INPUT_PORTS_START( p2000_uniface )
    PORT_START("datain")
    PORT_DIPNAME(   0x01, 0x00, "INP-D0") PORT_DIPLOCATION("datain:1")
	PORT_DIPSETTING(0x01, "High")
	PORT_DIPSETTING(0x00, "Low")
    PORT_DIPNAME(   0x02, 0x00, "INP-D1") PORT_DIPLOCATION("datain:2")
	PORT_DIPSETTING(0x02, "High")
	PORT_DIPSETTING(0x00, "Low")
    PORT_DIPNAME(   0x04, 0x00, "INP-D2") PORT_DIPLOCATION("datain:3")
	PORT_DIPSETTING(0x04, "High")
	PORT_DIPSETTING(0x00, "Low")
    PORT_DIPNAME(   0x08, 0x00, "INP-D3") PORT_DIPLOCATION("datain:4")
	PORT_DIPSETTING(0x08, "High")
	PORT_DIPSETTING(0x00, "Low")
    PORT_DIPNAME(   0x10, 0x00, "INP-D4") PORT_DIPLOCATION("datain:5")
	PORT_DIPSETTING(0x10, "High")
	PORT_DIPSETTING(0x00, "Low")
    PORT_DIPNAME(   0x20, 0x00, "INP-D5") PORT_DIPLOCATION("datain:6")
	PORT_DIPSETTING(0x20, "High")
	PORT_DIPSETTING(0x00, "Low")
    PORT_DIPNAME(   0x40, 0x00, "INP-D6") PORT_DIPLOCATION("datain:7")
	PORT_DIPSETTING(0x40, "High")
	PORT_DIPSETTING(0x00, "Low")
    PORT_DIPNAME(   0x80, 0x00, "INP-D7") PORT_DIPLOCATION("datain:8")
	PORT_DIPSETTING(0x80, "High")
	PORT_DIPSETTING(0x00, "Low")

    PORT_START("status")
	PORT_DIPNAME(   0x01, 0x01, "STATUS-0") PORT_DIPLOCATION("status:1")
	PORT_DIPSETTING(0x00, "Analog")
	PORT_DIPSETTING(0x01, "Digital")
    PORT_DIPNAME(   0x02, 0x02, "STATUS-1") PORT_DIPLOCATION("status:2")
	PORT_DIPSETTING(0x00, "Input")
	PORT_DIPSETTING(0x02, "Ouput")
    PORT_DIPNAME(   0x40, 0x00, "STATUS-6") PORT_DIPLOCATION("status:3")
	PORT_DIPSETTING(0x00, "Present")
	PORT_DIPSETTING(0x40, "Absent")
    PORT_DIPNAME(   0x80, 0x00, "STATUS-7") PORT_DIPLOCATION("status:4")
	PORT_DIPSETTING(0x00, "Present")
	PORT_DIPSETTING(0x80, "Absent")
INPUT_PORTS_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------
void p2000_uniface_device::device_add_mconfig(machine_config &config)
{
    OUTPUT_LATCH(config, m_address_bus);
	m_address_bus->bit_handler<0>().set_output("unifaceA0");
    m_address_bus->bit_handler<1>().set_output("unifaceA1");
    m_address_bus->bit_handler<2>().set_output("unifaceA2");
    m_address_bus->bit_handler<3>().set_output("unifaceA3");
    m_address_bus->bit_handler<4>().set_output("unifaceA4");
    m_address_bus->bit_handler<5>().set_output("unifaceA5");
    m_address_bus->bit_handler<6>().set_output("unifaceA6");
    m_address_bus->bit_handler<7>().set_output("unifaceA7");

    OUTPUT_LATCH(config, m_data_out);
    m_data_out->bit_handler<0>().set_output("unifaceD0");
    m_data_out->bit_handler<1>().set_output("unifaceD1");
    m_data_out->bit_handler<2>().set_output("unifaceD2");
    m_data_out->bit_handler<3>().set_output("unifaceD3");
    m_data_out->bit_handler<4>().set_output("unifaceD4");
    m_data_out->bit_handler<5>().set_output("unifaceD5");
    m_data_out->bit_handler<6>().set_output("unifaceD6");
    m_data_out->bit_handler<7>().set_output("unifaceD7");
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void p2000_uniface_device::device_start()
{
    m_slot->io_space().install_readwrite_handler(0x60, 0x60, read8smo_delegate(*this, FUNC(port_60_r)), write8smo_delegate(*this, FUNC(port_60_w)));
    m_slot->io_space().install_readwrite_handler(0x61, 0x61, read8smo_delegate(*this, FUNC(port_61_r)), write8smo_delegate(*this, FUNC(port_61_w)));
}

//-------------------------------------------------
//  device_reset
//-------------------------------------------------
void p2000_uniface_device::device_reset()
{
    m_data_out->reset();
    m_address_bus->reset();
}

//-------------------------------------------------
//  input_ports - device-specific dipswitch ports
//-------------------------------------------------
ioport_constructor p2000_uniface_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( p2000_uniface );
}

//-------------------------------------------------
//  Uniface - data register
//-------------------------------------------------
void p2000_uniface_device::port_60_w(uint8_t data)
{
    LOG("Send data %02x\n", data);
    m_data_out->write(data);
}

uint8_t p2000_uniface_device::port_60_r()
{
    LOG("Read data %02x\n", m_data_in->read());
    return m_data_in->read();
}
    
void p2000_uniface_device::port_61_w(uint8_t data)
{
    LOG("Select address %02x\n", data);
    m_address_bus->write(data);
}
uint8_t p2000_uniface_device::port_61_r()
{
    LOG("Read status %02x\n", m_status_bus->read());
    return m_status_bus->read();
}
