// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Hybrid Music 2000 Interface

    https://www.retro-kit.co.uk/page.cfm/content/Hybrid-Music-2000-Interface/
    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Hybrid_M2000.html

**********************************************************************/


#ifndef MAME_BUS_BBC_1MHZBUS_M2000_H
#define MAME_BUS_BBC_1MHZBUS_M2000_H

#include "1mhzbus.h"
#include "machine/6850acia.h"
#include "machine/clock.h"
#include "machine/input_merger.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class bbc_m2000_device :
	public device_t,
	public device_bbc_1mhzbus_interface
{
public:
	// construction/destruction
	bbc_m2000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual uint8_t fred_r(offs_t offset) override;
	virtual void fred_w(offs_t offset, uint8_t data) override;
	virtual uint8_t jim_r(offs_t offset) override;
	virtual void jim_w(offs_t offset, uint8_t data) override;

private:
	void write_acia_clock(int state);

	required_device<bbc_1mhzbus_slot_device> m_1mhzbus;
	required_device<acia6850_device> m_acia1;
	required_device<acia6850_device> m_acia2;
	required_device<acia6850_device> m_acia3;
	required_device<clock_device> m_acia_clock;
	required_device<input_merger_device> m_irqs;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_M2000, bbc_m2000_device);


#endif /* MAME_BUS_BBC_1MHZBUS_M2000_H */
