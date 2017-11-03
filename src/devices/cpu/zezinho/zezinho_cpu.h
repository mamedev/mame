// license:GPL-2.0+
// copyright-holders:Felipe Sanches
#ifndef MAME_DEVICES_CPU_ZEZINHO_CPU_H
#define MAME_DEVICES_CPU_ZEZINHO_CPU_H

#pragma once

/* register IDs */
enum
{
	ZEZINHO_CI=1, ZEZINHO_ACC
};


class zezinho_cpu_device : public cpu_device {
public:
	// construction/destruction
	zezinho_cpu_device(const machine_config &mconfig, const char *_tag, device_t *_owner, uint32_t _clock);

protected:

	virtual void execute_run() override;
	virtual offs_t disasm_disassemble(std::ostream &stream, offs_t pc, const uint8_t *oprom, const uint8_t *opram, uint32_t options) override;

	address_space_config m_data_config;
	address_space_config m_program_config;

	offs_t m_addr;
	unsigned char m_opcode;

	/* processor registers */
	unsigned int m_acc; /* accumulator (10 bits) */
	unsigned int m_pc;   /* program counter (12 bits) */

	/* processor state flip-flops */
	bool m_run; /* processor is running */

	int m_icount;

	address_space *m_program;
	address_space *m_data;

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const override { return 1; }
	virtual uint32_t execute_max_cycles() const override { return 2; }

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_disasm_interface overrides
	virtual uint32_t disasm_min_opcode_bytes() const override { return 2; }
	virtual uint32_t disasm_max_opcode_bytes() const override { return 2; }

private:
	void execute_instruction();
};

DECLARE_DEVICE_TYPE(ZEZINHO2_CPU, zezinho_cpu_device)

#endif // MAME_DEVICES_CPU_ZEZINHO_CPU_H
