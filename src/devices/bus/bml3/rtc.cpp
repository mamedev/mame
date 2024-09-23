// license:GPL-2.0+
// copyright-holders:Russell Bull
/*********************************************************************

    bml3rtc.cpp

    Hitachi RTC card for the MB-6890

  I cannot find any reference to the original Hitachi board that
  had this feature, but I was given a schematic back in the 1980's
  and I designed a board that included the circuitry.
  The standard ROM supported this battery-backed RTC and detects
  it's presence on start up.

*********************************************************************/

#include "emu.h"
#include "rtc.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BML3BUS_RTC, bml3bus_rtc_device, "bml3rtc", "Hitachi Real Time Clock Card")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bml3bus_rtc_device::device_add_mconfig(machine_config &config)
{
	MSM5832(config, m_rtc, 32.768_kHz_XTAL);
}


uint8_t bml3bus_rtc_device::read(offs_t offset)
{
	uint8_t data = 0x00;

	if (offset == 2) // read @ 0xff3a
	{
		if (BIT(m_addr_latch, 7)) //bit 7 is the !write buffer enable
		{
			data = m_rtc->data_r();
		}
		else
		{
			data = m_data_latch; //the Hitachi ROM code detects RTC presence by reading back what it wrote to the data latch
		}
	}
	return data | 0xf0; // return low nibble only
}

void bml3bus_rtc_device::write(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0: // addr latch @ 0xff38
		m_rtc->cs_w(1); //always selected
		m_rtc->write_w(BIT(data, 6));
		m_rtc->read_w(BIT(data, 7));
		m_rtc->address_w(data & 0x0f);

		m_addr_latch = data;
		break;

	case 1: // data latch @ 0xff39
		m_rtc->hold_w(BIT(data, 6));

		if (BIT(m_addr_latch, 6))
		{
			m_rtc->data_w(data & 0x0f);
		}

		m_data_latch = data;
		break;
	}
}
//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

bml3bus_rtc_device::bml3bus_rtc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BML3BUS_RTC, tag, owner, clock)
	, device_bml3bus_card_interface(mconfig, *this)
	, m_rtc(*this, "rtc")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bml3bus_rtc_device::device_start()
{
	m_addr_latch = 0;
	m_data_latch = 0;

	save_item(NAME(m_addr_latch));
	save_item(NAME(m_data_latch));
}

void bml3bus_rtc_device::map_io(address_space_installer &space)
{
	// install into memory
	space.install_readwrite_handler(0xff38, 0xff3a, read8sm_delegate(*this, FUNC(bml3bus_rtc_device::read)), write8sm_delegate(*this, FUNC(bml3bus_rtc_device::write)));
}
