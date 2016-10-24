// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*************************************************************************

    Tail to Nose / Super Formula

*************************************************************************/

#include "machine/gen_latch.h"
#include "video/k051316.h"

class tail2nos_state : public driver_device
{
public:
	tail2nos_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_txvideoram(*this, "txvideoram"),
		m_spriteram(*this, "spriteram"),
		m_zoomram(*this, "k051316"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_k051316(*this, "k051316"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch") { }

	/* memory pointers */
	required_shared_ptr<uint16_t> m_txvideoram;
	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_zoomram;

	/* video-related */
	tilemap_t   *m_tx_tilemap;
	int         m_txbank;
	int         m_txpalette;
	int         m_video_enable;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<k051316_device> m_k051316;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	void sound_command_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void tail2nos_txvideoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void tail2nos_zoomdata_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void tail2nos_gfxbank_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void sound_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_tail2nos(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void tail2nos_postload();
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	K051316_CB_MEMBER(zoom_callback);
};
