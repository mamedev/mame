// license:BSD-3-Clause
// copyright-holders:Carl
#ifndef MAME_CPU_I86_I186_H
#define MAME_CPU_I86_I186_H

#pragma once

#include "i86.h"

DECLARE_DEVICE_TYPE(I80186, i80186_cpu_device)
DECLARE_DEVICE_TYPE(I80188, i80188_cpu_device)
DECLARE_DEVICE_TYPE(AM186EM, am186em_device)
DECLARE_DEVICE_TYPE(AM188EM, am188em_device)

class i80186_cpu_device : public i8086_common_cpu_device
{
public:
	// construction/destruction
	i80186_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto read_slave_ack_callback() { return m_read_slave_ack_func.bind(); }
	auto chip_select_callback() { return m_out_chip_select_func.bind(); }
	auto tmrout0_handler() { return m_out_tmrout0_func.bind(); }
	auto tmrout1_handler() { return m_out_tmrout1_func.bind(); }
	auto irmx_irq_cb() { return m_irmx_irq_cb.bind(); }
	auto irqa_cb() { return m_irqa_cb.bind(); }
	template <typename... T> void set_irmx_irq_ack(T &&... args) { m_irmx_irq_ack.set(std::forward<T>(args)...); }

	IRQ_CALLBACK_MEMBER(int_callback);
	IRQ_CALLBACK_MEMBER(inta_callback);
	void drq0_w(int state) { m_dma[0].drq_state = state; }
	void drq1_w(int state) { m_dma[1].drq_state = state; }
	void tmrin0_w(int state) { external_tmrin(0, state); }
	void tmrin1_w(int state) { external_tmrin(1, state); }
	void int0_w(int state) { external_int(0, state); }
	void int1_w(int state) { external_int(1, state); }
	void int2_w(int state) { external_int(2, state); }
	void int3_w(int state) { external_int(3, state); }

	// This a hack, only use if there are sync problems with another cpu
	void dma_sync_req(int which) { drq_callback(which); }

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
	virtual uint64_t execute_clocks_to_cycles(uint64_t clocks) const noexcept override { return (clocks / 2); }
	virtual uint64_t execute_cycles_to_clocks(uint64_t cycles) const noexcept override { return (cycles * 2); }
	virtual void execute_run() override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual uint8_t fetch() override;
	uint32_t update_pc() { return m_pc = (m_sregs[CS] << 4) + m_ip; }

	virtual uint8_t read_port_byte(uint16_t port) override;
	virtual uint16_t read_port_word(uint16_t port) override;
	virtual void write_port_byte(uint16_t port, uint8_t data) override;
	virtual void write_port_byte_al(uint16_t port) override;
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
	void restart_timer(int which);
	void internal_timer_sync(int which);
	void internal_timer_update(int which, int new_count, int new_maxA, int new_maxB, int new_control);
	void external_tmrin(int which, int state);
	void update_dma_control(int which, int new_control);
	void drq_callback(int which);
	void inc_timer(int which);
	uint16_t internal_port_r(offs_t offset, uint16_t mem_mask = ~0);
	void internal_port_w(offs_t offset, uint16_t data);

	TIMER_CALLBACK_MEMBER(timer_elapsed);

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
		uint8_t       vector;
		uint8_t       pending;
		uint16_t      ack_mask;
		uint16_t      priority_mask;
		uint16_t      in_service;
		uint16_t      request;
		uint16_t      status;
		uint16_t      poll_status;
		uint16_t      timer[3];
		uint16_t      dma[2];
		uint16_t      ext[4];
		uint8_t       ext_state;
	};

	timer_state     m_timer[3];
	dma_state       m_dma[2];
	intr_state      m_intr;
	mem_state       m_mem;
	bool            m_last_dma;

	uint16_t m_reloc;

	address_space_config m_program_config;
	address_space_config m_opcodes_config;
	address_space_config m_io_config;

	devcb_read8 m_read_slave_ack_func;
	devcb_write16 m_out_chip_select_func;
	devcb_write_line m_out_tmrout0_func;
	devcb_write_line m_out_tmrout1_func;
	devcb_write_line m_irmx_irq_cb;
	devcb_write_line m_irqa_cb;
	device_irq_acknowledge_delegate m_irmx_irq_ack;
};

class i80188_cpu_device : public i80186_cpu_device
{
public:
	// construction/destruction
	i80188_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class am186em_device : public i80186_cpu_device
{
public:
	// construction/destruction
	am186em_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_execute_interface overrides
	virtual uint64_t execute_clocks_to_cycles(uint64_t clocks) const noexcept override { return clocks; }
	virtual uint64_t execute_cycles_to_clocks(uint64_t cycles) const noexcept override { return cycles; }
};

class am188em_device : public i80186_cpu_device
{
public:
	// construction/destruction
	am188em_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_execute_interface overrides
	virtual uint64_t execute_clocks_to_cycles(uint64_t clocks) const noexcept override { return clocks; }
	virtual uint64_t execute_cycles_to_clocks(uint64_t cycles) const noexcept override { return cycles; }
};

#endif // MAME_CPU_I86_I186_H
