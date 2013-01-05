#ifndef _N68681_H
#define _N68681_H

#include "diserial.h"

// forward declaration
class duartn68681_device;

struct duartn68681_config
{
	devcb_write_line	m_out_irq_cb;
	devcb_write8		m_out_a_tx_cb;
	devcb_write8		m_out_b_tx_cb;
	devcb_read8			m_in_port_cb;
	devcb_write8		m_out_port_cb;

	/* clocks for external baud rates */
	INT32 ip3clk, ip4clk, ip5clk, ip6clk;
};

#define MC68681_RX_FIFO_SIZE				3

struct DUART68681_CHANNEL
{
	/* Registers */
	UINT8 CR;  /* Command register */
	UINT8 CSR; /* Clock select register */
	UINT8 MR1; /* Mode register 1 */
	UINT8 MR2; /* Mode register 2 */
	UINT8 MR_ptr; /* Mode register pointer */
	UINT8 SR;  /* Status register */

	/* State */
	int   baud_rate;

	/* Receiver */
	UINT8 rx_enabled;
	UINT8 rx_fifo[MC68681_RX_FIFO_SIZE];
	int   rx_fifo_read_ptr;
	int   rx_fifo_write_ptr;
	int   rx_fifo_num;

	/* Transmitter */
	UINT8 tx_enabled;
	UINT8 tx_data;
	UINT8 tx_ready;
	emu_timer *tx_timer;
};

class duartn68681_device : public device_t, public device_serial_interface, public duartn68681_config
{
public:
	duartn68681_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_HANDLER(read);
	DECLARE_WRITE8_HANDLER(write);

	void duart68681_rx_data(int ch, UINT8 data);

	TIMER_CALLBACK_MEMBER( tx_timer_callback );
	TIMER_CALLBACK_MEMBER( duart_timer_callback );

	UINT8 get_irq_vector() { return IVR; }

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

	// device_serial overrides
	virtual void rcv_complete();	// Rx complete
	virtual void tra_complete();	// Tx complete
	virtual void tra_callback();	// Tx bit ready
	void input_callback(UINT8 state);

private:
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

	/* UART channels */
	DUART68681_CHANNEL channel[2];

	void update_interrupts();
	double duart68681_get_ct_rate();
	UINT16 duart68681_get_ct_count();
	void duart68681_start_ct(int count);
	void duart68681_write_MR(int ch, UINT8 data);
	void duart68681_write_CSR(int ch, UINT8 data, UINT8 ACR);
	void duart68681_write_CR(int ch, UINT8 data);
	UINT8 duart68681_read_rx_fifo(int ch);
	void duart68681_write_TX(int ch, UINT8 data);

	devcb_resolved_write_line	m_out_irq_func;
    devcb_resolved_write8		m_out_a_tx_func;
    devcb_resolved_write8		m_out_b_tx_func;
	devcb_resolved_read8		m_in_port_func;
    devcb_resolved_write8		m_out_port_func;

};

#define MCFG_DUARTN68681_ADD(_tag, _clock, _config) \
	MCFG_DEVICE_ADD(_tag, DUARTN68681, _clock) \
	MCFG_DEVICE_CONFIG(_config)

extern const device_type DUARTN68681;

#endif //_N68681_H

