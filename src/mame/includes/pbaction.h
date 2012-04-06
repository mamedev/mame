/*************************************************************************

    Pinball Action

*************************************************************************/

class pbaction_state : public driver_device
{
public:
	pbaction_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *    m_videoram;
	UINT8 *    m_videoram2;
	UINT8 *    m_colorram;
	UINT8 *    m_colorram2;
	UINT8 *    m_work_ram;
	UINT8 *    m_spriteram;
//  UINT8 *    m_paletteram;    // currently this uses generic palette handling
	size_t     m_spriteram_size;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	tilemap_t  *m_fg_tilemap;
	int        m_scroll;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;

	UINT8      m_nmi_mask;
	DECLARE_WRITE8_MEMBER(pbaction_sh_command_w);
	DECLARE_WRITE8_MEMBER(nmi_mask_w);
	DECLARE_READ8_MEMBER(pbactio3_prot_kludge_r);
	DECLARE_WRITE8_MEMBER(pbaction_videoram_w);
	DECLARE_WRITE8_MEMBER(pbaction_colorram_w);
	DECLARE_WRITE8_MEMBER(pbaction_videoram2_w);
	DECLARE_WRITE8_MEMBER(pbaction_colorram2_w);
	DECLARE_WRITE8_MEMBER(pbaction_scroll_w);
	DECLARE_WRITE8_MEMBER(pbaction_flipscreen_w);
};


/*----------- defined in video/pbaction.c -----------*/


extern VIDEO_START( pbaction );
extern SCREEN_UPDATE_IND16( pbaction );
