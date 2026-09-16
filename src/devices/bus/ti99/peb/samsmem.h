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
#include "machine/74610.h"
#include "machine/ram.h"

namespace bus::ti99::peb {

class sams_memory_expansion_device : public device_t, public device_ti99_peribox_card_interface
{
public:
	sams_memory_expansion_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	void readz(offs_t offset, uint8_t *value) override;
	void write(offs_t offset, uint8_t data) override;

	void crureadz(offs_t offset, uint8_t *value) override { };
	void cruwrite(offs_t offset, uint8_t data) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	void reset_in(int state) override;

private:
	required_device<ls259_device> m_crulatch_u8;
	required_device<ttl74612_device> m_mapper_u12;
	required_device<ram_device> m_ram_u13;
	required_device<ram_device> m_ram_u14;

	line_state memsel(offs_t offset);
	line_state mapsel(offs_t offset);

	int m_select_bit;

	uint8_t m_upper_bits;
};

} // end namespace bus::ti99::peb

DECLARE_DEVICE_TYPE_NS(TI99_SAMSMEM, bus::ti99::peb, sams_memory_expansion_device)

#endif // MAME_BUS_TI99_PEB_SAMSMEM_H
