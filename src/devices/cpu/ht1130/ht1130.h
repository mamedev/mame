// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_CPU_HT1130_HT1130_H
#define MAME_CPU_HT1130_HT1130_H

#pragma once


class ht1130_device : public cpu_device
{
public:
	ht1130_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	enum {
		HT1130_PC,
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

	memory_access<8, 0, 0, ENDIANNESS_LITTLE>::cache m_cache;
	memory_access<8, 0, 0, ENDIANNESS_LITTLE>::specific m_space;

	void internal_map(address_map &map);

	inline u8 fetch();
	void do_op();

	u32 m_pc;
	s32 m_icount;
};

DECLARE_DEVICE_TYPE(HT1130, ht1130_device)

#endif // MAME_CPU_HT1130_HT1130_H
