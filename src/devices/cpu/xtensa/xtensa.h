// license:BSD-3-Clause
// copyright-holders:AJR

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
		XTENSA_LOOPBEGIN,
		XTENSA_LOOPEND,
		XTENSA_LOOPCOUNT,
		XTENSA_A0, XTENSA_A1, XTENSA_A2, XTENSA_A3,
		XTENSA_A4, XTENSA_A5, XTENSA_A6, XTENSA_A7,
		XTENSA_A8, XTENSA_A9, XTENSA_A10, XTENSA_A11,
		XTENSA_A12, XTENSA_A13, XTENSA_A14, XTENSA_A15,
	/*
		// with windowed extensions there 32 or 64 physical regs
		XTENSA_A16, XTENSA_A17, XTENSA_A18, XTENSA_A19,
		XTENSA_A20, XTENSA_A21, XTENSA_A22, XTENSA_A23,
		XTENSA_A24, XTENSA_A25, XTENSA_A26, XTENSA_A27,
		XTENSA_A28, XTENSA_A29, XTENSA_A30, XTENSA_A31,
	*/
	};

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

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

	uint32_t extreg_windowbase_r();
	void extreg_windowbase_w(u32 data);
	uint32_t extreg_windowstart_r();
	void extreg_windowstart_w(u32 data);
	uint32_t extreg_lbeg_r();
	void extreg_lbeg_w(u32 data);
	uint32_t extreg_lend_r();
	void extreg_lend_w(u32 data);
	uint32_t extreg_lcount_r();
	void extreg_lcount_w(u32 data);

	void ext_regs(address_map &map);

	void getop_and_execute();

	inline u32 get_reg(u8 reg);
	inline void set_reg(u8 reg, u32 value);
	inline u32 get_mem32(u32 addr);
	inline void set_mem32(u32 addr, u32 data);

	void handle_reserved(u32 inst);

	// internal state
	std::vector<uint32_t> m_a; // actually 32 or 64 physical registers with Windowed extension
	u32 m_pc;
	s32 m_icount;

	u32 m_extreg_windowbase;
	u32 m_extreg_windowstart;
	u32 m_extreg_sar;
	u32 m_extreg_lbeg;
	u32 m_extreg_lend;
	u32 m_extreg_lcount;

	u32 m_nextpc;

	// config
	u32 m_num_physical_regs;
};

DECLARE_DEVICE_TYPE(XTENSA, xtensa_device)

#endif // MAME_CPU_XTENSA_XTENSA_H
