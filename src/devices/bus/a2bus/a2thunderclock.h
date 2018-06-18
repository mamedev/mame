// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2thunderclock.h

    Implemention of the Thunderware Thunderclock Plus.

*********************************************************************/

#ifndef MAME_BUS_A2BUS_A2THUNDERCLOCK_H
#define MAME_BUS_A2BUS_A2THUNDERCLOCK_H

#pragma once

#include "a2bus.h"
#include "machine/upd1990a.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_thunderclock_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_thunderclock_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	a2bus_thunderclock_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;
	virtual uint8_t read_cnxx(uint8_t offset) override;
	virtual uint8_t read_c800(uint16_t offset) override;

	required_device<upd1990a_device> m_upd1990ac;

private:
	DECLARE_WRITE_LINE_MEMBER( upd_dataout_w );

	uint8_t *m_rom;
	int m_dataout;
};

// device type definition
DECLARE_DEVICE_TYPE(A2BUS_THUNDERCLOCK, a2bus_thunderclock_device)

#endif // MAME_BUS_A2BUS_A2THUNDERCLOCK_H
