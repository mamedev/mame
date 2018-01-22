// license:BSD-3-Clause
// copyright-holders:Alex Pasadyn,Zsolt Vasvari
/***************************************************************************

    TMS34010: Portable Texas Instruments TMS34010 emulator

    Copyright Alex Pasadyn/Zsolt Vasvari
    Parts based on code by Aaron Giles

***************************************************************************/

#ifndef MAME_CPU_TMS34010_34010OPS_H
#define MAME_CPU_TMS34010_34010OPS_H

#pragma once




/***************************************************************************
    MEMORY I/O MACROS
***************************************************************************/

#define TMS34010_RDMEM(A)         ((unsigned)m_program->read_byte (A))
#define TMS34010_RDMEM_WORD(A)    ((unsigned)m_program->read_word (A))
inline uint32_t tms340x0_device::TMS34010_RDMEM_DWORD(offs_t A)
{
	uint32_t result = m_program->read_word(A);
	return result | (m_program->read_word(A+16)<<16);
}

#define TMS34010_WRMEM(A,V)       (m_program->write_byte(A,V))
#define TMS34010_WRMEM_WORD(A,V)  (m_program->write_word(A,V))
inline void tms340x0_device::TMS34010_WRMEM_DWORD(offs_t A, uint32_t V)
{
	m_program->write_word(A,V);
	m_program->write_word(A+16,V>>16);
}



/* IO registers accessor */
#define IOREG(reg)                (m_IOregs[reg])
#define SMART_IOREG(reg)          (m_IOregs[m_is_34020 ? (int)REG020_##reg : (int)REG_##reg])
#define PBH()                     (IOREG(REG_CONTROL) & 0x0100)
#define PBV()                     (IOREG(REG_CONTROL) & 0x0200)



/***************************************************************************
    FIELD WRITE MACROS
***************************************************************************/

#define WFIELDMAC(MASK,MAX)                                                         \
	uint32_t shift = offset & 0x0f;                                                 \
	uint32_t masked_data = data & (MASK);                                           \
	uint32_t old;                                                                   \
																					\
	offset = offset & 0xfffffff0;                                                   \
																					\
	if (shift >= MAX)                                                               \
	{                                                                               \
		old = (uint32_t)TMS34010_RDMEM_DWORD(offset) & ~((MASK) << shift);          \
		TMS34010_WRMEM_DWORD(offset, (masked_data << shift) | old);                 \
	}                                                                               \
	else                                                                            \
	{                                                                               \
		old = (uint32_t)TMS34010_RDMEM_WORD(offset) & ~((MASK) << shift);           \
		TMS34010_WRMEM_WORD(offset, ((masked_data & (MASK)) << shift) | old);       \
	}

#define WFIELDMAC_BIG(MASK,MAX)                                                     \
	uint32_t shift = offset & 0x0f;                                                 \
	uint32_t masked_data = data & (MASK);                                           \
	uint32_t old;                                                                   \
																					\
	offset = offset & 0xfffffff0;                                                   \
																					\
	old = (uint32_t)TMS34010_RDMEM_DWORD(offset) & ~(uint32_t)((MASK) << shift);    \
	TMS34010_WRMEM_DWORD(offset, (uint32_t)(masked_data << shift) | old);           \
	if (shift >= MAX)                                                               \
	{                                                                               \
		shift = 32 - shift;                                                         \
		old = (uint32_t)TMS34010_RDMEM_WORD(offset + 0x20) & ~((MASK) >> shift);    \
		TMS34010_WRMEM_WORD(offset, (masked_data >> shift) | old);                  \
	}

#define WFIELDMAC_8()                                                               \
	if (offset & 0x07)                                                              \
	{                                                                               \
		WFIELDMAC(0xff,9);                                                          \
	}                                                                               \
	else                                                                            \
		TMS34010_WRMEM(offset, data);

#define RFIELDMAC_8()                                                               \
	if (offset & 0x07)                                                              \
	{                                                                               \
		RFIELDMAC(0xff,9);                                                          \
	}                                                                               \
	else                                                                            \
		ret = TMS34010_RDMEM(offset);

#define WFIELDMAC_32()                                                              \
	if (offset & 0x0f)                                                              \
	{                                                                               \
		uint32_t shift = offset&0x0f;                                               \
		uint32_t old;                                                               \
		uint32_t hiword;                                                            \
		offset &= 0xfffffff0;                                                       \
		old =    ((uint32_t) TMS34010_RDMEM_DWORD (offset     )&(0xffffffff>>(0x20-shift)));   \
		hiword = ((uint32_t) TMS34010_RDMEM_DWORD (offset+0x20)&(0xffffffff<<shift));      \
		TMS34010_WRMEM_DWORD(offset     ,(data<<      shift) |old);                 \
		TMS34010_WRMEM_DWORD(offset+0x20,(data>>(0x20-shift))|hiword);              \
	}                                                                               \
	else                                                                            \
		TMS34010_WRMEM_DWORD(offset,data);


/***************************************************************************
    FIELD READ MACROS
***************************************************************************/

#define RFIELDMAC(MASK,MAX)                                                         \
	uint32_t shift = offset & 0x0f;                                                 \
	offset = offset & 0xfffffff0;                                                   \
																					\
	if (shift >= MAX)                                                               \
		ret = (TMS34010_RDMEM_DWORD(offset) >> shift) & (MASK);                     \
	else                                                                            \
		ret = (TMS34010_RDMEM_WORD(offset) >> shift) & (MASK);

#define RFIELDMAC_BIG(MASK,MAX)                                                     \
	uint32_t shift = offset & 0x0f;                                                 \
	offset = offset & 0xfffffff0;                                                   \
																					\
	ret = (uint32_t)TMS34010_RDMEM_DWORD(offset) >> shift;                          \
	if (shift >= MAX)                                                               \
		ret |= (TMS34010_RDMEM_WORD(offset + 0x20) << (32 - shift));                \
	ret &= MASK;

#define RFIELDMAC_32()                                                              \
	if (offset&0x0f)                                                                \
	{                                                                               \
		uint32_t shift = offset&0x0f;                                               \
		offset &= 0xfffffff0;                                                       \
		return (((uint32_t)TMS34010_RDMEM_DWORD (offset     )>>      shift) |       \
						  (TMS34010_RDMEM_DWORD (offset+0x20)<<(0x20-shift)));      \
	}                                                                               \
	else                                                                            \
		return TMS34010_RDMEM_DWORD(offset);


#endif // MAME_CPU_TMS34010_34010OPS_H
