/*****************************************************************************

    h6280.c - Portable HuC6280 emulator

    Copyright Bryan McPhail, mish@tendril.co.uk

    This source code is based (with permission!) on the 6502 emulator by
    Juergen Buchmueller.  It is released as part of the Mame emulator project.
    Let me know if you intend to use this code in any other project.


    NOTICE:

    This code is around 99% complete!  Several things are unimplemented,
    some due to lack of time, some due to lack of documentation, mainly
    due to lack of programs using these features.

    csh, csl opcodes are not supported.

    I am unsure if flag B is set upon execution of rti.

    Cycle counts should be quite accurate.


    Changelog, version 1.02:
        JMP + indirect X (0x7c) opcode fixed.
        SMB + RMB opcodes fixed in disassembler.
        change_pc function calls removed.
        TSB & TRB now set flags properly.
        BIT opcode altered.

    Changelog, version 1.03:
        Swapped IRQ mask for IRQ1 & IRQ2 (thanks Yasuhiro)

    Changelog, version 1.04, 28/9/99-22/10/99:
        Adjusted RTI (thanks Karl)
        TST opcodes fixed in disassembler (missing break statements in a case!).
        TST behaviour fixed.
        SMB/RMB/BBS/BBR fixed in disassembler.

    Changelog, version 1.05, 8/12/99-16/12/99:
        Added CAB's timer implementation (note: irq ack & timer reload are changed).
        Fixed STA IDX.
        Fixed B flag setting on BRK.
        Assumed CSH & CSL to take 2 cycles each.

        Todo:  Performance could be improved by precalculating timer fire position.

    Changelog, version 1.06, 4/5/00 - last opcode bug found?
        JMP indirect was doing a EAL++; instead of EAD++; - Obviously causing
        a corrupt read when L = 0xff!  This fixes Bloody Wolf and Trio The Punch!

    Changelog, version 1.07, 3/9/00:
        Changed timer to be single shot - fixes Crude Buster music in level 1.

    Changelog, version 1.08, 8/11/05: (Charles MacDonald)

        Changed timer implementation, no longer single shot and reading the
        timer registers returns the count only. Fixes the following:
        - Mesopotamia: Music tempo & in-game timer
        - Dragon Saber: DDA effects
        - Magical Chase: Music tempo and speed regulation
        - Cadash: Allows the first level to start
        - Turrican: Allows the game to start

        Changed PLX and PLY to set NZ flags. Fixes:
        - Afterburner: Graphics unpacking
        - Aoi Blink: Collision detection with background

        Fixed the decimal version of ADC/SBC to *not* update the V flag,
        only the binary ones do.

        Fixed B flag handling so it is always set outside of an interrupt;
        even after being set by PLP and RTI.

        Fixed P state after reset to set I and B, leaving T, D cleared and
        NVZC randomized (cleared in this case).

        Fixed interrupt processing order (Timer has highest priority followed
        by IRQ1 and finally IRQ2).

    Changelog, version 1.09, 1/07/06: (Rob Bohms)

        Added emulation of the T flag, fixes PCE Ankuku Densetsu title screen

    Changelog, version 1.10, 5/09/07: (Wilbert Pol)

        - Taking of interrupts is delayed to respect a pending instruction already
          in the instruction pipeline; fixes After Burner.
        - Added 1 cycle for decimal mode ADC and SBC instructions.
        - Changed cycle counts for CSH and CSL instructions to 3.
        - Added T flag support to the SBC instruction.
        - Fixed ADC T flag to set the Z flag based on the value read.
        - Added 3 cycle penalty to ADC, AND, EOR, ORA, and SBC instructions
          when the T flag is set.
        - Fixed cycle count and support for 65536 byte blocks for the TAI, TDD,
          TIA, TII, and TIN instructions.
        - Fixed RDWORD macro in the disassembler.
        - Fixed setting of N and V flags in the TST instructions.
        - Removed unneeded debug_mmr code.
        - Fixed TSB and TRB instructions.
        - Added 1 delay when accessing the VDC or VCE areas.
        - Implemented low and high speed cpu modes.

    Changelog, version 1.11, 18/09/07: (Wilbert Pol)

        - Improvements to the handling of taking of delayed interrupts.

******************************************************************************/

#include "h6280.h"
#include "debugger.h"

#undef BIT

/* 6280 flags */
enum
{
	_fC = 0x01,
	_fZ = 0x02,
	_fI = 0x04,
	_fD = 0x08,
	_fB = 0x10,
	_fT = 0x20,
	_fV = 0x40,
	_fN = 0x80
};

/* some shortcuts for improved readability */
#define A   m_a
#define X   m_x
#define Y   m_y
#define P   m_p
#define S   m_sp.b.l

#define EAL m_ea.b.l
#define EAH m_ea.b.h
#define EAW m_ea.w.l
#define EAD m_ea.d

#define ZPL m_zp.b.l
#define ZPH m_zp.b.h
#define ZPW m_zp.w.l
#define ZPD m_zp.d

#define PCL m_pc.b.l
#define PCH m_pc.b.h
#define PCW m_pc.w.l
#define PCD m_pc.d

//**************************************************************************
//  DEVICE INTERFACE
//**************************************************************************

const device_type H6280 = &device_creator<h6280_device>;

//-------------------------------------------------
//  h6280_device - constructor
//-------------------------------------------------

h6280_device::h6280_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, H6280, "H6280", tag, owner, clock, "h6280", __FILE__),
	m_program_config("program", ENDIANNESS_LITTLE, 8, 21),
	m_io_config("io", ENDIANNESS_LITTLE, 8, 2)
{
	// build the opcode table
	for (int op = 0; op < 256; op++)
		m_opcode[op] = s_opcodetable[op];
}


const h6280_device::ophandler h6280_device::s_opcodetable[256] =
{
	&h6280_device::op_00, &h6280_device::op_01, &h6280_device::op_02, &h6280_device::op_03, &h6280_device::op_04, &h6280_device::op_05, &h6280_device::op_06, &h6280_device::op_07,
	&h6280_device::op_08, &h6280_device::op_09, &h6280_device::op_0a, &h6280_device::op_0b, &h6280_device::op_0c, &h6280_device::op_0d, &h6280_device::op_0e, &h6280_device::op_0f,
	&h6280_device::op_10, &h6280_device::op_11, &h6280_device::op_12, &h6280_device::op_13, &h6280_device::op_14, &h6280_device::op_15, &h6280_device::op_16, &h6280_device::op_17,
	&h6280_device::op_18, &h6280_device::op_19, &h6280_device::op_1a, &h6280_device::op_1b, &h6280_device::op_1c, &h6280_device::op_1d, &h6280_device::op_1e, &h6280_device::op_1f,
	&h6280_device::op_20, &h6280_device::op_21, &h6280_device::op_22, &h6280_device::op_23, &h6280_device::op_24, &h6280_device::op_25, &h6280_device::op_26, &h6280_device::op_27,
	&h6280_device::op_28, &h6280_device::op_29, &h6280_device::op_2a, &h6280_device::op_2b, &h6280_device::op_2c, &h6280_device::op_2d, &h6280_device::op_2e, &h6280_device::op_2f,
	&h6280_device::op_30, &h6280_device::op_31, &h6280_device::op_32, &h6280_device::op_33, &h6280_device::op_34, &h6280_device::op_35, &h6280_device::op_36, &h6280_device::op_37,
	&h6280_device::op_38, &h6280_device::op_39, &h6280_device::op_3a, &h6280_device::op_3b, &h6280_device::op_3c, &h6280_device::op_3d, &h6280_device::op_3e, &h6280_device::op_3f,
	&h6280_device::op_40, &h6280_device::op_41, &h6280_device::op_42, &h6280_device::op_43, &h6280_device::op_44, &h6280_device::op_45, &h6280_device::op_46, &h6280_device::op_47,
	&h6280_device::op_48, &h6280_device::op_49, &h6280_device::op_4a, &h6280_device::op_4b, &h6280_device::op_4c, &h6280_device::op_4d, &h6280_device::op_4e, &h6280_device::op_4f,
	&h6280_device::op_50, &h6280_device::op_51, &h6280_device::op_52, &h6280_device::op_53, &h6280_device::op_54, &h6280_device::op_55, &h6280_device::op_56, &h6280_device::op_57,
	&h6280_device::op_58, &h6280_device::op_59, &h6280_device::op_5a, &h6280_device::op_5b, &h6280_device::op_5c, &h6280_device::op_5d, &h6280_device::op_5e, &h6280_device::op_5f,
	&h6280_device::op_60, &h6280_device::op_61, &h6280_device::op_62, &h6280_device::op_63, &h6280_device::op_64, &h6280_device::op_65, &h6280_device::op_66, &h6280_device::op_67,
	&h6280_device::op_68, &h6280_device::op_69, &h6280_device::op_6a, &h6280_device::op_6b, &h6280_device::op_6c, &h6280_device::op_6d, &h6280_device::op_6e, &h6280_device::op_6f,
	&h6280_device::op_70, &h6280_device::op_71, &h6280_device::op_72, &h6280_device::op_73, &h6280_device::op_74, &h6280_device::op_75, &h6280_device::op_76, &h6280_device::op_77,
	&h6280_device::op_78, &h6280_device::op_79, &h6280_device::op_7a, &h6280_device::op_7b, &h6280_device::op_7c, &h6280_device::op_7d, &h6280_device::op_7e, &h6280_device::op_7f,
	&h6280_device::op_80, &h6280_device::op_81, &h6280_device::op_82, &h6280_device::op_83, &h6280_device::op_84, &h6280_device::op_85, &h6280_device::op_86, &h6280_device::op_87,
	&h6280_device::op_88, &h6280_device::op_89, &h6280_device::op_8a, &h6280_device::op_8b, &h6280_device::op_8c, &h6280_device::op_8d, &h6280_device::op_8e, &h6280_device::op_8f,
	&h6280_device::op_90, &h6280_device::op_91, &h6280_device::op_92, &h6280_device::op_93, &h6280_device::op_94, &h6280_device::op_95, &h6280_device::op_96, &h6280_device::op_97,
	&h6280_device::op_98, &h6280_device::op_99, &h6280_device::op_9a, &h6280_device::op_9b, &h6280_device::op_9c, &h6280_device::op_9d, &h6280_device::op_9e, &h6280_device::op_9f,
	&h6280_device::op_a0, &h6280_device::op_a1, &h6280_device::op_a2, &h6280_device::op_a3, &h6280_device::op_a4, &h6280_device::op_a5, &h6280_device::op_a6, &h6280_device::op_a7,
	&h6280_device::op_a8, &h6280_device::op_a9, &h6280_device::op_aa, &h6280_device::op_ab, &h6280_device::op_ac, &h6280_device::op_ad, &h6280_device::op_ae, &h6280_device::op_af,
	&h6280_device::op_b0, &h6280_device::op_b1, &h6280_device::op_b2, &h6280_device::op_b3, &h6280_device::op_b4, &h6280_device::op_b5, &h6280_device::op_b6, &h6280_device::op_b7,
	&h6280_device::op_b8, &h6280_device::op_b9, &h6280_device::op_ba, &h6280_device::op_bb, &h6280_device::op_bc, &h6280_device::op_bd, &h6280_device::op_be, &h6280_device::op_bf,
	&h6280_device::op_c0, &h6280_device::op_c1, &h6280_device::op_c2, &h6280_device::op_c3, &h6280_device::op_c4, &h6280_device::op_c5, &h6280_device::op_c6, &h6280_device::op_c7,
	&h6280_device::op_c8, &h6280_device::op_c9, &h6280_device::op_ca, &h6280_device::op_cb, &h6280_device::op_cc, &h6280_device::op_cd, &h6280_device::op_ce, &h6280_device::op_cf,
	&h6280_device::op_d0, &h6280_device::op_d1, &h6280_device::op_d2, &h6280_device::op_d3, &h6280_device::op_d4, &h6280_device::op_d5, &h6280_device::op_d6, &h6280_device::op_d7,
	&h6280_device::op_d8, &h6280_device::op_d9, &h6280_device::op_da, &h6280_device::op_db, &h6280_device::op_dc, &h6280_device::op_dd, &h6280_device::op_de, &h6280_device::op_df,
	&h6280_device::op_e0, &h6280_device::op_e1, &h6280_device::op_e2, &h6280_device::op_e3, &h6280_device::op_e4, &h6280_device::op_e5, &h6280_device::op_e6, &h6280_device::op_e7,
	&h6280_device::op_e8, &h6280_device::op_e9, &h6280_device::op_ea, &h6280_device::op_eb, &h6280_device::op_ec, &h6280_device::op_ed, &h6280_device::op_ee, &h6280_device::op_ef,
	&h6280_device::op_f0, &h6280_device::op_f1, &h6280_device::op_f2, &h6280_device::op_f3, &h6280_device::op_f4, &h6280_device::op_f5, &h6280_device::op_f6, &h6280_device::op_f7,
	&h6280_device::op_f8, &h6280_device::op_f9, &h6280_device::op_fa, &h6280_device::op_fb, &h6280_device::op_fc, &h6280_device::op_fd, &h6280_device::op_fe, &h6280_device::op_ff
};

void h6280_device::device_start()
{
	// register our state for the debugger
	state_add(STATE_GENPC,      "GENPC",        m_pc.w.l).noshow();
	state_add(STATE_GENFLAGS,   "GENFLAGS",     m_p).callimport().callexport().formatstr("%8s").noshow();
	state_add(H6280_PC,         "PC",           m_pc.d).mask(0xffff);
	state_add(H6280_S,          "S",            m_sp.b.l).mask(0xff);
	state_add(H6280_P,          "P",            m_p).mask(0xff);
	state_add(H6280_A,          "A",            m_a).mask(0xff);
	state_add(H6280_X,          "X",            m_x).mask(0xff);
	state_add(H6280_Y,          "Y",            m_y).mask(0xff);
	state_add(H6280_IRQ_MASK,   "IM",           m_irq_mask).mask(0xff);
	state_add(H6280_TIMER_STATE,"TMR",          m_timer_status).mask(0xff);
	state_add(H6280_NMI_STATE,  "NMI",          m_nmi_state).mask(0xf);
	state_add(H6280_IRQ1_STATE, "IRQ1",        m_irq_state[0]).mask(0xf);
	state_add(H6280_IRQ2_STATE, "IRQ2",        m_irq_state[1]).mask(0xf);
	state_add(H6280_IRQT_STATE, "IRQT",        m_irq_state[2]).mask(0xf);
	state_add(H6280_M1,         "M1",           m_mmr[0]).mask(0xff);
	state_add(H6280_M2,         "M2",           m_mmr[1]).mask(0xff);
	state_add(H6280_M3,         "M3",           m_mmr[2]).mask(0xff);
	state_add(H6280_M4,         "M4",           m_mmr[3]).mask(0xff);
	state_add(H6280_M5,         "M5",           m_mmr[4]).mask(0xff);
	state_add(H6280_M6,         "M6",           m_mmr[5]).mask(0xff);
	state_add(H6280_M7,         "M7",           m_mmr[6]).mask(0xff);
	state_add(H6280_M8,         "M8",           m_mmr[7]).mask(0xff);

	save_item(NAME(m_ppc.w.l));
	save_item(NAME(m_pc.w.l));
	save_item(NAME(m_sp.w.l));
	save_item(NAME(m_zp.w.l));
	save_item(NAME(m_ea.w.l));
	save_item(NAME(m_a));
	save_item(NAME(m_x));
	save_item(NAME(m_y));
	save_item(NAME(m_p));
	save_item(NAME(m_mmr));
	save_item(NAME(m_irq_mask));
	save_item(NAME(m_timer_status));
	save_item(NAME(m_timer_ack));
	save_item(NAME(m_clocks_per_cycle));
	save_item(NAME(m_timer_value));
	save_item(NAME(m_timer_load));
	save_item(NAME(m_nmi_state));
	save_item(NAME(m_irq_state[0]));
	save_item(NAME(m_irq_state[1]));
	save_item(NAME(m_irq_state[2]));
	save_item(NAME(m_irq_pending));

#if LAZY_FLAGS
	save_item(NAME(m_nz));
#endif
	save_item(NAME(m_io_buffer));

	// set our instruction counter
	m_icountptr = &m_icount;
	m_icount = 0;

	/* clear pending interrupts */
	for (int i = 0; i < 3; i++)
	{
		m_irq_state[i] = CLEAR_LINE;
	}
	m_nmi_state = CLEAR_LINE;
}

void h6280_device::device_reset()
{
	/* wipe out the h6280 structure */
	m_ppc.d = 0;
	m_pc.d = 0;
	m_zp.d = 0;
	m_ea.d = 0;
	m_a = 0;
	m_x = 0;
	m_y = 0;
	m_p = 0;
	memset(m_mmr, 0, sizeof(UINT8) * 8);
	m_irq_mask = 0;
	m_timer_ack = 0;
	m_timer_value = 0;
#if LAZY_FLAGS
	m_nz = 0;
#endif
	m_io_buffer = 0;

	m_program = &space(AS_PROGRAM);
	m_direct = &m_program->direct();
	m_io = &space(AS_IO);

	/* set I and B flags */
	P = _fI | _fB;

	/* stack starts at 0x01ff */
	m_sp.d = 0x1ff;

	/* read the reset vector into PC */
	PCL = program_read8(H6280_RESET_VEC);
	PCH = program_read8(H6280_RESET_VEC + 1);

	/* CPU starts in low speed mode */
	m_clocks_per_cycle = 4;

	/* timer off by default */
	m_timer_status = 0;
	m_timer_load = 128 * 1024;

	m_irq_pending = 0;
}

void h6280_device::device_stop()
{
	/* nothing */
}


inline UINT32 h6280_device::TRANSLATED(UINT16 addr)
{
	return ((m_mmr[((addr) >> 13) & 7] << 13) | ((addr) & 0x1fff));
}

inline void h6280_device::H6280_CYCLES(int cyc)
{
	m_icount -= ((cyc) * m_clocks_per_cycle);
	m_timer_value -= ((cyc) * m_clocks_per_cycle);
}

#if LAZY_FLAGS

#define NZ  m_NZ
inline void h6280_device::SET_NZ(UINT8 n)
{
	P &= ~_fT;
	NZ = ((n & _fN) << 8) | n;
}

#else

inline void h6280_device::SET_NZ(UINT8 n)
{
	P = (P & ~(_fN|_fT|_fZ)) |
		(n & _fN) |
		((n == 0) ? _fZ : 0);
}

#endif

inline void h6280_device::CLEAR_T()
{
	P &= ~_fT;
}

inline void h6280_device::DO_INTERRUPT(UINT16 vector)
{
	H6280_CYCLES(7);    /* 7 cycles for an int */
	push(PCH);
	push(PCL);
	COMPOSE_P(0, _fB);
	push(P);
	P = (P & ~_fD) | _fI;   /* knock out D and set I flag */
	PCL = program_read8(vector);
	PCH = program_read8(vector + 1);
}

inline void h6280_device::CHECK_AND_TAKE_IRQ_LINES()
{
	if ( m_nmi_state != CLEAR_LINE ) {
		m_nmi_state = CLEAR_LINE;
		DO_INTERRUPT(H6280_NMI_VEC);
	}
	else if( !(P & _fI) )
	{
		if ( m_irq_state[2] != CLEAR_LINE &&
				!(m_irq_mask & 0x4) )
		{
			DO_INTERRUPT(H6280_TIMER_VEC);
		} else
		if ( m_irq_state[0] != CLEAR_LINE &&
				!(m_irq_mask & 0x2) )
		{
			DO_INTERRUPT(H6280_IRQ1_VEC);
			standard_irq_callback(0);
		} else
		if ( m_irq_state[1] != CLEAR_LINE &&
				!(m_irq_mask & 0x1) )
		{
			DO_INTERRUPT(H6280_IRQ2_VEC);
			standard_irq_callback(1);
		}
	}
}

inline void h6280_device::CHECK_IRQ_LINES()
{
	if (!m_irq_pending)
		m_irq_pending = 2;
}

/***************************************************************
 *  CHECK_VDC_VCE_PENALTY
 * The CPU inserts 1 clock delay when accessing the VDC or VCE
 * area.
 ***************************************************************/
inline void h6280_device::CHECK_VDC_VCE_PENALTY(UINT16 addr)
{
	if ( ( TRANSLATED(addr) & 0x1FF800 ) == 0x1FE000 ) {
		H6280_CYCLES(1);
	}
}

/***************************************************************
 *  BRA  branch relative
 ***************************************************************/
inline void h6280_device::BRA(bool cond)
{
	CLEAR_T();
	if (cond)
	{
		H6280_CYCLES(4);
		UINT8 tmp = read_opcode_arg();
		PCW++;
		EAW = PCW + (signed char)tmp;
		PCD = EAD;
	}
	else
	{
		PCW++;
		H6280_CYCLES(2);
	}
}

/***************************************************************
 *
 * Helper macros to build the effective address
 *
 ***************************************************************/

/***************************************************************
 *  EA = zero page address
 ***************************************************************/
inline void h6280_device::EA_ZPG()
{
	ZPL = read_opcode_arg();
	PCW++;
	EAD = ZPD;
}

/***************************************************************
 *  EA = zero page address - T flag
 ***************************************************************/
inline void h6280_device::EA_TFLG()
{
	ZPL = X;
	EAD = ZPD;
}

/***************************************************************
 *  EA = zero page address + X
 ***************************************************************/
inline void h6280_device::EA_ZPX()
{
	ZPL = read_opcode_arg() + X;
	PCW++;
	EAD = ZPD;
}

/***************************************************************
 *  EA = zero page address + Y
 ***************************************************************/
inline void h6280_device::EA_ZPY()
{
	ZPL = read_opcode_arg() + Y;
	PCW++;
	EAD = ZPD;
}

/***************************************************************
 *  EA = absolute address
 ***************************************************************/
inline void h6280_device::EA_ABS()
{
	EAL = read_opcode_arg();
	PCW++;
	EAH = read_opcode_arg();
	PCW++;
}

/***************************************************************
 *  EA = absolute address + X
 ***************************************************************/
inline void h6280_device::EA_ABX()
{
	EA_ABS();
	EAW += X;
}

/***************************************************************
 *  EA = absolute address + Y
 ***************************************************************/
inline void h6280_device::EA_ABY()
{
	EA_ABS();
	EAW += Y;
}

/***************************************************************
 *  EA = zero page indirect (65c02 pre indexed w/o X)
 ***************************************************************/
inline void h6280_device::EA_ZPI()
{
	ZPL = read_opcode_arg();
	PCW++;
	EAD = program_read16z(ZPD);
}

/***************************************************************
 *  EA = zero page + X indirect (pre indexed)
 ***************************************************************/
inline void h6280_device::EA_IDX()
{
	ZPL = read_opcode_arg() + X;
	PCW++;
	EAD = program_read16z(ZPD);
}

/***************************************************************
 *  EA = zero page indirect + Y (post indexed)
 ***************************************************************/
inline void h6280_device::EA_IDY()
{
	ZPL = read_opcode_arg();
	PCW++;
	EAD = program_read16z(ZPD);
	EAW += Y;
}

/***************************************************************
 *  EA = indirect (only used by JMP)
 ***************************************************************/
inline void h6280_device::EA_IND()
{
	EA_ABS();
	UINT8 tmp = program_read8(EAD);
	EAD++;
	EAH = program_read8(EAD);
	EAL = tmp;
}

/***************************************************************
 *  EA = indirect plus x (only used by JMP)
 ***************************************************************/
inline void h6280_device::EA_IAX()
{
	EA_ABS();
	EAD+=X;
	UINT8 tmp = program_read8(EAD);
	EAD++;
	EAH = program_read8(EAD);
	EAL = tmp;
}

inline UINT8 h6280_device::RD_IMM()
{
	UINT8 tmp = read_opcode_arg();
	PCW++;
	return tmp;
}

inline UINT8 h6280_device::RD_ZPG()
{
	EA_ZPG();
	return program_read8z(EAD);
}

inline UINT8 h6280_device::RD_ZPX()
{
	EA_ZPX();
	return program_read8z(EAD);
}

inline UINT8 h6280_device::RD_ZPY()
{
	EA_ZPY();
	return program_read8z(EAD);
}

inline UINT8 h6280_device::RD_ABS()
{
	EA_ABS();
	return program_read8(EAD);
}

inline UINT8 h6280_device::RD_ABX()
{
	EA_ABX();
	return program_read8(EAD);
}

inline UINT8 h6280_device::RD_ABY()
{
	EA_ABY();
	return program_read8(EAD);
}

inline UINT8 h6280_device::RD_ZPI()
{
	EA_ZPI();
	return program_read8(EAD);
}

inline UINT8 h6280_device::RD_IDX()
{
	EA_IDX();
	return program_read8(EAD);
}

inline UINT8 h6280_device::RD_IDY()
{
	EA_IDY();
	return program_read8(EAD);
}

inline UINT8 h6280_device::RD_TFL()
{
	EA_TFLG();
	return program_read8z(EAD);
}

inline void h6280_device::WR_ZPG(UINT8 tmp)
{
	EA_ZPG();
	WB_EAZ(tmp);
}

inline void h6280_device::WR_ZPX(UINT8 tmp)
{
	EA_ZPX();
	WB_EAZ(tmp);
}

inline void h6280_device::WR_ZPY(UINT8 tmp)
{
	EA_ZPY();
	WB_EAZ(tmp);
}

inline void h6280_device::WR_ABS(UINT8 tmp)
{
	EA_ABS();
	WB_EA(tmp);
}

inline void h6280_device::WR_ABX(UINT8 tmp)
{
	EA_ABX();
	WB_EA(tmp);
}

inline void h6280_device::WR_ABY(UINT8 tmp)
{
	EA_ABY();
	WB_EA(tmp);
}

inline void h6280_device::WR_ZPI(UINT8 tmp)
{
	EA_ZPI();
	WB_EA(tmp);
}

inline void h6280_device::WR_IDX(UINT8 tmp)
{
	EA_IDX();
	WB_EA(tmp);
}

inline void h6280_device::WR_IDY(UINT8 tmp)
{
	EA_IDY();
	WB_EA(tmp);
}

inline void h6280_device::WB_EA(UINT8 tmp)
{
	program_write8(EAD, tmp);
}

inline void h6280_device::WB_EAZ(UINT8 tmp)
{
	program_write8z(EAD, tmp);
}

/***************************************************************
 *
 * Macros to emulate the 6280 opcodes
 *
 ***************************************************************/

/***************************************************************
 * compose the real flag register by
 * including N and Z and set any
 * SET and clear any CLR bits also
 ***************************************************************/
#if LAZY_FLAGS

inline void h6280_device::COMPOSE_P(UINT8 SET, UINT8 CLR)
{
	P = (P & ~(_fN | _fZ | CLR)) |
		(NZ >> 8) |
		((NZ & 0xff) ? 0 : _fZ) |
		SET;
}

#else

inline void h6280_device::COMPOSE_P(UINT8 SET, UINT8 CLR)
{
	P = (P & ~CLR) | SET;
}

#endif

/* 6280 ********************************************************
 *  ADC Add with carry
 ***************************************************************/
inline void h6280_device::TADC(UINT8 tmp)
{
	CLEAR_T();
	int tflagtemp = RD_TFL();
	if (P & _fD)
	{
		int c = (P & _fC);
		int lo = (tflagtemp & 0x0f) + (tmp & 0x0f) + c;
		int hi = (tflagtemp & 0xf0) + (tmp & 0xf0);
		P &= ~_fC;
		if (lo > 0x09)
		{
			hi += 0x10;
			lo += 0x06;
		}
		if (hi > 0x90)
			hi += 0x60;
		if (hi & 0xff00)
			P |= _fC;
		tflagtemp = (lo & 0x0f) + (hi & 0xf0);
		H6280_CYCLES(1);
	}
	else
	{
		int c = (P & _fC);
		int sum = tflagtemp + tmp + c;
		P &= ~(_fV | _fC);
		if (~(tflagtemp^tmp) & (tflagtemp^sum) & _fN)
			P |= _fV;
		if (sum & 0xff00)
			P |= _fC;
		tflagtemp = (UINT8) sum;
	}
	SET_NZ(tflagtemp);
	WB_EAZ(tflagtemp);
	H6280_CYCLES(3);
}


inline void h6280_device::ADC(UINT8 tmp)
{
	if(P & _fT)
		TADC(tmp);
	else {
		if (P & _fD)
		{
			int c = (P & _fC);
			int lo = (A & 0x0f) + (tmp & 0x0f) + c;
			int hi = (A & 0xf0) + (tmp & 0xf0);
			P &= ~_fC;
			if (lo > 0x09)
			{
				hi += 0x10;
				lo += 0x06;
			}
			if (hi > 0x90)
				hi += 0x60;
			if (hi & 0xff00)
				P |= _fC;
			A = (lo & 0x0f) + (hi & 0xf0);
			H6280_CYCLES(1);
		}
		else
		{
			int c = (P & _fC);
			int sum = A + tmp + c;
			P &= ~(_fV | _fC);
			if (~(A^tmp) & (A^sum) & _fN)
				P |= _fV;
			if (sum & 0xff00)
				P |= _fC;
			A = (UINT8) sum;
		}
		SET_NZ(A);
	}
}

/* 6280 ********************************************************
 *  AND Logical and
 ***************************************************************/
inline void h6280_device::TAND(UINT8 tmp)
{
	CLEAR_T();
	int tflagtemp = RD_TFL();
	tflagtemp = (UINT8)(tflagtemp & tmp);
	WB_EAZ(tflagtemp);
	SET_NZ(tflagtemp);
	H6280_CYCLES(3);
}

inline void h6280_device::AND(UINT8 tmp)
{
	if(P & _fT)
		TAND(tmp);
	else {
		A = (UINT8)(A & tmp);
		SET_NZ(A);
	}
}

/* 6280 ********************************************************
 *  ASL Arithmetic shift left
 ***************************************************************/
inline UINT8 h6280_device::ASL(UINT8 tmp)
{
	CLEAR_T();
	P = (P & ~_fC) | ((tmp >> 7) & _fC);
	tmp = (UINT8)(tmp << 1);
	SET_NZ(tmp);
	return tmp;
}

/* 6280 ********************************************************
 *  BBR Branch if bit is reset
 ***************************************************************/
inline void h6280_device::BBR(int bit, UINT8 tmp)
{
	BRA(!(tmp & (1<<bit)));
}

/* 6280 ********************************************************
 *  BBS Branch if bit is set
 ***************************************************************/
inline void h6280_device::BBS(int bit, UINT8 tmp)
{
	BRA(tmp & (1<<bit));
}

/* 6280 ********************************************************
 *  BCC Branch if carry clear
 ***************************************************************/
inline void h6280_device::BCC()
{
	BRA(!(P & _fC));
}

/* 6280 ********************************************************
 *  BCS Branch if carry set
 ***************************************************************/
inline void h6280_device::BCS()
{
	BRA(P & _fC);
}

/* 6280 ********************************************************
 *  BEQ Branch if equal
 ***************************************************************/
inline void h6280_device::BEQ()
{
#if LAZY_FLAGS
	BRA(!(NZ & 0xff));
#else
	BRA(P & _fZ);
#endif
}

/* 6280 ********************************************************
 *  BIT Bit test
 ***************************************************************/
inline void h6280_device::BIT(UINT8 tmp)
{
	P = (P & ~(_fN|_fV|_fT|_fZ))
		| ((tmp&0x80) ? _fN:0)
		| ((tmp&0x40) ? _fV:0)
		| ((tmp&A)  ? 0:_fZ);
}

/* 6280 ********************************************************
 *  BMI Branch if minus
 ***************************************************************/
inline void h6280_device::BMI()
{
#if LAZY_FLAGS
	BRA(NZ & 0x8000);
#else
	BRA(P & _fN);
#endif
}

/* 6280 ********************************************************
 *  BNE Branch if not equal
 ***************************************************************/
inline void h6280_device::BNE()
{
#if LAZY_FLAGS
	BRA(NZ & 0xff);
#else
	BRA(!(P & _fZ));
#endif
}

/* 6280 ********************************************************
 *  BPL Branch if plus
 ***************************************************************/
inline void h6280_device::BPL()
{
#if LAZY_FLAGS
	BRA(!(NZ & 0x8000));
#else
	BRA(!(P & _fN));
#endif
}

/* 6280 ********************************************************
 *  BRK Break
 *  increment PC, push PC hi, PC lo, flags (with B bit set),
 *  set I flag, reset D flag and jump via IRQ vector
 ***************************************************************/
inline void h6280_device::BRK()
{
	logerror("BRK %04xn",PCW);
	CLEAR_T();
	PCW++;
	push(PCH);
	push(PCL);
	push(P);
	P = (P & ~_fD) | _fI;
	PCL = program_read8(H6280_IRQ2_VEC);
	PCH = program_read8(H6280_IRQ2_VEC+1);
}

/* 6280 ********************************************************
 *  BSR Branch to subroutine
 ***************************************************************/
inline void h6280_device::BSR()
{
	push(PCH);
	push(PCL);
	H6280_CYCLES(4); /* 4 cycles here, 4 in BRA */
	BRA(1);
}

/* 6280 ********************************************************
 *  BVC Branch if overflow clear
 ***************************************************************/
inline void h6280_device::BVC()
{
	BRA(!(P & _fV));
}

/* 6280 ********************************************************
 *  BVS Branch if overflow set
 ***************************************************************/
inline void h6280_device::BVS()
{
	BRA(P & _fV);
}

/* 6280 ********************************************************
 *  CLA Clear accumulator
 ***************************************************************/
inline void h6280_device::CLA()
{
	CLEAR_T();
	A = 0;
}

/* 6280 ********************************************************
 *  CLC Clear carry flag
 ***************************************************************/
inline void h6280_device::CLC()
{
	CLEAR_T();
	P &= ~_fC;
}

/* 6280 ********************************************************
 *  CLD Clear decimal flag
 ***************************************************************/
inline void h6280_device::CLD()
{
	CLEAR_T();
	P &= ~_fD;
}

/* 6280 ********************************************************
 *  CLI Clear interrupt flag
 ***************************************************************/
inline void h6280_device::CLI()
{
	CLEAR_T();
	if( P & _fI )
	{
		P &= ~_fI;
		CHECK_IRQ_LINES();
	}
}


/* 6280 ********************************************************
 *  CLV Clear overflow flag
 ***************************************************************/
inline void h6280_device::CLV()
{
	CLEAR_T();
	P &= ~_fV;
}

/* 6280 ********************************************************
 *  CLX Clear index X
 ***************************************************************/
inline void h6280_device::CLX()
{
	CLEAR_T();
	X = 0;
}

/* 6280 ********************************************************
 *  CLY Clear index Y
 ***************************************************************/
inline void h6280_device::CLY()
{
	CLEAR_T();
	Y = 0;
}

/* 6280 ********************************************************
 *  CMP Compare accumulator
 ***************************************************************/
inline void h6280_device::CMP(UINT8 tmp)
{
	CLEAR_T();
	P &= ~_fC;
	if (A >= tmp)
		P |= _fC;
	SET_NZ((UINT8)(A - tmp));
}

/* 6280 ********************************************************
 *  CPX Compare index X
 ***************************************************************/
inline void h6280_device::CPX(UINT8 tmp)
{
	CLEAR_T();
	P &= ~_fC;
	if (X >= tmp)
		P |= _fC;
	SET_NZ((UINT8)(X - tmp));
}

/* 6280 ********************************************************
 *  CPY Compare index Y
 ***************************************************************/
inline void h6280_device::CPY(UINT8 tmp)
{
	CLEAR_T();
	P &= ~_fC;
	if (Y >= tmp)
		P |= _fC;
	SET_NZ((UINT8)(Y - tmp));
}

/* 6280 ********************************************************
 *  DEC Decrement memory
 ***************************************************************/
inline UINT8 h6280_device::DEC(UINT8 tmp)
{
	CLEAR_T();
	tmp = (UINT8)(tmp-1);
	SET_NZ(tmp);
	return tmp;
}

/* 6280 ********************************************************
 *  DEX Decrement index X
 ***************************************************************/
inline void h6280_device::DEX()
{
	CLEAR_T();
	X = (UINT8)(X - 1);
	SET_NZ(X);
}

/* 6280 ********************************************************
 *  DEY Decrement index Y
 ***************************************************************/
inline void h6280_device::DEY()
{
	CLEAR_T();
	Y = (UINT8)(Y - 1);
	SET_NZ(Y);
}

/* 6280 ********************************************************
 *  EOR Logical exclusive or
 ***************************************************************/
inline void h6280_device::TEOR(UINT8 tmp)
{
	CLEAR_T();
	int tflagtemp = RD_TFL();
	tflagtemp = (UINT8)(tflagtemp ^ tmp);
	WB_EAZ(tflagtemp);
	SET_NZ(tflagtemp);
	H6280_CYCLES(3);
}

inline void h6280_device::EOR(UINT8 tmp)
{
	if(P & _fT)
		TEOR(tmp);
	else {
		A = (UINT8)(A ^ tmp);
		SET_NZ(A);
	}
}

/* 6280 ********************************************************
 *  INC Increment memory
 ***************************************************************/
inline UINT8 h6280_device::INC(UINT8 tmp)
{
	CLEAR_T();
	tmp = (UINT8)(tmp+1);
	SET_NZ(tmp);
	return tmp;
}

/* 6280 ********************************************************
 *  INX Increment index X
 ***************************************************************/
inline void h6280_device::INX()
{
	CLEAR_T();
	X = (UINT8)(X + 1);
	SET_NZ(X);
}

/* 6280 ********************************************************
 *  INY Increment index Y
 ***************************************************************/
inline void h6280_device::INY()
{
	CLEAR_T();
	Y = (UINT8)(Y + 1);
	SET_NZ(Y);
}

/* 6280 ********************************************************
 *  JMP Jump to address
 *  set PC to the effective address
 ***************************************************************/
inline void h6280_device::JMP()
{
	CLEAR_T();
	PCD = EAD;
}

/* 6280 ********************************************************
 *  JSR Jump to subroutine
 *  decrement PC (sic!) push PC hi, push PC lo and set
 *  PC to the effective address
 ***************************************************************/
inline void h6280_device::JSR()
{
	CLEAR_T();
	PCW--;
	push(PCH);
	push(PCL);
	PCD = EAD;
}

/* 6280 ********************************************************
 *  LDA Load accumulator
 ***************************************************************/
inline void h6280_device::LDA(UINT8 tmp)
{
	CLEAR_T();
	A = (UINT8)tmp;
	SET_NZ(A);
}

/* 6280 ********************************************************
 *  LDX Load index X
 ***************************************************************/
inline void h6280_device::LDX(UINT8 tmp)
{
	CLEAR_T();
	X = (UINT8)tmp;
	SET_NZ(X);
}

/* 6280 ********************************************************
 *  LDY Load index Y
 ***************************************************************/
inline void h6280_device::LDY(UINT8 tmp)
{
	CLEAR_T();
	Y = (UINT8)tmp;
	SET_NZ(Y);
}

/* 6280 ********************************************************
 *  LSR Logic shift right
 *  0 -> [7][6][5][4][3][2][1][0] -> C
 ***************************************************************/
inline UINT8 h6280_device::LSR(UINT8 tmp)
{
	CLEAR_T();
	P = (P & ~_fC) | (tmp & _fC);
	tmp = (UINT8)tmp >> 1;
	SET_NZ(tmp);
	return tmp;
}

/* 6280 ********************************************************
 *  NOP No operation
 ***************************************************************/
inline void h6280_device::NOP()
{
	CLEAR_T();
}

/* 6280 ********************************************************
 *  ORA Logical inclusive or
 ***************************************************************/

inline void h6280_device::TORA(UINT8 tmp)
{
	CLEAR_T();
	int tflagtemp = RD_TFL();
	tflagtemp = (UINT8)(tflagtemp | tmp);
	WB_EAZ(tflagtemp);
	SET_NZ(tflagtemp);
	H6280_CYCLES(3);
}

inline void h6280_device::ORA(UINT8 tmp)
{
	if(P & _fT)
		TORA(tmp);
	else {
		A = (UINT8)(A | tmp);
		SET_NZ(A);
	}
}

/* 6280 ********************************************************
 *  PHA Push accumulator
 ***************************************************************/
inline void h6280_device::PHA()
{
	CLEAR_T();
	push(A);
}

/* 6280 ********************************************************
 *  PHP Push processor status (flags)
 ***************************************************************/
inline void h6280_device::PHP()
{
	CLEAR_T();
	COMPOSE_P(0,0);
	push(P);
}

/* 6280 ********************************************************
 *  PHX Push index X
 ***************************************************************/
inline void h6280_device::PHX()
{
	CLEAR_T();
	push(X);
}

/* 6280 ********************************************************
 *  PHY Push index Y
 ***************************************************************/
inline void h6280_device::PHY()
{
	CLEAR_T();
	push(Y);
}

/* 6280 ********************************************************
 *  PLA Pull accumulator
 ***************************************************************/
inline void h6280_device::PLA()
{
	CLEAR_T();
	pull(A);
	SET_NZ(A);
}

/* 6280 ********************************************************
 *  PLP Pull processor status (flags)
 ***************************************************************/
inline void h6280_device::PLP()
{
#if LAZY_FLAGS
	pull(P);
	P |= _fB;
	NZ = ((P & _fN) << 8) |
			((P & _fZ) ^ _fZ);
	CHECK_IRQ_LINES();
#else
	pull(P);
	P |= _fB;
	CHECK_IRQ_LINES();
#endif
}

/* 6280 ********************************************************
 *  PLX Pull index X
 ***************************************************************/
inline void h6280_device::PLX()
{
	CLEAR_T();
	pull(X);
	SET_NZ(X);
}

/* 6280 ********************************************************
 *  PLY Pull index Y
 ***************************************************************/
inline void h6280_device::PLY()
{
	CLEAR_T();
	pull(Y);
	SET_NZ(Y);
}

/* 6280 ********************************************************
 *  RMB Reset memory bit
 ***************************************************************/
inline UINT8 h6280_device::RMB(int bit, UINT8 tmp)
{
	CLEAR_T();
	tmp &= ~(1<<bit);
	return tmp;
}

/* 6280 ********************************************************
 *  ROL Rotate left
 *  new C <- [7][6][5][4][3][2][1][0] <- C
 ***************************************************************/
inline UINT8 h6280_device::ROL(UINT8 tmp)
{
	CLEAR_T();
	int tmp9 = (tmp << 1) | (P & _fC);
	P = (P & ~_fC) | ((tmp9 >> 8) & _fC);
	tmp = (UINT8)tmp9;
	SET_NZ(tmp);
	return tmp;
}

/* 6280 ********************************************************
 *  ROR Rotate right
 *  C -> [7][6][5][4][3][2][1][0] -> new C
 ***************************************************************/
inline UINT8 h6280_device::ROR(UINT8 tmp)
{
	CLEAR_T();
	int tmp9 = tmp | (P & _fC) << 8;
	P = (P & ~_fC) | (tmp & _fC);
	tmp = (UINT8)(tmp9 >> 1);
	SET_NZ(tmp);
	return tmp;
}

/* 6280 ********************************************************
 *  RTI Return from interrupt
 *  pull flags, pull PC lo, pull PC hi and increment PC
 ***************************************************************/
inline void h6280_device::RTI()
{
#if LAZY_FLAGS
	pull(P);
	P |= _fB;
	NZ = ((P & _fN) << 8) |
			((P & _fZ) ^ _fZ);
	pull(PCL);
	pull(PCH);
	CHECK_IRQ_LINES();
#else
	pull(P);
	P |= _fB;
	pull(PCL);
	pull(PCH);
	CHECK_IRQ_LINES();
#endif
}

/* 6280 ********************************************************
 *  RTS Return from subroutine
 *  pull PC lo, PC hi and increment PC
 ***************************************************************/
inline void h6280_device::RTS()
{
	CLEAR_T();
	pull(PCL);
	pull(PCH);
	PCW++;
}

/* 6280 ********************************************************
 *  SAX Swap accumulator and index X
 ***************************************************************/
inline void h6280_device::SAX()
{
	CLEAR_T();
	UINT8 tmp = X;
	X = A;
	A = tmp;
}

/* 6280 ********************************************************
 *  SAY Swap accumulator and index Y
 ***************************************************************/
inline void h6280_device::SAY()
{
	CLEAR_T();
	UINT8 tmp = Y;
	Y = A;
	A = tmp;
}

/* 6280 ********************************************************
 *  SBC Subtract with carry
 ***************************************************************/
inline void h6280_device::TSBC(UINT8 tmp)
{
	CLEAR_T();
	int tflagtemp = RD_TFL();
	if (P & _fD)
	{
		int c = (P & _fC) ^ _fC;
		int sum = tflagtemp - tmp -c;
		int lo = (tflagtemp & 0x0f) - (tmp & 0x0f) - c;
		int hi = (tflagtemp & 0xf0) - (tmp & 0xf0);
		P &= ~_fC;
		if (lo & 0xf0)
			lo -= 6;
		if (lo & 0x80)
			hi -= 0x10;
		if (hi & 0x0f00)
			hi -= 0x60;
		if ((sum & 0xff00) == 0)
			P |= _fC;
		tflagtemp = (lo & 0x0f) + (hi & 0xf0);
		H6280_CYCLES(1);
	}
	else
	{
		int c = (P & _fC) ^ _fC;
		int sum = tflagtemp - tmp - c;
		P &= ~(_fV | _fC);
		if ((tflagtemp^tmp) & (tflagtemp^sum) & _fN)
			P |= _fV;
		if ((sum & 0xff00) == 0)
			P |= _fC;
		tflagtemp = (UINT8) sum;
	}
	SET_NZ(tflagtemp);
	WB_EAZ(tflagtemp);
	H6280_CYCLES(3);
}

inline void h6280_device::SBC(UINT8 tmp)
{
	if(P & _fT)
		TSBC(tmp);
	else {
		if (P & _fD)
		{
			int c = (P & _fC) ^ _fC;
			int sum = A - tmp - c;
			int lo = (A & 0x0f) - (tmp & 0x0f) - c;
			int hi = (A & 0xf0) - (tmp & 0xf0);
			P &= ~_fC;
			if (lo & 0xf0)
				lo -= 6;
			if (lo & 0x80)
				hi -= 0x10;
			if (hi & 0x0f00)
				hi -= 0x60;
			if ((sum & 0xff00) == 0)
				P |= _fC;
			A = (lo & 0x0f) + (hi & 0xf0);
			H6280_CYCLES(1);
		}
		else
		{
			int c = (P & _fC) ^ _fC;
			int sum = A - tmp - c;
			P &= ~(_fV | _fC);
			if ((A^tmp) & (A^sum) & _fN)
				P |= _fV;
			if ((sum & 0xff00) == 0)
				P |= _fC;
			A = (UINT8) sum;
		}
		SET_NZ(A);
	}
}

/* 6280 ********************************************************
 *  SEC Set carry flag
 ***************************************************************/
#if defined(SEC)
#undef SEC
#endif
inline void h6280_device::SEC()
{
	CLEAR_T();
	P |= _fC;
}

/* 6280 ********************************************************
 *  SED Set decimal flag
 ***************************************************************/
inline void h6280_device::SED()
{
	CLEAR_T();
	P |= _fD;
}

/* 6280 ********************************************************
 *  SEI Set interrupt flag
 ***************************************************************/
inline void h6280_device::SEI()
{
	CLEAR_T();
	P |= _fI;
}

/* 6280 ********************************************************
 *  SET Set t flag
 ***************************************************************/
inline void h6280_device::SET()
{
	P |= _fT;
}

/* 6280 ********************************************************
 *  SMB Set memory bit
 ***************************************************************/
inline UINT8 h6280_device::SMB(int bit, UINT8 tmp)
{
	CLEAR_T();
	tmp |= (1<<bit);
	return tmp;
}

/* 6280 ********************************************************
 *  ST0 Store at hardware address 0
 ***************************************************************/
inline void h6280_device::ST0(UINT8 tmp)
{
	CLEAR_T();
	m_io->write_byte(0x0000,tmp);
}

/* 6280 ********************************************************
 *  ST1 Store at hardware address 2
 ***************************************************************/
inline void h6280_device::ST1(UINT8 tmp)
{
	CLEAR_T();
	m_io->write_byte(0x0002,tmp);
}

/* 6280 ********************************************************
 *  ST2 Store at hardware address 3
 ***************************************************************/
inline void h6280_device::ST2(UINT8 tmp)
{
	CLEAR_T();
	m_io->write_byte(0x0003,tmp);
}

/* 6280 ********************************************************
 *  STA Store accumulator
 ***************************************************************/
inline UINT8 h6280_device::STA()
{
	CLEAR_T();
	return A;
}

/* 6280 ********************************************************
 *  STX Store index X
 ***************************************************************/
inline UINT8 h6280_device::STX()
{
	CLEAR_T();
	return X;
}

/* 6280 ********************************************************
 *  STY Store index Y
 ***************************************************************/
inline UINT8 h6280_device::STY()
{
	CLEAR_T();
	return Y;
}

/* 6280 ********************************************************
 * STZ  Store zero
 ***************************************************************/
inline UINT8 h6280_device::STZ()
{
	CLEAR_T();
	return 0;
}

/* H6280 *******************************************************
 *  SXY Swap index X and index Y
 ***************************************************************/
inline void h6280_device::SXY()
{
	CLEAR_T();
	UINT8 tmp = X;
	X = Y;
	Y = tmp;
}

/* H6280 *******************************************************
 *  TAI Transfer Alternate Increment
 ***************************************************************/
inline void h6280_device::TAI()
{
	CLEAR_T();
	int from = program_read16(PCW);
	int to = program_read16(PCW + 2);
	int length = program_read16(PCW + 4);
	PCW += 6;
	int alternate = 0;
	if (!length) length = 0x10000;
	H6280_CYCLES( ((6 * length) + 17) );
	while ((length--) != 0)
	{
		program_write8(to, program_read8(from + alternate));
		to++;
		alternate ^= 1;
	}
}

/* H6280 *******************************************************
 *  TAM Transfer accumulator to memory mapper register(s)
 ***************************************************************/
inline void h6280_device::TAM(UINT8 tmp)
{
	CLEAR_T();
	if (tmp&0x01) m_mmr[0] = A;
	if (tmp&0x02) m_mmr[1] = A;
	if (tmp&0x04) m_mmr[2] = A;
	if (tmp&0x08) m_mmr[3] = A;
	if (tmp&0x10) m_mmr[4] = A;
	if (tmp&0x20) m_mmr[5] = A;
	if (tmp&0x40) m_mmr[6] = A;
	if (tmp&0x80) m_mmr[7] = A;
}

/* 6280 ********************************************************
 *  TAX Transfer accumulator to index X
 ***************************************************************/
inline void h6280_device::TAX()
{
	CLEAR_T();
	X = A;
	SET_NZ(X);
}

/* 6280 ********************************************************
 *  TAY Transfer accumulator to index Y
 ***************************************************************/
inline void h6280_device::TAY()
{
	CLEAR_T();
	Y = A;
	SET_NZ(Y);
}

/* 6280 ********************************************************
 *  TDD Transfer Decrement Decrement
 ***************************************************************/
inline void h6280_device::TDD()
{
	CLEAR_T();
	int from = program_read16(PCW);
	int to = program_read16(PCW + 2);
	int length = program_read16(PCW + 4);
	PCW+=6;
	if (!length) length = 0x10000;
	H6280_CYCLES( ((6 * length) + 17) );
	while ((length--) != 0) {
		program_write8(to, program_read8(from));
		to--;
		from--;
	}
}

/* 6280 ********************************************************
 *  TIA Transfer Increment Alternate
 ***************************************************************/
inline void h6280_device::TIA()
{
	CLEAR_T();
	int from = program_read16(PCW);
	int to  = program_read16(PCW + 2);
	int length = program_read16(PCW + 4);
	PCW+=6;
	int alternate=0;
	if (!length) length = 0x10000;
	H6280_CYCLES( ((6 * length) + 17) );
	while ((length--) != 0) {
		program_write8(to + alternate, program_read8(from));
		from++;
		alternate ^= 1;
	}
}

/* 6280 ********************************************************
 *  TII Transfer Increment Increment
 ***************************************************************/
inline void h6280_device::TII()
{
	CLEAR_T();
	int from = program_read16(PCW);
	int to = program_read16(PCW + 2);
	int length = program_read16(PCW + 4);
	PCW += 6;
	if (!length) length = 0x10000;
	H6280_CYCLES( ((6 * length) + 17) );
	while ((length--) != 0) {
		program_write8(to, program_read8(from));
		to++;
		from++;
	}
}

/* 6280 ********************************************************
 *  TIN Transfer block, source increments every loop
 ***************************************************************/
inline void h6280_device::TIN()
{
	CLEAR_T();
	int from = program_read16(PCW);
	int to = program_read16(PCW + 2);
	int length = program_read16(PCW + 4);
	PCW+=6;
	if (!length) length = 0x10000;
	H6280_CYCLES( ((6 * length) + 17) );
	while ((length--) != 0) {
		program_write8(to, program_read8(from));
		from++;
	}
}

/* 6280 ********************************************************
 *  TMA Transfer memory mapper register(s) to accumulator
 *  the highest bit set in tmp is the one that counts
 ***************************************************************/
inline void h6280_device::TMA(UINT8 tmp)
{
	CLEAR_T();
	if (tmp&0x01) A = m_mmr[0];
	if (tmp&0x02) A = m_mmr[1];
	if (tmp&0x04) A = m_mmr[2];
	if (tmp&0x08) A = m_mmr[3];
	if (tmp&0x10) A = m_mmr[4];
	if (tmp&0x20) A = m_mmr[5];
	if (tmp&0x40) A = m_mmr[6];
	if (tmp&0x80) A = m_mmr[7];
}

/* 6280 ********************************************************
 * TRB  Test and reset bits
 ***************************************************************/
inline UINT8 h6280_device::TRB(UINT8 tmp)
{
	CLEAR_T();
	P = (P & ~(_fN|_fV|_fT|_fZ))
		| ((tmp&0x80) ? _fN:0)
		| ((tmp&0x40) ? _fV:0)
		| ((tmp&~A)  ? 0:_fZ);
	tmp &= ~A;
	return tmp;
}

/* 6280 ********************************************************
 * TSB  Test and set bits
 ***************************************************************/
inline UINT8 h6280_device::TSB(UINT8 tmp)
{
	CLEAR_T();
	P = (P & ~(_fN|_fV|_fT|_fZ))
		| ((tmp&0x80) ? _fN:0)
		| ((tmp&0x40) ? _fV:0)
		| ((tmp|A)  ? 0:_fZ);
	tmp |= A;
	return tmp;
}

/* 6280 ********************************************************
 *  TSX Transfer stack LSB to index X
 ***************************************************************/
inline void h6280_device::TSX()
{
	CLEAR_T();
	X = S;
	SET_NZ(X);
}

/* 6280 ********************************************************
 *  TST
 ***************************************************************/
inline void h6280_device::TST(UINT8 imm, UINT8 tmp)
{
	P = (P & ~(_fN|_fV|_fT|_fZ))
		| ((tmp&0x80) ? _fN:0)
		| ((tmp&0x40) ? _fV:0)
		| ((tmp&imm)  ? 0:_fZ);
}

/* 6280 ********************************************************
 *  TXA Transfer index X to accumulator
 ***************************************************************/
inline void h6280_device::TXA()
{
	CLEAR_T();
	A = X;
	SET_NZ(A);
}

/* 6280 ********************************************************
 *  TXS Transfer index X to stack LSB
 *  no flags changed (sic!)
 ***************************************************************/
inline void h6280_device::TXS()
{
	CLEAR_T();
	S = X;
}

/* 6280 ********************************************************
 *  TYA Transfer index Y to accumulator
 ***************************************************************/
inline void h6280_device::TYA()
{
	CLEAR_T();
	A = Y;
	SET_NZ(A);
}

/* 6280 ********************************************************
 * CSH Set CPU in high speed mode
 ***************************************************************/
inline void h6280_device::CSH()
{
	m_clocks_per_cycle = 1;
}

/* 6280 ********************************************************
 * CSL Set CPU in low speed mode
 ***************************************************************/
inline void h6280_device::CSL()
{
	m_clocks_per_cycle = 4;
}


#define OP(prefix,opcode)  void h6280_device::prefix##_##opcode()

/*****************************************************************************
 *****************************************************************************
 *
 *   Hu6280 opcodes
 *
 *****************************************************************************
 *    op    cycles           opc                       ***********************/
OP(op,00) { H6280_CYCLES(8); BRK();                    } // 8 BRK
OP(op,20) { H6280_CYCLES(7); EA_ABS(); JSR();          } // 7 JSR  ABS
OP(op,40) { H6280_CYCLES(7); RTI();                    } // 7 RTI
OP(op,60) { H6280_CYCLES(7); RTS();                    } // 7 RTS
OP(op,80) {                  BRA(1);                   } // 4 BRA  REL
OP(op,a0) { H6280_CYCLES(2); LDY(RD_IMM());            } // 2 LDY  IMM
OP(op,c0) { H6280_CYCLES(2); CPY(RD_IMM());            } // 2 CPY  IMM
OP(op,e0) { H6280_CYCLES(2); CPX(RD_IMM());            } // 2 CPX  IMM

OP(op,10) {                  BPL();                    } // 2/4 BPL  REL
OP(op,30) {                  BMI();                    } // 2/4 BMI  REL
OP(op,50) {                  BVC();                    } // 2/4 BVC  REL
OP(op,70) {                  BVS();                    } // 2/4 BVS  REL
OP(op,90) {                  BCC();                    } // 2/4 BCC  REL
OP(op,b0) {                  BCS();                    } // 2/4 BCS  REL
OP(op,d0) {                  BNE();                    } // 2/4 BNE  REL
OP(op,f0) {                  BEQ();                    } // 2/4 BEQ  REL

OP(op,01) { H6280_CYCLES(7); ORA(RD_IDX());            } // 7 ORA  IDX
OP(op,21) { H6280_CYCLES(7); AND(RD_IDX());            } // 7 AND  IDX
OP(op,41) { H6280_CYCLES(7); EOR(RD_IDX());            } // 7 EOR  IDX
OP(op,61) { H6280_CYCLES(7); ADC(RD_IDX());            } // 7 ADC  IDX
OP(op,81) { H6280_CYCLES(7); WR_IDX(STA());            } // 7 STA  IDX
OP(op,a1) { H6280_CYCLES(7); LDA(RD_IDX());            } // 7 LDA  IDX
OP(op,c1) { H6280_CYCLES(7); CMP(RD_IDX());            } // 7 CMP  IDX
OP(op,e1) { H6280_CYCLES(7); SBC(RD_IDX());            } // 7 SBC  IDX

OP(op,11) { H6280_CYCLES(7); ORA(RD_IDY());            } // 7 ORA  IDY
OP(op,31) { H6280_CYCLES(7); AND(RD_IDY());            } // 7 AND  IDY
OP(op,51) { H6280_CYCLES(7); EOR(RD_IDY());            } // 7 EOR  IDY
OP(op,71) { H6280_CYCLES(7); ADC(RD_IDY());            } // 7 ADC  AZP
OP(op,91) { H6280_CYCLES(7); WR_IDY(STA());            } // 7 STA  IDY
OP(op,b1) { H6280_CYCLES(7); LDA(RD_IDY());            } // 7 LDA  IDY
OP(op,d1) { H6280_CYCLES(7); CMP(RD_IDY());            } // 7 CMP  IDY
OP(op,f1) { H6280_CYCLES(7); SBC(RD_IDY());            } // 7 SBC  IDY

OP(op,02) { H6280_CYCLES(3); SXY();                    } // 3 SXY
OP(op,22) { H6280_CYCLES(3); SAX();                    } // 3 SAX
OP(op,42) { H6280_CYCLES(3); SAY();                    } // 3 SAY
OP(op,62) { H6280_CYCLES(2); CLA();                    } // 2 CLA
OP(op,82) { H6280_CYCLES(2); CLX();                    } // 2 CLX
OP(op,a2) { H6280_CYCLES(2); LDX(RD_IMM());            } // 2 LDX  IMM
OP(op,c2) { H6280_CYCLES(2); CLY();                    } // 2 CLY
OP(op,e2) { H6280_CYCLES(2); NOP();                    } // 2 NOP

OP(op,12) { H6280_CYCLES(7); ORA(RD_ZPI());            } // 7 ORA  ZPI
OP(op,32) { H6280_CYCLES(7); AND(RD_ZPI());            } // 7 AND  ZPI
OP(op,52) { H6280_CYCLES(7); EOR(RD_ZPI());            } // 7 EOR  ZPI
OP(op,72) { H6280_CYCLES(7); ADC(RD_ZPI());            } // 7 ADC  ZPI
OP(op,92) { H6280_CYCLES(7); WR_ZPI(STA());            } // 7 STA  ZPI
OP(op,b2) { H6280_CYCLES(7); LDA(RD_ZPI());            } // 7 LDA  ZPI
OP(op,d2) { H6280_CYCLES(7); CMP(RD_ZPI());            } // 7 CMP  ZPI
OP(op,f2) { H6280_CYCLES(7); SBC(RD_ZPI());            } // 7 SBC  ZPI

OP(op,03) { H6280_CYCLES(5); ST0(RD_IMM());            } // 4 + 1 penalty cycle ST0  IMM
OP(op,23) { H6280_CYCLES(5); ST2(RD_IMM());            } // 4 + 1 penalty cycle ST2  IMM
OP(op,43) { H6280_CYCLES(4); TMA(RD_IMM());            } // 4 TMA
OP(op,63) { H6280_CYCLES(4); NOP();                    } // 2 NOP
OP(op,83) { H6280_CYCLES(7); int imm = RD_IMM(); TST(imm, RD_ZPG()); } // 7 TST  IMM,ZPG
OP(op,a3) { H6280_CYCLES(7); int imm = RD_IMM(); TST(imm, RD_ZPX()); } // 7 TST  IMM,ZPX
OP(op,c3) {                  TDD();                    } // 6*l+17 TDD  XFER
OP(op,e3) {                  TIA();                    } // 6*l+17 TIA  XFER

OP(op,13) { H6280_CYCLES(5); ST1(RD_IMM());            } // 4 + 1 penalty cycle ST1
OP(op,33) { H6280_CYCLES(2); NOP();                    } // 2 NOP
OP(op,53) { H6280_CYCLES(5); TAM(RD_IMM());            } // 5 TAM  IMM
OP(op,73) {                  TII();                    } // 6*l+17 TII  XFER
OP(op,93) { H6280_CYCLES(8); int imm = RD_IMM(); TST(imm, RD_ABS()); } // 8 TST  IMM,ABS
OP(op,b3) { H6280_CYCLES(8); int imm = RD_IMM(); TST(imm, RD_ABX()); } // 8 TST  IMM,ABX
OP(op,d3) {                  TIN();                    } // 6*l+17 TIN  XFER
OP(op,f3) {                  TAI();                    } // 6*l+17 TAI  XFER

OP(op,04) { H6280_CYCLES(6); WB_EAZ(TSB(RD_ZPG()));    } // 6 TSB  ZPG
OP(op,24) { H6280_CYCLES(4); BIT(RD_ZPG());            } // 4 BIT  ZPG
OP(op,44) {                  BSR();                    } // 8 BSR  REL
OP(op,64) { H6280_CYCLES(4); WR_ZPG(STZ());            } // 4 STZ  ZPG
OP(op,84) { H6280_CYCLES(4); WR_ZPG(STY());            } // 4 STY  ZPG
OP(op,a4) { H6280_CYCLES(4); LDY(RD_ZPG());            } // 4 LDY  ZPG
OP(op,c4) { H6280_CYCLES(4); CPY(RD_ZPG());            } // 4 CPY  ZPG
OP(op,e4) { H6280_CYCLES(4); CPX(RD_ZPG());            } // 4 CPX  ZPG

OP(op,14) { H6280_CYCLES(6); WB_EAZ(TRB(RD_ZPG()));    } // 6 TRB  ZPG
OP(op,34) { H6280_CYCLES(4); BIT(RD_ZPX());            } // 4 BIT  ZPX
OP(op,54) { H6280_CYCLES(3); CSL();                    } // 3 CSL
OP(op,74) { H6280_CYCLES(4); WR_ZPX(STZ());            } // 4 STZ  ZPX
OP(op,94) { H6280_CYCLES(4); WR_ZPX(STY());            } // 4 STY  ZPX
OP(op,b4) { H6280_CYCLES(4); LDY(RD_ZPX());            } // 4 LDY  ZPX
OP(op,d4) { H6280_CYCLES(3); CSH();                    } // 3 CSH
OP(op,f4) { H6280_CYCLES(2); SET();                    } // 2 SET

OP(op,05) { H6280_CYCLES(4); ORA(RD_ZPG());            } // 4 ORA  ZPG
OP(op,25) { H6280_CYCLES(4); AND(RD_ZPG());            } // 4 AND  ZPG
OP(op,45) { H6280_CYCLES(4); EOR(RD_ZPG());            } // 4 EOR  ZPG
OP(op,65) { H6280_CYCLES(4); ADC(RD_ZPG());            } // 4 ADC  ZPG
OP(op,85) { H6280_CYCLES(4); WR_ZPG(STA());            } // 4 STA  ZPG
OP(op,a5) { H6280_CYCLES(4); LDA(RD_ZPG());            } // 4 LDA  ZPG
OP(op,c5) { H6280_CYCLES(4); CMP(RD_ZPG());            } // 4 CMP  ZPG
OP(op,e5) { H6280_CYCLES(4); SBC(RD_ZPG());            } // 4 SBC  ZPG

OP(op,15) { H6280_CYCLES(4); ORA(RD_ZPX());            } // 4 ORA  ZPX
OP(op,35) { H6280_CYCLES(4); AND(RD_ZPX());            } // 4 AND  ZPX
OP(op,55) { H6280_CYCLES(4); EOR(RD_ZPX());            } // 4 EOR  ZPX
OP(op,75) { H6280_CYCLES(4); ADC(RD_ZPX());            } // 4 ADC  ZPX
OP(op,95) { H6280_CYCLES(4); WR_ZPX(STA());            } // 4 STA  ZPX
OP(op,b5) { H6280_CYCLES(4); LDA(RD_ZPX());            } // 4 LDA  ZPX
OP(op,d5) { H6280_CYCLES(4); CMP(RD_ZPX());            } // 4 CMP  ZPX
OP(op,f5) { H6280_CYCLES(4); SBC(RD_ZPX());            } // 4 SBC  ZPX

OP(op,06) { H6280_CYCLES(6); WB_EAZ(ASL(RD_ZPG()));    } // 6 ASL  ZPG
OP(op,26) { H6280_CYCLES(6); WB_EAZ(ROL(RD_ZPG()));    } // 6 ROL  ZPG
OP(op,46) { H6280_CYCLES(6); WB_EAZ(LSR(RD_ZPG()));    } // 6 LSR  ZPG
OP(op,66) { H6280_CYCLES(6); WB_EAZ(ROR(RD_ZPG()));    } // 6 ROR  ZPG
OP(op,86) { H6280_CYCLES(4); WR_ZPG(STX());            } // 4 STX  ZPG
OP(op,a6) { H6280_CYCLES(4); LDX(RD_ZPG());            } // 4 LDX  ZPG
OP(op,c6) { H6280_CYCLES(6); WB_EAZ(DEC(RD_ZPG()));    } // 6 DEC  ZPG
OP(op,e6) { H6280_CYCLES(6); WB_EAZ(INC(RD_ZPG()));    } // 6 INC  ZPG

OP(op,16) { H6280_CYCLES(6); WB_EAZ(ASL(RD_ZPX()));    } // 6 ASL  ZPX
OP(op,36) { H6280_CYCLES(6); WB_EAZ(ROL(RD_ZPX()));    } // 6 ROL  ZPX
OP(op,56) { H6280_CYCLES(6); WB_EAZ(LSR(RD_ZPX()));    } // 6 LSR  ZPX
OP(op,76) { H6280_CYCLES(6); WB_EAZ(ROR(RD_ZPX()));    } // 6 ROR  ZPX
OP(op,96) { H6280_CYCLES(4); WR_ZPY(STX());            } // 4 STX  ZPY
OP(op,b6) { H6280_CYCLES(4); LDX(RD_ZPY());            } // 4 LDX  ZPY
OP(op,d6) { H6280_CYCLES(6); WB_EAZ(DEC(RD_ZPX()));    } // 6 DEC  ZPX
OP(op,f6) { H6280_CYCLES(6); WB_EAZ(INC(RD_ZPX()));    } // 6 INC  ZPX

OP(op,07) { H6280_CYCLES(7); WB_EAZ(RMB(0, RD_ZPG())); } // 7 RMB0 ZPG
OP(op,27) { H6280_CYCLES(7); WB_EAZ(RMB(2, RD_ZPG())); } // 7 RMB2 ZPG
OP(op,47) { H6280_CYCLES(7); WB_EAZ(RMB(4, RD_ZPG())); } // 7 RMB4 ZPG
OP(op,67) { H6280_CYCLES(7); WB_EAZ(RMB(6, RD_ZPG())); } // 7 RMB6 ZPG
OP(op,87) { H6280_CYCLES(7); WB_EAZ(SMB(0, RD_ZPG())); } // 7 SMB0 ZPG
OP(op,a7) { H6280_CYCLES(7); WB_EAZ(SMB(2, RD_ZPG())); } // 7 SMB2 ZPG
OP(op,c7) { H6280_CYCLES(7); WB_EAZ(SMB(4, RD_ZPG())); } // 7 SMB4 ZPG
OP(op,e7) { H6280_CYCLES(7); WB_EAZ(SMB(6, RD_ZPG())); } // 7 SMB6 ZPG

OP(op,17) { H6280_CYCLES(7); WB_EAZ(RMB(1, RD_ZPG())); } // 7 RMB1 ZPG
OP(op,37) { H6280_CYCLES(7); WB_EAZ(RMB(3, RD_ZPG())); } // 7 RMB3 ZPG
OP(op,57) { H6280_CYCLES(7); WB_EAZ(RMB(5, RD_ZPG())); } // 7 RMB5 ZPG
OP(op,77) { H6280_CYCLES(7); WB_EAZ(RMB(7, RD_ZPG())); } // 7 RMB7 ZPG
OP(op,97) { H6280_CYCLES(7); WB_EAZ(SMB(1, RD_ZPG())); } // 7 SMB1 ZPG
OP(op,b7) { H6280_CYCLES(7); WB_EAZ(SMB(3, RD_ZPG())); } // 7 SMB3 ZPG
OP(op,d7) { H6280_CYCLES(7); WB_EAZ(SMB(5, RD_ZPG())); } // 7 SMB5 ZPG
OP(op,f7) { H6280_CYCLES(7); WB_EAZ(SMB(7, RD_ZPG())); } // 7 SMB7 ZPG

OP(op,08) { H6280_CYCLES(3); PHP();                    } // 3 PHP
OP(op,28) { H6280_CYCLES(4); PLP();                    } // 4 PLP
OP(op,48) { H6280_CYCLES(3); PHA();                    } // 3 PHA
OP(op,68) { H6280_CYCLES(4); PLA();                    } // 4 PLA
OP(op,88) { H6280_CYCLES(2); DEY();                    } // 2 DEY
OP(op,a8) { H6280_CYCLES(2); TAY();                    } // 2 TAY
OP(op,c8) { H6280_CYCLES(2); INY();                    } // 2 INY
OP(op,e8) { H6280_CYCLES(2); INX();                    } // 2 INX

OP(op,18) { H6280_CYCLES(2); CLC();                    } // 2 CLC
OP(op,38) { H6280_CYCLES(2); SEC();                    } // 2 SEC
OP(op,58) { H6280_CYCLES(2); CLI();                    } // 2 CLI
OP(op,78) { H6280_CYCLES(2); SEI();                    } // 2 SEI
OP(op,98) { H6280_CYCLES(2); TYA();                    } // 2 TYA
OP(op,b8) { H6280_CYCLES(2); CLV();                    } // 2 CLV
OP(op,d8) { H6280_CYCLES(2); CLD();                    } // 2 CLD
OP(op,f8) { H6280_CYCLES(2); SED();                    } // 2 SED

OP(op,09) { H6280_CYCLES(2); ORA(RD_IMM());            } // 2 ORA  IMM
OP(op,29) { H6280_CYCLES(2); AND(RD_IMM());            } // 2 AND  IMM
OP(op,49) { H6280_CYCLES(2); EOR(RD_IMM());            } // 2 EOR  IMM
OP(op,69) { H6280_CYCLES(2); ADC(RD_IMM());            } // 2 ADC  IMM
OP(op,89) { H6280_CYCLES(2); BIT(RD_IMM());            } // 2 BIT  IMM
OP(op,a9) { H6280_CYCLES(2); LDA(RD_IMM());            } // 2 LDA  IMM
OP(op,c9) { H6280_CYCLES(2); CMP(RD_IMM());            } // 2 CMP  IMM
OP(op,e9) { H6280_CYCLES(2); SBC(RD_IMM());            } // 2 SBC  IMM

OP(op,19) { H6280_CYCLES(5); ORA(RD_ABY());            } // 5 ORA  ABY
OP(op,39) { H6280_CYCLES(5); AND(RD_ABY());            } // 5 AND  ABY
OP(op,59) { H6280_CYCLES(5); EOR(RD_ABY());            } // 5 EOR  ABY
OP(op,79) { H6280_CYCLES(5); ADC(RD_ABY());            } // 5 ADC  ABY
OP(op,99) { H6280_CYCLES(5); WR_ABY(STA());            } // 5 STA  ABY
OP(op,b9) { H6280_CYCLES(5); LDA(RD_ABY());            } // 5 LDA  ABY
OP(op,d9) { H6280_CYCLES(5); CMP(RD_ABY());            } // 5 CMP  ABY
OP(op,f9) { H6280_CYCLES(5); SBC(RD_ABY());            } // 5 SBC  ABY

OP(op,0a) { H6280_CYCLES(2); A = ASL(A);               } // 2 ASL  A
OP(op,2a) { H6280_CYCLES(2); A = ROL(A);               } // 2 ROL  A
OP(op,4a) { H6280_CYCLES(2); A = LSR(A);               } // 2 LSR  A
OP(op,6a) { H6280_CYCLES(2); A = ROR(A);               } // 2 ROR  A
OP(op,8a) { H6280_CYCLES(2); TXA();                    } // 2 TXA
OP(op,aa) { H6280_CYCLES(2); TAX();                    } // 2 TAX
OP(op,ca) { H6280_CYCLES(2); DEX();                    } // 2 DEX
OP(op,ea) { H6280_CYCLES(2); NOP();                    } // 2 NOP

OP(op,1a) { H6280_CYCLES(2); A = INC(A);               } // 2 INC  A
OP(op,3a) { H6280_CYCLES(2); A = DEC(A);               } // 2 DEC  A
OP(op,5a) { H6280_CYCLES(3); PHY();                    } // 3 PHY
OP(op,7a) { H6280_CYCLES(4); PLY();                    } // 4 PLY
OP(op,9a) { H6280_CYCLES(2); TXS();                    } // 2 TXS
OP(op,ba) { H6280_CYCLES(2); TSX();                    } // 2 TSX
OP(op,da) { H6280_CYCLES(3); PHX();                    } // 3 PHX
OP(op,fa) { H6280_CYCLES(4); PLX();                    } // 4 PLX

OP(op,0b) { H6280_CYCLES(2); NOP();                    } // 2 NOP
OP(op,2b) { H6280_CYCLES(2); NOP();                    } // 2 NOP
OP(op,4b) { H6280_CYCLES(2); NOP();                    } // 2 NOP
OP(op,6b) { H6280_CYCLES(2); NOP();                    } // 2 NOP
OP(op,8b) { H6280_CYCLES(2); NOP();                    } // 2 NOP
OP(op,ab) { H6280_CYCLES(2); NOP();                    } // 2 NOP
OP(op,cb) { H6280_CYCLES(2); NOP();                    } // 2 NOP
OP(op,eb) { H6280_CYCLES(2); NOP();                    } // 2 NOP

OP(op,1b) { H6280_CYCLES(2); NOP();                    } // 2 NOP
OP(op,3b) { H6280_CYCLES(2); NOP();                    } // 2 NOP
OP(op,5b) { H6280_CYCLES(2); NOP();                    } // 2 NOP
OP(op,7b) { H6280_CYCLES(2); NOP();                    } // 2 NOP
OP(op,9b) { H6280_CYCLES(2); NOP();                    } // 2 NOP
OP(op,bb) { H6280_CYCLES(2); NOP();                    } // 2 NOP
OP(op,db) { H6280_CYCLES(2); NOP();                    } // 2 NOP
OP(op,fb) { H6280_CYCLES(2); NOP();                    } // 2 NOP

OP(op,0c) { H6280_CYCLES(7); WB_EA(TSB(RD_ABS()));     } // 7 TSB  ABS
OP(op,2c) { H6280_CYCLES(5); BIT(RD_ABS());            } // 5 BIT  ABS
OP(op,4c) { H6280_CYCLES(4); EA_ABS(); JMP();          } // 4 JMP  ABS
OP(op,6c) { H6280_CYCLES(7); EA_IND(); JMP();          } // 7 JMP  IND
OP(op,8c) { H6280_CYCLES(5); WR_ABS(STY());            } // 5 STY  ABS
OP(op,ac) { H6280_CYCLES(5); LDY(RD_ABS());            } // 5 LDY  ABS
OP(op,cc) { H6280_CYCLES(5); CPY(RD_ABS());            } // 5 CPY  ABS
OP(op,ec) { H6280_CYCLES(5); CPX(RD_ABS());            } // 5 CPX  ABS

OP(op,1c) { H6280_CYCLES(7); WB_EA(TRB(RD_ABS()));     } // 7 TRB  ABS
OP(op,3c) { H6280_CYCLES(5); BIT(RD_ABX());            } // 5 BIT  ABX
OP(op,5c) { H6280_CYCLES(2); NOP();                    } // 2 NOP
OP(op,7c) { H6280_CYCLES(7); EA_IAX(); JMP();          } // 7 JMP  IAX
OP(op,9c) { H6280_CYCLES(5); WR_ABS(STZ());            } // 5 STZ  ABS
OP(op,bc) { H6280_CYCLES(5); LDY(RD_ABX());            } // 5 LDY  ABX
OP(op,dc) { H6280_CYCLES(2); NOP();                    } // 2 NOP
OP(op,fc) { H6280_CYCLES(2); NOP();                    } // 2 NOP

OP(op,0d) { H6280_CYCLES(5); ORA(RD_ABS());            } // 5 ORA  ABS
OP(op,2d) { H6280_CYCLES(5); AND(RD_ABS());            } // 5 AND  ABS
OP(op,4d) { H6280_CYCLES(5); EOR(RD_ABS());            } // 5 EOR  ABS
OP(op,6d) { H6280_CYCLES(5); ADC(RD_ABS());            } // 5 ADC  ABS
OP(op,8d) { H6280_CYCLES(5); WR_ABS(STA());            } // 5 STA  ABS
OP(op,ad) { H6280_CYCLES(5); LDA(RD_ABS());            } // 5 LDA  ABS
OP(op,cd) { H6280_CYCLES(5); CMP(RD_ABS());            } // 5 CMP  ABS
OP(op,ed) { H6280_CYCLES(5); SBC(RD_ABS());            } // 5 SBC  ABS

OP(op,1d) { H6280_CYCLES(5); ORA(RD_ABX());            } // 5 ORA  ABX
OP(op,3d) { H6280_CYCLES(5); AND(RD_ABX());            } // 5 AND  ABX
OP(op,5d) { H6280_CYCLES(5); EOR(RD_ABX());            } // 5 EOR  ABX
OP(op,7d) { H6280_CYCLES(5); ADC(RD_ABX());            } // 5 ADC  ABX
OP(op,9d) { H6280_CYCLES(5); WR_ABX(STA());            } // 5 STA  ABX
OP(op,bd) { H6280_CYCLES(5); LDA(RD_ABX());            } // 5 LDA  ABX
OP(op,dd) { H6280_CYCLES(5); CMP(RD_ABX());            } // 5 CMP  ABX
OP(op,fd) { H6280_CYCLES(5); SBC(RD_ABX());            } // 5 SBC  ABX

OP(op,0e) { H6280_CYCLES(7); WB_EA(ASL(RD_ABS()));     } // 7 ASL  ABS
OP(op,2e) { H6280_CYCLES(7); WB_EA(ROL(RD_ABS()));     } // 7 ROL  ABS
OP(op,4e) { H6280_CYCLES(7); WB_EA(LSR(RD_ABS()));     } // 7 LSR  ABS
OP(op,6e) { H6280_CYCLES(7); WB_EA(ROR(RD_ABS()));     } // 7 ROR  ABS
OP(op,8e) { H6280_CYCLES(5); WR_ABS(STX());            } // 5 STX  ABS
OP(op,ae) { H6280_CYCLES(5); LDX(RD_ABS());            } // 5 LDX  ABS
OP(op,ce) { H6280_CYCLES(7); WB_EA(DEC(RD_ABS()));     } // 7 DEC  ABS
OP(op,ee) { H6280_CYCLES(7); WB_EA(INC(RD_ABS()));     } // 7 INC  ABS

OP(op,1e) { H6280_CYCLES(7); WB_EA(ASL(RD_ABX()));     } // 7 ASL  ABX
OP(op,3e) { H6280_CYCLES(7); WB_EA(ROL(RD_ABX()));     } // 7 ROL  ABX
OP(op,5e) { H6280_CYCLES(7); WB_EA(LSR(RD_ABX()));     } // 7 LSR  ABX
OP(op,7e) { H6280_CYCLES(7); WB_EA(ROR(RD_ABX()));     } // 7 ROR  ABX
OP(op,9e) { H6280_CYCLES(5); WR_ABX(STZ());            } // 5 STZ  ABX
OP(op,be) { H6280_CYCLES(5); LDX(RD_ABY());            } // 5 LDX  ABY
OP(op,de) { H6280_CYCLES(7); WB_EA(DEC(RD_ABX()));     } // 7 DEC  ABX
OP(op,fe) { H6280_CYCLES(7); WB_EA(INC(RD_ABX()));     } // 7 INC  ABX

OP(op,0f) { H6280_CYCLES(4); BBR(0, RD_ZPG());         } // 6/8 BBR0 ZPG,REL
OP(op,2f) { H6280_CYCLES(4); BBR(2, RD_ZPG());         } // 6/8 BBR2 ZPG,REL
OP(op,4f) { H6280_CYCLES(4); BBR(4, RD_ZPG());         } // 6/8 BBR4 ZPG,REL
OP(op,6f) { H6280_CYCLES(4); BBR(6, RD_ZPG());         } // 6/8 BBR6 ZPG,REL
OP(op,8f) { H6280_CYCLES(4); BBS(0, RD_ZPG());         } // 6/8 BBS0 ZPG,REL
OP(op,af) { H6280_CYCLES(4); BBS(2, RD_ZPG());         } // 6/8 BBS2 ZPG,REL
OP(op,cf) { H6280_CYCLES(4); BBS(4, RD_ZPG());         } // 6/8 BBS4 ZPG,REL
OP(op,ef) { H6280_CYCLES(4); BBS(6, RD_ZPG());         } // 6/8 BBS6 ZPG,REL

OP(op,1f) { H6280_CYCLES(4); BBR(1, RD_ZPG());         } // 6/8 BBR1 ZPG,REL
OP(op,3f) { H6280_CYCLES(4); BBR(3, RD_ZPG());         } // 6/8 BBR3 ZPG,REL
OP(op,5f) { H6280_CYCLES(4); BBR(5, RD_ZPG());         } // 6/8 BBR5 ZPG,REL
OP(op,7f) { H6280_CYCLES(4); BBR(7, RD_ZPG());         } // 6/8 BBR7 ZPG,REL
OP(op,9f) { H6280_CYCLES(4); BBS(1, RD_ZPG());         } // 6/8 BBS1 ZPG,REL
OP(op,bf) { H6280_CYCLES(4); BBS(3, RD_ZPG());         } // 6/8 BBS3 ZPG,REL
OP(op,df) { H6280_CYCLES(4); BBS(5, RD_ZPG());         } // 6/8 BBS5 ZPG,REL
OP(op,ff) { H6280_CYCLES(4); BBS(7, RD_ZPG());         } // 6/8 BBS7 ZPG,REL

//-------------------------------------------------
//  state_string_export - export state as a string
//  for the debugger
//-------------------------------------------------

void h6280_device::state_string_export(const device_state_entry &entry, astring &string)
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			string.printf("%c%c%c%c%c%c%c%c",
				(m_p & 0x80) ? 'N':'.',
				(m_p & 0x40) ? 'V':'.',
				(m_p & 0x20) ? 'R':'.',
				(m_p & 0x10) ? 'B':'.',
				(m_p & 0x08) ? 'D':'.',
				(m_p & 0x04) ? 'I':'.',
				(m_p & 0x02) ? 'Z':'.',
				(m_p & 0x01) ? 'C':'.');
			break;
	}
}


//-------------------------------------------------
//  disasm_min_opcode_bytes - return the length
//  of the shortest instruction, in bytes
//-------------------------------------------------

UINT32 h6280_device::disasm_min_opcode_bytes() const
{
	return 1;
}


//-------------------------------------------------
//  disasm_max_opcode_bytes - return the length
//  of the longest instruction, in bytes
//-------------------------------------------------

UINT32 h6280_device::disasm_max_opcode_bytes() const
{
	return 7;
}


//-------------------------------------------------
//  disasm_disassemble - call the disassembly
//  helper function
//-------------------------------------------------

offs_t h6280_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( h6280 );
	return CPU_DISASSEMBLE_NAME(h6280)(this, buffer, pc, oprom, opram, options);
}


//-------------------------------------------------
//  execute_min_cycles - return minimum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

UINT32 h6280_device::execute_min_cycles() const
{
	return 2;
}


//-------------------------------------------------
//  execute_max_cycles - return maximum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

UINT32 h6280_device::execute_max_cycles() const
{
	return 17 + 6*65536;
}


//-------------------------------------------------
//  execute_input_lines - return the number of
//  input/interrupt lines
//-------------------------------------------------

UINT32 h6280_device::execute_input_lines() const
{
	return 4;
}


//-------------------------------------------------
//  execute_set_input - act on a changed input/
//  interrupt line
//-------------------------------------------------

void h6280_device::execute_set_input(int inputnum, int state)
{
	switch(inputnum)
	{
		case H6280_IRQ1_STATE:
		case 0:
			set_irq_line(0, state);
			break;
		case H6280_IRQ2_STATE:
		case 1:
			set_irq_line(1, state);
			break;
		case H6280_IRQT_STATE:
		case 2:
			set_irq_line(2, state);
			break;
		case H6280_NMI_STATE:
		case INPUT_LINE_NMI:
			set_irq_line(INPUT_LINE_NMI, state);
			break;
	}
}

/***************************************************************
 *  program_read8       read memory
 ***************************************************************/
UINT8 h6280_device::program_read8(offs_t addr)
{
	CHECK_VDC_VCE_PENALTY(addr);
	return m_program->read_byte(TRANSLATED(addr));
}

/***************************************************************
 *  program_write8      write memory
 ***************************************************************/
void h6280_device::program_write8(offs_t addr, UINT8 data)
{
	CHECK_VDC_VCE_PENALTY(addr);
	m_program->write_byte(TRANSLATED(addr), data);
}

/***************************************************************
 *  program_read8z      read memory - zero page
 ***************************************************************/
UINT8 h6280_device::program_read8z(offs_t addr)
{
	return m_program->read_byte((m_mmr[1] << 13) | (addr & 0x1fff));
}

/***************************************************************
 *  program_write8z     write memory - zero page
 ***************************************************************/
void h6280_device::program_write8z(offs_t addr, UINT8 data)
{
	m_program->write_byte((m_mmr[1] << 13) | (addr & 0x1fff), data);
}

/***************************************************************
 *  program_read16      read word from memory
 ***************************************************************/
UINT16 h6280_device::program_read16(offs_t addr)
{
	return m_program->read_byte(TRANSLATED(addr)) |
			(m_program->read_byte(TRANSLATED(addr + 1)) << 8);
}

/***************************************************************
 *  program_read16z     read a word from a zero page address
 ***************************************************************/
UINT16 h6280_device::program_read16z(offs_t addr)
{
	if ((addr & 0xff) == 0xff)
	{
		return m_program->read_byte((m_mmr[1] << 13) | (addr & 0x1fff)) |
				(m_program->read_byte((m_mmr[1] << 13) | ((addr - 0xff) & 0x1fff)) << 8);
	}
	else
	{
		return m_program->read_byte((m_mmr[1] << 13) | (addr & 0x1fff)) |
				(m_program->read_byte((m_mmr[1] << 13) | ((addr + 1) & 0x1fff)) << 8);
	}
}

/***************************************************************
 * push a register onto the stack
 ***************************************************************/
void h6280_device::push(UINT8 value)
{
	m_program->write_byte((m_mmr[1] << 13) | m_sp.d, value);
	S--;
}

/***************************************************************
 * pull a register from the stack
 ***************************************************************/
void h6280_device::pull(UINT8 &value)
{
	S++;
	value = m_program->read_byte((m_mmr[1] << 13) | m_sp.d);
}

/***************************************************************
 *  read_opcode     read an opcode
 ***************************************************************/
UINT8 h6280_device::read_opcode()
{
	return m_direct->read_decrypted_byte(TRANSLATED(PCW));
}

/***************************************************************
 *  read_opcode_arg read an opcode argument
 ***************************************************************/
UINT8 h6280_device::read_opcode_arg()
{
	return m_direct->read_raw_byte(TRANSLATED(PCW));
}


//-------------------------------------------------
//  execute_run - execute a timeslice's worth of
//  opcodes
//-------------------------------------------------

void h6280_device::execute_run()
{
	int in;

	if (m_irq_pending == 2)
	{
		m_irq_pending--;
	}

	/* Execute instructions */
	do
	{
		m_ppc = m_pc;

		debugger_instruction_hook(this, PCW);

		/* Execute 1 instruction */
		in = read_opcode();
		PCW++;
		(this->*m_opcode[in])();

		if (m_irq_pending)
		{
			if (m_irq_pending == 1)
			{
				if (!(P & _fI))
				{
					m_irq_pending--;
					CHECK_AND_TAKE_IRQ_LINES();
				}
			}
			else
			{
				m_irq_pending--;
			}
		}

		/* Check internal timer */
		if (m_timer_status)
		{
			if (m_timer_value<=0)
			{
				if (!m_irq_pending)
				{
					m_irq_pending = 1;
				}
				while (m_timer_value <= 0)
				{
					m_timer_value += m_timer_load;
				}
				set_irq_line(2, ASSERT_LINE);
			}
		}
	} while (m_icount > 0);
}


//**************************************************************************
//  IRQ HANDLING
//**************************************************************************

void h6280_device::set_irq_line(int irqline, int state)
{
	if (irqline == INPUT_LINE_NMI)
	{
		if (state != ASSERT_LINE)
			return;
		m_nmi_state = state;
		CHECK_IRQ_LINES();
	}
	else if (irqline < 3)
	{
		/* If the state has not changed, just return */
		if (m_irq_state[irqline] == state)
			return;

		m_irq_state[irqline] = state;

		CHECK_IRQ_LINES();
	}
}


//**************************************************************************
//  REGISTER HANDLING
//**************************************************************************

READ8_MEMBER( h6280_device::irq_status_r )
{
	int status;

	switch (offset & 3)
	{
	default:
		return m_io_buffer;
	case 3:
		{
			status = 0;
			if (m_irq_state[1] != CLEAR_LINE)
				status |= 1; /* IRQ 2 */
			if (m_irq_state[0] != CLEAR_LINE)
				status |= 2; /* IRQ 1 */
			if (m_irq_state[2] != CLEAR_LINE)
				status |= 4; /* TIMER */
			return status | (m_io_buffer & (~H6280_IRQ_MASK));
		}
	case 2:
		return m_irq_mask | (m_io_buffer & (~H6280_IRQ_MASK));
	}
}

WRITE8_MEMBER( h6280_device::irq_status_w )
{
	m_io_buffer = data;
	switch (offset & 3)
	{
		default:
			m_io_buffer = data;
			break;

		case 2: /* Write irq mask */
			m_irq_mask = data & 0x7;
			CHECK_IRQ_LINES();
			break;

		case 3: /* Timer irq ack */
			set_irq_line(2, CLEAR_LINE);
			break;
	}
}

READ8_MEMBER( h6280_device::timer_r )
{
	/* only returns countdown */
	return ((m_timer_value >> 10) & 0x7F) | (m_io_buffer & 0x80);
}

WRITE8_MEMBER( h6280_device::timer_w )
{
	m_io_buffer = data;
	switch (offset & 1)
	{
		case 0: /* Counter preload */
			m_timer_load = m_timer_value = ((data & 127) + 1) * 1024;
			return;

		case 1: /* Counter enable */
			if (data & 1)
			{/* stop -> start causes reload */
				if(m_timer_status == 0)
					m_timer_value = m_timer_load;
			}
			m_timer_status = data & 1;
			return;
	}
}

bool h6280_device::memory_translate(address_spacenum spacenum, int intention, offs_t &address)
{
	if (spacenum == AS_PROGRAM)
		address = TRANSLATED(address);

	return TRUE;
}

UINT8 h6280_device::io_get_buffer()
{
	return m_io_buffer;
}

void h6280_device::io_set_buffer(UINT8 data)
{
	m_io_buffer = data;
}
