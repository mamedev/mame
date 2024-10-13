// license:GPL-2.0+
// copyright-holders:Jonathan Edwards
/*********************************************************************

    bml3kanji.h

    Hitachi MP-9740 (?) kanji character ROM for the MB-689x

*********************************************************************/

#ifndef MAME_BUS_BML3_BML3KANJI_H
#define MAME_BUS_BML3_BML3KANJI_H

#pragma once

#include "bml3bus.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class bml3bus_kanji_device:
	public device_t,
	public device_bml3bus_card_interface
{
public:
	// construction/destruction
	bml3bus_kanji_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual void map_io(address_space_installer &space) override;

	uint16_t m_kanji_addr;

private:
	required_region_ptr<uint8_t> m_rom;
};

// device type definition
DECLARE_DEVICE_TYPE(BML3BUS_KANJI, bml3bus_kanji_device)

#endif // MAME_BUS_BML3_BML3KANJI_H
