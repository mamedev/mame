// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller
/*****************************************************************************
 *
 * includes/vtech2.h
 *
 ****************************************************************************/
#ifndef MAME_INCLUDES_VTECH2_H
#define MAME_INCLUDES_VTECH2_H

#pragma once

#include "machine/bankdev.h"
#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "bus/vtech/ioexp/ioexp.h"
#include "imagedev/cassette.h"
#include "imagedev/flopdrv.h"
#include "sound/spkrdev.h"
#include "emupal.h"

#define TRKSIZE_FM  3172    /* size of a standard FM mode track */

class vtech2_state : public driver_device
{
public:
	vtech2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_speaker(*this, "speaker")
		, m_cassette(*this, "cassette")
		, m_cart(*this, "cartslot")
		, m_laser_file(*this, {FLOPPY_0, FLOPPY_1})
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_videoram(*this, "videoram")
		, m_io_keyboard(*this, {"ROW0", "ROW1", "ROW2", "ROW3", "ROW4", "ROW5", "ROW6", "ROW7", "ROWD", "ROWC", "ROWB", "ROWA"})
		, m_banka(*this, "banka")
		, m_bankb(*this, "bankb")
		, m_bankc(*this, "bankc")
		, m_bankd(*this, "bankd")
		, m_ioexp(*this, "io")
	{ }

	void laser350(machine_config &config);
	void laser700(machine_config &config);
	void laser500(machine_config &config);

	void init_laser();

	DECLARE_INPUT_CHANGED_MEMBER(reset_button);

protected:
	virtual void machine_reset() override;

private:
	DECLARE_WRITE8_MEMBER(laser_bank_select_w);
	DECLARE_WRITE8_MEMBER(laser_fdc_w);
	DECLARE_WRITE8_MEMBER(laser_bg_mode_w);
	DECLARE_WRITE8_MEMBER(laser_two_color_w);
	DECLARE_READ8_MEMBER(laser_fdc_r);
	void vtech2_palette(palette_device &palette) const;
	uint32_t screen_update_laser(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vtech2_interrupt);
	DECLARE_WRITE8_MEMBER(mmio_w);
	DECLARE_READ8_MEMBER(mmio_r);
	DECLARE_READ8_MEMBER(cart_r);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);

	void laser_get_track();
	void laser_put_track();

	void io_map(address_map &map);
	void mem_map(address_map &map);
	void m_map350(address_map &map);
	void m_map500(address_map &map);
	void m_map700(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;
	required_device<cassette_image_device> m_cassette;
	required_device<generic_slot_device> m_cart;
	optional_device_array<legacy_floppy_image_device, 2> m_laser_file;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_shared_ptr<u8> m_videoram;
	required_ioport_array<12> m_io_keyboard;
	required_device<address_map_bank_device> m_banka;
	required_device<address_map_bank_device> m_bankb;
	required_device<address_map_bank_device> m_bankc;
	required_device<address_map_bank_device> m_bankd;
	required_device<vtech_ioexp_slot_device> m_ioexp;

	char m_laser_frame_message[64+1];
	int m_laser_frame_time;
	u8 m_laser_latch;
	uint8_t m_laser_track_x2[2];
	uint8_t m_laser_fdc_status;
	uint8_t m_laser_fdc_data[TRKSIZE_FM];
	int m_laser_data;
	int m_laser_fdc_edge;
	int m_laser_fdc_bits;
	int m_laser_drive;
	int m_laser_fdc_start;
	int m_laser_fdc_write;
	int m_laser_fdc_offs;
	int m_laser_fdc_latch;
	int m_level_old;
	int m_cassette_bit;
	int m_laser_bg_mode;
	int m_laser_two_color;
	u8 m_language;
	u32 m_cart_size;

	memory_region *m_cart_rom;
};

#endif // MAME_INCLUDES_VTECH2_H
