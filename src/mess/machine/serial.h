#ifndef __SERIAL_H__
#define __SERIAL_H__

#include "emu.h"

#define MCFG_SERIAL_PORT_ADD(_tag, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, SERIAL_PORT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)

#define MCFG_RS232_PORT_ADD(_tag, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, RS232_PORT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)

#define MCFG_SERIAL_OUT_RX_HANDLER(_devcb) \
	devcb = &serial_port_device::set_out_rx_handler(*device, DEVCB2_##_devcb);

class device_serial_port_interface : public device_slot_card_interface
{
public:
	device_serial_port_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_serial_port_interface();

	virtual void tx(UINT8 state) { m_tbit = state; }
	virtual UINT8 rx() { return m_rbit; }
protected:
	UINT8 m_rbit;
	UINT8 m_tbit;
};

class serial_port_device : public device_t,
	public device_slot_interface
{
public:
	serial_port_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	serial_port_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	virtual ~serial_port_device();

	// static configuration helpers
	template<class _Object> static devcb2_base &set_out_rx_handler(device_t &device, _Object object) { return downcast<serial_port_device &>(device).m_out_rx_handler.set_callback(object); }

	DECLARE_WRITE_LINE_MEMBER( tx ) { if(m_dev) m_dev->tx(state); }
	DECLARE_READ_LINE_MEMBER( rx )  { return (m_dev) ? m_dev->rx() : 1; }

	void out_rx(UINT8 param)  { m_out_rx_handler(param); }

protected:
	virtual void device_start();
	virtual void device_config_complete();
	device_serial_port_interface *m_dev;

private:
	devcb2_write_line m_out_rx_handler;
};

extern const device_type SERIAL_PORT;


class device_rs232_port_interface : public device_serial_port_interface
{
public:
	device_rs232_port_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_rs232_port_interface();

	virtual void dtr_w(UINT8 state) { m_dtr = state; }
	virtual void rts_w(UINT8 state) { m_rts = state; }

	virtual UINT8 dcd_r() { return m_dcd; }
	virtual UINT8 dsr_r() { return m_dsr; }
	virtual UINT8 ri_r()  { return m_ri; }
	virtual UINT8 cts_r() { return m_cts; }

protected:
	UINT8 m_dtr;
	UINT8 m_rts;
	UINT8 m_dcd;
	UINT8 m_dsr;
	UINT8 m_ri;
	UINT8 m_cts;
};


#define MCFG_RS232_OUT_DCD_HANDLER(_devcb) \
	devcb = &rs232_port_device::set_out_dcd_handler(*device, DEVCB2_##_devcb);

#define MCFG_RS232_OUT_DSR_HANDLER(_devcb) \
	devcb = &rs232_port_device::set_out_dsr_handler(*device, DEVCB2_##_devcb);

#define MCFG_RS232_OUT_RI_HANDLER(_devcb) \
	devcb = &rs232_port_device::set_out_ri_handler(*device, DEVCB2_##_devcb);

#define MCFG_RS232_OUT_CTS_HANDLER(_devcb) \
	devcb = &rs232_port_device::set_out_cts_handler(*device, DEVCB2_##_devcb);


class rs232_port_device : public serial_port_device
{
public:
	rs232_port_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~rs232_port_device();

	// static configuration helpers
	template<class _Object> static devcb2_base &set_out_dcd_handler(device_t &device, _Object object) { return downcast<rs232_port_device &>(device).m_out_dcd_handler.set_callback(object); }
	template<class _Object> static devcb2_base &set_out_dsr_handler(device_t &device, _Object object) { return downcast<rs232_port_device &>(device).m_out_dsr_handler.set_callback(object); }
	template<class _Object> static devcb2_base &set_out_ri_handler(device_t &device, _Object object) { return downcast<rs232_port_device &>(device).m_out_ri_handler.set_callback(object); }
	template<class _Object> static devcb2_base &set_out_cts_handler(device_t &device, _Object object) { return downcast<rs232_port_device &>(device).m_out_cts_handler.set_callback(object); }

	DECLARE_WRITE_LINE_MEMBER( dtr_w );
	DECLARE_WRITE_LINE_MEMBER( rts_w );

	DECLARE_READ_LINE_MEMBER( dcd_r ) { return (m_dev) ? m_dev->dcd_r() : loopdtr; }
	DECLARE_READ_LINE_MEMBER( dsr_r ) { return (m_dev) ? m_dev->dsr_r() : loopdtr; }
	DECLARE_READ_LINE_MEMBER( ri_r )  { return (m_dev) ? m_dev->ri_r() : 0; }
	DECLARE_READ_LINE_MEMBER( cts_r ) { return (m_dev) ? m_dev->cts_r() : looprts; }

	void out_dcd(UINT8 param) { m_out_dcd_handler(param); }
	void out_dsr(UINT8 param) { m_out_dsr_handler(param); }
	void out_ri(UINT8 param)  { m_out_ri_handler(param); }
	void out_cts(UINT8 param) { m_out_cts_handler(param); }

protected:
	virtual void device_start();
	virtual void device_config_complete();

private:
	device_rs232_port_interface *m_dev;
	devcb2_write_line  m_out_dcd_handler;
	devcb2_write_line  m_out_dsr_handler;
	devcb2_write_line  m_out_ri_handler;
	devcb2_write_line  m_out_cts_handler;
	UINT8 loopdtr;
	UINT8 looprts;
};

extern const device_type RS232_PORT;

SLOT_INTERFACE_EXTERN( default_rs232_devices );

#endif
