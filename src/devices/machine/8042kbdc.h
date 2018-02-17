// license:BSD-3-Clause
// copyright-holders:Peter Trauner
/**********************************************************************

    8042 Keyboard Controller Emulation

    This is the keyboard controller used in the IBM AT and further
    models.  It is a popular controller for PC style keyboards

**********************************************************************/

#ifndef MAME_MACHINE_8042KBDC_H
#define MAME_MACHINE_8042KBDC_H

#pragma once

#include "machine/pckeybrd.h"

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_KBDC8042_KEYBOARD_TYPE(_kbdt) \
	kbdc8042_device::set_keyboard_type(*device, kbdc8042_device::_kbdt);

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
	enum kbdc8042_type_t
	{
		KBDC8042_STANDARD,
		KBDC8042_PS2,       /* another timing of integrated controller */
		KBDC8042_AT386      /* hack for at386 driver */
	};

	// construction/destruction
	kbdc8042_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static void set_keyboard_type(device_t &device, kbdc8042_type_t keybtype) { downcast<kbdc8042_device &>(device).m_keybtype = keybtype; }
	template <class Object> static devcb_base &set_system_reset_callback(device_t &device, Object &&cb) { return downcast<kbdc8042_device &>(device).m_system_reset_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_gate_a20_callback(device_t &device, Object &&cb) { return downcast<kbdc8042_device &>(device).m_gate_a20_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_input_buffer_full_callback(device_t &device, Object &&cb) { return downcast<kbdc8042_device &>(device).m_input_buffer_full_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_output_buffer_empty_callback(device_t &device, Object &&cb) { return downcast<kbdc8042_device &>(device).m_output_buffer_empty_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_speaker_callback(device_t &device, Object &&cb) { return downcast<kbdc8042_device &>(device).m_speaker_cb.set_callback(std::forward<Object>(cb)); }

	DECLARE_READ8_MEMBER( data_r );
	DECLARE_WRITE8_MEMBER( data_w );

	DECLARE_WRITE_LINE_MEMBER( write_out2 );

	void at_8042_set_outport(uint8_t data, int initial);
	void at_8042_receive(uint8_t data);
	void at_8042_check_keyboard();
	void at_8042_clear_keyboard_received();

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	uint8_t m_inport, m_outport, m_data, m_command;

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

	DECLARE_WRITE_LINE_MEMBER( keyboard_w );
};

// device type definition
DECLARE_DEVICE_TYPE(KBDC8042, kbdc8042_device)

#endif // MAME_MACHINE_8042KBDC_H
