// license:BSD-3-Clause
// copyright-holders:Peter Trauner
/**********************************************************************

    8042 Keyboard Controller Emulation

    This is the keyboard controller used in the IBM AT and further
    models.  It is a popular controller for PC style keyboards

**********************************************************************/

#ifndef KBDC8042_H
#define KBDC8042_H

#include "emu.h"
#include "machine/pckeybrd.h"

enum kbdc8042_type_t
{
	KBDC8042_STANDARD,
	KBDC8042_PS2,       /* another timing of integrated controller */
	KBDC8042_AT386      /* hack for at386 driver */
};

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_KBDC8042_KEYBOARD_TYPE(_kbdt) \
	kbdc8042_device::set_keyboard_type(*device, _kbdt);

#define MCFG_KBDC8042_SYSTEM_RESET_CB(_devcb) \
	devcb = &kbdc8042_device::set_system_reset_callback(*device, DEVCB_##_devcb);

#define MCFG_KBDC8042_GATE_A20_CB(_devcb) \
	devcb = &kbdc8042_device::set_gate_a20_callback(*device, DEVCB_##_devcb);

#define MCFG_KBDC8042_INPUT_BUFFER_FULL_CB(_devcb) \
	devcb = &kbdc8042_device::set_input_buffer_full_callback(*device, DEVCB_##_devcb);

#define MCFG_KBDC8042_OUTPUT_BUFFER_EMPTY_CB(_devcb) \
	devcb = &kbdc8042_device::set_output_buffer_empty_callback(*device, DEVCB_##_devcb);

#define MCFG_KBDC8042_SPEAKER_CB(_devcb) \
	devcb = &kbdc8042_device::set_speaker_callback(*device, DEVCB_##_devcb);

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> kbdc8042_device

class kbdc8042_device : public device_t
{
public:
	// construction/destruction
	kbdc8042_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual machine_config_constructor device_mconfig_additions() const override;

	static void set_keyboard_type(device_t &device, kbdc8042_type_t keybtype) { downcast<kbdc8042_device &>(device).m_keybtype = keybtype; }
	template<class _Object> static devcb_base &set_system_reset_callback(device_t &device, _Object object) { return downcast<kbdc8042_device &>(device).m_system_reset_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_gate_a20_callback(device_t &device, _Object object) { return downcast<kbdc8042_device &>(device).m_gate_a20_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_input_buffer_full_callback(device_t &device, _Object object) { return downcast<kbdc8042_device &>(device).m_input_buffer_full_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_output_buffer_empty_callback(device_t &device, _Object object) { return downcast<kbdc8042_device &>(device).m_output_buffer_empty_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_speaker_callback(device_t &device, _Object object) { return downcast<kbdc8042_device &>(device).m_speaker_cb.set_callback(object); }

	DECLARE_READ8_MEMBER( data_r );
	DECLARE_WRITE8_MEMBER( data_w );

	DECLARE_WRITE_LINE_MEMBER( write_out2 );
	DECLARE_WRITE_LINE_MEMBER( keyboard_w );

	void at_8042_set_outport(UINT8 data, int initial);
	TIMER_CALLBACK_MEMBER( kbdc8042_clr_int );
	void at_8042_receive(UINT8 data);
	void at_8042_check_keyboard();
	void at_8042_clear_keyboard_received();

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	UINT8 m_inport, m_outport, m_data, m_command;

	struct {
		int received;
		int on;
	} m_keyboard;
	struct {
		int received;
		int on;
	} m_mouse;

	int m_last_write_to_control;
	int m_sending;
	int m_send_to_mouse;

	int m_operation_write_state;
	int m_status_read_mode;

	int m_speaker;
	int m_out2;

	/* temporary hack */
	int m_offset1;

	int m_poll_delay;

	required_device<at_keyboard_device> m_keyboard_dev;

	kbdc8042_type_t     m_keybtype;

	devcb_write_line    m_system_reset_cb;
	devcb_write_line    m_gate_a20_cb;
	devcb_write_line    m_input_buffer_full_cb;
	devcb_write_line    m_output_buffer_empty_cb;

	devcb_write8        m_speaker_cb;
};

// device type definition
extern const device_type KBDC8042;


#endif /* KBDC8042_H */
