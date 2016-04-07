// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "pci.h"

const device_type PCI_ROOT   = &device_creator<pci_root_device>;
const device_type PCI_BRIDGE = &device_creator<pci_bridge_device>;

DEVICE_ADDRESS_MAP_START(config_map, 32, pci_device)
	AM_RANGE(0x00, 0x03) AM_READ16     (vendor_r,                                 0x0000ffff)
	AM_RANGE(0x00, 0x03) AM_READ16     (device_r,                                 0xffff0000)
	AM_RANGE(0x04, 0x07) AM_READWRITE16(command_r,           command_w,           0x0000ffff)
	AM_RANGE(0x04, 0x07) AM_READ16     (status_r,                                 0xffff0000)
	AM_RANGE(0x08, 0x0b) AM_READ       (class_rev_r)
	AM_RANGE(0x0c, 0x0f) AM_READ8      (cache_line_size_r,                        0x000000ff)
	AM_RANGE(0x0c, 0x0f) AM_READ8      (latency_timer_r,                          0x0000ff00)
	AM_RANGE(0x0c, 0x0f) AM_READ8      (header_type_r,                            0x00ff0000)
	AM_RANGE(0x0c, 0x0f) AM_READ8      (bist_r,                                   0xff000000)
	AM_RANGE(0x0c, 0x0f) AM_WRITENOP
	AM_RANGE(0x10, 0x27) AM_READWRITE  (address_base_r,      address_base_w)
	// Cardbus CIS pointer at 28
	AM_RANGE(0x2c, 0x2f) AM_READ16     (subvendor_r,                              0x0000ffff)
	AM_RANGE(0x2c, 0x2f) AM_READ16     (subsystem_r,                              0xffff0000)
	AM_RANGE(0x2c, 0x2f) AM_WRITENOP
	AM_RANGE(0x30, 0x33) AM_READWRITE  (expansion_base_r,    expansion_base_w)
	AM_RANGE(0x34, 0x37) AM_READ8      (capptr_r,                                 0x000000ff)
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START(config_map, 32, pci_bridge_device)
	AM_RANGE(0x00, 0x03) AM_READ16     (vendor_r,                                 0x0000ffff)
	AM_RANGE(0x00, 0x03) AM_READ16     (device_r,                                 0xffff0000)
	AM_RANGE(0x04, 0x07) AM_READWRITE16(command_r,           command_w,           0x0000ffff)
	AM_RANGE(0x04, 0x07) AM_READ16     (status_r,                                 0xffff0000)
	AM_RANGE(0x08, 0x0b) AM_READ       (class_rev_r)
	AM_RANGE(0x0c, 0x0f) AM_READ8      (cache_line_size_r,                        0x000000ff)
	AM_RANGE(0x0c, 0x0f) AM_READ8      (latency_timer_r,                          0x0000ff00)
	AM_RANGE(0x0c, 0x0f) AM_READ8      (header_type_r,                            0x00ff0000)
	AM_RANGE(0x0c, 0x0f) AM_READ8      (bist_r,                                   0xff000000)
	AM_RANGE(0x10, 0x17) AM_READWRITE  (b_address_base_r,    b_address_base_w)
	AM_RANGE(0x18, 0x1b) AM_READWRITE8 (primary_bus_r,       primary_bus_w,       0x000000ff)
	AM_RANGE(0x18, 0x1b) AM_READWRITE8 (secondary_bus_r,     secondary_bus_w,     0x0000ff00)
	AM_RANGE(0x18, 0x1b) AM_READWRITE8 (subordinate_bus_r,   subordinate_bus_w,   0x00ff0000)
	AM_RANGE(0x18, 0x1b) AM_READWRITE8 (secondary_latency_r, secondary_latency_w, 0xff000000)
	AM_RANGE(0x1c, 0x1f) AM_READWRITE8 (iobase_r,            iobase_w,            0x000000ff)
	AM_RANGE(0x1c, 0x1f) AM_READWRITE8 (iolimit_r,           iolimit_w,           0x0000ff00)
	AM_RANGE(0x1c, 0x1f) AM_READWRITE16(secondary_status_r,  secondary_status_w,  0xffff0000)
	AM_RANGE(0x20, 0x23) AM_READWRITE16(memory_base_r,       memory_base_w,       0x0000ffff)
	AM_RANGE(0x20, 0x23) AM_READWRITE16(memory_limit_r,      memory_limit_w,      0xffff0000)
	AM_RANGE(0x24, 0x27) AM_READWRITE16(prefetch_base_r,     prefetch_base_w,     0x0000ffff)
	AM_RANGE(0x24, 0x27) AM_READWRITE16(prefetch_limit_r,    prefetch_limit_w,    0xffff0000)
	AM_RANGE(0x28, 0x2b) AM_READWRITE  (prefetch_baseu_r,    prefetch_baseu_w)
	AM_RANGE(0x2c, 0x2f) AM_READWRITE  (prefetch_limitu_r,   prefetch_limitu_w)
	AM_RANGE(0x30, 0x33) AM_READWRITE16(iobaseu_r,           iobaseu_w,           0x0000ffff)
	AM_RANGE(0x30, 0x33) AM_READWRITE16(iolimitu_r,          iolimitu_w,          0xffff0000)
	AM_RANGE(0x34, 0x37) AM_READ8      (capptr_r,                                 0x000000ff)
	AM_RANGE(0x38, 0x3b) AM_READWRITE  (expansion_base_r,    expansion_base_w)
	AM_RANGE(0x3c, 0x3f) AM_READWRITE8 (interrupt_line_r,    interrupt_line_w,    0x000000ff)
	AM_RANGE(0x3c, 0x3f) AM_READWRITE8 (interrupt_pin_r,     interrupt_pin_w,     0x0000ff00)
	AM_RANGE(0x3c, 0x3f) AM_READWRITE16(bridge_control_r,    bridge_control_w,    0xffff0000)
ADDRESS_MAP_END

pci_device::pci_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		m_region(*this, DEVICE_SELF)
{
	main_id = 0xffffffff;
	revision = 0x00;
	pclass = 0xffffff;
	subsystem_id = 0xffffffff;
	is_multifunction_device = false;
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
		bank_infos[i].adr = -1;
		bank_infos[i].size = 0;
		bank_infos[i].flags = 0;
		bank_reg_infos[i].bank = -1;
		bank_reg_infos[i].hi = 0;
	}

	bank_count = 0;
	bank_reg_count = 0;

	expansion_rom = nullptr;
	expansion_rom_size = 0;
	expansion_rom_base = 0;
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
	return (bank_infos[bid].adr & ~(bank_infos[bid].size - 1)) | (flags & M_IO ? 1 : 0) | (flags & M_64A ? 4 : 0) | (flags & M_PREF ? 8 : 0);
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

void pci_device::set_multifunction_device(bool enable)
{
	is_multifunction_device = enable;
}

READ8_MEMBER(pci_device::header_type_r)
{
	return is_multifunction_device ? 0x80 : 0x00;
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

READ32_MEMBER(pci_device::expansion_base_r)
{
	return expansion_rom_base;
}


WRITE32_MEMBER(pci_device::expansion_base_w)
{
	COMBINE_DATA(&expansion_rom_base);
	if(!expansion_rom_size)
		expansion_rom_base = 0;
	else {
		// Trick to get an address resolution at expansion_rom_size with minimal granularity of 0x800, plus bit 1 set to keep the on/off information
		expansion_rom_base &= 0xfffff801 & (1-expansion_rom_size);
	}
	remap_cb();
}

READ8_MEMBER(pci_device::capptr_r)
{
	return 0x00;
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
		if(UINT32(bi.adr) == 0xffffffff)
			continue;
		if(!bi.size || (bi.flags & M_DISABLED))
			continue;

		address_space *space;
		UINT64 start = bi.adr & ~(bi.size - 1);

		if(bi.flags & M_IO) {
			space = io_space;
			start += io_offset;
		} else {
			space = memory_space;
			start += memory_offset;
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
		logerror("%s: map %s at %0*x-%0*x\n", tag(), bi.map.name(), bi.flags & M_IO ? 4 : 8, UINT32(start), bi.flags & M_IO ? 4 : 8, UINT32(end));
	}

	map_extra(memory_window_start, memory_window_end, memory_offset, memory_space,
				io_window_start, io_window_end, io_offset, io_space);

	if(expansion_rom_base & 1) {
		logerror("%s: map expansion rom at %08x-%08x\n", tag(), expansion_rom_base & ~1, (expansion_rom_base & ~1) + expansion_rom_size - 1);
		UINT32 start = (expansion_rom_base & ~1) + memory_offset;
		UINT32 end = start + expansion_rom_size - 1;
		if(end > memory_window_end)
			end = memory_window_end;
		memory_space->install_rom(start, end, (void *)expansion_rom);
	}
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

	logerror("Device %s (%s) has 0x%x bytes of %s named %s\n", tag(), name(), size, flags & M_IO ? "io" : "memory", bank_infos[bid].map.name());
}

void pci_device::add_rom(const UINT8 *rom, UINT32 size)
{
	expansion_rom = rom;
	expansion_rom_size = size;
	logerror("Device %s (%s) has 0x%x bytes of expansion rom\n", tag(), name(), size);
}

void pci_device::add_rom_from_region()
{
	add_rom(m_region->base(), m_region->bytes());
}

void pci_device::set_map_address(int id, UINT64 adr)
{
	bank_infos[id].adr = adr;
	remap_cb();
}

void pci_device::set_map_size(int id, UINT64 size)
{
	bank_infos[id].size = size;
	remap_cb();
}

void pci_device::set_map_flags(int id, int flags)
{
	bank_infos[id].flags = flags;
	remap_cb();
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
	return spacenum == AS_PROGRAM ? &configure_space_config : nullptr;
}

device_t *pci_bridge_device::bus_root()
{
	return this;
}

void pci_bridge_device::set_remap_cb(mapper_cb _remap_cb)
{
	remap_cb = _remap_cb;
	for(unsigned int i=0; i != all_devices.size(); i++)
		if(all_devices[i] != this)
			all_devices[i]->set_remap_cb(_remap_cb);
}

void pci_bridge_device::device_start()
{
	pci_device::device_start();

	for(auto & elem : sub_devices)
		elem = nullptr;

	for (device_t &d : bus_root()->subdevices())
	{
		const char *t = d.tag();
		int l = strlen(t);
		if(l <= 4 || t[l-5] != ':' || t[l-2] != '.')
			continue;
		int id = strtol(t+l-4, nullptr, 16);
		int fct = t[l-1] - '0';
		sub_devices[(id << 3) | fct] = downcast<pci_device *>(&d);
	}

	mapper_cb cf_cb(FUNC(pci_bridge_device::regenerate_config_mapping), this);

	for(int i=0; i<32*8; i++)
		if(sub_devices[i]) {
			if((i & 7) && sub_devices[i & ~7])
				sub_devices[i & ~7]->set_multifunction_device(true);

			all_devices.push_back(sub_devices[i]);
			if(sub_devices[i] != this) {
				sub_devices[i]->remap_config_cb = cf_cb;
				sub_devices[i]->set_remap_cb(remap_cb);
				pci_bridge_device *bridge = dynamic_cast<pci_bridge_device *>(sub_devices[i]);
				if(bridge)
					all_bridges.push_back(bridge);
			}
		}
}

void pci_bridge_device::device_reset()
{
	pci_device::device_reset();

	bridge_control = 0x0000;
	primary_bus = 0x00;
	secondary_bus = 0x00;
	subordinate_bus = 0x00;
	regenerate_config_mapping();
}

void pci_bridge_device::reset_all_mappings()
{
	pci_device::reset_all_mappings();

	for(unsigned int i=0; i != all_devices.size(); i++)
		if(all_devices[i] != this)
			all_devices[i]->reset_all_mappings();

	prefetch_baseu = 0;
	prefetch_limitu = 0;
	memory_base = 0;
	memory_limit = 0;
	prefetch_base = 0;
	prefetch_limit = 0;
	iobaseu = 0;
	iolimitu = 0;
	iobase = 0;
	iolimit = 0;
}

void pci_bridge_device::map_device(UINT64 memory_window_start, UINT64 memory_window_end, UINT64 memory_offset, address_space *memory_space,
									UINT64 io_window_start, UINT64 io_window_end, UINT64 io_offset, address_space *io_space)
{
	for(int i = int(all_devices.size())-1; i>=0; i--)
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

UINT32 pci_bridge_device::do_config_read(UINT8 bus, UINT8 device, UINT16 reg, UINT32 mem_mask)
{
	if(sub_devices[device]) {
		UINT32 data = space(AS_PROGRAM).read_dword((device << 12) | reg, mem_mask);
		logerror("%s: config_read %02x:%02x.%x:%02x %08x @ %08x\n", tag(), bus, device >> 3, device & 7, reg, data, mem_mask);
		return data;
	} else
		return 0xffffffff;
}

UINT32 pci_bridge_device::propagate_config_read(UINT8 bus, UINT8 device, UINT16 reg, UINT32 mem_mask)
{
	UINT32 data = 0xffffffff;
	for(unsigned int i=0; i != all_bridges.size(); i++)
		data &= all_bridges[i]->config_read(bus, device, reg, mem_mask);
	return data;
}

UINT32 pci_bridge_device::config_read(UINT8 bus, UINT8 device, UINT16 reg, UINT32 mem_mask)
{
	if(bus == secondary_bus)
		return do_config_read(bus, device, reg, mem_mask);

	if(bus > secondary_bus && bus <= subordinate_bus)
		return propagate_config_read(bus, device, reg, mem_mask);

	return 0xffffffff;
}

void pci_bridge_device::do_config_write(UINT8 bus, UINT8 device, UINT16 reg, UINT32 data, UINT32 mem_mask)
{
	if(sub_devices[device]) {
		space(AS_PROGRAM).write_dword((device << 12) | reg, data, mem_mask);
		logerror("%s: config_write %02x:%02x.%x:%02x %08x @ %08x\n", tag(), bus, device >> 3, device & 7, reg, data, mem_mask);
	}
}

void pci_bridge_device::propagate_config_write(UINT8 bus, UINT8 device, UINT16 reg, UINT32 data, UINT32 mem_mask)
{
	for(unsigned int i=0; i != all_bridges.size(); i++)
		all_bridges[i]->config_write(bus, device, reg, data, mem_mask);
}

void pci_bridge_device::config_write(UINT8 bus, UINT8 device, UINT16 reg, UINT32 data, UINT32 mem_mask)
{
	if(bus == secondary_bus)
		do_config_write(bus, device, reg, data, mem_mask);

	else if(bus > secondary_bus && bus <= subordinate_bus)
		propagate_config_write(bus, device, reg, data, mem_mask);
}

READ32_MEMBER (pci_bridge_device::b_address_base_r)
{
	logerror("%s: b_address_base_r %d\n", tag(), offset);
	return 0xffffffff;
}

WRITE32_MEMBER(pci_bridge_device::b_address_base_w)
{
	logerror("%s: b_address_base_w %d, %08x\n", tag(), offset, data);
}

READ8_MEMBER  (pci_bridge_device::primary_bus_r)
{
	logerror("%s: primary_bus_r\n", tag());
	return primary_bus;
}

WRITE8_MEMBER (pci_bridge_device::primary_bus_w)
{
	primary_bus = data;
	logerror("%s: primary_bus_w %02x\n", tag(), data);
}

READ8_MEMBER  (pci_bridge_device::secondary_bus_r)
{
	logerror("%s: secondary_bus_r\n", tag());
	return secondary_bus;
}

WRITE8_MEMBER (pci_bridge_device::secondary_bus_w)
{
	secondary_bus = data;
	logerror("%s: secondary_bus_w %02x\n", tag(), data);
}

READ8_MEMBER  (pci_bridge_device::subordinate_bus_r)
{
	logerror("%s: subordinate_bus_r\n", tag());
	return subordinate_bus;
}

WRITE8_MEMBER (pci_bridge_device::subordinate_bus_w)
{
	subordinate_bus = data;
	logerror("%s: subordinate_bus_w %02x\n", tag(), data);
}

READ8_MEMBER  (pci_bridge_device::secondary_latency_r)
{
	logerror("%s: secondary_latency_r\n", tag());
	return 0xff;
}

WRITE8_MEMBER (pci_bridge_device::secondary_latency_w)
{
	logerror("%s: secondary_latency_w %02x\n", tag(), data);
}

READ8_MEMBER  (pci_bridge_device::iobase_r)
{
	return iobase;
}

WRITE8_MEMBER (pci_bridge_device::iobase_w)
{
	iobase = data;
	logerror("%s: iobase_w %02x\n", tag(), data);
}

READ8_MEMBER  (pci_bridge_device::iolimit_r)
{
	return iolimit;
}

WRITE8_MEMBER (pci_bridge_device::iolimit_w)
{
	iolimit = data;
	logerror("%s: iolimit_w %02x\n", tag(), data);
}

READ16_MEMBER (pci_bridge_device::secondary_status_r)
{
	logerror("%s: secondary_status_r\n", tag());
	return 0xffff;
}

WRITE16_MEMBER(pci_bridge_device::secondary_status_w)
{
	logerror("%s: secondary_status_w %04x\n", tag(), data);
}

READ16_MEMBER (pci_bridge_device::memory_base_r)
{
	return memory_base;
}

WRITE16_MEMBER(pci_bridge_device::memory_base_w)
{
	COMBINE_DATA(&memory_base);
	logerror("%s: memory_base_w %04x\n", tag(), memory_base);
}

READ16_MEMBER (pci_bridge_device::memory_limit_r)
{
	return memory_limit;
}

WRITE16_MEMBER(pci_bridge_device::memory_limit_w)
{
	COMBINE_DATA(&memory_limit);
	logerror("%s: memory_limit_w %04x\n", tag(), memory_limit);
}

READ16_MEMBER (pci_bridge_device::prefetch_base_r)
{
	return prefetch_base;
}

WRITE16_MEMBER(pci_bridge_device::prefetch_base_w)
{
	COMBINE_DATA(&prefetch_base);
	logerror("%s: prefetch_base_w %04x\n", tag(), prefetch_base);
}

READ16_MEMBER (pci_bridge_device::prefetch_limit_r)
{
	return prefetch_limit;
}

WRITE16_MEMBER(pci_bridge_device::prefetch_limit_w)
{
	COMBINE_DATA(&prefetch_limit);
	logerror("%s: prefetch_limit_w %04x\n", tag(), prefetch_limit);
}

READ32_MEMBER (pci_bridge_device::prefetch_baseu_r)
{
	return prefetch_baseu;
}

WRITE32_MEMBER(pci_bridge_device::prefetch_baseu_w)
{
	COMBINE_DATA(&prefetch_baseu);
	logerror("%s: prefetch_baseu_w %08x\n", tag(), prefetch_baseu);
}

READ32_MEMBER (pci_bridge_device::prefetch_limitu_r)
{
	return prefetch_limitu;
}

WRITE32_MEMBER(pci_bridge_device::prefetch_limitu_w)
{
	COMBINE_DATA(&prefetch_limitu);
	logerror("%s: prefetch_limitu_w %08x\n", tag(), prefetch_limitu);
}

READ16_MEMBER (pci_bridge_device::iobaseu_r)
{
	return iobaseu;
}

WRITE16_MEMBER(pci_bridge_device::iobaseu_w)
{
	COMBINE_DATA(&iobaseu);
	logerror("%s: iobaseu_w %04x\n", tag(), iobaseu);
}

READ16_MEMBER (pci_bridge_device::iolimitu_r)
{
	return iolimitu;
}

WRITE16_MEMBER(pci_bridge_device::iolimitu_w)
{
	COMBINE_DATA(&iolimitu);
	logerror("%s: iolimitu_w %04x\n", tag(), iolimitu);
}

READ8_MEMBER  (pci_bridge_device::interrupt_line_r)
{
	logerror("%s: interrupt_line_r\n", tag());
	return 0xff;
}

WRITE8_MEMBER (pci_bridge_device::interrupt_line_w)
{
	logerror("%s: interrupt_line_w %02x\n", tag(), data);
}

READ8_MEMBER  (pci_bridge_device::interrupt_pin_r)
{
	logerror("%s: interrupt_pin_r\n", tag());
	return 0xff;
}

WRITE8_MEMBER (pci_bridge_device::interrupt_pin_w)
{
	logerror("%s: interrupt_pin_w %02x\n", tag(), data);
}

READ16_MEMBER (pci_bridge_device::bridge_control_r)
{
	return bridge_control;
}

WRITE16_MEMBER(pci_bridge_device::bridge_control_w)
{
	COMBINE_DATA(&bridge_control);
	logerror("%s: bridge_control_w %04x\n", tag(), bridge_control);
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

	reset_all_mappings();
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
	return config_address & 0x80000000 ? root_config_read((config_address >> 16) & 0xff, (config_address >> 8) & 0xff, config_address & 0xfc, mem_mask) : 0xffffffff;
}

WRITE32_MEMBER(pci_host_device::config_data_w)
{
	if(config_address & 0x80000000)
		root_config_write((config_address >> 16) & 0xff, (config_address >> 8) & 0xff, config_address & 0xfc, data, mem_mask);
}

UINT32 pci_host_device::root_config_read(UINT8 bus, UINT8 device, UINT16 reg, UINT32 mem_mask)
{
	if(bus == 0x00)
		return do_config_read(bus, device, reg, mem_mask);

	return propagate_config_read(bus, device, reg, mem_mask);
}

void pci_host_device::root_config_write(UINT8 bus, UINT8 device, UINT16 reg, UINT32 data, UINT32 mem_mask)
{
	if(bus == 0x00)
		do_config_write(bus, device, reg, data, mem_mask);

	else
		propagate_config_write(bus, device, reg, data, mem_mask);
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
