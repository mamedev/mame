// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_SEGA_MODEL3_PCI_H
#define MAME_SEGA_MODEL3_PCI_H

#pragma once

#include "bus/pci/pci_slot.h"

#include <span>

// Real3D graphics memory and upload ports as seen from PCI
class model3_real3d_pci_device : public pci_device
{
public:
	enum class step : u8 { STEP_1_0, STEP_1_5, STEP_2_0, STEP_2_1 };
	// header, encoded length, payload words
	using texture_delegate = device_delegate<void (u32, u32, std::span<const u32>)>;
	// address, header, third word, source from the address to the end of memory
	using vrom_delegate = device_delegate<void (u32, u32, u32, std::span<const u32>)>;
	using submit_delegate = device_delegate<void ()>;

	model3_real3d_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	void set_step(step revision) { m_step = revision; }
	// command and graphics base loaded on reset
	void set_reset_command(u16 data) { m_reset_command = data; }
	void set_reset_base(u32 data) { m_reset_base = data; }
	// interleaved VROM words, owned by the driver
	void set_vrom(std::span<const u32> words) { m_vrom = words; }
	std::span<const u32> vrom() const { return m_vrom; }

	template <typename... T> void set_texture_callback(T &&... args) { m_texture_callback.set(std::forward<T>(args)...); }
	template <typename... T> void set_vrom_callback(T &&... args) { m_vrom_callback.set(std::forward<T>(args)...); }
	template <typename... T> void set_submit_callback(T &&... args) { m_submit_callback.set(std::forward<T>(args)...); }
	auto status_callback() { return m_status_callback.bind(); }

	std::span<const u32> display_list_ram() const { return { m_display_list_ram.target(), DISPLAY_WORDS }; }
	std::span<const u32> culling_ram() const { return { m_culling_ram.target(), CULLING_WORDS }; }
	std::span<const u32> polygon_ram() const { return { m_polygon_ram.target(), POLYGON_WORDS }; }
	// raw 0x9c register words
	std::span<const u32, 4> buffer_config() const { return m_buffer_config; }

	virtual void config_map(address_map &map) override ATTR_COLD;
	void status_map(address_map &map) ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void map_extra(u64 memory_window_start, u64 memory_window_end, u64 memory_offset, address_space *memory_space,
		u64 io_window_start, u64 io_window_end, u64 io_offset, address_space *io_space) override;

private:
	static constexpr u32 DISPLAY_WORDS = 0x100000 / 4;
	static constexpr u32 CULLING_WORDS = 0x400000 / 4;
	static constexpr u32 POLYGON_WORDS = 0x400000 / 4;
	static constexpr u32 FIFO_WORDS = 0x100000 / 4;

	void command_map(address_map &map) ATTR_COLD;
	void culling_map(address_map &map) ATTR_COLD;
	void display_list_map(address_map &map) ATTR_COLD;
	void vrom_map(address_map &map) ATTR_COLD;
	void fifo_map(address_map &map) ATTR_COLD;
	void polygon_map(address_map &map) ATTR_COLD;
	void buffer_config_map(address_map &map) ATTR_COLD;
	void buffer_config_w(offs_t offset, u32 data, u32 mem_mask);
	u32 graphics_base_r();
	void graphics_base_w(offs_t offset, u32 data, u32 mem_mask);
	u32 status_r(offs_t offset, u32 mem_mask);
	void command_w(offs_t offset, u32 data, u32 mem_mask);
	void vrom_w(offs_t offset, u32 data, u32 mem_mask);
	void fifo_w(offs_t offset, u32 data, u32 mem_mask);
	void enqueue_vrom(u32 address);
	void flush_textures();
	static u32 record_words(u32 length) { return (length / 2 + 2) / 4; }

	step m_step;
	u16 m_reset_command;
	u32 m_reset_base;
	std::span<const u32> m_vrom;
	texture_delegate m_texture_callback;
	vrom_delegate m_vrom_callback;
	submit_delegate m_submit_callback;
	devcb_read32 m_status_callback;
	memory_share_creator<u32> m_display_list_ram;
	memory_share_creator<u32> m_culling_ram;
	memory_share_creator<u32> m_polygon_ram;
	memory_share_creator<u32> m_texture_fifo;
	u32 m_fifo_count;
	u32 m_vrom_words[2];
	u8 m_vrom_count;
	bool m_fifo_overflow;
	u32 m_buffer_config[4];
	u32 m_graphics_base;
};

DECLARE_DEVICE_TYPE(MODEL3_REAL3D_PCI, model3_real3d_pci_device)

// Daytona 2's PCI ROM board (315-6183); the bank latch also selects the CPU-side CROM bank
class model3_rom_pci_device : public pci_device
{
public:
	model3_rom_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
	template <typename T> void set_rom_tag(T &&tag) { m_rom.set_tag(std::forward<T>(tag)); }
	auto bank_callback() { return m_bank_callback.bind(); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void memory_map(address_map &map) ATTR_COLD;
	u32 rom_r(offs_t offset);
	void bank_w(u8 data);

	required_region_ptr<u64> m_rom;
	devcb_write8 m_bank_callback;
	u8 m_bank;
};

DECLARE_DEVICE_TYPE(MODEL3_ROM_PCI, model3_rom_pci_device)

// Step 2 Real3D DMA engine; graphics configuration space is reached through +0x10/+0x14
class model3_real3d_dma_pci_device : public pci_card_device, public device_memory_interface
{
public:
	model3_real3d_dma_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// for machine configuration, before finders resolve
	model3_real3d_pci_device &graphics() const { return *m_graphics.lookup(); }
	// status query result; reads the graphics status registers when unset
	auto status_query_callback() { return m_status_query_callback.bind(); }
	virtual void config_map(address_map &map) override ATTR_COLD;
	virtual void set_remap_cb(mapper_cb callback) override ATTR_COLD;

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual space_config_vector memory_space_config() const override ATTR_COLD;
	virtual void map_extra(u64 memory_window_start, u64 memory_window_end, u64 memory_offset, address_space *memory_space,
		u64 io_window_start, u64 io_window_end, u64 io_offset, address_space *io_space) override;

private:
	void downstream_config_map(address_map &map) ATTR_COLD;
	void downstream_status_map(address_map &map) ATTR_COLD;
	void config_port_map(address_map &map) ATTR_COLD;
	void command_w(offs_t offset, u16 data, u16 mem_mask);
	u32 parameter_r(offs_t offset);
	void parameter_w(offs_t offset, u32 data, u32 mem_mask);
	u32 length_r() { return m_dma_words; }
	void length_w(offs_t offset, u32 data, u32 mem_mask);
	u8 dma_status_r() { return ((m_dma_active || m_query_busy) ? 0x80 : 0) | (m_irq_pending ? 1 : 0); }
	void acknowledge_w(u8 data);
	u8 control_r() { return m_dma_control; }
	void control_w(u8 data) { m_dma_control = data; }
	void update_irq();
	TIMER_CALLBACK_MEMBER(dma_tick);
	TIMER_CALLBACK_MEMBER(query_done);
	u16 cache_latency_r() { return m_cache_latency; }
	void cache_latency_w(offs_t offset, u16 data, u16 mem_mask) { COMBINE_DATA(&m_cache_latency); }
	u32 dma_base_r();
	void dma_base_w(offs_t offset, u32 data, u32 mem_mask);
	u32 selector_r() { return m_selector; }
	void selector_w(offs_t offset, u32 data, u32 mem_mask);
	u32 data_r(offs_t offset, u32 mem_mask);
	void data_w(offs_t offset, u32 data, u32 mem_mask);
	bool config_selected() const { return (m_selector & ~0xfcU) == 0x20000000; }
	bool status_selected() const { return (m_selector & ~0x3cU) == 0x80000000; }

	const address_space_config m_config_space;
	const address_space_config m_status_space;
	required_device<model3_real3d_pci_device> m_graphics;
	devcb_read32 m_status_query_callback;
	emu_timer *m_dma_timer;
	emu_timer *m_query_timer;
	u32 m_dma_base;
	u32 m_selector;
	u16 m_cache_latency;
	u32 m_dma_source;
	u32 m_dma_destination;
	u32 m_dma_words;
	u32 m_status_result;
	u8 m_dma_control;
	u8 m_transfer_control;
	bool m_dma_active;
	bool m_query_busy;
	bool m_irq_pending;
};

DECLARE_DEVICE_TYPE(MODEL3_REAL3D_DMA_PCI, model3_real3d_dma_pci_device)

#endif // MAME_SEGA_MODEL3_PCI_H
