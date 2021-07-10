// license:BSD-3-Clause
// copyright-holders:Kelvin Sherlock
/*********************************************************************

    lancegs.h

    Apple II LANceGS Card

*********************************************************************/

#ifndef MAME_BUS_A2BUS_LANCEGS_H
#define MAME_BUS_A2BUS_LANCEGS_H

#include "a2bus.h"
#include "machine/smc91c9x.h"
#include "machine/i2cmem.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_lancegs_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_lancegs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	a2bus_lancegs_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	virtual void device_start() override;
	virtual void device_reset() override;

	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;

private:
	required_device<smc91c96_device> m_netinf;
	required_device<i2c_24c04_device> m_i2cmem;
	bool m_shadow;

	DECLARE_WRITE_LINE_MEMBER( netinf_irq_w );
};

// device type definition
DECLARE_DEVICE_TYPE(A2BUS_LANCEGS, a2bus_lancegs_device)

#endif // MAME_BUS_A2BUS_LANCEGS_H
