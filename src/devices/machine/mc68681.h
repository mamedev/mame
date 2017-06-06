// license:BSD-3-Clause
// copyright-holders:Mariusz Wojcieszek, R. Belmont
#ifndef MAME_MACHINE_MC68681_H
#define MAME_MACHINE_MC68681_H

#pragma once


#define MCFG_MC68681_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, MC68681, _clock)

#define MCFG_MC68681_REPLACE(_tag, _clock) \
	MCFG_DEVICE_REPLACE(_tag, MC68681, _clock)

#define MCFG_MC68681_IRQ_CALLBACK(_cb) \
	devcb = &mc68681_base_device::set_irq_cb(*device, DEVCB_##_cb);

#define MCFG_MC68681_A_TX_CALLBACK(_cb) \
	devcb = &mc68681_base_device::set_a_tx_cb(*device, DEVCB_##_cb);

#define MCFG_MC68681_B_TX_CALLBACK(_cb) \
	devcb = &mc68681_base_device::set_b_tx_cb(*device, DEVCB_##_cb);

// deprecated: use ipX_w() instead
#define MCFG_MC68681_INPORT_CALLBACK(_cb) \
	devcb = &mc68681_base_device::set_inport_cb(*device, DEVCB_##_cb);

#define MCFG_MC68681_OUTPORT_CALLBACK(_cb) \
	devcb = &mc68681_base_device::set_outport_cb(*device, DEVCB_##_cb);

#define MCFG_MC68681_SET_EXTERNAL_CLOCKS(_a, _b, _c, _d) \
	mc68681_base_device::static_set_clocks(*device, _a, _b, _c, _d);

// SC28C94 specific callbacks
#define MCFG_SC28C94_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, SC28C94, _clock)

#define MCFG_SC28C94_C_TX_CALLBACK(_cb) \
	devcb = &sc28c94_device::set_c_tx_cb(*device, DEVCB_##_cb);

#define MCFG_SC28C94_D_TX_CALLBACK(_cb) \
	devcb = &sc28c94_device::set_d_tx_cb(*device, DEVCB_##_cb);

#define MC68681_RX_FIFO_SIZE                3

// forward declaration
class mc68681_base_device;

// mc68681_channel class
class mc68681_channel : public device_t, public device_serial_interface
{
public:
	mc68681_channel(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// device_serial overrides
	virtual void rcv_complete() override;    // Rx completed receiving byte
	virtual void tra_complete() override;    // Tx completed sending byte
	virtual void tra_callback() override;    // Tx send bit

	uint8_t read_chan_reg(int reg);
	void write_chan_reg(int reg, uint8_t data);
	void update_interrupts();

	uint8_t read_rx_fifo();

	void ACR_updated();

	uint8_t get_chan_CSR();

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
	uint8_t rx_fifo[MC68681_RX_FIFO_SIZE];
	int   rx_fifo_read_ptr;
	int   rx_fifo_write_ptr;
	int   rx_fifo_num;

	int m_ch;

	/* Transmitter */
	uint8_t tx_enabled;
	uint8_t tx_data;
	uint8_t tx_ready;

	mc68681_base_device *m_uart;

	void write_MR(uint8_t data);
	void write_CR(uint8_t data);
	void write_TX(uint8_t data);
	void recalc_framing();
};

class mc68681_base_device : public device_t
{
	friend class mc68681_channel;

public:
	required_device<mc68681_channel> m_chanA;
	required_device<mc68681_channel> m_chanB;
	optional_device<mc68681_channel> m_chanC;
	optional_device<mc68681_channel> m_chanD;

	// inline configuration helpers
	static void static_set_clocks(device_t &device, int clk3, int clk4, int clk5, int clk6);

	// API
	virtual DECLARE_READ8_MEMBER(read);
	virtual DECLARE_WRITE8_MEMBER(write);
	uint8_t get_irq_vector() { m_read_vector = true; return IVR; }

	DECLARE_WRITE_LINE_MEMBER( rx_a_w ) { m_chanA->device_serial_interface::rx_w((uint8_t)state); }
	DECLARE_WRITE_LINE_MEMBER( rx_b_w ) { m_chanB->device_serial_interface::rx_w((uint8_t)state); }

	template <class Object> static devcb_base &set_irq_cb(device_t &device, Object &&cb) { return downcast<mc68681_base_device &>(device).write_irq.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_a_tx_cb(device_t &device, Object &&cb) { return downcast<mc68681_base_device &>(device).write_a_tx.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_b_tx_cb(device_t &device, Object &&cb) { return downcast<mc68681_base_device &>(device).write_b_tx.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_inport_cb(device_t &device, Object &&cb) { return downcast<mc68681_base_device &>(device).read_inport.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_outport_cb(device_t &device, Object &&cb) { return downcast<mc68681_base_device &>(device).write_outport.set_callback(std::forward<Object>(cb)); }

	// new-style push handlers for input port bits
	DECLARE_WRITE_LINE_MEMBER( ip0_w );
	DECLARE_WRITE_LINE_MEMBER( ip1_w );
	DECLARE_WRITE_LINE_MEMBER( ip2_w );
	DECLARE_WRITE_LINE_MEMBER( ip3_w );
	DECLARE_WRITE_LINE_MEMBER( ip4_w );
	DECLARE_WRITE_LINE_MEMBER( ip5_w );

protected:
	mc68681_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	devcb_write_line write_irq, write_a_tx, write_b_tx, write_c_tx, write_d_tx;
	devcb_read8 read_inport;
	devcb_write8 write_outport;
	int32_t ip3clk, ip4clk, ip5clk, ip6clk;

private:
	TIMER_CALLBACK_MEMBER( duart_timer_callback );

	/* registers */
	uint8_t ACR;  /* Auxiliary Control Register */
	uint8_t IMR;  /* Interrupt Mask Register */
	uint8_t ISR;  /* Interrupt Status Register */
	uint8_t IVR;  /* Interrupt Vector Register */
	uint8_t OPCR; /* Output Port Conf. Register */
	uint8_t OPR;  /* Output Port Register */
	PAIR  CTR;  /* Counter/Timer Preset Value */
	uint8_t IPCR; /* Input Port Control Register */

	bool m_read_vector; // if this is read and IRQ is active, it counts as pulling IACK

	/* state */
	uint8_t IP_last_state; /* last state of IP bits */

	/* timer */
	uint8_t half_period;
	emu_timer *duart_timer;

	double duart68681_get_ct_rate();
	uint16_t duart68681_get_ct_count();
	void duart68681_start_ct(int count);
	int calc_baud(int ch, uint8_t data);
	void clear_ISR_bits(int mask);
	void set_ISR_bits(int mask);
	void update_interrupts();

	int get_ch(mc68681_channel *ch)
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

	mc68681_channel *get_channel(int chan);
};

class mc68681_device : public mc68681_base_device
{
public:
	mc68681_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class sc28c94_device : public mc68681_base_device
{
public:
	sc28c94_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <class Object> static devcb_base &set_c_tx_cb(device_t &device, Object &&cb) { return downcast<sc28c94_device &>(device).write_c_tx.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_d_tx_cb(device_t &device, Object &&cb) { return downcast<sc28c94_device &>(device).write_d_tx.set_callback(std::forward<Object>(cb)); }

	DECLARE_WRITE_LINE_MEMBER( rx_c_w ) { m_chanC->device_serial_interface::rx_w((uint8_t)state); }
	DECLARE_WRITE_LINE_MEMBER( rx_d_w ) { m_chanD->device_serial_interface::rx_w((uint8_t)state); }

	virtual DECLARE_READ8_MEMBER(read) override;
	virtual DECLARE_WRITE8_MEMBER(write) override;

protected:
	virtual void device_add_mconfig(machine_config &config) override;

private:
};

DECLARE_DEVICE_TYPE(MC68681, mc68681_device)
DECLARE_DEVICE_TYPE(SC28C94, sc28c94_device)
DECLARE_DEVICE_TYPE(MC68681_CHANNEL, mc68681_channel)

#endif // MAME_MACHINE_MC68681_H
