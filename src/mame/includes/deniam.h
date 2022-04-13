// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria

/*************************************************************************

    Deniam games

*************************************************************************/

#include "machine/gen_latch.h"
#include "sound/okim6295.h"
#include "emupal.h"
#include "tilemap.h"

class deniam_state : public driver_device
{
public:
	deniam_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_textram(*this, "textram"),
		m_spriteram(*this, "spriteram"),
		m_spritegfx(*this, "spritegfx"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_oki(*this, "oki"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch")
	{ }

	void deniam16c(machine_config &config);
	void deniam16b(machine_config &config);

	void init_karianx();
	void init_logicpro();

private:
	/* memory pointers */
	required_shared_ptr<u16> m_videoram;
	required_shared_ptr<u16> m_textram;
	required_shared_ptr<u16> m_spriteram;

	required_memory_region m_spritegfx;

	/* video-related */
	tilemap_t      *m_fg_tilemap = nullptr;
	tilemap_t      *m_bg_tilemap = nullptr;
	tilemap_t      *m_tx_tilemap = nullptr;
	int            m_display_enable = 0;
	int            m_bg_scrollx_offs = 0;
	int            m_bg_scrolly_offs = 0;
	int            m_fg_scrollx_offs = 0;
	int            m_fg_scrolly_offs = 0;
	int            m_bg_scrollx_reg = 0;
	int            m_bg_scrolly_reg = 0;
	int            m_bg_page_reg = 0;
	int            m_fg_scrollx_reg = 0;
	int            m_fg_scrolly_reg = 0;
	int            m_fg_page_reg = 0;
	int            m_bg_page[4]{};
	int            m_fg_page[4]{};
	u16            m_coinctrl = 0;

	/* devices */
	void irq_ack_w(u16 data);
	void videoram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void textram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 coinctrl_r();
	void coinctrl_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void deniam16b_oki_rom_bank_w(u8 data);
	void deniam16c_oki_rom_bank_w(u8 data);
	TILEMAP_MAPPER_MEMBER(scan_pages);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void deniam_common_init(  );
	void draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect );
	void set_bg_page( int page, int value );
	void set_fg_page( int page, int value );
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu; // system 16c does not have sound CPU
	required_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_device<generic_latch_8_device> m_soundlatch;
	void deniam16b_map(address_map &map);
	void deniam16c_map(address_map &map);
	void sound_io_map(address_map &map);
	void sound_map(address_map &map);
};
