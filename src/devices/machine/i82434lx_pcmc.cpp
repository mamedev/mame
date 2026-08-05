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
	save_item(NAME(m_cse));
	save_item(NAME(m_trc));
	save_item(NAME(m_forw));
	save_item(NAME(m_pcams));

	save_item(NAME(m_hcs));
	save_item(NAME(m_dfc));
	save_item(NAME(m_scc));
	save_item(NAME(m_hbc));
	save_item(NAME(m_pbc));
	save_item(NAME(m_dramc));
	save_item(NAME(m_dramt));
	save_item(NAME(m_pam));
	save_item(NAME(m_drb));
	save_item(NAME(m_drbe));
	save_item(NAME(m_errcmd));
	save_item(NAME(m_errsts));
	save_item(NAME(m_msg));
	save_item(NAME(m_fbr));
	save_item(NAME(m_smrs));
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
	m_cse = m_forw = m_trc = m_pcams = 0;

	// LX: 0x82
	// NX: 0xa2 for default 60 Mhz, we are under 66 MHz
	m_hcs = 0xa3;
	m_dfc = 0x80;
	// TODO: strapping for bits 7-5
	m_scc = 0x0a;
	m_hbc = 0x00;
	m_pbc = 0x00;

	m_dramc = 0x31;
	m_dramt = 0x00;
	std::fill(std::begin(m_pam), std::end(m_pam), 0U);
	std::fill(std::begin(m_drb), std::end(m_drb), 2U);
	std::fill(std::begin(m_drbe), std::end(m_drbe), 0U);

	m_errcmd = 0x00;
	m_errsts = 0x00;

	m_smrs = 0x00;
	m_smrs_mask = 0x3f;

	m_msg = 0x0000;
	m_fbr = 0x00000000;

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

	// xxx- ---- HCT Host CPU type
	// 100- ---- LX (Pentium)
	// 101- ---- NX (<reserved>)
	// ---- -x-- FLCE First Level Cache Enable
	// ---- --xx HOF Host Operating Frequency
	// ---- --x- LX: <reserved>
	// ---- ---0     60 MHz
	// ---- ---1     66 MHz
	// ---- --00 NX: <reserved>
	// ---- --01     50 MHz
	// ---- --10     60 MHz
	// ---- --11     66 MHz
	map(0x50, 0x50).lrw8(
		NAME([this] (offs_t offset) {
			return m_hcs | 0xa0;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("50h: HCS Host CPU Selection %02x\n", data);
			m_hcs = data & 0x7;
		})
	);
	map(0x51, 0x51).lrw8(
		NAME([this] (offs_t offset) {
			return m_dfc;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("51h: DFC Deturbo Frequency Control %02x\n", data);
			m_dfc = data & 0xc0;
		})
	);
	map(0x52, 0x52).lrw8(
		NAME([this] (offs_t offset) {
			return m_scc;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("52h: SCC Secondary Cache Control %02x\n", data);
			m_scc = data;
		})
	);
	map(0x53, 0x53).lrw8(
		NAME([this] (offs_t offset) {
			return m_hbc;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("53h: HBC Host R/W Buffer Control %02x\n", data);
			m_hbc = data & 0xb;
		})
	);
	map(0x54, 0x54).lrw8(
		NAME([this] (offs_t offset) {
			return m_pbc;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("54h: PBC PCI R/W Buffer Control %02x\n", data);
			m_pbc = data & 0x7;
		})
	);

	// xx-- ---- LX: <reserved>
	// xx-- ---- NX: DBT DRAM Burst Timing
	// 00-- ---- X-4-4-4 r/w timing
	// 01-- ---- X-4-4-4 r, X-3-3-3 w timing
	// 10-- ---- <reserved>
	// 11-- ---- X-3-3-3 r/w timing
	// --x- ---- PERRM Parity Error Mask
	// ---x ---- 0-Active RAS# Mode
	// ---- x--- SMRE SMRAM Enable
	// ---- -x-- BFR Burst of 4 Refresh
	// ---- --x- RT Refresh Type (slightly different on NX)
	// ---- ---x RE Refresh Enable
	map(0x57, 0x57).lrw8(
		NAME([this] (offs_t offset) {
			return m_dramc;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("57h: DRAMC DRAM Control %02x\n", data);
			m_dramc = data;
		})
	);
	map(0x58, 0x58).lrw8(
		NAME([this] (offs_t offset) {
			return m_dramt;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("58h: DRAMT DRAM Timing %02x\n", data);
			// bit 1 reserved on LX
			m_dramt = data & 3;
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

	// DRB[5:0] only for LX
	map(0x60, 0x67).lrw8(
		NAME([this] (offs_t offset) {
			return m_drb[offset];
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("%02Xh: DRB%d DRAM Row Boundary %02x\n", offset + 0x60, offset, data);
			m_drb[offset] = data;
		})
	);
	map(0x68, 0x6b).lrw8(
		NAME([this] (offs_t offset) {
			const u8 base_reg = offset * 2;
			return m_drbe[base_reg] | (m_drbe[base_reg + 1] << 4);
		}),
		NAME([this] (offs_t offset, u8 data) {
			// 4-bit nibbles
			const u8 base_reg = offset * 2;
			LOG("%02Xh: DRBE%d DRAM Row Boundary Extension %02x\n", offset + 0x68, base_reg, data);
			m_drbe[base_reg + 0] = data & 0xf;
			m_drbe[base_reg + 1] = data >> 4;
		})
	);

	map(0x70, 0x70).lrw8(
		NAME([this] (offs_t offset) {
			return m_errcmd;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("70h: ERRCMD Error Command %02x\n", data);
			// bits 5 to 3 are <reserved> on LX
			m_errcmd = data;
		})
	);
	map(0x71, 0x71).lrw8(
		NAME([this] (offs_t offset) {
			return m_errsts;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("71h: ERRSTS Error Status %02x\n", data);
			// write clear register
			// bits 5-4 are <reserved> on LX
			m_errsts &= ~(data & 0x7d);
		})
	);

	// TODO: also ties with m_dramc bit 3
	// --x- ---- OSS Open SMRAM Space (POST access)
	// ---x ---- CSS Close SMRAM Space (SMM = code, PCI forward = data)
	// ---- x--- LSS Lock SMRAM Space (disables OSS, only POST can enable it back)
	// ---- -xxx SMM Base Segment (unlisted combinations <reserved>)
	// ---- -000 Top of Main Memory
	// ---- -010 $a0000-$affff
	// ---- -011 $b0000-$bffff
	map(0x72, 0x72).lrw8(
		NAME([this] (offs_t offset) {
			return m_smrs;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("72h: SMRS SMRAM Space %02x\n", data);
			m_smrs = data & m_smrs_mask;
			if (BIT(data, 3))
				m_smrs_mask = 0x1f;
			// remap_cb();
		})
	);

	map(0x78, 0x79).lrw16(
		NAME([this] (offs_t offset) {
			return m_msg;
		}),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			LOG("78h: MSG Memory Space Gap %04x & %04x\n", data, mem_mask);
			COMBINE_DATA(&m_msg);
			m_msg &= 0xf0f0;
		})
	);

	map(0x7c, 0x7f).lrw32(
		NAME([this] (offs_t offset) {
			return m_fbr;
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOG("7ch: FBR Frame Buffer Range %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_fbr);
			m_fbr &= 0xffff'328f;
		})
	);
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

uint32_t i82434nx_pcmc_device::config_address_r(offs_t offset, uint32_t mem_mask)
{
	// TODO: if NX m_pcams is 0 then it can't access the canonical PCI range
	if (mem_mask == 0xffff'ffff)
		return pci_host_device::config_address_r();

	switch(mem_mask)
	{
		case 0x0000'00ff:
			return m_cse;

		case 0x0000'ff00:
			return m_trc;

		case 0x00ff'0000:
			return m_forw;

		case 0xff00'0000:
			return m_pcams;
	}

	if (!machine().side_effects_disabled())
		LOG("Warning: config_address_r access with a %08x access!\n", mem_mask);
	return 0xff;
}

void i82434nx_pcmc_device::config_address_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	// TODO: as above
	if (mem_mask == 0xffff'ffff)
		pci_host_device::config_address_w(offset, data, mem_mask);
	else
	{
		switch(mem_mask)
		{
			case 0x0000'00ff:
				// bit 0 is special cycle enabled (unsupported by this flavour)
				m_cse = data & 0xfe;
				break;

			case 0x0000'ff00:
				m_trc = (data >> 8) & 0x7;
				if (m_trc)
					LOG("TRC write %02x\n", data);
				break;

			case 0x00ff'0000:
				m_forw = data >> 16;
				break;

			case 0xff00'0000:
				// PMC register, NX only
				m_pcams = BIT(data, 24);
				break;

			default:
				LOG("Warning: config_address_w access with a %08x -> %08x access!\n", mem_mask, data);
				break;
		}
	}
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

void i82434nx_pcmc_device::io_configuration_access_map(address_map &map)
{
	pci_host_device::io_configuration_access_map(map);

	// https://wiki.osdev.org/PCI#Configuration_Space_Access_Mechanism_#2
	// uses an extra I/O space for accessing PCI devices
	// we handle key at this level for now, BIOSes loves to flip CSE on -> <access> -> off again,
	// which causes a performance problem due of the remap_cb calls.
	// NOTE: lx has no regular CONFADD/CONFDATA!
	// TODO: FORW implementation, for bus number
	map(0xc000, 0xcfff).lrw32(
		NAME([this] (offs_t offset, u32 mem_mask) {
			if (!(m_cse & 0xf0))
			{
				if (!machine().side_effects_disabled())
					LOG("Warning: read $%04x space with CSE key disabled!\n", offset + 0xc000);
				return 0xffff'ffff;
			}
			const u32 config_number = offset * 4;
			const u8 devnum = (config_number & 0xf00) >> 8;
			const u8 fnum = (m_cse >> 1) & 7;
			return root_config_read(0, (devnum << 3) | fnum, config_number & 0xfc, mem_mask);
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			if (!(m_cse & 0xf0))
			{
				if (!machine().side_effects_disabled())
					LOG("Warning: write $%04x & %08x -> %08x space with CSE key disabled!\n", offset + 0xc000, mem_mask, data);
				return;
			}

			const u32 config_number = offset * 4;
			const u8 devnum = (config_number & 0xf00) >> 8;
			const u8 fnum = (m_cse >> 1) & 7;
			root_config_write(0, (devnum << 3) | fnum, config_number & 0xfc, data, mem_mask);
		})
	);
}

void i82434nx_pcmc_device::map_extra(
	uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
	uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space
) {
	io_space->install_device(0, 0xffff,  *static_cast<i82434nx_pcmc_device *>(this), &i82434nx_pcmc_device::io_configuration_access_map);

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


