// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    h8.h

    H8-300 base cpu emulation


***************************************************************************/

#ifndef __H8_H__
#define __H8_H__

class h8_device : public cpu_device {
public:
	enum {
		// digital I/O ports
		// ports 4-B are valid on 16-bit H8/3xx, ports 1-9 on 8-bit H8/3xx
		// H8S/2394 has 12 ports named 1-6 and A-G
		PORT_1,  // 0
		PORT_2,  // 1
		PORT_3,  // 2
		PORT_4,  // 3
		PORT_5,  // 4
		PORT_6,  // 5
		PORT_7,  // 6
		PORT_8,  // 7
		PORT_9,  // 8
		PORT_A,  // 9
		PORT_B,  // A
		PORT_C,  // B
		PORT_D,  // C
		PORT_E,  // D
		PORT_F,  // E
		PORT_G,  // F

		// analog inputs
		ADC_0,
		ADC_1,
		ADC_2,
		ADC_3,
		ADC_4,
		ADC_5,
		ADC_6,
		ADC_7
	};

	h8_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source, bool mode_a16, address_map_delegate map_delegate);

	void internal_update();

	void set_irq(int irq_vector, int irq_level, bool irq_nmi);

protected:
	struct disasm_entry {
		int slot;
		UINT32 val, mask;
		UINT16 val0, mask0;
		const char *opcode;
		int am1, am2;
		offs_t flags;
	};

	enum {
		STATE_RESET = 0x10000,
		STATE_IRQ   = 0x10001,
		STATE_TRACE = 0x10002
	};

	enum {
		F_I  = 0x80,
		F_UI = 0x40,
		F_H  = 0x20,
		F_U  = 0x10,
		F_N  = 0x08,
		F_Z  = 0x04,
		F_V  = 0x02,
		F_C  = 0x01,

		EXR_T  = 0x80,
		EXR_NC = 0x78,
		EXR_I  = 0x07
	};

	enum {
		DASM_none,     /* no additional arguments */

		DASM_r8l,      /* 8-bits register in bits 0-3 */
		DASM_r8h,      /* 8-bits register in bits 4-7 */
		DASM_r8u,      /* 8-bits register in bits 8-15 */
		DASM_r16l,     /* 16-bits register in bits 0-3 */
		DASM_r16h,     /* 16-bits register in bits 4-7 */
		DASM_r32l,     /* 32-bits register in bits 0-3 */
		DASM_r32h,     /* 32-bits register in bits 4-7 */

		DASM_r16ih,    /* indexed through 16-bits register in bits 4-6 */
		DASM_r16ihh,   /* indexed through 16-bits register in bits 4-6 in 4-bytes instruction */
		DASM_pr16h,    /* indexed through predecremented 16-bits register in bits 4-6 */
		DASM_r16ph,    /* indexed through postincremented 16-bits register in bits 4-6 */
		DASM_r16d16h,  /* indexed through 16-bits register in bits 4-6 with 16-bits displacement at end of instruction */

		DASM_r32ih,    /* indexed through 32-bits register in bits 4-6 */
		DASM_r32ihh,   /* indexed through 32-bits register in bits 4-6 in 4-bytes instruction */
		DASM_pr32h,    /* indexed through predecremented 32-bits register in bits 4-6 */
		DASM_r32pl,    /* indexed through postincremented 32-bits register in bits 0-2 */
		DASM_r32ph,    /* indexed through postincremented 32-bits register in bits 4-6 */
		DASM_r32d16h,  /* indexed through 32-bits register in bits 4-6 with 16-bits displacement at end of instruction */
		DASM_r32d32hh, /* indexed through 32-bits register in bits 20-22 with 32-bits displacement at end of instruction */

		DASM_psp,      /* indexed through predecremented stack pointer */
		DASM_spp,      /* indexed through postincremented stack pointer */

		DASM_r32n2l,   /* Block of 2 registers */
		DASM_r32n3l,   /* Block of 3 registers */
		DASM_r32n4l,   /* Block of 4 registers */

		DASM_abs8,     /* 8-bit address present at +1 */
		DASM_abs16,    /* 16-bit address present at end of instruction */
		DASM_abs32,    /* 32-bit address present at end of instruction */
		DASM_abs8i,    /* 8-bit indirect jump address present at +1 */
		DASM_abs16e,   /* 16-bit jump address present at +2 */
		DASM_abs24e,   /* 24-bit jump address present at +1 */

		DASM_rel8,     /* 8-bit pc-relative jump address at +1, offset=2 */
		DASM_rel16,    /* 16-bit pc-relative jump address at +2, offset=4 */

		DASM_one,      /* immediate value 1 */
		DASM_two,      /* immediate value 2 */
		DASM_four,     /* immediate value 4 */

		DASM_imm2,     /* 2-bit immediate in bits 4-5 (trapa) */
		DASM_imm3,     /* 3-bit immediate in bits 4-6 (bit selection */
		DASM_imm8,     /* 8-bit immediate at +1 */
		DASM_imm16,    /* 16-bit immediate at +2 */
		DASM_imm32,    /* 32-bit immediate at +2 */

		DASM_ccr,      /* internal register ccr */
		DASM_exr,      /* internal register exr */
		DASM_macl,     /* internal register macl */
		DASM_mach      /* internal register mach */
	};

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const override;
	virtual UINT32 execute_max_cycles() const override;
	virtual UINT32 execute_input_lines() const override;
	virtual void execute_run() override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const override;
	virtual UINT32 disasm_max_opcode_bytes() const override;
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

	address_space_config program_config, io_config;
	address_space *program, *io;
	direct_read_data *direct;

	UINT32  PPC;                    /* previous program counter */
	UINT32  NPC;                    /* next start-of-instruction program counter */
	UINT32  PC;                     /* program counter */
	UINT16  PIR;                    /* Prefetched word */
	UINT16  IR[5];                  /* Fetched instruction */
	UINT16  R[16];                  /* Rn (0-7), En (8-15, h8-300h+) */
	UINT8   EXR;                    /* Interrupt/trace register (h8s/2000+) */
	UINT8   CCR;                    /* Condition-code register */
	INT64   MAC;                    /* Multiply accumulator (h8s/2600+) */
	UINT8   MACF;                   /* MAC flags (h8s/2600+) */
	UINT32  TMP1, TMP2;
	UINT32  TMPR;                   /* For debugger ER register import */

	bool has_exr, has_trace, supports_advanced, mode_advanced, mac_saturating;

	int inst_state, inst_substate;
	int icount, bcount;
	int irq_vector, taken_irq_vector;
	int irq_level, taken_irq_level;
	bool irq_required, irq_nmi;

	static const disasm_entry disasm_entries[];

	offs_t disassemble_generic(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options, const disasm_entry *table);
	void disassemble_am(char *&buffer, int am, offs_t pc, const UINT8 *oprom, UINT32 opcode, int offset);

	virtual void do_exec_full();
	virtual void do_exec_partial();
	static void add_event(UINT64 &event_time, UINT64 new_event);
	virtual bool exr_in_stack() const;
	virtual void update_irq_filter() = 0;
	virtual void interrupt_taken() = 0;
	virtual void internal_update(UINT64 current_time) = 0;
	void recompute_bcount(UINT64 event_time);
	virtual int trace_setup();
	virtual int trapa_setup();
	virtual void irq_setup() = 0;

	UINT16 read16i(UINT32 adr);
	UINT16 fetch();
	inline void fetch(int slot) { IR[slot] = fetch(); }
	UINT8 read8(UINT32 adr);
	void write8(UINT32 adr, UINT8 data);
	UINT16 read16(UINT32 adr);
	void write16(UINT32 adr, UINT16 data);
	void internal(int cycles);
	inline void prefetch() { prefetch_start(); prefetch_done(); }
	inline void prefetch_noirq() { prefetch_start(); prefetch_done_noirq(); }
	inline void prefetch_noirq_notrace() { prefetch_start(); prefetch_done_noirq_notrace(); }
	void prefetch_start() { NPC = PC; PIR = fetch(); }
	void prefetch_switch(UINT32 pc, UINT16 ir) { NPC = pc; PC = pc+2; PIR = ir; }
	void prefetch_done();
	void prefetch_done_noirq();
	void prefetch_done_noirq_notrace();
	void illegal();

	UINT8 do_addx8(UINT8 a, UINT8 b);
	UINT8 do_subx8(UINT8 a, UINT8 b);

	UINT8 do_inc8(UINT8 a, UINT8 b);
	UINT16 do_inc16(UINT16 a, UINT16 b);
	UINT32 do_inc32(UINT32 a, UINT32 b);

	UINT8 do_add8(UINT8 a, UINT8 b);
	UINT16 do_add16(UINT16 a, UINT16 b);
	UINT32 do_add32(UINT32 a, UINT32 b);

	UINT8 do_dec8(UINT8 a, UINT8 b);
	UINT16 do_dec16(UINT16 a, UINT16 b);
	UINT32 do_dec32(UINT32 a, UINT32 b);

	UINT8 do_sub8(UINT8 a, UINT8 b);
	UINT16 do_sub16(UINT16 a, UINT16 b);
	UINT32 do_sub32(UINT32 a, UINT32 b);

	UINT8 do_shal8(UINT8 v);
	UINT16 do_shal16(UINT16 v);
	UINT32 do_shal32(UINT32 v);

	UINT8 do_shar8(UINT8 v);
	UINT16 do_shar16(UINT16 v);
	UINT32 do_shar32(UINT32 v);

	UINT8 do_shll8(UINT8 v);
	UINT16 do_shll16(UINT16 v);
	UINT32 do_shll32(UINT32 v);

	UINT8 do_shlr8(UINT8 v);
	UINT16 do_shlr16(UINT16 v);
	UINT32 do_shlr32(UINT32 v);

	UINT8 do_rotl8(UINT8 v);
	UINT16 do_rotl16(UINT16 v);
	UINT32 do_rotl32(UINT32 v);

	UINT8 do_rotr8(UINT8 v);
	UINT16 do_rotr16(UINT16 v);
	UINT32 do_rotr32(UINT32 v);

	UINT8 do_rotxl8(UINT8 v);
	UINT16 do_rotxl16(UINT16 v);
	UINT32 do_rotxl32(UINT32 v);

	UINT8 do_rotxr8(UINT8 v);
	UINT16 do_rotxr16(UINT16 v);
	UINT32 do_rotxr32(UINT32 v);

	UINT8 do_shal2_8(UINT8 v);
	UINT16 do_shal2_16(UINT16 v);
	UINT32 do_shal2_32(UINT32 v);

	UINT8 do_shar2_8(UINT8 v);
	UINT16 do_shar2_16(UINT16 v);
	UINT32 do_shar2_32(UINT32 v);

	UINT8 do_shll2_8(UINT8 v);
	UINT16 do_shll2_16(UINT16 v);
	UINT32 do_shll2_32(UINT32 v);

	UINT8 do_shlr2_8(UINT8 v);
	UINT16 do_shlr2_16(UINT16 v);
	UINT32 do_shlr2_32(UINT32 v);

	UINT8 do_rotl2_8(UINT8 v);
	UINT16 do_rotl2_16(UINT16 v);
	UINT32 do_rotl2_32(UINT32 v);

	UINT8 do_rotr2_8(UINT8 v);
	UINT16 do_rotr2_16(UINT16 v);
	UINT32 do_rotr2_32(UINT32 v);

	UINT8 do_rotxl2_8(UINT8 v);
	UINT16 do_rotxl2_16(UINT16 v);
	UINT32 do_rotxl2_32(UINT32 v);

	UINT8 do_rotxr2_8(UINT8 v);
	UINT16 do_rotxr2_16(UINT16 v);
	UINT32 do_rotxr2_32(UINT32 v);

	void set_nzv8(UINT8 v);
	void set_nzv16(UINT16 v);
	void set_nzv32(UINT32 v);

	void set_nz16(UINT16 v);
	void set_nz32(UINT32 v);

	inline void r8_w(int reg, UINT8 val) {
		if(reg & 8)
			R[reg & 7] = (R[reg & 7] & 0xff00) | val;
		else
			R[reg & 7] = (R[reg & 7] & 0xff) | (val << 8);
	}

	inline UINT8 r8_r(int reg) {
		if(reg & 8)
			return R[reg & 7];
		else
			return R[reg & 7] >> 8;
	}

	// Note that the decode is so that there's no risk of a h8-300
	// hitting the E registers even with the 0xf mask - the
	// instruction would not be called in the first place
	//
	// Well, except for the instructions where the h8-300 mode is r16
	// and the h8-300h is r32 of course, we have to be careful to mask
	// in h8.lst there if the top bit is 1.

	inline void r16_w(int reg, UINT16 val) { R[reg & 0xf] = val; }
	inline UINT16 r16_r(int reg) { return R[reg & 0xf]; }

#define O(o) void o ## _full(); void o ## _partial()
	O(add_b_imm8_r8u); O(add_b_r8h_r8l); O(add_w_imm16_r16l); O(add_w_r16h_r16l);
	O(adds_l_one_r16l); O(adds_l_two_r16l); O(adds_l_four_r16l);
	O(addx_b_imm8_r8u); O(addx_b_r8h_r8l);
	O(and_b_imm8_r8u); O(and_w_imm16_r16l); O(and_b_r8h_r8l);
	O(andc_imm8_ccr);
	O(band_imm3_abs16); O(band_imm3_abs8); O(band_imm3_r8l); O(band_imm3_r16ihh);
	O(bcc_rel8);
	O(bclr_imm3_abs16); O(bclr_imm3_abs8); O(bclr_imm3_r8l); O(bclr_imm3_r16ihh); O(bclr_r8h_abs16); O(bclr_r8h_abs8); O(bclr_r8h_r8l); O(bclr_r8h_r16ihh);
	O(bcs_rel8);
	O(beq_rel8);
	O(bf_rel8);
	O(bge_rel8);
	O(bgt_rel8);
	O(bhi_rel8);
	O(biand_imm3_abs16); O(biand_imm3_abs8); O(biand_imm3_r8l); O(biand_imm3_r16ihh);
	O(bild_imm3_abs16); O(bild_imm3_abs8); O(bild_imm3_r8l); O(bild_imm3_r16ihh);
	O(bior_imm3_abs16); O(bior_imm3_abs8); O(bior_imm3_r8l); O(bior_imm3_r16ihh);
	O(bist_imm3_abs16); O(bist_imm3_abs8); O(bist_imm3_r8l); O(bist_imm3_r16ihh);
	O(bixor_imm3_abs16); O(bixor_imm3_abs8); O(bixor_imm3_r8l); O(bixor_imm3_r16ihh);
	O(bld_imm3_abs16); O(bld_imm3_abs8); O(bld_imm3_r8l); O(bld_imm3_r16ihh);
	O(ble_rel8);
	O(bls_rel8);
	O(blt_rel8);
	O(bmi_rel8);
	O(bne_rel8);
	O(bnot_imm3_abs16); O(bnot_imm3_abs8); O(bnot_imm3_r8l); O(bnot_imm3_r16ihh);
	O(bnot_r8h_abs16); O(bnot_r8h_abs8); O(bnot_r8h_r8l); O(bnot_r8h_r16ihh);
	O(bor_imm3_abs16); O(bor_imm3_abs8); O(bor_imm3_r8l); O(bor_imm3_r16ihh);
	O(bpl_rel8);
	O(bset_imm3_abs16); O(bset_imm3_abs8); O(bset_imm3_r8l); O(bset_imm3_r16ihh);
	O(bset_r8h_abs16); O(bset_r8h_abs8); O(bset_r8h_r8l); O(bset_r8h_r16ihh);
	O(bsr_rel8);
	O(bst_imm3_abs16); O(bst_imm3_abs8); O(bst_imm3_r8l); O(bst_imm3_r16ihh);
	O(bt_rel8);
	O(btst_imm3_abs16); O(btst_imm3_abs8); O(btst_imm3_r8l); O(btst_imm3_r16ihh);
	O(btst_r8h_abs16); O(btst_r8h_abs8); O(btst_r8h_r8l); O(btst_r8h_r16ihh);
	O(bvc_rel8);
	O(bvs_rel8);
	O(bxor_imm3_abs16); O(bxor_imm3_abs8); O(bxor_imm3_r8l); O(bxor_imm3_r16ihh);
	O(cmp_b_imm8_r8u); O(cmp_b_r8h_r8l); O(cmp_w_imm16_r16l); O(cmp_w_r16h_r16l);
	O(daa_b_r8l);
	O(das_b_r8l);
	O(dec_b_one_r8l); O(dec_w_one_r16l); O(dec_w_two_r16l);
	O(divxu_b_r8h_r16l);
	O(eepmov_b);
	O(inc_b_one_r8l);
	O(jmp_abs8i); O(jmp_abs16e);
	O(jsr_abs8i); O(jsr_abs16e); O(jsr_r16h);
	O(ldc_imm8_ccr); O(ldc_r8l_ccr);
	O(mov_b_abs16_r8l); O(mov_b_abs8_r8u); O(mov_b_imm8_r8u); O(mov_b_r8h_r8l); O(mov_b_r8l_abs16); O(mov_b_r8u_abs8); O(mov_b_r16ih_r8l); O(mov_b_r8l_r16ih); O(mov_b_r16d16h_r8l); O(mov_b_r8l_r16d16h); O(mov_b_r16ph_r8l); O(mov_b_r8l_pr16h);
	O(mov_w_abs16_r16l); O(mov_w_imm16_r16l); O(mov_w_r16h_r16l); O(mov_w_r16l_abs16); O(mov_w_r16ih_r16l); O(mov_w_r16l_r16ih); O(mov_w_r16ph_r16l); O(mov_w_r16l_pr16h); O(mov_w_r16l_r16d16h); O(mov_w_r16d16h_r16l);
	O(movfpe_abs16_r8l);
	O(movtpe_r8l_abs16);
	O(mulxu_b_r8h_r16l);
	O(neg_b_r8l);
	O(nop);
	O(not_b_r8l);
	O(or_b_imm8_r8u); O(or_b_r8h_r8l); O(or_w_imm16_r16l);
	O(orc_imm8_ccr);
	O(rotl_b_r8l);
	O(rotr_b_r8l);
	O(rotxl_b_r8l);
	O(rotxr_b_r8l);
	O(rte);
	O(rts);
	O(shal_b_r8l);
	O(shar_b_r8l);
	O(shll_b_r8l);
	O(shlr_b_r8l);
	O(sleep);
	O(stc_ccr_r8l); O(stc_exr_r8l);
	O(sub_b_r8h_r8l); O(sub_w_imm16_r16l); O(sub_w_r16h_r16l);
	O(subs_l_one_r16l); O(subs_l_two_r16l); O(subs_l_four_r16l);
	O(subx_b_imm8_r8u); O(subx_b_r8h_r8l);
	O(xor_b_imm8_r8u); O(xor_b_r8h_r8l); O(xor_w_imm16_r16l);
	O(xorc_imm8_ccr);

	O(dispatch_0100);
	O(dispatch_01007800);
	O(dispatch_0110);
	O(dispatch_0120);
	O(dispatch_0130);
	O(dispatch_0140);
	O(dispatch_01407800);
	O(dispatch_01407880);
	O(dispatch_0141);
	O(dispatch_01417800);
	O(dispatch_01417880);
	O(dispatch_0160);
	O(dispatch_01c0);
	O(dispatch_01d0);
	O(dispatch_01e0);
	O(dispatch_01f0);
	O(dispatch_6a10);
	O(dispatch_6a18);
	O(dispatch_6a30);
	O(dispatch_6a38);
	O(dispatch_7800);
	O(dispatch_7b5c);
	O(dispatch_7bd4);
	O(dispatch_7c00);
	O(dispatch_7d00);
	O(dispatch_7e00);
	O(dispatch_7f00);

	O(state_reset);
	O(state_irq);
#undef O
};

enum {
	H8_PC  = 1,
	H8_R0,
	H8_R1,
	H8_R2,
	H8_R3,
	H8_R4,
	H8_R5,
	H8_R6,
	H8_R7,
	H8_E0,
	H8_E1,
	H8_E2,
	H8_E3,
	H8_E4,
	H8_E5,
	H8_E6,
	H8_E7,
	H8_CCR,
	H8_EXR
};

#endif
