// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2themill.h

    Implementation of the Stellation Two The Mill 6809 card

*********************************************************************/

#ifndef MAME_BUS_A2BUS_A2THEMILL_H
#define MAME_BUS_A2BUS_A2THEMILL_H

#pragma once

#include "a2bus.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_themill_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_themill_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	a2bus_themill_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;
	virtual bool take_c800() override;

private:
	required_device<cpu_device> m_6809;
	required_ioport m_cfgsw;

	bool m_bEnabled;
	bool m_flipAddrSpace;
	bool m_6809Mode;
	uint8_t m_status;

	uint8_t dma_r(offs_t offset);
	void dma_w(offs_t offset, uint8_t data);

	void m6809_mem(address_map &map) ATTR_COLD;
};

// device type definition
DECLARE_DEVICE_TYPE(A2BUS_THEMILL, a2bus_themill_device)

#endif // MAME_BUS_A2BUS_A2THEMILL_H
