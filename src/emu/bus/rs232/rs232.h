#ifndef __BUS_RS232_H__
#define __BUS_RS232_H__

#include "emu.h"

#define MCFG_RS232_PORT_ADD(_tag, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, RS232_PORT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)

#define MCFG_RS232_RXD_HANDLER(_devcb) \
	devcb = &rs232_port_device::set_rxd_handler(*device, DEVCB2_##_devcb);

#define MCFG_RS232_DCD_HANDLER(_devcb) \
	devcb = &rs232_port_device::set_dcd_handler(*device, DEVCB2_##_devcb);

#define MCFG_RS232_DSR_HANDLER(_devcb) \
	devcb = &rs232_port_device::set_dsr_handler(*device, DEVCB2_##_devcb);

#define MCFG_RS232_RI_HANDLER(_devcb) \
	devcb = &rs232_port_device::set_ri_handler(*device, DEVCB2_##_devcb);

#define MCFG_RS232_CTS_HANDLER(_devcb) \
	devcb = &rs232_port_device::set_cts_handler(*device, DEVCB2_##_devcb);

class device_rs232_port_interface;

class rs232_port_device : public device_t,
	public device_slot_interface
{
	friend class device_rs232_port_interface;

public:
	rs232_port_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	rs232_port_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	virtual ~rs232_port_device();

	// static configuration helpers
	template<class _Object> static devcb2_base &set_rxd_handler(device_t &device, _Object object) { return downcast<rs232_port_device &>(device).m_rxd_handler.set_callback(object); }
	template<class _Object> static devcb2_base &set_dcd_handler(device_t &device, _Object object) { return downcast<rs232_port_device &>(device).m_dcd_handler.set_callback(object); }
	template<class _Object> static devcb2_base &set_dsr_handler(device_t &device, _Object object) { return downcast<rs232_port_device &>(device).m_dsr_handler.set_callback(object); }
	template<class _Object> static devcb2_base &set_ri_handler(device_t &device, _Object object) { return downcast<rs232_port_device &>(device).m_ri_handler.set_callback(object); }
	template<class _Object> static devcb2_base &set_cts_handler(device_t &device, _Object object) { return downcast<rs232_port_device &>(device).m_cts_handler.set_callback(object); }

	DECLARE_WRITE_LINE_MEMBER( write_txd );
	DECLARE_WRITE_LINE_MEMBER( write_dtr );
	DECLARE_WRITE_LINE_MEMBER( write_rts );
	DECLARE_WRITE_LINE_MEMBER( write_etc );

	DECLARE_READ_LINE_MEMBER( rxd_r ) { return m_rxd; }
	DECLARE_READ_LINE_MEMBER( dcd_r ) { return m_dcd; }
	DECLARE_READ_LINE_MEMBER( dsr_r ) { return m_dsr; }
	DECLARE_READ_LINE_MEMBER( ri_r )  { return m_ri; }
	DECLARE_READ_LINE_MEMBER( cts_r ) { return m_cts; }

protected:
	virtual void device_start();
	virtual void device_config_complete();

	int m_rxd;
	int m_dcd;
	int m_dsr;
	int m_ri;
	int m_cts;

	devcb2_write_line m_rxd_handler;
	devcb2_write_line m_dcd_handler;
	devcb2_write_line m_dsr_handler;
	devcb2_write_line m_ri_handler;
	devcb2_write_line m_cts_handler;

private:
	device_rs232_port_interface *m_dev;
};

class device_rs232_port_interface : public device_slot_card_interface
{
	friend class rs232_port_device;

public:
	device_rs232_port_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_rs232_port_interface();

	virtual DECLARE_WRITE_LINE_MEMBER( input_txd ) {}
	virtual DECLARE_WRITE_LINE_MEMBER( input_dtr ) {}
	virtual DECLARE_WRITE_LINE_MEMBER( input_rts ) {}
	virtual DECLARE_WRITE_LINE_MEMBER( input_etc ) {}

	DECLARE_WRITE_LINE_MEMBER( output_rxd ) { m_port->m_rxd = state; m_port->m_rxd_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( output_dcd ) { m_port->m_dcd = state; m_port->m_dcd_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( output_dsr ) { m_port->m_dsr = state; m_port->m_dsr_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( output_ri )  { m_port->m_ri = state; m_port->m_ri_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( output_cts ) { m_port->m_cts = state; m_port->m_cts_handler(state); }

protected:
	rs232_port_device *m_port;
};

extern const device_type RS232_PORT;

SLOT_INTERFACE_EXTERN( default_rs232_devices );

#endif
