// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#pragma once

#ifndef __MINX_H__
#define __MINX_H__


enum
{
		MINX_PC=1, MINX_SP, MINX_BA, MINX_HL, MINX_X, MINX_Y,
		MINX_U, MINX_V, MINX_F, MINX_E, MINX_N, MINX_I,
		MINX_XI, MINX_YI
};


class minx_cpu_device :  public cpu_device
{
public:
	// construction/destruction
	minx_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const override { return 1; }
	virtual UINT32 execute_max_cycles() const override { return 4; }
	virtual UINT32 execute_input_lines() const override { return 1; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override { return (spacenum == AS_PROGRAM) ? &m_program_config : nullptr; }

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const override { return 1; }
	virtual UINT32 disasm_max_opcode_bytes() const override { return 5; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

private:
	address_space_config m_program_config;

	UINT16  m_PC;
	UINT16  m_SP;
	UINT16  m_BA;
	UINT16  m_HL;
	UINT16  m_X;
	UINT16  m_Y;
	UINT8   m_U;
	UINT8   m_V;
	UINT8   m_F;
	UINT8   m_E;
	UINT8   m_N;
	UINT8   m_I;
	UINT8   m_XI;
	UINT8   m_YI;
	UINT8   m_halted;
	UINT8   m_interrupt_pending;
	address_space *m_program;
	int m_icount;
	// For debugger
	UINT32 m_curpc;
	UINT16 m_flags;

	UINT16 rd16( UINT32 offset );
	void wr16( UINT32 offset, UINT16 data );
	UINT8 rdop();
	UINT16 rdop16();
	UINT8 ADD8( UINT8 arg1, UINT8 arg2 );
	UINT16 ADD16( UINT16 arg1, UINT16 arg2 );
	UINT8 ADDC8( UINT8 arg1, UINT8 arg2 );
	UINT16 ADDC16( UINT16 arg1, UINT16 arg2 );
	UINT8 INC8( UINT8 arg );
	UINT16 INC16( UINT16 arg );
	UINT8 SUB8( UINT8 arg1, UINT8 arg2 );
	UINT16 SUB16( UINT16 arg1, UINT16 arg2 );
	UINT8 SUBC8( UINT8 arg1, UINT8 arg2 );
	UINT16 SUBC16( UINT16 arg1, UINT16 arg2 );
	UINT8 DEC8( UINT8 arg );
	UINT16 DEC16( UINT16 arg );
	UINT8 AND8( UINT8 arg1, UINT8 arg2 );
	UINT8 OR8( UINT8 arg1, UINT8 arg2 );
	UINT8 XOR8( UINT8 arg1, UINT8 arg2 );
	UINT8 NOT8( UINT8 arg );
	UINT8 NEG8( UINT8 arg );
	UINT8 SAL8( UINT8 arg );
	UINT8 SAR8( UINT8 arg );
	UINT8 SHL8( UINT8 arg );
	UINT8 SHR8( UINT8 arg );
	UINT8 ROLC8( UINT8 arg );
	UINT8 RORC8( UINT8 arg );
	UINT8 ROL8( UINT8 arg );
	UINT8 ROR8( UINT8 arg );
	void PUSH8( UINT8 arg );
	void PUSH16( UINT16 arg );
	UINT8 POP8();
	UINT16 POP16();
	void JMP( UINT16 arg );
	void CALL( UINT16 arg );

	void execute_one();
	void execute_one_ce();
	void execute_one_cf();

	static const int insnminx_cycles[256];
	static const int insnminx_cycles_CE[256];
	static const int insnminx_cycles_CF[256];

};


extern const device_type MINX;


#endif /* __MINX_H__ */
