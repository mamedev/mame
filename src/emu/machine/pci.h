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

	pci_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	void set_ids(UINT32 main_id, UINT8 revision, UINT32 pclass, UINT32 subsystem_id);

	virtual void set_remap_cb(mapper_cb _remap_cb);
	virtual void reset_all_mappings();
	virtual void map_device(UINT64 memory_window_start, UINT64 memory_window_end, UINT64 memory_offset, address_space *memory_space,
							UINT64 io_window_start, UINT64 io_window_end, UINT64 io_offset, address_space *io_space);
	virtual void map_extra(UINT64 memory_window_start, UINT64 memory_window_end, UINT64 memory_offset, address_space *memory_space,
							UINT64 io_window_start, UINT64 io_window_end, UINT64 io_offset, address_space *io_space);

	void map_config(UINT8 device, address_space *config_space);

	virtual DECLARE_ADDRESS_MAP(config_map, 32);

	UINT32 unmapped_r(offs_t offset, UINT32 mem_mask, int bank);
	void unmapped_w(offs_t offset, UINT32 data, UINT32 mem_mask, int bank);

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
	virtual DECLARE_READ8_MEMBER(capptr_r);

protected:
	enum {
		M_MEM = 0,
		M_IO  = 1,
		M_64D = 2,
		M_64A = 4,
		M_PREF = 8
	};

	struct bank_info {
		address_map_delegate map;
		UINT64 adr;
		UINT32 size;
		int flags;
	};

	struct bank_reg_info {
		int bank, hi;
	};

	bank_info bank_infos[6];
	int bank_count, bank_reg_count;
	bank_reg_info bank_reg_infos[6];

	UINT32 main_id, subsystem_id;
	UINT32 pclass;
	UINT8 revision;
	UINT16 command, command_mask, status;

	virtual void device_start();
	virtual void device_reset();

	static void scan_sub_devices(pci_device **devices, dynamic_array<pci_device *> &all, dynamic_array<pci_device *> &bridges, device_t *root);

	void skip_map_regs(int count);
	void add_map(UINT64 size, int flags, address_map_delegate &map);
	template <typename T> void add_map(UINT64 size, int flags, void (T::*map)(address_map &map, device_t &device), const char *name) {
		address_map_delegate delegate(map, name, static_cast<T *>(this));
		add_map(size, flags, delegate);
	}
};

class agp_device : public pci_device {
public:
	agp_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

protected:
	virtual void device_start();
	virtual void device_reset();
};

class pci_bridge_device : public pci_device, public device_memory_interface {
public:
	pci_bridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	pci_bridge_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	virtual void set_remap_cb(mapper_cb _remap_cb);
	virtual void map_device(UINT64 memory_window_start, UINT64 memory_window_end, UINT64 memory_offset, address_space *memory_space,
							UINT64 io_window_start, UINT64 io_window_end, UINT64 io_offset, address_space *io_space);
	virtual void reset_all_mappings();

	virtual DECLARE_READ8_MEMBER(header_type_r);

protected:
	pci_device *sub_devices[32*8];
	dynamic_array<pci_device *> all_devices;
	dynamic_array<pci_device *> all_bridges;

	virtual void device_start();
	virtual void device_reset();
	virtual const address_space_config *memory_space_config(address_spacenum spacenum) const;

	virtual device_t *bus_root();
	virtual void regenerate_config_mapping();

private:
	address_space_config configure_space_config;
};

class agp_bridge_device : public pci_bridge_device {
public:
	agp_bridge_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

protected:
	virtual void device_start();
	virtual void device_reset();
};

class pci_host_device : public pci_bridge_device {
public:
	DECLARE_ADDRESS_MAP(io_configuration_access_map, 32);

	pci_host_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

protected:
	address_space *memory_space, *io_space;

	UINT64 memory_window_start, memory_window_end, memory_offset;
	UINT64 io_window_start, io_window_end, io_offset;

	virtual void device_start();
	virtual void device_reset();

	virtual device_t *bus_root();

	UINT32 config_address;

	DECLARE_READ32_MEMBER(config_address_r);
	DECLARE_WRITE32_MEMBER(config_address_w);
	DECLARE_READ32_MEMBER(config_data_r);
	DECLARE_WRITE32_MEMBER(config_data_w);

	UINT32 config_read(UINT8 bus, UINT8 device, UINT16 reg, UINT32 mem_mask);
	void config_write(UINT8 bus, UINT8 device, UINT16 reg, UINT32 data, UINT32 mem_mask);

	void regenerate_mapping();
};

class pci_root_device : public device_t {
public:
	pci_root_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual void device_start();
	virtual void device_reset();
};

extern const device_type PCI_ROOT;
extern const device_type PCI_BRIDGE;

#endif
