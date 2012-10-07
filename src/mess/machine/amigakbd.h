#ifndef AMIGAKBD_H
#define AMIGAKBD_H


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_AMIGA_KEYBOARD_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, AMIGAKBD, 0)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> amigakbd_device

class amigakbd_device :  public device_t
{
public:
    // construction/destruction
    amigakbd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const;

	DECLARE_INPUT_CHANGED_MEMBER( kbd_update );

protected:
    // device-level overrides
    virtual void device_start();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	void kbd_sendscancode(UINT8 scancode );
private:
	UINT8 *m_buf;
	int m_buf_pos;
	int m_cur_pos;
	emu_timer *m_timer;
};

// device type definition
extern const device_type AMIGAKBD;
#endif /* AMIGAKBD_H */
