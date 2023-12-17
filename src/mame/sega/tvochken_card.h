// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/******************************************************************************

    tvochken_card.h

    Sega TV Ocha-ken barcode card loader

*******************************************************************************/
#ifndef MAME_SEGA_TVOCHKEN_CARD_H
#define MAME_SEGA_TVOCHKEN_CARD_H

#pragma once

#include <string>
#include <system_error>
#include <utility>


class tvochken_card_device : public device_t, public device_image_interface
{
public:
	tvochken_card_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);
	virtual ~tvochken_card_device();

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override ATTR_COLD;
	virtual void call_unload() override ATTR_COLD;
	virtual bool is_readable() const noexcept override { return true; }
	virtual bool is_writeable() const noexcept override { return false; }
	virtual bool is_creatable() const noexcept override { return false; }
	virtual bool is_reset_on_load() const noexcept override { return false; }
	virtual char const *image_interface() const noexcept override { return "tvochken_card"; }
	virtual char const *file_extensions() const noexcept override { return "png,jpg,bmp"; } // loose media not supported yet
	virtual char const *image_type_name() const noexcept override { return "card"; }
	virtual char const *image_brief_type_name() const noexcept override { return "crd"; }

	u16 barcode() const noexcept { return m_barcode; }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	// device_image_interface implementation
	virtual software_list_loader const &get_software_list_loader() const override ATTR_COLD;

private:
	u16 m_barcode;
};


DECLARE_DEVICE_TYPE(TVOCHKEN_CARD, tvochken_card_device)

#endif // MAME_SEGA_TVOCHKEN_CARD_H
