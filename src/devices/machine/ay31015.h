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
	ay31015_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void set_auto_rdav(bool auto_rdav) { m_auto_rdav = auto_rdav; }

	auto read_si_callback() { return m_read_si_cb.bind(); }
	auto write_so_callback() { return m_write_so_cb.bind(); }
	auto write_pe_callback() { return m_write_pe_cb.bind(); }
	auto write_fe_callback() { return m_write_fe_cb.bind(); }
	auto write_or_callback() { return m_write_or_cb.bind(); }
	auto write_dav_callback() { return m_write_dav_cb.bind(); }
	auto write_tbmt_callback() { return m_write_tbmt_cb.bind(); }
	auto write_eoc_callback() { return m_write_eoc_cb.bind(); }

	/* Set an input pin */
	void write_swe(int state) { set_input_pin(SWE, state); }
	void write_rcp(int state) { set_input_pin(RCP, state); }
	void write_rdav(int state) { set_input_pin(RDAV, state); }
	void write_si(int state) { set_input_pin(SI, state); }
	void write_xr(int state) { set_input_pin(XR, state); }
	void write_cs(int state) { set_input_pin(CS, state); }
	void write_np(int state) { set_input_pin(NP, state); }
	void write_tsb(int state) { set_input_pin(TSB, state); }
	void write_nb2(int state) { set_input_pin(NB2, state); }
	void write_nb1(int state) { set_input_pin(NB1, state); }
	void write_eps(int state) { set_input_pin(EPS, state); }
	void write_tcp(int state) { set_input_pin(TCP, state); }

	/* Get an output pin */
	int pe_r() { return get_output_pin(PE); }
	int fe_r() { return get_output_pin(FE); }
	int or_r() { return get_output_pin(OR); }
	int dav_r() { return get_output_pin(DAV); }
	int tbmt_r() { return get_output_pin(TBMT); }
	int eoc_r() { return get_output_pin(EOC); }
	int so_r() { return get_output_pin(SO); }

	/* Read the received data */
	/* The received data is available on RD8-RD1 (pins 5-12) */
	uint8_t receive();

	/* Set the transmitter buffer */
	/* The data to transmit is set on DB1-DB8 (pins 26-33) */
	void transmit( uint8_t data );

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

	ay31015_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

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

	state_t m_tx_state;
	uint8_t m_tx_data;      // byte being sent
	uint8_t m_tx_buffer;    // next byte to send
	uint8_t m_tx_parity;
	uint16_t m_tx_pulses;   // total pulses left

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
	ay51013_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	virtual void internal_reset() override;

};

ALLOW_SAVE_TYPE(ay31015_device::state_t);

DECLARE_DEVICE_TYPE(AY31015, ay31015_device)   // For AY-3-1014A, AY-3-1015(D) and HD6402 variants
DECLARE_DEVICE_TYPE(AY51013, ay51013_device)   // For AY-3-1014, AY-5-1013 and AY-6-1013 variants

#endif // MAME_MACHINE_AY31015_H
