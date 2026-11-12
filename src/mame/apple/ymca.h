// license:BSD-3-Clause
// copyright-holders:R. Belmont

#ifndef MAME_APPLE_YMCA_H
#define MAME_APPLE_YMCA_H

#pragma once

class ymca_device :  public device_t
{
public:
	// construction/destruction
	ymca_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// interface routines
	virtual void map(address_map &map);

	template <typename... T> void set_maincpu_tag(T &&... args) { m_maincpu.set_tag(std::forward<T>(args)...); }
	template <typename... T> void set_rom_tag(T &&... args) { m_rom.set_tag(std::forward<T>(args)...); }
	void set_ram_info(u32 *ram, u32 size);

	void set_cpu_id(int id) { m_cpu_id = id & 0xf; }

	auto write_irq() { return m_irq.bind(); }
	auto write_ntscpalsel() { return m_ntscpalsel.bind(); }
	auto write_rgb_bypass() { return m_rgb_bypass.bind(); }

protected:
	ymca_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	u32 rom_switch_r(offs_t offset);

private:
	u32 regs_r(offs_t offset, u32 mem_mask);
	void regs_w(offs_t offset, u32 data, u32 mem_mask);

	required_device<cpu_device> m_maincpu;
	required_region_ptr<u32> m_rom;
	devcb_write_line m_irq, m_ntscpalsel, m_rgb_bypass;

	bool m_overlay;
	u32 *m_ram_ptr, *m_rom_ptr;
	u32 m_ram_size, m_rom_size;
	u8 m_cpu_id;
};

// device type definition
DECLARE_DEVICE_TYPE(YMCA, ymca_device)

#endif // MAME_APPLE_YMCA_H
