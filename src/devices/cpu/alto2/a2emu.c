// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/*****************************************************************************
 *
 *   Xerox AltoII emulator task
 *
 *****************************************************************************/
#include "alto2cpu.h"

/** @brief CTL2K_U3 address line for F2 function */
#define CTL2K_U3(f2) (f2 == f2_emu_idisp ? 0x80 : 0x00)

/**
 * width,from,to of the 16 bit instruction register
 *                     1 1 1 1 1 1
 * 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
 * =============================================================
 * x - - - - - - - - - - - - - - - arithmetic operation
 * 0 m m - - - - - - - - - - - - - memory function
 * 0 0 0 - - - - - - - - - - - - - jump functions
 * 0 0 1 d d - - - - - - - - - - - LDA dstAC
 * 0 1 0 d d - - - - - - - - - - - STA dstAC
 * 0 1 1 - - - - - - - - - - - - - augmented functions
 * 1 s s - - - - - - - - - - - - - source accumulator (0-3)
 * 1 - - d d - - - - - - - - - - - destination accumulator (0-3)
 * 1 s s d d x x x - - - - - - - - accumulator function
 * 1 s s d d 0 0 0 - - - - - - - - COM dstAC, srcAC
 * 1 s s d d 0 0 1 - - - - - - - - NEG dstAC, srcAC
 * 1 s s d d 0 1 0 - - - - - - - - MOV dstAC, srcAC
 * 1 s s d d 0 1 1 - - - - - - - - INC dstAC, srcAC
 * 1 s s d d 1 0 0 - - - - - - - - ADC dstAC, srcAC
 * 1 s s d d 1 0 1 - - - - - - - - SUB dstAC, srcAC
 * 1 s s d d 1 1 0 - - - - - - - - ADD dstAC, srcAC
 * 1 s s d d 1 1 1 - - - - - - - - AND dstAC, srcAC
 * 1 - - - - - - - x x - - - - - - shift operation
 * 1 - - - - - - - 0 0 - - - - - - nothing
 * 1 - - - - - - - 0 1 - - - - - - rotate left through carry
 * 1 - - - - - - - 1 0 - - - - - - rotate right through carry
 * 1 - - - - - - - 1 1 - - - - - - swap byte halves
 * 1 - - - - - - - - - x x - - - - carry in mode
 * 1 - - - - - - - - - 0 0 - - - - nothing
 * 1 - - - - - - - - - 0 1 - - - - Z carry in is zero
 * 1 - - - - - - - - - 1 0 - - - - O carry in is one
 * 1 - - - - - - - - - 1 1 - - - - C carry in is complemented carry
 * 1 - - - - - - - - - - - x - - - NL
 * - - - - - - - - - - - - - x x x conditional execution
 * - - - - - - - - - - - - - 0 0 0 NVR never skip
 * - - - - - - - - - - - - - 0 0 1 SKP always skip
 * - - - - - - - - - - - - - 0 1 0 SZC skip if carry result is zero
 * - - - - - - - - - - - - - 0 1 1 SNC skip if carry result is non-zero
 * - - - - - - - - - - - - - 1 0 0 SZR skip if 16 bit result is zero
 * - - - - - - - - - - - - - 1 0 1 SNR skip if 16 bit result is non-zero
 * - - - - - - - - - - - - - 1 1 0 SEZ skip if either result is zero
 * - - - - - - - - - - - - - 1 1 1 SBN skip if both results are non-zero
 */
#define IR_ARITH(ir)    X_RDBITS(ir,16, 0, 0)
#define IR_SrcAC(ir)    X_RDBITS(ir,16, 1, 2)
#define IR_DstAC(ir)    X_RDBITS(ir,16, 3, 4)
#define IR_AFunc(ir)    X_RDBITS(ir,16, 5, 7)
#define IR_SH(ir)       X_RDBITS(ir,16, 8, 9)
#define IR_CY(ir)       X_RDBITS(ir,16,10,11)
#define IR_NL(ir)       X_RDBITS(ir,16,12,12)
#define IR_SK(ir)       X_RDBITS(ir,16,13,15)

#define IR_MFunc(ir)    X_RDBITS(ir,16, 1, 2)
#define IR_JFunc(ir)    X_RDBITS(ir,16, 3, 4)
#define IR_I(ir)        X_RDBITS(ir,16, 5, 5)
#define IR_X(ir)        X_RDBITS(ir,16, 6, 7)
#define IR_DISP(ir)     X_RDBITS(ir,16, 8,15)
#define IR_AUGFUNC(ir)  X_RDBITS(ir,16, 3, 7)

#define op_MFUNC_MASK   0060000     //!< instruction register memory function mask
#define op_MFUNC_JUMP   0000000     //!< jump functions value
#define op_JUMP_MASK    0014000     //!< jump functions mask
#define op_JMP          0000000     //!< jump
#define op_JSR          0004000     //!< jump to subroutine
#define op_ISZ          0010000     //!< increment and skip if zero
#define op_DSZ          0014000     //!< decrement and skip if zero
#define op_LDA          0020000     //!< load accu functions value
#define op_STA          0040000     //!< store accu functions value
#define op_AUGMENTED    0060000     //!< store accu functions value
#define op_AUGM_MASK    0077400     //!< mask covering all augmented functions
#define op_AUGM_NODISP  0061000     //!< augmented functions w/o displacement
#define op_AUGM_SUBFUNC 0000037     //!< mask for augmented subfunctions in DISP
#define op_CYCLE        0060000     //!< cycle AC0
#define op_NODISP       0061000     //!< NODISP: opcodes without displacement
#define op_DIR          0061000     //!< disable interrupts
#define op_EIR          0061001     //!< enable interrupts
#define op_BRI          0061002     //!< branch and return from interrupt
#define op_RCLK         0061003     //!< read clock to AC0, AC1
#define op_SIO          0061004     //!< start I/O
#define op_BLT          0061005     //!< block transfer
#define op_BLKS         0061006     //!< block set value
#define op_SIT          0061007     //!< start interval timer
#define op_JMPRAM       0061010     //!< jump to microcode RAM (actually ROM, too)
#define op_RDRAM        0061011     //!< read microcode RAM
#define op_WRTRAM       0061012     //!< write microcode RAM
#define op_DIRS         0061013     //!< disable interrupts, and skip, if already disabled
#define op_VERS         0061014     //!< get microcode version in AC0
#define op_DREAD        0061015     //!< double word read (Alto II)
#define op_DWRITE       0061016     //!< double word write (Alto II)
#define op_DEXCH        0061017     //!< double word exchange (Alto II)
#define op_MUL          0061020     //!< unsigned multiply
#define op_DIV          0061021     //!< unsigned divide
#define op_DIAGNOSE1    0061022     //!< write two different accus in fast succession
#define op_DIAGNOSE2    0061023     //!< write Hamming code and memory
#define op_BITBLT       0061024     //!< bit-aligned block transfer
#define op_XMLDA        0061025     //!< load accu AC0 from extended memory (Alto II/XM)
#define op_XMSTA        0061026     //!< store accu AC0 to extended memory (Alto II/XM)
#define op_JSRII        0064400     //!< jump to subroutine PC relative, doubly indirect
#define op_JSRIS        0065000     //!< jump to subroutine AC2 relative, doubly indirect
#define op_CONVERT      0067000     //!< convert bitmapped font to bitmap
#define op_ARITH_MASK   0103400     //!< mask for arithmetic functions
#define op_COM          0100000     //!< one's complement
#define op_NEG          0100400     //!< two's complement
#define op_MOV          0101000     //!< accu transfer
#define op_INC          0101400     //!< increment
#define op_ADC          0102000     //!< add one's complement
#define op_SUB          0102400     //!< subtract by adding two's complement
#define op_ADD          0103000     //!< add
#define op_AND          0103400     //!< logical and

#define ea_DIRECT       0000000     //!< effective address is direct
#define ea_INDIRECT     0002000     //!< effective address is indirect
#define ea_MASK         0001400     //!< mask for effective address modes
#define ea_PAGE0        0000000     //!< e is page 0 address
#define ea_PCREL        0000400     //!< e is PC + signed displacement
#define ea_AC2REL       0001000     //!< e is AC2 + signed displacement
#define ea_AC3REL       0001400     //!< e is AC3 + signed displacement


#define sh_MASK         0000300     //!< shift mode mask (do novel shifts)
#define sh_L            0000100     //!< rotate left through carry
#define sh_R            0000200     //!< rotate right through carry
#define sh_S            0000300     //!< swap byte halves

#define cy_MASK         0000060     //!< carry in mode mask
#define cy_Z            0000020     //!< carry in is zero
#define cy_O            0000040     //!< carry in is one
#define cy_C            0000060     //!< carry in is complemented carry

#define nl_MASK         0000010     //!< no-load mask
#define nl_NONE         0000010     //!< do not load DstAC nor carry

#define sk_MASK         0000007     //!< skip mask
#define sk_NVR          0000000     //!< never skip
#define sk_SKP          0000001     //!< always skip
#define sk_SZC          0000002     //!< skip if carry result is zero
#define sk_SNC          0000003     //!< skip if carry result is non-zero
#define sk_SZR          0000004     //!< skip if 16-bit result is zero
#define sk_SNR          0000005     //!< skip if 16-bit result is non-zero
#define sk_SEZ          0000006     //!< skip if either result is zero
#define sk_SBN          0000007     //!< skip if both results are non-zero

/**
 * @brief register selection
 *
 * <PRE>
 * From the schematics: 08_ALU, page 6 (PDF page 4)
 *
 * EMACT            emulator task active
 * F2[0-2]=111b     <-ACSOURCE and F2_17
 * F2[0-2]=101b     DNS<- and ACDEST<-
 *
 *  u49 (8 input NAND 74S30)
 *  ----------------------------------------------
 *  F2[0] & F2[2] & F2[1]' & IR[03]' & EMACT
 *
 *      F2[0-2] IR[03]  EMACT   output u49pin8
 *      --------------------------------------
 *      101     0       1       0
 *      all others              1
 *
 *
 *  u59 (8 input NAND 74S30)
 *  ----------------------------------------------
 *  F2[0] & F2[2] & F2[1] & IR[01]' & EMACT
 *
 *      F2[0-2] IR[01] EMACT    output u59pin8
 *      --------------------------------------
 *      111     0      1        0
 *      all others              1
 *
 *  u70d (2 input NOR 74S02 used as inverter)
 *  ---------------------------------------------
 *  RSEL3 -> RSEL3'
 *
 *  u79b (3 input NAND 74S10)
 *  ---------------------------------------------
 *      u49pin8 u59pin8 RSEL3'  output 6RA3
 *      -------------------------------------
 *      1       1       1       0
 *      0       x       x       1
 *      x       0       x       1
 *      x       x       0       1
 *
 *
 *  u60 (8 input NAND 74S30)
 *  ----------------------------------------------
 *  F2[0] & F2[2] & F2[1]' & IR[02]' & EMACT
 *
 *      F2[0-2] IR[02]  EMACT   output u60pin8
 *      --------------------------------------
 *      101     0       1       0
 *      all others              1
 *
 *  u50 (8 input NAND 74S30)
 *  ----------------------------------------------
 *  F2[0] & F2[2] & F2[1] & IR[04]' & EMACT
 *
 *      F2[0-2] IR[04]  EMACT   output u50pin8
 *      --------------------------------------
 *      111     0       1       0
 *      all others              1
 *
 *  u70c (2 input NOR 74S02 used as inverter)
 *  ---------------------------------------------
 *  RSEL4 -> RSEL4'
 *
 *
 *  u79c (3 input NAND 74S10)
 *  ---------------------------------------------
 *  u60pin8 u50pin8 RSEL4'  output 8RA4
 *  -------------------------------------
 *  1       1       1       0
 *  0       x       x       1
 *  x       0       x       1
 *  x       x       0       1
 *
 * BUG?: schematics seem to have swapped IR(04)' and IR(02)' inputs for the
 * RA4 decoding, because SrcAC is selected from IR[1-2]?
 * </PRE>
 */

/**
 * @brief bs_disp early: drive bus by IR[8-15], possibly sign extended
 *
 * The high order bits of IR cannot be read directly, but the
 * displacement field of IR (8 low order bits) may be read with
 * the <-DISP bus source. If the X field of the instruction is
 * zero (i.e., it specifies page 0 addressing), then the DISP
 * field of the instruction is put on BUS[8-15] and BUS[0-7]
 * is zeroed. If the X field of the instruction is non-zero
 * (i.e. it specifies PC-relative or base-register addressing)
 * then the DISP field is sign-extended and put on the bus.
 *
 */
void alto2_cpu_device::bs_early_emu_disp()
{
	UINT16 r = IR_DISP(m_emu.ir);
	if (IR_X(m_emu.ir)) {
		r = ((signed char)r) & 0177777;
	}
	LOG((this,LOG_EMU,2, "   <-DISP (%06o)\n", r));
	m_bus &= r;
}

/**
 * @brief f1_block early: block task
 *
 * The task request for the active task is cleared
 */
void alto2_cpu_device::f1_early_emu_block()
{
#if 0
	CPU_CLR_TASK_WAKEUP(m_task);
	LOG((this,LOG_EMU,2, "   BLOCK %02o:%s\n", m_task, task_name(m_task)));
#elif   0
	fatal(1, "Emulator task want's to BLOCK.\n" \
		"%s-%04o: r:%02o af:%02o bs:%02o f1:%02o f2:%02o" \
		" t:%o l:%o next:%05o next2:%05o cycle:%lld\n",
		task_name(m_task), m_mpc,
		m_rsel, m_daluf, m_dbs, m_df1, mdf2,
		m_dloadt, m_dloatl, m_next, m_next2,
		ntime() / CPU_MICROCYCLE_TIME);
#else
	/* just ignore (?) */
#endif
}

/**
 * @brief f1_load_rmr late: load the reset mode register
 */
void alto2_cpu_device::f1_late_emu_load_rmr()
{
	LOG((this,LOG_EMU,2,"    RMR<-; BUS (%#o)\n", m_bus));
	m_reset_mode = m_bus;
}

/**
 * @brief f1_load_esrb late: load the extended S register bank from BUS[12-14]
 */
void alto2_cpu_device::f1_late_emu_load_esrb()
{
	LOG((this,LOG_EMU,2,"    ESRB<-; BUS[12-14] (%#o)\n", m_bus));
	m_s_reg_bank[m_task] = X_RDBITS(m_bus,16,12,14);
}

/**
 * @brief f1_rsnf early: drive the bus from the Ethernet node ID
 *
 * TODO: move this to the Ethernet code? It's really a emulator
 * specific function that is decoded by the Ethernet card.
 */
void alto2_cpu_device::f1_early_rsnf()
{
	UINT16 r = 0177400 | m_ether_id;
	LOG((this,LOG_EMU,2,"    <-RSNF; (%#o)\n", r));
	m_bus &= r;
}

/**
 * @brief f1_startf early: defines commands for for I/O hardware, including Ethernet
 * <PRE>
 * (SIO) Start I/O is included to facilitate I/O control, It places the contents of
 * AC0 on the processor bus and executes the STARTF function (F1 = 17B). By convention,
 * bits of AC0 must be "1" in order to signal devices. See Appendix C for a summary of
 * assigned bits.
 *    Bit 0  100000B   Standard Alto: Software boot feature
 *    Bit 14 000002B   Standard Alto: Ethernet
 *    Bit 15 000001B   Standard Alto: Ethernet
 * If bit 0 of AC0 is 1, and if an Ethernet board is plugged into the Alto, the machine
 * will boot, just as if the "boot button" were pressed (see sections 3.4, 8.4 and 9.2.2
 * for discussions of bootstrapping).
 *
 * SIO also returns a result in AC0. If the Ethernet hardware is installed, the serial
 * number and/or Ethernet host address of the machine (0-377B) is loaded into AC0[8-15].
 * (On Alto I, the serial number and Ethernet host address are equivalent; on Alto II,
 * the value loaded into AC0 is the Ethernet host address only.) If Ethernet hardware
 * is missing, AC0[8-15] = 377B. Microcode installed after June 1976, which this manual
 * describes, returns AC0[0] = 0. Microcode installed prior to June 1976 returns
 * AC0[0] = 1; this is a quick way to acquire the approximate vintage of a machine's
 * microcode.
 * </PRE>
 *
 * TODO: move this to the Ethernet code? It's really a emulator
 * specific function that is decoded by the Ethernet card.
 */
void alto2_cpu_device::f1_early_startf()
{
	LOG((this,LOG_EMU,2,"    STARTF (BUS is %06o)\n", m_bus));
	/* TODO: what do we do here? reset the CPU on bit 0? */
	if (X_BIT(m_bus,16,0)) {
		LOG((this,LOG_EMU,2,"****    Software boot feature\n"));
		soft_reset();
	} else {
		LOG((this,LOG_EMU,2,"****    Ethernet start function\n"));
		eth_startf();
	}
}

/**
 * @brief branch on odd bus
 */
void alto2_cpu_device::f2_late_busodd()
{
	UINT16 r = m_bus & 1;
	LOG((this,LOG_EMU,2,"    BUSODD; %sbranch (%#o|%#o)\n", r ? "" : "no ", m_next2, r));
	m_next2 |= r;
}

/**
 * @brief f2_magic late: shift and use T[0] or T[15] for bit 15 or 0
 */
void alto2_cpu_device::f2_late_magic()
{
	int XC;
	switch (m_d_f1) {
	case f1_l_lsh_1:    // <-L MLSH 1
		XC = (m_t >> 15) & 1;
		m_shifter = (m_l << 1) | XC;
		LOG((this,LOG_EMU,2,"    <-L MLSH 1 (shifer:%06o XC:%o)", m_shifter, XC));
		break;
	case f1_l_rsh_1:    // <-L MRSH 1
		XC = (m_t & 1) << 15;
		m_shifter = (m_l >> 1) | XC;
		LOG((this,LOG_EMU,2,"    <-L MRSH 1 (shifter:%06o XC:%o)", m_shifter, XC));
		break;
	case f1_l_lcy_8:    // <-L LCY 8
		m_shifter = (m_l >> 8) | (m_l << 8);
		break;
	default:            // other
		m_shifter = m_l;
		break;
	}
}

/**
 * @brief do novel shifts: modify RESELECT with DstAC = (3 - IR[3-4])
 */
void alto2_cpu_device::f2_early_load_dns()
{
	X_WRBITS(m_rsel, 5, 3, 4, IR_DstAC(m_emu.ir) ^ 3);
	LOG((this,LOG_EMU,2,"    DNS<-; rsel := DstAC (%#o %s)\n", m_rsel, r_name(m_rsel)));
}

/**
 * @brief do novel shifts
 *
 * <PRE>
 * New emulator carry is selected by instruction register
 * bits CY = IR[10-11]. R register and emulator carry are
 * loaded only if NL = IR[12] is 0 (NL = no load).
 * SKIP is set according to SK = IR[13-15].
 *
 *  CARRY     = !m_emu.cy
 *  exorB     = IR11 ^ IR10
 *  ORA       = !(exorB | CARRY)
 *            = (exorB | CARRY) ^ 1
 *  exorC     = ORA ^ !IR11
 *            = ORA ^ IR11 ^ 1
 *  exorD     = exorC ^ LALUC0
 *  XC        = !(!(DNS & exorD) & !(MAGIC & OUTza))
 *            = (DNS & exorD) | (MAGIC & OUTza)
 *            = exorD, because this is DNS
 *  NEWCARRY  = [XC, L(00), L(15), XC] for F1 = no shift, <-L RSH 1, <-L LSH 1, LCY 8
 *  SHZERO    = shifter == 0
 *  DCARRY    = !((!IR12 & NEWCARRY) | (IR12 & CARRY))
 *            = (((IR12 ^ 1) & NEWCARRY) | (IR12 & CARRY)) ^ 1
 *  DSKIP     = !((!NEWCARRY & IR14) | (SHZERO & IR13)) ^ !IR15
 *            = ((((NEWCARRY ^ 1) & IR14) | (SHZERO & IR13)) ^ 1) ^ (IR15 ^ 1)
 *            = (((NEWCARRY ^ 1) & IR14) | (SHZERO & IR13)) ^ IR15
 * </PRE>
 */
void alto2_cpu_device::f2_late_load_dns()
{
	UINT8 IR10 = X_BIT(m_emu.ir,16,10);
	UINT8 IR11 = X_BIT(m_emu.ir,16,11);
	UINT8 IR12 = X_BIT(m_emu.ir,16,12);
	UINT8 IR13 = X_BIT(m_emu.ir,16,13);
	UINT8 IR14 = X_BIT(m_emu.ir,16,14);
	UINT8 IR15 = X_BIT(m_emu.ir,16,15);
	UINT8 exorB = IR11 ^ IR10;
	UINT8 CARRY = m_emu.cy ^ 1;
	UINT8 ORA = (exorB | CARRY) ^ 1;
	UINT8 exorC = ORA ^ (IR11 ^ 1);
	UINT8 exorD = exorC ^ m_laluc0;
	UINT8 XC = exorD;
	UINT8 NEWCARRY;
	UINT8 DCARRY;
	UINT8 DSKIP;
	UINT8 SHZERO;

	switch (m_d_f1) {
	case f1_l_rsh_1:    // <-L RSH 1
		NEWCARRY = m_l & 1;
		m_shifter = ((m_l >> 1) | (XC << 15)) & 0177777;
		LOG((this,LOG_EMU,2,"    DNS; <-L RSH 1 (shifter:%06o XC:%o NEWCARRY:%o)", m_shifter, XC, NEWCARRY));
		break;
	case f1_l_lsh_1:    // <-L LSH 1
		NEWCARRY = (m_l >> 15) & 1;
		m_shifter = ((m_l << 1) | XC) & 0177777;
		LOG((this,LOG_EMU,2,"    DNS; <-L LSH 1 (shifter:%06o XC:%o NEWCARRY:%o)", m_shifter, XC, NEWCARRY));
		break;
	case f1_l_lcy_8:    // <-L LCY 8
		NEWCARRY = XC;
		m_shifter = (m_l >> 8) | (m_l << 8);
		LOG((this,LOG_EMU,2,"    DNS; (shifter:%06o NEWCARRY:%o)", m_shifter, NEWCARRY));
		break;
	default:            // other
		NEWCARRY = XC;
		m_shifter = m_l;
		LOG((this,LOG_EMU,2,"    DNS; (shifter:%06o NEWCARRY:%o)", m_shifter, NEWCARRY));
		break;
	}
	SHZERO = (m_shifter == 0);
	DCARRY = (((IR12 ^ 1) & NEWCARRY) | (IR12 & CARRY)) ^ 1;
	DSKIP = (((NEWCARRY ^ 1) & IR14) | (SHZERO & IR13)) ^ IR15;

	m_emu.cy = DCARRY;      // DCARRY is latched as new m_emu.cy
	m_emu.skip = DSKIP;     // DSKIP is latched as new m_emu.skip

	/* !(IR12 & DNS) -> WR' = 0 for the register file */
	if (!IR12) {
		m_r[m_rsel] = m_shifter;
	}
}

/**
 * @brief destiantion accu: modify RSELECT with DstAC = (3 - IR[3-4])
 */
void alto2_cpu_device::f2_early_acdest()
{
	X_WRBITS(m_rsel, 5, 3, 4, IR_DstAC(m_emu.ir) ^ 3);
	LOG((this,LOG_EMU,2,"    ACDEST<-; mux (rsel:%#o %s)\n", m_rsel, r_name(m_rsel)));
}

#if ALTO2_DEBUG
void alto2_cpu_device::bitblt_info()
{
	static const char *type_name[4] = {"bitmap","complement","and gray","gray"};
	static const char *oper_name[4] = {"replace","paint","invert","erase"};
	int bbt = m_r[rsel_ac2];
	int val = debug_read_mem(bbt);

	LOG((this,LOG_EMU,3,"    BITBLT AC1:%06o AC2:%06o\n", m_r[rsel_ac1], m_r[rsel_ac2]));
	LOG((this,LOG_EMU,3,"        function  : %06o\n", val));
	LOG((this,LOG_EMU,3,"            src extRAM: %o\n", X_BIT(val,16,10)));
	LOG((this,LOG_EMU,3,"            dst extRAM: %o\n", X_BIT(val,16,11)));
	LOG((this,LOG_EMU,3,"            src type  : %o (%s)\n", X_RDBITS(val,16,12,13), type_name[X_RDBITS(val,16,12,13)]));
	LOG((this,LOG_EMU,3,"            operation : %o (%s)\n", X_RDBITS(val,16,14,15), oper_name[X_RDBITS(val,16,14,15)]));
	val = debug_read_mem(bbt+1);
	LOG((this,LOG_EMU,3,"        unused AC2: %06o (%d)\n", val, val));
	val = debug_read_mem(bbt+2);
	LOG((this,LOG_EMU,3,"        DBCA      : %06o (%d)\n", val, val));
	val = debug_read_mem(bbt+3);
	LOG((this,LOG_EMU,3,"        DBMR      : %06o (%d words)\n", val, val));
	val = debug_read_mem(bbt+4);
	LOG((this,LOG_EMU,3,"        DLX       : %06o (%d bits)\n", val, val));
	val = debug_read_mem(bbt+5);
	LOG((this,LOG_EMU,3,"        DTY       : %06o (%d scanlines)\n", val, val));
	val = debug_read_mem(bbt+6);
	LOG((this,LOG_EMU,3,"        DW        : %06o (%d bits)\n", val, val));
	val = debug_read_mem(bbt+7);
	LOG((this,LOG_EMU,3,"        DH        : %06o (%d scanlines)\n", val, val));
	val = debug_read_mem(bbt+8);
	LOG((this,LOG_EMU,3,"        SBCA      : %06o (%d)\n", val, val));
	val = debug_read_mem(bbt+9);
	LOG((this,LOG_EMU,3,"        SBMR      : %06o (%d words)\n", val, val));
	val = debug_read_mem(bbt+10);
	LOG((this,LOG_EMU,3,"        SLX       : %06o (%d bits)\n", val, val));
	val = debug_read_mem(bbt+11);
	LOG((this,LOG_EMU,3,"        STY       : %06o (%d scanlines)\n", val, val));
	LOG((this,LOG_EMU,3,"        GRAY0-3   : %06o %06o %06o %06o\n",
		debug_read_mem(bbt+12), debug_read_mem(bbt+13),
		debug_read_mem(bbt+14), debug_read_mem(bbt+15)));
}
#endif  /* DEBUG */

/**
 * @brief load instruction register IR and branch on IR[0,5-7]
 *
 * Loading the IR clears the skip latch.
 */
void alto2_cpu_device::f2_late_load_ir()
{
	UINT16 r = (X_BIT(m_bus,16,0) << 3) | X_RDBITS(m_bus,16,5,7);

#if ALTO2_DEBUG
	/* special logging of some opcodes */
	switch (m_bus) {
	case op_CYCLE:
		LOG((this,LOG_EMU,3,"    CYCLE AC0:#o\n", m_r[rsel_ac0]));
		break;
	case op_CYCLE + 1: case op_CYCLE + 2: case op_CYCLE + 3: case op_CYCLE + 4:
	case op_CYCLE + 5: case op_CYCLE + 6: case op_CYCLE + 7: case op_CYCLE + 8:
	case op_CYCLE + 9: case op_CYCLE +10: case op_CYCLE +11: case op_CYCLE +12:
	case op_CYCLE +13: case op_CYCLE +14: case op_CYCLE +15:
		LOG((this,LOG_EMU,3,"    CYCLE %#o\n", m_bus - op_CYCLE));
		break;
	case op_BLT:
		LOG((this,LOG_EMU,3,"    BLT dst:%#o src:%#o size:%#o\n",
			(m_r[rsel_ac1] + m_r[rsel_ac3] + 1) & 0177777,
			(m_r[rsel_ac0] + 1) & 017777, -m_r[rsel_ac3] & 0177777));
		break;
	case op_BLKS:
		LOG((this,LOG_EMU,3,"    BLKS dst:%#o val:%#o size:%#o\n",
			(m_r[rsel_ac1] + m_r[rsel_ac3] + 1) & 0177777,
			m_r[rsel_ac0], -m_r[rsel_ac3] & 0177777));
		break;
	case op_DIAGNOSE1:
		LOG((this,LOG_EMU,3,"    DIAGNOSE1 AC0:%06o AC1:%06o AC2:%06o AC3:%06o\n",
			m_r[rsel_ac0], m_r[rsel_ac1],
			m_r[rsel_ac2], m_r[rsel_ac3]));
		break;
	case op_DIAGNOSE2:
		LOG((this,LOG_EMU,3,"    DIAGNOSE2 AC0:%06o AC1:%06o AC2:%06o AC3:%06o\n",
			m_r[rsel_ac0], m_r[rsel_ac1],
			m_r[rsel_ac2], m_r[rsel_ac3]));
		break;
	case op_BITBLT:
		bitblt_info();
		break;
	case op_RDRAM:
		LOG((this,LOG_EMU,3,"    RDRAM addr:%#o\n", m_r[rsel_ac1]));
		break;
	case op_WRTRAM:
		LOG((this,LOG_EMU,3,"    WRTAM addr:%#o upper:%06o lower:%06o\n", m_r[rsel_ac1], m_r[rsel_ac0], m_r[rsel_ac3]));
		break;
	case op_JMPRAM:
		LOG((this,LOG_EMU,3,"    JMPRAM addr:%#o\n", m_r[rsel_ac1]));
		break;
	case op_XMLDA:
		LOG((this,LOG_EMU,3,"    XMLDA AC0 = [bank:%o AC1:#o]\n", m_bank_reg[m_task] & 3, m_r[rsel_ac1]));
		break;
	case op_XMSTA:
		LOG((this,LOG_EMU,3,"    XMSTA [bank:%o AC1:#o] = AC0 (%#o)\n", m_bank_reg[m_task] & 3, m_r[rsel_ac1], m_r[rsel_ac0]));
		break;
	}
#endif
	m_emu.ir = m_bus;
	m_emu.skip = 0;
	m_next2 |= r;
}


/**
 * @brief branch on: arithmetic IR_SH, others PROM ctl2k_u3[IR[1-7]]
 */
void alto2_cpu_device::f2_late_idisp()
{
	UINT16 r;

	if (IR_ARITH(m_emu.ir)) {
		/* 1xxxxxxxxxxxxxxx */
		r = IR_SH(m_emu.ir) ^ 3;            /* complement of SH */
		LOG((this,LOG_EMU,2,"    IDISP<-; branch on SH^3 (%#o|%#o)\n", m_next2, r));
	} else {
		int addr = CTL2K_U3(f2_emu_idisp) + X_RDBITS(m_emu.ir,16,1,7);
		/* 0???????xxxxxxxx */
		r = m_ctl2k_u3[addr];
		LOG((this,LOG_EMU,2,"    IDISP<-; IR (%#o) branch on PROM ctl2k_u3[%03o] (%#o|%#o)\n", m_emu.ir, addr, m_next2, r));
	}
	m_next2 |= r;
}

/**
 * @brief source accu: modify RSELECT with SrcAC = (3 - IR[1-2])
 */
void alto2_cpu_device::f2_early_acsource()
{
	X_WRBITS(m_rsel, 5, 3, 4, IR_SrcAC(m_emu.ir) ^ 3);
	LOG((this,LOG_EMU,2,"    <-ACSOURCE; rsel := SrcAC (%#o %s)\n", m_rsel, r_name(m_rsel)));
}

/**
 * @brief branch on: arithmetic IR_SH, others PROM ctl2k_u3[IR[1-7]]
 */
void alto2_cpu_device::f2_late_acsource()
{
	UINT16 r;

	if (IR_ARITH(m_emu.ir)) {
		/* 1xxxxxxxxxxxxxxx */
		r = IR_SH(m_emu.ir) ^ 3;            /* complement of SH */
		LOG((this,LOG_EMU,2,"    <-ACSOURCE; branch on SH^3 (%#o|%#o)\n", m_next2, r));
	} else {
		int addr = CTL2K_U3(f2_emu_acsource) + X_RDBITS(m_emu.ir,16,1,7);
		/* 0???????xxxxxxxx */
		r = m_ctl2k_u3[addr];
		LOG((this,LOG_EMU,2,"    <-ACSOURCE; branch on PROM ctl2k_u3[%03o] (%#o|%#o)\n", addr, m_next2, r));
	}
	m_next2 |= r;
}

void alto2_cpu_device::init_emu(int task)
{
	memset(&m_emu, 0, sizeof(m_emu));
	save_item(NAME(m_emu.ir));
	save_item(NAME(m_emu.skip));
	save_item(NAME(m_emu.cy));

	init_ram(task);

	set_bs(task, bs_emu_read_sreg,      &alto2_cpu_device::bs_early_read_sreg, 0);
	set_bs(task, bs_emu_load_sreg,      &alto2_cpu_device::bs_early_load_sreg, &alto2_cpu_device::bs_late_load_sreg);
	set_bs(task, bs_disp,               &alto2_cpu_device::bs_early_emu_disp, 0);

	set_f1(task, f1_block,              &alto2_cpu_device::f1_early_emu_block, 0);  // catch the emulator task trying to block (wrong branch)
	set_f1(task, f1_emu_swmode,         0, &alto2_cpu_device::f1_late_swmode);
	set_f1(task, f1_emu_wrtram,         0, &alto2_cpu_device::f1_late_wrtram);
	set_f1(task, f1_emu_rdram,          0, &alto2_cpu_device::f1_late_rdram);
	set_f1(task, f1_emu_load_rmr,       0, &alto2_cpu_device::f1_late_emu_load_rmr);
	set_f1(task, f1_task_14,            0, 0);                                      // F1 014 is undefined (?)
	set_f1(task, f1_emu_load_esrb,      0, &alto2_cpu_device::f1_late_emu_load_esrb);
	set_f1(task, f1_emu_rsnf,           &alto2_cpu_device::f1_early_rsnf, 0);
	set_f1(task, f1_emu_startf,         &alto2_cpu_device::f1_early_startf, 0);

	set_f2(task, f2_emu_busodd,         0, &alto2_cpu_device::f2_late_busodd);
	set_f2(task, f2_emu_magic,          0, &alto2_cpu_device::f2_late_magic);
	set_f2(task, f2_emu_load_dns,       &alto2_cpu_device::f2_early_load_dns, &alto2_cpu_device::f2_late_load_dns);
	set_f2(task, f2_emu_acdest,         &alto2_cpu_device::f2_early_acdest, 0);
	set_f2(task, f2_emu_load_ir,        0, &alto2_cpu_device::f2_late_load_ir);
	set_f2(task, f2_emu_idisp,          0, &alto2_cpu_device::f2_late_idisp);
	set_f2(task, f2_emu_acsource,       &alto2_cpu_device::f2_early_acsource, &alto2_cpu_device::f2_late_acsource);
}

void alto2_cpu_device::exit_emu()
{
	// nothing to do yet
}

void alto2_cpu_device::reset_emu()
{
	m_emu.ir = 0;
	m_emu.skip = 0;
	m_emu.cy = 0;
}
