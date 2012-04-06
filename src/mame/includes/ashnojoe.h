/*************************************************************************

    Success Joe / Ashita no Joe

*************************************************************************/

class ashnojoe_state : public driver_device
{
public:
	ashnojoe_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT16 *    m_tileram;
	UINT16 *    m_tileram_1;
	UINT16 *    m_tileram_2;
	UINT16 *    m_tileram_3;
	UINT16 *    m_tileram_4;
	UINT16 *    m_tileram_5;
	UINT16 *    m_tileram_6;
	UINT16 *    m_tileram_7;
	UINT16 *    m_tilemap_reg;
//  UINT16 *    m_paletteram; // currently this uses generic palette handling

	/* video-related */
	tilemap_t     *m_joetilemap;
	tilemap_t     *m_joetilemap2;
	tilemap_t     *m_joetilemap3;
	tilemap_t     *m_joetilemap4;
	tilemap_t     *m_joetilemap5;
	tilemap_t     *m_joetilemap6;
	tilemap_t     *m_joetilemap7;

	/* sound-related */
	UINT8       m_adpcm_byte;
	int         m_soundlatch_status;
	int         m_msm5205_vclk_toggle;

	/* devices */
	device_t *m_audiocpu;
	DECLARE_READ16_MEMBER(fake_4a00a_r);
	DECLARE_WRITE16_MEMBER(ashnojoe_soundlatch_w);
	DECLARE_WRITE8_MEMBER(adpcm_w);
	DECLARE_READ8_MEMBER(sound_latch_r);
	DECLARE_READ8_MEMBER(sound_latch_status_r);
	DECLARE_WRITE16_MEMBER(ashnojoe_tileram_w);
	DECLARE_WRITE16_MEMBER(ashnojoe_tileram2_w);
	DECLARE_WRITE16_MEMBER(ashnojoe_tileram3_w);
	DECLARE_WRITE16_MEMBER(ashnojoe_tileram4_w);
	DECLARE_WRITE16_MEMBER(ashnojoe_tileram5_w);
	DECLARE_WRITE16_MEMBER(ashnojoe_tileram6_w);
	DECLARE_WRITE16_MEMBER(ashnojoe_tileram7_w);
	DECLARE_WRITE16_MEMBER(joe_tilemaps_xscroll_w);
	DECLARE_WRITE16_MEMBER(joe_tilemaps_yscroll_w);
};


/*----------- defined in video/ashnojoe.c -----------*/


VIDEO_START( ashnojoe );
SCREEN_UPDATE_IND16( ashnojoe );
