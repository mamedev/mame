// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    RC2014 Real Time Clock

****************************************************************************/

#include "emu.h"
#include "rtc.h"
#include "machine/ds1302.h"

namespace {

//**************************************************************************
//  Real Time Clock DS1302 module
//  Module author: Ed Brindley
//**************************************************************************

class rc2014_ds1302_device : public device_t, public device_rc2014_card_interface
{
public:
	// construction/destruction
	rc2014_ds1302_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;

	uint8_t rtc_r(offs_t offset);
	void rtc_w(offs_t offset, uint8_t data);
private:
	required_device<ds1302_device> m_rtc;
	required_ioport_array<6> m_addr;
};

rc2014_ds1302_device::rc2014_ds1302_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, RC2014_DS1302_RTC, tag, owner, clock)
	, device_rc2014_card_interface(mconfig, *this)
	, m_rtc(*this, "rtc")
	, m_addr(*this, "A%u", 2U)
{
}

void rc2014_ds1302_device::device_start()
{
}

void rc2014_ds1302_device::device_reset()
{
	uint8_t addr = 0x00;
	uint8_t index = 2;
	for (auto& port: m_addr)
	{
		addr |= port->read() << index;
		index++;
	}

	m_bus->installer(AS_IO)->install_readwrite_handler(addr, addr, 0, 0, 0, read8sm_delegate(*this, FUNC(rc2014_ds1302_device::rtc_r)), write8sm_delegate(*this, FUNC(rc2014_ds1302_device::rtc_w)));
}

void rc2014_ds1302_device::device_add_mconfig(machine_config &config)
{
	DS1302(config, m_rtc, 32.768_kHz_XTAL);
}

uint8_t rc2014_ds1302_device::rtc_r(offs_t offset)
{
	return m_rtc->io_r() ? 0x01 : 0x00;
}

void rc2014_ds1302_device::rtc_w(offs_t, uint8_t data)
{
	m_rtc->ce_w(BIT(data,4));
	if (BIT(data,5)==0) m_rtc->io_w(BIT(data,7));
	m_rtc->sclk_w(BIT(data,6));
}

static INPUT_PORTS_START( rc2014_ds1302_jumpers )
	PORT_START("A2")
	PORT_CONFNAME( 0x1, 0x0, "A2" )
	PORT_CONFSETTING( 0x0, "0" )
	PORT_CONFSETTING( 0x1, "1" )
	PORT_START("A3")
	PORT_CONFNAME( 0x1, 0x0, "A3" )
	PORT_CONFSETTING( 0x0, "0" )
	PORT_CONFSETTING( 0x1, "1" )
	PORT_START("A4")
	PORT_CONFNAME( 0x1, 0x0, "A4" )
	PORT_CONFSETTING( 0x0, "0" )
	PORT_CONFSETTING( 0x1, "1" )
	PORT_START("A5")
	PORT_CONFNAME( 0x1, 0x0, "A5" )
	PORT_CONFSETTING( 0x0, "0" )
	PORT_CONFSETTING( 0x1, "1" )
	PORT_START("A6")
	PORT_CONFNAME( 0x1, 0x1, "A6" )
	PORT_CONFSETTING( 0x0, "0" )
	PORT_CONFSETTING( 0x1, "1" )
	PORT_START("A7")
	PORT_CONFNAME( 0x1, 0x1, "A7" )
	PORT_CONFSETTING( 0x0, "0" )
	PORT_CONFSETTING( 0x1, "1" )
INPUT_PORTS_END

ioport_constructor rc2014_ds1302_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( rc2014_ds1302_jumpers );
}

}
//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(RC2014_DS1302_RTC, device_rc2014_card_interface, rc2014_ds1302_device, "rc2014_ds1302", "Real Time Clock DS1302 module")
