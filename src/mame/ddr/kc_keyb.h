// license:GPL-2.0+
// copyright-holders:Kevin Thacker,Sandro Ronco
/*********************************************************************

    kc_keyb.h

*********************************************************************/

#ifndef MAME_DDR_KC_KEYB_H
#define MAME_DDR_KC_KEYB_H

#pragma once


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> kc_keyboard_device

class kc_keyboard_device : public device_t
{
public:
	// construction/destruction
	kc_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~kc_keyboard_device();

	auto out_wr_callback() { return m_write_out.bind(); }

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

protected:
	/* number of pulses that can be stored */
	static constexpr unsigned TRANSMIT_BUFFER_LENGTH = 256;

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(transmit_pulse);

	void add_pulse_to_transmit_buffer(int pulse_state, int pulse_number = 1);
	void add_bit(int bit);
	void transmit_scancode(uint8_t scan_code);

private:
	// internal state
	emu_timer *        m_timer_transmit_pulse;
	devcb_write_line   m_write_out;

	// pulses to transmit
	struct
	{
		uint8_t data[TRANSMIT_BUFFER_LENGTH>>3];
		int pulse_sent;
		int pulse_count;
	} m_transmit_buffer;
};

// device type definition
DECLARE_DEVICE_TYPE(KC_KEYBOARD, kc_keyboard_device)

#endif // MAME_DDR_KC_KEYB_H
