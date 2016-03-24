// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
/***************************************************************************

    mcs96.h

    MCS96

***************************************************************************/

#ifndef __MCS96_H__
#define __MCS96_H__

class mcs96_device : public cpu_device {
public:
	enum {
		EXINT_LINE = 1
	};

	mcs96_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, int data_width, const char *shortname, const char *source);

protected:
	enum {
		STATE_FETCH = 0x200,
		STATE_FETCH_NOIRQ = 0x201
	};

	enum {
		F_ST = 0x0100,
		F_I  = 0x0200,
		F_C  = 0x0800,
		F_VT = 0x1000,
		F_V  = 0x2000,
		F_N  = 0x4000,
		F_Z  = 0x8000
	};

	struct disasm_entry {
		const char *opcode, *opcode_fe;
		int mode;
		offs_t flags;
	};

	enum {
		DASM_none,              /* No parameters */
		DASM_nop_2,             /* One ignored parameter byte */
		DASM_rel8,              /* Relative, 8 bits */
		DASM_rel11,             /* Relative, 11 bits */
		DASM_rel16,             /* Relative, 16 bits */
		DASM_rrel8,             /* Register + relative, 8 bits */
		DASM_brrel8,            /* Bit test + register + relative, 8 bits */
		DASM_direct_1,          /* Register-direct references, 1 operator */
		DASM_direct_2,          /* Register-direct references, 2 operators */
		DASM_direct_3,          /* Register-direct references, 3 operators */
		DASM_immed_1b,          /* Immediate references to byte, 1 operator */
		DASM_immed_2b,          /* Immediate references to byte, 2 operators */
		DASM_immed_or_reg_2b,   /* Immediate references to byte or register, 2 operators */
		DASM_immed_3b,          /* Immediate references to byte, 3 operators */
		DASM_immed_1w,          /* Immediate references to word, 1 operator */
		DASM_immed_2w,          /* Immediate references to word, 2 operators */
		DASM_immed_3w,          /* Immediate references to word, 3 operators */
		DASM_indirect_1n,       /* Indirect normal, 1 operator */
		DASM_indirect_1,        /* Indirect, normal or auto-incrementing, 1 operator */
		DASM_indirect_2,        /* Indirect, normal or auto-incrementing, 2 operators */
		DASM_indirect_3,        /* Indirect, normal or auto-incrementing, 3 operators */
		DASM_indexed_1,         /* Indexed, short or long, 1 operator */
		DASM_indexed_2,         /* Indexed, short or long, 2 operators */
		DASM_indexed_3         /* Indexed, short or long, 3 operators */
	};

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const override;
	virtual UINT32 execute_max_cycles() const override;
	virtual UINT32 execute_input_lines() const override;
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const override;
	virtual UINT32 disasm_max_opcode_bytes() const override;
	virtual offs_t disasm_generic(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options, const disasm_entry *entries);

	address_space_config program_config;
	address_space *program;
	direct_read_data *direct;

	int icount, bcount, inst_state, cycles_scaling;
	UINT8 pending_irq;
	UINT16 PC, PPC, PSW;
	UINT16 OP1;
	UINT8 OP2, OP3, OPI;
	UINT32 TMP;
	UINT16 R[0x74];
	bool irq_requested;

	virtual void do_exec_full() = 0;
	virtual void do_exec_partial() = 0;
	virtual void internal_update(UINT64 current_time) = 0;
	virtual void io_w8(UINT8 adr, UINT8 data) = 0;
	virtual void io_w16(UINT8 adr, UINT16 data) = 0;
	virtual UINT8 io_r8(UINT8 adr) = 0;
	virtual UINT16 io_r16(UINT8 adr) = 0;

	void recompute_bcount(UINT64 event_time);
	static std::string regname(UINT8 reg);

	inline void next(int cycles) { icount -= cycles_scaling*cycles; inst_state = STATE_FETCH; }
	inline void next_noirq(int cycles) { icount -= cycles_scaling*cycles; inst_state = STATE_FETCH_NOIRQ; }
	void check_irq();
	inline UINT8 read_pc() { return direct->read_byte(PC++); }

	void reg_w8(UINT8 adr, UINT8 data);
	void reg_w16(UINT8 adr, UINT16 data);
	void any_w8(UINT16 adr, UINT8 data);
	void any_w16(UINT16 adr, UINT16 data);

	UINT8 reg_r8(UINT8 adr);
	UINT16 reg_r16(UINT8 adr);
	UINT8 any_r8(UINT16 adr);
	UINT16 any_r16(UINT16 adr);

	UINT8 do_addb(UINT8 v1, UINT8 v2);
	UINT16 do_add(UINT16 v1, UINT16 v2);
	UINT8 do_subb(UINT8 v1, UINT8 v2);
	UINT16 do_sub(UINT16 v1, UINT16 v2);

	UINT8 do_addcb(UINT8 v1, UINT8 v2);
	UINT16 do_addc(UINT16 v1, UINT16 v2);
	UINT8 do_subcb(UINT8 v1, UINT8 v2);
	UINT16 do_subc(UINT16 v1, UINT16 v2);

	void set_nz8(UINT8 v);
	void set_nz16(UINT16 v);

#define O(o) void o ## _full(); void o ## _partial()

	O(add_direct_2); O(add_direct_3); O(add_immed_2w); O(add_immed_3w); O(add_indexed_2); O(add_indexed_3); O(add_indirect_2); O(add_indirect_3);
	O(addb_direct_2); O(addb_direct_3); O(addb_immed_2b); O(addb_immed_3b); O(addb_indexed_2); O(addb_indexed_3); O(addb_indirect_2); O(addb_indirect_3);
	O(addc_direct_2); O(addc_immed_2w); O(addc_indexed_2); O(addc_indirect_2);
	O(addcb_direct_2); O(addcb_immed_2w); O(addcb_indexed_2); O(addcb_indirect_2);
	O(and_direct_2); O(and_direct_3); O(and_immed_2w); O(and_immed_3w); O(and_indexed_2); O(and_indexed_3); O(and_indirect_2); O(and_indirect_3);
	O(andb_direct_2); O(andb_direct_3); O(andb_immed_2b); O(andb_immed_3b); O(andb_indexed_2); O(andb_indexed_3); O(andb_indirect_2); O(andb_indirect_3);
	O(br_indirect_1n);
	O(clr_direct_1);
	O(clrb_direct_1);
	O(clrc_none);
	O(clrvt_none);
	O(cmp_direct_2); O(cmp_immed_2w); O(cmp_indexed_2); O(cmp_indirect_2);
	O(cmpb_direct_2); O(cmpb_immed_2b); O(cmpb_indexed_2); O(cmpb_indirect_2);
	O(dec_direct_1);
	O(decb_direct_1);
	O(di_none);
	O(div_direct_2); O(div_immed_2w); O(div_indexed_2); O(div_indirect_2);
	O(divb_direct_2); O(divb_immed_2b); O(divb_indexed_2); O(divb_indirect_2);
	O(divu_direct_2); O(divu_immed_2w); O(divu_indexed_2); O(divu_indirect_2);
	O(divub_direct_2); O(divub_immed_2b); O(divub_indexed_2); O(divub_indirect_2);
	O(djnz_rrel8);
	O(djnzw_rrel8);
	O(ei_none);
	O(ext_direct_1);
	O(extb_direct_1);
	O(idlpd_none);
	O(inc_direct_1);
	O(incb_direct_1);
	O(jbc_brrel8);
	O(jbs_brrel8);
	O(jc_rel8);
	O(je_rel8);
	O(jge_rel8);
	O(jgt_rel8);
	O(jh_rel8);
	O(jle_rel8);
	O(jlt_rel8);
	O(jnc_rel8);
	O(jne_rel8);
	O(jnh_rel8);
	O(jnst_rel8);
	O(jnv_rel8);
	O(jnvt_rel8);
	O(jst_rel8);
	O(jv_rel8);
	O(jvt_rel8);
	O(lcall_rel16);
	O(ld_direct_2); O(ld_immed_2w); O(ld_indexed_2); O(ld_indirect_2);
	O(ldb_direct_2); O(ldb_immed_2b); O(ldb_indexed_2); O(ldb_indirect_2);
	O(ldbse_direct_2); O(ldbse_immed_2b); O(ldbse_indexed_2); O(ldbse_indirect_2);
	O(ldbze_direct_2); O(ldbze_immed_2b); O(ldbze_indexed_2); O(ldbze_indirect_2);
	O(ljmp_rel16);
	O(mul_direct_2); O(mul_direct_3); O(mul_immed_2w); O(mul_immed_3w); O(mul_indexed_2); O(mul_indexed_3); O(mul_indirect_2); O(mul_indirect_3);
	O(mulb_direct_2); O(mulb_direct_3); O(mulb_immed_2b); O(mulb_immed_3b); O(mulb_indexed_2); O(mulb_indexed_3); O(mulb_indirect_2); O(mulb_indirect_3);
	O(mulu_direct_2); O(mulu_direct_3); O(mulu_immed_2w); O(mulu_immed_3w); O(mulu_indexed_2); O(mulu_indexed_3); O(mulu_indirect_2); O(mulu_indirect_3);
	O(mulub_direct_2); O(mulub_direct_3); O(mulub_immed_2b); O(mulub_immed_3b); O(mulub_indexed_2); O(mulub_indexed_3); O(mulub_indirect_2); O(mulub_indirect_3);
	O(neg_direct_1);
	O(negb_direct_1);
	O(nop_none);
	O(norml_direct_2);
	O(not_direct_1);
	O(notb_direct_1);
	O(or_direct_2); O(or_immed_2w); O(or_indexed_2); O(or_indirect_2);
	O(orb_direct_2); O(orb_immed_2b); O(orb_indexed_2); O(orb_indirect_2);
	O(pop_direct_1); O(pop_indexed_1); O(pop_indirect_1);
	O(popf_none);
	O(push_direct_1); O(push_immed_1w); O(push_indexed_1); O(push_indirect_1);
	O(pushf_none);
	O(ret_none);
	O(rst_none);
	O(scall_rel11);
	O(setc_none);
	O(shl_immed_or_reg_2b);
	O(shlb_immed_or_reg_2b);
	O(shll_immed_or_reg_2b);
	O(shr_immed_or_reg_2b);
	O(shra_immed_or_reg_2b);
	O(shrab_immed_or_reg_2b);
	O(shral_immed_or_reg_2b);
	O(shrb_immed_or_reg_2b);
	O(shrl_immed_or_reg_2b);
	O(sjmp_rel11);
	O(skip_immed_1b);
	O(st_direct_2); O(st_indexed_2); O(st_indirect_2);
	O(stb_direct_2); O(stb_indexed_2); O(stb_indirect_2);
	O(sub_direct_2); O(sub_direct_3); O(sub_immed_2w); O(sub_immed_3w); O(sub_indexed_2); O(sub_indexed_3); O(sub_indirect_2); O(sub_indirect_3);
	O(subb_direct_2); O(subb_direct_3); O(subb_immed_2b); O(subb_immed_3b); O(subb_indexed_2); O(subb_indexed_3); O(subb_indirect_2); O(subb_indirect_3);
	O(subc_direct_2); O(subc_immed_2w); O(subc_indexed_2); O(subc_indirect_2);
	O(subcb_direct_2); O(subcb_immed_2w); O(subcb_indexed_2); O(subcb_indirect_2);
	O(trap_none);
	O(xch_direct_2);
	O(xchb_direct_2);
	O(xor_direct_2); O(xor_immed_2w); O(xor_indexed_2); O(xor_indirect_2);
	O(xorb_direct_2); O(xorb_immed_2b); O(xorb_indexed_2); O(xorb_indirect_2);

	O(fetch);
	O(fetch_noirq);

#undef O
};

enum {
	MCS96_PC = 1,
	MCS96_PSW,
	MCS96_R       // 0x74 entries
};

#endif
