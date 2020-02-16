// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_BUS_A2BUS_BYTE8251_H
#define MAME_BUS_A2BUS_BYTE8251_H

#include "a2bus.h"
#include "machine/i8251.h"
#include "machine/mm5307.h"

class a2bus_byte8251_device : public device_t, public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_byte8251_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	DECLARE_INPUT_CHANGED_MEMBER(rate_changed);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_add_mconfig(machine_config &config) override;

	// device_a2bus_card_interface overrides
	virtual u8 read_c0nx(u8 offset) override;
	virtual void write_c0nx(u8 offset, u8 data) override;

private:
	// object finders
	required_device<i8251_device> m_usart;
	required_device<mm5307_device> m_brg;
	required_ioport m_switches;
};

// device type definition
DECLARE_DEVICE_TYPE(A2BUS_BYTE8251, a2bus_byte8251_device)

#endif // MAME_BUS_A2BUS_BYTE8251_H
