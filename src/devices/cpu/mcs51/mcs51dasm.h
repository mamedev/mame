// license:BSD-3-Clause
// copyright-holders:Steve Ellenoff
/*****************************************************************************
 *
 * Portable MCS-51 Family Disassembler
 * Copyright Steve Ellenoff
 *
 *****************************************************************************/

#ifndef MAME_CPU_MCS51_MCS51DASM_H
#define MAME_CPU_MCS51_MCS51DASM_H

#pragma once

#include <unordered_map>


class mcs51_disassembler : public util::disasm_interface
{
public:
	struct mem_info {
		int addr;
		const char *name;
	};

	static const mem_info default_names[];
	static const mem_info i8052_names[];
	static const mem_info i80c52_names[];
	static const mem_info i8xc51fx_names[];
	static const mem_info i8xc51gb_names[];
	static const mem_info ds5002fp_names[];
	static const mem_info i8xc751_names[];
	static const mem_info ds80c320_names[];
	static const mem_info sab80515_names[];
	static const mem_info sab80c515_names[];
	static const mem_info rupi44_names[];
	static const mem_info p8xc552_names[];
	static const mem_info p8xc562_names[];

	template<typename ...Names> mcs51_disassembler(Names &&... names) : mcs51_disassembler() {
		add_names(names...);
	}

	mcs51_disassembler();
	virtual ~mcs51_disassembler() = default;

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

class i8051_disassembler : public mcs51_disassembler
{
public:
	i8051_disassembler();
	virtual ~i8051_disassembler() = default;
};

class i8052_disassembler : public mcs51_disassembler
{
public:
	i8052_disassembler();
	virtual ~i8052_disassembler() = default;
};

class i80c51_disassembler : public mcs51_disassembler
{
public:
	i80c51_disassembler();
	virtual ~i80c51_disassembler() = default;
};

class i80c52_disassembler : public mcs51_disassembler
{
public:
	i80c52_disassembler();
	virtual ~i80c52_disassembler() = default;
};

class i8xc51fx_disassembler : public mcs51_disassembler
{
public:
	i8xc51fx_disassembler();
	virtual ~i8xc51fx_disassembler() = default;
};

class i8xc51gb_disassembler : public mcs51_disassembler
{
public:
	i8xc51gb_disassembler();
	virtual ~i8xc51gb_disassembler() = default;
};

class ds5002fp_disassembler : public mcs51_disassembler
{
public:
	ds5002fp_disassembler();
	virtual ~ds5002fp_disassembler() = default;
};

class ds80c320_disassembler : public mcs51_disassembler
{
public:
	ds80c320_disassembler();
	virtual ~ds80c320_disassembler() = default;
};

class sab80515_disassembler : public mcs51_disassembler
{
public:
	sab80515_disassembler();
	virtual ~sab80515_disassembler() = default;
};

class sab80c515_disassembler : public mcs51_disassembler
{
public:
	sab80c515_disassembler();
	virtual ~sab80c515_disassembler() = default;
};

class rupi44_disassembler : public mcs51_disassembler
{
public:
	rupi44_disassembler();
	virtual ~rupi44_disassembler() = default;
};

class p8xc552_disassembler : public mcs51_disassembler
{
public:
	p8xc552_disassembler();
	virtual ~p8xc552_disassembler() = default;
};

class p8xc562_disassembler : public mcs51_disassembler
{
public:
	p8xc562_disassembler();
	virtual ~p8xc562_disassembler() = default;
};


#endif
