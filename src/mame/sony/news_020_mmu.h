// license:BSD-3-Clause
// copyright-holders:Brice Onken

#ifndef MAME_SONY_NEWS_020_MMU_H
#define MAME_SONY_NEWS_020_MMU_H

#pragma once

class news_020_mmu_device : public device_t, public device_memory_interface
{
public:
	news_020_mmu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) ATTR_COLD;

	template <typename... T>
	void set_bus_error_callback(T &&...args) { m_bus_error.set(std::forward<T>(args)...); }

	// MMU entry accessors
	uint32_t mmu_entry_r(offs_t offset, uint32_t mem_mask);
	void mmu_entry_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void clear_entries(offs_t offset, uint8_t data);

	// MMU control register accessors
	void set_mmu_enable(bool enabled);
	void set_rom_enable(bool enabled);

	// Bus accessors for CPU to use when going from CPU->HB
	uint32_t hyperbus_r(offs_t offset, uint32_t mem_mask, bool is_supervisor);
	void hyperbus_w(offs_t offset, uint32_t data, uint32_t mem_mask, bool is_supervisor);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual space_config_vector memory_space_config() const override ATTR_COLD;

private:
	const address_space_config m_hyperbus_config;
	memory_access<32, 2, 0, ENDIANNESS_BIG>::specific m_hyperbus;
	device_delegate<void(offs_t, uint32_t, bool, uint8_t)> m_bus_error;
	bool m_enabled;
	bool m_romdis;
	std::unique_ptr<u32[]> m_mmu_user_ram;
	std::unique_ptr<u32[]> m_mmu_system_ram;
	std::unique_ptr<u32[]> m_mmu_user_tag_ram;
	std::unique_ptr<u32[]> m_mmu_system_tag_ram;

	void clear_user_entries();
	void clear_kernel_entries();
};

DECLARE_DEVICE_TYPE(NEWS_020_MMU, news_020_mmu_device)

#endif // MAME_SONY_NEWS_020_MMU_H
