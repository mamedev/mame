/*********************************************************************

    6551.h

    MOS Technology 6551 Asynchronous Communications Interface Adapter

*********************************************************************/

#ifndef __6551_H__
#define __6551_H__

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_ACIA6551_ADD(_tag) \
	MCFG_DEVICE_ADD((_tag), ACIA6551, 0)


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> acia6551_device

class acia6551_device :  public device_t,
							public device_serial_interface
{
public:
	// construction/destruction
	acia6551_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	/* read data register */
	DECLARE_READ8_MEMBER(read);

	/* write data register */
	DECLARE_WRITE8_MEMBER(write);

	void receive_character(UINT8 ch);

	void timer_callback();

	virtual void input_callback(UINT8 state);
protected:
	// device-level overrides
	virtual void device_start();

	void refresh_ints();
	void update_data_form();

private:
	UINT8 m_transmit_data_register;
	UINT8 m_receive_data_register;
	UINT8 m_status_register;
	UINT8 m_command_register;
	UINT8 m_control_register;

	/* internal baud rate timer */
	emu_timer   *m_timer;
};

// device type definition
extern const device_type ACIA6551;

#endif /* __6551_H__ */
