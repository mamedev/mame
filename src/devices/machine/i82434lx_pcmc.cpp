// license:BSD-3-Clause
// copyright-holders: Angelo Salese

#include "emu.h"
#include "i82434lx_pcmc.h"

#define LOG_MAP         (1U << 1)

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info

#define LOGMAP(...)    LOGMASKED(LOG_MAP,  __VA_ARGS__)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(I82434NX_PCMC, i82434nx_pcmc_device, "i82434nx_pcmc", "Intel 82434NX PCI, Cache and Memory Controller (PCMC)")

i82434nx_pcmc_device::i82434nx_pcmc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_host_device(mconfig, I82434NX_PCMC, tag, owner, clock)
	, m_host_cpu(*this, finder_base::DUMMY_TAG)
{
}

void i82434nx_pcmc_device::device_start()
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

	save_item(NAME(m_latency_timer));
	save_item(NAME(m_pam));
}

void i82434nx_pcmc_device::device_reset()
{
	pci_host_device::device_reset();

	command = 0x0007;
	// has SERR# and PERR# support, cannot change bus master
	command_mask = 0x0143;
	// TODO: Medium DEVSEL#?
	// default value 0x40 (?), "can also assert in medium time"
	status = 0x0200;

	m_latency_timer = 0x20;
	std::fill(std::begin(m_pam), std::end(m_pam), 0U);

	remap_cb();
}

void i82434nx_pcmc_device::reset_all_mappings()
{
	pci_host_device::reset_all_mappings();
}

void i82434nx_pcmc_device::config_map(address_map &map)
{
	pci_host_device::config_map(map);
	map(0x0d, 0x0d).rw(FUNC(i82434nx_pcmc_device::latency_timer_r), FUNC(i82434nx_pcmc_device::latency_timer_w));

	// <reserved> range by this controller
	map(0x10, 0x4f).lr8(NAME([] () { return 0; }));

//  map(0x50, 0x50) HCS
//  map(0x51, 0x51) DFC
//  map(0x52, 0x52) SCC
//  map(0x53, 0x53) HBC
//  map(0x54, 0x54) PBC

//  map(0x57, 0x57) DRAMC
//  map(0x58, 0x58) DRAMT
	map(0x59, 0x5f).lrw8(
		NAME([this] (offs_t offset) {
			return m_pam[offset];
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_pam[offset] = data;
			remap_cb();
		})
	);
//  map(0x60, 0x65) DRB[5:0]
//  map(0x66, 0x67) DRB[7:6]
//  map(0x68, 0x6b) DRBE

//  map(0x70, 0x70) ERRCMD
//  map(0x71, 0x71) ERRSTS
//  map(0x72, 0x72) SMRS

//  map(0x78, 0x79) MSG

//  map(0x7c, 0x7f) FBR
}

/*
 * config space mapping
 */

u8 i82434nx_pcmc_device::latency_timer_r()
{
	return m_latency_timer;
}

// bit 3-0 reserved
void i82434nx_pcmc_device::latency_timer_w(u8 data)
{
	m_latency_timer = data & 0xf0;
}

/*
 * PCI config host
 */

void i82434nx_pcmc_device::config_address_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	// TODO: same byte vs. dword access as i82425ex_psc
	//if (mem_mask == 0x0000'ff00)
	//  m_ib->trc_w(offset, data >> 8);
	//else
	pci_host_device::config_address_w(offset, data, mem_mask);
}

// For each PAM register:
// -x-- CE Cache Enable
// --x- WE Write Enable
// ---x RE Read Enable
// very similar to earlier i420ex chipset, except no PCI bit 3
void i82434nx_pcmc_device::map_shadowram(address_space *memory_space, offs_t start_offs, offs_t end_offs, u8 setting)
{
	LOGMAP("- 0x%08x-0x%08x ", start_offs, end_offs);

	switch(setting)
	{
		case 0:
			LOGMAP("shadow RAM off\n");
			break;
		case 1:
			LOGMAP("shadow RAM r/o\n");
			memory_space->install_rom(start_offs, end_offs, &m_ram[start_offs/4]);
			break;
		case 2:
			LOGMAP("shadow RAM w/o\n");
			memory_space->install_writeonly(start_offs, end_offs, &m_ram[start_offs/4]);
			break;
		case 3:
			LOGMAP("shadow RAM r/w\n");
			memory_space->install_ram(start_offs, end_offs, &m_ram[start_offs/4]);
			break;
	}
}

void i82434nx_pcmc_device::map_extra(
	uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
	uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space
) {
	io_space->install_device(0, 0xffff,  *static_cast<pci_host_device *>(this), &pci_host_device::io_configuration_access_map);

	regenerate_config_mapping();

	// TODO: PAM0 bits 3-0 memory hole at 512K
	memory_space->install_ram(0x00000000, 0x0009ffff, &m_ram[0x00000000/4]);

	int i;

	// handle 0xc0000-0xdffff
	for (i = 0; i < 8; i++)
	{
		const offs_t start_offs = 0xc0000 + (i * 0x4000);
		const offs_t end_offs = start_offs + 0x3fff;
		const u8 reg = i >> 1;
		const u8 shift = BIT(i, 0) * 4;
		map_shadowram(memory_space, start_offs, end_offs, (m_pam[1 + reg] >> shift) & 3);
	}

	// handle 0xe0000-0xeffff
	for (i = 0; i < 4; i++)
	{
		const offs_t start_offs = 0xe0000 + (i * 0x4000);
		const offs_t end_offs = start_offs + 0x3fff;
		const u8 reg = BIT(i, 1);
		const u8 shift = BIT(i, 0) * 4;
		map_shadowram(memory_space, start_offs, end_offs, (m_pam[5 + reg] >> shift) & 3);
	}

	map_shadowram(memory_space, 0xf0000, 0xfffff, (m_pam[0] >> 4) & 3);

	memory_space->install_ram(0x00100000, m_ram_size - 1, &m_ram[0x00100000/4]);
}


