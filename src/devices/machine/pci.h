// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef PCI_H
#define PCI_H

#include "emu.h"

#define MCFG_PCI_ROOT_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, PCI_ROOT, 0)

#define MCFG_PCI_DEVICE_ADD(_tag, _type, _main_id, _revision, _pclass, _subsystem_id) \
	MCFG_DEVICE_ADD(_tag, _type, 0) \
	downcast<pci_device *>(device)->set_ids(_main_id, _revision, _pclass, _subsystem_id);

#define MCFG_AGP_DEVICE_ADD(_tag, _type, _main_id, _revision, _subsystem_id) \
	MCFG_PCI_DEVICE_ADD(_tag, _type, _main_id, _revision, 0x030000, _subsystem_id)

#define MCFG_PCI_HOST_ADD(_tag, _type, _main_id, _revision, _subsystem_id) \
	MCFG_PCI_DEVICE_ADD(_tag, _type, _main_id, _revision, 0x060000, _subsystem_id)

#define MCFG_PCI_BRIDGE_ADD(_tag, _main_id, _revision) \
	MCFG_PCI_DEVICE_ADD(_tag, PCI_BRIDGE, _main_id, _revision, 0x060400, 0x00000000)

#define MCFG_AGP_BRIDGE_ADD(_tag, _type, _main_id, _revision) \
	MCFG_PCI_DEVICE_ADD(_tag, _type, _main_id, _revision, 0x060400, 0x00000000)

class pci_device : public device_t {
public:
	typedef delegate<void ()> mapper_cb;

	mapper_cb remap_cb, remap_config_cb;

	pci_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

	void set_ids(uint32_t main_id, uint8_t revision, uint32_t pclass, uint32_t subsystem_id);
	void set_multifunction_device(bool enable);

	virtual void set_remap_cb(mapper_cb _remap_cb);
	virtual void reset_all_mappings();
	virtual void map_device(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space);
	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space);

	void map_config(uint8_t device, address_space *config_space);

	virtual DECLARE_ADDRESS_MAP(config_map, 32);

	uint32_t unmapped_r(offs_t offset, uint32_t mem_mask, int bank);
	void unmapped_w(offs_t offset, uint32_t data, uint32_t mem_mask, int bank);

	uint32_t unmapped0_r(address_space &space, offs_t offset, uint32_t mem_mask);
	void unmapped0_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t unmapped1_r(address_space &space, offs_t offset, uint32_t mem_mask);
	void unmapped1_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t unmapped2_r(address_space &space, offs_t offset, uint32_t mem_mask);
	void unmapped2_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t unmapped3_r(address_space &space, offs_t offset, uint32_t mem_mask);
	void unmapped3_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t unmapped4_r(address_space &space, offs_t offset, uint32_t mem_mask);
	void unmapped4_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t unmapped5_r(address_space &space, offs_t offset, uint32_t mem_mask);
	void unmapped5_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask);


	uint16_t vendor_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t device_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t command_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void command_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t status_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint32_t class_rev_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	virtual uint8_t cache_line_size_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	virtual uint8_t latency_timer_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	virtual uint8_t header_type_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	virtual uint8_t bist_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint32_t address_base_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void address_base_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint16_t subvendor_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t subsystem_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint32_t expansion_base_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void expansion_base_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	virtual uint8_t capptr_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

protected:
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
		// One of the two
		address_map_delegate map;

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

	virtual void device_start() override;
	virtual void device_reset() override;

	void skip_map_regs(int count);
	void add_map(uint64_t size, int flags, address_map_delegate &map);
	template <typename T> void add_map(uint64_t size, int flags, void (T::*map)(address_map &map, device_t &device), const char *name) {
		address_map_delegate delegate(map, name, static_cast<T *>(this));
		add_map(size, flags, delegate);
	}

	void add_rom(const uint8_t *data, uint32_t size);
	void add_rom_from_region();

	void set_map_address(int id, uint64_t adr);
	void set_map_size(int id, uint64_t size);
	void set_map_flags(int id, int flags);
};

class agp_device : public pci_device {
public:
	agp_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
};

class pci_bridge_device : public pci_device, public device_memory_interface {
public:
	pci_bridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	pci_bridge_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

	virtual void set_remap_cb(mapper_cb _remap_cb) override;
	virtual void map_device(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;
	virtual void reset_all_mappings() override;

	virtual uint8_t header_type_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;

	virtual DECLARE_ADDRESS_MAP(config_map, 32) override;

	uint32_t b_address_base_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void b_address_base_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint8_t primary_bus_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void primary_bus_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t secondary_bus_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void secondary_bus_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t subordinate_bus_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void subordinate_bus_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t secondary_latency_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void secondary_latency_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t iobase_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void iobase_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t iolimit_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void iolimit_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint16_t secondary_status_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void secondary_status_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t memory_base_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void memory_base_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t memory_limit_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void memory_limit_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t prefetch_base_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void prefetch_base_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t prefetch_limit_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void prefetch_limit_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint32_t prefetch_baseu_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void prefetch_baseu_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t prefetch_limitu_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void prefetch_limitu_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint16_t iobaseu_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void iobaseu_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t iolimitu_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void iolimitu_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint8_t interrupt_line_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void interrupt_line_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t interrupt_pin_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void interrupt_pin_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint16_t bridge_control_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void bridge_control_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

protected:
	pci_device *sub_devices[32*8];
	std::vector<pci_device *> all_devices;
	std::vector<pci_bridge_device *> all_bridges;

	uint32_t prefetch_baseu, prefetch_limitu;
	uint16_t bridge_control, memory_base, memory_limit, prefetch_base, prefetch_limit, iobaseu, iolimitu;
	uint8_t primary_bus, secondary_bus, subordinate_bus, iobase, iolimit;

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual const address_space_config *memory_space_config(address_spacenum spacenum) const override;

	virtual device_t *bus_root();
	virtual void regenerate_config_mapping();

	uint32_t do_config_read(uint8_t bus, uint8_t device, uint16_t reg, uint32_t mem_mask);
	uint32_t propagate_config_read(uint8_t bus, uint8_t device, uint16_t reg, uint32_t mem_mask);
	uint32_t config_read(uint8_t bus, uint8_t device, uint16_t reg, uint32_t mem_mask);
	void do_config_write(uint8_t bus, uint8_t device, uint16_t reg, uint32_t data, uint32_t mem_mask);
	void propagate_config_write(uint8_t bus, uint8_t device, uint16_t reg, uint32_t data, uint32_t mem_mask);
	void config_write(uint8_t bus, uint8_t device, uint16_t reg, uint32_t data, uint32_t mem_mask);

private:
	address_space_config configure_space_config;
};

class agp_bridge_device : public pci_bridge_device {
public:
	agp_bridge_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
};

class pci_host_device : public pci_bridge_device {
public:
	DECLARE_ADDRESS_MAP(io_configuration_access_map, 32);

	pci_host_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

protected:
	address_space *memory_space, *io_space;

	uint64_t memory_window_start, memory_window_end, memory_offset;
	uint64_t io_window_start, io_window_end, io_offset;

	virtual void device_start() override;
	virtual void device_reset() override;

	virtual device_t *bus_root() override;

	uint32_t config_address;

	uint32_t config_address_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void config_address_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t config_data_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void config_data_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);

	uint32_t root_config_read(uint8_t bus, uint8_t device, uint16_t reg, uint32_t mem_mask);
	void root_config_write(uint8_t bus, uint8_t device, uint16_t reg, uint32_t data, uint32_t mem_mask);

	void regenerate_mapping();
};

class pci_root_device : public device_t {
public:
	pci_root_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
};

extern const device_type PCI_ROOT;
extern const device_type PCI_BRIDGE;

#endif
