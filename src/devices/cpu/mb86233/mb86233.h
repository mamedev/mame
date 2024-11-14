// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#ifndef MAME_CPU_MB86233_MB86233_H
#define MAME_CPU_MB86233_MB86233_H

#pragma once

class mb86233_device : public cpu_device
{
public:
	enum address_space_ids {
		AS_RF = 4,
		AS_EXTERNAL = 5
	};

	enum regs_ids {
		REG_A, REG_B, REG_D, REG_P,
		REG_R, REG_C0, REG_C1,
		REG_B0, REG_B1, REG_X0, REG_X1, REG_I0, REG_I1,
		REG_SFT, REG_VSM, REG_MASK, REG_M,
		REG_PCS0, REG_PCS1, REG_PCS2, REG_PCS3,
		REG_SP
	};

	enum st_flags {
		F_ZRC  = 0x00000001,
		F_ZRD  = 0x00000002,
		F_SGC  = 0x00000004,
		F_SGD  = 0x00000008,
		F_CPC  = 0x00000010,
		F_CPD  = 0x00000020,
		F_OVC  = 0x00000040,
		F_OVD  = 0x00000080,
		F_UNC  = 0x00000100,
		F_UND  = 0x00000200,
		F_DVZC = 0x00000400,
		F_DVZD = 0x00000800,
		F_CA   = 0x00001000,
		F_CPP  = 0x00002000,
		F_OVM  = 0x00004000,
		F_UNM  = 0x00008000,
		F_SIF0 = 0x00010000,
		F_SIF1 = 0x00020000,
		F_SOF0 = 0x00040000,

		F_PIF  = 0x00100000,
		F_POF  = 0x00200000,
		F_PAIF = 0x00400000,
		F_PAOF = 0x00800000,
		F_F0S  = 0x01000000,
		F_F1S  = 0x02000000,
		F_IT   = 0x04000000,
		F_ZX0  = 0x08000000,
		F_ZX1  = 0x10000000,
		F_ZX2  = 0x20000000,
		F_ZC0  = 0x40000000,
		F_ZC1  = 0x80000000
	};

	mb86233_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void stall() { m_stall = true; }

	void gpio0_w(int state);
	void gpio1_w(int state);
	void gpio2_w(int state);
	void gpio3_w(int state);

protected:
	mb86233_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual uint32_t execute_min_cycles() const noexcept override { return 1; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 1; }
	virtual void execute_run() override;

	virtual space_config_vector memory_space_config() const override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

private:
	address_space_config m_program_config;
	address_space_config m_data_config;
	address_space_config m_io_config;
	address_space_config m_rf_config;

	memory_access<16, 2, -2, ENDIANNESS_LITTLE>::cache m_cache;
	memory_access<16, 2, -2, ENDIANNESS_LITTLE>::specific m_program;
	memory_access<16, 2, -2, ENDIANNESS_LITTLE>::specific m_data;
	memory_access<16, 2, -2, ENDIANNESS_LITTLE>::specific m_io;
	memory_access< 4, 2, -2, ENDIANNESS_LITTLE>::specific m_rf;

	int m_icount;

	u32 m_st, m_a, m_b, m_d, m_p;
	u32 m_alu_stmask, m_alu_stset, m_alu_r1, m_alu_r2;
	u16 m_ppc, m_pc, m_sp, m_b0, m_b1, m_x0, m_x1, m_i0, m_i1, m_vsmr, m_pcs[4], m_mask, m_m;
	u8 m_r, m_rpc, m_c0, m_c1, m_sft, m_vsm;
	bool m_gpio0, m_gpio1, m_gpio2, m_gpio3;

	bool m_stall;

	static u32 set_exp(u32 val, u32 exp);
	static u32 set_mant(u32 val, u32 mant);
	static u32 get_exp(u32 val);
	static u32 get_mant(u32 val);

	void testdz();
	void alu_update_st();
	void alu_pre(u32 alu);
	void alu_post(u32 alu);
	u16 ea_pre_0(u32 r);
	void ea_post_0(u32 r);
	u16 ea_pre_1(u32 r);
	void ea_post_1(u32 r);
	void pcs_push();
	void pcs_pop();
	inline void stset_set_sz_int(u32 val);
	inline void stset_set_sz_fp(u32 val);

	u32 read_reg(u32 r);
	void write_reg(u32 r, u32 v);
	void write_mem_internal_1(u32 r, u32 v, bool bank);
	void write_mem_external_1(u32 r, u32 v);
	void write_mem_io_1(u32 r, u32 v);
};

class mb86234_device : public mb86233_device
{
public:
	mb86234_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


DECLARE_DEVICE_TYPE(MB86233, mb86233_device)
DECLARE_DEVICE_TYPE(MB86234, mb86234_device)

#endif // MAME_CPU_MB86233_MB86233_H
