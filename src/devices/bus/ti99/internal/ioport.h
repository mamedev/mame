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
	virtual DECLARE_READ8Z_MEMBER( readz ) { };
	virtual DECLARE_WRITE8_MEMBER( write ) { };
	virtual DECLARE_SETADDRESS_DBIN_MEMBER( setaddress_dbin ) { };
	virtual DECLARE_READ8Z_MEMBER( crureadz ) { };
	virtual DECLARE_WRITE8_MEMBER( cruwrite ) { };
	virtual DECLARE_WRITE_LINE_MEMBER( memen_in ) { };
	virtual DECLARE_WRITE_LINE_MEMBER( msast_in ) { };
	virtual DECLARE_WRITE_LINE_MEMBER( clock_in ) { };

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
	ioport_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template<class Object> devcb_base &set_extint_callback(Object &&cb) {  return m_console_extint.set_callback(std::forward<Object>(cb)); }
	template<class Object> devcb_base &set_ready_callback(Object &&cb)  {  return m_console_ready.set_callback(std::forward<Object>(cb)); }

	// Methods called from the console
	DECLARE_READ8Z_MEMBER( readz );
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_SETADDRESS_DBIN_MEMBER( setaddress_dbin );
	DECLARE_READ8Z_MEMBER( crureadz );
	DECLARE_WRITE8_MEMBER( cruwrite );
	DECLARE_WRITE_LINE_MEMBER( memen_in );
	DECLARE_WRITE_LINE_MEMBER( msast_in );
	DECLARE_WRITE_LINE_MEMBER( clock_in );

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

void ti99_io_port(device_slot_interface &device);
void ti99_io_port_ev(device_slot_interface &device);

#define MCFG_IOPORT_ADD( _tag )  \
	MCFG_DEVICE_ADD(_tag, TI99_IOPORT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(ti99_io_port, nullptr, false)

#define MCFG_IOPORT_ADD_WITH_PEB( _tag )  \
	MCFG_DEVICE_ADD(_tag, TI99_IOPORT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(ti99_io_port_ev, "peb", false)

#define MCFG_IOPORT_EXTINT_HANDLER( _extint ) \
	devcb = &downcast<bus::ti99::internal::ioport_device &>(*device).set_extint_callback(DEVCB_##_extint);

#define MCFG_IOPORT_READY_HANDLER( _ready ) \
	devcb = &downcast<bus::ti99::internal::ioport_device &>(*device).set_ready_callback(DEVCB_##_ready);

#endif /* __TI99IOPORT__ */
