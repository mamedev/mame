// license:BSD-3-Clause
// copyright-holders:R. Belmont

#ifndef MAME_APPLE_DJMEMC_H
#define MAME_APPLE_DJMEMC_H

#pragma once

#include "dafb.h"

// ======================> djmemc_device

class djmemc_device :  public device_t
{
public:
	// construction/destruction
	djmemc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// interface routines
	virtual void map(address_map &map) ATTR_COLD;

	template <typename... T> void set_maincpu_tag(T &&... args) { m_maincpu.set_tag(std::forward<T>(args)...); }
	template <typename... T> void set_rom_tag(T &&... args) { m_rom.set_tag(std::forward<T>(args)...); }
	void set_ram_info(u32 *ram, u32 size);

	auto write_irq() { return m_irq.bind(); }

protected:
	djmemc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	u32 rom_switch_r(offs_t offset);
	void vbl_w(int state);

	required_device<dafb_base> m_video;

private:
	required_device<cpu_device> m_maincpu;
	required_region_ptr<u32> m_rom;
	devcb_write_line m_irq;

	std::unique_ptr<u32[]> m_vram;
	bool m_overlay;
	u32 *m_ram_ptr, *m_rom_ptr;
	u32 m_ram_size, m_rom_size;
};

class memcjr_device : public djmemc_device
{
public:
	// construction/destruction
	memcjr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void map(address_map &map) override ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	u32 dafb_holding_r(offs_t offset);
	void dafb_holding_w(offs_t offset, u32 data);
	u32 memcjr_r(offs_t offset);
	void memcjr_w(offs_t offset, u32 data);

	u16 m_dafb_holding;
};

// device type definition
DECLARE_DEVICE_TYPE(DJMEMC, djmemc_device)
DECLARE_DEVICE_TYPE(MEMCJR, memcjr_device)

#endif // MAME_APPLE_DJMEMC_H
