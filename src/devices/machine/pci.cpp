// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "pci.h"

DEFINE_DEVICE_TYPE(PCI_ROOT,   pci_root_device,   "pci_root",   "PCI virtual root")
DEFINE_DEVICE_TYPE(PCI_BRIDGE, pci_bridge_device, "pci_bridge", "PCI-PCI Bridge")


void pci_device::config_map(address_map &map)
{
	map(0x00, 0x01).r(FUNC(pci_device::vendor_r));
	map(0x02, 0x03).r(FUNC(pci_device::device_r));
	map(0x04, 0x05).rw(FUNC(pci_device::command_r), FUNC(pci_device::command_w));
	map(0x06, 0x07).r(FUNC(pci_device::status_r));
	map(0x08, 0x0b).r(FUNC(pci_device::class_rev_r));
	map(0x0c, 0x0c).r(FUNC(pci_device::cache_line_size_r));
	map(0x0d, 0x0d).r(FUNC(pci_device::latency_timer_r));
	map(0x0e, 0x0e).r(FUNC(pci_device::header_type_r));
	map(0x0f, 0x0f).r(FUNC(pci_device::bist_r));
	map(0x0c, 0x0f).nopw();
	map(0x10, 0x27).rw(FUNC(pci_device::address_base_r), FUNC(pci_device::address_base_w));
	// Cardbus CIS pointer at 28
	map(0x2c, 0x2d).r(FUNC(pci_device::subvendor_r));
	map(0x2e, 0x2f).r(FUNC(pci_device::subsystem_r));
	map(0x2c, 0x2f).nopw();
	map(0x30, 0x33).rw(FUNC(pci_device::expansion_base_r), FUNC(pci_device::expansion_base_w));
	map(0x34, 0x34).r(FUNC(pci_device::capptr_r));
	map(0x3c, 0x3c).rw(FUNC(pci_device::interrupt_line_r), FUNC(pci_device::interrupt_line_w));
	map(0x3d, 0x3d).rw(FUNC(pci_device::interrupt_pin_r), FUNC(pci_device::interrupt_pin_w));
}

void pci_bridge_device::config_map(address_map &map)
{
	map(0x00, 0x01).r(FUNC(pci_bridge_device::vendor_r));
	map(0x02, 0x03).r(FUNC(pci_bridge_device::device_r));
	map(0x04, 0x05).rw(FUNC(pci_bridge_device::command_r), FUNC(pci_bridge_device::command_w));
	map(0x06, 0x07).r(FUNC(pci_bridge_device::status_r));
	map(0x08, 0x0b).r(FUNC(pci_bridge_device::class_rev_r));
	map(0x0c, 0x0c).r(FUNC(pci_bridge_device::cache_line_size_r));
	map(0x0d, 0x0d).r(FUNC(pci_bridge_device::latency_timer_r));
	map(0x0e, 0x0e).r(FUNC(pci_bridge_device::header_type_r));
	map(0x0f, 0x0f).r(FUNC(pci_bridge_device::bist_r));
	map(0x10, 0x17).rw(FUNC(pci_bridge_device::b_address_base_r), FUNC(pci_bridge_device::b_address_base_w));
	map(0x18, 0x18).rw(FUNC(pci_bridge_device::primary_bus_r), FUNC(pci_bridge_device::primary_bus_w));
	map(0x19, 0x19).rw(FUNC(pci_bridge_device::secondary_bus_r), FUNC(pci_bridge_device::secondary_bus_w));
	map(0x1a, 0x1a).rw(FUNC(pci_bridge_device::subordinate_bus_r), FUNC(pci_bridge_device::subordinate_bus_w));
	map(0x1b, 0x1b).rw(FUNC(pci_bridge_device::secondary_latency_r), FUNC(pci_bridge_device::secondary_latency_w));
	map(0x1c, 0x1c).rw(FUNC(pci_bridge_device::iobase_r), FUNC(pci_bridge_device::iobase_w));
	map(0x1d, 0x1d).rw(FUNC(pci_bridge_device::iolimit_r), FUNC(pci_bridge_device::iolimit_w));
	map(0x1e, 0x1f).rw(FUNC(pci_bridge_device::secondary_status_r), FUNC(pci_bridge_device::secondary_status_w));
	map(0x20, 0x21).rw(FUNC(pci_bridge_device::memory_base_r), FUNC(pci_bridge_device::memory_base_w));
	map(0x22, 0x23).rw(FUNC(pci_bridge_device::memory_limit_r), FUNC(pci_bridge_device::memory_limit_w));
	map(0x24, 0x25).rw(FUNC(pci_bridge_device::prefetch_base_r), FUNC(pci_bridge_device::prefetch_base_w));
	map(0x26, 0x27).rw(FUNC(pci_bridge_device::prefetch_limit_r), FUNC(pci_bridge_device::prefetch_limit_w));
	map(0x28, 0x2b).rw(FUNC(pci_bridge_device::prefetch_baseu_r), FUNC(pci_bridge_device::prefetch_baseu_w));
	map(0x2c, 0x2f).rw(FUNC(pci_bridge_device::prefetch_limitu_r), FUNC(pci_bridge_device::prefetch_limitu_w));
	map(0x30, 0x31).rw(FUNC(pci_bridge_device::iobaseu_r), FUNC(pci_bridge_device::iobaseu_w));
	map(0x32, 0x33).rw(FUNC(pci_bridge_device::iolimitu_r), FUNC(pci_bridge_device::iolimitu_w));
	map(0x34, 0x34).r(FUNC(pci_bridge_device::capptr_r));
	map(0x38, 0x3b).rw(FUNC(pci_bridge_device::expansion_base_r), FUNC(pci_bridge_device::expansion_base_w));
	map(0x3c, 0x3c).rw(FUNC(pci_bridge_device::interrupt_line_r), FUNC(pci_bridge_device::interrupt_line_w));
	map(0x3d, 0x3d).rw(FUNC(pci_bridge_device::interrupt_pin_r), FUNC(pci_bridge_device::interrupt_pin_w));
	map(0x3e, 0x3f).rw(FUNC(pci_bridge_device::bridge_control_r), FUNC(pci_bridge_device::bridge_control_w));
}

pci_device::pci_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_region(*this, DEVICE_SELF)
{
	main_id = 0xffffffff;
	revision = 0x00;
	pclass = 0xffffff;
	subsystem_id = 0xffffffff;
	is_multifunction_device = false;
	intr_pin = 0x0;
	intr_line = 0xff;
}

void pci_device::set_ids(uint32_t _main_id, uint8_t _revision, uint32_t _pclass, uint32_t _subsystem_id)
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

	for (int i = 0; i < ARRAY_LENGTH(bank_infos); i++) {
		save_item(NAME(bank_infos[i].adr), i);
	}
	save_item(NAME(command));
	save_item(NAME(command_mask));
	save_item(NAME(status));
	save_item(NAME(intr_line));
	save_item(NAME(intr_pin));
}

void pci_device::device_reset()
{
}

uint32_t pci_device::unmapped_r(offs_t offset, uint32_t mem_mask, int bank)
{
	logerror("%s: unmapped read from %08x & %08x (%s)\n", machine().describe_context(), offset*4, mem_mask, bank_infos[bank].map.name());
	return 0;
}

void pci_device::unmapped_w(offs_t offset, uint32_t data, uint32_t mem_mask, int bank)
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
		logerror("write to address base (%d, %08x) not linked to any bank\n", offset, data);
		return;
	}

	int bid = bank_reg_infos[offset].bank;
	if(bank_reg_infos[offset].hi)
		bank_infos[bid].adr = (bank_infos[bid].adr & 0xffffffff) | (uint64_t(data) << 32);
	else {
		bank_infos[bid].adr = (bank_infos[bid].adr & 0xffffffff00000000U) | data;
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
	uint16_t old = command;

	mem_mask &= command_mask;
	COMBINE_DATA(&command);
	logerror("command = %04x\n", command);
	if ((old ^ command) & 3)
		remap_cb();
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

READ8_MEMBER(pci_device::interrupt_line_r)
{
	logerror("interrupt_line_r = %02x\n", intr_line);
	return intr_line;
}

WRITE8_MEMBER(pci_device::interrupt_line_w)
{
	COMBINE_DATA(&intr_line);
	logerror("interrupt_line_w %02x\n", data);
}

READ8_MEMBER(pci_device::interrupt_pin_r)
{
	logerror("interrupt_pin_r = %02x\n", intr_pin);
	return intr_pin;
}

WRITE8_MEMBER(pci_device::interrupt_pin_w)
{
	COMBINE_DATA(&intr_pin);
	logerror("interrupt_pin_w = %02x\n", data);
}

void pci_device::set_remap_cb(mapper_cb _remap_cb)
{
	remap_cb = _remap_cb;
}

void pci_device::reset_all_mappings()
{
}

void pci_device::map_device(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	for(int i=0; i<bank_count; i++) {
		bank_info &bi = bank_infos[i];
		if(uint32_t(bi.adr) >= 0xfffffffc)
			continue;
		if (bi.flags & M_IO) {
			if (~command & 1)
				continue;
		} else {
			if (~command & 2)
				continue;
		}
		if(!bi.size || (bi.flags & M_DISABLED))
			continue;

		address_space *space;
		uint64_t start = bi.adr & ~(bi.size - 1);

		if(bi.flags & M_IO) {
			space = io_space;
			start += io_offset;
		} else {
			space = memory_space;
			start += memory_offset;
		}
		uint64_t end = start + bi.size-1;
		switch(i) {
		case 0: space->install_readwrite_handler(start, end, read32_delegate(*this, FUNC(pci_device::unmapped0_r)), write32_delegate(*this, FUNC(pci_device::unmapped0_w))); break;
		case 1: space->install_readwrite_handler(start, end, read32_delegate(*this, FUNC(pci_device::unmapped1_r)), write32_delegate(*this, FUNC(pci_device::unmapped1_w))); break;
		case 2: space->install_readwrite_handler(start, end, read32_delegate(*this, FUNC(pci_device::unmapped2_r)), write32_delegate(*this, FUNC(pci_device::unmapped2_w))); break;
		case 3: space->install_readwrite_handler(start, end, read32_delegate(*this, FUNC(pci_device::unmapped3_r)), write32_delegate(*this, FUNC(pci_device::unmapped3_w))); break;
		case 4: space->install_readwrite_handler(start, end, read32_delegate(*this, FUNC(pci_device::unmapped4_r)), write32_delegate(*this, FUNC(pci_device::unmapped4_w))); break;
		case 5: space->install_readwrite_handler(start, end, read32_delegate(*this, FUNC(pci_device::unmapped5_r)), write32_delegate(*this, FUNC(pci_device::unmapped5_w))); break;
		}

		space->install_device_delegate(start, end, *bi.device, bi.map);
		logerror("map %s at %0*x-%0*x\n", bi.map.name(), bi.flags & M_IO ? 4 : 8, uint32_t(start), bi.flags & M_IO ? 4 : 8, uint32_t(end));
	}

	map_extra(memory_window_start, memory_window_end, memory_offset, memory_space,
				io_window_start, io_window_end, io_offset, io_space);

	if(expansion_rom_base & 1) {
		logerror("map expansion rom at %08x-%08x\n", expansion_rom_base & ~1, (expansion_rom_base & ~1) + expansion_rom_size - 1);
		uint32_t start = (expansion_rom_base & ~1) + memory_offset;
		uint32_t end = start + expansion_rom_size - 1;
		if(end > memory_window_end)
			end = memory_window_end;
		memory_space->install_rom(start, end, (void *)expansion_rom);
	}
}

void pci_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
}

void pci_device::map_config(uint8_t device, address_space *config_space)
{
	config_space->install_device(device << 12, (device << 12) | 0xfff, *this, &pci_device::config_map);
}

void pci_device::skip_map_regs(int count)
{
	bank_reg_count += count;
	assert(bank_reg_count <= 6);
}

void pci_device::add_map(uint64_t size, int flags, const address_map_constructor &map, device_t *relative_to)
{
	assert(bank_count < 6);
	assert((size & 3) == 0);
	int bid = bank_count++;
	bank_infos[bid].map = map;
	bank_infos[bid].device = relative_to ? relative_to : this;
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

void pci_device::add_rom(const uint8_t *rom, uint32_t size)
{
	expansion_rom = rom;
	expansion_rom_size = size;
	logerror("Device %s (%s) has 0x%x bytes of expansion rom\n", tag(), name(), size);
}

void pci_device::add_rom_from_region()
{
	add_rom(m_region->base(), m_region->bytes());
}

void pci_device::set_map_address(int id, uint64_t adr)
{
	bank_infos[id].adr = adr;
	remap_cb();
}

void pci_device::set_map_size(int id, uint64_t size)
{
	bank_infos[id].size = size;
	remap_cb();
}

void pci_device::set_map_flags(int id, int flags)
{
	bank_infos[id].flags = flags;
	remap_cb();
}

agp_device::agp_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, type, tag, owner, clock)
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



pci_bridge_device::pci_bridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_bridge_device(mconfig, PCI_BRIDGE, tag, owner, clock)
{
}

pci_bridge_device::pci_bridge_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, type, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, configure_space_config("configuration_space", ENDIANNESS_LITTLE, 32, 20)
{
}

READ8_MEMBER(pci_bridge_device::header_type_r)
{
	return 0x01;
}

device_memory_interface::space_config_vector pci_bridge_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PCI_CONFIG, &configure_space_config)
	};
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

	mapper_cb cf_cb(&pci_bridge_device::regenerate_config_mapping, this);

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

void pci_bridge_device::map_device(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
									uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	const int count = int(all_devices.size()) - 1;

	for(int i = count; i >= 0; i--)
		if (all_devices[i] != this)
			if (all_devices[i]->map_first())
				all_devices[i]->map_device(memory_window_start, memory_window_end, memory_offset, memory_space,
											io_window_start, io_window_end, io_offset, io_space);
	for(int i = count; i>=0; i--)
		if(all_devices[i] != this)
			if (!all_devices[i]->map_first())
				all_devices[i]->map_device(memory_window_start, memory_window_end, memory_offset, memory_space,
					io_window_start, io_window_end, io_offset, io_space);

	map_extra(memory_window_start, memory_window_end, memory_offset, memory_space,
				io_window_start, io_window_end, io_offset, io_space);
}


void pci_bridge_device::regenerate_config_mapping()
{
	address_space *config_space = &space(AS_PCI_CONFIG);
	config_space->unmap_readwrite(0x00000, 0xfffff);
	for(int i=0; i<32*8; i++)
		if(sub_devices[i])
			sub_devices[i]->map_config(i, config_space);
}

uint32_t pci_bridge_device::do_config_read(uint8_t bus, uint8_t device, uint16_t reg, uint32_t mem_mask)
{
	if(sub_devices[device]) {
		uint32_t data = space(AS_PCI_CONFIG).read_dword((device << 12) | reg, mem_mask);
		logerror("config_read %02x:%02x.%x:%02x %08x @ %08x\n", bus, device >> 3, device & 7, reg, data, mem_mask);
		return data;
	} else
		return 0xffffffff;
}

uint32_t pci_bridge_device::propagate_config_read(uint8_t bus, uint8_t device, uint16_t reg, uint32_t mem_mask)
{
	uint32_t data = 0xffffffff;
	for(unsigned int i=0; i != all_bridges.size(); i++)
		data &= all_bridges[i]->config_read(bus, device, reg, mem_mask);
	return data;
}

uint32_t pci_bridge_device::config_read(uint8_t bus, uint8_t device, uint16_t reg, uint32_t mem_mask)
{
	if(bus == secondary_bus)
		return do_config_read(bus, device, reg, mem_mask);

	if(bus > secondary_bus && bus <= subordinate_bus)
		return propagate_config_read(bus, device, reg, mem_mask);

	return 0xffffffff;
}

void pci_bridge_device::do_config_write(uint8_t bus, uint8_t device, uint16_t reg, uint32_t data, uint32_t mem_mask)
{
	if(sub_devices[device]) {
		space(AS_PCI_CONFIG).write_dword((device << 12) | reg, data, mem_mask);
		logerror("config_write %02x:%02x.%x:%02x %08x @ %08x\n", bus, device >> 3, device & 7, reg, data, mem_mask);
	}
}

void pci_bridge_device::propagate_config_write(uint8_t bus, uint8_t device, uint16_t reg, uint32_t data, uint32_t mem_mask)
{
	for(unsigned int i=0; i != all_bridges.size(); i++)
		all_bridges[i]->config_write(bus, device, reg, data, mem_mask);
}

void pci_bridge_device::config_write(uint8_t bus, uint8_t device, uint16_t reg, uint32_t data, uint32_t mem_mask)
{
	if(bus == secondary_bus)
		do_config_write(bus, device, reg, data, mem_mask);

	else if(bus > secondary_bus && bus <= subordinate_bus)
		propagate_config_write(bus, device, reg, data, mem_mask);
}

READ32_MEMBER (pci_bridge_device::b_address_base_r)
{
	logerror("b_address_base_r %d\n", offset);
	return 0xffffffff;
}

WRITE32_MEMBER(pci_bridge_device::b_address_base_w)
{
	logerror("b_address_base_w %d, %08x\n", offset, data);
}

READ8_MEMBER  (pci_bridge_device::primary_bus_r)
{
	logerror("primary_bus_r\n");
	return primary_bus;
}

WRITE8_MEMBER (pci_bridge_device::primary_bus_w)
{
	primary_bus = data;
	logerror("primary_bus_w %02x\n", data);
}

READ8_MEMBER  (pci_bridge_device::secondary_bus_r)
{
	logerror("secondary_bus_r\n");
	return secondary_bus;
}

WRITE8_MEMBER (pci_bridge_device::secondary_bus_w)
{
	secondary_bus = data;
	logerror("secondary_bus_w %02x\n", data);
}

READ8_MEMBER  (pci_bridge_device::subordinate_bus_r)
{
	logerror("subordinate_bus_r\n");
	return subordinate_bus;
}

WRITE8_MEMBER (pci_bridge_device::subordinate_bus_w)
{
	subordinate_bus = data;
	logerror("subordinate_bus_w %02x\n", data);
}

READ8_MEMBER  (pci_bridge_device::secondary_latency_r)
{
	logerror("secondary_latency_r\n");
	return 0xff;
}

WRITE8_MEMBER (pci_bridge_device::secondary_latency_w)
{
	logerror("secondary_latency_w %02x\n", data);
}

READ8_MEMBER  (pci_bridge_device::iobase_r)
{
	return iobase;
}

WRITE8_MEMBER (pci_bridge_device::iobase_w)
{
	iobase = data;
	logerror("iobase_w %02x\n", data);
}

READ8_MEMBER  (pci_bridge_device::iolimit_r)
{
	return iolimit;
}

WRITE8_MEMBER (pci_bridge_device::iolimit_w)
{
	iolimit = data;
	logerror("iolimit_w %02x\n", data);
}

READ16_MEMBER (pci_bridge_device::secondary_status_r)
{
	logerror("secondary_status_r\n");
	return 0xffff;
}

WRITE16_MEMBER(pci_bridge_device::secondary_status_w)
{
	logerror("secondary_status_w %04x\n", data);
}

READ16_MEMBER (pci_bridge_device::memory_base_r)
{
	return memory_base;
}

WRITE16_MEMBER(pci_bridge_device::memory_base_w)
{
	COMBINE_DATA(&memory_base);
	logerror("memory_base_w %04x\n", memory_base);
}

READ16_MEMBER (pci_bridge_device::memory_limit_r)
{
	return memory_limit;
}

WRITE16_MEMBER(pci_bridge_device::memory_limit_w)
{
	COMBINE_DATA(&memory_limit);
	logerror("memory_limit_w %04x\n", memory_limit);
}

READ16_MEMBER (pci_bridge_device::prefetch_base_r)
{
	return prefetch_base;
}

WRITE16_MEMBER(pci_bridge_device::prefetch_base_w)
{
	COMBINE_DATA(&prefetch_base);
	logerror("prefetch_base_w %04x\n", prefetch_base);
}

READ16_MEMBER (pci_bridge_device::prefetch_limit_r)
{
	return prefetch_limit;
}

WRITE16_MEMBER(pci_bridge_device::prefetch_limit_w)
{
	COMBINE_DATA(&prefetch_limit);
	logerror("prefetch_limit_w %04x\n", prefetch_limit);
}

READ32_MEMBER (pci_bridge_device::prefetch_baseu_r)
{
	return prefetch_baseu;
}

WRITE32_MEMBER(pci_bridge_device::prefetch_baseu_w)
{
	COMBINE_DATA(&prefetch_baseu);
	logerror("prefetch_baseu_w %08x\n", prefetch_baseu);
}

READ32_MEMBER (pci_bridge_device::prefetch_limitu_r)
{
	return prefetch_limitu;
}

WRITE32_MEMBER(pci_bridge_device::prefetch_limitu_w)
{
	COMBINE_DATA(&prefetch_limitu);
	logerror("prefetch_limitu_w %08x\n", prefetch_limitu);
}

READ16_MEMBER (pci_bridge_device::iobaseu_r)
{
	return iobaseu;
}

WRITE16_MEMBER(pci_bridge_device::iobaseu_w)
{
	COMBINE_DATA(&iobaseu);
	logerror("iobaseu_w %04x\n", iobaseu);
}

READ16_MEMBER (pci_bridge_device::iolimitu_r)
{
	return iolimitu;
}

WRITE16_MEMBER(pci_bridge_device::iolimitu_w)
{
	COMBINE_DATA(&iolimitu);
	logerror("iolimitu_w %04x\n", iolimitu);
}

READ16_MEMBER (pci_bridge_device::bridge_control_r)
{
	return bridge_control;
}

WRITE16_MEMBER(pci_bridge_device::bridge_control_w)
{
	COMBINE_DATA(&bridge_control);
	logerror("bridge_control_w %04x\n", bridge_control);
}


agp_bridge_device::agp_bridge_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: pci_bridge_device(mconfig, type, tag, owner, clock)
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



void pci_host_device::io_configuration_access_map(address_map &map)
{
	map(0xcf8, 0xcfb).rw(FUNC(pci_host_device::config_address_r), FUNC(pci_host_device::config_address_w));
	map(0xcfc, 0xcff).rw(FUNC(pci_host_device::config_data_r), FUNC(pci_host_device::config_data_w));
}


pci_host_device::pci_host_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: pci_bridge_device(mconfig, type, tag, owner, clock)
{
}

device_t *pci_host_device::bus_root()
{
	return owner();
}

void pci_host_device::device_start()
{
	remap_cb = mapper_cb(&pci_host_device::regenerate_mapping, this);

	pci_bridge_device::device_start();

	memory_window_start = memory_window_end = memory_offset = 0;
	io_window_start = io_window_end = io_offset = 0;

	reset_all_mappings();

	save_item(NAME(config_address));

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

uint32_t pci_host_device::root_config_read(uint8_t bus, uint8_t device, uint16_t reg, uint32_t mem_mask)
{
	if(bus == 0x00)
		return do_config_read(bus, device, reg, mem_mask);

	return propagate_config_read(bus, device, reg, mem_mask);
}

void pci_host_device::root_config_write(uint8_t bus, uint8_t device, uint16_t reg, uint32_t data, uint32_t mem_mask)
{
	if(bus == 0x00)
		do_config_write(bus, device, reg, data, mem_mask);

	else
		propagate_config_write(bus, device, reg, data, mem_mask);
}


pci_root_device::pci_root_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PCI_ROOT, tag, owner, clock)
{
}

void pci_root_device::device_start()
{
}

void pci_root_device::device_reset()
{
}
