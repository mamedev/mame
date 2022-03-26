// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Cisco Terminal

**********************************************************************/


#ifndef MAME_BUS_BBC_1MHZBUS_CISCO_H
#define MAME_BUS_BBC_1MHZBUS_CISCO_H

#include "1mhzbus.h"
#include "dirtc.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class bbc_cisco_device
	: public device_t
	, public device_bbc_1mhzbus_interface
	, public device_rtc_interface
{
public:
	// construction/destruction
	bbc_cisco_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	virtual uint8_t fred_r(offs_t offset) override;
	virtual void fred_w(offs_t offset, uint8_t data) override;
	virtual uint8_t jim_r(offs_t offset) override;
	virtual void jim_w(offs_t offset, uint8_t data) override;

private:
	required_region_ptr<uint8_t> m_rom;
	std::unique_ptr<uint8_t[]> m_ram;
	std::unique_ptr<uint8_t[]> m_cfg;

	uint16_t m_rom_page;
	uint8_t m_rtc_minute;
	uint8_t m_rtc_hour;
	uint8_t m_rtc_day;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_CISCO, bbc_cisco_device);


#endif /* MAME_BUS_BBC_1MHZBUS_CISCO_H */
