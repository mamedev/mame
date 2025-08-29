// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * IBM Rosetta MMU.
 *
 * Sources:
 *   - IBM RT PC Hardware Technical Reference Volume I, 75X0232, March 1987
 *
 * TODO:
 *   - tighten error detection/reporting
 *   - external device and multiple errors
 *   - ras diagnostic modes
 *   - advanced/enhanced variants
 *   - ram holes
 */

#include "emu.h"
#include "rosetta.h"

#define LOG_TLB     (1U << 1)
#define LOG_RELOAD  (1U << 2)
#define LOG_ECC     (1U << 3)
#define LOG_INVALID (1U << 4)
#define LOG_LED     (1U << 5)

//#define VERBOSE (LOG_GENERAL|LOG_TLB|LOG_RELOAD|LOG_ECC|LOG_INVALID)
#include "logmacro.h"

enum roms_mask : u32
{
	ROMS_SIZE  = 0x0000'000f, // rom size
	ROMS_START = 0x0000'0ff0, // rom address
	ROMS_P     = 0x0000'1000, // parity enable
};

enum segment_mask : u32
{
	SEGMENT_K  = 0x0000'0001, // key
	SEGMENT_S  = 0x0000'0002, // special
	SEGMENT_ID = 0x0000'3ffc, // identifier
	SEGMENT_I  = 0x0000'4000, // i/o access protect
	SEGMENT_R  = 0x0000'8000, // system processor access protect
	SEGMENT_P  = 0x0001'0000, // present
};

enum rams_mask : u32
{
	RAMS_SIZE  = 0x0000'000f, // ram size
	RAMS_START = 0x0000'0ff0, // ram address
};

enum tcr_mask : u32
{
	TCR_HIB = 0x0000'00ff, // hat/ipt base address
	TCR_S   = 0x0000'0100, // page size (1=4k pages)
	TCR_P   = 0x0000'0200, // r/c array parity enable (advanced/enhanced only?)
	TCR_R   = 0x0000'0400, // enable interrupt on successful tlb reload
	TCR_C   = 0x0000'0800, // enable interrupt on correctable ecc error
	TCR_I   = 0x0000'1000, // terminate long ipt search
	TCR_D   = 0x0000'2000, // enable ras diagnostic mode
	TCR_E   = 0x0000'4000, // interrupt on successful parity error retry
	TCR_V   = 0x0000'8000, // segment register zero virtual equal to real
};

enum mer_mask : u32
{
	MER_D = 0x0000'0001, // data
	MER_P = 0x0000'0002, // protection
	MER_S = 0x0000'0004, // tlb specification
	MER_F = 0x0000'0008, // page fault
	MER_M = 0x0000'0010, // multiple exception
	MER_E = 0x0000'0020, // external device exception
	MER_I = 0x0000'0040, // ipt specification error
	MER_W = 0x0000'0080, // write to rom
	MER_R = 0x0000'0100, // r/c array parity error (advanced/enhanced only?)
	MER_T = 0x0000'0200, // successful tlb reload
	MER_C = 0x0000'0400, // correctable ecc error
	MER_U = 0x0000'0800, // uncorrectable memory error
	MER_L = 0x0000'1000, // access type (1=load)
	MER_O = 0x0000'2000, // invalid i/o address
	MER_B = 0x0000'4000, // invalid memory address
	MER_N = 0x0000'8000, // processor channel nakd
	MER_A = 0x0001'0000, // processor channel ackd
	MER_V = 0x0002'0000, // segment protection violation
};

enum trar_mask : u32
{
	TRAR_A = 0x00ff'ffff, // real memory address
	TRAR_I = 0x8000'0000, // invalid
};

enum rmdr_mask : u32
{
	RMDR_CHECK = 0x0000'ff00, // array check bits
	RMDR_ALT   = 0x0000'00ff, // alternate check bits
};

enum tlb_mask : u32
{
	// field 0
	TLB_SEG  = 0x1ffe'0000, // segment identifier
	TLB_AT2K = 0x1fff'fff0, // address tag (2k page)
	TLB_AT4K = 0x1fff'ffe0, // address tag (4k page)

	// field 1
	TLB_KEY   = 0x0000'0003, // key bits
	TLB_V     = 0x0000'0004, // valid bit
	TLB_RPN2K = 0x0000'fff8, // real page number (2k page)
	TLB_RPN4K = 0x0000'fff0, // real page number (4k page)

	// field 2
	TLB_LB    = 0x0000'ffff, // lock bits
	TLB_TID   = 0x00ff'0000, // transaction identifier
	TLB_W     = 0x0100'0000, // write bit

};

enum hat_mask : u32
{
	HAT_AT2K = 0x1fff'ffff, // address tag (2k page)
	HAT_AT4K = 0x1fff'fffe, // address tag (4k page)
	HAT_KEY  = 0xc000'0000, // key

	HAT_IPTP = 0x0000'1fff, // ipt pointer
	HAT_L    = 0x0000'8000, // last
	HAT_HATP = 0x1fff'0000, // hat pointer
	HAT_E    = 0x8000'0000, // empty

	HAT_LB   = 0x0000'ffff, // lock bits
	HAT_TID  = 0x00ff'0000, // transaction identifier
	HAT_W    = 0x0100'0000, // write protect
};

enum rca_mask : u8
{
	RCA_C = 0x01, // change
	RCA_R = 0x02, // reference
};

enum reg : unsigned
{
	IOBA = 0, // i/o base address
	MER  = 1, // memory exception
	MEAR = 2, // memory exception address
	TRAR = 3, // translated real address
	TID  = 4, // transaction identifier
	TCR  = 5, // translation control
	RAMS = 6, // ram specification
	ROMS = 7, // rom specification
	RMDR = 8, // ras mode diagnostic
};

static char const *const control_names[] = { "IOBA", "MER", "MEAR", "TRAR", "TID", "TCR", "RAMS", "ROMS", "RMDR" };

static u8 const ecc_bits[] =
{
	0xa8, 0x68, 0xa4, 0x64, 0xa2, 0x62, 0xa1, 0x61,
	0x98, 0x58, 0x94, 0x54, 0x92, 0x52, 0x91, 0x51,
	0x8a, 0x89, 0x4a, 0x49, 0x2a, 0x29, 0x1a, 0x19,
	0x86, 0x85, 0x46, 0x45, 0x26, 0x25, 0x16, 0x15,
};

// 7-segment diagnostic led
static u8 const led_pattern[16] =
{
	0x3f, 0x06, 0x5b, 0x4f,
	0x66, 0x6d, 0x7d, 0x07,
	0x7f, 0x6f, 0x77, 0x7c,
	0x39, 0x5e, 0x79, 0x00,
};

DEFINE_DEVICE_TYPE(ROSETTA, rosetta_device, "rosetta", "IBM Rosetta")

ALLOW_SAVE_TYPE(rosetta_device::mear_state)

rosetta_device::rosetta_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock, ram_size ram)
	: device_t(mconfig, ROSETTA, tag, owner, clock)
	, rsc_cpu_interface(mconfig, *this)
	, m_mem_space(*this, finder_base::DUMMY_TAG, -1, 32)
	, m_rom(*this, finder_base::DUMMY_TAG)
	, m_leds(*this, "led%u", 0U)
	, m_out_pchk(*this)
	, m_out_mchk(*this)
	, m_segment{}
	, m_control{}
	, m_mear_lock(UNLOCKED)
	, m_rmdr_lock(false)
	, m_led_lock(true)
	, m_pchk_state(false)
	, m_tlb{ {}, {} }
	, m_tlb_lru(0)
	, m_ram_size(ram)
{
}

void rosetta_device::device_validity_check(validity_checker &valid) const
{
	if (!m_ram_size)
		osd_printf_error("invalid ram size\n");
}

void rosetta_device::device_start()
{
	m_leds.resolve();

	save_item(NAME(m_segment));
	save_item(NAME(m_control));
	save_item(NAME(m_mear_lock));
	save_item(NAME(m_rmdr_lock));
	save_item(NAME(m_led_lock));
	save_item(NAME(m_pchk_state));

	save_item(STRUCT_MEMBER(m_tlb, field0));
	save_item(STRUCT_MEMBER(m_tlb, field1));
	save_item(STRUCT_MEMBER(m_tlb, field2));
	save_item(NAME(m_tlb_lru));

	save_pointer(NAME(m_ram), m_ram_size);
	save_pointer(NAME(m_ecc), m_ram_size);
	save_pointer(NAME(m_rca), 2048);

	config_tlb();

	m_ram = std::make_unique<u32[]>(m_ram_size);
	m_ecc = std::make_unique<u8[]>(m_ram_size);
	m_rca = std::make_unique<u8[]>(2048);

	m_mem_space->cache(m_mem);
}

void rosetta_device::device_reset()
{
	m_mear_lock = UNLOCKED;
	m_rmdr_lock = false;
	m_led_lock = true;

	set_pchk(false);
	set_mchk(false);
}

void rosetta_device::device_post_load()
{
	config_tlb();
}

bool rosetta_device::ior(u32 address, u32 &data)
{
	if ((address >> 16) == u8(m_control[IOBA]))
	{
		u16 const offset = u16(address);

		switch (offset)
		{
		case 0x00: case 0x01: case 0x02: case 0x03:
		case 0x04: case 0x05: case 0x06: case 0x07:
		case 0x08: case 0x09: case 0x0a: case 0x0b:
		case 0x0c: case 0x0d: case 0x0e: case 0x0f:
			data = segment_r(offset & 0xf);
			return true;

		case 0x10: case 0x11: case 0x12: case 0x13:
		case 0x14: case 0x15: case 0x16: case 0x17:
		case 0x18:
			data = control_r(offset & 0xf);
			return true;

		default:
			if (offset - 0x20U < 0x60U)
			{
				data = tlb_r(offset - 0x20U);
				return true;
			}
			else if (offset - 0x1000U < 0x2000U)
			{
				data = rca_r(offset - 0x1000U);
				return true;
			}
			else
			{
				// invalid i/o address logic only applies to accesses within the mapped range
				LOGMASKED(LOG_INVALID, "ior invalid address 0x%06x (%s)\n", address, machine().describe_context());
				m_control[MER] |= MER_O;
				set_mear(address, MEMORY);

				return bool(m_control[TCR] & TCR_D);
			}
			break;
		}
	}
	else if (address == 0x80'8000U)
	{
		data = m_control[IOBA];
		return true;
	}

	LOGMASKED(LOG_INVALID, "ior unknown address 0x%06x (%s)\n", address, machine().describe_context());
	return false;
}

bool rosetta_device::iow(u32 address, u32 data)
{
	if ((address >> 16) == u8(m_control[IOBA]))
	{
		u16 const offset = u16(address);

		switch (offset)
		{
		case 0x00: case 0x01: case 0x02: case 0x03:
		case 0x04: case 0x05: case 0x06: case 0x07:
		case 0x08: case 0x09: case 0x0a: case 0x0b:
		case 0x0c: case 0x0d: case 0x0e: case 0x0f:
			segment_w(offset & 0xf, data);
			return true;

		case 0x10: case 0x11: case 0x12: case 0x13:
		case 0x14: case 0x15: case 0x16: case 0x17:
		case 0x18:
			control_w(offset & 0xf, data);
			return true;

		case 0x80: tlb_inv_all(data); return true;
		case 0x81: tlb_inv_segment(data); return true;
		case 0x82: tlb_inv_address(data); return true;
		case 0x83: compute_address(data); return true;

		default:
			if (offset - 0x20U < 0x60U)
			{
				tlb_w(offset - 0x20U, data);
				return true;
			}
			else if (offset - 0x1000U < 0x2000U)
			{
				rca_w(offset - 0x1000U, data);
				return true;
			}
			else
			{
				// invalid i/o address logic only applies to accesses within the mapped range
				LOGMASKED(LOG_INVALID, "iow invalid address 0x%06x data 0x%08x (%s)\n", address, data, machine().describe_context());
				m_control[MER] |= MER_O;
				set_mear(address, MEMORY);
				set_pchk(true);

				return true;
			}
			break;
		}
	}
	else if (address == 0x80'8000U)
	{
		m_control[IOBA] = data;
		return true;
	}

	LOGMASKED(LOG_INVALID, "iow unknown address 0x%06x data 0x%08x (%s)\n", address, data, machine().describe_context());
	return true;
}

// translate logical to physical address ignoring protection and without side effects
bool rosetta_device::translate(u32 &address) const
{
	unsigned const segment = address >> 28;

	// segment present
	if (!(m_segment[segment] & SEGMENT_P))
		return false;

	// segment zero virtual equal to real
	if ((m_control[TCR] & TCR_V) && !segment)
	{
		address &= 0x00ff'ffffU;

		return true;
	}

	u64 const virtual_address = (u64(m_segment[segment] & SEGMENT_ID) << 26) | (address & 0x0fff'ffffU);

	// ignore tlb and directly search inverted page table
	u32 page_address = 0;
	{
		// compute hat index
		u16 hat_offset = (((virtual_address >> 26) & SEGMENT_ID) ^ (virtual_address >> (m_page_shift - 2))) & m_hat_mask;

		// fetch hat entry
		u32 hat_entry = m_ram[m_hat_base + hat_offset + 1];

		// page fault
		if (hat_entry & HAT_E)
			return false;

		// compute ipt entry pointer
		hat_offset = ((hat_entry & HAT_HATP) >> 14);
		u32 const address = (virtual_address >> 11) & m_atag_mask;

		// ipt search
		for (unsigned count = 0; count < 1024; count++)
		{
			// fetch ipt entry
			u32 const ipt_entry = m_ram[m_hat_base + hat_offset + 0];

			if ((ipt_entry & m_atag_mask) == address)
			{
				// reload tlb
				page_address = (hat_offset << 1) | TLB_V | (ipt_entry >> 30);

				// found
				break;
			}

			// fetch next hat entry
			hat_entry = m_ram[m_hat_base + hat_offset + 1];
			if (hat_entry & HAT_L)
				return false;

			// select next ipt entry
			hat_offset = (hat_entry & HAT_IPTP) << 2;
		}

		// search failed
		if (!page_address)
			return false;
	}

	u32 const real_page = (page_address << 8) & m_page_mask;

	address = real_page | (address & ~m_page_mask);

	return true;
}

bool rosetta_device::translate(u32 &address, bool system_processor, bool store)
{
	unsigned const segment = address >> 28;

	// segment present
	if (!(m_segment[segment] & SEGMENT_P))
		fatalerror("rosetta_device::translate() segment %d absent (%s)\n", segment, machine().describe_context());

	// segment access protection
	if (system_processor && (m_segment[segment] & SEGMENT_R))
	{
		m_control[MER] |= MER_V;

		return false;
	}
	else if (!system_processor && (m_segment[segment] & SEGMENT_I))
		return false;

	// segment zero virtual equal to real
	if ((m_control[TCR] & TCR_V) && !segment)
	{
		address &= 0x00ff'ffffU;

		return true;
	}

	u64 const virtual_address = (u64(m_segment[segment] & SEGMENT_ID) << 26) | (address & 0x0fff'ffffU);

	tlb_entry const te = tlb_search(virtual_address, m_segment[segment] & SEGMENT_S);
	if (!(te.field1 & TLB_V))
		return false;

	u32 const real_page = (te.field1 << 8) & m_page_mask;

	// special segment lockbit processing
	if (m_segment[segment] & SEGMENT_S)
	{
		// check transaction identifier
		if ((m_control[TID] & 0xff) != ((te.field2 & TLB_TID) >> 16))
		{
			if (system_processor)
				m_control[MER] |= MER_D;

			return false;
		}

		// check write bit, lock bit and operation
		bool const lockbit = BIT(te.field2, 15 - ((address >> (m_page_shift - 4)) & 15));
		if (te.field2 & TLB_W)
		{
			if (!lockbit && store)
			{
				if (system_processor)
					m_control[MER] |= MER_D;

				return false;
			}
		}
		else
		{
			if (!lockbit || store)
			{
				if (system_processor)
					m_control[MER] |= MER_D;

				return false;
			}
		}
	}
	else
	{
		// non-special segment memory protection processing
		switch (te.field1 & TLB_KEY)
		{
		case 0:
			// key 0 fetch-protected
			if (m_segment[segment] & SEGMENT_K)
			{
				if (system_processor)
					m_control[MER] |= MER_P;

				return false;
			}
			break;
		case 1:
			// key 0 read/write
			if ((m_segment[segment] & SEGMENT_K) && store)
			{
				if (system_processor)
					m_control[MER] |= MER_P;

				return false;
			}
			break;
		case 2:
			// public read/write
			break;
		case 3:
			// public read-only
			if (store)
			{
				if (system_processor)
					m_control[MER] |= MER_P;

				return false;
			}
			break;
		}
	}

	address = real_page | (address & ~m_page_mask);

	return true;
}

rosetta_device::tlb_entry rosetta_device::tlb_search(u64 const virtual_address, bool const special)
{
	unsigned const tlb_index = (virtual_address >> m_page_shift) & 15;
	tlb_entry te = { 0U, 0U, 0U };

	// check first set
	if ((m_tlb[tlb_index][0].field1 & TLB_V) && !((m_tlb[tlb_index][0].field0 ^ (virtual_address >> 11)) & m_atag_mask))
	{
		te = m_tlb[tlb_index][0];

		// tlb set 1 is least-recently used
		m_tlb_lru |= (1U << tlb_index);
	}

	// check second set
	if ((m_tlb[tlb_index][1].field1 & TLB_V) && !((m_tlb[tlb_index][1].field0 ^ (virtual_address >> 11)) & m_atag_mask))
	{
		if (!(te.field1 & TLB_V))
		{
			te = m_tlb[tlb_index][1];

			// tlb set 0 is least-recently used
			m_tlb_lru &= ~(1U << tlb_index);
		}
		else
		{
			// double tlb set hit
			m_control[MER] |= MER_S;

			fatalerror("rosetta_device::tlb_search() double tlb hit 0x%010x (%s)\n",
				virtual_address, machine().describe_context());

			if (!(m_control[TCR] & TCR_D))
			{
				set_mchk(true);

				return tlb_entry{ 0U, 0U, 0U };
			}
			else
				// if store, address is logical OR of both real page numbers
				// TODO: what about load?
				te.field1 |= m_tlb[tlb_index][1].field1;
		}
	}

	// handle tlb miss
	if (!(te.field1 & TLB_V))
	{
		unsigned const lru = BIT(m_tlb_lru, tlb_index);

		u32 const mer = tlb_reload(te, virtual_address, special);
		if (mer & (MER_T | MER_I | MER_F))
		{
			// FIXME: set I&F only for system processor
			m_control[MER] |= mer;

			return te;
		}
		else
			m_tlb[tlb_index][lru] = te;
	}

	return te;
}

u32 rosetta_device::tlb_reload(rosetta_device::tlb_entry &tlb_entry, u64 const virtual_address, bool special)
{
	// compute hat index
	u16 hat_offset = (((virtual_address >> 26) & SEGMENT_ID) ^ (virtual_address >> (m_page_shift - 2))) & m_hat_mask;

	// fetch hat entry
	u32 hat_entry = m_ram[m_hat_base + hat_offset + 1];

	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_RELOAD, "reload 0x%010x hat base 0x%x index 0x%x entry 0x%08x\n",
			virtual_address, m_hat_base, hat_offset, hat_entry);

	// set reference bit
	m_rca[(m_hat_base + hat_offset) >> 11] |= (RCA_R << (((m_hat_base + hat_offset) >> 8) & 6));

	if (hat_entry & HAT_E)
		return MER_F;

	// compute ipt entry pointer
	hat_offset = ((hat_entry & HAT_HATP) >> 14);
	u32 const address = (virtual_address >> 11) & m_atag_mask;

	// ipt search
	for (unsigned count = 0; count < 1024; count++)
	{
		// fetch ipt entry
		u32 const ipt_entry = m_ram[m_hat_base + hat_offset + 0];

		// set reference bit
		m_rca[(m_hat_base + hat_offset) >> 11] |= (RCA_R << (((m_hat_base + hat_offset) >> 8) & 6));

		if ((ipt_entry & m_atag_mask) == address)
		{
			// reload tlb
			tlb_entry.field0 = ipt_entry & m_atag_mask;
			tlb_entry.field1 = (hat_offset << 1) | TLB_V | (ipt_entry >> 30);

			if (special)
				tlb_entry.field2 = m_ram[m_hat_base + hat_offset + 2];

			if (!machine().side_effects_disabled())
				LOGMASKED(LOG_RELOAD, "reload complete count %d f0 0x%08x f1 0x%08x f2 0x%08x\n",
					count, tlb_entry.field0, tlb_entry.field1, tlb_entry.field2);

			if (m_control[TCR] & TCR_R)
				return MER_T;

			return 0;
		}

		// terminate long ipt search
		if ((m_control[TCR] & TCR_I) && (count == 127))
		{
			LOGMASKED(LOG_RELOAD, "reload long search abort\n");

			return MER_I;
		}

		// fetch next hat entry
		hat_entry = m_ram[m_hat_base + hat_offset + 1];
		if (hat_entry & HAT_L)
		{
			LOGMASKED(LOG_RELOAD, "reload fault\n");

			return MER_F;
		}

		// select next ipt entry
		hat_offset = (hat_entry & HAT_IPTP) << 2;
	}

	fatalerror("rosetta_device::reload() endless loop detected\n");
}

void rosetta_device::set_mear(u32 const address, mear_state lock)
{
	if (m_mear_lock == LOCKED)
		return;

	if (m_mear_lock == MEMORY && lock == MEMORY)
		return;

	m_control[MEAR] = address;
	m_mear_lock = lock;
}

void rosetta_device::set_rmdr(u8 const ecc, bool lock)
{
	if (m_rmdr_lock)
		return;

	m_control[RMDR] = (m_control[RMDR] & ~RMDR_CHECK) | (ecc << 8);
	m_rmdr_lock = lock;
}

u32 rosetta_device::segment_r(offs_t offset)
{
	return m_segment[offset];
}

void rosetta_device::segment_w(offs_t offset, u32 data)
{
	LOG("segment_w 0x%x data 0x%x (%s)\n", offset, data, machine().describe_context());

	m_segment[offset] = data;
}

u32 rosetta_device::control_r(offs_t offset)
{
	u32 data = 0;

	switch (offset)
	{
	case MER:
		data = m_control[offset];
		set_pchk(false);
		break;

	case MEAR:
		m_mear_lock = UNLOCKED;
		data = m_control[offset];
		break;

	case RMDR:
		m_mear_lock = UNLOCKED;
		m_rmdr_lock = false;
		data = m_control[offset];
		break;

	default:
		data = m_control[offset];
		break;
	}

	LOG("control_r %s data 0x%08x (%s)\n", control_names[offset], data, machine().describe_context());

	return data;
}

void rosetta_device::control_w(offs_t offset, u32 data)
{
	bool reconfig_tlb = false;
	bool reconfig_map = false;

	LOG("control_w %s data 0x%08x (%s)\n", control_names[offset], data, machine().describe_context());

	switch (offset)
	{
	case MEAR:
		m_mear_lock = UNLOCKED;
		m_control[offset] = data;
		break;

	case TCR:
		reconfig_tlb = (m_control[offset] ^ data) & (TCR_S | TCR_HIB);
		m_control[offset] = data;
		break;

	case RAMS:
		reconfig_tlb = m_control[offset] ^ data;
		reconfig_map = m_control[offset] ^ data;
		m_control[offset] = data;
		break;

	case ROMS:
		reconfig_map = m_control[offset] ^ data;
		m_control[offset] = data;
		break;

	case RMDR:
		// only alternate check bits are writeable
		m_control[offset] = (m_control[offset] & ~RMDR_ALT) | (data & RMDR_ALT);
		break;

	default:
		m_control[offset] = data;
		break;
	}

	if (reconfig_map)
		config_map();

	if (reconfig_tlb)
		config_tlb();
}

u32 rosetta_device::tlb_r(offs_t offset)
{
	unsigned const tlb_set = BIT(offset, 4);
	u32 data = 0;

	switch (offset & 0x60)
	{
	case 0x00: data = m_tlb[offset & 0xf][tlb_set].field0; break;
	case 0x20: data = m_tlb[offset & 0xf][tlb_set].field1; break;
	case 0x40: data = m_tlb[offset & 0xf][tlb_set].field2; break;
	}

	LOG("tlb_r offset %x data %x\n", offset, data);

	return data;
}

void rosetta_device::tlb_w(offs_t offset, u32 data)
{
	unsigned const tlb_set = BIT(offset, 4);

	switch (offset & 0x60)
	{
	case 0x00: m_tlb[offset & 0xf][tlb_set].field0 = data; break;
	case 0x20: m_tlb[offset & 0xf][tlb_set].field1 = data; break;
	case 0x40: m_tlb[offset & 0xf][tlb_set].field2 = data; break;
	}

	LOG("tlb_w offset %x data %x\n", offset, data);
}

u32 rosetta_device::rca_r(offs_t offset)
{
	unsigned const shift = (offset & 3) * 2;

	if (!m_led_lock)
	{
		LOGMASKED(LOG_LED, "led 0x%02x (%s)\n", u8(offset), machine().describe_context());

		m_leds[0] = led_pattern[(offset >> 0) & 15];
		m_leds[1] = led_pattern[(offset >> 4) & 15];
	}

	return (m_rca[offset >> 2] >> shift) & 3;
}

void rosetta_device::rca_w(offs_t offset, u32 data)
{
	unsigned const shift = (offset & 3) * 2;

	m_rca[offset >> 2] &= ~(3 << shift);
	m_rca[offset >> 2] |= (data & 3) << shift;

	m_led_lock = true;
}

u32 rosetta_device::rom_r(offs_t offset, u32 mem_mask)
{
	if (!machine().side_effects_disabled())
		m_led_lock = false;

	return m_rom[offset];
}

void rosetta_device::rom_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_INVALID, "rom_w invalid 0x%06x data 0x%08x mask 0x%08x (%s)\n",
		offset << 2, data, mem_mask, machine().describe_context());

	if (!machine().side_effects_disabled())
		m_control[MER] |= MER_W;
}

void rosetta_device::ram_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (offset < m_ram_size)
	{
		m_ram[offset] = (m_ram[offset] & ~mem_mask) | (data & mem_mask);

		// set reference and change bits
		if (!machine().side_effects_disabled())
			m_rca[offset >> 11] |= ((RCA_R | RCA_C) << ((offset >> 8) & 6));

		m_ecc[offset] = (m_control[TCR] & TCR_D) ? (m_control[RMDR] & RMDR_ALT) : compute_ecc(m_ram[offset]);
	}
	else
	{
		LOGMASKED(LOG_INVALID, "ram_w invalid 0x%06x data 0x%08x mask 0x%08x (%s)\n",
			offset << 2, data, mem_mask, machine().describe_context());

		m_control[MER] |= MER_B;
	}
}

u32 rosetta_device::ram_r(offs_t offset, u32 mem_mask)
{
	if (offset < m_ram_size)
	{
		u32 data = m_ram[offset];

		if (!machine().side_effects_disabled())
		{
			// set reference bit
			m_rca[offset >> 11] |= (RCA_R << ((offset >> 8) & 6));

			u8 const ecc = m_ecc[offset];

			switch (check_ecc(data, ecc))
			{
			case 1:
				// correctable error
				if ((m_control[TCR] & TCR_C) || (m_control[TCR] & TCR_D))
				{
					if (!(m_control[MER] & MER_U))
						m_control[MER] |= MER_C;
					set_rmdr(ecc, true);
				}
				break;
			case 2:
				// uncorrectable error
				m_control[MER] |= MER_U;
				set_rmdr(ecc, true);
				break;
			}
		}

		return data;
	}
	else
	{
		LOGMASKED(LOG_INVALID, "ram_r invalid 0x%06x mask 0x%08x (%s)\n",
			offset << 2, mem_mask, machine().describe_context());

		m_control[MER] |= MER_B;

		return 0;
	}
}

u8 rosetta_device::compute_ecc(u32 const data) const
{
	u8 result = 0;

	for (unsigned i = 0; i < std::size(ecc_bits); i++)
		result ^= BIT(data, 31 - i) ? ecc_bits[i] : 0;

	return result;
}

unsigned rosetta_device::check_ecc(u32 &data, u8 const ecc) const
{
	u8 const error = compute_ecc(data) ^ ecc;

	if (error)
	{
		for (unsigned i = 0; i < std::size(ecc_bits); i++)
		{
			if (error == ecc_bits[i])
			{
				LOGMASKED(LOG_ECC, "check_ecc single-bit error 0x%08x ecc 0x%02x error 0x%02x\n", data, ecc, error);

				// correct error
				data ^= (0x8000'0000U >> i);

				return 1;
			}
		}

		// multiple-bit error
		LOGMASKED(LOG_ECC, "check_ecc multiple-bit error 0x%08x ecc 0x%02x error 0x%02x\n", data, ecc, error);
		return 2;
	}
	else
		return 0;
}

void rosetta_device::config_map()
{
	// unmap everything
	m_mem_space->unmap_readwrite(0x000000, 0xffffff);

	// map rom
	if (m_control[ROMS] & ROMS_SIZE)
	{
		unsigned const shift = (m_control[ROMS] & ROMS_SIZE) > 7 ? (m_control[ROMS] & ROMS_SIZE) - 7 : 0;
		size_t const size = 0x10000ULL << shift;
		unsigned const factor = (m_control[ROMS] & ROMS_START) >> (4 + shift);

		LOG("config rom size %dk map 0x%06x-0x%06x\n", size >> 10, size * factor, size * factor + size - 1);

		/*
		 * IPL code assumes ROM is mirrored within the configured range, making it
		 * difficult to identify under what circumstances an invalid memory access to
		 * the configured ROM address range would be reported.
		 */
		size_t const available = std::min(size, m_rom.bytes());
		m_mem_space->install_readwrite_handler(size * factor, size * factor + available - 1, available - 1, (size - 1) & ~(available - 1), 0,
			read32s_delegate(*this, FUNC(rosetta_device::rom_r)),
			write32s_delegate(*this, FUNC(rosetta_device::rom_w)));
	}

	// map ram
	if (m_control[RAMS] & RAMS_SIZE)
	{
		unsigned const shift = (m_control[RAMS] & RAMS_SIZE) > 7 ? (m_control[RAMS] & RAMS_SIZE) - 7 : 0;
		unsigned const size = 0x10000U << shift;
		unsigned const factor = (m_control[RAMS] & RAMS_START) >> (4 + shift);

		LOG("config ram size %dk map 0x%06x-0x%06x\n", size >> 10, size * factor, size * factor + size - 1);

		m_mem_space->install_readwrite_handler(size * factor, size * factor + size - 1,
			read32s_delegate(*this, FUNC(rosetta_device::ram_r)),
			write32s_delegate(*this, FUNC(rosetta_device::ram_w)));
	}
}

void rosetta_device::config_tlb()
{
	unsigned const ram_size = std::max(m_control[RAMS] & RAMS_SIZE, 7U);

	if (m_control[TCR] & TCR_S)
	{
		m_atag_mask = TLB_AT4K;
		m_page_mask = 0xffff'f000U;
		m_page_shift = 12;

		m_hat_base = (m_control[TCR] & TCR_HIB) << (ram_size - 1);
		m_hat_mask = (1U << (ram_size - 1)) - 4;
	}
	else
	{
		m_atag_mask = TLB_AT2K;
		m_page_mask = 0xffff'f800U;
		m_page_shift = 11;

		m_hat_base = (m_control[TCR] & TCR_HIB) << ram_size;
		m_hat_mask = (1U << ram_size) - 4;
	}

	LOG("config page size %d hat base 0x%08x mask 0x%04x\n", 1 << m_page_shift, m_hat_base, m_hat_mask);
}

void rosetta_device::tlb_inv_all(u32 data)
{
	LOGMASKED(LOG_TLB, "tlb_inv_all (%s)\n", machine().describe_context());

	for (unsigned i = 0; i < 16; i++)
	{
		m_tlb[i][0].field1 &= ~TLB_V;
		m_tlb[i][1].field1 &= ~TLB_V;
	}
}

void rosetta_device::tlb_inv_segment(u32 data)
{
	LOGMASKED(LOG_TLB, "tlb_inv_segment %x (%s)\n", data & 15, machine().describe_context());

	unsigned const identifier = (m_segment[data & 15] & SEGMENT_ID) >> 2;

	for (unsigned i = 0; i < 16; i++)
	{
		if (((m_tlb[i][0].field0 & TLB_SEG) >> 17) == identifier)
			m_tlb[i][0].field1 &= ~TLB_V;

		if (((m_tlb[i][1].field0 & TLB_SEG) >> 17) == identifier)
			m_tlb[i][1].field1 &= ~TLB_V;
	}
}

void rosetta_device::tlb_inv_address(u32 data)
{
	u64 const virtual_address = (u64(m_segment[data >> 28] & SEGMENT_ID) << 26) | (data & 0x0fff'ffffU);
	unsigned const tlb_index = (data >> m_page_shift) & 15;

	if ((m_tlb[tlb_index][0].field1 & TLB_V) && !((m_tlb[tlb_index][0].field0 ^ (virtual_address >> 11)) & m_atag_mask))
	{
		LOGMASKED(LOG_TLB, "tlb_inv_address 0x%08x set 0 (%s)\n", data, machine().describe_context());

		m_tlb[tlb_index][0].field1 &= ~TLB_V;
		m_tlb_lru &= ~(1U << tlb_index);
	}

	if ((m_tlb[tlb_index][1].field1 & TLB_V) && !((m_tlb[tlb_index][1].field0 ^ (virtual_address >> 11)) & m_atag_mask))
	{
		LOGMASKED(LOG_TLB, "tlb_inv_address 0x%08x set 1 (%s)\n", data, machine().describe_context());

		m_tlb[tlb_index][1].field1 &= ~TLB_V;
		m_tlb_lru |= (1U << tlb_index);
	}
}

void rosetta_device::compute_address(u32 data)
{
	unsigned const segment = data >> 28;
	tlb_entry t;

	m_control[TRAR] = TRAR_I;

	if ((m_control[TCR] & TCR_V) && !segment)
		m_control[TRAR] = data & 0x00ff'ffffU;
	else if (!tlb_reload(t, (u64(m_segment[segment] & SEGMENT_ID) << 26) | (data & 0x0fff'ffffU)))
		m_control[TRAR] = (m_control[TCR] & TCR_S)
			? ((t.field1 & TLB_RPN4K) << 8) | (data & 0x0fffU)
			: ((t.field1 & TLB_RPN2K) << 8) | (data & 0x07ffU);
}

bool rosetta_device::fetch(u32 address, u16 &data, rsc_mode const mode)
{
	if (mode & rsc_mode::RSC_T)
	{
		// FIXME: differences between fetch and load
		if (!translate(address, true, false))
			return false;

		// FIXME: unconditionally access RAM
	}

	// access memory and handle errors
	u32 const mer = m_control[MER] & (MER_B | MER_U | MER_C);
	data = m_mem.read_word(address);

	switch ((mer ^ m_control[MER]) & (MER_B | MER_U | MER_C))
	{
	case MER_U:
		set_mear(address, MEMORY);
		if (!(m_control[TCR] & TCR_D))
		{
			set_mchk(true);
			return false;
		}
		break;

	case MER_C:
		set_mear(address, MEMORY);
		if (!(m_control[TCR] & TCR_D))
			set_mchk(true);
		break;

	case MER_B:
		set_mear(address, MEMORY);
		return false;
	}

	return true;
}

template <typename T> bool rosetta_device::load(u32 address, T &data, rsc_mode const mode, bool sp)
{
	if (mode & rsc_mode::RSC_T)
	{
		if (!translate(address, sp, false))
		{
			m_control[MER] |= MER_L;
			if ((sp && (m_control[MER] & (MER_V | MER_I | MER_F | MER_S | MER_P | MER_D))) || (m_control[MER] & (MER_S)))
				set_mear(address, LOCKED);

			return false;
		}

		// FIXME: unconditionally access RAM
	}

	// access memory and handle errors
	u32 const mer = m_control[MER] & (MER_B | MER_U | MER_C);
	switch (sizeof(T))
	{
	case 1: data = m_mem.read_byte(address); break;
	case 2: data = m_mem.read_word(address); break;
	case 4: data = m_mem.read_dword(address); break;
	}

	switch ((mer ^ m_control[MER]) & (MER_B | MER_U | MER_C))
	{
	case MER_U:
		set_mear(address, MEMORY);
		if (!(m_control[TCR] & TCR_D))
		{
			set_mchk(true);
			return false;
		}
		break;

	case MER_C:
		set_mear(address, MEMORY);
		if (!(m_control[TCR] & TCR_D))
			set_mchk(true);
		break;

	case MER_B:
		set_mear(address, MEMORY);
		return false;
	}

	return true;
}

template <typename T> bool rosetta_device::store(u32 address, T data, rsc_mode const mode, bool sp)
{
	if (mode & rsc_mode::RSC_T)
	{
		if (!translate(address, sp, true))
		{
			m_control[MER] &= ~MER_L;
			if ((sp && (m_control[MER] & (MER_V | MER_I | MER_F | MER_S | MER_P | MER_D))) || (m_control[MER] & (MER_S)))
				set_mear(address, LOCKED);

			return false;
		}

		// FIXME: unconditionally access RAM
	}

	// access memory and handle errors
	u32 const mer = m_control[MER] & (MER_B | MER_W);
	switch (sizeof(T))
	{
	case 1: m_mem.write_byte(address, data); break;
	case 2: m_mem.write_word(address, data); break;
	case 4: m_mem.write_dword(address, data); break;
	}

	switch ((mer ^ m_control[MER]) & (MER_B | MER_W))
	{
	case MER_W:
		set_mear(address, MEMORY);
		set_pchk(true);
		break;

	case MER_B:
		set_mear(address, MEMORY);
		if (mode & RSC_T)
			return false;
		else
			set_pchk(true);
		break;
	}

	return true;
}

template <typename T> bool rosetta_device::modify(u32 address, std::function<T(T)> f, rsc_mode const mode)
{
	if (mode & rsc_mode::RSC_T)
	{
		if (!translate(address, true, true))
		{
			m_control[MER] &= ~MER_L;
			if (m_control[MER] & (MER_V | MER_I | MER_F | MER_S | MER_P | MER_D))
				set_mear(address, LOCKED);

			return false;
		}

		// FIXME: unconditionally access RAM
	}

	// access memory and handle errors
	u32 const mer = m_control[MER] & (MER_B | MER_W);
	switch (sizeof(T))
	{
	case 1: m_mem.write_byte(address, f(m_mem.read_byte(address))); break;
	case 2: m_mem.write_word(address, f(m_mem.read_word(address))); break;
	case 4: m_mem.write_dword(address, f(m_mem.read_dword(address))); break;
	}

	switch ((mer ^ m_control[MER]) & (MER_B | MER_W))
	{
	case MER_W:
		set_mear(address, MEMORY);
		set_pchk(true);
		break;

	case MER_B:
		set_mear(address, MEMORY);
		if (mode & RSC_T)
			return false;
		else
			set_pchk(true);
		break;
	}

	return true;
}
