/***************************************************************************

    Centronics printer interface

***************************************************************************/

#ifndef __CTRONICS_H__
#define __CTRONICS_H__

#include "imagedev/printer.h"

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/
// ======================> device_centronics_peripheral_interface

class device_centronics_peripheral_interface : public device_slot_card_interface
{
public:
	device_centronics_peripheral_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_centronics_peripheral_interface();
public:
	virtual void write(UINT8 data) { m_data = data; }
	virtual UINT8 read() { return m_data; }

	virtual void strobe_w(UINT8 state) { m_strobe = state; }
	virtual void init_prime_w(UINT8 state) { m_init = state; }
	virtual void autofeed_w(UINT8 state) { m_auto_fd = state; }

	virtual UINT8 ack_r() { return m_ack;}
	virtual UINT8 busy_r(){ return m_busy; }
	virtual UINT8 pe_r()  { return m_pe;}
	virtual UINT8 not_busy_r() { return !m_busy; }
	virtual UINT8 vcc_r() { return TRUE; }
	virtual UINT8 fault_r() { return m_fault; }
	virtual void set_line(int line, int state) { if (state) m_data |= 1 << line; else m_data &= ~(1 << line); }
protected:
	UINT8 m_strobe;
	UINT8 m_busy;
	UINT8 m_ack;
	UINT8 m_auto_fd;
	UINT8 m_pe;
	UINT8 m_fault;
	UINT8 m_init;
	UINT8 m_data;
};

// ======================> centronics_interface
struct centronics_interface
{
	devcb_write_line m_out_ack_cb;
	devcb_write_line m_out_busy_cb;
	devcb_write_line m_out_not_busy_cb;
};

// ======================> centronics_device
class centronics_device :	public device_t,
							public centronics_interface,
							public device_slot_interface
{
public:
	// construction/destruction
	centronics_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~centronics_device();

	DECLARE_WRITE8_MEMBER( write ) { if (m_dev) m_dev->write(data); }
	DECLARE_READ8_MEMBER( read ) {  return (m_dev) ? m_dev->read() : 0x00; }

	/* access to the individual bits */
	DECLARE_WRITE_LINE_MEMBER( d0_w ) { if (m_dev) m_dev->set_line(0, state); }
	DECLARE_WRITE_LINE_MEMBER( d1_w ) { if (m_dev) m_dev->set_line(1, state); }
	DECLARE_WRITE_LINE_MEMBER( d2_w ) { if (m_dev) m_dev->set_line(2, state); }
	DECLARE_WRITE_LINE_MEMBER( d3_w ) { if (m_dev) m_dev->set_line(3, state); }
	DECLARE_WRITE_LINE_MEMBER( d4_w ) { if (m_dev) m_dev->set_line(4, state); }
	DECLARE_WRITE_LINE_MEMBER( d5_w ) { if (m_dev) m_dev->set_line(5, state); }
	DECLARE_WRITE_LINE_MEMBER( d6_w ) { if (m_dev) m_dev->set_line(6, state); }
	DECLARE_WRITE_LINE_MEMBER( d7_w ) { if (m_dev) m_dev->set_line(7, state); }

	DECLARE_WRITE_LINE_MEMBER( strobe_w ) { if (m_dev) m_dev->strobe_w(state); }
	DECLARE_WRITE_LINE_MEMBER( init_prime_w ) { if (m_dev) m_dev->init_prime_w(state); }
	DECLARE_WRITE_LINE_MEMBER( autofeed_w ) { if (m_dev) m_dev->autofeed_w(state); }

	DECLARE_READ_LINE_MEMBER( ack_r ) { return (m_dev) ? m_dev->ack_r() : 0;}
	DECLARE_READ_LINE_MEMBER( busy_r ){ return (m_dev) ? m_dev->busy_r() : 1; }
	DECLARE_READ_LINE_MEMBER( pe_r )  { return (m_dev) ? m_dev->pe_r() : 0;}
	DECLARE_READ_LINE_MEMBER( not_busy_r ) { return (m_dev) ? m_dev->not_busy_r() : 0; }
	DECLARE_READ_LINE_MEMBER( vcc_r ) { return (m_dev) ? m_dev->vcc_r() : 0; }
	DECLARE_READ_LINE_MEMBER( fault_r ) { return (m_dev) ? m_dev->fault_r() : 0; }

	void out_ack(UINT8 param) { m_out_ack_func(param); }
	void out_busy(UINT8 param) { m_out_busy_func(param); }
	void out_not_busy(UINT8 param) { m_out_not_busy_func(param); }

protected:
	// device-level overrides
    virtual void device_config_complete();
	virtual void device_start();
private:
	device_centronics_peripheral_interface *m_dev;

	devcb_resolved_write_line m_out_ack_func;
	devcb_resolved_write_line m_out_busy_func;
	devcb_resolved_write_line m_out_not_busy_func;
};

// device type definition
extern const device_type CENTRONICS;

// ======================> centronics_printer_device

class centronics_printer_device :
		public device_t,
		public device_centronics_peripheral_interface
{
public:
    // construction/destruction
    centronics_printer_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;
	// for printer
	DECLARE_WRITE_LINE_MEMBER(printer_online);

	void ack_callback(UINT8 param);
	void busy_callback(UINT8 param);

	// optional centronics overrides
	virtual void strobe_w(UINT8 state);
	virtual void init_prime_w(UINT8 state);
	virtual UINT8 read() {  return 0x00; }
protected:
    // device-level overrides
    virtual void device_start();
    virtual void device_reset();
	virtual void device_config_complete() { m_shortname = "centronics_printer"; }
private:
	printer_image_device *m_printer;
	centronics_device *m_owner;
};
// device type definition
extern const device_type CENTRONICS_PRINTER;

SLOT_INTERFACE_EXTERN(centronics_printer);

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_CENTRONICS_ADD(_tag, _intf, _slot_intf, _def_slot, _def_inp) \
	MCFG_DEVICE_ADD(_tag, CENTRONICS, 0) \
	MCFG_DEVICE_CONFIG(_intf) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, _def_inp) \

#define MCFG_CENTRONICS_PRINTER_ADD(_tag, _intf) \
	MCFG_CENTRONICS_ADD(_tag, _intf, centronics_printer, "printer", NULL) \


/***************************************************************************
    DEFAULT INTERFACES
***************************************************************************/

extern const centronics_interface standard_centronics;


#endif /* __CTRONICS_H__ */
