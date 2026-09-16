// license:GPL-2.0+
// copyright-holders:Peter Trauner, Dan Boris, Dirk Best, Robbbert
/*****************************************************************************
 *
 * includes/aim65.h
 *
 * Rockwell AIM-65
 *
 ****************************************************************************/
#ifndef MAME_ROCKWELL_AIM65_H
#define MAME_ROCKWELL_AIM65_H

#pragma once

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "cpu/m6502/m6502.h"
#include "imagedev/cassette.h"
#include "bus/rs232/rs232.h"
#include "machine/6522via.h"
#include "machine/6821pia.h"
#include "machine/mos6530.h"
#include "machine/ram.h"
#include "video/dl1416.h"
#include "emupal.h"
#include "screen.h"


class aim65_state : public driver_device
{
public:
	aim65_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_palette(*this, "palette")
		, m_riot_port_a(0)
		, m_pb_save(0)
		, m_kb_en(true)
		, m_ca2(true)
		, m_cb2(true)
		, m_printer_x(0)
		, m_printer_y(0)
		, m_printer_flag(0)
		, m_printer_level(0)
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
		, m_rs232(*this, "rs232")
		, m_via0(*this, "via0")
		, m_via1(*this, "via1")
		, m_pia(*this, "pia")
		, m_riot(*this, "riot")
		, m_io_keyboard(*this, "KEY.%u", 0)
	{
	}

	void aim65(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(reset_button);

private:
	virtual void machine_start() override ATTR_COLD;

	void aim65_palette(palette_device &palette) const;
	void u1_pa_w(u8 data);
	void u1_pb_w(u8 data);
	u8 z33_pb_r();
	void z32_pa_w(u8 data);
	void z32_pb_w(u8 data);
	void z32_cb2_w(bool state);
	u8 z32_pb_r();

	template <unsigned D> void update_ds(offs_t offset, u16 data);

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(z24_load) { return load_cart(image, m_z24, "z24"); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(z25_load) { return load_cart(image, m_z25, "z25"); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(z26_load) { return load_cart(image, m_z26, "z26"); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(z12_load) { return load_cart(image, m_z12, "z12"); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(z13_load) { return load_cart(image, m_z13, "z13"); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(z14_load) { return load_cart(image, m_z14, "z14"); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(z15_load) { return load_cart(image, m_z15, "z15"); }
	emu_timer *m_print_timer = nullptr;
	TIMER_CALLBACK_MEMBER(printer_timer);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	std::pair<std::error_condition, std::string> load_cart(device_image_interface &image, generic_slot_device *slot, const char *slot_tag);

	void mem_map(address_map &map) ATTR_COLD;

	optional_device<palette_device> m_palette;
	uint8_t m_riot_port_a = 0U;
	uint8_t m_pb_save = 0U;
	bool m_kb_en = false;
	bool m_ca2 = false;
	bool m_cb2 = false;
	u8 m_printer_x =0U;
	u8 m_printer_y = 0U;
	u8 m_printer_flag = 0U;
	bool m_printer_level = false;
	std::unique_ptr<uint16_t[]> m_printerRAM {};

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
	required_device<rs232_port_device> m_rs232;
	required_device<via6522_device> m_via0;
	required_device<via6522_device> m_via1;
	required_device<pia6821_device> m_pia;
	required_device<mos6532_device> m_riot;
	required_ioport_array<8> m_io_keyboard;
};


#endif // MAME_ROCKWELL_AIM65_H
