// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/******************************************************************************

    9h0-0008_card.h

    Sega Toys 9H0-0008 barcode card loader

*******************************************************************************/
#ifndef MAME_SEGA_9H0_0008_CARD_H
#define MAME_SEGA_9H0_0008_CARD_H

#pragma once

#include <string>
#include <system_error>
#include <utility>

#include "9h0-0008_iox.h"

class sega_9h0_0008_card_device : public sega_9h0_0008_iox_slot_device, public device_image_interface
{
public:
	// construction/destruction
	template <typename T>
	sega_9h0_0008_card_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt, bool const fixed)
		: sega_9h0_0008_card_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(fixed);
	}
	sega_9h0_0008_card_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);
	virtual ~sega_9h0_0008_card_device();

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override ATTR_COLD;
	virtual void call_unload() override ATTR_COLD;
	virtual bool is_readable() const noexcept override { return true; }
	virtual bool is_writeable() const noexcept override { return false; }
	virtual bool is_creatable() const noexcept override { return false; }
	virtual bool is_reset_on_load() const noexcept override { return false; }
	virtual char const *image_interface() const noexcept override { return "sega_9h0_0008_card"; }
	virtual char const *file_extensions() const noexcept override { return "png,jpg,bmp"; } // loose media not supported yet
	virtual char const *image_type_name() const noexcept override { return "card"; }
	virtual char const *image_brief_type_name() const noexcept override { return "crd"; }

	virtual u16 data() const noexcept override { return m_barcode; }

protected:
	sega_9h0_0008_card_device(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	// device_image_interface implementation
	virtual software_list_loader const &get_software_list_loader() const override ATTR_COLD;

private:
	u16 m_barcode;
};


DECLARE_DEVICE_TYPE(SEGA_9H0_0008_CARD, sega_9h0_0008_card_device)

#endif // MAME_SEGA_9H0_0008_CARD_H
