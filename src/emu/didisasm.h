// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    didisasm.h

    Device disassembly interfaces.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __DIDISASM_H__
#define __DIDISASM_H__


//**************************************************************************
//  CONSTANTS
//**************************************************************************

// Disassembler constants
const UINT32 DASMFLAG_SUPPORTED     = 0x80000000;   // are disassembly flags supported?
const UINT32 DASMFLAG_STEP_OUT      = 0x40000000;   // this instruction should be the end of a step out sequence
const UINT32 DASMFLAG_STEP_OVER     = 0x20000000;   // this instruction should be stepped over by setting a breakpoint afterwards
const UINT32 DASMFLAG_OVERINSTMASK  = 0x18000000;   // number of extra instructions to skip when stepping over
const UINT32 DASMFLAG_OVERINSTSHIFT = 27;           // bits to shift after masking to get the value
const UINT32 DASMFLAG_LENGTHMASK    = 0x0000ffff;   // the low 16-bits contain the actual length



//**************************************************************************
//  MACROS
//**************************************************************************

#define DASMFLAG_STEP_OVER_EXTRA(x)         ((x) << DASMFLAG_OVERINSTSHIFT)



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_DEVICE_DISASSEMBLE_OVERRIDE(_class, _func) \
	device_disasm_interface::static_set_dasm_override(*device, dasm_override_delegate(&_class::_func, #_class "::" #_func, nullptr, (_class *)nullptr));



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

typedef device_delegate<offs_t (device_t &device, char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, int options)> dasm_override_delegate;

// ======================> device_disasm_interface

// class representing interface-specific live disasm
class device_disasm_interface : public device_interface
{
public:
	// construction/destruction
	device_disasm_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_disasm_interface();

	// configuration access
	UINT32 min_opcode_bytes() const { return disasm_min_opcode_bytes(); }
	UINT32 max_opcode_bytes() const { return disasm_max_opcode_bytes(); }

	// static inline configuration helpers
	static void static_set_dasm_override(device_t &device, dasm_override_delegate dasm_override);

	// interface for disassembly
	offs_t disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options = 0);

protected:
	// required operation overrides
	virtual UINT32 disasm_min_opcode_bytes() const = 0;
	virtual UINT32 disasm_max_opcode_bytes() const = 0;
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) = 0;

	// interface-level overrides
	virtual void interface_pre_start() override;

private:
	dasm_override_delegate  m_dasm_override;            // provided override function
};

// iterator
typedef device_interface_iterator<device_disasm_interface> disasm_interface_iterator;


#endif  /* __DIDISASM_H__ */
