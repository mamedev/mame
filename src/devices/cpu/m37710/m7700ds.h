// license:BSD-3-Clause
// copyright-holders:R. Belmont, Karl Stenerud
#ifndef MAME_CPU_M37710_M7700DS_H
#define MAME_CPU_M37710_M7700DS_H

#pragma once

#ifdef __sun
#undef SEC
#endif

/*

Mitsubishi 7700 CPU Emulator v0.10
By R. Belmont

Based on:
G65C816 CPU Emulator V0.92

Copyright Karl Stenerud

*/

class m7700_disassembler : public util::disasm_interface
{
public:
	struct config {
		virtual ~config() = default;

		virtual bool get_m_flag() const = 0;
		virtual bool get_x_flag() const = 0;
	};

	m7700_disassembler(config *conf);
	virtual ~m7700_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	enum class op : unsigned
	{
		ADC ,  AND ,  ASL ,  BCC ,  BCS ,  BEQ ,  BIT ,  BMI ,  BNE ,  BPL ,  BRA ,
		BRK ,  BRL ,  BVC ,  BVS ,  CLC ,  CLD ,  CLI ,  CLV ,  CMP ,  COP ,  CPX ,
		CPY ,  DEA ,  DEC ,  DEX ,  DEY ,  EOR ,  INA ,  INC ,  INX ,  INY ,  JML ,
		JMP ,  JSL ,  JSR ,  LDA ,  LDX ,  LDY ,  LSR ,  MVN ,  MVP ,  NOP ,  ORA ,
		PEA ,  PEI ,  PER ,  PHA ,  PHT ,  PHD ,  PHK ,  PHP ,  PHX ,  PHY ,  PLA ,
		PLB ,  PLD ,  PLP ,  PLX ,  PLY ,  CLP ,  ROL ,  ROR ,  RTI ,  RTL ,  RTS ,
		SBC ,  SEC ,  SED ,  SEI ,  SEP ,  STA ,  STP ,  STX ,  STY ,  STZ ,  TAX ,
		TAY ,  TCS ,  TCD ,  TDC ,  TRB ,  TSB ,  TSC ,  TSX ,  TXA ,  TXS ,  TXY ,
		TYA ,  TYX ,  WAI ,  WDM ,  XBA ,  XCE ,  MPY ,  DIV ,  MPYS,  DIVS,  RLA ,
		EXTS, EXTZ ,  LDT ,  LDM ,  UNK ,  SEB ,  SEM ,  CLM ,  STB ,  LDB ,  ADCB ,
		SBCB, EORB ,  TBX ,  CMPB,  INB ,  DEB ,  TXB ,  TYB ,  LSRB,  ORB ,  CLB ,
		BBC,   BBS,   TBY,   ANDB,  PUL ,  PSH ,  PLAB,  XAB ,  PHB
	};

	enum
	{
		IMP , ACC , RELB, RELW, IMM , A   , AI  , AL  , ALX , AX  , AXI ,
		AY  , D   , DI  , DIY , DLI , DLIY, DX  , DXI , DY  , S   , SIY ,
		SIG , MVN , MVP , PEA , PEI , PER , LDM4, LDM5, LDM4X, LDM5X,
		BBCD, BBCA, ACCB
	};

	enum
	{
		I, /* ignore */
		M, /* check m bit */
		X  /* check x bit */
	};

	class m7700_opcode_struct
	{
	public:
		bool is_call() const { return m_name == op::JSR; }
		bool is_return() const { return (m_name == op::RTS) || (m_name == op::RTI); }
		bool is_bcond() const { return (ea == RELB && m_name != op::BRA) || (m_name == op::BBS) || (m_name == op::BBC); }
		const char *name() const { return s_opnames[unsigned(m_name)]; }

		static const m7700_opcode_struct &get(unsigned char ins) { return s_opcodes[ins]; }
		static const m7700_opcode_struct &get_prefix42(unsigned char ins) { return s_opcodes_prefix42[ins]; }
		static const m7700_opcode_struct &get_prefix89(unsigned char ins) { return s_opcodes_prefix89[ins]; }

		unsigned char flag;
		unsigned char ea;
		op m_name;

		m7700_opcode_struct(op n, unsigned char f, unsigned char e)
			: flag(f)
			, ea(e)
			, m_name(n)
		{
		}
	};

	static const char *const s_opnames[];
	static const m7700_opcode_struct s_opcodes[256];
	static const m7700_opcode_struct s_opcodes_prefix42[256];
	static const m7700_opcode_struct s_opcodes_prefix89[256];

	config *m_config;

	std::string int_8_str(unsigned int val);
	std::string int_16_str(unsigned int val);
};

#endif /* MAME_CPU_M37710_M7700DS_H */
