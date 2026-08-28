// license:BSD-3-Clause
// copyright-holders:superctr
/***************************************************************************

    Mitsubishi M37409M2 8-bit microcontroller (M740 core)

    Communication slave with three UARTs, a timer, two 8-bit ports and a
    192-byte dual-port RAM plus IPC mailbox/semaphore registers reachable
    from a master CPU through an 8-bit system bus (system_r/system_w).

***************************************************************************/
#ifndef MAME_CPU_M6502_M37409_H
#define MAME_CPU_M6502_M37409_H

#pragma once

#include "m740.h"

#include "diserial.h"


// ======================> m37409_uart_device

class m37409_uart_device : public device_t, public device_serial_interface
{
public:
	m37409_uart_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	auto txd_handler() { return m_txd_handler.bind(); }
	auto cts_handler() { return m_cts_handler.bind(); }     // CTS pin driven as an output
	auto rx_irq_handler() { return m_rx_irq_handler.bind(); }
	auto tx_irq_handler() { return m_tx_irq_handler.bind(); }

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

	void rxd_w(int state) { device_serial_interface::rx_w(state); }
	void cts_w(int state);

	bool rx_irq_enabled() const { return BIT(m_control, 3); }
	bool tx_irq_enabled() const { return BIT(m_control, 1); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void tra_callback() override;
	virtual void tra_complete() override;
	virtual void rcv_complete() override;

private:
	void update_frame();
	void update_cts();
	void try_transmit();

	devcb_write_line m_txd_handler;
	devcb_write_line m_cts_handler;
	devcb_write_line m_rx_irq_handler;
	devcb_write_line m_tx_irq_handler;

	u8 m_mode;
	u8 m_control;
	u8 m_status;
	u8 m_divider;
	u8 m_tx_buffer;
	u8 m_rx_buffer;
	int m_cts_in;
	int m_cts_out;
};


// ======================> m37409_device

class m37409_device : public m740_device
{
public:
	m37409_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	template <std::size_t Port> auto read_p() { return m_read_p[Port].bind(); }
	template <std::size_t Port> auto write_p() { return m_write_p[Port].bind(); }
	template <std::size_t Uart> auto txd_handler() { return m_txd_handler[Uart].bind(); }
	template <std::size_t Uart> auto cts_handler() { return m_cts_handler[Uart].bind(); }

	template <std::size_t Uart> void rxd_w(int state) { m_uart[Uart]->rxd_w(state); }
	template <std::size_t Uart> void cts_w(int state) { m_uart[Uart]->cts_w(state); }

	// master CPU side (256-byte system bus window)
	u8 system_r(offs_t offset);
	void system_w(offs_t offset, u8 data);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual u64 execute_clocks_to_cycles(u64 clocks) const noexcept override { return (clocks + 4 - 1) / 4; }
	virtual u64 execute_cycles_to_clocks(u64 cycles) const noexcept override { return cycles * 4; }

private:
	enum : u8
	{
		IRQ_UART1_RX = 0x80,
		IRQ_UART2_RX = 0x40,
		IRQ_UART3_RX = 0x20,
		IRQ_IPCM0    = 0x10,
		IRQ_TIMER    = 0x08,
		IRQ_UART1_TX = 0x04,
		IRQ_UART2_TX = 0x02,
		IRQ_UART3_TX = 0x01
	};

	void map(address_map &map) ATTR_COLD;

	u8 access_flag_r(offs_t offset);
	u8 dpram_r(offs_t offset);
	void dpram_w(offs_t offset, u8 data);
	u8 dpram_system_r(offs_t offset);
	void dpram_system_w(offs_t offset, u8 data);

	u8 p1_r();
	void p1_w(u8 data);
	void p1_dir_w(u8 data);
	u8 p0_r();
	void p0_w(u8 data);
	void p0_dir_w(u8 data);
	void update_p0();
	void update_p1();

	u8 ipcm_r(offs_t offset);
	u8 ipce_r(offs_t offset);
	void ipce_w(offs_t offset, u8 data);
	u8 semaphore_r();
	void semaphore_w(u8 data);
	u8 collision_r();
	void collision_w(u8 data);
	u8 int_enable_r();
	void int_enable_w(u8 data);
	u8 int_request_r();
	void int_request_w(u8 data);
	u8 prescaler_r();
	void prescaler_w(u8 data);
	u8 timer_r();
	void timer_w(u8 data);
	u8 timer_control_r();
	void timer_control_w(u8 data);
	u8 vector_r(offs_t offset);

	template <int N> void uart_rx_irq_w(int state);
	template <int N> void uart_tx_irq_w(int state);
	void set_irq_request(u8 mask);
	void recalc_irqs();
	void restart_timer();
	TIMER_CALLBACK_MEMBER(timer_tick);

	required_device_array<m37409_uart_device, 3> m_uart;
	required_region_ptr<u8> m_rom;

	devcb_read8::array<2> m_read_p;
	devcb_write8::array<2> m_write_p;
	devcb_write_line::array<3> m_txd_handler;
	devcb_write_line::array<3> m_cts_handler;

	emu_timer *m_timer;

	u8 m_dpram[0xc0];
	u8 m_access_flag[0x18];
	u8 m_dpram_dir;
	u8 m_p0_latch, m_p0_dir;
	u8 m_p1_latch, m_p1_dir;
	u8 m_ipcm[4];
	u8 m_ipce[4];
	u8 m_semaphore;
	u8 m_collision;
	u8 m_int_enable;
	u8 m_int_request;
	u8 m_prescaler;
	u8 m_timer_latch;
	u8 m_timer_control;
	u16 m_last_irqs;
};

DECLARE_DEVICE_TYPE(M37409, m37409_device)
DECLARE_DEVICE_TYPE(M37409_UART, m37409_uart_device)

#endif // MAME_CPU_M6502_M37409_H
