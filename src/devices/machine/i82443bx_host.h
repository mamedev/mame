// license: BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_MACHINE_I82443BX_HOST_H
#define MAME_MACHINE_I82443BX_HOST_H

#pragma once

#include "pci.h"
#include "machine/i82439hx.h"

class i82443bx_host_device : public i82439hx_host_device
{
public:
	template <typename T>
	i82443bx_host_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpu_tag, int ram_size)
		: i82443bx_host_device(mconfig, tag, owner, clock)
	{
		set_cpu_tag(std::forward<T>(cpu_tag));
		set_ram_size(ram_size);
	}
	i82443bx_host_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	i82443bx_host_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void config_map(address_map &map) override ATTR_COLD;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void apbase_map(address_map &map) ATTR_COLD;

	virtual u8 capptr_r() override;

private:
	virtual std::tuple<bool, bool> read_memory_holes() override;

	u8 m_fdhc = 0;
	u8 m_bspad[8]{};

	u8 fdhc_r();
	void fdhc_w(u8 data);
	u8 bspad_r(offs_t offset);
	void bspad_w(offs_t offset, u8 data);
};

class i82443lx_host_device : public i82443bx_host_device
{
public:
	template <typename T>
	i82443lx_host_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpu_tag, int ram_size)
		: i82443lx_host_device(mconfig, tag, owner, clock)
	{
		set_cpu_tag(std::forward<T>(cpu_tag));
		set_ram_size(ram_size);
	}
	i82443lx_host_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class i82443bx_bridge_device : public pci_bridge_device
{
public:
	/*template <typename T> sis630_bridge_device(
	    const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock,
	    T &&gui_tag
	) : sis630_bridge_device(mconfig, tag, owner, clock)
	{
	    // either 0001 or 6001 as device ID
	    set_ids_bridge(0x10396001, 0x00);
	    //set_multifunction_device(true);
	    //m_vga.set_tag(std::forward<T>(gui_tag));
	}*/

	i82443bx_bridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	i82443bx_bridge_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
						   uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

private:
	//required_device<sis630_gui_device> m_vga;

	virtual void bridge_control_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;
};

class i82443lx_bridge_device : public i82443bx_bridge_device
{
public:
	i82443lx_bridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

DECLARE_DEVICE_TYPE(I82443BX_HOST, i82443bx_host_device)
DECLARE_DEVICE_TYPE(I82443LX_HOST, i82443lx_host_device)

DECLARE_DEVICE_TYPE(I82443BX_BRIDGE, i82443bx_bridge_device)
DECLARE_DEVICE_TYPE(I82443LX_BRIDGE, i82443lx_bridge_device)


#endif // MAME_MACHINE_I82443BX_HOST_H
