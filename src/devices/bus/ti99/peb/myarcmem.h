// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    TI-99 Myarc memory expansion
    See myarcmem.c for documentation

    Michael Zapf, September 2010
    February 2012: Rewritten as class

*****************************************************************************/

#ifndef MAME_BUS_TI99_PEB_MYARCMEM_H
#define MAME_BUS_TI99_PEB_MYARCMEM_H

#pragma once

#include "peribox.h"
#include "machine/ram.h"

DECLARE_DEVICE_TYPE(TI99_MYARCMEM, myarc_memory_expansion_device)

class myarc_memory_expansion_device : public ti_expansion_card_device
{
public:
	myarc_memory_expansion_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	DECLARE_READ8Z_MEMBER(readz) override;
	DECLARE_WRITE8_MEMBER(write) override;

	DECLARE_READ8Z_MEMBER(crureadz) override;
	DECLARE_WRITE8_MEMBER(cruwrite) override;

protected:
	void device_start() override;
	void device_reset() override;
	const tiny_rom_entry *device_rom_region() const override;
	ioport_constructor device_input_ports() const override;
	machine_config_constructor device_mconfig_additions() const override;

private:
	int     get_base(int offset);
	required_device<ram_device> m_ram;
	uint8_t*  m_dsrrom;
	int     m_bank;
	int     m_size;
};

#endif // MAME_BUS_TI99_PEB_MYARCMEM_H
