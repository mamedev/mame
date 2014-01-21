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

#include "emu.h"
#include "debugger.h"
#include "h6280.h"

// include the macros
#include "h6280ops.h"

//static void set_irq_line(h6280_Regs* cpustate, int irqline, int state);

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

	/* clear pending interrupts */
	for (int i = 0; i < 3; i++)
	{
		m_irq_state[i] = CLEAR_LINE;
	}
	m_nmi_state = CLEAR_LINE;

	m_irq_pending = 0;
}

void h6280_device::device_stop()
{
	/* nothing */
}


//-------------------------------------------------
//  memory_space_config - return the configuration
//  of the specified address space, or NULL if
//  the space doesn't exist
//-------------------------------------------------

const address_space_config *h6280_device::memory_space_config(address_spacenum spacenum) const
{
	if (spacenum == AS_PROGRAM)
	{
		return &m_program_config;
	}
	else if (spacenum == AS_IO)
	{
		return &m_io_config;
	}
	return NULL;
}

#define OP(prefix,opcode)  void h6280_device::prefix##_##opcode()

/*****************************************************************************
 *****************************************************************************
 *
 *   Hu6280 opcodes
 *
 *****************************************************************************
 * op       temp     cycles           rdmem   opc   wrmem   ******************/
OP(op,00) {          H6280_CYCLES(8);         BRK;          } // 8 BRK
OP(op,20) {          H6280_CYCLES(7); EA_ABS; JSR;          } // 7 JSR  ABS
OP(op,40) {          H6280_CYCLES(7);         RTI;          } // 7 RTI
OP(op,60) {          H6280_CYCLES(7);         RTS;          } // 7 RTS
OP(op,80) { int tmp;                          BRA(1);       } // 4 BRA  REL
OP(op,a0) { int tmp; H6280_CYCLES(2); RD_IMM; LDY;          } // 2 LDY  IMM
OP(op,c0) { int tmp; H6280_CYCLES(2); RD_IMM; CPY;          } // 2 CPY  IMM
OP(op,e0) { int tmp; H6280_CYCLES(2); RD_IMM; CPX;          } // 2 CPX  IMM

OP(op,10) { int tmp;                          BPL;          } // 2/4 BPL  REL
OP(op,30) { int tmp;                          BMI;          } // 2/4 BMI  REL
OP(op,50) { int tmp;                          BVC;          } // 2/4 BVC  REL
OP(op,70) { int tmp;                          BVS;          } // 2/4 BVS  REL
OP(op,90) { int tmp;                          BCC;          } // 2/4 BCC  REL
OP(op,b0) { int tmp;                          BCS;          } // 2/4 BCS  REL
OP(op,d0) { int tmp;                          BNE;          } // 2/4 BNE  REL
OP(op,f0) { int tmp;                          BEQ;          } // 2/4 BEQ  REL

OP(op,01) { int tmp; H6280_CYCLES(7); RD_IDX; ORA;          } // 7 ORA  IDX
OP(op,21) { int tmp; H6280_CYCLES(7); RD_IDX; AND;          } // 7 AND  IDX
OP(op,41) { int tmp; H6280_CYCLES(7); RD_IDX; EOR;          } // 7 EOR  IDX
OP(op,61) { int tmp; H6280_CYCLES(7); RD_IDX; ADC;          } // 7 ADC  IDX
OP(op,81) { int tmp; H6280_CYCLES(7);         STA;  WR_IDX; } // 7 STA  IDX
OP(op,a1) { int tmp; H6280_CYCLES(7); RD_IDX; LDA;          } // 7 LDA  IDX
OP(op,c1) { int tmp; H6280_CYCLES(7); RD_IDX; CMP;          } // 7 CMP  IDX
OP(op,e1) { int tmp; H6280_CYCLES(7); RD_IDX; SBC;          } // 7 SBC  IDX

OP(op,11) { int tmp; H6280_CYCLES(7); RD_IDY; ORA;          } // 7 ORA  IDY
OP(op,31) { int tmp; H6280_CYCLES(7); RD_IDY; AND;          } // 7 AND  IDY
OP(op,51) { int tmp; H6280_CYCLES(7); RD_IDY; EOR;          } // 7 EOR  IDY
OP(op,71) { int tmp; H6280_CYCLES(7); RD_IDY; ADC;          } // 7 ADC  AZP
OP(op,91) { int tmp; H6280_CYCLES(7);         STA;  WR_IDY; } // 7 STA  IDY
OP(op,b1) { int tmp; H6280_CYCLES(7); RD_IDY; LDA;          } // 7 LDA  IDY
OP(op,d1) { int tmp; H6280_CYCLES(7); RD_IDY; CMP;          } // 7 CMP  IDY
OP(op,f1) { int tmp; H6280_CYCLES(7); RD_IDY; SBC;          } // 7 SBC  IDY

OP(op,02) { int tmp; H6280_CYCLES(3);         SXY;          } // 3 SXY
OP(op,22) { int tmp; H6280_CYCLES(3);         SAX;          } // 3 SAX
OP(op,42) { int tmp; H6280_CYCLES(3);         SAY;          } // 3 SAY
OP(op,62) {          H6280_CYCLES(2);         CLA;          } // 2 CLA
OP(op,82) {          H6280_CYCLES(2);         CLX;          } // 2 CLX
OP(op,a2) { int tmp; H6280_CYCLES(2); RD_IMM; LDX;          } // 2 LDX  IMM
OP(op,c2) {          H6280_CYCLES(2);         CLY;          } // 2 CLY
OP(op,e2) {          H6280_CYCLES(2);         NOP;          } // 2 NOP

OP(op,12) { int tmp; H6280_CYCLES(7); RD_ZPI; ORA;          } // 7 ORA  ZPI
OP(op,32) { int tmp; H6280_CYCLES(7); RD_ZPI; AND;          } // 7 AND  ZPI
OP(op,52) { int tmp; H6280_CYCLES(7); RD_ZPI; EOR;          } // 7 EOR  ZPI
OP(op,72) { int tmp; H6280_CYCLES(7); RD_ZPI; ADC;          } // 7 ADC  ZPI
OP(op,92) { int tmp; H6280_CYCLES(7);         STA;  WR_ZPI; } // 7 STA  ZPI
OP(op,b2) { int tmp; H6280_CYCLES(7); RD_ZPI; LDA;          } // 7 LDA  ZPI
OP(op,d2) { int tmp; H6280_CYCLES(7); RD_ZPI; CMP;          } // 7 CMP  ZPI
OP(op,f2) { int tmp; H6280_CYCLES(7); RD_ZPI; SBC;          } // 7 SBC  ZPI

OP(op,03) { int tmp; H6280_CYCLES(5); RD_IMM; ST0;          } // 4 + 1 penalty cycle ST0  IMM
OP(op,23) { int tmp; H6280_CYCLES(5); RD_IMM; ST2;          } // 4 + 1 penalty cycle ST2  IMM
OP(op,43) { int tmp; H6280_CYCLES(4); RD_IMM; TMA;          } // 4 TMA
OP(op,63) {          H6280_CYCLES(4);         NOP;          } // 2 NOP
OP(op,83) { int tmp,tmp2; H6280_CYCLES(7); RD_IMM2; RD_ZPG; TST; } // 7 TST  IMM,ZPG
OP(op,a3) { int tmp,tmp2; H6280_CYCLES(7); RD_IMM2; RD_ZPX; TST; } // 7 TST  IMM,ZPX
OP(op,c3) { int to,from,length;               TDD;          } // 6*l+17 TDD  XFER
OP(op,e3) { int to,from,length,alternate;     TIA;          } // 6*l+17 TIA  XFER

OP(op,13) { int tmp; H6280_CYCLES(5); RD_IMM; ST1;          } // 4 + 1 penalty cycle ST1
OP(op,33) {          H6280_CYCLES(2);         NOP;          } // 2 NOP
OP(op,53) { int tmp; H6280_CYCLES(5); RD_IMM; TAM;          } // 5 TAM  IMM
OP(op,73) { int to,from,length;               TII;          } // 6*l+17 TII  XFER
OP(op,93) { int tmp,tmp2; H6280_CYCLES(8); RD_IMM2; RD_ABS; TST; } // 8 TST  IMM,ABS
OP(op,b3) { int tmp,tmp2; H6280_CYCLES(8); RD_IMM2; RD_ABX; TST; } // 8 TST  IMM,ABX
OP(op,d3) { int to,from,length;               TIN;          } // 6*l+17 TIN  XFER
OP(op,f3) { int to,from,length,alternate;     TAI;          } // 6*l+17 TAI  XFER

OP(op,04) { int tmp; H6280_CYCLES(6); RD_ZPG; TSB;  WB_EAZ; } // 6 TSB  ZPG
OP(op,24) { int tmp; H6280_CYCLES(4); RD_ZPG; HBIT;         } // 4 BIT  ZPG
OP(op,44) { int tmp;                          BSR;          } // 8 BSR  REL
OP(op,64) { int tmp; H6280_CYCLES(4);         STZ;  WR_ZPG; } // 4 STZ  ZPG
OP(op,84) { int tmp; H6280_CYCLES(4);         STY;  WR_ZPG; } // 4 STY  ZPG
OP(op,a4) { int tmp; H6280_CYCLES(4); RD_ZPG; LDY;          } // 4 LDY  ZPG
OP(op,c4) { int tmp; H6280_CYCLES(4); RD_ZPG; CPY;          } // 4 CPY  ZPG
OP(op,e4) { int tmp; H6280_CYCLES(4); RD_ZPG; CPX;          } // 4 CPX  ZPG

OP(op,14) { int tmp; H6280_CYCLES(6); RD_ZPG; TRB;  WB_EAZ; } // 6 TRB  ZPG
OP(op,34) { int tmp; H6280_CYCLES(4); RD_ZPX; HBIT;         } // 4 BIT  ZPX
OP(op,54) {         H6280_CYCLES(3);          CSL;          } // 3 CSL
OP(op,74) { int tmp; H6280_CYCLES(4);         STZ;  WR_ZPX; } // 4 STZ  ZPX
OP(op,94) { int tmp; H6280_CYCLES(4);         STY;  WR_ZPX; } // 4 STY  ZPX
OP(op,b4) { int tmp; H6280_CYCLES(4); RD_ZPX; LDY;          } // 4 LDY  ZPX
OP(op,d4) {         H6280_CYCLES(3);          CSH;          } // 3 CSH
OP(op,f4) {         H6280_CYCLES(2);          SET;          } // 2 SET

OP(op,05) { int tmp; H6280_CYCLES(4); RD_ZPG; ORA;          } // 4 ORA  ZPG
OP(op,25) { int tmp; H6280_CYCLES(4); RD_ZPG; AND;          } // 4 AND  ZPG
OP(op,45) { int tmp; H6280_CYCLES(4); RD_ZPG; EOR;          } // 4 EOR  ZPG
OP(op,65) { int tmp; H6280_CYCLES(4); RD_ZPG; ADC;          } // 4 ADC  ZPG
OP(op,85) { int tmp; H6280_CYCLES(4);         STA;  WR_ZPG; } // 4 STA  ZPG
OP(op,a5) { int tmp; H6280_CYCLES(4); RD_ZPG; LDA;          } // 4 LDA  ZPG
OP(op,c5) { int tmp; H6280_CYCLES(4); RD_ZPG; CMP;          } // 4 CMP  ZPG
OP(op,e5) { int tmp; H6280_CYCLES(4); RD_ZPG; SBC;          } // 4 SBC  ZPG

OP(op,15) { int tmp; H6280_CYCLES(4); RD_ZPX; ORA;          } // 4 ORA  ZPX
OP(op,35) { int tmp; H6280_CYCLES(4); RD_ZPX; AND;          } // 4 AND  ZPX
OP(op,55) { int tmp; H6280_CYCLES(4); RD_ZPX; EOR;          } // 4 EOR  ZPX
OP(op,75) { int tmp; H6280_CYCLES(4); RD_ZPX; ADC;          } // 4 ADC  ZPX
OP(op,95) { int tmp; H6280_CYCLES(4);         STA;  WR_ZPX; } // 4 STA  ZPX
OP(op,b5) { int tmp; H6280_CYCLES(4); RD_ZPX; LDA;          } // 4 LDA  ZPX
OP(op,d5) { int tmp; H6280_CYCLES(4); RD_ZPX; CMP;          } // 4 CMP  ZPX
OP(op,f5) { int tmp; H6280_CYCLES(4); RD_ZPX; SBC;          } // 4 SBC  ZPX

OP(op,06) { int tmp; H6280_CYCLES(6); RD_ZPG; ASL;  WB_EAZ; } // 6 ASL  ZPG
OP(op,26) { int tmp; H6280_CYCLES(6); RD_ZPG; ROL;  WB_EAZ; } // 6 ROL  ZPG
OP(op,46) { int tmp; H6280_CYCLES(6); RD_ZPG; LSR;  WB_EAZ; } // 6 LSR  ZPG
OP(op,66) { int tmp; H6280_CYCLES(6); RD_ZPG; ROR;  WB_EAZ; } // 6 ROR  ZPG
OP(op,86) { int tmp; H6280_CYCLES(4);         STX;  WR_ZPG; } // 4 STX  ZPG
OP(op,a6) { int tmp; H6280_CYCLES(4); RD_ZPG; LDX;          } // 4 LDX  ZPG
OP(op,c6) { int tmp; H6280_CYCLES(6); RD_ZPG; DEC;  WB_EAZ; } // 6 DEC  ZPG
OP(op,e6) { int tmp; H6280_CYCLES(6); RD_ZPG; INC;  WB_EAZ; } // 6 INC  ZPG

OP(op,16) { int tmp; H6280_CYCLES(6); RD_ZPX; ASL;  WB_EAZ; } // 6 ASL  ZPX
OP(op,36) { int tmp; H6280_CYCLES(6); RD_ZPX; ROL;  WB_EAZ; } // 6 ROL  ZPX
OP(op,56) { int tmp; H6280_CYCLES(6); RD_ZPX; LSR;  WB_EAZ; } // 6 LSR  ZPX
OP(op,76) { int tmp; H6280_CYCLES(6); RD_ZPX; ROR;  WB_EAZ; } // 6 ROR  ZPX
OP(op,96) { int tmp; H6280_CYCLES(4);         STX;  WR_ZPY; } // 4 STX  ZPY
OP(op,b6) { int tmp; H6280_CYCLES(4); RD_ZPY; LDX;          } // 4 LDX  ZPY
OP(op,d6) { int tmp; H6280_CYCLES(6); RD_ZPX; DEC;  WB_EAZ; } // 6 DEC  ZPX
OP(op,f6) { int tmp; H6280_CYCLES(6); RD_ZPX; INC;  WB_EAZ; } // 6 INC  ZPX

OP(op,07) { int tmp; H6280_CYCLES(7); RD_ZPG; RMB(0);WB_EAZ;} // 7 RMB0 ZPG
OP(op,27) { int tmp; H6280_CYCLES(7); RD_ZPG; RMB(2);WB_EAZ;} // 7 RMB2 ZPG
OP(op,47) { int tmp; H6280_CYCLES(7); RD_ZPG; RMB(4);WB_EAZ;} // 7 RMB4 ZPG
OP(op,67) { int tmp; H6280_CYCLES(7); RD_ZPG; RMB(6);WB_EAZ;} // 7 RMB6 ZPG
OP(op,87) { int tmp; H6280_CYCLES(7); RD_ZPG; SMB(0);WB_EAZ;} // 7 SMB0 ZPG
OP(op,a7) { int tmp; H6280_CYCLES(7); RD_ZPG; SMB(2);WB_EAZ;} // 7 SMB2 ZPG
OP(op,c7) { int tmp; H6280_CYCLES(7); RD_ZPG; SMB(4);WB_EAZ;} // 7 SMB4 ZPG
OP(op,e7) { int tmp; H6280_CYCLES(7); RD_ZPG; SMB(6);WB_EAZ;} // 7 SMB6 ZPG

OP(op,17) { int tmp; H6280_CYCLES(7); RD_ZPG; RMB(1);WB_EAZ;} // 7 RMB1 ZPG
OP(op,37) { int tmp; H6280_CYCLES(7); RD_ZPG; RMB(3);WB_EAZ;} // 7 RMB3 ZPG
OP(op,57) { int tmp; H6280_CYCLES(7); RD_ZPG; RMB(5);WB_EAZ;} // 7 RMB5 ZPG
OP(op,77) { int tmp; H6280_CYCLES(7); RD_ZPG; RMB(7);WB_EAZ;} // 7 RMB7 ZPG
OP(op,97) { int tmp; H6280_CYCLES(7); RD_ZPG; SMB(1);WB_EAZ;} // 7 SMB1 ZPG
OP(op,b7) { int tmp; H6280_CYCLES(7); RD_ZPG; SMB(3);WB_EAZ;} // 7 SMB3 ZPG
OP(op,d7) { int tmp; H6280_CYCLES(7); RD_ZPG; SMB(5);WB_EAZ;} // 7 SMB5 ZPG
OP(op,f7) { int tmp; H6280_CYCLES(7); RD_ZPG; SMB(7);WB_EAZ;} // 7 SMB7 ZPG

OP(op,08) {          H6280_CYCLES(3);         PHP;          } // 3 PHP
OP(op,28) {          H6280_CYCLES(4);         PLP;          } // 4 PLP
OP(op,48) {          H6280_CYCLES(3);         PHA;          } // 3 PHA
OP(op,68) {          H6280_CYCLES(4);         PLA;          } // 4 PLA
OP(op,88) {          H6280_CYCLES(2);         DEY;          } // 2 DEY
OP(op,a8) {          H6280_CYCLES(2);         TAY;          } // 2 TAY
OP(op,c8) {          H6280_CYCLES(2);         INY;          } // 2 INY
OP(op,e8) {          H6280_CYCLES(2);         INX;          } // 2 INX

OP(op,18) {          H6280_CYCLES(2);         CLC;          } // 2 CLC
OP(op,38) {          H6280_CYCLES(2);         SEC;          } // 2 SEC
OP(op,58) {          H6280_CYCLES(2);         CLI;          } // 2 CLI
OP(op,78) {          H6280_CYCLES(2);         SEI;          } // 2 SEI
OP(op,98) {          H6280_CYCLES(2);         TYA;          } // 2 TYA
OP(op,b8) {          H6280_CYCLES(2);         CLV;          } // 2 CLV
OP(op,d8) {          H6280_CYCLES(2);         CLD;          } // 2 CLD
OP(op,f8) {          H6280_CYCLES(2);         SED;          } // 2 SED

OP(op,09) { int tmp; H6280_CYCLES(2); RD_IMM; ORA;          } // 2 ORA  IMM
OP(op,29) { int tmp; H6280_CYCLES(2); RD_IMM; AND;          } // 2 AND  IMM
OP(op,49) { int tmp; H6280_CYCLES(2); RD_IMM; EOR;          } // 2 EOR  IMM
OP(op,69) { int tmp; H6280_CYCLES(2); RD_IMM; ADC;          } // 2 ADC  IMM
OP(op,89) { int tmp; H6280_CYCLES(2); RD_IMM; HBIT;         } // 2 BIT  IMM
OP(op,a9) { int tmp; H6280_CYCLES(2); RD_IMM; LDA;          } // 2 LDA  IMM
OP(op,c9) { int tmp; H6280_CYCLES(2); RD_IMM; CMP;          } // 2 CMP  IMM
OP(op,e9) { int tmp; H6280_CYCLES(2); RD_IMM; SBC;          } // 2 SBC  IMM

OP(op,19) { int tmp; H6280_CYCLES(5); RD_ABY; ORA;          } // 5 ORA  ABY
OP(op,39) { int tmp; H6280_CYCLES(5); RD_ABY; AND;          } // 5 AND  ABY
OP(op,59) { int tmp; H6280_CYCLES(5); RD_ABY; EOR;          } // 5 EOR  ABY
OP(op,79) { int tmp; H6280_CYCLES(5); RD_ABY; ADC;          } // 5 ADC  ABY
OP(op,99) { int tmp; H6280_CYCLES(5);         STA;  WR_ABY; } // 5 STA  ABY
OP(op,b9) { int tmp; H6280_CYCLES(5); RD_ABY; LDA;          } // 5 LDA  ABY
OP(op,d9) { int tmp; H6280_CYCLES(5); RD_ABY; CMP;          } // 5 CMP  ABY
OP(op,f9) { int tmp; H6280_CYCLES(5); RD_ABY; SBC;          } // 5 SBC  ABY

OP(op,0a) { int tmp; H6280_CYCLES(2); RD_ACC; ASL;  WB_ACC; } // 2 ASL  A
OP(op,2a) { int tmp; H6280_CYCLES(2); RD_ACC; ROL;  WB_ACC; } // 2 ROL  A
OP(op,4a) { int tmp; H6280_CYCLES(2); RD_ACC; LSR;  WB_ACC; } // 2 LSR  A
OP(op,6a) { int tmp; H6280_CYCLES(2); RD_ACC; ROR;  WB_ACC; } // 2 ROR  A
OP(op,8a) {         H6280_CYCLES(2);          TXA;          } // 2 TXA
OP(op,aa) {         H6280_CYCLES(2);          TAX;          } // 2 TAX
OP(op,ca) {         H6280_CYCLES(2);          DEX;          } // 2 DEX
OP(op,ea) {         H6280_CYCLES(2);          NOP;          } // 2 NOP

OP(op,1a) {         H6280_CYCLES(2);          INA;          } // 2 INC  A
OP(op,3a) {         H6280_CYCLES(2);          DEA;          } // 2 DEC  A
OP(op,5a) {         H6280_CYCLES(3);          PHY;          } // 3 PHY
OP(op,7a) {         H6280_CYCLES(4);          PLY;          } // 4 PLY
OP(op,9a) {         H6280_CYCLES(2);          TXS;          } // 2 TXS
OP(op,ba) {         H6280_CYCLES(2);          TSX;          } // 2 TSX
OP(op,da) {         H6280_CYCLES(3);          PHX;          } // 3 PHX
OP(op,fa) {         H6280_CYCLES(4);          PLX;          } // 4 PLX

OP(op,0b) {          H6280_CYCLES(2);         NOP;          } // 2 NOP
OP(op,2b) {          H6280_CYCLES(2);         NOP;          } // 2 NOP
OP(op,4b) {          H6280_CYCLES(2);         NOP;          } // 2 NOP
OP(op,6b) {          H6280_CYCLES(2);         NOP;          } // 2 NOP
OP(op,8b) {          H6280_CYCLES(2);         NOP;          } // 2 NOP
OP(op,ab) {          H6280_CYCLES(2);         NOP;          } // 2 NOP
OP(op,cb) {          H6280_CYCLES(2);         NOP;          } // 2 NOP
OP(op,eb) {          H6280_CYCLES(2);         NOP;          } // 2 NOP

OP(op,1b) {          H6280_CYCLES(2);         NOP;          } // 2 NOP
OP(op,3b) {          H6280_CYCLES(2);         NOP;          } // 2 NOP
OP(op,5b) {          H6280_CYCLES(2);         NOP;          } // 2 NOP
OP(op,7b) {          H6280_CYCLES(2);         NOP;          } // 2 NOP
OP(op,9b) {          H6280_CYCLES(2);         NOP;          } // 2 NOP
OP(op,bb) {          H6280_CYCLES(2);         NOP;          } // 2 NOP
OP(op,db) {          H6280_CYCLES(2);         NOP;          } // 2 NOP
OP(op,fb) {          H6280_CYCLES(2);         NOP;          } // 2 NOP

OP(op,0c) { int tmp; H6280_CYCLES(7); RD_ABS; TSB;  WB_EA;  } // 7 TSB  ABS
OP(op,2c) { int tmp; H6280_CYCLES(5); RD_ABS; HBIT;         } // 5 BIT  ABS
OP(op,4c) {          H6280_CYCLES(4); EA_ABS; JMP;          } // 4 JMP  ABS
OP(op,6c) { int tmp; H6280_CYCLES(7); EA_IND; JMP;          } // 7 JMP  IND
OP(op,8c) { int tmp; H6280_CYCLES(5);         STY;  WR_ABS; } // 5 STY  ABS
OP(op,ac) { int tmp; H6280_CYCLES(5); RD_ABS; LDY;          } // 5 LDY  ABS
OP(op,cc) { int tmp; H6280_CYCLES(5); RD_ABS; CPY;          } // 5 CPY  ABS
OP(op,ec) { int tmp; H6280_CYCLES(5); RD_ABS; CPX;          } // 5 CPX  ABS

OP(op,1c) { int tmp; H6280_CYCLES(7); RD_ABS; TRB;  WB_EA;  } // 7 TRB  ABS
OP(op,3c) { int tmp; H6280_CYCLES(5); RD_ABX; HBIT;         } // 5 BIT  ABX
OP(op,5c) {          H6280_CYCLES(2);         NOP;          } // 2 NOP
OP(op,7c) { int tmp; H6280_CYCLES(7); EA_IAX; JMP;          } // 7 JMP  IAX
OP(op,9c) { int tmp; H6280_CYCLES(5);         STZ;  WR_ABS; } // 5 STZ  ABS
OP(op,bc) { int tmp; H6280_CYCLES(5); RD_ABX; LDY;          } // 5 LDY  ABX
OP(op,dc) {          H6280_CYCLES(2);         NOP;          } // 2 NOP
OP(op,fc) {          H6280_CYCLES(2);         NOP;          } // 2 NOP

OP(op,0d) { int tmp; H6280_CYCLES(5); RD_ABS; ORA;          } // 5 ORA  ABS
OP(op,2d) { int tmp; H6280_CYCLES(5); RD_ABS; AND;          } // 5 AND  ABS
OP(op,4d) { int tmp; H6280_CYCLES(5); RD_ABS; EOR;          } // 5 EOR  ABS
OP(op,6d) { int tmp; H6280_CYCLES(5); RD_ABS; ADC;          } // 5 ADC  ABS
OP(op,8d) { int tmp; H6280_CYCLES(5);         STA;  WR_ABS; } // 5 STA  ABS
OP(op,ad) { int tmp; H6280_CYCLES(5); RD_ABS; LDA;          } // 5 LDA  ABS
OP(op,cd) { int tmp; H6280_CYCLES(5); RD_ABS; CMP;          } // 5 CMP  ABS
OP(op,ed) { int tmp; H6280_CYCLES(5); RD_ABS; SBC;          } // 5 SBC  ABS

OP(op,1d) { int tmp; H6280_CYCLES(5); RD_ABX; ORA;          } // 5 ORA  ABX
OP(op,3d) { int tmp; H6280_CYCLES(5); RD_ABX; AND;          } // 5 AND  ABX
OP(op,5d) { int tmp; H6280_CYCLES(5); RD_ABX; EOR;          } // 5 EOR  ABX
OP(op,7d) { int tmp; H6280_CYCLES(5); RD_ABX; ADC;          } // 5 ADC  ABX
OP(op,9d) { int tmp; H6280_CYCLES(5);         STA;  WR_ABX; } // 5 STA  ABX
OP(op,bd) { int tmp; H6280_CYCLES(5); RD_ABX; LDA;          } // 5 LDA  ABX
OP(op,dd) { int tmp; H6280_CYCLES(5); RD_ABX; CMP;          } // 5 CMP  ABX
OP(op,fd) { int tmp; H6280_CYCLES(5); RD_ABX; SBC;          } // 5 SBC  ABX

OP(op,0e) { int tmp; H6280_CYCLES(7); RD_ABS; ASL;  WB_EA;  } // 7 ASL  ABS
OP(op,2e) { int tmp; H6280_CYCLES(7); RD_ABS; ROL;  WB_EA;  } // 7 ROL  ABS
OP(op,4e) { int tmp; H6280_CYCLES(7); RD_ABS; LSR;  WB_EA;  } // 7 LSR  ABS
OP(op,6e) { int tmp; H6280_CYCLES(7); RD_ABS; ROR;  WB_EA;  } // 7 ROR  ABS
OP(op,8e) { int tmp; H6280_CYCLES(5);         STX;  WR_ABS; } // 5 STX  ABS
OP(op,ae) { int tmp; H6280_CYCLES(5); RD_ABS; LDX;          } // 5 LDX  ABS
OP(op,ce) { int tmp; H6280_CYCLES(7); RD_ABS; DEC;  WB_EA;  } // 7 DEC  ABS
OP(op,ee) { int tmp; H6280_CYCLES(7); RD_ABS; INC;  WB_EA;  } // 7 INC  ABS

OP(op,1e) { int tmp; H6280_CYCLES(7); RD_ABX; ASL;  WB_EA;  } // 7 ASL  ABX
OP(op,3e) { int tmp; H6280_CYCLES(7); RD_ABX; ROL;  WB_EA;  } // 7 ROL  ABX
OP(op,5e) { int tmp; H6280_CYCLES(7); RD_ABX; LSR;  WB_EA;  } // 7 LSR  ABX
OP(op,7e) { int tmp; H6280_CYCLES(7); RD_ABX; ROR;  WB_EA;  } // 7 ROR  ABX
OP(op,9e) { int tmp; H6280_CYCLES(5);         STZ;  WR_ABX; } // 5 STZ  ABX
OP(op,be) { int tmp; H6280_CYCLES(5); RD_ABY; LDX;          } // 5 LDX  ABY
OP(op,de) { int tmp; H6280_CYCLES(7); RD_ABX; DEC;  WB_EA;  } // 7 DEC  ABX
OP(op,fe) { int tmp; H6280_CYCLES(7); RD_ABX; INC;  WB_EA;  } // 7 INC  ABX

OP(op,0f) { int tmp; H6280_CYCLES(4); RD_ZPG; BBR(0);       } // 6/8 BBR0 ZPG,REL
OP(op,2f) { int tmp; H6280_CYCLES(4); RD_ZPG; BBR(2);       } // 6/8 BBR2 ZPG,REL
OP(op,4f) { int tmp; H6280_CYCLES(4); RD_ZPG; BBR(4);       } // 6/8 BBR4 ZPG,REL
OP(op,6f) { int tmp; H6280_CYCLES(4); RD_ZPG; BBR(6);       } // 6/8 BBR6 ZPG,REL
OP(op,8f) { int tmp; H6280_CYCLES(4); RD_ZPG; BBS(0);       } // 6/8 BBS0 ZPG,REL
OP(op,af) { int tmp; H6280_CYCLES(4); RD_ZPG; BBS(2);       } // 6/8 BBS2 ZPG,REL
OP(op,cf) { int tmp; H6280_CYCLES(4); RD_ZPG; BBS(4);       } // 6/8 BBS4 ZPG,REL
OP(op,ef) { int tmp; H6280_CYCLES(4); RD_ZPG; BBS(6);       } // 6/8 BBS6 ZPG,REL

OP(op,1f) { int tmp; H6280_CYCLES(4); RD_ZPG; BBR(1);       } // 6/8 BBR1 ZPG,REL
OP(op,3f) { int tmp; H6280_CYCLES(4); RD_ZPG; BBR(3);       } // 6/8 BBR3 ZPG,REL
OP(op,5f) { int tmp; H6280_CYCLES(4); RD_ZPG; BBR(5);       } // 6/8 BBR5 ZPG,REL
OP(op,7f) { int tmp; H6280_CYCLES(4); RD_ZPG; BBR(7);       } // 6/8 BBR7 ZPG,REL
OP(op,9f) { int tmp; H6280_CYCLES(4); RD_ZPG; BBS(1);       } // 6/8 BBS1 ZPG,REL
OP(op,bf) { int tmp; H6280_CYCLES(4); RD_ZPG; BBS(3);       } // 6/8 BBS3 ZPG,REL
OP(op,df) { int tmp; H6280_CYCLES(4); RD_ZPG; BBS(5);       } // 6/8 BBS5 ZPG,REL
OP(op,ff) { int tmp; H6280_CYCLES(4); RD_ZPG; BBS(7);       } // 6/8 BBS7 ZPG,REL

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
					CHECK_AND_TAKE_IRQ_LINES;
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
		CHECK_IRQ_LINES;
	}
	else if (irqline < 3)
	{
		/* If the state has not changed, just return */
		if (m_irq_state[irqline] == state)
			return;

		m_irq_state[irqline] = state;

		CHECK_IRQ_LINES;
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
			CHECK_IRQ_LINES;
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
			{   /* stop -> start causes reload */
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
