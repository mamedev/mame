// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_MACHINE_PCI_H
#define MAME_MACHINE_PCI_H

#pragma once


class device_pci_interface : public device_interface
{
public:
	typedef delegate<void ()> pci_mapper_cb;

	pci_mapper_cb m_pci_remap_cb, m_pci_remap_config_cb;

	void pci_set_ids(uint32_t main_id, uint8_t revision, uint32_t pclass, uint32_t subsystem_id);
	void pci_set_ids_host(uint32_t main_id, uint32_t revision, uint64_t subsystem_id) { pci_set_ids(main_id, revision, 0x060000, subsystem_id); }
	void pci_set_ids_bridge(uint32_t main_id, uint32_t revision) { pci_set_ids(main_id, revision, 0x060400, 0x00000000); }
	void pci_set_ids_agp(uint64_t main_id, uint32_t revision, uint32_t subsystem_id) { pci_set_ids(main_id, revision, 0x030000, subsystem_id); }
	void pci_set_multifunction_device(bool enable);

	virtual void pci_set_remap_cb(pci_mapper_cb _remap_cb);
	virtual void pci_reset_all_mappings();
	virtual void pci_map_device(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space);
	virtual void pci_map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space);

	// Specify if this device must be mapped before all the others on the pci bus
	virtual bool pci_map_first() const { return false; }

	void pci_map_config(uint8_t device, address_space *config_space);

	virtual void pci_config_map(address_map &map);

	template<int N> uint32_t pci_unmapped_r(offs_t offset, uint32_t mem_mask);
	template<int N> void pci_unmapped_w(offs_t offset, uint32_t data, uint32_t mem_mask);

	uint16_t pci_vendor_r();
	uint16_t pci_device_r();
	uint16_t pci_command_r();
	void pci_command_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t pci_status_r();
	uint32_t pci_class_rev_r();
	virtual uint8_t pci_cache_line_size_r();
	virtual uint8_t pci_latency_timer_r();
	virtual uint8_t pci_header_type_r();
	virtual uint8_t pci_bist_r();
	uint32_t pci_address_base_r(offs_t offset);
	void pci_address_base_w(offs_t offset, uint32_t data);
	uint16_t pci_subvendor_r();
	uint16_t pci_subsystem_r();
	uint32_t pci_expansion_base_r();
	void pci_expansion_base_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	virtual uint8_t pci_capptr_r();
	uint8_t pci_interrupt_line_r();
	void pci_interrupt_line_w(offs_t offset, uint8_t data, uint8_t mem_mask = ~0);
	uint8_t pci_interrupt_pin_r();
	void pci_interrupt_pin_w(offs_t offset, uint8_t data, uint8_t mem_mask = ~0);

protected:
	device_pci_interface(const machine_config &mconfig, device_t &device);

	optional_memory_region m_pci_region;

	enum {
		M_MEM = 0,
		M_IO  = 1,
		M_64D = 2,
		M_64A = 4,
		M_PREF = 8,
		M_DISABLED = 16
	};

	struct pci_bank_info {
		address_map_constructor map;
		device_t *device;

		uint64_t adr;
		uint32_t size;
		int flags;
	};

	struct pci_bank_reg_info {
		int bank, hi;
	};

	pci_bank_info m_pci_bank_info[6];
	int m_pci_bank_count, m_pci_bank_reg_count;
	pci_bank_reg_info m_pci_bank_reg_info[6];

	uint32_t m_pci_main_id, m_pci_subsystem_id;
	uint32_t m_pci_pclass;
	uint8_t m_pci_revision;
	uint16_t m_pci_command, m_pci_command_mask, m_pci_status;
	const uint8_t *m_pci_expansion_rom;
	uint32_t m_pci_expansion_rom_size;
	uint32_t m_pci_expansion_rom_base;
	bool m_pci_is_multifunction_device;
	uint8_t m_pci_intr_line, m_pci_intr_pin;

	virtual void interface_pre_start() override;
	virtual void interface_pre_reset() override;

	void pci_skip_map_regs(int count);
	void pci_add_map(uint64_t size, int flags, const address_map_constructor &map, device_t *relative_to = nullptr);
	template <typename T> void pci_add_map(uint64_t size, int flags, void (T::*map)(address_map &map), const char *name) {
		address_map_constructor delegate(map, name, static_cast<T *>(this));
		pci_add_map(size, flags, delegate);
	}
	template <typename T> void pci_add_map(uint64_t size, int flags, T &device, void (T::*map)(address_map &map), const char *name) {
		address_map_constructor delegate(map, name, &device);
		pci_add_map(size, flags, delegate, &device);
	}

	void pci_add_rom(const uint8_t *data, uint32_t size);
	void pci_add_rom_from_region();

	void pci_set_map_address(int id, uint64_t adr);
	void pci_set_map_size(int id, uint64_t size);
	void pci_set_map_flags(int id, int flags);
};

class pci_device : public device_t, public device_pci_interface
{
protected:
	pci_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
};


class agp_device : public pci_device {
protected:
	agp_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;
};

class pci_bridge_device : public pci_device, public device_memory_interface {
public:
	pci_bridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, uint32_t main_id, uint32_t revision)
		: pci_bridge_device(mconfig, tag, owner, clock)
	{
		pci_set_ids_bridge(main_id, revision);
	}
	pci_bridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void pci_set_remap_cb(pci_mapper_cb _remap_cb) override;
	virtual void pci_map_device(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;
	virtual void pci_reset_all_mappings() override;

	virtual uint8_t pci_header_type_r() override;

	virtual void pci_config_map(address_map &map) override;

	uint32_t b_address_base_r(offs_t offset);
	void b_address_base_w(offs_t offset, uint32_t data);
	uint8_t primary_bus_r();
	void primary_bus_w(uint8_t data);
	uint8_t secondary_bus_r();
	void secondary_bus_w(uint8_t data);
	uint8_t subordinate_bus_r();
	void subordinate_bus_w(uint8_t data);
	uint8_t secondary_latency_r();
	void secondary_latency_w(uint8_t data);
	uint8_t iobase_r();
	void iobase_w(uint8_t data);
	uint8_t iolimit_r();
	void iolimit_w(uint8_t data);
	uint16_t secondary_status_r();
	void secondary_status_w(uint16_t data);
	uint16_t memory_base_r();
	void memory_base_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t memory_limit_r();
	void memory_limit_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t prefetch_base_r();
	void prefetch_base_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t prefetch_limit_r();
	void prefetch_limit_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint32_t prefetch_baseu_r();
	void prefetch_baseu_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t prefetch_limitu_r();
	void prefetch_limitu_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint16_t iobaseu_r();
	void iobaseu_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t iolimitu_r();
	void iolimitu_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t bridge_control_r();
	void bridge_control_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

protected:
	enum
	{
		AS_PCI_CONFIG = 0
	};

	pci_bridge_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual space_config_vector memory_space_config() const override;

	virtual device_t *bus_root();
	virtual void regenerate_config_mapping();

	uint32_t do_config_read(uint8_t bus, uint8_t device, uint16_t reg, uint32_t mem_mask);
	uint32_t propagate_config_read(uint8_t bus, uint8_t device, uint16_t reg, uint32_t mem_mask);
	uint32_t config_read(uint8_t bus, uint8_t device, uint16_t reg, uint32_t mem_mask);
	void do_config_write(uint8_t bus, uint8_t device, uint16_t reg, uint32_t data, uint32_t mem_mask);
	void propagate_config_write(uint8_t bus, uint8_t device, uint16_t reg, uint32_t data, uint32_t mem_mask);
	void config_write(uint8_t bus, uint8_t device, uint16_t reg, uint32_t data, uint32_t mem_mask);

	pci_device *sub_devices[32*8];
	std::vector<pci_device *> all_devices;
	std::vector<pci_bridge_device *> all_bridges;

	uint32_t prefetch_baseu, prefetch_limitu;
	uint16_t bridge_control, memory_base, memory_limit, prefetch_base, prefetch_limit, iobaseu, iolimitu;
	uint8_t primary_bus, secondary_bus, subordinate_bus, iobase, iolimit;

private:
	address_space_config configure_space_config;
};

class agp_bridge_device : public pci_bridge_device {
protected:
	agp_bridge_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;
};

class pci_host_device : public pci_bridge_device {
public:
	void io_configuration_access_map(address_map &map);

protected:
	pci_host_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	uint32_t config_address_r();
	void config_address_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t config_data_r(offs_t offset, uint32_t mem_mask = ~0);
	void config_data_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	virtual void device_start() override;
	virtual void device_reset() override;

	virtual device_t *bus_root() override;

	uint32_t root_config_read(uint8_t bus, uint8_t device, uint16_t reg, uint32_t mem_mask);
	void root_config_write(uint8_t bus, uint8_t device, uint16_t reg, uint32_t data, uint32_t mem_mask);

	void regenerate_mapping();

	address_space *memory_space, *io_space;

	uint64_t memory_window_start, memory_window_end, memory_offset;
	uint64_t io_window_start, io_window_end, io_offset;

	uint32_t config_address;
};

class pci_root_device : public device_t {
public:
	pci_root_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
};

DECLARE_DEVICE_TYPE(PCI_ROOT,   pci_root_device)
DECLARE_DEVICE_TYPE(PCI_BRIDGE, pci_bridge_device)

#endif // MAME_MACHINE_PCI_H
