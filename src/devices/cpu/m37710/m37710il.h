// license:BSD-3-Clause
// copyright-holders:R. Belmont, Karl Stenerud
#ifndef MAME_CPU_M37710_M37710IL_H
#define MAME_CPU_M37710_M37710IL_H

#pragma once

/* ======================================================================== */
/* ================================= MEMORY =============================== */
/* ======================================================================== */

inline uint32_t m37710_cpu_device::m37710i_read_8_normal(uint32_t address)
{
	return m37710_read_8(address);
}

inline uint32_t m37710_cpu_device::m37710i_read_8_immediate(uint32_t address)
{
	return m37710_read_8_immediate(address);
}

inline uint32_t m37710_cpu_device::m37710i_read_8_direct(uint32_t address)
{
	return m37710_read_8(address);
}

inline void m37710_cpu_device::m37710i_write_8_normal(uint32_t address, uint32_t value)
{
	m37710_write_8(address, value);
}

inline void m37710_cpu_device::m37710i_write_8_direct(uint32_t address, uint32_t value)
{
	m37710_write_8(address, value);
}

inline uint32_t m37710_cpu_device::m37710i_read_16_normal(uint32_t address)
{
	return m37710_read_16(address);
}

inline uint32_t m37710_cpu_device::m37710i_read_16_immediate(uint32_t address)
{
	if (!WORD_ALIGNED(address))
		return m37710_read_8_immediate(address) | (m37710_read_8_immediate(address+1)<<8);
	else
		return m37710_read_16_immediate(address);
}

inline uint32_t m37710_cpu_device::m37710i_read_16_direct(uint32_t address)
{
	return m37710_read_16(address);
}

inline void m37710_cpu_device::m37710i_write_16_normal(uint32_t address, uint32_t value)
{
	m37710_write_16(address, value);
}

inline void m37710_cpu_device::m37710i_write_16_direct(uint32_t address, uint32_t value)
{
	m37710_write_16(address, value);
}

inline uint32_t m37710_cpu_device::m37710i_read_24_normal(uint32_t address)
{
	if (!WORD_ALIGNED(address))
		return m37710_read_8(address) | (m37710_read_16(address+1)<<8);
	else
		return m37710_read_16(address) | (m37710_read_8(address+2)<<16);
}

inline uint32_t m37710_cpu_device::m37710i_read_24_immediate(uint32_t address)
{
	if (!WORD_ALIGNED(address))
		return m37710_read_8_immediate(address) | (m37710_read_16_immediate(address+1)<<8);
	else
		return m37710_read_16_immediate(address) | (m37710_read_8_immediate(address+2)<<16);
}

inline uint32_t m37710_cpu_device::m37710i_read_24_direct(uint32_t address)
{
	if (!WORD_ALIGNED(address))
		return m37710_read_8(address) | (m37710_read_16(address+1)<<8);
	else
		return m37710_read_16(address) | (m37710_read_8(address+2)<<16);
}


/* ======================================================================== */
/* ================================= STACK ================================ */
/* ======================================================================== */

inline void m37710_cpu_device::m37710i_push_8(uint32_t value)
{
	m37710_write_8(REG_S, value);
	REG_S = MAKE_UINT_16(REG_S-1);
}

inline uint32_t m37710_cpu_device::m37710i_pull_8()
{
	REG_S = MAKE_UINT_16(REG_S+1);
	return m37710_read_8(REG_S);
}

inline void m37710_cpu_device::m37710i_push_16(uint32_t value)
{
	m37710i_push_8(value>>8);
	m37710i_push_8(value);
}

inline uint32_t m37710_cpu_device::m37710i_pull_16()
{
	uint32_t res = m37710i_pull_8();
	return res | (m37710i_pull_8() << 8);
}

inline void m37710_cpu_device::m37710i_push_24(uint32_t value)
{
	m37710i_push_8(value>>16);
	m37710i_push_8((value>>8));
	m37710i_push_8(value);
}

inline uint32_t m37710_cpu_device::m37710i_pull_24()
{
	uint32_t res = m37710i_pull_8();
	res |= m37710i_pull_8() << 8;
	return res | (m37710i_pull_8() << 16);
}


/* ======================================================================== */
/* ============================ PROGRAM COUNTER =========================== */
/* ======================================================================== */

inline void m37710_cpu_device::m37710i_jump_16(uint32_t address)
{
	REG_PC = MAKE_UINT_16(address);
}

inline void m37710_cpu_device::m37710i_jump_24(uint32_t address)
{
	REG_PG = address&0xff0000;
	REG_PC = MAKE_UINT_16(address);
}

inline void m37710_cpu_device::m37710i_branch_8(uint32_t offset)
{
	REG_PC = MAKE_UINT_16(REG_PC + MAKE_INT_8(offset));
}

inline void m37710_cpu_device::m37710i_branch_16(uint32_t offset)
{
	REG_PC = MAKE_UINT_16(REG_PC + offset);
}


/* ======================================================================== */
/* ============================ STATUS REGISTER =========================== */
/* ======================================================================== */

inline uint32_t m37710_cpu_device::m37710i_get_reg_ps()
{
	return  (FLAG_N&0x80)       |
			((FLAG_V>>1)&0x40)  |
			FLAG_M              |
			FLAG_X              |
			FLAG_D              |
			FLAG_I              |
			((!FLAG_Z)<<1)      |
			((FLAG_C>>8)&1);
}

inline void m37710_cpu_device::m37710i_set_reg_ipl(uint32_t value)
{
	m_ipl = value & 7;
}


/* ======================================================================== */
/* ============================= ADDRESS MODES ============================ */
/* ======================================================================== */

inline uint32_t m37710_cpu_device::EA_IMM8()  {REG_PC += 1; return REG_PG | MAKE_UINT_16(REG_PC-1);}
inline uint32_t m37710_cpu_device::EA_IMM16() {REG_PC += 2; return REG_PG | MAKE_UINT_16(REG_PC-2);}
inline uint32_t m37710_cpu_device::EA_IMM24() {REG_PC += 3; return REG_PG | MAKE_UINT_16(REG_PC-3);}
inline uint32_t m37710_cpu_device::EA_D()     {if(MAKE_UINT_8(REG_DPR)) CLK(1); return MAKE_UINT_16(REG_DPR + OPER_8_IMM());}
inline uint32_t m37710_cpu_device::EA_A()     {return REG_DT | OPER_16_IMM();}
inline uint32_t m37710_cpu_device::EA_AL()    {return OPER_24_IMM();}
inline uint32_t m37710_cpu_device::EA_DX()    {return MAKE_UINT_16(REG_DPR + OPER_8_IMM() + REG_X);}
inline uint32_t m37710_cpu_device::EA_DY()    {return MAKE_UINT_16(REG_DPR + OPER_8_IMM() + REG_Y);}
inline uint32_t m37710_cpu_device::EA_AX()    {uint32_t tmp = EA_A(); if((tmp^(tmp+REG_X))&0xff00) CLK(1); return tmp + REG_X;}
inline uint32_t m37710_cpu_device::EA_ALX()   {return EA_AL() + REG_X;}
inline uint32_t m37710_cpu_device::EA_AY()    {uint32_t tmp = EA_A(); if((tmp^(tmp+REG_X))&0xff00) CLK(1); return tmp + REG_Y;}
inline uint32_t m37710_cpu_device::EA_DI()    {return REG_DT | OPER_16_D();}
inline uint32_t m37710_cpu_device::EA_DLI()   {return OPER_24_D();}
inline uint32_t m37710_cpu_device::EA_AI()    {return read_16_A(OPER_16_IMM());}
inline uint32_t m37710_cpu_device::EA_ALI()   {return OPER_24_A();}
inline uint32_t m37710_cpu_device::EA_DXI()   {return REG_DT | OPER_16_DX();}
inline uint32_t m37710_cpu_device::EA_DIY()   {uint32_t tmp = REG_DT | OPER_16_D(); if((tmp^(tmp+REG_X))&0xff00) CLK(1); return tmp + REG_Y;}
inline uint32_t m37710_cpu_device::EA_DLIY()  {return OPER_24_D() + REG_Y;}
inline uint32_t m37710_cpu_device::EA_AXI()   {return read_16_AXI(MAKE_UINT_16(OPER_16_IMM() + REG_X));}
inline uint32_t m37710_cpu_device::EA_S()     {return MAKE_UINT_16(REG_S + OPER_8_IMM());}
inline uint32_t m37710_cpu_device::EA_SIY()   {return MAKE_UINT_16(read_16_SIY(REG_S + OPER_8_IMM()) + REG_Y) | REG_DT;}

#endif /* MAME_CPU_M37710_M37710IL_H */
