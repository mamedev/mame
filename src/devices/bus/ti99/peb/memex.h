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

namespace bus::ti99::peb {

class geneve_memex_device : public device_t, public device_ti99_peribox_card_interface
{
public:
	geneve_memex_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	void readz(offs_t offset, uint8_t *value) override;
	void write(offs_t offset, uint8_t data) override;

	void crureadz(offs_t offset, uint8_t *value) override { }
	void cruwrite(offs_t offset, uint8_t data) override { }

protected:
	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	bool    access_enabled(offs_t offset);
	required_device<ram_device> m_ram;
	uint8_t   m_switches;
};

} // end namespace bus::ti99::peb

DECLARE_DEVICE_TYPE_NS(TI99_MEMEX, bus::ti99::peb, geneve_memex_device)

#endif // MAME_BUS_TI99_PEB_MEMEX_H
