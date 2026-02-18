// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#include "emu.h"
#include "vt82c598mvp.h"

#define LOG_MAP         (1U << 1)

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_warning

#define LOGMAP(...)    LOGMASKED(LOG_MAP,  __VA_ARGS__)


#include "logmacro.h"

DEFINE_DEVICE_TYPE(VT82C598MVP_HOST, vt82c598mvp_host_device, "vt82c598mvp_host", "Via VT82C598MVP \"Apollo MVP3\" Host")

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
	// TODO: special, uses register 84h to define the size
//  add_map(256 * 1024 * 1024, M_MEM | M_PREF, FUNC(vt82c598mvp_host_device::aperture_map));

	save_item(NAME(m_cache_control_1));
	save_item(NAME(m_cache_control_2));
	save_item(NAME(m_noncache_control));
	save_item(NAME(m_system_perf_control));
	save_item(NAME(m_noncache_region));
	save_item(NAME(m_dram_ma_map_type));
	save_item(NAME(m_bank_ending));
	save_item(NAME(m_dram_type));
	save_item(NAME(m_shadow_ram_control));
	save_item(NAME(m_dram_timing));
	save_item(NAME(m_dram_control));
	save_item(NAME(m_refresh_counter));
	save_item(NAME(m_dram_arbitration_control));
	save_item(NAME(m_sdram_control));
	save_item(NAME(m_dram_drive_strength));
	save_item(NAME(m_ecc_control));
	save_item(NAME(m_ecc_status));

	save_item(NAME(m_pci_buffer_control));
	save_item(NAME(m_pci_flow_control));
	save_item(NAME(m_pci_master_control));
	save_item(NAME(m_pci_arbitration));
	save_item(NAME(m_pmu_control));

	save_item(NAME(m_gart_control));
	save_item(NAME(m_aperture_size));
	save_item(NAME(m_ga_translation_table_base));

	save_item(NAME(m_agp_command));
	save_item(NAME(m_agp_control));
}

void vt82c598mvp_host_device::device_reset()
{
	pci_host_device::device_reset();

	command = 0x0006;
	status = 0x0290;

	m_cache_control_1 = m_cache_control_2 = m_noncache_control = m_system_perf_control = 0;
	m_dram_type = 0;
	std::fill(std::begin(m_shadow_ram_control), std::end(m_shadow_ram_control), 0);
	std::fill(std::begin(m_noncache_region), std::end(m_noncache_region), 0);
	std::fill(std::begin(m_dram_timing), std::end(m_dram_timing), 0);
	m_dram_control = 0;
	m_refresh_counter = 0;
	m_dram_arbitration_control = 0;
	m_sdram_control = 0;
	m_dram_drive_strength = 0;
	m_ecc_control = 0;
	m_ecc_status = 0;

	m_pci_buffer_control = 0;
	std::fill(std::begin(m_pci_flow_control), std::end(m_pci_flow_control), 0);
	std::fill(std::begin(m_pci_master_control), std::end(m_pci_master_control), 0);
	std::fill(std::begin(m_pci_arbitration), std::end(m_pci_arbitration), 0);
	m_pmu_control = 0;

	m_gart_control = 0;
	m_aperture_size = 0;
	m_ga_translation_table_base = 0;

	m_agp_command = 0;
	m_agp_control = 0;

	remap_cb();
}

u8 vt82c598mvp_host_device::capptr_r()
{
	return 0xa0;
}

void vt82c598mvp_host_device::config_map(address_map &map)
{
	pci_host_device::config_map(map);
	map(0x50, 0x50).lrw8(
		NAME([this] () { return m_cache_control_1; }),
		NAME([this] (offs_t offset, u8 data) {
			m_cache_control_1 = data & 0xf8;
			LOG("50h: Cache Control 1 %02x\n", data);
		})
	);
	// bits 7-6 are <reserved> but apparently r/w
	map(0x51, 0x51).lrw8(
		NAME([this] () { return m_cache_control_2; }),
		NAME([this] (offs_t offset, u8 data) {
			m_cache_control_2 = data & 0xeb;
			LOG("51h: Cache Control 2 %02x\n", data);
		})
	);
	map(0x52, 0x52).lrw8(
		NAME([this] () { return m_noncache_control; }),
		NAME([this] (offs_t offset, u8 data) {
			m_noncache_control = data & 0xf5;
			LOG("52h: Non-Cacheable Control 2 %02x\n", data);
		})
	);
	map(0x53, 0x53).lrw8(
		NAME([this] () { return m_system_perf_control; }),
		NAME([this] (offs_t offset, u8 data) {
			m_system_perf_control = data & 0xf0;
			LOG("53h: System Performance Control 2 %02x\n", data);
		})
	);
	map(0x54, 0x57).lrw16(
		NAME([this] (offs_t offset) { return m_noncache_region[offset]; }),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			COMBINE_DATA(&m_noncache_region[offset]);
			LOG("%02Xh: Non-Cacheable Region #%d %04x & %04x\n", (offset * 2) + 0x54, offset + 1, data, mem_mask);
		})
	);
	map(0x58, 0x59).lrw16(
		NAME([this] () { return m_dram_ma_map_type; }),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			COMBINE_DATA(&m_dram_ma_map_type);
			m_dram_ma_map_type &= 0xf0ff;
			LOG("58h: DRAM MA Map Type %04x & %04x\n", data, mem_mask);
		})
	);
	map(0x5a, 0x5f).lrw8(
		NAME([this] (offs_t offset) { return m_bank_ending[offset]; }),
		NAME([this] (offs_t offset, u8 data) {
			m_bank_ending[offset] = data;
			LOG("%02Xh: DRAM Row Ending Address bank %d %02x (%08x)\n", offset + 0x5a, offset, data, data << 23);
		})
	);
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
			LOG("%02Xh: Shadow RAM Control %d %02x\n", offset + 0x61, offset + 1, data);
			remap_cb();
		})
	);
	map(0x64, 0x66).lrw8(
		NAME([this] (offs_t offset) { return m_dram_timing[offset]; }),
		NAME([this] (offs_t offset, u8 data) {
			// NOTE: bit 0 <reserved> with FPG / EDO
			m_dram_timing[offset] = data;
			LOG("%02Xh: DRAM Timing (Banks %d,%d) %02x\n", offset + 0x64, offset * 2, offset * 2 + 1, data);
		})
	);
	map(0x68, 0x68).lrw8(
		NAME([this] () {
			return m_dram_control;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_dram_control = data & 0xef;
			LOG("68h: DRAM Control %02x\n", data);
		})
	);
	// 0x69 DRAM Clock Select
	// x--- ---- DRAM Operating Frequency (0) CPU frequency (1) AGP frequency

	map(0x6a, 0x6a).lrw8(
		NAME([this] () {
			return m_refresh_counter;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_refresh_counter = data;
			LOG("6Ah: Refresh Counter %02x\n", data);
		})
	);
	map(0x6b, 0x6b).lrw8(
		NAME([this] () { return m_dram_arbitration_control; }),
		NAME([this] (offs_t offset, u8 data) {
			m_dram_arbitration_control = data & 0xc1;
			LOG("6Bh: DRAM Arbitration Control %02x\n", data);
		})
	);
	map(0x6c, 0x6c).lrw8(
		NAME([this] () { return m_sdram_control; }),
		NAME([this] (offs_t offset, u8 data) {
			m_sdram_control = data & 0x7f;
			LOG("6Ch: SDRAM Control %02x\n", data);
		})
	);
	map(0x6d, 0x6d).lrw8(
		NAME([this] () { return m_dram_drive_strength; }),
		NAME([this] (offs_t offset, u8 data) {
			m_dram_drive_strength = data & 0x7f;
			LOG("6Dh: DRAM Drive Strength %02x\n", data);
		})
	);
	map(0x6e, 0x6e).lrw8(
		NAME([this] () { return m_ecc_control; }),
		NAME([this] (offs_t offset, u8 data) {
			m_ecc_control = data & 0xbf;
			LOG("6Eh: ECC Control %02x\n", data);
		})
	);
	map(0x6f, 0x6f).lrw8(
		NAME([this] () { return m_ecc_status; }),
		NAME([this] (offs_t offset, u8 data) {
			if (BIT(data, 7))
				m_ecc_status &= ~(1 << 7);
			if (BIT(data, 3))
				m_ecc_status &= ~(1 << 3);
			// TODO: is this section write clear too?
			const u8 dram_error = data & 0x77;
			m_ecc_status &= ~(0x77);
			m_ecc_status |= (dram_error & 0x77);
			// logging intentionally ignored
		})
	);
	map(0x70, 0x70).lrw8(
		NAME([this] () { return m_pci_buffer_control; }),
		NAME([this] (offs_t offset, u8 data) {
			m_pci_buffer_control = data & 0xf6;
			LOG("70h: PCI Buffer Control %02x\n", data);
		})
	);
	map(0x71, 0x71).lrw8(
		NAME([this] () { return m_pci_flow_control[0]; }),
		NAME([this] (offs_t offset, u8 data) {
			m_pci_flow_control[0] = data & 0xdf;
			LOG("71h: CPU to PCI Flow Control 1 %02x\n", data);
		})
	);
	map(0x72, 0x72).lrw8(
		NAME([this] () { return m_pci_flow_control[1]; }),
		NAME([this] (offs_t offset, u8 data) {
			m_pci_flow_control[1] = data & 0xfe;
			LOG("72h: CPU to PCI Flow Control 2 %02x\n", data);
		})
	);
	map(0x73, 0x73).lrw8(
		NAME([this] () { return m_pci_master_control[0]; }),
		NAME([this] (offs_t offset, u8 data) {
			m_pci_master_control[0] = data & 0x7f;
			LOG("73h: PCI Master Control 1 %02x\n", data);
		})
	);
	map(0x74, 0x74).lrw8(
		NAME([this] () { return m_pci_master_control[1]; }),
		NAME([this] (offs_t offset, u8 data) {
			m_pci_master_control[1] = data & 0xc0;
			LOG("74h: PCI Master Control 2 %02x\n", data);
		})
	);
	map(0x75, 0x75).lrw8(
		NAME([this] () { return m_pci_arbitration[0]; }),
		NAME([this] (offs_t offset, u8 data) {
			m_pci_arbitration[0] = data;
			LOG("75h: PCI Arbitration 1 %02x\n", data);
		})
	);
	map(0x76, 0x76).lrw8(
		NAME([this] () { return m_pci_arbitration[1]; }),
		NAME([this] (offs_t offset, u8 data) {
			m_pci_arbitration[1] = data & 0xf0;
			LOG("76h: PCI Arbitration 2 %02x\n", data);
		})
	);
	// 0x77 Chip Test Mode
	map(0x78, 0x78).lrw8(
		NAME([this] () { return m_pmu_control; }),
		NAME([this] (offs_t offset, u8 data) {
			m_pmu_control = data & 0xfb;
			LOG("78h: PMU Control %02x\n", data);
			remap_cb();
		})
	);
	// 0x7e ~ 0x7f DLL Test Mode

	map(0x80, 0x83).lrw32(
		NAME([this] () { return m_gart_control; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			// NOTE: only lower port has a write meaning
			// 15-8 are for test mode status (r/o)
			// 31-16 are <reserved>
			if (ACCESSING_BITS_0_7)
				m_gart_control = data & 0x8f;
			LOG("80h: GART/TLB Control %08x & %08x\n", data, mem_mask);
		})
	);
	map(0x84, 0x84).lrw8(
		NAME([this] () { return m_aperture_size; }),
		NAME([this] (offs_t offset, u8 data) {
			m_aperture_size = data;
			LOG("84h: Graphics Aperture Size %08x\n", data);
			// TODO: BAR setup here
		})
	);
	map(0x88, 0x8b).lrw32(
		NAME([this] () { return m_ga_translation_table_base; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			COMBINE_DATA(&m_ga_translation_table_base);
			m_ga_translation_table_base &= 0xffffff07;
			LOG("88h: GA Translation Table Base %08x & %08x\n", data, mem_mask);
		})
	);

	// AGP Control (version 1.0)
	// bits 23-16 AGP v1.0
	// bits 15-8 0x00 NEXT_PTR (NULL terminator)
	// bits 7-0 CAP_ID (0x02 for AGP)
	map(0xa0, 0xa3).lr32(NAME([] () { return 0x0010'0002; }));
	// AGP Status
	// bits 31-24 Maximum AGP Requests (8)
	// bit 9 supports SideBand Addressing
	// bit 1 2X Rate Supported (config defined, below)
	// bit 0 1X Rate Supported
	map(0xa4, 0xa7).lr32(NAME([this] () {
		return (7 << 24) | (1 << 9) | (BIT(m_agp_control, 3) << 1) | 1;
	}));
	// AGP Command
	map(0xa8, 0xab).lrw32(
		NAME([this] () { return m_agp_command; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			COMBINE_DATA(&m_agp_command);
			// knock off any bit that doesn't belong to host
			// (in particular request depth bits 31-24)
			m_agp_command &= 0x0000'0303;
			LOG("A8h: AGP Command %08x & %08x\n", data, mem_mask);
		})
	);
	map(0xac, 0xac).lrw8(
		NAME([this] () { return m_agp_control; }),
		NAME([this] (offs_t offset, u8 data) {
			m_agp_control = data & 0xf;
			LOG("ACh: AGP Control %02x\n", data);
		})
	);
	// 0xfc ~ 0xff <reserved>
}

void vt82c598mvp_host_device::map_shadowram(address_space *memory_space, offs_t start_offs, offs_t end_offs, u8 setting)
{
	LOGMAP("- 0x%08x-0x%08x ", start_offs, end_offs);

	switch(setting)
	{
		case 0:
			LOGMAP("shadow RAM off\n");
			break;
		case 1:
			LOGMAP("shadow RAM w/o\n");
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
	// if (BIT(m_pmu_control, 7))

	const u8 memory_hole = m_shadow_ram_control[2] & 0xc;

	if(memory_hole == 0x4)
		memory_space->install_ram      (0x00000000, 0x0007ffff, &m_ram[0x00000000/4]);
	else
		memory_space->install_ram      (0x00000000, 0x0009ffff, &m_ram[0x00000000/4]);

	// TODO: SMI mapping control (bits 1-0 of shadow ram control [2])

	// upper memory hole settings
	memory_space->install_ram          (0x00100000, 0x00dfffff, &m_ram[0x00100000/4]);
	switch(memory_hole)
	{
		// 15M-16M
		case 0x8:
			memory_space->install_ram  (0x00e00000, 0x00efffff, &m_ram[0x00e00000/4]);
			break;
		// 14M-16M
		case 0xc:
			break;
		default:
			memory_space->install_ram  (0x00e00000, 0x00ffffff, &m_ram[0x00e00000/4]);
			break;
	}

	memory_space->install_ram          (0x01000000, m_ram_size-1, &m_ram[0x01000000/4]);

	// Shadow RAM section
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

}

/*
 *
 * VT82C598MVP Bridge section
 *
 */

DEFINE_DEVICE_TYPE(VT82C598MVP_BRIDGE, vt82c598mvp_bridge_device, "vt82c598mvp_bridge", "Via VT82C598MVP \"Apollo MVP3\" PCI-to-PCI Bridge")

vt82c598mvp_bridge_device::vt82c598mvp_bridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_bridge_device(mconfig, VT82C598MVP_BRIDGE, tag, owner, clock)
//  , m_vga(*this, finder_base::DUMMY_TAG)
{
	set_ids_bridge(0x11068598, 0x00);
}

void vt82c598mvp_bridge_device::device_start()
{
	pci_bridge_device::device_start();

	save_item(NAME(m_pci2_flow_control));
	save_item(NAME(m_pci2_master_control));
}

void vt82c598mvp_bridge_device::device_reset()
{
	pci_bridge_device::device_reset();

	std::fill(std::begin(m_pci2_flow_control), std::end(m_pci2_flow_control), 0);
	m_pci2_master_control = 0;
}

void vt82c598mvp_bridge_device::config_map(address_map &map)
{
	pci_bridge_device::config_map(map);
	map(0x40, 0x40).lrw8(
		NAME([this] () { return m_pci2_flow_control[0]; }),
		NAME([this] (offs_t offset, u8 data) {
			m_pci2_flow_control[0] = data;
			LOG("40h: CPU-to-PCI#2 Flow Control 1 %02x\n", data);
		})
	);
	map(0x41, 0x41).lrw8(
		NAME([this] () { return m_pci2_flow_control[1]; }),
		NAME([this] (offs_t offset, u8 data) {
			if (BIT(data, 7))
				m_pci2_flow_control[1] &= ~(1 << 7);
			m_pci2_flow_control[1] &= ~0x7e;
			m_pci2_flow_control[1] |= (data & 0x7e);
			LOG("41h: CPU-to-PCI#2 Flow Control 2 %02x -> %02x\n", data, m_pci2_flow_control[1]);
		})
	);
	map(0x42, 0x42).lrw8(
		NAME([this] () { return m_pci2_master_control; }),
		NAME([this] (offs_t offset, u8 data) {
			m_pci2_master_control = data;
			LOG("42h: CPU-to-PCI#2 Master Control %02x\n", data);
		})
	);
}
