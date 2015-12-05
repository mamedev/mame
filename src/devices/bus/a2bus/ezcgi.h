// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    ezcgi.h

    "E-Z Color Graphics Interface" by Steve Ciarcia
    from BYTE Magazine, August, 1982
    https://archive.org/details/byte-magazine-1982-08-rescan

*********************************************************************/

#ifndef __A2BUS_EZCGI__
#define __A2BUS_EZCGI__

#include "emu.h"
#include "a2bus.h"
#include "video/tms9928a.h"
#include "video/v9938.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_ezcgi_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_ezcgi_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	a2bus_ezcgi_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;

	DECLARE_WRITE_LINE_MEMBER( tms_irq_w );

protected:
	virtual void device_start();
	virtual void device_reset();

	// overrides of standard a2bus slot functions
	virtual UINT8 read_c0nx(address_space &space, UINT8 offset);
	virtual void write_c0nx(address_space &space, UINT8 offset, UINT8 data);

	required_device<tms9918a_device> m_tms;
};

class a2bus_ezcgi_9938_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_ezcgi_9938_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	a2bus_ezcgi_9938_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;

	DECLARE_WRITE_LINE_MEMBER( tms_irq_w );

protected:
	virtual void device_start();
	virtual void device_reset();

	// overrides of standard a2bus slot functions
	virtual UINT8 read_c0nx(address_space &space, UINT8 offset);
	virtual void write_c0nx(address_space &space, UINT8 offset, UINT8 data);

	required_device<v9938_device> m_tms;
};

class a2bus_ezcgi_9958_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_ezcgi_9958_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	a2bus_ezcgi_9958_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;

	DECLARE_WRITE_LINE_MEMBER( tms_irq_w );

protected:
	virtual void device_start();
	virtual void device_reset();

	// overrides of standard a2bus slot functions
	virtual UINT8 read_c0nx(address_space &space, UINT8 offset);
	virtual void write_c0nx(address_space &space, UINT8 offset, UINT8 data);

	required_device<v9958_device> m_tms;
};

// device type definition
extern const device_type A2BUS_EZCGI;
extern const device_type A2BUS_EZCGI_9938;
extern const device_type A2BUS_EZCGI_9958;

#endif  /* __A2BUS_EZCGI__ */
