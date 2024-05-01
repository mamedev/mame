// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

    SiS630 host implementation (northbridge)

    TODO:
    - AGP and VGA interfaces;
    - Is ACPI declared here shared with LPC or a different one?
    \- shutms11 maps it to the exact same place (I/O $5000), may be interleaved?
    - HW trap control;
    - PCI-Hole;
    - Convert RAM to device;
    - Integrated VGA control;

**************************************************************************************************/

#include "emu.h"
#include "sis630_host.h"


#define LOG_IO     (1U << 1) // log PCI register accesses
#define LOG_TODO   (1U << 2) // log unimplemented registers
#define LOG_MAP    (1U << 3) // log full remaps
#define LOG_AGP    (1U << 4) // log AGP

#define VERBOSE (LOG_GENERAL | LOG_IO | LOG_TODO | LOG_AGP)
//#define LOG_OUTPUT_FUNC osd_printf_warning

#include "logmacro.h"

#define LOGIO(...)     LOGMASKED(LOG_IO,   __VA_ARGS__)
#define LOGMAP(...)    LOGMASKED(LOG_MAP,  __VA_ARGS__)
#define LOGTODO(...)   LOGMASKED(LOG_TODO, __VA_ARGS__)
#define LOGAGP(...)    LOGMASKED(LOG_AGP, __VA_ARGS__)

DEFINE_DEVICE_TYPE(SIS630_HOST, sis630_host_device, "sis630_host", "SiS 630 Host-to-PCI Bridge")

sis630_host_device::sis630_host_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_host_device(mconfig, SIS630_HOST, tag, owner, clock)
	, m_host_cpu(*this, finder_base::DUMMY_TAG)
{
}

void sis630_host_device::device_start()
{
	pci_host_device::device_start();

	memory_window_start = 0;
	memory_window_end   = 0xffffffff;
	memory_offset       = 0;
	io_window_start = 0;
	io_window_end   = 0xffff;
	io_offset       = 0;

	set_spaces(&m_host_cpu->space(AS_PROGRAM), &m_host_cpu->space(AS_IO));
	add_map(8*1024*1024, M_MEM, FUNC(sis630_host_device::memory_map));

	m_ram.resize(m_ram_size/4);
}

void sis630_host_device::device_reset()
{
	pci_host_device::device_reset();

	command = 0x0005;
	status = 0x0210;

	m_shadow_ram_ctrl = 0;
	m_vga_control = 0;
	std::fill(std::begin(m_agp_mailbox), std::end(m_agp_mailbox), 0);

	remap_cb();
}

void sis630_host_device::device_add_mconfig(machine_config &config)
{

}

void sis630_host_device::config_map(address_map &map)
{
	pci_host_device::config_map(map);
	// override header type, needed for actual IDE detection
	map(0x0e, 0x0e).lr8(NAME([] () { return 0x80; }));

	// override first BAR slot for gfx window base address
	map(0x10, 0x13).rw(FUNC(pci_device::address_base_r), FUNC(pci_device::address_base_w));

	map(0x34, 0x34).r(FUNC(sis630_host_device::capptr_r));

	// host & DRAM regs
//  map(0x50, 0x51) host interface control
//  map(0x52, 0x53) DRAM misc control 1 & 2
//  map(0x54, 0x55) DRAM timing control 1 & 2
//  map(0x56, 0x56) DRAM misc control 3
//  map(0x57, 0x57) SDRAM/VCM init control
//  map(0x58, 0x58) DRAM buffer slew rating
//  map(0x59, 0x5a) DRAM buffer strength and current rating
//  map(0x5b, 0x5b) PCI buffer strength and current rating
//  map(0x60, 0x62) DRAMx type register (x = 0, 1 or 2)
	map(0x63, 0x63).rw(FUNC(sis630_host_device::dram_status_r), FUNC(sis630_host_device::dram_status_w));
//  map(0x64, 0x64) FBC control register
//  map(0x65, 0x65) DIMM switch control
//  map(0x68, 0x69) ACPI I/O base
	map(0x6a, 0x6a).rw(FUNC(sis630_host_device::smram_r), FUNC(sis630_host_device::smram_w));
//  map(0x6b, 0x6b) self refresh command output timing control
//  map(0x6c, 0x6c) power management DRAM self refresh control

//  Shadow RAM & PCI-Hole area
	map(0x70, 0x73).rw(FUNC(sis630_host_device::shadow_ram_ctrl_r), FUNC(sis630_host_device::shadow_ram_ctrl_w));
	map(0x77, 0x77).rw(FUNC(sis630_host_device::pci_hole_r), FUNC(sis630_host_device::pci_hole_w));
//  map(0x78, 0x79) PCI-Hole #1 allocation
//  map(0x7a, 0x7b) PCI-Hole #2 allocation

//  HW Trap control
//  map(0x7c, 0x7c) VGA
//  map(0x7d, 0x7d) Southbridge
//  map(0x7e, 0x7f) Northbridge

//  Host Bridge & PCI arbiter characteristics
//  map(0x80, 0x80) Target bridge DRAM characteristics
//  map(0x81, 0x81) PCI discard timer for delay transaction
//  map(0x82, 0x82) PCI target bridge bus characteristics
//  map(0x83, 0x83) CPU to PCI characteristics
//  map(0x84, 0x85) PCI grant timer
//  map(0x86, 0x86) CPU idle timer for PCI
//  map(0x87, 0x87) Host bridge & PCI master priority timer
//  map(0x88, 0x89) PCI discard timer for PCI hold

//  Clock Control
//  map(0x8c, 0x8c) SDRCLK/SDWCLK
//  map(0x8d, 0x8d) SDWCLK
//  map(0x8e, 0x8e) CPU & SDRAM clock relationship
//  map(0x8f, 0x8f) FBCRCLK/FBCWCLK control

//  GART and page table regs
//  map(0x90, 0x93) GART base address
//  map(0x94, 0x94) Graphic window control
//  map(0x97, 0x97) Page table cache control
//  map(0x98, 0x98) Page table cache invalidation control

	// Integrated VGA control
	map(0x9c, 0x9c).rw(FUNC(sis630_host_device::vga_control_r), FUNC(sis630_host_device::vga_control_w));

//  AGP
//  map(0xa0, 0xa3) DRAM priority timer control
	map(0xa0, 0xa3).rw(FUNC(sis630_host_device::agp_priority_timer_r), FUNC(sis630_host_device::agp_priority_timer_w));
//  map(0xa4, 0xaf) General purpose register (generic mailboxes?)
	map(0xa4, 0xaf).rw(FUNC(sis630_host_device::agp_mailbox_r), FUNC(sis630_host_device::agp_mailbox_w));
//  map(0xc0, 0xc3) AGP capability identifier
	map(0xc0, 0xc3).r(FUNC(sis630_host_device::agp_id_r));
	map(0xc4, 0xc7).r(FUNC(sis630_host_device::agp_status_r));
	map(0xc8, 0xcb).rw(FUNC(sis630_host_device::agp_command_r), FUNC(sis630_host_device::agp_command_w));
}

// TODO: verify if we need these trampolines
void sis630_host_device::memory_map(address_map &map)
{
}

void sis630_host_device::map_shadowram(address_space *memory_space, uint32_t start_offs, uint32_t end_offs, bool read_enable, bool write_enable)
{
	LOGMAP("- 0x%08x-0x%08x ", start_offs, end_offs);

	switch(write_enable << 1 | read_enable)
	{
		case 0:
			LOGMAP("shadow RAM off\n");
			//memory_space->unmap_write(start_offs, end_offs);
			break;
		case 1:
			LOGMAP("shadow RAM r/o\n");
			memory_space->install_rom(start_offs, end_offs, &m_ram[start_offs/4]);
			break;
		case 2:
			LOGMAP("shadow RAM w/o\n");
			//memory_space->install_rom(start_offs, end_offs, m_region->base() + bios_rom_offset);
			memory_space->install_writeonly(start_offs, end_offs, &m_ram[start_offs/4]);
			break;
		case 3:
			LOGMAP("shadow RAM r/w\n");
			memory_space->install_ram(start_offs, end_offs, &m_ram[start_offs/4]);
			break;
	}
}


void sis630_host_device::map_extra(
	uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
	uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space
) {
	io_space->install_device(0, 0xffff,  *static_cast<pci_host_device *>(this), &pci_host_device::io_configuration_access_map);

	regenerate_config_mapping();

	memory_space->install_ram(0x00000000, 0x0009ffff, &m_ram[0x00000000/4]);
//  memory_space->install_ram(0x000a0000, 0x000bffff, &m_ram[0x000a0000/4]);

	LOGMAP("Host Remapping table (shadow: %08x smram: %02x):\n", m_shadow_ram_ctrl, m_smram);

	for (int i = 0; i < 12; i ++)
	{
		u32 start_offs = 0x000c0000 + i * 0x4000;
		u32 end_offs = start_offs + 0x3fff;

		map_shadowram(
			memory_space,
			start_offs, end_offs,
			bool(BIT(m_shadow_ram_ctrl, i)), bool(BIT(m_shadow_ram_ctrl, i + 16))
		);
	}

	map_shadowram(
		memory_space,
		0xf0000, 0xfffff,
		bool(BIT(m_shadow_ram_ctrl, 12)), bool(BIT(m_shadow_ram_ctrl, 28))
	);

	// System Management Memory Region handling
	// Potentially overrides VGA VRAM if on
	if (BIT(m_smram, 4))
	{
		u8 smram_config = m_smram >> 5;

		// POST checks for config 6 only, other settings aren't tested
		// TODO: setting 3 and 5 are undocumented, verify if mirror logic is correct
		if (smram_config == 3 || smram_config == 5)
			throw emu_fatalerror("SMRAM config = %d!", smram_config);

		const u32 host_addresses[8] = {
			0xe0000, 0xb0000, 0xe0000, 0xb0000,
			0xe0000, 0xe0000, 0xa0000, 0xa0000
		};
		const u32 smram_sizes[8] = {
			0x07fff, 0xffff, 0x7fff, 0xffff,
			0x07fff, 0x7fff, 0xffff, 0x1ffff
		};
		const u32 system_memory_addresses[8] = {
			0xe0000, 0xb0000, 0xa0000, 0xb0000,
			0xb0000, 0xb0000, 0xa0000, 0xa0000
		};
		const u32 host_address_start = host_addresses[smram_config];
		const u32 host_address_end = host_address_start + smram_sizes[smram_config];
		const u32 system_memory_address = system_memory_addresses[smram_config];
		LOGMAP("- SMRAM %02x relocation %08x-%08x to %08x\n"
			, m_smram
			, host_address_start
			, host_address_end
			, system_memory_address
		);
		memory_space->install_ram(host_address_start, host_address_end, &m_ram[system_memory_address/4]);
	}

	// TODO: shadow RAM bit 15?
	// Always on after POST, should give shared access to PCI cards on the bus,
	// BIOS mentions 8M of "shared memory", unknown how this works out.
	// TODO: undocumented shadow RAM configs bits 7 and 23 after POST IDE check on shutms11 (programmer errors?)

	memory_space->install_ram(0x00100000, m_ram_size - 1, &m_ram[0x00100000/4]);
}





/*
 *
 * I/O implemtation
 *
 */

u8 sis630_host_device::capptr_r()
{
	LOGIO("Read capptr_r [$34]\n");
	return 0xc0;
}

u8 sis630_host_device::dram_status_r()
{
	LOGIO("Read DRAM status [$63] (%02x)\n", m_dram_status);
	return m_dram_status;
}

void sis630_host_device::dram_status_w(u8 data)
{
	LOGIO("Write DRAM status [$63] %02x\n", data);

	m_dram_status = data;
	// TODO: bit 7 is shared memory control
}


u8 sis630_host_device::smram_r()
{
	LOGIO("Read SMRAM [$6a] (%02x)\n", m_smram);
	return m_smram;
}

void sis630_host_device::smram_w(u8 data)
{
	LOGIO("Write SMRAM [$6a] %02x\n", data);
	m_smram = data;
	remap_cb();
}

u32 sis630_host_device::shadow_ram_ctrl_r(offs_t offset, uint32_t mem_mask)
{
	LOGIO("Read shadow RAM setting [$70] %08x (%08x)\n", mem_mask, m_shadow_ram_ctrl);
	return m_shadow_ram_ctrl;
}

void sis630_host_device::shadow_ram_ctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_shadow_ram_ctrl);
	LOGMAP("Write shadow RAM setting [$70] %08x & %08x (%08x)\n", data, mem_mask, m_shadow_ram_ctrl);
	remap_cb();
}

u8 sis630_host_device::pci_hole_r()
{
	LOGIO("Read PCI hole [$77]\n");
	return 0;
}

void sis630_host_device::pci_hole_w(u8 data)
{
	LOGIO("Write PCI hole [$77] %02x\n", data);
	if (data)
		LOG("Warning: PCI hole area enabled! %02x\n", data);
}

u8 sis630_host_device::vga_control_r()
{
	LOGIO("Read integrated VGA control data [$9c] %02x\n", m_vga_control);
	return m_vga_control;
}

void sis630_host_device::vga_control_w(u8 data)
{
	LOGIO("Write integrated VGA control data [$9c] %02x\n", data);
	// TODO: "integrated VGA control" (?)
	m_vga_control = data;
//  remap_cb();
}

u32 sis630_host_device::agp_priority_timer_r(offs_t offset, uint32_t mem_mask)
{
	LOGIO("Read AGP priority timer [$a0] %08x (%08x)\n", mem_mask, m_agp_priority_timer);
	return m_agp_priority_timer;
}

void sis630_host_device::agp_priority_timer_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_agp_priority_timer);
	LOGIO("Write AGP priority timer [$a0] %08x & %08x (%08x)\n", data, mem_mask, m_agp_priority_timer);
}

u8 sis630_host_device::agp_mailbox_r(offs_t offset)
{
	LOGIO("Read AGP mailbox [$%02x] (%02x)\n", offset + 0xa4, m_agp_mailbox[offset]);
	return m_agp_mailbox[offset];
}

void sis630_host_device::agp_mailbox_w(offs_t offset, u8 data)
{
	LOGIO("Write AGP mailbox [$%02x] %02x\n", offset + 0xa4, data);
	m_agp_mailbox[offset] = data;
}

// TODO: move to generic interface
u32 sis630_host_device::agp_id_r()
{
	LOGAGP("Read AGP ID [$c0]\n");
	// bits 23-16 AGP v2.0
	// bits 15-8 0x00 no NEXT_PTR (NULL terminates here)
	// bits 7-0 CAP_ID (0x02 for AGP)
	return 0x00200002;
}

u32 sis630_host_device::agp_status_r()
{
	LOGAGP("Read AGP status [$c4]\n");
	// bits 31-24 RQ max number of AGP command requests (0x1f + 1 = 32)
	// bit 9: SBA, side band addressing enabled
	// ---- -xxx RATE
	// ---- -1-- 4X transfer capable
	// ---- --1- 2X transfer capable
	// ---- ---1 1X transfer capable
	// NB: documentation claims a RATE of 0x03 then contradicts with "111b" value, do the math
	// It gets setup with a 4X at POST, assume 0x07 is right

	// Stuff that isn't enabled here:
	// bit 5: 4G support address greater than 4 GB
	// bit 4: FW transfer support

	return 0x1f000207;
}

u32 sis630_host_device::agp_command_r(offs_t offset, uint32_t mem_mask)
{
	LOGAGP("Read AGP command [$c8] %d %d %02x\n", m_agp.sba_enable, m_agp.enable, m_agp.data_rate);
	// TODO: enable gets cleared by AGP_RESET, or even from PCI RST#
	return m_agp.sba_enable << 9 | m_agp.enable << 8 | (m_agp.data_rate & 7);
}

void sis630_host_device::agp_command_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGAGP("Write AGP command [$c8] %08x & %08x\n", data, mem_mask);
	if (ACCESSING_BITS_8_15)
	{
		m_agp.sba_enable = bool(BIT(m_agp.sba_enable, 9));
		m_agp.enable = bool(BIT(m_agp.enable, 8));
		LOGAGP("- SBA_ENABLE = %d AGP_ENABLE = %d\n", m_agp.sba_enable, m_agp.enable);
	}

	if (ACCESSING_BITS_0_7)
	{
		// quick checker, to be translated into an AGP interface
		std::map<u8, std::string> agp_transfer_rates = {
			{ 0, "(illegal 0)" },
			{ 1, "1X" },
			{ 2, "2X" },
			{ 3, "(illegal 3)" },
			{ 4, "4X" },
			{ 5, "(illegal 5)" },
			{ 6, "(illegal 6)" },
			{ 7, "(illegal 7)" }
		};

		// make sure the AGP DATA_RATE specs are honored
		const u8 data_rate = data & 7;
		LOGAGP("- DATA_RATE = %s enabled=%d\n", agp_transfer_rates.at(data_rate), m_agp.enable);
		m_agp.data_rate = data_rate;

		// should probably never be enabled since it reads out from the ID
		if (data & 0x30)
			LOG("Warning: AGP unsupported i/f set 4G=%d FW_Enable=%d\n", bool(BIT(data, 5)), bool(BIT(data, 4)));
	}
}
