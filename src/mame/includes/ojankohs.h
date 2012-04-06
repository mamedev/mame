/*************************************************************************

    Ojanko High School & other Video System mahjong series

*************************************************************************/

class ojankohs_state : public driver_device
{
public:
	ojankohs_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *   m_videoram;
	UINT8 *   m_colorram;
	UINT8 *   m_paletteram;

	/* video-related */
	tilemap_t  *m_tilemap;
	bitmap_ind16 m_tmpbitmap;
	int       m_gfxreg;
	int       m_flipscreen;
	int       m_flipscreen_old;
	int       m_scrollx;
	int       m_scrolly;
	int       m_screen_refresh;

	/* misc */
	int       m_portselect;
	int       m_adpcm_reset;
	int       m_adpcm_data;
	int       m_vclk_left;

	/* devices */
	device_t *m_maincpu;
	device_t *m_msm;
	DECLARE_WRITE8_MEMBER(ojankohs_rombank_w);
	DECLARE_WRITE8_MEMBER(ojankoy_rombank_w);
	DECLARE_WRITE8_MEMBER(ojankohs_msm5205_w);
	DECLARE_WRITE8_MEMBER(ojankoc_ctrl_w);
	DECLARE_WRITE8_MEMBER(ojankohs_portselect_w);
	DECLARE_READ8_MEMBER(ojankohs_keymatrix_r);
	DECLARE_READ8_MEMBER(ojankoc_keymatrix_r);
	DECLARE_READ8_MEMBER(ccasino_dipsw3_r);
	DECLARE_READ8_MEMBER(ccasino_dipsw4_r);
	DECLARE_WRITE8_MEMBER(ojankoy_coinctr_w);
	DECLARE_WRITE8_MEMBER(ccasino_coinctr_w);
	DECLARE_WRITE8_MEMBER(ojankohs_palette_w);
	DECLARE_WRITE8_MEMBER(ccasino_palette_w);
	DECLARE_WRITE8_MEMBER(ojankoc_palette_w);
	DECLARE_WRITE8_MEMBER(ojankohs_videoram_w);
	DECLARE_WRITE8_MEMBER(ojankohs_colorram_w);
	DECLARE_WRITE8_MEMBER(ojankohs_gfxreg_w);
	DECLARE_WRITE8_MEMBER(ojankohs_flipscreen_w);
	DECLARE_WRITE8_MEMBER(ojankoc_videoram_w);
};


/*----------- defined in video/ojankohs.c -----------*/


PALETTE_INIT( ojankoy );

VIDEO_START( ojankohs );
VIDEO_START( ojankoy );
VIDEO_START( ojankoc );

SCREEN_UPDATE_IND16( ojankohs );
SCREEN_UPDATE_IND16( ojankoc );

void ojankoc_flipscreen(address_space *space, int data);

