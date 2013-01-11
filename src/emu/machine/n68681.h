#ifndef _N68681_H
#define _N68681_H

#include "diserial.h"

#define MCFG_DUARTN68681_ADD(_tag, _clock, _config) \
	MCFG_DEVICE_ADD(_tag, DUARTN68681, _clock) \
	MCFG_DEVICE_CONFIG(_config)

#define MCFG_DUARTN68681_REPLACE(_tag, _clock, _config) \
	MCFG_DEVICE_REPLACE(_tag, DUARTN68681, _clock) \
	MCFG_DEVICE_CONFIG(_config)

#define MCFG_DUART68681_CHANNEL_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, DUART68681CHANNEL, 0) \

// forward declaration
class duartn68681_device;

struct duartn68681_config
{
	devcb_write_line    m_out_irq_cb;
	devcb_write_line    m_out_a_tx_cb;
	devcb_write_line    m_out_b_tx_cb;
	devcb_read8         m_in_port_cb;
	devcb_write8        m_out_port_cb;

	/* clocks for external baud rates */
	INT32 ip3clk, ip4clk, ip5clk, ip6clk;
};

#define MC68681_RX_FIFO_SIZE                3

// forward declaration
class duartn68681_device;

// n68681 channel class
class duart68681_channel : public device_t, public device_serial_interface
{
public:
	duart68681_channel(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_serial overrides
	virtual void rcv_complete();    // Rx completed receiving byte
	virtual void tra_complete();    // Tx completed sending byte
	virtual void tra_callback();    // Tx send bit
	void input_callback(UINT8 state);

	UINT8 read_chan_reg(int reg);
	void write_chan_reg(int reg, UINT8 data);
	void update_interrupts();

	UINT8 read_rx_fifo();

private:
	/* Registers */
	UINT8 CR;  /* Command register */
	UINT8 CSR; /* Clock select register */
	UINT8 MR1; /* Mode register 1 */
	UINT8 MR2; /* Mode register 2 */
	UINT8 MR_ptr; /* Mode register pointer */
	UINT8 SR;  /* Status register */

	/* State */
	int tx_baud_rate, rx_baud_rate;

	/* Receiver */
	UINT8 rx_enabled;
	UINT8 rx_fifo[MC68681_RX_FIFO_SIZE];
	int   rx_fifo_read_ptr;
	int   rx_fifo_write_ptr;
	int   rx_fifo_num;

	int m_ch;

	/* Transmitter */
	UINT8 tx_enabled;
	UINT8 tx_data;
	UINT8 tx_ready;

	duartn68681_device *m_uart;

	void write_MR(UINT8 data);
	void write_CR(UINT8 data);
	void write_TX(UINT8 data);
	void recalc_framing();
};

class duartn68681_device : public device_t, public duartn68681_config
{
	friend class duart68681_channel;

public:
	duartn68681_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	required_device<duart68681_channel> m_chanA;
	required_device<duart68681_channel> m_chanB;

	// API
	DECLARE_READ8_HANDLER(read);
	DECLARE_WRITE8_HANDLER(write);
	UINT8 get_irq_vector() { return IVR; }

	DECLARE_WRITE_LINE_MEMBER( rx_a_w ) { m_chanA->check_for_start((UINT8)state); }
	DECLARE_WRITE_LINE_MEMBER( rx_b_w ) { m_chanB->check_for_start((UINT8)state); }

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
	virtual machine_config_constructor device_mconfig_additions() const;

private:
	TIMER_CALLBACK_MEMBER( duart_timer_callback );

	/* registers */
	UINT8 ACR;  /* Auxiliary Control Register */
	UINT8 IMR;  /* Interrupt Mask Register */
	UINT8 ISR;  /* Interrupt Status Register */
	UINT8 IVR;  /* Interrupt Vector Register */
	UINT8 OPCR; /* Output Port Conf. Register */
	UINT8 OPR;  /* Output Port Register */
	PAIR  CTR;  /* Counter/Timer Preset Value */

	/* state */
	UINT8 IP_last_state; /* last state of IP bits */

	/* timer */
	UINT8 half_period;
	emu_timer *duart_timer;

	double duart68681_get_ct_rate();
	UINT16 duart68681_get_ct_count();
	void duart68681_start_ct(int count);
	int calc_baud(int ch, UINT8 data);
	int get_ch(duart68681_channel *ch) { return (ch == m_chanA) ? 0 : 1; }
	void clear_ISR_bits(int mask);
	void set_ISR_bits(int mask);
	void update_interrupts();

	duart68681_channel *get_channel(int chan);

	devcb_resolved_write_line   m_out_irq_func;
	devcb_resolved_write_line   m_out_a_tx_func;
	devcb_resolved_write_line   m_out_b_tx_func;
	devcb_resolved_read8        m_in_port_func;
	devcb_resolved_write8       m_out_port_func;
};

extern const device_type DUARTN68681;
extern const device_type DUART68681CHANNEL;

#endif //_N68681_H
