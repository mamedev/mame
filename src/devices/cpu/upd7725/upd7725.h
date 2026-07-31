// license:BSD-3-Clause
// copyright-holders:R. Belmont,byuu
/***************************************************************************

    upd7725.h

    Core implementation for the portable NEC uPD7725/uPD96050 emulator

****************************************************************************/

#ifndef MAME_CPU_UPD7725_UPD7725_H
#define MAME_CPU_UPD7725_UPD7725_H

#pragma once

//**************************************************************************
//  ENUMERATIONS
//**************************************************************************

// input lines
enum
{
	NECDSP_INPUT_LINE_INT = 0
	// add more here as needed
};

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> necdsp_device

class necdsp_device : public cpu_device
{
public:
	auto p0() { return m_out_p0_cb.bind(); }
	auto p1() { return m_out_p1_cb.bind(); }

	uint8_t status_r();
	uint8_t data_r();
	void data_w(uint8_t data);

protected:
	// construction/destruction
	necdsp_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t abits, uint32_t dbits, uint32_t drambits);

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

	uint16_t m_pc;          //program counter
	uint16_t m_stack[16];   //LIFO
	uint16_t m_rp;          //ROM pointer
	uint16_t m_dp;          //data pointer
	uint8_t  m_sp;          //stack pointer
	int16_t  m_k;
	int16_t  m_l;
	int16_t  m_m;
	int16_t  m_n;
	int16_t  m_a;         //accumulator
	int16_t  m_b;         //accumulator
	Flag     m_flaga;
	Flag     m_flagb;
	uint16_t m_tr;        //temporary register
	uint16_t m_trb;       //temporary register
	uint16_t m_sr;        //status register
	uint16_t m_dr;        //data register
	uint16_t m_si;
	uint16_t m_so;
	uint16_t m_idb;
	bool     m_siack;     // Serial in ACK
	bool     m_soack;     // Serial out ACK

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
	memory_access<14, 2, -2, ENDIANNESS_BIG>::cache m_cache;
	memory_access<14, 2, -2, ENDIANNESS_BIG>::specific m_program;
	memory_access<12, 1, -1, ENDIANNESS_BIG>::specific m_data;
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
	//devcb_write8      m_out_so_cb;
	//devcb_write_line  m_out_sorq_cb;
	//devcb_write_line  m_out_drq_cb;

	void dataram_map(address_map &map) ATTR_COLD;
};

class upd7725_device : public necdsp_device
{
public:
	// construction/destruction
	upd7725_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class upd96050_device : public necdsp_device
{
public:
	// construction/destruction
	upd96050_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint16_t dataram_r(uint16_t addr) { return m_dataram.read_word(addr & m_drammask); }
	void dataram_w(uint16_t addr, uint16_t data, uint16_t mem_mask = uint16_t(~0))
	{
		uint16_t temp = (m_dataram.read_word(addr & m_drammask) & (~mem_mask));
		temp |= data & mem_mask;
		m_dataram.write_word(addr & m_drammask, temp);
	}
};

// device type definition
DECLARE_DEVICE_TYPE(UPD7725,  upd7725_device)
DECLARE_DEVICE_TYPE(UPD96050, upd96050_device)

//**************************************************************************
//  ENUMERATIONS
//**************************************************************************

// registers
enum
{
	D7725_PC = 1,
	D7725_RP,
	D7725_DP,
	D7725_K,
	D7725_L,
	D7725_M,
	D7725_N,
	D7725_A,
	D7725_B,
	D7725_FLAGA,
	D7725_FLAGB,
	D7725_SR,
	D7725_DR,
	D7725_SP,
	D7725_TR,
	D7725_TRB,
	D7725_SI,
	D7725_SO,
	D7725_IDB,
	D7725_SIACK,
	D7725_SOACK
};

// alu ops
enum
{
	D7725ALU_NOP = 0,
	D7725ALU_OR,
	D7725ALU_AND,
	D7725ALU_XOR,
	D7725ALU_SUB,
	D7725ALU_ADD,
	D7725ALU_SBB,
	D7725ALU_ADC,
	D7725ALU_DEC,
	D7725ALU_INC,
	D7725ALU_CMP,
	D7725ALU_SHR1,
	D7725ALU_SHL1,
	D7725ALU_SHL2,
	D7725ALU_SHL4,
	D7725ALU_XCHG
};

// sr bitmasks
enum
{
	D7725SR_P0 =  0x0001,
	D7725SR_P1 =  0x0002,
	D7725SR_EI =  0x0080,
	D7725SR_SIC = 0x0100,
	D7725SR_SOC = 0x0200,
	D7725SR_DRC = 0x0400,
	D7725SR_DMA = 0x0800,
	D7725SR_DRS = 0x1000,
	D7725SR_USF0= 0x2000,
	D7725SR_USF1= 0x4000,
	D7725SR_RQM = 0x8000
};


#endif // MAME_CPU_UPD7725_UPD7725_H
