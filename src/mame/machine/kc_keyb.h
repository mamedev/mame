// license:GPL-2.0+
// copyright-holders:Kevin Thacker,Sandro Ronco
/*********************************************************************

    kc_keyb.h

*********************************************************************/

#ifndef __KC_KEYB_H__
#define __KC_KEYB_H__

#define MCFG_KC_KEYBOARD_OUT_CALLBACK(_write) \
	devcb = &kc_keyboard_device::set_out_wr_callback(*device, DEVCB_##_write);

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* number of pulses that can be stored */
#define KC_TRANSMIT_BUFFER_LENGTH 256

// ======================> kc_keyboard_device

class kc_keyboard_device : public device_t
{
public:
	// construction/destruction
	kc_keyboard_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	virtual ~kc_keyboard_device();

	template<class _Object> static devcb_base &set_out_wr_callback(device_t &device, _Object object) { return downcast<kc_keyboard_device &>(device).m_write_out.set_callback(object); }

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	void add_pulse_to_transmit_buffer(int pulse_state, int pulse_number = 1);
	void add_bit(int bit);
	void transmit_scancode(UINT8 scan_code);

private:
	static const device_timer_id TIMER_TRANSMIT_PULSE = 1;

	// internal state
	emu_timer *                 m_timer_transmit_pulse;
	devcb_write_line   m_write_out;

	// pulses to transmit
	struct
	{
		UINT8 data[KC_TRANSMIT_BUFFER_LENGTH>>3];
		int pulse_sent;
		int pulse_count;
	} m_transmit_buffer;
};

// device type definition
extern const device_type KC_KEYBOARD;

#endif /* __KC_KEYB_H__ */
