// license:BSD-3-Clause
// copyright-holders:Brice Onken

#ifndef MAME_SONY_NEWS_020_MMU_H
#define MAME_SONY_NEWS_020_MMU_H

#pragma once

class news_020_mmu_device : public device_t, public device_memory_interface
{
public:
	news_020_mmu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

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
	virtual void device_start() override;
	virtual space_config_vector memory_space_config() const override;

private:
	// Helper class for reading page table entries
	struct news_020_pte
	{
		news_020_pte(uint32_t pte_bits) : pte(pte_bits) {}

		const uint32_t pte;

		bool valid() const
		{
			return pte & 0x80000000;
		}

		bool kernel_writable() const
		{
			return pte & 0x40000000;
		}

		bool kernel_readable() const
		{
			return pte & 0x20000000;
		}

		bool user_writable() const
		{
			return pte & 0x10000000;
		}

		bool user_readable() const
		{
			return pte & 0x08000000;
		}

		bool modified() const
		{
			return pte & 0x04000000;
		}

		bool fill_on_demand() const
		{
			return pte & 0x02000000;
		}

		// between FOD and pfnum are 5 unused bits for memory, there can be data here for memory-mapped file I/O

		uint32_t pfnum() const
		{
			return pte & 0x000fffff; // 20 bits
		}

		// Additional helper functions
		bool writeable() const
		{
			return kernel_writable() || user_writable();
		}
	};

	const address_space_config m_hyperbus_config;
	device_delegate<void(offs_t, uint32_t, bool, uint8_t)> m_bus_error;

	bool m_enabled = false;
	bool m_romdis = false; // ROMDIS bit controls if ROM is mapped to the start of address space or not

	std::unique_ptr<u32[]> m_mmu_user_ram;
	std::unique_ptr<u32[]> m_mmu_system_ram;
	std::unique_ptr<u32[]> m_mmu_user_tag_ram;
	std::unique_ptr<u32[]> m_mmu_system_tag_ram;

	void clear_user_entries();
	void clear_kernel_entries();
};

DECLARE_DEVICE_TYPE(NEWS_020_MMU, news_020_mmu_device)

#endif // MAME_SONY_NEWS_020_MMU_H
