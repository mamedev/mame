// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    TI-99 32K memory expansion
    See ti32kmem.c for documentation

    Michael Zapf

    February 2012: Rewritten as class

*****************************************************************************/

#ifndef MAME_BUS_TI99_PEB_TI_32KMEM_H
#define MAME_BUS_TI99_PEB_TI_32KMEM_H

#pragma once

#include "peribox.h"
#include "machine/ram.h"

namespace bus::ti99::peb {

class ti_32k_expcard_device : public device_t, public device_ti99_peribox_card_interface
{
public:
	ti_32k_expcard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	void readz(offs_t offset, uint8_t *value) override;
	void write(offs_t offset, uint8_t data) override;

	void crureadz(offs_t offset, uint8_t *value) override { }
	void cruwrite(offs_t offset, uint8_t data) override { }

protected:
	void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<ram_device> m_ram;
};

} // end namespace bus::ti99::peb

DECLARE_DEVICE_TYPE_NS(TI99_32KMEM, bus::ti99::peb, ti_32k_expcard_device)

#endif // MAME_BUS_TI99_PEB_TI_32KMEM_H
