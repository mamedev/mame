// license:BSD-3-Clause
// copyright-holders:R. Belmont,byuu
/***************************************************************************

    upd7725.h

    Core implementation for the portable NEC uPD7725/uPD96050 emulator

****************************************************************************/

#pragma once

#ifndef __UPD7725_H__
#define __UPD7725_H__

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

class necdsp_device;
class upd7725_device;
class upd96050_device;


#define MCFG_NECDSP_IN_INT_CB(_devcb) \
	devcb = &necdsp_device::set_in_int_callback(*device, DEVCB_##_devcb);

#define MCFG_NECDSP_IN_SI_CB(_devcb) \
	devcb = &necdsp_device::set_in_si_callback(*device, DEVCB_##_devcb);

#define MCFG_NECDSP_IN_SCK_CB(_devcb) \
	devcb = &necdsp_device::set_in_sck_callback(*device, DEVCB_##_devcb);

#define MCFG_NECDSP_IN_SIEN_CB(_devcb) \
	devcb = &necdsp_device::set_in_sien_callback(*device, DEVCB_##_devcb);

#define MCFG_NECDSP_IN_SOEN_CB(_devcb) \
	devcb = &necdsp_device::set_in_soen_callback(*device, DEVCB_##_devcb);

#define MCFG_NECDSP_IN_DACK_CB(_devcb) \
	devcb = &necdsp_device::set_in_dack_callback(*device, DEVCB_##_devcb);

#define MCFG_NECDSP_OUT_P0_CB(_devcb) \
	devcb = &necdsp_device::set_out_p0_callback(*device, DEVCB_##_devcb);

#define MCFG_NECDSP_OUT_P1_CB(_devcb) \
	devcb = &necdsp_device::set_out_p1_callback(*device, DEVCB_##_devcb);

#define MCFG_NECDSP_OUT_SO_CB(_devcb) \
	devcb = &necdsp_device::set_out_so_callback(*device, DEVCB_##_devcb);

#define MCFG_NECDSP_OUT_SORQ_CB(_devcb) \
	devcb = &necdsp_device::set_out_sorq_callback(*device, DEVCB_##_devcb);

#define MCFG_NECDSP_OUT_DRQ_CB(_devcb) \
	devcb = &necdsp_device::set_out_drq_callback(*device, DEVCB_##_devcb);


// ======================> necdsp_device

class necdsp_device : public cpu_device
{
protected:
	// construction/destruction
	necdsp_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t abits, uint32_t dbits, const char *name, const char *shortname, const char *source);

public:

	template<class _Object> static devcb_base &set_in_int_callback(device_t &device, _Object object) { return downcast<necdsp_device &>(device).m_in_int_cb.set_callback(object); }
	//template<class _Object> static devcb_base &set_in_si_callback(device_t &device, _Object object) { return downcast<necdsp_device &>(device).m_in_si_cb.set_callback(object); }
	//template<class _Object> static devcb_base &set_in_sck_callback(device_t &device, _Object object) { return downcast<necdsp_device &>(device).m_in_sck_cb.set_callback(object); }
	//template<class _Object> static devcb_base &set_in_sien_callback(device_t &device, _Object object) { return downcast<necdsp_device &>(device).m_in_sien_cb.set_callback(object); }
	//template<class _Object> static devcb_base &set_in_soen_callback(device_t &device, _Object object) { return downcast<necdsp_device &>(device).m_in_soen_cb.set_callback(object); }
	//template<class _Object> static devcb_base &set_in_dack_callback(device_t &device, _Object object) { return downcast<necdsp_device &>(device).m_in_dack_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_p0_callback(device_t &device, _Object object) { return downcast<necdsp_device &>(device).m_out_p0_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_p1_callback(device_t &device, _Object object) { return downcast<necdsp_device &>(device).m_out_p1_cb.set_callback(object); }
	//template<class _Object> static devcb_base &set_out_so_callback(device_t &device, _Object object) { return downcast<necdsp_device &>(device).m_out_so_cb.set_callback(object); }
	//template<class _Object> static devcb_base &set_out_sorq_callback(device_t &device, _Object object) { return downcast<necdsp_device &>(device).m_out_sorq_cb.set_callback(object); }
	//template<class _Object> static devcb_base &set_out_drq_callback(device_t &device, _Object object) { return downcast<necdsp_device &>(device).m_out_drq_cb.set_callback(object); }

	uint8_t snesdsp_read(bool mode);
	void snesdsp_write(bool mode, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const override;
	virtual uint32_t execute_max_cycles() const override;
	virtual uint32_t execute_input_lines() const override;
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual uint32_t disasm_min_opcode_bytes() const override;
	virtual uint32_t disasm_max_opcode_bytes() const override;
	virtual offs_t disasm_disassemble(std::ostream &stream, offs_t pc, const uint8_t *oprom, const uint8_t *opram, uint32_t options) override;

	// inline data
	const address_space_config m_program_config, m_data_config;

	uint16_t dataRAM[2048];

private:
	struct Flag
	{
		bool s1, s0, c, z, ov1, ov0, ov0p, ov0pp;

		inline operator unsigned() const
		{
			return (s1 << 7) + (s0 << 6) + (c << 5) + (z << 4) + (ov1 << 3) + (ov0 << 2) + (ov0p << 1) + (ov0pp << 0);
		}

		inline unsigned operator=(unsigned d)
		{
			s1 = d & 0x80; s0 = d & 0x40; c = d & 0x20; z = d & 0x10; ov1 = d & 0x08; ov0 = d & 0x04; ov0p = d & 0x02; ov0pp = d & 0x01;
			return d;
		}
	};

	struct Status
	{
		bool rqm, usf1, usf0, drs, dma, drc, soc, sic, ei, p1, p0;

		inline operator unsigned() const
		{
			return (rqm << 15) + (usf1 << 14) + (usf0 << 13) + (drs << 12)
				+ (dma << 11) + (drc  << 10) + (soc  <<  9) + (sic <<  8)
				+ (ei  <<  7) + (p1   <<  1) + (p0   <<  0);
		}

		inline unsigned operator=(unsigned d)
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
	int m_irq; // old irq line state, for detecting rising edges.

	address_space *m_program, *m_data;
	direct_read_data *m_direct;

protected:
// device callbacks
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

	uint16_t dataram_r(uint16_t addr) { return dataRAM[addr]; }
	void dataram_w(uint16_t addr, uint16_t data) { dataRAM[addr] = data; }
};

// device type definition
extern const device_type UPD7725;
extern const device_type UPD96050;

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

#endif /* __UPD7725_H__ */
