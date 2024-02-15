// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#include "emu.h"

#include "ns32000.h"
#include "ns32000d.h"
#include "debug/debugcpu.h"

#define LOG_TRANSLATE (1U << 1)
//#define VERBOSE (LOG_TRANSLATE)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(NS32008, ns32008_device, "ns32008", "National Semiconductor NS32008")
DEFINE_DEVICE_TYPE(NS32016, ns32016_device, "ns32016", "National Semiconductor NS32016")
DEFINE_DEVICE_TYPE(NS32032, ns32032_device, "ns32032", "National Semiconductor NS32032")
DEFINE_DEVICE_TYPE(NS32332, ns32332_device, "ns32332", "National Semiconductor NS32332")
DEFINE_DEVICE_TYPE(NS32532, ns32532_device, "ns32532", "National Semiconductor NS32532")

/*
 * TODO:
 *  - instruction timing
 *  - prefetch queue/caches
 *  - overflow exceptions
 *  - debug registers
 *  - translation look-aside buffer
 */

enum psr_mask : u16
{
	// accessible in user mode
	PSR_C   = 0x0001, // carry/borrow condition
	PSR_T   = 0x0002, // trace trap enable
	PSR_L   = 0x0004, // less than condition
					  // unused
	PSR_V   = 0x0010, // (32532 only) enable overflow trap
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
};

enum cfg_mask : u32
{
	CFG_I   = 0x0001, // vectored interrupts
	CFG_F   = 0x0002, // fpu present
	CFG_M   = 0x0004, // mmu present
	CFG_C   = 0x0008, // custom coprocessor present
	CFG_FF  = 0x0010, // (32332 only) fast fpu protocol
	CFG_FM  = 0x0020, // (32332 only) fast mmu protocol
	CFG_FC  = 0x0040, // (32332 only) fast custom coprocessor protocol
	CFG_P   = 0x0080, // (32332 only) page size >= 4kb
	CFG_DE  = 0x0100, // (32532 only) direct exception mode enable
	CFG_DC  = 0x0200, // (32532 only) data cache enable
	CFG_LDC = 0x0400, // (32532 only) lock data cache
	CFG_IC  = 0x0800, // (32532 only) instruction cache enable
	CFG_LIC = 0x1000, // (32532 only) lock instruction cache
	CFG_PF  = 0x2000, // (32532 only) pipelined floating-point execution
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
	BER   = 11, // (32332 only) bus error
	RBE   = 11, // (32532 only) restartable bus error
	NBE   = 12, // (32532 only) non-restartable bus error
	OVF   = 13, // (32532 only) integer overflow trap
	DBG   = 14, // (32532 only) debug trap
};

class ns32000_abort : public std::exception { };
class ns32000_delay : public std::exception { };

static const u32 size_mask[] = { 0x0000'00ffU, 0x0000'ffffU, 0x0000'0000U, 0xffff'ffffU };

template <int HighBits, int Width>ns32000_device<HighBits, Width>::ns32000_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_address_mask(util::make_bitmask<u32>(HighBits))
	, m_program_config("program", ENDIANNESS_LITTLE, 8 << Width, HighBits, 0)
	, m_iam_config("iam", ENDIANNESS_LITTLE, 8 << Width, HighBits, 0)
	, m_iac_config("iac", ENDIANNESS_LITTLE, 8 << Width, HighBits, 0)
	, m_eim_config("eim", ENDIANNESS_LITTLE, 8 << Width, HighBits, 0)
	, m_eic_config("eic", ENDIANNESS_LITTLE, 8 << Width, HighBits, 0)
	, m_sif_config("sif", ENDIANNESS_LITTLE, 8 << Width, HighBits, 0)
	, m_nif_config("nif", ENDIANNESS_LITTLE, 8 << Width, HighBits, 0)
	, m_odt_config("odt", ENDIANNESS_LITTLE, 8 << Width, HighBits, 0)
	, m_rmw_config("rmw", ENDIANNESS_LITTLE, 8 << Width, HighBits, 0)
	, m_ear_config("ear", ENDIANNESS_LITTLE, 8 << Width, HighBits, 0)
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
	, m_r{}
	, m_nmi_line(false)
	, m_int_line(false)
	, m_wait(false)
	, m_sequential(false)
{
}

ns32008_device::ns32008_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: ns32000_device(mconfig, NS32008, tag, owner, clock)
{
}

ns32016_device::ns32016_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: ns32000_device(mconfig, NS32016, tag, owner, clock)
{
}

ns32032_device::ns32032_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: ns32000_device(mconfig, NS32032, tag, owner, clock)
{
}

ns32332_device::ns32332_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: ns32000_device(mconfig, NS32332, tag, owner, clock)
{
}

ns32532_device::ns32532_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: ns32000_device(mconfig, NS32532, tag, owner, clock)
	, ns32000_mmu_interface(mconfig, *this)
	, m_pt1_config("pt1", ENDIANNESS_LITTLE, 32, 32, 0)
	, m_pt2_config("pt2", ENDIANNESS_LITTLE, 32, 32, 0)
{
}

template <int HighBits, int Width> void ns32000_device<HighBits, Width>::device_start()
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

template <int HighBits, int Width> void ns32000_device<HighBits, Width>::device_reset()
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

template <int HighBits, int Width> void ns32000_device<HighBits, Width>::state_string_export(device_state_entry const &entry, std::string &str) const
{
	switch (entry.index())
	{
	case STATE_GENFLAGS:
		str = string_format("%c%c%c%c%c%c%c%c%c%c%c",
			(m_psr & PSR_I) ? 'I' : '.',
			(m_psr & PSR_P) ? 'P' : '.',
			(m_psr & PSR_S) ? 'S' : '.',
			(m_psr & PSR_U) ? 'U' : '.',
			(m_psr & PSR_N) ? 'N' : '.',
			(m_psr & PSR_Z) ? 'Z' : '.',
			(m_psr & PSR_F) ? 'F' : '.',
			(m_psr & PSR_V) ? 'V' : '.',
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
template <int HighBits, int Width> template<typename T> T ns32000_device<HighBits, Width>::mem_read(unsigned st, u32 address, bool user, bool pfs)
{
	u32 physical = address;
	ns32000_mmu_interface::translate_result tr = m_mmu ?
		m_mmu->translate(m_bus[st].space(), st, physical, (m_psr & PSR_U) || user, false, pfs) : ns32000_mmu_interface::COMPLETE;

	if (tr == ns32000_mmu_interface::COMPLETE)
	{
		u32 const unitmask = sizeof(T) - 1;
		unsigned const offset = address & unitmask;

		T data = 0;
		m_ready = true;

		if (offset)
		{
			u32 const pagemask = ((m_cfg & CFG_P) ? 0xfffU : 0x1ffU) & ~unitmask;

			if (!m_mmu || (~address & pagemask))
			{
				// unaligned access, one page (or no mmu)
				switch (sizeof(T))
				{
				case 1: abort(); // can't happen
				case 2: data = m_bus[st].read_word_unaligned(physical); break;
				case 4: data = m_bus[st].read_dword_unaligned(physical); break;
				case 8: data = m_bus[st].read_qword_unaligned(physical); break;
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
			case 1: data = m_bus[st].read_byte(physical); break;
			case 2: data = m_bus[st].read_word(physical); break;
			case 4: data = m_bus[st].read_dword(physical); break;
			case 8: data = m_bus[st].read_qword(physical); break;
			}
		}

		if (!m_ready)
			throw ns32000_delay();
		else
			return data;
	}
	else if (tr == ns32000_mmu_interface::ABORT)
		throw ns32000_abort();

	return 0;
}

template <int HighBits, int Width> template<typename T> void ns32000_device<HighBits, Width>::mem_write(unsigned st, u32 address, u64 data, bool user)
{
	u32 physical = address;
	ns32000_mmu_interface::translate_result tr = m_mmu ?
		m_mmu->translate(m_bus[st].space(), st, physical, (m_psr & PSR_U) || user, true) : ns32000_mmu_interface::COMPLETE;

	if (tr == ns32000_mmu_interface::COMPLETE)
	{
		u32 const unitmask = sizeof(T) - 1;
		unsigned const offset = address & unitmask;

		m_ready = true;

		if (offset)
		{
			u32 const pagemask = ((m_cfg & CFG_P) ? 0xfffU : 0x1ffU) & ~unitmask;

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
			/*
			 * Tektronix 4132 firmware requires that MOVB rN,<mem> (where mem
			 * is a word-aligned memory address) drives a 16-bit value from the
			 * register onto the data bus (with /HBE deasserted). The effect is
			 * important when the data is being written to a fixed-width 16-bit
			 * register (Am9516 in this case), as the byte enables are ignored
			 * and a 16-bit value is latched. This code assumes the same effect
			 * occurs with word/dword-aligned MOVB/MOVW on the 32032 and 32332.
			 */
			// TODO: verify how real hardware behaves
			switch (sizeof(T))
			{
			case 1:
				if (Width == 1)
				{
					unsigned const shift = (physical & 1) * 8;

					m_bus[st].write_word(physical, data << shift, 0xffU << shift);
				}
				else if (Width == 2)
				{
					unsigned const shift = (physical & 3) * 8;

					m_bus[st].write_dword(physical, data << shift, 0xffU << shift);
				}
				else
					m_bus[st].write_byte(physical, data);
				break;
			case 2:
				if (Width == 2)
				{
					unsigned const shift = (physical & 2) * 8;

					m_bus[st].write_dword(physical, data << shift, 0xffffU << shift);
				}
				else
					m_bus[st].write_word(physical, data);
				break;
			case 4: m_bus[st].write_dword(physical, data); break;
			case 8: m_bus[st].write_qword(physical, data); break;
			}
		}

		if (!m_ready)
			throw ns32000_delay();
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
template <int HighBits, int Width> template<typename T> T ns32000_device<HighBits, Width>::fetch(unsigned &bytes)
{
	T const data = mem_read<T>(m_sequential ? ns32000::ST_SIF : ns32000::ST_NIF, m_pc + bytes, false, bytes == 0);

	bytes += sizeof(T);
	m_sequential = true;

	return data;
}

template <int HighBits, int Width> s32 ns32000_device<HighBits, Width>::displacement(unsigned &bytes)
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

			return util::sext((byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3, 30);
		}
		else
		{
			// word displacement
			u8 const byte1 = fetch<u8>(bytes);

			return util::sext((byte0 << 8) | byte1, 14);
		}
	}
	else
		// byte displacement
		return util::sext(byte0, 7);
}

template <int HighBits, int Width> void ns32000_device<HighBits, Width>::decode(addr_mode *mode, unsigned &bytes)
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
			mode[i].base = SP() + displacement(bytes);
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
			mode[i].base = SP();
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
			mode[i].base = SP() + displacement(bytes);
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
template <int HighBits, int Width> u32 &ns32000_device<HighBits, Width>::SP()
{
	return SP(m_psr & PSR_S);
}

template <int HighBits, int Width> u32 &ns32000_device<HighBits, Width>::SP(bool user)
{
	return user ? m_sp1 : m_sp0;
}

template <int HighBits, int Width> u32 ns32000_device<HighBits, Width>::ea(addr_mode const mode)
{
	u32 base;

	switch (mode.type)
	{
	case REG:
		base = m_r[mode.gen];
		break;

	case REL:
		base = mem_read<u32>(ns32000::ST_EAR, mode.base);
		break;

	case EXT:
		base = mem_read<u32>(ns32000::ST_EAR, m_mod + 4);
		base = mem_read<u32>(ns32000::ST_EAR, base + mode.base);
		break;

	default:
		base = mode.base;
		break;
	}

	return base + mode.disp;
}

template <int HighBits, int Width> u64 ns32000_device<HighBits, Width>::gen_read(addr_mode mode)
{
	if (mode.type == IMM)
		return mode.imm;

	if (mode.type == REG)
		return m_r[mode.gen] & size_mask[mode.size];

	unsigned const st = (mode.access == RMW) ? ns32000::ST_RMW : ns32000::ST_ODT;
	u32 const address = (mode.type == TOS) ? SP() : ea(mode);
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
		SP() += mode.size + 1;

	return data;
}

template <int HighBits, int Width> s64 ns32000_device<HighBits, Width>::gen_read_sx(addr_mode mode)
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

template <int HighBits, int Width> void ns32000_device<HighBits, Width>::gen_write(addr_mode mode, u64 data)
{
	if (mode.type == REG)
	{
		m_r[mode.gen] = (m_r[mode.gen] & ~size_mask[mode.size]) | (data & size_mask[mode.size]);
		return;
	}

	// pre-decrement stack pointer
	if (mode.type == TOS && mode.access == WRITE)
		SP() -= mode.size + 1;

	u32 const address = (mode.type == TOS) ? SP() : ea(mode);

	switch (mode.size)
	{
	case SIZE_B: mem_write<u8>(ns32000::ST_ODT, address, data); break;
	case SIZE_W: mem_write<u16>(ns32000::ST_ODT, address, data); break;
	case SIZE_D: mem_write<u32>(ns32000::ST_ODT, address, data); break;
	case SIZE_Q: mem_write<u64>(ns32000::ST_ODT, address, data); break;
	}

	m_icount -= top(mode.size, address);
}

template <int HighBits, int Width> bool ns32000_device<HighBits, Width>::condition(unsigned const cc)
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

template <int HighBits, int Width> void ns32000_device<HighBits, Width>::flags(u32 const src1, u32 const src2, u32 const dest, unsigned const size, bool const subtraction)
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

template <int HighBits, int Width> void ns32000_device<HighBits, Width>::interrupt(unsigned const trap)
{
	unsigned offset = trap * 4;

	switch (trap)
	{
	case NVI:
		// maskable interrupt
		m_sps = m_psr;
		m_psr &= ~(PSR_I | PSR_P | PSR_S | PSR_U | PSR_V | PSR_T);

		if (m_cfg & CFG_I)
		{
			// acknowledge interrupt and read vector
			s8 vector = mem_read<u8>(ns32000::ST_IAM, 0xffff'fe00U & m_address_mask);
			if (vector < 0 && vector >= -16)
			{
				// vectored mode, cascaded
				u32 const cascade = mem_read<u32>(ns32000::ST_ODT, m_intbase + vector * 4);

				vector = mem_read<u8>(ns32000::ST_IAC, cascade);
			}

			offset = vector * 4;
		}
		else
			// acknowledge non-vectored interrupt
			mem_read<u8>(ns32000::ST_IAM, 0xffff'fe00U & m_address_mask);
		break;

	case NMI:
		// non-maskable interrupt
		m_sps = m_psr;
		m_psr &= ~(PSR_I | PSR_P | PSR_S | PSR_U | PSR_V | PSR_T);

		// acknowledge interrupt and discard vector
		mem_read<u8>(ns32000::ST_IAM, 0xffff'ff00U & m_address_mask);
		m_nmi_line = false;
		break;

	case ABT:
	case BER:
		// abort
		SP() = m_ssp;
		m_psr &= ~PSR_P;
		m_sps = m_psr;
		m_psr &= ~(PSR_I | PSR_S | PSR_U | PSR_V | PSR_T);
		break;

	case TRC:
		// trace trap
		m_psr &= ~PSR_P;
		m_sps = m_psr;
		m_psr &= ~(PSR_S | PSR_U | PSR_V | PSR_T);
		break;

	default:
		// traps other than trace
		SP() = m_ssp;
		m_psr = m_sps;
		if ((type() == NS32332 && trap == ILL) || (type() == NS32532 && (trap == ILL || trap == UND)))
			m_sps &= ~PSR_P;
		m_psr &= ~(PSR_P | PSR_S | PSR_U | PSR_V | PSR_T);
		break;
	}

	// push saved program status and mod
	m_sp0 -= 4;
	mem_write<u32>(ns32000::ST_ODT, m_sp0, u32(m_sps) << 16 | m_mod);

	// push return address
	m_sp0 -= 4;
	mem_write<u32>(ns32000::ST_ODT, m_sp0, m_pc);

	if (!(m_cfg & CFG_DE))
	{
		// fetch external procedure descriptor
		u32 const desc = mem_read<u32>(ns32000::ST_ODT, m_intbase + offset);

		// update mod, sb, pc
		m_mod = u16(desc);
		m_sb = mem_read<u32>(ns32000::ST_ODT, m_mod + 0);
		m_pc = mem_read<u32>(ns32000::ST_ODT, m_mod + 8) + (desc >> 16);
	}
	else
		m_pc = mem_read<u32>(ns32000::ST_ODT, m_intbase + offset);

	// TODO: flush queue
	m_sequential = false;

	m_icount -= top(SIZE_W, m_sp0) * 2 + top(SIZE_W, m_intbase + offset) + top(SIZE_D, m_sp0) + top(SIZE_D, m_mod);

	if (machine().debug_enabled() && (trap > ABT))
		debug()->exception_hook(trap);
}

template <int HighBits, int Width> void ns32000_device<HighBits, Width>::lpr(unsigned reg, addr_mode const mode, bool user, unsigned &tex)
{
	switch (reg)
	{
	case 0x0:
		m_psr = ((m_psr & 0xff00) | u8(gen_read(mode))) & PSR_MSK();
		break;
	case 0x8:
		m_fp = gen_read(mode);
		break;
	case 0x9:
		SP() = gen_read(mode);
		break;
	case 0xa:
		m_sb = gen_read(mode);
		break;
	case 0xd:
		if (!user)
		{
			u32 const src = gen_read(mode);

			if (mode.size == SIZE_B)
				m_psr = ((m_psr & 0xff00) | u8(src)) & PSR_MSK();
			else
				m_psr = src & PSR_MSK();
		}
		else
			interrupt(ILL);
		break;
	case 0xe:
		if (!user)
			m_intbase = gen_read(mode);
		else
			interrupt(ILL);
		break;
	case 0xf:
		m_mod = gen_read(mode);
		break;
	default:
		interrupt(UND);
		break;
	}

	// TODO: tcy 19-33
	tex = mode.tea + 19;
}

template <int HighBits, int Width> void ns32000_device<HighBits, Width>::spr(unsigned reg, addr_mode const mode, bool user, unsigned &tex)
{
	switch (reg)
	{
	case 0x0:
		gen_write(mode, u8(m_psr));
		break;
	case 0x8:
		gen_write(mode, m_fp);
		break;
	case 0x9:
		gen_write(mode, SP());
		break;
	case 0xa:
		gen_write(mode, m_sb);
		break;
	case 0xd:
		if (!user)
			gen_write(mode, m_psr);
		else
			interrupt(ILL);
		break;
	case 0xe:
		if (!user)
			gen_write(mode, m_intbase);
		else
			interrupt(ILL);
		break;
	case 0xf:
		gen_write(mode, m_mod);
		break;
	default:
		interrupt(UND);
		break;
	}

	// TODO: tcy 21-27
	tex = mode.tea + 21;
}

template <int HighBits, int Width> void ns32000_device<HighBits, Width>::execute_run()
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
				// notify the debugger
				if (machine().debug_enabled())
					debug()->interrupt_hook(INPUT_LINE_NMI, m_pc);

				// service interrupt
				interrupt(NMI);
			}
			else if (m_int_line && (m_psr & PSR_I))
			{
				// notify the debugger
				if (machine().debug_enabled())
					debug()->interrupt_hook(INPUT_LINE_IRQ0, m_pc);

				// service interrupt
				interrupt(NVI);
			}

			// update trace pending
			if (m_psr & PSR_T)
				m_psr |= PSR_P;
			else
				m_psr &= ~PSR_P;

			debugger_instruction_hook(m_pc);

			// save state
			m_ssp = SP();
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

						SP() -= 4;
						mem_write<u32>(ns32000::ST_ODT, SP(), m_pc + bytes);

						m_pc += dst;
						m_sequential = false;
						tex = top(SIZE_D, SP()) + 6;
					}
					break;
				case 0x1:
					// RET constant
					//     disp
					{
						s32 const constant = displacement(bytes);

						u32 const addr = mem_read<u32>(ns32000::ST_ODT, SP());
						SP() += 4;

						m_pc = addr;
						m_sequential = false;
						tex = top(SIZE_D, SP()) + 2;
						SP() += constant;
					}
					break;
				case 0x2:
					// CXP index
					//     disp
					{
						s32 const index = displacement(bytes);

						u32 const link_base = mem_read<u32>(ns32000::ST_ODT, m_mod + 4);
						u32 const desc = mem_read<u32>(ns32000::ST_ODT, link_base + index * 4);

						SP() -= 4;
						mem_write<u16>(ns32000::ST_ODT, SP(), m_mod);
						SP() -= 4;
						mem_write<u32>(ns32000::ST_ODT, SP(), m_pc + bytes);

						u16 const mod = u16(desc);
						u32 const sb = mem_read<u32>(ns32000::ST_ODT, mod + 0);
						u32 const pc = mem_read<u32>(ns32000::ST_ODT, mod + 8) + (desc >> 16);

						tex = top(SIZE_D, m_mod + 4) + top(SIZE_W, link_base + index * 4) * 2 + top(SIZE_W, SP()) + top(SIZE_D, SP()) + top(SIZE_D, mod) * 2 + 16;

						m_pc = pc;
						m_mod = mod;
						m_sb = sb;
						m_sequential = false;
					}
					break;
				case 0x3:
					// RXP constant
					//     disp
					{
						s32 const constant = displacement(bytes);

						u32 const pc = mem_read<u32>(ns32000::ST_ODT, SP());
						SP() += 4;
						u16 const mod = mem_read<u16>(ns32000::ST_ODT, SP());
						SP() += 4;
						u32 const sb = mem_read<u32>(ns32000::ST_ODT, mod);

						tex = top(SIZE_D, SP()) + top(SIZE_W, SP()) + top(SIZE_D, mod) + 2;

						m_pc = pc;
						m_mod = mod;
						m_sb = sb;
						SP() += constant;
						m_sequential = false;
					}
					break;
				case 0x4:
					// RETT constant
					//      disp
					if (!(m_psr & PSR_U))
					{
						s32 const constant = displacement(bytes);

						u32 const pc = mem_read<u32>(ns32000::ST_ODT, SP() + 0);
						u16 const psr = mem_read<u16>(ns32000::ST_ODT, SP() + 6) & PSR_MSK();

						if (!(m_cfg & CFG_DE))
						{
							u16 const mod = mem_read<u16>(ns32000::ST_ODT, SP() + 4);
							u32 const sb = mem_read<u32>(ns32000::ST_ODT, mod);

							m_mod = mod;
							m_sb = sb;

							tex = top(SIZE_D, SP()) + top(SIZE_W, SP()) * 2 + top(SIZE_D, mod) + 35;
						}
						else
							tex = top(SIZE_D, SP()) + top(SIZE_W, SP()) + 35;

						SP() += 8;

						m_pc = pc;
						m_psr = psr;
						SP() += constant;

						m_sequential = false;
					}
					else
						interrupt(ILL);
					break;
				case 0x5:
					// RETI
					if (!(m_psr & PSR_U))
					{
						// end of interrupt, master
						s8 vector = mem_read<u8>(ns32000::ST_EIM, 0xffff'fe00U & m_address_mask);

						// check for vectored mode
						if (m_cfg & CFG_I)
						{
							if (vector < 0 && vector >= -16)
							{
								u32 const cascade = mem_read<u32>(ns32000::ST_ODT, m_intbase + vector * 4);

								// end of interrupt, cascaded
								vector = mem_read<u8>(ns32000::ST_EIC, cascade);
							}
						}

						u32 const pc = mem_read<u32>(ns32000::ST_ODT, SP() + 0);
						u16 const psr = mem_read<u16>(ns32000::ST_ODT, SP() + 6) & PSR_MSK();

						if (!(m_cfg & CFG_DE))
						{
							u16 const mod = mem_read<u16>(ns32000::ST_ODT, SP() + 4);
							u32 const sb = mem_read<u32>(ns32000::ST_ODT, mod);

							m_mod = mod;
							m_sb = sb;

							tex = top(SIZE_B) + top(SIZE_W, SP()) * 3 + top(SIZE_D) * 3 + 39;
						}
						else
							tex = top(SIZE_B) + top(SIZE_W, SP()) * 2 + top(SIZE_D) * 2 + 39;

						SP() += 8;

						m_pc = pc;
						m_psr = psr;

						m_sequential = false;
					}
					else
						interrupt(ILL);
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
								SP() -= 4;
								mem_write<u32>(ns32000::ST_ODT, SP(), m_r[i]);
								tex += top(SIZE_D, SP()) + 4;
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
								m_r[7 - i] = mem_read<u32>(ns32000::ST_ODT, SP());
								tex += top(SIZE_D, SP()) + 5;
								SP() += 4;
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

						SP() -= 4;
						mem_write<u32>(ns32000::ST_ODT, SP(), m_fp);
						u32 const fp = SP();
						tex = top(SIZE_D, SP()) + 18;
						SP() -= constant;

						for (unsigned i = 0; i < 8; i++)
						{
							if (BIT(reglist, i))
							{
								SP() -= 4;
								mem_write<u32>(ns32000::ST_ODT, SP(), m_r[i]);
								tex += top(SIZE_D, SP()) + 4;
							}
						}

						m_fp = fp;
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
								m_r[7 - i] = mem_read<u32>(ns32000::ST_ODT, SP());
								tex += top(SIZE_D, SP()) + 5;
								SP() += 4;
							}
						}
						SP() = m_fp;
						m_fp = mem_read<u32>(ns32000::ST_ODT, SP());
						tex += top(SIZE_D, SP());
						SP() += 4;
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
						interrupt(FLG);
						tex = 44;
					}
					else
						tex = 6;
					break;
				case 0xe:
					// SVC
					interrupt(SVC);
					tex = 40;
					break;
				case 0xf:
					// BPT
					interrupt(BPT);
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

						u32 const src1 = util::sext(quick, 4);
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

						u32 const src1 = util::sext(quick, 4) & size_mask[size];
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

					spr(quick, mode[0], m_psr & PSR_U, tex);
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

						s32 const inc = util::sext(quick, 4);
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

					gen_write(mode[0], util::sext(quick, 4));

					tex = (mode[0].type == REG) ? 3 : mode[0].tea + 2;
					break;
				case 6:
					// LPRi procreg,src
					//      short,gen
					//            read.i
					mode[0].read_i(size);
					decode(mode, bytes);

					lpr(quick, mode[0], m_psr & PSR_U, tex);
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

							SP() -= 4;
							mem_write<u16>(ns32000::ST_ODT, SP(), m_mod);
							SP() -= 4;
							mem_write<u32>(ns32000::ST_ODT, SP(), m_pc + bytes);

							u32 const desc = mem_read<u32>(ns32000::ST_ODT, address);
							u16 const mod = u16(desc);
							u32 const sb = mem_read<u32>(ns32000::ST_ODT, mod + 0);
							u32 const pc = mem_read<u32>(ns32000::ST_ODT, mod + 8) + (desc >> 16);

							tex = mode[0].tea + top(SIZE_W, address) * 3 + top(SIZE_D, mod) * 3 + 13;

							m_pc = pc;
							m_mod = mod;
							m_sb = sb;
							m_sequential = false;
						}
						else
							interrupt(UND);
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
								interrupt(ILL);
						}
						else
							interrupt(UND);
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
							interrupt(UND);
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

								m_psr |= src & PSR_MSK();

								tex = mode[0].tea + ((size == SIZE_B) ? 18 : 30);
							}
							else
								interrupt(ILL);
						}
						else
							interrupt(UND);
						break;
					case 0xa:
						// ADJSPi src
						//        gen
						//        read.i
						{
							mode[0].read_i(size);
							decode(mode, bytes);

							s32 const src = gen_read_sx(mode[0]);

							SP() -= src;

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

							SP() -= 4;
							mem_write<u32>(ns32000::ST_ODT, SP(), m_pc + bytes);

							m_pc = ea(mode[0]);
							m_sequential = false;

							// TODO: where does the TOPi come from?
							tex = mode[0].tea + top(SIZE_D, SP()) + top(size) + 5;
						}
						else
							interrupt(UND);
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
						interrupt(UND);
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

						// special-case non-masked source data when moving from
						// register to memory; see comments in mem_write()
						u32 const src = (mode[0].type == REG) ? m_r[mode[0].gen] : gen_read(mode[0]);

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
						interrupt(UND);
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
							u8 const byte = mem_read<u8>(ns32000::ST_ODT, ea(mode[1]) + (offset >> 3));

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
								(size == SIZE_D) ? mem_read<u32>(ns32000::ST_ODT, m_r[1]) :
								(size == SIZE_W) ? mem_read<u16>(ns32000::ST_ODT, m_r[1]) :
								mem_read<u8>(ns32000::ST_ODT, m_r[1]);

							if (translate)
								data = mem_read<u8>(ns32000::ST_ODT, m_r[3] + u8(data));

							tex += top(size, m_r[1]) + (translate ? top(SIZE_B) + 27 : (backward || uw) ? 24 : 13);

							bool const match = !((m_r[4] ^ data) & size_mask[size]);
							if ((uw == 1 && !match) || (uw == 3 && match))
							{
								m_psr |= PSR_F;
								break;
							}

							if (size == SIZE_D)
								mem_write<u32>(ns32000::ST_ODT, m_r[2], data);
							else if (size == SIZE_W)
								mem_write<u16>(ns32000::ST_ODT, m_r[2], data);
							else
								mem_write<u8>(ns32000::ST_ODT, m_r[2], data);

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
								(size == SIZE_D) ? mem_read<u32>(ns32000::ST_ODT, m_r[1]) :
								(size == SIZE_W) ? mem_read<u16>(ns32000::ST_ODT, m_r[1]) :
								mem_read<u8>(ns32000::ST_ODT, m_r[1]);
							u32 src2 =
								(size == SIZE_D) ? mem_read<u32>(ns32000::ST_ODT, m_r[2]) :
								(size == SIZE_W) ? mem_read<u16>(ns32000::ST_ODT, m_r[2]) :
								mem_read<u8>(ns32000::ST_ODT, m_r[2]);

							if (translate)
								src1 = mem_read<u8>(ns32000::ST_ODT, m_r[3] + u8(src1));

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
							if (type() == NS32532)
								m_cfg = BIT(opword, 7, 4) | (CFG_P | CFG_FC | CFG_FM | CFG_FF);
							else if (type() == NS32332)
								m_cfg = BIT(opword, 7, 8);
							else
								m_cfg = BIT(opword, 7, 4);

							tex = 15;
						}
						else
							interrupt(ILL);
						break;
					case 3:
						// SKPSi options
						tex = 51;

						m_psr &= ~PSR_F;
						while (m_r[0])
						{
							u32 data =
								(size == SIZE_D) ? mem_read<u32>(ns32000::ST_ODT, m_r[1]) :
								(size == SIZE_W) ? mem_read<u16>(ns32000::ST_ODT, m_r[1]) :
								mem_read<u8>(ns32000::ST_ODT, m_r[1]);

							if (translate)
								data = mem_read<u8>(ns32000::ST_ODT, m_r[3] + u8(data));

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
						interrupt(UND);
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
								u8 const byte = mem_read<u8>(ns32000::ST_ODT, byte_ea);

								if (BIT(byte, offset & 7))
									m_psr |= PSR_F;
								else
									m_psr &= ~PSR_F;

								mem_write<u8>(ns32000::ST_ODT, byte_ea, byte & ~(1U << (offset & 7)));

								tex = mode[0].tea + mode[1].tea + top(SIZE_B) * 2 + 15;
							}
						}
						break;
					case 0x4:
						interrupt(UND);
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
								u8 const byte = mem_read<u8>(ns32000::ST_ODT, byte_ea);

								if (BIT(byte, offset & 7))
									m_psr |= PSR_F;
								else
									m_psr &= ~PSR_F;

								mem_write<u8>(ns32000::ST_ODT, byte_ea, byte | (1U << (offset & 7)));

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
						interrupt(UND);
						break;
					case 0xb:
						// SUBPi src,dst
						//      gen,gen
						//      read.i,rmw.i
						{
							mode[0].read_i(size);
							mode[1].rmw_i(size);
							decode(mode, bytes);

							u32 const src1 = gen_read(mode[0]);
							u32 const src2 = gen_read(mode[1]);

							// binary coded decimal subtraction with carry
							// TODO: CHECK
							u32 dst = 0;
							bool carry = m_psr & PSR_C;
							unsigned const tcy = carry ? 18 : 16;
							for (unsigned digit = 0; digit < (size + 1) * 2; digit++)
							{
								signed sum = BIT(src2, digit * 4, 4) - BIT(src1, digit * 4, 4) - carry;

								if (sum < 0)
								{
									sum = sum + 10;
									carry = true;
								}
								else
									carry = false;

								dst |= sum << digit * 4;
							}

							if (carry)
								m_psr |= PSR_C;
							else
								m_psr &= ~PSR_C;

							gen_write(mode[1], dst);

							tex = mode[0].tea + mode[1].tea + tcy;
						}
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
								u8 const byte = mem_read<u8>(ns32000::ST_ODT, byte_ea);

								if (BIT(byte, offset & 7))
									m_psr |= PSR_F;
								else
									m_psr &= ~PSR_F;

								mem_write<u8>(ns32000::ST_ODT, byte_ea, byte ^ (1U << (offset & 7)));

								tex = mode[0].tea + mode[1].tea + top(SIZE_B) * 2 + 17;
							}
						}
						break;
					case 0xf:
						// ADDPi src,dst
						//       gen,gen
						//       read.i,rmw.i
						{
							mode[0].read_i(size);
							mode[1].rmw_i(size);
							decode(mode, bytes);

							u32 const src1 = gen_read(mode[0]);
							u32 const src2 = gen_read(mode[1]);

							// binary coded decimal addition with carry
							// TODO: CHECK
							u32 dst = 0;
							bool carry = m_psr & PSR_C;
							unsigned const tcy = carry ? 18 : 16;
							for (unsigned digit = 0; digit < (size + 1) * 2; digit++)
							{
								unsigned sum = BIT(src1, digit * 4, 4) + BIT(src2, digit * 4, 4) + carry;

								if (sum > 9)
								{
									sum = sum - 10;
									carry = true;
								}
								else
									carry = false;

								dst |= sum << digit * 4;
							}

							if (carry)
								m_psr |= PSR_C;
							else
								m_psr &= ~PSR_C;

							gen_write(mode[1], dst);

							tex = mode[0].tea + mode[1].tea + tcy;
						}
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
								case SIZE_B: mem_write<u8>(ns32000::ST_ODT, block2, mem_read<u8>(ns32000::ST_ODT, block1)); break;
								case SIZE_W: mem_write<u16>(ns32000::ST_ODT, block2, mem_read<u16>(ns32000::ST_ODT, block1)); break;
								case SIZE_D: mem_write<u32>(ns32000::ST_ODT, block2, mem_read<u32>(ns32000::ST_ODT, block1)); break;
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
									int1 = s8(mem_read<u8>(ns32000::ST_ODT, block1));
									int2 = s8(mem_read<u8>(ns32000::ST_ODT, block2));
									break;
								case SIZE_W:
									int1 = s16(mem_read<u16>(ns32000::ST_ODT, block1));
									int2 = s16(mem_read<u16>(ns32000::ST_ODT, block2));
									break;
								case SIZE_D:
									int1 = s32(mem_read<u32>(ns32000::ST_ODT, block1));
									int2 = s32(mem_read<u32>(ns32000::ST_ODT, block2));
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
							interrupt(UND);
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
							interrupt(UND);
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
							interrupt(UND);
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
							interrupt(UND);
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
						interrupt(UND);
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
								interrupt(DVZ);
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
								interrupt(DVZ);
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
								interrupt(DVZ);
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
								interrupt(DVZ);
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
								interrupt(DVZ);
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
								u32 const base = mem_read<u32>(ns32000::ST_ODT, base_ea);

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
							interrupt(UND);
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
								u32 const base = mem_read<u32>(ns32000::ST_ODT, base_ea);
								u32 const mask = ((1U << length) - 1) << (offset & 7);

								mem_write<u32>(ns32000::ST_ODT, base_ea, (base & ~mask) | ((src << (offset & 7)) & mask));

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
								upper = s8(mem_read<u8>(ns32000::ST_ODT, bounds + 0));
								lower = s8(mem_read<u8>(ns32000::ST_ODT, bounds + 1));
								break;
							case SIZE_W:
								upper = s16(mem_read<u16>(ns32000::ST_ODT, bounds + 0));
								lower = s16(mem_read<u16>(ns32000::ST_ODT, bounds + 2));
								break;
							case SIZE_D:
								upper = s32(mem_read<u32>(ns32000::ST_ODT, bounds + 0));
								lower = s32(mem_read<u32>(ns32000::ST_ODT, bounds + 4));
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
								case SIZE_B: mem_write<u8>(ns32000::ST_ODT, ea(mode[1]), mem_read<u8>(ns32000::ST_ODT, ea(mode[0]), reg == 3), reg == 1); break;
								case SIZE_W: mem_write<u16>(ns32000::ST_ODT, ea(mode[1]), mem_read<u16>(ns32000::ST_ODT, ea(mode[0]), reg == 3), reg == 1); break;
								case SIZE_D: mem_write<u32>(ns32000::ST_ODT, ea(mode[1]), mem_read<u32>(ns32000::ST_ODT, ea(mode[0]), reg == 3), reg == 1); break;
								default:
									// can't happen
									break;
								}

								tex = mode[0].tea + mode[1].tea + top(size) * 2 + 33;
							}
							else
								interrupt(UND);
						}
						else
							interrupt(ILL);
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
							interrupt(SLV);
						break;
					case 1:
						// LFSR src
						//      gen
						//      read.D
						mode[0].read_i(size);
						decode(mode, bytes);

						if (slave(opbyte, opword, mode[0], mode[1]))
							interrupt(SLV);
						break;
					case 2:
						// MOVLF src,dst
						//       gen,gen
						//       read.L,write.F
						mode[0].read_f(SIZE_Q);
						mode[1].write_f(size_f);
						decode(mode, bytes);

						if (slave(opbyte, opword, mode[0], mode[1]))
							interrupt(SLV);
						break;
					case 3:
						// MOVFL src,dst
						//       gen,gen
						//       read.F,write.L
						mode[0].read_f(SIZE_D);
						mode[1].write_f(size_f);
						decode(mode, bytes);

						if (slave(opbyte, opword, mode[0], mode[1]))
							interrupt(SLV);
						break;
					case 4:
						// ROUNDfi src,dst
						//         gen,gen
						//         read.f,write.i
						mode[0].read_f(size_f);
						mode[1].write_i(size);
						decode(mode, bytes);

						if (slave(opbyte, opword, mode[0], mode[1]))
							interrupt(SLV);
						break;
					case 5:
						// TRUNCfi src,dst
						//         gen,gen
						//         read.f,write.i
						mode[0].read_f(size_f);
						mode[1].write_i(size);
						decode(mode, bytes);

						if (slave(opbyte, opword, mode[0], mode[1]))
							interrupt(SLV);
						break;
					case 6:
						// SFSR dst
						//      gen
						//      write.D
						mode[0].write_i(size);
						decode(mode, bytes);

						if (slave(opbyte, opword, mode[1], mode[0]))
							interrupt(SLV);
						break;
					case 7:
						// FLOORfi src,dst
						//         gen,gen
						//         read.f,write.i
						mode[0].read_f(size_f);
						mode[1].write_i(size);
						decode(mode, bytes);

						if (slave(opbyte, opword, mode[0], mode[1]))
							interrupt(SLV);
						break;
					}
				}
				else
					interrupt(UND);
				break;
			case 0x7e: // format 10
				interrupt(UND);
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
							interrupt(SLV);
						break;
					case 0x1:
						// MOVf src,dst
						//      gen,gen
						//      read.f,write.f
						mode[0].read_f(size_f);
						mode[1].write_f(size_f);
						decode(mode, bytes);

						if (slave(opbyte, opword, mode[0], mode[1]))
							interrupt(SLV);
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
								interrupt(SLV);
						}
						break;
					case 0x3:
						// Trap(SLAVE)
						// not defined; treat like CMPf
						mode[0].read_f(size_f);
						mode[1].read_f(size_f);
						decode(mode, bytes);

						if (slave(opbyte, opword, mode[0], mode[1]))
							interrupt(SLV);
						break;
					case 0x4:
						// SUBf src,dst
						//      gen,gen
						//      read.f,rmw.f
						mode[0].read_f(size_f);
						mode[1].rmw_f(size_f);
						decode(mode, bytes);

						if (slave(opbyte, opword, mode[0], mode[1]))
							interrupt(SLV);
						break;
					case 0x5:
						// NEGf src,dst
						//      gen,gen
						//      read.f,write.f
						mode[0].read_f(size_f);
						mode[1].write_f(size_f);
						decode(mode, bytes);

						if (slave(opbyte, opword, mode[0], mode[1]))
							interrupt(SLV);
						break;
					case 0x8:
						// DIVf src,dst
						//      gen,gen
						//      read.f,rmw.f
						mode[0].read_f(size_f);
						mode[1].rmw_f(size_f);
						decode(mode, bytes);

						if (slave(opbyte, opword, mode[0], mode[1]))
							interrupt(SLV);
						break;
					case 0x9:
						// Trap(SLAVE)
						// not defined; treat like MOVf
						mode[0].read_f(size_f);
						mode[1].write_f(size_f);
						decode(mode, bytes);

						if (slave(opbyte, opword, mode[0], mode[1]))
							interrupt(SLV);
						break;
					case 0xc:
						// MULf src,dst
						//      gen,gen
						//      read.f,rmw.f
						mode[0].read_f(size_f);
						mode[1].rmw_f(size_f);
						decode(mode, bytes);

						if (slave(opbyte, opword, mode[0], mode[1]))
							interrupt(SLV);
						break;
					case 0xd:
						// ABSf src,dst
						//      gen,gen
						//      read.f,write.f
						mode[0].read_f(size_f);
						mode[1].write_f(size_f);
						decode(mode, bytes);

						if (slave(opbyte, opword, mode[0], mode[1]))
							interrupt(SLV);
						break;
					}
				}
				else
					interrupt(UND);
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
							interrupt(SLV);
						break;
					case 0x3:
						// DOTf src,dst
						//      gen,gen
						//      read.f,read.f
						mode[0].read_f(size_f);
						mode[1].read_f(size_f);
						decode(mode, bytes);

						if (slave(opbyte, opword, mode[0], mode[1]))
							interrupt(SLV);
						break;
					case 0x4:
						// SCALBf src,dst
						//        gen,gen
						//        read.f,rmw.f
						mode[0].read_f(size_f);
						mode[1].rmw_f(size_f);
						decode(mode, bytes);

						if (slave(opbyte, opword, mode[0], mode[1]))
							interrupt(SLV);
						break;
					case 0x5:
						// LOGBf src,dst
						//       gen,gen
						//       read.f,write.f
						mode[0].read_f(size_f);
						mode[1].write_f(size_f);
						decode(mode, bytes);

						if (slave(opbyte, opword, mode[0], mode[1]))
							interrupt(SLV);
						break;
					case 0x0: // REMf
					case 0x8: // Trap(SLV)
					case 0xc: // ATAN2f
						// not defined; treat like ADDf
						mode[0].read_f(size_f);
						mode[1].rmw_f(size_f);
						decode(mode, bytes);

						if (slave(opbyte, opword, mode[0], mode[1]))
							interrupt(SLV);
						break;
					case 0x1: // SQRTf
					case 0x9: // Trap(SLV)
					case 0xd: // SICOSf
						// not defined; treat like MOVf
						mode[0].read_f(size_f);
						mode[1].write_f(size_f);
						decode(mode, bytes);

						if (slave(opbyte, opword, mode[0], mode[1]))
							interrupt(SLV);
						break;
					case 0x6: // Trap(UND)
					case 0x7: // Trap(UND)
					case 0xa: // Trap(UND)
					case 0xb: // Trap(UND)
					case 0xe: // Trap(UND)
					case 0xf: // Trap(UND)
						interrupt(UND);
						break;
					}
				}
				else
					interrupt(UND);
				break;
			case 0x9e: // format 13
				interrupt(UND);
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

								u16 const status = slave(opbyte, opword, mode[0], mode[1]);
								if (!(status & ns32000_slave_interface::SLAVE_Q))
								{
									if (status & ns32000_slave_interface::SLAVE_F)
										m_psr |= PSR_F;
									else
										m_psr &= ~PSR_F;
								}
								else
									interrupt(SLV);

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

								u16 const status = slave(opbyte, opword, mode[0], mode[1]);
								if (!(status & ns32000_slave_interface::SLAVE_Q))
								{
									if (status & ns32000_slave_interface::SLAVE_F)
										m_psr |= PSR_F;
									else
										m_psr &= ~PSR_F;
								}
								else
									interrupt(SLV);

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
								interrupt(SLV);

							tex = mode[0].tea + top(size);
							break;
						case 3:
							// SMR mmureg,dst
							//     short,gen
							//           write.D
							mode[0].write_i(size);
							decode(mode, bytes);

							if (slave(opbyte, opword, mode[1], mode[0]))
								interrupt(SLV);

							tex = mode[0].tea + top(size);
							break;
						case 9:
							if (type() == NS32532)
							{
								// CINV options,src
								//      short,gen
								//            read.D
								mode[0].read_i(size);
								decode(mode, bytes);

								if (slave(opbyte, opword, mode[0], mode[1]))
									interrupt(SLV);

								// TODO: tex
							}
							else
								interrupt(UND);
							break;
						default:
							interrupt(UND);
							break;
						}
					}
					else
						interrupt(UND);
				}
				else
					interrupt(ILL);
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
				interrupt(UND);
				break;
			}

			if (m_sequential)
				m_pc += bytes;

			// trace trap
			if (m_psr & PSR_P)
				interrupt(TRC);

			m_icount -= tex;
		}
		catch (ns32000_delay const &)
		{
			// HACK: approximate handling /RDY by restarting the instruction;
			// this approach can easily result in repeated incorrect memory
			// accesses, but is sufficient to support pc532 SCSI pseudo-DMA.

			// restore state
			SP() = m_ssp;
			m_psr = m_sps;

			// burn a cycle
			m_icount -= 1;
		}
		catch (ns32000_abort const &)
		{
			interrupt(ABT);
		}
	}
}

template <int HighBits, int Width> void ns32000_device<HighBits, Width>::execute_set_input(int inputnum, int state)
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

template <int HighBits, int Width> device_memory_interface::space_config_vector ns32000_device<HighBits, Width>::memory_space_config() const
{
	return space_config_vector{
		std::make_pair(AS_PROGRAM, &m_program_config),

		std::make_pair(ns32000::ST_IAM, &m_iam_config),
		std::make_pair(ns32000::ST_IAC, &m_iac_config),
		std::make_pair(ns32000::ST_EIM, &m_eim_config),
		std::make_pair(ns32000::ST_EIC, &m_eic_config),
		std::make_pair(ns32000::ST_SIF, &m_sif_config),
		std::make_pair(ns32000::ST_NIF, &m_nif_config),
		std::make_pair(ns32000::ST_ODT, &m_odt_config),
		std::make_pair(ns32000::ST_RMW, &m_rmw_config),
		std::make_pair(ns32000::ST_EAR, &m_ear_config),
	};
}

template <int HighBits, int Width> bool ns32000_device<HighBits, Width>::memory_translate(int spacenum, int intention, offs_t &address, address_space *&target_space)
{
	target_space = &space(spacenum);
	return !m_mmu || m_mmu->translate(space(spacenum), spacenum, address, m_psr & PSR_U, intention == TR_WRITE, false, true) == ns32000_mmu_interface::COMPLETE;
}

template <int HighBits, int Width> std::unique_ptr<util::disasm_interface> ns32000_device<HighBits, Width>::create_disassembler()
{
	return std::make_unique<ns32000_disassembler>();
}

template <int HighBits, int Width> u16 ns32000_device<HighBits, Width>::slave(u8 opbyte, u16 opword, addr_mode op1, addr_mode op2)
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

template <int HighBits, int Width> u16 ns32000_device<HighBits, Width>::slave_slow(ns32000_slow_slave_interface &slave, u8 opbyte, u16 opword, addr_mode op1, addr_mode op2)
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
	else if (op1.access == ADDR)
	{
		u32 const data = ea(op1);

		slave.write_op(u16(data >> 0));
		slave.write_op(u16(data >> 16));

		// single-byte memory read cycle
		mem_read<u8>(ns32000::ST_ODT, data, true);
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

template <int HighBits, int Width> u16 ns32000_device<HighBits, Width>::slave_fast(ns32000_fast_slave_interface &slave, u8 opbyte, u16 opword, addr_mode op1, addr_mode op2)
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
	else if (op1.access == ADDR)
	{
		u32 const data = ea(op1);

		slave.write(u32(data));

		// single-byte memory read cycle
		mem_read<u8>(ns32000::ST_ODT, data, true);
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
	u32 const status = slave.read_st32(&m_icount);

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

void ns32532_device::device_add_mconfig(machine_config &config)
{
	set_mmu(*this);
}

void ns32532_device::device_reset()
{
	ns32000_device<32, 2>::device_reset();

	m_cfg = CFG_P | CFG_FC | CFG_FM | CFG_FF;
}

device_memory_interface::space_config_vector ns32532_device::memory_space_config() const
{
	auto vector = ns32000_device<32, 2>::memory_space_config();

	vector.push_back(std::make_pair(ns32000::ST_PT1, &m_pt1_config));
	vector.push_back(std::make_pair(ns32000::ST_PT2, &m_pt2_config));

	return vector;
}

void ns32532_device::state_add(device_state_interface &parent, int &index)
{
	save_item(NAME(m_ptb));
	save_item(NAME(m_tear));
	save_item(NAME(m_mcr));
	save_item(NAME(m_msr));

	save_item(NAME(m_dcr));
	save_item(NAME(m_dsr));
	save_item(NAME(m_car));
	save_item(NAME(m_bpc));

	parent.state_add(index++, "PTB0", m_ptb[0]);
	parent.state_add(index++, "PTB1", m_ptb[1]);
	parent.state_add(index++, "TEAR", m_tear);
	parent.state_add(index++, "MCR", m_mcr);
	parent.state_add(index++, "MSR", m_msr);

	parent.state_add(index++, "DCR", m_dcr);
	parent.state_add(index++, "DSR", m_dsr);
	parent.state_add(index++, "CAR", m_car);
	parent.state_add(index++, "BPC", m_bpc);
}

static constexpr u32 MSR(unsigned st, bool user, bool write, unsigned tex)
{
	enum msr_mask : u32
	{
		MSR_TEX = 0x0003, // translation exception
		MSR_DDT = 0x0004, // data direction (1=write)
		MSR_UST = 0x0008, // user/supervisor (1=user)
		MSR_STT = 0x00f0, // cpu status
	};

	return u32(((st << 4) & MSR_STT) | (user ? MSR_UST : 0) | (write ? MSR_DDT : 0) | (tex & MSR_TEX));
}

ns32000_mmu_interface::translate_result ns32532_device::translate(address_space &space, unsigned st, u32 &address, bool user, bool write, bool rdwrval, bool suppress)
{
	enum mcr_mask : u32
	{
		MCR_TU = 0x0001, // translate user-mode addresses
		MCR_TS = 0x0002, // translate supervisor-mode addresses
		MCR_DS = 0x0004, // dual-space translation
		MCR_AO = 0x0008, // access level override
	};

	enum msr_tex_mask : u32
	{
		TEX_PTE1 = 0x1,
		TEX_PTE2 = 0x2,
		TEX_PROT = 0x3,
	};

	enum pte_mask : u32
	{
		PTE_V   = 0x0000'0001, // valid
		PTE_PL  = 0x0000'0006, // protection level
		PTE_CI  = 0x0000'0020, // cache inhibit
		PTE_R   = 0x0000'0040, // referenced
		PTE_M   = 0x0000'0080, // modified
		PTE_PFN = 0xffff'f000, // page frame number
	};

	enum pte_pl_mask : u32
	{
		PL_SRO = 0x00000000, // supervisor read only
		PL_SRW = 0x00000002, // supervisor read write
		PL_URO = 0x00000004, // user read only
		PL_URW = 0x00000006, // user read write
	};

	// check translation required
	if ((!user && !(m_mcr & MCR_TS)) || (user && !(m_mcr & MCR_TU)))
		return COMPLETE;

	// TODO: translation look-aside buffer

	bool const address_space = (m_mcr & MCR_DS) && user;
	unsigned const access_level = (user && !(m_mcr & MCR_AO))
		? ((write || st == ns32000::ST_RMW) ? PL_URW : PL_URO)
		: ((write || st == ns32000::ST_RMW) ? PL_SRW : PL_SRO);

	LOGMASKED(LOG_TRANSLATE, "translate address_space %d access_level %d page table 0x%08x address 0x%08x\n",
		address_space, access_level, m_ptb[address_space], address);

	// read level 1 page table entry
	u32 const pte1_address = m_ptb[address_space] | (BIT(address, 22, 10) * 4);
	u32 const pte1 = m_bus[ns32000::ST_PT1].read_dword(pte1_address);
	LOGMASKED(LOG_TRANSLATE, "translate level 1 page table address 0x%08x entry 0x%08x\n", pte1_address, pte1);

	// access check
	if ((pte1 & PTE_PL) < access_level)
	{
		if (!suppress)
		{
			m_msr = MSR(st, user, write, TEX_PROT);
			m_tear = address;

			LOGMASKED(LOG_TRANSLATE, "translate level 1 protection exception\n");

			return ABORT;
		}
		else
			return CANCEL;
	}
	if (!(pte1 & PTE_V))
	{
		if (!suppress)
		{
			m_msr = MSR(st, user, write, TEX_PTE1);
			m_tear = address;

			LOGMASKED(LOG_TRANSLATE, "translate level 1 invalid\n");
		}

		return ABORT;
	}

	// set referenced
	if (!(pte1 & PTE_R) && !suppress)
		m_bus[ns32000::ST_PT1].write_byte(pte1_address, pte1 | PTE_R);

	// read level 2 page table entry
	u32 const pte2_address = (pte1 & PTE_PFN) | (BIT(address, 12, 10) * 4);
	u32 const pte2 = m_bus[ns32000::ST_PT2].read_dword(pte2_address);
	LOGMASKED(LOG_TRANSLATE, "translate level 2 page table address 0x%08x entry 0x%08x\n", pte2_address, pte2);

	// access check
	if ((pte2 & PTE_PL) < access_level)
	{
		if (!suppress)
		{
			m_msr = MSR(st, user, write, TEX_PROT);
			m_tear = address;

			LOGMASKED(LOG_TRANSLATE, "translate level 2 protection exception\n");

			return ABORT;
		}
		else
			return CANCEL;
	}
	if (!(pte2 & PTE_V))
	{
		if (!suppress)
		{
			m_msr = MSR(st, user, write, TEX_PTE2);
			m_tear = address;

			LOGMASKED(LOG_TRANSLATE, "translate level 2 invalid\n");

			return ABORT;
		}
		else
			return rdwrval ? COMPLETE : CANCEL;
	}

	// set modified and referenced
	if ((!(pte2 & PTE_R) || ((write || st == ns32000::ST_RMW) && !(pte2 & PTE_M))) && !suppress)
		m_bus[ns32000::ST_PT2].write_dword(pte2_address, pte2 | ((write || st == ns32000::ST_RMW) ? PTE_M : 0) | PTE_R);

	address = (pte2 & PTE_PFN) | BIT(address, 0, 12);
	LOGMASKED(LOG_TRANSLATE, "translate complete 0x%08x\n", address);

	return COMPLETE;
}

void ns32532_device::lpr(unsigned reg, addr_mode const mode, bool user, unsigned &tex)
{
	switch (reg)
	{
	case 0x1:
		if (!user)
			m_dcr = gen_read(mode);
		else
			interrupt(ILL);
		break;
	case 0x2:
		if (!user)
			m_bpc = gen_read(mode);
		else
			interrupt(ILL);
		break;
	case 0x3:
		if (!user)
			m_dsr = gen_read(mode);
		else
			interrupt(ILL);
		break;
	case 0x4:
		if (!user)
			m_car = gen_read(mode);
		else
			interrupt(ILL);
		break;
	case 0xb:
		if (!user)
			SP(true) = gen_read(mode);
		else
			interrupt(ILL);
		break;
	case 0xc:
		if (!user)
			m_cfg = gen_read(mode) | (CFG_P | CFG_FC | CFG_FM | CFG_FF);
		else
			interrupt(ILL);
		break;

	default:
		ns32000_device<32, 2>::lpr(reg, mode, user, tex);
		return;
	}

	// TODO: tcy 19-33
	tex = mode.tea + 19;
}

void ns32532_device::spr(unsigned reg, addr_mode const mode, bool user, unsigned &tex)
{
	switch (reg)
	{
	case 0x1:
		if (!user)
			gen_write(mode, m_dcr);
		else
			interrupt(ILL);
		break;
	case 0x2:
		if (!user)
			gen_write(mode, m_bpc);
		else
			interrupt(ILL);
		break;
	case 0x3:
		if (!user)
			gen_write(mode, m_dsr);
		else
			interrupt(ILL);
		break;
	case 0x4:
		if (!user)
			gen_write(mode, m_car);
		else
			interrupt(ILL);
		break;
	case 0xb:
		if (!user)
			gen_write(mode, SP(true));
		else
			interrupt(ILL);
		break;
	case 0xc:
		if (!user)
			gen_write(mode, m_cfg);
		else
			interrupt(ILL);
		break;

	default:
		ns32000_device<32, 2>::spr(reg, mode, user, tex);
		return;
	}

	// TODO: tcy 21-27
	tex = mode.tea + 21;
}


u16 ns32532_device::slave(u8 opbyte, u16 opword, addr_mode op1, addr_mode op2)
{
	if (opbyte == 0x1e)
	{
		switch (BIT(opword, 2, 4))
		{
		case 0: // rdval
		case 1: // wrval
			{
				u32 address = ea(op1);

				switch (translate(space(AS_PROGRAM), ns32000::ST_ODT, address, true, BIT(opword, 2), true, true))
				{
				case CANCEL:
					return ns32000_slave_interface::SLAVE_F;

				case ABORT:
					interrupt(ABT);
					break;

				default:
					break;
				}
			}
			break;

		case 2: // lmr
			switch (BIT(opword, 7, 4))
			{
			case 0x8: break;
			case 0x9: m_mcr = gen_read(op1); break;
			case 0xa: m_msr = gen_read(op1); break;
			case 0xb: m_tear = gen_read(op1); break;
			case 0xc: m_ptb[0] = gen_read(op1) & ~0xfffU; break; // TODO: invalidate TLB
			case 0xd: m_ptb[1] = gen_read(op1) & ~0xfffU; break; // TODO: invalidate TLB
			case 0xe: gen_read(op1); break; // ivar0
			case 0xf: gen_read(op1); break; // ivar1
			default:
				interrupt(UND);
				break;
			}
			break;

		case 3: // smr
			switch (BIT(opword, 7, 4))
			{
			case 0x8: break;
			case 0x9: gen_write(op2, m_mcr); break;
			case 0xa: gen_write(op2, m_msr); break;
			case 0xb: gen_write(op2, m_tear); break;
			case 0xc: gen_write(op2, m_ptb[0]); break;
			case 0xd: gen_write(op2, m_ptb[1]); break;
			case 0xe: break;
			case 0xf: break;
			default:
				interrupt(UND);
				break;
			}
			break;

		case 9: // cinv
			// TODO: invalidate cache
			break;

		default:
			interrupt(UND);
			break;
		}
	}
	else
		return ns32000_device<32, 2>::slave(opbyte, opword, op1, op2);

	return ns32000_slave_interface::SLAVE_OK;
}
