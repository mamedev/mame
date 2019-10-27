// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    didisasm.h

    Device disassembly interfaces.

***************************************************************************/
#ifndef MAME_EMU_DIDISASM_H
#define MAME_EMU_DIDISASM_H

#pragma once

#include "disasmintf.h"

#include <memory>
#include <utility>


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

typedef device_delegate<offs_t (std::ostream &stream, offs_t pc, const util::disasm_interface::data_buffer &opcodes, const util::disasm_interface::data_buffer &params)> dasm_override_delegate;

// ======================> device_disasm_interface

// class representing interface-specific live disasm
class device_disasm_interface : public device_interface
{
public:
	// construction/destruction
	device_disasm_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_disasm_interface() = default;

	// Override
	template <typename... T> void set_dasm_override(T &&... args) { m_dasm_override.set(std::forward<T>(args)...); }

	// disassembler request
	util::disasm_interface &get_disassembler();

protected:
	// disassembler creation
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() = 0;

	// delegate resolving
	virtual void interface_pre_start() override;

private:
	std::unique_ptr<util::disasm_interface> m_disasm;
	dasm_override_delegate m_dasm_override;
	bool m_started;
};

// iterator
typedef device_interface_iterator<device_disasm_interface> disasm_interface_iterator;

#endif // MAME_EMU_DIDISASM_H
