// license:BSD-3-Clause
// copyright-holders:Brice Onken

/*
 * Sony NEWS Memory Management Unit for 68020-based workstations and servers
 *
 * Note: There is very limited documentation about this MMU because starting with the
 *       '030 generation of machines, the custom MMU was no longer needed. Therefore,
 *       this emulation is not very complete (yet).
 */

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
	void clear_user_entries();
	void clear_kernel_entries();

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
	struct news_020_pte
	{
		bool valid;
		uint8_t access_settings; // actually 4 bits
		bool modified;
		bool fill_on_demand;
		uint8_t unused; // actually 5 bits
		uint32_t pfnum; // actually 20 bits
	};

	const address_space_config m_hyperbus_config;
	device_delegate<void(offs_t, uint32_t, bool, uint8_t)> m_bus_error;

	bool m_enabled = false;
	bool m_romdis = false; // ROMDIS bit controls if ROM is mapped to the start of address space or not

	// TODO: these may not actually be split - maybe the system bit is part of the tag? That might work?
	std::unique_ptr<u32[]> m_mmu_user_ram; // TODO: some kind of required_shared_ptr?
	std::unique_ptr<u32[]> m_mmu_system_ram; // TODO: some kind of required_shared_ptr?
	std::unique_ptr<u32[]> m_mmu_user_tag_ram; // TODO: don't do it this way, but this is OK for testing
	std::unique_ptr<u32[]> m_mmu_system_tag_ram; // TODO: don't do it this way, but this is OK for testing

	news_020_pte unpack_mmu_entry(const uint32_t entry);
};

DECLARE_DEVICE_TYPE(NEWS_020_MMU, news_020_mmu_device)

#endif // MAME_SONY_NEWS_020_MMU_H
