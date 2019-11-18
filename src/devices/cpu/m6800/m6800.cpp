// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*** m6800: Portable 6800 class  emulator *************************************

    m68xx.c

    References:

        6809 Simulator V09, By L.C. Benschop, Eindhoven The Netherlands.

        m6809: Portable 6809 emulator, DS (6809 code in MAME, derived from
            the 6809 Simulator V09)

        6809 Microcomputer Programming & Interfacing with Experiments"
            by Andrew C. Staugaard, Jr.; Howard W. Sams & Co., Inc.

    System dependencies:    uint16_t must be 16 bit unsigned int
                            uint8_t must be 8 bit unsigned int
                            uint32_t must be more than 16 bits
                            arrays up to 65536 bytes must be supported
                            machine must be twos complement

History
991031  ZV
    Added NSC-8105 support

990319  HJB
    Fixed wrong LSB/MSB order for push/pull word.
    Subtract .extra_cycles at the beginning/end of the exectution loops.

990316  HJB
    Renamed to 6800, since that's the basic CPU.
    Added different cycle count tables for M6800/2/8, M6801/3 and m68xx.

990314  HJB
    Also added the M6800 subtype.

990311  HJB
    Added _info functions. Now uses static m6808_Regs struct instead
    of single statics. Changed the 16 bit registers to use the generic
    PAIR union. Registers defined using macros. Split the core into
    four execution loops for M6802, M6803, M6808 and HD63701.
    TST, TSTA and TSTB opcodes reset carry flag.
TODO:
    Verify invalid opcodes for the different CPU types.
    Add proper credits to _info functions.
    Integrate m6808_Flags into the registers (multiple m6808 type CPUs?)

990301  HJB
    Modified the interrupt handling. No more pending interrupt checks.
    WAI opcode saves state, when an interrupt is taken (IRQ or OCI),
    the state is only saved if not already done by WAI.

*****************************************************************************/

/*

    Chip                RAM     NVRAM   ROM     SCI     r15-f   ports
    -----------------------------------------------------------------
    MC6800              -       -       -       no      no      -
    MC6802              128     32      -       no      no      -
    MC6802NS            128     -       -       no      no      -
    MC6808              -       -       -       no      no      -

    MC6801              128     64      2K      yes     no      4
    MC68701             128     64      2K      yes     no      4
    MC6803              128     64      -       yes     no      4
    MC6803NR            -       -       -       yes     no      4

    MC6801U4            192     32      4K      yes     yes     4
    MC6803U4            192     32      -       yes     yes     4

    MC68120             128(DP) -       2K      yes     IPC     3
    MC68121             128(DP) -       -       yes     IPC     3

    HD6801              128     64      2K      yes     no      4
    HD6301V             128     -       4K      yes     no      4
    HD63701V            192     -       4K      yes     no      4
    HD6303R             128     -       -       yes     no      4

    HD6301X             192     -       4K      yes     yes     6
    HD6301Y             256     -       16K     yes     yes     6
    HD6303X             192     -       -       yes     yes     6
    HD6303Y             256     -       -       yes     yes     6

    NSC8105
    MS2010-A

*/

#include "emu.h"
#include "debugger.h"
#include "m6800.h"

#define VERBOSE 0

#define LOG(x)  do { if (VERBOSE) logerror x; } while (0)


#define pPPC    m_ppc
#define pPC     m_pc
#define pS      m_s
#define pX      m_x
#define pD      m_d

#define PC      m_pc.w.l
#define PCD     m_pc.d
#define S       m_s.w.l
#define SD      m_s.d
#define X       m_x.w.l
#define D       m_d.w.l
#define A       m_d.b.h
#define B       m_d.b.l
#define CC      m_cc

#define EAD     m_ea.d
#define EA      m_ea.w.l

/* memory interface */

/****************************************************************************/
/* Read a byte from given memory location                                   */
/****************************************************************************/
#define RM(Addr) ((unsigned)m_program->read_byte(Addr))

/****************************************************************************/
/* Write a byte to given memory location                                    */
/****************************************************************************/
#define WM(Addr,Value) (m_program->write_byte(Addr,Value))

/****************************************************************************/
/* M6800_RDOP() is identical to M6800_RDMEM() except it is used for reading */
/* opcodes. In case of system with memory mapped I/O, this function can be  */
/* used to greatly speed up emulation                                       */
/****************************************************************************/
#define M_RDOP(Addr) ((unsigned)m_opcodes_cache->read_byte(Addr))

/****************************************************************************/
/* M6800_RDOP_ARG() is identical to M6800_RDOP() but it's used for reading  */
/* opcode arguments. This difference can be used to support systems that    */
/* use different encoding mechanisms for opcodes and opcode arguments       */
/****************************************************************************/
#define M_RDOP_ARG(Addr) ((unsigned)m_cache->read_byte(Addr))

/* macros to access memory */
#define IMMBYTE(b)  b = M_RDOP_ARG(PCD); PC++
#define IMMWORD(w)  w.d = (M_RDOP_ARG(PCD)<<8) | M_RDOP_ARG((PCD+1)&0xffff); PC+=2

#define PUSHBYTE(b) WM(SD,b); --S
#define PUSHWORD(w) WM(SD,w.b.l); --S; WM(SD,w.b.h); --S
#define PULLBYTE(b) S++; b = RM(SD)
#define PULLWORD(w) S++; w.d = RM(SD)<<8; S++; w.d |= RM(SD)

/* operate one instruction for */
#define ONE_MORE_INSN() {       \
	uint8_t ireg;                           \
	pPPC = pPC;                             \
	debugger_instruction_hook(PCD);         \
	ireg=M_RDOP(PCD);                       \
	PC++;                                   \
	(this->*m_insn[ireg])();                \
	increment_counter(m_cycles[ireg]);      \
}

/* CC masks                       HI NZVC
                                7654 3210   */
#define CLR_HNZVC   CC&=0xd0
#define CLR_NZV     CC&=0xf1
#define CLR_HNZC    CC&=0xd2
#define CLR_NZVC    CC&=0xf0
#define CLR_Z       CC&=0xfb
#define CLR_ZC      CC&=0xfa
#define CLR_C       CC&=0xfe

/* macros for CC -- CC bits affected should be reset before calling */
#define SET_Z(a)        if(!(a))SEZ
#define SET_Z8(a)       SET_Z((uint8_t)(a))
#define SET_Z16(a)      SET_Z((uint16_t)(a))
#define SET_N8(a)       CC|=(((a)&0x80)>>4)
#define SET_N16(a)      CC|=(((a)&0x8000)>>12)
#define SET_H(a,b,r)    CC|=((((a)^(b)^(r))&0x10)<<1)
#define SET_C8(a)       CC|=(((a)&0x100)>>8)
#define SET_C16(a)      CC|=(((a)&0x10000)>>16)
#define SET_V8(a,b,r)   CC|=((((a)^(b)^(r)^((r)>>1))&0x80)>>6)
#define SET_V16(a,b,r)  CC|=((((a)^(b)^(r)^((r)>>1))&0x8000)>>14)

const uint8_t m6800_cpu_device::flags8i[256]=     /* increment */
{
0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x0a,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08
};


const uint8_t m6800_cpu_device::flags8d[256]= /* decrement */
{
0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,
0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08
};

#define SET_FLAGS8I(a)      {CC|=flags8i[(a)&0xff];}
#define SET_FLAGS8D(a)      {CC|=flags8d[(a)&0xff];}

/* combos */
#define SET_NZ8(a)          {SET_N8(a);SET_Z8(a);}
#define SET_NZ16(a)         {SET_N16(a);SET_Z16(a);}
#define SET_FLAGS8(a,b,r)   {SET_N8(r);SET_Z8(r);SET_V8(a,b,r);SET_C8(r);}
#define SET_FLAGS16(a,b,r)  {SET_N16(r);SET_Z16(r);SET_V16(a,b,r);SET_C16(r);}

/* for treating an uint8_t as a signed int16_t */
#define SIGNED(b) ((int16_t)(b&0x80?b|0xff00:b))

/* Macros for addressing modes */
#define DIRECT IMMBYTE(EAD)
#define IMM8 EA=PC++
#define IMM16 {EA=PC;PC+=2;}
#define EXTENDED IMMWORD(m_ea)
#define INDEXED {EA=X+(uint8_t)M_RDOP_ARG(PCD);PC++;}

/* macros to set status flags */
#if defined(SEC)
#undef SEC
#endif
#define SEC CC|=0x01
#define CLC CC&=0xfe
#define SEZ CC|=0x04
#define CLZ CC&=0xfb
#define SEN CC|=0x08
#define CLN CC&=0xf7
#define SEV CC|=0x02
#define CLV CC&=0xfd
#define SEH CC|=0x20
#define CLH CC&=0xdf
#define SEI CC|=0x10
#define CLI CC&=~0x10

/* macros for convenience */
#define DIRBYTE(b) {DIRECT;b=RM(EAD);}
#define DIRWORD(w) {DIRECT;w.d=RM16(EAD);}
#define EXTBYTE(b) {EXTENDED;b=RM(EAD);}
#define EXTWORD(w) {EXTENDED;w.d=RM16(EAD);}

#define IDXBYTE(b) {INDEXED;b=RM(EAD);}
#define IDXWORD(w) {INDEXED;w.d=RM16(EAD);}

/* Macros for branch instructions */
#define BRANCH(f) {IMMBYTE(t);if(f){PC+=SIGNED(t);}}
#define NXORV  ((CC&0x08)^((CC&0x02)<<2))
#define NXORC  ((CC&0x08)^((CC&0x01)<<3))

/* Note: don't use 0 cycles here for invalid opcodes so that we don't */
/* hang in an infinite loop if we hit one */
#define XX 5 // invalid opcode unknown cc
const uint8_t m6800_cpu_device::cycles_6800[256] =
{
		/* 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
	/*0*/ XX, 2,XX,XX,XX,XX, 2, 2, 4, 4, 2, 2, 2, 2, 2, 2,
	/*1*/  2, 2,XX,XX,XX,XX, 2, 2,XX, 2,XX, 2,XX,XX,XX,XX,
	/*2*/  4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	/*3*/  4, 4, 4, 4, 4, 4, 4, 4,XX, 5,XX,10,XX,XX, 9,12,
	/*4*/  2,XX,XX, 2, 2,XX, 2, 2, 2, 2, 2,XX, 2, 2,XX, 2,
	/*5*/  2,XX,XX, 2, 2,XX, 2, 2, 2, 2, 2,XX, 2, 2,XX, 2,
	/*6*/  7,XX,XX, 7, 7,XX, 7, 7, 7, 7, 7,XX, 7, 7, 4, 7,
	/*7*/  6,XX,XX, 6, 6,XX, 6, 6, 6, 6, 6,XX, 6, 6, 3, 6,
	/*8*/  2, 2, 2,XX, 2, 2, 2, 3, 2, 2, 2, 2, 3, 8, 3, 4,
	/*9*/  3, 3, 3,XX, 3, 3, 3, 4, 3, 3, 3, 3, 4, 6, 4, 5,
	/*A*/  5, 5, 5,XX, 5, 5, 5, 6, 5, 5, 5, 5, 6, 8, 6, 7,
	/*B*/  4, 4, 4,XX, 4, 4, 4, 5, 4, 4, 4, 4, 5, 9, 5, 6,
	/*C*/  2, 2, 2,XX, 2, 2, 2, 3, 2, 2, 2, 2,XX,XX, 3, 4,
	/*D*/  3, 3, 3,XX, 3, 3, 3, 4, 3, 3, 3, 3,XX,XX, 4, 5,
	/*E*/  5, 5, 5,XX, 5, 5, 5, 6, 5, 5, 5, 5,XX,XX, 6, 7,
	/*F*/  4, 4, 4,XX, 4, 4, 4, 5, 4, 4, 4, 4,XX,XX, 5, 6
};

const uint8_t m6800_cpu_device::cycles_nsc8105[256] =
{
		/* 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
	/*0*/  5,XX, 2,XX,XX, 2,XX, 2, 4, 2, 4, 2, 2, 2, 2, 2,
	/*1*/  2,XX, 2,XX,XX, 2,XX, 2,XX,XX, 2, 2,XX,XX,XX,XX,
	/*2*/  4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	/*3*/  4, 4, 4, 4, 4, 4, 4, 4,XX,XX, 5,10,XX, 9,XX,12,
	/*4*/  2, 2, 2,XX, 2, 2, 2, 3, 2, 2, 2, 2, 3, 3, 8, 4,
	/*5*/  3, 3, 3,XX, 3, 3, 3, 4, 3, 3, 3, 3, 4, 4, 6, 5,
	/*6*/  5, 5, 5,XX, 5, 5, 5, 6, 5, 5, 5, 5, 6, 6, 8, 7,
	/*7*/  4, 4, 4,XX, 4, 4, 4, 5, 4, 4, 4, 4, 5, 5, 9, 6,
	/*8*/  2,XX,XX, 2, 2, 2,XX, 2, 2, 2, 2,XX, 2,XX, 2, 2,
	/*9*/  2,XX,XX, 2, 2, 2,XX, 2, 2, 2, 2,XX, 2,XX, 2, 2,
	/*A*/  7,XX,XX, 7, 7, 7,XX, 7, 7, 7, 7,XX, 7, 4, 7, 7,
	/*B*/  6,XX,XX, 6, 6, 6,XX, 6, 6, 6, 6, 5, 6, 3, 6, 6,
	/*C*/  2, 2, 2,XX, 2, 2, 2, 3, 2, 2, 2, 2,XX, 3,XX, 4,
	/*D*/  3, 3, 3,XX, 3, 3, 3, 4, 3, 3, 3, 3,XX, 4,XX, 5,
	/*E*/  5, 5, 5,XX, 5, 5, 5, 6, 5, 5, 5, 5, 5, 6,XX, 7,
	/*F*/  4, 4, 4,XX, 4, 4, 4, 5, 4, 4, 4, 4, 4, 5,XX, 6
};
#undef XX // /invalid opcode unknown cc


DEFINE_DEVICE_TYPE(M6800, m6800_cpu_device, "m6800", "Motorola MC6800")
DEFINE_DEVICE_TYPE(M6802, m6802_cpu_device, "m6802", "Motorola MC6802")
DEFINE_DEVICE_TYPE(M6808, m6808_cpu_device, "m6808", "Motorola MC6808")
DEFINE_DEVICE_TYPE(NSC8105, nsc8105_cpu_device, "nsc8105", "NSC8105")


m6800_cpu_device::m6800_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: m6800_cpu_device(mconfig, M6800, tag, owner, clock, m6800_insn, cycles_6800, address_map_constructor())
{
}

m6800_cpu_device::m6800_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, const op_func *insn, const uint8_t *cycles, address_map_constructor internal)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_BIG, 8, 16, 0, internal)
	, m_decrypted_opcodes_config("program", ENDIANNESS_BIG, 8, 16, 0)
	, m_insn(insn)
	, m_cycles(cycles)
{
}

m6802_cpu_device::m6802_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: m6802_cpu_device(mconfig, M6802, tag, owner, clock, m6800_insn, cycles_6800)
{
}

m6802_cpu_device::m6802_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, const op_func *insn, const uint8_t *cycles)
	: m6800_cpu_device(mconfig, type, tag, owner, clock, insn, cycles, address_map_constructor())
{
}

m6808_cpu_device::m6808_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: m6802_cpu_device(mconfig, M6808, tag, owner, clock, m6800_insn, cycles_6800)
{
}

nsc8105_cpu_device::nsc8105_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: m6802_cpu_device(mconfig, NSC8105, tag, owner, clock, nsc8105_insn, cycles_nsc8105)
{
}

device_memory_interface::space_config_vector m6800_cpu_device::memory_space_config() const
{
	if(has_configured_map(AS_OPCODES))
		return space_config_vector {
			std::make_pair(AS_PROGRAM, &m_program_config),
			std::make_pair(AS_OPCODES, &m_decrypted_opcodes_config)
		};
	else
		return space_config_vector {
			std::make_pair(AS_PROGRAM, &m_program_config)
		};
}

uint32_t m6800_cpu_device::RM16(uint32_t Addr )
{
	uint32_t result = RM(Addr) << 8;
	return result | RM((Addr+1)&0xffff);
}

void m6800_cpu_device::WM16(uint32_t Addr, PAIR *p )
{
	WM( Addr, p->b.h );
	WM( (Addr+1)&0xffff, p->b.l );
}

/* IRQ enter */
void m6800_cpu_device::enter_interrupt(const char *message,uint16_t irq_vector)
{
	LOG((message));
	if( m_wai_state & (M6800_WAI|M6800_SLP) )
	{
		if( m_wai_state & M6800_WAI )
			m_icount -= 4;
		m_wai_state &= ~(M6800_WAI|M6800_SLP);
	}
	else
	{
		PUSHWORD(pPC);
		PUSHWORD(pX);
		PUSHBYTE(A);
		PUSHBYTE(B);
		PUSHBYTE(CC);
		m_icount -= 12;
	}
	SEI;
	PCD = RM16( irq_vector );
}



/* check the IRQ lines for pending interrupts */
void m6800_cpu_device::CHECK_IRQ_LINES()
{
	// TODO: IS3 interrupt

	if (m_nmi_pending)
	{
		if(m_wai_state & M6800_SLP)
			m_wai_state &= ~M6800_SLP;

		m_nmi_pending = false;
		enter_interrupt("take NMI\n", 0xfffc);
	}
	else
	{
		if( m_irq_state[M6800_IRQ_LINE] != CLEAR_LINE )
		{   /* standard IRQ */
			if(m_wai_state & M6800_SLP)
				m_wai_state &= ~M6800_SLP;

			if( !(CC & 0x10) )
			{
				enter_interrupt("take IRQ1\n", 0xfff8);
				standard_irq_callback(M6800_IRQ_LINE);
			}
		}
		else
			if( !(CC & 0x10) )
				m6800_check_irq2();
	}
}

void m6800_cpu_device::increment_counter(int amount)
{
	m_icount -= amount;
}

void m6800_cpu_device::EAT_CYCLES()
{
	increment_counter(m_icount);
}

/* include the opcode prototypes and function pointer tables */
#include "6800tbl.hxx"

/* include the opcode functions */
#include "6800ops.hxx"


void m6800_cpu_device::device_start()
{
	m_program = &space(AS_PROGRAM);
	m_cache = m_program->cache<0, 0, ENDIANNESS_BIG>();
	m_opcodes = has_space(AS_OPCODES) ? &space(AS_OPCODES) : m_program;
	m_opcodes_cache = m_opcodes->cache<0, 0, ENDIANNESS_BIG>();

	m_pc.d = 0;
	m_s.d = 0;
	m_x.d = 0;
	m_d.d = 0;
	m_cc = 0;
	m_wai_state = 0;
	m_irq_state[0] = m_irq_state[1] = m_irq_state[2] = 0;

	save_item(NAME(m_ppc.w.l));
	save_item(NAME(m_pc.w.l));
	save_item(NAME(m_s.w.l));
	save_item(NAME(m_x.w.l));
	save_item(NAME(m_d.w.l));
	save_item(NAME(m_cc));
	save_item(NAME(m_wai_state));
	save_item(NAME(m_nmi_state));
	save_item(NAME(m_nmi_pending));
	save_item(NAME(m_irq_state));

	state_add( M6800_A,         "A", m_d.b.h).formatstr("%02X");
	state_add( M6800_B,         "B", m_d.b.l).formatstr("%02X");
	state_add( M6800_PC,        "PC", m_pc.w.l).formatstr("%04X");
	state_add( M6800_S,         "S", m_s.w.l).formatstr("%04X");
	state_add( M6800_X,         "X", m_x.w.l).formatstr("%04X");
	state_add( M6800_CC,        "CC", m_cc).formatstr("%02X");
	state_add( M6800_WAI_STATE, "WAI", m_wai_state).formatstr("%01X");

	state_add( STATE_GENPC, "GENPC", m_pc.w.l).noshow();
	state_add( STATE_GENPCBASE, "CURPC", m_pc.w.l).noshow();
	state_add( STATE_GENFLAGS, "GENFLAGS", m_cc).formatstr("%8s").noshow();

	set_icountptr(m_icount);
}

void m6800_cpu_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format("%c%c%c%c%c%c%c%c",
				m_cc & 0x80 ? '?':'.',
				m_cc & 0x40 ? '?':'.',
				m_cc & 0x20 ? 'H':'.',
				m_cc & 0x10 ? 'I':'.',
				m_cc & 0x08 ? 'N':'.',
				m_cc & 0x04 ? 'Z':'.',
				m_cc & 0x02 ? 'V':'.',
				m_cc & 0x01 ? 'C':'.');
			break;
	}
}

void m6800_cpu_device::device_reset()
{
	m_cc = 0xc0;
	SEI;                /* IRQ disabled */
	PCD = RM16( 0xfffe );

	m_wai_state = 0;
	m_nmi_state = 0;
	m_nmi_pending = 0;
	m_irq_state[M6800_IRQ_LINE] = 0;
}


void m6800_cpu_device::execute_set_input(int irqline, int state)
{
	switch (irqline)
	{
	case INPUT_LINE_NMI:
		if (!m_nmi_state && state != CLEAR_LINE)
			m_nmi_pending = true;
		m_nmi_state = state;
		break;

	default:
		LOG(("set_irq_line %d,%d\n", irqline, state));
		m_irq_state[irqline] = state;
		break;
	}
}

/****************************************************************************
 * Execute cycles CPU cycles. Return number of cycles really executed
 ****************************************************************************/
void m6800_cpu_device::execute_run()
{
	uint8_t ireg;

	CHECK_IRQ_LINES(); /* HJB 990417 */

	CLEANUP_COUNTERS();

	do
	{
		if( m_wai_state & (M6800_WAI|M6800_SLP) )
		{
			EAT_CYCLES();
		}
		else
		{
			pPPC = pPC;
			debugger_instruction_hook(PCD);
			ireg=M_RDOP(PCD);
			PC++;
			(this->*m_insn[ireg])();
			increment_counter(m_cycles[ireg]);
		}
	} while( m_icount>0 );
}

std::unique_ptr<util::disasm_interface> m6800_cpu_device::create_disassembler()
{
	return std::make_unique<m680x_disassembler>(6800);
}

std::unique_ptr<util::disasm_interface> m6802_cpu_device::create_disassembler()
{
	return std::make_unique<m680x_disassembler>(6802);
}

std::unique_ptr<util::disasm_interface> m6808_cpu_device::create_disassembler()
{
	return std::make_unique<m680x_disassembler>(6808);
}

std::unique_ptr<util::disasm_interface> nsc8105_cpu_device::create_disassembler()
{
	return std::make_unique<m680x_disassembler>(8105);
}
