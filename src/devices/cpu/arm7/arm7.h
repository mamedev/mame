// license:BSD-3-Clause
// copyright-holders:Steve Ellenoff,R. Belmont,Ryan Holtz
/*****************************************************************************
 *
 *   arm7.h
 *   Portable ARM7TDMI CPU Emulator
 *
 *   Copyright Steve Ellenoff, all rights reserved.
 *
 *  This work is based on:
 *  #1) 'Atmel Corporation ARM7TDMI (Thumb) Datasheet - January 1999'
 *  #2) Arm 2/3/6 emulator By Bryan McPhail (bmcphail@tendril.co.uk) and Phil Stroffolino (MAME CORE 0.76)
 *
 *****************************************************************************

 This file contains everything related to the arm7 cpu specific implementation.
 Anything related to the arm7 core itself is defined in arm7core.h instead.

 ******************************************************************************/

#pragma once

#ifndef __ARM7_H__
#define __ARM7_H__

#include "cpu/drcfe.h"
#include "cpu/drcuml.h"
#include "cpu/drcumlsh.h"


#define ARM7_MAX_FASTRAM       4
#define ARM7_MAX_HOTSPOTS      16


/***************************************************************************
    COMPILER-SPECIFIC OPTIONS
***************************************************************************/

#define ARM7DRC_STRICT_VERIFY      0x0001          /* verify all instructions */
#define ARM7DRC_FLUSH_PC           0x0008          /* flush the PC value before each memory access */

#define ARM7DRC_COMPATIBLE_OPTIONS (ARM7DRC_STRICT_VERIFY | ARM7DRC_FLUSH_PC)
#define ARM7DRC_FASTEST_OPTIONS    (0)

/****************************************************************************************************
 *  PUBLIC FUNCTIONS
 ***************************************************************************************************/

class arm7_cpu_device : public cpu_device
{
public:
	// construction/destruction
	arm7_cpu_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	arm7_cpu_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source, UINT8 archRev, UINT8 archFlags, endianness_t endianness = ENDIANNESS_LITTLE);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const override { return 3; }
	virtual UINT32 execute_max_cycles() const override { return 4; }
	virtual UINT32 execute_input_lines() const override { return 4; } /* There are actually only 2 input lines: we use 3 variants of the ABORT line while there is only 1 real one */
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override { return (spacenum == AS_PROGRAM) ? &m_program_config : nullptr; }
	virtual bool memory_translate(address_spacenum spacenum, int intention, offs_t &address) override;

	// device_state_interface overrides
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const override { return 2; }
	virtual UINT32 disasm_max_opcode_bytes() const override { return 4; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

	address_space_config m_program_config;

	UINT32 m_r[/*NUM_REGS*/37];
	UINT32 m_pendingIrq;
	UINT32 m_pendingFiq;
	UINT32 m_pendingAbtD;
	UINT32 m_pendingAbtP;
	UINT32 m_pendingUnd;
	UINT32 m_pendingSwi;
	int m_icount;
	endianness_t m_endian;
	address_space *m_program;
	direct_read_data *m_direct;

	/* Coprocessor Registers */
	UINT32 m_control;
	UINT32 m_tlbBase;
	UINT32 m_faultStatus[2];
	UINT32 m_faultAddress;
	UINT32 m_fcsePID;
	UINT32 m_domainAccessControl;

	UINT8 m_archRev;          // ARM architecture revision (3, 4, and 5 are valid)
	UINT8 m_archFlags;        // architecture flags

//#if ARM7_MMU_ENABLE_HACK
//  UINT32 mmu_enable_addr; // workaround for "MMU is enabled when PA != VA" problem
//#endif

	UINT32 m_copro_id;

	// For debugger
	UINT32 m_pc;

	INT64 saturate_qbit_overflow(INT64 res);
	void SwitchMode(UINT32 cpsr_mode_val);
	UINT32 decodeShift(UINT32 insn, UINT32 *pCarry);
	int loadInc(UINT32 pat, UINT32 rbv, UINT32 s, int mode);
	int loadDec(UINT32 pat, UINT32 rbv, UINT32 s, int mode);
	int storeInc(UINT32 pat, UINT32 rbv, int mode);
	int storeDec(UINT32 pat, UINT32 rbv, int mode);
	void HandleCoProcDO(UINT32 insn);
	void HandleCoProcRT(UINT32 insn);
	void HandleCoProcDT(UINT32 insn);
	void HandleBranch(UINT32 insn);
	void HandleMemSingle(UINT32 insn);
	void HandleHalfWordDT(UINT32 insn);
	void HandleSwap(UINT32 insn);
	void HandlePSRTransfer(UINT32 insn);
	void HandleALU(UINT32 insn);
	void HandleMul(UINT32 insn);
	void HandleSMulLong(UINT32 insn);
	void HandleUMulLong(UINT32 insn);
	void HandleMemBlock(UINT32 insn);
	void arm7ops_0123(UINT32 insn);
	void arm7ops_4567(UINT32 insn);
	void arm7ops_89(UINT32 insn);
	void arm7ops_ab(UINT32 insn);
	void arm7ops_cd(UINT32 insn);
	void arm7ops_e(UINT32 insn);
	void arm7ops_f(UINT32 insn);
	void set_cpsr(UINT32 val);
	bool arm7_tlb_translate(offs_t &addr, int flags);
	UINT32 arm7_tlb_get_first_level_descriptor( UINT32 vaddr );
	UINT32 arm7_tlb_get_second_level_descriptor( UINT32 granularity, UINT32 first_desc, UINT32 vaddr );
	int detect_fault(int permission, int ap, int flags);
	void arm7_check_irq_state();
	void arm7_cpu_write32(UINT32 addr, UINT32 data);
	void arm7_cpu_write16(UINT32 addr, UINT16 data);
	void arm7_cpu_write8(UINT32 addr, UINT8 data);
	UINT32 arm7_cpu_read32(UINT32 addr);
	UINT16 arm7_cpu_read16(UINT32 addr);
	UINT8 arm7_cpu_read8(UINT32 addr);

	// Coprocessor support
	DECLARE_WRITE32_MEMBER( arm7_do_callback );
	DECLARE_READ32_MEMBER( arm7_rt_r_callback );
	DECLARE_WRITE32_MEMBER( arm7_rt_w_callback );
	void arm7_dt_r_callback(UINT32 insn, UINT32 *prn);
	void arm7_dt_w_callback(UINT32 insn, UINT32 *prn);

	void tg00_0(UINT32 pc, UINT32 insn);
	void tg00_1(UINT32 pc, UINT32 insn);
	void tg01_0(UINT32 pc, UINT32 insn);
	void tg01_10(UINT32 pc, UINT32 insn);
	void tg01_11(UINT32 pc, UINT32 insn);
	void tg01_12(UINT32 pc, UINT32 insn);
	void tg01_13(UINT32 pc, UINT32 insn);
	void tg02_0(UINT32 pc, UINT32 insn);
	void tg02_1(UINT32 pc, UINT32 insn);
	void tg03_0(UINT32 pc, UINT32 insn);
	void tg03_1(UINT32 pc, UINT32 insn);
	void tg04_00_00(UINT32 pc, UINT32 insn);
	void tg04_00_01(UINT32 pc, UINT32 insn);
	void tg04_00_02(UINT32 pc, UINT32 insn);
	void tg04_00_03(UINT32 pc, UINT32 insn);
	void tg04_00_04(UINT32 pc, UINT32 insn);
	void tg04_00_05(UINT32 pc, UINT32 insn);
	void tg04_00_06(UINT32 pc, UINT32 insn);
	void tg04_00_07(UINT32 pc, UINT32 insn);
	void tg04_00_08(UINT32 pc, UINT32 insn);
	void tg04_00_09(UINT32 pc, UINT32 insn);
	void tg04_00_0a(UINT32 pc, UINT32 insn);
	void tg04_00_0b(UINT32 pc, UINT32 insn);
	void tg04_00_0c(UINT32 pc, UINT32 insn);
	void tg04_00_0d(UINT32 pc, UINT32 insn);
	void tg04_00_0e(UINT32 pc, UINT32 insn);
	void tg04_00_0f(UINT32 pc, UINT32 insn);
	void tg04_01_00(UINT32 pc, UINT32 insn);
	void tg04_01_01(UINT32 pc, UINT32 insn);
	void tg04_01_02(UINT32 pc, UINT32 insn);
	void tg04_01_03(UINT32 pc, UINT32 insn);
	void tg04_01_10(UINT32 pc, UINT32 insn);
	void tg04_01_11(UINT32 pc, UINT32 insn);
	void tg04_01_12(UINT32 pc, UINT32 insn);
	void tg04_01_13(UINT32 pc, UINT32 insn);
	void tg04_01_20(UINT32 pc, UINT32 insn);
	void tg04_01_21(UINT32 pc, UINT32 insn);
	void tg04_01_22(UINT32 pc, UINT32 insn);
	void tg04_01_23(UINT32 pc, UINT32 insn);
	void tg04_01_30(UINT32 pc, UINT32 insn);
	void tg04_01_31(UINT32 pc, UINT32 insn);
	void tg04_01_32(UINT32 pc, UINT32 insn);
	void tg04_01_33(UINT32 pc, UINT32 insn);
	void tg04_0203(UINT32 pc, UINT32 insn);
	void tg05_0(UINT32 pc, UINT32 insn);
	void tg05_1(UINT32 pc, UINT32 insn);
	void tg05_2(UINT32 pc, UINT32 insn);
	void tg05_3(UINT32 pc, UINT32 insn);
	void tg05_4(UINT32 pc, UINT32 insn);
	void tg05_5(UINT32 pc, UINT32 insn);
	void tg05_6(UINT32 pc, UINT32 insn);
	void tg05_7(UINT32 pc, UINT32 insn);
	void tg06_0(UINT32 pc, UINT32 insn);
	void tg06_1(UINT32 pc, UINT32 insn);
	void tg07_0(UINT32 pc, UINT32 insn);
	void tg07_1(UINT32 pc, UINT32 insn);
	void tg08_0(UINT32 pc, UINT32 insn);
	void tg08_1(UINT32 pc, UINT32 insn);
	void tg09_0(UINT32 pc, UINT32 insn);
	void tg09_1(UINT32 pc, UINT32 insn);
	void tg0a_0(UINT32 pc, UINT32 insn);
	void tg0a_1(UINT32 pc, UINT32 insn);
	void tg0b_0(UINT32 pc, UINT32 insn);
	void tg0b_1(UINT32 pc, UINT32 insn);
	void tg0b_2(UINT32 pc, UINT32 insn);
	void tg0b_3(UINT32 pc, UINT32 insn);
	void tg0b_4(UINT32 pc, UINT32 insn);
	void tg0b_5(UINT32 pc, UINT32 insn);
	void tg0b_6(UINT32 pc, UINT32 insn);
	void tg0b_7(UINT32 pc, UINT32 insn);
	void tg0b_8(UINT32 pc, UINT32 insn);
	void tg0b_9(UINT32 pc, UINT32 insn);
	void tg0b_a(UINT32 pc, UINT32 insn);
	void tg0b_b(UINT32 pc, UINT32 insn);
	void tg0b_c(UINT32 pc, UINT32 insn);
	void tg0b_d(UINT32 pc, UINT32 insn);
	void tg0b_e(UINT32 pc, UINT32 insn);
	void tg0b_f(UINT32 pc, UINT32 insn);
	void tg0c_0(UINT32 pc, UINT32 insn);
	void tg0c_1(UINT32 pc, UINT32 insn);
	void tg0d_0(UINT32 pc, UINT32 insn);
	void tg0d_1(UINT32 pc, UINT32 insn);
	void tg0d_2(UINT32 pc, UINT32 insn);
	void tg0d_3(UINT32 pc, UINT32 insn);
	void tg0d_4(UINT32 pc, UINT32 insn);
	void tg0d_5(UINT32 pc, UINT32 insn);
	void tg0d_6(UINT32 pc, UINT32 insn);
	void tg0d_7(UINT32 pc, UINT32 insn);
	void tg0d_8(UINT32 pc, UINT32 insn);
	void tg0d_9(UINT32 pc, UINT32 insn);
	void tg0d_a(UINT32 pc, UINT32 insn);
	void tg0d_b(UINT32 pc, UINT32 insn);
	void tg0d_c(UINT32 pc, UINT32 insn);
	void tg0d_d(UINT32 pc, UINT32 insn);
	void tg0d_e(UINT32 pc, UINT32 insn);
	void tg0d_f(UINT32 pc, UINT32 insn);
	void tg0e_0(UINT32 pc, UINT32 insn);
	void tg0e_1(UINT32 pc, UINT32 insn);
	void tg0f_0(UINT32 pc, UINT32 insn);
	void tg0f_1(UINT32 pc, UINT32 insn);

	typedef void ( arm7_cpu_device::*arm7thumb_ophandler ) (UINT32, UINT32);
	static const arm7thumb_ophandler thumb_handler[0x40*0x10];

	typedef void ( arm7_cpu_device::*arm7ops_ophandler )(UINT32);
	static const arm7ops_ophandler ops_handler[0x10];

	//
	// DRC
	//

	/* fast RAM info */
	struct fast_ram_info
	{
		offs_t              start;                      /* start of the RAM block */
		offs_t              end;                        /* end of the RAM block */
		UINT8               readonly;                   /* TRUE if read-only */
		void *              base;                       /* base in memory where the RAM lives */
	};

	struct hotspot_info
	{
		UINT32             pc;
		UINT32             opcode;
		UINT32             cycles;
	};

	/* internal compiler state */
	struct compiler_state
	{
		UINT32              cycles;                     /* accumulated cycles */
		UINT8               checkints;                  /* need to check interrupts before next instruction */
		UINT8               checksoftints;              /* need to check software interrupts before next instruction */
		uml::code_label  labelnum;                   /* index for local labels */
	};

	/* ARM7 registers */
	struct arm7imp_state
	{
		/* core state */
		drc_cache *         cache;                      /* pointer to the DRC code cache */
		drcuml_state *      drcuml;                     /* DRC UML generator state */
		//arm7_frontend *     drcfe;                      /* pointer to the DRC front-end state */
		UINT32              drcoptions;                 /* configurable DRC options */

		/* internal stuff */
		UINT8               cache_dirty;                /* true if we need to flush the cache */
		UINT32              jmpdest;                    /* destination jump target */

		/* parameters for subroutines */
		UINT64              numcycles;                  /* return value from gettotalcycles */
		UINT32              mode;                       /* current global mode */
		const char *        format;                     /* format string for print_debug */
		UINT32              arg0;                       /* print_debug argument 1 */
		UINT32              arg1;                       /* print_debug argument 2 */

		/* register mappings */
		uml::parameter   regmap[/*NUM_REGS*/37];               /* parameter to register mappings for all 16 integer registers */

		/* subroutines */
		uml::code_handle *   entry;                      /* entry point */
		uml::code_handle *   nocode;                     /* nocode exception handler */
		uml::code_handle *   out_of_cycles;              /* out of cycles exception handler */
		uml::code_handle *   tlb_translate;              /* tlb translation handler */
		uml::code_handle *   detect_fault;               /* tlb fault detection handler */
		uml::code_handle *   check_irq;                  /* irq check handler */
		uml::code_handle *   read8;                      /* read byte */
		uml::code_handle *   write8;                     /* write byte */
		uml::code_handle *   read16;                     /* read half */
		uml::code_handle *   write16;                    /* write half */
		uml::code_handle *   read32;                     /* read word */
		uml::code_handle *   write32;                    /* write word */

		/* fast RAM */
		UINT32              fastram_select;
		fast_ram_info       fastram[ARM7_MAX_FASTRAM];

		/* hotspots */
		UINT32              hotspot_select;
		hotspot_info        hotspot[ARM7_MAX_HOTSPOTS];
	} m_impstate;

	typedef void ( arm7_cpu_device::*arm7thumb_drcophandler)(drcuml_block*, compiler_state*, const opcode_desc*);
	static const arm7thumb_drcophandler drcthumb_handler[0x40*0x10];

	void drctg00_0(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); /* Shift left */
	void drctg00_1(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); /* Shift right */
	void drctg01_0(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void drctg01_10(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void drctg01_11(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); /* SUB Rd, Rs, Rn */
	void drctg01_12(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); /* ADD Rd, Rs, #imm */
	void drctg01_13(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); /* SUB Rd, Rs, #imm */
	void drctg02_0(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void drctg02_1(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void drctg03_0(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); /* ADD Rd, #Offset8 */
	void drctg03_1(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); /* SUB Rd, #Offset8 */
	void drctg04_00_00(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); /* AND Rd, Rs */
	void drctg04_00_01(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); /* EOR Rd, Rs */
	void drctg04_00_02(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); /* LSL Rd, Rs */
	void drctg04_00_03(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); /* LSR Rd, Rs */
	void drctg04_00_04(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); /* ASR Rd, Rs */
	void drctg04_00_05(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); /* ADC Rd, Rs */
	void drctg04_00_06(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);  /* SBC Rd, Rs */
	void drctg04_00_07(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); /* ROR Rd, Rs */
	void drctg04_00_08(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); /* TST Rd, Rs */
	void drctg04_00_09(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); /* NEG Rd, Rs */
	void drctg04_00_0a(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); /* CMP Rd, Rs */
	void drctg04_00_0b(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); /* CMN Rd, Rs - check flags, add dasm */
	void drctg04_00_0c(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); /* ORR Rd, Rs */
	void drctg04_00_0d(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); /* MUL Rd, Rs */
	void drctg04_00_0e(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); /* BIC Rd, Rs */
	void drctg04_00_0f(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); /* MVN Rd, Rs */
	void drctg04_01_00(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void drctg04_01_01(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); /* ADD Rd, HRs */
	void drctg04_01_02(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); /* ADD HRd, Rs */
	void drctg04_01_03(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); /* Add HRd, HRs */
	void drctg04_01_10(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);  /* CMP Rd, Rs */
	void drctg04_01_11(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); /* CMP Rd, Hs */
	void drctg04_01_12(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); /* CMP Hd, Rs */
	void drctg04_01_13(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); /* CMP Hd, Hs */
	void drctg04_01_20(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); /* MOV Rd, Rs (undefined) */
	void drctg04_01_21(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); /* MOV Rd, Hs */
	void drctg04_01_22(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); /* MOV Hd, Rs */
	void drctg04_01_23(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); /* MOV Hd, Hs */
	void drctg04_01_30(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void drctg04_01_31(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void drctg04_01_32(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void drctg04_01_33(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void drctg04_0203(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void drctg05_0(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);  /* STR Rd, [Rn, Rm] */
	void drctg05_1(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);  /* STRH Rd, [Rn, Rm] */
	void drctg05_2(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);  /* STRB Rd, [Rn, Rm] */
	void drctg05_3(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);  /* LDSB Rd, [Rn, Rm] todo, add dasm */
	void drctg05_4(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);  /* LDR Rd, [Rn, Rm] */
	void drctg05_5(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);  /* LDRH Rd, [Rn, Rm] */
	void drctg05_6(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);  /* LDRB Rd, [Rn, Rm] */
	void drctg05_7(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);  /* LDSH Rd, [Rn, Rm] */
	void drctg06_0(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); /* Store */
	void drctg06_1(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); /* Load */
	void drctg07_0(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); /* Store */
	void drctg07_1(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);  /* Load */
	void drctg08_0(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); /* Store */
	void drctg08_1(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); /* Load */
	void drctg09_0(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); /* Store */
	void drctg09_1(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); /* Load */
	void drctg0a_0(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);  /* ADD Rd, PC, #nn */
	void drctg0a_1(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); /* ADD Rd, SP, #nn */
	void drctg0b_0(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); /* ADD SP, #imm */
	void drctg0b_1(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void drctg0b_2(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void drctg0b_3(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void drctg0b_4(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); /* PUSH {Rlist} */
	void drctg0b_5(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); /* PUSH {Rlist}{LR} */
	void drctg0b_6(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void drctg0b_7(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void drctg0b_8(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void drctg0b_9(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void drctg0b_a(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void drctg0b_b(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void drctg0b_c(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); /* POP {Rlist} */
	void drctg0b_d(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); /* POP {Rlist}{PC} */
	void drctg0b_e(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void drctg0b_f(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void drctg0c_0(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); /* Store */
	void drctg0c_1(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); /* Load */
	void drctg0d_0(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); // COND_EQ:
	void drctg0d_1(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); // COND_NE:
	void drctg0d_2(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); // COND_CS:
	void drctg0d_3(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); // COND_CC:
	void drctg0d_4(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); // COND_MI:
	void drctg0d_5(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); // COND_PL:
	void drctg0d_6(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); // COND_VS:
	void drctg0d_7(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); // COND_VC:
	void drctg0d_8(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); // COND_HI:
	void drctg0d_9(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); // COND_LS:
	void drctg0d_a(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); // COND_GE:
	void drctg0d_b(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); // COND_LT:
	void drctg0d_c(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); // COND_GT:
	void drctg0d_d(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); // COND_LE:
	void drctg0d_e(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); // COND_AL:
	void drctg0d_f(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); // SWI (this is sort of a "hole" in the opcode encoding)
	void drctg0e_0(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void drctg0e_1(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void drctg0f_0(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void drctg0f_1(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); /* BL */

	void load_fast_iregs(drcuml_block *block);
	void save_fast_iregs(drcuml_block *block);
	void arm7_drc_init();
	void arm7_drc_exit();
	void execute_run_drc();
	void arm7drc_set_options(UINT32 options);
	void arm7drc_add_fastram(offs_t start, offs_t end, UINT8 readonly, void *base);
	void arm7drc_add_hotspot(offs_t pc, UINT32 opcode, UINT32 cycles);
	void code_flush_cache();
	void code_compile_block(UINT8 mode, offs_t pc);
	void cfunc_get_cycles();
	void cfunc_unimplemented();
	void static_generate_entry_point();
	void static_generate_check_irq();
	void static_generate_nocode_handler();
	void static_generate_out_of_cycles();
	void static_generate_detect_fault(uml::code_handle **handleptr);
	void static_generate_tlb_translate(uml::code_handle **handleptr);
	void static_generate_memory_accessor(int size, bool istlb, bool iswrite, const char *name, uml::code_handle **handleptr);
	void generate_update_cycles(drcuml_block *block, compiler_state *compiler, uml::parameter param);
	void generate_checksum_block(drcuml_block *block, compiler_state *compiler, const opcode_desc *seqhead, const opcode_desc *seqlast);
	void generate_sequence_instruction(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_delay_slot_and_branch(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT8 linkreg);

	typedef bool ( arm7_cpu_device::*drcarm7ops_ophandler)(drcuml_block*, compiler_state*, const opcode_desc*, UINT32);
	static const drcarm7ops_ophandler drcops_handler[0x10];

	void saturate_qbit_overflow(drcuml_block *block);
	bool drcarm7ops_0123(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT32 op);
	bool drcarm7ops_4567(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT32 op);
	bool drcarm7ops_89(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT32 op);
	bool drcarm7ops_ab(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT32 op);
	bool drcarm7ops_cd(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT32 op);
	bool drcarm7ops_e(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT32 op);
	bool drcarm7ops_f(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT32 op);
	int generate_opcode(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);

};


class arm7_be_cpu_device : public arm7_cpu_device
{
public:
	// construction/destruction
	arm7_be_cpu_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

};


class arm7500_cpu_device : public arm7_cpu_device
{
public:
	// construction/destruction
	arm7500_cpu_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

};


class arm9_cpu_device : public arm7_cpu_device
{
public:
	// construction/destruction
	arm9_cpu_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

};


class arm920t_cpu_device : public arm7_cpu_device
{
public:
	// construction/destruction
	arm920t_cpu_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

};


class pxa255_cpu_device : public arm7_cpu_device
{
public:
	// construction/destruction
	pxa255_cpu_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

};


class sa1110_cpu_device : public arm7_cpu_device
{
public:
	// construction/destruction
	sa1110_cpu_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

};


extern const device_type ARM7;
extern const device_type ARM7_BE;
extern const device_type ARM7500;
extern const device_type ARM9;
extern const device_type ARM920T;
extern const device_type PXA255;
extern const device_type SA1110;

#endif /* __ARM7_H__ */
