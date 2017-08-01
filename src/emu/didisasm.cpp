// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    didisasm.c

    Device disasm interfaces.

***************************************************************************/

#include "emu.h"


//**************************************************************************
//  DEVICE DISASM INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_disasm_interface - constructor
//-------------------------------------------------

device_disasm_interface::device_disasm_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "disasm")
{
}


//-------------------------------------------------
//  ~device_disasm_interface - destructor
//-------------------------------------------------

device_disasm_interface::~device_disasm_interface()
{
}


//-------------------------------------------------
//  interface_pre_start - work to be done prior to
//  actually starting a device
//-------------------------------------------------

void device_disasm_interface::interface_pre_start()
{
	// bind delegate
	m_dasm_override.bind_relative_to(*device().owner());
}


//-------------------------------------------------
//  static_set_dasm_override - configuration
//  helper to override disassemble function
//-------------------------------------------------

void device_disasm_interface::static_set_dasm_override(device_t &device, dasm_override_delegate dasm_override)
{
	device_disasm_interface *dasm;
	if (!device.interface(dasm))
		throw emu_fatalerror("MCFG_DEVICE_DISASSEMBLE_OVERRIDE called on device '%s' with no disasm interface", device.tag());
	dasm->m_dasm_override = dasm_override;
}


//-------------------------------------------------
//  disassemble - interface for disassembly
//-------------------------------------------------

offs_t device_disasm_interface::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params, u32 options)
{
	offs_t result = 0;

	// check for disassembler override
	if (!m_dasm_override.isnull())
		result = m_dasm_override(device(), stream, pc, opcodes, params, options);
	if (result == 0)
		result = disassemble(stream, pc, opcodes, params, options);

	// make sure we get good results
	assert((result & DASMFLAG_LENGTHMASK) != 0);

	return result;
}

u32 device_disasm_interface::disasm_interface_flags() const
{
	return 0;
}

u32 device_disasm_interface::disasm_page_address_bits() const
{
	throw emu_fatalerror("%s: unimplemented disasm_page_address_bits called", device().tag());
}

u32 device_disasm_interface::disasm_page2_address_bits() const
{
	throw emu_fatalerror("%s: unimplemented disasm_page2_address_bits called", device().tag());
}

offs_t device_disasm_interface::disasm_pc_linear_to_real(offs_t pc) const
{
	throw emu_fatalerror("%s: unimplemented disasm_pc_linear_to_real called", device().tag());
}

offs_t device_disasm_interface::disasm_pc_real_to_linear(offs_t pc) const
{
	throw emu_fatalerror("%s: unimplemented disasm_pc_real_to_linear called", device().tag());
}

u8 device_disasm_interface::disasm_decrypt8(u8 value, offs_t pc, bool opcode) const
{
	throw emu_fatalerror("%s: unimplemented disasm_decrypt8 called", device().tag());
}

u16 device_disasm_interface::disasm_decrypt16(u16 value, offs_t pc, bool opcode) const
{
	throw emu_fatalerror("%s: unimplemented disasm_decrypt16 called", device().tag());
}

u32 device_disasm_interface::disasm_decrypt32(u32 value, offs_t pc, bool opcode) const
{
	throw emu_fatalerror("%s: unimplemented disasm_decrypt32 called", device().tag());
}

u64 device_disasm_interface::disasm_decrypt64(u64 value, offs_t pc, bool opcode) const
{
	throw emu_fatalerror("%s: unimplemented disasm_decrypt64 called", device().tag());
}

