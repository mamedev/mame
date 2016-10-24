// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
/***************************************************************************

    Jailbreak

***************************************************************************/

#include "sound/vlm5030.h"

#define MASTER_CLOCK        XTAL_18_432MHz
#define VOICE_CLOCK         XTAL_3_579545MHz

class jailbrek_state : public driver_device
{
public:
	jailbrek_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_scroll_x(*this, "scroll_x"),
		m_scroll_dir(*this, "scroll_dir"),
		m_maincpu(*this, "maincpu"),
		m_vlm(*this, "vlm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_scroll_x;
	required_shared_ptr<uint8_t> m_scroll_dir;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<vlm5030_device> m_vlm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	/* video-related */
	tilemap_t      *m_bg_tilemap;

	/* misc */
	uint8_t        m_irq_enable;
	uint8_t        m_nmi_enable;
	void ctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void coin_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void colorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t speech_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void speech_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void palette_init_jailbrek(palette_device &palette);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void interrupt(device_t &device);
	void interrupt_nmi(device_t &device);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
};
