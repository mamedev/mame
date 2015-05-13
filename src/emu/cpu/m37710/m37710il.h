// license:BSD-3-Clause
// copyright-holders:R. Belmont, Karl Stenerud, hap
#pragma once

#ifndef __M37710IL_H__
#define __M37710IL_H__


/* ======================================================================== */
/* ================================= MEMORY =============================== */
/* ======================================================================== */

inline UINT32 m37710_cpu_device::m37710i_read_8_normal(UINT32 address)
{
	return m37710_read_8(address);
}

inline UINT32 m37710_cpu_device::m37710i_read_8_immediate(UINT32 address)
{
	return m37710_read_8_immediate(address);
}

inline UINT32 m37710_cpu_device::m37710i_read_8_direct(UINT32 address)
{
	return m37710_read_8(address);
}

inline void m37710_cpu_device::m37710i_write_8_normal(UINT32 address, UINT32 value)
{
	m37710_write_8(address, value);
}

inline void m37710_cpu_device::m37710i_write_8_direct(UINT32 address, UINT32 value)
{
	m37710_write_8(address, value);
}

inline UINT32 m37710_cpu_device::m37710i_read_16_normal(UINT32 address)
{
	return m37710_read_16(address);
}

inline UINT32 m37710_cpu_device::m37710i_read_16_immediate(UINT32 address)
{
	if (address & 1)
		return m37710_read_8_immediate(address) | (m37710_read_8_immediate(address+1)<<8);
	else
		return m37710_read_16_immediate(address);
}

inline UINT32 m37710_cpu_device::m37710i_read_16_direct(UINT32 address)
{
	return m37710_read_16(address);
}

inline void m37710_cpu_device::m37710i_write_16_normal(UINT32 address, UINT32 value)
{
	m37710_write_16(address, value);
}

inline void m37710_cpu_device::m37710i_write_16_direct(UINT32 address, UINT32 value)
{
	m37710_write_16(address, value);
}

inline UINT32 m37710_cpu_device::m37710i_read_24_normal(UINT32 address)
{
	if (address & 1)
		return m37710_read_8(address) | (m37710_read_16(address+1)<<8);
	else
		return m37710_read_16(address) | (m37710_read_8(address+2)<<16);
}

inline UINT32 m37710_cpu_device::m37710i_read_24_immediate(UINT32 address)
{
	if (address & 1)
		return m37710_read_8_immediate(address) | (m37710_read_16_immediate(address+1)<<8);
	else
		return m37710_read_16_immediate(address) | (m37710_read_8_immediate(address+2)<<16);
}

inline UINT32 m37710_cpu_device::m37710i_read_24_direct(UINT32 address)
{
	if (address & 1)
		return m37710_read_8(address) | (m37710_read_16(address+1)<<8);
	else
		return m37710_read_16(address) | (m37710_read_8(address+2)<<16);
}


/* ======================================================================== */
/* ================================= STACK ================================ */
/* ======================================================================== */

inline void m37710_cpu_device::m37710i_push_8(UINT32 value)
{
	m37710_write_8(REG_S, value);
	REG_S = MAKE_UINT_16(REG_S-1);
}

inline UINT32 m37710_cpu_device::m37710i_pull_8()
{
	REG_S = MAKE_UINT_16(REG_S+1);
	return m37710_read_8(REG_S);
}

inline void m37710_cpu_device::m37710i_push_16(UINT32 value)
{
	m37710i_push_8(value>>8);
	m37710i_push_8(value);
}

inline UINT32 m37710_cpu_device::m37710i_pull_16()
{
	UINT32 res = m37710i_pull_8();
	return res | (m37710i_pull_8() << 8);
}

inline void m37710_cpu_device::m37710i_push_24(UINT32 value)
{
	m37710i_push_8(value>>16);
	m37710i_push_8((value>>8));
	m37710i_push_8(value);
}

inline UINT32 m37710_cpu_device::m37710i_pull_24()
{
	UINT32 res = m37710i_pull_8();
	res |= m37710i_pull_8() << 8;
	return res | (m37710i_pull_8() << 16);
}


/* ======================================================================== */
/* ============================ PROGRAM COUNTER =========================== */
/* ======================================================================== */

inline void m37710_cpu_device::m37710i_jump_16(UINT32 address)
{
	REG_PC = MAKE_UINT_16(address);
}

inline void m37710_cpu_device::m37710i_jump_24(UINT32 address)
{
	REG_PB = address&0xff0000;
	REG_PC = MAKE_UINT_16(address);
}

inline void m37710_cpu_device::m37710i_branch_8(UINT32 offset)
{
	REG_PC = MAKE_UINT_16(REG_PC + MAKE_INT_8(offset));
}

inline void m37710_cpu_device::m37710i_branch_16(UINT32 offset)
{
	REG_PC = MAKE_UINT_16(REG_PC + offset);
}


/* ======================================================================== */
/* ============================ STATUS REGISTER =========================== */
/* ======================================================================== */

inline UINT32 m37710_cpu_device::m37710i_get_reg_p()
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

inline void m37710_cpu_device::m37710i_set_reg_ipl(UINT32 value)
{
	m_ipl = value & 7;
}


/* ======================================================================== */
/* ============================= ADDRESS MODES ============================ */
/* ======================================================================== */

inline UINT32 m37710_cpu_device::EA_IMM8()  {REG_PC += 1; return REG_PB | MAKE_UINT_16(REG_PC-1);}
inline UINT32 m37710_cpu_device::EA_IMM16() {REG_PC += 2; return REG_PB | MAKE_UINT_16(REG_PC-2);}
inline UINT32 m37710_cpu_device::EA_IMM24() {REG_PC += 3; return REG_PB | MAKE_UINT_16(REG_PC-3);}
inline UINT32 m37710_cpu_device::EA_D()     {if(MAKE_UINT_8(REG_D)) CLK(1); return MAKE_UINT_16(REG_D + OPER_8_IMM());}
inline UINT32 m37710_cpu_device::EA_A()     {return REG_DB | OPER_16_IMM();}
inline UINT32 m37710_cpu_device::EA_AL()    {return OPER_24_IMM();}
inline UINT32 m37710_cpu_device::EA_DX()    {return MAKE_UINT_16(REG_D + OPER_8_IMM() + REG_X);}
inline UINT32 m37710_cpu_device::EA_DY()    {return MAKE_UINT_16(REG_D + OPER_8_IMM() + REG_Y);}
inline UINT32 m37710_cpu_device::EA_AX()    {UINT32 tmp = EA_A(); if((tmp^(tmp+REG_X))&0xff00) CLK(1); return tmp + REG_X;}
inline UINT32 m37710_cpu_device::EA_ALX()   {return EA_AL() + REG_X;}
inline UINT32 m37710_cpu_device::EA_AY()    {UINT32 tmp = EA_A(); if((tmp^(tmp+REG_X))&0xff00) CLK(1); return tmp + REG_Y;}
inline UINT32 m37710_cpu_device::EA_DI()    {return REG_DB | OPER_16_D();}
inline UINT32 m37710_cpu_device::EA_DLI()   {return OPER_24_D();}
inline UINT32 m37710_cpu_device::EA_AI()    {return read_16_A(OPER_16_IMM());}
inline UINT32 m37710_cpu_device::EA_ALI()   {return OPER_24_A();}
inline UINT32 m37710_cpu_device::EA_DXI()   {return REG_DB | OPER_16_DX();}
inline UINT32 m37710_cpu_device::EA_DIY()   {UINT32 tmp = REG_DB | OPER_16_D(); if((tmp^(tmp+REG_X))&0xff00) CLK(1); return tmp + REG_Y;}
inline UINT32 m37710_cpu_device::EA_DLIY()  {return OPER_24_D() + REG_Y;}
inline UINT32 m37710_cpu_device::EA_AXI()   {return read_16_AXI(MAKE_UINT_16(OPER_16_IMM() + REG_X));}
inline UINT32 m37710_cpu_device::EA_S()     {return MAKE_UINT_16(REG_S + OPER_8_IMM());}
inline UINT32 m37710_cpu_device::EA_SIY()   {return MAKE_UINT_16(read_16_SIY(REG_S + OPER_8_IMM()) + REG_Y) | REG_DB;}

#endif /* __M37710IL_H__ */
