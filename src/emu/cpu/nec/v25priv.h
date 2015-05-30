// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, Alex W. Jackson
/* Cpu types, steps of 8 to help the cycle count calculation */
#define V33_TYPE 0
#define V30_TYPE 8
#define V20_TYPE 16

#ifndef FALSE
#define FALSE 0
#define TRUE 1
#endif

/* interrupt vectors */
enum
{
	NEC_DIVIDE_VECTOR   = 0,
	NEC_TRAP_VECTOR     = 1,
	NEC_NMI_VECTOR      = 2,
	NEC_BRKV_VECTOR     = 4,
	NEC_CHKIND_VECTOR   = 5,
	NEC_IBRK_VECTOR     = 19,
	NEC_INTTU0_VECTOR   = 28,
	NEC_INTTU1_VECTOR   = 29,
	NEC_INTTU2_VECTOR   = 30,
	NEC_INTD0_VECTOR    = 20,
	NEC_INTD1_VECTOR    = 21,
	NEC_INTP0_VECTOR    = 24,
	NEC_INTP1_VECTOR    = 25,
	NEC_INTP2_VECTOR    = 26,
	NEC_INTSER0_VECTOR  = 12,
	NEC_INTSR0_VECTOR   = 13,
	NEC_INTST0_VECTOR   = 14,
	NEC_INTSER1_VECTOR  = 16,
	NEC_INTSR1_VECTOR   = 17,
	NEC_INTST1_VECTOR   = 18,
	NEC_INTTB_VECTOR    = 31
};

/* interrupt sources */
enum INTSOURCES
{
	BRK     = 0,
	INT_IRQ = 1,
	NMI_IRQ = 1 << 1,
	INTTU0  = 1 << 2,
	INTTU1  = 1 << 3,
	INTTU2  = 1 << 4,
	INTD0   = 1 << 5,
	INTD1   = 1 << 6,
	INTP0   = 1 << 7,
	INTP1   = 1 << 8,
	INTP2   = 1 << 9,
	INTSER0 = 1 << 10,
	INTSR0  = 1 << 11,
	INTST0  = 1 << 12,
	INTSER1 = 1 << 13,
	INTSR1  = 1 << 14,
	INTST1  = 1 << 15,
	INTTB   = 1 << 16,
	BRKN    = 1 << 17,
	BRKS    = 1 << 18
};

enum {
	VECTOR_PC = 0x02/2,
	PSW_SAVE  = 0x04/2,
	PC_SAVE   = 0x06/2
};

enum SREGS {
	DS1 = 0x0E/2,
	PS  = 0x0C/2,
	SS  = 0x0A/2,
	DS0 = 0x08/2
};

enum WREGS {
	AW = 0x1E/2,
	CW = 0x1C/2,
	DW = 0x1A/2,
	BW = 0x18/2,
	SP = 0x16/2,
	BP = 0x14/2,
	IX = 0x12/2,
	IY = 0x10/2
};

enum BREGS {
	AL = NATIVE_ENDIAN_VALUE_LE_BE(0x1E, 0x1F),
	AH = NATIVE_ENDIAN_VALUE_LE_BE(0x1F, 0x1E),
	CL = NATIVE_ENDIAN_VALUE_LE_BE(0x1C, 0x1D),
	CH = NATIVE_ENDIAN_VALUE_LE_BE(0x1D, 0x1C),
	DL = NATIVE_ENDIAN_VALUE_LE_BE(0x1A, 0x1B),
	DH = NATIVE_ENDIAN_VALUE_LE_BE(0x1B, 0x1A),
	BL = NATIVE_ENDIAN_VALUE_LE_BE(0x18, 0x19),
	BH = NATIVE_ENDIAN_VALUE_LE_BE(0x19, 0x18)
};

#define SetRB(x)        do { m_RBW = (x) << 4; m_RBB = (x) << 5; } while (0)

#define Sreg(x)         m_ram.w[m_RBW + (x)]
#define Wreg(x)         m_ram.w[m_RBW + (x)]
#define Breg(x)         m_ram.b[m_RBB + (x)]

#define PC()       ((Sreg(PS)<<4)+m_ip)

#define CF      (m_CarryVal!=0)
#define SF      (m_SignVal<0)
#define ZF      (m_ZeroVal==0)
#define PF      parity_table[(BYTE)m_ParityVal]
#define AF      (m_AuxVal!=0)
#define OF      (m_OverVal!=0)
#define RB      (m_RBW >> 4)

/************************************************************************/

#define read_mem_byte(a)            v25_read_byte((a))
#define read_mem_word(a)            v25_read_word((a))
#define write_mem_byte(a,d)         v25_write_byte((a),(d))
#define write_mem_word(a,d)         v25_write_word((a),(d))

#define read_port_byte(a)       m_io->read_byte(a)
#define read_port_word(a)       m_io->read_word_unaligned(a)
#define write_port_byte(a,d)    m_io->write_byte((a),(d))
#define write_port_word(a,d)    m_io->write_word_unaligned((a),(d))

/************************************************************************/

#define CHANGE_PC do { EMPTY_PREFETCH(); } while (0)

#define SegBase(Seg) (Sreg(Seg) << 4)

#define DefaultBase(Seg) ((m_seg_prefix && (Seg==DS0 || Seg==SS)) ? m_prefix_base : Sreg(Seg) << 4)

#define GetMemB(Seg,Off) (read_mem_byte(DefaultBase(Seg) + (Off)))
#define GetMemW(Seg,Off) (read_mem_word(DefaultBase(Seg) + (Off)))

#define PutMemB(Seg,Off,x) { write_mem_byte(DefaultBase(Seg) + (Off), (x)); }
#define PutMemW(Seg,Off,x) { write_mem_word(DefaultBase(Seg) + (Off), (x)); }

/* prefetch timing */

#define FETCH()             fetch()
#define FETCHWORD()         fetchword()
#define EMPTY_PREFETCH()    m_prefetch_reset = 1


#define PUSH(val) { Wreg(SP) -= 2; write_mem_word(((Sreg(SS)<<4)+Wreg(SP)), val); }
#define POP(var) { Wreg(SP) += 2; var = read_mem_word(((Sreg(SS)<<4) + ((Wreg(SP)-2) & 0xffff))); }

#define GetModRM UINT32 ModRM=FETCH()

/* Cycle count macros:
    CLK  - cycle count is the same on all processors
    CLKS - cycle count differs between processors, list all counts
    CLKW - cycle count for word read/write differs for odd/even source/destination address
    CLKM - cycle count for reg/mem instructions
    CLKR - cycle count for reg/mem instructions with different counts for odd/even addresses


    Prefetch & buswait time is not emulated.
    Extra cycles for PUSH'ing or POP'ing registers to odd addresses is not emulated.
*/

#define CLK(all) m_icount-=all
#define CLKS(v20,v30,v33) { const UINT32 ccount=(v20<<16)|(v30<<8)|v33; m_icount-=(ccount>>m_chip_type)&0x7f; }
#define CLKW(v20o,v30o,v33o,v20e,v30e,v33e,addr) { const UINT32 ocount=(v20o<<16)|(v30o<<8)|v33o, ecount=(v20e<<16)|(v30e<<8)|v33e; m_icount-=(addr&1)?((ocount>>m_chip_type)&0x7f):((ecount>>m_chip_type)&0x7f); }
#define CLKM(v20,v30,v33,v20m,v30m,v33m) { const UINT32 ccount=(v20<<16)|(v30<<8)|v33, mcount=(v20m<<16)|(v30m<<8)|v33m; m_icount-=( ModRM >=0xc0 )?((ccount>>m_chip_type)&0x7f):((mcount>>m_chip_type)&0x7f); }
#define CLKR(v20o,v30o,v33o,v20e,v30e,v33e,vall,addr) { const UINT32 ocount=(v20o<<16)|(v30o<<8)|v33o, ecount=(v20e<<16)|(v30e<<8)|v33e; if (ModRM >=0xc0) m_icount-=vall; else m_icount-=(addr&1)?((ocount>>m_chip_type)&0x7f):((ecount>>m_chip_type)&0x7f); }

/************************************************************************/
#define CompressFlags() (WORD)(CF | (m_IBRK << 1) | (PF << 2) | (m_F0 << 3) | (AF << 4) \
				| (m_F1 << 5) | (ZF << 6) | (SF << 7) | (m_TF << 8) | (m_IF << 9) \
				| (m_DF << 10) | (OF << 11) | (RB << 12) | (m_MF << 15))

#define ExpandFlags(f) \
{ \
	m_CarryVal = (f) & 0x0001; \
	m_IBRK = ((f) & 0x0002) == 0x0002; \
	m_ParityVal = !((f) & 0x0004); \
	m_F0 = ((f) & 0x0008) == 0x0008; \
	m_AuxVal = (f) & 0x0010; \
	m_F1 = ((f) & 0x0020) == 0x0020; \
	m_ZeroVal = !((f) & 0x0040); \
	m_SignVal = (f) & 0x0080 ? -1 : 0; \
	m_TF = ((f) & 0x0100) == 0x0100; \
	m_IF = ((f) & 0x0200) == 0x0200; \
	m_DF = ((f) & 0x0400) == 0x0400; \
	m_OverVal = (f) & 0x0800; \
	/* RB only changes on BRKCS/RETRBI/TSKSW, so skip it */ \
	m_MF = ((f) & 0x8000) == 0x8000; \
}
