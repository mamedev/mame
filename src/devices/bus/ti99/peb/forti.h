// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/*******************************************************************************
    FORTi Sound card

    4 x TMS9919 sound generators
    4 sound outputs; may be coupled to two stereo outputs

    Michael Zapf
    March 2020

*******************************************************************************/

#ifndef MAME_BUS_TI99_PEB_FORTI_H
#define MAME_BUS_TI99_PEB_FORTI_H

#pragma once

#include "peribox.h"
#include "sound/sn76496.h"
#include "speaker.h"

namespace bus::ti99::peb {

class forti_device : public device_t, public device_ti99_peribox_card_interface
{
public:
	forti_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	void write(offs_t offset, uint8_t data) override;
	void readz(offs_t offset, uint8_t *value) override;
	void crureadz(offs_t offset, uint8_t *value) override { }
	void cruwrite(offs_t offset, uint8_t data) override { }

	DECLARE_WRITE_LINE_MEMBER( ready_sound );

private:
	void device_start() override;
	void device_reset() override;
	void device_add_mconfig(machine_config &config) override;

	// TODO: Replace by TMS9919 when available
	required_device<sn94624_device> m_generator1;
	required_device<sn94624_device> m_generator2;
	required_device<sn94624_device> m_generator3;
	required_device<sn94624_device> m_generator4;
};

} // end namespace bus::ti99::peb

DECLARE_DEVICE_TYPE_NS(TI99_FORTI, bus::ti99::peb, forti_device)

#endif // MAME_BUS_TI99_PEB_FORTI_H
