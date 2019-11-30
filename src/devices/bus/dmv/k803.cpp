// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
// thanks-to:rfka01
/***************************************************************************

    K803 RTC module

***************************************************************************/

#include "emu.h"
#include "k803.h"


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

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

DEFINE_DEVICE_TYPE(DMV_K803, dmv_k803_device, "dmv_k803", "K803 RTC")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  dmv_k803_device - constructor
//-------------------------------------------------

dmv_k803_device::dmv_k803_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, DMV_K803, tag, owner, clock),
	device_dmvslot_interface( mconfig, *this ),
	m_rtc(*this, "rtc"),
	m_dsw(*this, "DSW"),
	m_latch(0), m_rtc_int(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dmv_k803_device::device_start()
{
	// register for state saving
	save_item(NAME(m_latch));
	save_item(NAME(m_rtc_int));
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
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void dmv_k803_device::device_add_mconfig(machine_config &config)
{
	MM58167(config, m_rtc, XTAL(32'768));
	m_rtc->irq().set(FUNC(dmv_k803_device::rtc_irq_w));
}

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor dmv_k803_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( dmv_k803 );
}

void dmv_k803_device::io_read(int ifsel, offs_t offset, uint8_t &data)
{
	uint8_t dsw = m_dsw->read() & 0x0f;
	if ((dsw >> 1) == ifsel && BIT(offset, 3) == BIT(dsw, 0))
	{
		if (offset & 0x04)
			data = m_rtc->read(((m_latch & 0x07) << 2) | (offset & 0x03));
	}
}

void dmv_k803_device::io_write(int ifsel, offs_t offset, uint8_t data)
{
	uint8_t dsw = m_dsw->read() & 0x0f;
	if ((dsw >> 1) == ifsel && BIT(offset, 3) == BIT(dsw, 0))
	{
		if (offset & 0x04)
			m_rtc->write(((m_latch & 0x07) << 2) | (offset & 0x03), data);
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
	out_int(state ? ASSERT_LINE : CLEAR_LINE);
}
