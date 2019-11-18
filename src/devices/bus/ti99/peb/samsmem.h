// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    TI-99 SuperAMS memory expansion
    See samsmem.c for documentation

    Michael Zapf
    September 2010

    February 2012: Rewritten as class

*****************************************************************************/

#ifndef MAME_BUS_TI99_PEB_SAMSMEM_H
#define MAME_BUS_TI99_PEB_SAMSMEM_H

#pragma once

#include "peribox.h"
#include "machine/74259.h"
#include "machine/ram.h"

namespace bus { namespace ti99 { namespace peb {

class sams_memory_expansion_device : public device_t, public device_ti99_peribox_card_interface
{
public:
	sams_memory_expansion_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	DECLARE_READ8Z_MEMBER(readz) override;
	void write(offs_t offset, uint8_t data) override;

	DECLARE_READ8Z_MEMBER(crureadz) override;
	void cruwrite(offs_t offset, uint8_t data) override;

protected:
	void device_start() override;
	void device_reset() override;

	virtual void device_add_mconfig(machine_config &config) override;

private:
	DECLARE_WRITE_LINE_MEMBER(access_mapper_w);
	DECLARE_WRITE_LINE_MEMBER(map_mode_w);

	// Console RAM
	required_device<ram_device> m_ram;
	required_device<ls259_device> m_crulatch;
	int     m_mapper[16];
	bool    m_map_mode;
	bool    m_access_mapper;
};

} } } // end namespace bus::ti99::peb

DECLARE_DEVICE_TYPE_NS(TI99_SAMSMEM, bus::ti99::peb, sams_memory_expansion_device)

#endif // MAME_BUS_TI99_PEB_SAMSMEM_H
