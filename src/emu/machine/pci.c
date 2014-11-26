#include "pci.h"

const device_type PCI_ROOT   = &device_creator<pci_root_device>;
const device_type PCI_BRIDGE = &device_creator<pci_bridge_device>;

DEVICE_ADDRESS_MAP_START(config_map, 32, pci_device)
	AM_RANGE(0x00, 0x03) AM_READ16     (vendor_r,                          0x0000ffff)
	AM_RANGE(0x00, 0x03) AM_READ16     (device_r,                          0xffff0000)
	AM_RANGE(0x04, 0x07) AM_READWRITE16(command_r,         command_w,      0x0000ffff)
	AM_RANGE(0x04, 0x07) AM_READ16     (status_r,                          0xffff0000)
	AM_RANGE(0x08, 0x0b) AM_READ       (class_rev_r)
	AM_RANGE(0x0c, 0x0f) AM_READ8      (cache_line_size_r,                 0x000000ff)
	AM_RANGE(0x0c, 0x0f) AM_READ8      (latency_timer_r,                   0x0000ff00)
	AM_RANGE(0x0c, 0x0f) AM_READ8      (header_type_r,                     0x00ff0000)
	AM_RANGE(0x0c, 0x0f) AM_READ8      (bist_r,                            0xff000000)
	AM_RANGE(0x10, 0x27) AM_READWRITE  (address_base_r,    address_base_w)

	AM_RANGE(0x2c, 0x2f) AM_READ16     (subvendor_r,                       0x0000ffff)
	AM_RANGE(0x2c, 0x2f) AM_READ16     (subsystem_r,                       0xffff0000)
	AM_RANGE(0x2c, 0x2f) AM_WRITENOP

	AM_RANGE(0x34, 0x37) AM_READ8      (capptr_r,                          0x000000ff)
ADDRESS_MAP_END

pci_device::pci_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source)
{
	main_id = 0xffffffff;
	revision = 0x00;
	pclass = 0xffffff;
	subsystem_id = 0xffffffff;
}

void pci_device::set_ids(UINT32 _main_id, UINT8 _revision, UINT32 _pclass, UINT32 _subsystem_id)
{
	main_id = _main_id;
	revision = _revision;
	pclass = _pclass;
	subsystem_id = _subsystem_id;
}

void pci_device::device_start()
{
	command = 0x0080;
	command_mask = 0x01bf;
	status = 0x0000;

	for(int i=0; i<6; i++) {
		bank_infos[i].adr = 0;
		bank_infos[i].size = 0;
		bank_infos[i].flags = 0;
		bank_reg_infos[i].bank = -1;
		bank_reg_infos[i].hi = 0;
	}

	bank_count = 0;
	bank_reg_count = 0;
}

void pci_device::device_reset()
{
}

UINT32 pci_device::unmapped_r(offs_t offset, UINT32 mem_mask, int bank)
{
	logerror("%s: unmapped read from %08x & %08x (%s)\n", machine().describe_context(), offset*4, mem_mask, bank_infos[bank].map.name());
	return 0;
}

void pci_device::unmapped_w(offs_t offset, UINT32 data, UINT32 mem_mask, int bank)
{
	logerror("%s: unmapped write to %08x = %08x & %08x (%s)\n", machine().describe_context(), offset*4, data, mem_mask, bank_infos[bank].map.name());
}

READ32_MEMBER(pci_device::unmapped0_r) { return unmapped_r(offset, mem_mask, 0); }
WRITE32_MEMBER(pci_device::unmapped0_w) { return unmapped_w(offset, data, mem_mask, 0); }
READ32_MEMBER(pci_device::unmapped1_r) { return unmapped_r(offset, mem_mask, 1); }
WRITE32_MEMBER(pci_device::unmapped1_w) { return unmapped_w(offset, data, mem_mask, 1); }
READ32_MEMBER(pci_device::unmapped2_r) { return unmapped_r(offset, mem_mask, 2); }
WRITE32_MEMBER(pci_device::unmapped2_w) { return unmapped_w(offset, data, mem_mask, 2); }
READ32_MEMBER(pci_device::unmapped3_r) { return unmapped_r(offset, mem_mask, 3); }
WRITE32_MEMBER(pci_device::unmapped3_w) { return unmapped_w(offset, data, mem_mask, 3); }
READ32_MEMBER(pci_device::unmapped4_r) { return unmapped_r(offset, mem_mask, 4); }
WRITE32_MEMBER(pci_device::unmapped4_w) { return unmapped_w(offset, data, mem_mask, 4); }
READ32_MEMBER(pci_device::unmapped5_r) { return unmapped_r(offset, mem_mask, 5); }
WRITE32_MEMBER(pci_device::unmapped5_w) { return unmapped_w(offset, data, mem_mask, 5); }


READ32_MEMBER(pci_device::address_base_r)
{
	if(bank_reg_infos[offset].bank == -1)
		return 0;
	int bid = bank_reg_infos[offset].bank;
	if(bank_reg_infos[offset].hi)
		return bank_infos[bid].adr >> 32;
	int flags = bank_infos[bid].flags;
	return bank_infos[bid].adr | (flags & M_IO ? 1 : 0) | (flags & M_64A ? 4 : 0) | (flags & M_PREF ? 8 : 0);
}

WRITE32_MEMBER(pci_device::address_base_w)
{
	if(bank_reg_infos[offset].bank == -1) {
		logerror("%s: write to address base (%d, %08x) not linked to any bank\n", tag(), offset, data);
		return;
	}

	int bid = bank_reg_infos[offset].bank;
	if(bank_reg_infos[offset].hi)
		bank_infos[bid].adr = (bank_infos[bid].adr & 0xffffffff) | (UINT64(data) << 32);
	else {
		data &= ~(bank_infos[bid].size - 1);
		bank_infos[bid].adr = (bank_infos[bid].adr & U64(0xffffffff00000000)) | data;
	}
	remap_cb();
}

READ16_MEMBER(pci_device::vendor_r)
{
	return main_id >> 16;
}

READ16_MEMBER(pci_device::device_r)
{
	return main_id;
}

READ16_MEMBER(pci_device::command_r)
{
	return command;
}

WRITE16_MEMBER(pci_device::command_w)
{
	mem_mask &= command_mask;
	COMBINE_DATA(&command);
	logerror("%s: command = %04x\n", tag(), command);
}

READ16_MEMBER(pci_device::status_r)
{
	return status;
}

READ32_MEMBER(pci_device::class_rev_r)
{
	return (pclass << 8) | revision;
}

READ8_MEMBER(pci_device::cache_line_size_r)
{
	return 0x00;
}

READ8_MEMBER(pci_device::latency_timer_r)
{
	return 0x00;
}

READ8_MEMBER(pci_device::header_type_r)
{
	return 0x00;
}

READ8_MEMBER(pci_device::bist_r)
{
	return 0x00;
}

READ16_MEMBER(pci_device::subvendor_r)
{
	return subsystem_id >> 16;
}

READ16_MEMBER(pci_device::subsystem_r)
{
	return subsystem_id;
}

READ8_MEMBER(pci_device::capptr_r)
{
	return 0x00;
}

void pci_device::scan_sub_devices(pci_device **devices, dynamic_array<pci_device *> &all, dynamic_array<pci_device *> &bridges, device_t *root)
{
}

void pci_device::set_remap_cb(mapper_cb _remap_cb)
{
	remap_cb = _remap_cb;
}

void pci_device::reset_all_mappings()
{
}

void pci_device::map_device(UINT64 memory_window_start, UINT64 memory_window_end, UINT64 memory_offset, address_space *memory_space,
							UINT64 io_window_start, UINT64 io_window_end, UINT64 io_offset, address_space *io_space)
{
	for(int i=0; i<bank_count; i++) {
		bank_info &bi = bank_infos[i];
		if(!bi.adr)
			continue;
		if(UINT32(bi.adr) == UINT32(~(bi.size - 1)))
			continue;

		UINT64 start;
		address_space *space;
		if(bi.flags & M_IO) {
			space = io_space;
			start = bi.adr + io_offset;
		} else {
			space = memory_space;
			start = bi.adr + memory_offset;
		}
		UINT64 end = start + bi.size-1;
		switch(i) {
		case 0: space->install_readwrite_handler(start, end, 0, 0, read32_delegate(FUNC(pci_device::unmapped0_r), this), write32_delegate(FUNC(pci_device::unmapped0_w), this)); break;
		case 1: space->install_readwrite_handler(start, end, 0, 0, read32_delegate(FUNC(pci_device::unmapped1_r), this), write32_delegate(FUNC(pci_device::unmapped1_w), this)); break;
		case 2: space->install_readwrite_handler(start, end, 0, 0, read32_delegate(FUNC(pci_device::unmapped2_r), this), write32_delegate(FUNC(pci_device::unmapped2_w), this)); break;
		case 3: space->install_readwrite_handler(start, end, 0, 0, read32_delegate(FUNC(pci_device::unmapped3_r), this), write32_delegate(FUNC(pci_device::unmapped3_w), this)); break;
		case 4: space->install_readwrite_handler(start, end, 0, 0, read32_delegate(FUNC(pci_device::unmapped4_r), this), write32_delegate(FUNC(pci_device::unmapped4_w), this)); break;
		case 5: space->install_readwrite_handler(start, end, 0, 0, read32_delegate(FUNC(pci_device::unmapped5_r), this), write32_delegate(FUNC(pci_device::unmapped5_w), this)); break;
		}
		space->install_device_delegate(start, end, *this, bi.map);
	}

	map_extra(memory_window_start, memory_window_end, memory_offset, memory_space,
				io_window_start, io_window_end, io_offset, io_space);
}

void pci_device::map_extra(UINT64 memory_window_start, UINT64 memory_window_end, UINT64 memory_offset, address_space *memory_space,
							UINT64 io_window_start, UINT64 io_window_end, UINT64 io_offset, address_space *io_space)
{
}

void pci_device::map_config(UINT8 device, address_space *config_space)
{
	config_space->install_device(device << 12, (device << 12) | 0xfff, *this, &pci_device::config_map);
}

void pci_device::skip_map_regs(int count)
{
	bank_reg_count += count;
	assert(bank_reg_count <= 6);
}

void pci_device::add_map(UINT64 size, int flags, address_map_delegate &map)
{
	assert(bank_count < 6);
	int bid = bank_count++;
	bank_infos[bid].map = map;
	bank_infos[bid].adr = 0;
	bank_infos[bid].size = size;
	bank_infos[bid].flags = flags;

	if(flags & M_64A) {
		assert(bank_reg_count < 5);
		int breg = bank_reg_count;
		bank_reg_infos[breg].bank = bid;
		bank_reg_infos[breg].hi = 0;
		bank_reg_infos[breg+1].bank = bid;
		bank_reg_infos[breg+1].hi = 1;
		bank_reg_count += 2;
	} else {
		assert(bank_reg_count < 6);
		int breg = bank_reg_count++;
		bank_reg_infos[breg].bank = bid;
		bank_reg_infos[breg].hi = 0;
	}

	logerror("Device %s (%s) has 0x%" I64FMT "x bytes of %s named %s\n", tag(), name(), size, flags & M_IO ? "io" : "memory", map.name());
}

agp_device::agp_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: pci_device(mconfig, type, name, tag, owner, clock, shortname, source)
{
}

void agp_device::device_start()
{
	pci_device::device_start();
}

void agp_device::device_reset()
{
	pci_device::device_reset();
}



pci_bridge_device::pci_bridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: pci_device(mconfig, PCI_BRIDGE, "PCI-PCI Bridge", tag, owner, clock, "pci_bridge", __FILE__),
		device_memory_interface(mconfig, *this),
		configure_space_config("configuration_space", ENDIANNESS_LITTLE, 32, 20)
{
}

pci_bridge_device::pci_bridge_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: pci_device(mconfig, type, name, tag, owner, clock, shortname, source),
		device_memory_interface(mconfig, *this),
		configure_space_config("configuration_space", ENDIANNESS_LITTLE, 32, 20)
{
}

READ8_MEMBER(pci_bridge_device::header_type_r)
{
	return 0x01;
}

const address_space_config *pci_bridge_device::memory_space_config(address_spacenum spacenum) const
{
	return spacenum == AS_PROGRAM ? &configure_space_config : NULL;
}

device_t *pci_bridge_device::bus_root()
{
	return this;
}

void pci_bridge_device::set_remap_cb(mapper_cb _remap_cb)
{
	remap_cb = _remap_cb;
	for(int i=0; i != all_devices.count(); i++)
		if(all_devices[i] != this)
			all_devices[i]->set_remap_cb(_remap_cb);
}

void pci_bridge_device::device_start()
{
	pci_device::device_start();

	for(int i=0; i<32*8; i++)
		sub_devices[i] = NULL;

	for(device_t *d = bus_root()->first_subdevice(); d != NULL; d = d->next()) {
		const char *t = d->tag();
		int l = strlen(t);
		if(l <= 4 || t[l-5] != ':' || t[l-2] != '.')
			continue;
		int id = strtol(t+l-4, 0, 16);
		int fct = t[l-1] - '0';
		sub_devices[(id << 3) | fct] = downcast<pci_device *>(d);
	}

	mapper_cb cf_cb(FUNC(pci_bridge_device::regenerate_config_mapping), this);

	for(int i=0; i<32*8; i++)
		if(sub_devices[i]) {
			all_devices.append(sub_devices[i]);
			if(sub_devices[i] != this) {
				sub_devices[i]->remap_config_cb = cf_cb;
				sub_devices[i]->set_remap_cb(remap_cb);
				pci_bridge_device *bridge = dynamic_cast<pci_bridge_device *>(sub_devices[i]);
				if(bridge)
					all_bridges.append(bridge);
			}
		}
}

void pci_bridge_device::device_reset()
{
	pci_device::device_reset();
	regenerate_config_mapping();
}

void pci_bridge_device::reset_all_mappings()
{
	for(int i=0; i != all_devices.count(); i++)
		if(all_devices[i] != this)
			all_devices[i]->reset_all_mappings();
}


void pci_bridge_device::map_device(UINT64 memory_window_start, UINT64 memory_window_end, UINT64 memory_offset, address_space *memory_space,
									UINT64 io_window_start, UINT64 io_window_end, UINT64 io_offset, address_space *io_space)
{
	for(int i = all_devices.count()-1; i>=0; i--)
		if(all_devices[i] != this)
			all_devices[i]->map_device(memory_window_start, memory_window_end, memory_offset, memory_space,
										io_window_start, io_window_end, io_offset, io_space);

	map_extra(memory_window_start, memory_window_end, memory_offset, memory_space,
				io_window_start, io_window_end, io_offset, io_space);
}


void pci_bridge_device::regenerate_config_mapping()
{
	address_space *config_space = &space(AS_PROGRAM);
	config_space->unmap_readwrite(0x00000, 0xfffff);
	for(int i=0; i<32*8; i++)
		if(sub_devices[i])
			sub_devices[i]->map_config(i, config_space);
}


agp_bridge_device::agp_bridge_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: pci_bridge_device(mconfig, type, name, tag, owner, clock, shortname, source)
{
}

void agp_bridge_device::device_start()
{
	pci_bridge_device::device_start();
}

void agp_bridge_device::device_reset()
{
	pci_bridge_device::device_reset();
}



DEVICE_ADDRESS_MAP_START(io_configuration_access_map, 32, pci_host_device)
	AM_RANGE(0xcf8, 0xcfb) AM_READWRITE(config_address_r, config_address_w)
	AM_RANGE(0xcfc, 0xcff) AM_READWRITE(config_data_r,    config_data_w)
ADDRESS_MAP_END


pci_host_device::pci_host_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: pci_bridge_device(mconfig, type, name, tag, owner, clock, shortname, source)
{
}

device_t *pci_host_device::bus_root()
{
	return owner();
}

void pci_host_device::device_start()
{
	remap_cb = mapper_cb(FUNC(pci_host_device::regenerate_mapping), this);

	pci_bridge_device::device_start();

	memory_window_start = memory_window_end = memory_offset = 0;
	io_window_start = io_window_end = io_offset = 0;

	for(int i=0; i != all_devices.count(); i++)
		if(all_devices[i] != this)
			all_devices[i]->reset_all_mappings();
}

void pci_host_device::device_reset()
{
	pci_bridge_device::device_reset();
	reset_all_mappings();
	regenerate_mapping();

	config_address = 0;
}

void pci_host_device::regenerate_mapping()
{
	logerror("Regenerating mapping\n");
	memory_space->unmap_readwrite(memory_window_start, memory_window_end);
	io_space->unmap_readwrite(io_window_start, io_window_end);

	map_device(memory_window_start, memory_window_end, memory_offset, memory_space,
				io_window_start, io_window_end, io_offset, io_space);
}

READ32_MEMBER(pci_host_device::config_address_r)
{
	return config_address;
}

WRITE32_MEMBER(pci_host_device::config_address_w)
{
	COMBINE_DATA(&config_address);
}

READ32_MEMBER(pci_host_device::config_data_r)
{
	return config_address & 0x80000000 ? config_read((config_address >> 16) & 0xff, (config_address >> 8) & 0xff, config_address & 0xfc, mem_mask) : 0xffffffff;
}

WRITE32_MEMBER(pci_host_device::config_data_w)
{
	if(config_address & 0x80000000)
		config_write((config_address >> 16) & 0xff, (config_address >> 8) & 0xff, config_address & 0xfc, data, mem_mask);
}

UINT32 pci_host_device::config_read(UINT8 bus, UINT8 device, UINT16 reg, UINT32 mem_mask)
{
	UINT32 data = 0xffffffff;
	if(!bus) {
		if(sub_devices[device]) {
			data = space(AS_PROGRAM).read_dword((device << 12) | reg, mem_mask);
			logerror("config_read %02x:%02x.%x:%02x %08x @ %08x\n", bus, device >> 3, device & 7, reg, data, mem_mask);
		}
	} else
		abort();

	return data;
}

void pci_host_device::config_write(UINT8 bus, UINT8 device, UINT16 reg, UINT32 data, UINT32 mem_mask)
{
	if(!bus) {
		if(sub_devices[device]) {
			space(AS_PROGRAM).write_dword((device << 12) | reg, data, mem_mask);
			logerror("config_write %02x:%02x.%x:%02x %08x @ %08x\n", bus, device >> 3, device & 7, reg, data, mem_mask);
		}
	} else
		abort();
}


pci_root_device::pci_root_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, PCI_ROOT,"PCI virtual root", tag, owner, clock, "pci_root", __FILE__)
{
}

void pci_root_device::device_start()
{
}

void pci_root_device::device_reset()
{
}
