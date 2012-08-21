/*********************************************************************

    kc_keyb.h

*********************************************************************/

#ifndef __KC_KEYB_H__
#define __KC_KEYB_H__

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* number of pulses that can be stored */
#define KC_TRANSMIT_BUFFER_LENGTH 256

// ======================> kc_keyb_interface

struct kc_keyb_interface
{
    devcb_write_line				m_keyboard_out_cb;
};

// ======================> kc_keyboard_device

class kc_keyboard_device : public device_t,
						   public kc_keyb_interface
{
public:
	// construction/destruction
	kc_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~kc_keyboard_device();

	// optional information overrides
	virtual ioport_constructor device_input_ports() const;

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_config_complete();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	void add_pulse_to_transmit_buffer(int pulse_state, int pulse_number = 1);
	void add_bit(int bit);
	void transmit_scancode(UINT8 scan_code);

private:
	static const device_timer_id TIMER_TRANSMIT_PULSE = 1;

	// internal state
	emu_timer *					m_timer_transmit_pulse;
	devcb_resolved_write_line	m_keyboard_out_func;

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


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_KC_KEYBOARD_ADD(_tag,_clock,_config) \
	MCFG_DEVICE_ADD(_tag, KC_KEYBOARD, _clock) \
	MCFG_DEVICE_CONFIG(_config) \

#endif /* __KC_KEYB_H__ */
