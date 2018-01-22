// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    ezcgi.h

    "E-Z Color Graphics Interface" by Steve Ciarcia
    from BYTE Magazine, August, 1982
    https://archive.org/details/byte-magazine-1982-08-rescan

*********************************************************************/

#ifndef MAME_BUS_A2BUS_A2EZCGI_H
#define MAME_BUS_A2BUS_A2EZCGI_H

#pragma once

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
	a2bus_ezcgi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	a2bus_ezcgi_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;

	required_device<tms9918a_device> m_tms;

private:
	DECLARE_WRITE_LINE_MEMBER( tms_irq_w );
};

class a2bus_ezcgi_9938_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_ezcgi_9938_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	a2bus_ezcgi_9938_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;

	required_device<v9938_device> m_tms;

private:
	DECLARE_WRITE_LINE_MEMBER( tms_irq_w );
};

class a2bus_ezcgi_9958_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_ezcgi_9958_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	a2bus_ezcgi_9958_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;

	required_device<v9958_device> m_tms;

private:
	DECLARE_WRITE_LINE_MEMBER( tms_irq_w );
};

// device type definition
DECLARE_DEVICE_TYPE(A2BUS_EZCGI,      a2bus_ezcgi_device)
DECLARE_DEVICE_TYPE(A2BUS_EZCGI_9938, a2bus_ezcgi_9938_device)
DECLARE_DEVICE_TYPE(A2BUS_EZCGI_9958, a2bus_ezcgi_9958_device)

#endif  // MAME_BUS_A2BUS_A2EZCGI_H
