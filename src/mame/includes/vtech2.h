// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller
/*****************************************************************************
 *
 * includes/vtech2.h
 *
 ****************************************************************************/

#ifndef VTECH2_H_
#define VTECH2_H_
#include "sound/speaker.h"
#include "imagedev/cassette.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#define TRKSIZE_FM  3172    /* size of a standard FM mode track */

class vtech2_state : public driver_device
{
public:
	vtech2_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_speaker(*this, "speaker"),
		m_cassette(*this, "cassette"),
		m_cart(*this, "cartslot"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")  { }

	UINT8 *m_videoram;
	int m_laser_latch;
	char m_laser_frame_message[64+1];
	int m_laser_frame_time;
	UINT8 *m_mem;
	int m_laser_bank_mask;
	int m_laser_bank[4];
	int m_laser_video_bank;
	UINT8 m_laser_track_x2[2];
	UINT8 m_laser_fdc_status;
	UINT8 m_laser_fdc_data[TRKSIZE_FM];
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
	int m_row_a;
	int m_row_b;
	int m_row_c;
	int m_row_d;
	int m_laser_bg_mode;
	int m_laser_two_color;
	DECLARE_WRITE8_MEMBER(laser_bank_select_w);
	DECLARE_WRITE8_MEMBER(laser_fdc_w);
	DECLARE_WRITE8_MEMBER(laser_bg_mode_w);
	DECLARE_WRITE8_MEMBER(laser_two_color_w);
	DECLARE_READ8_MEMBER(laser_fdc_r);
	DECLARE_DRIVER_INIT(laser);
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(vtech2);
	DECLARE_MACHINE_RESET(laser500);
	DECLARE_MACHINE_RESET(laser700);
	UINT32 screen_update_laser(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vtech2_interrupt);

	int mra_bank(int bank, int offs);
	void mwa_bank(int bank, int offs, int data);
	memory_region *m_cart_rom;

	DECLARE_WRITE8_MEMBER(mwa_bank1);
	DECLARE_WRITE8_MEMBER(mwa_bank2);
	DECLARE_WRITE8_MEMBER(mwa_bank3);
	DECLARE_WRITE8_MEMBER(mwa_bank4);
	DECLARE_READ8_MEMBER(mra_bank1);
	DECLARE_READ8_MEMBER(mra_bank2);
	DECLARE_READ8_MEMBER(mra_bank3);
	DECLARE_READ8_MEMBER(mra_bank4);
	required_device<cpu_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;
	required_device<cassette_image_device> m_cassette;
	required_device<generic_slot_device> m_cart;
	void laser_machine_init(int bank_mask, int video_mask);
	void laser_get_track();
	void laser_put_track();
	device_t *laser_file();
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};


#endif /* VTECH2_H_ */
