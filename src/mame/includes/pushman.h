/*************************************************************************

    Pushman

*************************************************************************/

class pushman_state : public driver_device
{
public:
	pushman_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT16 *   m_videoram;
	UINT16 *   m_spriteram;
//  UINT16 *   m_paletteram;    // currently this uses generic palette handling

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	tilemap_t  *m_tx_tilemap;
	UINT16     m_control[2];

	/* misc */
	UINT8      m_shared_ram[8];
	UINT16     m_latch;
	UINT16     m_new_latch;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	device_t *m_mcu;
	DECLARE_WRITE16_MEMBER(pushman_flipscreen_w);
	DECLARE_WRITE16_MEMBER(pushman_control_w);
	DECLARE_READ16_MEMBER(pushman_68705_r);
	DECLARE_WRITE16_MEMBER(pushman_68705_w);
	DECLARE_READ16_MEMBER(bballs_68705_r);
	DECLARE_WRITE16_MEMBER(bballs_68705_w);
	DECLARE_READ8_MEMBER(pushman_68000_r);
	DECLARE_WRITE8_MEMBER(pushman_68000_w);
	DECLARE_WRITE16_MEMBER(pushman_scroll_w);
	DECLARE_WRITE16_MEMBER(pushman_videoram_w);
};


/*----------- defined in video/pushman.c -----------*/


VIDEO_START( pushman );

SCREEN_UPDATE_IND16( pushman );
