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
#include "debugger.h"
#include "sh4.h"
#include "sh4regs.h"
#include "sh4comn.h"
#include "sh3comn.h"
#include "sh4tmu.h"

#if SH4_USE_FASTRAM_OPTIMIZATION
void sh34_base_device::add_fastram(offs_t start, offs_t end, UINT8 readonly, void *base)
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
void sh34_base_device::add_fastram(offs_t start, offs_t end, UINT8 readonly, void *base)
{
}
#endif


CPU_DISASSEMBLE( sh4 );
CPU_DISASSEMBLE( sh4be );


const device_type SH3LE = &device_creator<sh3_device>;
const device_type SH3BE = &device_creator<sh3be_device>;
const device_type SH4LE = &device_creator<sh4_device>;
const device_type SH4BE = &device_creator<sh4be_device>;


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
	AM_RANGE(0xF6000000, 0xF7FFFFFF) AM_READWRITE(sh4_tlb_r,sh4_tlb_w)
	AM_RANGE(0xFE000000, 0xFFFFFFFF) AM_READWRITE32(sh4_internal_r, sh4_internal_w, U64(0xffffffffffffffff))
ADDRESS_MAP_END

static ADDRESS_MAP_START( sh3_internal_map, AS_PROGRAM, 64, sh3_base_device )
	AM_RANGE(SH3_LOWER_REGBASE, SH3_LOWER_REGEND) AM_READWRITE32(sh3_internal_r, sh3_internal_w, U64(0xffffffffffffffff))
	AM_RANGE(SH3_UPPER_REGBASE, SH3_UPPER_REGEND) AM_READWRITE32(sh3_internal_high_r, sh3_internal_high_w, U64(0xffffffffffffffff))
ADDRESS_MAP_END


sh34_base_device::sh34_base_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, endianness_t endianness, address_map_constructor internal)
	: cpu_device(mconfig, type, name, tag, owner, clock, shortname, __FILE__)
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


sh3_base_device::sh3_base_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, endianness_t endianness)
	: sh34_base_device(mconfig, type, name, tag, owner, clock, shortname, endianness, ADDRESS_MAP_NAME(sh3_internal_map))
{
	m_cpu_type = CPU_TYPE_SH3;
}


sh4_base_device::sh4_base_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, endianness_t endianness)
	: sh34_base_device(mconfig, type, name, tag, owner, clock, shortname, endianness, ADDRESS_MAP_NAME(sh4_internal_map))
{
	m_cpu_type = CPU_TYPE_SH4;
}


sh3_device::sh3_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: sh3_base_device(mconfig, SH3LE, "SH-3 (little)", tag, owner, clock, "sh3", ENDIANNESS_LITTLE)
{
}


sh3be_device::sh3be_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: sh3_base_device(mconfig, SH3BE, "SH-3 (big)", tag, owner, clock, "sh3be", ENDIANNESS_BIG)
{
}


sh4_device::sh4_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: sh4_base_device(mconfig, SH4LE, "SH-4 (little)", tag, owner, clock, "sh4", ENDIANNESS_LITTLE)
{
}


sh4be_device::sh4be_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: sh4_base_device(mconfig, SH4BE, "SH-4 (big)", tag, owner, clock, "sh4be", ENDIANNESS_BIG)
{
}


offs_t sh34_base_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( sh4 );

	return CPU_DISASSEMBLE_NAME(sh4)(this, buffer, pc, oprom, opram, options);
}


offs_t sh3be_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( sh4be );

	return CPU_DISASSEMBLE_NAME(sh4be)(this, buffer, pc, oprom, opram, options);
}


offs_t sh4be_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( sh4be );

	return CPU_DISASSEMBLE_NAME(sh4be)(this, buffer, pc, oprom, opram, options);
}


/* Called for unimplemented opcodes */
void sh34_base_device::TODO(const UINT16 opcode)
{
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
UINT32 abs;

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

inline UINT8 sh34_base_device::RB(offs_t A)
{
	if (A >= 0xe0000000)
		return m_program->read_byte(A);

#if SH4_USE_FASTRAM_OPTIMIZATION
	const offs_t _A = A & AM;
	for (int ramnum = 0; ramnum < m_fastram_select; ramnum++)
	{
		if (_A < m_fastram[ramnum].start || _A > m_fastram[ramnum].end)
		{
			continue;
		}
		UINT8 *fastbase = (UINT8*)m_fastram[ramnum].base - m_fastram[ramnum].start;
		return fastbase[_A ^ m_byte_xor];
	}
	return m_program->read_byte(_A);
#else
	return m_program->read_byte(A & AM);
#endif

}

inline UINT16 sh34_base_device::RW(offs_t A)
{
	if (A >= 0xe0000000)
		return m_program->read_word(A);

#if SH4_USE_FASTRAM_OPTIMIZATION
	const offs_t _A = A & AM;
	for (int ramnum = 0; ramnum < m_fastram_select; ramnum++)
	{
		if (_A < m_fastram[ramnum].start || _A > m_fastram[ramnum].end)
		{
			continue;
		}
		UINT8 *fastbase = (UINT8*)m_fastram[ramnum].base - m_fastram[ramnum].start;
		return ((UINT16*)fastbase)[(_A ^ m_word_xor) >> 1];
	}
	return m_program->read_word(_A);
#else
	return m_program->read_word(A & AM);
#endif

}

inline UINT32 sh34_base_device::RL(offs_t A)
{
	if (A >= 0xe0000000)
		return m_program->read_dword(A);

#if SH4_USE_FASTRAM_OPTIMIZATION
	const offs_t _A = A & AM;
	for (int ramnum = 0; ramnum < m_fastram_select; ramnum++)
	{
		if (_A < m_fastram[ramnum].start || _A > m_fastram[ramnum].end)
		{
			continue;
		}
		UINT8 *fastbase = (UINT8*)m_fastram[ramnum].base - m_fastram[ramnum].start;
		return ((UINT32*)fastbase)[(_A^m_dword_xor) >> 2];
	}
	return m_program->read_dword(_A);
#else
	return m_program->read_dword(A & AM);
#endif

}

inline void sh34_base_device::WB(offs_t A, UINT8 V)
{
	if (A >= 0xe0000000)
	{
		m_program->write_byte(A,V);
		return;
	}
#if SH4_USE_FASTRAM_OPTIMIZATION
	const offs_t _A = A & AM;
	for (int ramnum = 0; ramnum < m_fastram_select; ramnum++)
	{
		if (m_fastram[ramnum].readonly == TRUE || _A < m_fastram[ramnum].start || _A > m_fastram[ramnum].end)
		{
			continue;
		}
		UINT8 *fastbase = (UINT8*)m_fastram[ramnum].base - m_fastram[ramnum].start;
		fastbase[_A ^ m_byte_xor] = V;
		return;
	}
	m_program->write_byte(_A,V);
#else
	m_program->write_byte(A & AM,V);
#endif

}

inline void sh34_base_device::WW(offs_t A, UINT16 V)
{
	if (A >= 0xe0000000)
	{
		m_program->write_word(A,V);
		return;
	}
#if SH4_USE_FASTRAM_OPTIMIZATION
	const offs_t _A = A & AM;
	for (int ramnum = 0; ramnum < m_fastram_select; ramnum++)
	{
		if (m_fastram[ramnum].readonly == TRUE || _A < m_fastram[ramnum].start || _A > m_fastram[ramnum].end)
		{
			continue;
		}
		void *fastbase = (UINT8*)m_fastram[ramnum].base - m_fastram[ramnum].start;
		((UINT16*)fastbase)[(_A ^ m_word_xor) >> 1] = V;
		return;
	}
	m_program->write_word(_A,V);
#else
	m_program->write_word(A & AM,V);
#endif

}

inline void sh34_base_device::WL(offs_t A, UINT32 V)
{
	if (A >= 0xe0000000)
	{
		m_program->write_dword(A,V);
		return;
	}
#if SH4_USE_FASTRAM_OPTIMIZATION
	const offs_t _A = A & AM;
	for (int ramnum = 0; ramnum < m_fastram_select; ramnum++)
	{
		if (m_fastram[ramnum].readonly == TRUE || _A < m_fastram[ramnum].start || _A > m_fastram[ramnum].end)
		{
			continue;
		}
		void *fastbase = (UINT8*)m_fastram[ramnum].base - m_fastram[ramnum].start;
		((UINT32*)fastbase)[(_A ^ m_dword_xor) >> 2] = V;
		return;
	}
	m_program->write_dword(_A,V);
#else
	m_program->write_dword(A & AM,V);
#endif
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 1100  1       -
 *  ADD     Rm,Rn
 */
inline void sh34_base_device::ADD(const UINT16 opcode)
{
	m_r[Rn] += m_r[Rm];
}

/*  code                 cycles  t-bit
 *  0111 nnnn iiii iiii  1       -
 *  ADD     #imm,Rn
 */
inline void sh34_base_device::ADDI(const UINT16 opcode)
{
	m_r[Rn] += (INT32)(INT16)(INT8)(opcode&0xff);
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 1110  1       carry
 *  ADDC    Rm,Rn
 */
inline void sh34_base_device::ADDC(const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;
	UINT32 tmp0, tmp1;

	tmp1 = m_r[n] + m_r[m];
	tmp0 = m_r[n];
	m_r[n] = tmp1 + (m_sr & T);
	if (tmp0 > tmp1)
		m_sr |= T;
	else
		m_sr &= ~T;
	if (tmp1 > m_r[n])
		m_sr |= T;
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 1111  1       overflow
 *  ADDV    Rm,Rn
 */
inline void sh34_base_device::ADDV(const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;
	INT32 dest, src, ans;

	if ((INT32) m_r[n] >= 0)
		dest = 0;
	else
		dest = 1;
	if ((INT32) m_r[m] >= 0)
		src = 0;
	else
		src = 1;
	src += dest;
	m_r[n] += m_r[m];
	if ((INT32) m_r[n] >= 0)
		ans = 0;
	else
		ans = 1;
	ans += dest;
	if (src == 0 || src == 2)
	{
		if (ans == 1)
			m_sr |= T;
		else
			m_sr &= ~T;
	}
	else
		m_sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0010 nnnn mmmm 1001  1       -
 *  AND     Rm,Rn
 */
inline void sh34_base_device::AND(const UINT16 opcode)
{
	m_r[Rn] &= m_r[Rm];
}


/*  code                 cycles  t-bit
 *  1100 1001 iiii iiii  1       -
 *  AND     #imm,R0
 */
inline void sh34_base_device::ANDI(const UINT16 opcode)
{
	m_r[0] &= (opcode&0xff);
}

/*  code                 cycles  t-bit
 *  1100 1101 iiii iiii  1       -
 *  AND.B   #imm,@(R0,GBR)
 */
inline void sh34_base_device::ANDM(const UINT16 opcode)
{
	UINT32 temp;

	m_ea = m_gbr + m_r[0];
	temp = (opcode&0xff) & RB( m_ea );
	WB(m_ea, temp );
	m_sh4_icount -= 2;
}

/*  code                 cycles  t-bit
 *  1000 1011 dddd dddd  3/1     -
 *  BF      disp8
 */
inline void sh34_base_device::BF(const UINT16 opcode)
{
	if ((m_sr & T) == 0)
	{
		INT32 disp = ((INT32)(opcode&0xff) << 24) >> 24;
		m_pc = m_ea = m_pc + disp * 2 + 2;
		m_sh4_icount -= 2;
	}
}

/*  code                 cycles  t-bit
 *  1000 1111 dddd dddd  3/1     -
 *  BFS     disp8
 */
inline void sh34_base_device::BFS(const UINT16 opcode)
{
	if ((m_sr & T) == 0)
	{
		INT32 disp = ((INT32)(opcode&0xff) << 24) >> 24;
		m_delay = m_pc;
		m_pc = m_ea = m_pc + disp * 2 + 2;
		m_sh4_icount--;
	}
}

/*  code                 cycles  t-bit
 *  1010 dddd dddd dddd  2       -
 *  BRA     disp12
 */
inline void sh34_base_device::BRA(const UINT16 opcode)
{
	INT32 disp = ((INT32)(opcode&0xfff) << 20) >> 20;

#if BUSY_LOOP_HACKS
	if (disp == -2)
	{
		UINT32 next_opcode = RW(m_ppc & AM);
		/* BRA  $
		 * NOP
		 */
		if (next_opcode == 0x0009)
			m_sh4_icount %= 3;   /* cycles for BRA $ and NOP taken (3) */
	}
#endif
	m_delay = m_pc;
	m_pc = m_ea = m_pc + disp * 2 + 2;
	m_sh4_icount--;
}

/*  code                 cycles  t-bit
 *  0000 mmmm 0010 0011  2       -
 *  BRAF    Rm
 */
inline void sh34_base_device::BRAF(const UINT16 opcode)
{
	m_delay = m_pc;
	m_pc += m_r[Rn] + 2;
	m_sh4_icount--;
}

/*  code                 cycles  t-bit
 *  1011 dddd dddd dddd  2       -
 *  BSR     disp12
 */
inline void sh34_base_device::BSR(const UINT16 opcode)
{
	INT32 disp = ((INT32)(opcode&0xfff) << 20) >> 20;

	m_pr = m_pc + 2;
	m_delay = m_pc;
	m_pc = m_ea = m_pc + disp * 2 + 2;
	m_sh4_icount--;
}

/*  code                 cycles  t-bit
 *  0000 mmmm 0000 0011  2       -
 *  BSRF    Rm
 */
inline void sh34_base_device::BSRF(const UINT16 opcode)
{
	m_pr = m_pc + 2;
	m_delay = m_pc;
	m_pc += m_r[Rn] + 2;
	m_sh4_icount--;
}

/*  code                 cycles  t-bit
 *  1000 1001 dddd dddd  3/1     -
 *  BT      disp8
 */
inline void sh34_base_device::BT(const UINT16 opcode)
{
	if ((m_sr & T) != 0)
	{
		INT32 disp = ((INT32)(opcode&0xff) << 24) >> 24;
		m_pc = m_ea = m_pc + disp * 2 + 2;
		m_sh4_icount -= 2;
	}
}

/*  code                 cycles  t-bit
 *  1000 1101 dddd dddd  2/1     -
 *  BTS     disp8
 */
inline void sh34_base_device::BTS(const UINT16 opcode)
{
	if ((m_sr & T) != 0)
	{
		INT32 disp = ((INT32)(opcode&0xff) << 24) >> 24;
		m_delay = m_pc;
		m_pc = m_ea = m_pc + disp * 2 + 2;
		m_sh4_icount--;
	}
}

/*  code                 cycles  t-bit
 *  0000 0000 0010 1000  1       -
 *  CLRMAC
 */
inline void sh34_base_device::CLRMAC(const UINT16 opcode)
{
	m_mach = 0;
	m_macl = 0;
}

/*  code                 cycles  t-bit
 *  0000 0000 0000 1000  1       -
 *  CLRT
 */
inline void sh34_base_device::CLRT(const UINT16 opcode)
{
	m_sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 0000  1       comparison result
 *  CMP_EQ  Rm,Rn
 */
inline void sh34_base_device::CMPEQ(const UINT16 opcode)
{
	if (m_r[Rn] == m_r[Rm])
		m_sr |= T;
	else
		m_sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 0011  1       comparison result
 *  CMP_GE  Rm,Rn
 */
inline void sh34_base_device::CMPGE(const UINT16 opcode)
{
	if ((INT32) m_r[Rn] >= (INT32) m_r[Rm])
		m_sr |= T;
	else
		m_sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 0111  1       comparison result
 *  CMP_GT  Rm,Rn
 */
inline void sh34_base_device::CMPGT(const UINT16 opcode)
{
	if ((INT32) m_r[Rn] > (INT32) m_r[Rm])
		m_sr |= T;
	else
		m_sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 0110  1       comparison result
 *  CMP_HI  Rm,Rn
 */
inline void sh34_base_device::CMPHI(const UINT16 opcode)
{
	if ((UINT32) m_r[Rn] > (UINT32) m_r[Rm])
		m_sr |= T;
	else
		m_sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 0010  1       comparison result
 *  CMP_HS  Rm,Rn
 */
inline void sh34_base_device::CMPHS(const UINT16 opcode)
{
	if ((UINT32) m_r[Rn] >= (UINT32) m_r[Rm])
		m_sr |= T;
	else
		m_sr &= ~T;
}


/*  code                 cycles  t-bit
 *  0100 nnnn 0001 0101  1       comparison result
 *  CMP_PL  Rn
 */
inline void sh34_base_device::CMPPL(const UINT16 opcode)
{
	if ((INT32) m_r[Rn] > 0)
		m_sr |= T;
	else
		m_sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0100 nnnn 0001 0001  1       comparison result
 *  CMP_PZ  Rn
 */
inline void sh34_base_device::CMPPZ(const UINT16 opcode)
{
	if ((INT32) m_r[Rn] >= 0)
		m_sr |= T;
	else
		m_sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0010 nnnn mmmm 1100  1       comparison result
 * CMP_STR  Rm,Rn
 */
inline void sh34_base_device::CMPSTR(const UINT16 opcode)
{
	UINT32 temp;
	INT32 HH, HL, LH, LL;
	temp = m_r[Rn] ^ m_r[Rm];
	HH = (temp >> 24) & 0xff;
	HL = (temp >> 16) & 0xff;
	LH = (temp >> 8) & 0xff;
	LL = temp & 0xff;
	if (HH && HL && LH && LL)
	m_sr &= ~T;
	else
	m_sr |= T;
	}


/*  code                 cycles  t-bit
 *  1000 1000 iiii iiii  1       comparison result
 *  CMP/EQ #imm,R0
 */
inline void sh34_base_device::CMPIM(const UINT16 opcode)
{
	UINT32 imm = (UINT32)(INT32)(INT16)(INT8)(opcode&0xff);

	if (m_r[0] == imm)
		m_sr |= T;
	else
		m_sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0010 nnnn mmmm 0111  1       calculation result
 *  DIV0S   Rm,Rn
 */
inline void sh34_base_device::DIV0S(const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;

	if ((m_r[n] & 0x80000000) == 0)
		m_sr &= ~Q;
	else
		m_sr |= Q;
	if ((m_r[m] & 0x80000000) == 0)
		m_sr &= ~M;
	else
		m_sr |= M;
	if ((m_r[m] ^ m_r[n]) & 0x80000000)
		m_sr |= T;
	else
		m_sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0000 0000 0001 1001  1       0
 *  DIV0U
 */
inline void sh34_base_device::DIV0U(const UINT16 opcode)
{
	m_sr &= ~(M | Q | T);
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 0100  1       calculation result
 *  DIV1 Rm,Rn
 */
inline void sh34_base_device::DIV1(const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;

	UINT32 tmp0;
	UINT32 old_q;

	old_q = m_sr & Q;
	if (0x80000000 & m_r[n])
		m_sr |= Q;
	else
		m_sr &= ~Q;

	m_r[n] = (m_r[n] << 1) | (m_sr & T);

	if (!old_q)
	{
		if (!(m_sr & M))
		{
			tmp0 = m_r[n];
			m_r[n] -= m_r[m];
			if(!(m_sr & Q))
				if(m_r[n] > tmp0)
					m_sr |= Q;
				else
					m_sr &= ~Q;
			else
				if(m_r[n] > tmp0)
					m_sr &= ~Q;
				else
					m_sr |= Q;
		}
		else
		{
			tmp0 = m_r[n];
			m_r[n] += m_r[m];
			if(!(m_sr & Q))
			{
				if(m_r[n] < tmp0)
					m_sr &= ~Q;
				else
					m_sr |= Q;
			}
			else
			{
				if(m_r[n] < tmp0)
					m_sr |= Q;
				else
					m_sr &= ~Q;
			}
		}
	}
	else
	{
		if (!(m_sr & M))
		{
			tmp0 = m_r[n];
			m_r[n] += m_r[m];
			if(!(m_sr & Q))
				if(m_r[n] < tmp0)
					m_sr |= Q;
				else
					m_sr &= ~Q;
			else
				if(m_r[n] < tmp0)
					m_sr &= ~Q;
				else
					m_sr |= Q;
		}
		else
		{
			tmp0 = m_r[n];
			m_r[n] -= m_r[m];
			if(!(m_sr & Q))
				if(m_r[n] > tmp0)
					m_sr &= ~Q;
				else
					m_sr |= Q;
			else
				if(m_r[n] > tmp0)
					m_sr |= Q;
				else
					m_sr &= ~Q;
		}
	}

	tmp0 = (m_sr & (Q | M));
	if((!tmp0) || (tmp0 == 0x300)) /* if Q == M set T else clear T */
		m_sr |= T;
	else
		m_sr &= ~T;
}

/*  DMULS.L Rm,Rn */
inline void sh34_base_device::DMULS(const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;

	UINT32 RnL, RnH, RmL, RmH, Res0, Res1, Res2;
	UINT32 temp0, temp1, temp2, temp3;
	INT32 tempm, tempn, fnLmL;

	tempn = (INT32) m_r[n];
	tempm = (INT32) m_r[m];
	if (tempn < 0)
		tempn = 0 - tempn;
	if (tempm < 0)
		tempm = 0 - tempm;
	if ((INT32) (m_r[n] ^ m_r[m]) < 0)
		fnLmL = -1;
	else
		fnLmL = 0;
	temp1 = (UINT32) tempn;
	temp2 = (UINT32) tempm;
	RnL = temp1 & 0x0000ffff;
	RnH = (temp1 >> 16) & 0x0000ffff;
	RmL = temp2 & 0x0000ffff;
	RmH = (temp2 >> 16) & 0x0000ffff;
	temp0 = RmL * RnL;
	temp1 = RmH * RnL;
	temp2 = RmL * RnH;
	temp3 = RmH * RnH;
	Res2 = 0;
	Res1 = temp1 + temp2;
	if (Res1 < temp1)
		Res2 += 0x00010000;
	temp1 = (Res1 << 16) & 0xffff0000;
	Res0 = temp0 + temp1;
	if (Res0 < temp0)
		Res2++;
	Res2 = Res2 + ((Res1 >> 16) & 0x0000ffff) + temp3;
	if (fnLmL < 0)
	{
		Res2 = ~Res2;
		if (Res0 == 0)
			Res2++;
		else
			Res0 = (~Res0) + 1;
	}
	m_mach = Res2;
	m_macl = Res0;
	m_sh4_icount--;
}

/*  DMULU.L Rm,Rn */
inline void sh34_base_device::DMULU(const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;

	UINT32 RnL, RnH, RmL, RmH, Res0, Res1, Res2;
	UINT32 temp0, temp1, temp2, temp3;

	RnL = m_r[n] & 0x0000ffff;
	RnH = (m_r[n] >> 16) & 0x0000ffff;
	RmL = m_r[m] & 0x0000ffff;
	RmH = (m_r[m] >> 16) & 0x0000ffff;
	temp0 = RmL * RnL;
	temp1 = RmH * RnL;
	temp2 = RmL * RnH;
	temp3 = RmH * RnH;
	Res2 = 0;
	Res1 = temp1 + temp2;
	if (Res1 < temp1)
		Res2 += 0x00010000;
	temp1 = (Res1 << 16) & 0xffff0000;
	Res0 = temp0 + temp1;
	if (Res0 < temp0)
		Res2++;
	Res2 = Res2 + ((Res1 >> 16) & 0x0000ffff) + temp3;
	m_mach = Res2;
	m_macl = Res0;
	m_sh4_icount--;
}

/*  DT      Rn */
inline void sh34_base_device::DT(const UINT16 opcode)
{
	UINT32 n = Rn;

	m_r[n]--;
	if (m_r[n] == 0)
		m_sr |= T;
	else
		m_sr &= ~T;
#if BUSY_LOOP_HACKS
	{
		UINT32 next_opcode = RW(m_ppc & AM);
		/* DT   Rn
		 * BF   $-2
		 */
		if (next_opcode == 0x8bfd)
		{
			while (m_r[n] > 1 && m_sh4_icount > 4)
			{
				m_r[n]--;
				m_sh4_icount -= 4;   /* cycles for DT (1) and BF taken (3) */
			}
		}
	}
#endif
}

/*  EXTS.B  Rm,Rn */
inline void sh34_base_device::EXTSB(const UINT16 opcode)
{
	m_r[Rn] = ((INT32)m_r[Rm] << 24) >> 24;
}

/*  EXTS.W  Rm,Rn */
inline void sh34_base_device::EXTSW(const UINT16 opcode)
{
	m_r[Rn] = ((INT32)m_r[Rm] << 16) >> 16;
}

/*  EXTU.B  Rm,Rn */
inline void sh34_base_device::EXTUB(const UINT16 opcode)
{
	m_r[Rn] = m_r[Rm] & 0x000000ff;
}

/*  EXTU.W  Rm,Rn */
inline void sh34_base_device::EXTUW(const UINT16 opcode)
{
	m_r[Rn] = m_r[Rm] & 0x0000ffff;
}

/*  JMP     @Rm */
inline void sh34_base_device::JMP(const UINT16 opcode)
{
	m_delay = m_pc;
	m_pc = m_ea = m_r[Rn];
}

/*  JSR     @Rm */
inline void sh34_base_device::JSR(const UINT16 opcode)
{
	m_delay = m_pc;
	m_pr = m_pc + 2;
	m_pc = m_ea = m_r[Rn];
	m_sh4_icount--;
}


/*  LDC     Rm,SR */
inline void sh34_base_device::LDCSR(const UINT16 opcode)
{
	UINT32 reg;

	reg = m_r[Rn];
	if ((machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
		sh4_syncronize_register_bank((m_sr & sRB) >> 29);
	if ((m_r[Rn] & sRB) != (m_sr & sRB))
		sh4_change_register_bank(m_r[Rn] & sRB ? 1 : 0);
	m_sr = reg & FLAGS;
	sh4_exception_recompute();
}

/*  LDC     Rm,GBR */
inline void sh34_base_device::LDCGBR(const UINT16 opcode)
{
	m_gbr = m_r[Rn];
}

/*  LDC     Rm,VBR */
inline void sh34_base_device::LDCVBR(const UINT16 opcode)
{
	m_vbr = m_r[Rn];
}

/*  LDC.L   @Rm+,SR */
inline void sh34_base_device::LDCMSR(const UINT16 opcode)
{
	UINT32 old;

	old = m_sr;
	m_ea = m_r[Rn];
	m_sr = RL(m_ea ) & FLAGS;
	if ((machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
		sh4_syncronize_register_bank((old & sRB) >> 29);
	if ((old & sRB) != (m_sr & sRB))
		sh4_change_register_bank(m_sr & sRB ? 1 : 0);
	m_r[Rn] += 4;
	m_sh4_icount -= 2;
	sh4_exception_recompute();
}

/*  LDC.L   @Rm+,GBR */
inline void sh34_base_device::LDCMGBR(const UINT16 opcode)
{
	m_ea = m_r[Rn];
	m_gbr = RL(m_ea );
	m_r[Rn] += 4;
	m_sh4_icount -= 2;
}

/*  LDC.L   @Rm+,VBR */
inline void sh34_base_device::LDCMVBR(const UINT16 opcode)
{
	m_ea = m_r[Rn];
	m_vbr = RL(m_ea );
	m_r[Rn] += 4;
	m_sh4_icount -= 2;
}

/*  LDS     Rm,MACH */
inline void sh34_base_device::LDSMACH(const UINT16 opcode)
{
	m_mach = m_r[Rn];
}

/*  LDS     Rm,MACL */
inline void sh34_base_device::LDSMACL(const UINT16 opcode)
{
	m_macl = m_r[Rn];
}

/*  LDS     Rm,PR */
inline void sh34_base_device::LDSPR(const UINT16 opcode)
{
	m_pr = m_r[Rn];
}

/*  LDS.L   @Rm+,MACH */
inline void sh34_base_device::LDSMMACH(const UINT16 opcode)
{
	m_ea = m_r[Rn];
	m_mach = RL(m_ea );
	m_r[Rn] += 4;
}

/*  LDS.L   @Rm+,MACL */
inline void sh34_base_device::LDSMMACL(const UINT16 opcode)
{
	m_ea = m_r[Rn];
	m_macl = RL(m_ea );
	m_r[Rn] += 4;
}

/*  LDS.L   @Rm+,PR */
inline void sh34_base_device::LDSMPR(const UINT16 opcode)
{
	m_ea = m_r[Rn];
	m_pr = RL(m_ea );
	m_r[Rn] += 4;
}

/*  MAC.L   @Rm+,@Rn+ */
inline void sh34_base_device::MAC_L(const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;

	UINT32 RnL, RnH, RmL, RmH, Res0, Res1, Res2;
	UINT32 temp0, temp1, temp2, temp3;
	INT32 tempm, tempn, fnLmL;

	tempn = (INT32) RL(m_r[n] );
	m_r[n] += 4;
	tempm = (INT32) RL(m_r[m] );
	m_r[m] += 4;
	if ((INT32) (tempn ^ tempm) < 0)
		fnLmL = -1;
	else
		fnLmL = 0;
	if (tempn < 0)
		tempn = 0 - tempn;
	if (tempm < 0)
		tempm = 0 - tempm;
	temp1 = (UINT32) tempn;
	temp2 = (UINT32) tempm;
	RnL = temp1 & 0x0000ffff;
	RnH = (temp1 >> 16) & 0x0000ffff;
	RmL = temp2 & 0x0000ffff;
	RmH = (temp2 >> 16) & 0x0000ffff;
	temp0 = RmL * RnL;
	temp1 = RmH * RnL;
	temp2 = RmL * RnH;
	temp3 = RmH * RnH;
	Res2 = 0;
	Res1 = temp1 + temp2;
	if (Res1 < temp1)
		Res2 += 0x00010000;
	temp1 = (Res1 << 16) & 0xffff0000;
	Res0 = temp0 + temp1;
	if (Res0 < temp0)
		Res2++;
	Res2 = Res2 + ((Res1 >> 16) & 0x0000ffff) + temp3;
	if (fnLmL < 0)
	{
		Res2 = ~Res2;
		if (Res0 == 0)
			Res2++;
		else
			Res0 = (~Res0) + 1;
	}
	if (m_sr & S)
	{
		Res0 = m_macl + Res0;
		if (m_macl > Res0)
			Res2++;
		Res2 += (m_mach & 0x0000ffff);
		if (((INT32) Res2 < 0) && (Res2 < 0xffff8000))
		{
			Res2 = 0x00008000;
			Res0 = 0x00000000;
		}
		else if (((INT32) Res2 > 0) && (Res2 > 0x00007fff))
		{
			Res2 = 0x00007fff;
			Res0 = 0xffffffff;
		}
		m_mach = Res2;
		m_macl = Res0;
	}
	else
	{
		Res0 = m_macl + Res0;
		if (m_macl > Res0)
			Res2++;
		Res2 += m_mach;
		m_mach = Res2;
		m_macl = Res0;
	}
	m_sh4_icount -= 2;
}

/*  MAC.W   @Rm+,@Rn+ */
inline void sh34_base_device::MAC_W(const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;

	INT32 tempm, tempn, dest, src, ans;
	UINT32 templ;

	tempn = (INT32) RW(m_r[n] );
	m_r[n] += 2;
	tempm = (INT32) RW(m_r[m] );
	m_r[m] += 2;
	templ = m_macl;
	tempm = ((INT32) (short) tempn * (INT32) (short) tempm);
	if ((INT32) m_macl >= 0)
		dest = 0;
	else
		dest = 1;
	if ((INT32) tempm >= 0)
	{
		src = 0;
		tempn = 0;
	}
	else
	{
		src = 1;
		tempn = 0xffffffff;
	}
	src += dest;
	m_macl += tempm;
	if ((INT32) m_macl >= 0)
		ans = 0;
	else
		ans = 1;
	ans += dest;
	if (m_sr & S)
	{
		if (ans == 1)
			{
				if (src == 0)
					m_macl = 0x7fffffff;
				if (src == 2)
					m_macl = 0x80000000;
			}
	}
	else
	{
		m_mach += tempn;
		if (templ > m_macl)
			m_mach += 1;
	}
	m_sh4_icount -= 2;
}

/*  MOV     Rm,Rn */
inline void sh34_base_device::MOV(const UINT16 opcode)
{
	m_r[Rn] = m_r[Rm];
}

/*  MOV.B   Rm,@Rn */
inline void sh34_base_device::MOVBS(const UINT16 opcode)
{
	m_ea = m_r[Rn];
	WB(m_ea, m_r[Rm] & 0x000000ff);
}

/*  MOV.W   Rm,@Rn */
inline void sh34_base_device::MOVWS(const UINT16 opcode)
{
	m_ea = m_r[Rn];
	WW(m_ea, m_r[Rm] & 0x0000ffff);
}

/*  MOV.L   Rm,@Rn */
inline void sh34_base_device::MOVLS(const UINT16 opcode)
{
	m_ea = m_r[Rn];
	WL(m_ea, m_r[Rm] );
}

/*  MOV.B   @Rm,Rn */
inline void sh34_base_device::MOVBL(const UINT16 opcode)
{
	m_ea = m_r[Rm];
	m_r[Rn] = (UINT32)(INT32)(INT16)(INT8) RB( m_ea );
}

/*  MOV.W   @Rm,Rn */
inline void sh34_base_device::MOVWL(const UINT16 opcode)
{
	m_ea = m_r[Rm];
	m_r[Rn] = (UINT32)(INT32)(INT16) RW(m_ea );
}

/*  MOV.L   @Rm,Rn */
inline void sh34_base_device::MOVLL(const UINT16 opcode)
{
	m_ea = m_r[Rm];
	m_r[Rn] = RL(m_ea );
}

/*  MOV.B   Rm,@-Rn */
inline void sh34_base_device::MOVBM(const UINT16 opcode)
{
	UINT32 data = m_r[Rm] & 0x000000ff;

	m_r[Rn] -= 1;
	WB(m_r[Rn], data );
}

/*  MOV.W   Rm,@-Rn */
inline void sh34_base_device::MOVWM(const UINT16 opcode)
{
	UINT32 data = m_r[Rm] & 0x0000ffff;

	m_r[Rn] -= 2;
	WW(m_r[Rn], data );
}

/*  MOV.L   Rm,@-Rn */
inline void sh34_base_device::MOVLM(const UINT16 opcode)
{
	UINT32 data = m_r[Rm];

	m_r[Rn] -= 4;
	WL(m_r[Rn], data );
}

/*  MOV.B   @Rm+,Rn */
inline void sh34_base_device::MOVBP(const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;

	m_r[n] = (UINT32)(INT32)(INT16)(INT8) RB( m_r[m] );
	if (n != m)
		m_r[m] += 1;
}

/*  MOV.W   @Rm+,Rn */
inline void sh34_base_device::MOVWP(const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;

	m_r[n] = (UINT32)(INT32)(INT16) RW(m_r[m] );
	if (n != m)
		m_r[m] += 2;
}

/*  MOV.L   @Rm+,Rn */
inline void sh34_base_device::MOVLP(const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;

	m_r[n] = RL(m_r[m] );
	if (n != m)
		m_r[m] += 4;
}

/*  MOV.B   Rm,@(R0,Rn) */
inline void sh34_base_device::MOVBS0(const UINT16 opcode)
{
	m_ea = m_r[Rn] + m_r[0];
	WB(m_ea, m_r[Rm] & 0x000000ff );
}

/*  MOV.W   Rm,@(R0,Rn) */
inline void sh34_base_device::MOVWS0(const UINT16 opcode)
{
	m_ea = m_r[Rn] + m_r[0];
	WW(m_ea, m_r[Rm] & 0x0000ffff );
}

/*  MOV.L   Rm,@(R0,Rn) */
inline void sh34_base_device::MOVLS0(const UINT16 opcode)
{
	m_ea = m_r[Rn] + m_r[0];
	WL(m_ea, m_r[Rm] );
}

/*  MOV.B   @(R0,Rm),Rn */
inline void sh34_base_device::MOVBL0(const UINT16 opcode)
{
	m_ea = m_r[Rm] + m_r[0];
	m_r[Rn] = (UINT32)(INT32)(INT16)(INT8) RB( m_ea );
}

/*  MOV.W   @(R0,Rm),Rn */
inline void sh34_base_device::MOVWL0(const UINT16 opcode)
{
	m_ea = m_r[Rm] + m_r[0];
	m_r[Rn] = (UINT32)(INT32)(INT16) RW(m_ea );
}

/*  MOV.L   @(R0,Rm),Rn */
inline void sh34_base_device::MOVLL0(const UINT16 opcode)
{
	m_ea = m_r[Rm] + m_r[0];
	m_r[Rn] = RL(m_ea );
}

/*  MOV     #imm,Rn */
inline void sh34_base_device::MOVI(const UINT16 opcode)
{
	m_r[Rn] = (UINT32)(INT32)(INT16)(INT8)(opcode&0xff);
}

/*  MOV.W   @(disp8,PC),Rn */
inline void sh34_base_device::MOVWI(const UINT16 opcode)
{
	UINT32 disp = opcode & 0xff;
	m_ea = m_pc + disp * 2 + 2;
	m_r[Rn] = (UINT32)(INT32)(INT16) RW(m_ea );
}

/*  MOV.L   @(disp8,PC),Rn */
inline void sh34_base_device::MOVLI(const UINT16 opcode)
{
	UINT32 disp = opcode & 0xff;
	m_ea = ((m_pc + 2) & ~3) + disp * 4;
	m_r[Rn] = RL(m_ea );
}

/*  MOV.B   @(disp8,GBR),R0 */
inline void sh34_base_device::MOVBLG(const UINT16 opcode)
{
	UINT32 disp = opcode & 0xff;
	m_ea = m_gbr + disp;
	m_r[0] = (UINT32)(INT32)(INT16)(INT8) RB( m_ea );
}

/*  MOV.W   @(disp8,GBR),R0 */
inline void sh34_base_device::MOVWLG(const UINT16 opcode)
{
	UINT32 disp = opcode & 0xff;
	m_ea = m_gbr + disp * 2;
	m_r[0] = (INT32)(INT16) RW(m_ea );
}

/*  MOV.L   @(disp8,GBR),R0 */
inline void sh34_base_device::MOVLLG(const UINT16 opcode)
{
	UINT32 disp = opcode & 0xff;
	m_ea = m_gbr + disp * 4;
	m_r[0] = RL(m_ea );
}

/*  MOV.B   R0,@(disp8,GBR) */
inline void sh34_base_device::MOVBSG(const UINT16 opcode)
{
	UINT32 disp = opcode & 0xff;
	m_ea = m_gbr + disp;
	WB(m_ea, m_r[0] & 0x000000ff );
}

/*  MOV.W   R0,@(disp8,GBR) */
inline void sh34_base_device::MOVWSG(const UINT16 opcode)
{
	UINT32 disp = opcode & 0xff;
	m_ea = m_gbr + disp * 2;
	WW(m_ea, m_r[0] & 0x0000ffff );
}

/*  MOV.L   R0,@(disp8,GBR) */
inline void sh34_base_device::MOVLSG(const UINT16 opcode)
{
	UINT32 disp = opcode & 0xff;
	m_ea = m_gbr + disp * 4;
	WL(m_ea, m_r[0] );
}

/*  MOV.B   R0,@(disp4,Rm) */
inline void sh34_base_device::MOVBS4(const UINT16 opcode)
{
	UINT32 disp = opcode & 0x0f;
	m_ea = m_r[Rm] + disp;
	WB(m_ea, m_r[0] & 0x000000ff );
}

/*  MOV.W   R0,@(disp4,Rm) */
inline void sh34_base_device::MOVWS4(const UINT16 opcode)
{
	UINT32 disp = opcode & 0x0f;
	m_ea = m_r[Rm] + disp * 2;
	WW(m_ea, m_r[0] & 0x0000ffff );
}

/* MOV.L Rm,@(disp4,Rn) */
inline void sh34_base_device::MOVLS4(const UINT16 opcode)
{
	UINT32 disp = opcode & 0x0f;
	m_ea = m_r[Rn] + disp * 4;
	WL(m_ea, m_r[Rm] );
}

/*  MOV.B   @(disp4,Rm),R0 */
inline void sh34_base_device::MOVBL4(const UINT16 opcode)
{
	UINT32 disp = opcode & 0x0f;
	m_ea = m_r[Rm] + disp;
	m_r[0] = (UINT32)(INT32)(INT16)(INT8) RB( m_ea );
}

/*  MOV.W   @(disp4,Rm),R0 */
inline void sh34_base_device::MOVWL4(const UINT16 opcode)
{
	UINT32 disp = opcode & 0x0f;
	m_ea = m_r[Rm] + disp * 2;
	m_r[0] = (UINT32)(INT32)(INT16) RW(m_ea );
}

/*  MOV.L   @(disp4,Rm),Rn */
inline void sh34_base_device::MOVLL4(const UINT16 opcode)
{
	UINT32 disp = opcode & 0x0f;
	m_ea = m_r[Rm] + disp * 4;
	m_r[Rn] = RL(m_ea );
}

/*  MOVA    @(disp8,PC),R0 */
inline void sh34_base_device::MOVA(const UINT16 opcode)
{
	UINT32 disp = opcode & 0xff;
	m_ea = ((m_pc + 2) & ~3) + disp * 4;
	m_r[0] = m_ea;
}

/*  MOVT    Rn */
void sh34_base_device::MOVT(const UINT16 opcode)
{
	m_r[Rn] = m_sr & T;
}

/*  MUL.L   Rm,Rn */
inline void sh34_base_device::MULL(const UINT16 opcode)
{
	m_macl = m_r[Rn] * m_r[Rm];
	m_sh4_icount--;
}

/*  MULS    Rm,Rn */
inline void sh34_base_device::MULS(const UINT16 opcode)
{
	m_macl = (INT16) m_r[Rn] * (INT16) m_r[Rm];
}

/*  MULU    Rm,Rn */
inline void sh34_base_device::MULU(const UINT16 opcode)
{
	m_macl = (UINT16) m_r[Rn] * (UINT16) m_r[Rm];
}

/*  NEG     Rm,Rn */
inline void sh34_base_device::NEG(const UINT16 opcode)
{
	m_r[Rn] = 0 - m_r[Rm];
}

/*  NEGC    Rm,Rn */
inline void sh34_base_device::NEGC(const UINT16 opcode)
{
	UINT32 temp;

	temp = m_r[Rm];
	m_r[Rn] = -temp - (m_sr & T);
	if (temp || (m_sr & T))
		m_sr |= T;
	else
		m_sr &= ~T;
}

/*  NOP */
inline void sh34_base_device::NOP(const UINT16 opcode)
{
}

/*  NOT     Rm,Rn */
inline void sh34_base_device::NOT(const UINT16 opcode)
{
	m_r[Rn] = ~m_r[Rm];
}

/*  OR      Rm,Rn */
inline void sh34_base_device::OR(const UINT16 opcode)
{
	m_r[Rn] |= m_r[Rm];
}

/*  OR      #imm,R0 */
inline void sh34_base_device::ORI(const UINT16 opcode)
{
	m_r[0] |= (opcode&0xff);
	m_sh4_icount -= 2;
}

/*  OR.B    #imm,@(R0,GBR) */
inline void sh34_base_device::ORM(const UINT16 opcode)
{
	UINT32 temp;

	m_ea = m_gbr + m_r[0];
	temp = RB( m_ea );
	temp |= (opcode&0xff);
	WB(m_ea, temp );
}

/*  ROTCL   Rn */
inline void sh34_base_device::ROTCL(const UINT16 opcode)
{
	UINT32 n = Rn;

	UINT32 temp;

	temp = (m_r[n] >> 31) & T;
	m_r[n] = (m_r[n] << 1) | (m_sr & T);
	m_sr = (m_sr & ~T) | temp;
}

/*  ROTCR   Rn */
inline void sh34_base_device::ROTCR(const UINT16 opcode)
{
	UINT32 n = Rn;

	UINT32 temp;
	temp = (m_sr & T) << 31;
	if (m_r[n] & T)
		m_sr |= T;
	else
		m_sr &= ~T;
	m_r[n] = (m_r[n] >> 1) | temp;
}

/*  ROTL    Rn */
inline void sh34_base_device::ROTL(const UINT16 opcode)
{
	UINT32 n = Rn;

	m_sr = (m_sr & ~T) | ((m_r[n] >> 31) & T);
	m_r[n] = (m_r[n] << 1) | (m_r[n] >> 31);
}

/*  ROTR    Rn */
inline void sh34_base_device::ROTR(const UINT16 opcode)
{
	UINT32 n = Rn;

	m_sr = (m_sr & ~T) | (m_r[n] & T);
	m_r[n] = (m_r[n] >> 1) | (m_r[n] << 31);
}

/*  RTE */
inline void sh34_base_device::RTE(const UINT16 opcode)
{
	m_delay = m_pc;
	m_pc = m_ea = m_spc;
	if ((machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
		sh4_syncronize_register_bank((m_sr & sRB) >> 29);
	if ((m_ssr & sRB) != (m_sr & sRB))
		sh4_change_register_bank(m_ssr & sRB ? 1 : 0);
	m_sr = m_ssr;
	m_sh4_icount--;
	sh4_exception_recompute();
}

/*  RTS */
inline void sh34_base_device::RTS(const UINT16 opcode)
{
	m_delay = m_pc;
	m_pc = m_ea = m_pr;
	m_sh4_icount--;
}

/*  SETT */
inline void sh34_base_device::SETT(const UINT16 opcode)
{
	m_sr |= T;
}

/*  SHAL    Rn      (same as SHLL) */
inline void sh34_base_device::SHAL(const UINT16 opcode)
{
	UINT32 n = Rn;

	m_sr = (m_sr & ~T) | ((m_r[n] >> 31) & T);
	m_r[n] <<= 1;
}

/*  SHAR    Rn */
inline void sh34_base_device::SHAR(const UINT16 opcode)
{
	UINT32 n = Rn;

	m_sr = (m_sr & ~T) | (m_r[n] & T);
	m_r[n] = (UINT32)((INT32)m_r[n] >> 1);
}

/*  SHLL    Rn      (same as SHAL) */
inline void sh34_base_device::SHLL(const UINT16 opcode)
{
	UINT32 n = Rn;

	m_sr = (m_sr & ~T) | ((m_r[n] >> 31) & T);
	m_r[n] <<= 1;
}

/*  SHLL2   Rn */
inline void sh34_base_device::SHLL2(const UINT16 opcode)
{
	m_r[Rn] <<= 2;
}

/*  SHLL8   Rn */
inline void sh34_base_device::SHLL8(const UINT16 opcode)
{
	m_r[Rn] <<= 8;
}

/*  SHLL16  Rn */
inline void sh34_base_device::SHLL16(const UINT16 opcode)
{
	m_r[Rn] <<= 16;
}

/*  SHLR    Rn */
inline void sh34_base_device::SHLR(const UINT16 opcode)
{
	UINT32 n = Rn;

	m_sr = (m_sr & ~T) | (m_r[n] & T);
	m_r[n] >>= 1;
}

/*  SHLR2   Rn */
inline void sh34_base_device::SHLR2(const UINT16 opcode)
{
	m_r[Rn] >>= 2;
}

/*  SHLR8   Rn */
inline void sh34_base_device::SHLR8(const UINT16 opcode)
{
	m_r[Rn] >>= 8;
}

/*  SHLR16  Rn */
inline void sh34_base_device::SHLR16(const UINT16 opcode)
{
	m_r[Rn] >>= 16;
}

/*  SLEEP */
inline void sh34_base_device::SLEEP(const UINT16 opcode)
{
	/* 0 = normal mode */
	/* 1 = enters into power-down mode */
	/* 2 = go out the power-down mode after an exception */
	if(m_sleep_mode != 2)
		m_pc -= 2;
	m_sh4_icount -= 2;
	/* Wait_for_exception; */
	if(m_sleep_mode == 0)
		m_sleep_mode = 1;
	else if(m_sleep_mode == 2)
		m_sleep_mode = 0;
}

/*  STC     SR,Rn */
inline void sh34_base_device::STCSR(const UINT16 opcode)
{
	m_r[Rn] = m_sr;
}

/*  STC     GBR,Rn */
inline void sh34_base_device::STCGBR(const UINT16 opcode)
{
	m_r[Rn] = m_gbr;
}

/*  STC     VBR,Rn */
inline void sh34_base_device::STCVBR(const UINT16 opcode)
{
	m_r[Rn] = m_vbr;
}

/*  STC.L   SR,@-Rn */
inline void sh34_base_device::STCMSR(const UINT16 opcode)
{
	UINT32 n = Rn;

	m_r[n] -= 4;
	m_ea = m_r[n];
	WL(m_ea, m_sr );
	m_sh4_icount--;
}

/*  STC.L   GBR,@-Rn */
inline void sh34_base_device::STCMGBR(const UINT16 opcode)
{
	UINT32 n = Rn;

	m_r[n] -= 4;
	m_ea = m_r[n];
	WL(m_ea, m_gbr );
	m_sh4_icount--;
}

/*  STC.L   VBR,@-Rn */
inline void sh34_base_device::STCMVBR(const UINT16 opcode)
{
	UINT32 n = Rn;

	m_r[n] -= 4;
	m_ea = m_r[n];
	WL(m_ea, m_vbr );
	m_sh4_icount--;
}

/*  STS     MACH,Rn */
inline void sh34_base_device::STSMACH(const UINT16 opcode)
{
	m_r[Rn] = m_mach;
}

/*  STS     MACL,Rn */
inline void sh34_base_device::STSMACL(const UINT16 opcode)
{
	m_r[Rn] = m_macl;
}

/*  STS     PR,Rn */
inline void sh34_base_device::STSPR(const UINT16 opcode)
{
	m_r[Rn] = m_pr;
}

/*  STS.L   MACH,@-Rn */
inline void sh34_base_device::STSMMACH(const UINT16 opcode)
{
	UINT32 n = Rn;

	m_r[n] -= 4;
	m_ea = m_r[n];
	WL(m_ea, m_mach );
}

/*  STS.L   MACL,@-Rn */
inline void sh34_base_device::STSMMACL(const UINT16 opcode)
{
	UINT32 n = Rn;

	m_r[n] -= 4;
	m_ea = m_r[n];
	WL(m_ea, m_macl );
}

/*  STS.L   PR,@-Rn */
inline void sh34_base_device::STSMPR(const UINT16 opcode)
{
	UINT32 n = Rn;

	m_r[n] -= 4;
	m_ea = m_r[n];
	WL(m_ea, m_pr );
}

/*  SUB     Rm,Rn */
inline void sh34_base_device::SUB(const UINT16 opcode)
{
	m_r[Rn] -= m_r[Rm];
}

/*  SUBC    Rm,Rn */
inline void sh34_base_device::SUBC(const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;

	UINT32 tmp0, tmp1;

	tmp1 = m_r[n] - m_r[m];
	tmp0 = m_r[n];
	m_r[n] = tmp1 - (m_sr & T);
	if (tmp0 < tmp1)
		m_sr |= T;
	else
		m_sr &= ~T;
	if (tmp1 < m_r[n])
		m_sr |= T;
}

/*  SUBV    Rm,Rn */
inline void sh34_base_device::SUBV(const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;

	INT32 dest, src, ans;

	if ((INT32) m_r[n] >= 0)
		dest = 0;
	else
		dest = 1;
	if ((INT32) m_r[m] >= 0)
		src = 0;
	else
		src = 1;
	src += dest;
	m_r[n] -= m_r[m];
	if ((INT32) m_r[n] >= 0)
		ans = 0;
	else
		ans = 1;
	ans += dest;
	if (src == 1)
	{
		if (ans == 1)
			m_sr |= T;
		else
			m_sr &= ~T;
	}
	else
		m_sr &= ~T;
}

/*  SWAP.B  Rm,Rn */
inline void sh34_base_device::SWAPB(const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;

	UINT32 temp0, temp1;

	temp0 = m_r[m] & 0xffff0000;
	temp1 = (m_r[m] & 0x000000ff) << 8;
	m_r[n] = (m_r[m] >> 8) & 0x000000ff;
	m_r[n] = m_r[n] | temp1 | temp0;
}

/*  SWAP.W  Rm,Rn */
inline void sh34_base_device::SWAPW(const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;

	UINT32 temp;

	temp = (m_r[m] >> 16) & 0x0000ffff;
	m_r[n] = (m_r[m] << 16) | temp;
}

/*  TAS.B   @Rn */
inline void sh34_base_device::TAS(const UINT16 opcode)
{
	UINT32 n = Rn;

	UINT32 temp;
	m_ea = m_r[n];
	/* Bus Lock enable */
	temp = RB( m_ea );
	if (temp == 0)
		m_sr |= T;
	else
		m_sr &= ~T;
	temp |= 0x80;
	/* Bus Lock disable */
	WB(m_ea, temp );
	m_sh4_icount -= 3;
}

/*  TRAPA   #imm */
inline void sh34_base_device::TRAPA(const UINT16 opcode)
{
	UINT32 imm = opcode & 0xff;

	if (m_cpu_type == CPU_TYPE_SH4)
	{
		m_m[TRA] = imm << 2;
	}
	else /* SH3 */
	{
		m_sh3internal_upper[SH3_TRA_ADDR] = imm << 2;
	}


	m_ssr = m_sr;
	m_spc = m_pc;
	m_sgr = m_r[15];

	m_sr |= MD;
	if ((machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
		sh4_syncronize_register_bank((m_sr & sRB) >> 29);
	if (!(m_sr & sRB))
		sh4_change_register_bank(1);
	m_sr |= sRB;
	m_sr |= BL;
	sh4_exception_recompute();

	if (m_cpu_type == CPU_TYPE_SH4)
	{
		m_m[EXPEVT] = 0x00000160;
	}
	else /* SH3 */
	{
		m_sh3internal_upper[SH3_EXPEVT_ADDR] = 0x00000160;
	}

	m_pc = m_vbr + 0x00000100;

	m_sh4_icount -= 7;
}

/*  TST     Rm,Rn */
inline void sh34_base_device::TST(const UINT16 opcode)
{
	if ((m_r[Rn] & m_r[Rm]) == 0)
		m_sr |= T;
	else
		m_sr &= ~T;
}

/*  TST     #imm,R0 */
inline void sh34_base_device::TSTI(const UINT16 opcode)
{
	UINT32 imm = opcode & 0xff;

	if ((imm & m_r[0]) == 0)
		m_sr |= T;
	else
		m_sr &= ~T;
}

/*  TST.B   #imm,@(R0,GBR) */
inline void sh34_base_device::TSTM(const UINT16 opcode)
{
	UINT32 imm = opcode & 0xff;

	m_ea = m_gbr + m_r[0];
	if ((imm & RB( m_ea )) == 0)
		m_sr |= T;
	else
		m_sr &= ~T;
	m_sh4_icount -= 2;
}

/*  XOR     Rm,Rn */
inline void sh34_base_device::XOR(const UINT16 opcode)
{
	m_r[Rn] ^= m_r[Rm];
}

/*  XOR     #imm,R0 */
inline void sh34_base_device::XORI(const UINT16 opcode)
{
	UINT32 imm = opcode & 0xff;
	m_r[0] ^= imm;
}

/*  XOR.B   #imm,@(R0,GBR) */
inline void sh34_base_device::XORM(const UINT16 opcode)
{
	UINT32 imm = opcode & 0xff;
	UINT32 temp;

	m_ea = m_gbr + m_r[0];
	temp = RB( m_ea );
	temp ^= imm;
	WB(m_ea, temp );
	m_sh4_icount -= 2;
}

/*  XTRCT   Rm,Rn */
inline void sh34_base_device::XTRCT(const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;

	UINT32 temp;

	temp = (m_r[m] << 16) & 0xffff0000;
	m_r[n] = (m_r[n] >> 16) & 0x0000ffff;
	m_r[n] |= temp;
}

/*  STC     SSR,Rn */
inline void sh34_base_device::STCSSR(const UINT16 opcode)
{
	m_r[Rn] = m_ssr;
}

/*  STC     SPC,Rn */
inline void sh34_base_device::STCSPC(const UINT16 opcode)
{
	m_r[Rn] = m_spc;
}

/*  STC     SGR,Rn */
inline void sh34_base_device::STCSGR(const UINT16 opcode)
{
	m_r[Rn] = m_sgr;
}

/*  STS     FPUL,Rn */
inline void sh34_base_device::STSFPUL(const UINT16 opcode)
{
	m_r[Rn] = m_fpul;
}

/*  STS     FPSCR,Rn */
inline void sh34_base_device::STSFPSCR(const UINT16 opcode)
{
	m_r[Rn] = m_fpscr & 0x003FFFFF;
}

/*  STC     DBR,Rn */
inline void sh34_base_device::STCDBR(const UINT16 opcode)
{
	m_r[Rn] = m_dbr;
}

/*  STCRBANK   Rm_BANK,Rn */
inline void sh34_base_device::STCRBANK(const UINT16 opcode)
{
	UINT32 m = Rm;

	m_r[Rn] = m_rbnk[m_sr&sRB ? 0 : 1][m & 7];
}

/*  STCMRBANK   Rm_BANK,@-Rn */
inline void sh34_base_device::STCMRBANK(const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;

	m_r[n] -= 4;
	m_ea = m_r[n];
	WL(m_ea, m_rbnk[m_sr&sRB ? 0 : 1][m & 7]);
	m_sh4_icount--;
}

/*  MOVCA.L     R0,@Rn */
inline void sh34_base_device::MOVCAL(const UINT16 opcode)
{
	m_ea = m_r[Rn];
	WL(m_ea, m_r[0] );
}

inline void sh34_base_device::CLRS(const UINT16 opcode)
{
	m_sr &= ~S;
}

inline void sh34_base_device::SETS(const UINT16 opcode)
{
	m_sr |= S;
}

/*  STS.L   SGR,@-Rn */
inline void sh34_base_device::STCMSGR(const UINT16 opcode)
{
	UINT32 n = Rn;

	m_r[n] -= 4;
	m_ea = m_r[n];
	WL(m_ea, m_sgr );
}

/*  STS.L   FPUL,@-Rn */
inline void sh34_base_device::STSMFPUL(const UINT16 opcode)
{
	UINT32 n = Rn;

	m_r[n] -= 4;
	m_ea = m_r[n];
	WL(m_ea, m_fpul );
}

/*  STS.L   FPSCR,@-Rn */
inline void sh34_base_device::STSMFPSCR(const UINT16 opcode)
{
	UINT32 n = Rn;

	m_r[n] -= 4;
	m_ea = m_r[n];
	WL(m_ea, m_fpscr & 0x003FFFFF);
}

/*  STC.L   DBR,@-Rn */
inline void sh34_base_device::STCMDBR(const UINT16 opcode)
{
	UINT32 n = Rn;

	m_r[n] -= 4;
	m_ea = m_r[n];
	WL(m_ea, m_dbr );
}

/*  STC.L   SSR,@-Rn */
inline void sh34_base_device::STCMSSR(const UINT16 opcode)
{
	UINT32 n = Rn;

	m_r[n] -= 4;
	m_ea = m_r[n];
	WL(m_ea, m_ssr );
}

/*  STC.L   SPC,@-Rn */
inline void sh34_base_device::STCMSPC(const UINT16 opcode)
{
	UINT32 n = Rn;

	m_r[n] -= 4;
	m_ea = m_r[n];
	WL(m_ea, m_spc );
}

/*  LDS.L   @Rm+,FPUL */
inline void sh34_base_device::LDSMFPUL(const UINT16 opcode)
{
	m_ea = m_r[Rn];
	m_fpul = RL(m_ea );
	m_r[Rn] += 4;
}

/*  LDS.L   @Rm+,FPSCR */
inline void sh34_base_device::LDSMFPSCR(const UINT16 opcode)
{
	UINT32 s;

	s = m_fpscr;
	m_ea = m_r[Rn];
	m_fpscr = RL(m_ea );
	m_fpscr &= 0x003FFFFF;
	m_r[Rn] += 4;
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
inline void sh34_base_device::LDCMDBR(const UINT16 opcode)
{
	m_ea = m_r[Rn];
	m_dbr = RL(m_ea );
	m_r[Rn] += 4;
}

/*  LDC.L   @Rn+,Rm_BANK */
inline void sh34_base_device::LDCMRBANK(const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;

	m_ea = m_r[n];
	m_rbnk[m_sr&sRB ? 0 : 1][m & 7] = RL(m_ea );
	m_r[n] += 4;
}

/*  LDC.L   @Rm+,SSR */
inline void sh34_base_device::LDCMSSR(const UINT16 opcode)
{
	m_ea = m_r[Rn];
	m_ssr = RL(m_ea );
	m_r[Rn] += 4;
}

/*  LDC.L   @Rm+,SPC */
inline void sh34_base_device::LDCMSPC(const UINT16 opcode)
{
	m_ea = m_r[Rn];
	m_spc = RL(m_ea );
	m_r[Rn] += 4;
}

/*  LDS     Rm,FPUL */
inline void sh34_base_device::LDSFPUL(const UINT16 opcode)
{
	m_fpul = m_r[Rn];
}

/*  LDS     Rm,FPSCR */
inline void sh34_base_device::LDSFPSCR(const UINT16 opcode)
{
	UINT32 s;

	s = m_fpscr;
	m_fpscr = m_r[Rn] & 0x003FFFFF;
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
inline void sh34_base_device::LDCDBR(const UINT16 opcode)
{
	m_dbr = m_r[Rn];
}

/*  SHAD    Rm,Rn */
inline void sh34_base_device::SHAD(const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;

	if ((m_r[m] & 0x80000000) == 0)
		m_r[n] = m_r[n] << (m_r[m] & 0x1F);
	else if ((m_r[m] & 0x1F) == 0) {
		if ((m_r[n] & 0x80000000) == 0)
			m_r[n] = 0;
		else
			m_r[n] = 0xFFFFFFFF;
	} else
		m_r[n]=(INT32)m_r[n] >> ((~m_r[m] & 0x1F)+1);
}

/*  SHLD    Rm,Rn */
inline void sh34_base_device::SHLD(const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;

	if ((m_r[m] & 0x80000000) == 0)
		m_r[n] = m_r[n] << (m_r[m] & 0x1F);
	else if ((m_r[m] & 0x1F) == 0)
		m_r[n] = 0;
	else
		m_r[n] = m_r[n] >> ((~m_r[m] & 0x1F)+1);
}

/*  LDCRBANK   Rn,Rm_BANK */
inline void sh34_base_device::LDCRBANK(const UINT16 opcode)
{
	UINT32 m = Rm;

	m_rbnk[m_sr&sRB ? 0 : 1][m & 7] = m_r[Rn];
}

/*  LDC     Rm,SSR */
inline void sh34_base_device::LDCSSR(const UINT16 opcode)
{
	m_ssr = m_r[Rn];
}

/*  LDC     Rm,SPC */
inline void sh34_base_device::LDCSPC(const UINT16 opcode)
{
	m_spc = m_r[Rn];
}

/*  PREF     @Rn */
inline void sh34_base_device::PREFM(const UINT16 opcode)
{
	int a;
	UINT32 addr,dest,sq;

	addr = m_r[Rn]; // address
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

















/*  FMOV.S  @Rm+,FRn PR=0 SZ=0 1111nnnnmmmm1001 */
/*  FMOV    @Rm+,DRn PR=0 SZ=1 1111nnn0mmmm1001 */
/*  FMOV    @Rm+,XDn PR=0 SZ=1 1111nnn1mmmm1001 */
/*  FMOV    @Rm+,XDn PR=1      1111nnn1mmmm1001 */
inline void sh34_base_device::FMOVMRIFR(const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;

	if (m_fpu_pr) { /* PR = 1 */
		n = n & 14;
		m_ea = m_r[m];
		m_r[m] += 8;
		m_xf[n+NATIVE_ENDIAN_VALUE_LE_BE(1,0)] = RL(m_ea );
		m_xf[n+NATIVE_ENDIAN_VALUE_LE_BE(0,1)] = RL(m_ea+4 );
	} else {              /* PR = 0 */
		if (m_fpu_sz) { /* SZ = 1 */
			if (n & 1) {
				n = n & 14;
				m_ea = m_r[m];
				m_xf[n] = RL(m_ea );
				m_r[m] += 4;
				m_xf[n+1] = RL(m_ea+4 );
				m_r[m] += 4;
			} else {
				m_ea = m_r[m];
				m_fr[n] = RL(m_ea );
				m_r[m] += 4;
				m_fr[n+1] = RL(m_ea+4 );
				m_r[m] += 4;
			}
		} else {              /* SZ = 0 */
			m_ea = m_r[m];
			m_fr[n] = RL(m_ea );
			m_r[m] += 4;
		}
	}
}

/*  FMOV.S  FRm,@Rn PR=0 SZ=0 1111nnnnmmmm1010 */
/*  FMOV    DRm,@Rn PR=0 SZ=1 1111nnnnmmm01010 */
/*  FMOV    XDm,@Rn PR=0 SZ=1 1111nnnnmmm11010 */
/*  FMOV    XDm,@Rn PR=1      1111nnnnmmm11010 */
inline void sh34_base_device::FMOVFRMR(const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;

	if (m_fpu_pr) { /* PR = 1 */
		m= m & 14;
		m_ea = m_r[n];
		WL(m_ea,m_xf[m+NATIVE_ENDIAN_VALUE_LE_BE(1,0)] );
		WL(m_ea+4,m_xf[m+NATIVE_ENDIAN_VALUE_LE_BE(0,1)] );
	} else {              /* PR = 0 */
		if (m_fpu_sz) { /* SZ = 1 */
			if (m & 1) {
				m= m & 14;
				m_ea = m_r[n];
				WL(m_ea,m_xf[m] );
				WL(m_ea+4,m_xf[m+1] );
			} else {
				m_ea = m_r[n];
				WL(m_ea,m_fr[m] );
				WL(m_ea+4,m_fr[m+1] );
			}
		} else {              /* SZ = 0 */
			m_ea = m_r[n];
			WL(m_ea,m_fr[m] );
		}
	}
}

/*  FMOV.S  FRm,@-Rn PR=0 SZ=0 1111nnnnmmmm1011 */
/*  FMOV    DRm,@-Rn PR=0 SZ=1 1111nnnnmmm01011 */
/*  FMOV    XDm,@-Rn PR=0 SZ=1 1111nnnnmmm11011 */
/*  FMOV    XDm,@-Rn PR=1      1111nnnnmmm11011 */
inline void sh34_base_device::FMOVFRMDR(const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;

	if (m_fpu_pr) { /* PR = 1 */
		m= m & 14;
		m_r[n] -= 8;
		m_ea = m_r[n];
		WL(m_ea,m_xf[m+NATIVE_ENDIAN_VALUE_LE_BE(1,0)] );
		WL(m_ea+4,m_xf[m+NATIVE_ENDIAN_VALUE_LE_BE(0,1)] );
	} else {              /* PR = 0 */
		if (m_fpu_sz) { /* SZ = 1 */
			if (m & 1) {
				m= m & 14;
				m_r[n] -= 8;
				m_ea = m_r[n];
				WL(m_ea,m_xf[m] );
				WL(m_ea+4,m_xf[m+1] );
			} else {
				m_r[n] -= 8;
				m_ea = m_r[n];
				WL(m_ea,m_fr[m] );
				WL(m_ea+4,m_fr[m+1] );
			}
		} else {              /* SZ = 0 */
			m_r[n] -= 4;
			m_ea = m_r[n];
			WL(m_ea,m_fr[m] );
		}
	}
}

/*  FMOV.S  FRm,@(R0,Rn) PR=0 SZ=0 1111nnnnmmmm0111 */
/*  FMOV    DRm,@(R0,Rn) PR=0 SZ=1 1111nnnnmmm00111 */
/*  FMOV    XDm,@(R0,Rn) PR=0 SZ=1 1111nnnnmmm10111 */
/*  FMOV    XDm,@(R0,Rn) PR=1      1111nnnnmmm10111 */
inline void sh34_base_device::FMOVFRS0(const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;

	if (m_fpu_pr) { /* PR = 1 */
		m= m & 14;
		m_ea = m_r[0] + m_r[n];
		WL(m_ea,m_xf[m+NATIVE_ENDIAN_VALUE_LE_BE(1,0)] );
		WL(m_ea+4,m_xf[m+NATIVE_ENDIAN_VALUE_LE_BE(0,1)] );
	} else {              /* PR = 0 */
		if (m_fpu_sz) { /* SZ = 1 */
			if (m & 1) {
				m= m & 14;
				m_ea = m_r[0] + m_r[n];
				WL(m_ea,m_xf[m] );
				WL(m_ea+4,m_xf[m+1] );
			} else {
				m_ea = m_r[0] + m_r[n];
				WL(m_ea,m_fr[m] );
				WL(m_ea+4,m_fr[m+1] );
			}
		} else {              /* SZ = 0 */
			m_ea = m_r[0] + m_r[n];
			WL(m_ea,m_fr[m] );
		}
	}
}

/*  FMOV.S  @(R0,Rm),FRn PR=0 SZ=0 1111nnnnmmmm0110 */
/*  FMOV    @(R0,Rm),DRn PR=0 SZ=1 1111nnn0mmmm0110 */
/*  FMOV    @(R0,Rm),XDn PR=0 SZ=1 1111nnn1mmmm0110 */
/*  FMOV    @(R0,Rm),XDn PR=1      1111nnn1mmmm0110 */
inline void sh34_base_device::FMOVS0FR(const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;

	if (m_fpu_pr) { /* PR = 1 */
		n= n & 14;
		m_ea = m_r[0] + m_r[m];
		m_xf[n+NATIVE_ENDIAN_VALUE_LE_BE(1,0)] = RL(m_ea );
		m_xf[n+NATIVE_ENDIAN_VALUE_LE_BE(0,1)] = RL(m_ea+4 );
	} else {              /* PR = 0 */
		if (m_fpu_sz) { /* SZ = 1 */
			if (n & 1) {
				n= n & 14;
				m_ea = m_r[0] + m_r[m];
				m_xf[n] = RL(m_ea );
				m_xf[n+1] = RL(m_ea+4 );
			} else {
				m_ea = m_r[0] + m_r[m];
				m_fr[n] = RL(m_ea );
				m_fr[n+1] = RL(m_ea+4 );
			}
		} else {              /* SZ = 0 */
			m_ea = m_r[0] + m_r[m];
			m_fr[n] = RL(m_ea );
		}
	}
}

/*  FMOV.S  @Rm,FRn PR=0 SZ=0 1111nnnnmmmm1000 */
/*  FMOV    @Rm,DRn PR=0 SZ=1 1111nnn0mmmm1000 */
/*  FMOV    @Rm,XDn PR=0 SZ=1 1111nnn1mmmm1000 */
/*  FMOV    @Rm,XDn PR=1      1111nnn1mmmm1000 */
/*  FMOV    @Rm,DRn PR=1      1111nnn0mmmm1000 */
inline void sh34_base_device::FMOVMRFR(const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;

	if (m_fpu_pr) { /* PR = 1 */
		if (n & 1) {
			n= n & 14;
			m_ea = m_r[m];
			m_xf[n+NATIVE_ENDIAN_VALUE_LE_BE(1,0)] = RL(m_ea );
			m_xf[n+NATIVE_ENDIAN_VALUE_LE_BE(0,1)] = RL(m_ea+4 );
		} else {
			n= n & 14;
			m_ea = m_r[m];
			m_fr[n+NATIVE_ENDIAN_VALUE_LE_BE(1,0)] = RL(m_ea );
			m_fr[n+NATIVE_ENDIAN_VALUE_LE_BE(0,1)] = RL(m_ea+4 );
		}
	} else {              /* PR = 0 */
		if (m_fpu_sz) { /* SZ = 1 */
			if (n & 1) {
				n= n & 14;
				m_ea = m_r[m];
				m_xf[n] = RL(m_ea );
				m_xf[n+1] = RL(m_ea+4 );
			} else {
				n= n & 14;
				m_ea = m_r[m];
				m_fr[n] = RL(m_ea );
				m_fr[n+1] = RL(m_ea+4 );
			}
		} else {              /* SZ = 0 */
			m_ea = m_r[m];
			m_fr[n] = RL(m_ea );
		}
	}
}

/*  FMOV    FRm,FRn PR=0 SZ=0 FRm -> FRn 1111nnnnmmmm1100 */
/*  FMOV    DRm,DRn PR=0 SZ=1 DRm -> DRn 1111nnn0mmm01100 */
/*  FMOV    XDm,DRn PR=1      XDm -> DRn 1111nnn0mmm11100 */
/*  FMOV    DRm,XDn PR=1      DRm -> XDn 1111nnn1mmm01100 */
/*  FMOV    XDm,XDn PR=1      XDm -> XDn 1111nnn1mmm11100 */
inline void sh34_base_device::FMOVFR(const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;

	if ((m_fpu_sz == 0) && (m_fpu_pr == 0)) /* SZ = 0 */
		m_fr[n] = m_fr[m];
	else { /* SZ = 1 or PR = 1 */
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
inline void sh34_base_device::FLDI1(const UINT16 opcode)
{
	m_fr[Rn] = 0x3F800000;
}

/*  FLDI0  FRn 1111nnnn10001101 */
inline void sh34_base_device::FLDI0(const UINT16 opcode)
{
	m_fr[Rn] = 0;
}

/*  FLDS FRm,FPUL 1111mmmm00011101 */
inline void sh34_base_device:: FLDS(const UINT16 opcode)
{
	m_fpul = m_fr[Rn];
}

/*  FSTS FPUL,FRn 1111nnnn00001101 */
inline void sh34_base_device:: FSTS(const UINT16 opcode)
{
	m_fr[Rn] = m_fpul;
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
inline void sh34_base_device::FTRC(const UINT16 opcode)
{
	UINT32 n = Rn;

	if (m_fpu_pr) { /* PR = 1 */
		if(n & 1)
			fatalerror("SH-4: FTRC opcode used with n %d",n);

		n = n & 14;
		*((INT32 *)&m_fpul) = (INT32)FP_RFD(n);
	} else {              /* PR = 0 */
		/* read m_fr[n] as float -> truncate -> fpul(32) */
		*((INT32 *)&m_fpul) = (INT32)FP_RFS(n);
	}
}

/* FLOAT FPUL,FRn PR=0 1111nnnn00101101 */
/* FLOAT FPUL,DRn PR=1 1111nnn000101101 */
inline void sh34_base_device::FLOAT(const UINT16 opcode)
{
	UINT32 n = Rn;

	if (m_fpu_pr) { /* PR = 1 */
		if(n & 1)
			fatalerror("SH-4: FLOAT opcode used with n %d",n);

		n = n & 14;
		FP_RFD(n) = (double)*((INT32 *)&m_fpul);
	} else {              /* PR = 0 */
		FP_RFS(n) = (float)*((INT32 *)&m_fpul);
	}
}

/* FNEG FRn PR=0 1111nnnn01001101 */
/* FNEG DRn PR=1 1111nnn001001101 */
inline void sh34_base_device::FNEG(const UINT16 opcode)
{
	UINT32 n = Rn;

	if (m_fpu_pr) { /* PR = 1 */
		FP_RFD(n) = -FP_RFD(n);
	} else {              /* PR = 0 */
		FP_RFS(n) = -FP_RFS(n);
	}
}

/* FABS FRn PR=0 1111nnnn01011101 */
/* FABS DRn PR=1 1111nnn001011101 */
inline void sh34_base_device::FABS(const UINT16 opcode)
{
	UINT32 n = Rn;

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
inline void sh34_base_device::FCMP_EQ(const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;

	if (m_fpu_pr) { /* PR = 1 */
		n = n & 14;
		m = m & 14;
		if (FP_RFD(n) == FP_RFD(m))
			m_sr |= T;
		else
			m_sr &= ~T;
	} else {              /* PR = 0 */
		if (FP_RFS(n) == FP_RFS(m))
			m_sr |= T;
		else
			m_sr &= ~T;
	}
}

/* FCMP/GT FRm,FRn PR=0 1111nnnnmmmm0101 */
/* FCMP/GT DRm,DRn PR=1 1111nnn0mmm00101 */
inline void sh34_base_device::FCMP_GT(const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;

	if (m_fpu_pr) { /* PR = 1 */
		n = n & 14;
		m = m & 14;
		if (FP_RFD(n) > FP_RFD(m))
			m_sr |= T;
		else
			m_sr &= ~T;
	} else {              /* PR = 0 */
		if (FP_RFS(n) > FP_RFS(m))
			m_sr |= T;
		else
			m_sr &= ~T;
	}
}

/* FCNVDS DRm,FPUL PR=1 1111mmm010111101 */
inline void sh34_base_device::FCNVDS(const UINT16 opcode)
{
	UINT32 n = Rn;

	if (m_fpu_pr) { /* PR = 1 */
		n = n & 14;
		if (m_fpscr & RM)
			m_fr[n | NATIVE_ENDIAN_VALUE_LE_BE(0,1)] &= 0xe0000000; /* round toward zero*/
		*((float *)&m_fpul) = (float)FP_RFD(n);
	}
}

/* FCNVSD FPUL, DRn PR=1 1111nnn010101101 */
inline void sh34_base_device::FCNVSD(const UINT16 opcode)
{
	UINT32 n = Rn;

	if (m_fpu_pr) { /* PR = 1 */
		n = n & 14;
		FP_RFD(n) = (double)*((float *)&m_fpul);
	}
}

/* FADD FRm,FRn PR=0 1111nnnnmmmm0000 */
/* FADD DRm,DRn PR=1 1111nnn0mmm00000 */
inline void sh34_base_device::FADD(const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;

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
inline void sh34_base_device::FSUB(const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;

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
inline void sh34_base_device::FMUL(const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;

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
inline void sh34_base_device::FDIV(const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;

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
inline void sh34_base_device::FMAC(const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;

	if (m_fpu_pr == 0) { /* PR = 0 */
		FP_RFS(n) = (FP_RFS(0) * FP_RFS(m)) + FP_RFS(n);
	}
}

/* FSQRT FRn PR=0 1111nnnn01101101 */
/* FSQRT DRn PR=1 1111nnnn01101101 */
inline void sh34_base_device::FSQRT(const UINT16 opcode)
{
	UINT32 n = Rn;

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
inline void sh34_base_device::FSRRA(const UINT16 opcode)
{
	UINT32 n = Rn;

	if (FP_RFS(n) < 0)
		return;
	FP_RFS(n) = 1.0f / sqrtf(FP_RFS(n));
}

/*  FSSCA FPUL,FRn PR=0 1111nnn011111101 */
void sh34_base_device::FSSCA(const UINT16 opcode)
{
	UINT32 n = Rn;

	float angle;

	angle = (((float)(m_fpul & 0xFFFF)) / 65536.0f) * 2.0f * (float) M_PI;
	FP_RFS(n) = sinf(angle);
	FP_RFS(n+1) = cosf(angle);
}

/* FIPR FVm,FVn PR=0 1111nnmm11101101 */
inline void sh34_base_device::FIPR(const UINT16 opcode)
{
	UINT32 n = Rn;

UINT32 m;
float ml[4];
int a;

	m = (n & 3) << 2;
	n = n & 12;
	for (a = 0;a < 4;a++)
		ml[a] = FP_RFS(n+a) * FP_RFS(m+a);
	FP_RFS(n+3) = ml[0] + ml[1] + ml[2] + ml[3];
}

/* FTRV XMTRX,FVn PR=0 1111nn0111111101 */
void sh34_base_device::FTRV(const UINT16 opcode)
{
	UINT32 n = Rn;

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

inline void sh34_base_device::op1111_0xf13(const UINT16 opcode)
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
						debugger_break(machine());
						break;
				}
			} else {
				FTRV(opcode);
			}
		} else {
			FSSCA(opcode);
		}
}

void sh34_base_device::dbreak(const UINT16 opcode)
{
	debugger_break(machine());
}


inline void sh34_base_device::op1111_0x13(UINT16 opcode)
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
	m_ppc = 0;
	m_spc = 0;
	m_pr = 0;
	m_sr = 0;
	m_ssr = 0;
	m_gbr = 0;
	m_vbr = 0;
	m_mach = 0;
	m_macl = 0;
	memset(m_r, 0, sizeof(m_r));
	memset(m_rbnk, 0, sizeof(m_rbnk));
	m_sgr = 0;
	memset(m_fr, 0, sizeof(m_fr));
	memset(m_xf, 0, sizeof(m_xf));
	m_ea = 0;
	m_delay = 0;
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

	m_pc = 0xa0000000;
	m_r[15] = RL(4);
	m_sr = 0x700000f0;
	m_fpscr = 0x00040001;
	m_fpu_sz = (m_fpscr & SZ) ? 1 : 0;
	m_fpu_pr = (m_fpscr & PR) ? 1 : 0;
	m_fpul = 0;
	m_dbr = 0;

	m_internal_irq_level = -1;
	m_irln = 15;
	m_sleep_mode = 0;

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

inline void sh34_base_device::execute_one_0000(const UINT16 opcode)
{
	switch(opcode & 0xff)
	{
		// 0x00
		case 0x00:  NOP(opcode); break;
		case 0x10:  NOP(opcode); break;
		case 0x20:  NOP(opcode); break;
		case 0x30:  NOP(opcode); break;
		case 0x40:  NOP(opcode); break;
		case 0x50:  NOP(opcode); break;
		case 0x60:  NOP(opcode); break;
		case 0x70:  NOP(opcode); break;
		case 0x80:  NOP(opcode); break;
		case 0x90:  NOP(opcode); break;
		case 0xa0:  NOP(opcode); break;
		case 0xb0:  NOP(opcode); break;
		case 0xc0:  NOP(opcode); break;
		case 0xd0:  NOP(opcode); break;
		case 0xe0:  NOP(opcode); break;
		case 0xf0:  NOP(opcode); break;
		// 0x10
		case 0x01:  NOP(opcode); break;
		case 0x11:  NOP(opcode); break;
		case 0x21:  NOP(opcode); break;
		case 0x31:  NOP(opcode); break;
		case 0x41:  NOP(opcode); break;
		case 0x51:  NOP(opcode); break;
		case 0x61:  NOP(opcode); break;
		case 0x71:  NOP(opcode); break;
		case 0x81:  NOP(opcode); break;
		case 0x91:  NOP(opcode); break;
		case 0xa1:  NOP(opcode); break;
		case 0xb1:  NOP(opcode); break;
		case 0xc1:  NOP(opcode); break;
		case 0xd1:  NOP(opcode); break;
		case 0xe1:  NOP(opcode); break;
		case 0xf1:  NOP(opcode); break;
		// 0x20
		case 0x02:  STCSR(opcode); break;
		case 0x12:  STCGBR(opcode); break;
		case 0x22:  STCVBR(opcode); break;
		case 0x32:  STCSSR(opcode); break;
		case 0x42:  STCSPC(opcode); break;
		case 0x52:  NOP(opcode); break;
		case 0x62:  NOP(opcode); break;
		case 0x72:  NOP(opcode); break;
		case 0x82:  STCRBANK(opcode); break;
		case 0x92:  STCRBANK(opcode); break;
		case 0xa2:  STCRBANK(opcode); break;
		case 0xb2:  STCRBANK(opcode); break;
		case 0xc2:  STCRBANK(opcode); break;
		case 0xd2:  STCRBANK(opcode); break;
		case 0xe2:  STCRBANK(opcode); break;
		case 0xf2:  STCRBANK(opcode); break;
		// 0x30
		case 0x03:  BSRF(opcode); break;
		case 0x13:  NOP(opcode); break;
		case 0x23:  BRAF(opcode); break;
		case 0x33:  NOP(opcode); break;
		case 0x43:  NOP(opcode); break;
		case 0x53:  NOP(opcode); break;
		case 0x63:  NOP(opcode); break;
		case 0x73:  NOP(opcode); break;
		case 0x83:  PREFM(opcode); break;
		case 0x93:  TODO(opcode); break;
		case 0xa3:  TODO(opcode); break;
		case 0xb3:  TODO(opcode); break;
		case 0xc3:  MOVCAL(opcode); break;
		case 0xd3:  NOP(opcode); break;
		case 0xe3:  NOP(opcode); break;
		case 0xf3:  NOP(opcode); break;
		// 0x40
		case 0x04:  MOVBS0(opcode); break;
		case 0x14:  MOVBS0(opcode); break;
		case 0x24:  MOVBS0(opcode); break;
		case 0x34:  MOVBS0(opcode); break;
		case 0x44:  MOVBS0(opcode); break;
		case 0x54:  MOVBS0(opcode); break;
		case 0x64:  MOVBS0(opcode); break;
		case 0x74:  MOVBS0(opcode); break;
		case 0x84:  MOVBS0(opcode); break;
		case 0x94:  MOVBS0(opcode); break;
		case 0xa4:  MOVBS0(opcode); break;
		case 0xb4:  MOVBS0(opcode); break;
		case 0xc4:  MOVBS0(opcode); break;
		case 0xd4:  MOVBS0(opcode); break;
		case 0xe4:  MOVBS0(opcode); break;
		case 0xf4:  MOVBS0(opcode); break;
		// 0x50
		case 0x05:  MOVWS0(opcode); break;
		case 0x15:  MOVWS0(opcode); break;
		case 0x25:  MOVWS0(opcode); break;
		case 0x35:  MOVWS0(opcode); break;
		case 0x45:  MOVWS0(opcode); break;
		case 0x55:  MOVWS0(opcode); break;
		case 0x65:  MOVWS0(opcode); break;
		case 0x75:  MOVWS0(opcode); break;
		case 0x85:  MOVWS0(opcode); break;
		case 0x95:  MOVWS0(opcode); break;
		case 0xa5:  MOVWS0(opcode); break;
		case 0xb5:  MOVWS0(opcode); break;
		case 0xc5:  MOVWS0(opcode); break;
		case 0xd5:  MOVWS0(opcode); break;
		case 0xe5:  MOVWS0(opcode); break;
		case 0xf5:  MOVWS0(opcode); break;
		// 0x60
		case 0x06:  MOVLS0(opcode); break;
		case 0x16:  MOVLS0(opcode); break;
		case 0x26:  MOVLS0(opcode); break;
		case 0x36:  MOVLS0(opcode); break;
		case 0x46:  MOVLS0(opcode); break;
		case 0x56:  MOVLS0(opcode); break;
		case 0x66:  MOVLS0(opcode); break;
		case 0x76:  MOVLS0(opcode); break;
		case 0x86:  MOVLS0(opcode); break;
		case 0x96:  MOVLS0(opcode); break;
		case 0xa6:  MOVLS0(opcode); break;
		case 0xb6:  MOVLS0(opcode); break;
		case 0xc6:  MOVLS0(opcode); break;
		case 0xd6:  MOVLS0(opcode); break;
		case 0xe6:  MOVLS0(opcode); break;
		case 0xf6:  MOVLS0(opcode); break;
		// 0x70
		case 0x07:  MULL(opcode); break;
		case 0x17:  MULL(opcode); break;
		case 0x27:  MULL(opcode); break;
		case 0x37:  MULL(opcode); break;
		case 0x47:  MULL(opcode); break;
		case 0x57:  MULL(opcode); break;
		case 0x67:  MULL(opcode); break;
		case 0x77:  MULL(opcode); break;
		case 0x87:  MULL(opcode); break;
		case 0x97:  MULL(opcode); break;
		case 0xa7:  MULL(opcode); break;
		case 0xb7:  MULL(opcode); break;
		case 0xc7:  MULL(opcode); break;
		case 0xd7:  MULL(opcode); break;
		case 0xe7:  MULL(opcode); break;
		case 0xf7:  MULL(opcode); break;
		// 0x80
		case 0x08:  CLRT(opcode); break;
		case 0x18:  SETT(opcode); break;
		case 0x28:  CLRMAC(opcode); break;
		case 0x38:  TODO(opcode); break;
		case 0x48:  CLRS(opcode); break;
		case 0x58:  SETS(opcode); break;
		case 0x68:  NOP(opcode); break;
		case 0x78:  NOP(opcode); break;
		case 0x88:  CLRT(opcode); break;
		case 0x98:  SETT(opcode); break;
		case 0xa8:  CLRMAC(opcode); break;
		case 0xb8:  TODO(opcode); break;
		case 0xc8:  CLRS(opcode); break;
		case 0xd8:  SETS(opcode); break;
		case 0xe8:  NOP(opcode); break;
		case 0xf8:  NOP(opcode); break;
		// 0x90
		case 0x09:  NOP(opcode); break;
		case 0x19:  DIV0U(opcode); break;
		case 0x29:  MOVT(opcode); break;
		case 0x39:  NOP(opcode); break;
		case 0x49:  NOP(opcode); break;
		case 0x59:  DIV0U(opcode); break;
		case 0x69:  MOVT(opcode); break;
		case 0x79:  NOP(opcode); break;
		case 0x89:  NOP(opcode); break;
		case 0x99:  DIV0U(opcode); break;
		case 0xa9:  MOVT(opcode); break;
		case 0xb9:  NOP(opcode); break;
		case 0xc9:  NOP(opcode); break;
		case 0xd9:  DIV0U(opcode); break;
		case 0xe9:  MOVT(opcode); break;
		case 0xf9:  NOP(opcode); break;
		// 0xa0
		case 0x0a:  STSMACH(opcode); break;
		case 0x1a:  STSMACL(opcode); break;
		case 0x2a:  STSPR(opcode); break;
		case 0x3a:  STCSGR(opcode); break;
		case 0x4a:  NOP(opcode); break;
		case 0x5a:  STSFPUL(opcode); break;
		case 0x6a:  STSFPSCR(opcode); break;
		case 0x7a:  STCDBR(opcode); break;
		case 0x8a:  STSMACH(opcode); break;
		case 0x9a:  STSMACL(opcode); break;
		case 0xaa:  STSPR(opcode); break;
		case 0xba:  STCSGR(opcode); break;
		case 0xca:  NOP(opcode); break;
		case 0xda:  STSFPUL(opcode); break;
		case 0xea:  STSFPSCR(opcode); break;
		case 0xfa:  STCDBR(opcode); break;
		// 0xb0
		case 0x0b:  RTS(opcode); break;
		case 0x1b:  SLEEP(opcode); break;
		case 0x2b:  RTE(opcode); break;
		case 0x3b:  NOP(opcode); break;
		case 0x4b:  RTS(opcode); break;
		case 0x5b:  SLEEP(opcode); break;
		case 0x6b:  RTE(opcode); break;
		case 0x7b:  NOP(opcode); break;
		case 0x8b:  RTS(opcode); break;
		case 0x9b:  SLEEP(opcode); break;
		case 0xab:  RTE(opcode); break;
		case 0xbb:  NOP(opcode); break;
		case 0xcb:  RTS(opcode); break;
		case 0xdb:  SLEEP(opcode); break;
		case 0xeb:  RTE(opcode); break;
		case 0xfb:  NOP(opcode); break;
		// 0xc0
		case 0x0c:  MOVBL0(opcode); break;
		case 0x1c:  MOVBL0(opcode); break;
		case 0x2c:  MOVBL0(opcode); break;
		case 0x3c:  MOVBL0(opcode); break;
		case 0x4c:  MOVBL0(opcode); break;
		case 0x5c:  MOVBL0(opcode); break;
		case 0x6c:  MOVBL0(opcode); break;
		case 0x7c:  MOVBL0(opcode); break;
		case 0x8c:  MOVBL0(opcode); break;
		case 0x9c:  MOVBL0(opcode); break;
		case 0xac:  MOVBL0(opcode); break;
		case 0xbc:  MOVBL0(opcode); break;
		case 0xcc:  MOVBL0(opcode); break;
		case 0xdc:  MOVBL0(opcode); break;
		case 0xec:  MOVBL0(opcode); break;
		case 0xfc:  MOVBL0(opcode); break;
		// 0xd0
		case 0x0d:  MOVWL0(opcode); break;
		case 0x1d:  MOVWL0(opcode); break;
		case 0x2d:  MOVWL0(opcode); break;
		case 0x3d:  MOVWL0(opcode); break;
		case 0x4d:  MOVWL0(opcode); break;
		case 0x5d:  MOVWL0(opcode); break;
		case 0x6d:  MOVWL0(opcode); break;
		case 0x7d:  MOVWL0(opcode); break;
		case 0x8d:  MOVWL0(opcode); break;
		case 0x9d:  MOVWL0(opcode); break;
		case 0xad:  MOVWL0(opcode); break;
		case 0xbd:  MOVWL0(opcode); break;
		case 0xcd:  MOVWL0(opcode); break;
		case 0xdd:  MOVWL0(opcode); break;
		case 0xed:  MOVWL0(opcode); break;
		case 0xfd:  MOVWL0(opcode); break;
		// 0xe0
		case 0x0e:  MOVLL0(opcode); break;
		case 0x1e:  MOVLL0(opcode); break;
		case 0x2e:  MOVLL0(opcode); break;
		case 0x3e:  MOVLL0(opcode); break;
		case 0x4e:  MOVLL0(opcode); break;
		case 0x5e:  MOVLL0(opcode); break;
		case 0x6e:  MOVLL0(opcode); break;
		case 0x7e:  MOVLL0(opcode); break;
		case 0x8e:  MOVLL0(opcode); break;
		case 0x9e:  MOVLL0(opcode); break;
		case 0xae:  MOVLL0(opcode); break;
		case 0xbe:  MOVLL0(opcode); break;
		case 0xce:  MOVLL0(opcode); break;
		case 0xde:  MOVLL0(opcode); break;
		case 0xee:  MOVLL0(opcode); break;
		case 0xfe:  MOVLL0(opcode); break;
		// 0xf0
		case 0x0f:  MAC_L(opcode); break;
		case 0x1f:  MAC_L(opcode); break;
		case 0x2f:  MAC_L(opcode); break;
		case 0x3f:  MAC_L(opcode); break;
		case 0x4f:  MAC_L(opcode); break;
		case 0x5f:  MAC_L(opcode); break;
		case 0x6f:  MAC_L(opcode); break;
		case 0x7f:  MAC_L(opcode); break;
		case 0x8f:  MAC_L(opcode); break;
		case 0x9f:  MAC_L(opcode); break;
		case 0xaf:  MAC_L(opcode); break;
		case 0xbf:  MAC_L(opcode); break;
		case 0xcf:  MAC_L(opcode); break;
		case 0xdf:  MAC_L(opcode); break;
		case 0xef:  MAC_L(opcode); break;
		case 0xff:  MAC_L(opcode); break;
	}
};

inline void sh34_base_device::execute_one_4000(const UINT16 opcode)
{
	switch(opcode & 0xff)
	{
		// 0x00
		case 0x00:  SHLL(opcode); break;
		case 0x10:  DT(opcode); break;
		case 0x20:  SHAL(opcode); break;
		case 0x30:  NOP(opcode); break;
		case 0x40:  SHLL(opcode); break;
		case 0x50:  DT(opcode); break;
		case 0x60:  SHAL(opcode); break;
		case 0x70:  NOP(opcode); break;
		case 0x80:  SHLL(opcode); break;
		case 0x90:  DT(opcode); break;
		case 0xa0:  SHAL(opcode); break;
		case 0xb0:  NOP(opcode); break;
		case 0xc0:  SHLL(opcode); break;
		case 0xd0:  DT(opcode); break;
		case 0xe0:  SHAL(opcode); break;
		case 0xf0:  NOP(opcode); break;
		// 0x10
		case 0x01:  SHLR(opcode); break;
		case 0x11:  CMPPZ(opcode); break;
		case 0x21:  SHAR(opcode); break;
		case 0x31:  NOP(opcode); break;
		case 0x41:  SHLR(opcode); break;
		case 0x51:  CMPPZ(opcode); break;
		case 0x61:  SHAR(opcode); break;
		case 0x71:  NOP(opcode); break;
		case 0x81:  SHLR(opcode); break;
		case 0x91:  CMPPZ(opcode); break;
		case 0xa1:  SHAR(opcode); break;
		case 0xb1:  NOP(opcode); break;
		case 0xc1:  SHLR(opcode); break;
		case 0xd1:  CMPPZ(opcode); break;
		case 0xe1:  SHAR(opcode); break;
		case 0xf1:  NOP(opcode); break;
		// 0x20
		case 0x02:  STSMMACH(opcode); break;
		case 0x12:  STSMMACL(opcode); break;
		case 0x22:  STSMPR(opcode); break;
		case 0x32:  STCMSGR(opcode); break;
		case 0x42:  NOP(opcode); break;
		case 0x52:  STSMFPUL(opcode); break;
		case 0x62:  STSMFPSCR(opcode); break;
		case 0x72:  NOP(opcode); break;
		case 0x82:  NOP(opcode); break;
		case 0x92:  NOP(opcode); break;
		case 0xa2:  NOP(opcode); break;
		case 0xb2:  NOP(opcode); break;
		case 0xc2:  NOP(opcode); break;
		case 0xd2:  NOP(opcode); break;
		case 0xe2:  NOP(opcode); break;
		case 0xf2:  STCMDBR(opcode); break;
		// 0x30
		case 0x03:  STCMSR(opcode); break;
		case 0x13:  STCMGBR(opcode); break;
		case 0x23:  STCMVBR(opcode); break;
		case 0x33:  STCMSSR(opcode); break;
		case 0x43:  STCMSPC(opcode); break;
		case 0x53:  NOP(opcode); break;
		case 0x63:  NOP(opcode); break;
		case 0x73:  NOP(opcode); break;
		case 0x83:  STCMRBANK(opcode); break;
		case 0x93:  STCMRBANK(opcode); break;
		case 0xa3:  STCMRBANK(opcode); break;
		case 0xb3:  STCMRBANK(opcode); break;
		case 0xc3:  STCMRBANK(opcode); break;
		case 0xd3:  STCMRBANK(opcode); break;
		case 0xe3:  STCMRBANK(opcode); break;
		case 0xf3:  STCMRBANK(opcode); break;
		// 0x40
		case 0x04:  ROTL(opcode); break;
		case 0x14:  NOP(opcode); break;
		case 0x24:  ROTCL(opcode); break;
		case 0x34:  NOP(opcode); break;
		case 0x44:  ROTL(opcode); break;
		case 0x54:  NOP(opcode); break;
		case 0x64:  ROTCL(opcode); break;
		case 0x74:  NOP(opcode); break;
		case 0x84:  ROTL(opcode); break;
		case 0x94:  NOP(opcode); break;
		case 0xa4:  ROTCL(opcode); break;
		case 0xb4:  NOP(opcode); break;
		case 0xc4:  ROTL(opcode); break;
		case 0xd4:  NOP(opcode); break;
		case 0xe4:  ROTCL(opcode); break;
		case 0xf4:  NOP(opcode); break;
		// 0x50
		case 0x05:  ROTR(opcode); break;
		case 0x15:  CMPPL(opcode); break;
		case 0x25:  ROTCR(opcode); break;
		case 0x35:  NOP(opcode); break;
		case 0x45:  ROTR(opcode); break;
		case 0x55:  CMPPL(opcode); break;
		case 0x65:  ROTCR(opcode); break;
		case 0x75:  NOP(opcode); break;
		case 0x85:  ROTR(opcode); break;
		case 0x95:  CMPPL(opcode); break;
		case 0xa5:  ROTCR(opcode); break;
		case 0xb5:  NOP(opcode); break;
		case 0xc5:  ROTR(opcode); break;
		case 0xd5:  CMPPL(opcode); break;
		case 0xe5:  ROTCR(opcode); break;
		case 0xf5:  NOP(opcode); break;
		// 0x60
		case 0x06:  LDSMMACH(opcode); break;
		case 0x16:  LDSMMACL(opcode); break;
		case 0x26:  LDSMPR(opcode); break;
		case 0x36:  NOP(opcode); break;
		case 0x46:  NOP(opcode); break;
		case 0x56:  LDSMFPUL(opcode); break;
		case 0x66:  LDSMFPSCR(opcode); break;
		case 0x76:  NOP(opcode); break;
		case 0x86:  NOP(opcode); break;
		case 0x96:  NOP(opcode); break;
		case 0xa6:  NOP(opcode); break;
		case 0xb6:  NOP(opcode); break;
		case 0xc6:  NOP(opcode); break;
		case 0xd6:  NOP(opcode); break;
		case 0xe6:  NOP(opcode); break;
		case 0xf6:  LDCMDBR(opcode); break;
		// 0x70
		case 0x07:  LDCMSR(opcode); break;
		case 0x17:  LDCMGBR(opcode); break;
		case 0x27:  LDCMVBR(opcode); break;
		case 0x37:  LDCMSSR(opcode); break;
		case 0x47:  LDCMSPC(opcode); break;
		case 0x57:  NOP(opcode); break;
		case 0x67:  NOP(opcode); break;
		case 0x77:  NOP(opcode); break;
		case 0x87:  LDCMRBANK(opcode); break;
		case 0x97:  LDCMRBANK(opcode); break;
		case 0xa7:  LDCMRBANK(opcode); break;
		case 0xb7:  LDCMRBANK(opcode); break;
		case 0xc7:  LDCMRBANK(opcode); break;
		case 0xd7:  LDCMRBANK(opcode); break;
		case 0xe7:  LDCMRBANK(opcode); break;
		case 0xf7:  LDCMRBANK(opcode); break;
		// 0x80
		case 0x08:  SHLL2(opcode); break;
		case 0x18:  SHLL8(opcode); break;
		case 0x28:  SHLL16(opcode); break;
		case 0x38:  NOP(opcode); break;
		case 0x48:  SHLL2(opcode); break;
		case 0x58:  SHLL8(opcode); break;
		case 0x68:  SHLL16(opcode); break;
		case 0x78:  NOP(opcode); break;
		case 0x88:  SHLL2(opcode); break;
		case 0x98:  SHLL8(opcode); break;
		case 0xa8:  SHLL16(opcode); break;
		case 0xb8:  NOP(opcode); break;
		case 0xc8:  SHLL2(opcode); break;
		case 0xd8:  SHLL8(opcode); break;
		case 0xe8:  SHLL16(opcode); break;
		case 0xf8:  NOP(opcode); break;
		// 0x90
		case 0x09:  SHLR2(opcode); break;
		case 0x19:  SHLR8(opcode); break;
		case 0x29:  SHLR16(opcode); break;
		case 0x39:  NOP(opcode); break;
		case 0x49:  SHLR2(opcode); break;
		case 0x59:  SHLR8(opcode); break;
		case 0x69:  SHLR16(opcode); break;
		case 0x79:  NOP(opcode); break;
		case 0x89:  SHLR2(opcode); break;
		case 0x99:  SHLR8(opcode); break;
		case 0xa9:  SHLR16(opcode); break;
		case 0xb9:  NOP(opcode); break;
		case 0xc9:  SHLR2(opcode); break;
		case 0xd9:  SHLR8(opcode); break;
		case 0xe9:  SHLR16(opcode); break;
		case 0xf9:  NOP(opcode); break;
		// 0xa0
		case 0x0a:  LDSMACH(opcode); break;
		case 0x1a:  LDSMACL(opcode); break;
		case 0x2a:  LDSPR(opcode); break;
		case 0x3a:  NOP(opcode); break;
		case 0x4a:  NOP(opcode); break;
		case 0x5a:  LDSFPUL(opcode); break;
		case 0x6a:  LDSFPSCR(opcode); break;
		case 0x7a:  NOP(opcode); break;
		case 0x8a:  NOP(opcode); break;
		case 0x9a:  NOP(opcode); break;
		case 0xaa:  NOP(opcode); break;
		case 0xba:  NOP(opcode); break;
		case 0xca:  NOP(opcode); break;
		case 0xda:  NOP(opcode); break;
		case 0xea:  NOP(opcode); break;
		case 0xfa:  LDCDBR(opcode); break;
		// 0xb0
		case 0x0b:  JSR(opcode); break;
		case 0x1b:  TAS(opcode); break;
		case 0x2b:  JMP(opcode); break;
		case 0x3b:  NOP(opcode); break;
		case 0x4b:  JSR(opcode); break;
		case 0x5b:  TAS(opcode); break;
		case 0x6b:  JMP(opcode); break;
		case 0x7b:  NOP(opcode); break;
		case 0x8b:  JSR(opcode); break;
		case 0x9b:  TAS(opcode); break;
		case 0xab:  JMP(opcode); break;
		case 0xbb:  NOP(opcode); break;
		case 0xcb:  JSR(opcode); break;
		case 0xdb:  TAS(opcode); break;
		case 0xeb:  JMP(opcode); break;
		case 0xfb:  NOP(opcode); break;
		// 0xc0
		case 0x0c:  SHAD(opcode); break;
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
		case 0x0d:  SHLD(opcode); break;
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
		case 0x0e:  LDCSR(opcode); break;
		case 0x1e:  LDCGBR(opcode); break;
		case 0x2e:  LDCVBR(opcode); break;
		case 0x3e:  LDCSSR(opcode); break;
		case 0x4e:  LDCSPC(opcode); break;
		case 0x5e:  NOP(opcode); break;
		case 0x6e:  NOP(opcode); break;
		case 0x7e:  NOP(opcode); break;
		case 0x8e:  LDCRBANK(opcode); break;
		case 0x9e:  LDCRBANK(opcode); break;
		case 0xae:  LDCRBANK(opcode); break;
		case 0xbe:  LDCRBANK(opcode); break;
		case 0xce:  LDCRBANK(opcode); break;
		case 0xde:  LDCRBANK(opcode); break;
		case 0xee:  LDCRBANK(opcode); break;
		case 0xfe:  LDCRBANK(opcode); break;
		// 0xf0
		case 0x0f:  MAC_W(opcode); break;
		case 0x1f:  MAC_W(opcode); break;
		case 0x2f:  MAC_W(opcode); break;
		case 0x3f:  MAC_W(opcode); break;
		case 0x4f:  MAC_W(opcode); break;
		case 0x5f:  MAC_W(opcode); break;
		case 0x6f:  MAC_W(opcode); break;
		case 0x7f:  MAC_W(opcode); break;
		case 0x8f:  MAC_W(opcode); break;
		case 0x9f:  MAC_W(opcode); break;
		case 0xaf:  MAC_W(opcode); break;
		case 0xbf:  MAC_W(opcode); break;
		case 0xcf:  MAC_W(opcode); break;
		case 0xdf:  MAC_W(opcode); break;
		case 0xef:  MAC_W(opcode); break;
		case 0xff:  MAC_W(opcode); break;
	}
}


inline void sh34_base_device::execute_one(const UINT16 opcode)
{
	switch(opcode & 0xf000)
	{
		case 0x0000:
			execute_one_0000(opcode);
			break;

		case 0x1000:
			MOVLS4(opcode);
			break;

		case 0x2000:
			switch(opcode & 0x0f)
			{
				case 0x00:  MOVBS(opcode); break;
				case 0x01:  MOVWS(opcode); break;
				case 0x02:  MOVLS(opcode); break;
				case 0x03:  NOP(opcode); break;
				case 0x04:  MOVBM(opcode); break;
				case 0x05:  MOVWM(opcode); break;
				case 0x06:  MOVLM(opcode); break;
				case 0x07:  DIV0S(opcode); break;
				case 0x08:  TST(opcode); break;
				case 0x09:  AND(opcode); break;
				case 0x0a:  XOR(opcode); break;
				case 0x0b:  OR(opcode); break;
				case 0x0c:  CMPSTR(opcode); break;
				case 0x0d:  XTRCT(opcode); break;
				case 0x0e:  MULU(opcode); break;
				case 0x0f:  MULS(opcode); break;
			}
			break;

		case 0x3000:
			switch(opcode & 0x0f)
			{
				case 0x00:  CMPEQ(opcode); break;
				case 0x01:  NOP(opcode); break;
				case 0x02:  CMPHS(opcode); break;
				case 0x03:  CMPGE(opcode); break;
				case 0x04:  DIV1(opcode); break;
				case 0x05:  DMULU(opcode); break;
				case 0x06:  CMPHI(opcode); break;
				case 0x07:  CMPGT(opcode); break;
				case 0x08:  SUB(opcode); break;
				case 0x09:  NOP(opcode); break;
				case 0x0a:  SUBC(opcode); break;
				case 0x0b:  SUBV(opcode); break;
				case 0x0c:  ADD(opcode); break;
				case 0x0d:  DMULS(opcode); break;
				case 0x0e:  ADDC(opcode); break;
				case 0x0f:  ADDV(opcode); break;
			}
			break;

		case 0x4000:
			execute_one_4000(opcode);
			break;

		case 0x5000:
			MOVLL4(opcode);
			break;

		case 0x6000:
			switch(opcode & 0x0f)
			{
				case 0x00:  MOVBL(opcode); break;
				case 0x01:  MOVWL(opcode); break;
				case 0x02:  MOVLL(opcode); break;
				case 0x03:  MOV(opcode); break;
				case 0x04:  MOVBP(opcode); break;
				case 0x05:  MOVWP(opcode); break;
				case 0x06:  MOVLP(opcode); break;
				case 0x07:  NOT(opcode); break;
				case 0x08:  SWAPB(opcode); break;
				case 0x09:  SWAPW(opcode); break;
				case 0x0a:  NEGC(opcode); break;
				case 0x0b:  NEG(opcode); break;
				case 0x0c:  EXTUB(opcode); break;
				case 0x0d:  EXTUW(opcode); break;
				case 0x0e:  EXTSB(opcode); break;
				case 0x0f:  EXTSW(opcode); break;
			}
			break;

		case 0x7000:
			ADDI(opcode);
			break;

		case 0x8000:
			switch((opcode >> 8) & 0x0f)
			{
				case 0x00:  MOVBS4(opcode); break;
				case 0x01:  MOVWS4(opcode); break;
				case 0x02:  NOP(opcode); break;
				case 0x03:  NOP(opcode); break;
				case 0x04:  MOVBL4(opcode); break;
				case 0x05:  MOVWL4(opcode); break;
				case 0x06:  NOP(opcode); break;
				case 0x07:  NOP(opcode); break;
				case 0x08:  CMPIM(opcode); break;
				case 0x09:  BT(opcode); break;
				case 0x0a:  NOP(opcode); break;
				case 0x0b:  BF(opcode); break;
				case 0x0c:  NOP(opcode); break;
				case 0x0d:  BTS(opcode); break;
				case 0x0e:  NOP(opcode); break;
				case 0x0f:  BFS(opcode); break;
			}
			break;

		case 0x9000:
			MOVWI(opcode);
			break;

		case 0xa000:
			BRA(opcode);
			break;

		case 0xb000:
			BSR(opcode);
			break;

		case 0xc000:
			switch((opcode >> 8) & 0x0f)
			{
				case 0x00:  MOVBSG(opcode); break;
				case 0x01:  MOVWSG(opcode); break;
				case 0x02:  MOVLSG(opcode); break;
				case 0x03:  TRAPA(opcode); break;
				case 0x04:  MOVBLG(opcode); break;
				case 0x05:  MOVWLG(opcode); break;
				case 0x06:  MOVLLG(opcode); break;
				case 0x07:  MOVA(opcode); break;
				case 0x08:  TSTI(opcode); break;
				case 0x09:  ANDI(opcode); break;
				case 0x0a:  XORI(opcode); break;
				case 0x0b:  ORI(opcode); break;
				case 0x0c:  TSTM(opcode); break;
				case 0x0d:  ANDM(opcode); break;
				case 0x0e:  XORM(opcode); break;
				case 0x0f:  ORM(opcode); break;
			}
			break;

		case 0xd000:
			MOVLI(opcode);
			break;

		case 0xe000:
			MOVI(opcode);
			break;

		case 0xf000:
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
		m_sh4_icount = 0;
		return;
	}

	do
	{
		if (m_delay)
		{
			const UINT16 opcode = m_direct->read_word((UINT32)(m_delay & AM), WORD2_XOR_LE(0));

			debugger_instruction_hook(this, (m_pc-2) & AM);

			m_delay = 0;
			m_ppc = m_pc;

			execute_one(opcode);

			if (m_test_irq && !m_delay)
			{
				sh4_check_pending_irq("mame_sh4_execute");
			}
		}
		else
		{
			const UINT16 opcode = m_direct->read_word((UINT32)(m_pc & AM), WORD2_XOR_LE(0));

			debugger_instruction_hook(this, m_pc & AM);

			m_pc += 2;
			m_ppc = m_pc;

			execute_one(opcode);

			if (m_test_irq && !m_delay)
			{
				sh4_check_pending_irq("mame_sh4_execute");
			}
		}

		m_sh4_icount--;
	} while( m_sh4_icount > 0 );
}

void sh3be_device::execute_run()
{
	if (m_cpu_off)
	{
		m_sh4_icount = 0;
		return;
	}

	do
	{
		if (m_delay)
		{
			const UINT16 opcode = m_direct->read_word((UINT32)(m_delay & AM), WORD_XOR_LE(6));

			debugger_instruction_hook(this, m_delay & AM);

			m_delay = 0;
			m_ppc = m_pc;

			execute_one(opcode);


			if (m_test_irq && !m_delay)
			{
				sh4_check_pending_irq("mame_sh4_execute");
			}


		}
		else
		{
			const UINT16 opcode = m_direct->read_word((UINT32)(m_pc & AM), WORD_XOR_LE(6));

			debugger_instruction_hook(this, m_pc & AM);

			m_pc += 2;
			m_ppc = m_pc;

			execute_one(opcode);

			if (m_test_irq && !m_delay)
			{
				sh4_check_pending_irq("mame_sh4_execute");
			}
		}

		m_sh4_icount--;
	} while( m_sh4_icount > 0 );
}

void sh4be_device::execute_run()
{
	if (m_cpu_off)
	{
		m_sh4_icount = 0;
		return;
	}

	do
	{
		if (m_delay)
		{
			const UINT16 opcode = m_direct->read_word((UINT32)(m_delay & AM), WORD_XOR_LE(6));

			debugger_instruction_hook(this, m_delay & AM);

			m_delay = 0;
			m_ppc = m_pc;

			execute_one(opcode);


			if (m_test_irq && !m_delay)
			{
				sh4_check_pending_irq("mame_sh4_execute");
			}


		}
		else
		{
			const UINT16 opcode = m_direct->read_word((UINT32)(m_pc & AM), WORD_XOR_LE(6));

			debugger_instruction_hook(this, m_pc & AM);

			m_pc += 2;
			m_ppc = m_pc;

			execute_one(opcode);

			if (m_test_irq && !m_delay)
			{
				sh4_check_pending_irq("mame_sh4_execute");
			}
		}

		m_sh4_icount--;
	} while( m_sh4_icount > 0 );
}

void sh34_base_device::device_start()
{
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

	save_item(NAME(m_pc));
	save_item(NAME(m_r[15]));
	save_item(NAME(m_sr));
	save_item(NAME(m_pr));
	save_item(NAME(m_gbr));
	save_item(NAME(m_vbr));
	save_item(NAME(m_mach));
	save_item(NAME(m_macl));
	save_item(NAME(m_spc));
	save_item(NAME(m_ssr));
	save_item(NAME(m_sgr));
	save_item(NAME(m_fpscr));
	save_item(NAME(m_r[ 0]));
	save_item(NAME(m_r[ 1]));
	save_item(NAME(m_r[ 2]));
	save_item(NAME(m_r[ 3]));
	save_item(NAME(m_r[ 4]));
	save_item(NAME(m_r[ 5]));
	save_item(NAME(m_r[ 6]));
	save_item(NAME(m_r[ 7]));
	save_item(NAME(m_r[ 8]));
	save_item(NAME(m_r[ 9]));
	save_item(NAME(m_r[10]));
	save_item(NAME(m_r[11]));
	save_item(NAME(m_r[12]));
	save_item(NAME(m_r[13]));
	save_item(NAME(m_r[14]));
	save_item(NAME(m_fr[ 0]));
	save_item(NAME(m_fr[ 1]));
	save_item(NAME(m_fr[ 2]));
	save_item(NAME(m_fr[ 3]));
	save_item(NAME(m_fr[ 4]));
	save_item(NAME(m_fr[ 5]));
	save_item(NAME(m_fr[ 6]));
	save_item(NAME(m_fr[ 7]));
	save_item(NAME(m_fr[ 8]));
	save_item(NAME(m_fr[ 9]));
	save_item(NAME(m_fr[10]));
	save_item(NAME(m_fr[11]));
	save_item(NAME(m_fr[12]));
	save_item(NAME(m_fr[13]));
	save_item(NAME(m_fr[14]));
	save_item(NAME(m_fr[15]));
	save_item(NAME(m_xf[ 0]));
	save_item(NAME(m_xf[ 1]));
	save_item(NAME(m_xf[ 2]));
	save_item(NAME(m_xf[ 3]));
	save_item(NAME(m_xf[ 4]));
	save_item(NAME(m_xf[ 5]));
	save_item(NAME(m_xf[ 6]));
	save_item(NAME(m_xf[ 7]));
	save_item(NAME(m_xf[ 8]));
	save_item(NAME(m_xf[ 9]));
	save_item(NAME(m_xf[10]));
	save_item(NAME(m_xf[11]));
	save_item(NAME(m_xf[12]));
	save_item(NAME(m_xf[13]));
	save_item(NAME(m_xf[14]));
	save_item(NAME(m_xf[15]));
	save_item(NAME(m_ea));
	save_item(NAME(m_fpul));
	save_item(NAME(m_dbr));
	save_item(NAME(m_exception_priority));
	save_item(NAME(m_exception_requesting));

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

	// Debugger state

	state_add(SH4_PC,             "PC", m_pc).formatstr("%08X").callimport();
	state_add(SH4_SR,             "SR", m_sr).formatstr("%08X").callimport();
	state_add(SH4_PR,             "PR", m_pr).formatstr("%08X");
	state_add(SH4_GBR,            "GBR", m_gbr).formatstr("%08X");
	state_add(SH4_VBR,            "VBR", m_vbr).formatstr("%08X");
	state_add(SH4_DBR,            "DBR", m_dbr).formatstr("%08X");
	state_add(SH4_MACH,           "MACH", m_mach).formatstr("%08X");
	state_add(SH4_MACL,           "MACL", m_macl).formatstr("%08X");
	state_add(SH4_R0,             "R0", m_r[ 0]).formatstr("%08X");
	state_add(SH4_R1,             "R1", m_r[ 1]).formatstr("%08X");
	state_add(SH4_R2,             "R2", m_r[ 2]).formatstr("%08X");
	state_add(SH4_R3,             "R3", m_r[ 3]).formatstr("%08X");
	state_add(SH4_R4,             "R4", m_r[ 4]).formatstr("%08X");
	state_add(SH4_R5,             "R5", m_r[ 5]).formatstr("%08X");
	state_add(SH4_R6,             "R6", m_r[ 6]).formatstr("%08X");
	state_add(SH4_R7,             "R7", m_r[ 7]).formatstr("%08X");
	state_add(SH4_R8,             "R8", m_r[ 8]).formatstr("%08X");
	state_add(SH4_R9,             "R9", m_r[ 9]).formatstr("%08X");
	state_add(SH4_R10,            "R10", m_r[10]).formatstr("%08X");
	state_add(SH4_R11,            "R11", m_r[11]).formatstr("%08X");
	state_add(SH4_R12,            "R12", m_r[12]).formatstr("%08X");
	state_add(SH4_R13,            "R13", m_r[13]).formatstr("%08X");
	state_add(SH4_R14,            "R14", m_r[14]).formatstr("%08X");
	state_add(SH4_R15,            "R15", m_r[15]).formatstr("%08X");
	state_add(SH4_EA,             "EA", m_ea).formatstr("%08X");
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
	state_add(STATE_GENSP, "GENSP", m_r[15]).noshow();
	state_add(STATE_GENPCBASE, "GENPCBASE", m_ppc).noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS", m_sr).formatstr("%20s").noshow();

	m_icountptr = &m_sh4_icount;
}

void sh34_base_device::state_import(const device_state_entry &entry)
{
#ifdef LSB_FIRST
	UINT8 fpu_xor = m_fpu_pr;
#else
	UINT8 fpu_xor = 0;
#endif

	switch (entry.index())
	{
		case STATE_GENPC:
			m_pc = m_debugger_temp;
		case SH4_PC:
			m_delay = 0;
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
			m_debugger_temp = (m_delay) ? (m_delay & AM) : (m_pc & AM);
			break;
	}
}

void sh34_base_device::state_string_export(const device_state_entry &entry, std::string &str)
{
#ifdef LSB_FIRST
	UINT8 fpu_xor = m_fpu_pr;
#else
	UINT8 fpu_xor = 0;
#endif

	switch (entry.index())
	{
		case STATE_GENFLAGS:
			strprintf(str, "%s%s%s%s%c%c%d%c%c",
					m_sr & MD ? "MD ":"   ",
					m_sr & sRB ? "RB ":"   ",
					m_sr & BL ? "BL ":"   ",
					m_sr & FD ? "FD ":"   ",
					m_sr & M ? 'M':'.',
					m_sr & Q ? 'Q':'.',
					(m_sr & I) >> 4,
					m_sr & S ? 'S':'.',
					m_sr & T ? 'T':'.');
			break;

		case SH4_FR0:
			strprintf(str, "%08X %f", m_fr[0 ^ fpu_xor], (double)FP_RFS(0 ^ fpu_xor));
			break;

		case SH4_FR1:
			strprintf(str, "%08X %f", m_fr[1 ^ fpu_xor], (double)FP_RFS(1 ^ fpu_xor));
			break;

		case SH4_FR2:
			strprintf(str, "%08X %f", m_fr[2 ^ fpu_xor], (double)FP_RFS(2 ^ fpu_xor));
			break;

		case SH4_FR3:
			strprintf(str, "%08X %f", m_fr[3 ^ fpu_xor], (double)FP_RFS(3 ^ fpu_xor));
			break;

		case SH4_FR4:
			strprintf(str, "%08X %f", m_fr[4 ^ fpu_xor], (double)FP_RFS(4 ^ fpu_xor));
			break;

		case SH4_FR5:
			strprintf(str, "%08X %f", m_fr[5 ^ fpu_xor], (double)FP_RFS(5 ^ fpu_xor));
			break;

		case SH4_FR6:
			strprintf(str, "%08X %f", m_fr[6 ^ fpu_xor], (double)FP_RFS(6 ^ fpu_xor));
			break;

		case SH4_FR7:
			strprintf(str, "%08X %f", m_fr[7 ^ fpu_xor], (double)FP_RFS(7 ^ fpu_xor));
			break;

		case SH4_FR8:
			strprintf(str, "%08X %f", m_fr[8 ^ fpu_xor], (double)FP_RFS(8 ^ fpu_xor));
			break;

		case SH4_FR9:
			strprintf(str, "%08X %f", m_fr[9 ^ fpu_xor], (double)FP_RFS(9 ^ fpu_xor));
			break;

		case SH4_FR10:
			strprintf(str, "%08X %f", m_fr[10 ^ fpu_xor], (double)FP_RFS(10 ^ fpu_xor));
			break;

		case SH4_FR11:
			strprintf(str, "%08X %f", m_fr[11 ^ fpu_xor], (double)FP_RFS(11 ^ fpu_xor));
			break;

		case SH4_FR12:
			strprintf(str, "%08X %f", m_fr[12 ^ fpu_xor], (double)FP_RFS(12 ^ fpu_xor));
			break;

		case SH4_FR13:
			strprintf(str, "%08X %f", m_fr[13 ^ fpu_xor], (double)FP_RFS(13 ^ fpu_xor));
			break;

		case SH4_FR14:
			strprintf(str, "%08X %f", m_fr[14 ^ fpu_xor], (double)FP_RFS(14 ^ fpu_xor));
			break;

		case SH4_FR15:
			strprintf(str, "%08X %f", m_fr[15 ^ fpu_xor], (double)FP_RFS(15 ^ fpu_xor));
			break;

		case SH4_XF0:
			strprintf(str, "%08X %f", m_xf[0 ^ fpu_xor], (double)FP_XFS(0 ^ fpu_xor));
			break;

		case SH4_XF1:
			strprintf(str, "%08X %f", m_xf[1 ^ fpu_xor], (double)FP_XFS(1 ^ fpu_xor));
			break;

		case SH4_XF2:
			strprintf(str, "%08X %f", m_xf[2 ^ fpu_xor], (double)FP_XFS(2 ^ fpu_xor));
			break;

		case SH4_XF3:
			strprintf(str, "%08X %f", m_xf[3 ^ fpu_xor], (double)FP_XFS(3 ^ fpu_xor));
			break;

		case SH4_XF4:
			strprintf(str, "%08X %f", m_xf[4 ^ fpu_xor], (double)FP_XFS(4 ^ fpu_xor));
			break;

		case SH4_XF5:
			strprintf(str, "%08X %f", m_xf[5 ^ fpu_xor], (double)FP_XFS(5 ^ fpu_xor));
			break;

		case SH4_XF6:
			strprintf(str, "%08X %f", m_xf[6 ^ fpu_xor], (double)FP_XFS(6 ^ fpu_xor));
			break;

		case SH4_XF7:
			strprintf(str, "%08X %f", m_xf[7 ^ fpu_xor], (double)FP_XFS(7 ^ fpu_xor));
			break;

		case SH4_XF8:
			strprintf(str, "%08X %f", m_xf[8 ^ fpu_xor], (double)FP_XFS(8 ^ fpu_xor));
			break;

		case SH4_XF9:
			strprintf(str, "%08X %f", m_xf[9 ^ fpu_xor], (double)FP_XFS(9 ^ fpu_xor));
			break;

		case SH4_XF10:
			strprintf(str, "%08X %f", m_xf[10 ^ fpu_xor], (double)FP_XFS(10 ^ fpu_xor));
			break;

		case SH4_XF11:
			strprintf(str, "%08X %f", m_xf[11 ^ fpu_xor], (double)FP_XFS(11 ^ fpu_xor));
			break;

		case SH4_XF12:
			strprintf(str, "%08X %f", m_xf[12 ^ fpu_xor], (double)FP_XFS(12 ^ fpu_xor));
			break;

		case SH4_XF13:
			strprintf(str, "%08X %f", m_xf[13 ^ fpu_xor], (double)FP_XFS(13 ^ fpu_xor));
			break;

		case SH4_XF14:
			strprintf(str, "%08X %f", m_xf[14 ^ fpu_xor], (double)FP_XFS(14 ^ fpu_xor));
			break;

		case SH4_XF15:
			strprintf(str, "%08X %f", m_xf[15 ^ fpu_xor], (double)FP_XFS(15 ^ fpu_xor));
			break;

	}
}


void sh34_base_device::sh4_set_ftcsr_callback(sh4_ftcsr_callback callback)
{
	m_ftcsr_read_callback = callback;
}
