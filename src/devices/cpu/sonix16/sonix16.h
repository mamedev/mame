// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_CPU_SONIX16_SONIX16_H
#define MAME_CPU_SONIX16_SONIX16_H

#pragma once


class sonix16_device : public cpu_device
{
public:
	static constexpr flags_type emulation_flags() { return flags::SAVE_UNSUPPORTED; }

	sonix16_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual uint32_t execute_min_cycles() const noexcept override { return 1; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 1; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	virtual space_config_vector memory_space_config() const override;

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

private:
	address_space_config m_program_config;
	address_space_config m_data_config;

	uint32_t m_pc;

	int m_icount;
};


DECLARE_DEVICE_TYPE(SONIX16, sonix16_device)

#endif // MAME_CPU_SONIX16_SONIX16_H
