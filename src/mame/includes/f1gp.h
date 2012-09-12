
class f1gp_state : public driver_device
{
public:
	f1gp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_sharedram(*this, "sharedram"),
		m_spr1vram(*this, "spr1vram"),
		m_spr2vram(*this, "spr2vram"),
		m_spr1cgram(*this, "spr1cgram"),
		m_spr2cgram(*this, "spr2cgram"),
		m_fgvideoram(*this, "fgvideoram"),
		m_rozvideoram(*this, "rozvideoram"),
		m_sprcgram(*this, "sprcgram"),
		m_spritelist(*this, "spritelist"),
		m_spriteram(*this, "spriteram"),
		m_fgregs(*this, "fgregs"),
		m_rozregs(*this, "rozregs"){ }

	/* memory pointers */
	required_shared_ptr<UINT16> m_sharedram;
	optional_shared_ptr<UINT16> m_spr1vram;
	optional_shared_ptr<UINT16> m_spr2vram;
	optional_shared_ptr<UINT16> m_spr1cgram;
	optional_shared_ptr<UINT16> m_spr2cgram;
	required_shared_ptr<UINT16> m_fgvideoram;
	required_shared_ptr<UINT16> m_rozvideoram;
	optional_shared_ptr<UINT16> m_sprcgram;
	optional_shared_ptr<UINT16> m_spritelist;
	optional_shared_ptr<UINT16> m_spriteram;
	optional_shared_ptr<UINT16> m_fgregs;
	optional_shared_ptr<UINT16> m_rozregs;
	UINT16 *  m_zoomdata;
//      UINT16 *  m_paletteram;    // currently this uses generic palette handling

	/* video-related */
	tilemap_t   *m_fg_tilemap;
	tilemap_t   *m_roz_tilemap;
	int       m_roz_bank;
	int       m_flipscreen;
	int       m_gfxctrl;
	int       m_scroll[2];

	/* misc */
	int       m_pending_command;

	/* devices */
	cpu_device *m_audiocpu;
	device_t *m_k053936;
	DECLARE_READ16_MEMBER(sharedram_r);
	DECLARE_WRITE16_MEMBER(sharedram_w);
	DECLARE_READ16_MEMBER(extrarom_r);
	DECLARE_READ16_MEMBER(extrarom2_r);
	DECLARE_WRITE8_MEMBER(f1gp_sh_bankswitch_w);
	DECLARE_WRITE16_MEMBER(sound_command_w);
	DECLARE_READ16_MEMBER(command_pending_r);
	DECLARE_WRITE8_MEMBER(pending_command_clear_w);
	DECLARE_WRITE16_MEMBER(f1gpb_misc_w);
	DECLARE_READ16_MEMBER(f1gp_zoomdata_r);
	DECLARE_WRITE16_MEMBER(f1gp_zoomdata_w);
	DECLARE_READ16_MEMBER(f1gp_rozvideoram_r);
	DECLARE_WRITE16_MEMBER(f1gp_rozvideoram_w);
	DECLARE_WRITE16_MEMBER(f1gp_fgvideoram_w);
	DECLARE_WRITE16_MEMBER(f1gp_fgscroll_w);
	DECLARE_WRITE16_MEMBER(f1gp_gfxctrl_w);
	DECLARE_WRITE16_MEMBER(f1gp2_gfxctrl_w);
	TILE_GET_INFO_MEMBER(f1gp_get_roz_tile_info);
	TILE_GET_INFO_MEMBER(f1gp2_get_roz_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
};

/*----------- defined in video/f1gp.c -----------*/


VIDEO_START( f1gp );
VIDEO_START( f1gpb );
VIDEO_START( f1gp2 );
SCREEN_UPDATE_IND16( f1gp );
SCREEN_UPDATE_IND16( f1gpb );
SCREEN_UPDATE_IND16( f1gp2 );
