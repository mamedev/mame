// license:BSD-3-Clause
// copyright-holders:Bryan McPhail

namespace {

/* Cpu types, steps of 8 to help the cycle count calculation */
#define V33_TYPE 0
#define V30_TYPE 8
#define V20_TYPE 16

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

#define read_mem_byte(a)            m_program->read_byte(m_chip_type == V33_TYPE ? v33_translate(a) : (a))
#define read_mem_word(a)            m_program->read_word_unaligned(m_chip_type == V33_TYPE ? v33_translate(a) : (a))
#define write_mem_byte(a,d)         m_program->write_byte(m_chip_type == V33_TYPE ? v33_translate(a) : (a), (d))
#define write_mem_word(a,d)         m_program->write_word_unaligned(m_chip_type == V33_TYPE ? v33_translate(a) : (a), (d))

#define read_port_byte(a)       io_read_byte(a)
#define read_port_word(a)       io_read_word(a)
#define write_port_byte(a,d)    io_write_byte((a),(d))
#define write_port_word(a,d)    io_write_word((a),(d))

/************************************************************************/

#define CHANGE_PC do { EMPTY_PREfetch(); } while (0)

#define SegBase(Seg) (Sreg(Seg) << 4)

#define DefaultBase(Seg) ((m_seg_prefix && (Seg==DS0 || Seg==SS)) ? m_prefix_base : Sreg(Seg) << 4)

#define GetMemB(Seg,Off) (read_mem_byte(DefaultBase(Seg) + (Off)))
#define GetMemW(Seg,Off) (read_mem_word(DefaultBase(Seg) + (Off)))

#define PutMemB(Seg,Off,x) { write_mem_byte(DefaultBase(Seg) + (Off), (x)); }
#define PutMemW(Seg,Off,x) { write_mem_word(DefaultBase(Seg) + (Off), (x)); }

/* prefetch timing */

#define EMPTY_PREfetch()    m_prefetch_reset = 1


#define PUSH(val) { Wreg(SP) -= 2; write_mem_word(((Sreg(SS)<<4)+Wreg(SP)), val); }
#define POP(var) { Wreg(SP) += 2; var = read_mem_word(((Sreg(SS)<<4) + ((Wreg(SP)-2) & 0xffff))); }

#define PUSH80(val) { Wreg(BP) -= 2; write_mem_word(((Sreg(DS0)<<4)+Wreg(BP)), val); }
#define POP80(var) { Wreg(BP) += 2; var = read_mem_word(((Sreg(DS0)<<4) + ((Wreg(BP)-2) & 0xffff))); }

#define BRKXA(xa) { if (m_chip_type == V33_TYPE) { nec_brk(fetch()); m_xa = xa; } else logerror("%06x: %sXA instruction is V33 exclusive\n", PC(), xa ? "BRK" : "RET"); }
#define BRKEM { if (m_chip_type == V33_TYPE) logerror("%06x: BRKEM not supported on V33\n",PC()); else nec_brk(fetch()); }

#define GetModRM uint32_t ModRM=fetch()

/* Cycle count macros:
    CLK  - cycle count is the same on all processors
    CLKS - cycle count differs between processors, list all counts
    CLKW - cycle count for word read/write differs for odd/even source/destination address
    CLKM - cycle count for reg/mem instructions
    CLKR - cycle count for reg/mem instructions with different counts for odd/even addresses

    Prefetch & buswait time is not emulated.
    Extra cycles for PUSH'ing or POP'ing registers to odd addresses is not emulated.
*/

#define CLK(all) do { int cc = all; m_cur_cycles+=cc; m_icount-=cc; } while(0)
#define CLKS(v20,v30,v33) do { const uint32_t ccount=(v20<<16)|(v30<<8)|v33; CLK((ccount>>m_chip_type)&0x7f); } while(0)
#define CLKW(v20o,v30o,v33o,v20e,v30e,v33e,addr) do { const uint32_t ocount=(v20o<<16)|(v30o<<8)|v33o, ecount=(v20e<<16)|(v30e<<8)|v33e; CLK((addr&1)?((ocount>>m_chip_type)&0x7f):((ecount>>m_chip_type)&0x7f)); } while(0)
#define CLKM(v20,v30,v33,v20m,v30m,v33m) do { const uint32_t ccount=(v20<<16)|(v30<<8)|v33, mcount=(v20m<<16)|(v30m<<8)|v33m; CLK(( ModRM >=0xc0 )?((ccount>>m_chip_type)&0x7f):((mcount>>m_chip_type)&0x7f)); } while(0)
#define CLKR(v20o,v30o,v33o,v20e,v30e,v33e,vall,addr) do { const uint32_t ocount=(v20o<<16)|(v30o<<8)|v33o, ecount=(v20e<<16)|(v30e<<8)|v33e; if (ModRM >=0xc0) CLK(vall); else CLK((addr&1)?((ocount>>m_chip_type)&0x7f):((ecount>>m_chip_type)&0x7f)); } while(0)

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
	m_MF = (((f) & 0x8000) == 0x8000) || m_em; \
}

} // anonymous namespace
