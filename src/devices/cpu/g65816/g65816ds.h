// license:BSD-3-Clause
// copyright-holders:Karl Stenerud
#pragma once

#ifndef __G65816DS_H__
#define __G65816DS_H__
/* ======================================================================== */
/* =============================== COPYRIGHT ============================== */
/* ======================================================================== */
/*

G65C816 CPU Emulator V0.92

Copyright Karl Stenerud
All rights reserved.

*/

class g65816_disassembler : public util::disasm_interface
{
public:
	class config {
	public:
		virtual ~config() = default;
		virtual bool get_m_flag() const = 0;
		virtual bool get_x_flag() const = 0;
	};

	g65816_disassembler(config *conf);

	virtual u32 opcode_alignment() const override;
	virtual u32 interface_flags() const override;
	virtual u32 page_address_bits() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	enum class op : unsigned
	{
		ADC ,  AND ,  ASL ,  BCC ,  BCS ,  BEQ ,  BIT ,  BMI ,  BNE ,  BPL ,  BRA ,
		BRK ,  BRL ,  BVC ,  BVS ,  CLC ,  CLD ,  CLI ,  CLV ,  CMP ,  COP ,  CPX ,
		CPY ,  DEA ,  DEC ,  DEX ,  DEY ,  EOR ,  INA ,  INC ,  INX ,  INY ,  JML ,
		JMP ,  JSL ,  JSR ,  LDA ,  LDX ,  LDY ,  LSR ,  MVN ,  MVP ,  NOP ,  ORA ,
		PEA ,  PEI ,  PER ,  PHA ,  PHB ,  PHD ,  PHK ,  PHP ,  PHX ,  PHY ,  PLA ,
		PLB ,  PLD ,  PLP ,  PLX ,  PLY ,  REP ,  ROL ,  ROR ,  RTI ,  RTL ,  RTS ,
		SBC ,  SEC ,  SED ,  SEI ,  SEP ,  STA ,  STP ,  STX ,  STY ,  STZ ,  TAX ,
		TAY ,  TCS ,  TCD ,  TDC ,  TRB ,  TSB ,  TSC ,  TSX ,  TXA ,  TXS ,  TXY ,
		TYA ,  TYX ,  WAI ,  WDM ,  XBA ,  XCE
	};

	enum
	{
		IMP , ACC , RELB, RELW, IMM , A   , AI  , AL  , ALX , AX  , AXI ,
		AY  , D   , DI  , DIY , DLI , DLIY, DX  , DXI , DY  , S   , SIY ,
		SIG , MVN , MVP , PEA , PEI , PER
	};

	enum
	{
		I, /* ignore */
		M, /* check m bit */
		X  /* check x bit */
	};

	class opcode_struct {
	public:
		op m_name;
		u8 flag;
		u8 ea;

		opcode_struct(op n, u8 f, u8 e);
		const char *name() const;
		bool is_call() const;
		bool is_return() const;
	};

	static const char *const s_opnames[];
	static const opcode_struct s_opcodes[256];

	config *m_config;

	std::string int_8_str(u8 val);
	std::string int_16_str(u16 val);
};

#endif /* __G65816DS_H__ */
