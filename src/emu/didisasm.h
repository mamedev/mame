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

#ifndef MAME_EMU_DIDISASM_H
#define MAME_EMU_DIDISASM_H


//**************************************************************************
//  CONSTANTS
//**************************************************************************

// Disassembler constants
constexpr u32 DASMFLAG_SUPPORTED       = 0x80000000;   // are disassembly flags supported?
constexpr u32 DASMFLAG_STEP_OUT        = 0x40000000;   // this instruction should be the end of a step out sequence
constexpr u32 DASMFLAG_STEP_OVER       = 0x20000000;   // this instruction should be stepped over by setting a breakpoint afterwards
constexpr u32 DASMFLAG_OVERINSTMASK    = 0x18000000;   // number of extra instructions to skip when stepping over
constexpr u32 DASMFLAG_OVERINSTSHIFT   = 27;           // bits to shift after masking to get the value
constexpr u32 DASMFLAG_LENGTHMASK      = 0x0000ffff;   // the low 16-bits contain the actual length

//**************************************************************************
//  MACROS
//**************************************************************************

#define DASMFLAG_STEP_OVER_EXTRA(x)         ((x) << DASMFLAG_OVERINSTSHIFT)



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_DEVICE_DISASSEMBLE_OVERRIDE(_class, _func) \
	device_disasm_interface::static_set_dasm_override(*device, device_disasm_interface::dasm_override_delegate(&_class::_func, #_class "::" #_func, nullptr, (_class *)nullptr));



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> device_disasm_interface

// class representing interface-specific live disasm
class device_disasm_interface : public device_interface
{
public:
	class data_buffer {
	public:
		virtual ~data_buffer() = default;
		virtual u8  r8 (offs_t pc) const = 0;
		virtual u16 r16(offs_t pc) const = 0;
		virtual u32 r32(offs_t pc) const = 0;
		virtual u64 r64(offs_t pc) const = 0;
	};

	enum {
		DASMINTF_NONLINEAR_PC        = 0x00000001,
		DASMINTF_PAGED               = 0x00000002,
		DASMINTF_PAGED2LEVEL         = 0x00000006,
		DASMINTF_INTERNAL_DECRYPTION = 0x00000008,
		DASMINTF_SPLIT_DECRYPTION    = 0x00000018
	};

	typedef device_delegate<offs_t (device_t &device, std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params, int options)> dasm_override_delegate;

	// construction/destruction
	device_disasm_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_disasm_interface();

	// configuration access
	virtual u32 opcode_alignment() const = 0;

	// static inline configuration helpers
	static void static_set_dasm_override(device_t &device, dasm_override_delegate dasm_override);

	// interface for disassembly
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params, u32 options = 0) = 0;

	// optional operation overrides
	virtual u32 disasm_interface_flags() const;
	virtual u32 disasm_page_address_bits() const;
	virtual u32 disasm_page2_address_bits() const;
	virtual offs_t disasm_pc_linear_to_real(offs_t pc) const;
	virtual offs_t disasm_pc_real_to_linear(offs_t pc) const;
	virtual u8  disasm_decrypt8 (u8  value, offs_t pc, bool opcode) const;
	virtual u16 disasm_decrypt16(u16 value, offs_t pc, bool opcode) const;
	virtual u32 disasm_decrypt32(u32 value, offs_t pc, bool opcode) const;
	virtual u64 disasm_decrypt64(u64 value, offs_t pc, bool opcode) const;

protected:
	// interface-level overrides
	virtual void interface_pre_start() override;

private:
	dasm_override_delegate  m_dasm_override;            // provided override function
};

// iterator
typedef device_interface_iterator<device_disasm_interface> disasm_interface_iterator;


#endif  /* MAME_EMU_DIDISASM_H */
