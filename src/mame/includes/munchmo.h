// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
/*************************************************************************

    Munch Mobile

*************************************************************************/

#include "machine/gen_latch.h"

class munchmo_state : public driver_device
{
public:
	munchmo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_sprite_xpos(*this, "sprite_xpos"),
		m_sprite_tile(*this, "sprite_tile"),
		m_sprite_attr(*this, "sprite_attr"),
		m_videoram(*this, "videoram"),
		m_status_vram(*this, "status_vram"),
		m_vreg(*this, "vreg"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_sprite_xpos;
	required_shared_ptr<uint8_t> m_sprite_tile;
	required_shared_ptr<uint8_t> m_sprite_attr;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_status_vram;
	required_shared_ptr<uint8_t> m_vreg;

	/* video-related */
	std::unique_ptr<bitmap_ind16> m_tmpbitmap;
	int          m_palette_bank;
	int          m_flipscreen;

	/* misc */
	int          m_nmi_enable;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	void mnchmobl_nmi_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mnchmobl_soundlatch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sound_nmi_ack_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mnchmobl_palette_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mnchmobl_flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t munchmo_ay1reset_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t munchmo_ay2reset_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void palette_init_munchmo(palette_device &palette);
	uint32_t screen_update_mnchmobl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void mnchmobl_vblank_irq(device_t &device);
	void mnchmobl_sound_irq(device_t &device);
	void draw_status( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void draw_background( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
};
