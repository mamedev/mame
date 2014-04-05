#ifndef AMIGAKBD_H
#define AMIGAKBD_H


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_AMIGA_KEYBOARD_KCLK_CALLBACK(_write) \
	devcb = &amigakbd_device::set_kclk_wr_callback(*device, DEVCB2_##_write);

#define MCFG_AMIGA_KEYBOARD_KDAT_CALLBACK(_write) \
	devcb = &amigakbd_device::set_kdat_wr_callback(*device, DEVCB2_##_write);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> amigakbd_device

class amigakbd_device :  public device_t
{
public:
	// construction/destruction
	amigakbd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb2_base &set_kclk_wr_callback(device_t &device, _Object object) { return downcast<amigakbd_device &>(device).m_write_kclk.set_callback(object); }
	template<class _Object> static devcb2_base &set_kdat_wr_callback(device_t &device, _Object object) { return downcast<amigakbd_device &>(device).m_write_kdat.set_callback(object); }

	// optional information overrides
	virtual ioport_constructor device_input_ports() const;

	DECLARE_INPUT_CHANGED_MEMBER( kbd_update );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	void kbd_sendscancode(UINT8 scancode );

private:
	devcb2_write_line m_write_kclk;
	devcb2_write_line m_write_kdat;

	UINT8 *m_buf;
	int m_buf_pos;
	int m_cur_pos;
	emu_timer *m_timer;
};

// device type definition
extern const device_type AMIGAKBD;
#endif /* AMIGAKBD_H */
