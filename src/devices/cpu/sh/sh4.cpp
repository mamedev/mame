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

#include "debugger.h"


#if SH4_USE_FASTRAM_OPTIMIZATION
void sh34_base_device::add_fastram(offs_t start, offs_t end, uint8_t readonly, void *base)
{
	if (m_fastram_select < ARRAY_LENGTH(m_fastram))
	{
		m_fastram[m_fastram_select].start = start;
		m_fastram[m_fastram_select].end = end;
		m_fastram[m_fastram_select].readonly = readonly;
		m_fastram[m_fastram_select].base = base;
		m_fastram_select++;
	}
}
#else
void sh34_base_device::add_fastram(offs_t start, offs_t end, uint8_t readonly, void *base)
{
}
#endif


CPU_DISASSEMBLE( sh4 );
CPU_DISASSEMBLE( sh4be );


DEFINE_DEVICE_TYPE(SH3LE, sh3_device,   "sh3le", "SH-3 (little)")
DEFINE_DEVICE_TYPE(SH3BE, sh3be_device, "sh3be", "SH-3 (big)")
DEFINE_DEVICE_TYPE(SH4LE, sh4_device,   "sh4le", "SH-4 (little)")
DEFINE_DEVICE_TYPE(SH4BE, sh4be_device, "sh4be", "SH-4 (big)")


#if 0
/*When OC index mode is off (CCR.OIX = 0)*/
static ADDRESS_MAP_START( sh4_internal_map, AS_PROGRAM, 64, sh4_base_device )
	AM_RANGE(0x1C000000, 0x1C000FFF) AM_RAM AM_MIRROR(0x03FFD000)
	AM_RANGE(0x1C002000, 0x1C002FFF) AM_RAM AM_MIRROR(0x03FFD000)
	AM_RANGE(0xE0000000, 0xE000003F) AM_RAM AM_MIRROR(0x03FFFFC0)
ADDRESS_MAP_END
#endif

/*When OC index mode is on (CCR.OIX = 1)*/
static ADDRESS_MAP_START( sh4_internal_map, AS_PROGRAM, 64, sh4_base_device )
	AM_RANGE(0x1C000000, 0x1C000FFF) AM_RAM AM_MIRROR(0x01FFF000)
	AM_RANGE(0x1E000000, 0x1E000FFF) AM_RAM AM_MIRROR(0x01FFF000)
	AM_RANGE(0xE0000000, 0xE000003F) AM_RAM AM_MIRROR(0x03FFFFC0) // todo: store queues should be write only on DC's SH4, executing PREFM shouldn't cause an actual memory read access!

	AM_RANGE(0xF6000000, 0xF6FFFFFF) AM_READWRITE(sh4_utlb_address_array_r,sh4_utlb_address_array_w)
	AM_RANGE(0xF7000000, 0xF77FFFFF) AM_READWRITE(sh4_utlb_data_array1_r,sh4_utlb_data_array1_w)
	AM_RANGE(0xF7800000, 0xF7FFFFFF) AM_READWRITE(sh4_utlb_data_array2_r,sh4_utlb_data_array2_w)

	AM_RANGE(0xFE000000, 0xFFFFFFFF) AM_READWRITE32(sh4_internal_r, sh4_internal_w, 0xffffffffffffffffU)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sh3_internal_map, AS_PROGRAM, 64, sh3_base_device )
	AM_RANGE(SH3_LOWER_REGBASE, SH3_LOWER_REGEND) AM_READWRITE32(sh3_internal_r, sh3_internal_w, 0xffffffffffffffffU)
	AM_RANGE(SH3_UPPER_REGBASE, SH3_UPPER_REGEND) AM_READWRITE32(sh3_internal_high_r, sh3_internal_high_w, 0xffffffffffffffffU)
ADDRESS_MAP_END


sh34_base_device::sh34_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, endianness_t endianness, address_map_constructor internal)
	: cpu_device(mconfig, type, tag, owner, clock)
	, sh_common_execution()
	, m_program_config("program", endianness, 64, 32, 0, internal)
	, m_io_config("io", endianness, 64, 8)
	, c_md2(0)
	, c_md1(0)
	, c_md0(0)
	, c_md6(0)
	, c_md4(0)
	, c_md3(0)
	, c_md5(0)
	, c_md7(0)
	, c_md8(0)
	, c_clock(0)
	, m_mmuhack(1)
#if SH4_USE_FASTRAM_OPTIMIZATION
	, m_bigendian(endianness == ENDIANNESS_BIG)
	, m_byte_xor(m_bigendian ? BYTE8_XOR_BE(0) : BYTE8_XOR_LE(0))
	, m_word_xor(m_bigendian ? WORD2_XOR_BE(0) : WORD2_XOR_LE(0))
	, m_dword_xor(m_bigendian ? DWORD_XOR_BE(0) : DWORD_XOR_LE(0))
	, m_fastram_select(0)
#endif
{
#if SH4_USE_FASTRAM_OPTIMIZATION
	memset(m_fastram, 0, sizeof(m_fastram));
#endif
}

device_memory_interface::space_config_vector sh34_base_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_IO,      &m_io_config)
	};
}


sh3_base_device::sh3_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, endianness_t endianness)
	: sh34_base_device(mconfig, type, tag, owner, clock, endianness, ADDRESS_MAP_NAME(sh3_internal_map))
{
	m_cpu_type = CPU_TYPE_SH3;
}


sh4_base_device::sh4_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, endianness_t endianness)
	: sh34_base_device(mconfig, type, tag, owner, clock, endianness, ADDRESS_MAP_NAME(sh4_internal_map))
{
	m_cpu_type = CPU_TYPE_SH4;
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


offs_t sh34_base_device::disasm_disassemble(std::ostream &stream, offs_t pc, const uint8_t *oprom, const uint8_t *opram, uint32_t options)
{
	extern CPU_DISASSEMBLE( sh4 );

	return CPU_DISASSEMBLE_NAME(sh4)(this, stream, pc, oprom, opram, options);
}


offs_t sh3be_device::disasm_disassemble(std::ostream &stream, offs_t pc, const uint8_t *oprom, const uint8_t *opram, uint32_t options)
{
	extern CPU_DISASSEMBLE( sh4be );

	return CPU_DISASSEMBLE_NAME(sh4be)(this, stream, pc, oprom, opram, options);
}


offs_t sh4be_device::disasm_disassemble(std::ostream &stream, offs_t pc, const uint8_t *oprom, const uint8_t *opram, uint32_t options)
{
	extern CPU_DISASSEMBLE( sh4be );

	return CPU_DISASSEMBLE_NAME(sh4be)(this, stream, pc, oprom, opram, options);
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
	m_utlb[replace].VPN =  (m_m[PTEH] & 0xfffffc00) >> 10;
//  m_utlb[replace].D =    (m_m[PTEH] & 0x00000200) >> 9; // from PTEL
//  m_utlb[replace].V =    (m_m[PTEH] & 0x00000100) >> 8; // from PTEL
	m_utlb[replace].ASID = (m_m[PTEH] & 0x000000ff) >> 0;
	// these come from PTEL
	m_utlb[replace].PPN = (m_m[PTEL] & 0x1ffffc00) >> 10;
	m_utlb[replace].V =   (m_m[PTEL] & 0x00000100) >> 8;
	m_utlb[replace].PSZ = (m_m[PTEL] & 0x00000080) >> 6;
	m_utlb[replace].PSZ |=(m_m[PTEL] & 0x00000010) >> 4;
	m_utlb[replace].PPR=  (m_m[PTEL] & 0x00000060) >> 5;
	m_utlb[replace].C =   (m_m[PTEL] & 0x00000008) >> 3;
	m_utlb[replace].D =   (m_m[PTEL] & 0x00000004) >> 2;
	m_utlb[replace].SH =  (m_m[PTEL] & 0x00000002) >> 1;
	m_utlb[replace].WT =  (m_m[PTEL] & 0x00000001) >> 0;
	// these come from PTEA
	m_utlb[replace].TC = (m_m[PTEA] & 0x00000008) >> 3;
	m_utlb[replace].SA = (m_m[PTEA] & 0x00000007) >> 0;
}

#if 0
int sign_of(int n)
{
	return(m_fr[n]>>31);
}

void zero(int n,int sign)
{
if (sign == 0)
	m_fr[n] = 0x00000000;
else
	m_fr[n] = 0x80000000;
if ((m_fpscr & PR) == 1)
	m_fr[n+1] = 0x00000000;
}

int data_type_of(int n)
{
uint32_t abs;

	abs = m_fr[n] & 0x7fffffff;
	if ((m_fpscr & PR) == 0) { /* Single-precision */
		if (abs < 0x00800000) {
			if (((m_fpscr & DN) == 1) || (abs == 0x00000000)) {
				if (sign_of(n) == 0) {
					zero(n, 0);
					return(SH4_FPU_PZERO);
				} else {
					zero(n, 1);
					return(SH4_FPU_NZERO);
				}
			} else
				return(SH4_FPU_DENORM);
		} else
			if (abs < 0x7f800000)
				return(SH4_FPU_NORM);
			else
				if (abs == 0x7f800000) {
					if (sign_of(n) == 0)
						return(SH4_FPU_PINF);
					else
						return(SH4_FPU_NINF);
				} else
					if (abs < 0x7fc00000)
						return(SH4_FPU_qNaN);
					else
						return(SH4_FPU_sNaN);
	} else { /* Double-precision */
		if (abs < 0x00100000) {
			if (((m_fpscr & DN) == 1) || ((abs == 0x00000000) && (m_fr[n+1] == 0x00000000))) {
				if(sign_of(n) == 0) {
					zero(n, 0);
					return(SH4_FPU_PZERO);
				} else {
					zero(n, 1);
					return(SH4_FPU_NZERO);
				}
			} else
				return(SH4_FPU_DENORM);
		} else
			if (abs < 0x7ff00000)
				return(SH4_FPU_NORM);
			else
				if ((abs == 0x7ff00000) && (m_fr[n+1] == 0x00000000)) {
					if (sign_of(n) == 0)
						return(SH4_FPU_PINF);
					else
						return(SH4_FPU_NINF);
				} else
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
#if SH4_USE_FASTRAM_OPTIMIZATION
		const offs_t _A = A & SH34_AM;
		for (int ramnum = 0; ramnum < m_fastram_select; ramnum++)
		{
			if (_A < m_fastram[ramnum].start || _A > m_fastram[ramnum].end)
			{
				continue;
			}
			uint8_t *fastbase = (uint8_t*)m_fastram[ramnum].base - m_fastram[ramnum].start;
			return fastbase[_A ^ m_byte_xor];
		}
		return m_program->read_byte(_A);
#else
		return m_program->read_byte(A & SH34_AM);
#endif
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
#if SH4_USE_FASTRAM_OPTIMIZATION
		const offs_t _A = A & SH34_AM;
		for (int ramnum = 0; ramnum < m_fastram_select; ramnum++)
		{
			if (_A < m_fastram[ramnum].start || _A > m_fastram[ramnum].end)
			{
				continue;
			}
			uint8_t *fastbase = (uint8_t*)m_fastram[ramnum].base - m_fastram[ramnum].start;
			return ((uint16_t*)fastbase)[(_A ^ m_word_xor) >> 1];
		}
		return m_program->read_word(_A);
#else
		return m_program->read_word(A & SH34_AM);
#endif
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
#if SH4_USE_FASTRAM_OPTIMIZATION
		const offs_t _A = A & SH34_AM;
		for (int ramnum = 0; ramnum < m_fastram_select; ramnum++)
		{
			if (_A < m_fastram[ramnum].start || _A > m_fastram[ramnum].end)
			{
				continue;
			}
			uint8_t *fastbase = (uint8_t*)m_fastram[ramnum].base - m_fastram[ramnum].start;
			return ((uint32_t*)fastbase)[(_A^m_dword_xor) >> 2];
		}
		return m_program->read_dword(_A);
#else
		return m_program->read_dword(A & SH34_AM);
#endif
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
		m_program->write_byte(A,V);
		return;
	}

	if (A >= 0x80000000) // P1/P2/P3 region
	{
#if SH4_USE_FASTRAM_OPTIMIZATION
		const offs_t _A = A & SH34_AM;
		for (int ramnum = 0; ramnum < m_fastram_select; ramnum++)
		{
			if (m_fastram[ramnum].readonly == true || _A < m_fastram[ramnum].start || _A > m_fastram[ramnum].end)
			{
				continue;
			}
			uint8_t *fastbase = (uint8_t*)m_fastram[ramnum].base - m_fastram[ramnum].start;
			fastbase[_A ^ m_byte_xor] = V;
			return;
		}
		m_program->write_byte(_A, V);
#else
		m_program->write_byte(A & SH34_AM, V);
#endif
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
		m_program->write_word(A,V);
		return;
	}

	if (A >= 0x80000000) // P1/P2/P3 region
	{
#if SH4_USE_FASTRAM_OPTIMIZATION
		const offs_t _A = A & SH34_AM;
		for (int ramnum = 0; ramnum < m_fastram_select; ramnum++)
		{
			if (m_fastram[ramnum].readonly == true || _A < m_fastram[ramnum].start || _A > m_fastram[ramnum].end)
			{
				continue;
			}
			void *fastbase = (uint8_t*)m_fastram[ramnum].base - m_fastram[ramnum].start;
			((uint16_t*)fastbase)[(_A ^ m_word_xor) >> 1] = V;
			return;
		}
		m_program->write_word(_A, V);
#else
		m_program->write_word(A & SH34_AM, V);
#endif
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
		m_program->write_dword(A,V);
		return;
	}

	if (A >= 0x80000000) // P1/P2/P3 region
	{
#if SH4_USE_FASTRAM_OPTIMIZATION
		const offs_t _A = A & SH34_AM;
		for (int ramnum = 0; ramnum < m_fastram_select; ramnum++)
		{
			if (m_fastram[ramnum].readonly == true || _A < m_fastram[ramnum].start || _A > m_fastram[ramnum].end)
			{
				continue;
			}
			void *fastbase = (uint8_t*)m_fastram[ramnum].base - m_fastram[ramnum].start;
			((uint32_t*)fastbase)[(_A ^ m_dword_xor) >> 2] = V;
			return;
		}
		m_program->write_dword(_A, V);
#else
		m_program->write_dword(A & SH34_AM, V);
#endif
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
	WL(m_sh2_state->ea, m_sh2_state->r[0] );
}

inline void sh34_base_device::CLRS(const uint16_t opcode)
{
	m_sh2_state->sr &= ~S;
}

inline void sh34_base_device::SETS(const uint16_t opcode)
{
	m_sh2_state->sr |= S;
}

/*  LDC     Rm,SR */
inline void sh34_base_device::LDCSR(const uint16_t opcode)
{
	uint32_t reg;

	reg = m_sh2_state->r[Rn];
	if ((machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
		sh4_syncronize_register_bank((m_sh2_state->sr & sRB) >> 29);
	if ((m_sh2_state->r[Rn] & sRB) != (m_sh2_state->sr & sRB))
		sh4_change_register_bank(m_sh2_state->r[Rn] & sRB ? 1 : 0);
	m_sh2_state->sr = reg & FLAGS;
	sh4_exception_recompute();
}



/*  LDC.L   @Rm+,SR */
inline void sh34_base_device::LDCMSR(const uint16_t opcode)
{
	uint32_t old;

	old = m_sh2_state->sr;
	m_sh2_state->ea = m_sh2_state->r[Rn];
	m_sh2_state->sr = RL(m_sh2_state->ea ) & FLAGS;
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
	m_sh2_state->m_delay = m_sh2_state->ea = m_spc;
	if ((machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
		sh4_syncronize_register_bank((m_sh2_state->sr & sRB) >> 29);
	if ((m_ssr & sRB) != (m_sh2_state->sr & sRB))
		sh4_change_register_bank(m_ssr & sRB ? 1 : 0);
	m_sh2_state->sr = m_ssr;
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


	m_ssr = m_sh2_state->sr;
	m_spc = m_sh2_state->pc;
	m_sgr = m_sh2_state->r[15];

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

	m_sh2_state->r[Rn] = m_rbnk[m_sh2_state->sr&sRB ? 0 : 1][m & 7];
}

/*  STCMRBANK   Rm_BANK,@-Rn */
inline void sh34_base_device::STCMRBANK(const uint16_t opcode)
{
	uint32_t m = Rm; uint32_t n = Rn;

	m_sh2_state->r[n] -= 4;
	m_sh2_state->ea = m_sh2_state->r[n];
	WL(m_sh2_state->ea, m_rbnk[m_sh2_state->sr&sRB ? 0 : 1][m & 7]);
	m_sh2_state->icount--;
}

/*  STS.L   SGR,@-Rn */
inline void sh34_base_device::STCMSGR(const uint16_t opcode)
{
	uint32_t n = Rn;

	m_sh2_state->r[n] -= 4;
	m_sh2_state->ea = m_sh2_state->r[n];
	WL(m_sh2_state->ea, m_sgr );
}

/*  STS.L   FPUL,@-Rn */
inline void sh34_base_device::STSMFPUL(const uint16_t opcode)
{
	uint32_t n = Rn;

	m_sh2_state->r[n] -= 4;
	m_sh2_state->ea = m_sh2_state->r[n];
	WL(m_sh2_state->ea, m_fpul );
}

/*  STS.L   FPSCR,@-Rn */
inline void sh34_base_device::STSMFPSCR(const uint16_t opcode)
{
	uint32_t n = Rn;

	m_sh2_state->r[n] -= 4;
	m_sh2_state->ea = m_sh2_state->r[n];
	WL(m_sh2_state->ea, m_fpscr & 0x003FFFFF);
}

/*  STC.L   DBR,@-Rn */
inline void sh34_base_device::STCMDBR(const uint16_t opcode)
{
	uint32_t n = Rn;

	m_sh2_state->r[n] -= 4;
	m_sh2_state->ea = m_sh2_state->r[n];
	WL(m_sh2_state->ea, m_dbr );
}

/*  STC.L   SSR,@-Rn */
inline void sh34_base_device::STCMSSR(const uint16_t opcode)
{
	uint32_t n = Rn;

	m_sh2_state->r[n] -= 4;
	m_sh2_state->ea = m_sh2_state->r[n];
	WL(m_sh2_state->ea, m_ssr );
}

/*  STC.L   SPC,@-Rn */
inline void sh34_base_device::STCMSPC(const uint16_t opcode)
{
	uint32_t n = Rn;

	m_sh2_state->r[n] -= 4;
	m_sh2_state->ea = m_sh2_state->r[n];
	WL(m_sh2_state->ea, m_spc );
}

/*  LDS.L   @Rm+,FPUL */
inline void sh34_base_device::LDSMFPUL(const uint16_t opcode)
{
	m_sh2_state->ea = m_sh2_state->r[Rn];
	m_fpul = RL(m_sh2_state->ea );
	m_sh2_state->r[Rn] += 4;
}

/*  LDS.L   @Rm+,FPSCR */
inline void sh34_base_device::LDSMFPSCR(const uint16_t opcode)
{
	uint32_t s;

	s = m_fpscr;
	m_sh2_state->ea = m_sh2_state->r[Rn];
	m_fpscr = RL(m_sh2_state->ea );
	m_fpscr &= 0x003FFFFF;
	m_sh2_state->r[Rn] += 4;
	if ((s & FR) != (m_fpscr & FR))
		sh4_swap_fp_registers();
#ifdef LSB_FIRST
	if ((s & PR) != (m_fpscr & PR))
		sh4_swap_fp_couples();
#endif
	m_fpu_sz = (m_fpscr & SZ) ? 1 : 0;
	m_fpu_pr = (m_fpscr & PR) ? 1 : 0;
}

/*  LDC.L   @Rm+,DBR */
inline void sh34_base_device::LDCMDBR(const uint16_t opcode)
{
	m_sh2_state->ea = m_sh2_state->r[Rn];
	m_dbr = RL(m_sh2_state->ea );
	m_sh2_state->r[Rn] += 4;
}

/*  LDC.L   @Rn+,Rm_BANK */
inline void sh34_base_device::LDCMRBANK(const uint16_t opcode)
{
	uint32_t m = Rm; uint32_t n = Rn;

	m_sh2_state->ea = m_sh2_state->r[n];
	m_rbnk[m_sh2_state->sr&sRB ? 0 : 1][m & 7] = RL(m_sh2_state->ea );
	m_sh2_state->r[n] += 4;
}

/*  LDC.L   @Rm+,SSR */
inline void sh34_base_device::LDCMSSR(const uint16_t opcode)
{
	m_sh2_state->ea = m_sh2_state->r[Rn];
	m_ssr = RL(m_sh2_state->ea );
	m_sh2_state->r[Rn] += 4;
}

/*  LDC.L   @Rm+,SPC */
inline void sh34_base_device::LDCMSPC(const uint16_t opcode)
{
	m_sh2_state->ea = m_sh2_state->r[Rn];
	m_spc = RL(m_sh2_state->ea );
	m_sh2_state->r[Rn] += 4;
}

/*  LDS     Rm,FPUL */
inline void sh34_base_device::LDSFPUL(const uint16_t opcode)
{
	m_fpul = m_sh2_state->r[Rn];
}

/*  LDS     Rm,FPSCR */
inline void sh34_base_device::LDSFPSCR(const uint16_t opcode)
{
	uint32_t s;

	s = m_fpscr;
	m_fpscr = m_sh2_state->r[Rn] & 0x003FFFFF;
	if ((s & FR) != (m_fpscr & FR))
		sh4_swap_fp_registers();
#ifdef LSB_FIRST
	if ((s & PR) != (m_fpscr & PR))
		sh4_swap_fp_couples();
#endif
	m_fpu_sz = (m_fpscr & SZ) ? 1 : 0;
	m_fpu_pr = (m_fpscr & PR) ? 1 : 0;
}

/*  LDC     Rm,DBR */
inline void sh34_base_device::LDCDBR(const uint16_t opcode)
{
	m_dbr = m_sh2_state->r[Rn];
}


/*  STC     SSR,Rn */
inline void sh34_base_device::STCSSR(const uint16_t opcode)
{
	m_sh2_state->r[Rn] = m_ssr;
}

/*  STC     SPC,Rn */
inline void sh34_base_device::STCSPC(const uint16_t opcode)
{
	m_sh2_state->r[Rn] = m_spc;
}

/*  STC     SGR,Rn */
inline void sh34_base_device::STCSGR(const uint16_t opcode)
{
	m_sh2_state->r[Rn] = m_sgr;
}

/*  STS     FPUL,Rn */
inline void sh34_base_device::STSFPUL(const uint16_t opcode)
{
	m_sh2_state->r[Rn] = m_fpul;
}

/*  STS     FPSCR,Rn */
inline void sh34_base_device::STSFPSCR(const uint16_t opcode)
{
	m_sh2_state->r[Rn] = m_fpscr & 0x003FFFFF;
}

/*  STC     DBR,Rn */
inline void sh34_base_device::STCDBR(const uint16_t opcode)
{
	m_sh2_state->r[Rn] = m_dbr;
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
	} else
		m_sh2_state->r[n]=(int32_t)m_sh2_state->r[n] >> ((~m_sh2_state->r[m] & 0x1F)+1);
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
		m_sh2_state->r[n] = m_sh2_state->r[n] >> ((~m_sh2_state->r[m] & 0x1F)+1);
}

/*  LDCRBANK   Rn,Rm_BANK */
inline void sh34_base_device::LDCRBANK(const uint16_t opcode)
{
	uint32_t m = Rm;

	m_rbnk[m_sh2_state->sr&sRB ? 0 : 1][m & 7] = m_sh2_state->r[Rn];
}

/*  LDC     Rm,SSR */
inline void sh34_base_device::LDCSSR(const uint16_t opcode)
{
	m_ssr = m_sh2_state->r[Rn];
}

/*  LDC     Rm,SPC */
inline void sh34_base_device::LDCSPC(const uint16_t opcode)
{
	m_spc = m_sh2_state->r[Rn];
}

/*  PREF     @Rn */
inline void sh34_base_device::PREFM(const uint16_t opcode)
{
	int a;
	uint32_t addr,dest,sq;

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

	if (m_fpu_sz) { /* SZ = 1 */
		if (n & 1) {
			n &= 14;
#ifdef LSB_FIRST
			n ^= m_fpu_pr;
#endif
			m_sh2_state->ea = m_sh2_state->r[m];
			m_xf[n] = RL(m_sh2_state->ea );
			m_sh2_state->r[m] += 4;
			m_xf[n^1] = RL(m_sh2_state->ea+4 );
			m_sh2_state->r[m] += 4;
		} else {
#ifdef LSB_FIRST
			n ^= m_fpu_pr;
#endif
			m_sh2_state->ea = m_sh2_state->r[m];
			m_fr[n] = RL(m_sh2_state->ea );
			m_sh2_state->r[m] += 4;
			m_fr[n^1] = RL(m_sh2_state->ea+4 );
			m_sh2_state->r[m] += 4;
		}
	} else {              /* SZ = 0 */
		m_sh2_state->ea = m_sh2_state->r[m];
#ifdef LSB_FIRST
		n ^= m_fpu_pr;
#endif
		m_fr[n] = RL(m_sh2_state->ea );
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

	if (m_fpu_sz) { /* SZ = 1 */
		if (m & 1) {
			m &= 14;
#ifdef LSB_FIRST
			m ^= m_fpu_pr;
#endif
			m_sh2_state->ea = m_sh2_state->r[n];
			WL(m_sh2_state->ea,m_xf[m] );
			WL(m_sh2_state->ea+4,m_xf[m^1] );
		} else {
#ifdef LSB_FIRST
			m ^= m_fpu_pr;
#endif
			m_sh2_state->ea = m_sh2_state->r[n];
			WL(m_sh2_state->ea,m_fr[m] );
			WL(m_sh2_state->ea+4,m_fr[m^1] );
		}
	} else {              /* SZ = 0 */
		m_sh2_state->ea = m_sh2_state->r[n];
#ifdef LSB_FIRST
		m ^= m_fpu_pr;
#endif
		WL(m_sh2_state->ea,m_fr[m] );
	}
}

/*  FMOV.S  FRm,@-Rn PR=0 SZ=0 1111nnnnmmmm1011 */
/*  FMOV    DRm,@-Rn PR=0 SZ=1 1111nnnnmmm01011 */
/*  FMOV    XDm,@-Rn PR=0 SZ=1 1111nnnnmmm11011 */
/*  FMOV    XDm,@-Rn PR=1      1111nnnnmmm11011 */
inline void sh34_base_device::FMOVFRMDR(const uint16_t opcode)
{
	uint32_t m = Rm; uint32_t n = Rn;

	if (m_fpu_sz) { /* SZ = 1 */
		if (m & 1) {
			m &= 14;
#ifdef LSB_FIRST
			m ^= m_fpu_pr;
#endif
			m_sh2_state->r[n] -= 8;
			m_sh2_state->ea = m_sh2_state->r[n];
			WL(m_sh2_state->ea,m_xf[m] );
			WL(m_sh2_state->ea+4,m_xf[m^1] );
		} else {
#ifdef LSB_FIRST
			m ^= m_fpu_pr;
#endif
			m_sh2_state->r[n] -= 8;
			m_sh2_state->ea = m_sh2_state->r[n];
			WL(m_sh2_state->ea,m_fr[m] );
			WL(m_sh2_state->ea+4,m_fr[m^1] );
		}
	} else {              /* SZ = 0 */
		m_sh2_state->r[n] -= 4;
		m_sh2_state->ea = m_sh2_state->r[n];
#ifdef LSB_FIRST
		m ^= m_fpu_pr;
#endif
		WL(m_sh2_state->ea,m_fr[m] );
	}
}

/*  FMOV.S  FRm,@(R0,Rn) PR=0 SZ=0 1111nnnnmmmm0111 */
/*  FMOV    DRm,@(R0,Rn) PR=0 SZ=1 1111nnnnmmm00111 */
/*  FMOV    XDm,@(R0,Rn) PR=0 SZ=1 1111nnnnmmm10111 */
/*  FMOV    XDm,@(R0,Rn) PR=1      1111nnnnmmm10111 */
inline void sh34_base_device::FMOVFRS0(const uint16_t opcode)
{
	uint32_t m = Rm; uint32_t n = Rn;

	if (m_fpu_sz) { /* SZ = 1 */
		if (m & 1) {
			m &= 14;
#ifdef LSB_FIRST
			m ^= m_fpu_pr;
#endif
			m_sh2_state->ea = m_sh2_state->r[0] + m_sh2_state->r[n];
			WL(m_sh2_state->ea,m_xf[m] );
			WL(m_sh2_state->ea+4,m_xf[m^1] );
		} else {
#ifdef LSB_FIRST
			m ^= m_fpu_pr;
#endif
			m_sh2_state->ea = m_sh2_state->r[0] + m_sh2_state->r[n];
			WL(m_sh2_state->ea,m_fr[m] );
			WL(m_sh2_state->ea+4,m_fr[m^1] );
		}
	} else {              /* SZ = 0 */
		m_sh2_state->ea = m_sh2_state->r[0] + m_sh2_state->r[n];
#ifdef LSB_FIRST
		m ^= m_fpu_pr;
#endif
		WL(m_sh2_state->ea,m_fr[m] );
	}
}

/*  FMOV.S  @(R0,Rm),FRn PR=0 SZ=0 1111nnnnmmmm0110 */
/*  FMOV    @(R0,Rm),DRn PR=0 SZ=1 1111nnn0mmmm0110 */
/*  FMOV    @(R0,Rm),XDn PR=0 SZ=1 1111nnn1mmmm0110 */
/*  FMOV    @(R0,Rm),XDn PR=1      1111nnn1mmmm0110 */
inline void sh34_base_device::FMOVS0FR(const uint16_t opcode)
{
	uint32_t m = Rm; uint32_t n = Rn;

	if (m_fpu_sz) { /* SZ = 1 */
		if (n & 1) {
			n &= 14;
#ifdef LSB_FIRST
			n ^= m_fpu_pr;
#endif
			m_sh2_state->ea = m_sh2_state->r[0] + m_sh2_state->r[m];
			m_xf[n] = RL(m_sh2_state->ea );
			m_xf[n^1] = RL(m_sh2_state->ea+4 );
		} else {
#ifdef LSB_FIRST
			n ^= m_fpu_pr;
#endif
			m_sh2_state->ea = m_sh2_state->r[0] + m_sh2_state->r[m];
			m_fr[n] = RL(m_sh2_state->ea );
			m_fr[n^1] = RL(m_sh2_state->ea+4 );
		}
	} else {              /* SZ = 0 */
		m_sh2_state->ea = m_sh2_state->r[0] + m_sh2_state->r[m];
#ifdef LSB_FIRST
		n ^= m_fpu_pr;
#endif
		m_fr[n] = RL(m_sh2_state->ea );
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

	if (m_fpu_sz) { /* SZ = 1 */
		if (n & 1) {
			n &= 14;
#ifdef LSB_FIRST
			n ^= m_fpu_pr;
#endif
			m_sh2_state->ea = m_sh2_state->r[m];
			m_xf[n] = RL(m_sh2_state->ea );
			m_xf[n^1] = RL(m_sh2_state->ea+4 );
		} else {
#ifdef LSB_FIRST
			n ^= m_fpu_pr;
#endif
			m_sh2_state->ea = m_sh2_state->r[m];
			m_fr[n] = RL(m_sh2_state->ea );
			m_fr[n^1] = RL(m_sh2_state->ea+4 );
		}
	} else {              /* SZ = 0 */
		m_sh2_state->ea = m_sh2_state->r[m];
#ifdef LSB_FIRST
		n ^= m_fpu_pr;
#endif
		m_fr[n] = RL(m_sh2_state->ea );
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

	if (m_fpu_sz == 0)  {  /* SZ = 0 */
#ifdef LSB_FIRST
		n ^= m_fpu_pr;
		m ^= m_fpu_pr;
#endif
		m_fr[n] = m_fr[m];
	}
	else { /* SZ = 1 */
		if (m & 1) {
			if (n & 1) {
				m_xf[n & 14] = m_xf[m & 14];
				m_xf[n | 1] = m_xf[m | 1];
			} else {
				m_fr[n] = m_xf[m & 14];
				m_fr[n | 1] = m_xf[m | 1];
			}
		} else {
			if (n & 1) {
				m_xf[n & 14] = m_fr[m];
				m_xf[n | 1] = m_fr[m | 1]; // (a&14)+1 -> a|1
			} else {
				m_fr[n] = m_fr[m];
				m_fr[n | 1] = m_fr[m | 1];
			}
		}
	}
}

/*  FLDI1  FRn 1111nnnn10011101 */
inline void sh34_base_device::FLDI1(const uint16_t opcode)
{
#ifdef LSB_FIRST
	m_fr[Rn ^ m_fpu_pr] = 0x3F800000;
#else
	m_fr[Rn] = 0x3F800000;
#endif
}

/*  FLDI0  FRn 1111nnnn10001101 */
inline void sh34_base_device::FLDI0(const uint16_t opcode)
{
#ifdef LSB_FIRST
	m_fr[Rn ^ m_fpu_pr] = 0;
#else
	m_fr[Rn] = 0;
#endif
}

/*  FLDS FRm,FPUL 1111mmmm00011101 */
inline void sh34_base_device:: FLDS(const uint16_t opcode)
{
#ifdef LSB_FIRST
	m_fpul = m_fr[Rn ^ m_fpu_pr];
#else
	m_fpul = m_fr[Rn];
#endif
}

/*  FSTS FPUL,FRn 1111nnnn00001101 */
inline void sh34_base_device:: FSTS(const uint16_t opcode)
{
#ifdef LSB_FIRST
	m_fr[Rn ^ m_fpu_pr] = m_fpul;
#else
	m_fr[Rn] = m_fpul;
#endif
}

/* FRCHG 1111101111111101 */
void sh34_base_device::FRCHG()
{
	m_fpscr ^= FR;
	sh4_swap_fp_registers();
}

/* FSCHG 1111001111111101 */
void sh34_base_device::FSCHG()
{
	m_fpscr ^= SZ;
	m_fpu_sz = (m_fpscr & SZ) ? 1 : 0;
}

/* FTRC FRm,FPUL PR=0 1111mmmm00111101 */
/* FTRC DRm,FPUL PR=1 1111mmm000111101 */
inline void sh34_base_device::FTRC(const uint16_t opcode)
{
	uint32_t n = Rn;

	if (m_fpu_pr) { /* PR = 1 */
		if(n & 1)
			fatalerror("SH-4: FTRC opcode used with n %d",n);

		n = n & 14;
		*((int32_t *)&m_fpul) = (int32_t)FP_RFD(n);
	} else {              /* PR = 0 */
		/* read m_fr[n] as float -> truncate -> fpul(32) */
		*((int32_t *)&m_fpul) = (int32_t)FP_RFS(n);
	}
}

/* FLOAT FPUL,FRn PR=0 1111nnnn00101101 */
/* FLOAT FPUL,DRn PR=1 1111nnn000101101 */
inline void sh34_base_device::FLOAT(const uint16_t opcode)
{
	uint32_t n = Rn;

	if (m_fpu_pr) { /* PR = 1 */
		if(n & 1)
			fatalerror("SH-4: FLOAT opcode used with n %d",n);

		n = n & 14;
		FP_RFD(n) = (double)*((int32_t *)&m_fpul);
	} else {              /* PR = 0 */
		FP_RFS(n) = (float)*((int32_t *)&m_fpul);
	}
}

/* FNEG FRn PR=0 1111nnnn01001101 */
/* FNEG DRn PR=1 1111nnn001001101 */
inline void sh34_base_device::FNEG(const uint16_t opcode)
{
	uint32_t n = Rn;

	if (m_fpu_pr) { /* PR = 1 */
		FP_RFD(n) = -FP_RFD(n);
	} else {              /* PR = 0 */
		FP_RFS(n) = -FP_RFS(n);
	}
}

/* FABS FRn PR=0 1111nnnn01011101 */
/* FABS DRn PR=1 1111nnn001011101 */
inline void sh34_base_device::FABS(const uint16_t opcode)
{
	uint32_t n = Rn;

	if (m_fpu_pr) { /* PR = 1 */
#ifdef LSB_FIRST
		n = n | 1; // n & 14 + 1
		m_fr[n] = m_fr[n] & 0x7fffffff;
#else
		n = n & 14;
		m_fr[n] = m_fr[n] & 0x7fffffff;
#endif
	} else {              /* PR = 0 */
		m_fr[n] = m_fr[n] & 0x7fffffff;
	}
}

/* FCMP/EQ FRm,FRn PR=0 1111nnnnmmmm0100 */
/* FCMP/EQ DRm,DRn PR=1 1111nnn0mmm00100 */
inline void sh34_base_device::FCMP_EQ(const uint16_t opcode)
{
	uint32_t m = Rm; uint32_t n = Rn;

	if (m_fpu_pr) { /* PR = 1 */
		n = n & 14;
		m = m & 14;
		if (FP_RFD(n) == FP_RFD(m))
			m_sh2_state->sr |= T;
		else
			m_sh2_state->sr &= ~T;
	} else {              /* PR = 0 */
		if (FP_RFS(n) == FP_RFS(m))
			m_sh2_state->sr |= T;
		else
			m_sh2_state->sr &= ~T;
	}
}

/* FCMP/GT FRm,FRn PR=0 1111nnnnmmmm0101 */
/* FCMP/GT DRm,DRn PR=1 1111nnn0mmm00101 */
inline void sh34_base_device::FCMP_GT(const uint16_t opcode)
{
	uint32_t m = Rm; uint32_t n = Rn;

	if (m_fpu_pr) { /* PR = 1 */
		n = n & 14;
		m = m & 14;
		if (FP_RFD(n) > FP_RFD(m))
			m_sh2_state->sr |= T;
		else
			m_sh2_state->sr &= ~T;
	} else {              /* PR = 0 */
		if (FP_RFS(n) > FP_RFS(m))
			m_sh2_state->sr |= T;
		else
			m_sh2_state->sr &= ~T;
	}
}

/* FCNVDS DRm,FPUL PR=1 1111mmm010111101 */
inline void sh34_base_device::FCNVDS(const uint16_t opcode)
{
	uint32_t n = Rn;

	if (m_fpu_pr) { /* PR = 1 */
		n = n & 14;
		if (m_fpscr & RM)
			m_fr[n | NATIVE_ENDIAN_VALUE_LE_BE(0,1)] &= 0xe0000000; /* round toward zero*/
		*((float *)&m_fpul) = (float)FP_RFD(n);
	}
}

/* FCNVSD FPUL, DRn PR=1 1111nnn010101101 */
inline void sh34_base_device::FCNVSD(const uint16_t opcode)
{
	uint32_t n = Rn;

	if (m_fpu_pr) { /* PR = 1 */
		n = n & 14;
		FP_RFD(n) = (double)*((float *)&m_fpul);
	}
}

/* FADD FRm,FRn PR=0 1111nnnnmmmm0000 */
/* FADD DRm,DRn PR=1 1111nnn0mmm00000 */
inline void sh34_base_device::FADD(const uint16_t opcode)
{
	uint32_t m = Rm; uint32_t n = Rn;

	if (m_fpu_pr) { /* PR = 1 */
		n = n & 14;
		m = m & 14;
		FP_RFD(n) = FP_RFD(n) + FP_RFD(m);
	} else {              /* PR = 0 */
		FP_RFS(n) = FP_RFS(n) + FP_RFS(m);
	}
}

/* FSUB FRm,FRn PR=0 1111nnnnmmmm0001 */
/* FSUB DRm,DRn PR=1 1111nnn0mmm00001 */
inline void sh34_base_device::FSUB(const uint16_t opcode)
{
	uint32_t m = Rm; uint32_t n = Rn;

	if (m_fpu_pr) { /* PR = 1 */
		n = n & 14;
		m = m & 14;
		FP_RFD(n) = FP_RFD(n) - FP_RFD(m);
	} else {              /* PR = 0 */
		FP_RFS(n) = FP_RFS(n) - FP_RFS(m);
	}
}


/* FMUL FRm,FRn PR=0 1111nnnnmmmm0010 */
/* FMUL DRm,DRn PR=1 1111nnn0mmm00010 */
inline void sh34_base_device::FMUL(const uint16_t opcode)
{
	uint32_t m = Rm; uint32_t n = Rn;

	if (m_fpu_pr) { /* PR = 1 */
		n = n & 14;
		m = m & 14;
		FP_RFD(n) = FP_RFD(n) * FP_RFD(m);
	} else {              /* PR = 0 */
		FP_RFS(n) = FP_RFS(n) * FP_RFS(m);
	}
}

/* FDIV FRm,FRn PR=0 1111nnnnmmmm0011 */
/* FDIV DRm,DRn PR=1 1111nnn0mmm00011 */
inline void sh34_base_device::FDIV(const uint16_t opcode)
{
	uint32_t m = Rm; uint32_t n = Rn;

	if (m_fpu_pr) { /* PR = 1 */
		n = n & 14;
		m = m & 14;
		if (FP_RFD(m) == 0)
			return;
		FP_RFD(n) = FP_RFD(n) / FP_RFD(m);
	} else {              /* PR = 0 */
		if (FP_RFS(m) == 0)
			return;
		FP_RFS(n) = FP_RFS(n) / FP_RFS(m);
	}
}

/* FMAC FR0,FRm,FRn PR=0 1111nnnnmmmm1110 */
inline void sh34_base_device::FMAC(const uint16_t opcode)
{
	uint32_t m = Rm; uint32_t n = Rn;

	if (m_fpu_pr == 0) { /* PR = 0 */
		FP_RFS(n) = (FP_RFS(0) * FP_RFS(m)) + FP_RFS(n);
	}
}

/* FSQRT FRn PR=0 1111nnnn01101101 */
/* FSQRT DRn PR=1 1111nnnn01101101 */
inline void sh34_base_device::FSQRT(const uint16_t opcode)
{
	uint32_t n = Rn;

	if (m_fpu_pr) { /* PR = 1 */
		n = n & 14;
		if (FP_RFD(n) < 0)
			return;
		FP_RFD(n) = sqrtf(FP_RFD(n));
	} else {              /* PR = 0 */
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

	angle = (((float)(m_fpul & 0xFFFF)) / 65536.0f) * 2.0f * (float) M_PI;
	FP_RFS(n) = sinf(angle);
	FP_RFS(n+1) = cosf(angle);
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
		ml[a] = FP_RFS(n+a) * FP_RFS(m+a);
	FP_RFS(n+3) = ml[0] + ml[1] + ml[2] + ml[3];
}

/* FTRV XMTRX,FVn PR=0 1111nn0111111101 */
void sh34_base_device::FTRV(const uint16_t opcode)
{
	uint32_t n = Rn;

int i,j;
float sum[4];

	n = n & 12;
	for (i = 0;i < 4;i++) {
		sum[i] = 0;
		for (j=0;j < 4;j++)
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
			} else {
				FTRV(opcode);
			}
		} else {
			FSSCA(opcode);
		}
}

void sh34_base_device::dbreak(const uint16_t opcode)
{
	machine().debug_break();
}


inline void sh34_base_device::op1111_0x13(uint16_t opcode)
{
	switch((opcode >> 4) & 0x0f)
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
	m_spc = 0;
	m_sh2_state->pr = 0;
	m_sh2_state->sr = 0;
	m_ssr = 0;
	m_sh2_state->gbr = 0;
	m_sh2_state->vbr = 0;
	m_sh2_state->mach = 0;
	m_sh2_state->macl = 0;
	memset(m_sh2_state->r, 0, sizeof(m_sh2_state->r));
	memset(m_rbnk, 0, sizeof(m_rbnk));
	m_sgr = 0;
	memset(m_fr, 0, sizeof(m_fr));
	memset(m_xf, 0, sizeof(m_xf));
	m_sh2_state->ea = 0;
	m_sh2_state->m_delay = 0;
	m_cpu_off = 0;
	m_pending_irq = 0;
	m_test_irq = 0;
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
	m_frt_input = 0;
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
	m_ppc = m_sh2_state->pc & SH34_AM;
	m_sh2_state->r[15] = RL(4);
	m_sh2_state->sr = 0x700000f0;
	m_fpscr = 0x00040001;
	m_fpu_sz = (m_fpscr & SZ) ? 1 : 0;
	m_fpu_pr = (m_fpscr & PR) ? 1 : 0;
	m_fpul = 0;
	m_dbr = 0;

	m_internal_irq_level = -1;
	m_irln = 15;
	m_sh2_state->sleep_mode = 0;

	m_sh4_mmu_enabled = 0;
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
	switch(opcode & 0xff)
	{
		// 0x00
		case 0x00:  ILLEGAL(); break;
		case 0x10:  ILLEGAL(); break;
		case 0x20:  ILLEGAL(); break;
		case 0x30:  ILLEGAL(); break;
		case 0x40:  ILLEGAL(); break;
		case 0x50:  ILLEGAL(); break;
		case 0x60:  ILLEGAL(); break;
		case 0x70:  ILLEGAL(); break;
		case 0x80:  ILLEGAL(); break;
		case 0x90:  ILLEGAL(); break;
		case 0xa0:  ILLEGAL(); break;
		case 0xb0:  ILLEGAL(); break;
		case 0xc0:  ILLEGAL(); break;
		case 0xd0:  ILLEGAL(); break;
		case 0xe0:  ILLEGAL(); break;
		case 0xf0:  ILLEGAL(); break;
		// 0x10
		case 0x01:  ILLEGAL(); break;
		case 0x11:  ILLEGAL(); break;
		case 0x21:  ILLEGAL(); break;
		case 0x31:  ILLEGAL(); break;
		case 0x41:  ILLEGAL(); break;
		case 0x51:  ILLEGAL(); break;
		case 0x61:  ILLEGAL(); break;
		case 0x71:  ILLEGAL(); break;
		case 0x81:  ILLEGAL(); break;
		case 0x91:  ILLEGAL(); break;
		case 0xa1:  ILLEGAL(); break;
		case 0xb1:  ILLEGAL(); break;
		case 0xc1:  ILLEGAL(); break;
		case 0xd1:  ILLEGAL(); break;
		case 0xe1:  ILLEGAL(); break;
		case 0xf1:  ILLEGAL(); break;
		// 0x20
		case 0x02:  STCSR(Rn); break;
		case 0x12:  STCGBR(Rn); break;
		case 0x22:  STCVBR(Rn); break;
		case 0x32:  STCSSR(opcode); break; // sh4 only
		case 0x42:  STCSPC(opcode); break; // sh4 only
		case 0x52:  ILLEGAL(); break;
		case 0x62:  ILLEGAL(); break;
		case 0x72:  ILLEGAL(); break;
		case 0x82:  STCRBANK(opcode); break; // sh4 only
		case 0x92:  STCRBANK(opcode); break;
		case 0xa2:  STCRBANK(opcode); break;
		case 0xb2:  STCRBANK(opcode); break;
		case 0xc2:  STCRBANK(opcode); break;
		case 0xd2:  STCRBANK(opcode); break;
		case 0xe2:  STCRBANK(opcode); break;
		case 0xf2:  STCRBANK(opcode); break;
		// 0x30
		case 0x03:  BSRF(Rn); break;
		case 0x13:  ILLEGAL(); break;
		case 0x23:  BRAF(Rn); break;
		case 0x33:  ILLEGAL(); break;
		case 0x43:  ILLEGAL(); break;
		case 0x53:  ILLEGAL(); break;
		case 0x63:  ILLEGAL(); break;
		case 0x73:  ILLEGAL(); break;
		case 0x83:  PREFM(opcode); break; // sh4 only
		case 0x93:  TODO(opcode); break;
		case 0xa3:  TODO(opcode); break;
		case 0xb3:  TODO(opcode); break;
		case 0xc3:  MOVCAL(opcode); break; // sh4 only
		case 0xd3:  ILLEGAL(); break;
		case 0xe3:  ILLEGAL(); break;
		case 0xf3:  ILLEGAL(); break;
		// 0x40
		case 0x04:  MOVBS0(Rm, Rn); break;
		case 0x14:  MOVBS0(Rm, Rn); break;
		case 0x24:  MOVBS0(Rm, Rn); break;
		case 0x34:  MOVBS0(Rm, Rn); break;
		case 0x44:  MOVBS0(Rm, Rn); break;
		case 0x54:  MOVBS0(Rm, Rn); break;
		case 0x64:  MOVBS0(Rm, Rn); break;
		case 0x74:  MOVBS0(Rm, Rn); break;
		case 0x84:  MOVBS0(Rm, Rn); break;
		case 0x94:  MOVBS0(Rm, Rn); break;
		case 0xa4:  MOVBS0(Rm, Rn); break;
		case 0xb4:  MOVBS0(Rm, Rn); break;
		case 0xc4:  MOVBS0(Rm, Rn); break;
		case 0xd4:  MOVBS0(Rm, Rn); break;
		case 0xe4:  MOVBS0(Rm, Rn); break;
		case 0xf4:  MOVBS0(Rm, Rn); break;
		// 0x50
		case 0x05:  MOVWS0(Rm, Rn); break;
		case 0x15:  MOVWS0(Rm, Rn); break;
		case 0x25:  MOVWS0(Rm, Rn); break;
		case 0x35:  MOVWS0(Rm, Rn); break;
		case 0x45:  MOVWS0(Rm, Rn); break;
		case 0x55:  MOVWS0(Rm, Rn); break;
		case 0x65:  MOVWS0(Rm, Rn); break;
		case 0x75:  MOVWS0(Rm, Rn); break;
		case 0x85:  MOVWS0(Rm, Rn); break;
		case 0x95:  MOVWS0(Rm, Rn); break;
		case 0xa5:  MOVWS0(Rm, Rn); break;
		case 0xb5:  MOVWS0(Rm, Rn); break;
		case 0xc5:  MOVWS0(Rm, Rn); break;
		case 0xd5:  MOVWS0(Rm, Rn); break;
		case 0xe5:  MOVWS0(Rm, Rn); break;
		case 0xf5:  MOVWS0(Rm, Rn); break;
		// 0x60
		case 0x06:  MOVLS0(Rm, Rn); break;
		case 0x16:  MOVLS0(Rm, Rn); break;
		case 0x26:  MOVLS0(Rm, Rn); break;
		case 0x36:  MOVLS0(Rm, Rn); break;
		case 0x46:  MOVLS0(Rm, Rn); break;
		case 0x56:  MOVLS0(Rm, Rn); break;
		case 0x66:  MOVLS0(Rm, Rn); break;
		case 0x76:  MOVLS0(Rm, Rn); break;
		case 0x86:  MOVLS0(Rm, Rn); break;
		case 0x96:  MOVLS0(Rm, Rn); break;
		case 0xa6:  MOVLS0(Rm, Rn); break;
		case 0xb6:  MOVLS0(Rm, Rn); break;
		case 0xc6:  MOVLS0(Rm, Rn); break;
		case 0xd6:  MOVLS0(Rm, Rn); break;
		case 0xe6:  MOVLS0(Rm, Rn); break;
		case 0xf6:  MOVLS0(Rm, Rn); break;
		// 0x70
		case 0x07:  MULL(Rm, Rn); break;
		case 0x17:  MULL(Rm, Rn); break;
		case 0x27:  MULL(Rm, Rn); break;
		case 0x37:  MULL(Rm, Rn); break;
		case 0x47:  MULL(Rm, Rn); break;
		case 0x57:  MULL(Rm, Rn); break;
		case 0x67:  MULL(Rm, Rn); break;
		case 0x77:  MULL(Rm, Rn); break;
		case 0x87:  MULL(Rm, Rn); break;
		case 0x97:  MULL(Rm, Rn); break;
		case 0xa7:  MULL(Rm, Rn); break;
		case 0xb7:  MULL(Rm, Rn); break;
		case 0xc7:  MULL(Rm, Rn); break;
		case 0xd7:  MULL(Rm, Rn); break;
		case 0xe7:  MULL(Rm, Rn); break;
		case 0xf7:  MULL(Rm, Rn); break;
		// 0x80
		case 0x08:  CLRT(); break;
		case 0x88:  CLRT(); break;

		case 0x18:  SETT(); break;
		case 0x98:  SETT(); break;

		case 0x28:  CLRMAC(); break;
		case 0xa8:  CLRMAC(); break;

		case 0x38:  LDTLB(opcode); break; // sh4 only
		case 0xb8:  LDTLB(opcode); break; // sh4 only

		case 0x48:  CLRS(opcode); break; // sh4 only
		case 0xc8:  CLRS(opcode); break; // sh4 only

		case 0x58:  SETS(opcode); break; // sh4 only
		case 0xd8:  SETS(opcode); break; // sh4 only

		case 0x68:  ILLEGAL(); break;
		case 0xe8:  ILLEGAL(); break;

		case 0x78:  ILLEGAL(); break;
		case 0xf8:  ILLEGAL(); break;
		// 0x90
		case 0x09:  NOP(); break;
		case 0x49:  NOP(); break;
		case 0x89:  NOP(); break;
		case 0xc9:  NOP(); break;

		case 0x19:  DIV0U(); break;
		case 0x59:  DIV0U(); break;
		case 0x99:  DIV0U(); break;
		case 0xd9:  DIV0U(); break;

		case 0x29:  MOVT(Rn); break;
		case 0x69:  MOVT(Rn); break;
		case 0xa9:  MOVT(Rn); break;
		case 0xe9:  MOVT(Rn); break;

		case 0x39:  ILLEGAL(); break;
		case 0x79:  ILLEGAL(); break;
		case 0xb9:  ILLEGAL(); break;
		case 0xf9:  ILLEGAL(); break;
		
		// 0xa0
		case 0x0a:  STSMACH(Rn); break;
		case 0x8a:  STSMACH(Rn); break;

		case 0x1a:  STSMACL(Rn); break;
		case 0x9a:  STSMACL(Rn); break;

		case 0x2a:  STSPR(Rn); break;
		case 0xaa:  STSPR(Rn); break;

		case 0x3a:  STCSGR(opcode); break; // sh4 only
		case 0xba:  STCSGR(opcode); break; // sh4 only

		case 0x4a:  ILLEGAL(); break;
		case 0xca:  ILLEGAL(); break;

		case 0x5a:  STSFPUL(opcode); break; // sh4 only
		case 0xda:  STSFPUL(opcode); break; // sh4only

		case 0x6a:  STSFPSCR(opcode); break; // sh4 only
		case 0xea:  STSFPSCR(opcode); break; // sh4only

		case 0x7a:  STCDBR(opcode); break; // sh4 only
		case 0xfa:  STCDBR(opcode); break; // sh4 only
		// 0xb0
		case 0x0b:  RTS(); break;
		case 0x4b:  RTS(); break;
		case 0x8b:  RTS(); break;
		case 0xcb:  RTS(); break;

		case 0x1b:  SLEEP(); break;
		case 0x5b:  SLEEP(); break;
		case 0x9b:  SLEEP(); break;
		case 0xdb:  SLEEP(); break;

		case 0x2b:  RTE(); break;
		case 0x6b:  RTE(); break;
		case 0xab:  RTE(); break;
		case 0xeb:  RTE(); break;

		case 0x3b:  ILLEGAL(); break;
		case 0x7b:  ILLEGAL(); break;
		case 0xbb:  ILLEGAL(); break;
		case 0xfb:  ILLEGAL(); break;
		// 0xc0
		case 0x0c:  MOVBL0(Rm, Rn);  break;
		case 0x1c:  MOVBL0(Rm, Rn);  break;
		case 0x2c:  MOVBL0(Rm, Rn);  break;
		case 0x3c:  MOVBL0(Rm, Rn);  break;
		case 0x4c:  MOVBL0(Rm, Rn);  break;
		case 0x5c:  MOVBL0(Rm, Rn);  break;
		case 0x6c:  MOVBL0(Rm, Rn);  break;
		case 0x7c:  MOVBL0(Rm, Rn);  break;
		case 0x8c:  MOVBL0(Rm, Rn);  break;
		case 0x9c:  MOVBL0(Rm, Rn);  break;
		case 0xac:  MOVBL0(Rm, Rn);  break;
		case 0xbc:  MOVBL0(Rm, Rn);  break;
		case 0xcc:  MOVBL0(Rm, Rn);  break;
		case 0xdc:  MOVBL0(Rm, Rn);  break;
		case 0xec:  MOVBL0(Rm, Rn);  break;
		case 0xfc:  MOVBL0(Rm, Rn);  break;
		// 0xd0
		case 0x0d:  MOVWL0(Rm, Rn); break;
		case 0x1d:  MOVWL0(Rm, Rn); break;
		case 0x2d:  MOVWL0(Rm, Rn); break;
		case 0x3d:  MOVWL0(Rm, Rn); break;
		case 0x4d:  MOVWL0(Rm, Rn); break;
		case 0x5d:  MOVWL0(Rm, Rn); break;
		case 0x6d:  MOVWL0(Rm, Rn); break;
		case 0x7d:  MOVWL0(Rm, Rn); break;
		case 0x8d:  MOVWL0(Rm, Rn); break;
		case 0x9d:  MOVWL0(Rm, Rn); break;
		case 0xad:  MOVWL0(Rm, Rn); break;
		case 0xbd:  MOVWL0(Rm, Rn); break;
		case 0xcd:  MOVWL0(Rm, Rn); break;
		case 0xdd:  MOVWL0(Rm, Rn); break;
		case 0xed:  MOVWL0(Rm, Rn); break;
		case 0xfd:  MOVWL0(Rm, Rn); break;
		// 0xe0
		case 0x0e:  MOVLL0(Rm, Rn); break;
		case 0x1e:  MOVLL0(Rm, Rn); break;
		case 0x2e:  MOVLL0(Rm, Rn); break;
		case 0x3e:  MOVLL0(Rm, Rn); break;
		case 0x4e:  MOVLL0(Rm, Rn); break;
		case 0x5e:  MOVLL0(Rm, Rn); break;
		case 0x6e:  MOVLL0(Rm, Rn); break;
		case 0x7e:  MOVLL0(Rm, Rn); break;
		case 0x8e:  MOVLL0(Rm, Rn); break;
		case 0x9e:  MOVLL0(Rm, Rn); break;
		case 0xae:  MOVLL0(Rm, Rn); break;
		case 0xbe:  MOVLL0(Rm, Rn); break;
		case 0xce:  MOVLL0(Rm, Rn); break;
		case 0xde:  MOVLL0(Rm, Rn); break;
		case 0xee:  MOVLL0(Rm, Rn); break;
		case 0xfe:  MOVLL0(Rm, Rn); break;
		// 0xf0
		case 0x0f:  MAC_L(Rm, Rn); break;
		case 0x1f:  MAC_L(Rm, Rn); break;
		case 0x2f:  MAC_L(Rm, Rn); break;
		case 0x3f:  MAC_L(Rm, Rn); break;
		case 0x4f:  MAC_L(Rm, Rn); break;
		case 0x5f:  MAC_L(Rm, Rn); break;
		case 0x6f:  MAC_L(Rm, Rn); break;
		case 0x7f:  MAC_L(Rm, Rn); break;
		case 0x8f:  MAC_L(Rm, Rn); break;
		case 0x9f:  MAC_L(Rm, Rn); break;
		case 0xaf:  MAC_L(Rm, Rn); break;
		case 0xbf:  MAC_L(Rm, Rn); break;
		case 0xcf:  MAC_L(Rm, Rn); break;
		case 0xdf:  MAC_L(Rm, Rn); break;
		case 0xef:  MAC_L(Rm, Rn); break;
		case 0xff:  MAC_L(Rm, Rn); break;
	}
}

inline void sh34_base_device::execute_one_4000(const uint16_t opcode)
{
	switch(opcode & 0xff)
	{
		// 0x00
		case 0x00:  SHLL(Rn); break;
		case 0x40:  SHLL(Rn); break;
		case 0x80:  SHLL(Rn); break;
		case 0xc0:  SHLL(Rn); break;

		case 0x10:  DT(Rn); break;
		case 0x50:  DT(Rn); break;
		case 0x90:  DT(Rn); break;
		case 0xd0:  DT(Rn); break;

		case 0x20:  SHAL(Rn); break;
		case 0x60:  SHAL(Rn); break;
		case 0xa0:  SHAL(Rn); break;
		case 0xe0:  SHAL(Rn); break;

		case 0x30:  ILLEGAL(); break;
		case 0x70:  ILLEGAL(); break;
		case 0xb0:  ILLEGAL(); break;
		case 0xf0:  ILLEGAL(); break;
		
		// 0x10
		case 0x01:  SHLR(Rn); break;
		case 0x41:  SHLR(Rn); break;
		case 0x81:  SHLR(Rn); break;
		case 0xc1:  SHLR(Rn); break;

		case 0x11:  CMPPZ(Rn);  break;
		case 0x51:  CMPPZ(Rn);  break;
		case 0x91:  CMPPZ(Rn);  break;
		case 0xd1:  CMPPZ(Rn);  break;

		case 0x21:  SHAR(Rn); break;
		case 0x61:  SHAR(Rn); break;
		case 0xa1:  SHAR(Rn); break;
		case 0xe1:  SHAR(Rn); break;

		case 0x31:  ILLEGAL(); break;
		case 0x71:  ILLEGAL(); break;
		case 0xb1:  ILLEGAL(); break;
		case 0xf1:  ILLEGAL(); break;
		// 0x20
		case 0x02:  STSMMACH(Rn); break;
		case 0x12:  STSMMACL(Rn);  break;
		case 0x22:  STSMPR(Rn); break;
		case 0x32:  STCMSGR(opcode); break; // sh4 only
		case 0x42:  ILLEGAL(); break;
		case 0x52:  STSMFPUL(opcode); break; // sh4 only
		case 0x62:  STSMFPSCR(opcode); break; // sh4 only
		case 0x72:  ILLEGAL(); break;
		case 0x82:  ILLEGAL(); break;
		case 0x92:  ILLEGAL(); break;
		case 0xa2:  ILLEGAL(); break;
		case 0xb2:  ILLEGAL(); break;
		case 0xc2:  ILLEGAL(); break;
		case 0xd2:  ILLEGAL(); break;
		case 0xe2:  ILLEGAL(); break;
		case 0xf2:  STCMDBR(opcode); break; // sh4 only
		// 0x30
		case 0x03:  STCMSR(Rn); break;
		case 0x13:  STCMGBR(Rn);  break;
		case 0x23:  STCMVBR(Rn); break;
		case 0x33:  STCMSSR(opcode); break; // sh4 only
		case 0x43:  STCMSPC(opcode); break; // sh4 only
		case 0x53:  ILLEGAL(); break;
		case 0x63:  ILLEGAL(); break;
		case 0x73:  ILLEGAL(); break;
		case 0x83:  STCMRBANK(opcode); break; // sh4 only
		case 0x93:  STCMRBANK(opcode); break;
		case 0xa3:  STCMRBANK(opcode); break;
		case 0xb3:  STCMRBANK(opcode); break;
		case 0xc3:  STCMRBANK(opcode); break;
		case 0xd3:  STCMRBANK(opcode); break;
		case 0xe3:  STCMRBANK(opcode); break;
		case 0xf3:  STCMRBANK(opcode); break;
		// 0x40
		case 0x04:  ROTL(Rn); break;
		case 0x44:  ROTL(Rn); break;
		case 0x84:  ROTL(Rn); break;
		case 0xc4:  ROTL(Rn); break;

		case 0x14:  ILLEGAL(); break;
		case 0x34:  ILLEGAL(); break;
		case 0x74:  ILLEGAL(); break;
		case 0xb4:  ILLEGAL(); break;

		case 0x24:  ROTCL(Rn); break;
		case 0x64:  ROTCL(Rn); break;
		case 0xa4:  ROTCL(Rn); break;
		case 0xe4:  ROTCL(Rn); break;

		case 0x54:  ILLEGAL(); break;
		case 0x94:  ILLEGAL(); break;
		case 0xd4:  ILLEGAL(); break;
		case 0xf4:  ILLEGAL(); break;
		// 0x50
		case 0x05:  ROTR(Rn);  break;
		case 0x45:  ROTR(Rn);  break;
		case 0x85:  ROTR(Rn);  break;
		case 0xc5:  ROTR(Rn);  break;

		case 0x15:  CMPPL(Rn); break;
		case 0x55:  CMPPL(Rn); break;
		case 0x95:  CMPPL(Rn); break;
		case 0xd5:  CMPPL(Rn); break;

		case 0x25:  ROTCR(Rn); break;
		case 0x65:  ROTCR(Rn); break;
		case 0xa5:  ROTCR(Rn); break;
		case 0xe5:  ROTCR(Rn); break;

		case 0x35:  ILLEGAL(); break;
		case 0x75:  ILLEGAL(); break;
		case 0xb5:  ILLEGAL(); break;
		case 0xf5:  ILLEGAL(); break;
		
		// 0x60
		case 0x06:  LDSMMACH(Rn); break;
		case 0x16:  LDSMMACL(Rn); break;
		case 0x26:  LDSMPR(Rn);  break;
		case 0x36:  ILLEGAL(); break;
		case 0x46:  ILLEGAL(); break;
		case 0x56:  LDSMFPUL(opcode); break; // sh4 only
		case 0x66:  LDSMFPSCR(opcode); break; // sh4 only
		case 0x76:  ILLEGAL(); break;
		case 0x86:  ILLEGAL(); break;
		case 0x96:  ILLEGAL(); break;
		case 0xa6:  ILLEGAL(); break;
		case 0xb6:  ILLEGAL(); break;
		case 0xc6:  ILLEGAL(); break;
		case 0xd6:  ILLEGAL(); break;
		case 0xe6:  ILLEGAL(); break;
		case 0xf6:  LDCMDBR(opcode); break; // sh4 only
		// 0x70
		case 0x07:  LDCMSR(opcode);  break; // sh2/4 flag difference
		case 0x17:  LDCMGBR(Rn); break;
		case 0x27:  LDCMVBR(Rn); break;
		case 0x37:  LDCMSSR(opcode); break; // sh4 only
		case 0x47:  LDCMSPC(opcode); break; // sh4 only
		case 0x57:  ILLEGAL(); break;
		case 0x67:  ILLEGAL(); break;
		case 0x77:  ILLEGAL(); break;
		case 0x87:  LDCMRBANK(opcode); break; // sh4 only
		case 0x97:  LDCMRBANK(opcode); break;
		case 0xa7:  LDCMRBANK(opcode); break;
		case 0xb7:  LDCMRBANK(opcode); break;
		case 0xc7:  LDCMRBANK(opcode); break;
		case 0xd7:  LDCMRBANK(opcode); break;
		case 0xe7:  LDCMRBANK(opcode); break;
		case 0xf7:  LDCMRBANK(opcode); break;
		// 0x80
		case 0x08:  SHLL2(Rn);  break;
		case 0x48:  SHLL2(Rn);  break;
		case 0x88:  SHLL2(Rn);  break;
		case 0xc8:  SHLL2(Rn);  break;

		case 0x18:  SHLL8(Rn); break;
		case 0x58:  SHLL8(Rn); break;
		case 0x98:  SHLL8(Rn); break;
		case 0xd8:  SHLL8(Rn); break;

		case 0x28:  SHLL16(Rn); break;
		case 0x68:  SHLL16(Rn); break;
		case 0xa8:  SHLL16(Rn); break;
		case 0xe8:  SHLL16(Rn); break;

		case 0x38:  ILLEGAL(); break;
		case 0x78:  ILLEGAL(); break;
		case 0xb8:  ILLEGAL(); break;
		case 0xf8:  ILLEGAL(); break;
		// 0x90
		case 0x09:  SHLR2(Rn); break;
		case 0x49:  SHLR2(Rn); break;
		case 0x89:  SHLR2(Rn); break;
		case 0xc9:  SHLR2(Rn); break;

		case 0x19:  SHLR8(Rn); break;
		case 0x59:  SHLR8(Rn); break;
		case 0x99:  SHLR8(Rn); break;
		case 0xd9:  SHLR8(Rn); break;

		case 0x29:  SHLR16(Rn); break;
		case 0x69:  SHLR16(Rn); break;
		case 0xa9:  SHLR16(Rn); break;
		case 0xe9:  SHLR16(Rn); break;

		case 0x39:  ILLEGAL(); break;
		case 0x79:  ILLEGAL(); break;
		case 0xb9:  ILLEGAL(); break;
		case 0xf9:  ILLEGAL(); break;
		// 0xa0
		case 0x0a:  LDSMACH(Rn); break;
		case 0x1a:  LDSMACL(Rn); break;
		case 0x2a:  LDSPR(Rn); break;
		case 0x3a:  ILLEGAL(); break;
		case 0x4a:  ILLEGAL(); break;
		case 0x5a:  LDSFPUL(opcode); break; // sh4 only
		case 0x6a:  LDSFPSCR(opcode); break; // sh4 only
		case 0x7a:  ILLEGAL(); break;
		case 0x8a:  ILLEGAL(); break;
		case 0x9a:  ILLEGAL(); break;
		case 0xaa:  ILLEGAL(); break;
		case 0xba:  ILLEGAL(); break;
		case 0xca:  ILLEGAL(); break;
		case 0xda:  ILLEGAL(); break;
		case 0xea:  ILLEGAL(); break;
		case 0xfa:  LDCDBR(opcode); break; // sh4 only
		// 0xb0
		case 0x0b:  JSR(Rn);  break;
		case 0x4b:  JSR(Rn);  break;
		case 0x8b:  JSR(Rn);  break;
		case 0xcb:  JSR(Rn);  break;

		case 0x1b:  TAS(Rn); break;
		case 0x5b:  TAS(Rn); break;
		case 0x9b:  TAS(Rn); break;
		case 0xdb:  TAS(Rn); break;

		case 0x2b:  JMP(Rn);  break;
		case 0x6b:  JMP(Rn);  break;
		case 0xab:  JMP(Rn);  break;
		case 0xeb:  JMP(Rn);  break;

		case 0x3b:  ILLEGAL(); break;
		case 0x7b:  ILLEGAL(); break;
		case 0xbb:  ILLEGAL(); break;
		case 0xfb:  ILLEGAL(); break;
		// 0xc0
		case 0x0c:  SHAD(opcode); break; // sh4 only
		case 0x1c:  SHAD(opcode); break;
		case 0x2c:  SHAD(opcode); break;
		case 0x3c:  SHAD(opcode); break;
		case 0x4c:  SHAD(opcode); break;
		case 0x5c:  SHAD(opcode); break;
		case 0x6c:  SHAD(opcode); break;
		case 0x7c:  SHAD(opcode); break;
		case 0x8c:  SHAD(opcode); break;
		case 0x9c:  SHAD(opcode); break;
		case 0xac:  SHAD(opcode); break;
		case 0xbc:  SHAD(opcode); break;
		case 0xcc:  SHAD(opcode); break;
		case 0xdc:  SHAD(opcode); break;
		case 0xec:  SHAD(opcode); break;
		case 0xfc:  SHAD(opcode); break;
		// 0xd0
		case 0x0d:  SHLD(opcode); break; // sh4 only
		case 0x1d:  SHLD(opcode); break;
		case 0x2d:  SHLD(opcode); break;
		case 0x3d:  SHLD(opcode); break;
		case 0x4d:  SHLD(opcode); break;
		case 0x5d:  SHLD(opcode); break;
		case 0x6d:  SHLD(opcode); break;
		case 0x7d:  SHLD(opcode); break;
		case 0x8d:  SHLD(opcode); break;
		case 0x9d:  SHLD(opcode); break;
		case 0xad:  SHLD(opcode); break;
		case 0xbd:  SHLD(opcode); break;
		case 0xcd:  SHLD(opcode); break;
		case 0xdd:  SHLD(opcode); break;
		case 0xed:  SHLD(opcode); break;
		case 0xfd:  SHLD(opcode); break;
		// 0xe0
		case 0x0e:  LDCSR(opcode); break; // sh2/4 flag difference
		case 0x1e:  LDCGBR(Rn); break;
		case 0x2e:  LDCVBR(Rn); break;
		case 0x3e:  LDCSSR(opcode); break; // sh4 only
		case 0x4e:  LDCSPC(opcode); break; // sh4 only
		case 0x5e:  ILLEGAL(); break;
		case 0x6e:  ILLEGAL(); break;
		case 0x7e:  ILLEGAL(); break;
		case 0x8e:  LDCRBANK(opcode); break; // sh4 only
		case 0x9e:  LDCRBANK(opcode); break;
		case 0xae:  LDCRBANK(opcode); break;
		case 0xbe:  LDCRBANK(opcode); break;
		case 0xce:  LDCRBANK(opcode); break;
		case 0xde:  LDCRBANK(opcode); break;
		case 0xee:  LDCRBANK(opcode); break;
		case 0xfe:  LDCRBANK(opcode); break;
		// 0xf0
		case 0x0f:  MAC_W(Rm, Rn); break;
		case 0x1f:  MAC_W(Rm, Rn); break;
		case 0x2f:  MAC_W(Rm, Rn); break;
		case 0x3f:  MAC_W(Rm, Rn); break;
		case 0x4f:  MAC_W(Rm, Rn); break;
		case 0x5f:  MAC_W(Rm, Rn); break;
		case 0x6f:  MAC_W(Rm, Rn); break;
		case 0x7f:  MAC_W(Rm, Rn); break;
		case 0x8f:  MAC_W(Rm, Rn); break;
		case 0x9f:  MAC_W(Rm, Rn); break;
		case 0xaf:  MAC_W(Rm, Rn); break;
		case 0xbf:  MAC_W(Rm, Rn); break;
		case 0xcf:  MAC_W(Rm, Rn); break;
		case 0xdf:  MAC_W(Rm, Rn); break;
		case 0xef:  MAC_W(Rm, Rn); break;
		case 0xff:  MAC_W(Rm, Rn); break;
	}
}


inline void sh34_base_device::execute_one(const uint16_t opcode)
{
	switch(opcode & 0xf000)
	{
		case 0x0000:
			execute_one_0000(opcode);
			break;

		case 0x1000:
			MOVLS4(Rm, opcode & 0x0f, Rn);
			break;

		case 0x2000: // void sh2_device::op0010(uint16_t opcode)
			op0010(opcode);
			break;

		case 0x3000: // void sh2_device::op0011(uint16_t opcode)
			op0011(opcode);
			break;

		case 0x4000:
			execute_one_4000(opcode);
			break;

		case 0x5000:
			MOVLL4(Rm, opcode & 0x0f, Rn);
			break;

		case 0x6000: // void sh2_device::op0110(uint16_t opcode)
			op0110(opcode);
			break;

		case 0x7000:
			ADDI(opcode & 0xff, Rn);
			break;

		case 0x8000:
			op1000(opcode);
			break;

		case 0x9000:
			MOVWI(opcode & 0xff, Rn);
			break;

		case 0xa000:
			BRA(opcode & 0xfff);
			break;

		case 0xb000:
			BSR(opcode & 0xfff);
			break;

		case 0xc000:
			op1100(opcode);
			break;

		case 0xd000:
			MOVLI(opcode & 0xff, Rn);
			break;

		case 0xe000:
			MOVI(opcode & 0xff, Rn);
			break;

		case 0xf000: // sh4 only
			switch(opcode & 0x0f)
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
			break;
	}
}


/* Execute cycles - returns number of cycles actually run */
void sh34_base_device::execute_run()
{
	if (m_cpu_off)
	{
		m_sh2_state->icount = 0;
		return;
	}

	do
	{
		m_ppc = m_sh2_state->pc & SH34_AM;
		debugger_instruction_hook(this, m_sh2_state->pc & SH34_AM);

		uint16_t opcode;

		if (!m_sh4_mmu_enabled) opcode = m_direct->read_word(m_sh2_state->pc & SH34_AM, WORD2_XOR_LE(0));
		else opcode = RW(m_sh2_state->pc); // should probably use a different function as this needs to go through the ITLB

		if (m_sh2_state->m_delay)
		{
			m_sh2_state->pc = m_sh2_state->m_delay;
			m_sh2_state->m_delay = 0;
		}
		else
			m_sh2_state->pc += 2;

		execute_one(opcode);

		if (m_test_irq && !m_sh2_state->m_delay)
		{
			sh4_check_pending_irq("mame_sh4_execute");
		}

		m_sh2_state->icount--;
	} while( m_sh2_state->icount > 0 );
}

void sh3be_device::execute_run()
{
	if (m_cpu_off)
	{
		m_sh2_state->icount = 0;
		return;
	}

	do
	{
		m_ppc = m_sh2_state->pc & SH34_AM;
		debugger_instruction_hook(this, m_sh2_state->pc & SH34_AM);

		const uint16_t opcode = m_direct->read_word(m_sh2_state->pc & SH34_AM, WORD_XOR_LE(6));

		if (m_sh2_state->m_delay)
		{
			m_sh2_state->pc = m_sh2_state->m_delay;
			m_sh2_state->m_delay = 0;
		}
		else
			m_sh2_state->pc += 2;

		execute_one(opcode);

		if (m_test_irq && !m_sh2_state->m_delay)
		{
			sh4_check_pending_irq("mame_sh4_execute");
		}

		m_sh2_state->icount--;
	} while( m_sh2_state->icount > 0 );
}

void sh4be_device::execute_run()
{
	if (m_cpu_off)
	{
		m_sh2_state->icount = 0;
		return;
	}

	do
	{
		m_ppc = m_sh2_state->pc & SH34_AM;
		debugger_instruction_hook(this, m_sh2_state->pc & SH34_AM);

		const uint16_t opcode = m_direct->read_word(m_sh2_state->pc & SH34_AM, WORD_XOR_LE(6));

		if (m_sh2_state->m_delay)
		{
			m_sh2_state->pc = m_sh2_state->m_delay;
			m_sh2_state->m_delay = 0;
		}
		else
			m_sh2_state->pc += 2;

		execute_one(opcode);

		if (m_test_irq && !m_sh2_state->m_delay)
		{
			sh4_check_pending_irq("mame_sh4_execute");
		}

		m_sh2_state->icount--;
	} while( m_sh2_state->icount > 0 );
}

void sh4_base_device::device_start()
{
	sh34_base_device::device_start();

	int i;
	for (i=0;i<64;i++)
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

	for (i=0;i<64;i++)
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
	/* allocate the implementation-specific state from the full cache */
	m_sh2_state = (internal_sh2_state *)m_cache.alloc_near(sizeof(internal_sh2_state));

	for (int i=0; i<3; i++)
	{
		m_timer[i] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(sh34_base_device::sh4_timer_callback), this));
		m_timer[i]->adjust(attotime::never, i);
	}

	for (int i=0; i<4; i++)
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
	m_direct = &m_program->direct();
	sh4_default_exception_priorities();
	m_irln = 15;
	m_test_irq = 0;

	save_item(NAME(m_sh2_state->pc));
	save_item(NAME(m_sh2_state->r));
	save_item(NAME(m_sh2_state->sr));
	save_item(NAME(m_sh2_state->pr));
	save_item(NAME(m_sh2_state->gbr));
	save_item(NAME(m_sh2_state->vbr));
	save_item(NAME(m_sh2_state->mach));
	save_item(NAME(m_sh2_state->macl));
	save_item(NAME(m_spc));
	save_item(NAME(m_ssr));
	save_item(NAME(m_sgr));
	save_item(NAME(m_fpscr));
	save_item(NAME(m_rbnk));
	save_item(NAME(m_fr));
	save_item(NAME(m_xf));
	save_item(NAME(m_sh2_state->ea));
	save_item(NAME(m_sh2_state->m_delay));
	save_item(NAME(m_cpu_off));
	save_item(NAME(m_pending_irq));
	save_item(NAME(m_test_irq));
	save_item(NAME(m_fpul));
	save_item(NAME(m_dbr));
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
	save_item(NAME(m_sh2_state->sleep_mode));
	save_item(NAME(m_frt_input));
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
	save_item(NAME(m_sh2_state->icount));
	save_item(NAME(m_fpu_sz));
	save_item(NAME(m_fpu_pr));
	save_item(NAME(m_ioport16_pullup));
	save_item(NAME( m_ioport16_direction));
	save_item(NAME(m_ioport4_pullup));
	save_item(NAME(m_ioport4_direction));
	save_item(NAME(m_sh4_mmu_enabled));
	save_item(NAME(m_sh3internal_upper));
	save_item(NAME(m_sh3internal_lower));

	// Debugger state

	state_add(SH4_PC,             "PC", m_sh2_state->pc).formatstr("%08X").callimport();
	state_add(SH4_SR,             "SR", m_sh2_state->sr).formatstr("%08X").callimport();
	state_add(SH4_PR,             "PR", m_sh2_state->pr).formatstr("%08X");
	state_add(SH4_GBR,            "GBR", m_sh2_state->gbr).formatstr("%08X");
	state_add(SH4_VBR,            "VBR", m_sh2_state->vbr).formatstr("%08X");
	state_add(SH4_DBR,            "DBR", m_dbr).formatstr("%08X");
	state_add(SH4_MACH,           "MACH", m_sh2_state->mach).formatstr("%08X");
	state_add(SH4_MACL,           "MACL", m_sh2_state->macl).formatstr("%08X");
	state_add(SH4_R0,             "R0", m_sh2_state->r[ 0]).formatstr("%08X");
	state_add(SH4_R1,             "R1", m_sh2_state->r[ 1]).formatstr("%08X");
	state_add(SH4_R2,             "R2", m_sh2_state->r[ 2]).formatstr("%08X");
	state_add(SH4_R3,             "R3", m_sh2_state->r[ 3]).formatstr("%08X");
	state_add(SH4_R4,             "R4", m_sh2_state->r[ 4]).formatstr("%08X");
	state_add(SH4_R5,             "R5", m_sh2_state->r[ 5]).formatstr("%08X");
	state_add(SH4_R6,             "R6", m_sh2_state->r[ 6]).formatstr("%08X");
	state_add(SH4_R7,             "R7", m_sh2_state->r[ 7]).formatstr("%08X");
	state_add(SH4_R8,             "R8", m_sh2_state->r[ 8]).formatstr("%08X");
	state_add(SH4_R9,             "R9", m_sh2_state->r[ 9]).formatstr("%08X");
	state_add(SH4_R10,            "R10", m_sh2_state->r[10]).formatstr("%08X");
	state_add(SH4_R11,            "R11", m_sh2_state->r[11]).formatstr("%08X");
	state_add(SH4_R12,            "R12", m_sh2_state->r[12]).formatstr("%08X");
	state_add(SH4_R13,            "R13", m_sh2_state->r[13]).formatstr("%08X");
	state_add(SH4_R14,            "R14", m_sh2_state->r[14]).formatstr("%08X");
	state_add(SH4_R15,            "R15", m_sh2_state->r[15]).formatstr("%08X");
	state_add(SH4_EA,             "EA", m_sh2_state->ea).formatstr("%08X");
	state_add(SH4_R0_BK0,         "R0 BK 0", m_rbnk[0][0]).formatstr("%08X");
	state_add(SH4_R1_BK0,         "R1 BK 0", m_rbnk[0][1]).formatstr("%08X");
	state_add(SH4_R2_BK0,         "R2 BK 0", m_rbnk[0][2]).formatstr("%08X");
	state_add(SH4_R3_BK0,         "R3 BK 0", m_rbnk[0][3]).formatstr("%08X");
	state_add(SH4_R4_BK0,         "R4 BK 0", m_rbnk[0][4]).formatstr("%08X");
	state_add(SH4_R5_BK0,         "R5 BK 0", m_rbnk[0][5]).formatstr("%08X");
	state_add(SH4_R6_BK0,         "R6 BK 0", m_rbnk[0][6]).formatstr("%08X");
	state_add(SH4_R7_BK0,         "R7 BK 0", m_rbnk[0][7]).formatstr("%08X");
	state_add(SH4_R0_BK1,         "R0 BK 1", m_rbnk[1][0]).formatstr("%08X");
	state_add(SH4_R1_BK1,         "R1 BK 1", m_rbnk[1][1]).formatstr("%08X");
	state_add(SH4_R2_BK1,         "R2 BK 1", m_rbnk[1][2]).formatstr("%08X");
	state_add(SH4_R3_BK1,         "R3 BK 1", m_rbnk[1][3]).formatstr("%08X");
	state_add(SH4_R4_BK1,         "R4 BK 1", m_rbnk[1][4]).formatstr("%08X");
	state_add(SH4_R5_BK1,         "R5 BK 1", m_rbnk[1][5]).formatstr("%08X");
	state_add(SH4_R6_BK1,         "R6 BK 1", m_rbnk[1][6]).formatstr("%08X");
	state_add(SH4_R7_BK1,         "R7 BK 1", m_rbnk[1][7]).formatstr("%08X");
	state_add(SH4_SPC,            "SPC", m_spc).formatstr("%08X");
	state_add(SH4_SSR,            "SSR", m_ssr).formatstr("%08X");
	state_add(SH4_SGR,            "SGR", m_sgr).formatstr("%08X");
	state_add(SH4_FPSCR,          "FPSCR", m_fpscr).formatstr("%08X");
	state_add(SH4_FPUL,           "FPUL", m_fpul).formatstr("%08X");

	state_add(SH4_FR0,            "FR0", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_FR1,            "FR1", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_FR2,            "FR2", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_FR3,            "FR3", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_FR4,            "FR4", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_FR5,            "FR5", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_FR6,            "FR6", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_FR7,            "FR7", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_FR8,            "FR8", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_FR9,            "FR9", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_FR10,           "FR10", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_FR11,           "FR11", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_FR12,           "FR12", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_FR13,           "FR13", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_FR14,           "FR14", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_FR15,           "FR15", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_XF0,            "XF0", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_XF1,            "XF1", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_XF2,            "XF2", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_XF3,            "XF3", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_XF4,            "XF4", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_XF5,            "XF5", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_XF6,            "XF6", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_XF7,            "XF7", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_XF8,            "XF8", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_XF9,            "XF9", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_XF10,           "XF10", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_XF11,           "XF11", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_XF12,           "XF12", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_XF13,           "XF13", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_XF14,           "XF14", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_XF15,           "XF15", m_debugger_temp).callimport().formatstr("%25s");

	state_add(STATE_GENPC, "GENPC", m_debugger_temp).callimport().callexport().noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_ppc).noshow();
	state_add(STATE_GENSP, "GENSP", m_sh2_state->r[15]).noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS", m_sh2_state->sr).formatstr("%20s").noshow();

	m_icountptr = &m_sh2_state->icount;
}

void sh34_base_device::state_import(const device_state_entry &entry)
{
#ifdef LSB_FIRST
	uint8_t fpu_xor = m_fpu_pr;
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

		case SH4_SR:
			sh4_exception_recompute();
			sh4_check_pending_irq("sh4_set_info");
			break;

		case SH4_FR0:
			m_fr[0 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_FR1:
			m_fr[1 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_FR2:
			m_fr[2 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_FR3:
			m_fr[3 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_FR4:
			m_fr[4 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_FR5:
			m_fr[5 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_FR6:
			m_fr[6 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_FR7:
			m_fr[7 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_FR8:
			m_fr[8 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_FR9:
			m_fr[9 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_FR10:
			m_fr[10 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_FR11:
			m_fr[11 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_FR12:
			m_fr[12 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_FR13:
			m_fr[13 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_FR14:
			m_fr[14 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_FR15:
			m_fr[15 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_XF0:
			m_xf[0 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_XF1:
			m_xf[1 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_XF2:
			m_xf[2 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_XF3:
			m_xf[3 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_XF4:
			m_xf[4 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_XF5:
			m_xf[5 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_XF6:
			m_xf[6 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_XF7:
			m_xf[7 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_XF8:
			m_xf[8 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_XF9:
			m_xf[9 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_XF10:
			m_xf[10 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_XF11:
			m_xf[11 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_XF12:
			m_xf[12 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_XF13:
			m_xf[13 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_XF14:
			m_xf[14 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_XF15:
			m_xf[15 ^ fpu_xor] = m_debugger_temp;
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
	uint8_t fpu_xor = m_fpu_pr;
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
					m_sh2_state->sr & M ? 'M':'.',
					m_sh2_state->sr & Q ? 'Q':'.',
					(m_sh2_state->sr & I) >> 4,
					m_sh2_state->sr & S ? 'S':'.',
					m_sh2_state->sr & T ? 'T':'.');
			break;

		case SH4_FR0:
			str = string_format("%08X %f", m_fr[0 ^ fpu_xor], (double)FP_RFS(0 ^ fpu_xor));
			break;

		case SH4_FR1:
			str = string_format("%08X %f", m_fr[1 ^ fpu_xor], (double)FP_RFS(1 ^ fpu_xor));
			break;

		case SH4_FR2:
			str = string_format("%08X %f", m_fr[2 ^ fpu_xor], (double)FP_RFS(2 ^ fpu_xor));
			break;

		case SH4_FR3:
			str = string_format("%08X %f", m_fr[3 ^ fpu_xor], (double)FP_RFS(3 ^ fpu_xor));
			break;

		case SH4_FR4:
			str = string_format("%08X %f", m_fr[4 ^ fpu_xor], (double)FP_RFS(4 ^ fpu_xor));
			break;

		case SH4_FR5:
			str = string_format("%08X %f", m_fr[5 ^ fpu_xor], (double)FP_RFS(5 ^ fpu_xor));
			break;

		case SH4_FR6:
			str = string_format("%08X %f", m_fr[6 ^ fpu_xor], (double)FP_RFS(6 ^ fpu_xor));
			break;

		case SH4_FR7:
			str = string_format("%08X %f", m_fr[7 ^ fpu_xor], (double)FP_RFS(7 ^ fpu_xor));
			break;

		case SH4_FR8:
			str = string_format("%08X %f", m_fr[8 ^ fpu_xor], (double)FP_RFS(8 ^ fpu_xor));
			break;

		case SH4_FR9:
			str = string_format("%08X %f", m_fr[9 ^ fpu_xor], (double)FP_RFS(9 ^ fpu_xor));
			break;

		case SH4_FR10:
			str = string_format("%08X %f", m_fr[10 ^ fpu_xor], (double)FP_RFS(10 ^ fpu_xor));
			break;

		case SH4_FR11:
			str = string_format("%08X %f", m_fr[11 ^ fpu_xor], (double)FP_RFS(11 ^ fpu_xor));
			break;

		case SH4_FR12:
			str = string_format("%08X %f", m_fr[12 ^ fpu_xor], (double)FP_RFS(12 ^ fpu_xor));
			break;

		case SH4_FR13:
			str = string_format("%08X %f", m_fr[13 ^ fpu_xor], (double)FP_RFS(13 ^ fpu_xor));
			break;

		case SH4_FR14:
			str = string_format("%08X %f", m_fr[14 ^ fpu_xor], (double)FP_RFS(14 ^ fpu_xor));
			break;

		case SH4_FR15:
			str = string_format("%08X %f", m_fr[15 ^ fpu_xor], (double)FP_RFS(15 ^ fpu_xor));
			break;

		case SH4_XF0:
			str = string_format("%08X %f", m_xf[0 ^ fpu_xor], (double)FP_XFS(0 ^ fpu_xor));
			break;

		case SH4_XF1:
			str = string_format("%08X %f", m_xf[1 ^ fpu_xor], (double)FP_XFS(1 ^ fpu_xor));
			break;

		case SH4_XF2:
			str = string_format("%08X %f", m_xf[2 ^ fpu_xor], (double)FP_XFS(2 ^ fpu_xor));
			break;

		case SH4_XF3:
			str = string_format("%08X %f", m_xf[3 ^ fpu_xor], (double)FP_XFS(3 ^ fpu_xor));
			break;

		case SH4_XF4:
			str = string_format("%08X %f", m_xf[4 ^ fpu_xor], (double)FP_XFS(4 ^ fpu_xor));
			break;

		case SH4_XF5:
			str = string_format("%08X %f", m_xf[5 ^ fpu_xor], (double)FP_XFS(5 ^ fpu_xor));
			break;

		case SH4_XF6:
			str = string_format("%08X %f", m_xf[6 ^ fpu_xor], (double)FP_XFS(6 ^ fpu_xor));
			break;

		case SH4_XF7:
			str = string_format("%08X %f", m_xf[7 ^ fpu_xor], (double)FP_XFS(7 ^ fpu_xor));
			break;

		case SH4_XF8:
			str = string_format("%08X %f", m_xf[8 ^ fpu_xor], (double)FP_XFS(8 ^ fpu_xor));
			break;

		case SH4_XF9:
			str = string_format("%08X %f", m_xf[9 ^ fpu_xor], (double)FP_XFS(9 ^ fpu_xor));
			break;

		case SH4_XF10:
			str = string_format("%08X %f", m_xf[10 ^ fpu_xor], (double)FP_XFS(10 ^ fpu_xor));
			break;

		case SH4_XF11:
			str = string_format("%08X %f", m_xf[11 ^ fpu_xor], (double)FP_XFS(11 ^ fpu_xor));
			break;

		case SH4_XF12:
			str = string_format("%08X %f", m_xf[12 ^ fpu_xor], (double)FP_XFS(12 ^ fpu_xor));
			break;

		case SH4_XF13:
			str = string_format("%08X %f", m_xf[13 ^ fpu_xor], (double)FP_XFS(13 ^ fpu_xor));
			break;

		case SH4_XF14:
			str = string_format("%08X %f", m_xf[14 ^ fpu_xor], (double)FP_XFS(14 ^ fpu_xor));
			break;

		case SH4_XF15:
			str = string_format("%08X %f", m_xf[15 ^ fpu_xor], (double)FP_XFS(15 ^ fpu_xor));
			break;

	}
}


void sh34_base_device::sh4_set_ftcsr_callback(sh4_ftcsr_callback callback)
{
	m_ftcsr_read_callback = callback;
}
