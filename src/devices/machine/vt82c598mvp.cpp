// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#include "emu.h"
#include "vt82c598mvp.h"

#define LOG_MAP         (1U << 1)

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_warning

#define LOGMAP(...)    LOGMASKED(LOG_MAP,  __VA_ARGS__)


#include "logmacro.h"

DEFINE_DEVICE_TYPE(VT82C598MVP_HOST, vt82c598mvp_host_device, "vt82c598mvp_host", "Via VT82C598MVP \"Apollo MVP3\"")

vt82c598mvp_host_device::vt82c598mvp_host_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_host_device(mconfig, VT82C598MVP_HOST, tag, owner, clock)
	, m_host_cpu(*this, finder_base::DUMMY_TAG)
{
}

void vt82c598mvp_host_device::device_start()
{
	pci_host_device::device_start();
	set_spaces(&m_host_cpu->space(AS_PROGRAM), &m_host_cpu->space(AS_IO));

	memory_window_start = 0;
	memory_window_end   = 0xffffffff;
	memory_offset       = 0;
	io_window_start = 0;
	io_window_end   = 0xffff;
	io_offset       = 0;

	m_ram.resize(m_ram_size/4);

	save_item(NAME(m_dram_type));
	save_item(NAME(m_shadow_ram_control));
}

void vt82c598mvp_host_device::device_reset()
{
	pci_host_device::device_reset();

	command = 0x0006;
	status = 0x0290;

	m_dram_type = 0;
	m_shadow_ram_control[0] = m_shadow_ram_control[1] = m_shadow_ram_control[2] = 0;

	remap_cb();
}

void vt82c598mvp_host_device::config_map(address_map &map)
{
	pci_host_device::config_map(map);
	// TODO: everything else

	map(0x60, 0x60).lrw8(
		NAME([this] () {
			return m_dram_type;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_dram_type = data & 0x3f;
			LOG("60h: DRAM Type %02x\n", data);
		})
	);
	map(0x61, 0x63).lrw8(
		NAME([this] (offs_t offset) {
			return m_shadow_ram_control[offset];
		}),
		NAME([this] (offs_t offset, u8 data){
			m_shadow_ram_control[offset] = data;
			LOG("%02xh: Shadow RAM Control %d %02x\n", offset + 0x61, offset + 1, data);
			remap_cb();
		})
	);
}

void vt82c598mvp_host_device::map_shadowram(address_space *memory_space, offs_t start_offs, offs_t end_offs, u8 setting)
{
	LOGMAP("- 0x%08x-0x%08x ", start_offs, end_offs);

	switch(setting)
	{
		case 0:
			LOGMAP("shadow RAM off\n");
			//memory_space->unmap_write(start_offs, end_offs);
			break;
		case 1:
			LOGMAP("shadow RAM w/o\n");
			//memory_space->install_rom(start_offs, end_offs, m_region->base() + bios_rom_offset);
			memory_space->install_writeonly(start_offs, end_offs, &m_ram[start_offs/4]);
			break;
		case 2:
			LOGMAP("shadow RAM r/o\n");
			memory_space->install_rom(start_offs, end_offs, &m_ram[start_offs/4]);
			break;
		case 3:
			LOGMAP("shadow RAM r/w\n");
			memory_space->install_ram(start_offs, end_offs, &m_ram[start_offs/4]);
			break;
	}
}


void vt82c598mvp_host_device::map_extra(
	uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
	uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space
) {
	io_space->install_device(0, 0xffff,  *static_cast<pci_host_device *>(this), &pci_host_device::io_configuration_access_map);

	regenerate_config_mapping();

	// TODO: config port for PCI Arbiter Disable at $22

	// TODO: two other Memory Hole settings, mutually exclusive
	const u8 memory_hole_lower = (m_shadow_ram_control[2] & 0xc) == 4;

	if(memory_hole_lower)
		memory_space->install_ram      (0x00000000, 0x0007ffff, &m_ram[0x00000000/4]);
	else
		memory_space->install_ram      (0x00000000, 0x0009ffff, &m_ram[0x00000000/4]);

	memory_space->install_ram      (0x00000000, 0x0009ffff, &m_ram[0x00000000/4]);

	int i;

	// handle both $c0000 / $d0000
	for (i = 0; i < 8; i++)
	{
		const offs_t start_offs = 0xc0000 + (i * 0x4000);
		const offs_t end_offs = start_offs + 0x3fff;
		const u8 reg = BIT(i, 2);
		const u8 shift = (i & 3) * 2;
		map_shadowram(memory_space, start_offs, end_offs, (m_shadow_ram_control[reg] >> shift) & 3);
	}

	map_shadowram(memory_space, 0xe0000, 0xeffff, (m_shadow_ram_control[2] >> 6) & 3);
	map_shadowram(memory_space, 0xf0000, 0xfffff, (m_shadow_ram_control[2] >> 4) & 3);

	memory_space->install_ram(0x00100000, m_ram_size - 1, &m_ram[0x00100000/4]);
}

