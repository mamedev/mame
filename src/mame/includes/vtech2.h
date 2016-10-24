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
	vtech2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_speaker(*this, "speaker"),
		m_cassette(*this, "cassette"),
		m_cart(*this, "cartslot"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")  { }

	uint8_t *m_videoram;
	int m_laser_latch;
	char m_laser_frame_message[64+1];
	int m_laser_frame_time;
	uint8_t *m_mem;
	int m_laser_bank_mask;
	int m_laser_bank[4];
	int m_laser_video_bank;
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
	int m_row_a;
	int m_row_b;
	int m_row_c;
	int m_row_d;
	int m_laser_bg_mode;
	int m_laser_two_color;
	void laser_bank_select_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void laser_fdc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void laser_bg_mode_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void laser_two_color_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t laser_fdc_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void init_laser();
	virtual void machine_reset() override;
	virtual void video_start() override;
	void palette_init_vtech2(palette_device &palette);
	void machine_reset_laser500();
	void machine_reset_laser700();
	uint32_t screen_update_laser(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vtech2_interrupt(device_t &device);

	int mra_bank(int bank, int offs);
	void mwa_bank(int bank, int offs, int data);
	memory_region *m_cart_rom;

	void mwa_bank1(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mwa_bank2(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mwa_bank3(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mwa_bank4(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t mra_bank1(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t mra_bank2(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t mra_bank3(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t mra_bank4(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
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
