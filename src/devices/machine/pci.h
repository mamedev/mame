// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_MACHINE_PCI_H
#define MAME_MACHINE_PCI_H

#pragma once

class pci_device : public device_t {
public:
	typedef delegate<void ()> mapper_cb;

	mapper_cb remap_cb, remap_config_cb;

	void set_ids(uint32_t main_id, uint8_t revision, uint32_t pclass, uint32_t subsystem_id);
	void set_ids_host(uint32_t main_id, uint32_t revision, uint64_t subsystem_id) { set_ids(main_id, revision, 0x060000, subsystem_id); }
	void set_ids_bridge(uint32_t main_id, uint32_t revision) { set_ids(main_id, revision, 0x060400, 0x00000000); }
	void set_ids_agp(uint64_t main_id, uint32_t revision, uint32_t subsystem_id) { set_ids(main_id, revision, 0x030000, subsystem_id); }
	void set_multifunction_device(bool enable);

	virtual void set_remap_cb(mapper_cb _remap_cb);
	virtual void reset_all_mappings();
	virtual void map_device(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space);
	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space);

	// Specify if this device must be mapped before all the others on the pci bus
	virtual bool map_first() const { return false; }

	void map_config(uint8_t device, address_space *config_space);

	virtual void config_map(address_map &map) ATTR_COLD;

	uint32_t unmapped_r(offs_t offset, uint32_t mem_mask, int bank);
	void unmapped_w(offs_t offset, uint32_t data, uint32_t mem_mask, int bank);

	uint32_t unmapped0_r(offs_t offset, uint32_t mem_mask = ~0);
	void unmapped0_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t unmapped1_r(offs_t offset, uint32_t mem_mask = ~0);
	void unmapped1_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t unmapped2_r(offs_t offset, uint32_t mem_mask = ~0);
	void unmapped2_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t unmapped3_r(offs_t offset, uint32_t mem_mask = ~0);
	void unmapped3_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t unmapped4_r(offs_t offset, uint32_t mem_mask = ~0);
	void unmapped4_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t unmapped5_r(offs_t offset, uint32_t mem_mask = ~0);
	void unmapped5_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);


	uint16_t vendor_r();
	uint16_t device_r();
	uint16_t command_r();
	void command_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t status_r();
	uint32_t class_rev_r();
	virtual uint8_t cache_line_size_r();
	virtual uint8_t latency_timer_r();
	virtual uint8_t header_type_r();
	virtual uint8_t bist_r();
	uint32_t address_base_r(offs_t offset);
	void address_base_w(offs_t offset, uint32_t data);
	uint16_t subvendor_r();
	uint16_t subsystem_r();
	uint32_t expansion_base_r();
	void expansion_base_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	virtual uint8_t capptr_r();
	uint8_t interrupt_line_r();
	void interrupt_line_w(offs_t offset, uint8_t data, uint8_t mem_mask = ~0);
	uint8_t interrupt_pin_r();
	void interrupt_pin_w(offs_t offset, uint8_t data, uint8_t mem_mask = ~0);

protected:
	pci_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	optional_memory_region m_region;

	enum {
		M_MEM = 0,
		M_IO  = 1,
		M_64D = 2,
		M_64A = 4,
		M_PREF = 8,
		M_DISABLED = 16
	};

	struct bank_info {
		address_map_constructor map;
		device_t *device;

		uint64_t adr;
		uint32_t size;
		int flags;
	};

	struct bank_reg_info {
		int bank, hi;
	};

	bank_info bank_infos[6];
	int bank_count, bank_reg_count;
	bank_reg_info bank_reg_infos[6];

	class pci_root_device *m_pci_root;

	uint32_t main_id, subsystem_id;
	uint32_t pclass;
	uint8_t revision;
	uint16_t command, command_mask, status;
	const uint8_t *expansion_rom;
	uint32_t expansion_rom_size;
	uint32_t expansion_rom_base;
	bool is_multifunction_device;
	uint8_t intr_line, intr_pin;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void skip_map_regs(int count);
	void add_map(uint64_t size, int flags, const address_map_constructor &map, device_t *relative_to = nullptr);
	template <typename T> void add_map(uint64_t size, int flags, void (T::*map)(address_map &map), const char *name) {
		address_map_constructor delegate(map, name, static_cast<T *>(this));
		add_map(size, flags, delegate);
	}
	template <typename T> void add_map(uint64_t size, int flags, T &device, void (T::*map)(address_map &map), const char *name) {
		address_map_constructor delegate(map, name, &device);
		add_map(size, flags, delegate, &device);
	}

	void add_rom(const uint8_t *data, uint32_t size);
	void add_rom_from_region();

	void set_map_address(int id, uint64_t adr);
	void set_map_size(int id, uint64_t size);
	void set_map_flags(int id, int flags);

	inline address_space *get_pci_busmaster_space() const;
};

class agp_device : public pci_device {
protected:
	agp_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
};

class pci_bridge_device : public pci_device, public device_memory_interface {
public:
	pci_bridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, uint32_t main_id, uint32_t revision)
		: pci_bridge_device(mconfig, tag, owner, clock)
	{
		set_ids_bridge(main_id, revision);
	}
	pci_bridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void set_remap_cb(mapper_cb _remap_cb) override;
	virtual void map_device(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;
	virtual void reset_all_mappings() override;

	virtual uint8_t header_type_r() override;

	virtual void config_map(address_map &map) override ATTR_COLD;

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
	virtual void bridge_control_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

protected:
	enum
	{
		AS_PCI_CONFIG = 0
	};

	pci_bridge_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void interface_post_reset() override;
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

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
};

class pci_host_device : public pci_bridge_device {
public:
	void io_configuration_access_map(address_map &map) ATTR_COLD;

	void set_spaces(address_space *memory, address_space *io = nullptr, address_space *busmaster = nullptr);

protected:
	pci_host_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	uint32_t config_address_r();
	void config_address_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t config_data_r(offs_t offset, uint32_t mem_mask = ~0);
	void config_data_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t config_data_ex_r(offs_t offset, uint32_t mem_mask = ~0);
	void config_data_ex_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void interface_post_reset() override;

	virtual device_t *bus_root() override;

	uint32_t root_config_read(uint8_t bus, uint8_t device, uint16_t reg, uint32_t mem_mask);
	void root_config_write(uint8_t bus, uint8_t device, uint16_t reg, uint32_t data, uint32_t mem_mask);

	void regenerate_mapping();

	uint64_t memory_window_start, memory_window_end, memory_offset;
	uint64_t io_window_start, io_window_end, io_offset;

	uint32_t config_address;

private:
	address_space *memory_space, *io_space;
};

using pci_pin_mapper = device_delegate<int (int)>;
using pci_irq_handler = device_delegate<void (int, int)>;

class pci_root_device : public device_t {
public:
	pci_root_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void irq_pin_w(int pin, int state);
	void irq_w(int line, int state);

	void set_pin_mapper(pci_pin_mapper &&mapper) { m_pin_mapper = std::move(mapper); }
	void set_irq_handler(pci_irq_handler &&handler) { m_irq_handler = std::move(handler); }

	address_space *get_pci_busmaster_space() const { return m_pci_busmaster_space; }

	void set_pci_busmaster_space(address_space *space) { m_pci_busmaster_space = space; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	pci_pin_mapper m_pin_mapper;
	pci_irq_handler m_irq_handler;
	address_space *m_pci_busmaster_space;
};

address_space *pci_device::get_pci_busmaster_space() const
{
	return m_pci_root->get_pci_busmaster_space();
}

DECLARE_DEVICE_TYPE(PCI_ROOT,   pci_root_device)
DECLARE_DEVICE_TYPE(PCI_BRIDGE, pci_bridge_device)

#endif // MAME_MACHINE_PCI_H
