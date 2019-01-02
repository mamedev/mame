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

	virtual void config_map(address_map &map);

	uint32_t unmapped_r(offs_t offset, uint32_t mem_mask, int bank);
	void unmapped_w(offs_t offset, uint32_t data, uint32_t mem_mask, int bank);

	READ32_MEMBER (unmapped0_r);
	WRITE32_MEMBER(unmapped0_w);
	READ32_MEMBER (unmapped1_r);
	WRITE32_MEMBER(unmapped1_w);
	READ32_MEMBER (unmapped2_r);
	WRITE32_MEMBER(unmapped2_w);
	READ32_MEMBER (unmapped3_r);
	WRITE32_MEMBER(unmapped3_w);
	READ32_MEMBER (unmapped4_r);
	WRITE32_MEMBER(unmapped4_w);
	READ32_MEMBER (unmapped5_r);
	WRITE32_MEMBER(unmapped5_w);


	DECLARE_READ16_MEMBER(vendor_r);
	DECLARE_READ16_MEMBER(device_r);
	DECLARE_READ16_MEMBER(command_r);
	DECLARE_WRITE16_MEMBER(command_w);
	DECLARE_READ16_MEMBER(status_r);
	DECLARE_READ32_MEMBER(class_rev_r);
	virtual DECLARE_READ8_MEMBER(cache_line_size_r);
	virtual DECLARE_READ8_MEMBER(latency_timer_r);
	virtual DECLARE_READ8_MEMBER(header_type_r);
	virtual DECLARE_READ8_MEMBER(bist_r);
	DECLARE_READ32_MEMBER(address_base_r);
	DECLARE_WRITE32_MEMBER(address_base_w);
	DECLARE_READ16_MEMBER(subvendor_r);
	DECLARE_READ16_MEMBER(subsystem_r);
	DECLARE_READ32_MEMBER (expansion_base_r);
	DECLARE_WRITE32_MEMBER(expansion_base_w);
	virtual DECLARE_READ8_MEMBER(capptr_r);
	DECLARE_READ8_MEMBER(interrupt_line_r);
	DECLARE_WRITE8_MEMBER(interrupt_line_w);
	DECLARE_READ8_MEMBER(interrupt_pin_r);
	DECLARE_WRITE8_MEMBER(interrupt_pin_w);

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

	uint32_t main_id, subsystem_id;
	uint32_t pclass;
	uint8_t revision;
	uint16_t command, command_mask, status;
	const uint8_t *expansion_rom;
	uint32_t expansion_rom_size;
	uint32_t expansion_rom_base;
	bool is_multifunction_device;
	uint8_t intr_line, intr_pin;

	virtual void device_start() override;
	virtual void device_reset() override;

	void skip_map_regs(int count);
	void add_map(uint64_t size, int flags, const address_map_constructor &map, device_t *relative_to = nullptr);
	template <typename T> void add_map(uint64_t size, int flags, void (T::*map)(address_map &map), const char *name) {
		address_map_constructor delegate(map, name, static_cast<T *>(this));
		add_map(size, flags, delegate);
	}

	void add_rom(const uint8_t *data, uint32_t size);
	void add_rom_from_region();

	void set_map_address(int id, uint64_t adr);
	void set_map_size(int id, uint64_t size);
	void set_map_flags(int id, int flags);
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
		set_ids_bridge(main_id, revision);
	}
	pci_bridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void set_remap_cb(mapper_cb _remap_cb) override;
	virtual void map_device(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;
	virtual void reset_all_mappings() override;

	virtual DECLARE_READ8_MEMBER(header_type_r) override;

	virtual void config_map(address_map &map) override;

	DECLARE_READ32_MEMBER (b_address_base_r);
	DECLARE_WRITE32_MEMBER(b_address_base_w);
	DECLARE_READ8_MEMBER  (primary_bus_r);
	DECLARE_WRITE8_MEMBER (primary_bus_w);
	DECLARE_READ8_MEMBER  (secondary_bus_r);
	DECLARE_WRITE8_MEMBER (secondary_bus_w);
	DECLARE_READ8_MEMBER  (subordinate_bus_r);
	DECLARE_WRITE8_MEMBER (subordinate_bus_w);
	DECLARE_READ8_MEMBER  (secondary_latency_r);
	DECLARE_WRITE8_MEMBER (secondary_latency_w);
	DECLARE_READ8_MEMBER  (iobase_r);
	DECLARE_WRITE8_MEMBER (iobase_w);
	DECLARE_READ8_MEMBER  (iolimit_r);
	DECLARE_WRITE8_MEMBER (iolimit_w);
	DECLARE_READ16_MEMBER (secondary_status_r);
	DECLARE_WRITE16_MEMBER(secondary_status_w);
	DECLARE_READ16_MEMBER (memory_base_r);
	DECLARE_WRITE16_MEMBER(memory_base_w);
	DECLARE_READ16_MEMBER (memory_limit_r);
	DECLARE_WRITE16_MEMBER(memory_limit_w);
	DECLARE_READ16_MEMBER (prefetch_base_r);
	DECLARE_WRITE16_MEMBER(prefetch_base_w);
	DECLARE_READ16_MEMBER (prefetch_limit_r);
	DECLARE_WRITE16_MEMBER(prefetch_limit_w);
	DECLARE_READ32_MEMBER (prefetch_baseu_r);
	DECLARE_WRITE32_MEMBER(prefetch_baseu_w);
	DECLARE_READ32_MEMBER (prefetch_limitu_r);
	DECLARE_WRITE32_MEMBER(prefetch_limitu_w);
	DECLARE_READ16_MEMBER (iobaseu_r);
	DECLARE_WRITE16_MEMBER(iobaseu_w);
	DECLARE_READ16_MEMBER (iolimitu_r);
	DECLARE_WRITE16_MEMBER(iolimitu_w);
	DECLARE_READ16_MEMBER (bridge_control_r);
	DECLARE_WRITE16_MEMBER(bridge_control_w);

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

	DECLARE_READ32_MEMBER(config_address_r);
	DECLARE_WRITE32_MEMBER(config_address_w);
	DECLARE_READ32_MEMBER(config_data_r);
	DECLARE_WRITE32_MEMBER(config_data_w);

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
