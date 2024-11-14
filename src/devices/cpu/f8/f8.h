// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/*****************************************************************************
 *
 *   Portable Fairchild F8 emulator interface
 *
 *****************************************************************************/

#ifndef MAME_CPU_F8_F8_H
#define MAME_CPU_F8_F8_H

#pragma once


#define F8_INPUT_LINE_INT_REQ   1

class f8_cpu_device : public cpu_device
{
public:
	// construction/destruction
	f8_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// used by F3850 systems that override the normal zero reset address
	auto romc08_callback() { return m_romc08_callback.bind(); }

protected:
	enum
	{
		F8_PC0=1, F8_PC1, F8_DC0, F8_DC1, F8_W, F8_A, F8_IS,
		F8_R0, F8_R1, F8_R2, F8_R3, F8_R4, F8_R5, F8_R6, F8_R7,
		F8_R8, F8_J, F8_HU, F8_HL, F8_KU, F8_KL, F8_QU, F8_QL,
		F8_H, F8_K, F8_Q
	};

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface implementation
	virtual u32 execute_min_cycles() const noexcept override { return 4; }
	virtual u32 execute_max_cycles() const noexcept override { return 26; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface implementation
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface implementation
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface implementation
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

private:
	void regs_map(address_map &map) ATTR_COLD;

	address_space_config m_program_config;
	address_space_config m_regs_config;
	address_space_config m_io_config;

	devcb_read8 m_romc08_callback;

	u16  m_pc0;    /* program counter 0 */
	u16  m_pc1;    /* program counter 1 */
	u16  m_dc0;    /* data counter 0 */
	u16  m_dc1;    /* data counter 1 */
	u8   m_a;      /* accumulator */
	u8   m_w;      /* processor status */
	u8   m_is;     /* scratchpad pointer */
	u8   m_dbus;   /* data bus value */
	u16  m_io;     /* last I/O address */
	u16  m_irq_vector;
	memory_access<16, 0, 0, ENDIANNESS_BIG>::cache m_program;
	memory_access<6, 0, 0, ENDIANNESS_BIG>::cache m_r;
	memory_access<8, 0, 0, ENDIANNESS_BIG>::specific m_ios;
	int m_icount;
	int m_irq_request;

	u16 m_debug_pc; // only for the MAME debugger

	inline void CLR_OZCS();
	inline void SET_SZ(u8 n);
	inline u8 do_add(u8 n, u8 m, u8 c = 0);
	inline u8 do_add_decimal(u8 augend, u8 addend);

	void ROMC_00(int insttim);
	void ROMC_01();
	void ROMC_02();
	void ROMC_03(int insttim);
	void ROMC_04();
	void ROMC_05();
	void ROMC_06();
	void ROMC_07();
	void ROMC_08();
	void ROMC_09();
	void ROMC_0A();
	void ROMC_0B();
	void ROMC_0C();
	void ROMC_0D();
	void ROMC_0E();
	void ROMC_0F();
	void ROMC_10();
	void ROMC_11();
	void ROMC_12();
	void ROMC_13();
	void ROMC_14();
	void ROMC_15();
	void ROMC_16();
	void ROMC_17();
	void ROMC_18();
	void ROMC_19();
	void ROMC_1A();
	void ROMC_1B();
	void ROMC_1C(int insttim);
	void ROMC_1D();
	void ROMC_1E();
	void ROMC_1F();
	void illegal();
	void f8_lr_a_ku();
	void f8_lr_a_kl();
	void f8_lr_a_qu();
	void f8_lr_a_ql();
	void f8_lr_ku_a();
	void f8_lr_kl_a();
	void f8_lr_qu_a();
	void f8_lr_ql_a();
	void f8_lr_k_p();
	void f8_lr_p_k();
	void f8_lr_a_is();
	void f8_lr_is_a();
	void f8_pk();
	void f8_lr_p0_q();
	void f8_lr_q_dc();
	void f8_lr_dc_q();
	void f8_lr_dc_h();
	void f8_lr_h_dc();
	void f8_sr_1();
	void f8_sl_1();
	void f8_sr_4();
	void f8_sl_4();
	void f8_lm();
	void f8_st();
	void f8_com();
	void f8_lnk();
	void f8_di();
	void f8_ei();
	void f8_pop();
	void f8_lr_w_j();
	void f8_lr_j_w();
	void f8_inc();
	void f8_li();
	void f8_ni();
	void f8_oi();
	void f8_xi();
	void f8_ai();
	void f8_ci();
	void f8_in();
	void f8_out();
	void f8_pi();
	void f8_jmp();
	void f8_dci();
	void f8_nop();
	void f8_xdc();
	void f8_ds_r(int r);
	void f8_ds_isar();
	void f8_ds_isar_i();
	void f8_ds_isar_d();
	void f8_lr_a_r(int r);
	void f8_lr_a_isar();
	void f8_lr_a_isar_i();
	void f8_lr_a_isar_d();
	void f8_lr_r_a(int r);
	void f8_lr_isar_a();
	void f8_lr_isar_i_a();
	void f8_lr_isar_d_a();
	void f8_lisu(int e);
	void f8_lisl(int e);
	void f8_lis(int i);
	void f8_bt(int e);
	void f8_am();
	void f8_amd();
	void f8_nm();
	void f8_om();
	void f8_xm();
	void f8_cm();
	void f8_adc();
	void f8_br7();
	void f8_bf(int t);
	void f8_ins_0(int n);
	void f8_ins_1(int n);
	void f8_outs_0(int n);
	void f8_outs_1(int n);
	void f8_as(int r);
	void f8_as_isar();
	void f8_as_isar_i();
	void f8_as_isar_d();
	void f8_asd(int r);
	void f8_asd_isar();
	void f8_asd_isar_i();
	void f8_asd_isar_d();
	void f8_xs(int r);
	void f8_xs_isar();
	void f8_xs_isar_i();
	void f8_xs_isar_d();
	void f8_ns(int r);
	void f8_ns_isar();
	void f8_ns_isar_i();
	void f8_ns_isar_d();
};


DECLARE_DEVICE_TYPE(F8, f8_cpu_device)

#endif // MAME_CPU_F8_F8_H
