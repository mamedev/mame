// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    uthernet.h

    Apple II Uthernet Card

*********************************************************************/

#ifndef MAME_BUS_A2BUS_UTHERNET_H
#define MAME_BUS_A2BUS_UTHERNET_H

#include "a2bus.h"
#include "machine/cs8900a.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_uthernet_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_uthernet_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	a2bus_uthernet_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	virtual void device_add_mconfig(machine_config &config) override;

	virtual void device_start() override;
	virtual void device_reset() override;

	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;

private:
	required_device<cs8900a_device> m_netinf;

};

// device type definition
DECLARE_DEVICE_TYPE(A2BUS_UTHERNET, a2bus_uthernet_device)

#endif // MAME_BUS_A2BUS_UTHERNET_H
