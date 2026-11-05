// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn

#ifndef MAME_TVGAMES_XAVIX_IO_DUELMAST_SCANNER_H
#define MAME_TVGAMES_XAVIX_IO_DUELMAST_SCANNER_H

#pragma once

#include "xavix_io_image_sensor.h"

class picture_image_device;

class xavix_duelmast_scanner_device : public xavix_analog_image_sensor_device_base
{
public:
	static constexpr feature_type imperfect_features() { return feature::CAMERA; }

	xavix_duelmast_scanner_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	auto card_data_callback() { return m_card_data_cb.bind(); }
	auto expanded_cards_callback() { return m_expanded_cards_cb.bind(); }

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	virtual void capture_image() override;

private:
	static constexpr std::array<double, 4> CARD_FIELD{{ 140.66666666666667 / 650, 169.84375 / 907, 356.66666666666667 / 650, 351.0 / 907 }};
	static constexpr std::array<std::array<u8, 2>, 4> COVER_MARK_CENTRES{{ { 8, 6 }, { 24, 6 }, { 8, 24 }, { 24, 24 } }};

	required_device<picture_image_device> m_picture;
	required_ioport m_input;
	devcb_read8 m_card_data_cb;
	devcb_read_line m_expanded_cards_cb;
	std::array<double, 256> m_source_response;
	bool capture_manual_card(unsigned card);
};

DECLARE_DEVICE_TYPE(XAVIX_DUELMAST_SCANNER, xavix_duelmast_scanner_device)

#endif // MAME_TVGAMES_XAVIX_IO_DUELMAST_SCANNER_H
