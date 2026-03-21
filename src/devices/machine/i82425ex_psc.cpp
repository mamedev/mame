// license:BSD-3-Clause
// copyright-holders: Angelo Salese

#include "emu.h"
#include "i82425ex_psc.h"

#define LOG_MAP         (1U << 1)

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info

#define LOGMAP(...)    LOGMASKED(LOG_MAP,  __VA_ARGS__)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(I82425EX_PSC, i82425ex_psc_device, "i82425ex_psc", "Intel 82425EX PCI System Controller")

i82425ex_psc_device::i82425ex_psc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_host_device(mconfig, I82425EX_PSC, tag, owner, clock)
	, m_host_cpu(*this, finder_base::DUMMY_TAG)
	, m_ib(*this, finder_base::DUMMY_TAG)
	, m_ide(*this, "ide%u", 1U)
	, m_ide1_irq(*this)
	, m_ide2_irq(*this)
{
}

void i82425ex_psc_device::device_add_mconfig(machine_config &config)
{
	IDE_CONTROLLER_32(config, m_ide[0]).options(ata_devices, "hdd", nullptr, false);
	m_ide[0]->irq_handler().set([this] (int state) { m_ide1_irq(state); });

	IDE_CONTROLLER_32(config, m_ide[1]).options(ata_devices, "cdrom", nullptr, false);
	m_ide[1]->irq_handler().set([this] (int state) { m_ide2_irq(state); });
}

void i82425ex_psc_device::device_start()
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

	save_item(NAME(m_pcicon));
	save_item(NAME(m_xbcsa));
	save_item(NAME(m_hostdev));
	save_item(NAME(m_lbide));
	save_item(NAME(m_iort));
	save_item(NAME(m_prev));
	save_item(NAME(m_hostsel));
	save_item(NAME(m_dfc));
	save_item(NAME(m_scc));
	save_item(NAME(m_dramc));
	save_item(NAME(m_pam));
	save_item(NAME(m_drb));
	save_item(NAME(m_pirqrc));
	save_item(NAME(m_dmh));
	save_item(NAME(m_tom));
	save_item(NAME(m_smramcon));
	save_item(NAME(m_smicntl));
	save_item(NAME(m_smien));
	save_item(NAME(m_see));
	save_item(NAME(m_ftmr));
	save_item(NAME(m_smireq));
	save_item(NAME(m_ctltmrl));
	save_item(NAME(m_ctltmrh));

}

void i82425ex_psc_device::device_reset()
{
	pci_host_device::device_reset();

	command = 0x0007;
	// has SERR# support only, cannot clear command 0x07
	command_mask = 0x0100;
	// Medium DEVSEL#
	status = 0x0200;

	m_pcicon = 0;
	m_hostdev = 0;
	m_lbide = 0x0000;
	m_xbcsa = 0x03;
	m_iort = 0x4d;
	m_prev = false;
	m_hostsel = 0;
	m_dfc = 0x80;
	m_scc = 0;
	m_dramc = 0;
	std::fill(std::begin(m_pam), std::end(m_pam), 0U);
	std::fill(std::begin(m_drb), std::end(m_drb), 1U);
	m_pirqrc[0] = m_pirqrc[1] = 0x80;
	m_dmh = 0;
	m_tom = 0x02;
	m_smramcon = 0;
	m_smicntl = 0x08;
	m_smien = 0;
	m_see = 0;
	m_ftmr = 0x0f;
	m_smireq = 0;
	m_ctltmrl = 0;
	m_ctltmrh = 0;

	remap_cb();
}

void i82425ex_psc_device::reset_all_mappings()
{
	pci_host_device::reset_all_mappings();
}

void i82425ex_psc_device::config_map(address_map &map)
{
	pci_host_device::config_map(map);
	map(0x09, 0x3f).lr8(NAME([] () { return 0; }));
	map(0x40, 0x40).lrw8(
		NAME([this] (offs_t offset) {
			return m_pcicon;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("40h: PCICON PCI Control %02x\n", data);
			m_pcicon = data & 0x7f;
		})
	);
	map(0x44, 0x44).lrw8(
		NAME([this] (offs_t offset) {
			return m_hostdev;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("44h: HOSTDEV Host Device Control %02x\n", data);
			m_hostdev = data & 7;
		})
	);
	map(0x48, 0x49).lrw16(
		NAME([this] (offs_t offset) {
			return m_lbide;
		}),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			LOG("48h: LBIDE PCI Local Bus IDE Control %04x & %04x\n", data, mem_mask);
			COMBINE_DATA(&m_lbide);
			m_lbide &= 0x1f3f;
			remap_cb();
		})
	);
	map(0x4c, 0x4c).lrw8(
		NAME([this] (offs_t offset) {
			return m_iort;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("4Ch: IORT ISA I/O Recovery timer %02x\n", data);
			m_iort = data;
		})
	);
	map(0x4d, 0x4d).lrw8(
		NAME([this] (offs_t offset) {
			// xxx- ---- IB Fabrication House ID
			// ---- xxxx Revision ID
			return m_prev << 4;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("4Dh: PREV Part Revision %02x\n", data);
			m_prev = !!BIT(data, 4);
		})
	);
	map(0x4e, 0x4e).lrw8(
		NAME([this] (offs_t offset) {
			return m_xbcsa;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("4Eh: XBCSA X-Bus Chip Select A %02x\n", data);
			m_xbcsa = data;
			remap_cb();
		})
	);
	map(0x50, 0x50).lrw8(
		NAME([this] (offs_t offset) {
			// TODO: bits 2-1 are r/o, determined by strapping on SIDLE# and CMDV#
			return m_hostsel | (1 << 1);
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("50h: HOSTSEL Host Select %02x\n", data);
			m_hostsel = data & 9;
		})
	);
	map(0x51, 0x51).lrw8(
		NAME([this] (offs_t offset) {
			return m_dfc;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("51h: DFC Deturbo Frequency Control %02x\n", data);
			m_dfc = data;
		})
	);
	map(0x52, 0x53).lrw16(
		NAME([this] (offs_t offset) {
			return m_scc;
		}),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			LOG("51h: SCC Secondary L2 Cache Control %04x & %04x\n", data, mem_mask);
			COMBINE_DATA(&m_scc);
			m_scc &= 0x1f7f;
		})
	);
	map(0x56, 0x57).lrw16(
		NAME([this] (offs_t offset) {
			return m_dramc;
		}),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			LOG("56h: DRAMC DRAM Control %04x & %04x\n", data, mem_mask);
			COMBINE_DATA(&m_dramc);
			m_dramc &= 0xff3e;
		})
	);
	map(0x59, 0x5f).lrw8(
		NAME([this] (offs_t offset) {
			return m_pam[offset];
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_pam[offset] = data;
			remap_cb();
		})
	);
	map(0x60, 0x64).lrw8(
		NAME([this] (offs_t offset) {
			return m_drb[offset];
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_drb[offset] = data;
			//remap_cb();
		})
	);
	map(0x66, 0x67).lrw8(
		NAME([this] (offs_t offset) {
			return m_pirqrc[offset];
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("%02X: PIRQ%dRC %02x\n", offset + 0x66, offset, data);
			m_pirqrc[offset] = data;
		})
	);
	map(0x68, 0x68).lrw8(
		NAME([this] (offs_t offset) {
			return m_dmh;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("68h: DMH DRAM Memory Hole %02x\n", data);
			m_dmh = data;
			remap_cb();
		})
	);
	map(0x69, 0x69).lrw8(
		NAME([this] (offs_t offset) {
			return m_tom;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("69h: TOM Top of Memory %02x\n", data);
			m_tom = data;
		})
	);
	map(0x70, 0x70).lrw8(
		NAME([this] (offs_t offset) {
			return m_smramcon;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("70h: SMRAMCON SMRAM Control %02x\n", data);
			m_smramcon = data & 0x77;
			remap_cb();
		})
	);
	map(0xa0, 0xa0).lrw8(
		NAME([this] (offs_t offset) {
			return m_smicntl;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("A0h: SMICNTL SMI Control %02x\n", data);
			m_smicntl = data & 0x1f;
		})
	);
	map(0xa2, 0xa3).lrw16(
		NAME([this] (offs_t offset) {
			return m_smien;
		}),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			LOG("A2h: SMIEN SMI Enable %04x & %04x\n", data, mem_mask);
			COMBINE_DATA(&m_smien);
			m_smien &= 0xff;
		})
	);
	map(0xa4, 0xa7).lrw32(
		NAME([this] (offs_t offset) {
			return m_see;
		}),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			LOG("A4h: SEE System Event Enable %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_see);
			m_see &= 0xe000'0000;
		})
	);
	map(0xa8, 0xa8).lrw8(
		NAME([this] (offs_t offset) {
			return m_ftmr;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("A8h: FTMR Fast Off Timer %02x\n", data);
			m_ftmr = data;
		})
	);
	map(0xaa, 0xab).lrw16(
		NAME([this] (offs_t offset) {
			return m_smireq;
		}),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			LOG("AAh: SMIREQ SMI Request %04x & %04x\n", data, mem_mask);
			COMBINE_DATA(&m_smireq);
			m_smireq &= 0xff;
		})
	);
	map(0xac, 0xac).lrw8(
		NAME([this] (offs_t offset) {
			return m_ctltmrl;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("ACh: CTLTMRL Clock Throttle STPCLK# Low Timer %02x\n", data);
			m_ctltmrl = data;
		})
	);
	map(0xae, 0xae).lrw8(
		NAME([this] (offs_t offset) {
			return m_ctltmrh;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("AEh: CTLTMRH Clock Throttle STPCLK# High Timer %02x\n", data);
			m_ctltmrh = data;
		})
	);
}

void i82425ex_psc_device::config_address_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (mem_mask == 0x0000'ff00)
		m_ib->trc_w(offset, data >> 8);
	else
		pci_host_device::config_address_w(offset, data, mem_mask);
}

void i82425ex_psc_device::map_bios(address_space *memory_space, uint32_t start, uint32_t end)
{
	uint32_t mask = m_region->bytes() - 1;
	memory_space->install_rom(start, end, m_region->base() + (start & mask));
}

// For each PAM register:
// x--- PE PCI Enable
// -x-- CE Cache Enable (from L2)
// --x- WE Write Enable
// ---x RE Read Enable
void i82425ex_psc_device::map_shadowram(address_space *memory_space, offs_t start_offs, offs_t end_offs, u8 setting)
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

void i82425ex_psc_device::map_extra(
	uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
	uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space
) {
	// TODO: $cf9 for TRC register
	io_space->install_device(0, 0xffff,  *static_cast<pci_host_device *>(this), &pci_host_device::io_configuration_access_map);

	regenerate_config_mapping();

	m_ib->remap_bridge();
	io_space->install_device(0x0000, 0x07ff, *m_ib, &i82426ex_ib_device::io_map);

	memory_space->install_ram(0x00000000, 0x0009ffff, &m_ram[0x00000000/4]);

	// TODO: dictated by XBCSA
	map_bios(memory_space, 0xffffffff - m_region->bytes() + 1, 0xffffffff);
	map_bios(memory_space, 0x000e0000, 0x000fffff);

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

	// bits 3-0 are reserved on PAM0
	map_shadowram(memory_space, 0xf0000, 0xfffff, (m_pam[0] >> 4) & 3);

	memory_space->install_ram(0x00100000, m_ram_size - 1, &m_ram[0x00100000/4]);

	const u8 idee = m_lbide & 3;

	// TODO: should be IDEE = 1 primary only, IDEE = 2 secondary only
	// But in doing so freedos13 never enable cdrom drive,
	// is this chipset/BIOS believing that IDEs are for HDDs only?
	if (idee == 1 || idee == 2)
	{
		io_space->install_readwrite_handler(0x1f0, 0x1f7, read32s_delegate(*m_ide[0], FUNC(ide_controller_32_device::cs0_r)), write32s_delegate(*m_ide[0], FUNC(ide_controller_32_device::cs0_w)));
		io_space->install_readwrite_handler(0x3f0, 0x3f7, read32s_delegate(*m_ide[0], FUNC(ide_controller_32_device::cs1_r)), write32s_delegate(*m_ide[0], FUNC(ide_controller_32_device::cs1_w)));

		io_space->install_readwrite_handler(0x170, 0x177, read32s_delegate(*m_ide[1], FUNC(ide_controller_32_device::cs0_r)), write32s_delegate(*m_ide[1], FUNC(ide_controller_32_device::cs0_w)));
		io_space->install_readwrite_handler(0x370, 0x377, read32s_delegate(*m_ide[1], FUNC(ide_controller_32_device::cs1_r)), write32s_delegate(*m_ide[1], FUNC(ide_controller_32_device::cs1_w)));
	}
}

