// license:BSD-3-Clause
// copyright-holders:Steve Ellenoff, David Haywood

#ifndef MAME_CPU_SFR_AXC51DASM_H
#define MAME_CPU_SFR_AXC51DASM_H

#pragma once

#include <unordered_map>


class axc51_disassembler : public util::disasm_interface
{
public:
	struct mem_info {
		int addr;
		const char *name;
	};

	static const mem_info default_names[];

	template<typename ...Names> axc51_disassembler(Names &&... names) : axc51_disassembler() {
		add_names(names...);
	}

	axc51_disassembler();
	virtual ~axc51_disassembler() = default;

	template<typename ...Names> void add_names(const mem_info *info, Names &&... names)
	{
		add_names(names...);
		add_names(info);
	}

	void add_names(const mem_info *info);

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

protected:
	virtual void disassemble_op_ljmp(std::ostream& stream, unsigned& PC, const data_buffer& params);
	virtual void disassemble_op_lcall(std::ostream& stream, unsigned& PC, const data_buffer& params);
	virtual offs_t disassemble_op(std::ostream &stream, unsigned PC, offs_t pc, const data_buffer &opcodes, const data_buffer &params, uint8_t op);

	std::string get_data_address( uint8_t arg ) const;
	std::string get_bit_address( uint8_t arg ) const;
private:
	std::unordered_map<offs_t, const char *> m_names;

};

class axc51core_disassembler : public axc51_disassembler
{
public:
	axc51core_disassembler();

	axc51core_disassembler(const mem_info* names) : axc51_disassembler(names) {}

	virtual ~axc51core_disassembler() = default;

	static const mem_info axc51core_names[];

protected:
	virtual offs_t disassemble_op(std::ostream &stream, unsigned PC, offs_t pc, const data_buffer &opcodes, const data_buffer &params, uint8_t op) override;

private:
	offs_t disassemble_extended_a5(std::ostream& stream, unsigned PC, offs_t pc, const data_buffer& opcodes, const data_buffer& params);
	offs_t disassemble_extended_a5_0e(std::ostream& stream, unsigned PC, offs_t pc, const data_buffer& opcodes, const data_buffer& params);
	offs_t disassemble_extended_a5_0f(std::ostream& stream, unsigned PC, offs_t pc, const data_buffer& opcodes, const data_buffer& params);
	offs_t disassemble_extended_a5_d0(std::ostream& stream, unsigned PC, offs_t pc, const data_buffer& opcodes, const data_buffer& params);
	offs_t disassemble_extended_a5_d1(std::ostream& stream, unsigned PC, offs_t pc, const data_buffer& opcodes, const data_buffer& params);

};

class ax208_disassembler : public axc51core_disassembler
{
public:
	ax208_disassembler();
	virtual ~ax208_disassembler() = default;

	struct ax208_bios_info {
		int addr;
		const char *name;
	};

	static const ax208_bios_info bios_call_names[];

protected:
	virtual void disassemble_op_ljmp(std::ostream& stream, unsigned &PC, const data_buffer& params) override;
	virtual void disassemble_op_lcall(std::ostream& stream, unsigned &PC, const data_buffer& params) override;
};


#endif
