class taotaido_state : public driver_device
{
public:
	taotaido_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_spriteram(*this, "spriteram"),
		m_spriteram2(*this, "spriteram2"),
		m_scrollram(*this, "scrollram"),
		m_bgram(*this, "bgram"){ }

	int m_pending_command;
	required_shared_ptr<UINT16> m_spriteram;
	required_shared_ptr<UINT16> m_spriteram2;
	required_shared_ptr<UINT16> m_scrollram;
	required_shared_ptr<UINT16> m_bgram;
	UINT16 m_sprite_character_bank_select[8];
	UINT16 m_video_bank_select[8];
	tilemap_t *m_bg_tilemap;
	UINT16 *m_spriteram_old;
	UINT16 *m_spriteram_older;
	UINT16 *m_spriteram2_old;
	UINT16 *m_spriteram2_older;
	DECLARE_READ16_MEMBER(pending_command_r);
	DECLARE_WRITE16_MEMBER(sound_command_w);
	DECLARE_WRITE8_MEMBER(pending_command_clear_w);
	DECLARE_WRITE8_MEMBER(taotaido_sh_bankswitch_w);
	DECLARE_WRITE16_MEMBER(taotaido_sprite_character_bank_select_w);
	DECLARE_WRITE16_MEMBER(taotaido_tileregs_w);
	DECLARE_WRITE16_MEMBER(taotaido_bgvideoram_w);
	TILE_GET_INFO_MEMBER(taotaido_bg_tile_info);
	TILEMAP_MAPPER_MEMBER(taotaido_tilemap_scan_rows);
	virtual void video_start();
};


/*----------- defined in video/taotaido.c -----------*/


SCREEN_UPDATE_IND16( taotaido );
SCREEN_VBLANK( taotaido );
