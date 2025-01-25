// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
#include "emu.h"
#include "zxbus_adapter.h"

DEFINE_DEVICE_TYPE(ISA8_ZXBUS, zxbus_adapter_device, "zxbus_adapter", "ISA8 to ZXBUS Adapter")

zxbus_adapter_device::zxbus_adapter_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ISA8_ZXBUS, tag, owner, clock)
	, device_isa8_card_interface(mconfig, *this)
	, m_isa_io_view(*this, "isa_io_view")
	, m_zxbus(*this, "zxbus")
{
}

void zxbus_adapter_device::map_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xffff).view(m_isa_io_view);
	m_zxbus->set_io_space(m_isa_io_view[0], m_isa_io_view[0]);
	m_isa_io_view.select(0);
}

void zxbus_adapter_device::device_start()
{
	set_isa_device();
	m_isa->install_device(0x0000, 0xffff, *this, &zxbus_adapter_device::map_io);
}

void zxbus_adapter_device::device_add_mconfig(machine_config &config)
{
	ZXBUS(config, m_zxbus, 0);
	ZXBUS_SLOT(config, "slot", 0, m_zxbus, zxbus_cards, nullptr);
}
