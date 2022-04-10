// license:BSD-3-Clause
// copyright-holders:Dan Boris

#ifndef MAME_INCLUDES_ADVISION_H
#define MAME_INCLUDES_ADVISION_H

#pragma once

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "cpu/cop400/cop400.h"
#include "cpu/mcs48/mcs48.h"
#include "sound/dac.h"
#include "emupal.h"

#define SCREEN_TAG  "screen"
#define I8048_TAG   "i8048"
#define COP411_TAG  "cop411"

class advision_state : public driver_device
{
public:
	advision_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, I8048_TAG)
		, m_soundcpu(*this, COP411_TAG)
		, m_dac(*this, "dac")
		, m_cart(*this, "cartslot")
		, m_bank1(*this, "bank1")
		, m_joy(*this, "joystick")
		, m_palette(*this, "palette")
	{ }

	required_device<i8048_device> m_maincpu;
	required_device<cop411_cpu_device> m_soundcpu;
	required_device<dac_byte_interface> m_dac;
	required_device<generic_slot_device> m_cart;
	required_memory_bank m_bank1;
	required_ioport m_joy;
	required_device<palette_device> m_palette;

	virtual void machine_start() override;
	virtual void machine_reset() override;

	virtual void video_start() override;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void update_dac();
	void vh_write(int data);
	void vh_update(int x);

	uint8_t rom_r(offs_t offset);
	uint8_t ext_ram_r(offs_t offset);
	void ext_ram_w(offs_t offset, uint8_t data);
	uint8_t controller_r();
	void bankswitch_w(uint8_t data);
	void av_control_w(uint8_t data);
	DECLARE_READ_LINE_MEMBER( vsync_r );

	TIMER_CALLBACK_MEMBER( sound_cmd_sync );
	uint8_t sound_cmd_r();
	void sound_g_w(uint8_t data);
	void sound_d_w(uint8_t data);

	memory_region *m_cart_rom = nullptr;

	int m_ea_bank = 0;

	/* external RAM state */
	std::vector<uint8_t> m_ext_ram;
	int m_rambank = 0;

	/* video state */
	int m_frame_count = 0;
	int m_frame_start = 0;
	int m_video_enable = 0;
	int m_video_bank = 0;
	int m_video_hpos = 0;
	uint8_t m_led_latch[8] = {};
	std::unique_ptr<uint8_t []> m_display;

	/* sound state */
	int m_sound_cmd = 0;
	int m_sound_d = 0;
	int m_sound_g = 0;
	void advision_palette(palette_device &palette) const;
	void advision(machine_config &config);
	void io_map(address_map &map);
	void program_map(address_map &map);
};

#endif
