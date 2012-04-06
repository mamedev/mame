/***************************************************************************

    Battle Lane Vol. 5

***************************************************************************/

class battlane_state : public driver_device
{
public:
	battlane_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *     m_tileram;
	UINT8 *     m_spriteram;

	/* video-related */
	tilemap_t     *m_bg_tilemap;
	bitmap_ind8 m_screen_bitmap;
	int         m_video_ctrl;
	int         m_cpu_control;	/* CPU interrupt control register */

	/* devices */
	device_t *m_maincpu;
	device_t *m_subcpu;
	DECLARE_WRITE8_MEMBER(battlane_cpu_command_w);
	DECLARE_WRITE8_MEMBER(battlane_palette_w);
	DECLARE_WRITE8_MEMBER(battlane_scrollx_w);
	DECLARE_WRITE8_MEMBER(battlane_scrolly_w);
	DECLARE_WRITE8_MEMBER(battlane_tileram_w);
	DECLARE_WRITE8_MEMBER(battlane_spriteram_w);
	DECLARE_WRITE8_MEMBER(battlane_bitmap_w);
	DECLARE_WRITE8_MEMBER(battlane_video_ctrl_w);
};


/*----------- defined in video/battlane.c -----------*/


VIDEO_START( battlane );
SCREEN_UPDATE_IND16( battlane );
