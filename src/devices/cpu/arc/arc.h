// license:BSD-3-Clause
// copyright-holders:David Haywood
/*********************************\

 ARCtangent (A4) core
 ARC == Argonaut RISC Core

\*********************************/

#ifndef MAME_CPU_ARC_ARC_H
#define MAME_CPU_ARC_ARC_H

#pragma once

enum
{
	ARC_PC = STATE_GENPC
};

class arc_cpu_device : public cpu_device
{
public:
	// construction/destruction
	arc_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override { return 5; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 5; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

private:
	address_space_config m_program_config;

	// 0 - 28 = r00 - r28 (General Purpose Registers)
	//     29 = r29 (ILINK1)
	//     30 = r30 (ILINE2)
	//     31 = r31 (BLINK)
	// 32- 59 = r32 - r59 (Reserved Registers)
	//     60 = LPCOUNT
	//     61 = Short Immediate Data Indicator Settings Flag
	//     62 = Long Immediate Data Indicator
	//     63 = Short Immediate Data Indicator NOT Settings Flag
	uint32_t m_pc;
	//uint32_t m_r[64];


	address_space *m_program;
	int m_icount;

	uint32_t m_debugger_temp;

	void unimplemented_opcode(uint16_t op);
	inline uint32_t READ32(uint32_t address);
	inline void WRITE32(uint32_t address, uint32_t data);
};


DECLARE_DEVICE_TYPE(ARC, arc_cpu_device)

#endif // MAME_CPU_ARC_ARC_H
