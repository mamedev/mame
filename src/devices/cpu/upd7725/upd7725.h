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

	uint8_t snesdsp_read(bool mode);
	void snesdsp_write(bool mode, uint8_t data);

protected:
	// construction/destruction
	necdsp_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t abits, uint32_t dbits);

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
	const address_space_config m_program_config, m_data_config;

	uint16_t dataRAM[2048];

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

	struct Status
	{
		bool rqm, usf1, usf0, drs, dma, drc, soc, sic, ei, p1, p0;

		operator unsigned() const
		{
			return (rqm << 15) | (usf1 << 14) | (usf0 << 13) | (drs << 12)
				| (dma << 11) | (drc  << 10) | (soc  <<  9) | (sic <<  8)
				| (ei  <<  7) | (p1   <<  1) | (p0   <<  0);
		}

		unsigned operator=(unsigned d)
		{
			rqm = d & 0x8000; usf1 = d & 0x4000; usf0 = d & 0x2000; drs = d & 0x1000;
			dma = d & 0x0800; drc  = d & 0x0400; soc  = d & 0x0200; sic = d & 0x0100;
			ei  = d & 0x0080; p1   = d & 0x0002; p0   = d & 0x0001;
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
		uint16_t trb;       //temporary register
		Status sr;        //status register
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

	uint16_t dataram_r(uint16_t addr) { return dataRAM[addr & 0x07ff]; }
	void dataram_w(uint16_t addr, uint16_t data) { dataRAM[addr & 0x07ff] = data; }
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
	UPD7725_PC = 1,
	UPD7725_RP,
	UPD7725_DP,
	UPD7725_K,
	UPD7725_L,
	UPD7725_M,
	UPD7725_N,
	UPD7725_A,
	UPD7725_B,
	UPD7725_FLAGA,
	UPD7725_FLAGB,
	UPD7725_SR,
	UPD7725_DR,
	UPD7725_SP,
	UPD7725_TR,
	UPD7725_TRB,
	UPD7725_SI,
	UPD7725_SO,
	UPD7725_IDB,
	UPD7725_SIACK,
	UPD7725_SOACK
};

#endif // MAME_CPU_UPD7725_UPD7725_H
