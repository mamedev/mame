/***************************************************************************

    ay3600.h

    Include file for AY-3600 keyboard
								   
***************************************************************************/

#ifndef AY3600_H
#define AY3600_H

// keymod flags returned by AY3600_keymod_r()
#define AY3600_KEYMOD_SHIFT     1
#define AY3600_KEYMOD_CONTROL   2
#define AY3600_KEYMOD_CAPSLOCK  4
#define AY3600_KEYMOD_REPEAT    8
#define AY3600_KEYMOD_KEYPAD    0x10
#define AY3600_KEYMOD_MODLATCH  0x20
#define AY3600_KEYMOD_COMMAND   0x40
#define AY3600_KEYMOD_OPTION    0x80

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ay3600n_device

class ay3600n_device :  public device_t
{
public:
	// construction/destruction
	ay3600n_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// interface routines
	int keydata_strobe_r();
	int anykey_clearstrobe_r();
	int keymod_r();

	// config
	void set_runtime_config(bool keypad, bool repeat);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
	UINT8 AY3600_get_keycode(unicode_char ch);
	int AY3600_keyboard_queue_chars(const unicode_char *text, size_t text_len);
	bool AY3600_keyboard_accept_char(unicode_char ch);
	bool AY3600_keyboard_charqueue_empty();

	bool m_keypad, m_repeat;
	unsigned int *m_ay3600_keys;
	UINT8 m_keycode;
	UINT8 m_keycode_unmodified;
	UINT8 m_keywaiting;
	UINT8 m_keystilldown;
	UINT8 m_keymodreg;
	int m_last_key;
	int m_last_key_unmodified;
	unsigned int m_time_until_repeat;
	emu_timer *m_timer;
};

// device type definition
extern const device_type AY3600N;

#endif /* AY3600_H */
