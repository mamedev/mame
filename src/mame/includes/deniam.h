// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*************************************************************************

    Deniam games

*************************************************************************/
#include "sound/okim6295.h"

class deniam_state : public driver_device
{
public:
	deniam_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_textram(*this, "textram"),
		m_spriteram(*this, "spriteram"),
		m_paletteram(*this, "paletteram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_oki(*this, "oki"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<UINT16> m_videoram;
	required_shared_ptr<UINT16> m_textram;
	required_shared_ptr<UINT16> m_spriteram;
	required_shared_ptr<UINT16> m_paletteram;

	/* video-related */
	tilemap_t        *m_fg_tilemap;
	tilemap_t        *m_bg_tilemap;
	tilemap_t        *m_tx_tilemap;
	int            m_display_enable;
	int            m_bg_scrollx_offs;
	int            m_bg_scrolly_offs;
	int            m_fg_scrollx_offs;
	int            m_fg_scrolly_offs;
	int            m_bg_scrollx_reg;
	int            m_bg_scrolly_reg;
	int            m_bg_page_reg;
	int            m_fg_scrollx_reg;
	int            m_fg_scrolly_reg;
	int            m_fg_page_reg;
	int            m_bg_page[4];
	int            m_fg_page[4];
	UINT16         m_coinctrl;

	/* devices */
	DECLARE_WRITE16_MEMBER(sound_command_w);
	DECLARE_WRITE16_MEMBER(deniam_irq_ack_w);
	DECLARE_WRITE16_MEMBER(deniam_videoram_w);
	DECLARE_WRITE16_MEMBER(deniam_textram_w);
	DECLARE_WRITE16_MEMBER(deniam_palette_w);
	DECLARE_READ16_MEMBER(deniam_coinctrl_r);
	DECLARE_WRITE16_MEMBER(deniam_coinctrl_w);
	DECLARE_WRITE8_MEMBER(deniam16b_oki_rom_bank_w);
	DECLARE_WRITE16_MEMBER(deniam16c_oki_rom_bank_w);
	DECLARE_DRIVER_INIT(karianx);
	DECLARE_DRIVER_INIT(logicpro);
	TILEMAP_MAPPER_MEMBER(scan_pages);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_deniam(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void deniam_common_init(  );
	void draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect );
	void set_bg_page( int page, int value );
	void set_fg_page( int page, int value );
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu; // system 16c does not have sound CPU
	required_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};
