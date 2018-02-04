// license:BSD-3-Clause
// copyright-holders:Aaron Giles
#ifndef MAME_CPU_M6805_M6805DEFS_H
#define MAME_CPU_M6805_M6805DEFS_H

#pragma once

#define SP_MASK m_params.m_sp_mask  // stack pointer mask
#define SP_LOW  m_params.m_sp_floor // stack pointer low water mark
#define PC      m_pc.w.l            // program counter lower word
#define S       m_s.w.l             // stack pointer lower word
#define A       m_a                 // accumulator
#define X       m_x                 // index register
#define CC      m_cc                // condition codes

#define EAD     m_ea.d
#define EA      m_ea.w.l

// pre-clear a PAIR union; clearing h2 and h3 only might be faster?
inline void clear_pair(PAIR &p) { p.d = 0; }

/* macros to tweak the PC and SP */
#define SP_INC  if (++S > SP_MASK) S = SP_LOW
#define SP_DEC  if (--S < SP_LOW) S = SP_MASK
#define SP_ADJUST(s) (((s) & SP_MASK) | SP_LOW)

inline void m6805_base_device::rm16(u32 addr, PAIR &p)
{
	clear_pair(p);
	p.b.h = rm(addr);
	p.b.l = rm(addr + 1);
}

inline void m6805_base_device::pushbyte(u8 b)
{
	wm(S, b);
	SP_DEC;
}

inline void m6805_base_device::pushword(PAIR const &p)
{
	pushbyte(p.b.l);
	pushbyte(p.b.h);
}

inline void m6805_base_device::pullbyte(u8 &b)
{
	SP_INC;
	b = rm(S);
}

inline void m6805_base_device::pullword(PAIR &p)
{
	clear_pair(p);
	pullbyte(p.b.h);
	pullbyte(p.b.l);
}

/* macros to access memory */
template <typename T> inline void m6805_base_device::immbyte(T &b) { b = rdop_arg(PC++); }
inline void m6805_base_device::immword(PAIR &w) { w.d = 0; immbyte(w.b.h); immbyte(w.b.l); }
inline void m6805_base_device::skipbyte() { rdop_arg(PC++); }

/* for treating an unsigned uint8_t as a signed int16_t */
#define SIGNED(b) (int16_t(b & 0x80 ? b | 0xff00 : b))

/* Macros for addressing modes */
#define DIRECT do { EAD=0; immbyte(m_ea.b.l); } while (false)
#define IMM8 do { EA = PC++; } while (false)
#define EXTENDED immword(m_ea)
#define INDEXED do { EA = X; } while (false)
#define INDEXED1 do { EAD = 0; immbyte(m_ea.b.l); EA += X; } while (false)
#define INDEXED2 do { immword(m_ea); EA += X;} while (false)

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
#define ARGADDR \
		do { switch (M) { \
		case addr_mode::IM: static_assert(addr_mode::IM != M, "invalid mode for this instruction"); break; \
		case addr_mode::DI: DIRECT; break; \
		case addr_mode::EX: EXTENDED; break; \
		case addr_mode::IX: INDEXED; break; \
		case addr_mode::IX1: INDEXED1; break; \
		case addr_mode::IX2: INDEXED2; break; \
		} } while (false)
#define DIRBYTE(b) do { DIRECT; b = rm(EAD); } while (false)
#define EXTBYTE(b) do { EXTENDED; b = rm(EAD); } while (false)
#define IDXBYTE(b) do { INDEXED; b = rm(EAD); } while (false)
#define IDX1BYTE(b) do { INDEXED1; b = rm(EAD); } while (false)
#define IDX2BYTE(b) do { INDEXED2; b = rm(EAD); } while (false)
#define ARGBYTE(b) \
		do { switch (M) { \
		case addr_mode::IM: immbyte(b); break; \
		case addr_mode::DI: DIRBYTE(b); break; \
		case addr_mode::EX: EXTBYTE(b); break; \
		case addr_mode::IX: IDXBYTE(b); break; \
		case addr_mode::IX1: IDX1BYTE(b); break; \
		case addr_mode::IX2: IDX2BYTE(b); break; \
		default: b = 0; break; \
		} } while (false)

/* Macros for branch instructions */
#define BRANCH(f) do { u8 t; immbyte(t); if (bool(f) == bool(C)) PC += SIGNED(t); } while (false)

#endif // MAME_CPU_M6805_M6805DEFS_H
