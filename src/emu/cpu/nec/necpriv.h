// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
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
	NEC_CHKIND_VECTOR   = 5
};

/* interrupt sources */
enum INTSOURCES
{
	BRK = 0,
	INT_IRQ = 1,
	NMI_IRQ = 2
};


enum SREGS { DS1=0, PS, SS, DS0 };
enum WREGS { AW=0, CW, DW, BW, SP, BP, IX, IY };
enum BREGS {
	AL = NATIVE_ENDIAN_VALUE_LE_BE(0x0, 0x1),
	AH = NATIVE_ENDIAN_VALUE_LE_BE(0x1, 0x0),
	CL = NATIVE_ENDIAN_VALUE_LE_BE(0x2, 0x3),
	CH = NATIVE_ENDIAN_VALUE_LE_BE(0x3, 0x2),
	DL = NATIVE_ENDIAN_VALUE_LE_BE(0x4, 0x5),
	DH = NATIVE_ENDIAN_VALUE_LE_BE(0x5, 0x4),
	BL = NATIVE_ENDIAN_VALUE_LE_BE(0x6, 0x7),
	BH = NATIVE_ENDIAN_VALUE_LE_BE(0x7, 0x6)
};

#define Sreg(x)         m_sregs[x]
#define Wreg(x)         m_regs.w[x]
#define Breg(x)         m_regs.b[x]

#define PC()       ((Sreg(PS)<<4)+m_ip)

#define CF      (m_CarryVal!=0)
#define SF      (m_SignVal<0)
#define ZF      (m_ZeroVal==0)
#define PF      parity_table[(BYTE)m_ParityVal]
#define AF      (m_AuxVal!=0)
#define OF      (m_OverVal!=0)

/************************************************************************/

#define read_mem_byte(a)            m_program->read_byte(a)
#define read_mem_word(a)            m_program->read_word_unaligned(a)
#define write_mem_byte(a,d)         m_program->write_byte((a),(d))
#define write_mem_word(a,d)         m_program->write_word_unaligned((a),(d))

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
#define CompressFlags() (WORD)(int(CF) | 0x02 | (int(PF) << 2) | (int(AF) << 4) | (int(ZF) << 6) \
				| (int(SF) << 7) | (m_TF << 8) | (m_IF << 9) \
				| (m_DF << 10) | (int(OF) << 11) | 0x7000 | (m_MF << 15))

#define ExpandFlags(f) \
{ \
	m_CarryVal = (f) & 0x0001; \
	m_ParityVal = !((f) & 0x0004); \
	m_AuxVal = (f) & 0x0010; \
	m_ZeroVal = !((f) & 0x0040); \
	m_SignVal = (f) & 0x0080 ? -1 : 0; \
	m_TF = ((f) & 0x0100) == 0x0100; \
	m_IF = ((f) & 0x0200) == 0x0200; \
	m_DF = ((f) & 0x0400) == 0x0400; \
	m_OverVal = (f) & 0x0800; \
	m_MF = ((f) & 0x8000) == 0x8000; \
}
