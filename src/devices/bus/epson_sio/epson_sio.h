// license:GPL-2.0+
// copyright-holders:Dirk Best
/**********************************************************************

    EPSON SIO port emulation

**********************************************************************/

#ifndef MAME_BUS_EPSON_SIO_EPSON_SIO_H
#define MAME_BUS_EPSON_SIO_EPSON_SIO_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class device_epson_sio_interface;

// supported devices
void epson_sio_devices(device_slot_interface &device);

class epson_sio_device : public device_t, public device_single_card_slot_interface<device_epson_sio_interface>
{
public:
	// construction/destruction
	epson_sio_device(machine_config const &mconfig, char const *tag, device_t *owner, char const *dflt)
		: epson_sio_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		epson_sio_devices(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	epson_sio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~epson_sio_device();

	// callbacks
	auto rx_callback() { return m_write_rx.bind(); }
	auto pin_callback() { return m_write_pin.bind(); }

	// called from owner
	void tx_w(int state);
	void pout_w(int state);

	// called from subdevice
	void rx_w(int state) { m_write_rx(state); }
	void pin_w(int state) { m_write_pin(state); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	device_epson_sio_interface *m_cart;

private:
	devcb_write_line m_write_rx;
	devcb_write_line m_write_pin;
};


// class representing interface-specific live sio device
class device_epson_sio_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_epson_sio_interface();

	virtual void tx_w(int state) { }
	virtual void pout_w(int state) { }

protected:
	device_epson_sio_interface(const machine_config &mconfig, device_t &device);

	epson_sio_device *m_slot;
};


// device type definition
DECLARE_DEVICE_TYPE(EPSON_SIO, epson_sio_device)

#endif // MAME_BUS_EPSON_SIO_EPSON_SIO_H
