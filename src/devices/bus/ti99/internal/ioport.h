// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************
     I/O port
*****************************************************************************/

#ifndef MAME_BUS_TI99_INTERNAL_IOPORT_H
#define MAME_BUS_TI99_INTERNAL_IOPORT_H

#pragma once

#include "bus/ti99/ti99defs.h"

namespace bus { namespace ti99 { namespace internal {

extern const device_type IOPORT;

class ioport_device;

/********************************************************************
    Common parent class of all devices attached to the I/O port
********************************************************************/
class ioport_attached_device : public device_t
{
public:
	ioport_attached_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
		device_t(mconfig, type, tag, owner, clock),
		m_ioport(nullptr)
	{ }

	// Methods called from the console / ioport
	virtual DECLARE_READ8Z_MEMBER( readz ) { }
	virtual void write(offs_t offset, uint8_t data) { }
	virtual DECLARE_SETADDRESS_DBIN_MEMBER( setaddress_dbin ) { }
	virtual DECLARE_READ8Z_MEMBER( crureadz ) { }
	virtual void cruwrite(offs_t offset, uint8_t data) { }
	virtual DECLARE_WRITE_LINE_MEMBER( memen_in ) { }
	virtual DECLARE_WRITE_LINE_MEMBER( msast_in ) { }
	virtual DECLARE_WRITE_LINE_MEMBER( clock_in ) { }

	void set_ioport(ioport_device* ioport) { m_ioport = ioport; }

protected:
	// Methods called from the external device
	DECLARE_WRITE_LINE_MEMBER( set_extint );
	DECLARE_WRITE_LINE_MEMBER( set_ready );
private:
	ioport_device* m_ioport;
};

/********************************************************************
    I/O port
********************************************************************/

class ioport_device : public device_t, public device_slot_interface
{
	friend class ioport_attached_device;

public:
	template <typename U>
	ioport_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, U &&opts, const char *dflt)
		: ioport_device(mconfig, tag, owner, clock)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	ioport_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// Methods called from the console
	DECLARE_READ8Z_MEMBER( readz );
	void write(offs_t offset, uint8_t data);
	DECLARE_SETADDRESS_DBIN_MEMBER( setaddress_dbin );
	DECLARE_READ8Z_MEMBER( crureadz );
	void cruwrite(offs_t offset, uint8_t data);
	DECLARE_WRITE_LINE_MEMBER( memen_in );
	DECLARE_WRITE_LINE_MEMBER( msast_in );
	DECLARE_WRITE_LINE_MEMBER( clock_in );

	// Callbacks
	auto extint_cb() { return m_console_extint.bind(); }
	auto ready_cb() { return m_console_ready.bind(); }

protected:
	void device_start() override;
	void device_config_complete() override;

	// Methods called back from the external device
	devcb_write_line m_console_extint;   // EXTINT line
	devcb_write_line m_console_ready;  // READY line

private:
	ioport_attached_device*    m_connected;
};
}   }  } // end namespace bus::ti99::internal

DECLARE_DEVICE_TYPE_NS(TI99_IOPORT, bus::ti99::internal, ioport_device)

void ti99_ioport_options_plain(device_slot_interface &device);
void ti99_ioport_options_evpc(device_slot_interface &device);

#endif /* __TI99IOPORT__ */
