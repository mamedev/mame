// license:BSD-3-Clause
// copyright-holders:R. Belmont,byuu,Jonathan Gevaryahu
/***************************************************************************

    upd7720.h

    Core implementation for the portable NEC uPD7720 emulator

****************************************************************************/

#ifndef MAME_CPU_UPD7720_UPD7720_H
#define MAME_CPU_UPD7720_UPD7720_H

#pragma once

//**************************************************************************
//  ENUMERATIONS
//**************************************************************************

// input lines
enum
{
	UPD7720_INPUT_LINE_INT = 0
	// add more here as needed
};

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> upd7720_device

class upd772x_device : public cpu_device
{
public:
	auto p0() { return m_out_p0_cb.bind(); }
	auto p1() { return m_out_p1_cb.bind(); }
	auto so16() { return m_out_so16_cb.bind(); }

	uint8_t status_r();
	uint8_t data_r();
	void data_w(uint8_t data);

protected:
	// construction/destruction
	upd772x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t abits, uint32_t dbits);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface implementation
	virtual uint32_t execute_min_cycles() const noexcept override;
	virtual uint32_t execute_max_cycles() const noexcept override;
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface implementation
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface implementation
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// inline data
	const address_space_config m_program_config, m_data_config, m_dataram_config;

private:
	struct Flag
	{
		bool s1, s0, c, z, ov1, ov0;

		operator unsigned() const
		{
			return (s1 << 5) | (s0 << 4) | (c << 3) | (z << 2) | (ov1 << 1) | (ov0 << 0);
		}

		unsigned operator=(unsigned d)
		{
			s1 = d & 0x20; s0 = d & 0x10; c = d & 0x08; z = d & 0x04; ov1 = d & 0x02; ov0 = d & 0x01;
			return d;
		}
	};


	struct Regs
	{
		uint16_t pc;          //program counter
		uint16_t stack[16];   //LIFO
		uint16_t rp;          //ROM pointer
		uint16_t dp;          //data pointer
		uint8_t  sp;          //stack pointer
		int16_t  k;
		int16_t  l;
		int16_t  m;
		int16_t  n;
		int16_t  a;         //accumulator
		int16_t  b;         //accumulator
		Flag  flaga;
		Flag  flagb;
		uint16_t tr;        //temporary register
		uint16_t sr;        //status register
		uint16_t dr;        //data register
		uint16_t si;
		uint16_t so;
		uint16_t idb;
		bool siack;         // Serial in ACK
		bool soack;         // Serial out ACK
	} regs;

	void exec_op(uint32_t opcode);
	void exec_rt(uint32_t opcode);
	void exec_jp(uint32_t opcode);
	void exec_ld(uint32_t opcode);

protected:
	uint16_t m_drammask;
private:
	int m_icount;
	bool m_irq; // old irq line state, for detecting rising edges.
	// m_irq_firing: if an irq has fired; 0 = not fired or has already finished firing
	// 1 = next opcode is the first half of int firing 'NOP'
	// 2 = next opcode is the second half of int firing 'CALL 0100'
	int m_irq_firing;
	memory_access<14, 2, -2, ENDIANNESS_LITTLE>::cache m_cache;
	memory_access<14, 2, -2, ENDIANNESS_LITTLE>::specific m_program;
	memory_access<12, 1, -1, ENDIANNESS_LITTLE>::specific m_data;
protected:
	memory_access<11, 1, -1, ENDIANNESS_BIG>::specific m_dataram;

protected:
	devcb_read_line     m_in_int_cb;
	//devcb_read8       m_in_si_cb;
	//devcb_read_line   m_in_sck_cb;
	//devcb_read_line   m_in_sien_cb;
	//devcb_read_line   m_in_soen_cb;
	//devcb_read_line   m_in_dack_cb;
	devcb_write_line    m_out_p0_cb;
	devcb_write_line    m_out_p1_cb;
	devcb_write16       m_out_so16_cb;
	//devcb_write_line  m_out_so_cb;
	//devcb_write_line  m_out_sorq_cb;
	//devcb_write_line  m_out_drq_cb;

	void dataram_map(address_map &map) ATTR_COLD;
};

class upd7720_device : public upd772x_device
{
public:
	// construction/destruction
	upd7720_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// device type definition
DECLARE_DEVICE_TYPE(UPD7720,  upd7720_device)

//**************************************************************************
//  ENUMERATIONS
//**************************************************************************

// registers
enum
{
	D7720_PC = 1,
	D7720_RP,
	D7720_DP,
	D7720_K,
	D7720_L,
	D7720_M,
	D7720_N,
	D7720_A,
	D7720_B,
	D7720_FLAGA,
	D7720_FLAGB,
	D7720_SR,
	D7720_DR,
	D7720_SP,
	D7720_TR,
	D7720_SI,
	D7720_SO,
	D7720_IDB,
	D7720_SIACK,
	D7720_SOACK
};

// alu ops
enum
{
	D7720ALU_NOP = 0,
	D7720ALU_OR,
	D7720ALU_AND,
	D7720ALU_XOR,
	D7720ALU_SUB,
	D7720ALU_ADD,
	D7720ALU_SBB,
	D7720ALU_ADC,
	D7720ALU_DEC,
	D7720ALU_INC,
	D7720ALU_CMP,
	D7720ALU_SHR1,
	D7720ALU_SHL1,
	D7720ALU_SHL2,
	D7720ALU_SHL4,
	D7720ALU_XCHG
};

// sr bitmasks
enum
{
	D7720SR_P0 =  0x0001,
	D7720SR_P1 =  0x0002,
	D7720SR_EI =  0x0080,
	D7720SR_SIC = 0x0100,
	D7720SR_SOC = 0x0200,
	D7720SR_DRC = 0x0400,
	D7720SR_DMA = 0x0800,
	D7720SR_DRS = 0x1000,
	D7720SR_USF0= 0x2000,
	D7720SR_USF1= 0x4000,
	D7720SR_RQM = 0x8000
};


#endif // MAME_CPU_UPD7720_UPD7720_H
