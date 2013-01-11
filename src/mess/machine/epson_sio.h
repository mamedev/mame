/**********************************************************************

    EPSON SIO port emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __EPSON_SIO_H__
#define __EPSON_SIO_H__

#include "emu.h"


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_EPSON_SIO_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, EPSON_SIO, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(epson_sio_devices, NULL, NULL, false)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class device_epson_sio_interface;


class epson_sio_device : public device_t,
							public device_slot_interface
{
public:
	// construction/destruction
	epson_sio_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~epson_sio_device();

	DECLARE_READ_LINE_MEMBER(rx_r);
	DECLARE_READ_LINE_MEMBER(pin_r);

	DECLARE_WRITE_LINE_MEMBER(tx_w);
	DECLARE_WRITE_LINE_MEMBER(pout_w);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	device_epson_sio_interface *m_cart;
};


// class representing interface-specific live sio device
class device_epson_sio_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	device_epson_sio_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_epson_sio_interface();

	virtual int rx_r() { return 1; };
	virtual int pin_r() { return 1; };

	virtual void tx_w(int state) { };
	virtual void pout_w(int state) { };

protected:
	epson_sio_device *m_slot;
};


// device type definition
extern const device_type EPSON_SIO;


// supported devices
SLOT_INTERFACE_EXTERN( epson_sio_devices );


#endif // __EPSON_SIO_H__
