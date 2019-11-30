// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
// thanks-to:rfka01
/***************************************************************************

    K012 Internal HD Interface
    C3282 External HD Interface

***************************************************************************/

#include "emu.h"
#include "k012.h"

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

static INPUT_PORTS_START( dmv_k012 )
	PORT_START("JUMPERS")
	PORT_DIPNAME( 0x1f, 0x10, "IFSEL" )  PORT_DIPLOCATION("J:!1,J:!2,J:!3,J:!4,J:!5")
	PORT_DIPSETTING( 0x01, "0A" )
	PORT_DIPSETTING( 0x02, "1A" )
	PORT_DIPSETTING( 0x04, "2A" )
	PORT_DIPSETTING( 0x08, "3A" )
	PORT_DIPSETTING( 0x10, "4A" )   // default
INPUT_PORTS_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(DMV_K012,  dmv_k012_device,  "dmv_k012",  "K012 Internal HD Interface")
DEFINE_DEVICE_TYPE(DMV_C3282, dmv_c3282_device, "dmv_c3282", "C3282 External HD Interface")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  dmv_k012_device - constructor
//-------------------------------------------------

dmv_k012_device::dmv_k012_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: dmv_k012_device(mconfig, DMV_K012, tag, owner, clock)
{
}

dmv_k012_device::dmv_k012_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_dmvslot_interface( mconfig, *this )
	, m_hdc(*this, "hdc")
	, m_jumpers(*this, "JUMPERS")
{
}

//-------------------------------------------------
//  dmv_c3282_device - constructor
//-------------------------------------------------

dmv_c3282_device::dmv_c3282_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: dmv_k012_device(mconfig, DMV_C3282, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dmv_k012_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void dmv_k012_device::device_reset()
{
}

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor dmv_k012_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( dmv_k012 );
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void dmv_k012_device::device_add_mconfig(machine_config &config)
{
	WD1000(config, m_hdc, 20_MHz_XTAL / 4);         // WD1010
	m_hdc->intrq_wr_callback().set(FUNC(dmv_k012_device::out_int));

	// default drive is 10MB (306,4,17)
	HARDDISK(config, "hdc:0", 0);
	HARDDISK(config, "hdc:1", 0);
	HARDDISK(config, "hdc:2", 0);
	HARDDISK(config, "hdc:3", 0);
}

void dmv_k012_device::io_read(int ifsel, offs_t offset, uint8_t &data)
{
	if ((1 << ifsel) == m_jumpers->read() && !(offset & 0x08))
		data = m_hdc->read(machine().dummy_space(), offset);
}

void dmv_k012_device::io_write(int ifsel, offs_t offset, uint8_t data)
{
	if ((1 << ifsel) == m_jumpers->read() && !(offset & 0x08))
		m_hdc->write(machine().dummy_space(), offset, data);
}
