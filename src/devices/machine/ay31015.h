// license:BSD-3-Clause
// copyright-holders:Robbbert
/* ay31015.h

    Written for MESS by Robbbert on May 29th, 2008.

*/

#ifndef MAME_MACHINE_AY31015_H
#define MAME_MACHINE_AY31015_H

#pragma once

/***************************************************************************
    DEVICE INTERFACE
***************************************************************************/

class ay31015_device : public device_t
{
public:
	ay31015_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_tx_clock(double tx_clock) { m_tx_clock = tx_clock; }
	void set_tx_clock(const XTAL &xtal) { set_tx_clock(xtal.dvalue()); }
	void set_rx_clock(double rx_clock) { m_rx_clock = rx_clock; }
	void set_rx_clock(const XTAL &xtal) { set_rx_clock(xtal.dvalue()); }
	void set_auto_rdav(bool auto_rdav) { m_auto_rdav = auto_rdav; }
	template <class Object> devcb_base &set_read_si_callback(Object &&cb) { return m_read_si_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_write_so_callback(Object &&cb) { return m_write_so_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_write_pe_callback(Object &&cb) { return m_write_pe_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_write_fe_callback(Object &&cb) { return m_write_fe_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_write_or_callback(Object &&cb) { return m_write_or_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_write_dav_callback(Object &&cb) { return m_write_dav_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_write_tbmt_callback(Object &&cb) { return m_write_tbmt_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_write_eoc_callback(Object &&cb) { return m_write_eoc_cb.set_callback(std::forward<Object>(cb)); }

	/* Set an input pin */
	DECLARE_WRITE_LINE_MEMBER(write_swe) { set_input_pin(SWE, state); }
	DECLARE_WRITE_LINE_MEMBER(write_rcp) { set_input_pin(RCP, state); }
	DECLARE_WRITE_LINE_MEMBER(write_rdav) { set_input_pin(RDAV, state); }
	DECLARE_WRITE_LINE_MEMBER(write_si) { set_input_pin(SI, state); }
	DECLARE_WRITE_LINE_MEMBER(write_xr) { set_input_pin(XR, state); }
	DECLARE_WRITE_LINE_MEMBER(write_cs) { set_input_pin(CS, state); }
	DECLARE_WRITE_LINE_MEMBER(write_np) { set_input_pin(NP, state); }
	DECLARE_WRITE_LINE_MEMBER(write_tsb) { set_input_pin(TSB, state); }
	DECLARE_WRITE_LINE_MEMBER(write_nb2) { set_input_pin(NB2, state); }
	DECLARE_WRITE_LINE_MEMBER(write_nb1) { set_input_pin(NB1, state); }
	DECLARE_WRITE_LINE_MEMBER(write_eps) { set_input_pin(EPS, state); }
	DECLARE_WRITE_LINE_MEMBER(write_tcp) { set_input_pin(TCP, state); }

	/* Get an output pin */
	DECLARE_READ_LINE_MEMBER(pe_r) { return get_output_pin(PE); }
	DECLARE_READ_LINE_MEMBER(fe_r) { return get_output_pin(FE); }
	DECLARE_READ_LINE_MEMBER(or_r) { return get_output_pin(OR); }
	DECLARE_READ_LINE_MEMBER(dav_r) { return get_output_pin(DAV); }
	DECLARE_READ_LINE_MEMBER(tbmt_r) { return get_output_pin(TBMT); }
	DECLARE_READ_LINE_MEMBER(eoc_r) { return get_output_pin(EOC); }
	DECLARE_READ_LINE_MEMBER(so_r) { return get_output_pin(SO); }

	/* Set a new transmitter clock (new_clock is in Hz) */
	void set_transmitter_clock( double new_clock );

	/* Set a new receiver clock (new_clock is in Hz) */
	void set_receiver_clock( double new_clock );

	/* Reead the received data */
	/* The received data is available on RD8-RD1 (pins 5-12) */
	uint8_t get_received_data();

	/* Set the transmitter buffer */
	/* The data to transmit is set on DB1-DB8 (pins 26-33) */
	void set_transmit_data( uint8_t data );

	DECLARE_READ8_MEMBER(receive);
	DECLARE_WRITE8_MEMBER(transmit);

protected:
	enum input_pin
	{
		SWE = 16,       // -SWE  - Pin 16 - Status word enable
		RCP = 17,       //  RCP  - Pin 17 - Receiver clock pulse
		RDAV = 18,      // -RDAV - Pin 18 - Reset data available
		SI = 20,        //  SI   - Pin 20 - Serial input
		XR = 21,        //  XR   - Pin 21 - External reset
		CS = 34,        //  CS   - Pin 34 - Control strobe
		NP = 35,        //  NP   - Pin 35 - No parity
		TSB = 36,       //  TSB  - Pin 36 - Number of stop bits
		NB2 = 37,       //  NB2  - Pin 37 - Number of bits #2
		NB1 = 38,       //  NB1  - Pin 38 - Number of bits #1
		EPS = 39,       //  EPS  - Pin 39 - Odd/Even parity select
		TCP = 40        //  TCP  - Pin 40 - Transmitter clock pulse
	};

	enum output_pin
	{
		PE = 13,        // PE   - Pin 13 - Parity error
		FE = 14,        // FE   - Pin 14 - Framing error
		OR = 15,        // OR   - Pin 15 - Over-run
		DAV = 19,       // DAV  - Pin 19 - Data available
		TBMT = 22,      // TBMT - Pin 22 - Transmit buffer empty
		EOC = 24,       // EOC  - Pin 24 - End of character
		SO = 25         // SO   - Pin 25 - Serial output
	};

	enum state_t : u8
	{
		IDLE,
		START_BIT,
		PROCESSING,
		PARITY_BIT,
		FIRST_STOP_BIT,
		SECOND_STOP_BIT,
		PREP_TIME
	};

	static constexpr device_timer_id TIMER_RX = 0;
	static constexpr device_timer_id TIMER_TX = 1;

	ay31015_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_resolve_objects() override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	void rx_process();
	void tx_process();
	virtual void internal_reset();

	// internal state
	inline uint8_t get_si();
	inline void set_so(int data);
	inline void update_status_pin(uint8_t reg_bit, output_pin pin, devcb_write_line &write_cb);
	void update_status_pins();
	void transfer_control_pins();
	void set_input_pin(input_pin pin, int data);
	int get_output_pin(output_pin pin);
	inline void update_rx_timer();
	inline void update_tx_timer();

	int m_pins[41];

	uint8_t m_control_reg;
	uint8_t m_status_reg;
	uint16_t m_second_stop_bit; // 0, 8, 16
	uint16_t m_total_pulses;    // bits * 16
	uint8_t m_internal_sample;

	state_t m_rx_state;
	uint8_t m_rx_data;      // byte being received
	uint8_t m_rx_buffer;    // received byte waiting to be accepted by computer
	uint8_t m_rx_bit_count;
	uint8_t m_rx_parity;
	uint16_t m_rx_pulses;   // total pulses left
	double m_rx_clock;    /* RCP - pin 17 */
	emu_timer *m_rx_timer;

	state_t m_tx_state;
	uint8_t m_tx_data;      // byte being sent
	uint8_t m_tx_buffer;    // next byte to send
	uint8_t m_tx_parity;
	uint16_t m_tx_pulses;   // total pulses left
	double m_tx_clock;    /* TCP - pin 40 */
	emu_timer *m_tx_timer;

	devcb_read_line m_read_si_cb;           // SI - pin 20 - This will be called whenever the SI pin is sampled. Optional
	devcb_write_line m_write_so_cb;         // SO - pin 25 - This will be called whenever data is put on the SO pin. Optional
	devcb_write_line m_write_pe_cb;         // PE - pin 13 - This will be called whenever the PE pin may have changed. Optional
	devcb_write_line m_write_fe_cb;         // FE - pin 14 - This will be called whenever the FE pin may have changed. Optional
	devcb_write_line m_write_or_cb;         // OR - pin 15 - This will be called whenever the OR pin may have changed. Optional
	devcb_write_line m_write_dav_cb;        // DAV - pin 19 - This will be called whenever the DAV pin may have changed. Optional
	devcb_write_line m_write_tbmt_cb;       // TBMT - pin 22 - This will be called whenever the TBMT pin may have changed. Optional
	devcb_write_line m_write_eoc_cb;        // EOC - pin 24 - This will be called whenever the EOC pin may have changed. Optional

	bool m_auto_rdav;                       // true if RDAV (pin 18) is tied to RDE (pin 4)
};

class ay51013_device : public ay31015_device
{
public:
	ay51013_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void internal_reset() override;

};

ALLOW_SAVE_TYPE(ay31015_device::state_t);

DECLARE_DEVICE_TYPE(AY31015, ay31015_device)   // For AY-3-1014A, AY-3-1015(D) and HD6402 variants
DECLARE_DEVICE_TYPE(AY51013, ay51013_device)   // For AY-3-1014, AY-5-1013 and AY-6-1013 variants



/***************************************************************************
 DEVICE CONFIGURATION MACROS
 ***************************************************************************/


#define MCFG_AY31015_TX_CLOCK(_txclk) \
	downcast<ay31015_device &>(*device).set_tx_clock(_txclk);

#define MCFG_AY31015_RX_CLOCK(_rxclk) \
	downcast<ay31015_device &>(*device).set_rx_clock(_rxclk);

#define MCFG_AY31015_AUTO_RDAV(_auto_rdav) \
	downcast<ay31015_device &>(*device).set_auto_rdav(_auto_rdav);

#define MCFG_AY31015_READ_SI_CB(_devcb) \
	devcb = &downcast<ay31015_device &>(*device).set_read_si_callback(DEVCB_##_devcb);

#define MCFG_AY31015_WRITE_SO_CB(_devcb) \
	devcb = &downcast<ay31015_device &>(*device).set_write_so_callback(DEVCB_##_devcb);

#define MCFG_AY31015_WRITE_PE_CB(_devcb) \
	devcb = &downcast<ay31015_device &>(*device).set_write_pe_callback(DEVCB_##_devcb);

#define MCFG_AY31015_WRITE_FE_CB(_devcb) \
	devcb = &downcast<ay31015_device &>(*device).set_write_fe_callback(DEVCB_##_devcb);

#define MCFG_AY31015_WRITE_OR_CB(_devcb) \
	devcb = &downcast<ay31015_device &>(*device).set_write_or_callback(DEVCB_##_devcb);

#define MCFG_AY31015_WRITE_DAV_CB(_devcb) \
	devcb = &downcast<ay31015_device &>(*device).set_write_dav_callback(DEVCB_##_devcb);

#define MCFG_AY31015_WRITE_TBMT_CB(_devcb) \
	devcb = &downcast<ay31015_device &>(*device).set_write_tbmt_callback(DEVCB_##_devcb);

#define MCFG_AY31015_WRITE_EOC_CB(_devcb) \
	devcb = &downcast<ay31015_device &>(*device).set_write_eoc_callback(DEVCB_##_devcb);


#define MCFG_AY51013_TX_CLOCK(_txclk) \
	downcast<ay51013_device &>(*device).set_tx_clock(_txclk);

#define MCFG_AY51013_RX_CLOCK(_rxclk) \
	downcast<ay51013_device &>(*device).set_rx_clock(_rxclk);

#define MCFG_AY51013_AUTO_RDAV(_auto_rdav) \
	downcast<ay51013_device &>(*device).set_auto_rdav(_auto_rdav);

#define MCFG_AY51013_READ_SI_CB(_devcb) \
	devcb = &downcast<ay51013_device &>(*device).set_read_si_callback(DEVCB_##_devcb);

#define MCFG_AY51013_WRITE_SO_CB(_devcb) \
	devcb = &downcast<ay51013_device &>(*device).set_write_so_callback(DEVCB_##_devcb);

#define MCFG_AY51013_WRITE_PE_CB(_devcb) \
	devcb = &downcast<ay51013_device &>(*device).set_write_pe_callback(DEVCB_##_devcb);

#define MCFG_AY51013_WRITE_FE_CB(_devcb) \
	devcb = &downcast<ay51013_device &>(*device).set_write_fe_callback(DEVCB_##_devcb);

#define MCFG_AY51013_WRITE_OR_CB(_devcb) \
	devcb = &downcast<ay51013_device &>(*device).set_write_or_callback(DEVCB_##_devcb);

#define MCFG_AY51013_WRITE_DAV_CB(_devcb) \
	devcb = &downcast<ay51013_device &>(*device).set_write_dav_callback(DEVCB_##_devcb);

#define MCFG_AY51013_WRITE_TBMT_CB(_devcb) \
	devcb = &downcast<ay51013_device &>(*device).set_write_tbmt_callback(DEVCB_##_devcb);

#define MCFG_AY51013_WRITE_EOC_CB(_devcb) \
	devcb = &downcast<ay51013_device &>(*device).set_write_eoc_callback(DEVCB_##_devcb);

#endif // MAME_MACHINE_AY31015_H
