// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
/***************************************************************************

    mcs96.h

    MCS96

***************************************************************************/

#ifndef MAME_CPU_MCS96_MCS96_H
#define MAME_CPU_MCS96_MCS96_H

#pragma once

class mcs96_device : public cpu_device {
public:
	enum {
		MCS96_PC = 1,
		MCS96_PSW,
		MCS96_INT_PENDING,
		MCS96_SP,
		MCS96_AX, MCS96_AL, MCS96_AH,
		MCS96_DX, MCS96_DL, MCS96_DH,
		MCS96_BX, MCS96_BL, MCS96_BH,
		MCS96_CX, MCS96_CL, MCS96_CH,
		MCS96_LAST_REG = MCS96_CH
	};

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


	mcs96_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int data_width, address_map_constructor regs_map);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override;
	virtual uint32_t execute_max_cycles() const noexcept override;
	virtual void execute_run() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;
	virtual bool memory_translate(int spacenum, int intention, offs_t &address, address_space *&target_space) override;

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	address_space_config program_config, regs_config;
	address_space *program, *regs;
	memory_access<16, 0, 0, ENDIANNESS_LITTLE>::cache m_cache8;
	memory_access<16, 1, 0, ENDIANNESS_LITTLE>::cache m_cache16;
	std::function<u8 (offs_t address)> m_pr8;
	required_shared_ptr<u16> register_file;

	int icount, bcount, inst_state, cycles_scaling;
	uint8_t pending_irq;
	uint16_t PC, PPC, PSW;
	uint16_t OP1;
	uint8_t OP2, OP3, OPI;
	uint32_t TMP;
	bool irq_requested;

	virtual void do_exec_full() = 0;
	virtual void do_exec_partial() = 0;
	virtual void internal_update(uint64_t current_time) = 0;

	void recompute_bcount(uint64_t event_time);

	inline void next(int cycles) { icount -= cycles_scaling*cycles; inst_state = STATE_FETCH; }
	inline void next_noirq(int cycles) { icount -= cycles_scaling*cycles; inst_state = STATE_FETCH_NOIRQ; }
	void check_irq();
	inline uint8_t read_pc() { return m_pr8(PC++); }

	void int_mask_w(u8 data);
	u8 int_mask_r();
	void int_pending_w(u8 data);
	u8 int_pending_r();

	void reg_w8(uint8_t adr, uint8_t data);
	void reg_w16(uint8_t adr, uint16_t data);
	void any_w8(uint16_t adr, uint8_t data);
	void any_w16(uint16_t adr, uint16_t data);

	uint8_t reg_r8(uint8_t adr);
	uint16_t reg_r16(uint8_t adr);
	uint8_t any_r8(uint16_t adr);
	uint16_t any_r16(uint16_t adr);

	uint8_t do_addb(uint8_t v1, uint8_t v2);
	uint16_t do_add(uint16_t v1, uint16_t v2);
	uint8_t do_subb(uint8_t v1, uint8_t v2);
	uint16_t do_sub(uint16_t v1, uint16_t v2);

	uint8_t do_addcb(uint8_t v1, uint8_t v2);
	uint16_t do_addc(uint16_t v1, uint16_t v2);
	uint8_t do_subcb(uint8_t v1, uint8_t v2);
	uint16_t do_subc(uint16_t v1, uint16_t v2);

	void set_nz8(uint8_t v);
	void set_nz16(uint16_t v);

#define O(o) void o ## _full(); void o ## _partial()

	O(add_direct_2w); O(add_direct_3w); O(add_immed_2w); O(add_immed_3w); O(add_indexed_2w); O(add_indexed_3w); O(add_indirect_2w); O(add_indirect_3w);
	O(addb_direct_2b); O(addb_direct_3b); O(addb_immed_2b); O(addb_immed_3b); O(addb_indexed_2b); O(addb_indexed_3b); O(addb_indirect_2b); O(addb_indirect_3b);
	O(addc_direct_2w); O(addc_immed_2w); O(addc_indexed_2w); O(addc_indirect_2w);
	O(addcb_direct_2b); O(addcb_immed_2b); O(addcb_indexed_2b); O(addcb_indirect_2b);
	O(and_direct_2w); O(and_direct_3w); O(and_immed_2w); O(and_immed_3w); O(and_indexed_2w); O(and_indexed_3w); O(and_indirect_2w); O(and_indirect_3w);
	O(andb_direct_2b); O(andb_direct_3b); O(andb_immed_2b); O(andb_immed_3b); O(andb_indexed_2b); O(andb_indexed_3b); O(andb_indirect_2b); O(andb_indirect_3b);
	O(br_indirect_1n);
	O(clr_direct_1w);
	O(clrb_direct_1b);
	O(clrc_none);
	O(clrvt_none);
	O(cmp_direct_2w); O(cmp_immed_2w); O(cmp_indexed_2w); O(cmp_indirect_2w);
	O(cmpb_direct_2b); O(cmpb_immed_2b); O(cmpb_indexed_2b); O(cmpb_indirect_2b);
	O(dec_direct_1w);
	O(decb_direct_1b);
	O(di_none);
	O(div_direct_2w); O(div_immed_2w); O(div_indexed_2w); O(div_indirect_2w);
	O(divb_direct_2e); O(divb_immed_2e); O(divb_indexed_2w); O(divb_indirect_2w);
	O(divu_direct_2w); O(divu_immed_2w); O(divu_indexed_2w); O(divu_indirect_2w);
	O(divub_direct_2e); O(divub_immed_2e); O(divub_indexed_2w); O(divub_indirect_2w);
	O(djnz_rrel8);
	O(djnzw_wrrel8);
	O(ei_none);
	O(ext_direct_1w);
	O(extb_direct_1b);
	O(idlpd_none);
	O(inc_direct_1w);
	O(incb_direct_1b);
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
	O(ld_direct_2w); O(ld_immed_2w); O(ld_indexed_2w); O(ld_indirect_2w);
	O(ldb_direct_2b); O(ldb_immed_2b); O(ldb_indexed_2b); O(ldb_indirect_2b);
	O(ldbse_direct_2e); O(ldbse_immed_2e); O(ldbse_indexed_2w); O(ldbse_indirect_2w);
	O(ldbze_direct_2e); O(ldbze_immed_2e); O(ldbze_indexed_2w); O(ldbze_indirect_2w);
	O(ljmp_rel16);
	O(mul_direct_2w); O(mul_direct_3w); O(mul_immed_2w); O(mul_immed_3w); O(mul_indexed_2w); O(mul_indexed_3w); O(mul_indirect_2w); O(mul_indirect_3w);
	O(mulb_direct_2b); O(mulb_direct_3e); O(mulb_immed_2b); O(mulb_immed_3e); O(mulb_indexed_2b); O(mulb_indexed_3e); O(mulb_indirect_2b); O(mulb_indirect_3e);
	O(mulu_direct_2w); O(mulu_direct_3w); O(mulu_immed_2w); O(mulu_immed_3w); O(mulu_indexed_2w); O(mulu_indexed_3w); O(mulu_indirect_2w); O(mulu_indirect_3w);
	O(mulub_direct_2b); O(mulub_direct_3e); O(mulub_immed_2b); O(mulub_immed_3e); O(mulub_indexed_2b); O(mulub_indexed_3e); O(mulub_indirect_2b); O(mulub_indirect_3e);
	O(neg_direct_1w);
	O(negb_direct_1b);
	O(nop_none);
	O(norml_direct_2e);
	O(not_direct_1w);
	O(notb_direct_1b);
	O(or_direct_2w); O(or_immed_2w); O(or_indexed_2w); O(or_indirect_2w);
	O(orb_direct_2b); O(orb_immed_2b); O(orb_indexed_2b); O(orb_indirect_2b);
	O(pop_direct_1w); O(pop_indexed_1w); O(pop_indirect_1w);
	O(popf_none);
	O(push_direct_1w); O(push_immed_1w); O(push_indexed_1w); O(push_indirect_1w);
	O(pushf_none);
	O(ret_none);
	O(rst_none);
	O(scall_rel11);
	O(setc_none);
	O(shl_immed_or_reg_2w);
	O(shlb_immed_or_reg_2b);
	O(shll_immed_or_reg_2w);
	O(shr_immed_or_reg_2w);
	O(shra_immed_or_reg_2w);
	O(shrab_immed_or_reg_2b);
	O(shral_immed_or_reg_2w);
	O(shrb_immed_or_reg_2b);
	O(shrl_immed_or_reg_2w);
	O(sjmp_rel11);
	O(skip_immed_1b);
	O(st_direct_2w); O(st_indexed_2w); O(st_indirect_2w);
	O(stb_direct_2b); O(stb_indexed_2b); O(stb_indirect_2b);
	O(sub_direct_2w); O(sub_direct_3w); O(sub_immed_2w); O(sub_immed_3w); O(sub_indexed_2w); O(sub_indexed_3w); O(sub_indirect_2w); O(sub_indirect_3w);
	O(subb_direct_2b); O(subb_direct_3b); O(subb_immed_2b); O(subb_immed_3b); O(subb_indexed_2b); O(subb_indexed_3b); O(subb_indirect_2b); O(subb_indirect_3b);
	O(subc_direct_2w); O(subc_immed_2w); O(subc_indexed_2w); O(subc_indirect_2w);
	O(subcb_direct_2b); O(subcb_immed_2b); O(subcb_indexed_2b); O(subcb_indirect_2b);
	O(trap_none);
	O(xch_direct_2w);
	O(xchb_direct_2b);
	O(xor_direct_2w); O(xor_immed_2w); O(xor_indexed_2w); O(xor_indirect_2w);
	O(xorb_direct_2b); O(xorb_immed_2b); O(xorb_indexed_2b); O(xorb_indirect_2b);

	O(fetch);
	O(fetch_noirq);

#undef O
};

#endif // MAME_CPU_MCS96_MCS96_H
