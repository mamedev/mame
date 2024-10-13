// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Motorola M88000 Family of RISC microprocessors.
 *
 * TODO:
 *  - instruction cycle counts
 *  - mc88110
 *  - little-endian mode
 */

#include "emu.h"
#include "m88000.h"
#include "m88000d.h"

#define LOG_EXCEPTION (1U << 1)

//#define VERBOSE       (LOG_GENERAL|LOG_EXCEPTION)

#include "logmacro.h"

enum exception_number : unsigned
{
	E_RESET         =   0, // reset exception
	E_INTERRUPT     =   1, // interrupt exception
	E_INSTRUCTION   =   2, // instruction access exception
	E_DATA          =   3, // data access exception
	E_MISALIGNED    =   4, // misaligned access exception
	E_UNIMPLEMENTED =   5, // unimplemented opcode exception
	E_PRIVILEGE     =   6, // privilege violation exception
	E_BOUNDS        =   7, // bounds check violation exception
	E_INT_DIVIDE    =   8, // illegal integer divide exception
	E_INT_OVERFLOW  =   9, // integer overflow exception
	E_ERROR         =  10, // error exception

	E_SFU1_P        = 114, // sfu 1 precise - floating-point precise exception
	E_SFU1_I        = 115, // sfu 1 imprecise - floating-point imprecise exception
	E_SFU2_P        = 116, // sfu 2 precise exception
	E_SFU3_P        = 118, // sfu 3 precise exception
	E_SFU4_P        = 120, // sfu 4 precise exception
	E_SFU5_P        = 122, // sfu 5 precise exception
	E_SFU6_P        = 124, // sfu 6 precise exception
	E_SFU7_P        = 126, // sfu 7 precise exception
};

enum cr_number : unsigned
{
	PID  =  0, // processor identification
	PSR  =  1, // processor status
	EPSR =  2, // exception-time processor status
	SSBR =  3, // shadow scoreboard
	SXIP =  4, // shadow execute instruction pointer
	SNIP =  5, // shadow next instruction pointer
	SFIP =  6, // shadow fetch instruction pointer
	VBR  =  7, // vector base
	DMT0 =  8, // data memory transaction 0
	DMD0 =  9, // data memory data 0
	DMA0 = 10, // data memory address 0
	DMT1 = 11, // data memory transaction 1
	DMD1 = 12, // data memory data 1
	DMA1 = 13, // data memory address 1
	DMT2 = 14, // data memory transaction 2
	DMD2 = 15, // data memory data 2
	DMA2 = 16, // data memory address 2
	SR0  = 17, // supervisor storage 0
	SR1  = 18, // supervisor storage 1
	SR2  = 19, // supervisor storage 2
	SR3  = 20, // supervisor storage 3
};

enum fcr_number : unsigned
{
	FPECR =  0, // floating-point exception cause
	FPHS1 =  1, // floating-point source 1 operand high
	FPLS1 =  2, // floating-point source 1 operand low
	FPHS2 =  3, // floating-point source 2 operand high
	FPLS2 =  4, // floating-point source 2 operand low
	FPPT  =  5, // floating-point precise operation type
	FPRH  =  6, // floating-point result high
	FPRL  =  7, // floating-point result low
	FPIT  =  8, // floating-point imprecise operation type
	FPSR  = 62, // floating-point user status
	FPCR  = 63, // floating-point user control
};

enum ip_mask : u32
{
	IP_A = 0xffff'fffc, // address
	IP_V = 0x0000'0002, // valid
	IP_E = 0x0000'0001, // exception
};

enum psr_mask : u32
{
	PSR_SFRZ = 0x0000'0001, // shadow freeze
	PSR_IND  = 0x0000'0002, // interrupt disable
	PSR_MXM  = 0x0000'0004, // misaligned access enable
	PSR_SFD1 = 0x0000'0008, // sfu1 disable
	PSR_SFD  = 0x0000'03f0, // sfu2-7 disable
	PSR_C    = 0x1000'0000, // carry
	PSR_SER  = 0x2000'0000, // serial mode
	PSR_BO   = 0x4000'0000, // byte ordering (1=little-endian)
	PSR_MODE = 0x8000'0000, // supervisor/user mode (1=supervisor)
};

enum cr_mask : u32
{
	PSR_MASK = 0xf000'000f,
	VBR_MASK = 0xffff'f000,
};

enum fcr_mask : u32
{
	FPECR_MASK = 0x0000'00ff,
	FPSR_MASK  = 0x0000'001f,
	FPCR_MASK  = 0x0000'c01f,
};

enum dmt_mask : u32
{
	DMT_VALID  = 0x0000'0001, // valid transaction bit
	DMT_WRITE  = 0x0000'0002, // read/write transaction bit
	DMT_EN0    = 0x0000'0004, // byte enable 0
	DMT_EN1    = 0x0000'0008, // byte enable 1
	DMT_EN2    = 0x0000'0010, // byte enable 2
	DMT_EN3    = 0x0000'0020, // byte enable 3
	DMT_SD     = 0x0000'0040, // sign-extend bit
	DMT_DREG   = 0x0000'0f80, // destination register
	DMT_LOCK   = 0x0000'1000, // bus lock
	DMT_DOUB1  = 0x0000'2000, // double word
	DMT_DAS    = 0x0000'4000, // data address space
	DMT_BO     = 0x0000'8000, // byte ordering
};

constexpr static u32 DMT_EN() { return (DMT_EN3 | DMT_EN2 | DMT_EN1 | DMT_EN0); }

// return data memory transaction byte enables given data width and address
template <typename T> static u32 DMT_EN(u32 const address)
{
	return (((DMT_EN() << (4 - sizeof(T))) & DMT_EN()) >> (address & 3));
}

// device type definitions
DEFINE_DEVICE_TYPE(MC88100, mc88100_device, "mc88100", "Motorola MC88100")

mc88100_device::mc88100_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: cpu_device(mconfig, MC88100, tag, owner, clock)
	, m_code_config("code", ENDIANNESS_BIG, 32, 32, 0)
	, m_data_config("data", ENDIANNESS_BIG, 32, 32, 0)
	, m_cmmu_code(nullptr)
	, m_cmmu_data(nullptr)
	, m_sb(0)
	, m_r{ 0 }
	, m_cr{ 0 }
	, m_fcr{ 0 }
	, m_int_state(false)
	, m_icount(0)
{
	m_cr[PID] = 0x00000001;

	softfloat_roundingMode = softfloat_round_near_even;
}

std::unique_ptr<util::disasm_interface> mc88100_device::create_disassembler()
{
	return std::make_unique<mc88100_disassembler>();
}

device_memory_interface::space_config_vector mc88100_device::memory_space_config() const
{
	if (has_configured_map(AS_DATA))
		return space_config_vector{ std::make_pair(AS_PROGRAM, &m_code_config), std::make_pair(AS_DATA, &m_data_config) };
	else
		return space_config_vector{ std::make_pair(AS_PROGRAM, &m_code_config) };
}

bool mc88100_device::memory_translate(int spacenum, int intention, offs_t &address, address_space *&target_space)
{
	target_space = &space(spacenum);

	switch (intention)
	{
	case TR_READ:
	case TR_WRITE:
		if (m_cmmu_data)
			return m_cmmu_data(address).translate(intention, address, m_cr[PSR] & PSR_MODE);
		break;

	case TR_FETCH:
		if (m_cmmu_code)
			return m_cmmu_code(address).translate(intention, address, m_cr[PSR] & PSR_MODE);
		break;
	}

	return true;
}

void mc88100_device::device_start()
{
	space(AS_PROGRAM).specific(m_code_space);

	if (has_configured_map(AS_DATA))
		space(AS_DATA).specific(m_data_space);
	else
		space(AS_PROGRAM).specific(m_data_space);

	set_icountptr(m_icount);

	state_add(STATE_GENPC,     "GENPC", m_xip).mask(IP_A).noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_xip).mask(IP_A).noshow();

	state_add(32, "XIP", m_xip).mask(IP_A).readonly();
	state_add(33, "NIP", m_nip).mask(IP_A).readonly();
	state_add(34, "FIP", m_fip,
		[this](u32 data)
		{
			m_xip = 0;
			m_nip = 0;
			m_fip = (data & IP_A) | IP_V;
		}).mask(IP_A);
	state_add(35, "SB", m_sb);

	state_add(36 + PSR, "PSR", m_cr[PSR]);
	state_add(36 + EPSR, "EPSR", m_cr[EPSR]);
	state_add(36 + SSBR, "SSBR", m_cr[SSBR]);
	state_add(36 + SXIP, "SXIP", m_cr[SXIP]);
	state_add(36 + SNIP, "SNIP", m_cr[SNIP]);
	state_add(36 + SFIP, "SFIP", m_cr[SFIP]);
	state_add(36 + VBR, "VBR", m_cr[VBR]);
	state_add(36 + SR0, "sr0", m_cr[SR0]);
	state_add(36 + SR1, "sr1", m_cr[SR1]);
	state_add(36 + SR2, "sr2", m_cr[SR2]);
	state_add(36 + SR3, "sr3", m_cr[SR3]);

	for (int i = 0; i < 32; i++)
		state_add(i, string_format("r%d", i).c_str(), m_r[i]);

	save_item(NAME(m_xip));
	save_item(NAME(m_nip));
	save_item(NAME(m_fip));
	save_item(NAME(m_sb));

	save_item(NAME(m_r));
	save_item(NAME(m_cr));
	save_item(NAME(m_fcr));

	save_item(NAME(m_int_state));
}

void mc88100_device::device_reset()
{
	m_cr[PSR] = PSR_MODE | PSR_SFD | PSR_SFD1 | PSR_IND | PSR_SFRZ;
	m_cr[EPSR] = PSR_SFD;
	m_cr[VBR] = 0;

	m_xip = 0;
	m_nip = 0;
	m_fip = IP_V;

	m_xop = 0;
	m_nop = 0;
	m_fop = 0;
}

void mc88100_device::execute_run()
{
	while (m_icount > 0)
	{
		// execute
		if (m_xip & IP_V)
		{
			debugger_instruction_hook(m_xip & IP_A);

			if (!(m_xip & IP_E))
				execute(m_xop);
			else
				exception(E_INSTRUCTION);

			// interrupt check
			if (m_int_state && !(m_cr[PSR] & PSR_IND))
				exception(E_INTERRUPT);
		}

		// fetch
		if (m_fip & IP_V)
			fetch(m_fip, m_fop);

		// advance pipeline
		m_xop = m_nop;
		m_xip = m_nip;
		m_nop = m_fop;
		m_nip = m_fip;
		m_fip &= ~IP_E;
		m_fip += 4;

		m_icount--;
	}
}

// decoding macros
#define D     BIT(inst, 21, 5)

#define S1    BIT(inst, 16, 5)
#define S1H   (((inst >> 16) + 0) & 31)
#define S1L   (((inst >> 16) + 1) & 31)

#define S2    BIT(inst, 0, 5)
#define S2H   ((inst + 0) & 31)
#define S2L   ((inst + 1) & 31)

#define IMM16 BIT(inst, 0, 16)
#define VEC9  BIT(inst, 0, 9)
#define CR    BIT(inst, 5, 6)

void mc88100_device::execute(u32 const inst)
{
	switch (BIT(inst, 26, 6))
	{
		// load/store/exchange immediate
	case 0x00: // xmem.bu: exchange register with memory unsigned byte
		xmem<u8>(m_r[S1] + IMM16, D);
		break;
	case 0x01: // xmem: exchange register with memory word
		xmem<u32>(m_r[S1] + IMM16, D);
		break;
	case 0x02: // ld.hu: load half word unsigned
		ld<u16>(m_r[S1] + IMM16, D);
		break;
	case 0x03: // ld.bu: load byte unsigned
		ld<u8>(m_r[S1] + IMM16, D);
		break;
	case 0x04: // ld.d: load double word
		ld<u64>(m_r[S1] + IMM16, D);
		break;
	case 0x05: // ld: load word
		ld<u32>(m_r[S1] + IMM16, D);
		break;
	case 0x06: // ld.h: load half word
		ld<s16>(m_r[S1] + IMM16, D);
		break;
	case 0x07: // ld.b: load byte
		ld<s8>(m_r[S1] + IMM16, D);
		break;
	case 0x08: // st.d: store double word (unscaled)
		st<u64>(m_r[S1] + IMM16, D);
		break;
	case 0x09: // st: store word (unscaled)
		st<u32>(m_r[S1] + IMM16, D);
		break;
	case 0x0a: // st.h: store half word (unscaled)
		st<u16>(m_r[S1] + IMM16, D);
		break;
	case 0x0b: // st.b: store byte (unscaled)
		st<u8>(m_r[S1] + IMM16, D);
		break;
	case 0x0c: // lda.d: load address double word (unscaled)
	case 0x0d: // lda: load address word (unscaled)
	case 0x0e: // lda.h: load address half word (unscaled)
	case 0x0f: // lda.b: load address byte (unscaled)
		m_r[D] = m_r[S1] + IMM16;
		break;

		// logical immediate
	case 0x10: // and: logical and (immediate)
		m_r[D] = m_r[S1] & (0xffff0000U | IMM16);
		break;
	case 0x11: // and.u: logical and upper (immediate)
		m_r[D] = m_r[S1] & ((u32(IMM16) << 16) | 0x0000ffffU);
		break;
	case 0x12: // mask: logical mask (immediate)
		m_r[D] = m_r[S1] & IMM16;
		break;
	case 0x13: // mask.u: logical mask upper (immediate)
		m_r[D] = m_r[S1] & (u32(IMM16) << 16);
		break;
	case 0x14: // xor: logical exclusive or (immediate)
		m_r[D] = m_r[S1] ^ IMM16;
		break;
	case 0x15: // xor.u: logical exclusive or upper (immediate)
		m_r[D] = m_r[S1] ^ (u32(IMM16) << 16);
		break;
	case 0x16: // or: logical or (immediate)
		m_r[D] = m_r[S1] | IMM16;
		break;
	case 0x17: // or.u: logical or upper (immediate)
		m_r[D] = m_r[S1] | (u32(IMM16) << 16);
		break;

		// integer arithmetic
	case 0x18: // addu: unsigned integer add (immediate)
		m_r[D] = m_r[S1] + IMM16;
		break;
	case 0x19: // subu: unsigned integer subtract (immediate)
		m_r[D] = m_r[S1] + ~IMM16 + 1;
		break;
	case 0x1a: // divu: unsigned integer divide (immediate)
		if (!(m_cr[PSR] & PSR_SFD1))
		{
			if (IMM16)
				m_r[D] = m_r[S1] / IMM16;
			else
				exception(E_INT_DIVIDE);
		}
		else
			exception(E_SFU1_P);
		break;
	case 0x1b: // mul: integer multiply (immediate)
		if (!(m_cr[PSR] & PSR_SFD1))
			m_r[D] = m_r[S1] * IMM16;
		else
			exception(E_SFU1_P);
		break;
	case 0x1c: // add: integer add (immediate)
		m_r[D] = m_r[S1] + IMM16;
		break;
	case 0x1d: // sub: integer subtract (immediate)
		m_r[D] = m_r[S1] + ~IMM16 + 1;
		break;
	case 0x1e: // div: signed integer divide (immediate)
		if (!(m_cr[PSR] & PSR_SFD1))
		{
			if (IMM16 && !BIT(m_r[S1], 31))
				m_r[D] = s32(m_r[S1]) / s32(IMM16);
			else
				exception(E_INT_DIVIDE);
		}
		else
			exception(E_SFU1_P);
		break;
	case 0x1f: // cmp: integer compare (immediate)
		m_r[D] = cmp(m_r[S1], IMM16);
		break;

		// control registers
	case 0x20:
		switch (BIT(inst, 11, 5))
		{
		case 0x08: // ldcr: load from control register (privileged)
			if (m_cr[PSR] & PSR_MODE)
			{
				switch (CR)
				{
				case SSBR: m_r[D] = (m_cr[PSR] & PSR_SFRZ) ? m_cr[CR] : m_sb; break;
				case SXIP: m_r[D] = (m_cr[PSR] & PSR_SFRZ) ? m_cr[CR] : m_xip; break;
				case SNIP: m_r[D] = (m_cr[PSR] & PSR_SFRZ) ? m_cr[CR] : m_nip; break;
				case SFIP: m_r[D] = (m_cr[PSR] & PSR_SFRZ) ? m_cr[CR] : m_fip; break;
				default:
					m_r[D] = m_cr[CR];
					break;
				}
			}
			else
				exception(E_PRIVILEGE);
			break;
		case 0x09: // fldcr: load from floating-point control register
			if ((m_cr[PSR] & PSR_MODE) || (CR >= FPSR))
				m_r[D] = m_fcr[CR];
			else
				exception(E_PRIVILEGE);
			break;
		case 0x10: // stcr: store to control register (privileged)
			if (m_cr[PSR] & PSR_MODE)
				set_cr(CR, m_r[S1]);
			else
				exception(E_PRIVILEGE);
			break;
		case 0x11: // fstcr: store to floating-point control register
			if ((m_cr[PSR] & PSR_MODE) || (CR >= FPSR))
				set_fcr(CR, m_r[S1]);
			else
				exception(E_PRIVILEGE);
			break;
		case 0x18: // xcr: exchange control register (privileged)
			if (m_cr[PSR] & PSR_MODE)
			{
				u32 const data = m_r[S1];

				switch (CR)
				{
				case SSBR: m_r[D] = (m_cr[PSR] & PSR_SFRZ) ? m_cr[CR] : m_sb; break;
				case SXIP: m_r[D] = (m_cr[PSR] & PSR_SFRZ) ? m_cr[CR] : m_xip; break;
				case SNIP: m_r[D] = (m_cr[PSR] & PSR_SFRZ) ? m_cr[CR] : m_nip; break;
				case SFIP: m_r[D] = (m_cr[PSR] & PSR_SFRZ) ? m_cr[CR] : m_fip; break;
				default:
					m_r[D] = m_cr[CR];
					break;
				}

				set_cr(CR, data);
			}
			else
				exception(E_PRIVILEGE);
			break;
		case 0x19: // fxcr: exchange floating-point control register
			if ((m_cr[PSR] & PSR_MODE) || (CR >= FPSR))
			{
				u32 const data = m_r[S1];

				m_r[D] = m_fcr[CR];
				set_fcr(CR, data);
			}
			else
				exception(E_PRIVILEGE);
			break;
		default:
			exception(E_UNIMPLEMENTED);
			break;
		}
		break;
	case 0x21: // floating-point
		if (!(m_cr[PSR] & PSR_SFD1) && D)
		{
			unsigned const td = BIT(inst, 5, 2);

			float64_t const s1 = BIT(inst, 9, 2) ? float64_t{ (u64(m_r[S1H]) << 32) | m_r[S1L] } : f32_to_f64(float32_t{ m_r[S1] });
			float64_t const s2 = BIT(inst, 7, 2) ? float64_t{ (u64(m_r[S2H]) << 32) | m_r[S2L] } : f32_to_f64(float32_t{ m_r[S2] });

			switch (BIT(inst, 11, 5))
			{
			case 0x00: fset(td, D, f64_mul(s1, s2)); break; // fmul
			case 0x04: fset(td, D, i32_to_f64(s32(m_r[S2]))); break; // flt
			case 0x05: fset(td, D, f64_add(s1, s2)); break; // fadd
			case 0x06: fset(td, D, f64_sub(s1, s2)); break; // fsub
			case 0x07: m_r[D] = fcmp(s1, s2); break; // fcmp
			case 0x09: m_r[D] = f64_to_i32(s2, softfloat_roundingMode, true); break; // int
			case 0x0a: m_r[D] = f64_to_i32(s2, softfloat_round_near_even, true); break; // nint
			case 0x0b: m_r[D] = f64_to_i32(s2, softfloat_round_minMag, true); break; // trunc
			case 0x0e: fset(td, D, f64_div(s1, s2)); break; // fdiv
			}
		}
		else
			exception(E_SFU1_P);
		break;

		// special-function units 2-7
	case 0x22: exception(E_SFU2_P); break;
	case 0x23: exception(E_SFU3_P); break;
	case 0x24: exception(E_SFU4_P); break;
	case 0x25: exception(E_SFU5_P); break;
	case 0x26: exception(E_SFU6_P); break;
	case 0x27: exception(E_SFU7_P); break;

	case 0x28: case 0x29: case 0x2a: case 0x2b:
	case 0x2c: case 0x2d: case 0x2e: case 0x2f:
		exception(E_UNIMPLEMENTED);
		break;
		// flow-control
	case 0x30: // br: unconditional branch
		m_fip = m_xip + (util::sext(inst, 26) << 2);
		m_nip &= ~IP_V;
		break;
	case 0x31: // br.n: unconditional branch (delayed)
		m_fip = m_xip + (util::sext(inst, 26) << 2);
		break;
	case 0x32: // bsr: branch to subroutine
		m_fip = m_xip + (util::sext(inst, 26) << 2);
		m_r[1] = m_nip & IP_A;
		m_nip &= ~IP_V;
		break;
	case 0x33: // bsr.n: branch to subroutine (delayed)
		m_fip = m_xip + (util::sext(inst, 26) << 2);
		m_r[1] = (m_nip & IP_A) + 4;
		break;
	case 0x34: // bb0: branch on bit clear
		if (!BIT(m_r[S1], D))
		{
			m_fip = m_xip + (util::sext(inst, 16) << 2);
			m_nip &= ~IP_V;
		}
		break;
	case 0x35: // bb0.n: branch on bit clear (delayed)
		if (!BIT(m_r[S1], D))
			m_fip = m_xip + (util::sext(inst, 16) << 2);
		break;
	case 0x36: // bb1: branch on bit set
		if (BIT(m_r[S1], D))
		{
			m_fip = m_xip + (util::sext(inst, 16) << 2);
			m_nip &= ~IP_V;
		}
		break;
	case 0x37: // bb1.n: branch on bit set (delayed)
		if (BIT(m_r[S1], D))
			m_fip = m_xip + (util::sext(inst, 16) << 2);
		break;
	case 0x38:
	case 0x39:
		exception(E_UNIMPLEMENTED);
		break;
	case 0x3a: // bcnd: conditional branch
		if (condition(D, m_r[S1]))
		{
			m_fip = m_xip + (util::sext(inst, 16) << 2);
			m_nip &= ~IP_V;
		}
		break;
	case 0x3b: // bcnd.n: conditional branch (delayed)
		if (condition(D, m_r[S1]))
			m_fip = m_xip + (util::sext(inst, 16) << 2);
		break;
	case 0x3c: // bit field
		switch (BIT(inst, 10, 6))
		{
		case 0x20: // clr: clear bit field (immediate)
			{
				unsigned const width = BIT(inst, 5, 5);
				unsigned const offset = inst & 31;

				m_r[D] = m_r[S1] & ~(make_bitmask<u32>(width ? width : 32) << offset);
			}
			break;
		case 0x22: // set: set bit field (immediate)
			{
				unsigned const width = BIT(inst, 5, 5);
				unsigned const offset = inst & 31;

				m_r[D] = m_r[S1] | (make_bitmask<u32>(width ? width : 32) << offset);
			}
			break;
		case 0x24: // ext: extract signed bit field (immediate)
			{
				unsigned const width = BIT(inst, 5, 5);
				unsigned const offset = inst & 31;

				if (width && (width + offset) < 32)
					m_r[D] = util::sext(m_r[S1] >> offset, width);
				else
					m_r[D] = s32(m_r[S1]) >> offset;
			}
			break;
		case 0x26: // extu: extract unsigned bit field (immediate)
			{
				unsigned const width = BIT(inst, 5, 5);
				unsigned const offset = inst & 31;

				if (width)
					m_r[D] = BIT(m_r[S1], offset, width);
				else
					m_r[D] = m_r[S1] >> offset;
			}
			break;
		case 0x28: // mak: make bit field (immediate)
			{
				unsigned const width = BIT(inst, 5, 5);
				unsigned const offset = inst & 31;

				if (width)
					m_r[D] = (m_r[S1] & make_bitmask<u32>(width)) << offset;
				else
					m_r[D] = m_r[S1] << offset;
			}
			break;
		case 0x2a: // rot: rotate (immediate)
			{
				unsigned const offset = inst & 31;

				m_r[D] = rotr_32(m_r[S1], offset);
			}
			break;

		case 0x34: // tb0: trap on bit clear
			if ((m_cr[PSR] & PSR_MODE) || (VEC9 > 127))
			{
				if (!BIT(m_r[S1], D))
					exception(VEC9, true);
			}
			else
				exception(E_PRIVILEGE);
			break;
		case 0x36: // tb1: trap on bit set
			if ((m_cr[PSR] & PSR_MODE) || (VEC9 > 127))
			{
				if (BIT(m_r[S1], D))
					exception(VEC9, true);
			}
			else
				exception(E_PRIVILEGE);
			break;
		case 0x3a: // tcnd: conditional trap
			// TODO: synchronize
			if ((m_cr[PSR] & PSR_MODE) || (VEC9 > 127))
			{
				if (condition(D, m_r[S1]))
					exception(VEC9, true);
			}
			else
				exception(E_PRIVILEGE);
			break;
		default:
			exception(E_UNIMPLEMENTED);
			break;
		}
		break;
	case 0x3d: // nonfloating-point
		switch (BIT(inst, 5, 11))
		{
		case 0x000: // xmem.bu: exchange register with memory byte unsigned
			xmem<u8>(m_r[S1] + m_r[S2], D);
			break;
		case 0x008: // xmem.bu.usr: exchange register with memory byte unsigned user (privileged)
			if (m_cr[PSR] & PSR_MODE)
				xmem<u8,true>(m_r[S1] + m_r[S2], D);
			else
				exception(E_PRIVILEGE);
			break;
		case 0x020: // xmem: exchange register with memory word
			xmem<u32>(m_r[S1] + m_r[S2], D);
			break;
		case 0x028: // xmem.usr: exchange register with memory word user (privileged)
			if (m_cr[PSR] & PSR_MODE)
				xmem<u32,true>(m_r[S1] + m_r[S2], D);
			else
				exception(E_PRIVILEGE);
			break;
		case 0x010: // xmem.bu: exchange register with memory byte unsigned (scaled)
			xmem<u8>(m_r[S1] + (m_r[S2] << 0), D);
			break;
		case 0x018: // xmem.bu.usr: exchange register with memory byte unsigned user (scaled, privileged)
			if (m_cr[PSR] & PSR_MODE)
				xmem<u8,true>(m_r[S1] + (m_r[S2] << 0), D);
			else
				exception(E_PRIVILEGE);
			break;
		case 0x030: // xmem: exchange register with memory word (scaled)
			xmem<u32>(m_r[S1] + (m_r[S2] << 2), D);
			break;
		case 0x038: // xmem.usr: exchange register with memory word user (scaled, privileged)
			if (m_cr[PSR] & PSR_MODE)
				xmem<u32,true>(m_r[S1] + (m_r[S2] << 2), D);
			else
				exception(E_PRIVILEGE);
			break;

			// logical register
		case 0x200: // and: logical and (register)
			m_r[D] = m_r[S1] & m_r[S2];
			break;
		case 0x220: // and.c: logical not-and (register)
			m_r[D] = m_r[S1] & ~m_r[S2];
			break;
		case 0x280: // xor: logical exclusive or (register)
			m_r[D] = m_r[S1] ^ m_r[S2];
			break;
		case 0x2a0: // xor.c: logical not-exclusive or (register)
			m_r[D] = m_r[S1] ^ ~m_r[S2];
			break;
		case 0x2c0: // or: logical or (register)
			m_r[D] = m_r[S1] | m_r[S2];
			break;
		case 0x2e0: // or.c: logical not-or (register)
			m_r[D] = m_r[S1] | ~m_r[S2];
			break;

			// integer arithmetic register
		case 0x300: // addu: unsigned integer add (register)
			m_r[D] = m_r[S1] + m_r[S2];
			break;
		case 0x308: // addu.co: unsigned integer add with carry out (register)
			{
				u32 const data = m_r[S1] + m_r[S2];

				// compute carry out
				if (carry(m_r[S1], m_r[S2], data))
					m_cr[PSR] |= PSR_C;
				else
					m_cr[PSR] &= ~PSR_C;

				m_r[D] = data;
			}
			break;
		case 0x310: // addu.ci: unsigned integer add with carry in (register)
			m_r[D] = m_r[S1] + m_r[S2] + bool(m_cr[PSR] & PSR_C);
			break;
		case 0x318: // addu.cio: unsigned integer add with carry in and out (register)
			{
				u32 const data = m_r[S1] + m_r[S2] + bool(m_cr[PSR] & PSR_C);

				// compute carry out
				if (carry(m_r[S1], m_r[S2], data))
					m_cr[PSR] |= PSR_C;
				else
					m_cr[PSR] &= ~PSR_C;

				m_r[D] = data;
			}
			break;
		case 0x320: // subu: unsigned integer subtract (register)
			m_r[D] = m_r[S1] + ~m_r[S2] + 1;
			break;
		case 0x328: // subu.co: unsigned integer subtract with borrow out (register)
			{
				u32 const data = m_r[S1] + ~m_r[S2] + 1;

				// compute borrow out
				if (carry(m_r[S1], ~m_r[S2] + 1, data))
					m_cr[PSR] |= PSR_C;
				else
					m_cr[PSR] &= ~PSR_C;

				m_r[D] = data;
			}
			break;
		case 0x330: // subu.ci: unsigned integer subtract with borrow in (register)
			m_r[D] = m_r[S1] + ~m_r[S2] + !bool(m_cr[PSR] & PSR_C);
			break;
		case 0x338: // subu.cio: unsigned integer subtract with borrow in and out (register)
			{
				u32 const data = m_r[S1] + ~m_r[S2] + !bool(m_cr[PSR] & PSR_C);

				// compute borrow out
				if (carry(m_r[S1], ~m_r[S2] + !bool(m_cr[PSR] & PSR_C), data))
					m_cr[PSR] |= PSR_C;
				else
					m_cr[PSR] &= ~PSR_C;

				m_r[D] = data;
			}
			break;
		case 0x340: // divu: unsigned integer divide (register)
		case 0x348:
			if (!(m_cr[PSR] & PSR_SFD1))
			{
				if (m_r[S2])
					m_r[D] = m_r[S1] / m_r[S2];
				else
					exception(E_INT_DIVIDE);
			}
			else
				exception(E_SFU1_P);
			break;
		case 0x360: // mul: integer multiply (register)
		case 0x368:
			if (!(m_cr[PSR] & PSR_SFD1))
				m_r[D] = m_r[S1] * m_r[S2];
			else
				exception(E_SFU1_P);
			break;
		case 0x380: // add: integer add (register)
			{
				u32 const data = m_r[S1] + m_r[S2];

				if (!overflow(m_r[S1], m_r[S2], data))
					m_r[D] = data;
				else
					exception(E_INT_OVERFLOW);
			}
			break;
		case 0x388: // add.co: integer add with carry out (register)
			{
				u32 const data = m_r[S1] + m_r[S2];

				if (!overflow(m_r[S1], m_r[S2], data))
				{
					// compute carry out
					if (carry(m_r[S1], m_r[S2], data))
						m_cr[PSR] |= PSR_C;
					else
						m_cr[PSR] &= ~PSR_C;

					m_r[D] = data;
				}
				else
					exception(E_INT_OVERFLOW);
			}
			break;
		case 0x390: // add.ci: integer add with carry in (register)
			{
				u32 const data = m_r[S1] + m_r[S2] + bool(m_cr[PSR] & PSR_C);

				if (!overflow(m_r[S1], m_r[S2], data))
					m_r[D] = data;
				else
					exception(E_INT_OVERFLOW);
			}
			break;
		case 0x398: // add.cio: integer add with carry in and out (register)
			{
				u32 const data = m_r[S1] + m_r[S2] + bool(m_cr[PSR] & PSR_C);

				if (!overflow(m_r[S1], m_r[S2], data))
				{
					// compute carry out
					if (carry(m_r[S1], m_r[S2], data))
						m_cr[PSR] |= PSR_C;
					else
						m_cr[PSR] &= ~PSR_C;

					m_r[D] = data;
				}
				else
					exception(E_INT_OVERFLOW);
			}
			break;
		case 0x3a0: // sub: integer subtract (register)
			m_r[D] = m_r[S1] + ~m_r[S2] + 1;
			break;
		case 0x3a8: // sub.co: integer subtract with borrow out (register)
			{
				u32 const data = m_r[S1] + ~m_r[S2] + 1;

				// compute borrow out
				if (carry(m_r[S1], ~m_r[S2] + 1, data))
					m_cr[PSR] |= PSR_C;
				else
					m_cr[PSR] &= ~PSR_C;

				m_r[D] = data;
			}
			break;
		case 0x3b0: // sub.ci: integer subtract with borrow in (register)
			m_r[D] = m_r[S1] + ~m_r[S2] + !bool(m_cr[PSR] & PSR_C);
			break;
		case 0x3b8: // sub.cio: integer subtract with borrow in and out (register)
			{
				u32 const data = m_r[S1] + ~m_r[S2] + !bool(m_cr[PSR] & PSR_C);

				// compute borrow out
				if (carry(m_r[S1], ~m_r[S2] + !bool(m_cr[PSR] & PSR_C), data))
					m_cr[PSR] |= PSR_C;
				else
					m_cr[PSR] &= ~PSR_C;

				m_r[D] = data;
			}
			break;
		case 0x3c0: // div: signed integer divide (register)
		case 0x3c8:
			if (!(m_cr[PSR] & PSR_SFD1))
			{
				if (m_r[S2] && !BIT(m_r[S1], 31) && !BIT(m_r[S2], 31))
					m_r[D] = s32(m_r[S1]) / s32(m_r[S2]);
				else
					exception(E_INT_DIVIDE);
			}
			else
				exception(E_SFU1_P);
			break;
		case 0x3e0: // cmp: integer compare (register)
		case 0x3e8:
			m_r[D] = cmp(m_r[S1], m_r[S2]);
			break;

			// bit field register
		case 0x400: // clr: clear bit field (register)
			{
				unsigned const width = BIT(m_r[S2], 5, 5);
				unsigned const offset = m_r[S2] & 31;

				m_r[D] = m_r[S1] & ~(make_bitmask<u32>(width ? width : 32) << offset);
			}
			break;
		case 0x440: // set: set bit field (register)
			{
				unsigned const width = BIT(m_r[S2], 5, 5);
				unsigned const offset = m_r[S2] & 31;

				m_r[D] = m_r[S1] | (make_bitmask<u32>(width ? width : 32) << offset);
			}
			break;
		case 0x480: // ext: extract signed bit field (register)
			{
				unsigned const width = BIT(m_r[S2], 5, 5);
				unsigned const offset = m_r[S2] & 31;

				if (width && (width + offset) < 32)
					m_r[D] = util::sext(m_r[S1] >> offset, width);
				else
					m_r[D] = s32(m_r[S1]) >> offset;
			}
			break;
		case 0x4c0: // extu: extract unsigned bit field (register)
			{
				unsigned const width = BIT(m_r[S2], 5, 5);
				unsigned const offset = m_r[S2] & 31;

				if (width)
					m_r[D] = BIT(m_r[S1], offset, width);
				else
					m_r[D] = m_r[S1] >> offset;
			}
			break;
		case 0x500: // mak: make bit field (register)
			{
				unsigned const width = BIT(m_r[S2], 5, 5);
				unsigned const offset = m_r[S2] & 31;

				if (width)
					m_r[D] = (m_r[S1] & make_bitmask<u32>(width)) << offset;
				else
					m_r[D] = m_r[S1] << offset;
			}
			break;
		case 0x540: // rot: rotate (register)
			m_r[D] = rotr_32(m_r[S1], m_r[S2]);
			break;
		case 0x740: // ff1: find first bit set
			{
				unsigned const count = count_leading_zeros_32(m_r[S2]);

				m_r[D] = (count == 32) ? count : 31 - count;
			}
			break;
		case 0x760: // ff0: find first bit clear
			{
				unsigned const count = count_leading_ones_32(m_r[S2]);

				m_r[D] = (count == 32) ? count : 31 - count;
			}
			break;

		case 0x600: // jmp: unconditional jump
			m_fip = (m_r[S2] & IP_A) | IP_V;
			m_nip &= ~IP_V;
			break;
		case 0x620: // jmp.n: unconditional jump (delayed)
			m_fip = (m_r[S2] & IP_A) | IP_V;
			break;
		case 0x640: // jsr: unconditional jump to subroutine
			m_fip = (m_r[S2] & IP_A) | IP_V;
			m_r[1] = m_nip & IP_A;
			m_nip &= ~IP_V;
			break;
		case 0x660: // jsr.n: unconditional jump to subroutine (delayed)
			m_fip = (m_r[S2] & IP_A) | IP_V;
			m_r[1] = (m_nip & IP_A) + 4;
			break;
		case 0x7c0: // tbnd: trap on bounds check (register)
			if (m_r[S1] > m_r[S2])
				exception(E_BOUNDS, true);
			break;
		case 0x7e0: // rte: return from exception (privileged)
			if (m_cr[PSR] & PSR_MODE)
			{
				m_cr[PSR] = m_cr[EPSR];

				m_sb = m_cr[SSBR];
				m_nip = m_cr[SNIP];
				m_fip = m_cr[SFIP];

				if (m_nip & IP_V)
					fetch(m_nip, m_nop);

				if (!(m_cr[EPSR] & PSR_MODE))
					debugger_privilege_hook();
			}
			else
				exception(E_PRIVILEGE);
			break;

		case 0x040: // ld.hu: load half word unsigned
			ld<u16>(m_r[S1] + m_r[S2], D);
			break;
		case 0x048: // ld.hu.usr: load half word unsigned user (privileged)
			if (m_cr[PSR] & PSR_MODE)
				ld<u16,true>(m_r[S1] + m_r[S2], D);
			else
				exception(E_PRIVILEGE);
			break;
		case 0x060: // ld.b: load byte unsigned
			ld<u8>(m_r[S1] + m_r[S2], D);
			break;
		case 0x068: // ld.b.usr: load byte unsigned user (privileged)
			if (m_cr[PSR] & PSR_MODE)
				ld<u8,true>(m_r[S1] + m_r[S2], D);
			else
				exception(E_PRIVILEGE);
			break;
		case 0x080: // ld.d: load double word
			ld<u64>(m_r[S1] + m_r[S2], D);
			break;
		case 0x088: // ld.d.usr: load double word user (privileged)
			if (m_cr[PSR] & PSR_MODE)
				ld<u64,true>(m_r[S1] + m_r[S2], D);
			else
				exception(E_PRIVILEGE);
			break;
		case 0x0a0: // ld: load word
			ld<u32>(m_r[S1] + m_r[S2], D);
			break;
		case 0x0a8: // ld.usr: load word user (privileged)
			if (m_cr[PSR] & PSR_MODE)
				ld<u32,true>(m_r[S1] + m_r[S2], D);
			else
				exception(E_PRIVILEGE);
			break;
		case 0x0c0: // ld.h: load half word
			ld<s16>(m_r[S1] + m_r[S2], D);
			break;
		case 0x0c8: // ld.h.usr: load half word user (privileged)
			if (m_cr[PSR] & PSR_MODE)
				ld<s16,true>(m_r[S1] + m_r[S2], D);
			else
				exception(E_PRIVILEGE);
			break;
		case 0x0e0: // ld.b: load byte
			ld<s8>(m_r[S1] + m_r[S2], D);
			break;
		case 0x0e8: // ld.b.usr: load byte user (privileged)
			if (m_cr[PSR] & PSR_MODE)
				ld<s8,true>(m_r[S1] + m_r[S2], D);
			else
				exception(E_PRIVILEGE);
			break;

		case 0x050: // ld.hu: load half word unsigned (scaled)
			ld<u16>(m_r[S1] + (m_r[S2] << 1), D);
			break;
		case 0x058: // ld.hu.usr: load half word unsigned user (scaled, privileged)
			if (m_cr[PSR] & PSR_MODE)
				ld<u16,true>(m_r[S1] + (m_r[S2] << 1), D);
			else
				exception(E_PRIVILEGE);
			break;
		case 0x070: // ld.b: load byte unsigned (scaled)
			ld<u8>(m_r[S1] + (m_r[S2] << 0), D);
			break;
		case 0x078: // ld.b.usr: load byte unsigned user (scaled, privileged)
			if (m_cr[PSR] & PSR_MODE)
				ld<u8,true>(m_r[S1] + (m_r[S2] << 0), D);
			else
				exception(E_PRIVILEGE);
			break;
		case 0x090: // ld.d: load double word (scaled)
			ld<u64>(m_r[S1] + (m_r[S2] << 3), D);
			break;
		case 0x098: // ld.d.usr: load double word user (scaled, privileged)
			if (m_cr[PSR] & PSR_MODE)
				ld<u64,true>(m_r[S1] + (m_r[S2] << 3), D);
			else
				exception(E_PRIVILEGE);
			break;
		case 0x0b0: // ld: load word (scaled)
			ld<u32>(m_r[S1] + (m_r[S2] << 2), D);
			break;
		case 0x0b8: // ld.usr: load word user (scaled, privileged)
			if (m_cr[PSR] & PSR_MODE)
				ld<u32,true>(m_r[S1] + (m_r[S2] << 2),D);
			else
				exception(E_PRIVILEGE);
			break;
		case 0x0d0: // ld.h: load half word (scaled)
			ld<s16>(m_r[S1] + (m_r[S2] << 1), D);
			break;
		case 0x0d8: // ld.h.usr: load half word user (scaled, privileged)
			if (m_cr[PSR] & PSR_MODE)
				ld<s16,true>(m_r[S1] + (m_r[S2] << 1), D);
			else
				exception(E_PRIVILEGE);
			break;
		case 0x0f0: // ld.b: load byte (scaled)
			ld<s8>(m_r[S1] + (m_r[S2] << 0), D);
			break;
		case 0x0f8: // ld.b.usr: load byte user (scaled, privileged)
			if (m_cr[PSR] & PSR_MODE)
				ld<s8,true>(m_r[S1] + (m_r[S2] << 0), D);
			else
				exception(E_PRIVILEGE);
			break;

		case 0x100: // st.d: store double word
			st<u64>(m_r[S1] + m_r[S2], D);
			break;
		case 0x108: // st.d.usr: store double word user (privileged)
			if (m_cr[PSR] & PSR_MODE)
				st<u64,true>(m_r[S1] + m_r[S2], D);
			else
				exception(E_PRIVILEGE);
			break;
		case 0x120: // st: store word
			st<u32>(m_r[S1] + m_r[S2], D);
			break;
		case 0x128: // st.usr: store word user (privileged)
			if (m_cr[PSR] & PSR_MODE)
				st<u32,true>(m_r[S1] + m_r[S2], D);
			else
				exception(E_PRIVILEGE);
			break;
		case 0x140: // st.h: store half word
			st<u16>(m_r[S1] + m_r[S2], D);
			break;
		case 0x148: // st.h.usr: store half word user (privileged)
			if (m_cr[PSR] & PSR_MODE)
				st<u16,true>(m_r[S1] + m_r[S2], D);
			else
				exception(E_PRIVILEGE);
			break;
		case 0x160: // st.b: store byte
			st<u8>(m_r[S1] + m_r[S2], D);
			break;
		case 0x168: // st.b.usr: store byte user (privileged)
			if (m_cr[PSR] & PSR_MODE)
				st<u8,true>(m_r[S1] + m_r[S2], D);
			else
				exception(E_PRIVILEGE);
			break;

		case 0x110: // st.d: store double word (scaled)
			st<u64>(m_r[S1] + (m_r[S2] << 3), D);
			break;
		case 0x118: // st.d.usr: store double word user (scaled, privileged)
			if (m_cr[PSR] & PSR_MODE)
				st<u64,true>(m_r[S1] + (m_r[S2] << 3), D);
			else
				exception(E_PRIVILEGE);
			break;
		case 0x130: // st: store word (scaled)
			st<u32>(m_r[S1] + (m_r[S2] << 2), D);
			break;
		case 0x138: // st.usr: store word user (scaled, privileged)
			if (m_cr[PSR] & PSR_MODE)
				st<u32,true>(m_r[S1] + (m_r[S2] << 2), D);
			else
				exception(E_PRIVILEGE);
			break;
		case 0x150: // st.h: store half word (scaled)
			st<u16>(m_r[S1] + (m_r[S2] << 1), D);
			break;
		case 0x158: // st.h.usr: store half word user (scaled, privileged)
			if (m_cr[PSR] & PSR_MODE)
				st<u16,true>(m_r[S1] + (m_r[S2] << 1), D);
			else
				exception(E_PRIVILEGE);
			break;
		case 0x170: // st.b: store byte (scaled)
			st<u8>(m_r[S1] + (m_r[S2] << 0), D);
			break;
		case 0x178: // st.b.usr: store byte user (scaled, privileged)
			if (m_cr[PSR] & PSR_MODE)
				st<u8,true>(m_r[S1] + (m_r[S2] << 0), D);
			else
				exception(E_PRIVILEGE);
			break;

		case 0x180: // lda.d: load address double word
		case 0x1a0: // lda: load address word
		case 0x1c0: // lda.h: load address half word
		case 0x1e0: // lda.b: load address byte
			m_r[D] = m_r[S1] + m_r[S2];
			break;
		case 0x190: // lda.d: load address double word (scaled)
			m_r[D] = m_r[S1] + (m_r[S2] << 3);
			break;
		case 0x1b0: // lda: load address word (scaled)
			m_r[D] = m_r[S1] + (m_r[S2] << 2);
			break;
		case 0x1d0: // lda.h: load address half word (scaled)
			m_r[D] = m_r[S1] + (m_r[S2] << 1);
			break;
		case 0x1f0: // lda.b: load address byte (scaled)
			m_r[D] = m_r[S1] + (m_r[S2] << 0);
			break;
		default:
			exception(E_UNIMPLEMENTED);
			break;
		}
		break;
	case 0x3e: // tbnd: trap on bounds check (immediate)
		if (m_r[S1] > IMM16)
			exception(E_BOUNDS, true);
		break;
	case 0x3f:
		exception(E_UNIMPLEMENTED);
		break;
	}

	m_r[0] = 0;
}

void mc88100_device::execute_set_input(int inputnum, int state)
{
	if (inputnum == INPUT_LINE_IRQ0)
		m_int_state = bool(state);
}

void mc88100_device::set_cr(unsigned const cr, u32 const data)
{
	switch (cr)
	{
	case PID:
	case SXIP:
		// read-only
		break;

	case PSR:
		if (data & PSR_SFRZ)
		{
			m_cr[SSBR] = m_sb;
			m_cr[SXIP] = m_xip;
			m_cr[SNIP] = m_nip;
			m_cr[SFIP] = m_fip;
		}

		if (!(data & PSR_MODE))
			debugger_privilege_hook();

		[[fallthrough]];
	case EPSR:
		if (data & PSR_BO)
			fatalerror("mc88100: little-endian mode not emulated (%s)\n", machine().describe_context());
		m_cr[cr] = (m_cr[cr] & ~PSR_MASK) | (data & PSR_MASK);
		break;

	case VBR:
		m_cr[cr] = data & VBR_MASK;
		break;

	case SSBR: case SNIP: case SFIP:
	case DMT0: case DMD0: case DMA0:
	case DMT1: case DMD1: case DMA1:
	case DMT2: case DMD2: case DMA2:
	case SR0: case SR1: case SR2: case SR3:
		m_cr[cr] = data;
		break;

	default:
		// unknown register
		logerror("set_cr unknown register %d data 0x%08x xip 0x%08x\n", cr, data, m_xip);
		break;
	}
}

void mc88100_device::set_fcr(unsigned const fcr, u32 const data)
{
	switch (fcr)
	{
	case FPECR:
		m_fcr[fcr] = data & FPECR_MASK;
		break;

	case FPHS1: case FPLS1:
	case FPHS2: case FPLS2:
	case FPPT:
	case FPRH: case FPRL:
	case FPIT:
		// read-only
		break;

	case FPSR:
		m_fcr[fcr] = data & FPSR_MASK;
		break;

	case FPCR:
		switch (BIT(data, 14, 2))
		{
		case 0: softfloat_roundingMode = softfloat_round_near_even; break;
		case 1: softfloat_roundingMode = softfloat_round_minMag; break;
		case 2: softfloat_roundingMode = softfloat_round_min; break;
		case 3: softfloat_roundingMode = softfloat_round_max; break;
		}

		m_fcr[fcr] = data & FPCR_MASK;
		break;

	default:
		// unknown register
		logerror("set_fcr unknown register %d data 0x%08x xip 0x%08x\n", fcr, data, m_xip);
		break;
	}
}

void mc88100_device::exception(unsigned vector, bool const trap)
{
	LOGMASKED(LOG_EXCEPTION, "exception %u xip 0x%08x\n", vector, m_xip & IP_A);

	if (!(m_cr[PSR] & PSR_SFRZ))
	{
		m_cr[SSBR] = m_sb;
		m_cr[SXIP] = m_xip;
		m_cr[SNIP] = m_nip;
		m_cr[SFIP] = m_fip;

		m_cr[EPSR] = m_cr[PSR];
	}
	else if (!trap)
		vector = E_ERROR;

	bool const supervisor = m_cr[PSR] & PSR_MODE;
	m_cr[PSR] |= PSR_MODE | PSR_SFD1 | PSR_IND | PSR_SFRZ;
	m_sb = 0;

	if (vector != E_DATA)
	{
		m_cr[DMT0] = 0;
		m_cr[DMT1] = 0;
		m_cr[DMT2] = 0;
	}

	// invalidate execution and next instruction pointers
	m_xip &= ~IP_V;
	m_nip &= ~IP_V;

	// update fetch instruction pointer
	m_fip = m_cr[VBR] | (vector << 3) | IP_V;

	// notify debugger
	if (machine().debug_flags & DEBUG_FLAG_ENABLED)
	{
		if (vector == E_INTERRUPT)
			debug()->interrupt_hook(INPUT_LINE_IRQ0, m_xip & IP_A);
		else
			debug()->exception_hook(vector);

		if (!supervisor)
			debug()->privilege_hook();
	}
}

bool mc88100_device::condition(unsigned const m5, u32 const src) const
{
	bool const sign = BIT(src, 31);
	bool const zero = !BIT(src, 0, 31);

	return BIT(m5, sign * 2 + zero);
}

enum cmp_mask : u32
{
	CMP_EQ = 0x0000'0004, // equal
	CMP_NE = 0x0000'0008, // not equal
	CMP_GT = 0x0000'0010, // signed greater than
	CMP_LE = 0x0000'0020, // signed less than or equal
	CMP_LT = 0x0000'0040, // signed less than
	CMP_GE = 0x0000'0080, // signed greater than or equal
	CMP_HI = 0x0000'0100, // unsigned greater than
	CMP_LS = 0x0000'0200, // unsigned less than or equal
	CMP_LO = 0x0000'0400, // unsigned less than
	CMP_HS = 0x0000'0800, // unsigned greater than or equal
};

u32 mc88100_device::cmp(u32 const src1, u32 const src2) const
{
	u32 result = (CMP_HS | CMP_LS | CMP_GE | CMP_LE | CMP_EQ);

	if (src1 != src2)
	{
		result = CMP_NE;

		if (src1 > src2)
			result |= (CMP_HS | CMP_HI);
		else
			result |= (CMP_LO | CMP_LS);

		if (s32(src1) > s32(src2))
			result |= (CMP_GE | CMP_GT);
		else
			result |= (CMP_LT | CMP_LE);
	}

	return result;
}

bool mc88100_device::carry(u32 const src1, u32 const src2, u32 const dest) const
{
	return BIT((src1 & src2) ^ ((src1 ^ src2) & ~dest), 31);
}

bool mc88100_device::overflow(u32 const src1, u32 const src2, u32 const dest) const
{
	return (BIT(src2, 31) == BIT(src1, 31)) && (BIT(dest, 31) != BIT(src2, 31));
}

enum fcmp_mask : u32
{
	FCMP_NC = 0x0000'0001, // not comparable
	FCMP_CP = 0x0000'0002, // comparable
	FCMP_EQ = 0x0000'0004, // equal
	FCMP_NE = 0x0000'0008, // not equal
	FCMP_GT = 0x0000'0010, // signed greater than
	FCMP_LE = 0x0000'0020, // signed less than or equal
	FCMP_LT = 0x0000'0040, // signed less than
	FCMP_GE = 0x0000'0080, // signed greater than or equal
	FCMP_OU = 0x0000'0100, // out of range
	FCMP_IB = 0x0000'0200, // in range or on boundary
	FCMP_IN = 0x0000'0400, // in range
	FCMP_OB = 0x0000'0800, // out of range or on boundary
};

u32 mc88100_device::fcmp(float64_t const src1, float64_t const src2)
{
	u32 result = FCMP_GE | FCMP_LE | FCMP_EQ | FCMP_CP;

	// TODO: reserved operands exception

	if (!f64_eq(src1, src2))
	{
		result = FCMP_NE | FCMP_CP;

		if (f64_lt(src1, src2))
			result |= (FCMP_LT | FCMP_LE);
		else
			result |= (FCMP_GE | FCMP_GT);
	}

	if (!BIT(src2.v, 63) && (result & FCMP_CP))
	{
		if (src1.v && (result & FCMP_NE))
		{
			if (BIT(src1.v, 63) || (result & FCMP_GT))
				result |= FCMP_OU | FCMP_OB;
			else if (!BIT(src1.v, 63) && (result & FCMP_LT))
				result |= FCMP_IN | FCMP_IB;
		}
		else
			result |= FCMP_OB | FCMP_IB;
	}

	return result;
}

void mc88100_device::fset(unsigned const td, unsigned const d, float64_t const data)
{
	switch (td)
	{
	case 0:
		m_r[d] = f64_to_f32(data).v;
		break;

	case 1:
		m_r[(d + 0) & 31] = u32(data.v >> 32);
		m_r[(d + 1) & 31] = u32(data.v >> 0);
		break;
	}
}

void mc88100_device::fetch(u32 &address, u32 &inst)
{
	if (m_cmmu_code)
	{
		std::optional<u32> data = m_cmmu_code(address & IP_A).read<u32>(address & IP_A, m_cr[PSR] & PSR_MODE);
		if (data.has_value())
			inst = data.value();
		else
			address |= IP_E;
	}
	else
		inst = m_code_space.read_dword(address & IP_A);
}

template <typename T, bool Usr> void mc88100_device::ld(u32 address, unsigned const reg)
{
	// alignment check
	if (address & (sizeof(T) - 1))
	{
		if (!(m_cr[PSR] & PSR_MXM))
		{
			exception(E_MISALIGNED);

			return;
		}
		else
			address &= ~(sizeof(T) - 1);
	}

	if (m_cmmu_data)
	{
		if constexpr (sizeof(T) < 8)
		{
			std::optional<T> const data = m_cmmu_data(address).read<typename std::make_unsigned<T>::type>(address, (m_cr[PSR] & PSR_MODE) && !Usr);

			if (data.has_value() && reg)
				m_r[reg] = std::is_signed<T>() ? s32(data.value()) : data.value();
			else if (!data.has_value())
			{
				m_cr[DMT0] = (reg << 7) | DMT_EN<T>(address) | DMT_VALID;
				m_cr[DMT1] = 0;
				m_cr[DMT2] = 0;
				if (std::is_signed<T>())
					m_cr[DMT0] |= DMT_SD;
				if ((m_cr[PSR] & PSR_MODE) && !Usr)
					m_cr[DMT0] |= DMT_DAS;
				if ((m_cr[PSR] & PSR_BO))
					m_cr[DMT0] |= DMT_BO;

				m_cr[DMA0] = address & ~3;
				m_cr[DMA1] = 0;
				m_cr[DMA2] = 0;

				m_cr[DMD0] = 0;
				m_cr[DMD1] = 0;
				m_cr[DMD2] = 0;

				exception(E_DATA);
			}
		}
		else
		{
			std::optional<u32> const hi = m_cmmu_data(address + 0).read<u32>(address + 0, (m_cr[PSR] & PSR_MODE) && !Usr);
			std::optional<u32> const lo = m_cmmu_data(address + 4).read<u32>(address + 4, (m_cr[PSR] & PSR_MODE) && !Usr);
			if (lo.has_value() && hi.has_value())
			{
				if (reg != 0)
					m_r[(reg + 0) & 31] = hi.value();
				if (reg != 31)
					m_r[(reg + 1) & 31] = lo.value();
			}
			else
			{
				m_cr[DMT0] = DMT_DOUB1 | (((reg + 0) & 31) << 7) | DMT_EN() | DMT_VALID;
				m_cr[DMT1] = (((reg + 1) & 31) << 7) | DMT_EN() | DMT_VALID;
				m_cr[DMT2] = 0;
				if ((m_cr[PSR] & PSR_MODE) && !Usr)
				{
					m_cr[DMT0] |= DMT_DAS;
					m_cr[DMT1] |= DMT_DAS;
				}
				if ((m_cr[PSR] & PSR_BO))
				{
					m_cr[DMT0] |= DMT_BO;
					m_cr[DMT1] |= DMT_BO;
				}

				m_cr[DMA0] = address + 0;
				m_cr[DMA1] = address + 4;
				m_cr[DMA2] = 0;

				m_cr[DMD0] = 0;
				m_cr[DMD1] = 0;
				m_cr[DMD2] = 0;

				exception(E_DATA);
			}
		}
	}
	else
	{
		if constexpr (sizeof(T) == 1)
		{
			u32 const data = std::is_signed<T>() ? s32(T(m_data_space.read_byte(address))) : m_data_space.read_byte(address);

			if (reg)
				m_r[reg] = data;
		}
		else if constexpr (sizeof(T) == 2)
		{
			u32 const data = std::is_signed<T>() ? s32(T(m_data_space.read_word(address))) : m_data_space.read_word(address);

			if (reg)
				m_r[reg] = data;
		}
		else if constexpr (sizeof(T) == 4)
		{
			u32 const data = m_data_space.read_dword(address);

			if (reg)
				m_r[reg] = data;
		}
		else if constexpr (sizeof(T) == 8)
		{
			u32 const hi = m_data_space.read_dword(address + 0);
			u32 const lo = m_data_space.read_dword(address + 4);

			if (reg != 0)
				m_r[(reg + 0) & 31] = hi;
			if (reg != 31)
				m_r[(reg + 1) & 31] = lo;
		}
	}
}

template <typename T, bool Usr> void mc88100_device::st(u32 address, unsigned const reg)
{
	// alignment check
	if (address & (sizeof(T) - 1))
	{
		if (!(m_cr[PSR] & PSR_MXM))
		{
			exception(E_MISALIGNED);

			return;
		}
		else
			address &= ~(sizeof(T) - 1);
	}

	if (m_cmmu_data)
	{
		if constexpr (sizeof(T) < 8)
		{
			if (!m_cmmu_data(address).write(address, T(m_r[reg]), (m_cr[PSR] & PSR_MODE) && !Usr))
			{
				m_cr[DMT0] = DMT_EN<T>(address) | DMT_WRITE | DMT_VALID;
				m_cr[DMT1] = 0;
				m_cr[DMT2] = 0;
				if ((m_cr[PSR] & PSR_MODE) && !Usr)
					m_cr[DMT0] |= DMT_DAS;
				if ((m_cr[PSR] & PSR_BO))
					m_cr[DMT0] |= DMT_BO;

				m_cr[DMA0] = address & ~3;
				m_cr[DMA1] = 0;
				m_cr[DMA2] = 0;

				m_cr[DMD0] = T(m_r[reg]);
				m_cr[DMD1] = 0;
				m_cr[DMD2] = 0;

				exception(E_DATA);
			}
		}
		else
		{
			bool result = true;
			result &= m_cmmu_data(address + 0).write(address + 0, m_r[(reg + 0) & 31], (m_cr[PSR] & PSR_MODE) && !Usr);
			result &= m_cmmu_data(address + 4).write(address + 4, m_r[(reg + 1) & 31], (m_cr[PSR] & PSR_MODE) && !Usr);

			if (!result)
			{
				m_cr[DMT0] = DMT_DOUB1 | DMT_EN() | DMT_WRITE | DMT_VALID;
				m_cr[DMT1] = DMT_EN() | DMT_WRITE | DMT_VALID;
				m_cr[DMT2] = 0;
				if ((m_cr[PSR] & PSR_MODE) && !Usr)
				{
					m_cr[DMT0] |= DMT_DAS;
					m_cr[DMT1] |= DMT_DAS;
				}
				if ((m_cr[PSR] & PSR_BO))
				{
					m_cr[DMT0] |= DMT_BO;
					m_cr[DMT1] |= DMT_BO;
				}

				m_cr[DMA0] = address + 0;
				m_cr[DMA1] = address + 4;
				m_cr[DMA2] = 0;

				m_cr[DMD0] = m_r[(reg + 0) & 31];
				m_cr[DMD1] = m_r[(reg + 1) & 31];
				m_cr[DMD2] = 0;

				exception(E_DATA);
			}
		}
	}
	else
	{
		if constexpr (sizeof(T) == 1)
			m_data_space.write_byte(address, m_r[reg]);
		else if constexpr (sizeof(T) == 2)
			m_data_space.write_word(address, m_r[reg]);
		else if constexpr (sizeof(T) == 4)
			m_data_space.write_dword(address, m_r[reg]);
		else if constexpr (sizeof(T) == 8)
		{
			m_data_space.write_dword(address + 0, m_r[(reg + 0) & 31]);
			m_data_space.write_dword(address + 4, m_r[(reg + 1) & 31]);
		}
	}
}

template <typename T, bool Usr> void mc88100_device::xmem(u32 address, unsigned const reg)
{
	// alignment check
	if (address & (sizeof(T) - 1))
	{
		if (!(m_cr[PSR] & PSR_MXM))
			exception(E_MISALIGNED);
		else
			address &= ~(sizeof(T) - 1);
	}

	// save source value
	T const src = m_r[reg];

	if (m_cmmu_data)
	{
		// read destination
		std::optional<T> const dst = m_cmmu_data(address).read<T>(address, (m_cr[PSR] & PSR_MODE) && !Usr);
		if (dst.has_value())
		{
			// update register
			if (reg)
				m_r[reg] = dst.value();

			// write destination
			if (m_cmmu_data(address).write<T>(address, src, (m_cr[PSR] & PSR_MODE) && !Usr))
				return;
		}

		m_cr[DMT0] = DMT_DOUB1 | DMT_LOCK | (reg << 7) | DMT_EN<T>(address) | DMT_VALID;
		m_cr[DMT1] = DMT_LOCK | DMT_EN<T>(address) | DMT_WRITE | DMT_VALID;
		m_cr[DMT2] = 0;
		if ((m_cr[PSR] & PSR_MODE) && !Usr)
		{
			m_cr[DMT0] |= DMT_DAS;
			m_cr[DMT1] |= DMT_DAS;
		}
		if ((m_cr[PSR] & PSR_BO))
		{
			m_cr[DMT0] |= DMT_BO;
			m_cr[DMT1] |= DMT_BO;
		}

		m_cr[DMA0] = address & ~3;
		m_cr[DMA1] = address & ~3;
		m_cr[DMA2] = 0;

		m_cr[DMD0] = 0;
		m_cr[DMD1] = src;
		m_cr[DMD2] = 0;

		exception(E_DATA);
	}
	else
	{
		if constexpr (sizeof(T) == 1)
		{
			// read destination
			T const dst = m_data_space.read_byte(address);

			// update register
			if (reg)
				m_r[reg] = dst;

			// write destination
			m_data_space.write_byte(address, src);
		}
		else if constexpr (sizeof(T) == 4)
		{
			// read destination
			T const dst = m_data_space.read_dword(address);

			// update register
			if (reg)
				m_r[reg] = dst;

			// write destination
			m_data_space.write_dword(address, src);
		}
	}
}
