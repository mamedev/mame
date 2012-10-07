/***************************************************************************

  pcecommn.h

  Headers for the common stuff for NEC PC Engine/TurboGrafx16.

***************************************************************************/

#ifndef PCECOMMON_H
#define PCECOMMON_H

#define	PCE_MAIN_CLOCK		21477270

class pce_common_state : public driver_device
{
public:
	pce_common_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	DECLARE_WRITE8_MEMBER(pce_joystick_w);
	DECLARE_READ8_MEMBER(pce_joystick_r);

	DECLARE_DRIVER_INIT(pce_common);

	virtual UINT8 joy_read();
private:
	UINT8 m_io_port_options;	/*driver-specific options for the PCE*/
	int m_joystick_port_select; /* internal index of joystick ports */
	int m_joystick_data_select; /* which nibble of joystick data we want */
};
#endif
