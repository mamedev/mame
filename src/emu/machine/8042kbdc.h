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

#define MCFG_KBDC8042_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, KBDC8042, 0) \
	MCFG_DEVICE_CONFIG(_interface)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> kbdc8042_interface

struct kbdc8042_interface
{
	kbdc8042_type_t     m_keybtype;
	// interface to the host pc
	devcb_write_line    m_system_reset_cb;
	devcb_write_line    m_gate_a20_cb;
	devcb_write_line    m_input_buffer_full_cb;
	devcb_write_line    m_output_buffer_empty_cb;

	devcb_write8        m_speaker_cb;
};

// ======================> kbdc8042_device

class kbdc8042_device : public device_t,
						public kbdc8042_interface
{
public:
	// construction/destruction
	kbdc8042_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual machine_config_constructor device_mconfig_additions() const;

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
	virtual void device_start();
	virtual void device_reset();
	virtual void device_config_complete();

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

	devcb_resolved_write_line   m_system_reset_func;
	devcb_resolved_write_line   m_gate_a20_func;
	devcb_resolved_write_line   m_input_buffer_full_func;
	devcb_resolved_write_line   m_output_buffer_empty_func;

	devcb_resolved_write8       m_speaker_func;
};

// device type definition
extern const device_type KBDC8042;


#endif /* KBDC8042_H */
