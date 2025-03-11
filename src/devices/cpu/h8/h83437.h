// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
// copyright-holders:Lubomir Rintel
/***************************************************************************

    h83437.h

    H8-3437 family emulation

    H8-300-based mcus.

    Variant         ROM        RAM
    H8/3434         32K         1K
    H8/3436         48K         2K
    H8/3437         60K         2K

***************************************************************************/

#ifndef MAME_CPU_H8_H83437_H
#define MAME_CPU_H8_H83437_H

#pragma once

#include "h83337.h"
#include "h8_port.h"

class h83437_device : public h83337_device {
public:
	h83437_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	auto read_porta()  { return m_read_port [PORT_A].bind(); }
	auto write_porta() { return m_write_port[PORT_A].bind(); }
	auto read_portb()  { return m_read_port [PORT_B].bind(); }
	auto write_portb() { return m_write_port[PORT_B].bind(); }

protected:
	h83437_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u32 start);

	required_device<h8_port_device> m_porta;
	required_device<h8_port_device> m_portb;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	void map(address_map &map) ATTR_COLD;
};

class h83434_device : public h83437_device {
public:
	h83434_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class h83436_device : public h83437_device {
public:
	h83436_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

DECLARE_DEVICE_TYPE(H83434, h83434_device)
DECLARE_DEVICE_TYPE(H83436, h83436_device)
DECLARE_DEVICE_TYPE(H83437, h83437_device)

#endif // MAME_CPU_H8_H83437_H
