// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*****************************************************************************
 *
 *   sh4.c
 *   Portable Hitachi SH-4 (SH7750 family) emulator
 *
 *   By R. Belmont, based on sh2.c by Juergen Buchmueller, Mariusz Wojcieszek,
 *      Olivier Galibert, Sylvain Glaize, and James Forshaw.
 *
 *
 *   TODO: FPU
 *         DMA
 *         on-board peripherals
 *
 *   DONE: boot/reset setup
 *         64-bit data bus
 *         banked registers
 *         additional registers for supervisor mode
 *         FPU status and data registers
 *         state save for the new registers
 *         interrupts
 *         store queues
 *
 *****************************************************************************/

#include "emu.h"
#include "sh4.h"
#include "sh4regs.h"
#include "sh4comn.h"
#include "sh3comn.h"
#include "sh4tmu.h"
#include "sh_dasm.h"
#include "cpu/drcumlsh.h"
#include "debugger.h"


DEFINE_DEVICE_TYPE(SH3LE, sh3_device,   "sh3le", "Hitachi SH-3 (little)")
DEFINE_DEVICE_TYPE(SH3BE, sh3be_device, "sh3be", "Hitachi SH-3 (big)")
DEFINE_DEVICE_TYPE(SH4LE, sh4_device,   "sh4le", "Hitachi SH-4 (little)")
DEFINE_DEVICE_TYPE(SH4BE, sh4be_device, "sh4be", "Hitachi SH-4 (big)")


#if 0
/*When OC index mode is off (CCR.OIX = 0)*/
void sh4_base_device::sh4_internal_map(address_map &map)
{
	map(0x1C000000, 0x1C000FFF).ram().mirror(0x03FFD000);
	map(0x1C002000, 0x1C002FFF).ram().mirror(0x03FFD000);
	map(0xE0000000, 0xE000003F).ram().mirror(0x03FFFFC0);
}
#endif

/*When OC index mode is on (CCR.OIX = 1)*/
void sh4_base_device::sh4_internal_map(address_map &map)
{
	map(0x1C000000, 0x1C000FFF).ram().mirror(0x01FFF000);
	map(0x1E000000, 0x1E000FFF).ram().mirror(0x01FFF000);
	map(0xE0000000, 0xE000003F).ram().mirror(0x03FFFFC0); // todo: store queues should be write only on DC's SH4, executing PREFM shouldn't cause an actual memory read access!

	map(0xF6000000, 0xF6FFFFFF).rw(FUNC(sh4_base_device::sh4_utlb_address_array_r), FUNC(sh4_base_device::sh4_utlb_address_array_w));
	map(0xF7000000, 0xF77FFFFF).rw(FUNC(sh4_base_device::sh4_utlb_data_array1_r), FUNC(sh4_base_device::sh4_utlb_data_array1_w));
	map(0xF7800000, 0xF7FFFFFF).rw(FUNC(sh4_base_device::sh4_utlb_data_array2_r), FUNC(sh4_base_device::sh4_utlb_data_array2_w));

	map(0xFE000000, 0xFFFFFFFF).rw(FUNC(sh4_base_device::sh4_internal_r), FUNC(sh4_base_device::sh4_internal_w)).umask32(0xffffffff);
}

void sh3_base_device::sh3_internal_map(address_map &map)
{
	map(SH3_LOWER_REGBASE, SH3_LOWER_REGEND).rw(FUNC(sh3_base_device::sh3_internal_r), FUNC(sh3_base_device::sh3_internal_w));
	map(SH3_UPPER_REGBASE, SH3_UPPER_REGEND).rw(FUNC(sh3_base_device::sh3_internal_high_r), FUNC(sh3_base_device::sh3_internal_high_w));
}


sh34_base_device::sh34_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, endianness_t endianness, address_map_constructor internal)
	: sh_common_execution(mconfig, type, tag, owner, clock, endianness, internal)
	, m_program_config("program", endianness, 64, 32, 0, internal)
	, m_io_config("io", endianness, 64, 8)
	, m_clock(0)
	, m_mmuhack(1)
	, m_bigendian(endianness == ENDIANNESS_BIG)
{
}

device_memory_interface::space_config_vector sh34_base_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_IO,      &m_io_config)
	};
}


sh3_base_device::sh3_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, endianness_t endianness)
	: sh34_base_device(mconfig, type, tag, owner, clock, endianness, address_map_constructor(FUNC(sh3_base_device::sh3_internal_map), this))
{
	m_cpu_type = CPU_TYPE_SH3;
	m_am = SH34_AM;
}


sh4_base_device::sh4_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, endianness_t endianness)
	: sh34_base_device(mconfig, type, tag, owner, clock, endianness, address_map_constructor(FUNC(sh4_base_device::sh4_internal_map), this))
{
	m_cpu_type = CPU_TYPE_SH4;
	m_am = SH34_AM;
}


sh3_device::sh3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sh3_base_device(mconfig, SH3LE, tag, owner, clock, ENDIANNESS_LITTLE)
{
}


sh3be_device::sh3be_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sh3_base_device(mconfig, SH3BE, tag, owner, clock, ENDIANNESS_BIG)
{
}


sh4_device::sh4_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sh4_base_device(mconfig, SH4LE, tag, owner, clock, ENDIANNESS_LITTLE)
{
}


sh4be_device::sh4be_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sh4_base_device(mconfig, SH4BE, tag, owner, clock, ENDIANNESS_BIG)
{
}


std::unique_ptr<util::disasm_interface> sh34_base_device::create_disassembler()
{
	return std::make_unique<sh_disassembler>(true);
}


/* Called for unimplemented opcodes */
void sh34_base_device::TODO(const uint16_t opcode)
{
}

void sh34_base_device::LDTLB(const uint16_t opcode)
{
	logerror("unhandled LDTLB for this CPU type\n");
}

void sh4_base_device::LDTLB(const uint16_t opcode)
{
	int replace = (m_m[MMUCR] & 0x0000fc00) >> 10;

	logerror("using LDTLB to replace UTLB entry %02x\n", replace);

	// these come from PTEH
	m_utlb[replace].VPN = (m_m[PTEH] & 0xfffffc00) >> 10;
	//  m_utlb[replace].D =    (m_m[PTEH] & 0x00000200) >> 9; // from PTEL
	//  m_utlb[replace].V =    (m_m[PTEH] & 0x00000100) >> 8; // from PTEL
	m_utlb[replace].ASID = (m_m[PTEH] & 0x000000ff) >> 0;
	// these come from PTEL
	m_utlb[replace].PPN = (m_m[PTEL] & 0x1ffffc00) >> 10;
	m_utlb[replace].V = (m_m[PTEL] & 0x00000100) >> 8;
	m_utlb[replace].PSZ = (m_m[PTEL] & 0x00000080) >> 6;
	m_utlb[replace].PSZ |= (m_m[PTEL] & 0x00000010) >> 4;
	m_utlb[replace].PPR = (m_m[PTEL] & 0x00000060) >> 5;
	m_utlb[replace].C = (m_m[PTEL] & 0x00000008) >> 3;
	m_utlb[replace].D = (m_m[PTEL] & 0x00000004) >> 2;
	m_utlb[replace].SH = (m_m[PTEL] & 0x00000002) >> 1;
	m_utlb[replace].WT = (m_m[PTEL] & 0x00000001) >> 0;
	// these come from PTEA
	m_utlb[replace].TC = (m_m[PTEA] & 0x00000008) >> 3;
	m_utlb[replace].SA = (m_m[PTEA] & 0x00000007) >> 0;
}

#if 0
int sign_of(int n)
{
	return(m_sh2_state->m_fr[n] >> 31);
}

void zero(int n, int sign)
{
	if (sign == 0)
		m_sh2_state->m_fr[n] = 0x00000000;
	else
		m_sh2_state->m_fr[n] = 0x80000000;
	if ((m_sh2_state->m_fpscr & PR) == 1)
		m_sh2_state->m_fr[n + 1] = 0x00000000;
}

int data_type_of(int n)
{
	uint32_t abs;

	abs = m_sh2_state->m_fr[n] & 0x7fffffff;
	if ((m_sh2_state->m_fpscr & PR) == 0) { /* Single-precision */
		if (abs < 0x00800000) {
			if (((m_sh2_state->m_fpscr & DN) == 1) || (abs == 0x00000000)) {
				if (sign_of(n) == 0) {
					zero(n, 0);
					return(SH4_FPU_PZERO);
				}
				else {
					zero(n, 1);
					return(SH4_FPU_NZERO);
				}
			}
			else
				return(SH4_FPU_DENORM);
		}
		else
			if (abs < 0x7f800000)
				return(SH4_FPU_NORM);
			else
				if (abs == 0x7f800000) {
					if (sign_of(n) == 0)
						return(SH4_FPU_PINF);
					else
						return(SH4_FPU_NINF);
				}
				else
					if (abs < 0x7fc00000)
						return(SH4_FPU_qNaN);
					else
						return(SH4_FPU_sNaN);
	}
	else { /* Double-precision */
		if (abs < 0x00100000) {
			if (((m_sh2_state->m_fpscr & DN) == 1) || ((abs == 0x00000000) && (m_sh2_state->m_fr[n + 1] == 0x00000000))) {
				if (sign_of(n) == 0) {
					zero(n, 0);
					return(SH4_FPU_PZERO);
				}
				else {
					zero(n, 1);
					return(SH4_FPU_NZERO);
				}
			}
			else
				return(SH4_FPU_DENORM);
		}
		else
			if (abs < 0x7ff00000)
				return(SH4_FPU_NORM);
			else
				if ((abs == 0x7ff00000) && (m_sh2_state->m_fr[n + 1] == 0x00000000)) {
					if (sign_of(n) == 0)
						return(SH4_FPU_PINF);
					else
						return(SH4_FPU_NINF);
				}
				else
					if (abs < 0x7ff80000)
						return(SH4_FPU_qNaN);
					else
						return(SH4_FPU_sNaN);
	}
	return(SH4_FPU_NORM);
}
#endif

inline uint8_t sh34_base_device::RB(offs_t A)
{
	if (A >= 0xe0000000)
		return m_program->read_byte(A);

	if (A >= 0x80000000) // P1/P2/P3 region
	{
		return m_program->read_byte(A & SH34_AM);
	}
	else // P0 region
	{
		if (!m_sh4_mmu_enabled)
		{
			return m_program->read_byte(A & SH34_AM);
		}
		else
		{
			A = get_remap(A & SH34_AM);
			return m_program->read_byte(A);
		}
	}
}

inline uint16_t sh34_base_device::RW(offs_t A)
{
	if (A >= 0xe0000000)
		return m_program->read_word(A);

	if (A >= 0x80000000) // P1/P2/P3 region
	{
		return m_program->read_word(A & SH34_AM);
	}
	else
	{
		if (!m_sh4_mmu_enabled)
		{
			return m_program->read_word(A & SH34_AM);
		}
		else
		{
			A = get_remap(A & SH34_AM);
			return m_program->read_word(A);
		}
	}
}

inline uint32_t sh34_base_device::RL(offs_t A)
{
	if (A >= 0xe0000000)
		return m_program->read_dword(A);

	if (A >= 0x80000000) // P1/P2/P3 region
	{
		return m_program->read_dword(A & SH34_AM);
	}
	else
	{
		if (!m_sh4_mmu_enabled)
		{
			return m_program->read_dword(A & SH34_AM);
		}
		else
		{
			A = get_remap(A & SH34_AM);
			return m_program->read_dword(A);
		}
	}
}

inline void sh34_base_device::WB(offs_t A, uint8_t V)
{
	if (A >= 0xe0000000)
	{
		m_program->write_byte(A, V);
		return;
	}

	if (A >= 0x80000000) // P1/P2/P3 region
	{
		m_program->write_byte(A & SH34_AM, V);
	}
	else
	{
		if (!m_sh4_mmu_enabled)
		{
			m_program->write_byte(A & SH34_AM, V);
		}
		else
		{
			A = get_remap(A & SH34_AM);
			m_program->write_byte(A, V);
		}
	}
}

inline void sh34_base_device::WW(offs_t A, uint16_t V)
{
	if (A >= 0xe0000000)
	{
		m_program->write_word(A, V);
		return;
	}

	if (A >= 0x80000000) // P1/P2/P3 region
	{
		m_program->write_word(A & SH34_AM, V);
	}
	else
	{
		if (!m_sh4_mmu_enabled)
		{
			m_program->write_word(A & SH34_AM, V);
		}
		else
		{
			A = get_remap(A & SH34_AM);
			m_program->write_word(A, V);
		}
	}
}

inline void sh34_base_device::WL(offs_t A, uint32_t V)
{
	if (A >= 0xe0000000)
	{
		m_program->write_dword(A, V);
		return;
	}

	if (A >= 0x80000000) // P1/P2/P3 region
	{
		m_program->write_dword(A & SH34_AM, V);
	}
	else
	{
		if (!m_sh4_mmu_enabled)
		{
			m_program->write_dword(A & SH34_AM, V);
		}
		else
		{
			A = get_remap(A & SH34_AM);
			m_program->write_dword(A, V);
		}
	}
}

inline void sh34_base_device::ILLEGAL()
{
	NOP();
}

/*  MOVCA.L     R0,@Rn */
inline void sh34_base_device::MOVCAL(const uint16_t opcode)
{
	m_sh2_state->ea = m_sh2_state->r[Rn];
	WL(m_sh2_state->ea, m_sh2_state->r[0]);
}

inline void sh34_base_device::CLRS(const uint16_t opcode)
{
	m_sh2_state->sr &= ~SH_S;
}

inline void sh34_base_device::SETS(const uint16_t opcode)
{
	m_sh2_state->sr |= SH_S;
}

/*  LDC     Rm,SR */
inline void sh34_base_device::LDCSR(const uint16_t opcode)
{
	// important to store the value now so that it doesn't get affected by the bank change
	uint32_t reg = m_sh2_state->r[Rn];

	if ((machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
		sh4_syncronize_register_bank((m_sh2_state->sr & sRB) >> 29);

	if ((m_sh2_state->r[Rn] & sRB) != (m_sh2_state->sr & sRB))
		sh4_change_register_bank(m_sh2_state->r[Rn] & sRB ? 1 : 0);

	m_sh2_state->sr = reg & SH34_FLAGS;
	sh4_exception_recompute();
}

/*  LDC.L   @Rm+,SR */
inline void sh34_base_device::LDCMSR(const uint16_t opcode)
{
	uint32_t old;

	old = m_sh2_state->sr;
	m_sh2_state->ea = m_sh2_state->r[Rn];
	m_sh2_state->sr = RL(m_sh2_state->ea) & SH34_FLAGS;
	if ((machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
		sh4_syncronize_register_bank((old & sRB) >> 29);
	if ((old & sRB) != (m_sh2_state->sr & sRB))
		sh4_change_register_bank(m_sh2_state->sr & sRB ? 1 : 0);
	m_sh2_state->r[Rn] += 4;
	m_sh2_state->icount -= 2;
	sh4_exception_recompute();
}

/*  RTE */
inline void sh34_base_device::RTE()
{
	m_sh2_state->m_delay = m_sh2_state->ea = m_sh2_state->m_spc;

	if ((machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
		sh4_syncronize_register_bank((m_sh2_state->sr & sRB) >> 29);
	if ((m_sh2_state->m_ssr & sRB) != (m_sh2_state->sr & sRB))
		sh4_change_register_bank(m_sh2_state->m_ssr & sRB ? 1 : 0);

	m_sh2_state->sr = m_sh2_state->m_ssr;
	m_sh2_state->icount--;
	sh4_exception_recompute();
}

/*  TRAPA   #imm */
inline void sh34_base_device::TRAPA(uint32_t i)
{
	uint32_t imm = i & 0xff;

	if (m_cpu_type == CPU_TYPE_SH4)
	{
		m_m[TRA] = imm << 2;
	}
	else /* SH3 */
	{
		m_sh3internal_upper[SH3_TRA_ADDR] = imm << 2;
	}

	m_sh2_state->m_ssr = m_sh2_state->sr;
	m_sh2_state->m_spc = m_sh2_state->pc;

	m_sh2_state->m_sgr = m_sh2_state->r[15];

	m_sh2_state->sr |= MD;
	if ((machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
		sh4_syncronize_register_bank((m_sh2_state->sr & sRB) >> 29);
	if (!(m_sh2_state->sr & sRB))
		sh4_change_register_bank(1);
	m_sh2_state->sr |= sRB;
	m_sh2_state->sr |= BL;
	sh4_exception_recompute();

	if (m_cpu_type == CPU_TYPE_SH4)
	{
		m_m[EXPEVT] = 0x00000160;
	}
	else /* SH3 */
	{
		m_sh3internal_upper[SH3_EXPEVT_ADDR] = 0x00000160;
	}

	m_sh2_state->pc = m_sh2_state->vbr + 0x00000100;

	m_sh2_state->icount -= 7;
}

/*  STCRBANK   Rm_BANK,Rn */
inline void sh34_base_device::STCRBANK(const uint16_t opcode)
{
	uint32_t m = Rm;

	m_sh2_state->r[Rn] = m_sh2_state->m_rbnk[m_sh2_state->sr&sRB ? 0 : 1][m & 7];
}

/*  STCMRBANK   Rm_BANK,@-Rn */
inline void sh34_base_device::STCMRBANK(const uint16_t opcode)
{
	uint32_t m = Rm; uint32_t n = Rn;

	m_sh2_state->r[n] -= 4;
	m_sh2_state->ea = m_sh2_state->r[n];
	WL(m_sh2_state->ea, m_sh2_state->m_rbnk[m_sh2_state->sr&sRB ? 0 : 1][m & 7]);
	m_sh2_state->icount--;
}

/*  STS.L   SGR,@-Rn */
inline void sh34_base_device::STCMSGR(const uint16_t opcode)
{
	uint32_t n = Rn;

	m_sh2_state->r[n] -= 4;
	m_sh2_state->ea = m_sh2_state->r[n];
	WL(m_sh2_state->ea, m_sh2_state->m_sgr);
}

/*  STS.L   FPUL,@-Rn */
inline void sh34_base_device::STSMFPUL(const uint16_t opcode)
{
	uint32_t n = Rn;

	m_sh2_state->r[n] -= 4;
	m_sh2_state->ea = m_sh2_state->r[n];
	WL(m_sh2_state->ea, m_sh2_state->m_fpul);
}

/*  STS.L   FPSCR,@-Rn */
inline void sh34_base_device::STSMFPSCR(const uint16_t opcode)
{
	uint32_t n = Rn;

	m_sh2_state->r[n] -= 4;
	m_sh2_state->ea = m_sh2_state->r[n];
	WL(m_sh2_state->ea, m_sh2_state->m_fpscr & 0x003FFFFF);
}

/*  STC.L   DBR,@-Rn */
inline void sh34_base_device::STCMDBR(const uint16_t opcode)
{
	uint32_t n = Rn;

	m_sh2_state->r[n] -= 4;
	m_sh2_state->ea = m_sh2_state->r[n];
	WL(m_sh2_state->ea, m_sh2_state->m_dbr);
}

/*  STC.L   SSR,@-Rn */
inline void sh34_base_device::STCMSSR(const uint16_t opcode)
{
	uint32_t n = Rn;

	m_sh2_state->r[n] -= 4;
	m_sh2_state->ea = m_sh2_state->r[n];
	WL(m_sh2_state->ea, m_sh2_state->m_ssr);
}

/*  STC.L   SPC,@-Rn */
inline void sh34_base_device::STCMSPC(const uint16_t opcode)
{
	uint32_t n = Rn;

	m_sh2_state->r[n] -= 4;
	m_sh2_state->ea = m_sh2_state->r[n];
	WL(m_sh2_state->ea, m_sh2_state->m_spc);
}

/*  LDS.L   @Rm+,FPUL */
inline void sh34_base_device::LDSMFPUL(const uint16_t opcode)
{
	m_sh2_state->ea = m_sh2_state->r[Rn];
	m_sh2_state->m_fpul = RL(m_sh2_state->ea);
	m_sh2_state->r[Rn] += 4;
}

/*  LDS.L   @Rm+,FPSCR */
inline void sh34_base_device::LDSMFPSCR(const uint16_t opcode)
{
	uint32_t s;

	s = m_sh2_state->m_fpscr;
	m_sh2_state->ea = m_sh2_state->r[Rn];
	m_sh2_state->m_fpscr = RL(m_sh2_state->ea);
	m_sh2_state->m_fpscr &= 0x003FFFFF;
	m_sh2_state->r[Rn] += 4;
	if ((s & FR) != (m_sh2_state->m_fpscr & FR))
		sh4_swap_fp_registers();
#ifdef LSB_FIRST
	if ((s & PR) != (m_sh2_state->m_fpscr & PR))
		sh4_swap_fp_couples();
#endif
	m_sh2_state->m_fpu_sz = (m_sh2_state->m_fpscr & SZ) ? 1 : 0;
	m_sh2_state->m_fpu_pr = (m_sh2_state->m_fpscr & PR) ? 1 : 0;
}

/*  LDC.L   @Rm+,DBR */
inline void sh34_base_device::LDCMDBR(const uint16_t opcode)
{
	m_sh2_state->ea = m_sh2_state->r[Rn];
	m_sh2_state->m_dbr = RL(m_sh2_state->ea);
	m_sh2_state->r[Rn] += 4;
}

/*  LDC.L   @Rn+,Rm_BANK */
inline void sh34_base_device::LDCMRBANK(const uint16_t opcode)
{
	uint32_t m = Rm; uint32_t n = Rn;

	m_sh2_state->ea = m_sh2_state->r[n];
	m_sh2_state->m_rbnk[m_sh2_state->sr&sRB ? 0 : 1][m & 7] = RL(m_sh2_state->ea);
	m_sh2_state->r[n] += 4;
}

/*  LDC.L   @Rm+,SSR */
inline void sh34_base_device::LDCMSSR(const uint16_t opcode)
{
	m_sh2_state->ea = m_sh2_state->r[Rn];
	m_sh2_state->m_ssr = RL(m_sh2_state->ea);
	m_sh2_state->r[Rn] += 4;
}

/*  LDC.L   @Rm+,SPC */
inline void sh34_base_device::LDCMSPC(const uint16_t opcode)
{
	m_sh2_state->ea = m_sh2_state->r[Rn];
	m_sh2_state->m_spc = RL(m_sh2_state->ea);
	m_sh2_state->r[Rn] += 4;
}

/*  LDS     Rm,FPUL */
inline void sh34_base_device::LDSFPUL(const uint16_t opcode)
{
	m_sh2_state->m_fpul = m_sh2_state->r[Rn];
}

/*  LDS     Rm,FPSCR */
inline void sh34_base_device::LDSFPSCR(const uint16_t opcode)
{
	uint32_t s;

	s = m_sh2_state->m_fpscr;
	m_sh2_state->m_fpscr = m_sh2_state->r[Rn] & 0x003FFFFF;
	if ((s & FR) != (m_sh2_state->m_fpscr & FR))
		sh4_swap_fp_registers();
#ifdef LSB_FIRST
	if ((s & PR) != (m_sh2_state->m_fpscr & PR))
		sh4_swap_fp_couples();
#endif
	m_sh2_state->m_fpu_sz = (m_sh2_state->m_fpscr & SZ) ? 1 : 0;
	m_sh2_state->m_fpu_pr = (m_sh2_state->m_fpscr & PR) ? 1 : 0;
}

/*  LDC     Rm,DBR */
inline void sh34_base_device::LDCDBR(const uint16_t opcode)
{
	m_sh2_state->m_dbr = m_sh2_state->r[Rn];
}


/*  STC     SSR,Rn */
inline void sh34_base_device::STCSSR(const uint16_t opcode)
{
	m_sh2_state->r[Rn] = m_sh2_state->m_ssr;
}

/*  STC     SPC,Rn */
inline void sh34_base_device::STCSPC(const uint16_t opcode)
{
	m_sh2_state->r[Rn] = m_sh2_state->m_spc;
}

/*  STC     SGR,Rn */
inline void sh34_base_device::STCSGR(const uint16_t opcode)
{
	m_sh2_state->r[Rn] = m_sh2_state->m_sgr;
}

/*  STS     FPUL,Rn */
inline void sh34_base_device::STSFPUL(const uint16_t opcode)
{
	m_sh2_state->r[Rn] = m_sh2_state->m_fpul;
}

/*  STS     FPSCR,Rn */
inline void sh34_base_device::STSFPSCR(const uint16_t opcode)
{
	m_sh2_state->r[Rn] = m_sh2_state->m_fpscr & 0x003FFFFF;
}

/*  STC     DBR,Rn */
inline void sh34_base_device::STCDBR(const uint16_t opcode)
{
	m_sh2_state->r[Rn] = m_sh2_state->m_dbr;
}

/*  SHAD    Rm,Rn */
inline void sh34_base_device::SHAD(const uint16_t opcode)
{
	uint32_t m = Rm; uint32_t n = Rn;

	if ((m_sh2_state->r[m] & 0x80000000) == 0)
		m_sh2_state->r[n] = m_sh2_state->r[n] << (m_sh2_state->r[m] & 0x1F);
	else if ((m_sh2_state->r[m] & 0x1F) == 0) {
		if ((m_sh2_state->r[n] & 0x80000000) == 0)
			m_sh2_state->r[n] = 0;
		else
			m_sh2_state->r[n] = 0xFFFFFFFF;
	}
	else
		m_sh2_state->r[n] = (int32_t)m_sh2_state->r[n] >> ((~m_sh2_state->r[m] & 0x1F) + 1);
}

/*  SHLD    Rm,Rn */
inline void sh34_base_device::SHLD(const uint16_t opcode)
{
	uint32_t m = Rm; uint32_t n = Rn;

	if ((m_sh2_state->r[m] & 0x80000000) == 0)
		m_sh2_state->r[n] = m_sh2_state->r[n] << (m_sh2_state->r[m] & 0x1F);
	else if ((m_sh2_state->r[m] & 0x1F) == 0)
		m_sh2_state->r[n] = 0;
	else
		m_sh2_state->r[n] = m_sh2_state->r[n] >> ((~m_sh2_state->r[m] & 0x1F) + 1);
}

/*  LDCRBANK   Rn,Rm_BANK */
inline void sh34_base_device::LDCRBANK(const uint16_t opcode)
{
	uint32_t m = Rm;

	m_sh2_state->m_rbnk[m_sh2_state->sr&sRB ? 0 : 1][m & 7] = m_sh2_state->r[Rn];
}

/*  LDC     Rm,SSR */
inline void sh34_base_device::LDCSSR(const uint16_t opcode)
{
	m_sh2_state->m_ssr = m_sh2_state->r[Rn];
}

/*  LDC     Rm,SPC */
inline void sh34_base_device::LDCSPC(const uint16_t opcode)
{
	m_sh2_state->m_spc = m_sh2_state->r[Rn];
}

/*  PREF     @Rn */
inline void sh34_base_device::PREFM(const uint16_t opcode)
{
	int a;
	uint32_t addr, dest, sq;

	addr = m_sh2_state->r[Rn]; // address
	if ((addr >= 0xE0000000) && (addr <= 0xE3FFFFFF))
	{
		if (m_sh4_mmu_enabled)
		{
			addr = addr & 0xFFFFFFE0;
			dest = sh4_getsqremap(addr); // good enough for naomi-gd rom, probably not much else

		}
		else
		{
			sq = (addr & 0x20) >> 5;
			dest = addr & 0x03FFFFE0;
			if (sq == 0)
			{
				if (m_cpu_type == CPU_TYPE_SH4)
				{
					dest |= (m_m[QACR0] & 0x1C) << 24;
				}
				else
				{
					fatalerror("m_cpu_type != CPU_TYPE_SH4 but access internal regs\n");
				}
			}
			else
			{
				if (m_cpu_type == CPU_TYPE_SH4)
				{
					dest |= (m_m[QACR1] & 0x1C) << 24;
				}
				else
				{
					fatalerror("m_cpu_type != CPU_TYPE_SH4 but access internal regs\n");
				}

			}
			addr = addr & 0xFFFFFFE0;
		}

		for (a = 0;a < 4;a++)
		{
			// shouldn't be causing a memory read, should store sq writes in registers.
			m_program->write_qword(dest, m_program->read_qword(addr));
			addr += 8;
			dest += 8;
		}
	}
}

/*****************************************************************************
 *  OPCODE DISPATCHERS
 *****************************************************************************/

// TODO: current SZ=1(64bit) FMOVs correct for SH4 in LE mode only

/*  FMOV.S  @Rm+,FRn PR=0 SZ=0 1111nnnnmmmm1001 */
/*  FMOV    @Rm+,DRn PR=0 SZ=1 1111nnn0mmmm1001 */
/*  FMOV    @Rm+,XDn PR=0 SZ=1 1111nnn1mmmm1001 */
/*  FMOV    @Rm+,XDn PR=1      1111nnn1mmmm1001 */
inline void sh34_base_device::FMOVMRIFR(const uint16_t opcode)
{
	uint32_t m = Rm; uint32_t n = Rn;

	if (m_sh2_state->m_fpu_sz) { /* SZ = 1 */
		if (n & 1) {
			n &= 14;
#ifdef LSB_FIRST
			n ^= m_sh2_state->m_fpu_pr;
#endif
			m_sh2_state->ea = m_sh2_state->r[m];
			m_sh2_state->m_xf[n] = RL(m_sh2_state->ea);
			m_sh2_state->r[m] += 4;
			m_sh2_state->m_xf[n ^ 1] = RL(m_sh2_state->ea + 4);
			m_sh2_state->r[m] += 4;
		}
		else {
#ifdef LSB_FIRST
			n ^= m_sh2_state->m_fpu_pr;
#endif
			m_sh2_state->ea = m_sh2_state->r[m];
			m_sh2_state->m_fr[n] = RL(m_sh2_state->ea);
			m_sh2_state->r[m] += 4;
			m_sh2_state->m_fr[n ^ 1] = RL(m_sh2_state->ea + 4);
			m_sh2_state->r[m] += 4;
		}
	}
	else {              /* SZ = 0 */
		m_sh2_state->ea = m_sh2_state->r[m];
#ifdef LSB_FIRST
		n ^= m_sh2_state->m_fpu_pr;
#endif
		m_sh2_state->m_fr[n] = RL(m_sh2_state->ea);
		m_sh2_state->r[m] += 4;
	}
}

/*  FMOV.S  FRm,@Rn PR=0 SZ=0 1111nnnnmmmm1010 */
/*  FMOV    DRm,@Rn PR=0 SZ=1 1111nnnnmmm01010 */
/*  FMOV    XDm,@Rn PR=0 SZ=1 1111nnnnmmm11010 */
/*  FMOV    XDm,@Rn PR=1      1111nnnnmmm11010 */
inline void sh34_base_device::FMOVFRMR(const uint16_t opcode)
{
	uint32_t m = Rm; uint32_t n = Rn;

	if (m_sh2_state->m_fpu_sz) { /* SZ = 1 */
		if (m & 1) {
			m &= 14;
#ifdef LSB_FIRST
			m ^= m_sh2_state->m_fpu_pr;
#endif
			m_sh2_state->ea = m_sh2_state->r[n];
			WL(m_sh2_state->ea, m_sh2_state->m_xf[m]);
			WL(m_sh2_state->ea + 4, m_sh2_state->m_xf[m ^ 1]);
		}
		else {
#ifdef LSB_FIRST
			m ^= m_sh2_state->m_fpu_pr;
#endif
			m_sh2_state->ea = m_sh2_state->r[n];
			WL(m_sh2_state->ea, m_sh2_state->m_fr[m]);
			WL(m_sh2_state->ea + 4, m_sh2_state->m_fr[m ^ 1]);
		}
	}
	else {              /* SZ = 0 */
		m_sh2_state->ea = m_sh2_state->r[n];
#ifdef LSB_FIRST
		m ^= m_sh2_state->m_fpu_pr;
#endif
		WL(m_sh2_state->ea, m_sh2_state->m_fr[m]);
	}
}

/*  FMOV.S  FRm,@-Rn PR=0 SZ=0 1111nnnnmmmm1011 */
/*  FMOV    DRm,@-Rn PR=0 SZ=1 1111nnnnmmm01011 */
/*  FMOV    XDm,@-Rn PR=0 SZ=1 1111nnnnmmm11011 */
/*  FMOV    XDm,@-Rn PR=1      1111nnnnmmm11011 */
inline void sh34_base_device::FMOVFRMDR(const uint16_t opcode)
{
	uint32_t m = Rm; uint32_t n = Rn;

	if (m_sh2_state->m_fpu_sz) { /* SZ = 1 */
		if (m & 1) {
			m &= 14;
#ifdef LSB_FIRST
			m ^= m_sh2_state->m_fpu_pr;
#endif
			m_sh2_state->r[n] -= 8;
			m_sh2_state->ea = m_sh2_state->r[n];
			WL(m_sh2_state->ea, m_sh2_state->m_xf[m]);
			WL(m_sh2_state->ea + 4, m_sh2_state->m_xf[m ^ 1]);
		}
		else {
#ifdef LSB_FIRST
			m ^= m_sh2_state->m_fpu_pr;
#endif
			m_sh2_state->r[n] -= 8;
			m_sh2_state->ea = m_sh2_state->r[n];
			WL(m_sh2_state->ea, m_sh2_state->m_fr[m]);
			WL(m_sh2_state->ea + 4, m_sh2_state->m_fr[m ^ 1]);
		}
	}
	else {              /* SZ = 0 */
		m_sh2_state->r[n] -= 4;
		m_sh2_state->ea = m_sh2_state->r[n];
#ifdef LSB_FIRST
		m ^= m_sh2_state->m_fpu_pr;
#endif
		WL(m_sh2_state->ea, m_sh2_state->m_fr[m]);
	}
}

/*  FMOV.S  FRm,@(R0,Rn) PR=0 SZ=0 1111nnnnmmmm0111 */
/*  FMOV    DRm,@(R0,Rn) PR=0 SZ=1 1111nnnnmmm00111 */
/*  FMOV    XDm,@(R0,Rn) PR=0 SZ=1 1111nnnnmmm10111 */
/*  FMOV    XDm,@(R0,Rn) PR=1      1111nnnnmmm10111 */
inline void sh34_base_device::FMOVFRS0(const uint16_t opcode)
{
	uint32_t m = Rm; uint32_t n = Rn;

	if (m_sh2_state->m_fpu_sz) { /* SZ = 1 */
		if (m & 1) {
			m &= 14;
#ifdef LSB_FIRST
			m ^= m_sh2_state->m_fpu_pr;
#endif
			m_sh2_state->ea = m_sh2_state->r[0] + m_sh2_state->r[n];
			WL(m_sh2_state->ea, m_sh2_state->m_xf[m]);
			WL(m_sh2_state->ea + 4, m_sh2_state->m_xf[m ^ 1]);
		}
		else {
#ifdef LSB_FIRST
			m ^= m_sh2_state->m_fpu_pr;
#endif
			m_sh2_state->ea = m_sh2_state->r[0] + m_sh2_state->r[n];
			WL(m_sh2_state->ea, m_sh2_state->m_fr[m]);
			WL(m_sh2_state->ea + 4, m_sh2_state->m_fr[m ^ 1]);
		}
	}
	else {              /* SZ = 0 */
		m_sh2_state->ea = m_sh2_state->r[0] + m_sh2_state->r[n];
#ifdef LSB_FIRST
		m ^= m_sh2_state->m_fpu_pr;
#endif
		WL(m_sh2_state->ea, m_sh2_state->m_fr[m]);
	}
}

/*  FMOV.S  @(R0,Rm),FRn PR=0 SZ=0 1111nnnnmmmm0110 */
/*  FMOV    @(R0,Rm),DRn PR=0 SZ=1 1111nnn0mmmm0110 */
/*  FMOV    @(R0,Rm),XDn PR=0 SZ=1 1111nnn1mmmm0110 */
/*  FMOV    @(R0,Rm),XDn PR=1      1111nnn1mmmm0110 */
inline void sh34_base_device::FMOVS0FR(const uint16_t opcode)
{
	uint32_t m = Rm; uint32_t n = Rn;

	if (m_sh2_state->m_fpu_sz) { /* SZ = 1 */
		if (n & 1) {
			n &= 14;
#ifdef LSB_FIRST
			n ^= m_sh2_state->m_fpu_pr;
#endif
			m_sh2_state->ea = m_sh2_state->r[0] + m_sh2_state->r[m];
			m_sh2_state->m_xf[n] = RL(m_sh2_state->ea);
			m_sh2_state->m_xf[n ^ 1] = RL(m_sh2_state->ea + 4);
		}
		else {
#ifdef LSB_FIRST
			n ^= m_sh2_state->m_fpu_pr;
#endif
			m_sh2_state->ea = m_sh2_state->r[0] + m_sh2_state->r[m];
			m_sh2_state->m_fr[n] = RL(m_sh2_state->ea);
			m_sh2_state->m_fr[n ^ 1] = RL(m_sh2_state->ea + 4);
		}
	}
	else {              /* SZ = 0 */
		m_sh2_state->ea = m_sh2_state->r[0] + m_sh2_state->r[m];
#ifdef LSB_FIRST
		n ^= m_sh2_state->m_fpu_pr;
#endif
		m_sh2_state->m_fr[n] = RL(m_sh2_state->ea);
	}
}

/*  FMOV.S  @Rm,FRn PR=0 SZ=0 1111nnnnmmmm1000 */
/*  FMOV    @Rm,DRn PR=0 SZ=1 1111nnn0mmmm1000 */
/*  FMOV    @Rm,XDn PR=0 SZ=1 1111nnn1mmmm1000 */
/*  FMOV    @Rm,XDn PR=1      1111nnn1mmmm1000 */
/*  FMOV    @Rm,DRn PR=1      1111nnn0mmmm1000 */
inline void sh34_base_device::FMOVMRFR(const uint16_t opcode)
{
	uint32_t m = Rm; uint32_t n = Rn;

	if (m_sh2_state->m_fpu_sz) { /* SZ = 1 */
		if (n & 1) {
			n &= 14;
#ifdef LSB_FIRST
			n ^= m_sh2_state->m_fpu_pr;
#endif
			m_sh2_state->ea = m_sh2_state->r[m];
			m_sh2_state->m_xf[n] = RL(m_sh2_state->ea);
			m_sh2_state->m_xf[n ^ 1] = RL(m_sh2_state->ea + 4);
		}
		else {
#ifdef LSB_FIRST
			n ^= m_sh2_state->m_fpu_pr;
#endif
			m_sh2_state->ea = m_sh2_state->r[m];
			m_sh2_state->m_fr[n] = RL(m_sh2_state->ea);
			m_sh2_state->m_fr[n ^ 1] = RL(m_sh2_state->ea + 4);
		}
	}
	else {              /* SZ = 0 */
		m_sh2_state->ea = m_sh2_state->r[m];
#ifdef LSB_FIRST
		n ^= m_sh2_state->m_fpu_pr;
#endif
		m_sh2_state->m_fr[n] = RL(m_sh2_state->ea);
	}
}

/*  FMOV    FRm,FRn PR=0 SZ=0 FRm -> FRn 1111nnnnmmmm1100 */
/*  FMOV    DRm,DRn PR=0 SZ=1 DRm -> DRn 1111nnn0mmm01100 */
/*  FMOV    XDm,DRn PR=1      XDm -> DRn 1111nnn0mmm11100 */
/*  FMOV    DRm,XDn PR=1      DRm -> XDn 1111nnn1mmm01100 */
/*  FMOV    XDm,XDn PR=1      XDm -> XDn 1111nnn1mmm11100 */
inline void sh34_base_device::FMOVFR(const uint16_t opcode)
{
	uint32_t m = Rm; uint32_t n = Rn;

	if (m_sh2_state->m_fpu_sz == 0) {  /* SZ = 0 */
#ifdef LSB_FIRST
		n ^= m_sh2_state->m_fpu_pr;
		m ^= m_sh2_state->m_fpu_pr;
#endif
		m_sh2_state->m_fr[n] = m_sh2_state->m_fr[m];
	}
	else { /* SZ = 1 */
		if (m & 1) {
			if (n & 1) {
				m_sh2_state->m_xf[n & 14] = m_sh2_state->m_xf[m & 14];
				m_sh2_state->m_xf[n | 1] = m_sh2_state->m_xf[m | 1];
			}
			else {
				m_sh2_state->m_fr[n] = m_sh2_state->m_xf[m & 14];
				m_sh2_state->m_fr[n | 1] = m_sh2_state->m_xf[m | 1];
			}
		}
		else {
			if (n & 1) {
				m_sh2_state->m_xf[n & 14] = m_sh2_state->m_fr[m];
				m_sh2_state->m_xf[n | 1] = m_sh2_state->m_fr[m | 1]; // (a&14)+1 -> a|1
			}
			else {
				m_sh2_state->m_fr[n] = m_sh2_state->m_fr[m];
				m_sh2_state->m_fr[n | 1] = m_sh2_state->m_fr[m | 1];
			}
		}
	}
}

/*  FLDI1  FRn 1111nnnn10011101 */
inline void sh34_base_device::FLDI1(const uint16_t opcode)
{
#ifdef LSB_FIRST
	m_sh2_state->m_fr[Rn ^ m_sh2_state->m_fpu_pr] = 0x3F800000;
#else
	m_sh2_state->m_fr[Rn] = 0x3F800000;
#endif
}

/*  FLDI0  FRn 1111nnnn10001101 */
inline void sh34_base_device::FLDI0(const uint16_t opcode)
{
#ifdef LSB_FIRST
	m_sh2_state->m_fr[Rn ^ m_sh2_state->m_fpu_pr] = 0;
#else
	m_sh2_state->m_fr[Rn] = 0;
#endif
}

/*  FLDS FRm,FPUL 1111mmmm00011101 */
inline void sh34_base_device::FLDS(const uint16_t opcode)
{
#ifdef LSB_FIRST
	m_sh2_state->m_fpul = m_sh2_state->m_fr[Rn ^ m_sh2_state->m_fpu_pr];
#else
	m_sh2_state->m_fpul = m_sh2_state->m_fr[Rn];
#endif
}

/*  FSTS FPUL,FRn 1111nnnn00001101 */
inline void sh34_base_device::FSTS(const uint16_t opcode)
{
#ifdef LSB_FIRST
	m_sh2_state->m_fr[Rn ^ m_sh2_state->m_fpu_pr] = m_sh2_state->m_fpul;
#else
	m_sh2_state->m_fr[Rn] = m_sh2_state->m_fpul;
#endif
}

/* FRCHG 1111101111111101 */
void sh34_base_device::FRCHG()
{
	m_sh2_state->m_fpscr ^= FR;
	sh4_swap_fp_registers();
}

/* FSCHG 1111001111111101 */
void sh34_base_device::FSCHG()
{
	m_sh2_state->m_fpscr ^= SZ;
	m_sh2_state->m_fpu_sz = (m_sh2_state->m_fpscr & SZ) ? 1 : 0;
}

/* FTRC FRm,FPUL PR=0 1111mmmm00111101 */
/* FTRC DRm,FPUL PR=1 1111mmm000111101 */
inline void sh34_base_device::FTRC(const uint16_t opcode)
{
	uint32_t n = Rn;

	if (m_sh2_state->m_fpu_pr) { /* PR = 1 */
		if (n & 1)
			fatalerror("SH-4: FTRC opcode used with n %d", n);

		n = n & 14;
		*((int32_t *)&m_sh2_state->m_fpul) = (int32_t)FP_RFD(n);
	}
	else {              /* PR = 0 */
	 /* read m_sh2_state->m_fr[n] as float -> truncate -> fpul(32) */
		*((int32_t *)&m_sh2_state->m_fpul) = (int32_t)FP_RFS(n);
	}
}

/* FLOAT FPUL,FRn PR=0 1111nnnn00101101 */
/* FLOAT FPUL,DRn PR=1 1111nnn000101101 */
inline void sh34_base_device::FLOAT(const uint16_t opcode)
{
	uint32_t n = Rn;

	if (m_sh2_state->m_fpu_pr) { /* PR = 1 */
		if (n & 1)
			fatalerror("SH-4: FLOAT opcode used with n %d", n);

		n = n & 14;
		FP_RFD(n) = (double)*((int32_t *)&m_sh2_state->m_fpul);
	}
	else {              /* PR = 0 */
		FP_RFS(n) = (float)*((int32_t *)&m_sh2_state->m_fpul);
	}
}

/* FNEG FRn PR=0 1111nnnn01001101 */
/* FNEG DRn PR=1 1111nnn001001101 */
inline void sh34_base_device::FNEG(const uint16_t opcode)
{
	uint32_t n = Rn;

	if (m_sh2_state->m_fpu_pr) { /* PR = 1 */
		FP_RFD(n) = -FP_RFD(n);
	}
	else {              /* PR = 0 */
		FP_RFS(n) = -FP_RFS(n);
	}
}

/* FABS FRn PR=0 1111nnnn01011101 */
/* FABS DRn PR=1 1111nnn001011101 */
inline void sh34_base_device::FABS(const uint16_t opcode)
{
	uint32_t n = Rn;

	if (m_sh2_state->m_fpu_pr) { /* PR = 1 */
#ifdef LSB_FIRST
		n = n | 1; // n & 14 + 1
		m_sh2_state->m_fr[n] = m_sh2_state->m_fr[n] & 0x7fffffff;
#else
		n = n & 14;
		m_sh2_state->m_fr[n] = m_sh2_state->m_fr[n] & 0x7fffffff;
#endif
	}
	else {              /* PR = 0 */
		m_sh2_state->m_fr[n] = m_sh2_state->m_fr[n] & 0x7fffffff;
	}
}

/* FCMP/EQ FRm,FRn PR=0 1111nnnnmmmm0100 */
/* FCMP/EQ DRm,DRn PR=1 1111nnn0mmm00100 */
inline void sh34_base_device::FCMP_EQ(const uint16_t opcode)
{
	uint32_t m = Rm; uint32_t n = Rn;

	if (m_sh2_state->m_fpu_pr) { /* PR = 1 */
		n = n & 14;
		m = m & 14;
		if (FP_RFD(n) == FP_RFD(m))
			m_sh2_state->sr |= SH_T;
		else
			m_sh2_state->sr &= ~SH_T;
	}
	else {              /* PR = 0 */
		if (FP_RFS(n) == FP_RFS(m))
			m_sh2_state->sr |= SH_T;
		else
			m_sh2_state->sr &= ~SH_T;
	}
}

/* FCMP/GT FRm,FRn PR=0 1111nnnnmmmm0101 */
/* FCMP/GT DRm,DRn PR=1 1111nnn0mmm00101 */
inline void sh34_base_device::FCMP_GT(const uint16_t opcode)
{
	uint32_t m = Rm; uint32_t n = Rn;

	if (m_sh2_state->m_fpu_pr) { /* PR = 1 */
		n = n & 14;
		m = m & 14;
		if (FP_RFD(n) > FP_RFD(m))
			m_sh2_state->sr |= SH_T;
		else
			m_sh2_state->sr &= ~SH_T;
	}
	else {              /* PR = 0 */
		if (FP_RFS(n) > FP_RFS(m))
			m_sh2_state->sr |= SH_T;
		else
			m_sh2_state->sr &= ~SH_T;
	}
}

/* FCNVDS DRm,FPUL PR=1 1111mmm010111101 */
inline void sh34_base_device::FCNVDS(const uint16_t opcode)
{
	uint32_t n = Rn;

	if (m_sh2_state->m_fpu_pr) { /* PR = 1 */
		n = n & 14;
		if (m_sh2_state->m_fpscr & RM)
			m_sh2_state->m_fr[n | NATIVE_ENDIAN_VALUE_LE_BE(0, 1)] &= 0xe0000000; /* round toward zero*/
		*((float *)&m_sh2_state->m_fpul) = (float)FP_RFD(n);
	}
}

/* FCNVSD FPUL, DRn PR=1 1111nnn010101101 */
inline void sh34_base_device::FCNVSD(const uint16_t opcode)
{
	uint32_t n = Rn;

	if (m_sh2_state->m_fpu_pr) { /* PR = 1 */
		n = n & 14;
		FP_RFD(n) = (double)*((float *)&m_sh2_state->m_fpul);
	}
}

/* FADD FRm,FRn PR=0 1111nnnnmmmm0000 */
/* FADD DRm,DRn PR=1 1111nnn0mmm00000 */
inline void sh34_base_device::FADD(const uint16_t opcode)
{
	uint32_t m = Rm; uint32_t n = Rn;

	if (m_sh2_state->m_fpu_pr) { /* PR = 1 */
		n = n & 14;
		m = m & 14;
		FP_RFD(n) = FP_RFD(n) + FP_RFD(m);
	}
	else {              /* PR = 0 */
		FP_RFS(n) = FP_RFS(n) + FP_RFS(m);
	}
}

/* FSUB FRm,FRn PR=0 1111nnnnmmmm0001 */
/* FSUB DRm,DRn PR=1 1111nnn0mmm00001 */
inline void sh34_base_device::FSUB(const uint16_t opcode)
{
	uint32_t m = Rm; uint32_t n = Rn;

	if (m_sh2_state->m_fpu_pr) { /* PR = 1 */
		n = n & 14;
		m = m & 14;
		FP_RFD(n) = FP_RFD(n) - FP_RFD(m);
	}
	else {              /* PR = 0 */
		FP_RFS(n) = FP_RFS(n) - FP_RFS(m);
	}
}


/* FMUL FRm,FRn PR=0 1111nnnnmmmm0010 */
/* FMUL DRm,DRn PR=1 1111nnn0mmm00010 */
inline void sh34_base_device::FMUL(const uint16_t opcode)
{
	uint32_t m = Rm; uint32_t n = Rn;

	if (m_sh2_state->m_fpu_pr) { /* PR = 1 */
		n = n & 14;
		m = m & 14;
		FP_RFD(n) = FP_RFD(n) * FP_RFD(m);
	}
	else {              /* PR = 0 */
		FP_RFS(n) = FP_RFS(n) * FP_RFS(m);
	}
}

/* FDIV FRm,FRn PR=0 1111nnnnmmmm0011 */
/* FDIV DRm,DRn PR=1 1111nnn0mmm00011 */
inline void sh34_base_device::FDIV(const uint16_t opcode)
{
	uint32_t m = Rm; uint32_t n = Rn;

	if (m_sh2_state->m_fpu_pr) { /* PR = 1 */
		n = n & 14;
		m = m & 14;
		if (FP_RFD(m) == 0)
			return;
		FP_RFD(n) = FP_RFD(n) / FP_RFD(m);
	}
	else {              /* PR = 0 */
		if (FP_RFS(m) == 0)
			return;
		FP_RFS(n) = FP_RFS(n) / FP_RFS(m);
	}
}

/* FMAC FR0,FRm,FRn PR=0 1111nnnnmmmm1110 */
inline void sh34_base_device::FMAC(const uint16_t opcode)
{
	uint32_t m = Rm; uint32_t n = Rn;

	if (m_sh2_state->m_fpu_pr == 0) { /* PR = 0 */
		FP_RFS(n) = (FP_RFS(0) * FP_RFS(m)) + FP_RFS(n);
	}
}

/* FSQRT FRn PR=0 1111nnnn01101101 */
/* FSQRT DRn PR=1 1111nnnn01101101 */
inline void sh34_base_device::FSQRT(const uint16_t opcode)
{
	uint32_t n = Rn;

	if (m_sh2_state->m_fpu_pr) { /* PR = 1 */
		n = n & 14;
		if (FP_RFD(n) < 0)
			return;
		FP_RFD(n) = sqrtf(FP_RFD(n));
	}
	else {              /* PR = 0 */
		if (FP_RFS(n) < 0)
			return;
		FP_RFS(n) = sqrtf(FP_RFS(n));
	}
}

/* FSRRA FRn PR=0 1111nnnn01111101 */
inline void sh34_base_device::FSRRA(const uint16_t opcode)
{
	uint32_t n = Rn;

	if (FP_RFS(n) < 0)
		return;
	FP_RFS(n) = 1.0f / sqrtf(FP_RFS(n));
}

/*  FSSCA FPUL,FRn PR=0 1111nnn011111101 */
void sh34_base_device::FSSCA(const uint16_t opcode)
{
	uint32_t n = Rn;

	float angle;

	angle = (((float)(m_sh2_state->m_fpul & 0xFFFF)) / 65536.0f) * 2.0f * (float)M_PI;
	FP_RFS(n) = sinf(angle);
	FP_RFS(n + 1) = cosf(angle);
}

/* FIPR FVm,FVn PR=0 1111nnmm11101101 */
inline void sh34_base_device::FIPR(const uint16_t opcode)
{
	uint32_t n = Rn;

	uint32_t m;
	float ml[4];
	int a;

	m = (n & 3) << 2;
	n = n & 12;
	for (a = 0;a < 4;a++)
		ml[a] = FP_RFS(n + a) * FP_RFS(m + a);
	FP_RFS(n + 3) = ml[0] + ml[1] + ml[2] + ml[3];
}

/* FTRV XMTRX,FVn PR=0 1111nn0111111101 */
void sh34_base_device::FTRV(const uint16_t opcode)
{
	uint32_t n = Rn;

	int i, j;
	float sum[4];

	n = n & 12;
	for (i = 0;i < 4;i++) {
		sum[i] = 0;
		for (j = 0;j < 4;j++)
			sum[i] += FP_XFS((j << 2) + i)*FP_RFS(n + j);
	}
	for (i = 0;i < 4;i++)
		FP_RFS(n + i) = sum[i];
}

inline void sh34_base_device::op1111_0xf13(const uint16_t opcode)
{
	if (opcode & 0x100) {
		if (opcode & 0x200) {
			switch (opcode & 0xC00)
			{
			case 0x000:
				FSCHG();
				break;
			case 0x800:
				FRCHG();
				break;
			default:
				machine().debug_break();
				break;
			}
		}
		else {
			FTRV(opcode);
		}
	}
	else {
		FSSCA(opcode);
	}
}

void sh34_base_device::dbreak(const uint16_t opcode)
{
	machine().debug_break();
}


inline void sh34_base_device::op1111_0x13(uint16_t opcode)
{
	switch ((opcode >> 4) & 0x0f)
	{
	case 0x00:  FSTS(opcode); break;
	case 0x01:  FLDS(opcode); break;
	case 0x02:  FLOAT(opcode); break;
	case 0x03:  FTRC(opcode); break;
	case 0x04:  FNEG(opcode); break;
	case 0x05:  FABS(opcode); break;
	case 0x06:  FSQRT(opcode); break;
	case 0x07:  FSRRA(opcode); break;
	case 0x08:  FLDI0(opcode); break;
	case 0x09:  FLDI1(opcode); break;
	case 0x0a:  FCNVSD(opcode); break;
	case 0x0b:  FCNVDS(opcode); break;
	case 0x0c:  dbreak(opcode); break;
	case 0x0d:  dbreak(opcode); break;
	case 0x0e:  FIPR(opcode); break;
	case 0x0f:  op1111_0xf13(opcode); break;
	}
}


/*****************************************************************************
 *  MAME CPU INTERFACE
 *****************************************************************************/

void sh34_base_device::device_reset()
{
	m_sh2_state->m_spc = 0;
	m_sh2_state->pr = 0;
	m_sh2_state->sr = 0;
	m_sh2_state->m_ssr = 0;
	m_sh2_state->gbr = 0;
	m_sh2_state->vbr = 0;
	m_sh2_state->mach = 0;
	m_sh2_state->macl = 0;
	memset(m_sh2_state->r, 0, sizeof(m_sh2_state->r));
	memset(m_sh2_state->m_rbnk, 0, sizeof(m_sh2_state->m_rbnk));
	m_sh2_state->m_sgr = 0;
	memset(m_sh2_state->m_fr, 0, sizeof(m_sh2_state->m_fr));
	memset(m_sh2_state->m_xf, 0, sizeof(m_sh2_state->m_xf));
	m_sh2_state->ea = 0;
	m_sh2_state->m_delay = 0;
	m_sh2_state->m_cpu_off = 0;
	m_sh2_state->m_pending_irq = 0;
	m_sh2_state->m_test_irq = 0;
	memset(m_exception_priority, 0, sizeof(m_exception_priority));
	memset(m_exception_requesting, 0, sizeof(m_exception_requesting));
	memset(m_m, 0, sizeof(m_m));
	memset(m_sh3internal_upper, 0, sizeof(m_sh3internal_upper));
	memset(m_sh3internal_lower, 0, sizeof(m_sh3internal_lower));
	memset(m_irq_line_state, 0, sizeof(m_irq_line_state));
	m_SH4_TSTR = 0;
	m_SH4_TCNT0 = 0;
	m_SH4_TCNT1 = 0;
	m_SH4_TCNT2 = 0;
	m_SH4_TCR0 = 0;
	m_SH4_TCR1 = 0;
	m_SH4_TCR2 = 0;
	m_SH4_TCOR0 = 0;
	m_SH4_TCOR1 = 0;
	m_SH4_TCOR2 = 0;
	m_SH4_TOCR = 0;
	m_SH4_TCPR2 = 0;
	m_SH4_IPRA = 0;
	m_SH4_IPRC = 0;
	m_SH4_SAR0 = 0;
	m_SH4_SAR1 = 0;
	m_SH4_SAR2 = 0;
	m_SH4_SAR3 = 0;
	m_SH4_DAR0 = 0;
	m_SH4_DAR1 = 0;
	m_SH4_DAR2 = 0;
	m_SH4_DAR3 = 0;
	m_SH4_CHCR0 = 0;
	m_SH4_CHCR1 = 0;
	m_SH4_CHCR2 = 0;
	m_SH4_CHCR3 = 0;
	m_SH4_DMATCR0 = 0;
	m_SH4_DMATCR1 = 0;
	m_SH4_DMATCR2 = 0;
	m_SH4_DMATCR3 = 0;
	m_SH4_DMAOR = 0;
	m_nmi_line_state = 0;
	m_sh2_state->m_frt_input = 0;
	m_internal_irq_vector = 0;
	m_refresh_timer_base = 0;
	memset(m_dma_timer_active, 0, sizeof(m_dma_timer_active));
	memset(m_dma_source, 0, sizeof(m_dma_source));
	memset(m_dma_destination, 0, sizeof(m_dma_destination));
	memset(m_dma_count, 0, sizeof(m_dma_count));
	memset(m_dma_wordsize, 0, sizeof(m_dma_wordsize));
	memset(m_dma_source_increment, 0, sizeof(m_dma_source_increment));
	memset(m_dma_destination_increment, 0, sizeof(m_dma_destination_increment));
	memset(m_dma_mode, 0, sizeof(m_dma_mode));
	m_ioport16_pullup = 0;
	m_ioport16_direction = 0;
	m_ioport4_pullup = 0;
	m_ioport4_direction = 0;

	sh4_default_exception_priorities();

	m_rtc_timer->adjust(attotime::from_hz(128));

	m_sh2_state->pc = 0xa0000000;
	m_sh2_state->m_ppc = m_sh2_state->pc & SH34_AM;
	m_sh2_state->r[15] = RL(4);
	m_sh2_state->sr = 0x700000f0;
	m_sh2_state->m_fpscr = 0x00040001;
	m_sh2_state->m_fpu_sz = (m_sh2_state->m_fpscr & SZ) ? 1 : 0;
	m_sh2_state->m_fpu_pr = (m_sh2_state->m_fpscr & PR) ? 1 : 0;
	m_sh2_state->m_fpul = 0;
	m_sh2_state->m_dbr = 0;

	m_internal_irq_level = -1;
	m_irln = 15;
	m_sh2_state->sleep_mode = 0;

	m_sh4_mmu_enabled = 0;
	m_cache_dirty = true;
}

/*-------------------------------------------------
    sh3_reset - reset the processor
-------------------------------------------------*/

void sh3_base_device::device_reset()
{
	sh34_base_device::device_reset();

	m_SH4_TCOR0 = 0xffffffff;
	m_SH4_TCNT0 = 0xffffffff;
	m_SH4_TCOR1 = 0xffffffff;
	m_SH4_TCNT1 = 0xffffffff;
	m_SH4_TCOR2 = 0xffffffff;
	m_SH4_TCNT2 = 0xffffffff;
}

void sh4_base_device::device_reset()
{
	sh34_base_device::device_reset();

	m_m[RCR2] = 0x09;
	m_SH4_TCOR0 = 0xffffffff;
	m_SH4_TCNT0 = 0xffffffff;
	m_SH4_TCOR1 = 0xffffffff;
	m_SH4_TCNT1 = 0xffffffff;
	m_SH4_TCOR2 = 0xffffffff;
	m_SH4_TCNT2 = 0xffffffff;
}

inline void sh34_base_device::execute_one_0000(const uint16_t opcode)
{
	switch (opcode & 0xff)
	{
	default:
		// fall through to SH2 handlers
		sh_common_execution::execute_one_0000(opcode); break;

	case 0x52:
	case 0x62:
	case 0x43:
	case 0x63:
	case 0xe3:
	case 0x68:
	case 0xe8:
	case 0x4a:
	case 0xca:
		ILLEGAL(); break; // illegal on sh4

	case 0x93:
	case 0xa3:
	case 0xb3:
		TODO(opcode); break;

	case 0x82:
	case 0x92:
	case 0xa2:
	case 0xb2:
	case 0xc2:
	case 0xd2:
	case 0xe2:
	case 0xf2:
		STCRBANK(opcode); break; // sh4 only

	case 0x32:  STCSSR(opcode); break; // sh4 only
	case 0x42:  STCSPC(opcode); break; // sh4 only
	case 0x83:  PREFM(opcode); break; // sh4 only
	case 0xc3:  MOVCAL(opcode); break; // sh4 only

	case 0x38:
	case 0xb8:
		LDTLB(opcode); break; // sh4 only

	case 0x48:
	case 0xc8:
		CLRS(opcode); break; // sh4 only

	case 0x58:
	case 0xd8:
		SETS(opcode); break; // sh4 only

	case 0x3a:
	case 0xba:
		STCSGR(opcode); break; // sh4 only

	case 0x5a:
	case 0xda:
		STSFPUL(opcode); break; // sh4 only

	case 0x6a:
	case 0xea:
		STSFPSCR(opcode); break; // sh4 only

	case 0x7a:
	case 0xfa:
		STCDBR(opcode); break; // sh4 only
	}
}

inline void sh34_base_device::execute_one_4000(const uint16_t opcode)
{
	switch (opcode & 0xff)
	{

	default: // LDCMSR (0x0e) has sh2/4 flag difference
		// fall through to SH2 handlers
		sh_common_execution::execute_one_4000(opcode); break;

	case 0x42:
	case 0x46:
	case 0x4a:
	case 0x53:
	case 0x57:
	case 0x5e:
	case 0x63:
	case 0x67:
	case 0x6e:
	case 0x82:
	case 0x86:
	case 0x8a:
	case 0x92:
	case 0x96:
	case 0x9a:
	case 0xa2:
	case 0xa6:
	case 0xaa:
	case 0xc2:
	case 0xc6:
	case 0xca:
	case 0xd2:
	case 0xd6:
	case 0xda:
	case 0xe2:
	case 0xe6:
	case 0xea:
		ILLEGAL(); break; // defined as illegal on SH4

	case 0x0c:
	case 0x1c:
	case 0x2c:
	case 0x3c:
	case 0x4c:
	case 0x5c:
	case 0x6c:
	case 0x7c:
	case 0x8c:
	case 0x9c:
	case 0xac:
	case 0xbc:
	case 0xcc:
	case 0xdc:
	case 0xec:
	case 0xfc:
		SHAD(opcode); break; // sh3/4 only

	case 0x0d:
	case 0x1d:
	case 0x2d:
	case 0x3d:
	case 0x4d:
	case 0x5d:
	case 0x6d:
	case 0x7d:
	case 0x8d:
	case 0x9d:
	case 0xad:
	case 0xbd:
	case 0xcd:
	case 0xdd:
	case 0xed:
	case 0xfd:
		SHLD(opcode); break; // sh3/4 only

	case 0x8e:
	case 0x9e:
	case 0xae:
	case 0xbe:
	case 0xce:
	case 0xde:
	case 0xee:
	case 0xfe:
		LDCRBANK(opcode); break; // sh3/4 only

	case 0x83:
	case 0x93:
	case 0xa3:
	case 0xb3:
	case 0xc3:
	case 0xd3:
	case 0xe3:
	case 0xf3:
		STCMRBANK(opcode); break; // sh3/4 only

	case 0x87:
	case 0x97:
	case 0xa7:
	case 0xb7:
	case 0xc7:
	case 0xd7:
	case 0xe7:
	case 0xf7:
		LDCMRBANK(opcode); break; // sh3/4 only

	case 0x32:  STCMSGR(opcode); break; // sh4 only
	case 0x33:  STCMSSR(opcode); break; // sh4 only
	case 0x37:  LDCMSSR(opcode); break; // sh4 only
	case 0x3e:  LDCSSR(opcode); break; // sh4 only
	case 0x43:  STCMSPC(opcode); break; // sh4 only
	case 0x47:  LDCMSPC(opcode); break; // sh4 only
	case 0x4e:  LDCSPC(opcode); break; // sh4 only
	case 0x52:  STSMFPUL(opcode); break; // sh4 only
	case 0x56:  LDSMFPUL(opcode); break; // sh4 only
	case 0x5a:  LDSFPUL(opcode); break; // sh4 only
	case 0x62:  STSMFPSCR(opcode); break; // sh4 only
	case 0x66:  LDSMFPSCR(opcode); break; // sh4 only
	case 0x6a:  LDSFPSCR(opcode); break; // sh4 only
	case 0xf2:  STCMDBR(opcode); break; // sh4 only
	case 0xf6:  LDCMDBR(opcode); break; // sh4 only
	case 0xfa:  LDCDBR(opcode); break; // sh4 only

	}
}

inline void sh34_base_device::execute_one_f000(const uint16_t opcode)
{
	// the SH3 doesn't have these?

	switch (opcode & 0x0f)
	{
	case 0x00:  FADD(opcode); break;
	case 0x01:  FSUB(opcode); break;
	case 0x02:  FMUL(opcode); break;
	case 0x03:  FDIV(opcode); break;
	case 0x04:  FCMP_EQ(opcode); break;
	case 0x05:  FCMP_GT(opcode); break;
	case 0x06:  FMOVS0FR(opcode); break;
	case 0x07:  FMOVFRS0(opcode); break;
	case 0x08:  FMOVMRFR(opcode); break;
	case 0x09:  FMOVMRIFR(opcode); break;
	case 0x0a:  FMOVFRMR(opcode); break;
	case 0x0b:  FMOVFRMDR(opcode); break;
	case 0x0c:  FMOVFR(opcode); break;
	case 0x0d:  op1111_0x13(opcode); break;
	case 0x0e:  FMAC(opcode); break;
	case 0x0f:  dbreak(opcode); break;
	}
}

/* Execute cycles - returns number of cycles actually run */
void sh34_base_device::execute_run()
{
	if ( m_isdrc )
	{
		execute_run_drc();
		return;
	}

	if (m_sh2_state->m_cpu_off)
	{
		m_sh2_state->icount = 0;
		return;
	}

	do
	{
		m_sh2_state->m_ppc = m_sh2_state->pc & SH34_AM;
		debugger_instruction_hook(m_sh2_state->pc & SH34_AM);

		uint16_t opcode;

		if (!m_sh4_mmu_enabled) opcode = m_pr16(m_sh2_state->pc & SH34_AM);
		else opcode = RW(m_sh2_state->pc); // should probably use a different function as this needs to go through the ITLB

		if (m_sh2_state->m_delay)
		{
			m_sh2_state->pc = m_sh2_state->m_delay;
			m_sh2_state->m_delay = 0;
		}
		else
			m_sh2_state->pc += 2;

		execute_one(opcode);

		if (m_sh2_state->m_test_irq && !m_sh2_state->m_delay)
		{
			sh4_check_pending_irq("mame_sh4_execute");
		}

		m_sh2_state->icount--;
	} while (m_sh2_state->icount > 0);
}

void sh3be_device::execute_run()
{
	if ( m_isdrc )
	{
		execute_run_drc();
		return;
	}

	if (m_sh2_state->m_cpu_off)
	{
		m_sh2_state->icount = 0;
		return;
	}

	do
	{
		m_sh2_state->m_ppc = m_sh2_state->pc & SH34_AM;
		debugger_instruction_hook(m_sh2_state->pc & SH34_AM);

		const uint16_t opcode = m_pr16(m_sh2_state->pc & SH34_AM);

		if (m_sh2_state->m_delay)
		{
			m_sh2_state->pc = m_sh2_state->m_delay;
			m_sh2_state->m_delay = 0;
		}
		else
			m_sh2_state->pc += 2;

		execute_one(opcode);

		if (m_sh2_state->m_test_irq && !m_sh2_state->m_delay)
		{
			sh4_check_pending_irq("mame_sh4_execute");
		}

		m_sh2_state->icount--;
	} while (m_sh2_state->icount > 0);
}

void sh4be_device::execute_run()
{
	if ( m_isdrc )
	{
		execute_run_drc();
		return;
	}

	if (m_sh2_state->m_cpu_off)
	{
		m_sh2_state->icount = 0;
		return;
	}

	do
	{
		m_sh2_state->m_ppc = m_sh2_state->pc & SH34_AM;
		debugger_instruction_hook(m_sh2_state->pc & SH34_AM);

		const uint16_t opcode = m_pr16(m_sh2_state->pc & SH34_AM);

		if (m_sh2_state->m_delay)
		{
			m_sh2_state->pc = m_sh2_state->m_delay;
			m_sh2_state->m_delay = 0;
		}
		else
			m_sh2_state->pc += 2;

		execute_one(opcode);

		if (m_sh2_state->m_test_irq && !m_sh2_state->m_delay)
		{
			sh4_check_pending_irq("mame_sh4_execute");
		}

		m_sh2_state->icount--;
	} while (m_sh2_state->icount > 0);
}

void sh4_base_device::device_start()
{
	sh34_base_device::device_start();

	int i;
	for (i = 0;i < 64;i++)
	{
		m_utlb[i].ASID = 0;
		m_utlb[i].VPN = 0;
		m_utlb[i].V = 0;
		m_utlb[i].PPN = 0;
		m_utlb[i].PSZ = 0;
		m_utlb[i].SH = 0;
		m_utlb[i].C = 0;
		m_utlb[i].PPR = 0;
		m_utlb[i].D = 0;
		m_utlb[i].WT = 0;
		m_utlb[i].SA = 0;
		m_utlb[i].TC = 0;
	}

	for (i = 0;i < 64;i++)
	{
		save_item(NAME(m_utlb[i].ASID), i);
		save_item(NAME(m_utlb[i].VPN), i);
		save_item(NAME(m_utlb[i].V), i);
		save_item(NAME(m_utlb[i].PPN), i);
		save_item(NAME(m_utlb[i].PSZ), i);
		save_item(NAME(m_utlb[i].SH), i);
		save_item(NAME(m_utlb[i].C), i);
		save_item(NAME(m_utlb[i].PPR), i);
		save_item(NAME(m_utlb[i].D), i);
		save_item(NAME(m_utlb[i].WT), i);
		save_item(NAME(m_utlb[i].SA), i);
		save_item(NAME(m_utlb[i].TC), i);
	}
}



void sh34_base_device::device_start()
{
	m_isdrc = allow_drc();

	sh_common_execution::device_start();

	for (int i = 0; i < 3; i++)
	{
		m_timer[i] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(sh34_base_device::sh4_timer_callback), this));
		m_timer[i]->adjust(attotime::never, i);
	}

	for (int i = 0; i < 4; i++)
	{
		m_dma_timer[i] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(sh34_base_device::sh4_dmac_callback), this));
		m_dma_timer[i]->adjust(attotime::never, i);
	}

	m_refresh_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(sh34_base_device::sh4_refresh_timer_callback), this));
	m_refresh_timer->adjust(attotime::never);
	m_refresh_timer_base = 0;

	m_rtc_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(sh34_base_device::sh4_rtc_timer_callback), this));
	m_rtc_timer->adjust(attotime::never);

	sh4_parse_configuration();

	m_internal = &space(AS_PROGRAM);
	m_program = &space(AS_PROGRAM);
	m_io = &space(AS_IO);
	if (m_program->endianness() == ENDIANNESS_LITTLE)
	{
		auto cache = m_program->cache<3, 0, ENDIANNESS_LITTLE>();
		m_pr16 = [cache](offs_t address) -> u16 { return cache->read_word(address); };
		if (ENDIANNESS_NATIVE != ENDIANNESS_LITTLE)
			m_prptr = [cache](offs_t address) -> const void * {
				const u16 *ptr = static_cast<u16 *>(cache->read_ptr(address & ~7));
				ptr += (~address >> 1) & 3;
				return ptr;
			};
		else
			m_prptr = [cache](offs_t address) -> const void * {
				const u16 *ptr = static_cast<u16 *>(cache->read_ptr(address & ~7));
				ptr += (address >> 1) & 3;
				return ptr;
			};
	}
	else
	{
		auto cache = m_program->cache<3, 0, ENDIANNESS_BIG>();
		m_pr16 = [cache](offs_t address) -> u16 { return cache->read_word(address); };
		if (ENDIANNESS_NATIVE != ENDIANNESS_BIG)
			m_prptr = [cache](offs_t address) -> const void * {
				const u16 *ptr = static_cast<u16 *>(cache->read_ptr(address & ~7));
				ptr += (~address >> 1) & 3;
				return ptr;
			};
		else
			m_prptr = [cache](offs_t address) -> const void * {
				const u16 *ptr = static_cast<u16 *>(cache->read_ptr(address & ~7));
				ptr += (address >> 1) & 3;
				return ptr;
			};
	}

	sh4_default_exception_priorities();
	m_irln = 15;
	m_sh2_state->m_test_irq = 0;


	save_item(NAME(m_sh2_state->m_spc));
	save_item(NAME(m_sh2_state->m_ssr));
	save_item(NAME(m_sh2_state->m_sgr));
	save_item(NAME(m_sh2_state->m_fpscr));
	save_item(NAME(m_sh2_state->m_rbnk));
	save_item(NAME(m_sh2_state->m_fr));
	save_item(NAME(m_sh2_state->m_xf));

	save_item(NAME(m_sh2_state->m_cpu_off));
	save_item(NAME(m_sh2_state->m_pending_irq));
	save_item(NAME(m_sh2_state->m_test_irq));
	save_item(NAME(m_sh2_state->m_fpul));
	save_item(NAME(m_sh2_state->m_dbr));
	save_item(NAME(m_exception_priority));
	save_item(NAME(m_exception_requesting));
	save_item(NAME(m_irq_line_state));
	save_item(NAME(m_m));
	save_item(NAME(m_SH4_TSTR));
	save_item(NAME(m_SH4_TCNT0));
	save_item(NAME(m_SH4_TCNT1));
	save_item(NAME(m_SH4_TCNT2));
	save_item(NAME(m_SH4_TCR0));
	save_item(NAME(m_SH4_TCR1));
	save_item(NAME(m_SH4_TCR2));
	save_item(NAME(m_SH4_TCOR0));
	save_item(NAME(m_SH4_TCOR1));
	save_item(NAME(m_SH4_TCOR2));
	save_item(NAME(m_SH4_TOCR));
	save_item(NAME(m_SH4_TCPR2));
	save_item(NAME(m_SH4_IPRA));
	save_item(NAME(m_SH4_IPRC));
	save_item(NAME(m_SH4_DAR0));
	save_item(NAME(m_SH4_DAR1));
	save_item(NAME(m_SH4_DAR2));
	save_item(NAME(m_SH4_DAR3));
	save_item(NAME(m_SH4_CHCR0));
	save_item(NAME(m_SH4_CHCR1));
	save_item(NAME(m_SH4_CHCR2));
	save_item(NAME(m_SH4_CHCR3));
	save_item(NAME(m_SH4_DMATCR0));
	save_item(NAME(m_SH4_DMATCR1));
	save_item(NAME(m_SH4_DMATCR2));
	save_item(NAME(m_SH4_DMATCR3));
	save_item(NAME(m_SH4_DMAOR));
	save_item(NAME(m_nmi_line_state));

	save_item(NAME(m_sh2_state->m_frt_input));
	save_item(NAME(m_irln));
	save_item(NAME(m_internal_irq_level));
	save_item(NAME(m_internal_irq_vector));
	save_item(NAME(m_refresh_timer_base));
	save_item(NAME(m_dma_timer_active));
	save_item(NAME(m_dma_source));
	save_item(NAME(m_dma_destination));
	save_item(NAME(m_dma_count));
	save_item(NAME(m_dma_wordsize));
	save_item(NAME(m_dma_source_increment));
	save_item(NAME(m_dma_destination_increment));
	save_item(NAME(m_dma_mode));


	save_item(NAME(m_sh2_state->m_fpu_sz));
	save_item(NAME(m_sh2_state->m_fpu_pr));
	save_item(NAME(m_ioport16_pullup));
	save_item(NAME(m_ioport16_direction));
	save_item(NAME(m_ioport4_pullup));
	save_item(NAME(m_ioport4_direction));
	save_item(NAME(m_sh4_mmu_enabled));
	save_item(NAME(m_sh3internal_upper));
	save_item(NAME(m_sh3internal_lower));

	// Debugger state


	state_add(SH4_DBR, "DBR", m_sh2_state->m_dbr).formatstr("%08X");

	state_add(SH4_R0_BK0, "R0 BK 0", m_sh2_state->m_rbnk[0][0]).formatstr("%08X");
	state_add(SH4_R1_BK0, "R1 BK 0", m_sh2_state->m_rbnk[0][1]).formatstr("%08X");
	state_add(SH4_R2_BK0, "R2 BK 0", m_sh2_state->m_rbnk[0][2]).formatstr("%08X");
	state_add(SH4_R3_BK0, "R3 BK 0", m_sh2_state->m_rbnk[0][3]).formatstr("%08X");
	state_add(SH4_R4_BK0, "R4 BK 0", m_sh2_state->m_rbnk[0][4]).formatstr("%08X");
	state_add(SH4_R5_BK0, "R5 BK 0", m_sh2_state->m_rbnk[0][5]).formatstr("%08X");
	state_add(SH4_R6_BK0, "R6 BK 0", m_sh2_state->m_rbnk[0][6]).formatstr("%08X");
	state_add(SH4_R7_BK0, "R7 BK 0", m_sh2_state->m_rbnk[0][7]).formatstr("%08X");
	state_add(SH4_R0_BK1, "R0 BK 1", m_sh2_state->m_rbnk[1][0]).formatstr("%08X");
	state_add(SH4_R1_BK1, "R1 BK 1", m_sh2_state->m_rbnk[1][1]).formatstr("%08X");
	state_add(SH4_R2_BK1, "R2 BK 1", m_sh2_state->m_rbnk[1][2]).formatstr("%08X");
	state_add(SH4_R3_BK1, "R3 BK 1", m_sh2_state->m_rbnk[1][3]).formatstr("%08X");
	state_add(SH4_R4_BK1, "R4 BK 1", m_sh2_state->m_rbnk[1][4]).formatstr("%08X");
	state_add(SH4_R5_BK1, "R5 BK 1", m_sh2_state->m_rbnk[1][5]).formatstr("%08X");
	state_add(SH4_R6_BK1, "R6 BK 1", m_sh2_state->m_rbnk[1][6]).formatstr("%08X");
	state_add(SH4_R7_BK1, "R7 BK 1", m_sh2_state->m_rbnk[1][7]).formatstr("%08X");
	state_add(SH4_SPC, "SPC", m_sh2_state->m_spc).formatstr("%08X");
	state_add(SH4_SSR, "SSR", m_sh2_state->m_ssr).formatstr("%08X");
	state_add(SH4_SGR, "SGR", m_sh2_state->m_sgr).formatstr("%08X");
	state_add(SH4_FPSCR, "FPSCR", m_sh2_state->m_fpscr).formatstr("%08X");
	state_add(SH4_FPUL, "FPUL", m_sh2_state->m_fpul).formatstr("%08X");

	state_add(SH4_FR0, "FR0", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_FR1, "FR1", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_FR2, "FR2", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_FR3, "FR3", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_FR4, "FR4", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_FR5, "FR5", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_FR6, "FR6", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_FR7, "FR7", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_FR8, "FR8", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_FR9, "FR9", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_FR10, "FR10", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_FR11, "FR11", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_FR12, "FR12", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_FR13, "FR13", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_FR14, "FR14", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_FR15, "FR15", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_XF0, "XF0", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_XF1, "XF1", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_XF2, "XF2", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_XF3, "XF3", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_XF4, "XF4", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_XF5, "XF5", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_XF6, "XF6", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_XF7, "XF7", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_XF8, "XF8", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_XF9, "XF9", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_XF10, "XF10", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_XF11, "XF11", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_XF12, "XF12", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_XF13, "XF13", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_XF14, "XF14", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_XF15, "XF15", m_debugger_temp).callimport().formatstr("%25s");

	state_add(STATE_GENPC, "GENPC", m_debugger_temp).callimport().callexport().noshow();
	//state_add(STATE_GENPCBASE, "CURPC", m_sh2_state->m_ppc).noshow();
	state_add( STATE_GENPCBASE, "CURPC", m_sh2_state->pc ).callimport().noshow();

	for (int regnum = 0; regnum < 16; regnum++)
	{
		m_fs_regmap[regnum] = uml::mem(((float *)(m_sh2_state->m_fr+(regnum))));
	}

	for (int regnum = 0; regnum < 16; regnum+=2)
	{
		m_fd_regmap[regnum] = uml::mem(((double *)(m_sh2_state->m_fr+(regnum))));
	}

	drc_start();
}

void sh34_base_device::state_import(const device_state_entry &entry)
{
#ifdef LSB_FIRST
	uint8_t fpu_xor = m_sh2_state->m_fpu_pr;
#else
	uint8_t fpu_xor = 0;
#endif

	switch (entry.index())
	{
		case STATE_GENPC:
			m_sh2_state->pc = m_debugger_temp;
		case SH4_PC:
			m_sh2_state->m_delay = 0;
			break;

		case SH_SR:
			sh4_exception_recompute();
			sh4_check_pending_irq("sh4_set_info");
			break;

		case SH4_FR0:
			m_sh2_state->m_fr[0 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_FR1:
			m_sh2_state->m_fr[1 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_FR2:
			m_sh2_state->m_fr[2 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_FR3:
			m_sh2_state->m_fr[3 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_FR4:
			m_sh2_state->m_fr[4 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_FR5:
			m_sh2_state->m_fr[5 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_FR6:
			m_sh2_state->m_fr[6 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_FR7:
			m_sh2_state->m_fr[7 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_FR8:
			m_sh2_state->m_fr[8 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_FR9:
			m_sh2_state->m_fr[9 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_FR10:
			m_sh2_state->m_fr[10 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_FR11:
			m_sh2_state->m_fr[11 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_FR12:
			m_sh2_state->m_fr[12 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_FR13:
			m_sh2_state->m_fr[13 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_FR14:
			m_sh2_state->m_fr[14 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_FR15:
			m_sh2_state->m_fr[15 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_XF0:
			m_sh2_state->m_xf[0 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_XF1:
			m_sh2_state->m_xf[1 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_XF2:
			m_sh2_state->m_xf[2 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_XF3:
			m_sh2_state->m_xf[3 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_XF4:
			m_sh2_state->m_xf[4 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_XF5:
			m_sh2_state->m_xf[5 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_XF6:
			m_sh2_state->m_xf[6 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_XF7:
			m_sh2_state->m_xf[7 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_XF8:
			m_sh2_state->m_xf[8 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_XF9:
			m_sh2_state->m_xf[9 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_XF10:
			m_sh2_state->m_xf[10 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_XF11:
			m_sh2_state->m_xf[11 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_XF12:
			m_sh2_state->m_xf[12 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_XF13:
			m_sh2_state->m_xf[13 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_XF14:
			m_sh2_state->m_xf[14 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_XF15:
			m_sh2_state->m_xf[15 ^ fpu_xor] = m_debugger_temp;
			break;
	}
}

void sh34_base_device::state_export(const device_state_entry &entry)
{
	switch (entry.index())
	{
	case STATE_GENPC:
		m_debugger_temp = (m_sh2_state->pc & SH34_AM);
		break;
	}
}

void sh34_base_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
#ifdef LSB_FIRST
	uint8_t fpu_xor = m_sh2_state->m_fpu_pr;
#else
	uint8_t fpu_xor = 0;
#endif

	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format("%s%s%s%s%c%c%d%c%c",
					m_sh2_state->sr & MD ? "MD ":"   ",
					m_sh2_state->sr & sRB ? "RB ":"   ",
					m_sh2_state->sr & BL ? "BL ":"   ",
					m_sh2_state->sr & FD ? "FD ":"   ",
					m_sh2_state->sr & SH_M ? 'M':'.',
					m_sh2_state->sr & SH_Q ? 'Q':'.',
					(m_sh2_state->sr & SH_I) >> 4,
					m_sh2_state->sr & SH_S ? 'S':'.',
					m_sh2_state->sr & SH_T ? 'T':'.');
			break;

		case SH4_FR0:
			str = string_format("%08X %f", m_sh2_state->m_fr[0 ^ fpu_xor], (double)FP_RFS(0 ^ fpu_xor));
			break;

		case SH4_FR1:
			str = string_format("%08X %f", m_sh2_state->m_fr[1 ^ fpu_xor], (double)FP_RFS(1 ^ fpu_xor));
			break;

		case SH4_FR2:
			str = string_format("%08X %f", m_sh2_state->m_fr[2 ^ fpu_xor], (double)FP_RFS(2 ^ fpu_xor));
			break;

		case SH4_FR3:
			str = string_format("%08X %f", m_sh2_state->m_fr[3 ^ fpu_xor], (double)FP_RFS(3 ^ fpu_xor));
			break;

		case SH4_FR4:
			str = string_format("%08X %f", m_sh2_state->m_fr[4 ^ fpu_xor], (double)FP_RFS(4 ^ fpu_xor));
			break;

		case SH4_FR5:
			str = string_format("%08X %f", m_sh2_state->m_fr[5 ^ fpu_xor], (double)FP_RFS(5 ^ fpu_xor));
			break;

		case SH4_FR6:
			str = string_format("%08X %f", m_sh2_state->m_fr[6 ^ fpu_xor], (double)FP_RFS(6 ^ fpu_xor));
			break;

		case SH4_FR7:
			str = string_format("%08X %f", m_sh2_state->m_fr[7 ^ fpu_xor], (double)FP_RFS(7 ^ fpu_xor));
			break;

		case SH4_FR8:
			str = string_format("%08X %f", m_sh2_state->m_fr[8 ^ fpu_xor], (double)FP_RFS(8 ^ fpu_xor));
			break;

		case SH4_FR9:
			str = string_format("%08X %f", m_sh2_state->m_fr[9 ^ fpu_xor], (double)FP_RFS(9 ^ fpu_xor));
			break;

		case SH4_FR10:
			str = string_format("%08X %f", m_sh2_state->m_fr[10 ^ fpu_xor], (double)FP_RFS(10 ^ fpu_xor));
			break;

		case SH4_FR11:
			str = string_format("%08X %f", m_sh2_state->m_fr[11 ^ fpu_xor], (double)FP_RFS(11 ^ fpu_xor));
			break;

		case SH4_FR12:
			str = string_format("%08X %f", m_sh2_state->m_fr[12 ^ fpu_xor], (double)FP_RFS(12 ^ fpu_xor));
			break;

		case SH4_FR13:
			str = string_format("%08X %f", m_sh2_state->m_fr[13 ^ fpu_xor], (double)FP_RFS(13 ^ fpu_xor));
			break;

		case SH4_FR14:
			str = string_format("%08X %f", m_sh2_state->m_fr[14 ^ fpu_xor], (double)FP_RFS(14 ^ fpu_xor));
			break;

		case SH4_FR15:
			str = string_format("%08X %f", m_sh2_state->m_fr[15 ^ fpu_xor], (double)FP_RFS(15 ^ fpu_xor));
			break;

		case SH4_XF0:
			str = string_format("%08X %f", m_sh2_state->m_xf[0 ^ fpu_xor], (double)FP_XFS(0 ^ fpu_xor));
			break;

		case SH4_XF1:
			str = string_format("%08X %f", m_sh2_state->m_xf[1 ^ fpu_xor], (double)FP_XFS(1 ^ fpu_xor));
			break;

		case SH4_XF2:
			str = string_format("%08X %f", m_sh2_state->m_xf[2 ^ fpu_xor], (double)FP_XFS(2 ^ fpu_xor));
			break;

		case SH4_XF3:
			str = string_format("%08X %f", m_sh2_state->m_xf[3 ^ fpu_xor], (double)FP_XFS(3 ^ fpu_xor));
			break;

		case SH4_XF4:
			str = string_format("%08X %f", m_sh2_state->m_xf[4 ^ fpu_xor], (double)FP_XFS(4 ^ fpu_xor));
			break;

		case SH4_XF5:
			str = string_format("%08X %f", m_sh2_state->m_xf[5 ^ fpu_xor], (double)FP_XFS(5 ^ fpu_xor));
			break;

		case SH4_XF6:
			str = string_format("%08X %f", m_sh2_state->m_xf[6 ^ fpu_xor], (double)FP_XFS(6 ^ fpu_xor));
			break;

		case SH4_XF7:
			str = string_format("%08X %f", m_sh2_state->m_xf[7 ^ fpu_xor], (double)FP_XFS(7 ^ fpu_xor));
			break;

		case SH4_XF8:
			str = string_format("%08X %f", m_sh2_state->m_xf[8 ^ fpu_xor], (double)FP_XFS(8 ^ fpu_xor));
			break;

		case SH4_XF9:
			str = string_format("%08X %f", m_sh2_state->m_xf[9 ^ fpu_xor], (double)FP_XFS(9 ^ fpu_xor));
			break;

		case SH4_XF10:
			str = string_format("%08X %f", m_sh2_state->m_xf[10 ^ fpu_xor], (double)FP_XFS(10 ^ fpu_xor));
			break;

		case SH4_XF11:
			str = string_format("%08X %f", m_sh2_state->m_xf[11 ^ fpu_xor], (double)FP_XFS(11 ^ fpu_xor));
			break;

		case SH4_XF12:
			str = string_format("%08X %f", m_sh2_state->m_xf[12 ^ fpu_xor], (double)FP_XFS(12 ^ fpu_xor));
			break;

		case SH4_XF13:
			str = string_format("%08X %f", m_sh2_state->m_xf[13 ^ fpu_xor], (double)FP_XFS(13 ^ fpu_xor));
			break;

		case SH4_XF14:
			str = string_format("%08X %f", m_sh2_state->m_xf[14 ^ fpu_xor], (double)FP_XFS(14 ^ fpu_xor));
			break;

		case SH4_XF15:
			str = string_format("%08X %f", m_sh2_state->m_xf[15 ^ fpu_xor], (double)FP_XFS(15 ^ fpu_xor));
			break;

	}
}

/*
void sh34_base_device::sh4_set_ftcsr_callback(sh4_ftcsr_callback callback)
{
    m_ftcsr_read_callback = callback;
}
*/

const opcode_desc* sh4be_device::get_desclist(offs_t pc)
{
	return m_drcfe->describe_code(pc);
}

void sh4be_device::init_drc_frontend()
{
	m_drcfe = std::make_unique<sh4be_frontend>(this, COMPILE_BACKWARDS_BYTES, COMPILE_FORWARDS_BYTES, SINGLE_INSTRUCTION_MODE ? 1 : COMPILE_MAX_SEQUENCE);
}


const opcode_desc* sh4_device::get_desclist(offs_t pc)
{
	return m_drcfe->describe_code(pc);
}

void sh4_device::init_drc_frontend()
{
	m_drcfe = std::make_unique<sh4_frontend>(this, COMPILE_BACKWARDS_BYTES, COMPILE_FORWARDS_BYTES, SINGLE_INSTRUCTION_MODE ? 1 : COMPILE_MAX_SEQUENCE);
}

const opcode_desc* sh3be_device::get_desclist(offs_t pc)
{
	return m_drcfe->describe_code(pc);
}

void sh3be_device::init_drc_frontend()
{
	m_drcfe = std::make_unique<sh4be_frontend>(this, COMPILE_BACKWARDS_BYTES, COMPILE_FORWARDS_BYTES, SINGLE_INSTRUCTION_MODE ? 1 : COMPILE_MAX_SEQUENCE);
}

const opcode_desc* sh3_device::get_desclist(offs_t pc)
{
	return m_drcfe->describe_code(pc);
}

void sh3_device::init_drc_frontend()
{
	m_drcfe = std::make_unique<sh4_frontend>(this, COMPILE_BACKWARDS_BYTES, COMPILE_FORWARDS_BYTES, SINGLE_INSTRUCTION_MODE ? 1 : COMPILE_MAX_SEQUENCE);
}

/*-------------------------------------------------
    generate_update_cycles - generate code to
    subtract cycles from the icount and generate
    an exception if out
-------------------------------------------------*/

void sh34_base_device::func_CHECKIRQ() { if (m_sh2_state->m_test_irq) sh4_check_pending_irq("mame_sh4_execute"); }
static void cfunc_CHECKIRQ(void *param) { ((sh34_base_device *)param)->func_CHECKIRQ(); };

void sh34_base_device::generate_update_cycles(drcuml_block &block, compiler_state &compiler, uml::parameter param, bool allow_exception)
{
	/* check full interrupts if pending */
	if (compiler.checkints)
	{
		compiler.checkints = false;

		/* param is pc + 2 (the opcode after the current one)
		   as we're calling from opcode handlers here that will point to the current opcode instead
		   but I believe the exception function requires it to point to the next one so update the
		   local copy of the PC variable here for that? */
		UML_MOV(block, mem(&m_sh2_state->pc), param);

	//  save_fast_iregs(block);
		UML_CALLC(block, cfunc_CHECKIRQ, this);
		load_fast_iregs(block);

		/* generate a hash jump via the current mode and PC
		   pc should be pointing to either the exception address
		   or have been left on the next PC set above? */
		UML_HASHJMP(block, 0, mem(&m_sh2_state->pc), *m_nocode);     // hashjmp <mode>,<pc>,nocode
	}

	/* account for cycles */
	if (compiler.cycles > 0)
	{
		UML_SUB(block, mem(&m_sh2_state->icount), mem(&m_sh2_state->icount), MAPVAR_CYCLES);    // sub     icount,icount,cycles
		UML_MAPVAR(block, MAPVAR_CYCLES, 0);                                        // mapvar  cycles,0
		if (allow_exception)
			UML_EXHc(block, COND_S, *m_out_of_cycles, param);
																					// exh     out_of_cycles,nextpc
	}
	compiler.cycles = 0;
}

/*-------------------------------------------------
    static_generate_entry_point - generate a
    static entry point
-------------------------------------------------*/


void sh34_base_device::static_generate_entry_point()
{
	//uml::code_label const skip = 1;

	/* begin generating */
	drcuml_block &block(m_drcuml->begin_block(200));

	/* forward references */
	alloc_handle(m_nocode, "nocode");
	alloc_handle(m_write32, "write32");     // necessary?
	alloc_handle(m_entry, "entry");
	UML_HANDLE(block, *m_entry);                         // handle  entry

	UML_CALLC(block, cfunc_CHECKIRQ, this);
	load_fast_iregs(block);

	/* generate a hash jump via the current mode and PC */
	UML_HASHJMP(block, 0, mem(&m_sh2_state->pc), *m_nocode);     // hashjmp <mode>,<pc>,nocode

	block.end();
}


/*------------------------------------------------------------------
    static_generate_memory_accessor
------------------------------------------------------------------*/

void sh34_base_device::static_generate_memory_accessor(int size, int iswrite, const char *name, uml::code_handle *&handleptr)
{
	/* on entry, address is in I0; data for writes is in I1 */
	/* on exit, read result is in I0 */
	/* routine trashes I0 */
	int label = 1;

	/* begin generating */
	drcuml_block &block(m_drcuml->begin_block(1024));

	/* add a global entry for this */
	alloc_handle(handleptr, name);
	UML_HANDLE(block, *handleptr);                         // handle  *handleptr

#if 0
	if (A >= 0xe0000000)
	{
		m_program->write_word(A, V);
		return;
	}

	if (A >= 0x80000000) // P1/P2/P3 region
	{
		m_program->write_word(A & SH34_AM, V);
	}
	else
	{
		// MMU handling
		m_program->write_word(A & SH34_AM, V);
	}
#endif

	UML_CMP(block, I0, 0xe0000000);
	UML_JMPc(block, COND_AE, label);

	UML_AND(block, I0, I0, SH34_AM);     // and r0, r0, #AM (0x1fffffff)

	UML_LABEL(block, label++);              // label:

	for (auto & elem : m_fastram)
	{
		if (elem.base != nullptr && (!iswrite || !elem.readonly))
		{
			void *fastbase = (uint8_t *)elem.base - elem.start;
			uint32_t skip = label++;
			if (elem.end != 0xffffffff)
			{
				UML_CMP(block, I0, elem.end);   // cmp     i0,end
				UML_JMPc(block, COND_A, skip);                                      // ja      skip
			}
			if (elem.start != 0x00000000)
			{
				UML_CMP(block, I0, elem.start);// cmp     i0,fastram_start
				UML_JMPc(block, COND_B, skip);                                      // jb      skip
			}

			if (!iswrite)
			{
				if (size == 1)
				{
					UML_XOR(block, I0, I0, m_bigendian ? BYTE8_XOR_BE(0) : BYTE8_XOR_LE(0));
					UML_LOAD(block, I0, fastbase, I0, SIZE_BYTE, SCALE_x1);             // load    i0,fastbase,i0,byte
				}
				else if (size == 2)
				{
					UML_XOR(block, I0, I0, m_bigendian ? WORD2_XOR_BE(0) : WORD2_XOR_LE(0));
					UML_LOAD(block, I0, fastbase, I0, SIZE_WORD, SCALE_x1);         // load    i0,fastbase,i0,word_x1
				}
				else if (size == 4)
				{

					UML_XOR(block, I0, I0, m_bigendian ? DWORD_XOR_BE(0) : DWORD_XOR_LE(0));
					UML_LOAD(block, I0, fastbase, I0, SIZE_DWORD, SCALE_x1);            // load    i0,fastbase,i0,dword_x1
				}
				UML_RET(block);                                                     // ret
			}
			else
			{
				if (size == 1)
				{
					UML_XOR(block, I0, I0, m_bigendian ? BYTE8_XOR_BE(0) : BYTE8_XOR_LE(0));
					UML_STORE(block, fastbase, I0, I1, SIZE_BYTE, SCALE_x1);// store   fastbase,i0,i1,byte
				}
				else if (size == 2)
				{
					UML_XOR(block, I0, I0, m_bigendian ? WORD2_XOR_BE(0) : WORD2_XOR_LE(0));
					UML_STORE(block, fastbase, I0, I1, SIZE_WORD, SCALE_x1);// store   fastbase,i0,i1,word_x1
				}
				else if (size == 4)
				{
					UML_XOR(block, I0, I0, m_bigendian ? DWORD_XOR_BE(0) : DWORD_XOR_LE(0));
					UML_STORE(block, fastbase, I0, I1, SIZE_DWORD, SCALE_x1);       // store   fastbase,i0,i1,dword_x1
				}
				UML_RET(block);                                                     // ret
			}

			UML_LABEL(block, skip);                                             // skip:
		}
	}

	if (iswrite)
	{
		switch (size)
		{
			case 1:
				UML_WRITE(block, I0, I1, SIZE_BYTE, SPACE_PROGRAM); // write r0, r1, program_byte
				break;

			case 2:
				UML_WRITE(block, I0, I1, SIZE_WORD, SPACE_PROGRAM); // write r0, r1, program_word
				break;

			case 4:
				UML_WRITE(block, I0, I1, SIZE_DWORD, SPACE_PROGRAM);    // write r0, r1, program_dword
				break;
		}
	}
	else
	{
		switch (size)
		{
			case 1:
				UML_READ(block, I0, I0, SIZE_BYTE, SPACE_PROGRAM);  // read r0, program_byte
				break;

			case 2:
				UML_READ(block, I0, I0, SIZE_WORD, SPACE_PROGRAM);  // read r0, program_word
				break;

			case 4:
				UML_READ(block, I0, I0, SIZE_DWORD, SPACE_PROGRAM); // read r0, program_dword
				break;
		}
	}

	UML_RET(block);                         // ret

	block.end();
}

bool sh34_base_device::generate_group_0(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	switch (opcode & 0xff)
	{
	default:
		// fall through to SH2 handlers
		return sh_common_execution::generate_group_0(block, compiler, desc, opcode, in_delay_slot, ovrpc);

	case 0x52:
	case 0x62:
	case 0x43:
	case 0x63:
	case 0xe3:
	case 0x68:
	case 0xe8:
	case 0x4a:
	case 0xca:
		return true; // ILLEGAL(); break; // illegal on sh4

	case 0x93:
	case 0xa3:
	case 0xb3:
		return true; // TODO(opcode); break;

	case 0x82:
	case 0x92:
	case 0xa2:
	case 0xb2:
	case 0xc2:
	case 0xd2:
	case 0xe2:
	case 0xf2:
		return generate_group_0_STCRBANK(block, compiler, desc, opcode, in_delay_slot, ovrpc);

	case 0x32: return generate_group_0_STCSSR(block, compiler, desc, opcode, in_delay_slot, ovrpc); // sh4 only
	case 0x42: return generate_group_0_STCSPC(block, compiler, desc, opcode, in_delay_slot, ovrpc); // sh4 only
	case 0x83: return generate_group_0_PREFM(block, compiler, desc, opcode, in_delay_slot, ovrpc);  // sh4 only
	case 0xc3: return generate_group_0_MOVCAL(block, compiler, desc, opcode, in_delay_slot, ovrpc); // sh4 only

	case 0x38:
	case 0xb8:
		return generate_group_0_LDTLB(block, compiler, desc, opcode, in_delay_slot, ovrpc); // sh4 only

	case 0x48:
	case 0xc8:
		return generate_group_0_CLRS(block, compiler, desc, opcode, in_delay_slot, ovrpc); // sh4 only

	case 0x58:
	case 0xd8:
		return generate_group_0_SETS(block, compiler, desc, opcode, in_delay_slot, ovrpc); // sh4 only

	case 0x3a:
	case 0xba:
		return generate_group_0_STCSGR(block, compiler, desc, opcode, in_delay_slot, ovrpc); // sh4 only

	case 0x5a:
	case 0xda:
		return generate_group_0_STSFPUL(block, compiler, desc, opcode, in_delay_slot, ovrpc); // sh4 only

	case 0x6a:
	case 0xea:
		return generate_group_0_STSFPSCR(block, compiler, desc, opcode, in_delay_slot, ovrpc); // sh4 only

	case 0x7a:
	case 0xfa:
		return generate_group_0_STCDBR(block, compiler, desc, opcode, in_delay_slot, ovrpc); // sh4 only
	}

	return false;
}

void sh34_base_device::func_STCRBANK() { STCRBANK(m_sh2_state->arg0); }
static void cfunc_STCRBANK(void *param) { ((sh34_base_device *)param)->func_STCRBANK(); };

bool sh34_base_device::generate_group_0_STCRBANK(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_STCRBANK, this);
	load_fast_iregs(block);
	return true;
}

bool sh34_base_device::generate_group_0_STCSSR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	UML_MOV(block, R32(Rn), mem(&m_sh2_state->m_ssr));
	return true;
}

bool sh34_base_device::generate_group_0_STCSPC(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	UML_MOV(block, R32(Rn), mem(&m_sh2_state->m_spc));
	return true;
}

void sh34_base_device::func_PREFM() { PREFM(m_sh2_state->arg0); }
static void cfunc_PREFM(void *param) { ((sh34_base_device *)param)->func_PREFM(); };

bool sh34_base_device::generate_group_0_PREFM(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_PREFM, this);
	load_fast_iregs(block);
	return true;
}

bool sh34_base_device::generate_group_0_MOVCAL(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	UML_MOV(block, I0, R32(Rn))
	SETEA(0);
	UML_MOV(block, I1, R32(0));
	UML_CALLH(block, *m_write32);
	return true;
}

void sh34_base_device::func_LDTLB() { LDTLB(m_sh2_state->arg0); }
static void cfunc_LDTLB(void *param) { ((sh34_base_device *)param)->func_LDTLB(); };

bool sh34_base_device::generate_group_0_LDTLB(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_LDTLB, this);
	load_fast_iregs(block);
	return true;
}

void sh34_base_device::func_CLRS() { CLRS(m_sh2_state->arg0); }
static void cfunc_CLRS(void *param) { ((sh34_base_device *)param)->func_CLRS(); };

bool sh34_base_device::generate_group_0_CLRS(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_CLRS, this);
	load_fast_iregs(block);
	return true;
}

void sh34_base_device::func_SETS() { SETS(m_sh2_state->arg0); }
static void cfunc_SETS(void *param) { ((sh34_base_device *)param)->func_SETS(); };

bool sh34_base_device::generate_group_0_SETS(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_SETS, this);
	load_fast_iregs(block);
	return true;
}

bool sh34_base_device::generate_group_0_STCSGR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	UML_MOV(block, R32(Rn), mem(&m_sh2_state->m_sgr));
	return true;

}

bool sh34_base_device::generate_group_0_STSFPUL(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	UML_MOV(block, R32(Rn), mem(&m_sh2_state->m_fpul));
	return true;
}

bool sh34_base_device::generate_group_0_STSFPSCR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	UML_AND(block, I0, 0x003FFFFF, mem(&m_sh2_state->m_fpscr));
	UML_MOV(block, R32(Rn), I0);
	return true;
}

bool sh34_base_device::generate_group_0_STCDBR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	UML_MOV(block, R32(Rn), mem(&m_sh2_state->m_dbr));
	return true;
}

void sh34_base_device::func_RTE() { RTE(); }
static void cfunc_RTE(void *param) { ((sh34_base_device *)param)->func_RTE();  };

bool sh34_base_device::generate_group_0_RTE(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	generate_delay_slot(block, compiler, desc, 0xffffffff);
	save_fast_iregs(block);
	UML_CALLC(block, cfunc_RTE, this);
	load_fast_iregs(block);

	compiler.checkints = true;

	UML_MOV(block, mem(&m_sh2_state->pc), mem(&m_sh2_state->m_delay));
	generate_update_cycles(block, compiler, uml::mem(&m_sh2_state->ea), true);  // <subtract cycles>
	UML_HASHJMP(block, 0, mem(&m_sh2_state->pc), *m_nocode); // and jump to the "resume PC"
	return true;
}

void sh34_base_device::func_TRAPA() { TRAPA(m_sh2_state->arg0 & 0xff); }
static void cfunc_TRAPA(void *param) { ((sh34_base_device *)param)->func_TRAPA(); };

bool sh34_base_device::generate_group_12_TRAPA(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_MOV(block, mem(&m_sh2_state->pc), desc->pc + 2); // copy the PC because we need to use it
	UML_CALLC(block, cfunc_TRAPA, this);
	load_fast_iregs(block);
	UML_HASHJMP(block, 0, mem(&m_sh2_state->pc), *m_nocode);
	return true;
}

void sh34_base_device::func_LDCSR() { LDCSR(m_sh2_state->arg0); }
static void cfunc_LDCSR(void *param) { ((sh34_base_device *)param)->func_LDCSR(); };

bool sh34_base_device::generate_group_4_LDCSR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_LDCSR, this);
	load_fast_iregs(block);

	compiler.checkints = true;

	return true;
}

void sh34_base_device::func_LDCMSR() { LDCMSR(m_sh2_state->arg0); }
static void cfunc_LDCMSR(void *param) { ((sh34_base_device *)param)->func_LDCMSR(); };

bool sh34_base_device::generate_group_4_LDCMSR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_LDCMSR, this);
	load_fast_iregs(block);

	compiler.checkints = true;
	if (!in_delay_slot)
		generate_update_cycles(block, compiler, desc->pc + 2, true);

	return true;
}

void sh34_base_device::func_SHAD() { SHAD(m_sh2_state->arg0); }
static void cfunc_SHAD(void *param) { ((sh34_base_device *)param)->func_SHAD(); };

bool sh34_base_device::generate_group_4_SHAD(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_SHAD, this);
	load_fast_iregs(block);
	return true;
}

void sh34_base_device::func_SHLD() { SHLD(m_sh2_state->arg0); }
static void cfunc_SHLD(void *param) { ((sh34_base_device *)param)->func_SHLD(); };

bool sh34_base_device::generate_group_4_SHLD(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_SHLD, this);
	load_fast_iregs(block);
	return true;
}

bool sh34_base_device::generate_group_4(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	switch (opcode & 0xff)
	{
	default: // LDCMSR (0x0e) has sh2/4 flag difference
		// fall through to SH2 handlers
		return sh_common_execution::generate_group_4(block, compiler, desc, opcode, in_delay_slot, ovrpc); break;

	case 0x42:
	case 0x46:
	case 0x4a:
	case 0x53:
	case 0x57:
	case 0x5e:
	case 0x63:
	case 0x67:
	case 0x6e:
	case 0x82:
	case 0x86:
	case 0x8a:
	case 0x92:
	case 0x96:
	case 0x9a:
	case 0xa2:
	case 0xa6:
	case 0xaa:
	case 0xc2:
	case 0xc6:
	case 0xca:
	case 0xd2:
	case 0xd6:
	case 0xda:
	case 0xe2:
	case 0xe6:
	case 0xea:
		return true; // ILLEGAL(); break; // defined as illegal on SH4

	case 0x0c:
	case 0x1c:
	case 0x2c:
	case 0x3c:
	case 0x4c:
	case 0x5c:
	case 0x6c:
	case 0x7c:
	case 0x8c:
	case 0x9c:
	case 0xac:
	case 0xbc:
	case 0xcc:
	case 0xdc:
	case 0xec:
	case 0xfc:
		return generate_group_4_SHAD(block, compiler, desc, opcode, in_delay_slot, ovrpc); break;

	case 0x0d:
	case 0x1d:
	case 0x2d:
	case 0x3d:
	case 0x4d:
	case 0x5d:
	case 0x6d:
	case 0x7d:
	case 0x8d:
	case 0x9d:
	case 0xad:
	case 0xbd:
	case 0xcd:
	case 0xdd:
	case 0xed:
	case 0xfd:
		return generate_group_4_SHLD(block, compiler, desc, opcode, in_delay_slot, ovrpc); break;

	case 0x8e:
	case 0x9e:
	case 0xae:
	case 0xbe:
	case 0xce:
	case 0xde:
	case 0xee:
	case 0xfe:
		return generate_group_4_LDCRBANK(block, compiler, desc, opcode, in_delay_slot, ovrpc); // sh3/4 only

	case 0x83:
	case 0x93:
	case 0xa3:
	case 0xb3:
	case 0xc3:
	case 0xd3:
	case 0xe3:
	case 0xf3:
		return generate_group_4_STCMRBANK(block, compiler, desc, opcode, in_delay_slot, ovrpc); // sh3/4 only

	case 0x87:
	case 0x97:
	case 0xa7:
	case 0xb7:
	case 0xc7:
	case 0xd7:
	case 0xe7:
	case 0xf7:
		return generate_group_4_LDCMRBANK(block, compiler, desc, opcode, in_delay_slot, ovrpc); // sh4 only

	case 0x32:  return generate_group_4_STCMSGR(block, compiler, desc, opcode, in_delay_slot, ovrpc);
	case 0x33:  return generate_group_4_STCMSSR(block, compiler, desc, opcode, in_delay_slot, ovrpc);
	case 0x37:  return generate_group_4_LDCMSSR(block, compiler, desc, opcode, in_delay_slot, ovrpc);
	case 0x3e:  return generate_group_4_LDCSSR(block, compiler, desc, opcode, in_delay_slot, ovrpc);
	case 0x43:  return generate_group_4_STCMSPC(block, compiler, desc, opcode, in_delay_slot, ovrpc);
	case 0x47:  return generate_group_4_LDCMSPC(block, compiler, desc, opcode, in_delay_slot, ovrpc);
	case 0x4e:  return generate_group_4_LDCSPC(block, compiler, desc, opcode, in_delay_slot, ovrpc);
	case 0x52:  return generate_group_4_STSMFPUL(block, compiler, desc, opcode, in_delay_slot, ovrpc);
	case 0x56:  return generate_group_4_LDSMFPUL(block, compiler, desc, opcode, in_delay_slot, ovrpc);
	case 0x5a:  return generate_group_4_LDSFPUL(block, compiler, desc, opcode, in_delay_slot, ovrpc);
	case 0x62:  return generate_group_4_STSMFPSCR(block, compiler, desc, opcode, in_delay_slot, ovrpc);
	case 0x66:  return generate_group_4_LDSMFPSCR(block, compiler, desc, opcode, in_delay_slot, ovrpc);
	case 0x6a:  return generate_group_4_LDSFPSCR(block, compiler, desc, opcode, in_delay_slot, ovrpc);
	case 0xf2:  return generate_group_4_STCMDBR(block, compiler, desc, opcode, in_delay_slot, ovrpc);
	case 0xf6:  return generate_group_4_LDCMDBR(block, compiler, desc, opcode, in_delay_slot, ovrpc);
	case 0xfa:  return generate_group_4_LDCDBR(block, compiler, desc, opcode, in_delay_slot, ovrpc);
	}
	return false;
}


void sh34_base_device::func_LDCRBANK() { LDCRBANK(m_sh2_state->arg0); }
static void cfunc_LDCRBANK(void *param) { ((sh34_base_device *)param)->func_LDCRBANK(); };

bool sh34_base_device::generate_group_4_LDCRBANK(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_LDCRBANK, this);
	load_fast_iregs(block);
	return true;
}

void sh34_base_device::func_STCMRBANK() { STCMRBANK(m_sh2_state->arg0); }
static void cfunc_STCMRBANK(void *param) { ((sh34_base_device *)param)->func_STCMRBANK(); };

bool sh34_base_device::generate_group_4_STCMRBANK(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_STCMRBANK, this);
	load_fast_iregs(block);
	return true;
}

void sh34_base_device::func_LDCMRBANK() { LDCMRBANK(m_sh2_state->arg0); }
static void cfunc_LDCMRBANK(void *param) { ((sh34_base_device *)param)->func_LDCMRBANK(); };

bool sh34_base_device::generate_group_4_LDCMRBANK(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_LDCMRBANK, this);
	load_fast_iregs(block);
	return true;
}

void sh34_base_device::func_STCMSGR() { STCMSGR(m_sh2_state->arg0); }
static void cfunc_STCMSGR(void *param) { ((sh34_base_device *)param)->func_STCMSGR(); };

bool sh34_base_device::generate_group_4_STCMSGR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_STCMSGR, this);
	load_fast_iregs(block);
	return true;
}

void sh34_base_device::func_STCMSSR() { STCMSSR(m_sh2_state->arg0); }
static void cfunc_STCMSSR(void *param) { ((sh34_base_device *)param)->func_STCMSSR(); };

bool sh34_base_device::generate_group_4_STCMSSR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_STCMSSR, this);
	load_fast_iregs(block);
	return true;
}

void sh34_base_device::func_LDCMSSR() { LDCMSSR(m_sh2_state->arg0); }
static void cfunc_LDCMSSR(void *param) { ((sh34_base_device *)param)->func_LDCMSSR(); };

bool sh34_base_device::generate_group_4_LDCMSSR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_LDCMSSR, this);
	load_fast_iregs(block);
	return true;
}

bool sh34_base_device::generate_group_4_LDCSSR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	UML_MOV(block, mem(&m_sh2_state->m_ssr), R32(Rn));
	return true;
}

void sh34_base_device::func_STCMSPC() { STCMSPC(m_sh2_state->arg0); }
static void cfunc_STCMSPC(void *param) { ((sh34_base_device *)param)->func_STCMSPC(); };

bool sh34_base_device::generate_group_4_STCMSPC(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_STCMSPC, this);
	load_fast_iregs(block);
	return true;
}

void sh34_base_device::func_LDCMSPC() { LDCMSPC(m_sh2_state->arg0); }
static void cfunc_LDCMSPC(void *param) { ((sh34_base_device *)param)->func_LDCMSPC(); };

bool sh34_base_device::generate_group_4_LDCMSPC(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_LDCMSPC, this);
	load_fast_iregs(block);
	return true;
}

bool sh34_base_device::generate_group_4_LDCSPC(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	UML_MOV(block, mem(&m_sh2_state->m_spc), R32(Rn));
	return true;
}

void sh34_base_device::func_STSMFPUL() { STSMFPUL(m_sh2_state->arg0); }
static void cfunc_STSMFPUL(void *param) { ((sh34_base_device *)param)->func_STSMFPUL(); };

bool sh34_base_device::generate_group_4_STSMFPUL(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_STSMFPUL, this);
	load_fast_iregs(block);
	return true;
}

void sh34_base_device::func_LDSMFPUL() { LDSMFPUL(m_sh2_state->arg0); }
static void cfunc_LDSMFPUL(void *param) { ((sh34_base_device *)param)->func_LDSMFPUL(); };

bool sh34_base_device::generate_group_4_LDSMFPUL(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_LDSMFPUL, this);
	load_fast_iregs(block);
	return true;
}

bool sh34_base_device::generate_group_4_LDSFPUL(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	UML_MOV(block, mem(&m_sh2_state->m_fpul), R32(Rn));
	return true;
}

void sh34_base_device::func_STSMFPSCR() { STSMFPSCR(m_sh2_state->arg0); }
static void cfunc_STSMFPSCR(void *param) { ((sh34_base_device *)param)->func_STSMFPSCR(); };

bool sh34_base_device::generate_group_4_STSMFPSCR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_STSMFPSCR, this);
	load_fast_iregs(block);
	return true;
}

void sh34_base_device::func_LDSMFPSCR() { LDSMFPSCR(m_sh2_state->arg0); }
static void cfunc_LDSMFPSCR(void *param) { ((sh34_base_device *)param)->func_LDSMFPSCR(); };


bool sh34_base_device::generate_group_4_LDSMFPSCR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_LDSMFPSCR, this);
	load_fast_iregs(block);
	return true;
}


void sh34_base_device::func_LDSFPSCR() { LDSFPSCR(m_sh2_state->arg0); }
static void cfunc_LDSFPSCR(void *param) { ((sh34_base_device *)param)->func_LDSFPSCR(); };

bool sh34_base_device::generate_group_4_LDSFPSCR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_LDSFPSCR, this);
	load_fast_iregs(block);
	return true;
}

void sh34_base_device::func_STCMDBR() { STCMDBR(m_sh2_state->arg0); }
static void cfunc_STCMDBR(void *param) { ((sh34_base_device *)param)->func_STCMDBR(); };

bool sh34_base_device::generate_group_4_STCMDBR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_STCMDBR, this);
	load_fast_iregs(block);
	return true;
}

void sh34_base_device::func_LDCMDBR() { LDCMDBR(m_sh2_state->arg0); }
static void cfunc_LDCMDBR(void *param) { ((sh34_base_device *)param)->func_LDCMDBR(); };

bool sh34_base_device::generate_group_4_LDCMDBR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_LDCMDBR, this);
	load_fast_iregs(block);
	return true;
}

void sh34_base_device::func_LDCDBR() { LDCDBR(m_sh2_state->arg0); }
static void cfunc_LDCDBR(void *param) { ((sh34_base_device *)param)->func_LDCDBR(); };

bool sh34_base_device::generate_group_4_LDCDBR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_LDCDBR, this);
	load_fast_iregs(block);
	return true;
}

bool sh34_base_device::generate_group_15(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	switch (opcode & 0x0f)
	{
	case 0x00:  return generate_group_15_FADD(block, compiler, desc, opcode, in_delay_slot, ovrpc);
	case 0x01:  return generate_group_15_FSUB(block, compiler, desc, opcode, in_delay_slot, ovrpc);
	case 0x02:  return generate_group_15_FMUL(block, compiler, desc, opcode, in_delay_slot, ovrpc);
	case 0x03:  return generate_group_15_FDIV(block, compiler, desc, opcode, in_delay_slot, ovrpc);
	case 0x04:  return generate_group_15_FCMP_EQ(block, compiler, desc, opcode, in_delay_slot, ovrpc);
	case 0x05:  return generate_group_15_FCMP_GT(block, compiler, desc, opcode, in_delay_slot, ovrpc);
	case 0x06:  return generate_group_15_FMOVS0FR(block, compiler, desc, opcode, in_delay_slot, ovrpc);
	case 0x07:  return generate_group_15_FMOVFRS0(block, compiler, desc, opcode, in_delay_slot, ovrpc);
	case 0x08:  return generate_group_15_FMOVMRFR(block, compiler, desc, opcode, in_delay_slot, ovrpc);
	case 0x09:  return generate_group_15_FMOVMRIFR(block, compiler, desc, opcode, in_delay_slot, ovrpc);
	case 0x0a:  return generate_group_15_FMOVFRMR(block, compiler, desc, opcode, in_delay_slot, ovrpc);
	case 0x0b:  return generate_group_15_FMOVFRMDR(block, compiler, desc, opcode, in_delay_slot, ovrpc);
	case 0x0c:  return generate_group_15_FMOVFR(block, compiler, desc, opcode, in_delay_slot, ovrpc);
	case 0x0d:  return generate_group_15_op1111_0x13(block, compiler, desc, opcode, in_delay_slot, ovrpc);
	case 0x0e:  return generate_group_15_FMAC(block, compiler, desc, opcode, in_delay_slot, ovrpc);
	case 0x0f:
		return true;
		//if (opcode == 0xffff) return true;    // atomiswave uses ffff as NOP?
		//return false; // dbreak(opcode); break;
	}
	return false;
}
bool sh34_base_device::generate_group_15_FADD(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	UML_TEST(block, mem(&m_sh2_state->m_fpu_pr), 0);
	UML_JMPc(block, COND_Z, compiler.labelnum);

	UML_FDADD(block, FPD32(Rn), FPD32(Rn), FPD32(Rm));
	UML_JMP(block, compiler.labelnum+1);

	UML_LABEL(block, compiler.labelnum++);  // labelnum:
	UML_FSADD(block, FPS32(Rn), FPS32(Rn), FPS32(Rm));

	UML_LABEL(block, compiler.labelnum++);  // labelnum+1:

	return true;
}

bool sh34_base_device::generate_group_15_FSUB(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	UML_TEST(block, mem(&m_sh2_state->m_fpu_pr), 0);
	UML_JMPc(block, COND_Z, compiler.labelnum);

	UML_FDSUB(block, FPD32(Rn), FPD32(Rn), FPD32(Rm));
	UML_JMP(block, compiler.labelnum+1);

	UML_LABEL(block, compiler.labelnum++);  // labelnum:
	UML_FSSUB(block, FPS32(Rn), FPS32(Rn), FPS32(Rm));

	UML_LABEL(block, compiler.labelnum++);  // labelnum+1:

	return true;
}

bool sh34_base_device::generate_group_15_FMUL(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	UML_TEST(block, mem(&m_sh2_state->m_fpu_pr), 0);
	UML_JMPc(block, COND_Z, compiler.labelnum);

	UML_FDMUL(block, FPD32(Rn), FPD32(Rn), FPD32(Rm));
	UML_JMP(block, compiler.labelnum+1);

	UML_LABEL(block, compiler.labelnum++);  // labelnum:
	UML_FSMUL(block, FPS32(Rn), FPS32(Rn), FPS32(Rm));

	UML_LABEL(block, compiler.labelnum++);  // labelnum+1:

	return true;
}

bool sh34_base_device::generate_group_15_FDIV(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	UML_TEST(block, mem(&m_sh2_state->m_fpu_pr), 0);
	UML_JMPc(block, COND_Z, compiler.labelnum);

	UML_FDDIV(block, FPD32(Rn), FPD32(Rn), FPD32(Rm));
	UML_JMP(block, compiler.labelnum+1);

	UML_LABEL(block, compiler.labelnum++);  // labelnum:
	UML_FSDIV(block, FPS32(Rn), FPS32(Rn), FPS32(Rm));

	UML_LABEL(block, compiler.labelnum++);  // labelnum+1:

	return true;
}

bool sh34_base_device::generate_group_15_FCMP_EQ(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	UML_TEST(block, uml::mem(&m_sh2_state->m_fpu_pr), 0);
	UML_JMPc(block, COND_Z, compiler.labelnum);

	UML_FDCMP(block, FPD32(Rm & 14), FPD32(Rn & 14));
	UML_SETc(block, COND_Z, I0);
	UML_ROLINS(block, uml::mem(&m_sh2_state->sr), I0, T_SHIFT, SH_T);

	UML_JMP(block, compiler.labelnum+1);

	UML_LABEL(block, compiler.labelnum++);  // labelnum:

	UML_FSCMP(block, FPS32(Rm), FPS32(Rn));
	UML_SETc(block, COND_Z, I0);
	UML_ROLINS(block, uml::mem(&m_sh2_state->sr), I0, T_SHIFT, SH_T);

	UML_LABEL(block, compiler.labelnum++);  // labelnum+1:
	return true;
}

bool sh34_base_device::generate_group_15_FCMP_GT(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	UML_TEST(block, uml::mem(&m_sh2_state->m_fpu_pr), 0);
	UML_JMPc(block, COND_Z, compiler.labelnum);

	UML_FDCMP(block, FPD32(Rm & 14), FPD32(Rn & 14));
	UML_SETc(block, COND_C, I0);
	UML_ROLINS(block, uml::mem(&m_sh2_state->sr), I0, T_SHIFT, SH_T);

	UML_JMP(block, compiler.labelnum+1);

	UML_LABEL(block, compiler.labelnum++);  // labelnum:

	UML_FSCMP(block, FPS32(Rm), FPS32(Rn));
	UML_SETc(block, COND_C, I0);
	UML_ROLINS(block, uml::mem(&m_sh2_state->sr), I0, T_SHIFT, SH_T);

	UML_LABEL(block, compiler.labelnum++);  // labelnum+1:
	return true;
}

void sh34_base_device::func_FMOVS0FR() { FMOVS0FR(m_sh2_state->arg0); }
static void cfunc_FMOVS0FR(void *param) { ((sh34_base_device *)param)->func_FMOVS0FR(); };

bool sh34_base_device::generate_group_15_FMOVS0FR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_FMOVS0FR, this);
	load_fast_iregs(block);
	return true;
}

void sh34_base_device::func_FMOVFRS0() { FMOVFRS0(m_sh2_state->arg0); }
static void cfunc_FMOVFRS0(void *param) { ((sh34_base_device *)param)->func_FMOVFRS0(); };

bool sh34_base_device::generate_group_15_FMOVFRS0(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_FMOVFRS0, this);
	load_fast_iregs(block);
	return true;
}

void sh34_base_device::func_FMOVMRFR() { FMOVMRFR(m_sh2_state->arg0); }
static void cfunc_FMOVMRFR(void *param) { ((sh34_base_device *)param)->func_FMOVMRFR(); };

bool sh34_base_device::generate_group_15_FMOVMRFR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_FMOVMRFR, this);
	load_fast_iregs(block);
	return true;
}

void sh34_base_device::func_FMOVMRIFR() { FMOVMRIFR(m_sh2_state->arg0); }
static void cfunc_FMOVMRIFR(void *param) { ((sh34_base_device *)param)->func_FMOVMRIFR(); };

bool sh34_base_device::generate_group_15_FMOVMRIFR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_FMOVMRIFR, this);
	load_fast_iregs(block);
	return true;
}

void sh34_base_device::func_FMOVFRMR() { FMOVFRMR(m_sh2_state->arg0); }
static void cfunc_FMOVFRMR(void *param) { ((sh34_base_device *)param)->func_FMOVFRMR(); };

bool sh34_base_device::generate_group_15_FMOVFRMR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_FMOVFRMR, this);
	load_fast_iregs(block);
	return true;
}

void sh34_base_device::func_FMOVFRMDR() { FMOVFRMDR(m_sh2_state->arg0); }
static void cfunc_FMOVFRMDR(void *param) { ((sh34_base_device *)param)->func_FMOVFRMDR(); };

bool sh34_base_device::generate_group_15_FMOVFRMDR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_FMOVFRMDR, this);
	load_fast_iregs(block);
	return true;
}

void sh34_base_device::func_FMOVFR() { FMOVFR(m_sh2_state->arg0); }
static void cfunc_FMOVFR(void *param) { ((sh34_base_device *)param)->func_FMOVFR(); };

bool sh34_base_device::generate_group_15_FMOVFR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_FMOVFR, this);
	load_fast_iregs(block);
	return true;
}

bool sh34_base_device::generate_group_15_FMAC(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	UML_TEST(block, mem(&m_sh2_state->m_fpu_pr), 0);
	UML_JMPc(block, COND_NZ, compiler.labelnum);

	UML_FSMUL(block, F0, FPS32(0), FPS32(Rm));
	UML_FSADD(block, FPS32(Rn), F0, FPS32(Rn));

	UML_LABEL(block, compiler.labelnum++);
	return true;
}

bool sh34_base_device::generate_group_15_op1111_0x13(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	switch ((opcode >> 4) & 0x0f)
	{
	case 0x00:  return generate_group_15_op1111_0x13_FSTS(block, compiler, desc, opcode, in_delay_slot, ovrpc); // FSTS(opcode); break;
	case 0x01:  return generate_group_15_op1111_0x13_FLDS(block, compiler, desc, opcode, in_delay_slot, ovrpc); // FLDS(opcode); break;
	case 0x02:  return generate_group_15_op1111_0x13_FLOAT(block, compiler, desc, opcode, in_delay_slot, ovrpc); // FLOAT(opcode); break;
	case 0x03:  return generate_group_15_op1111_0x13_FTRC(block, compiler, desc, opcode, in_delay_slot, ovrpc); // FTRC(opcode); break;
	case 0x04:  return generate_group_15_op1111_0x13_FNEG(block, compiler, desc, opcode, in_delay_slot, ovrpc); // FNEG(opcode); break;
	case 0x05:  return generate_group_15_op1111_0x13_FABS(block, compiler, desc, opcode, in_delay_slot, ovrpc); // FABS(opcode); break;
	case 0x06:  return generate_group_15_op1111_0x13_FSQRT(block, compiler, desc, opcode, in_delay_slot, ovrpc); // FSQRT(opcode); break;
	case 0x07:  return generate_group_15_op1111_0x13_FSRRA(block, compiler, desc, opcode, in_delay_slot, ovrpc); // FSRRA(opcode); break;
	case 0x08:  return generate_group_15_op1111_0x13_FLDI0(block, compiler, desc, opcode, in_delay_slot, ovrpc); // FLDI0(opcode); break;
	case 0x09:  return generate_group_15_op1111_0x13_FLDI1(block, compiler, desc, opcode, in_delay_slot, ovrpc); // FLDI1(opcode); break;
	case 0x0a:  return generate_group_15_op1111_0x13_FCNVSD(block, compiler, desc, opcode, in_delay_slot, ovrpc); // FCNVSD(opcode); break;
	case 0x0b:  return generate_group_15_op1111_0x13_FCNVDS(block, compiler, desc, opcode, in_delay_slot, ovrpc); // FCNVDS(opcode); break;
	case 0x0c:  return false; // dbreak(opcode); break;
	case 0x0d:  return false; //dbreak(opcode); break;
	case 0x0e:  return generate_group_15_op1111_0x13_FIPR(block, compiler, desc, opcode, in_delay_slot, ovrpc); // FIPR(opcode); break;
	case 0x0f:  return generate_group_15_op1111_0x13_op1111_0xf13(block, compiler, desc, opcode, in_delay_slot, ovrpc); // op1111_0xf13(opcode); break;
	}
	return false;
}

bool sh34_base_device::generate_group_15_op1111_0x13_FSTS(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
#ifdef LSB_FIRST
	UML_XOR(block, I0, Rn, uml::mem(&m_sh2_state->m_fpu_pr));
	UML_STORE(block, m_sh2_state->m_fr, I0, uml::mem(&m_sh2_state->m_fpul), SIZE_DWORD, SCALE_x4);
#else
	UML_MOV(block, FPS32(Rn), uml::mem(&m_sh2_state->m_fpul));
#endif
	return true;
}

void sh34_base_device::func_FLDS() { FLDS(m_sh2_state->arg0); }
static void cfunc_FLDS(void *param) { ((sh34_base_device *)param)->func_FLDS(); };

bool sh34_base_device::generate_group_15_op1111_0x13_FLDS(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_FLDS, this);
	load_fast_iregs(block);
	return true;
}

bool sh34_base_device::generate_group_15_op1111_0x13_FLOAT(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	UML_TEST(block, uml::mem(&m_sh2_state->m_fpu_pr), 0);
	UML_JMPc(block, COND_Z, compiler.labelnum);

	UML_FDFRINT(block, FPD32(Rn & 14), uml::mem(&m_sh2_state->m_fpul), SIZE_DWORD);
	UML_JMP(block, compiler.labelnum+1);

	UML_LABEL(block, compiler.labelnum++);  // labelnum:

	UML_FSFRINT(block, FPS32(Rn), uml::mem(&m_sh2_state->m_fpul), SIZE_DWORD);

	UML_LABEL(block, compiler.labelnum++);  // labelnum+1:

	return true;
}

bool sh34_base_device::generate_group_15_op1111_0x13_FTRC(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	UML_TEST(block, uml::mem(&m_sh2_state->m_fpu_pr), 0);
	UML_JMPc(block, COND_Z, compiler.labelnum);

	UML_FDTOINT(block, uml::mem(&m_sh2_state->m_fpul), FPD32(Rn & 14), SIZE_DWORD, ROUND_TRUNC);
	UML_JMP(block, compiler.labelnum+1);

	UML_LABEL(block, compiler.labelnum++);  // labelnum:

	UML_FSTOINT(block, uml::mem(&m_sh2_state->m_fpul), FPS32(Rn), SIZE_DWORD, ROUND_TRUNC);

	UML_LABEL(block, compiler.labelnum++);  // labelnum+1:
	return true;
}

bool sh34_base_device::generate_group_15_op1111_0x13_FNEG(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	UML_MOV(block, I0, 0);

	UML_TEST(block, mem(&m_sh2_state->m_fpu_pr), 0);
	UML_JMPc(block, COND_Z, compiler.labelnum);

	UML_FDFRINT(block, F1, I0, SIZE_DWORD);
	UML_FDSUB(block, FPD32(Rn), F1, FPD32(Rn));

	UML_JMP(block, compiler.labelnum+1);

	UML_LABEL(block, compiler.labelnum++);  // labelnum:

	UML_FSFRINT(block, F1, I0, SIZE_DWORD);
	UML_FSSUB(block, FPS32(Rn), F1, FPS32(Rn));

	UML_LABEL(block, compiler.labelnum++);  // labelnum+1:
	return true;
}

bool sh34_base_device::generate_group_15_op1111_0x13_FABS(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	UML_TEST(block, mem(&m_sh2_state->m_fpu_pr), 0);
	UML_JMPc(block, COND_Z, compiler.labelnum);

#ifdef LSB_FIRST
	UML_AND(block, FPS32(((Rn&14)|1)), FPS32(((Rn&14)|1)), 0x7fffffff);
#else
	UML_AND(block, FPS32(Rn&14), FPS32(Rn&14), 0x7fffffff);
#endif

	UML_JMP(block, compiler.labelnum+1);

	UML_LABEL(block, compiler.labelnum++);  // labelnum:

	UML_AND(block, FPS32(Rn), FPS32(Rn), 0x7fffffff);

	UML_LABEL(block, compiler.labelnum++);  // labelnum+1:
	return true;
}

void sh34_base_device::func_FSQRT() { FSQRT(m_sh2_state->arg0); }
static void cfunc_FSQRT(void *param) { ((sh34_base_device *)param)->func_FSQRT(); };

bool sh34_base_device::generate_group_15_op1111_0x13_FSQRT(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_FSQRT, this);
	load_fast_iregs(block);
	return true;
}

void sh34_base_device::func_FSRRA() { FSRRA(m_sh2_state->arg0); }
static void cfunc_FSRRA(void *param) { ((sh34_base_device *)param)->func_FSRRA(); };

bool sh34_base_device::generate_group_15_op1111_0x13_FSRRA(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_FSRRA, this);
	load_fast_iregs(block);
	return true;
}

bool sh34_base_device::generate_group_15_op1111_0x13_FLDI0(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
#ifdef LSB_FIRST
	UML_MOV(block, I0, Rn);
	UML_XOR(block, I0, I0, uml::mem(&m_sh2_state->m_fpu_pr));
	UML_STORE(block, m_sh2_state->m_fr, I0, 0, SIZE_DWORD, SCALE_x4);
#else
	UML_MOV(block, FP_RFS(Rn), 0);
#endif
	return true;
}

bool sh34_base_device::generate_group_15_op1111_0x13_FLDI1(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
#ifdef LSB_FIRST
	UML_MOV(block, I0, Rn);
	UML_XOR(block, I0, I0, uml::mem(&m_sh2_state->m_fpu_pr));
	UML_STORE(block, m_sh2_state->m_fr, I0, 0x3F800000, SIZE_DWORD, SCALE_x4);
#else
	UML_MOV(block, FP_RFS(Rn), 0x3F800000);
#endif
	return true;
}

void sh34_base_device::func_FCNVSD() { FCNVSD(m_sh2_state->arg0); }
static void cfunc_FCNVSD(void *param) { ((sh34_base_device *)param)->func_FCNVSD(); };

bool sh34_base_device::generate_group_15_op1111_0x13_FCNVSD(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_FCNVSD, this);
	load_fast_iregs(block);
	return true;
}

void sh34_base_device::func_FCNVDS() { FCNVDS(m_sh2_state->arg0); }
static void cfunc_FCNVDS(void *param) { ((sh34_base_device *)param)->func_FCNVDS(); };

bool sh34_base_device::generate_group_15_op1111_0x13_FCNVDS(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_FCNVDS, this);
	load_fast_iregs(block);
	return true;
}

void sh34_base_device::func_FIPR() { FIPR(m_sh2_state->arg0); }
static void cfunc_FIPR(void *param) { ((sh34_base_device *)param)->func_FIPR(); };

bool sh34_base_device::generate_group_15_op1111_0x13_FIPR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_FIPR, this);
	load_fast_iregs(block);
	return true;
}

bool sh34_base_device::generate_group_15_op1111_0x13_op1111_0xf13(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	if (opcode & 0x100) {
		if (opcode & 0x200) {
			switch (opcode & 0xC00)
			{
			case 0x000:
				return generate_group_15_op1111_0x13_op1111_0xf13_FSCHG(block, compiler, desc, opcode, in_delay_slot, ovrpc); // FSCHG();
				break;
			case 0x800:
				return generate_group_15_op1111_0x13_op1111_0xf13_FRCHG(block, compiler, desc, opcode, in_delay_slot, ovrpc); // FRCHG();
				break;
			default:
				return false; //machine().debug_break();
				break;
			}
		}
		else {
			return generate_group_15_op1111_0x13_op1111_0xf13_FTRV(block, compiler, desc, opcode, in_delay_slot, ovrpc); // FTRV(opcode);
		}
	}
	else {
		return generate_group_15_op1111_0x13_op1111_0xf13_FSSCA(block, compiler, desc, opcode, in_delay_slot, ovrpc); // FSSCA(opcode);
	}
	return false;
}

bool sh34_base_device::generate_group_15_op1111_0x13_op1111_0xf13_FSCHG(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	UML_XOR(block, I0, uml::mem(&m_sh2_state->m_fpscr), SZ);
	UML_MOV(block, uml::mem(&m_sh2_state->m_fpscr), I0);
	UML_TEST(block, I0, SZ);
	UML_SETc(block, COND_NZ, uml::mem(&m_sh2_state->m_fpu_sz));
	return true;
}

bool sh34_base_device::generate_group_15_op1111_0x13_op1111_0xf13_FRCHG(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	UML_XOR(block, uml::mem(&m_sh2_state->m_fpscr), uml::mem(&m_sh2_state->m_fpscr), FR);

	UML_MOV(block, I0, 0);
	UML_LABEL(block, compiler.labelnum);  // labelnum:

	UML_LOAD(block, I1, m_sh2_state->m_fr, I0, SIZE_DWORD, SCALE_x4);
	UML_LOAD(block, I2, m_sh2_state->m_xf, I0, SIZE_DWORD, SCALE_x4);
	UML_STORE(block, m_sh2_state->m_xf, I0, I1, SIZE_DWORD, SCALE_x4);
	UML_STORE(block, m_sh2_state->m_fr, I0, I2, SIZE_DWORD, SCALE_x4);
	UML_ADD(block, I0, I0, 1);
	UML_CMP(block, I0, 16);
	UML_JMPc(block, COND_NZ, compiler.labelnum);

	compiler.labelnum++;
	return true;
}

void sh34_base_device::func_FTRV() { FTRV(m_sh2_state->arg0); }
static void cfunc_FTRV(void *param) { ((sh34_base_device *)param)->func_FTRV(); };

bool sh34_base_device::generate_group_15_op1111_0x13_op1111_0xf13_FTRV(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_FTRV, this);
	load_fast_iregs(block);
	return true;
}

void sh34_base_device::func_FSSCA() { FSSCA(m_sh2_state->arg0); }
static void cfunc_FSSCA(void *param) { ((sh34_base_device *)param)->func_FSSCA(); };

bool sh34_base_device::generate_group_15_op1111_0x13_op1111_0xf13_FSSCA(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_FSSCA, this);
	load_fast_iregs(block);
	return true;
}

