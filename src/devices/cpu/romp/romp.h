// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_CPU_ROMP_ROMP_H
#define MAME_CPU_ROMP_ROMP_H

#pragma once

#include "rsc.h"

class romp_device : public cpu_device
{
public:
	romp_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	template <typename T> void set_mmu(T &&tag) { m_mmu.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_iou(T &&tag) { m_iou.set_tag(std::forward<T>(tag)); }

	void clk_w(int state);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface implementation
	virtual u32 execute_min_cycles() const noexcept override { return 1; }
	virtual u32 execute_max_cycles() const noexcept override { return 40; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface implementation
	virtual space_config_vector memory_space_config() const override;
	virtual bool memory_translate(int spacenum, int intention, offs_t &address, address_space *&target_space) override;

	// device_state_interface implementation
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface implementation
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

private:
	void flags_log(u32 const op1);
	void flags_add(u32 const op1, u32 const op2);
	void flags_sub(u32 const op1, u32 const op2);

	void set_scr(unsigned scr, u32 data);

	void interrupt_check();
	void machine_check(u32 mcs);
	void program_check(u32 pcs);
	void interrupt_enter(unsigned vector, u32 iar, u16 svc = 0);

	// memory accessors
	void fetch(offs_t address, std::function<void(u16)> f);
	template <typename T> void load(offs_t address, std::function<void(T)> f, bool mask = true);
	template <typename T> void store(offs_t address, T data, bool mask = true);
	template <typename T> void modify(offs_t address, std::function<T(T)> f, bool mask = true);

	// address spaces
	address_space_config const m_mem_config;

	required_device<rsc_cpu_interface> m_mmu;
	required_device<rsc_bus_interface> m_iou;

	// mame state
	int m_icount;

	// core registers
	u32 m_scr[16];
	u32 m_gpr[16];

	// input line state
	u8 m_reqi;
	bool m_trap;

	// internal state
	enum branch_state : unsigned
	{
		DEFAULT   = 0,
		BRANCH    = 1, // branch subject instruction active
		DELAY     = 2, // delayed branch instruction active
		EXCEPTION = 3,
		WAIT      = 4,
	}
	m_branch_state;
	u32 m_branch_source;
	u32 m_branch_target;
	bool m_defer_int;
};

DECLARE_DEVICE_TYPE(ROMP, romp_device)

#endif // MAME_CPU_ROMP_ROMP_H
