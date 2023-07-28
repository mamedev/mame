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
	djmemc_device(const machine_config &mconfig, const char *tag, device_t *owner)
		: djmemc_device(mconfig, tag, owner, (uint32_t)0)
	{
	}

	djmemc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// interface routines
	void map(address_map &map);

	template <typename... T> void set_maincpu_tag(T &&... args) { m_maincpu.set_tag(std::forward<T>(args)...); }
	template <typename... T> void set_rom_tag(T &&... args) { m_rom.set_tag(std::forward<T>(args)...); }
	void set_ram_info(u32 *ram, u32 size);

	auto write_irq() { return m_irq.bind(); }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<dafb_memc_device> m_video;
	required_region_ptr<u32> m_rom;
	devcb_write_line m_irq;

	std::unique_ptr<u32[]> m_vram;
	bool m_overlay;
	u32 *m_ram_ptr, *m_rom_ptr;
	u32 m_ram_size, m_rom_size;

	u32 rom_switch_r(offs_t offset);

	void vbl_w(int state);
};

// device type definition
DECLARE_DEVICE_TYPE(DJMEMC, djmemc_device)

#endif // MAME_APPLE_DJMEMC_H
