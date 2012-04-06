class taotaido_state : public driver_device
{
public:
	taotaido_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	int m_pending_command;
	UINT16 *m_spriteram;
	UINT16 *m_spriteram2;
	UINT16 *m_scrollram;
	UINT16 *m_bgram;
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
};


/*----------- defined in video/taotaido.c -----------*/

VIDEO_START( taotaido );
SCREEN_UPDATE_IND16( taotaido );
SCREEN_VBLANK( taotaido );
