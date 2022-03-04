// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#include "emu.h"

#include "ns32000.h"
#include "ns32000dasm.h"
#include "debugger.h"

DEFINE_DEVICE_TYPE(NS32008, ns32008_device, "ns32008", "National Semiconductor NS32008")
DEFINE_DEVICE_TYPE(NS32016, ns32016_device, "ns32016", "National Semiconductor NS32016")
DEFINE_DEVICE_TYPE(NS32032, ns32032_device, "ns32032", "National Semiconductor NS32032")
DEFINE_DEVICE_TYPE(NS32332, ns32332_device, "ns32332", "National Semiconductor NS32332")

/*
 * TODO:
 *  - prefetch queue
 *  - unimplemented instructions
 *      - format 6: subp,addp
 *  - 32332, 32532
 */

enum psr_mask : u16
{
	// accessible in user mode
	PSR_C   = 0x0001, // carry/borrow condition
	PSR_T   = 0x0002, // trace trap enable
	PSR_L   = 0x0004, // less than condition
					  // unused
					  // unused
	PSR_F   = 0x0020, // general condition
	PSR_Z   = 0x0040, // zero condition
	PSR_N   = 0x0080, // negative condition

	// accessible in supervisor mode
	PSR_U   = 0x0100, // user mode
	PSR_S   = 0x0200, // stack pointer select
	PSR_P   = 0x0400, // prevent multiple trace trap
	PSR_I   = 0x0800, // interrupt enable
					  // unused
					  // unused
					  // unused
					  // unused
	PSR_MSK = 0x0fe7, // write mask
};

enum cfg_mask : u32
{
	CFG_I  = 0x01, // vectored interrupts
	CFG_F  = 0x02, // fpu present
	CFG_M  = 0x04, // mmu present
	CFG_C  = 0x08, // custom coprocessor present
	CFG_FF = 0x10, // (32332 only) fast fpu protocol
	CFG_FM = 0x20, // (32332 only) fast mmu protocol
	CFG_FC = 0x40, // (32332 only) fast custom coprocessor protocol
	CFG_P  = 0x80, // (32332 only) page size >= 4kb
};

enum exception_type : unsigned
{
	NVI   =  0, // non-vectored interrupt
	NMI   =  1, // non-maskable interrupt
	ABT   =  2, // abort
	SLV   =  3, // slave processor
	ILL   =  4, // illegal operation
	SVC   =  5, // supervisor call
	DVZ   =  6, // integer divide by zero
	FLG   =  7, // flag instruction
	BPT   =  8, // breakpoint instruction
	TRC   =  9, // instruction trace
	UND   = 10, // undefined opcode
};

enum st_mask : unsigned
{
	ST_ICI = 0x0, // bus idle (CPU busy)
	ST_ICW = 0x1, // bus idle (CPU wait)
	ST_ISE = 0x3, // bus idle (slave execution)
	ST_IAM = 0x4, // interrupt acknowledge, master
	ST_IAC = 0x5, // interrupt acknowledge, cascaded
	ST_EIM = 0x6, // end of interrupt, master
	ST_EIC = 0x7, // end of interrupt, cascaded
	ST_SIF = 0x8, // sequential instruction fetch
	ST_NIF = 0x9, // non-sequential instruction fetch
	ST_ODT = 0xa, // operand data transfer
	ST_RMW = 0xb, // read RMW operand
	ST_EAR = 0xc, // effective address read
	ST_SOP = 0xd, // slave operand
	ST_SST = 0xe, // slave status
	ST_SID = 0xf, // slave ID
};

class ns32000_abort : public std::exception { };

static const u32 size_mask[] = { 0x000000ffU, 0x0000ffffU, 0x00000000U, 0xffffffffU };

#define SP ((m_psr & PSR_S) ? m_sp1 : m_sp0)

template <int Width>ns32000_device<Width>::ns32000_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int databits, int addrbits)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_address_mask(0xffffffffU >> (32 - addrbits))
	, m_program_config("program", ENDIANNESS_LITTLE, databits, addrbits, 0)
	, m_iam_config("iam", ENDIANNESS_LITTLE, databits, addrbits, 0)
	, m_iac_config("iac", ENDIANNESS_LITTLE, databits, addrbits, 0)
	, m_eim_config("eim", ENDIANNESS_LITTLE, databits, addrbits, 0)
	, m_eic_config("eic", ENDIANNESS_LITTLE, databits, addrbits, 0)
	, m_sif_config("sif", ENDIANNESS_LITTLE, databits, addrbits, 0)
	, m_nif_config("nif", ENDIANNESS_LITTLE, databits, addrbits, 0)
	, m_odt_config("odt", ENDIANNESS_LITTLE, databits, addrbits, 0)
	, m_rmw_config("rmw", ENDIANNESS_LITTLE, databits, addrbits, 0)
	, m_ear_config("ear", ENDIANNESS_LITTLE, databits, addrbits, 0)
	, m_fpu(*this, finder_base::DUMMY_TAG)
	, m_mmu(*this, finder_base::DUMMY_TAG)
	, m_icount(0)
	, m_pc(0)
	, m_sb(0)
	, m_fp(0)
	, m_sp1(0)
	, m_sp0(0)
	, m_intbase(0)
	, m_psr(0)
	, m_mod(0)
	, m_cfg(0)
	, m_r{0}
	, m_f{0}
	, m_nmi_line(false)
	, m_int_line(false)
	, m_wait(false)
	, m_sequential(false)
{
}

ns32008_device::ns32008_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: ns32000_device(mconfig, NS32008, tag, owner, clock, 8, 24)
{
}

ns32016_device::ns32016_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: ns32000_device(mconfig, NS32016, tag, owner, clock, 16, 24)
{
}

ns32032_device::ns32032_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: ns32000_device(mconfig, NS32032, tag, owner, clock, 32, 24)
{
}

ns32332_device::ns32332_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: ns32000_device(mconfig, NS32332, tag, owner, clock, 32, 32)
{
}

template <int Width> void ns32000_device<Width>::device_start()
{
	set_icountptr(m_icount);

	save_item(NAME(m_ssp));
	save_item(NAME(m_sps));

	save_item(NAME(m_pc));
	save_item(NAME(m_sb));
	save_item(NAME(m_fp));
	save_item(NAME(m_sp1));
	save_item(NAME(m_sp0));
	save_item(NAME(m_intbase));
	save_item(NAME(m_psr));
	save_item(NAME(m_mod));
	save_item(NAME(m_cfg));

	save_item(NAME(m_r));
	save_item(NAME(m_f));

	save_item(NAME(m_nmi_line));
	save_item(NAME(m_int_line));
	save_item(NAME(m_wait));

	state_add(STATE_GENPC, "GENPC", m_pc).noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_pc).noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS", m_psr).mask(0xfe7).formatstr("%10s").noshow();

	// dedicated registers
	int index = 0;
	state_add(index++, "PC", m_pc);
	state_add(index++, "SB", m_sb);
	state_add(index++, "FP", m_fp);
	state_add(index++, "SP1", m_sp1);
	state_add(index++, "SP0", m_sp0);
	state_add(index++, "INTBASE", m_intbase);
	state_add(index++, "PSR", m_psr);
	state_add(index++, "MOD", m_mod);
	state_add(index++, "CFG", m_cfg).formatstr("%5s");

	// general registers
	for (unsigned i = 0; i < 8; i++)
		state_add(index++, util::string_format("R%d", i).c_str(), m_r[i]);

	// floating point registers
	if (m_fpu)
		m_fpu->state_add(*this, index);

	// memory management registers
	if (m_mmu)
		m_mmu->state_add(*this, index);
}

template <int Width> void ns32000_device<Width>::device_reset()
{
	for (std::pair<int, address_space_config const *> s : memory_space_config())
		space(has_configured_map(s.first) ? s.first : 0).specific(m_bus[s.first]);

	m_pc = 0;
	m_psr = 0;
	m_cfg = 0;

	m_nmi_line = false;
	m_int_line = false;
	m_wait = false;
}

template <int Width> void ns32000_device<Width>::state_string_export(device_state_entry const &entry, std::string &str) const
{
	switch (entry.index())
	{
	case STATE_GENFLAGS:
		str = string_format("%c%c%c%c%c%c%c%c%c%c",
			(m_psr & PSR_I) ? 'I' : '.',
			(m_psr & PSR_P) ? 'P' : '.',
			(m_psr & PSR_S) ? 'S' : '.',
			(m_psr & PSR_U) ? 'U' : '.',
			(m_psr & PSR_N) ? 'N' : '.',
			(m_psr & PSR_Z) ? 'Z' : '.',
			(m_psr & PSR_F) ? 'F' : '.',
			(m_psr & PSR_L) ? 'L' : '.',
			(m_psr & PSR_T) ? 'T' : '.',
			(m_psr & PSR_C) ? 'C' : '.');
		break;

	case 8:
		str = string_format("%c%c%c%c%c",
			(m_cfg & CFG_P) ? 'P' : '.',
			(m_cfg & CFG_C) ? ((m_cfg & CFG_FC) ? 'C' : 'c') : '.',
			(m_cfg & CFG_M) ? ((m_cfg & CFG_FM) ? 'M' : 'm') : '.',
			(m_cfg & CFG_F) ? ((m_cfg & CFG_FF) ? 'F' : 'f') : '.',
			(m_cfg & CFG_I) ? 'I' : '.');
		break;
	}
}

/*
 * The optional MMU and lack of alignment restrictions require memory accessors
 * to handle several scenarios:
 *
 *   MMU  Aligned  Pages  Approach
 *    N      Y      N/A   aligned handler
 *    N      N      N/A   unaligned handler
 *    Y      Y       1    translate address, aligned handler
 *    Y      N       1    translate address, unaligned handler
 *    Y      N       2    translate two addresses, use masks/shifts to align
 *                        data, aligned handlers
 *
 * Underlying handlers further subdivide accesses by device bus width.
 */
template <int Width> template<typename T> T ns32000_device<Width>::mem_read(unsigned st, u32 address, bool user, bool pfs)
{
	u32 physical = address;
	ns32000_mmu_interface::translate_result tr = m_mmu ?
		m_mmu->translate(m_bus[st].space(), st, physical, (m_psr & PSR_U) || user, false, pfs) : ns32000_mmu_interface::COMPLETE;

	if (tr == ns32000_mmu_interface::COMPLETE)
	{
		u32 const unitmask = sizeof(T) - 1;
		unsigned const offset = address & unitmask;

		if (offset)
		{
			u32 const pagemask = u32(0x1ff) & ~unitmask;

			if (!m_mmu || (~address & pagemask))
			{
				// unaligned access, one page (or no mmu)
				switch (sizeof(T))
				{
				case 1: abort(); // can't happen
				case 2: return T(m_bus[st].read_word_unaligned(physical));
				case 4: return T(m_bus[st].read_dword_unaligned(physical));
				case 8: return T(m_bus[st].read_qword_unaligned(physical));
				}
			}
			else
			{
				// unaligned access, two pages
				T data = 0;

				// first page
				unsigned const shift = offset * 8;
				switch (sizeof(T))
				{
				case 1: abort(); // can't happen
				case 2: data |= m_bus[st].read_word(physical & ~unitmask, u16(-1) << shift) >> shift; break;
				case 4: data |= m_bus[st].read_dword(physical & ~unitmask, u32(-1) << shift) >> shift; break;
				case 8: data |= m_bus[st].read_qword(physical & ~unitmask, u64(-1) << shift) >> shift; break;
				}

				// second page
				physical = (address + sizeof(T)) & ~unitmask;
				tr = m_mmu->translate(m_bus[st].space(), st, physical, (m_psr & PSR_U) || user, false);

				if (tr == ns32000_mmu_interface::COMPLETE)
				{
					unsigned const shift = (sizeof(T) - offset) * 8;
					switch (sizeof(T))
					{
					case 1: abort(); // can't happen
					case 2: data |= m_bus[st].read_word(physical & ~unitmask, u16(-1) >> shift) << shift; break;
					case 4: data |= m_bus[st].read_dword(physical & ~unitmask, u32(-1) >> shift) << shift; break;
					case 8: data |= m_bus[st].read_qword(physical & ~unitmask, u64(-1) >> shift) << shift; break;
					}

					return data;
				}
				else if (tr == ns32000_mmu_interface::ABORT)
					throw ns32000_abort();
			}
		}
		else
		{
			// aligned access
			switch (sizeof(T))
			{
			case 1: return T(m_bus[st].read_byte(physical));
			case 2: return T(m_bus[st].read_word(physical));
			case 4: return T(m_bus[st].read_dword(physical));
			case 8: return T(m_bus[st].read_qword(physical));
			}
		}
	}
	else if (tr == ns32000_mmu_interface::ABORT)
		throw ns32000_abort();

	return 0;
}

template <int Width> template<typename T> void ns32000_device<Width>::mem_write(unsigned st, u32 address, T data, bool user)
{
	u32 physical = address;
	ns32000_mmu_interface::translate_result tr = m_mmu ?
		m_mmu->translate(m_bus[st].space(), st, physical, (m_psr & PSR_U) || user, true) : ns32000_mmu_interface::COMPLETE;

	if (tr == ns32000_mmu_interface::COMPLETE)
	{
		u32 const unitmask = sizeof(T) - 1;
		unsigned const offset = address & unitmask;

		if (offset)
		{
			u32 const pagemask = u32(0x1ff) & ~unitmask;

			if (!m_mmu || (~address & pagemask))
			{
				// unaligned access, one page (or no mmu)
				switch (sizeof(T))
				{
				case 1: abort(); // can't happen
				case 2: m_bus[st].write_word_unaligned(physical, data); break;
				case 4: m_bus[st].write_dword_unaligned(physical, data); break;
				case 8: m_bus[st].write_qword_unaligned(physical, data); break;
				}
			}
			else
			{
				// unaligned access, two pages

				// first page
				unsigned const shift = offset * 8;
				switch (sizeof(T))
				{
				case 1: abort(); // can't happen
				case 2: m_bus[st].write_word(physical & ~unitmask, data << shift, u16(-1) << shift); break;
				case 4: m_bus[st].write_dword(physical & ~unitmask, data << shift, u32(-1) << shift); break;
				case 8: m_bus[st].write_qword(physical & ~unitmask, data << shift, u64(-1) << shift); break;
				}

				// second page
				physical = (address + sizeof(T)) & ~unitmask;
				tr = m_mmu->translate(m_bus[st].space(), st, physical, (m_psr & PSR_U) || user, true);

				if (tr == ns32000_mmu_interface::COMPLETE)
				{
					unsigned const shift = (sizeof(T) - offset) * 8;
					switch (sizeof(T))
					{
					case 1: abort(); // can't happen
					case 2: m_bus[st].write_word(physical & ~unitmask, data >> shift, u16(-1) >> shift); break;
					case 4: m_bus[st].write_dword(physical & ~unitmask, data >> shift, u32(-1) >> shift); break;
					case 8: m_bus[st].write_qword(physical & ~unitmask, data >> shift, u64(-1) >> shift); break;
					}
				}
				else if (tr == ns32000_mmu_interface::ABORT)
					throw ns32000_abort();
			}
		}
		else
		{
			// aligned access
			switch (sizeof(T))
			{
			case 1: m_bus[st].write_byte(physical, data); break;
			case 2: m_bus[st].write_word(physical, data); break;
			case 4: m_bus[st].write_dword(physical, data); break;
			case 8: m_bus[st].write_qword(physical, data); break;
			}
		}
	}
	else if (tr == ns32000_mmu_interface::ABORT)
		throw ns32000_abort();
}

/*
 * TODO: this function still doesn't accurately emulate instruction fetch:
 *  - prefetch or opportunistic refill
 *  - buffer re-alignment
 *  - sequential fetch translation optimization
 *  - instruction fetch cycles
 */
template <int Width> template<typename T> T ns32000_device<Width>::fetch(unsigned &bytes)
{
	T const data = mem_read<T>(m_sequential ? ST_SIF : ST_NIF, m_pc + bytes, false, bytes == 0);

	bytes += sizeof(T);
	m_sequential = true;

	return data;
}

template <int Width> s32 ns32000_device<Width>::displacement(unsigned &bytes)
{
	u32 const byte0 = fetch<u8>(bytes);
	if (BIT(byte0, 7))
	{
		if (BIT(byte0, 6))
		{
			// double word displacement
			u32 const byte1 = fetch<u8>(bytes);
			u32 const byte2 = fetch<u8>(bytes);
			u32 const byte3 = fetch<u8>(bytes);

			return (s32((byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3) << 2) >> 2;
		}
		else
		{
			// word displacement
			u8 const byte1 = fetch<u8>(bytes);

			return s16(((byte0 << 8) | byte1) << 2) >> 2;
		}
	}
	else
		// byte displacement
		return s8(byte0 << 1) >> 1;
}

template <int Width> void ns32000_device<Width>::decode(addr_mode *mode, unsigned &bytes)
{
	bool scaled[] = { false, false };

	// scaled mode
	for (unsigned i = 0; i < 2; i++)
	{
		if (mode[i].gen > 0x1b)
		{
			u8 const index = fetch<u8>(bytes);

			mode[i].disp = m_r[index & 7] << (mode[i].gen & 3);

			static const unsigned tea[] = { 5, 7, 8, 10 };
			mode[i].tea = tea[mode[i].gen & 3];

			mode[i].gen = index >> 3;
			scaled[i] = true;
		}
	}

	// base mode
	for (unsigned i = 0; i < 2; i++)
	{
		switch (mode[i].gen)
		{
		case 0x00: case 0x01: case 0x02: case 0x03:
		case 0x04: case 0x05: case 0x06: case 0x07:
			// register
			if (scaled[i])
			{
				mode[i].base = m_r[mode[i].gen];
				mode[i].type = MEM;
				mode[i].tea += 5;
			}
			else
			{
				mode[i].type = REG;
				mode[i].tea += 2;
			}
			break;
		case 0x08: case 0x09: case 0x0a: case 0x0b:
		case 0x0c: case 0x0d: case 0x0e: case 0x0f:
			// register relative
			mode[i].base = m_r[mode[i].gen & 7] + displacement(bytes);
			mode[i].type = MEM;
			mode[i].tea += 5;
			break;
		case 0x10:
			// frame memory relative disp2(disp1(FP))
			mode[i].base = m_fp + displacement(bytes);
			mode[i].disp += displacement(bytes);
			mode[i].type = REL;
			mode[i].tea += 7 + top(SIZE_D, mode[i].base);
			break;
		case 0x11:
			// stack memory relative disp2(disp1(SP))
			mode[i].base = SP + displacement(bytes);
			mode[i].disp += displacement(bytes);
			mode[i].type = REL;
			mode[i].tea += 7 + top(SIZE_D, mode[i].base);
			break;
		case 0x12:
			// static memory relative disp2(disp1(SB))
			mode[i].base = m_sb + displacement(bytes);
			mode[i].disp += displacement(bytes);
			mode[i].type = REL;
			mode[i].tea += 7 + top(SIZE_D, mode[i].base);
			break;
		case 0x13:
			// reserved
			break;
		case 0x14:
			// immediate
			switch (mode[i].size)
			{
			case SIZE_B: mode[i].imm = fetch<u8>(bytes); break;
			case SIZE_W: mode[i].imm = swapendian_int16(fetch<u16>(bytes)); break;
			case SIZE_D: mode[i].imm = swapendian_int32(fetch<u32>(bytes)); break;
			case SIZE_Q: mode[i].imm = swapendian_int64(fetch<u64>(bytes)); break;
			}
			mode[i].type = IMM;
			mode[i].tea += 4;
			break;
		case 0x15:
			// absolute @disp
			mode[i].base = displacement(bytes);
			mode[i].type = MEM;
			mode[i].tea += 4;
			break;
		case 0x16:
			// external EXT(disp1) + disp2
			mode[i].base = displacement(bytes) * 4;
			mode[i].disp += displacement(bytes);
			mode[i].type = EXT;
			mode[i].tea += 11 + top(SIZE_D, m_mod) + top(SIZE_D, mode[i].base);
			break;
		case 0x17:
			// top of stack TOS
			mode[i].base = SP;
			mode[i].type = scaled[i] ? MEM : TOS;
			if (!scaled[i])
			{
				if (mode[i].access == WRITE)
					mode[i].tea += 4;
				else if (mode[i].access == READ)
					mode[i].tea += 2;
				else
					mode[i].tea += 3;
			}
			else
				mode[i].tea += 4;
			break;
		case 0x18:
			// frame memory disp(FP)
			mode[i].base = m_fp + displacement(bytes);
			mode[i].type = MEM;
			mode[i].tea += 5;
			break;
		case 0x19:
			// stack memory disp(SP)
			mode[i].base = SP + displacement(bytes);
			mode[i].type = MEM;
			mode[i].tea += 5;
			break;
		case 0x1a:
			// static memory disp(SB)
			mode[i].base = m_sb + displacement(bytes);
			mode[i].type = MEM;
			mode[i].tea += 5;
			break;
		case 0x1b:
			// program memory *+disp
			mode[i].base = m_pc + displacement(bytes);
			mode[i].type = MEM;
			mode[i].tea += 5;
			break;
		}
	}
}

template <int Width> u32 ns32000_device<Width>::ea(addr_mode const mode)
{
	u32 base;

	switch (mode.type)
	{
	case REG:
		base = m_r[mode.gen];
		break;

	case REL:
		base = mem_read<u32>(ST_EAR, mode.base);
		break;

	case EXT:
		base = mem_read<u32>(ST_EAR, m_mod + 4);
		base = mem_read<u32>(ST_EAR, base + mode.base);
		break;

	default:
		base = mode.base;
		break;
	}

	return base + mode.disp;
}

template <int Width> u64 ns32000_device<Width>::gen_read(addr_mode mode)
{
	if (mode.type == IMM)
		return mode.imm;

	if (mode.type == REG)
		return m_r[mode.gen] & size_mask[mode.size];

	unsigned const st = (mode.access == RMW) ? ST_RMW : ST_ODT;
	u32 const address = (mode.type == TOS) ? SP : ea(mode);
	u64 data = 0;

	switch (mode.size)
	{
	case SIZE_B: data = mem_read<u8>(st, address); break;
	case SIZE_W: data = mem_read<u16>(st, address); break;
	case SIZE_D: data = mem_read<u32>(st, address); break;
	case SIZE_Q: data = mem_read<u64>(st, address); break;
	}

	m_icount -= top(mode.size, address);

	// post-increment stack pointer
	if (mode.type == TOS && mode.access == READ)
		SP += mode.size + 1;

	return data;
}

template <int Width> s64 ns32000_device<Width>::gen_read_sx(addr_mode mode)
{
	u64 data = gen_read(mode);

	switch (mode.size)
	{
	case SIZE_B: data = s8(data); break;
	case SIZE_W: data = s16(data); break;
	case SIZE_D: data = s32(data); break;
	case SIZE_Q: data = s64(data); break;
	}

	return data;
}

template <int Width> void ns32000_device<Width>::gen_write(addr_mode mode, u64 data)
{
	if (mode.type == REG)
	{
		m_r[mode.gen] = (m_r[mode.gen] & ~size_mask[mode.size]) | (data & size_mask[mode.size]);
		return;
	}

	// pre-decrement stack pointer
	if (mode.type == TOS && mode.access == WRITE)
		SP -= mode.size + 1;

	u32 const address = (mode.type == TOS) ? SP : ea(mode);

	switch (mode.size)
	{
	case SIZE_B: mem_write<u8>(ST_ODT, address, data); break;
	case SIZE_W: mem_write<u16>(ST_ODT, address, data); break;
	case SIZE_D: mem_write<u32>(ST_ODT, address, data); break;
	case SIZE_Q: mem_write<u64>(ST_ODT, address, data); break;
	}

	m_icount -= top(mode.size, address);
}

template <int Width> bool ns32000_device<Width>::condition(unsigned const cc)
{
	switch (cc & 15)
	{
	case 0x0: // equal
		return (m_psr & PSR_Z);
	case 0x1: // not equal
		return !(m_psr & PSR_Z);
	case 0x2: // carry set
		return (m_psr & PSR_C);
	case 0x3: // carry clear
		return !(m_psr & PSR_C);
	case 0x4: // higher
		return (m_psr & PSR_L);
	case 0x5: // lower or same
		return !(m_psr & PSR_L);
	case 0x6: // greater than
		return (m_psr & PSR_N);
	case 0x7: // less or equal
		return !(m_psr & PSR_N);
	case 0x8: // flag set
		return (m_psr & PSR_F);
	case 0x9: // flag clear
		return !(m_psr & PSR_F);
	case 0xa: // lower
		return !(m_psr & PSR_L) && !(m_psr & PSR_Z);
	case 0xb: // higher or same
		return (m_psr & PSR_L) || (m_psr & PSR_Z);
	case 0xc: // less than
		return !(m_psr & PSR_N) && !(m_psr & PSR_Z);
	case 0xd: // greater or equal
		return (m_psr & PSR_N) || (m_psr & PSR_Z);
	case 0xe: // unconditionally true
		return true;
	case 0xf: // unconditionally false
		return false;
	}

	// can't happen
	return false;
}

template <int Width> void ns32000_device<Width>::flags(u32 const src1, u32 const src2, u32 const dest, unsigned const size, bool const subtraction)
{
	unsigned const sign_bit = (size + 1) * 8 - 1;

	bool const src1_s = BIT(src1, sign_bit);
	bool const src2_s = subtraction ? !BIT(src2, sign_bit) : BIT(src2, sign_bit);
	bool const dest_s = subtraction ? !BIT(dest, sign_bit) : BIT(dest, sign_bit);

	m_psr &= ~(PSR_F | PSR_C);
	if ((src2_s && src1_s) || (!dest_s && (src2_s || src1_s)))
		m_psr |= PSR_C;

	if ((src2_s == src1_s) && (dest_s != src2_s))
		m_psr |= PSR_F;
}

template <int Width> void ns32000_device<Width>::interrupt(unsigned const type, u32 const return_address)
{
	unsigned offset = type * 4;

	switch (type)
	{
	case NVI:
		// maskable interrupt
		m_sps = m_psr;
		m_psr &= ~(PSR_I | PSR_P | PSR_S | PSR_U | PSR_T);

		if (m_cfg & CFG_I)
		{
			// acknowledge interrupt and read vector
			s8 vector = mem_read<u8>(ST_IAM, 0xfffffe00 & m_address_mask);
			if (vector < 0 && vector >= -16)
			{
				// vectored mode, cascaded
				u32 const cascade = mem_read<u32>(ST_ODT, m_intbase + vector * 4);

				vector = mem_read<u8>(ST_IAC, cascade);
			}

			offset = vector * 4;
		}
		else
			// acknowledge non-vectored interrupt
			mem_read<u8>(ST_IAM, 0xfffffe00 & m_address_mask);
		break;

	case NMI:
		// non-maskable interrupt
		m_sps = m_psr;
		m_psr &= ~(PSR_I | PSR_P | PSR_S | PSR_U | PSR_T);

		// acknowledge interrupt and discard vector
		mem_read<u8>(ST_IAM, 0xffffff00 & m_address_mask);
		m_nmi_line = false;
		break;

	case ABT:
		// abort
		SP = m_ssp;
		m_psr &= ~PSR_P;
		m_sps = m_psr;
		m_psr &= ~(PSR_I | PSR_S | PSR_U | PSR_T);
		break;

	case TRC:
		// trace trap
		m_psr &= ~PSR_P;
		m_sps = m_psr;
		m_psr &= ~(PSR_S | PSR_U | PSR_T);
		break;

	default:
		// traps other than trace
		SP = m_ssp;
		m_psr = m_sps;
		m_psr &= ~(PSR_P | PSR_S | PSR_U | PSR_T);
		break;
	}

	// push saved program status
	m_sp0 -= 2;
	mem_write<u16>(ST_ODT, m_sp0, m_sps);

	// push mod
	m_sp0 -= 2;
	mem_write<u16>(ST_ODT, m_sp0, m_mod);

	// push return address
	m_sp0 -= 4;
	mem_write<u32>(ST_ODT, m_sp0, return_address);

	// fetch external procedure descriptor
	u32 const desc = mem_read<u32>(ST_ODT, m_intbase + offset);

	// update mod, sb, pc
	m_mod = u16(desc);
	m_sb = mem_read<u32>(ST_ODT, m_mod + 0);
	m_pc = mem_read<u32>(ST_ODT, m_mod + 8) + (desc >> 16);

	// TODO: flush queue
	m_sequential = false;

	m_icount -= top(SIZE_W, m_sp0) * 2 + top(SIZE_W, m_intbase + offset) + top(SIZE_D, m_sp0) + top(SIZE_D, m_mod);

	if (machine().debug_enabled() && (type > ABT))
		debug()->exception_hook(type);
}

template <int Width> void ns32000_device<Width>::execute_run()
{
	while (m_icount > 0)
	{
		if (m_wait)
		{
			m_icount = 0;
			continue;
		}

		try
		{
			if (m_nmi_line)
			{
				// service interrupt
				interrupt(NMI, m_pc);

				// notify the debugger
				if (machine().debug_enabled())
					debug()->interrupt_hook(INPUT_LINE_NMI);
			}
			else if (m_int_line && (m_psr & PSR_I))
			{
				// service interrupt
				interrupt(NVI, m_pc);

				// notify the debugger
				if (machine().debug_enabled())
					debug()->interrupt_hook(INPUT_LINE_IRQ0);
			}

			// update trace pending
			if (m_psr & PSR_T)
				m_psr |= PSR_P;
			else
				m_psr &= ~PSR_P;

			debugger_instruction_hook(m_pc);

			// save state
			m_ssp = SP;
			m_sps = m_psr;

			unsigned bytes = 0;
			u8 const opbyte = fetch<u8>(bytes);
			unsigned tex = 1;

			if ((opbyte & 15) == 10)
			{
				// format 0: cccc 1010
				// Bcond dst
				//       disp
				s32 const dst = displacement(bytes);

				if (condition(BIT(opbyte, 4, 4)))
				{
					m_pc += dst;
					m_sequential = false;
					tex = 6;
				}
				else
					tex = 7;
			}
			else if ((opbyte & 15) == 2)
			{
				// format 1: oooo 0010
				switch (BIT(opbyte, 4, 4))
				{
				case 0x0:
					// BSR dst
					//     disp
					{
						s32 const dst = displacement(bytes);

						SP -= 4;
						mem_write<u32>(ST_ODT, SP, m_pc + bytes);

						m_pc += dst;
						m_sequential = false;
						tex = top(SIZE_D, SP) + 6;
					}
					break;
				case 0x1:
					// RET constant
					//     disp
					{
						s32 const constant = displacement(bytes);

						u32 const addr = mem_read<u32>(ST_ODT, SP);
						SP += 4;

						m_pc = addr;
						m_sequential = false;
						tex = top(SIZE_D, SP) + 2;
						SP += constant;
					}
					break;
				case 0x2:
					// CXP index
					//     disp
					{
						s32 const index = displacement(bytes);

						u32 const link_base = mem_read<u32>(ST_ODT, m_mod + 4);
						u32 const desc = mem_read<u32>(ST_ODT, link_base + index * 4);

						SP -= 4;
						mem_write<u16>(ST_ODT, SP, m_mod);
						SP -= 4;
						mem_write<u32>(ST_ODT, SP, m_pc + bytes);

						tex = top(SIZE_D, m_mod + 4) + top(SIZE_W, link_base + index * 4) * 2 + top(SIZE_W, SP) + top(SIZE_D, SP) + top(SIZE_D, u16(desc)) * 2 + 16;

						m_mod = u16(desc);
						m_sb = mem_read<u32>(ST_ODT, m_mod + 0);
						m_pc = mem_read<u32>(ST_ODT, m_mod + 8) + (desc >> 16);
						m_sequential = false;
					}
					break;
				case 0x3:
					// RXP constant
					//     disp
					{
						s32 const constant = displacement(bytes);

						m_pc = mem_read<u32>(ST_ODT, SP);
						SP += 4;
						m_mod = mem_read<u16>(ST_ODT, SP);
						SP += 4;

						m_sb = mem_read<u32>(ST_ODT, m_mod + 0);
						tex = top(SIZE_D, SP) + top(SIZE_W, SP) + top(SIZE_D, SP) + 2;
						SP += constant;
						m_sequential = false;
					}
					break;
				case 0x4:
					// RETT constant
					//      disp
					if (!(m_psr & PSR_U))
					{
						s32 const constant = displacement(bytes);

						u32 &sp(SP);
						m_pc = mem_read<u32>(ST_ODT, sp);
						sp += 4;
						m_mod = mem_read<u16>(ST_ODT, sp);
						sp += 2;
						m_psr = mem_read<u16>(ST_ODT, sp) & PSR_MSK;
						sp += 2;

						m_sb = mem_read<u32>(ST_ODT, m_mod);

						SP += constant;
						m_sequential = false;
						tex = top(SIZE_D, sp) + top(SIZE_W, sp) * 2 + top(SIZE_D, m_mod) + 35;
					}
					else
						interrupt(ILL, m_pc);
					break;
				case 0x5:
					// RETI
					if (!(m_psr & PSR_U))
					{
						// end of interrupt, master
						s8 vector = mem_read<u8>(ST_EIM, 0xfffffe00 & m_address_mask);

						// check for vectored mode
						if (m_cfg & CFG_I)
						{
							if (vector < 0 && vector >= -16)
							{
								u32 const cascade = mem_read<u32>(ST_ODT, m_intbase + vector * 4);

								// end of interrupt, cascaded
								vector = mem_read<u8>(ST_EIC, cascade);
							}
						}

						u32 &sp(SP);
						m_pc = mem_read<u32>(ST_ODT, sp);
						sp += 2;
						mem_read<u16>(ST_ODT, sp);
						sp += 2;
						m_mod = mem_read<u16>(ST_ODT, sp);
						sp += 2;
						m_psr = mem_read<u16>(ST_ODT, sp) & PSR_MSK;
						sp += 2;

						m_sb = mem_read<u32>(ST_ODT, m_mod);
						m_sequential = false;

						tex = top(SIZE_B) + top(SIZE_W) * 3 + top(SIZE_D) * 3 + 39;
					}
					else
						interrupt(ILL, m_pc);
					break;
				case 0x6:
					// SAVE reglist
					//      imm
					{
						u8 const reglist = fetch<u8>(bytes);

						tex = 13;
						for (unsigned i = 0; i < 8; i++)
						{
							if (BIT(reglist, i))
							{
								SP -= 4;
								mem_write<u32>(ST_ODT, SP, m_r[i]);
								tex += top(SIZE_D, SP) + 4;
							}
						}
					}
					break;
				case 0x7:
					// RESTORE reglist
					//         imm
					{
						u8 const reglist = fetch<u8>(bytes);

						tex = 12;
						for (unsigned i = 0; i < 8; i++)
						{
							if (BIT(reglist, i))
							{
								m_r[7 - i] = mem_read<u32>(ST_ODT, SP);
								tex += top(SIZE_D, SP) + 5;
								SP += 4;
							}
						}
					}
					break;
				case 0x8:
					// ENTER reglist,constant
					//       imm,disp
					{
						u8 const reglist = fetch<u8>(bytes);
						s32 const constant = displacement(bytes);

						SP -= 4;
						mem_write<u32>(ST_ODT, SP, m_fp);
						tex = top(SIZE_D, SP) + 18;
						m_fp = SP;
						SP -= constant;

						for (unsigned i = 0; i < 8; i++)
						{
							if (BIT(reglist, i))
							{
								SP -= 4;
								mem_write<u32>(ST_ODT, SP, m_r[i]);
								tex += top(SIZE_D, SP) + 4;
							}
						}
					}
					break;
				case 0x9:
					// EXIT reglist
					//      imm
					{
						u8 const reglist = fetch<u8>(bytes);

						tex = 17;
						for (unsigned i = 0; i < 8; i++)
						{
							if (BIT(reglist, i))
							{
								m_r[7 - i] = mem_read<u32>(ST_ODT, SP);
								tex += top(SIZE_D, SP) + 5;
								SP += 4;
							}
						}
						SP = m_fp;
						m_fp = mem_read<u32>(ST_ODT, SP);
						tex += top(SIZE_D, SP);
						SP += 4;
					}
					break;
				case 0xa:
					// NOP
					tex = 3;
					break;
				case 0xb:
					// WAIT
					m_wait = true;
					tex = 6;
					break;
				case 0xc:
					// DIA
					tex = 3;
					m_sequential = false;
					break;
				case 0xd:
					// FLAG
					if (m_psr & PSR_F)
					{
						interrupt(FLG, m_pc);
						tex = 44;
					}
					else
						tex = 6;
					break;
				case 0xe:
					// SVC
					interrupt(SVC, m_pc);
					tex = 40;
					break;
				case 0xf:
					// BPT
					interrupt(BPT, m_pc);
					tex = 40;
					break;
				}
			}
			else if ((opbyte & 15) == 12 || (opbyte & 15) == 13 || (opbyte & 15) == 15)
			{
				// format 2: gggg gsss sooo 11ii
				u16 const opword = (u16(fetch<u8>(bytes)) << 8) | opbyte;

				// HACK: use reserved mode for second unused type
				addr_mode mode[] = { addr_mode(BIT(opword, 11, 5)), addr_mode(0x13) };

				unsigned const quick = BIT(opword, 7, 4);
				size_code const size = size_code(opbyte & 3);

				switch (BIT(opbyte, 4, 3))
				{
				case 0:
					// ADDQi src,dst
					//       quick,gen
					//             rmw.i
					{
						mode[0].rmw_i(size);
						decode(mode, bytes);

						u32 const src1 = s32(quick << 28) >> 28;
						u32 const src2 = gen_read(mode[0]);

						u32 const dst = src1 + src2;
						flags(src1, src2, dst, size, false);

						gen_write(mode[0], dst);

						tex = (mode[0].type == REG) ? 4 : mode[0].tea + 6;
					}
					break;
				case 1:
					// CMPQi src1,src2
					//       quick,gen
					//             read.i
					{
						mode[0].read_i(size);
						decode(mode, bytes);

						u32 const src1 = (s32(quick << 28) >> 28) & size_mask[size];
						u32 const src2 = gen_read(mode[0]);

						m_psr &= ~(PSR_N | PSR_Z | PSR_L);

						if ((size == SIZE_D && s32(src1) > s32(src2))
						|| ((size == SIZE_W && s16(src1) > s16(src2))
						|| ((size == SIZE_B && s8(src1) > s8(src2)))))
							m_psr |= PSR_N;

						if (src1 == src2)
							m_psr |= PSR_Z;

						if ((size == SIZE_D && u32(src1) > u32(src2))
						|| ((size == SIZE_W && u16(src1) > u16(src2))
						|| ((size == SIZE_B && u8(src1) > u8(src2)))))
							m_psr |= PSR_L;

						tex = (mode[0].type == REG) ? 3 : mode[0].tea + 3;
					}
					break;
				case 2:
					// SPRi procreg,dst
					//      short,gen
					//            write.i
					mode[0].write_i(size);
					decode(mode, bytes);

					switch (quick)
					{
					case 0x0: // UPSR
						gen_write(mode[0], u8(m_psr));
						break;
					case 0x8: // FP
						gen_write(mode[0], m_fp);
						break;
					case 0x9: // SP
						gen_write(mode[0], SP);
						break;
					case 0xa: // SB
						gen_write(mode[0], m_sb);
						break;
					case 0xd: // PSR
						if (!(m_psr & PSR_U))
							gen_write(mode[0], m_psr);
						else
							interrupt(ILL, m_pc);
						break;
					case 0xe: // INTBASE
						if (!(m_psr & PSR_U))
							gen_write(mode[0], m_intbase);
						else
							interrupt(ILL, m_pc);
						break;
					case 0xf: // MOD
						gen_write(mode[0], m_mod);
						break;
					}

					// TODO: tcy 21-27
					tex = mode[0].tea + 21;
					break;
				case 3:
					// Scondi dst
					//        gen
					//        write.i
					{
						mode[0].write_i(size);
						decode(mode, bytes);

						bool const dst = condition(quick);
						gen_write(mode[0], dst);

						tex = mode[0].tea + (dst ? 10 : 9);
					}
					break;
				case 4:
					// ACBi inc,index,dst
					//      quick,gen,disp
					//            rmw.i
					{
						mode[0].rmw_i(size);
						decode(mode, bytes);

						s32 const inc = s32(quick << 28) >> 28;
						u32 index = gen_read(mode[0]);
						s32 const dst = displacement(bytes);

						index += inc;
						gen_write(mode[0], index);

						if (index & size_mask[size])
						{
							m_pc += dst;
							m_sequential = false;

							tex = (mode[0].type == REG) ? 17 : mode[0].tea + 15;
						}
						else
							tex = (mode[0].type == REG) ? 18 : mode[0].tea + 16;
					}
					break;
				case 5:
					// MOVQi src,dst
					//       quick,gen
					//             write.i
					mode[0].write_i(size);
					decode(mode, bytes);

					gen_write(mode[0], s32(quick << 28) >> 28);

					tex = (mode[0].type == REG) ? 3 : mode[0].tea + 2;
					break;
				case 6:
					// LPRi procreg,src
					//      short,gen
					//            read.i
					mode[0].read_i(size);
					decode(mode, bytes);

					switch (quick)
					{
					case 0x0: // UPSR
						m_psr = ((m_psr & 0xff00) | u8(gen_read(mode[0]))) & PSR_MSK;
						break;
					case 0x8: // FP
						m_fp = gen_read(mode[0]);
						break;
					case 0x9: // SP
						SP = gen_read(mode[0]);
						break;
					case 0xa: // SB
						m_sb = gen_read(mode[0]);
						break;
					case 0xd: // PSR
						if (!(m_psr & PSR_U))
						{
							u32 const src = gen_read(mode[0]);

							if (size == SIZE_B)
								m_psr = ((m_psr & 0xff00) | u8(src)) & PSR_MSK;
							else
								m_psr = src & PSR_MSK;
						}
						else
							interrupt(ILL, m_pc);
						break;
					case 0xe: // INTBASE
						if (!(m_psr & PSR_U))
							m_intbase = gen_read(mode[0]);
						else
							interrupt(ILL, m_pc);
						break;
					case 0xf: // MOD
						m_mod = gen_read(mode[0]);
						break;
					default:
						interrupt(UND, m_pc);
						break;
					}

					// TODO: tcy 19-33
					tex = mode[0].tea + 19;
					break;
				case 7:
					// format 3: gggg gooo o111 11ii
					switch (BIT(opword, 7, 4))
					{
					case 0x0:
						// CXPD desc
						//      gen
						//      addr
						if (size == SIZE_D)
						{
							mode[0].addr();
							decode(mode, bytes);

							u32 const address = ea(mode[0]);

							SP -= 4;
							mem_write<u16>(ST_ODT, SP, m_mod);
							SP -= 4;
							mem_write<u32>(ST_ODT, SP, m_pc + bytes);

							u32 const desc = mem_read<u32>(ST_ODT, address);

							m_mod = u16(desc);
							m_sb = mem_read<u32>(ST_ODT, m_mod + 0);
							m_pc = mem_read<u32>(ST_ODT, m_mod + 8) + (desc >> 16);

							tex = mode[0].tea + top(SIZE_W, address) * 3 + top(SIZE_D, m_mod) * 3 + 13;
							m_sequential = false;
						}
						else
							interrupt(UND, m_pc);
						break;
					case 0x2:
						// BICPSRi src
						//         gen
						//         read.[BW]
						if (size == SIZE_B || size == SIZE_W)
						{
							mode[0].read_i(size);
							decode(mode, bytes);

							if (size == SIZE_B || !(m_psr & PSR_U))
							{
								u16 const src = gen_read(mode[0]);

								m_psr &= ~src;

								tex = mode[0].tea + ((size == SIZE_B) ? 18 : 30);
							}
							else
								interrupt(ILL, m_pc);
						}
						else
							interrupt(UND, m_pc);
						break;
					case 0x4:
						// JUMP dst
						//      gen
						//      addr
						if (size == SIZE_D)
						{
							mode[0].addr();
							decode(mode, bytes);

							m_pc = ea(mode[0]);
							m_sequential = false;

							tex = mode[0].tea + 2;
						}
						else
							interrupt(UND, m_pc);
						break;
					case 0x6:
						// BISPSRi src
						//         gen
						//         read.[BW]
						if (size == SIZE_B || size == SIZE_W)
						{
							mode[0].read_i(size);
							decode(mode, bytes);

							if (size == SIZE_B || !(m_psr & PSR_U))
							{
								u16 const src = gen_read(mode[0]);

								m_psr |= src & PSR_MSK;

								tex = mode[0].tea + ((size == SIZE_B) ? 18 : 30);
							}
							else
								interrupt(ILL, m_pc);
						}
						else
							interrupt(UND, m_pc);
						break;
					case 0xa:
						// ADJSPi src
						//        gen
						//        read.i
						{
							mode[0].read_i(size);
							decode(mode, bytes);

							s32 const src = gen_read_sx(mode[0]);

							SP -= src;

							tex = mode[0].tea + 6;
						}
						break;
					case 0xc:
						// JSR dst
						//     gen
						//     addr
						if (size == SIZE_D)
						{
							mode[0].addr();
							decode(mode, bytes);

							SP -= 4;
							mem_write<u32>(ST_ODT, SP, m_pc + bytes);

							m_pc = ea(mode[0]);
							m_sequential = false;

							// TODO: where does the TOPi come from?
							tex = mode[0].tea + top(SIZE_D, SP) + top(size) + 5;
						}
						else
							interrupt(UND, m_pc);
						break;
					case 0xe:
						// CASEi src
						//       gen
						//       read.i
						{
							mode[0].read_i(size);
							decode(mode, bytes);

							s32 const src = gen_read_sx(mode[0]);

							m_pc += src;
							m_sequential = false;

							tex = mode[0].tea + 4;
						}
						break;
					default:
						interrupt(UND, m_pc);
						break;
					}
					break;
				}
			}
			else if ((opbyte & 3) != 2)
			{
				// format 4: xxxx xyyy yyoo ooii
				u16 const opword = (u16(fetch<u8>(bytes)) << 8) | opbyte;

				addr_mode mode[] = { addr_mode(BIT(opword, 11, 5)), addr_mode(BIT(opword, 6, 5)) };
				size_code const size = size_code(opbyte & 3);

				switch (BIT(opbyte, 2, 4))
				{
				case 0x0:
					// ADDi src,dst
					//      gen,gen
					//      read.i,rmw.i
					{
						mode[0].read_i(size);
						mode[1].rmw_i(size);
						decode(mode, bytes);

						u32 const src1 = gen_read(mode[0]);
						u32 const src2 = gen_read(mode[1]);

						u32 const dst = src1 + src2;
						flags(src1, src2, dst, size, false);

						gen_write(mode[1], dst);

						if (mode[1].type != REG)
							tex = mode[0].tea + mode[1].tea + 3;
						else if (mode[0].type != REG)
							tex = mode[0].tea + 4;
						else
							tex = 4;
					}
					break;
				case 0x1:
					// CMPi src1,src2
					//      gen,gen
					//      read.i,read.i
					{
						mode[0].read_i(size);
						mode[1].read_i(size);
						decode(mode, bytes);

						u32 const src1 = gen_read(mode[0]);
						u32 const src2 = gen_read(mode[1]);

						m_psr &= ~(PSR_N | PSR_Z | PSR_L);

						if ((size == SIZE_D && s32(src1) > s32(src2))
						|| ((size == SIZE_W && s16(src1) > s16(src2))
						|| ((size == SIZE_B && s8(src1) > s8(src2)))))
							m_psr |= PSR_N;

						if (src1 == src2)
							m_psr |= PSR_Z;

						if ((size == SIZE_D && u32(src1) > u32(src2))
						|| ((size == SIZE_W && u16(src1) > u16(src2))
						|| ((size == SIZE_B && u8(src1) > u8(src2)))))
							m_psr |= PSR_L;

						if (mode[1].type != REG)
							tex = mode[0].tea + mode[1].tea + 3;
						else if (mode[0].type != REG)
							tex = mode[0].tea + 3;
						else
							tex = 3;
					}
					break;
				case 0x2:
					// BICi src,dst
					//      gen,gen
					//      read.i,rmw.i
					{
						mode[0].read_i(size);
						mode[1].rmw_i(size);
						decode(mode, bytes);

						u32 const src = gen_read(mode[0]);
						u32 const dst = gen_read(mode[1]);

						gen_write(mode[1], dst & ~src);

						if (mode[1].type != REG)
							tex = mode[0].tea + mode[1].tea + 3;
						else if (mode[0].type != REG)
							tex = mode[0].tea + 4;
						else
							tex = 4;
					}
					break;
				case 0x4:
					// ADDCi src,dst
					//      gen,gen
					//      read.i,rmw.i
					{
						mode[0].read_i(size);
						mode[1].rmw_i(size);
						decode(mode, bytes);

						u32 const src1 = gen_read(mode[0]);
						u32 const src2 = gen_read(mode[1]);

						u32 const dst = src1 + src2 + (m_psr & PSR_C);
						flags(src1, src2, dst, size, false);

						gen_write(mode[1], dst);

						if (mode[1].type != REG)
							tex = mode[0].tea + mode[1].tea + 3;
						else if (mode[0].type != REG)
							tex = mode[0].tea + 4;
						else
							tex = 4;
					}
					break;
				case 0x5:
					// MOVi src,dst
					//      gen,gen
					//      read.i,write.i
					{
						mode[0].read_i(size);
						mode[1].write_i(size);
						decode(mode, bytes);

						u32 const src = gen_read(mode[0]);

						gen_write(mode[1], src);

						if (mode[1].type != REG)
							tex = mode[0].tea + mode[1].tea + 1;
						else if (mode[0].type != REG)
							tex = mode[0].tea + 3;
						else
							tex = 3;
					}
					break;
				case 0x6:
					// ORi src,dst
					//      gen,gen
					//      read.i,rmw.i
					{
						mode[0].read_i(size);
						mode[1].rmw_i(size);
						decode(mode, bytes);

						u32 const src = gen_read(mode[0]);
						u32 const dst = gen_read(mode[1]);

						gen_write(mode[1], src | dst);

						if (mode[1].type != REG)
							tex = mode[0].tea + mode[1].tea + 3;
						else if (mode[0].type != REG)
							tex = mode[0].tea + 4;
						else
							tex = 4;
					}
					break;
				case 0x8:
					// SUBi src,dst
					//      gen,gen
					//      read.i,rmw.i
					{
						mode[0].read_i(size);
						mode[1].rmw_i(size);
						decode(mode, bytes);

						u32 const src1 = gen_read(mode[0]);
						u32 const src2 = gen_read(mode[1]);

						u32 const dst = src2 - src1;
						flags(src1, src2, dst, size, true);

						gen_write(mode[1], dst);

						if (mode[1].type != REG)
							tex = mode[0].tea + mode[1].tea + 3;
						else if (mode[0].type != REG)
							tex = mode[0].tea + 4;
						else
							tex = 4;
					}
					break;
				case 0x9:
					// ADDR src,dst
					//      gen,gen
					//      addr,write.D
					if (size == SIZE_D)
					{
						mode[0].addr();
						mode[1].write_i(size);
						decode(mode, bytes);

						gen_write(mode[1], ea(mode[0]));

						tex = (mode[1].type == REG) ? mode[0].tea + 3 : mode[0].tea + mode[1].tea + 2;
					}
					else
						interrupt(UND, m_pc);
					break;
				case 0xa:
					// ANDi src,dst
					//      gen,gen
					//      read.i,rmw.i
					{
						mode[0].read_i(size);
						mode[1].rmw_i(size);
						decode(mode, bytes);

						u32 const src = gen_read(mode[0]);
						u32 const dst = gen_read(mode[1]);

						gen_write(mode[1], src & dst);

						if (mode[1].type != REG)
							tex = mode[0].tea + mode[1].tea + 3;
						else if (mode[0].type != REG)
							tex = mode[0].tea + 4;
						else
							tex = 4;
					}
					break;
				case 0xc:
					// SUBCi src,dst
					//      gen,gen
					//      read.i,rmw.i
					{
						mode[0].read_i(size);
						mode[1].rmw_i(size);
						decode(mode, bytes);

						u32 const src1 = gen_read(mode[0]);
						u32 const src2 = gen_read(mode[1]);

						u32 const dst = src2 - src1 - (m_psr & PSR_C);
						flags(src1, src2, dst, size, true);

						gen_write(mode[1], dst);

						if (mode[1].type != REG)
							tex = mode[0].tea + mode[1].tea + 3;
						else if (mode[0].type != REG)
							tex = mode[0].tea + 4;
						else
							tex = 4;
					}
					break;
				case 0xd:
					// TBITi offset,base
					//       gen,gen
					//       read.i,regaddr
					{
						mode[0].read_i(size);
						mode[1].regaddr();
						decode(mode, bytes);

						s32 const offset = gen_read_sx(mode[0]);

						if (mode[1].type == REG)
						{
							if (BIT(m_r[mode[1].gen], offset & 31))
								m_psr |= PSR_F;
							else
								m_psr &= ~PSR_F;

							tex = mode[0].tea + 4;
						}
						else
						{
							u8 const byte = mem_read<u8>(ST_ODT, ea(mode[1]) + (offset >> 3));

							if (BIT(byte, offset & 7))
								m_psr |= PSR_F;
							else
								m_psr &= ~PSR_F;

							tex = mode[0].tea + mode[1].tea + top(SIZE_B) + 14;
						}
					}
					break;
				case 0xe:
					// XORi src,dst
					//      gen,gen
					//      read.i,rmw.i
					{
						mode[0].read_i(size);
						mode[1].rmw_i(size);
						decode(mode, bytes);

						u32 const src = gen_read(mode[0]);
						u32 const dst = gen_read(mode[1]);

						gen_write(mode[1], src ^ dst);

						if (mode[1].type != REG)
							tex = mode[0].tea + mode[1].tea + 3;
						else if (mode[0].type != REG)
							tex = mode[0].tea + 4;
						else
							tex = 4;
					}
					break;
				}
			}
			else switch (opbyte)
			{
			case 0x0e:
				// format 5: 0000 0sss s0oo ooii 0000 1110
				{
					u16 const opword = fetch<u16>(bytes);

					size_code const size = size_code(opword & 3);

					// string instruction options
					bool const translate = BIT(opword, 7);
					bool const backward = BIT(opword, 8);
					unsigned const uw = BIT(opword, 9, 2);

					switch (BIT(opword, 2, 4))
					{
					case 0:
						// MOVSi options
						tex = (translate || backward || uw) ? 54 : 18;

						m_psr &= ~PSR_F;
						while (m_r[0])
						{
							u32 data =
								(size == SIZE_D) ? mem_read<u32>(ST_ODT, m_r[1]) :
								(size == SIZE_W) ? mem_read<u16>(ST_ODT, m_r[1]) :
								mem_read<u8>(ST_ODT, m_r[1]);

							if (translate)
								data = mem_read<u8>(ST_ODT, m_r[3] + u8(data));

							tex += top(size, m_r[1]) + (translate ? top(SIZE_B) + 27 : (backward || uw) ? 24 : 13);

							bool const match = !((m_r[4] ^ data) & size_mask[size]);
							if ((uw == 1 && !match) || (uw == 3 && match))
							{
								m_psr |= PSR_F;
								break;
							}

							if (size == SIZE_D)
								mem_write<u32>(ST_ODT, m_r[2], data);
							else if (size == SIZE_W)
								mem_write<u16>(ST_ODT, m_r[2], data);
							else
								mem_write<u8>(ST_ODT, m_r[2], data);

							tex += top(size, m_r[2]);

							if (backward)
							{
								m_r[1] -= size + 1;
								m_r[2] -= size + 1;
							}
							else
							{
								m_r[1] += size + 1;
								m_r[2] += size + 1;
							}

							m_r[0]--;
						}
						break;
					case 1:
						// CMPSi options
						tex = 53;

						m_psr |= PSR_Z;
						m_psr &= ~(PSR_N | PSR_F | PSR_L);
						while (m_r[0])
						{
							u32 src1 =
								(size == SIZE_D) ? mem_read<u32>(ST_ODT, m_r[1]) :
								(size == SIZE_W) ? mem_read<u16>(ST_ODT, m_r[1]) :
								mem_read<u8>(ST_ODT, m_r[1]);
							u32 src2 =
								(size == SIZE_D) ? mem_read<u32>(ST_ODT, m_r[2]) :
								(size == SIZE_W) ? mem_read<u16>(ST_ODT, m_r[2]) :
								mem_read<u8>(ST_ODT, m_r[2]);

							if (translate)
								src1 = mem_read<u8>(ST_ODT, m_r[3] + u8(src1));

							tex += top(size, m_r[1]) + top(size, m_r[2]) + (translate ? top(SIZE_B) + 38 : 35);

							bool const match = !((m_r[4] ^ src1) & size_mask[size]);
							if ((uw == 1 && !match) || (uw == 3 && match))
							{
								m_psr |= PSR_F;
								break;
							}

							if (src1 != src2)
							{
								m_psr &= ~PSR_Z;

								if ((size == SIZE_D && s32(src1) > s32(src2))
								|| ((size == SIZE_W && s16(src1) > s16(src2))
								|| ((size == SIZE_B && s8(src1) > s8(src2)))))
									m_psr |= PSR_N;

								if ((size == SIZE_D && u32(src1) > u32(src2))
								|| ((size == SIZE_W && u16(src1) > u16(src2))
								|| ((size == SIZE_B && u8(src1) > u8(src2)))))
									m_psr |= PSR_L;

								break;
							}

							if (backward)
							{
								m_r[1] -= size + 1;
								m_r[2] -= size + 1;
							}
							else
							{
								m_r[1] += size + 1;
								m_r[2] += size + 1;
							}

							m_r[0]--;
						}
						break;
					case 2:
						// SETCFG cfglist
						//        short
						if (!(m_psr & PSR_U))
						{
							m_cfg = BIT(opword, 7, (type() == NS32332) ? 8 : 4);

							tex = 15;
						}
						else
							interrupt(ILL, m_pc);
						break;
					case 3:
						// SKPSi options
						tex = 51;

						m_psr &= ~PSR_F;
						while (m_r[0])
						{
							u32 data =
								(size == SIZE_D) ? mem_read<u32>(ST_ODT, m_r[1]) :
								(size == SIZE_W) ? mem_read<u16>(ST_ODT, m_r[1]) :
								mem_read<u8>(ST_ODT, m_r[1]);

							if (translate)
								data = mem_read<u8>(ST_ODT, m_r[3] + u8(data));

							tex += top(size, m_r[1]) + (translate ? top(SIZE_B) + 30 : 27);

							bool const match = !((m_r[4] ^ data) & size_mask[size]);
							if ((uw == 1 && !match) || (uw == 3 && match))
							{
								m_psr |= PSR_F;
								break;
							}

							if (backward)
								m_r[1] -= size + 1;
							else
								m_r[1] += size + 1;

							m_r[0]--;
						}
						break;
					default:
						interrupt(UND, m_pc);
						break;
					}
				}
				break;
			case 0x4e:
				// format 6: xxxx xyyy yyoo ooii 0100 1110
				{
					u16 const opword = fetch<u16>(bytes);

					addr_mode mode[] = { addr_mode(BIT(opword, 11, 5)), addr_mode(BIT(opword, 6, 5)) };
					size_code const size = size_code(opword & 3);

					switch (BIT(opword, 2, 4))
					{
					case 0x0:
						// ROTi count,dst
						//      gen,gen
						//      read.B,rmw.i
						{
							mode[0].read_i(SIZE_B);
							mode[1].rmw_i(size);
							decode(mode, bytes);

							s32 const count = gen_read_sx(mode[0]);
							u32 const src = gen_read(mode[1]);

							unsigned const limit = (size + 1) * 8 - 1;
							u32 const dst = ((src << (count & limit)) & size_mask[size]) | ((src & size_mask[size]) >> (limit - (count & limit) + 1));

							gen_write(mode[1], dst);

							tex = mode[0].tea + mode[1].tea + 14 + (count & limit);
						}
						break;
					case 0x1:
						// ASHi count,dst
						//      gen,gen
						//      read.B,rmw.i
						{
							mode[0].read_i(SIZE_B);
							mode[1].rmw_i(size);
							decode(mode, bytes);

							s32 const count = gen_read_sx(mode[0]);
							s32 const src = gen_read_sx(mode[1]);

							u32 const dst = (count < 0) ? (src >> -count) : (src << count);

							gen_write(mode[1], dst);

							tex = mode[0].tea + mode[1].tea + 14 + std::abs(count);
						}
						break;
					case 0x2:
						// CBITi offset,base
						//       gen,gen
						//       read.i,regaddr
					case 0x3:
						// CBITIi offset,base
						//        gen,gen
						//       read.i,regaddr
						{
							mode[0].read_i(size);
							mode[1].regaddr();
							decode(mode, bytes);

							s32 const offset = gen_read_sx(mode[0]);

							if (mode[1].type == REG)
							{
								if (BIT(m_r[mode[1].gen], offset & 31))
									m_psr |= PSR_F;
								else
									m_psr &= ~PSR_F;

								m_r[mode[1].gen] &= ~(1U << (offset & 31));

								tex = mode[0].tea + 7;
							}
							else
							{
								u32 const byte_ea = ea(mode[1]) + (offset >> 3);
								u8 const byte = mem_read<u8>(ST_ODT, byte_ea);

								if (BIT(byte, offset & 7))
									m_psr |= PSR_F;
								else
									m_psr &= ~PSR_F;

								mem_write<u8>(ST_ODT, byte_ea, byte & ~(1U << (offset & 7)));

								tex = mode[0].tea + mode[1].tea + top(SIZE_B) * 2 + 15;
							}
						}
						break;
					case 0x4:
						interrupt(UND, m_pc);
						break;
					case 0x5:
						// LSHi count,dst
						//      gen,gen
						//      read.B,rmw.i
						{
							mode[0].read_i(SIZE_B);
							mode[1].rmw_i(size);
							decode(mode, bytes);

							s32 const count = gen_read_sx(mode[0]);
							u32 const src = gen_read(mode[1]);

							u32 const dst = (count < 0) ? (src >> -count) : (src << count);

							gen_write(mode[1], dst);

							tex = mode[0].tea + mode[1].tea + 14 + std::abs(count);
						}
						break;
					case 0x6:
						// SBITi offset,base
						//       gen,gen
						//       read.i,regaddr
					case 0x7:
						// SBITIi offset,base
						//       gen,gen
						//       read.i,regaddr
						{
							mode[0].read_i(size);
							mode[1].regaddr();
							decode(mode, bytes);

							s32 const offset = gen_read_sx(mode[0]);

							if (mode[1].type == REG)
							{
								if (BIT(m_r[mode[1].gen], offset & 31))
									m_psr |= PSR_F;
								else
									m_psr &= ~PSR_F;

								m_r[mode[1].gen] |= (1U << (offset & 31));

								tex = mode[0].tea + 7;
							}
							else
							{
								u32 const byte_ea = ea(mode[1]) + (offset >> 3);
								u8 const byte = mem_read<u8>(ST_ODT, byte_ea);

								if (BIT(byte, offset & 7))
									m_psr |= PSR_F;
								else
									m_psr &= ~PSR_F;

								mem_write<u8>(ST_ODT, byte_ea, byte | (1U << (offset & 7)));

								tex = mode[0].tea + mode[1].tea + top(SIZE_B) * 2 + 15;
							}
						}
						break;
					case 0x8:
						// NEGi src,dst
						//      gen,gen
						//      read.i,write.i
						{
							mode[0].read_i(size);
							mode[1].write_i(size);
							decode(mode, bytes);

							u32 const src = gen_read(mode[0]);

							if (src)
								m_psr |= PSR_C;
							else
								m_psr &= ~PSR_C;

							if ((src ^ ~(size_mask[size] >> 1)) & size_mask[size])
							{
								m_psr &= ~PSR_F;
								gen_write(mode[1], -src);
							}
							else
							{
								m_psr |= PSR_F;
								gen_write(mode[1], src);
							}

							tex = mode[0].tea + mode[1].tea + 5;
						}
						break;
					case 0x9:
						// NOTi src,dst
						//      gen,gen
						//      read.i,write.i
						{
							mode[0].read_i(size);
							mode[1].write_i(size);
							decode(mode, bytes);

							u32 const src = gen_read(mode[0]);

							gen_write(mode[1], src ^ 1U);

							tex = mode[0].tea + mode[1].tea + 5;
						}
						break;
					case 0xa:
						interrupt(UND, m_pc);
						break;
					case 0xb:
						// SUBPi src,dst
						//      gen,gen
						//      read.i,rmw.i
						fatalerror("unimplemented: subp (%s)\n", machine().describe_context());

						// TODO: tcy 16/18
						break;
					case 0xc:
						// ABSi src,dst
						//      gen,gen
						//      read.i,write.i
						{
							mode[0].read_i(size);
							mode[1].write_i(size);
							decode(mode, bytes);

							s32 const src = gen_read_sx(mode[0]);

							m_psr &= ~PSR_F;
							if (src == s32(0x80000000) >> (32 - (size + 1) * 8))
							{
								m_psr |= PSR_F;
								gen_write(mode[1], src);
							}
							else
								gen_write(mode[1], std::abs(src));

							tex = mode[0].tea + mode[1].tea + ((src < 0) ? 9 : 8);
						}
						break;
					case 0xd:
						// COMi src,dst
						//      gen,gen
						//      read.i,write.i
						{
							mode[0].read_i(size);
							mode[1].write_i(size);
							decode(mode, bytes);

							u32 const src = gen_read(mode[0]);

							gen_write(mode[1], ~src);

							tex = mode[0].tea + mode[1].tea + 7;
						}
						break;
					case 0xe:
						// IBITi offset,base
						//       gen,gen
						//       read.i,regaddr
						{
							mode[0].read_i(size);
							mode[1].regaddr();
							decode(mode, bytes);

							s32 const offset = gen_read_sx(mode[0]);

							if (mode[1].type == REG)
							{
								if (BIT(m_r[mode[1].gen], offset & 31))
									m_psr |= PSR_F;
								else
									m_psr &= ~PSR_F;

								m_r[mode[1].gen] ^= (1U << (offset & 31));

								tex = mode[0].tea + 9;
							}
							else
							{
								u32 const byte_ea = ea(mode[1]) + (offset >> 3);
								u8 const byte = mem_read<u8>(ST_ODT, byte_ea);

								if (BIT(byte, offset & 7))
									m_psr |= PSR_F;
								else
									m_psr &= ~PSR_F;

								mem_write<u8>(ST_ODT, byte_ea, byte ^ (1U << (offset & 7)));

								tex = mode[0].tea + mode[1].tea + top(SIZE_B) * 2 + 17;
							}
						}
						break;
					case 0xf:
						// ADDPi src,dst
						//       gen,gen
						//       read.i,rmw.i
						fatalerror("unimplemented: addp (%s)\n", machine().describe_context());

						// TODO: tcy 16/18
						break;
					}
				}
				break;
			case 0xce:
				// format 7: xxxx xyyy yyoo ooii 1100 1110
				{
					u16 const opword = fetch<u16>(bytes);

					addr_mode mode[] = { addr_mode(BIT(opword, 11, 5)), addr_mode(BIT(opword, 6, 5)) };
					size_code const size = size_code(opword & 3);

					switch (BIT(opword, 2, 4))
					{
					case 0x0:
						// MOVMi block1,block2,length
						//       gen,gen,disp
						//       addr,addr
						{
							mode[0].addr();
							mode[1].addr();
							decode(mode, bytes);

							u32 block1 = ea(mode[0]);
							u32 block2 = ea(mode[1]);
							unsigned const num = displacement(bytes) / (size + 1) + 1;

							// TODO: aligned/unaligned transfers?
							for (unsigned i = 0; i < num; i++)
							{
								switch (size)
								{
								case SIZE_B: mem_write<u8>(ST_ODT, block2, mem_read<u8>(ST_ODT, block1)); break;
								case SIZE_W: mem_write<u16>(ST_ODT, block2, mem_read<u16>(ST_ODT, block1)); break;
								case SIZE_D: mem_write<u32>(ST_ODT, block2, mem_read<u32>(ST_ODT, block1)); break;
								default:
									// can't happen
									break;
								}

								block1 += (size + 1);
								block2 += (size + 1);
							}

							tex = mode[0].tea + mode[1].tea + (top(size, block1) + top(size, block2)) * num + 3 * num + 20;
						}
						break;
					case 0x1:
						// CMPMi block1,block2,length
						//       gen,gen,disp
						//       addr,addr
						{
							mode[0].addr();
							mode[1].addr();
							decode(mode, bytes);

							u32 block1 = ea(mode[0]);
							u32 block2 = ea(mode[1]);
							unsigned const num = displacement(bytes) / (size + 1) + 1;

							tex = mode[0].tea + mode[1].tea + 24;

							m_psr |= PSR_Z;
							m_psr &= ~(PSR_N | PSR_L);

							// TODO: aligned/unaligned transfers?
							for (unsigned i = 0; i < num; i++)
							{
								s32 int1 = 0;
								s32 int2 = 0;

								switch (size)
								{
								case SIZE_B:
									int1 = s8(mem_read<u8>(ST_ODT, block1));
									int2 = s8(mem_read<u8>(ST_ODT, block2));
									break;
								case SIZE_W:
									int1 = s16(mem_read<u16>(ST_ODT, block1));
									int2 = s16(mem_read<u16>(ST_ODT, block2));
									break;
								case SIZE_D:
									int1 = s32(mem_read<u32>(ST_ODT, block1));
									int2 = s32(mem_read<u32>(ST_ODT, block2));
									break;
								default:
									// can't happen
									break;
								}

								tex += top(size, block1) + top(size, block2) + 9;

								if (int1 != int2)
								{
									m_psr &= ~PSR_Z;
									if (int1 > int2)
										m_psr |= PSR_N;
									if (u32(int1) > u32(int2))
										m_psr |= PSR_L;

									break;
								}

								block1 += (size + 1);
								block2 += (size + 1);
							}
						}
						break;
					case 0x2:
						// INSSi src,base,offset,length
						//       gen,gen,imm
						//       read.i,regaddr
						{
							mode[0].read_i(size);
							mode[1].regaddr();
							decode(mode, bytes);

							u8 const imm = fetch<u8>(bytes);
							unsigned const offset = imm >> 5;
							u32 const mask = ((2ULL << (imm & 31)) - 1) << offset;

							u32 const src = gen_read(mode[0]);
							u32 const base = gen_read(mode[1]);

							gen_write(mode[1], (base & ~mask) | ((src << offset) & mask));

							// TODO: tcy 39-49
							tex = mode[0].tea + mode[1].tea + 39;
						}
						break;
					case 0x3:
						// EXTSi base,dst,offset,length
						//       gen,gen,imm
						//       regaddr,write.i
						{
							mode[0].regaddr();
							mode[1].write_i(size);
							decode(mode, bytes);

							u8 const imm = fetch<u8>(bytes);
							unsigned const offset = imm >> 5;
							u32 const mask = (2ULL << (imm & 31)) - 1;

							u32 const base = gen_read(mode[0]);
							u32 const dst = (base >> offset) & mask;

							gen_write(mode[1], dst);

							// TODO: tcy 26-36
							tex = mode[0].tea + mode[1].tea + 26;
						}
						break;
					case 0x4:
						// MOVXBW src,dst
						//        gen,gen
						//        read.B,write.W
						if (size == SIZE_B)
						{
							mode[0].read_i(size);
							mode[1].write_i(SIZE_W);
							decode(mode, bytes);

							u8 const src = gen_read(mode[0]);
							s16 const dst = s8(src);

							gen_write(mode[1], dst);

							tex = mode[0].tea + mode[1].tea + 6;
						}
						else
							interrupt(UND, m_pc);
						break;
					case 0x5:
						// MOVZBW src,dst
						//        gen,gen
						//        read.B,write.W
						if (size == SIZE_B)
						{
							mode[0].read_i(size);
							mode[1].write_i(SIZE_W);
							decode(mode, bytes);

							u8 const src = gen_read(mode[0]);
							u16 const dst = src;

							gen_write(mode[1], dst);

							tex = mode[0].tea + mode[1].tea + 5;
						}
						else
							interrupt(UND, m_pc);
						break;
					case 0x6:
						// MOVZiD src,dst
						//        gen,gen
						//        read.[BW],write.D
						if (size == SIZE_B || size == SIZE_W)
						{
							mode[0].read_i(size);
							mode[1].write_i(SIZE_D);
							decode(mode, bytes);

							u32 const src = gen_read(mode[0]);

							gen_write(mode[1], src);

							tex = mode[0].tea + mode[1].tea + 5;
						}
						else
							interrupt(UND, m_pc);
						break;
					case 0x7:
						// MOVXiD src,dst
						//        gen,gen
						//        read.[BW],write.D
						if (size == SIZE_B || size == SIZE_W)
						{
							mode[0].read_i(size);
							mode[1].write_i(SIZE_D);
							decode(mode, bytes);

							u32 const src = gen_read(mode[0]);
							s32 const dst = (size == SIZE_W) ? s16(src) : s8(src);

							gen_write(mode[1], dst);

							tex = mode[0].tea + mode[1].tea + 6;
						}
						else
							interrupt(UND, m_pc);
						break;
					case 0x8:
						// MULi src,dst
						//      gen,gen
						//      read.i,rmw.i
						{
							mode[0].read_i(size);
							mode[1].rmw_i(size);
							decode(mode, bytes);

							u32 const src1 = gen_read(mode[0]);
							u32 const src2 = gen_read(mode[1]);

							u32 const dst = src1 * src2;

							gen_write(mode[1], dst);

							tex = mode[0].tea + mode[1].tea + 15 + (size + 1) * 16; // 2+2 + 15 + 16 ==
						}
						break;
					case 0x9:
						// MEIi src,dst
						//      gen,gen
						//      read.i,rmw.2i
						{
							mode[0].read_i(size);
							mode[1].rmw_i(size);
							decode(mode, bytes);

							u32 const src1 = gen_read(mode[0]);
							u32 const src2 = gen_read(mode[1]);

							mode[1].rmw_i(size_code(size * 2 + 1));
							u64 const dst = mulu_32x32(src1, src2);

							if (mode[1].type == REG)
							{
								m_r[mode[1].gen ^ 0] = (m_r[mode[1].gen ^ 0] & ~size_mask[size]) | ((dst >> 0) & size_mask[size]);
								m_r[mode[1].gen ^ 1] = (m_r[mode[1].gen ^ 1] & ~size_mask[size]) | ((dst >> ((size + 1) * 8)) & size_mask[size]);
							}
							else
								// TODO: write high dword first
								gen_write(mode[1], dst);

							tex = mode[0].tea + mode[1].tea + 23 + (size + 1) * 16;
						}
						break;
					case 0xa:
						interrupt(UND, m_pc);
						break;
					case 0xb:
						// DEIi src,dst
						//      gen,gen
						//      read.i,rmw.2i
						{
							mode[0].read_i(size);
							mode[1].rmw_i(size_code(size * 2 + 1));
							decode(mode, bytes);

							u32 const src1 = gen_read(mode[0]);
							if (src1)
							{
								u64 const src2 = (mode[1].type == REG)
									? (u64(m_r[mode[1].gen ^ 1] & size_mask[size]) << ((size + 1) * 8)) | (m_r[mode[1].gen ^ 0] & size_mask[size])
									: gen_read(mode[1]);

								u32 const quotient = src2 / src1;
								u32 const remainder = src2 % src1;

								if (mode[1].type == REG)
								{
									m_r[mode[1].gen ^ 0] = (m_r[mode[1].gen ^ 0] & ~size_mask[size]) | (remainder & size_mask[size]);
									m_r[mode[1].gen ^ 1] = (m_r[mode[1].gen ^ 1] & ~size_mask[size]) | (quotient & size_mask[size]);

									tex = mode[0].tea + 31 + (size + 1) * 16;
								}
								else
								{
									gen_write(mode[1], (u64(quotient) << ((size + 1) * 8)) | remainder);

									tex = mode[0].tea + mode[1].tea + 38 + (size + 1) * 16;
								}
							}
							else
								interrupt(DVZ, m_pc);
						}
						break;
					case 0xc:
						// QUOi src,dst
						//      gen,gen
						//      read.i,rmw.i
						{
							mode[0].read_i(size);
							mode[1].rmw_i(size);
							decode(mode, bytes);

							s32 const src1 = gen_read_sx(mode[0]);
							if (src1)
							{
								s32 const src2 = gen_read_sx(mode[1]);

								s32 const dst = src2 / src1;

								gen_write(mode[1], dst);

								// TODO: tcy 49-55
								tex = mode[0].tea + mode[1].tea + 49 + (size + 1) * 16;
							}
							else
								interrupt(DVZ, m_pc);
						}
						break;
					case 0xd:
						// REMi src,dst
						//      gen,gen
						//      read.i,rmw.i
						{
							mode[0].read_i(size);
							mode[1].rmw_i(size);
							decode(mode, bytes);

							s32 const src1 = gen_read_sx(mode[0]);
							if (src1)
							{
								s32 const src2 = gen_read_sx(mode[1]);

								s32 const dst = src2 % src1;

								gen_write(mode[1], dst);

								// TODO: tcy 57-62
								tex = mode[0].tea + mode[1].tea + 57 + (size + 1) * 16;
							}
							else
								interrupt(DVZ, m_pc);
						}
						break;
					case 0xe:
						// MODi src,dst
						//      gen,gen
						//      read.i,rmw.i
						{
							mode[0].read_i(size);
							mode[1].rmw_i(size);
							decode(mode, bytes);

							s32 const src1 = gen_read_sx(mode[0]);
							if (src1)
							{
								s32 const src2 = gen_read_sx(mode[1]);

								s32 const dst = (src1 + (src2 % src1)) % src1;

								gen_write(mode[1], dst);

								// TODO: tcy 54-73
								tex = mode[0].tea + mode[1].tea + 54 + (size + 1) * 16;
							}
							else
								interrupt(DVZ, m_pc);
						}
						break;
					case 0xf:
						// DIVi src,dst
						//      gen,gen
						//      read.i,rmw.i
						{
							mode[0].read_i(size);
							mode[1].rmw_i(size);
							decode(mode, bytes);

							s32 const src1 = gen_read_sx(mode[0]);
							if (src1)
							{
								s32 const src2 = gen_read_sx(mode[1]);

								s32 const quotient = src2 / src1;
								s32 const remainder = src2 % src1;

								if ((quotient < 0) && remainder)
									gen_write(mode[1], quotient - 1);
								else
									gen_write(mode[1], quotient);

								// TODO: tcy 58-68
								tex = mode[0].tea + mode[1].tea + 58 + (size + 1) * 16;
							}
							else
								interrupt(DVZ, m_pc);
						}
						break;
					}
				}
				break;
			case 0x2e:
			case 0x6e:
			case 0xae:
			case 0xee:
				// format 8: xxxx xyyy yyrr roii oo10 1110
				{
					u16 const opword = fetch<u16>(bytes);

					addr_mode mode[] = { addr_mode(BIT(opword, 11, 5)), addr_mode(BIT(opword, 6, 5)) };
					unsigned const reg = BIT(opword, 3, 3);
					size_code const size = size_code(opword & 3);

					switch ((opword & 4) | BIT(opbyte, 6, 2))
					{
					case 0:
						// EXTi offset,base,dst,length
						//      reg,gen,gen,disp
						//          regaddr,write.i
						{
							mode[0].regaddr();
							mode[1].write_i(size);
							decode(mode, bytes);

							s32 const offset = m_r[reg];
							s32 const length = displacement(bytes);
							u32 const mask = (1U << length) - 1;

							if (mode[0].type == REG)
							{
								gen_write(mode[1], (m_r[mode[0].gen] >> (offset & 31)) & mask);

								// TODO: tcy 17-51
								tex = mode[0].tea + mode[1].tea + 17;
							}
							else
							{
								u32 const base_ea = ea(mode[0]) + (offset >> 3);
								u32 const base = mem_read<u32>(ST_ODT, base_ea);

								gen_write(mode[1], (base >> (offset & 7)) & mask);

								// TODO: tcy 19-29
								tex = mode[0].tea + mode[1].tea + top(SIZE_D, base_ea) + 19;
							}
						}
						break;
					case 1:
						// CVTP offset,base,dst
						//      reg,gen,gen
						//          addr,write.D
						if (size == SIZE_D)
						{
							mode[0].addr();
							mode[1].write_i(size);
							decode(mode, bytes);

							s32 const offset = s32(m_r[reg]);
							u32 const base = ea(mode[0]);

							gen_write(mode[1], base * 8 + offset);

							tex = mode[0].tea + mode[1].tea + 7;
						}
						else
							interrupt(UND, m_pc);
						break;
					case 2:
						// INSi offset,src,base,length
						//      reg,gen,gen,disp
						//          read.i,regaddr
						{
							mode[0].read_i(size);
							mode[1].regaddr();
							decode(mode, bytes);

							s32 const offset = m_r[reg];
							s32 const length = displacement(bytes);
							u32 const src = gen_read(mode[0]);

							if (mode[1].type == REG)
							{
								u32 const mask = ((1U << length) - 1) << (offset & 31);

								m_r[mode[1].gen] = (m_r[mode[1].gen] & ~mask) | ((src << (offset & 31)) & mask);

								// TODO: tcy 28-96
								tex = mode[0].tea + 28;
							}
							else
							{
								u32 const base_ea = ea(mode[1]) + (offset >> 3);
								u32 const base = mem_read<u32>(ST_ODT, base_ea);
								u32 const mask = ((1U << length) - 1) << (offset & 7);

								mem_write<u32>(ST_ODT, base_ea, (base & ~mask) | ((src << (offset & 7)) & mask));

								// TODO: tcy 29-39
								tex = mode[0].tea + mode[1].tea + top(SIZE_D, base_ea) * 2 + 29;
							}
						}
						break;
					case 3:
						// CHECKi dst,bounds,src
						//        reg,gen,gen
						//            addr,read.i
						{
							mode[0].addr();
							mode[1].read_i(size);
							decode(mode, bytes);

							u32 const bounds = ea(mode[0]);
							s32 const src = gen_read_sx(mode[1]);

							s32 lower = 0;
							s32 upper = 0;
							switch (size)
							{
							case SIZE_B:
								upper = s8(mem_read<u8>(ST_ODT, bounds + 0));
								lower = s8(mem_read<u8>(ST_ODT, bounds + 1));
								break;
							case SIZE_W:
								upper = s16(mem_read<u16>(ST_ODT, bounds + 0));
								lower = s16(mem_read<u16>(ST_ODT, bounds + 2));
								break;
							case SIZE_D:
								upper = s32(mem_read<u32>(ST_ODT, bounds + 0));
								lower = s32(mem_read<u32>(ST_ODT, bounds + 4));
								break;
							default:
								// can't happen
								break;
							}

							if (src >= lower && src <= upper)
							{
								m_psr &= ~PSR_F;

								tex = mode[0].tea + mode[1].tea + top(size, bounds) * 2 + 11;
							}
							else
							{
								m_psr |= PSR_F;

								tex = mode[0].tea + mode[1].tea + top(size, bounds) * 2 + ((src >= lower) ? 7 : 10);
							}

							// updating the destination when out of bounds
							// is undefined, but required by DB32016 firmware
							m_r[reg] = (src - lower) & size_mask[size];
						}
						break;
					case 4:
						// INDEXi accum,length,index
						//        reg,gen,gen
						//            read.i,read.i
						{
							mode[0].read_i(size);
							mode[1].read_i(size);
							decode(mode, bytes);

							u32 const length = gen_read(mode[0]);
							u32 const index = gen_read(mode[1]);

							m_r[reg] = m_r[reg] * (length + 1) + index;

							tex = mode[0].tea + mode[1].tea + 25 + (size + 1) * 16;
						}
						break;
					case 5:
						// FFSi base,offset
						//      gen,gen
						//      read.i,rmw.B
						{
							mode[0].read_i(size);
							mode[1].rmw_i(SIZE_B);
							decode(mode, bytes);

							u32 const base = gen_read(mode[0]);
							u32 offset = gen_read(mode[1]);
							unsigned const limit = (size + 1) * 8;

							m_psr |= PSR_F;
							while (offset < limit)
								if (BIT(base, offset))
								{
									m_psr &= ~PSR_F;
									break;
								}
								else
									offset++;

							gen_write(mode[1], offset & (limit - 1));

							// TODO: tcy 24-28
							tex = mode[0].tea + mode[1].tea + 24 + (size + 1) * 24;
						}
						break;
					case 6:
						// MOVSU/MOVUS src,dst
						//             gen,gen
						//             addr,addr
						if (!(m_psr & PSR_U))
						{
							if (reg == 1 || reg == 3)
							{
								mode[0].addr();
								mode[1].addr();
								decode(mode, bytes);

								switch (size)
								{
								case SIZE_B: mem_write<u8>(ST_ODT, ea(mode[1]), mem_read<u8>(ST_ODT, ea(mode[0]), reg == 3), reg == 1); break;
								case SIZE_W: mem_write<u16>(ST_ODT, ea(mode[1]), mem_read<u16>(ST_ODT, ea(mode[0]), reg == 3), reg == 1); break;
								case SIZE_D: mem_write<u32>(ST_ODT, ea(mode[1]), mem_read<u32>(ST_ODT, ea(mode[0]), reg == 3), reg == 1); break;
								default:
									// can't happen
									break;
								}

								tex = mode[0].tea + mode[1].tea + top(size) * 2 + 33;
							}
							else
								interrupt(UND, m_pc);
						}
						else
							interrupt(ILL, m_pc);
						break;
					}
				}
				break;
			case 0x3e:
				// format 9: xxxx xyyy yyoo ofii 0011 1110
				if (m_cfg & CFG_F)
				{
					u16 const opword = fetch<u16>(bytes);

					addr_mode mode[] = { addr_mode(BIT(opword, 11, 5)), addr_mode(BIT(opword, 6, 5)) };
					size_code const size_f = BIT(opword, 0) ? SIZE_D : SIZE_Q;
					size_code const size = size_code(opword & 3);

					switch (BIT(opword, 3, 3))
					{
					case 0:
						// MOVif src,dst
						//       gen,gen
						//       read.i,write.f
						mode[0].read_i(size);
						mode[1].write_f(size_f);
						decode(mode, bytes);

						if (slave(opbyte, opword, mode[0], mode[1]))
							interrupt(SLV, m_pc);
						break;
					case 1:
						// LFSR src
						//      gen
						//      read.D
						mode[0].read_i(size);
						decode(mode, bytes);

						if (slave(opbyte, opword, mode[0], mode[1]))
							interrupt(SLV, m_pc);
						break;
					case 2:
						// MOVLF src,dst
						//       gen,gen
						//       read.L,write.F
						mode[0].read_f(SIZE_Q);
						mode[1].write_f(size_f);
						decode(mode, bytes);

						if (slave(opbyte, opword, mode[0], mode[1]))
							interrupt(SLV, m_pc);
						break;
					case 3:
						// MOVFL src,dst
						//       gen,gen
						//       read.F,write.L
						mode[0].read_f(SIZE_D);
						mode[1].write_f(size_f);
						decode(mode, bytes);

						if (slave(opbyte, opword, mode[0], mode[1]))
							interrupt(SLV, m_pc);
						break;
					case 4:
						// ROUNDfi src,dst
						//         gen,gen
						//         read.f,write.i
						mode[0].read_f(size_f);
						mode[1].write_i(size);
						decode(mode, bytes);

						if (slave(opbyte, opword, mode[0], mode[1]))
							interrupt(SLV, m_pc);
						break;
					case 5:
						// TRUNCfi src,dst
						//         gen,gen
						//         read.f,write.i
						mode[0].read_f(size_f);
						mode[1].write_i(size);
						decode(mode, bytes);

						if (slave(opbyte, opword, mode[0], mode[1]))
							interrupt(SLV, m_pc);
						break;
					case 6:
						// SFSR dst
						//      gen
						//      write.D
						mode[0].write_i(size);
						decode(mode, bytes);

						if (slave(opbyte, opword, mode[1], mode[0]))
							interrupt(SLV, m_pc);
						break;
					case 7:
						// FLOORfi src,dst
						//         gen,gen
						//         read.f,write.i
						mode[0].read_f(size_f);
						mode[1].write_i(size);
						decode(mode, bytes);

						if (slave(opbyte, opword, mode[0], mode[1]))
							interrupt(SLV, m_pc);
						break;
					}
				}
				else
					interrupt(UND, m_pc);
				break;
			case 0x7e: // format 10
				interrupt(UND, m_pc);
				break;
			case 0xbe:
				// format 11: xxxx xyyy yyoo oo0f 1011 1110
				if (m_cfg & CFG_F)
				{
					u16 const opword = fetch<u16>(bytes);

					addr_mode mode[] = { addr_mode(BIT(opword, 11, 5)), addr_mode(BIT(opword, 6, 5)) };
					size_code const size_f = BIT(opword, 0) ? SIZE_D : SIZE_Q;

					switch (BIT(opword, 2, 4))
					{
					case 0x0:
						// ADDf src,dst
						//      gen,gen
						//      read.f,rmw.f
						mode[0].read_f(size_f);
						mode[1].rmw_f(size_f);
						decode(mode, bytes);

						if (slave(opbyte, opword, mode[0], mode[1]))
							interrupt(SLV, m_pc);
						break;
					case 0x1:
						// MOVf src,dst
						//      gen,gen
						//      read.f,write.f
						mode[0].read_f(size_f);
						mode[1].write_f(size_f);
						decode(mode, bytes);

						if (slave(opbyte, opword, mode[0], mode[1]))
							interrupt(SLV, m_pc);
						break;
					case 0x2:
						// CMPf src1,src2
						//      gen,gen
						//      read.f,read.f
						{
							mode[0].read_f(size_f);
							mode[1].read_f(size_f);
							decode(mode, bytes);

							u16 const status = slave(opbyte, opword, mode[0], mode[1]);
							if (!(status & ns32000_slave_interface::SLAVE_Q))
							{
								m_psr &= ~(PSR_N | PSR_Z | PSR_L);
								m_psr |= status & (ns32000_slave_interface::SLAVE_N | ns32000_slave_interface::SLAVE_Z | ns32000_slave_interface::SLAVE_L);
							}
							else
								interrupt(SLV, m_pc);
						}
						break;
					case 0x3:
						// Trap(SLAVE)
						// not defined; treat like CMPf
						mode[0].read_f(size_f);
						mode[1].read_f(size_f);
						decode(mode, bytes);

						if (slave(opbyte, opword, mode[0], mode[1]))
							interrupt(SLV, m_pc);
						break;
					case 0x4:
						// SUBf src,dst
						//      gen,gen
						//      read.f,rmw.f
						mode[0].read_f(size_f);
						mode[1].rmw_f(size_f);
						decode(mode, bytes);

						if (slave(opbyte, opword, mode[0], mode[1]))
							interrupt(SLV, m_pc);
						break;
					case 0x5:
						// NEGf src,dst
						//      gen,gen
						//      read.f,write.f
						mode[0].read_f(size_f);
						mode[1].write_f(size_f);
						decode(mode, bytes);

						if (slave(opbyte, opword, mode[0], mode[1]))
							interrupt(SLV, m_pc);
						break;
					case 0x8:
						// DIVf src,dst
						//      gen,gen
						//      read.f,rmw.f
						mode[0].read_f(size_f);
						mode[1].rmw_f(size_f);
						decode(mode, bytes);

						if (slave(opbyte, opword, mode[0], mode[1]))
							interrupt(SLV, m_pc);
						break;
					case 0x9:
						// Trap(SLAVE)
						// not defined; treat like MOVf
						mode[0].read_f(size_f);
						mode[1].write_f(size_f);
						decode(mode, bytes);

						if (slave(opbyte, opword, mode[0], mode[1]))
							interrupt(SLV, m_pc);
						break;
					case 0xc:
						// MULf src,dst
						//      gen,gen
						//      read.f,rmw.f
						mode[0].read_f(size_f);
						mode[1].rmw_f(size_f);
						decode(mode, bytes);

						if (slave(opbyte, opword, mode[0], mode[1]))
							interrupt(SLV, m_pc);
						break;
					case 0xd:
						// ABSf src,dst
						//      gen,gen
						//      read.f,write.f
						mode[0].read_f(size_f);
						mode[1].write_f(size_f);
						decode(mode, bytes);

						if (slave(opbyte, opword, mode[0], mode[1]))
							interrupt(SLV, m_pc);
						break;
					}
				}
				else
					interrupt(UND, m_pc);
				break;
			case 0xfe:
				// format 12: xxxx xyyy yyoo oo0f 1111 1110
				if ((m_cfg & CFG_F) && type() == NS32332)
				{
					u16 const opword = fetch<u16>(bytes);

					addr_mode mode[] = { addr_mode(BIT(opword, 11, 5)), addr_mode(BIT(opword, 6, 5)) };
					size_code const size_f = BIT(opword, 0) ? SIZE_D : SIZE_Q;

					switch (BIT(opword, 2, 4))
					{
					case 0x2:
						// POLYf src,dst
						//       gen,gen
						//       read.f,read.f
						mode[0].read_f(size_f);
						mode[1].read_f(size_f);
						decode(mode, bytes);

						if (slave(opbyte, opword, mode[0], mode[1]))
							interrupt(SLV, m_pc);
						break;
					case 0x3:
						// DOTf src,dst
						//      gen,gen
						//      read.f,read.f
						mode[0].read_f(size_f);
						mode[1].read_f(size_f);
						decode(mode, bytes);

						if (slave(opbyte, opword, mode[0], mode[1]))
							interrupt(SLV, m_pc);
						break;
					case 0x4:
						// SCALBf src,dst
						//        gen,gen
						//        read.f,rmw.f
						mode[0].read_f(size_f);
						mode[1].rmw_f(size_f);
						decode(mode, bytes);

						if (slave(opbyte, opword, mode[0], mode[1]))
							interrupt(SLV, m_pc);
						break;
					case 0x5:
						// LOGBf src,dst
						//       gen,gen
						//       read.f,write.f
						mode[0].read_f(size_f);
						mode[1].write_f(size_f);
						decode(mode, bytes);

						if (slave(opbyte, opword, mode[0], mode[1]))
							interrupt(SLV, m_pc);
						break;
					case 0x0: // REMf
					case 0x8: // Trap(SLV)
					case 0xc: // ATAN2f
						// not defined; treat like ADDf
						mode[0].read_f(size_f);
						mode[1].rmw_f(size_f);
						decode(mode, bytes);

						if (slave(opbyte, opword, mode[0], mode[1]))
							interrupt(SLV, m_pc);
						break;
					case 0x1: // SQRTf
					case 0x9: // Trap(SLV)
					case 0xd: // SICOSf
						// not defined; treat like MOVf
						mode[0].read_f(size_f);
						mode[1].write_f(size_f);
						decode(mode, bytes);

						if (slave(opbyte, opword, mode[0], mode[1]))
							interrupt(SLV, m_pc);
						break;
					case 0x6: // Trap(UND)
					case 0x7: // Trap(UND)
					case 0xa: // Trap(UND)
					case 0xb: // Trap(UND)
					case 0xe: // Trap(UND)
					case 0xf: // Trap(UND)
						interrupt(UND, m_pc);
						break;
					}
				}
				else
					interrupt(UND, m_pc);
				break;
			case 0x9e: // format 13
				interrupt(UND, m_pc);
				break;
			case 0x1e:
				// format 14: xxxx xsss s0oo ooii 0001 1110
				if (!(m_psr & PSR_U))
				{
					if (m_cfg & CFG_M)
					{
						u16 const opword = fetch<u16>(bytes);

						addr_mode mode[] = { addr_mode(BIT(opword, 11, 5)), addr_mode(0x13) };

						//unsigned const quick = BIT(opword, 7, 4);
						size_code const size = size_code(opword & 3);

						switch (BIT(opword, 2, 4))
						{
						case 0:
							// RDVAL loc
							//       gen
							//       addr
							{
								mode[0].addr();
								decode(mode, bytes);

								// dummy read
								mem_read<u8>(ST_ODT, ea(mode[0]), true);

								u16 const status = slave(opbyte, opword, mode[0], mode[1]);
								if (!(status & ns32000_slave_interface::SLAVE_Q))
								{
									if (status & ns32000_slave_interface::SLAVE_F)
										m_psr |= PSR_F;
									else
										m_psr &= ~PSR_F;
								}
								else
									interrupt(SLV, m_pc);

								tex = mode[0].tea + top(SIZE_B) + 21;
							}
							break;
						case 1:
							// WRVAL loc
							//       gen
							//       addr
							{
								mode[0].addr();
								decode(mode, bytes);

								// dummy read
								mem_read<u8>(ST_ODT, ea(mode[0]), true);

								u16 const status = slave(opbyte, opword, mode[0], mode[1]);
								if (!(status & ns32000_slave_interface::SLAVE_Q))
								{
									if (status & ns32000_slave_interface::SLAVE_F)
										m_psr |= PSR_F;
									else
										m_psr &= ~PSR_F;
								}
								else
									interrupt(SLV, m_pc);

								tex = mode[0].tea + top(SIZE_B) + 21;
							}
							break;
						case 2:
							// LMR mmureg,src
							//     short,gen
							//           read.D
							mode[0].read_i(size);
							decode(mode, bytes);

							if (slave(opbyte, opword, mode[0], mode[1]))
								interrupt(SLV, m_pc);

							tex = mode[0].tea + top(size);
							break;
						case 3:
							// SMR mmureg,dst
							//     short,gen
							//           write.D
							mode[0].write_i(size);
							decode(mode, bytes);

							if (slave(opbyte, opword, mode[1], mode[0]))
								interrupt(SLV, m_pc);

							tex = mode[0].tea + top(size);
							break;
						default:
							interrupt(UND, m_pc);
							break;
						}
					}
					else
						interrupt(UND, m_pc);
				}
				else
					interrupt(ILL, m_pc);
				break;
			case 0x16: // format 15.0
			case 0x36: // format 15.1
			case 0xb6: // format 15.5
				// TODO: custom coprocessor
				break;
			case 0x5e: // format 16
			case 0xde: // format 17
			case 0x8e: // format 18
			case 0x06:
			case 0x26:
			case 0x46:
			case 0x66:
			case 0x86:
			case 0xa6:
			case 0xc6:
			case 0xe6:
				// format 19
				interrupt(UND, m_pc);
				break;
			}

			if (m_sequential)
				m_pc += bytes;

			// trace trap
			if (m_psr & PSR_P)
				interrupt(TRC, m_pc);

			m_icount -= tex;
		}
		catch (ns32000_abort const &)
		{
			interrupt(ABT, m_pc);
		}
	}
}

template <int Width> void ns32000_device<Width>::execute_set_input(int inputnum, int state)
{
	if (state)
		m_wait = false;

	switch (inputnum)
	{
	case INPUT_LINE_NMI:
		// NMI is edge triggered
		m_nmi_line = m_nmi_line || (state == ASSERT_LINE);
		break;

	case INPUT_LINE_IRQ0:
		// INT is level triggered
		m_int_line = state == ASSERT_LINE;
		break;
	}
}

template <int Width> device_memory_interface::space_config_vector ns32000_device<Width>::memory_space_config() const
{
	return space_config_vector{
		std::make_pair(AS_PROGRAM, &m_program_config),

		std::make_pair(ST_IAM, &m_iam_config),
		std::make_pair(ST_IAC, &m_iac_config),
		std::make_pair(ST_EIM, &m_eim_config),
		std::make_pair(ST_EIC, &m_eic_config),
		std::make_pair(ST_SIF, &m_sif_config),
		std::make_pair(ST_NIF, &m_nif_config),
		std::make_pair(ST_ODT, &m_odt_config),
		std::make_pair(ST_RMW, &m_rmw_config),
		std::make_pair(ST_EAR, &m_ear_config),
	};
}

template <int Width> bool ns32000_device<Width>::memory_translate(int spacenum, int intention, offs_t &address)
{
	return !m_mmu || m_mmu->translate(space(spacenum), spacenum, address, intention & TRANSLATE_USER_MASK, intention & TRANSLATE_WRITE, false, intention & TRANSLATE_DEBUG_MASK) == ns32000_mmu_interface::COMPLETE;
}

template <int Width> std::unique_ptr<util::disasm_interface> ns32000_device<Width>::create_disassembler()
{
	return std::make_unique<ns32000_disassembler>();
}

template <int Width> u16 ns32000_device<Width>::slave(u8 opbyte, u16 opword, addr_mode op1, addr_mode op2)
{
	switch (opbyte)
	{
	case 0x1e:
		if (!m_mmu)
			fatalerror("slave mmu coprocessor not configured (%s)\n", machine().describe_context());

		if (m_cfg & CFG_FM)
			return slave_fast(dynamic_cast<ns32000_fast_slave_interface &>(*m_mmu), opbyte, opword, op1, op2);
		else
			return slave_slow(dynamic_cast<ns32000_slow_slave_interface &>(*m_mmu), opbyte, opword, op1, op2);
		break;

	case 0x3e:
	case 0xbe:
	case 0xfe:
		if (!m_fpu)
			fatalerror("slave fpu coprocessor not configured (%s)\n", machine().describe_context());

		if (m_cfg & CFG_FF)
			return slave_fast(dynamic_cast<ns32000_fast_slave_interface &>(*m_fpu), opbyte, opword, op1, op2);
		else
			return slave_slow(dynamic_cast<ns32000_slow_slave_interface &>(*m_fpu), opbyte, opword, op1, op2);
		break;

	default:
		fatalerror("slave coprocessor not supported (%s)\n", machine().describe_context());
		return ns32000_slave_interface::SLAVE_Q;
	}
}

template <int Width> u16 ns32000_device<Width>::slave_slow(ns32000_slow_slave_interface &slave, u8 opbyte, u16 opword, addr_mode op1, addr_mode op2)
{
	slave.write_id(opbyte);
	slave.write_op(swapendian_int16(opword));

	if ((op1.access == READ || op1.access == RMW) && !(op1.type == REG && op1.slave))
	{
		u64 const data = gen_read(op1);

		switch (op1.size)
		{
		case SIZE_B:
			slave.write_op(u8(data));
			break;
		case SIZE_W:
			slave.write_op(u16(data));
			break;
		case SIZE_D:
			slave.write_op(u16(data >> 0));
			slave.write_op(u16(data >> 16));
			break;
		case SIZE_Q:
			slave.write_op(u16(data >> 0));
			slave.write_op(u16(data >> 16));
			slave.write_op(u16(data >> 32));
			slave.write_op(u16(data >> 48));
			break;
		}
	}

	if ((op2.access == READ || op2.access == RMW) && !(op2.type == REG && op2.slave))
	{
		u64 const data = gen_read(op2);

		switch (op2.size)
		{
		case SIZE_B:
			slave.write_op(u8(data));
			break;
		case SIZE_W:
			slave.write_op(u16(data));
			break;
		case SIZE_D:
			slave.write_op(u16(data >> 0));
			slave.write_op(u16(data >> 16));
			break;
		case SIZE_Q:
			slave.write_op(u16(data >> 0));
			slave.write_op(u16(data >> 16));
			slave.write_op(u16(data >> 32));
			slave.write_op(u16(data >> 48));
			break;
		}
	}

	u16 const status = slave.read_st(&m_icount);

	if (!(status & ns32000_slave_interface::SLAVE_Q))
	{
		if ((op2.access == WRITE || op2.access == RMW) && !(op2.type == REG && op2.slave))
		{
			u64 data = slave.read_op();

			switch (op2.size)
			{
			case SIZE_D:
				data |= u64(slave.read_op()) << 16;
				break;
			case SIZE_Q:
				data |= u64(slave.read_op()) << 16;
				data |= u64(slave.read_op()) << 32;
				data |= u64(slave.read_op()) << 48;
				break;
			default:
				break;
			}

			gen_write(op2, data);
		}
	}

	return status;
}

template <int Width> u16 ns32000_device<Width>::slave_fast(ns32000_fast_slave_interface &slave, u8 opbyte, u16 opword, addr_mode op1, addr_mode op2)
{
	slave.write(u32(opbyte) << 24 | u32(swapendian_int16(opword)) << 8);

	if ((op1.access == READ || op1.access == RMW) && !(op1.type == REG && op1.slave))
	{
		u64 const data = gen_read(op1);

		switch (op1.size)
		{
		case SIZE_B:
			slave.write(u8(data));
			break;
		case SIZE_W:
			slave.write(u16(data));
			break;
		case SIZE_D:
			slave.write(u32(data));
			break;
		case SIZE_Q:
			slave.write(u32(data >> 0));
			slave.write(u32(data >> 32));
			break;
		}
	}

	if ((op2.access == READ || op2.access == RMW) && !(op2.type == REG && op2.slave))
	{
		u64 const data = gen_read(op2);

		switch (op2.size)
		{
		case SIZE_B:
			slave.write(u8(data));
			break;
		case SIZE_W:
			slave.write(u16(data));
			break;
		case SIZE_D:
			slave.write(u32(data));
			break;
		case SIZE_Q:
			slave.write(u32(data >> 0));
			slave.write(u32(data >> 32));
			break;
		}
	}

	// TODO: status is optional in fast protocol
	u32 const status = slave.read_st(&m_icount);

	if (!(status & ns32000_slave_interface::SLAVE_Q))
	{
		if ((op2.access == WRITE || op2.access == RMW) && !(op2.type == REG && op2.slave))
		{
			u64 data = slave.read();

			if (op2.size == SIZE_Q)
				data |= u64(slave.read()) << 32;

			gen_write(op2, data);
		}
	}

	return status;
}
