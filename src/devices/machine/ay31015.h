// license:BSD-3-Clause
// copyright-holders:Robbbert
/* ay31015.h

    Written for MESS by Robbbert on May 29th, 2008.

*/

#ifndef __AY31015_H_
#define __AY31015_H_

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/


enum ay31015_input_pin_t
{
	AY31015_SWE = 16,         /* -SWE  - Pin 16 - Status word enable */
	AY31015_RDAV = 18,        /* -RDAV - Pin 18 - Reset data available */
	AY31015_SI = 20,          /*  SI   - Pin 20 - Serial input */
	AY31015_XR = 21,          /*  XR   - Pin 21 - External reset */
	AY31015_CS = 34,          /*  CS   - Pin 34 - Control strobe */
	AY31015_NP = 35,          /*  NP   - Pin 35 - No parity */
	AY31015_TSB = 36,         /*  TSB  - Pin 36 - Number of stop bits */
	AY31015_NB2 = 37,         /*  NB2  - Pin 37 - Number of bits #2 */
	AY31015_NB1 = 38,         /*  NB1  - Pin 38 - Number of bits #1 */
	AY31015_EPS = 39          /*  EPS  - Pin 39 - Odd/Even parity select */
};


enum ay31015_output_pin_t
{
	AY31015_PE = 13,          /* PE   - Pin 13 - Parity error */
	AY31015_FE = 14,          /* FE   - Pin 14 - Framing error */
	AY31015_OR = 15,          /* OR   - Pin 15 - Over-run */
	AY31015_DAV = 19,         /* DAV  - Pin 19 - Data available */
	AY31015_TBMT = 22,        /* TBMT - Pin 22 - Transmit buffer empty */
	AY31015_EOC = 24,         /* EOC  - Pin 24 - End of character */
	AY31015_SO = 25           /* SO   - Pin 25 - Serial output */
};


/***************************************************************************
    DEVICE INTERFACE
***************************************************************************/

enum state_t
{
	IDLE,
	START_BIT,
	PROCESSING,
	PARITY_BIT,
	FIRST_STOP_BIT,
	SECOND_STOP_BIT,
	PREP_TIME
};

ALLOW_SAVE_TYPE(state_t);

class ay31015_device : public device_t
{
public:
	ay31015_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	ay31015_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	~ay31015_device() {}

	static void set_tx_clock(device_t &device, double tx_clock) { downcast<ay31015_device &>(device).m_tx_clock = tx_clock; }
	static void set_rx_clock(device_t &device, double rx_clock) { downcast<ay31015_device &>(device).m_rx_clock = rx_clock; }
	template<class _Object> static devcb_base &set_read_si_callback(device_t &device, _Object object) { return downcast<ay31015_device &>(device).m_read_si_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_write_so_callback(device_t &device, _Object object) { return downcast<ay31015_device &>(device).m_write_so_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_status_changed_callback(device_t &device, _Object object) { return downcast<ay31015_device &>(device).m_status_changed_cb.set_callback(object); }

	/* Set an input pin */
	void set_input_pin( ay31015_input_pin_t pin, int data );


	/* Get an output pin */
	int get_output_pin( ay31015_output_pin_t pin );


	/* Set a new transmitter clock (new_clock is in Hz) */
	void set_transmitter_clock( double new_clock );


	/* Set a new receiver clock (new_clock is in Hz) */
	void set_receiver_clock( double new_clock );


	/* Reead the received data */
	/* The received data is available on RD8-RD1 (pins 5-12) */
	UINT8 get_received_data();


	/* Set the transmitter buffer */
	/* The data to transmit is set on DB1-DB8 (pins 26-33) */
	void set_transmit_data( UINT8 data );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void internal_reset();

	// internal state
	inline UINT8 get_si();
	inline void set_so(int data);
	inline int update_status_pin(UINT8 reg_bit, ay31015_output_pin_t pin);
	void update_status_pins();
	void transfer_control_pins();
	inline void update_rx_timer();
	inline void update_tx_timer();
	TIMER_CALLBACK_MEMBER(rx_process);
	TIMER_CALLBACK_MEMBER(tx_process);

	int m_pins[41];

	UINT8 m_control_reg;
	UINT8 m_status_reg;
	UINT16 m_second_stop_bit; // 0, 8, 16
	UINT16 m_total_pulses;    // bits * 16
	UINT8 m_internal_sample;

	state_t m_rx_state;
	UINT8 m_rx_data;      // byte being received
	UINT8 m_rx_buffer;    // received byte waiting to be accepted by computer
	UINT8 m_rx_bit_count;
	UINT8 m_rx_parity;
	UINT16 m_rx_pulses;   // total pulses left
	double m_rx_clock;    /* RCP - pin 17 */
	emu_timer *m_rx_timer;

	state_t m_tx_state;
	UINT8 m_tx_data;      // byte being sent
	UINT8 m_tx_buffer;    // next byte to send
	UINT8 m_tx_parity;
	UINT16 m_tx_pulses;   // total pulses left
	double m_tx_clock;    /* TCP - pin 40 */
	emu_timer *m_tx_timer;

	devcb_read8 m_read_si_cb;                 /* SI - pin 20 - This will be called whenever the SI pin is sampled. Optional */
	devcb_write8 m_write_so_cb;                /* SO - pin 25 - This will be called whenever data is put on the SO pin. Optional */
	devcb_write8 m_status_changed_cb;          /* This will be called whenever one of the status pins may have changed. Optional */
};

class ay51013_device : public ay31015_device
{
public:
	ay51013_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual void internal_reset() override;

};

extern const device_type AY31015;   // For AY-3-1014A, AY-3-1015(D) and HD6402 variants
extern const device_type AY51013;   // For AY-3-1014, AY-5-1013 and AY-6-1013 variants



/***************************************************************************
 DEVICE CONFIGURATION MACROS
 ***************************************************************************/


#define MCFG_AY31015_TX_CLOCK(_txclk) \
	ay31015_device::set_tx_clock(*device, _txclk);

#define MCFG_AY31015_RX_CLOCK(_rxclk) \
	ay31015_device::set_rx_clock(*device, _rxclk);

#define MCFG_AY31015_READ_SI_CB(_devcb) \
	devcb = &ay31015_device::set_read_si_callback(*device, DEVCB_##_devcb);

#define MCFG_AY31015_WRITE_SO_CB(_devcb) \
	devcb = &ay31015_device::set_write_so_callback(*device, DEVCB_##_devcb);

#define MCFG_AY31015_STATUS_CHANGED_CB(_devcb) \
	devcb = &ay31015_device::set_status_changed_callback(*device, DEVCB_##_devcb);


#define MCFG_AY51013_TX_CLOCK(_txclk) \
	ay51013_device::set_tx_clock(*device, _txclk);

#define MCFG_AY51013_RX_CLOCK(_rxclk) \
	ay51013_device::set_rx_clock(*device, _rxclk);

#define MCFG_AY51013_READ_SI_CB(_devcb) \
	devcb = &ay51013_device::set_read_si_callback(*device, DEVCB_##_devcb);

#define MCFG_AY51013_WRITE_SO_CB(_devcb) \
	devcb = &ay51013_device::set_write_so_callback(*device, DEVCB_##_devcb);

#define MCFG_AY51013_STATUS_CHANGED_CB(_devcb) \
	devcb = &ay51013_device::set_status_changed_callback(*device, DEVCB_##_devcb);

#endif
