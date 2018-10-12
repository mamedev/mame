// license:GPL-2.0+
// copyright-holders:Dirk Best
/**********************************************************************

    EPSON SIO port emulation

**********************************************************************/

#ifndef MAME_BUS_EPSON_SIO_EPSON_SIO_H
#define MAME_BUS_EPSON_SIO_EPSON_SIO_H

#pragma once



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_EPSON_SIO_ADD(_tag, _def_slot) \
	MCFG_DEVICE_ADD(_tag, EPSON_SIO, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(epson_sio_devices, _def_slot, false)

#define MCFG_EPSON_SIO_RX(_rx) \
	downcast<epson_sio_device *>(device)->set_rx_callback(DEVCB_##_rx);

#define MCFG_EPSON_SIO_PIN(_pin) \
	downcast<epson_sio_device *>(device)->set_pin_callback(DEVCB_##_pin);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class device_epson_sio_interface;


class epson_sio_device : public device_t,
							public device_slot_interface
{
public:
	// construction/destruction
	epson_sio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~epson_sio_device();

	// callbacks
	template <class Object> devcb_base &set_rx_callback(Object &&rx) { return m_write_rx.set_callback(std::forward<Object>(rx)); }
	template <class Object> devcb_base &set_pin_callback(Object &&pin) { return m_write_pin.set_callback(std::forward<Object>(pin)); }

	// called from owner
	DECLARE_WRITE_LINE_MEMBER( tx_w );
	DECLARE_WRITE_LINE_MEMBER( pout_w );

	// called from subdevice
	DECLARE_WRITE_LINE_MEMBER( rx_w ) { m_write_rx(state); }
	DECLARE_WRITE_LINE_MEMBER( pin_w ) { m_write_pin(state); }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	device_epson_sio_interface *m_cart;

private:
	devcb_write_line m_write_rx;
	devcb_write_line m_write_pin;
};


// class representing interface-specific live sio device
class device_epson_sio_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	virtual ~device_epson_sio_interface();

	virtual void tx_w(int state) { };
	virtual void pout_w(int state) { };

protected:
	device_epson_sio_interface(const machine_config &mconfig, device_t &device);

	epson_sio_device *m_slot;
};


// device type definition
DECLARE_DEVICE_TYPE(EPSON_SIO, epson_sio_device)


// supported devices
void epson_sio_devices(device_slot_interface &device);


#endif // MAME_BUS_EPSON_SIO_EPSON_SIO_H
