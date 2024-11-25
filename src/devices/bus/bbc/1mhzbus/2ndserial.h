// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Sprow 2nd Serial Port

**********************************************************************/


#ifndef MAME_BUS_BBC_1MHZBUS_2NDSERIAL_H
#define MAME_BUS_BBC_1MHZBUS_2NDSERIAL_H

#include "1mhzbus.h"
#include "bus/rs232/rs232.h"
#include "machine/6850acia.h"
#include "machine/clock.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class bbc_2ndserial_device : public device_t, public device_bbc_1mhzbus_interface
{
public:
	// construction/destruction
	bbc_2ndserial_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry* device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual uint8_t fred_r(offs_t offset) override;
	virtual void fred_w(offs_t offset, uint8_t data) override;
	virtual uint8_t jim_r(offs_t offset) override;
	virtual void jim_w(offs_t offset, uint8_t data) override;

private:
	void write_acia_clock(int state);

	required_device<bbc_1mhzbus_slot_device> m_1mhzbus;
	required_device<acia6850_device> m_acia;
	required_device<clock_device> m_acia_clock_tx;
	required_device<clock_device> m_acia_clock_rx;
	required_device<rs232_port_device> m_rs232;
	required_ioport m_links;

	uint8_t m_control;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_2NDSERIAL, bbc_2ndserial_device);


#endif /* MAME_BUS_BBC_1MHZBUS_2NDSERIAL_H */
