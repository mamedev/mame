// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli
/*

 Hyperstone disassembler
 written by Pierpaolo Prazzoli

*/

#ifndef MAME_CPU_E132XS_32XSDASM_H
#define MAME_CPU_E132XS_32XSDASM_H

#pragma once

class hyperstone_disassembler : public util::disasm_interface
{
public:
	struct config {
		virtual ~config() = default;

		virtual u8 get_fp() const = 0;
		virtual bool get_h() const = 0;
	};

	hyperstone_disassembler(config *conf);
	virtual ~hyperstone_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	static const char *const L_REG[];
	static const char *const G_REG[];
	static const char *const SETxx[];

	config *m_config;

	int size;
	u8 global_fp;

	void LL_format(char *source, char *dest, uint16_t op);
	void LR_format(char *source, char *dest, uint16_t op);
	void RR_format(char *source, char *dest, uint16_t op, unsigned h_flag);
	uint32_t LRconst_format(char *source, char *dest, uint16_t op, offs_t &pc, const data_buffer &opcodes);
	uint32_t RRconst_format(char *source, char *dest, uint16_t op, offs_t &pc, const data_buffer &opcodes);
	int32_t Rimm_format(char *dest, uint16_t op, offs_t &pc, const data_buffer &opcodes, unsigned h_flag);
	uint8_t Ln_format(char *dest, uint16_t op);
	uint8_t Rn_format(char *dest, uint16_t op);
	int32_t PCrel_format(uint16_t op, offs_t pc, const data_buffer &opcodes);
	uint32_t RRdis_format(char *source, char *dest, uint16_t op, uint16_t next_op, offs_t pc, const data_buffer &opcodes);
};

#endif
