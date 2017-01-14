// license:BSD-3-Clause
// copyright-holders:Aaron Giles
#ifndef MAME_CPU_M6805_M6805DEFS_H
#define MAME_CPU_M6805_M6805DEFS_H

#pragma once

/****************************************************************************/
/* Read a byte from given memory location                                   */
/****************************************************************************/
#define M6805_RDMEM(addr) ((unsigned)m_program->read_byte(addr))

/****************************************************************************/
/* Write a byte to given memory location                                    */
/****************************************************************************/
#define M6805_WRMEM(addr, value) (m_program->write_byte(addr, value))

/****************************************************************************/
/* M6805_RDOP() is identical to M6805_RDMEM() except it is used for reading */
/* opcodes. In case of system with memory mapped I/O, this function can be  */
/* used to greatly speed up emulation                                       */
/****************************************************************************/
#define M6805_RDOP(addr) ((unsigned)m_direct->read_byte(addr))

/****************************************************************************/
/* M6805_RDOP_ARG() is identical to M6805_RDOP() but it's used for reading  */
/* opcode arguments. This difference can be used to support systems that    */
/* use different encoding mechanisms for opcodes and opcode arguments       */
/****************************************************************************/
#define M6805_RDOP_ARG(addr) ((unsigned)m_direct->read_byte(addr))

#define SP_MASK m_sp_mask   /* stack pointer mask */
#define SP_LOW  m_sp_low    /* stack pointer low water mark */
#define PC      m_pc.w.l    /* program counter lower word */
#define S       m_s.w.l     /* stack pointer lower word */
#define A       m_a         /* accumulator */
#define X       m_x         /* index register */
#define CC      m_cc        /* condition codes */

#define EAD m_ea.d
#define EA  m_ea.w.l


/* DS -- THESE ARE RE-DEFINED IN m6805.h TO RAM, ROM or FUNCTIONS IN cpuintrf.c */
#define RM(addr)            M6805_RDMEM(addr)
#define WM(addr, value)     M6805_WRMEM(addr, value)
#define M_RDOP(addr)        M6805_RDOP(addr)
#define M_RDOP_ARG(addr)    M6805_RDOP_ARG(addr)

/* macros to tweak the PC and SP */
#define SP_INC  if( ++S > SP_MASK) S = SP_LOW
#define SP_DEC  if( --S < SP_LOW) S = SP_MASK
#define SP_ADJUST(s) ( ( (s) & SP_MASK ) | SP_LOW )

/* macros to access memory */
#define IMMBYTE(b) {b = M_RDOP_ARG(PC++);}
#define IMMWORD(w) {w.d = 0; w.b.h = M_RDOP_ARG(PC); w.b.l = M_RDOP_ARG(PC+1); PC+=2;}
#define SKIPBYTE() {M_RDOP_ARG(PC++);}

#define PUSHBYTE(b) wr_s_handler_b(&b)
#define PUSHWORD(w) wr_s_handler_w(&w)
#define PULLBYTE(b) rd_s_handler_b(&b)
#define PULLWORD(w) rd_s_handler_w(&w)

/* CC masks      H INZC
              7654 3210 */
#define CFLAG 0x01
#define ZFLAG 0x02
#define NFLAG 0x04
#define IFLAG 0x08
#define HFLAG 0x10

#define CLR_NZ    CC&=~(NFLAG|ZFLAG)
#define CLR_HNZC  CC&=~(HFLAG|NFLAG|ZFLAG|CFLAG)
#define CLR_Z     CC&=~(ZFLAG)
#define CLR_NZC   CC&=~(NFLAG|ZFLAG|CFLAG)
#define CLR_ZC    CC&=~(ZFLAG|CFLAG)

/* macros for CC -- CC bits affected should be reset before calling */
#define SET_Z(a)       if(!a)SEZ
#define SET_Z8(a)      SET_Z((uint8_t)a)
#define SET_N8(a)      CC|=((a&0x80)>>5)
#define SET_H(a,b,r)   CC|=((a^b^r)&0x10)
#define SET_C8(a)      CC|=((a&0x100)>>8)

#define SET_FLAGS8I(a)      {CC |= m_flags8i[(a) & 0xff];}
#define SET_FLAGS8D(a)      {CC |= m_flags8d[(a) & 0xff];}

/* combos */
#define SET_NZ8(a)          {SET_N8(a); SET_Z(a);}
#define SET_FLAGS8(a,b,r)   {SET_N8(r); SET_Z8(r); SET_C8(r);}

/* for treating an unsigned uint8_t as a signed int16_t */
#define SIGNED(b) ((int16_t)(b & 0x80 ? b | 0xff00 : b))

/* Macros for addressing modes */
#define DIRECT EAD=0; IMMBYTE(m_ea.b.l)
#define IMM8 EA = PC++
#define EXTENDED IMMWORD(m_ea)
#define INDEXED EA = X
#define INDEXED1 {EAD = 0; IMMBYTE(m_ea.b.l); EA += X;}
#define INDEXED2 {IMMWORD(m_ea); EA += X;}

/* macros to set status flags */
#if defined(SEC)
#undef SEC
#endif
#define SEC CC |= CFLAG
#define CLC CC &=~ CFLAG
#define SEZ CC |= ZFLAG
#define CLZ CC &=~ ZFLAG
#define SEN CC |= NFLAG
#define CLN CC &=~ NFLAG
#define SEH CC |= HFLAG
#define CLH CC &=~ HFLAG
#define SEI CC |= IFLAG
#define CLI CC &=~ IFLAG

/* macros for convenience */
#define DIRBYTE(b) {DIRECT; b = RM(EAD);}
#define EXTBYTE(b) {EXTENDED; b = RM(EAD);}
#define IDXBYTE(b) {INDEXED; b = RM(EAD);}
#define IDX1BYTE(b) {INDEXED1; b = RM(EAD);}
#define IDX2BYTE(b) {INDEXED2; b = RM(EAD);}
/* Macros for branch instructions */
#define BRANCH(f) { uint8_t t; IMMBYTE(t); if(f) { PC += SIGNED(t); } }

/* pre-clear a PAIR union; clearing h2 and h3 only might be faster? */
#define CLEAR_PAIR(p)   p->d = 0

offs_t CPU_DISASSEMBLE_NAME(m6805)(
		cpu_device *device,
		std::ostream &stream,
		offs_t pc,
		const u8 *oprom,
		const u8 *opram,
		int options,
		std::pair<u16, char const *> const symbols[],
		std::size_t symbol_count);

template <size_t N>
inline offs_t CPU_DISASSEMBLE_NAME(m6805)(
		cpu_device *device,
		std::ostream &stream,
		offs_t pc,
		const u8 *oprom,
		const u8 *opram,
		int options,
		std::pair<u16, char const *> const (&symbols)[N])
{
	return CPU_DISASSEMBLE_NAME(m6805)(device, stream, pc, oprom, opram, options, symbols, N);
}

#endif // MAME_CPU_M6805_M6805DEFS_H
