/***************************************************************************

    TMS34010: Portable Texas Instruments TMS34010 emulator

    Copyright Alex Pasadyn/Zsolt Vasvari
    Parts based on code by Aaron Giles

***************************************************************************/

#pragma once

#ifndef __34010OPS_H__
#define __34010OPS_H__


/* Size of the memory buffer allocated for the shiftr register */
#define SHIFTREG_SIZE           (8 * 512 * sizeof(UINT16))



/***************************************************************************
    MEMORY I/O MACROS
***************************************************************************/

#define TMS34010_RDMEM(T,A)         ((unsigned)(T)->program->read_byte (A))
#define TMS34010_RDMEM_WORD(T,A)    ((unsigned)(T)->program->read_word (A))
INLINE UINT32 TMS34010_RDMEM_DWORD(tms34010_state *tms, offs_t A)
{
	UINT32 result = tms->program->read_word(A);
	return result | (tms->program->read_word(A+2)<<16);
}

#define TMS34010_WRMEM(T,A,V)       ((T)->program->write_byte(A,V))
#define TMS34010_WRMEM_WORD(T,A,V)  ((T)->program->write_word(A,V))
INLINE void TMS34010_WRMEM_DWORD(tms34010_state *tms, offs_t A,UINT32 V)
{
	tms->program->write_word(A,V);
	tms->program->write_word(A+2,V>>16);
}



/* IO registers accessor */
#define IOREG(T,reg)                ((T)->IOregs[reg])
#define SMART_IOREG(T,reg)          ((T)->IOregs[(T)->is_34020 ? (int)REG020_##reg : (int)REG_##reg])
#define PBH(T)                      (IOREG(T, REG_CONTROL) & 0x0100)
#define PBV(T)                      (IOREG(T, REG_CONTROL) & 0x0200)



/***************************************************************************
    FIELD WRITE MACROS
***************************************************************************/

#define WFIELDMAC(T,MASK,MAX)                                                       \
	UINT32 shift = offset & 0x0f;                                                   \
	UINT32 masked_data = data & (MASK);                                             \
	UINT32 old;                                                                     \
																					\
	offset = TOBYTE(offset & 0xfffffff0);                                           \
																					\
	if (shift >= MAX)                                                               \
	{                                                                               \
		old = (UINT32)TMS34010_RDMEM_DWORD(T, offset) & ~((MASK) << shift);             \
		TMS34010_WRMEM_DWORD(T, offset, (masked_data << shift) | old);                  \
	}                                                                               \
	else                                                                            \
	{                                                                               \
		old = (UINT32)TMS34010_RDMEM_WORD(T, offset) & ~((MASK) << shift);          \
		TMS34010_WRMEM_WORD(T, offset, ((masked_data & (MASK)) << shift) | old);        \
	}
#define WFIELDMAC_BIG(T,MASK,MAX)                                                       \
	UINT32 shift = offset & 0x0f;                                                   \
	UINT32 masked_data = data & (MASK);                                             \
	UINT32 old;                                                                     \
																					\
	offset = TOBYTE(offset & 0xfffffff0);                                           \
																					\
	old = (UINT32)TMS34010_RDMEM_DWORD(T, offset) & ~(UINT32)((MASK) << shift);     \
	TMS34010_WRMEM_DWORD(T, offset, (UINT32)(masked_data << shift) | old);              \
	if (shift >= MAX)                                                               \
	{                                                                               \
		shift = 32 - shift;                                                         \
		old = (UINT32)TMS34010_RDMEM_WORD(T, offset + 4) & ~((MASK) >> shift);          \
		TMS34010_WRMEM_WORD(T, offset, (masked_data >> shift) | old);                   \
	}
#define WFIELDMAC_8(T)                                                                  \
	if (offset & 0x07)                                                              \
	{                                                                               \
		WFIELDMAC(T,0xff,9);                                                            \
	}                                                                               \
	else                                                                            \
		TMS34010_WRMEM(T, TOBYTE(offset), data);
#define RFIELDMAC_8(T)                                                              \
	if (offset & 0x07)                                                              \
	{                                                                               \
		RFIELDMAC(T,0xff,9);                                                            \
	}                                                                               \
	else                                                                            \
		return TMS34010_RDMEM(T, TOBYTE(offset));
#define WFIELDMAC_32(T)                                                             \
	if (offset & 0x0f)                                                              \
	{                                                                               \
		UINT32 shift = offset&0x0f;                                                 \
		UINT32 old;                                                                 \
		UINT32 hiword;                                                              \
		offset &= 0xfffffff0;                                                       \
		old =    ((UINT32) TMS34010_RDMEM_DWORD (T, TOBYTE(offset     ))&(0xffffffff>>(0x20-shift)));   \
		hiword = ((UINT32) TMS34010_RDMEM_DWORD (T, TOBYTE(offset+0x20))&(0xffffffff<<shift));      \
		TMS34010_WRMEM_DWORD(T, TOBYTE(offset     ),(data<<      shift) |old);          \
		TMS34010_WRMEM_DWORD(T, TOBYTE(offset+0x20),(data>>(0x20-shift))|hiword);       \
	}                                                                               \
	else                                                                            \
		TMS34010_WRMEM_DWORD(T, TOBYTE(offset),data);


/***************************************************************************
    FIELD READ MACROS
***************************************************************************/

#define RFIELDMAC(T,MASK,MAX)                                                           \
	UINT32 shift = offset & 0x0f;                                                   \
	offset = TOBYTE(offset & 0xfffffff0);                                           \
																					\
	if (shift >= MAX)                                                               \
		ret = (TMS34010_RDMEM_DWORD(T, offset) >> shift) & (MASK);                      \
	else                                                                            \
		ret = (TMS34010_RDMEM_WORD(T, offset) >> shift) & (MASK);
#define RFIELDMAC_BIG(T,MASK,MAX)                                                       \
	UINT32 shift = offset & 0x0f;                                                   \
	offset = TOBYTE(offset & 0xfffffff0);                                           \
																					\
	ret = (UINT32)TMS34010_RDMEM_DWORD(T, offset) >> shift;                         \
	if (shift >= MAX)                                                               \
		ret |= (TMS34010_RDMEM_WORD(T, offset + 4) << (32 - shift));                    \
	ret &= MASK;
#define RFIELDMAC_32(T)                                                             \
	if (offset&0x0f)                                                                \
	{                                                                               \
		UINT32 shift = offset&0x0f;                                                 \
		offset &= 0xfffffff0;                                                       \
		return (((UINT32)TMS34010_RDMEM_DWORD (T, TOBYTE(offset     ))>>      shift) |  \
						(TMS34010_RDMEM_DWORD (T, TOBYTE(offset+0x20))<<(0x20-shift)));\
	}                                                                               \
	else                                                                            \
		return TMS34010_RDMEM_DWORD(T, TOBYTE(offset));


#endif /* __34010OPS_H__ */
