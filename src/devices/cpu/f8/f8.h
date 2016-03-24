// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/*****************************************************************************
 *
 *   f8.h
 *   Portable Fairchild F8 emulator interface
 *
 *****************************************************************************/

#pragma once

#ifndef __F8_H__
#define _F8_H


enum
{
	F8_PC0=1, F8_PC1, F8_DC0, F8_DC1, F8_W, F8_A, F8_IS,
		F8_J, F8_HU, F8_HL, F8_KU, F8_KL, F8_QU, F8_QL,

		F8_R0, F8_R1, F8_R2, F8_R3, F8_R4, F8_R5, F8_R6, F8_R7, F8_R8,
		F8_R16, F8_R17, F8_R18, F8_R19, F8_R20, F8_R21, F8_R22, F8_R23,
		F8_R24, F8_R25, F8_R26, F8_R27, F8_R28, F8_R29, F8_R30, F8_R31,
		F8_R32, F8_R33, F8_R34, F8_R35, F8_R36, F8_R37, F8_R38, F8_R39,
		F8_R40, F8_R41, F8_R42, F8_R43, F8_R44, F8_R45, F8_R46, F8_R47,
		F8_R48, F8_R49, F8_R50, F8_R51, F8_R52, F8_R53, F8_R54, F8_R55,
		F8_R56, F8_R57, F8_R58, F8_R59, F8_R60, F8_R61, F8_R62, F8_R63
};

#define F8_INPUT_LINE_INT_REQ   1

class f8_cpu_device :  public cpu_device
{
public:
	// construction/destruction
	f8_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const override { return 1; }
	virtual UINT32 execute_max_cycles() const override { return 7; }
	virtual UINT32 execute_input_lines() const override { return 1; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override { return (spacenum == AS_PROGRAM) ? &m_program_config : ( (spacenum == AS_IO) ? &m_io_config : nullptr ); }

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const override { return 1; }
	virtual UINT32 disasm_max_opcode_bytes() const override { return 3; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

private:
	address_space_config m_program_config;
	address_space_config m_io_config;

	UINT16  m_pc0;    /* program counter 0 */
	UINT16  m_pc1;    /* program counter 1 */
	UINT16  m_dc0;    /* data counter 0 */
	UINT16  m_dc1;    /* data counter 1 */
	UINT8   m_a;      /* accumulator */
	UINT8   m_w;      /* processor status */
	UINT8   m_is;     /* scratchpad pointer */
	UINT8   m_dbus;   /* data bus value */
	UINT16  m_io;     /* last I/O address */
	UINT16  m_irq_vector;
	address_space *m_program;
	direct_read_data *m_direct;
	address_space *m_iospace;
	int m_icount;
	UINT8   m_r[64];  /* scratchpad RAM */
	int     m_irq_request;

	/* timer shifter polynome values (will be used for timer interrupts) */
	UINT8 timer_shifter[256];

	UINT16 m_pc; // For the debugger

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
	void f8_cm();    /* SKR changed to match f8_ci(cpustate); */
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


extern const device_type F8;


#endif /* __F8_H__ */
