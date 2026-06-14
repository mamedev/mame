// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_MYLIFE_MYLIFE_H
#define MAME_CPU_MYLIFE_MYLIFE_H

#pragma once


class mylife_cpu_device : public cpu_device
{
public:
	mylife_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual space_config_vector memory_space_config() const override;

	virtual void execute_run() override;

private:
	const address_space_config m_program_config;
	const address_space_config m_data_config;

	s32 m_icount;

	u16 m_pc;
};


DECLARE_DEVICE_TYPE(MYLIFE_CPU, mylife_cpu_device)

#endif // MAME_CPU_MYLIFE_MYLIFE_H
