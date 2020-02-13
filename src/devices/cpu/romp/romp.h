// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_CPU_ROMP_ROMP_H
#define MAME_CPU_ROMP_ROMP_H

#pragma once

class romp_device : public cpu_device
{
public:
	romp_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	enum registers : unsigned
	{
		ROMP_SCR = 0,
		ROMP_GPR = 16,
	};

	enum scr : unsigned
	{
		COS =  6, // counter source
		COU =  7, // counter
		TS  =  8, // timer status
		MQ  = 10, // multiplier quotient
		MCS = 11, // machine check status
		PCS = 11, // program check status
		IRB = 12, // interrupt request buffer
		IAR = 13, // instruction address register
		ICS = 14, // interrupt control status
		CS  = 15, // condition status
	};

	enum cs : unsigned
	{
		TB = 0, // test bit
		OV = 1, // overflow
				// reserved
		C0 = 3, // carry zero
		GT = 4, // greater than
		EQ = 5, // equal
		LT = 6, // less than
		PZ = 7, // permanent zero
	};

	// device_t overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual u32 execute_min_cycles() const noexcept override { return 1; }
	virtual u32 execute_max_cycles() const noexcept override { return 40; }
	virtual u32 execute_input_lines() const noexcept override { return 6; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;
	virtual bool memory_translate(int spacenum, int intention, offs_t &address) override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

private:
	// insruction decode helpers
	u32 r3_0(unsigned reg) const { return reg ? m_gpr[reg] : 0; }
	s32 ji(u16 op) const { return s32(s8(op)) << 1; }
	u32 ba(u16 hi, u16 lo) const { return ((u32(hi) << 16) | lo) & 0x00ff'fffeU; }
	s32 bi(u16 hi, u16 lo) const { return s32((u32(hi) << 16 | lo) << 12) >> 11; }

	// address spaces
	address_space_config const m_mem_config;
	address_space_config const m_io_config;

	// mame state
	int m_icount;

	// core registers
	u32 m_scr[16];
	u32 m_gpr[16];

	// internal state
	enum branch_state : unsigned
	{
		NONE      = 0,
		SUBJECT   = 1, // branch subject instruction active
		BRANCH    = 2, // branch instruction active
	}
	m_branch_state;
	u32 m_branch_target;
};

DECLARE_DEVICE_TYPE(ROMP, romp_device)

#endif // MAME_CPU_ROMP_ROMP_H
