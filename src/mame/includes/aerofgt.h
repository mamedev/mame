
class aerofgt_state : public driver_device
{
public:
	aerofgt_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_bg1videoram(*this, "bg1videoram"),
		m_bg2videoram(*this, "bg2videoram"),
		m_rasterram(*this, "rasterram"),
		m_bitmapram(*this, "bitmapram"),
		m_spriteram1(*this, "spriteram1"),
		m_spriteram2(*this, "spriteram2"),
		m_spriteram3(*this, "spriteram3"),
		m_tx_tilemap_ram(*this, "tx_tilemap_ram"){ }

	/* memory pointers */
	required_shared_ptr<UINT16> m_bg1videoram;
	optional_shared_ptr<UINT16> m_bg2videoram;
	optional_shared_ptr<UINT16> m_rasterram;
	optional_shared_ptr<UINT16> m_bitmapram;
	optional_shared_ptr<UINT16> m_spriteram1;
	optional_shared_ptr<UINT16> m_spriteram2;
	required_shared_ptr<UINT16> m_spriteram3;
	optional_shared_ptr<UINT16> m_tx_tilemap_ram;
//  UINT16 *  m_paletteram;   // currently this uses generic palette handling

	/* video-related */
	tilemap_t   *m_bg1_tilemap;
	tilemap_t   *m_bg2_tilemap;
	UINT8     m_gfxbank[8];
	UINT16    m_bank[4];
	UINT16    m_bg1scrollx;
	UINT16    m_bg1scrolly;
	UINT16    m_bg2scrollx;
	UINT16    m_bg2scrolly;
	UINT16    m_wbbc97_bitmap_enable;
	int       m_charpalettebank;
	int       m_spritepalettebank;
	int       m_sprite_gfx;
	int       m_spikes91_lookup;

	/* misc */
	int       m_pending_command;

	/* devices */
	cpu_device *m_audiocpu;
	DECLARE_WRITE16_MEMBER(sound_command_w);
	DECLARE_WRITE16_MEMBER(turbofrc_sound_command_w);
	DECLARE_WRITE16_MEMBER(aerfboot_soundlatch_w);
	DECLARE_READ16_MEMBER(pending_command_r);
	DECLARE_WRITE8_MEMBER(pending_command_clear_w);
	DECLARE_WRITE8_MEMBER(aerofgt_sh_bankswitch_w);
	DECLARE_WRITE8_MEMBER(aerfboot_okim6295_banking_w);
	DECLARE_WRITE16_MEMBER(aerofgt_bg1videoram_w);
	DECLARE_WRITE16_MEMBER(aerofgt_bg2videoram_w);
	DECLARE_WRITE16_MEMBER(pspikes_gfxbank_w);
	DECLARE_WRITE16_MEMBER(pspikesb_gfxbank_w);
	DECLARE_WRITE16_MEMBER(spikes91_lookup_w);
	DECLARE_WRITE16_MEMBER(karatblz_gfxbank_w);
	DECLARE_WRITE16_MEMBER(spinlbrk_gfxbank_w);
	DECLARE_WRITE16_MEMBER(turbofrc_gfxbank_w);
	DECLARE_WRITE16_MEMBER(aerofgt_gfxbank_w);
	DECLARE_WRITE16_MEMBER(aerofgt_bg1scrollx_w);
	DECLARE_WRITE16_MEMBER(aerofgt_bg1scrolly_w);
	DECLARE_WRITE16_MEMBER(aerofgt_bg2scrollx_w);
	DECLARE_WRITE16_MEMBER(aerofgt_bg2scrolly_w);
	DECLARE_WRITE16_MEMBER(pspikes_palette_bank_w);
	DECLARE_WRITE16_MEMBER(wbbc97_bitmap_enable_w);
	DECLARE_WRITE16_MEMBER(pspikesb_oki_banking_w);
	DECLARE_WRITE16_MEMBER(aerfboo2_okim6295_banking_w);
	TILE_GET_INFO_MEMBER(get_pspikes_tile_info);
	TILE_GET_INFO_MEMBER(karatblz_bg1_tile_info);
	TILE_GET_INFO_MEMBER(karatblz_bg2_tile_info);
	TILE_GET_INFO_MEMBER(spinlbrk_bg1_tile_info);
	TILE_GET_INFO_MEMBER(get_bg1_tile_info);
	TILE_GET_INFO_MEMBER(get_bg2_tile_info);
	DECLARE_MACHINE_START(aerofgt);
	DECLARE_MACHINE_RESET(aerofgt);
	DECLARE_VIDEO_START(pspikes);
	DECLARE_MACHINE_START(common);
	DECLARE_MACHINE_RESET(common);
	DECLARE_VIDEO_START(karatblz);
	DECLARE_VIDEO_START(spinlbrk);
	DECLARE_VIDEO_START(turbofrc);
	DECLARE_VIDEO_START(wbbc97);
	UINT32 screen_update_pspikes(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_spikes91(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_pspikesb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_karatblz(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_spinlbrk(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_turbofrc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_aerofgt(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_aerfboot(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_aerfboo2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_wbbc97(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};
