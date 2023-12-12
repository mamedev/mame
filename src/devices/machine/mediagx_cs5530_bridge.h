// license: BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_MACHINE_MEDIAGX_CS5530_BRIDGE_H
#define MAME_MACHINE_MEDIAGX_CS5530_BRIDGE_H

#pragma once

#include "pci.h"
#include "mediagx_cs5530_bridge.h"

#include "bus/isa/isa.h"

class mediagx_cs5530_bridge_device : public pci_device
{
public:
	template <typename T>
	mediagx_cs5530_bridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpu_tag)
		: mediagx_cs5530_bridge_device(mconfig, tag, owner, clock)
	{
		set_ids(0x10780100, 0x00, 0x060100, 0x00000000);
		set_cpu_tag(std::forward<T>(cpu_tag));
	}

	mediagx_cs5530_bridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto boot_state_hook() { return m_boot_state_hook.bind(); }

	template <typename T>
	void set_cpu_tag(T &&tag) { m_maincpu.set_tag(std::forward<T>(tag)); }
protected:
	virtual void device_add_mconfig(machine_config & config) override;
	virtual void device_config_complete() override;
	virtual void device_reset() override;

	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
						   uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual bool map_first() const override { return true; }

	virtual void config_map(address_map &map) override;
private:
	void map_bios(address_space *memory_space, uint32_t start, uint32_t end);
	void internal_io_map(address_map &map);

	devcb_write8 m_boot_state_hook;

	required_device<cpu_device> m_maincpu;
	required_device<isa16_device> m_isabus;
};

DECLARE_DEVICE_TYPE(MEDIAGX_CS5530_BRIDGE, mediagx_cs5530_bridge_device)

#endif
