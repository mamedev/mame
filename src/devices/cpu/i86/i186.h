// license:BSD-3-Clause
// copyright-holders:Carl
#ifndef MAME_CPU_I86_I186_H
#define MAME_CPU_I86_I186_H

#pragma once

#include "i86.h"

DECLARE_DEVICE_TYPE(I80186, i80186_cpu_device)
DECLARE_DEVICE_TYPE(I80188, i80188_cpu_device)

class i80186_cpu_device : public i8086_common_cpu_device
{
public:
	// construction/destruction
	i80186_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto read_slave_ack_callback() { return m_read_slave_ack_func.bind(); }
	auto chip_select_callback() { return m_out_chip_select_func.bind(); }
	auto tmrout0_handler() { return m_out_tmrout0_func.bind(); }
	auto tmrout1_handler() { return m_out_tmrout1_func.bind(); }

	IRQ_CALLBACK_MEMBER(int_callback);
	DECLARE_WRITE_LINE_MEMBER(drq0_w) { m_dma[0].drq_state = state; }
	DECLARE_WRITE_LINE_MEMBER(drq1_w) { m_dma[1].drq_state = state; }
	DECLARE_WRITE_LINE_MEMBER(tmrin0_w) { if(state && (m_timer[0].control & 0x8004) == 0x8004) { inc_timer(0); } }
	DECLARE_WRITE_LINE_MEMBER(tmrin1_w) { if(state && (m_timer[1].control & 0x8004) == 0x8004) { inc_timer(1); } }
	DECLARE_WRITE_LINE_MEMBER(int0_w) { external_int(0, state); }
	DECLARE_WRITE_LINE_MEMBER(int1_w) { external_int(1, state); }
	DECLARE_WRITE_LINE_MEMBER(int2_w) { external_int(2, state); }
	DECLARE_WRITE_LINE_MEMBER(int3_w) { external_int(3, state); }

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

protected:
	enum
	{
		I80186_RELREG = I8086_HALT + 1,
		I80186_UMCS, I80186_LMCS, I80186_PACS, I80186_MMCS, I80186_MPCS,
		I80186_DxSRC,
		I80186_DxDST = I80186_DxSRC + 2,
		I80186_DxTC = I80186_DxDST + 2,
		I80186_DxCON = I80186_DxTC + 2,
		I80186_TxCNT = I80186_DxCON + 2,
		I80186_TxCMPA = I80186_TxCNT + 3,
		I80186_TxCMPB = I80186_TxCMPA + 3,
		I80186_TxCON = I80186_TxCMPB + 2,
		I80186_INSERV = I80186_TxCON + 3,
		I80186_REQST, I80186_PRIMSK, I80186_INTSTS,
		I80186_TCUCON, I80186_DMA0CON, I80186_DMA1CON,
		I80186_I0CON, I80186_I1CON, I80186_I2CON, I80186_I3CON,
		I80186_POLLSTS
	};

	i80186_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int data_bus_size);

	// device_execute_interface overrides
	virtual uint64_t execute_clocks_to_cycles(uint64_t clocks) const override { return (clocks / 2); }
	virtual uint64_t execute_cycles_to_clocks(uint64_t cycles) const override { return (cycles * 2); }
	virtual void execute_run() override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual uint32_t execute_input_lines() const override { return 1; }
	virtual uint8_t fetch() override;
	uint32_t update_pc() { return m_pc = (m_sregs[CS] << 4) + m_ip; }

	virtual uint8_t read_port_byte(uint16_t port) override;
	virtual uint16_t read_port_word(uint16_t port) override;
	virtual void write_port_byte(uint16_t port, uint8_t data) override;
	void write_port_byte_al(uint16_t port);
	virtual void write_port_word(uint16_t port, uint16_t data) override;
	virtual uint8_t read_byte(uint32_t addr) override;
	virtual uint16_t read_word(uint32_t addr) override;
	virtual void write_byte(uint32_t addr, uint8_t data) override;
	virtual void write_word(uint32_t addr, uint16_t data) override;

	static const uint8_t m_i80186_timing[200];

private:
	void update_interrupt_state();
	void handle_eoi(int data);
	void external_int(uint16_t intno, int state);
	void internal_timer_sync(int which);
	void internal_timer_update(int which, int new_count, int new_maxA, int new_maxB, int new_control);
	void update_dma_control(int which, int new_control);
	void drq_callback(int which);
	void inc_timer(int which);
	DECLARE_READ16_MEMBER(internal_port_r);
	DECLARE_WRITE16_MEMBER(internal_port_w);

	struct mem_state
	{
		uint16_t      lower;
		uint16_t      upper;
		uint16_t      middle;
		uint16_t      middle_size;
		uint16_t      peripheral;
	};

	struct timer_state
	{
		uint16_t      control;
		uint16_t      maxA;
		uint16_t      maxB;
		bool        active_count;
		uint16_t      count;
		emu_timer   *int_timer;
	};

	struct dma_state
	{
		bool        drq_state;
		uint32_t      source;
		uint32_t      dest;
		uint16_t      count;
		uint16_t      control;
	};

	struct intr_state
	{
		uint8_t       pending;
		uint16_t      ack_mask;
		uint16_t      priority_mask;
		uint16_t      in_service;
		uint16_t      request;
		uint16_t      status;
		uint16_t      poll_status;
		uint16_t      timer;
		uint16_t      dma[2];
		uint16_t      ext[4];
		uint8_t       ext_state;
	};

	timer_state     m_timer[3];
	dma_state       m_dma[2];
	intr_state      m_intr;
	mem_state       m_mem;
	bool            m_last_dma;

	static const device_timer_id TIMER_INT0 = 0;
	static const device_timer_id TIMER_INT1 = 1;
	static const device_timer_id TIMER_INT2 = 2;

	uint16_t m_reloc;

	address_space_config m_program_config;
	address_space_config m_opcodes_config;
	address_space_config m_io_config;

	devcb_read8 m_read_slave_ack_func;
	devcb_write16 m_out_chip_select_func;
	devcb_write_line m_out_tmrout0_func;
	devcb_write_line m_out_tmrout1_func;
};

class i80188_cpu_device : public i80186_cpu_device
{
public:
	// construction/destruction
	i80188_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

#endif // MAME_CPU_I86_I186_H
