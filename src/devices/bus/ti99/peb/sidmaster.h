// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    SID Master 99
    See sidmaster.cpp for documentation

    Michael Zapf
    September 2020

*****************************************************************************/

#ifndef MAME_BUS_TI99_PEB_SIDMASTER_H
#define MAME_BUS_TI99_PEB_SIDMASTER_H

#pragma once

#include "peribox.h"
#include "sound/mos6581.h"
#include "speaker.h"

namespace bus::ti99::peb {

class sidmaster_device : public device_t, public device_ti99_peribox_card_interface
{
public:
	sidmaster_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	void readz(offs_t offset, uint8_t *value) override { };  // No read
	void write(offs_t offset, uint8_t data) override;

	// Only for selection/deselection
	void crureadz(offs_t offset, uint8_t *value) override;
	void cruwrite(offs_t offset, uint8_t data) override;

private:
	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;
	void device_add_mconfig(machine_config &config) override;

	required_device<mos6581_device> m_sid;
};

} // end namespace bus::ti99::peb

DECLARE_DEVICE_TYPE_NS(TI99_SIDMASTER, bus::ti99::peb, sidmaster_device)

#endif // MAME_BUS_TI99_PEB_SIDMASTER_H
