/*************************************************************************

    Yun Sung 8 Bit Games

*************************************************************************/

class yunsung8_state : public driver_device
{
public:
	yunsung8_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* video-related */
	tilemap_t     *m_tilemap_0;
	tilemap_t     *m_tilemap_1;
	UINT8       *m_videoram_0;
	UINT8       *m_videoram_1;
	int         m_layers_ctrl;
	int         m_videobank;

	/* misc */
	int         m_adpcm;
	int         m_toggle;

	/* devices */
	device_t *m_audiocpu;

	/* memory */
	UINT8      m_videoram[0x4000];
	DECLARE_WRITE8_MEMBER(yunsung8_bankswitch_w);
	DECLARE_WRITE8_MEMBER(yunsung8_adpcm_w);
	DECLARE_WRITE8_MEMBER(yunsung8_videobank_w);
	DECLARE_READ8_MEMBER(yunsung8_videoram_r);
	DECLARE_WRITE8_MEMBER(yunsung8_videoram_w);
	DECLARE_WRITE8_MEMBER(yunsung8_flipscreen_w);
};


/*----------- defined in video/yunsung8.c -----------*/


VIDEO_START( yunsung8 );
SCREEN_UPDATE_IND16( yunsung8 );
