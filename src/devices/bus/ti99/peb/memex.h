// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    Geneve "Memex" memory expansion
    See memex.c for documentation

    Michael Zapf, February 2011
    February 2012: Rewritten as class

*****************************************************************************/

#ifndef MAME_BUS_TI99_PEB_MEMEX_H
#define MAME_BUS_TI99_PEB_MEMEX_H

#pragma once

#include "peribox.h"
#include "machine/ram.h"

namespace bus { namespace ti99 { namespace peb {


class geneve_memex_device : public device_t, public device_ti99_peribox_card_interface
{
public:
	geneve_memex_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	DECLARE_READ8Z_MEMBER(readz) override;
	void write(offs_t offset, uint8_t data) override;

	DECLARE_READ8Z_MEMBER(crureadz) override { }
	void cruwrite(offs_t offset, uint8_t data) override { }

protected:
	void device_start() override;
	void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;

private:
	bool    access_enabled(offs_t offset);
	required_device<ram_device> m_ram;
	uint8_t   m_switches;
};
} } } // end namespace bus::ti99::peb

DECLARE_DEVICE_TYPE_NS(TI99_MEMEX, bus::ti99::peb, geneve_memex_device)

#endif // MAME_BUS_TI99_PEB_MEMEX_H
