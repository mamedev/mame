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
};

/* interrupt sources */
enum INTSOURCES
{
	BRK = 0,
	INT_IRQ = 1,
	NMI_IRQ = 2,
};

/* NEC registers */
union necbasicregs
{                   /* eight general registers */
	UINT16 w[8];    /* viewed as 16 bits registers */
	UINT8  b[16];   /* or as 8 bit registers */
};

struct nec_state_t
{
	necbasicregs regs;
	offs_t  fetch_xor;
	UINT16  sregs[4];

	UINT16  ip;

	/* PSW flags */
	INT32   SignVal;
	UINT32  AuxVal, OverVal, ZeroVal, CarryVal, ParityVal;  /* 0 or non-0 valued flags */
	UINT8   TF, IF, DF, MF; /* 0 or 1 valued flags */

	/* interrupt related */
	UINT32  pending_irq;
	UINT32  nmi_state;
	UINT32  irq_state;
	UINT32  poll_state;
	UINT8   no_interrupt;
	UINT8   halted;

	device_irq_acknowledge_callback irq_callback;
	legacy_cpu_device *device;
	address_space *program;
	direct_read_data *direct;
	address_space *io;
	int     icount;

	UINT8   prefetch_size;
	UINT8   prefetch_cycles;
	INT8    prefetch_count;
	UINT8   prefetch_reset;
	UINT32  chip_type;

	UINT32  prefix_base;    /* base address of the latest prefix segment */
	UINT8   seg_prefix;     /* prefix segment indicator */
};

enum SREGS { DS1, PS, SS, DS0 };
enum WREGS { AW, CW, DW, BW, SP, BP, IX, IY };
enum BREGS {
	AL = NATIVE_ENDIAN_VALUE_LE_BE(0x0, 0x1),
	AH = NATIVE_ENDIAN_VALUE_LE_BE(0x1, 0x0),
	CL = NATIVE_ENDIAN_VALUE_LE_BE(0x2, 0x3),
	CH = NATIVE_ENDIAN_VALUE_LE_BE(0x3, 0x2),
	DL = NATIVE_ENDIAN_VALUE_LE_BE(0x4, 0x5),
	DH = NATIVE_ENDIAN_VALUE_LE_BE(0x5, 0x4),
	BL = NATIVE_ENDIAN_VALUE_LE_BE(0x6, 0x7),
	BH = NATIVE_ENDIAN_VALUE_LE_BE(0x7, 0x6),
};

#define Sreg(x)         nec_state->sregs[x]
#define Wreg(x)         nec_state->regs.w[x]
#define Breg(x)         nec_state->regs.b[x]

#define PC(n)       ((Sreg(PS)<<4)+(n)->ip)

#define CF      (nec_state->CarryVal!=0)
#define SF      (nec_state->SignVal<0)
#define ZF      (nec_state->ZeroVal==0)
#define PF      parity_table[(BYTE)nec_state->ParityVal]
#define AF      (nec_state->AuxVal!=0)
#define OF      (nec_state->OverVal!=0)

/************************************************************************/

#define read_mem_byte(a)            nec_state->program->read_byte(a)
#define read_mem_word(a)            nec_state->program->read_word_unaligned(a)
#define write_mem_byte(a,d)         nec_state->program->write_byte((a),(d))
#define write_mem_word(a,d)         nec_state->program->write_word_unaligned((a),(d))

#define read_port_byte(a)       nec_state->io->read_byte(a)
#define read_port_word(a)       nec_state->io->read_word_unaligned(a)
#define write_port_byte(a,d)    nec_state->io->write_byte((a),(d))
#define write_port_word(a,d)    nec_state->io->write_word_unaligned((a),(d))

/************************************************************************/

#define CHANGE_PC do { EMPTY_PREFETCH(); } while (0)

#define SegBase(Seg) (Sreg(Seg) << 4)

#define DefaultBase(Seg) ((nec_state->seg_prefix && (Seg==DS0 || Seg==SS)) ? nec_state->prefix_base : Sreg(Seg) << 4)

#define GetMemB(Seg,Off) (read_mem_byte(DefaultBase(Seg) + (Off)))
#define GetMemW(Seg,Off) (read_mem_word(DefaultBase(Seg) + (Off)))

#define PutMemB(Seg,Off,x) { write_mem_byte(DefaultBase(Seg) + (Off), (x)); }
#define PutMemW(Seg,Off,x) { write_mem_word(DefaultBase(Seg) + (Off), (x)); }

/* prefetch timing */

#define FETCH()             fetch(nec_state)
#define FETCHWORD()         fetchword(nec_state)
#define EMPTY_PREFETCH()    nec_state->prefetch_reset = 1


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

#define CLK(all) nec_state->icount-=all
#define CLKS(v20,v30,v33) { const UINT32 ccount=(v20<<16)|(v30<<8)|v33; nec_state->icount-=(ccount>>nec_state->chip_type)&0x7f; }
#define CLKW(v20o,v30o,v33o,v20e,v30e,v33e,addr) { const UINT32 ocount=(v20o<<16)|(v30o<<8)|v33o, ecount=(v20e<<16)|(v30e<<8)|v33e; nec_state->icount-=(addr&1)?((ocount>>nec_state->chip_type)&0x7f):((ecount>>nec_state->chip_type)&0x7f); }
#define CLKM(v20,v30,v33,v20m,v30m,v33m) { const UINT32 ccount=(v20<<16)|(v30<<8)|v33, mcount=(v20m<<16)|(v30m<<8)|v33m; nec_state->icount-=( ModRM >=0xc0 )?((ccount>>nec_state->chip_type)&0x7f):((mcount>>nec_state->chip_type)&0x7f); }
#define CLKR(v20o,v30o,v33o,v20e,v30e,v33e,vall,addr) { const UINT32 ocount=(v20o<<16)|(v30o<<8)|v33o, ecount=(v20e<<16)|(v30e<<8)|v33e; if (ModRM >=0xc0) nec_state->icount-=vall; else nec_state->icount-=(addr&1)?((ocount>>nec_state->chip_type)&0x7f):((ecount>>nec_state->chip_type)&0x7f); }

/************************************************************************/
#define CompressFlags() (WORD)(int(CF) | 0x02 | (int(PF) << 2) | (int(AF) << 4) | (int(ZF) << 6) \
				| (int(SF) << 7) | (nec_state->TF << 8) | (nec_state->IF << 9) \
				| (nec_state->DF << 10) | (int(OF) << 11) | 0x7000 | (nec_state->MF << 15))

#define ExpandFlags(f) \
{ \
	nec_state->CarryVal = (f) & 0x0001; \
	nec_state->ParityVal = !((f) & 0x0004); \
	nec_state->AuxVal = (f) & 0x0010; \
	nec_state->ZeroVal = !((f) & 0x0040); \
	nec_state->SignVal = (f) & 0x0080 ? -1 : 0; \
	nec_state->TF = ((f) & 0x0100) == 0x0100; \
	nec_state->IF = ((f) & 0x0200) == 0x0200; \
	nec_state->DF = ((f) & 0x0400) == 0x0400; \
	nec_state->OverVal = (f) & 0x0800; \
	nec_state->MF = ((f) & 0x8000) == 0x8000; \
}
