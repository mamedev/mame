// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

    K803 RTC module

***************************************************************************/

#include "emu.h"
#include "k803.h"


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

static MACHINE_CONFIG_FRAGMENT( dmv_k803 )
	MCFG_DEVICE_ADD("rtc", MM58167, XTAL_32_768kHz)
	MCFG_MM58167_IRQ_CALLBACK(WRITELINE(dmv_k803_device, rtc_irq_w))
MACHINE_CONFIG_END

static INPUT_PORTS_START( dmv_k803 )
	PORT_START("DSW")
	PORT_DIPNAME( 0x0f, 0x09, "K803 IFSEL" )  PORT_DIPLOCATION("S:!4,S:!3,S:!2,S:!1")
	PORT_DIPSETTING( 0x00, "0A" )
	PORT_DIPSETTING( 0x01, "0B" )
	PORT_DIPSETTING( 0x02, "1A" )
	PORT_DIPSETTING( 0x03, "1B" )
	PORT_DIPSETTING( 0x04, "2A" )
	PORT_DIPSETTING( 0x05, "2B" )
	PORT_DIPSETTING( 0x06, "3A" )
	PORT_DIPSETTING( 0x07, "3B" )
	PORT_DIPSETTING( 0x08, "4A" )
	PORT_DIPSETTING( 0x09, "4B" )   // default
INPUT_PORTS_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type DMV_K803 = &device_creator<dmv_k803_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  dmv_k803_device - constructor
//-------------------------------------------------

dmv_k803_device::dmv_k803_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
		: device_t(mconfig, DMV_K803, "K803 RTC", tag, owner, clock, "dmv_k803", __FILE__),
		device_dmvslot_interface( mconfig, *this ),
		m_rtc(*this, "rtc"),
		m_dsw(*this, "DSW"), m_bus(nullptr), m_latch(0), m_rtc_int(0)
	{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dmv_k803_device::device_start()
{
	m_bus = static_cast<dmvcart_slot_device*>(owner());
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void dmv_k803_device::device_reset()
{
	m_latch = 0;
	m_rtc_int = CLEAR_LINE;
}

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor dmv_k803_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( dmv_k803 );
}

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor dmv_k803_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( dmv_k803 );
}

void dmv_k803_device::io_read(address_space &space, int ifsel, offs_t offset, UINT8 &data)
{
	UINT8 dsw = m_dsw->read() & 0x0f;
	if ((dsw >> 1) == ifsel && BIT(offset, 3) == BIT(dsw, 0))
	{
		if (offset & 0x04)
			data = m_rtc->read(space, ((m_latch & 0x07) << 2) | (offset & 0x03));
	}
}

void dmv_k803_device::io_write(address_space &space, int ifsel, offs_t offset, UINT8 data)
{
	UINT8 dsw = m_dsw->read() & 0x0f;
	if ((dsw >> 1) == ifsel && BIT(offset, 3) == BIT(dsw, 0))
	{
		if (offset & 0x04)
			m_rtc->write(space, ((m_latch & 0x07) << 2) | (offset & 0x03), data);
		else
		{
			m_latch = data;
			update_int();
		}
	}
}

WRITE_LINE_MEMBER(dmv_k803_device::rtc_irq_w)
{
	m_rtc_int = state;
	update_int();
}

void dmv_k803_device::update_int()
{
	bool state = ((m_latch & 0x80) && m_rtc_int);
	m_bus->m_out_int_cb(state ? ASSERT_LINE : CLEAR_LINE);
}
