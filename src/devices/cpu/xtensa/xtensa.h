// license:BSD-3-Clause
// copyright-holders:AJR, David Haywood

#ifndef MAME_CPU_XTENSA_XTENSA_H
#define MAME_CPU_XTENSA_XTENSA_H

#pragma once


class xtensa_device : public cpu_device
{
public:
	xtensa_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	enum {
		AS_EXTREGS = AS_OPCODES + 1,
	};

	enum {
		XTENSA_PC,
		XTENSA_WINDOW,
		XTENSA_INTENABLE,
		XTENSA_LOOPBEGIN,
		XTENSA_LOOPEND,
		XTENSA_LOOPCOUNT,
		XTENSA_A0, XTENSA_A1, XTENSA_A2, XTENSA_A3,
		XTENSA_A4, XTENSA_A5, XTENSA_A6, XTENSA_A7,
		XTENSA_A8, XTENSA_A9, XTENSA_A10, XTENSA_A11,
		XTENSA_A12, XTENSA_A13, XTENSA_A14, XTENSA_A15,
	};

	void set_irq_vector(int bit, u32 vector)
	{
		m_irq_vectors[bit] = vector;
	}

	void set_startupvector(u32 vector)
	{
		m_startupvector = vector;
	}

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

private:
	// address space
	address_space_config m_space_config;
	address_space_config m_extregs_config;

	memory_access<32, 2, 0, ENDIANNESS_LITTLE>::cache m_cache;
	memory_access<32, 2, 0, ENDIANNESS_LITTLE>::specific m_space;

	u32 extreg_windowbase_r();
	void extreg_windowbase_w(u32 data);
	u32 extreg_windowstart_r();
	void extreg_windowstart_w(u32 data);
	u32 extreg_lbeg_r();
	void extreg_lbeg_w(u32 data);
	u32 extreg_lend_r();
	void extreg_lend_w(u32 data);
	u32 extreg_lcount_r();
	void extreg_lcount_w(u32 data);
	u32 extreg_ps_r();
	void extreg_ps_w(u32 data);
	u32 extreg_cacheattr_r();
	void extreg_cacheattr_w(u32 data);
	u32 extreg_epc1_r();
	void extreg_epc1_w(u32 data);
	u32 extreg_epc2_r();
	void extreg_epc2_w(u32 data);
	u32 extreg_epc3_r();
	void extreg_epc3_w(u32 data);
	u32 extreg_epc4_r();
	void extreg_epc4_w(u32 data);
	u32 extreg_epc5_r();
	void extreg_epc5_w(u32 data);
	u32 extreg_eps2_r();
	void extreg_eps2_w(u32 data);
	u32 extreg_eps3_r();
	void extreg_eps3_w(u32 data);
	u32 extreg_eps4_r();
	void extreg_eps4_w(u32 data);
	u32 extreg_eps5_r();
	void extreg_eps5_w(u32 data);
	u32 extreg_excsave1_r();
	void extreg_excsave1_w(u32 data);
	u32 extreg_excsave2_r();
	void extreg_excsave2_w(u32 data);
	u32 extreg_excsave3_r();
	void extreg_excsave3_w(u32 data);
	u32 extreg_excsave4_r();
	void extreg_excsave4_w(u32 data);
	u32 extreg_excsave5_r();
	void extreg_excsave5_w(u32 data);
	u32 extreg_ibreaka0_r();
	void extreg_ibreaka0_w(u32 data);
	u32 extreg_dbreaka0_r();
	void extreg_dbreaka0_w(u32 data);
	u32 extreg_dbreakc0_r();
	void extreg_dbreakc0_w(u32 data);
	u32 extreg_icountlevel_r();
	void extreg_icountlevel_w(u32 data);
	u32 extreg_ccompare0_r();
	void extreg_ccompare0_w(u32 data);
	u32 extreg_intenable_r();
	void extreg_intenable_w(u32 data);
	u32 extreg_intclr_r();
	void extreg_intclr_w(u32 data);
	u32 extreg_intset_r();
	void extreg_intset_w(u32 data);
	u32 extreg_ccount_r();
	void extreg_ccount_w(u32 data);
	u32 extreg_exccause_r();
	void extreg_exccause_w(u32 data);
	u32 extreg_sar_r();
	void extreg_sar_w(u32 data);

	void set_irqpri(u8 val);
	u8 get_irqpri();
	void clear_irqpri(u8 val);


	void set_callinc(u8 val);
	u8 get_callinc();

	void ext_regs(address_map &map) ATTR_COLD;

	bool handle_bz(u32 inst);
	void handle_retw();
	void check_interrupts();
	void getop_and_execute();

	inline u32 get_reg(u8 reg);
	inline void set_reg(u8 reg, u32 value);

	inline u32 get_mem32(u32 addr);
	inline void set_mem32(u32 addr, u32 data);

	inline u8 get_mem8(u32 addr);
	inline void set_mem8(u32 addr, u8 data);

	inline u16 get_mem16(u32 addr);
	inline void set_mem16(u32 addr, u16 data);

	void handle_reserved(u32 inst);

	void switch_windowbase(s32 change);

	// internal state
	std::vector<u32> m_a_real; // actually 32 or 64 physical registers with Windowed extension
	u32 m_a[16];
	u32 m_pc;
	s32 m_icount;

	u32 m_extreg_windowbase;
	u32 m_extreg_windowstart;
	u32 m_extreg_sar;
	u32 m_extreg_lbeg;
	u32 m_extreg_lend;
	u32 m_extreg_lcount;
	u32 m_extreg_ps;
	u32 m_extreg_cacheattr;
	u32 m_extreg_epc1;
	u32 m_extreg_epc2;
	u32 m_extreg_epc3;
	u32 m_extreg_epc4;
	u32 m_extreg_epc5;
	u32 m_extreg_eps2;
	u32 m_extreg_eps3;
	u32 m_extreg_eps4;
	u32 m_extreg_eps5;
	u32 m_extreg_excsave1;
	u32 m_extreg_excsave2;
	u32 m_extreg_excsave3;
	u32 m_extreg_excsave4;
	u32 m_extreg_excsave5;
	u32 m_extreg_ibreaka0;
	u32 m_extreg_dbreaka0;
	u32 m_extreg_dbreakc0;
	u32 m_extreg_icountlevel;
	u32 m_extreg_ccompare0;
	u32 m_extreg_intenable;
	u32 m_extreg_intclr;
	u32 m_extreg_intset;
	u32 m_extreg_ccount;
	u32 m_extreg_exccause;
	u32 m_nextpc;

	// config
	u32 m_num_physical_regs;

	u32 m_irq_vectors[32];
	u32 m_startupvector;
};

DECLARE_DEVICE_TYPE(XTENSA, xtensa_device)

#endif // MAME_CPU_XTENSA_XTENSA_H
