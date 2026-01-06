// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
#ifndef MAME_BUS_ISA_ZXBUS_ADAPTER_H
#define MAME_BUS_ISA_ZXBUS_ADAPTER_H

#include "isa.h"

#include "bus/spectrum/zxbus/bus.h"

class zxbus_adapter_device: public device_t, public device_isa8_card_interface
{
public:
	zxbus_adapter_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void remap(int space_id, offs_t start, offs_t end) override ATTR_COLD;

private:
	memory_view m_isa_io_view;
	required_device<zxbus_device> m_zxbus;

};

DECLARE_DEVICE_TYPE(ISA8_ZXBUS, zxbus_adapter_device)

#endif // MAME_BUS_ISA_ZXBUS_ADAPTER_H
