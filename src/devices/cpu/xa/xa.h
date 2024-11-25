// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_CPU_XA_XA_H
#define MAME_CPU_XA_XA_H

#pragma once

enum {
	XA_EXT_IRQ0,
	XA_EXT_IRQ1,
	XA_EXT_IRQ2,
	XA_EXT_IRQ3,
};

enum {
	XA_BANK0_R0,
	XA_BANK0_R1,
	XA_BANK0_R2,
	XA_BANK0_R3,

	XA_BANK1_R0,
	XA_BANK1_R1,
	XA_BANK1_R2,
	XA_BANK1_R3,

	XA_BANK2_R0,
	XA_BANK2_R1,
	XA_BANK2_R2,
	XA_BANK2_R3,

	XA_BANK3_R0,
	XA_BANK3_R1,
	XA_BANK3_R2,
	XA_BANK3_R3,

	XA_R4,
	XA_R5,
	XA_R6,
	XA_R7,

	XA_USP,
	XA_SSP,
};


class xa_cpu : public cpu_device
{
public:
	xa_cpu(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <unsigned N> auto port_in_cb() { return m_port_in_cb[N].bind(); }
	template <unsigned N> auto port_out_cb() { return m_port_out_cb[N].bind(); }

protected:
	xa_cpu(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, uint32_t clock, address_map_constructor prg_map, address_map_constructor dat_map);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual uint32_t execute_min_cycles() const noexcept override { return 1; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 5; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	virtual space_config_vector memory_space_config() const override;

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

private:
	void internal_map(address_map &map) ATTR_COLD;
	void internal_data_map(address_map &map) ATTR_COLD;
	void data_map(address_map &map) ATTR_COLD;
	void sfr_map(address_map &map) ATTR_COLD;

	address_space_config m_program_config;
	address_space_config m_data_config;
	address_space_config m_sfr_config;

	struct mem_info {
		int addr;
		const char *name;
	};


	static const mem_info default_names[];
	void add_names(const mem_info *info);

	std::string get_data_address(u16 arg) const;

	typedef void (xa_cpu::*op_func) (u8 op);
	static const op_func s_instruction[256];

	const char* m_regnames16[16] = { "R0", "R1", "R2", "R3", "R4", "R5", "R6", "R7", "illegal", "illegal", "illegal", "illegal", "illegal", "illegal", "illegal", "illegal" };
	const char* m_regnames8[16] = { "R0L", "R0H", "R1L", "R1H", "R2L", "R2H", "R3L", "R3H", "R4L", "R4H", "R5L", "R5H", "R6L", "R6H", "R7L", "R7H"};

	std::string get_bittext(int bit);
	std::string get_directtext(int bit);
	std::string show_expanded_data4(u16 data4, int size);
	std::string get_word_reglist(u8 op2);
	std::string get_byte_reglist(u8 op2, int h);

	void check_external_irq_level(int level);
	void check_interrupts();

	u16 expand_rel16(u16 rel16);
	u16 expand_rel8(u8 rel8);

	void cy(u8 cycles)
	{
		m_icount -= cycles;
	}

	void do_nz_flags_16(u16 data);
	void do_nz_flags_8(u8 data);

	u8 get_n_flag() { return m_nflag; }
	void set_n_flag() { m_nflag = 1; }
	void clear_n_flag() { m_nflag = 0; }

	u8 get_z_flag() { return m_zflag; }
	void set_z_flag()  { m_zflag = 1; }
	void clear_z_flag() { m_zflag = 0; }

	u8 get_c_flag() { return m_cflag; }
	void set_c_flag()  { m_cflag = 1; }
	void clear_c_flag() { m_cflag = 0; }

	u8 get_v_flag() { return m_vflag; }
	void set_v_flag()  { m_vflag = 1; }
	void clear_v_flag() { m_vflag = 0; }

	u8 get_ac_flag() { return m_acflag; }
	void set_ac_flag()  { m_acflag = 1; }
	void clear_ac_flag() { m_acflag = 0; }


	u8 gr8(int reg);
	void sr8(int reg, u8 data);

	u16 gr16(int reg);
	void sr16(int reg, u16 data);

	u32 gr32(int reg);
	void sr32(int reg, u32 data);

	void set_pc_in_current_page(u16 addr);

	void push_word_reglist(u8 op2, int h, bool force_user);
	void pull_word_reglist(u8 op2, int h, bool force_user);
	void push_byte_reglist(u8 op2, int h, bool force_user);
	void pull_byte_reglist(u8 op2, int h, bool force_user);

	void push_word_to_user_stack(u16 data);
	void push_word_to_system_stack(u16 data);
	void push_word_to_stack(u16 data);

	u16 pull_word_from_user_stack();
	u16 pull_word_from_system_stack();
	u16 pull_word_from_stack();

	void push_byte_to_user_stack(u8 data);
	void push_byte_to_system_stack(u8 data);
	void push_byte_to_stack(u16 data);

	u8 pull_byte_from_user_stack();
	u8 pull_byte_from_system_stack();
	u8 pull_byte_from_stack();

	void wdat8(int address, u8 data);
	void wdat16(int address, u16 data);

	u8 rdat8(int address);
	u16 rdat16(int address);

	u8 sfr_WDCON_r();

	u8 sfr_PSWL_r();
	void sfr_PSWL_w(u8 data);

	u8 sfr_PSWH_r();
	void sfr_PSWH_w(u8 data);
	void sfr_PSW51_w(u8 data);

	void sfr_SCR_w(u8 data);
	void sfr_WFEED1_w(u8 data);
	void sfr_WFEED2_w(u8 data);

	u8 sfr_IEL_r();
	void sfr_IEL_w(u8 data);

	u8 sfr_port_r(offs_t offset);
	void sfr_port_w(offs_t offset, u8 data);

	u8 sfr_PxCFGA_r(offs_t offset);
	void sfr_PxCFGA_w(offs_t offset, u8 data);

	u8 sfr_PxCFGB_r(offs_t offset);
	void sfr_PxCFGB_w(offs_t offset, u8 data);

	void set_bit_8_helper(u16 bit, u8 val);

	u32 asl32_helper(u32 fullreg, u8 amount);
	u32 lsr32_helper(u32 fullreg, u8 amount);
	u16 lsr16_helper(u16 fullreg, u8 amount);

	u8 read_direct8(u16 addr);
	u16 read_direct16(u16 addr);

	void write_direct8(u16 addr, u8 data);
	void write_direct16(u16 addr, u16 data);

	u16 do_subb_16(u16 val1, u16 val2);
	u16 do_sub_16(u16 val1, u16 val2);
	u16 do_sub_16_helper(u16 val1, u16 val2, u8 c);

	u16 do_addc_16(u16 val1, u16 val2);
	u16 do_add_16(u16 val1, u16 val2);
	u16 do_add_16_helper(u16 val1, u16 val2, u8 c);

	u16 do_xor_16(u16 val1, u16 val2);
	u16 do_or_16(u16 val1, u16 val2);
	u16 do_and_16(u16 val1, u16 val2);

	u8 do_sub_8(u8 val1, u8 val2);
	u8 do_subb_8(u8 val1, u8 val2);
	u8 do_sub_8_helper(u8 val1, u8 val2, u8 c);

	u8 do_add_8(u8 val1, u8 val2);
	u8 do_addc_8(u8 val1, u8 val2);
	u8 do_add_8_helper(u8 val1, u8 val2, u8 c);

	u8 do_xor_8(u8 val1, u8 val2);
	u8 do_or_8(u8 val1, u8 val2);
	u8 do_and_8(u8 val1, u8 val2);

	u8 do_cjne_8_helper(u8 val1, u8 val2);

	void handle_alu_type0(u8 op, int alu_op);
	void handle_alu_type1(u8 op, u8 op2);
	void handle_push_rlist(u8 op);
	void handle_pushu_rlist(u8 op);
	void handle_pop_rlist(u8 op);
	void handle_popu_rlist(u8 op);
	void handle_adds_movs(u8 op, int which);
	void handle_shift(u8 op, int shift_type);

	void e_illegal(u8 op);

	void e_nop(u8 op);
	void e_bitgroup(u8 op);
	void e_add(u8 op);
	void e_push_rlist(u8 op);
	void e_addc(u8 op);
	void e_pushu_rlist(u8 op);
	void e_sub(u8 op);
	void e_pop_rlist(u8 op);
	void e_subb(u8 op);
	void e_popu_rlist(u8 op);
	void e_lea_offset8(u8 op);
	void e_lea_offset16(u8 op);
	void e_cmp(u8 op);
	void e_xch_type1(u8 op);
	void e_and(u8 op);
	void e_xch_type2(u8 op);
	void e_or(u8 op);
	void e_xor(u8 op);
	void e_movc_rd_rsinc(u8 op);
	void e_mov(u8 op);
	void e_pushpop_djnz_subgroup(u8 op);
	void e_g9_subgroup(u8 op);
	void e_alu(u8 op);
	void e_jb_mov_subgroup(u8 op);
	void e_movdir(u8 op);
	void e_adds(u8 op);
	void e_movx_subgroup(u8 op);
	void e_rr(u8 op);
	void e_movs(u8 op);
	void e_rrc(u8 op);
	void e_lsr_fc(u8 op);
	void e_asl_c(u8 op);
	void e_asr_c(u8 op);
	void e_norm(u8 op);
	void e_lsr_fj(u8 op);
	void e_asl_j(u8 op);
	void e_asr_j(u8 op);
	void e_rl(u8 op);
	void e_rlc(u8 op);
	void e_djnz_cjne(u8 op);
	void e_mulu_b(u8 op);
	void e_divu_b(u8 op);
	void e_mulu_w(u8 op);
	void e_divu_w(u8 op);
	void e_mul_w(u8 op);
	void e_div_w(u8 op);
	void e_div_data8(u8 op);
	void e_div_d16(u8 op);
	void e_divu_d(u8 op);
	void e_div_d(u8 op);
	void e_cjne_d8(u8 op);
	void e_cjne_d16(u8 op);
	void e_jz_rel8(u8 op);
	void e_jnz_rel8(u8 op);
	void e_branch(u8 op);
	void e_bkpt(u8 op);

	void do_nop();

	void add_byte_rd_data8(u8 rd, u8 data8);
	void addc_byte_rd_data8(u8 rd, u8 data8);
	void sub_byte_rd_data8(u8 rd, u8 data8);
	void subb_byte_rd_data8(u8 rd, u8 data8);
	void cmp_byte_rd_data8(u8 rd, u8 data8);
	void and_byte_rd_data8(u8 rd, u8 data8);
	void or_byte_rd_data8(u8 rd, u8 data8);
	void xor_byte_rd_data8(u8 rd, u8 data8);
	void mov_byte_rd_data8(u8 rd, u8 data8);

	void add_byte_indrd_data8(u8 rd, u8 data8);
	void addc_byte_indrd_data8(u8 rd, u8 data8);
	void sub_byte_indrd_data8(u8 rd, u8 data8);
	void subb_byte_indrd_data8(u8 rd, u8 data8);
	void cmp_byte_indrd_data8(u8 rd, u8 data8);
	void and_byte_indrd_data8(u8 rd, u8 data8);
	void or_byte_indrd_data8(u8 rd, u8 data8);
	void xor_byte_indrd_data8(u8 rd, u8 data8);
	void mov_byte_indrd_data8(u8 rd, u8 data8);

	void add_byte_indrdinc_data8(u8 rd, u8 data8);
	void addc_byte_indrdinc_data8(u8 rd, u8 data8);
	void sub_byte_indrdinc_data8(u8 rd, u8 data8);
	void subb_byte_indrdinc_data8(u8 rd, u8 data8);
	void cmp_byte_indrdinc_data8(u8 rd, u8 data8);
	void and_byte_indrdinc_data8(u8 rd, u8 data8);
	void or_byte_indrdinc_data8(u8 rd, u8 data8);
	void xor_byte_indrdinc_data8(u8 rd, u8 data8);
	void mov_byte_indrdinc_data8(u8 rd, u8 data8);

	void add_byte_indrdoff8_data8(u8 rd, u8 offset8, u8 data8);
	void addc_byte_indrdoff8_data8(u8 rd, u8 offset8, u8 data8);
	void sub_byte_indrdoff8_data8(u8 rd, u8 offset8, u8 data8);
	void subb_byte_indrdoff8_data8(u8 rd, u8 offset8, u8 data8);
	void cmp_byte_indrdoff8_data8(u8 rd, u8 offset8, u8 data8);
	void and_byte_indrdoff8_data8(u8 rd, u8 offset8, u8 data8);
	void or_byte_indrdoff8_data8(u8 rd, u8 offset8, u8 data8);
	void xor_byte_indrdoff8_data8(u8 rd, u8 offset8, u8 data8);
	void mov_byte_indrdoff8_data8(u8 rd, u8 offset8, u8 data8);

	void add_byte_indrdoff16_data8(u8 rd, u16 offset16, u8 data8);
	void addc_byte_indrdoff16_data8(u8 rd, u16 offset16, u8 data8);
	void sub_byte_indrdoff16_data8(u8 rd, u16 offset16, u8 data8);
	void subb_byte_indrdoff16_data8(u8 rd, u16 offset16, u8 data8);
	void cmp_byte_indrdoff16_data8(u8 rd, u16 offset16, u8 data8);
	void and_byte_indrdoff16_data8(u8 rd, u16 offset16, u8 data8);
	void or_byte_indrdoff16_data8(u8 rd, u16 offset16, u8 data8);
	void xor_byte_indrdoff16_data8(u8 rd, u16 offset16, u8 data8);
	void mov_byte_indrdoff16_data8(u8 rd, u16 offset16, u8 data8);

	void add_byte_direct_data8(u16 direct, u8 data8);
	void addc_byte_direct_data8(u16 direct, u8 data8);
	void sub_byte_direct_data8(u16 direct, u8 data8);
	void subb_byte_direct_data8(u16 direct, u8 data8);
	void cmp_byte_direct_data8(u16 direct, u8 data8);
	void and_byte_direct_data8(u16 direct, u8 data8);
	void or_byte_direct_data8(u16 direct, u8 data8);
	void xor_byte_direct_data8(u16 direct, u8 data8);
	void mov_byte_direct_data8(u16 direct, u8 data8);

	void add_word_rd_data16(u8 rd, u16 data16);
	void addc_word_rd_data16(u8 rd, u16 data16);
	void sub_word_rd_data16(u8 rd, u16 data16);
	void subb_word_rd_data16(u8 rd, u16 data16);
	void cmp_word_rd_data16(u8 rd, u16 data16);
	void and_word_rd_data16(u8 rd, u16 data16);
	void or_word_rd_data16(u8 rd, u16 data16);
	void xor_word_rd_data16(u8 rd, u16 data16);
	void mov_word_rd_data16(u8 rd, u16 data16);

	void add_word_indrd_data16(u8 rd, u16 data16);
	void addc_word_indrd_data16(u8 rd, u16 data16);
	void sub_word_indrd_data16(u8 rd, u16 data16);
	void subb_word_indrd_data16(u8 rd, u16 data16);
	void cmp_word_indrd_data16(u8 rd, u16 data16);
	void and_word_indrd_data16(u8 rd, u16 data16);
	void or_word_indrd_data16(u8 rd, u16 data16);
	void xor_word_indrd_data16(u8 rd, u16 data16);
	void mov_word_indrd_data16(u8 rd, u16 data16);

	void add_word_indrdinc_data16(u8 rd, u16 data16);
	void addc_word_indrdinc_data16(u8 rd, u16 data16);
	void sub_word_indrdinc_data16(u8 rd, u16 data16);
	void subb_word_indrdinc_data16(u8 rd, u16 data16);
	void cmp_word_indrdinc_data16(u8 rd, u16 data16);
	void and_word_indrdinc_data16(u8 rd, u16 data16);
	void or_word_indrdinc_data16(u8 rd, u16 data16);
	void xor_word_indrdinc_data16(u8 rd, u16 data16);
	void mov_word_indrdinc_data16(u8 rd, u16 data16);

	void add_word_indrdoff8_data16(u8 rd, u8 offset8, u16 data16);
	void addc_word_indrdoff8_data16(u8 rd, u8 offset8, u16 data16);
	void sub_word_indrdoff8_data16(u8 rd, u8 offset8, u16 data16);
	void subb_word_indrdoff8_data16(u8 rd, u8 offset8, u16 data16);
	void cmp_word_indrdoff8_data16(u8 rd, u8 offset8, u16 data16);
	void and_word_indrdoff8_data16(u8 rd, u8 offset8, u16 data16);
	void or_word_indrdoff8_data16(u8 rd, u8 offset8, u16 data16);
	void xor_word_indrdoff8_data16(u8 rd, u8 offset8, u16 data16);
	void mov_word_indrdoff8_data16(u8 rd, u8 offset8, u16 data16);

	void add_word_indrdoff16_data16(u8 rd, u16 offset16, u16 data16);
	void addc_word_indrdoff16_data16(u8 rd, u16 offset16, u16 data16);
	void sub_word_indrdoff16_data16(u8 rd, u16 offset16, u16 data16);
	void subb_word_indrdoff16_data16(u8 rd, u16 offset16, u16 data16);
	void cmp_word_indrdoff16_data16(u8 rd, u16 offset16, u16 data16);
	void and_word_indrdoff16_data16(u8 rd, u16 offset16, u16 data16);
	void or_word_indrdoff16_data16(u8 rd, u16 offset16, u16 data16);
	void xor_word_indrdoff16_data16(u8 rd, u16 offset16, u16 data16);
	void mov_word_indrdoff16_data16(u8 rd, u16 offset16, u16 data16);

	void add_word_direct_data16(u16 direct, u16 data16);
	void addc_word_direct_data16(u16 direct, u16 data16);
	void sub_word_direct_data16(u16 direct, u16 data16);
	void subb_word_direct_data16(u16 direct, u16 data16);
	void cmp_word_direct_data16(u16 direct, u16 data16);
	void and_word_direct_data16(u16 direct, u16 data16);
	void or_word_direct_data16(u16 direct, u16 data16);
	void xor_word_direct_data16(u16 direct, u16 data16);
	void mov_word_direct_data16(u16 direct, u16 data16);

	void aluop_word_rd_rs(int alu_op, u8 rd, u8 rs);
	void aluop_byte_rd_rs(int alu_op, u8 rd, u8 rs);
	void aluop_word_rd_indrs(int alu_op, u8 rd, u8 rs);
	void aluop_byte_rd_indrs(int alu_op, u8 rd, u8 rs);
	void aluop_word_indrd_rs(int alu_op, u8 rd, u8 rs);
	void aluop_byte_indrd_rs(int alu_op, u8 rd, u8 rs);
	void aluop_word_rd_indrsinc(int alu_op, u8 rd, u8 rs);
	void aluop_byte_rd_indrsinc(int alu_op, u8 rd, u8 rs);
	void aluop_word_indrdinc_rs(int alu_op, u8 rd, u8 rs);
	void aluop_byte_indrdinc_rs(int alu_op, u8 rd, u8 rs);
	void aluop_word_rd_rsoff8(int alu_op, u8 rd, u8 rs, u8 offset8);
	void aluop_byte_rd_rsoff8(int alu_op, u8 rd, u8 rs, u8 offset8);
	void aluop_word_rdoff8_rs(int alu_op, u8 rd, u8 offset8, u8 rs);
	void aluop_byte_rdoff8_rs(int alu_op, u8 rd, u8 offset8, u8 rs);
	void aluop_word_rsoff16(int alu_op, u8 rd, u8 rs, u16 offset16);
	void aluop_byte_rsoff16(int alu_op, u8 rd, u8 rs, u16 offset16);
	void aluop_word_rdoff16_rs(int alu_op, u8 rd, u16 offset16, u8 rs);
	void aluop_byte_rdoff16_rs(int alu_op, u8 rd, u16 offset16, u8 rs);
	void aluop_word_rd_direct(int alu_op, u8 rd, u16 direct);
	void aluop_byte_rd_direct(int alu_op, u8 rd, u16 direct);
	void aluop_word_direct_rs(int alu_op, u16 direct, u8 rs);
	void aluop_byte_direct_rs(int alu_op, u16 direct, u8 rs);

	void aluop_byte_rd_data8(int alu_op, u8 rd, u8 data8);
	void aluop_byte_indrd_data8(int alu_op, u8 rd, u8 data8);
	void aluop_byte_indrdinc_data8(int alu_op, u8 rd, u8 data8);
	void aluop_byte_rdoff8_data8(int alu_op, u8 rd, u8 offset8, u8 data8);
	void aluop_byte_rdoff16_data8(int alu_op, u8 rd, u16 offset16, u8 data8);
	void aluop_byte_direct_data8(int alu_op, u16 direct, u8 data8);
	void aluop_byte_rd_data16(int alu_op, u8 rd, u16 data16);
	void aluop_byte_indrd_data16(int alu_op, u8 rd, u16 data16);
	void aluop_byte_indrdinc_data16(int alu_op, u8 rd, u16 data16);
	void aluop_byte_rdoff8_data16(int alu_op, u8 rd, u8 offset8, u16 data16);
	void aluop_byte_rdoff16_data16(int alu_op, u8 rd, u16 offset16, u16 data16);
	void aluop_byte_direct_data16(int alu_op, u16 direct, u16 data16);

	void add_word_rd_rs(u8 rd, u8 rs);
	void addc_word_rd_rs(u8 rd, u8 rs);
	void sub_word_rd_rs(u8 rd, u8 rs);
	void subb_word_rd_rs(u8 rd, u8 rs);
	void cmp_word_rd_rs(u8 rd, u8 rs);
	void and_word_rd_rs(u8 rd, u8 rs);
	void or_word_rd_rs(u8 rd, u8 rs);
	void xor_word_rd_rs(u8 rd, u8 rs);
	void mov_word_rd_rs(u8 rd, u8 rs);

	void add_byte_rd_rs(u8 rd, u8 rs);
	void addc_byte_rd_rs(u8 rd, u8 rs);
	void sub_byte_rd_rs(u8 rd, u8 rs);
	void subb_byte_rd_rs(u8 rd, u8 rs);
	void cmp_byte_rd_rs(u8 rd, u8 rs);
	void and_byte_rd_rs(u8 rd, u8 rs);
	void or_byte_rd_rs(u8 rd, u8 rs);
	void xor_byte_rd_rs(u8 rd, u8 rs);
	void mov_byte_rd_rs(u8 rd, u8 rs);

	void add_word_rd_indrs(u8 rd, u8 rs);
	void addc_word_rd_indrs(u8 rd, u8 rs);
	void sub_word_rd_indrs(u8 rd, u8 rs);
	void subb_word_rd_indrs(u8 rd, u8 rs);
	void cmp_word_rd_indrs(u8 rd, u8 rs);
	void and_word_rd_indrs(u8 rd, u8 rs);
	void or_word_rd_indrs(u8 rd, u8 rs);
	void xor_word_rd_indrs(u8 rd, u8 rs);
	void mov_word_rd_indrs(u8 rd, u8 rs);

	void add_byte_rd_indrs(u8 rd, u8 rs);
	void addc_byte_rd_indrs(u8 rd, u8 rs);
	void sub_byte_rd_indrs(u8 rd, u8 rs);
	void subb_byte_rd_indrs(u8 rd, u8 rs);
	void cmp_byte_rd_indrs(u8 rd, u8 rs);
	void and_byte_rd_indrs(u8 rd, u8 rs);
	void or_byte_rd_indrs(u8 rd, u8 rs);
	void xor_byte_rd_indrs(u8 rd, u8 rs);
	void mov_byte_rd_indrs(u8 rd, u8 rs);

	void add_word_indrd_rs(u8 rd, u8 rs);
	void addc_word_indrd_rs(u8 rd, u8 rs);
	void sub_word_indrd_rs(u8 rd, u8 rs);
	void subb_word_indrd_rs(u8 rd, u8 rs);
	void cmp_word_indrd_rs(u8 rd, u8 rs);
	void and_word_indrd_rs(u8 rd, u8 rs);
	void or_word_indrd_rs(u8 rd, u8 rs);
	void xor_word_indrd_rs(u8 rd, u8 rs);
	void mov_word_indrd_rs(u8 rd, u8 rs);

	void add_byte_indrd_rs(u8 rd, u8 rs);
	void addc_byte_indrd_rs(u8 rd, u8 rs);
	void sub_byte_indrd_rs(u8 rd, u8 rs);
	void subb_byte_indrd_rs(u8 rd, u8 rs);
	void cmp_byte_indrd_rs(u8 rd, u8 rs);
	void and_byte_indrd_rs(u8 rd, u8 rs);
	void or_byte_indrd_rs(u8 rd, u8 rs);
	void xor_byte_indrd_rs(u8 rd, u8 rs);
	void mov_byte_indrd_rs(u8 rd, u8 rs);

	void add_word_rd_indrsinc(u8 rd, u8 rs);
	void addc_word_rd_indrsinc(u8 rd, u8 rs);
	void sub_word_rd_indrsinc(u8 rd, u8 rs);
	void subb_word_rd_indrsinc(u8 rd, u8 rs);
	void cmp_word_rd_indrsinc(u8 rd, u8 rs);
	void and_word_rd_indrsinc(u8 rd, u8 rs);
	void or_word_rd_indrsinc(u8 rd, u8 rs);
	void xor_word_rd_indrsinc(u8 rd, u8 rs);
	void mov_word_rd_indrsinc(u8 rd, u8 rs);

	void add_byte_rd_indrsinc(u8 rd, u8 rs);
	void addc_byte_rd_indrsinc(u8 rd, u8 rs);
	void sub_byte_rd_indrsinc(u8 rd, u8 rs);
	void subb_byte_rd_indrsinc(u8 rd, u8 rs);
	void cmp_byte_rd_indrsinc(u8 rd, u8 rs);
	void and_byte_rd_indrsinc(u8 rd, u8 rs);
	void or_byte_rd_indrsinc(u8 rd, u8 rs);
	void xor_byte_rd_indrsinc(u8 rd, u8 rs);
	void mov_byte_rd_indrsinc(u8 rd, u8 rs);

	void add_word_indrdinc_rs(u8 rd, u8 rs);
	void addc_word_indrdinc_rs(u8 rd, u8 rs);
	void sub_word_indrdinc_rs(u8 rd, u8 rs);
	void subb_word_indrdinc_rs(u8 rd, u8 rs);
	void cmp_word_indrdinc_rs(u8 rd, u8 rs);
	void and_word_indrdinc_rs(u8 rd, u8 rs);
	void or_word_indrdinc_rs(u8 rd, u8 rs);
	void xor_word_indrdinc_rs(u8 rd, u8 rs);
	void mov_word_indrdinc_rs(u8 rd, u8 rs);

	void add_byte_indrdinc_rs(u8 rd, u8 rs);
	void addc_byte_indrdinc_rs(u8 rd, u8 rs);
	void sub_byte_indrdinc_rs(u8 rd, u8 rs);
	void subb_byte_indrdinc_rs(u8 rd, u8 rs);
	void cmp_byte_indrdinc_rs(u8 rd, u8 rs);
	void and_byte_indrdinc_rs(u8 rd, u8 rs);
	void or_byte_indrdinc_rs(u8 rd, u8 rs);
	void xor_byte_indrdinc_rs(u8 rd, u8 rs);
	void mov_byte_indrdinc_rs(u8 rd, u8 rs);

	void add_word_rd_rsoff8(u8 rd, u8 rs, u8 offset8);
	void addc_word_rd_rsoff8(u8 rd, u8 rs, u8 offset8);
	void sub_word_rd_rsoff8(u8 rd, u8 rs, u8 offset8);
	void subb_word_rd_rsoff8(u8 rd, u8 rs, u8 offset8);
	void cmp_word_rd_rsoff8(u8 rd, u8 rs, u8 offset8);
	void and_word_rd_rsoff8(u8 rd, u8 rs, u8 offset8);
	void or_word_rd_rsoff8(u8 rd, u8 rs, u8 offset8);
	void xor_word_rd_rsoff8(u8 rd, u8 rs, u8 offset8);
	void mov_word_rd_rsoff8(u8 rd, u8 rs, u8 offset8);

	void add_byte_rd_rsoff8(u8 rd, u8 rs, u8 offset8);
	void addc_byte_rd_rsoff8(u8 rd, u8 rs, u8 offset8);
	void sub_byte_rd_rsoff8(u8 rd, u8 rs, u8 offset8);
	void subb_byte_rd_rsoff8(u8 rd, u8 rs, u8 offset8);
	void cmp_byte_rd_rsoff8(u8 rd, u8 rs, u8 offset8);
	void and_byte_rd_rsoff8(u8 rd, u8 rs, u8 offset8);
	void or_byte_rd_rsoff8(u8 rd, u8 rs, u8 offset8);
	void xor_byte_rd_rsoff8(u8 rd, u8 rs, u8 offset8);
	void mov_byte_rd_rsoff8(u8 rd, u8 rs, u8 offset8);

	void add_word_rdoff8_rs(u8 rd, u8 offset8, u8 rs);
	void addc_word_rdoff8_rs(u8 rd, u8 offset8, u8 rs);
	void sub_word_rdoff8_rs(u8 rd, u8 offset8, u8 rs);
	void subb_word_rdoff8_rs(u8 rd, u8 offset8, u8 rs);
	void cmp_word_rdoff8_rs(u8 rd, u8 offset8, u8 rs);
	void and_word_rdoff8_rs(u8 rd, u8 offset8, u8 rs);
	void or_word_rdoff8_rs(u8 rd, u8 offset8, u8 rs);
	void xor_word_rdoff8_rs(u8 rd, u8 offset8, u8 rs);
	void mov_word_rdoff8_rs(u8 rd, u8 offset8, u8 rs);

	void add_byte_rdoff8_rs(u8 rd, u8 offset8, u8 rs);
	void addc_byte_rdoff8_rs(u8 rd, u8 offset8, u8 rs);
	void sub_byte_rdoff8_rs(u8 rd, u8 offset8, u8 rs);
	void subb_byte_rdoff8_rs(u8 rd, u8 offset8, u8 rs);
	void cmp_byte_rdoff8_rs(u8 rd, u8 offset8, u8 rs);
	void and_byte_rdoff8_rs(u8 rd, u8 offset8, u8 rs);
	void or_byte_rdoff8_rs(u8 rd, u8 offset8, u8 rs);
	void xor_byte_rdoff8_rs(u8 rd, u8 offset8, u8 rs);
	void mov_byte_rdoff8_rs(u8 rd, u8 offset8, u8 rs);

	void add_word_rd_rsoff16(u8 rd, u8 rs, u16 offset16);
	void addc_word_rd_rsoff16(u8 rd, u8 rs, u16 offset16);
	void sub_word_rd_rsoff16(u8 rd, u8 rs, u16 offset16);
	void subb_word_rd_rsoff16(u8 rd, u8 rs, u16 offset16);
	void cmp_word_rd_rsoff16(u8 rd, u8 rs, u16 offset16);
	void and_word_rd_rsoff16(u8 rd, u8 rs, u16 offset16);
	void or_word_rd_rsoff16(u8 rd, u8 rs, u16 offset16);
	void xor_word_rd_rsoff16(u8 rd, u8 rs, u16 offset16);
	void mov_word_rd_rsoff16(u8 rd, u8 rs, u16 offset16);

	void add_byte_rd_rsoff16(u8 rd, u8 rs, u16 offset16);
	void addc_byte_rd_rsoff16(u8 rd, u8 rs, u16 offset16);
	void sub_byte_rd_rsoff16(u8 rd, u8 rs, u16 offset16);
	void subb_byte_rd_rsoff16(u8 rd, u8 rs, u16 offset16);
	void cmp_byte_rd_rsoff16(u8 rd, u8 rs, u16 offset16);
	void and_byte_rd_rsoff16(u8 rd, u8 rs, u16 offset16);
	void or_byte_rd_rsoff16(u8 rd, u8 rs, u16 offset16);
	void xor_byte_rd_rsoff16(u8 rd, u8 rs, u16 offset16);
	void mov_byte_rd_rsoff16(u8 rd, u8 rs, u16 offset16);

	void add_word_rdoff16_rs(u8 rd, u16 offset16, u8 rs);
	void addc_word_rdoff16_rs(u8 rd, u16 offset16, u8 rs);
	void sub_word_rdoff16_rs(u8 rd, u16 offset16, u8 rs);
	void subb_word_rdoff16_rs(u8 rd, u16 offset16, u8 rs);
	void cmp_word_rdoff16_rs(u8 rd, u16 offset16, u8 rs);
	void and_word_rdoff16_rs(u8 rd, u16 offset16, u8 rs);
	void or_word_rdoff16_rs(u8 rd, u16 offset16, u8 rs);
	void xor_word_rdoff16_rs(u8 rd, u16 offset16, u8 rs);
	void mov_word_rdoff16_rs(u8 rd, u16 offset16, u8 rs);

	void add_byte_rdoff16_rs(u8 rd, u16 offset16, u8 rs);
	void addc_byte_rdoff16_rs(u8 rd, u16 offset16, u8 rs);
	void sub_byte_rdoff16_rs(u8 rd, u16 offset16, u8 rs);
	void subb_byte_rdoff16_rs(u8 rd, u16 offset16, u8 rs);
	void cmp_byte_rdoff16_rs(u8 rd, u16 offset16, u8 rs);
	void and_byte_rdoff16_rs(u8 rd, u16 offset16, u8 rs);
	void or_byte_rdoff16_rs(u8 rd, u16 offset16, u8 rs);
	void xor_byte_rdoff16_rs(u8 rd, u16 offset16, u8 rs);
	void mov_byte_rdoff16_rs(u8 rd, u16 offset16, u8 rs);

	void add_word_rd_direct(u8 rd, u16 direct);
	void addc_word_rd_direct(u8 rd, u16 direct);
	void sub_word_rd_direct(u8 rd, u16 direct);
	void subb_word_rd_direct(u8 rd, u16 direct);
	void cmp_word_rd_direct(u8 rd, u16 direct);
	void and_word_rd_direct(u8 rd, u16 direct);
	void or_word_rd_direct(u8 rd, u16 direct);
	void xor_word_rd_direct(u8 rd, u16 direct);
	void mov_word_rd_direct(u8 rd, u16 direct);

	void add_byte_rd_direct(u8 rd, u16 direct);
	void addc_byte_rd_direct(u8 rd, u16 direct);
	void sub_byte_rd_direct(u8 rd, u16 direct);
	void subb_byte_rd_direct(u8 rd, u16 direct);
	void cmp_byte_rd_direct(u8 rd, u16 direct);
	void and_byte_rd_direct(u8 rd, u16 direct);
	void or_byte_rd_direct(u8 rd, u16 direct);
	void xor_byte_rd_direct(u8 rd, u16 direct);
	void mov_byte_rd_direct(u8 rd, u16 direct);

	void add_word_direct_rs(u16 direct, u8 rs);
	void addc_word_direct_rs(u16 direct, u8 rs);
	void sub_word_direct_rs(u16 direct, u8 rs);
	void subb_word_direct_rs(u16 direct, u8 rs);
	void cmp_word_direct_rs(u16 direct, u8 rs);
	void and_word_direct_rs(u16 direct, u8 rs);
	void or_word_direct_rs(u16 direct, u8 rs);
	void xor_word_direct_rs(u16 direct, u8 rs);
	void mov_word_direct_rs(u16 direct, u8 rs);

	void add_byte_direct_rs(u16 direct, u8 rs);
	void addc_byte_direct_rs(u16 direct, u8 rs);
	void sub_byte_direct_rs(u16 direct, u8 rs);
	void subb_byte_direct_rs(u16 direct, u8 rs);
	void cmp_byte_direct_rs(u16 direct, u8 rs);
	void and_byte_direct_rs(u16 direct, u8 rs);
	void or_byte_direct_rs(u16 direct, u8 rs);
	void xor_byte_direct_rs(u16 direct, u8 rs);
	void mov_byte_direct_rs(u16 direct, u8 rs);

	void movs_word_rd_data4(u8 rd, u8 data4);
	void movs_byte_rd_data4(u8 rd, u8 data4);
	void adds_word_rd_data4(u8 rd, u8 data4);
	void adds_byte_rd_data4(u8 rd, u8 data4);

	void movs_word_indrd_data4(u8 rd, u8 data4);
	void movs_byte_indrd_data4(u8 rd, u8 data4);
	void adds_word_indrd_data4(u8 rd, u8 data4);
	void adds_byte_indrd_data4(u8 rd, u8 data4);

	void movs_word_indrdinc_data4(u8 rd, u8 data4);
	void movs_byte_indrdinc_data4(u8 rd, u8 data4);
	void adds_word_indrdinc_data4(u8 rd, u8 data4);
	void adds_byte_indrdinc_data4(u8 rd, u8 data4);

	void movs_word_indrdoff8_data4(u8 rd, u8 off8, u8 data4);
	void movs_byte_indrdoff8_data4(u8 rd, u8 off8, u8 data4);
	void adds_word_indrdoff8_data4(u8 rd, u8 off8, u8 data4);
	void adds_byte_indrdoff8_data4(u8 rd, u8 off8, u8 data4);

	void movs_word_indrdoff16_data4(u8 rd, u16 off16, u8 data4);
	void movs_byte_indrdoff16_data4(u8 rd, u16 off16, u8 data4);
	void adds_word_indrdoff16_data4(u8 rd, u16 off16, u8 data4);
	void adds_byte_indrdoff16_data4(u8 rd, u16 off16, u8 data4);

	void movs_word_direct_data4(u16 direct, u8 data4);
	void movs_byte_direct_data4(u16 direct, u8 data4);
	void adds_word_direct_data4(u16 direct, u8 data4);
	void adds_byte_direct_data4(u16 direct, u8 data4);

	void call_rel16(u16 rel16);

	void bcc_rel8(u8 rel8);
	void bcs_rel8(u8 rel8);
	void bne_rel8(u8 rel8);
	void beq_rel8(u8 rel8);
	void bnv_rel8(u8 rel8);
	void bov_rel8(u8 rel8);
	void bpl_rel8(u8 rel8);
	void bmi_rel8(u8 rel8);
	void bg_rel8(u8 rel8);
	void bl_rel8(u8 rel8);
	void bge_rel8(u8 rel8);
	void blt_rel8(u8 rel8);
	void bgt_rel8(u8 rel8);
	void ble_rel8(u8 rel8);
	void br_rel8(u8 rel8);

	void asl_byte_rd_imm4(u8 rd, u8 amount);
	void asl_word_rd_imm4(u8 rd, u8 amount);
	void asl_dword_rd_imm5(u8 rd, u8 amount);

	void asr_byte_rd_imm4(u8 rd, u8 amount);
	void asr_word_rd_imm4(u8 rd, u8 amount);
	void asr_dword_rd_imm5(u8 rd, u8 amount);

	void lsr_byte_rd_imm4(u8 rd, u8 amount);
	void lsr_word_rd_imm4(u8 rd, u8 amount);
	void lsr_dword_rd_imm5(u8 rd, u8 amount);

	void asl_byte_rd_rs(u8 rd, u8 rs);
	void asl_word_rd_rs(u8 rd, u8 rs);
	void asl_dword_rd_rs(u8 rd, u8 rs);

	void asr_byte_rd_rs(u8 rd, u8 rs);
	void asr_word_rd_rs(u8 rd, u8 rs);
	void asr_dword_rd_rs(u8 rd, u8 rs);

	void lsr_byte_rd_rs(u8 rd, u8 rs);
	void lsr_word_rd_rs(u8 rd, u8 rs);
	void lsr_dword_rd_rs(u8 rd, u8 rs);

	void norm_byte_rd_rs(u8 rd, u8 rs);
	void norm_word_rd_rs(u8 rd, u8 rs);
	void norm_dword_rd_rs(u8 rd, u8 rs);

	void mulu_byte_rd_rs(u8 rd, u8 rs);
	void divu_byte_rd_rs(u8 rd, u8 rs);
	void mulu_word_rd_rs(u8 rd, u8 rs);
	void divu_word_rd_rs(u8 rd, u8 rs);
	void mul_word_rd_rs(u8 rd, u8 rs);
	void div_word_rd_rs(u8 rd, u8 rs);
	void div_word_rd_data8(u8 rd, u8 data8);
	void divu_byte_rd_data8(u8 rd, u8 data8);
	void divu_word_rd_data8(u8 rd, u8 data8);
	void mulu_byte_rd_data8(u8 rd, u8 data8);
	void mulu_word_rd_data16(u8 rd, u16 data16);
	void mul_word_rd_data16(u8 rd, u16 data16);
	void divu_dword_rd_data16(u8 rd, u16 data16);
	void div_dword_rd_data16(u8 rd, u16 data16);
	void divu_dword_rd_rs(u8 rd, u8 rs);
	void div_dword_rd_rs(u8 rd, u8 rs);

	void clr_bit(u16 bit);
	void setb_bit(u16 bit);
	void mov_c_bit(u16 bit);
	void mov_bit_c(u16 bit);
	void anl_c_bit(u16 bit);
	void anl_c_notbit(u16 bit);
	void orl_c_bit(u16 bit);
	void orl_c_notbit(u16 bit);

	void lea_word_rd_rs_off8(u8 rd, u8 rs, u8 offs8);
	void lea_word_rd_rs_off16(u8 rd, u8 rs, u16 offs16);

	void xch_word_rd_indrs(u8 rd, u8 rs);
	void xch_byte_rd_indrs(u8 rd, u8 rs);
	void xch_word_rd_rs(u8 rd, u8 rs);
	void xch_byte_rd_rs(u8 rd, u8 rs);

	void movc_word_rd_indrsinc(u8 rd, u8 rs);
	void movc_byte_rd_indrsinc(u8 rd, u8 rs);

	void djnz_word_rd_rel8(u8 rd, u8 rel8);
	void djnz_byte_rd_rel8(u8 rd, u8 rel8);

	void popu_word_direct(u16 direct);
	void popu_byte_direct(u16 direct);
	void pop_word_direct(u16 direct);
	void pop_byte_direct(u16 direct);
	void pushu_word_direct(u16 direct);
	void pushu_byte_direct(u16 direct);
	void push_word_direct(u16 direct);
	void push_byte_direct(u16 direct);

	void mov_word_indrdinc_indrsinc(u8 rd, u8 rs);
	void mov_byte_indrdinc_indrsinc(u8 rd, u8 rs);

	void da_rd(u8 rd);
	void sext_word_rd(u8 rd);
	void sext_byte_rd(u8 rd);
	void cpl_word_rd(u8 rd);
	void cpl_byte_rd(u8 rd);
	void neg_word_rd(u8 rd);
	void neg_byte_rd(u8 rd);
	void movc_a_apc();
	void movc_a_adptr();
	void mov_rd_usp(u8 rd);
	void mov_usp_rs(u8 rs);

	void jb_bit_rel8(u16 bit, u8 rel8);
	void jnb_bit_rel8(u16 bit, u8 rel8);
	void jbc_bit_rel8(u16 bit, u8 rel8);

	void mov_word_direct_direct(u16 direct1, u16 direct2);
	void mov_byte_direct_direct(u16 direct1, u16 direct2);

	void xch_word_rd_direct(u8 rd, u16 direct);
	void xch_byte_rd_direct(u8 rd, u16 direct);

	void mov_word_direct_indrs(u16 direct, u8 rs);
	void mov_byte_direct_indrs(u16 direct, u8 rs);

	void mov_word_indrd_direct(u8 rd, u16 direct);
	void mov_byte_indrd_direct(u8 rd, u16 direct);

	void movx_word_indrd_rs(u8 rd, u8 rs);
	void movx_byte_indrd_rs(u8 rd, u8 rs);

	void movx_word_rd_indrs(u8 rd, u8 rs);
	void movx_byte_rd_indrs(u8 rd, u8 rs);

	void rr_word_rd_data4(u8 rd, u8 data4);
	void rr_byte_rd_data4(u8 rd, u8 data4);

	void rrc_word_rd_data4(u8 rd, u8 data4);
	void rrc_byte_rd_data4(u8 rd, u8 data4);

	void rl_word_rd_data4(u8 rd, u8 data4);
	void rl_byte_rd_data4(u8 rd, u8 data4);

	void rlc_word_rd_data4(u8 rd, u8 data4);
	void rlc_byte_rd_data4(u8 rd, u8 data4);

	void fcall_addr24(u32 addr24);
	void call_indrs(u8 rs);
	void fjmp_addr24(u32 addr24);
	void jmp_rel16(u16 rel16);

	void djnz_word_direct_rel8(u16 direct, u8 rel8);
	void djnz_byte_direct_rel8(u16 direct, u8 rel8);

	void cjne_word_rd_direct_rel8(u8 rd, u16 direct, u8 rel8);
	void cjne_byte_rd_direct_rel8(u8 rd, u16 direct, u8 rel8);

	void cjne_indrd_data8_rel8(u8 rd, u8 data8, u8 rel8);
	void cjne_rd_data8_rel8(u8 rd, u8 data8, u8 rel8);

	void cjne_indrd_data16_rel8(u8 rd, u16 data16, u8 rel8);
	void cjne_rd_data16_rel8(u8 rd, u16 data16, u8 rel8);

	void reset();
	void trap_data4(u8 data4);
	void jmp_ind_adptr();
	void jmp_dblindrs(u8 rs);
	void jmp_indrs(u8 rs);
	void ret();
	void reti();

	void jz_rel8(u8 rel8);
	void jnz_rel8(u8 rel8);

	void push_word_rlist(u8 bitfield, int h);
	void push_byte_rlist(u8 bitfield, int h);

	void pushu_word_rlist(u8 bitfield, int h);
	void pushu_byte_rlist(u8 bitfield, int h);

	void pop_word_rlist(u8 bitfield, int h);
	void pop_byte_rlist(u8 bitfield, int h);

	void popu_word_rlist(u8 bitfield, int h);
	void popu_byte_rlist(u8 bitfield, int h);

	std::unordered_map<offs_t, const char *> m_names;

	u8 m_im;
	u8 m_rs;
	u8 m_zflag;
	u8 m_nflag;
	u8 m_vflag;
	u8 m_cflag;
	u8 m_acflag;
	u8 m_sm_flag;
	u8 m_tm_flag;
	u8 m_p_flag;
	u8 m_f0_flag;
	u8 m_f1_flag;

	uint32_t m_pc;

	bool m_usermode;
	u8 m_pagezeromode;

	u16 m_USP; // user stack pointer
	u16 m_SSP; // system stack pointer

	u8 m_WDCON;
	u8 m_SCR;
	u8 m_IEL;
	u8 m_PSWL;
	u8 m_PSWH;

	u8 m_regbank;

	u8 m_PxCFGA[4];
	u8 m_PxCFGB[4];

	// hacks for IRQ testing
	u8 m_in_interrupt;
	u8 m_irq_pending;


	// 16-bit regs R0-R3 can have 4 selectable banks, R4-R7 are global
	// for 8-bit use each register can be seen as High and Low parts
	// R4 when split is R4H and R4L, R4L acts as ACC from i8051 for compatibility, R4H acts as B
	// R6H is DPH, R6L is DPL
	u16 m_regs[(4 * 4) + 4];

	address_space *m_program;
	address_space *m_data;
	address_space *m_sfr;
	int m_icount;

	devcb_read8::array<4> m_port_in_cb;
	devcb_write8::array<4> m_port_out_cb;
};

class mx10exa_cpu_device : public xa_cpu
{
public:
	mx10exa_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

private:
	void mx10exa_internal_map(address_map &map) ATTR_COLD;
	void mx10exa_internal_data_map(address_map &map) ATTR_COLD;

};


DECLARE_DEVICE_TYPE(XA, xa_cpu)
DECLARE_DEVICE_TYPE(MX10EXA, mx10exa_cpu_device)

#endif // MAME_CPU_XA_XA_H
