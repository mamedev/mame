// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/*
    Epson C33 STD Core (S1C33000)
*/

#ifndef MAME_CPU_C33_C33STD_H
#define MAME_CPU_C33_C33STD_H

#pragma once

#include "c33common.h"

#include <memory>
#include <string>


class c33std_cpu_device_base : public cpu_device, public c33_cpu_common
{
protected:
	c33std_cpu_device_base(
			machine_config const &mconfig,
			device_type type,
			char const *tag,
			device_t *owner,
			u32 clock,
			address_map_constructor internal_map);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface implementation
	virtual void execute_run() override;

	// device_memory_interface implementation
	virtual space_config_vector memory_space_config() const override ATTR_COLD;

	// device_disassembler_interface implementation
	std::unique_ptr<util::disasm_interface> create_disassembler() override ATTR_COLD;

	// device_state_interface implementation
	void state_string_export(device_state_entry const &entry, std::string &str) const override;

	address_space_config const m_memory_config;
	memory_access<28, 1, 0, ENDIANNESS_LITTLE>::cache m_opcodes;
	memory_access<28, 1, 0, ENDIANNESS_LITTLE>::specific m_data;

	int m_icount;
	u32 m_gprs[16];
	u32 m_pc;
	u32 m_psr;
	u32 m_sp;
	u32 m_alr;
	u32 m_ahr;
};

#endif // MAME_CPU_C33_C33STD_H
