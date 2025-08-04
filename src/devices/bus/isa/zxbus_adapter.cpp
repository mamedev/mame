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

void zxbus_adapter_device::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_IO)
	{
		m_isa->space(isa8_device::AS_ISA_IO).install_view(0x0000, 0xffff, m_isa_io_view);
	}
}

void zxbus_adapter_device::device_start()
{
	set_isa_device();
	remap(AS_IO, 0, 0xffff);

	m_zxbus->set_io_space(m_isa_io_view[0], m_isa_io_view[0]);
	m_isa_io_view.select(0);
}

void zxbus_adapter_device::device_add_mconfig(machine_config &config)
{
	ZXBUS(config, m_zxbus, 0);
	ZXBUS_SLOT(config, "card", 0, m_zxbus, zxbus_cards, nullptr);
}
