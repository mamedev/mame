// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************


    SCC68070 SoC peripheral emulation
    -------------------

    written by Ryan Holtz


*******************************************************************************

STATUS:

- Skeleton.  Just enough for the CD-i to run.

TODO:

- Proper handling of the 68070's internal devices (UART, DMA, Timers, etc.)

*******************************************************************************/

#ifndef MAME_MACHINE_SCC68070_H
#define MAME_MACHINE_SCC68070_H

#pragma once

#include "cpu/m68000/scc68070.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************
enum scc68070_ocr_bits
{
	SCC68070_OCR_D           = 0x80,
	SCC68070_OCR_D_M2D       = 0x00,
	SCC68070_OCR_D_D2M       = 0x80,
	SCC68070_OCR_OS          = 0x30,
	SCC68070_OCR_OS_BYTE     = 0x00,
	SCC68070_OCR_OS_WORD     = 0x10
};

// ======================> scc68070_device

class scc68070_device : public scc68070_base_device
{
public:
	scc68070_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto iack2_callback() { return m_iack2_callback.bind(); }
	auto iack4_callback() { return m_iack4_callback.bind(); }
	auto iack5_callback() { return m_iack5_callback.bind(); }
	auto iack7_callback() { return m_iack7_callback.bind(); }
	auto uart_tx_callback() { return m_uart_tx_callback.bind(); }
	auto uart_rtsn_callback() { return m_uart_rtsn_callback.bind(); }
	auto i2c_scl_w() { return m_i2c_scl_callback.bind(); }
	auto i2c_sda_w() { return m_i2c_sdaw_callback.bind(); }
	auto i2c_sda_r() { return m_i2c_sdar_callback.bind(); }

	void in2_w(int state);
	void in4_w(int state);
	void in5_w(int state);
	void nmi_w(int state);
	void int1_w(int state);
	void int2_w(int state);

	void write_scl(int state);

	TIMER_CALLBACK_MEMBER(timer0_callback);
	TIMER_CALLBACK_MEMBER(rx_callback);
	TIMER_CALLBACK_MEMBER(tx_callback);
	TIMER_CALLBACK_MEMBER(i2c_callback);

	// external callbacks
	void uart_rx(uint8_t data);
	void uart_ctsn(int state);

	// register structures
	struct i2c_regs_t
	{
		uint8_t reserved0;
		uint8_t data_register;
		uint8_t reserved1;
		uint8_t address_register;
		uint8_t reserved2;
		uint8_t status_register;
		uint8_t reserved3;
		uint8_t control_register;
		uint8_t reserved;
		uint8_t clock_control_register;
		emu_timer* timer;
		bool scl_out_state;
		bool scl_in_state;
		bool sda_out_state;
		bool sda_in_state;
		uint8_t clock_change_state;
		uint8_t clocks;
		uint8_t state;
		uint8_t counter;
		bool first_byte;
		bool ack_or_nak_sent;
	};

	struct uart_regs_t
	{
		uint8_t reserved0;
		uint8_t mode_register;
		uint8_t reserved1;
		uint8_t status_register;
		uint8_t reserved2;
		uint8_t clock_select;
		uint8_t reserved3;
		uint8_t command_register;
		uint8_t reserved4;
		uint8_t transmit_holding_register;
		uint8_t reserved5;
		uint8_t receive_holding_register;

		int16_t receive_pointer;
		uint8_t receive_buffer[32768];
		emu_timer* rx_timer;

		int16_t transmit_pointer;
		uint8_t transmit_buffer[32768];
		emu_timer* tx_timer;
		bool transmit_ctsn;
	};

	struct timer_regs_t
	{
		uint8_t timer_status_register;
		uint8_t timer_control_register;
		uint16_t reload_register;
		uint16_t timer0;
		uint16_t timer1;
		uint16_t timer2;
		emu_timer* timer0_timer;
	};

	struct dma_channel_t
	{
		uint8_t channel_status;
		uint8_t channel_error;

		uint8_t reserved0[2];

		uint8_t device_control;
		uint8_t operation_control;
		uint8_t sequence_control;
		uint8_t channel_control;

		uint8_t reserved1[3];

		uint16_t transfer_counter;

		uint32_t memory_address_counter;

		uint8_t reserved2[4];

		uint32_t device_address_counter;

		uint8_t reserved3[40];
	};

	struct dma_regs_t
	{
		dma_channel_t channel[2];
	};

	struct mmu_desc_t
	{
		uint16_t attr;
		uint16_t length;
		uint8_t  undefined;
		uint8_t  segment;
		uint16_t base;
	};

	struct mmu_regs_t
	{
		uint8_t status;
		uint8_t control;

		uint8_t reserved[0x3e];

		mmu_desc_t desc[8];
	};

	dma_regs_t& dma() { return m_dma; }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_config_complete() override;

	// device_execute_interface implementation
	virtual u64 execute_clocks_to_cycles(u64 clocks) const noexcept override { return (clocks + 2 - 1) / 2; }
	virtual u64 execute_cycles_to_clocks(u64 cycles) const noexcept override { return (cycles * 2); }

private:
	void internal_map(address_map &map) ATTR_COLD;
	void cpu_space_map(address_map &map) ATTR_COLD;

	void reset_peripherals(int state);

	void update_ipl();
	uint8_t iack_r(offs_t offset);

	// Interrupts
	uint8_t lir_r();
	void lir_w(uint8_t data);
	uint8_t picr1_r();
	void picr1_w(uint8_t data);
	uint8_t picr2_r();
	void picr2_w(uint8_t data);

	// I2C interface
	uint8_t idr_r();
	void idr_w(uint8_t data);
	uint8_t iar_r();
	void iar_w(uint8_t data);
	uint8_t isr_r();
	void isr_w(uint8_t data);
	uint8_t icr_r();
	void icr_w(uint8_t data);
	uint8_t iccr_r();
	void iccr_w(uint8_t data);
	void set_i2c_timer();
	void i2c_process_falling_scl();
	void i2c_process_rising_scl();
	void i2c_next_state();

	// UART interface
	uint8_t umr_r();
	void umr_w(uint8_t data);
	uint8_t usr_r();
	uint8_t ucsr_r();
	void ucsr_w(uint8_t data);
	uint8_t ucr_r();
	void ucr_w(uint8_t data);
	uint8_t uth_r();
	void uth_w(uint8_t data);
	uint8_t urh_r();

	// Timers
	uint16_t timer_r(offs_t offset, uint16_t mem_mask);
	void timer_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	// DMA controller
	uint16_t dma_r(offs_t offset, uint16_t mem_mask);
	void dma_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	// MMU
	uint16_t mmu_r(offs_t offset, uint16_t mem_mask);
	void mmu_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	void uart_rx_check();
	void uart_tx_check();
	void uart_tx(uint8_t data);
	void uart_do_tx();
	void set_timer_callback(int channel);

	// callbacks
	devcb_read8 m_iack2_callback;
	devcb_read8 m_iack4_callback;
	devcb_read8 m_iack5_callback;
	devcb_read8 m_iack7_callback;
	devcb_write8 m_uart_tx_callback;
	devcb_write_line m_uart_rtsn_callback;
	devcb_write_line m_i2c_scl_callback;
	devcb_write_line m_i2c_sdaw_callback;
	devcb_read8 m_i2c_sdar_callback;

	// internal state
	uint8_t m_ipl;
	int m_in2_line;
	int m_in4_line;
	int m_in5_line;
	int m_nmi_line;
	int m_int1_line;
	int m_int2_line;

	uint8_t m_lir;
	uint8_t m_picr1;
	uint8_t m_picr2;
	bool m_timer_int;
	bool m_i2c_int;
	bool m_uart_rx_int;
	bool m_uart_tx_int;

	i2c_regs_t m_i2c;
	uart_regs_t m_uart;
	timer_regs_t m_timers;
	dma_regs_t m_dma;
	mmu_regs_t m_mmu;
};

// device type definition
DECLARE_DEVICE_TYPE(SCC68070, scc68070_device)

#endif // MAME_MACHINE_SCC68070_H
