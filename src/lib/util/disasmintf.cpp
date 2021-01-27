#include "disasmintf.h"

util::disasm_interface::u32 util::disasm_interface::interface_flags() const
{
	return 0;
}

util::disasm_interface::u32 util::disasm_interface::page_address_bits() const
{
	throw ("unimplemented page_address_bits called");
}

util::disasm_interface::u32 util::disasm_interface::page2_address_bits() const
{
	throw ("unimplemented page2_address_bits called");
}

util::disasm_interface::offs_t util::disasm_interface::pc_linear_to_real(offs_t pc) const
{
	throw ("unimplemented pc_linear_to_real called");
}

util::disasm_interface::offs_t util::disasm_interface::pc_real_to_linear(offs_t pc) const
{
	throw ("unimplemented pc_real_to_linear called");
}

util::disasm_interface::u8 util::disasm_interface::decrypt8(u8 value, offs_t pc, bool opcode) const
{
	throw ("unimplemented decrypt8 called");
}

util::disasm_interface::u16 util::disasm_interface::decrypt16(u16 value, offs_t pc, bool opcode) const
{
	throw ("unimplemented decrypt16 called");
}

util::disasm_interface::u32 util::disasm_interface::decrypt32(u32 value, offs_t pc, bool opcode) const
{
	throw ("unimplemented decrypt32 called");
}

util::disasm_interface::u64 util::disasm_interface::decrypt64(u64 value, offs_t pc, bool opcode) const
{
	throw ("unimplemented decrypt64 called");
}

