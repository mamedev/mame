// license:BSD-3-Clause
// copyright-holders:Tatsuyuki Satoh
#include "sound/msm5205.h"

class appoooh_state : public driver_device
{
public:
	appoooh_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_fg_videoram(*this, "fg_videoram"),
		m_fg_colorram(*this, "fg_colorram"),
		m_spriteram_2(*this, "spriteram_2"),
		m_bg_videoram(*this, "bg_videoram"),
		m_bg_colorram(*this, "bg_colorram"),
		m_decrypted_opcodes(*this, "decrypted_opcodes"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_msm(*this, "msm") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_fg_videoram;
	required_shared_ptr<uint8_t> m_fg_colorram;
	required_shared_ptr<uint8_t> m_spriteram_2;
	required_shared_ptr<uint8_t> m_bg_videoram;
	required_shared_ptr<uint8_t> m_bg_colorram;
	optional_shared_ptr<uint8_t> m_decrypted_opcodes;

	/* video-related */
	tilemap_t  *m_fg_tilemap;
	tilemap_t  *m_bg_tilemap;
	int m_scroll_x;
	int m_priority;

	/* sound-related */
	uint32_t   m_adpcm_data;
	uint32_t   m_adpcm_address;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<msm5205_device> m_msm;

	uint8_t m_nmi_mask;
	void adpcm_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void scroll_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void fg_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void fg_colorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bg_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bg_colorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void out_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_robowresb();
	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	void video_start_appoooh();
	void palette_init_appoooh(palette_device &palette);
	void palette_init_robowres(palette_device &palette);
	uint32_t screen_update_appoooh(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_robowres(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vblank_irq(device_t &device);
	void appoooh_draw_sprites( bitmap_ind16 &dest_bmp, const rectangle &cliprect, gfx_element *gfx, uint8_t *sprite );
	void robowres_draw_sprites( bitmap_ind16 &dest_bmp, const rectangle &cliprect, gfx_element *gfx, uint8_t *sprite );
	void adpcm_int(int state);
};

#define CHR1_OFST   0x00  /* palette page of char set #1 */
#define CHR2_OFST   0x10  /* palette page of char set #2 */
