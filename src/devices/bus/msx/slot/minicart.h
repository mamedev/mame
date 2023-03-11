// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***************************************************************************

 MSX Yamaha minicart slot

Pinout of the 30 pin connector is unknown.

 ***************************************************************************/
#ifndef MAME_BUS_MSX_SLOT_MINICART_H
#define MAME_BUS_MSX_SLOT_MINICART_H

#pragma once

#include "cartridge.h"


DECLARE_DEVICE_TYPE(MSX_SLOT_YAMAHA_MINICART,  msx_slot_yamaha_minicart_device)


class msx_slot_yamaha_minicart_device : public msx_slot_cartridge_device
{
public:
	msx_slot_yamaha_minicart_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual const char *image_interface() const noexcept override { return "msx_yamaha_minicart"; }
	virtual const char *image_type_name() const noexcept override { return "yamahaminicart"; }
	virtual const char *image_brief_type_name() const noexcept override { return "mini"; }
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override { return software_get_default_slot("nomapper"); }

protected:
	virtual void device_start() override;
};

#endif // MAME_BUS_MSX_SLOT_MINICART_H
