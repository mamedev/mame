#include "sound/okim6295.h"
#include "machine/eeprom.h"

class playmark_state : public driver_device
{
public:
	playmark_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_bgvideoram(*this, "bgvideoram"),
		m_videoram1(*this, "videoram1"),
		m_videoram2(*this, "videoram2"),
		m_videoram3(*this, "videoram3"),
		m_spriteram(*this, "spriteram"),
		m_rowscroll(*this, "rowscroll"){ }

	/* memory pointers */
	optional_shared_ptr<UINT16> m_bgvideoram;
	required_shared_ptr<UINT16> m_videoram1;
	optional_shared_ptr<UINT16> m_videoram2;
	optional_shared_ptr<UINT16> m_videoram3;
	required_shared_ptr<UINT16> m_spriteram;
	optional_shared_ptr<UINT16> m_rowscroll;
//      UINT16 *     m_paletteram;    // currently this uses generic palette handling

	/* video-related */
	tilemap_t   *m_tx_tilemap;
	tilemap_t   *m_fg_tilemap;
	tilemap_t   *m_bg_tilemap;
	int         m_bgscrollx;
	int         m_bgscrolly;
	int         m_bg_enable;
	int         m_bg_full_size;
	int         m_fgscrollx;
	int         m_fg_rowscroll_enable;

	int         m_xoffset;
	int         m_yoffset;
	int         m_txt_tile_offset;
	int         m_pri_masks[3];
	UINT16      m_scroll[7];

	/* powerbal-specific */
	int         m_tilebank;
	int         m_bg_yoffset;

	/* misc */
	UINT16      m_snd_command;
	UINT16      m_snd_flag;
	UINT8       m_oki_control;
	UINT8       m_oki_command;
	int         m_old_oki_bank;

	/* devices */
	okim6295_device *m_oki;
	eeprom_device *m_eeprom;
	DECLARE_WRITE16_MEMBER(coinctrl_w);
	DECLARE_WRITE16_MEMBER(wbeachvl_coin_eeprom_w);
	DECLARE_WRITE16_MEMBER(hotmind_coin_eeprom_w);
	DECLARE_WRITE16_MEMBER(hrdtimes_coin_w);
	DECLARE_WRITE16_MEMBER(playmark_snd_command_w);
	DECLARE_READ8_MEMBER(playmark_snd_command_r);
	DECLARE_READ8_MEMBER(playmark_snd_flag_r);
	DECLARE_WRITE8_MEMBER(playmark_oki_w);
	DECLARE_WRITE8_MEMBER(playmark_snd_control_w);
	DECLARE_READ8_MEMBER(PIC16C5X_T0_clk_r);
	DECLARE_WRITE16_MEMBER(wbeachvl_txvideoram_w);
	DECLARE_WRITE16_MEMBER(wbeachvl_fgvideoram_w);
	DECLARE_WRITE16_MEMBER(wbeachvl_bgvideoram_w);
	DECLARE_WRITE16_MEMBER(hrdtimes_txvideoram_w);
	DECLARE_WRITE16_MEMBER(hrdtimes_fgvideoram_w);
	DECLARE_WRITE16_MEMBER(hrdtimes_bgvideoram_w);
	DECLARE_WRITE16_MEMBER(bigtwin_paletteram_w);
	DECLARE_WRITE16_MEMBER(bigtwin_scroll_w);
	DECLARE_WRITE16_MEMBER(wbeachvl_scroll_w);
	DECLARE_WRITE16_MEMBER(excelsr_scroll_w);
	DECLARE_WRITE16_MEMBER(hrdtimes_scroll_w);
	DECLARE_WRITE8_MEMBER(playmark_oki_banking_w);
	DECLARE_DRIVER_INIT(bigtwin);
	DECLARE_DRIVER_INIT(powerbal);
	DECLARE_DRIVER_INIT(magicstk);
	TILE_GET_INFO_MEMBER(bigtwin_get_tx_tile_info);
	TILE_GET_INFO_MEMBER(bigtwin_get_fg_tile_info);
	TILE_GET_INFO_MEMBER(wbeachvl_get_tx_tile_info);
	TILE_GET_INFO_MEMBER(wbeachvl_get_fg_tile_info);
	TILE_GET_INFO_MEMBER(wbeachvl_get_bg_tile_info);
	TILE_GET_INFO_MEMBER(hrdtimes_get_tx_tile_info);
	TILE_GET_INFO_MEMBER(bigtwinb_get_tx_tile_info);
	TILE_GET_INFO_MEMBER(hrdtimes_get_fg_tile_info);
	TILE_GET_INFO_MEMBER(hrdtimes_get_bg_tile_info);
};

/*----------- defined in video/playmark.c -----------*/


VIDEO_START( bigtwin );
VIDEO_START( bigtwinb );
VIDEO_START( wbeachvl );
VIDEO_START( excelsr );
VIDEO_START( hotmind );
VIDEO_START( hrdtimes );

SCREEN_UPDATE_IND16( bigtwin );
SCREEN_UPDATE_IND16( bigtwinb );
SCREEN_UPDATE_IND16( wbeachvl );
SCREEN_UPDATE_IND16( excelsr );
SCREEN_UPDATE_IND16( hrdtimes );
