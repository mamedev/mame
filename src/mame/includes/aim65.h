// license:GPL-2.0+
// copyright-holders:Peter Trauner, Dan Boris, Dirk Best, Robbbert
/*****************************************************************************
 *
 * includes/aim65.h
 *
 * Rockwell AIM-65
 *
 ****************************************************************************/
#ifndef MAME_INCLUDES_AIM65_H
#define MAME_INCLUDES_AIM65_H

#pragma once

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "cpu/m6502/m6502.h"
#include "imagedev/cassette.h"
#include "machine/6522via.h"
#include "machine/6821pia.h"
#include "machine/mos6530n.h"
#include "machine/ram.h"
#include "sound/wave.h"
#include "video/dl1416.h"


class aim65_state : public driver_device
{
public:
	aim65_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cassette1(*this, "cassette")
		, m_cassette2(*this, "cassette2")
		, m_z24(*this, "z24")
		, m_z25(*this, "z25")
		, m_z26(*this, "z26")
		, m_z12(*this, "z12")
		, m_z13(*this, "z13")
		, m_z14(*this, "z14")
		, m_z15(*this, "z15")
		, m_ram(*this, RAM_TAG)
		, m_ds(*this, "ds%u", 1)
		, m_digits(*this, "digit%u", 0U)
	{
	}

	void aim65(machine_config &config);

protected:
	virtual void machine_start() override;

	DECLARE_WRITE8_MEMBER(aim65_pia_a_w);
	DECLARE_WRITE8_MEMBER(aim65_pia_b_w);
	DECLARE_READ8_MEMBER(aim65_riot_b_r);
	DECLARE_WRITE8_MEMBER(aim65_riot_a_w);
	DECLARE_WRITE8_MEMBER(aim65_pb_w);
	DECLARE_READ8_MEMBER(aim65_pb_r);

	template <unsigned D> DECLARE_WRITE16_MEMBER(aim65_update_ds);

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(z24_load) { return load_cart(image, m_z24, "z24"); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(z25_load) { return load_cart(image, m_z25, "z25"); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(z26_load) { return load_cart(image, m_z26, "z26"); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(z12_load) { return load_cart(image, m_z12, "z12"); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(z13_load) { return load_cart(image, m_z13, "z13"); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(z14_load) { return load_cart(image, m_z14, "z14"); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(z15_load) { return load_cart(image, m_z15, "z15"); }

	image_init_result load_cart(device_image_interface &image, generic_slot_device *slot, const char *slot_tag);

	void aim65_mem(address_map &map);

private:
	uint8_t m_riot_port_a;
	uint8_t m_pb_save;

	required_device<cpu_device> m_maincpu;
	required_device<cassette_image_device> m_cassette1;
	required_device<cassette_image_device> m_cassette2;
	required_device<generic_slot_device> m_z24;
	required_device<generic_slot_device> m_z25;
	required_device<generic_slot_device> m_z26;
	required_device<generic_slot_device> m_z12;
	required_device<generic_slot_device> m_z13;
	required_device<generic_slot_device> m_z14;
	required_device<generic_slot_device> m_z15;
	required_device<ram_device> m_ram;
	required_device_array<dl1416_device, 5> m_ds;
	output_finder<20> m_digits;
};


#endif // MAME_INCLUDES_AIM65_H
