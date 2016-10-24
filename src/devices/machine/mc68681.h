// license:BSD-3-Clause
// copyright-holders:Mariusz Wojcieszek, R. Belmont
#ifndef _MC68681_H
#define _MC68681_H


#define MCFG_MC68681_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, MC68681, _clock)

#define MCFG_MC68681_REPLACE(_tag, _clock) \
	MCFG_DEVICE_REPLACE(_tag, MC68681, _clock)

#define MCFG_MC68681_IRQ_CALLBACK(_cb) \
	devcb = &mc68681_device::set_irq_cb(*device, DEVCB_##_cb);

#define MCFG_MC68681_A_TX_CALLBACK(_cb) \
	devcb = &mc68681_device::set_a_tx_cb(*device, DEVCB_##_cb);

#define MCFG_MC68681_B_TX_CALLBACK(_cb) \
	devcb = &mc68681_device::set_b_tx_cb(*device, DEVCB_##_cb);

// deprecated: use ipX_w() instead
#define MCFG_MC68681_INPORT_CALLBACK(_cb) \
	devcb = &mc68681_device::set_inport_cb(*device, DEVCB_##_cb);

#define MCFG_MC68681_OUTPORT_CALLBACK(_cb) \
	devcb = &mc68681_device::set_outport_cb(*device, DEVCB_##_cb);

#define MCFG_MC68681_SET_EXTERNAL_CLOCKS(_a, _b, _c, _d) \
	mc68681_device::static_set_clocks(*device, _a, _b, _c, _d);

#define MC68681_RX_FIFO_SIZE                3

// forward declaration
class mc68681_device;

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

	mc68681_device *m_uart;

	void write_MR(uint8_t data);
	void write_CR(uint8_t data);
	void write_TX(uint8_t data);
	void recalc_framing();
};

class mc68681_device : public device_t
{
	friend class mc68681_channel;

public:
	mc68681_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	required_device<mc68681_channel> m_chanA;
	required_device<mc68681_channel> m_chanB;

	// inline configuration helpers
	static void static_set_clocks(device_t &device, int clk3, int clk4, int clk5, int clk6);

	// API
	uint8_t read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t get_irq_vector() { m_read_vector = true; return IVR; }

	void rx_a_w(int state) { m_chanA->device_serial_interface::rx_w((uint8_t)state); }
	void rx_b_w(int state) { m_chanB->device_serial_interface::rx_w((uint8_t)state); }

	template<class _Object> static devcb_base &set_irq_cb(device_t &device, _Object object) { return downcast<mc68681_device &>(device).write_irq.set_callback(object); }
	template<class _Object> static devcb_base &set_a_tx_cb(device_t &device, _Object object) { return downcast<mc68681_device &>(device).write_a_tx.set_callback(object); }
	template<class _Object> static devcb_base &set_b_tx_cb(device_t &device, _Object object) { return downcast<mc68681_device &>(device).write_b_tx.set_callback(object); }
	template<class _Object> static devcb_base &set_inport_cb(device_t &device, _Object object) { return downcast<mc68681_device &>(device).read_inport.set_callback(object); }
	template<class _Object> static devcb_base &set_outport_cb(device_t &device, _Object object) { return downcast<mc68681_device &>(device).write_outport.set_callback(object); }

	devcb_write_line write_irq, write_a_tx, write_b_tx;
	devcb_read8 read_inport;
	devcb_write8 write_outport;
	int32_t ip3clk, ip4clk, ip5clk, ip6clk;

	// new-style push handlers for input port bits
	void ip0_w(int state);
	void ip1_w(int state);
	void ip2_w(int state);
	void ip3_w(int state);
	void ip4_w(int state);
	void ip5_w(int state);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

private:
	void duart_timer_callback(void *ptr, int32_t param);

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
	int get_ch(mc68681_channel *ch) { return (ch == m_chanA) ? 0 : 1; }
	void clear_ISR_bits(int mask);
	void set_ISR_bits(int mask);
	void update_interrupts();

	mc68681_channel *get_channel(int chan);
};

extern const device_type MC68681;
extern const device_type MC68681_CHANNEL;

#endif //_N68681_H
