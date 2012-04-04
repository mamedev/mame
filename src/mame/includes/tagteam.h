class tagteam_state : public driver_device
{
public:
	tagteam_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_videoram;
	UINT8 *m_colorram;
	int m_palettebank;
	tilemap_t *m_bg_tilemap;

	UINT8 m_sound_nmi_mask;
	DECLARE_WRITE8_MEMBER(sound_command_w);
	DECLARE_WRITE8_MEMBER(irq_clear_w);
	DECLARE_WRITE8_MEMBER(sound_nmi_mask_w);
};


/*----------- defined in video/tagteam.c -----------*/

WRITE8_HANDLER( tagteam_videoram_w );
WRITE8_HANDLER( tagteam_colorram_w );
READ8_HANDLER( tagteam_mirrorvideoram_r );
WRITE8_HANDLER( tagteam_mirrorvideoram_w );
READ8_HANDLER( tagteam_mirrorcolorram_r );
WRITE8_HANDLER( tagteam_mirrorcolorram_w );
WRITE8_HANDLER( tagteam_control_w );
WRITE8_HANDLER( tagteam_flipscreen_w );

PALETTE_INIT( tagteam );
VIDEO_START( tagteam );
SCREEN_UPDATE_IND16( tagteam );
