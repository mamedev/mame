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
//  RC2014 Real Time Clock DS1302 module
//  Module author: Ed Brindley
//**************************************************************************

class rc2014_ds1302_device : public device_t, public device_rc2014_card_interface
{
public:
	// construction/destruction
	rc2014_ds1302_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	uint8_t rtc_r(offs_t offset);
	void rtc_w(offs_t offset, uint8_t data);
private:
	required_device<ds1302_device> m_rtc;
	required_ioport m_addr;
};

rc2014_ds1302_device::rc2014_ds1302_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, RC2014_DS1302_RTC, tag, owner, clock)
	, device_rc2014_card_interface(mconfig, *this)
	, m_rtc(*this, "rtc")
	, m_addr(*this, "SW1")
{
}

void rc2014_ds1302_device::device_start()
{
}

void rc2014_ds1302_device::device_reset()
{
	// A15-A8, A1 and A0 not connected
	m_bus->installer(AS_IO)->install_readwrite_handler(m_addr->read(), m_addr->read(), 0, 0xff03, 0, read8sm_delegate(*this, FUNC(rc2014_ds1302_device::rtc_r)), write8sm_delegate(*this, FUNC(rc2014_ds1302_device::rtc_w)));
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
	PORT_START("SW1")
	PORT_DIPNAME(0x04, 0x00, "0x04") PORT_DIPLOCATION("Base Address:1")
	PORT_DIPSETTING(0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(0x04, DEF_STR( On ) )
	PORT_DIPNAME(0x08, 0x00, "0x08") PORT_DIPLOCATION("Base Address:2")
	PORT_DIPSETTING(0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(0x08, DEF_STR( On ) )
	PORT_DIPNAME(0x10, 0x00, "0x10") PORT_DIPLOCATION("Base Address:3")
	PORT_DIPSETTING(0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(0x10, DEF_STR( On ) )
	PORT_DIPNAME(0x20, 0x00, "0x20") PORT_DIPLOCATION("Base Address:4")
	PORT_DIPSETTING(0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(0x20, DEF_STR( On ) )
	PORT_DIPNAME(0x40, 0x40, "0x40") PORT_DIPLOCATION("Base Address:5")
	PORT_DIPSETTING(0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(0x40, DEF_STR( On ) )
	PORT_DIPNAME(0x80, 0x80, "0x80") PORT_DIPLOCATION("Base Address:6")
	PORT_DIPSETTING(0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(0x80, DEF_STR( On ) )
INPUT_PORTS_END

ioport_constructor rc2014_ds1302_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( rc2014_ds1302_jumpers );
}

}
//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(RC2014_DS1302_RTC, device_rc2014_card_interface, rc2014_ds1302_device, "rc2014_ds1302", "RC2014 Real Time Clock DS1302 module")
