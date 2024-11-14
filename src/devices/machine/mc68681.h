// license:BSD-3-Clause
// copyright-holders:Mariusz Wojcieszek, R. Belmont
#ifndef MAME_MACHINE_MC68681_H
#define MAME_MACHINE_MC68681_H

#pragma once

#include "diserial.h"

#define MC68681_RX_FIFO_SIZE                3

// forward declaration
class duart_base_device;

// duart_channel class
class duart_channel : public device_t, public device_serial_interface
{
public:
	duart_channel(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_serial overrides
	virtual void rcv_complete() override;    // Rx completed receiving byte
	virtual void tra_complete() override;    // Tx completed sending byte
	virtual void tra_callback() override;    // Tx send bit

	uint8_t read_chan_reg(int reg);
	void write_chan_reg(int reg, uint8_t data);
	void update_interrupts();

	void rx_fifo_push(uint8_t data, uint8_t errors);
	uint8_t read_rx_fifo();

	void baud_updated();

	uint8_t get_chan_CSR();

	// Access methods needed for 68340 serial module register model
	uint8_t read_MR1(){ return MR1; }
	uint8_t read_MR2(){ return MR2; }
	void write_MR1(uint8_t data){ MR1 = data; }
	void write_MR2(uint8_t data){ MR2 = data; }

	void tx_16x_clock_w(bool state);
	void rx_16x_clock_w(bool state);

	int get_tx_rate() const { return tx_baud_rate; }

private:
	/* Registers */
	uint8_t CR;  /* Command register */
	uint8_t CSR; /* Clock select register */
	uint8_t MR1; /* Mode register 1 */
	uint8_t MR2; /* Mode register 2 */
	uint8_t MR_ptr; /* Mode register pointer */
	uint8_t SR;  /* Status register */

	/* State */
	int tx_baud_rate, rx_baud_rate;

	/* Receiver */
	uint8_t rx_enabled;
	uint16_t rx_fifo[MC68681_RX_FIFO_SIZE + 1];
	int   rx_fifo_read_ptr;
	int   rx_fifo_write_ptr;
	int   rx_fifo_num;

	int m_ch;

	/* Transmitter */
	uint8_t m_tx_data;
	bool m_tx_data_in_buffer;
	bool m_tx_break;
	uint8_t m_bits_transmitted;

	/* Rx/Tx clocking */
	uint8_t m_rx_prescaler , m_tx_prescaler;

	duart_base_device *m_uart;

	void write_MR(uint8_t data);
	void write_CR(uint8_t data);
	void write_TX(uint8_t data);
	void recalc_framing();
};

class duart_base_device : public device_t
{
	friend class duart_channel;

public:
	required_device<duart_channel> m_chanA;
	required_device<duart_channel> m_chanB;
	optional_device<duart_channel> m_chanC;
	optional_device<duart_channel> m_chanD;

	// inline configuration helpers
	void set_clocks(int clk3, int clk4, int clk5, int clk6);
	void set_clocks(const XTAL &clk3, const XTAL &clk4, const XTAL &clk5, const XTAL &clk6) {
		set_clocks(clk3.value(), clk4.value(), clk5.value(), clk6.value());
	}

	// API
	virtual uint8_t read(offs_t offset);
	virtual void write(offs_t offset, uint8_t data);

	void rx_a_w(int state) { m_chanA->device_serial_interface::rx_w((uint8_t)state); }
	void rx_b_w(int state) { m_chanB->device_serial_interface::rx_w((uint8_t)state); }

	auto irq_cb() { return write_irq.bind(); }
	auto a_tx_cb() { return write_a_tx.bind(); }
	auto b_tx_cb() { return write_b_tx.bind(); }
	auto inport_cb() { return read_inport.bind(); } // deprecated: use ipX_w() instead
	auto outport_cb() { return write_outport.bind(); }

	// new-style push handlers for input port bits
	void ip0_w(int state);
	void ip1_w(int state);
	void ip2_w(int state);
	void ip3_w(int state);
	void ip4_w(int state);
	void ip5_w(int state);
	void ip6_w(int state);

	bool irq_pending() const { return (ISR & IMR) != 0; }

protected:
	duart_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	devcb_write_line write_irq, write_a_tx, write_b_tx, write_c_tx, write_d_tx;
	devcb_read8 read_inport;
	devcb_write8 write_outport;
	int32_t ip3clk, ip4clk, ip5clk, ip6clk;

protected:
	virtual void update_interrupts();

private:
	TIMER_CALLBACK_MEMBER(duart_timer_callback);

protected:
	/* registers */
	uint8_t ACR;  /* Auxiliary Control Register */
	uint8_t IMR;  /* Interrupt Mask Register */
	uint8_t ISR;  /* Interrupt Status Register */

private:
	uint8_t OPCR; /* Output Port Conf. Register */
	uint8_t OPR;  /* Output Port Register */
	PAIR  CTR;  /* Counter/Timer Preset Value */
	uint8_t IPCR; /* Input Port Control Register */

	/* state */
	uint8_t IP_last_state; /* last state of IP bits */

	/* timer */
	uint8_t half_period;
	emu_timer *duart_timer;

	bool m_irq_state;

	uint32_t get_ct_rate();
	uint16_t get_ct_count();
	void start_ct(int count);
	virtual int calc_baud(int ch, bool rx, uint8_t data);
	void clear_ISR_bits(int mask);
	void set_ISR_bits(int mask);

	int get_ch(duart_channel *ch)
	{
		if (ch == m_chanA)
		{
			return 0;
		}
		else if (ch == m_chanB)
		{
			return 1;
		}
		else if (ch == m_chanC)
		{
			return 2;
		}

		return 3;
	}

	duart_channel *get_channel(int chan);
};

class scn2681_device : public duart_base_device
{
public:
	scn2681_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class mc68681_device : public duart_base_device
{
public:
	mc68681_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;
	uint8_t get_irq_vector();

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void update_interrupts() override;
	mc68681_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

private:
	bool m_read_vector; // if this is read and IRQ is active, it counts as pulling IACK

	uint8_t IVR;  /* Interrupt Vector Register */
};

class sc28c94_device : public duart_base_device
{
public:
	sc28c94_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto c_tx_cb() { return write_c_tx.bind(); }
	auto d_tx_cb() { return write_d_tx.bind(); }

	void rx_c_w(int state) { m_chanC->device_serial_interface::rx_w((uint8_t)state); }
	void rx_d_w(int state) { m_chanD->device_serial_interface::rx_w((uint8_t)state); }

	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
};

class mc68340_duart_device : public duart_base_device
{
public:
	mc68340_duart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	mc68340_duart_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

private:
	virtual int calc_baud(int ch, bool rx, uint8_t data) override;
};

class xr68c681_device : public mc68681_device
{
public:
	xr68c681_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	virtual int calc_baud(int ch, bool rx, uint8_t data) override;

	bool m_XTXA,m_XRXA,m_XTXB,m_XRXB; /* X bits for the BRG (selects between 2 BRG tables) */
};

DECLARE_DEVICE_TYPE(SCN2681, scn2681_device)
DECLARE_DEVICE_TYPE(MC68681, mc68681_device)
DECLARE_DEVICE_TYPE(SC28C94, sc28c94_device)
DECLARE_DEVICE_TYPE(MC68340_DUART, mc68340_duart_device)
DECLARE_DEVICE_TYPE(XR68C681, xr68c681_device)
DECLARE_DEVICE_TYPE(DUART_CHANNEL, duart_channel)

#endif // MAME_MACHINE_MC68681_H
