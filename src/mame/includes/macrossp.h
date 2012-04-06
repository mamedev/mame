/*************************************************************************

    Macross Plus

*************************************************************************/

class macrossp_state : public driver_device
{
public:
	macrossp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT32 *         m_mainram;
	UINT32 *         m_scra_videoram;
	UINT32 *         m_scra_videoregs;
	UINT32 *         m_scrb_videoram;
	UINT32 *         m_scrb_videoregs;
	UINT32 *         m_scrc_videoram;
	UINT32 *         m_scrc_videoregs;
	UINT32 *         m_text_videoram;
	UINT32 *         m_text_videoregs;
	UINT32 *         m_spriteram;
	UINT32 *         m_spriteram_old;
	UINT32 *         m_spriteram_old2;
	UINT32 *         m_paletteram;
	size_t           m_spriteram_size;

	/* video-related */
	tilemap_t  *m_scra_tilemap;
	tilemap_t  *m_scrb_tilemap;
	tilemap_t  *m_scrc_tilemap;
	tilemap_t  *m_text_tilemap;

	/* misc */
	int              m_sndpending;
	int              m_snd_toggle;
	INT32            m_fade_effect;
	INT32			 m_old_fade;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	DECLARE_WRITE32_MEMBER(paletteram32_macrossp_w);
	DECLARE_READ32_MEMBER(macrossp_soundstatus_r);
	DECLARE_WRITE32_MEMBER(macrossp_soundcmd_w);
	DECLARE_READ16_MEMBER(macrossp_soundcmd_r);
	DECLARE_WRITE32_MEMBER(macrossp_palette_fade_w);
	DECLARE_WRITE32_MEMBER(macrossp_speedup_w);
	DECLARE_WRITE32_MEMBER(quizmoon_speedup_w);
	DECLARE_WRITE32_MEMBER(macrossp_scra_videoram_w);
	DECLARE_WRITE32_MEMBER(macrossp_scrb_videoram_w);
	DECLARE_WRITE32_MEMBER(macrossp_scrc_videoram_w);
	DECLARE_WRITE32_MEMBER(macrossp_text_videoram_w);
};

/*----------- defined in video/macrossp.c -----------*/


VIDEO_START(macrossp);
SCREEN_UPDATE_RGB32(macrossp);
SCREEN_VBLANK(macrossp);
