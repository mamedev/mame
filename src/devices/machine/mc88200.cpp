// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Motorola MC88200 Cache/Memory Management Unit (CMMU).
 *
 * Sources:
 *  - MC88200 Cache/Memory Management User's Manual, Second Edition (MC88200UM/AD, Rev 1)
 *
 * TODO:
 *  - probe commands
 *  - mbus snooping
 *  - cycle counting
 *  - cache inhibited accesses invalidate matching cache tags (no writeback)
 *  - mc88204 64k variant
 *  - find out where patc valid flag is stored
 */

#include "emu.h"

#include "mc88200.h"

//#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(MC88200, mc88200_device, "mc88200", "Motorola MC88200 Cache/Memory Management Unit")

mc88200_device::mc88200_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock, u8 id)
	: device_t(mconfig, MC88200, tag, owner, clock)
	, m_mbus(*this, finder_base::DUMMY_TAG, -1, 32)
	, m_id(u32(id) << 24)
{
}

enum idr_mask : u32
{
	IDR_VERSION = 0x001f0000,
	IDR_TYPE    = 0x00e00000,
	IDR_ID      = 0xff000000,
};

enum idr_type_mask : u32
{
	TYPE_MC88200 = 0x00a00000, // 16k cache
	TYPE_MC88204 = 0x00c00000, // 64k cache
};

enum ssr_mask : u32
{
	SSR_V  = 0x00000001, // valid
	SSR_BH = 0x00000002, // batc hit
	SSR_WP = 0x00000004, // write protection
	SSR_U  = 0x00000008, // used
	SSR_M  = 0x00000010, // modified
	SSR_CI = 0x00000040, // cache inhibit
	SSR_G  = 0x00000080, // global
	SSR_SP = 0x00000100, // supervisor privilege
	SSR_WT = 0x00000200, // writethrough
	SSR_BE = 0x00004000, // bus error
	SSR_CE = 0x00008000, // copyback error

	SSR_WM = 0x0000c3df,
};

enum sctr_mask : u32
{
	SCTR_PR = 0x00010000, // priority arbitration
	SCTR_SE = 0x00020000, // snoop enable
	SCTR_PE = 0x00040000, // parity enable
};

enum pfsr_mask : u32
{
	PFSR_OK = 0x00000000, // success (no fault)
	PFSR_BE = 0x00030000, // bus error
	PFSR_SF = 0x00040000, // segment fault
	PFSR_PF = 0x00050000, // page fault
	PFSR_SV = 0x00060000, // supervisor violation
	PFSR_WV = 0x00070000, // write violation

	PFSR_WM = 0x00070000,
};

enum apr_mask : u32
{
	APR_TE   = 0x00000001, // translation enable
	APR_CI   = 0x00000040, // cache inhibit
	APR_G    = 0x00000080, // global
	APR_WT   = 0x00000200, // writethrough
	APR_STBA = 0xfffff000, // segment table base address

	APR_WM   = 0xfffff2c1,
};

enum batc_mask : u32
{
	BATC_V   = 0x00000001, // valid
	BATC_WP  = 0x00000002, // write protect
	BATC_CI  = 0x00000004, // cache inhibit
	BATC_G   = 0x00000008, // global
	BATC_WT  = 0x00000010, // writethrough
	BATC_S   = 0x00000020, // supervisor
	BATC_PBA = 0x0007ffc0, // physical block address
	BATC_LBA = 0xfff80000, // logical block address
};

enum cssp_mask : u32
{
	CSSP_VV0 = 0x00003000, // line valid 0
	CSSP_VV1 = 0x0000c000, // line valid 1
	CSSP_VV2 = 0x00030000, // line valid 2
	CSSP_VV3 = 0x000c0000, // line valid 3
	CSSP_D0  = 0x00100000, // line disable 0
	CSSP_D1  = 0x00200000, // line disable 1
	CSSP_D2  = 0x00400000, // line disable 2
	CSSP_D3  = 0x00800000, // line disable 3
	CSSP_L0  = 0x01000000, // line 1 more recently used than line 0
	CSSP_L1  = 0x02000000, // line 2 more recently used than line 0
	CSSP_L2  = 0x04000000, // line 2 more recently used than line 1
	CSSP_L3  = 0x08000000, // line 3 more recently used than line 0
	CSSP_L4  = 0x10000000, // line 3 more recently used than line 1
	CSSP_L5  = 0x20000000, // line 3 more recently used than line 2

	CSSP_WM  = 0x3ffff000,
};

enum valid : unsigned
{
	EXU = 0, // exclusive unmodified
	EXM = 1, // exclusive modified
	SHU = 2, // shared unmodified
	INV = 3, // invalid
};

enum patc_mask : u64
{
	PATC_WP  = 0x0000'00000001, // write protect
	PATC_M   = 0x0000'00000002, // modified
	PATC_CI  = 0x0000'00000004, // cache inhibit
	PATC_G   = 0x0000'00000008, // global
	PATC_WT  = 0x0000'00000010, // writethrough
	PATC_S   = 0x0000'00000020, // supervisor
	PATC_PFA = 0x0000'03ffffc0, // page frame address
	PATC_LPA = 0x3fff'fc000000, // logical page address

	PATC_V   = 0x8000'00000000, // valid (??)
};

enum segment_descriptor_mask : u32
{
	SGD_V    = 0x00000001, // valid
	SGD_WP   = 0x00000004, // write protect
	SGD_CI   = 0x00000040, // cache inhibit
	SGD_G    = 0x00000080, // global
	SGD_SP   = 0x00000100, // supervisor protection
	SGD_WT   = 0x00000200, // writethrough
	SGD_PTBA = 0xfffff000, // page table base address
};

enum page_descriptor_mask : u32
{
	PGD_V   = 0x00000001, // valid
	PGD_WP  = 0x00000004, // write protect
	PGD_U   = 0x00000008, // used
	PGD_M   = 0x00000010, // modified
	PGD_CI  = 0x00000040, // cache inhibit
	PGD_G   = 0x00000080, // global
	PGD_SP  = 0x00000100, // supervisor protection
	PGD_WT  = 0x00000200, // writethrough
	PGD_PFA = 0xfffff000, // page frame address
};

enum logical_address_mask : u32
{
	LA_OFS = 0x00000fff,
	LA_PAG = 0x003ff000,
	LA_SEG = 0xffc00000,
};

void mc88200_device::device_start()
{
	m_cache = std::make_unique<cache_set[]>(CACHE_SETS);

	save_item(NAME(m_idr));
	save_item(NAME(m_scr));
	save_item(NAME(m_ssr));
	save_item(NAME(m_sar));
	save_item(NAME(m_sctr));
	save_item(NAME(m_pfsr));
	save_item(NAME(m_pfar));
	save_item(NAME(m_sapr));
	save_item(NAME(m_uapr));

	save_item(NAME(m_batc));
	save_item(NAME(m_patc));
	save_item(NAME(m_patc_ptr));

	save_item(NAME(m_bus_error));

	save_pointer(STRUCT_MEMBER(m_cache, status), CACHE_SETS);
	// TODO: save state for cache lines

	m_idr = m_id | TYPE_MC88200;
	m_mbus->install_device(0xfff00000U | ((m_idr & IDR_ID) >> 12), 0xfff00fffU | ((m_idr & IDR_ID) >> 12), *this, &mc88200_device::map);
}

void mc88200_device::device_reset()
{
	m_scr = 0;
	m_ssr = 0;
	m_sar = 0; // undefined
	m_sctr = 0;
	m_pfsr = 0;
	m_pfar = 0; // undefined
	m_sapr = APR_CI;
	m_uapr = APR_CI;

	std::fill_n(m_batc, std::size(m_batc), 0U);

	// batc contains two hard-wired entries
	m_batc[8] = 0xfff7ffb5;
	m_batc[9] = 0xfffffff5;

	std::fill_n(m_patc, std::size(m_patc), 0U);
	m_patc_ptr = 0;

	m_bus_error = false;

	idr_w(m_id);
}

void mc88200_device::map(address_map &map)
{
	// system interface registers
	map(0x000, 0x003).rw(FUNC(mc88200_device::idr_r), FUNC(mc88200_device::idr_w));
	map(0x004, 0x007).rw(FUNC(mc88200_device::scr_r), FUNC(mc88200_device::scr_w));
	map(0x008, 0x00b).rw(FUNC(mc88200_device::ssr_r), FUNC(mc88200_device::ssr_w));
	map(0x00c, 0x00f).rw(FUNC(mc88200_device::sar_r), FUNC(mc88200_device::sar_w));
	map(0x104, 0x107).rw(FUNC(mc88200_device::sctr_r), FUNC(mc88200_device::sctr_w));

	// p bus fault registers
	map(0x108, 0x10b).rw(FUNC(mc88200_device::pfsr_r), FUNC(mc88200_device::pfsr_w));
	map(0x10c, 0x10f).rw(FUNC(mc88200_device::pfar_r), FUNC(mc88200_device::pfar_w));

	// area pointers
	map(0x200, 0x203).rw(FUNC(mc88200_device::sapr_r), FUNC(mc88200_device::sapr_w));
	map(0x204, 0x207).rw(FUNC(mc88200_device::uapr_r), FUNC(mc88200_device::uapr_w));

	// batc write ports
	map(0x400, 0x41f).w(FUNC(mc88200_device::bwp_w)).mirror(0x20);

	// cache diagnostic ports
	map(0x800, 0x80f).rw(FUNC(mc88200_device::cdp_r), FUNC(mc88200_device::cdp_w)).mirror(0x30);
	map(0x840, 0x84f).rw(FUNC(mc88200_device::ctp_r), FUNC(mc88200_device::ctp_w)).mirror(0x30);
	map(0x880, 0x883).rw(FUNC(mc88200_device::cssp_r), FUNC(mc88200_device::cssp_w)).mirror(0x30);
}

void mc88200_device::idr_w(u32 data)
{
	if ((data ^ m_idr) & IDR_ID)
	{
		LOG("idr_w 0x%08x (%s)\n", data, machine().describe_context());

		m_mbus->unmap_readwrite(0xfff00000U | ((m_idr & IDR_ID) >> 12), 0xfff00fffU | ((m_idr & IDR_ID) >> 12));
		m_idr = (m_idr & ~IDR_ID) | (data & IDR_ID);
		m_mbus->install_device(0xfff00000U | ((m_idr & IDR_ID) >> 12), 0xfff00fffU | ((m_idr & IDR_ID) >> 12), *this, &mc88200_device::map);
	}
}

void mc88200_device::scr_w(u32 data)
{
	LOG("scr_w 0x%08x (%s)\n", data, machine().describe_context());

	char const *const action[] = { nullptr, "invalidate", "copy back", "copy back and invalidate" };

	switch (data & 0x3f)
	{
	case 0x00: case 0x01: case 0x02: case 0x03:
	case 0x04: case 0x05: case 0x06: case 0x07:
	case 0x08: case 0x09: case 0x0a: case 0x0b:
	case 0x0c: case 0x0d: case 0x0e: case 0x0f:
	case 0x10: case 0x11: case 0x12: case 0x13:
		LOG("no operation\n");
		break;
	case 0x14: // invalidate line
	case 0x18: // copy back line
	case 0x1c: // copy back and invalidate line
		LOG("data cache %s line 0x%08x\n", action[BIT(data, 2, 2)], m_sar);
		cache_flush(BIT(m_sar, 4, 8), BIT(m_sar, 4, 8) + 1, &mc88200_device::cache_set::cache_line::match_page, BIT(data, 3), BIT(data, 2));
		break;
	case 0x15: // invalidate page
	case 0x19: // copy back page
	case 0x1d: // copy back and invalidate page
		LOG("data cache %s page 0x%08x\n", action[BIT(data, 2, 2)], m_sar & ~LA_OFS);
		cache_flush(0, CACHE_SETS, &mc88200_device::cache_set::cache_line::match_page, BIT(data, 3), BIT(data, 2));
		break;
	case 0x16: // invalidate segment
	case 0x1a: // copy back segment
	case 0x1e: // copy back and invalidate segment
		LOG("data cache %s segment 0x%08x\n", action[BIT(data, 2, 2)], m_sar & LA_SEG);
		cache_flush(0, CACHE_SETS, &mc88200_device::cache_set::cache_line::match_segment, BIT(data, 3), BIT(data, 2));
		break;
	case 0x17: // invalidate all
	case 0x1b: // copy back all
	case 0x1f: // copy back and invalidate all
		LOG("data cache %s all\n", action[BIT(data, 2, 2)]);
		cache_flush(0, CACHE_SETS, nullptr, BIT(data, 3), BIT(data, 2));
		break;
	case 0x20: case 0x21: case 0x22: case 0x23:
	case 0x28: case 0x29: case 0x2a: case 0x2b:
		logerror("probe user address (unemulated)\n");
		break;
	case 0x24: case 0x25: case 0x26: case 0x27:
	case 0x2c: case 0x2d: case 0x2e: case 0x2f:
		logerror("probe supervisor (unemulated)\n");
		break;
	case 0x31: case 0x39: // page
		LOG("invalidate PATC descriptors (user, page 0x%08x)\n", m_sar & ~LA_OFS);
		for (u64 &patc : m_patc)
			if (!(patc & PATC_S) && BIT(patc, 26, 20) == BIT(m_sar, 12, 20))
				patc &= ~PATC_V;
		break;
	case 0x32: case 0x3a: // segment
		LOG("invalidate PATC descriptors (user, segment 0x%08x)\n", m_sar & LA_SEG);
		for (u64 &patc : m_patc)
			if (!(patc & PATC_S) && BIT(patc, 36, 10) == BIT(m_sar, 22, 10))
				patc &= ~PATC_V;
		break;
	case 0x33: case 0x3b: // all
		LOG("invalidate PATC descriptors (user, all)\n");
		for (u64 &patc : m_patc)
			if (!(patc & PATC_S))
				patc &= ~PATC_V;
		break;
	case 0x35: case 0x3d: // page
		LOG("invalidate PATC descriptors (supervisor, page 0x%08x)\n", m_sar & ~LA_OFS);
		for (u64 &patc : m_patc)
			if ((patc & PATC_S) && BIT(patc, 26, 20) == BIT(m_sar, 12, 20))
				patc &= ~PATC_V;
		break;
	case 0x36: case 0x3e: // segment
		LOG("invalidate PATC descriptors (supervisor, segment 0x%08x)\n", m_sar & LA_SEG);
		for (u64 &patc : m_patc)
			if ((patc & PATC_S) && BIT(patc, 36, 10) == BIT(m_sar, 22, 10))
				patc &= ~PATC_V;
		break;
	case 0x37: case 0x3f: // all
		LOG("invalidate PATC cache descriptors (supervisor, all)\n");
		for (u64 &patc : m_patc)
			if (patc & PATC_S)
				patc &= ~PATC_V;
		break;
	default:
		logerror("unknown operation 0x%08x (%s)\n", data, machine().describe_context());
		break;
	}

	m_scr = data & 0x3f;
}

void mc88200_device::ssr_w(u32 data) { LOG("ssr_w 0x%08x (%s)\n", data, machine().describe_context()); m_ssr = data & SSR_WM; }
void mc88200_device::sar_w(u32 data) { LOG("sar_w 0x%08x (%s)\n", data, machine().describe_context()); m_sar = data; }
void mc88200_device::sctr_w(u32 data) { LOG("sctr_w 0x%08x (%s)\n", data, machine().describe_context()); m_sctr = data; }

void mc88200_device::pfsr_w(u32 data) { LOG("pfsr_w 0x%08x (%s)\n", data, machine().describe_context()); m_pfsr = data & PFSR_WM; }
void mc88200_device::pfar_w(u32 data) { LOG("pfar_w 0x%08x (%s)\n", data, machine().describe_context()); m_pfar = data; }
void mc88200_device::sapr_w(u32 data) { LOG("sapr_w 0x%08x (%s)\n", data, machine().describe_context()); m_sapr = data & APR_WM; }
void mc88200_device::uapr_w(u32 data) { LOG("uapr_w 0x%08x (%s)\n", data, machine().describe_context()); m_uapr = data & APR_WM; }

void mc88200_device::bwp_w(offs_t offset, u32 data)
{
	LOG("bwp_w %x,0x%08x (%s)\n", offset, data, machine().describe_context());

	if (data & BATC_V)
	{
		for (unsigned i = 0; i < std::size(m_batc); i++)
		{
			if ((i != offset) && (m_batc[i] & BATC_V) && BIT(m_batc[i], 19, 13) == BIT(data, 19, 13))
			{
				logerror("duplicate batc entry 0x%08x invalidated (%s)\n", data, machine().describe_context());
				data &= ~BATC_V;
			}
		}
	}

	m_batc[offset] = data;
}

void mc88200_device::cdp_w(offs_t offset, u32 data)
{
	LOG("cdp_w set %d line %d word %d data 0x%08x (%s)\n",
		BIT(m_sar, 4, 8), offset, BIT(m_sar, 2, 2), data, machine().describe_context());

	m_cache[BIT(m_sar, 4, 8)].line[offset].data[BIT(m_sar, 2, 2)] = data;
}

void mc88200_device::ctp_w(offs_t offset, u32 data)
{
	LOG("ctp_w set %d line %d data 0x%08x (%s)\n",
		BIT(m_sar, 4, 8), offset, data, machine().describe_context());

	m_cache[BIT(m_sar, 4, 8)].line[offset].tag = data & ~LA_OFS;
}

void mc88200_device::cssp_w(u32 data)
{
	LOG("cssp_w 0x%08x (%s)\n", data, machine().describe_context());

	m_cache[BIT(m_sar, 4, 8)].status = data & CSSP_WM;
}

// abbreviated, side-effect free address translation for debugger
bool mc88200_device::translate(int intention, u32 &address, bool supervisor)
{
	// select area descriptor
	u32 const apr = supervisor ? m_sapr : m_uapr;
	if (apr & APR_TE)
	{
		// search block address translation cache
		for (u32 const &batc : m_batc)
		{
			if ((batc & BATC_V) && bool(batc & BATC_S) == supervisor && BIT(address, 19, 13) == BIT(batc, 19, 13))
			{
				address = ((batc & BATC_PBA) << 13) | (address & ~BATC_LBA);
				return true;
			}
		}

		// search page address translation cache
		for (u64 const &patc : m_patc)
		{
			if ((patc & PATC_V) && (bool(patc & PATC_S) == supervisor && BIT(address, 12, 20) == BIT(patc, 26, 20)))
			{
				address = ((patc & PATC_PFA) << 6) | (address & LA_OFS);
				return true;
			}
		}

		// load and check segment descriptor
		std::optional<u32> const sgd = mbus_read<u32>((apr & APR_STBA) | ((address & LA_SEG) >> 20));
		if (!sgd.has_value() || !(sgd.value() & SGD_V) || ((sgd.value() & SGD_SP) && !supervisor))
			return false;

		// load and check page descriptor
		std::optional<u32> pgd = mbus_read<u32>((sgd.value() & SGD_PTBA) | ((address & LA_PAG) >> 10));
		if (!pgd.has_value() || !(pgd.value() & PGD_V) || ((pgd.value() & PGD_SP) && !supervisor))
			return false;

		address = (pgd.value() & PGD_PFA) | (address & LA_OFS);
	}
	else
	{
		// apply hard-wired batc entries
		if (BIT(address, 19, 13) == BIT(m_batc[8], 19, 13))
			address = ((m_batc[8] & BATC_PBA) << 13) | (address & ~BATC_LBA);
		else if (BIT(address, 19, 13) == BIT(m_batc[9], 19, 13))
			address = ((m_batc[9] & BATC_PBA) << 13) | (address & ~BATC_LBA);
	}

	return true;
}

std::optional<mc88200_device::translate_result> mc88200_device::translate(u32 virtual_address, bool supervisor, bool write)
{
	// select area descriptor
	u32 const apr = supervisor ? m_sapr : m_uapr;
	if (apr & APR_TE)
	{
		// search block address translation cache
		for (u32 const &batc : m_batc)
		{
			if ((batc & BATC_V) && bool(batc & BATC_S) == supervisor && BIT(virtual_address, 19, 13) == BIT(batc, 19, 13))
			{
				if (!write || !(batc & BATC_WP))
					return translate_result(((batc & BATC_PBA) << 13) | (virtual_address & ~BATC_LBA), batc & BATC_CI, batc & BATC_G, batc & BATC_WT);
				else
				{
					// write violation
					m_pfsr = PFSR_WV;

					return std::nullopt;
				}
			}
		}

		// search page address translation cache
		bool patc_hit = false;
		for (u64 &patc : m_patc)
		{
			if ((patc & PATC_V) && (bool(patc & PATC_S) == supervisor && BIT(virtual_address, 12, 20) == BIT(patc, 26, 20)))
			{
				if (!write || !(patc & PATC_WP))
				{
					if (!write || (patc & PATC_M))
						return translate_result(((patc & PATC_PFA) << 6) | (virtual_address & LA_OFS), patc & PATC_CI, patc & PATC_G, patc & PATC_WT);

					patc |= PATC_M;
					patc_hit = true;
					break;
				}
				else
				{
					// write violation
					m_pfsr = PFSR_WV;

					return std::nullopt;
				}
			}
		}

		// load and check segment descriptor
		std::optional<u32> const sgd = mbus_read<u32>((apr & APR_STBA) | ((virtual_address & LA_SEG) >> 20));
		if (!sgd.has_value())
			return std::nullopt;
		if (!(sgd.value() & SGD_V))
		{
			m_pfsr = PFSR_SF;
			m_pfar = (apr & APR_STBA) | ((virtual_address & LA_SEG) >> 20);

			return std::nullopt;
		}
		if ((sgd.value() & SGD_SP) && !supervisor)
		{
			m_pfsr = PFSR_SV;
			m_pfar = (apr & APR_STBA) | ((virtual_address & LA_SEG) >> 20);

			return std::nullopt;
		}

		// load and check page descriptor
		std::optional<u32> pgd = mbus_read<u32>((sgd.value() & SGD_PTBA) | ((virtual_address & LA_PAG) >> 10));
		if (!pgd.has_value())
			return std::nullopt;
		if (!(pgd.value() & PGD_V))
		{
			m_pfsr = PFSR_PF;
			m_pfar = (sgd.value() & SGD_PTBA) | ((virtual_address & LA_PAG) >> 10);

			return std::nullopt;
		}
		if ((pgd.value() & PGD_SP) && !supervisor)
		{
			m_pfsr = PFSR_SV;
			m_pfar = (sgd.value() & SGD_PTBA) | ((virtual_address & LA_PAG) >> 10);

			return std::nullopt;
		}

		// check write protect
		if (write && ((sgd.value() | pgd.value()) & PGD_WP))
		{
			m_pfsr = PFSR_WV;
			return std::nullopt;
		}

		// update page descriptor used and modified bits
		if (!(pgd.value() & PGD_U) || (write && !(pgd.value() & PGD_M)))
		{
			pgd.value() |= (write ? PGD_M : 0) | PGD_U;

			if (!mbus_write((sgd.value() & SGD_PTBA) | ((virtual_address & LA_PAG) >> 10), pgd.value()))
				return std::nullopt;
		}

		if (!patc_hit)
		{
			// create patc entry (lpa,pfa,s,wt,g,ci,m,wp)
			m_patc[m_patc_ptr++] = PATC_V | (u64(virtual_address & ~LA_OFS) << 14) | ((pgd.value() & PGD_PFA) >> 6) | (supervisor ? PATC_S : 0) | bitswap<u64>(apr | sgd.value() | pgd.value(), 9, 7, 6, 4, 2);
			if (m_patc_ptr == std::size(m_patc))
				m_patc_ptr = 0;
		}

		return translate_result((pgd.value() & PGD_PFA) | (virtual_address & LA_OFS),
			(apr | sgd.value() | pgd.value()) & PGD_CI,
			(apr | sgd.value() | pgd.value()) & PGD_G,
			(apr | sgd.value() | pgd.value()) & PGD_WT);
	}
	else
	{
		/*
		 * The user manual states that the hardwired BATC entries are used in
		 * supervisor mode even when translation is disabled.
		 *
		 * Despite statements indicating that CMMU control space should be part
		 * of the supervisor address space, the MVME181 firmware "Cache Inhibit
		 * Bits" diagnostic expects that CMMU registers also be accessible from
		 * user mode when translation is disabled.
		 *
		 * The following logic assumes that the hardwired entries are applied
		 * when translation is disabled without regard to the active mode,
		 * ensuring cache inhibit is activated.
		 */
		for (unsigned i = 8; i < 10; i++)
			if (BIT(virtual_address, 19, 13) == BIT(m_batc[i], 19, 13))
				return translate_result(((m_batc[i] & BATC_PBA) << 13) | (virtual_address & ~BATC_LBA), m_batc[i] & BATC_CI, m_batc[i] & BATC_G, m_batc[i] & BATC_WT);

		return translate_result(virtual_address, apr & APR_CI, apr & APR_G, apr & APR_WT);
	}
}

void mc88200_device::cache_set::set_mru(unsigned const line)
{
	static const struct
	{
		u32 clr;
		u32 set;
	}
	flags[] =
	{
		{ CSSP_L3 | CSSP_L1 | CSSP_L0, 0                           },
		{ CSSP_L4 | CSSP_L2,           CSSP_L0                     },
		{ CSSP_L5,                     CSSP_L2 | CSSP_L1           },
		{ 0,                           CSSP_L5 | CSSP_L4 | CSSP_L3 },
	};

	status = (status & ~flags[line].clr) | flags[line].set;
}

void mc88200_device::cache_set::set_unmodified(unsigned const line)
{
	status &= ~(INV << (12 + line * 2));
}

void mc88200_device::cache_set::set_modified(unsigned const line)
{
	status &= ~(INV << (12 + line * 2));
	status |= EXM << (12 + line * 2);
}

void mc88200_device::cache_set::set_shared(unsigned const line)
{
	status &= ~(INV << (12 + line * 2));
	status |= SHU << (12 + line * 2);
}

void mc88200_device::cache_set::set_invalid(unsigned const line)
{
	status |= INV << (12 + line * 2);
}

bool mc88200_device::cache_set::modified(unsigned const line) const
{
	return BIT(status, 12 + line * 2, 2) == EXM;
}

bool mc88200_device::cache_set::shared(unsigned const line) const
{
	return BIT(status, 12 + line * 2, 2) == SHU;
}

bool mc88200_device::cache_set::invalid(unsigned const line) const
{
	return BIT(status, 12 + line * 2, 2) == INV;
}

bool mc88200_device::cache_set::enabled(unsigned const line) const
{
	return !BIT(status, 20 + line);
}

bool mc88200_device::cache_set::cache_line::match_segment(u32 const address) const
{
	return BIT(tag, 22, 10) == BIT(address, 22, 10);
}

bool mc88200_device::cache_set::cache_line::match_page(u32 const address) const
{
	return BIT(tag, 12, 20) == BIT(address, 12, 20);
}

bool mc88200_device::cache_set::cache_line::load_line(mc88200_device &cmmu, u32 const address)
{
	for (unsigned i = 0; i < 4; i++)
	{
		std::optional<u32> const read = cmmu.mbus_read<u32>(address | i * 4);

		if (read.has_value())
			data[i] = read.value();
		else
			return false;
	}

	return true;
}

bool mc88200_device::cache_set::cache_line::copy_back(mc88200_device &cmmu, u32 const address, bool const flush)
{
	for (unsigned i = 0; i < 4; i++)
		if (!cmmu.mbus_write(address | i * 4, data[i], flush))
			return false;

	return true;
}

std::optional<unsigned> mc88200_device::cache_replace(cache_set const &cs)
{
	// check for enabled invalid lines
	for (unsigned l = 0; l < std::size(cs.line); l++)
		if (cs.enabled(l) && cs.invalid(l))
			return l;

	/*
	 * This table encodes the cache line usage sequence for each combination of
	 * LRU flags. A zero value indicates an invalid flag combination, otherwise
	 * the two most-significant bits correspond to the most recently-used line
	 * and the two least-significant bits correspond to the least recently-used.
	 */
	static u8 const usage_table[] =
	{
		0x1b, 0x4b, 0x00, 0x63, 0x27, 0x00, 0x87, 0x93, // 00-07
		0x00, 0x00, 0x00, 0x6c, 0x00, 0x00, 0x00, 0x9c, // 08-0f
		0x00, 0x00, 0x00, 0x00, 0x2d, 0x00, 0x8d, 0x00, // 10-17
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xb1, 0xb4, // 18-1f
		0x1e, 0x4e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 20-27
		0x00, 0x72, 0x00, 0x78, 0x00, 0x00, 0x00, 0x00, // 28-2f
		0x36, 0x00, 0x00, 0x00, 0x39, 0x00, 0x00, 0x00, // 30-37
		0xc6, 0xd2, 0x00, 0xd8, 0xc9, 0x00, 0xe1, 0xe4, // 38-3f
	};

	u8 const usage = usage_table[BIT(cs.status, 24, 6)];
	if (usage)
	{
		// find least-recently used enabled line
		for (unsigned i = 0; i < 4; i++)
		{
			unsigned const l = BIT(usage, i * 2, 2);

			if (cs.enabled(i))
				return l;
		}
	}

	// invalid flags or no enabled lines
	return std::nullopt;
}

void mc88200_device::cache_flush(unsigned const start, unsigned const limit, match_function match, bool const copyback, bool const invalidate)
{
	for (unsigned s = start; s < limit; s++)
	{
		cache_set &cs = m_cache[s];

		for (unsigned l = 0; l < std::size(cs.line); l++)
		{
			if (!match || std::invoke(match, cs.line[l], m_sar))
			{
				// copy back
				if (copyback && cs.modified(l))
					if (!cs.line[l].copy_back(*this, cs.line[l].tag | (s << 4), true))
						return;

				// invalidate
				if (invalidate)
					cs.set_invalid(l);
			}
		}
	}
}

template <typename T> std::optional<T> mc88200_device::read(u32 virtual_address, bool supervisor)
{
	std::optional<mc88200_device::translate_result> result = translate(virtual_address, supervisor, false);
	if (!result.has_value())
		return std::nullopt;

	u32 const physical_address = result.value().address;
	if (!result.value().ci)
	{
		unsigned const s = BIT(physical_address, 4, 8);
		cache_set &cs = m_cache[s];

		for (unsigned l = 0; l < std::size(cs.line); l++)
		{
			// cache line hit: tag match, enabled, not invalid
			if (cs.line[l].match_page(physical_address) && cs.enabled(l) && !cs.invalid(l))
			{
				// set most recently used
				cs.set_mru(l);

				// return data
				u32 const data = cs.line[l].data[BIT(physical_address, 2, 2)];

				switch (sizeof(T))
				{
				case 1: return T(data >> (24 - (physical_address & 3) * 8));
				case 2: return T(data >> (16 - (physical_address & 3) * 8));
				case 4: return T(data);
				}
			}
		}

		// select cache line for replacement
		std::optional<unsigned> const l = cache_replace(cs);

		if (l.has_value())
		{
			// copy back modified line
			if (cs.modified(l.value()))
				if (!cs.line[l.value()].copy_back(*this, cs.line[l.value()].tag | (s << 4)))
					return std::nullopt;

			// mark line invalid
			cs.set_invalid(l.value());

			// update tag
			cs.line[l.value()].tag = physical_address & ~LA_OFS;

			// load line from memory
			if (!cs.line[l.value()].load_line(*this, physical_address & 0xfffffff0U))
				return std::nullopt;

			// mark line shared unmodified
			cs.set_shared(l.value());

			// set most recently used
			cs.set_mru(l.value());

			// return data
			u32 const data = cs.line[l.value()].data[BIT(physical_address, 2, 2)];

			switch (sizeof(T))
			{
			case 1: return T(data >> (24 - (physical_address & 3) * 8));
			case 2: return T(data >> (16 - (physical_address & 3) * 8));
			case 4: return T(data);
			}
		}
	}

	return mbus_read<T>(physical_address);
}

template <typename T> bool mc88200_device::write(u32 virtual_address, T data, bool supervisor)
{
	std::optional<mc88200_device::translate_result> result = translate(virtual_address, supervisor, true);
	if (!result.has_value())
		return false;

	u32 const physical_address = result.value().address;
	if (!result.value().ci)
	{
		unsigned const s = BIT(physical_address, 4, 8);
		cache_set &cs = m_cache[s];

		for (unsigned l = 0; l < std::size(cs.line); l++)
		{
			// cache line hit: tag match, enabled, not invalid
			if (cs.line[l].match_page(physical_address) && cs.enabled(l) && !cs.invalid(l))
			{
				// write data to cache
				u32 &cache_data = cs.line[l].data[BIT(physical_address, 2, 2)];
				switch (sizeof(T))
				{
				case 1: cache_data = (cache_data & ~(0x000000ffU << (24 - (physical_address & 3) * 8))) | (u32(data) << (24 - (physical_address & 3) * 8)); break;
				case 2: cache_data = (cache_data & ~(0x0000ffffU << (16 - (physical_address & 3) * 8))) | (u32(data) << (16 - (physical_address & 3) * 8)); break;
				case 4: cache_data = data; break;
				}

				// set most recently used
				cs.set_mru(l);

				// write data to memory
				if (result.value().wt || result.value().g)
					if (!mbus_write(physical_address, data))
						return false;

				// update line status
				if (cs.shared(l))
				{
					if (result.value().g)
						cs.set_unmodified(l);
					else if (!result.value().wt)
						cs.set_modified(l);
				}
				else
					cs.set_modified(l);

				return true;
			}
		}

		// select cache line for replacement
		std::optional<unsigned> const l = cache_replace(cs);

		if (l.has_value())
		{
			// copy back modified line
			if (cs.modified(l.value()))
				if (!cs.line[l.value()].copy_back(*this, cs.line[l.value()].tag | (s << 4)))
					return false;

			// mark line invalid
			cs.set_invalid(l.value());

			// load line from memory
			if (!cs.line[l.value()].load_line(*this, physical_address & 0xfffffff0U))
				return false;
		}

		// write data to memory
		if (!mbus_write(physical_address, data))
			return false;

		if (l.has_value())
		{
			// update tag
			cs.line[l.value()].tag = physical_address & ~LA_OFS;

			// write data into cache
			u32 &cache_data = cs.line[l.value()].data[BIT(physical_address, 2, 2)];
			switch (sizeof(T))
			{
			case 1: cache_data = (cache_data & ~(0x000000ffU << (24 - (physical_address & 3) * 8))) | (u32(data) << (24 - (physical_address & 3) * 8)); break;
			case 2: cache_data = (cache_data & ~(0x0000ffffU << (16 - (physical_address & 3) * 8))) | (u32(data) << (16 - (physical_address & 3) * 8)); break;
			case 4: cache_data = data; break;
			}

			// mark line exclusive unmodified
			cs.set_unmodified(l.value());

			// set most recently used
			cs.set_mru(l.value());
		}

		return true;
	}

	return mbus_write(result.value().address, data);
}

template std::optional<u8> mc88200_device::read(u32 virtual_address, bool supervisor);
template std::optional<u16> mc88200_device::read(u32 virtual_address, bool supervisor);
template std::optional<u32> mc88200_device::read(u32 virtual_address, bool supervisor);

template bool mc88200_device::write(u32 virtual_address, u8 data, bool supervisor);
template bool mc88200_device::write(u32 virtual_address, u16 data, bool supervisor);
template bool mc88200_device::write(u32 virtual_address, u32 data, bool supervisor);

template <typename T> std::optional<T> mc88200_device::mbus_read(u32 address)
{
	std::optional<T> data;

	m_bus_error = false;

	switch (sizeof(T))
	{
	case 1: data = m_mbus->read_byte(address); break;
	case 2: data = m_mbus->read_word(address); break;
	case 4: data = m_mbus->read_dword(address); break;
	}

	if (m_bus_error)
	{
		if (!machine().side_effects_disabled())
		{
			m_pfar = address;
			m_pfsr = PFSR_BE;
		}
		data = std::nullopt;
	}

	return data;
}

template <typename T> bool mc88200_device::mbus_write(u32 address, T data, bool flush)
{
	m_bus_error = false;

	switch (sizeof(T))
	{
	case 1: m_mbus->write_byte(address, data); break;
	case 2: m_mbus->write_word(address, data); break;
	case 4: m_mbus->write_dword(address, data); break;
	}

	if (m_bus_error)
	{
		if (!flush)
		{
			m_pfar = address;
			m_pfsr = PFSR_BE;
		}
		else
		{
			m_sar = address;
			m_ssr |= SSR_BE;
		}

		return false;
	}
	else
		return true;
}
