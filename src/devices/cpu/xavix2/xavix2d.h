// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, Nathan Gilbert

// Xavix2 disassembler

#ifndef MAME_CPU_XAVIX2_XAVIX2D_H
#define MAME_CPU_XAVIX2_XAVIX2D_H

#pragma once

class xavix2_disassembler : public util::disasm_interface
{
public:
	xavix2_disassembler() = default;
	virtual ~xavix2_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	static const u8 bpo[8];
	static const char *const reg_names[8];

	u32 m_pc, m_opcode;

	const char *r1();
	const char *r2();
	const char *r3();
	std::string val22h();
	std::string val22s();
	std::string val19s();
	std::string val19u();
	std::string val14h();
	std::string val14u();
	std::string val14s();
	std::string val14sa();
	std::string val11s();
	std::string val11u();
	std::string val6u();
	std::string val6s();
	std::string val3u();
	std::string off19s();
	std::string off14s();
	std::string off11s();
	std::string off6s();
	std::string off3s();
	std::string adr24();
	std::string adr16();
	std::string rel16();
	std::string rel8();
};

#endif // MAME_CPU_XAVIX2_XAVIX2D_H
