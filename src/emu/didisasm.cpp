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

offs_t device_disasm_interface::disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	offs_t result = 0;

	// check for disassembler override
	if (!m_dasm_override.isnull())
		result = m_dasm_override(device(), buffer, pc, oprom, opram, options);
	if (result == 0)
		result = disasm_disassemble(buffer, pc, oprom, opram, options);

	return result;
}
