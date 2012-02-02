/***************************************************************************

    Centronics printer interface

***************************************************************************/

#ifndef __CTRONICS_H__
#define __CTRONICS_H__

#include "imagedev/printer.h"

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> centronics_interface
struct centronics_interface
{
	devcb_write_line m_out_ack_cb;
	devcb_write_line m_out_busy_cb;
	devcb_write_line m_out_not_busy_cb;
};

// ======================> centronics_device
class centronics_device :	public device_t,
								public centronics_interface
{
public:
	// construction/destruction
	centronics_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~centronics_device();
	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;
		
	DECLARE_WRITE8_MEMBER( write ) { m_data = data; }
	DECLARE_READ8_MEMBER( read ) { return m_data; }

	/* access to the individual bits */
	DECLARE_WRITE_LINE_MEMBER( d0_w ) { set_line(0, state); }
	DECLARE_WRITE_LINE_MEMBER( d1_w ) { set_line(1, state); }
	DECLARE_WRITE_LINE_MEMBER( d2_w ) { set_line(2, state); }
	DECLARE_WRITE_LINE_MEMBER( d3_w ) { set_line(3, state); }
	DECLARE_WRITE_LINE_MEMBER( d4_w ) { set_line(4, state); }
	DECLARE_WRITE_LINE_MEMBER( d5_w ) { set_line(5, state); }
	DECLARE_WRITE_LINE_MEMBER( d6_w ) { set_line(6, state); }
	DECLARE_WRITE_LINE_MEMBER( d7_w ) { set_line(7, state); }

	DECLARE_WRITE_LINE_MEMBER( strobe_w );
	DECLARE_WRITE_LINE_MEMBER( init_prime_w );
	DECLARE_WRITE_LINE_MEMBER( autofeed_w ) { m_auto_fd = state; }
	
	DECLARE_READ_LINE_MEMBER( ack_r ) { return m_ack; }
	DECLARE_READ_LINE_MEMBER( busy_r ){ return m_busy; }
	DECLARE_READ_LINE_MEMBER( pe_r )  { return m_pe;  }
	DECLARE_READ_LINE_MEMBER( not_busy_r ) { return !m_busy; }
	DECLARE_READ_LINE_MEMBER( vcc_r ) { return TRUE; }
	DECLARE_READ_LINE_MEMBER( fault_r ) { return m_fault; }	
	
	// for printer
	DECLARE_WRITE_LINE_MEMBER(printer_online);

	void ack_callback(UINT8 param);
	void busy_callback(UINT8 param);
protected:
	// device-level overrides
    virtual void device_config_complete();
	virtual void device_start();

	void set_line(int line, int state);
private:
	printer_image_device *m_printer;

	devcb_resolved_write_line m_out_ack_func;
	devcb_resolved_write_line m_out_busy_func;
	devcb_resolved_write_line m_out_not_busy_func;

	UINT8 m_strobe;
	UINT8 m_busy;
	UINT8 m_ack;
	UINT8 m_auto_fd;
	UINT8 m_pe;
	UINT8 m_fault;

	UINT8 m_data;
};

// device type definition
extern const device_type CENTRONICS;

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_CENTRONICS_ADD(_tag, _intf) \
	MCFG_DEVICE_ADD(_tag, CENTRONICS, 0) \
	MCFG_DEVICE_CONFIG(_intf)


/***************************************************************************
    DEFAULT INTERFACES
***************************************************************************/

extern const centronics_interface standard_centronics;


#endif /* __CTRONICS_H__ */
