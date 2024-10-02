// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2softcard.h

    Implementation of the Microsoft SoftCard Z-80 card

*********************************************************************/

#ifndef MAME_BUS_A2BUS_A2SOFTCARD_H
#define MAME_BUS_A2BUS_A2SOFTCARD_H

#include "a2bus.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_softcard_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_softcard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	a2bus_softcard_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// overrides of standard a2bus slot functions
	virtual void write_cnxx(uint8_t offset, uint8_t data) override;
	virtual bool take_c800() override;

private:
	required_device<cpu_device> m_z80;
	bool m_bEnabled;
	bool m_FirstZ80Boot;

	uint8_t dma_r(offs_t offset);
	void dma_w(offs_t offset, uint8_t data);

	void z80_mem(address_map &map) ATTR_COLD;
};

// device type definition
DECLARE_DEVICE_TYPE(A2BUS_SOFTCARD, a2bus_softcard_device)

#endif // MAME_BUS_A2BUS_A2SOFTCARD_H
