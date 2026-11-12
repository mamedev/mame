// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#include "emu.h"
#include "vt82c585vp.h"

#define LOG_MAP         (1U << 1)

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info

#define LOGMAP(...)    LOGMASKED(LOG_MAP,  __VA_ARGS__)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(VT82C585VP_HOST, vt82c585vp_host_device, "vt82c585vp_host", "VIA VT82C585VP \"Apollo VP\" Host")

vt82c585vp_host_device::vt82c585vp_host_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: pci_host_device(mconfig, type, tag, owner, clock)
	, m_host_cpu(*this, finder_base::DUMMY_TAG)
{
}

vt82c585vp_host_device::vt82c585vp_host_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: vt82c585vp_host_device(mconfig, VT82C585VP_HOST, tag, owner, clock)
{
	// TODO: cross check the VPX datasheet, figure out revision numbers between VP and VPX (same PCI device)
	set_ids_host(0x11060585, 0x00, 0x000000);
}


void vt82c585vp_host_device::device_start()
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
	save_item(NAME(m_uma_control));
	save_item(NAME(m_refresh_counter));
	save_item(NAME(m_dram_arbitration_control));
	save_item(NAME(m_sdram_control));
	save_item(NAME(m_dram_drive_strength));

	save_item(NAME(m_pci_buffer_control));
	save_item(NAME(m_pci_flow_control));
	save_item(NAME(m_pci_master_control));
	save_item(NAME(m_pci_arbitration));

	save_item(NAME(m_smiact));
}

void vt82c585vp_host_device::device_reset()
{
	pci_host_device::device_reset();

	// I/O space, Memory Space and Bus Master always high
	command = 0x0007;
	// parity error response/SERR# enable/fast back-to-back
	command_mask = 0x0340;
	// 66MHz Capable, Fast Back-to-Back, Medium DEVSEL#
	status = 0x02a0;

	m_cache_control_1 = m_cache_control_2 = m_system_perf_control = 0;
	m_noncache_control = 2;
	m_dram_type = 0;
	std::fill(std::begin(m_shadow_ram_control), std::end(m_shadow_ram_control), 0);
	std::fill(std::begin(m_noncache_region), std::end(m_noncache_region), 0);
	std::fill(std::begin(m_dram_timing), std::end(m_dram_timing), 0);
	std::fill(std::begin(m_bank_ending), std::end(m_bank_ending), 1);
	m_dram_control = 0;
	m_dram_ma_map_type = 0x0540;
	m_uma_control = 0;
	m_refresh_counter = 0;
	m_dram_arbitration_control = 0;
	m_sdram_control = 0;
	m_dram_drive_strength = 0;

	m_pci_buffer_control = 0;
	std::fill(std::begin(m_pci_flow_control), std::end(m_pci_flow_control), 0);
	std::fill(std::begin(m_pci_master_control), std::end(m_pci_master_control), 0);
	std::fill(std::begin(m_pci_arbitration), std::end(m_pci_arbitration), 0);

	m_smiact = 1;
	remap_cb();
}

// NOTE: comments indicates stuff that is different on later MVP3
void vt82c585vp_host_device::config_map(address_map &map)
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
			m_noncache_control = data & 0xff; // & 0xf5
			LOG("52h: Non-Cacheable Control %02x\n", data);
		})
	);
	// renamed register in MVP3
	map(0x53, 0x53).lrw8(
		NAME([this] () { return m_system_perf_control; }),
		NAME([this] (offs_t offset, u8 data) {
			m_system_perf_control = data & 0xf8; // & 0xf0
			LOG("53h: Misc Control %02x\n", data);
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
			m_dram_ma_map_type &= 0xe7ee; //& 0xf0ff;
			LOG("58h: DRAM Configuration %04x & %04x\n", data, mem_mask);
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
	// TODO: very different (4 registers to begin with)
	map(0x64, 0x67).lrw8(
		NAME([this] (offs_t offset) { return m_dram_timing[offset]; }),
		NAME([this] (offs_t offset, u8 data) {
			// NOTE: bit 0 <reserved> with FPG / EDO
			m_dram_timing[offset] = data;
			LOG("%02Xh: DRAM Timing (Banks %d,%d) %02x\n", offset + 0x64, offset * 2, offset * 2 + 1, data);
		})
	);
	// Not present on MVP3
	map(0x68, 0x69).lrw16(
		NAME([this] () {
			return m_uma_control;
		}),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			COMBINE_DATA(&m_uma_control);
			m_uma_control &= 0xfff7;
			LOG("68h: UMA Control %04x & %04x\n", data, mem_mask);
		})
	);
	// 0x69 <reserved> (No DRAM Clock Select)

	map(0x6a, 0x6a).lrw8(
		NAME([this] () {
			return m_refresh_counter;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_refresh_counter = data;
			LOG("6Ah: Refresh Counter %02x\n", data);
		})
	);
	// very different, laconically labeled as "Misc."
	// x--- ---- "CBR" CAS-before-RAS refresh
	// -x-- ---- Burst Refresh (4 times)
	map(0x6b, 0x6b).lrw8(
		NAME([this] () { return m_dram_arbitration_control; }),
		NAME([this] (offs_t offset, u8 data) {
			m_dram_arbitration_control = data & 0xc0;
			LOG("6Bh: Misc. %02x\n", data);
		})
	);
	// Present on revision ID >= 1.0
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
			m_dram_drive_strength = data; //& 0x7f;
			LOG("6Dh: DRAM Drive Strength %02x\n", data);
		})
	);
	//map(0x6e, 0x6f) <reserved> (a.k.a. No ECC control)
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
			m_pci_flow_control[1] = data & 0xfc; //& 0xfe;
			LOG("72h: CPU to PCI Flow Control 2 %02x\n", data);
		})
	);
	map(0x73, 0x73).lrw8(
		NAME([this] () { return m_pci_master_control[0]; }),
		NAME([this] (offs_t offset, u8 data) {
			m_pci_master_control[0] = data; //& 0x7f;
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
			m_pci_arbitration[0] = data & 0xcf;
			LOG("75h: PCI Arbitration %02x\n", data);
		})
	);
	// register present on revision >= 1.0 only
	// labeled "Extension", still does arbitration stuff anyway
	map(0x76, 0x76).lrw8(
		NAME([this] () { return m_pci_arbitration[1]; }),
		NAME([this] (offs_t offset, u8 data) {
			m_pci_arbitration[1] = data & 0xb1; //& 0xf0;
			LOG("76h: PCI Arbitration Extension %02x\n", data);
		})
	);
}

void vt82c585vp_host_device::map_shadowram(address_space *memory_space, offs_t start_offs, offs_t end_offs, u8 setting)
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

// SMIACT# is canonically active low
void vt82c585vp_host_device::smi_act_w(int state)
{
	if (state)
		m_smiact = 0;
	else
		m_smiact = 1;

//  if (m_smiact == 0)
//      machine().debug_break();
	remap_cb();
}

void vt82c585vp_host_device::map_extra(
	uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
	uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space
) {
	io_space->install_device(0, 0xffff,  *static_cast<pci_host_device *>(this), &pci_host_device::io_configuration_access_map);

	regenerate_config_mapping();

	const u8 memory_hole = m_shadow_ram_control[2] & 0xc;

	if(memory_hole == 0x4)
		memory_space->install_ram      (0x00000000, 0x0007ffff, &m_ram[0x00000000/4]);
	else
		memory_space->install_ram      (0x00000000, 0x0009ffff, &m_ram[0x00000000/4]);

	// TODO: different than MVP3 (hot579 and ice/calchase.cpp cares)
	//const u8 smi_mapping = m_shadow_ram_control[2] & 3;

	//if ((smi_mapping == smi_bank() && m_smiact == 0) || smi_mapping == 1)
	//{
	//	memory_space->install_ram      (0x000a0000, 0x000bffff, &m_ram[0x000a0000/4]);
	//}

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
