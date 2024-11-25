// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_CPU_MINX_MINX_H
#define MAME_CPU_MINX_MINX_H

#pragma once


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
	minx_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override { return 1; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 4; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

private:
	address_space_config m_program_config;

	uint16_t  m_PC;
	uint16_t  m_SP;
	uint16_t  m_BA;
	uint16_t  m_HL;
	uint16_t  m_X;
	uint16_t  m_Y;
	uint8_t   m_U;
	uint8_t   m_V;
	uint8_t   m_F;
	uint8_t   m_E;
	uint8_t   m_N;
	uint8_t   m_I;
	uint8_t   m_XI;
	uint8_t   m_YI;
	uint8_t   m_halted;
	uint8_t   m_interrupt_pending;
	address_space *m_program;
	int m_icount;
	// For debugger
	uint32_t m_curpc;
	uint16_t m_flags;

	uint16_t rd16( uint32_t offset );
	void wr16( uint32_t offset, uint16_t data );
	uint8_t rdop();
	uint16_t rdop16();
	uint8_t ADD8( uint8_t arg1, uint8_t arg2 );
	uint16_t ADD16( uint16_t arg1, uint16_t arg2 );
	uint8_t ADDC8( uint8_t arg1, uint8_t arg2 );
	uint16_t ADDC16( uint16_t arg1, uint16_t arg2 );
	uint8_t INC8( uint8_t arg );
	uint16_t INC16( uint16_t arg );
	uint8_t SUB8( uint8_t arg1, uint8_t arg2 );
	uint16_t SUB16( uint16_t arg1, uint16_t arg2 );
	uint8_t SUBC8( uint8_t arg1, uint8_t arg2 );
	uint16_t SUBC16( uint16_t arg1, uint16_t arg2 );
	uint8_t DEC8( uint8_t arg );
	uint16_t DEC16( uint16_t arg );
	uint8_t AND8( uint8_t arg1, uint8_t arg2 );
	uint8_t OR8( uint8_t arg1, uint8_t arg2 );
	uint8_t XOR8( uint8_t arg1, uint8_t arg2 );
	uint8_t NOT8( uint8_t arg );
	uint8_t NEG8( uint8_t arg );
	uint8_t SAL8( uint8_t arg );
	uint8_t SAR8( uint8_t arg );
	uint8_t SHL8( uint8_t arg );
	uint8_t SHR8( uint8_t arg );
	uint8_t ROLC8( uint8_t arg );
	uint8_t RORC8( uint8_t arg );
	uint8_t ROL8( uint8_t arg );
	uint8_t ROR8( uint8_t arg );
	void PUSH8( uint8_t arg );
	void PUSH16( uint16_t arg );
	uint8_t POP8();
	uint16_t POP16();
	void JMP( uint16_t arg );
	void CALL( uint16_t arg );

	void execute_one();
	void execute_one_ce();
	void execute_one_cf();

	static const int insnminx_cycles[256];
	static const int insnminx_cycles_CE[256];
	static const int insnminx_cycles_CF[256];

};


DECLARE_DEVICE_TYPE(MINX, minx_cpu_device)

#endif // MAME_CPU_MINX_MINX_H
