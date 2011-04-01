/*************************************************************************

    Fast Lane

*************************************************************************/

class fastlane_state : public driver_device
{
public:
	fastlane_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *    m_videoram1;
	UINT8 *    m_videoram2;
	UINT8 *    m_paletteram;
	UINT8 *    m_spriteram;
	UINT8 *    m_k007121_regs;

	/* video-related */
	tilemap_t    *m_layer0;
	tilemap_t    *m_layer1;
	rectangle  m_clip0;
	rectangle  m_clip1;

	/* devices */
	device_t *m_konami2;
	device_t *m_k007121;
};




/*----------- defined in video/fastlane.c -----------*/

WRITE8_HANDLER( fastlane_vram1_w );
WRITE8_HANDLER( fastlane_vram2_w );

PALETTE_INIT( fastlane );
VIDEO_START( fastlane );
SCREEN_UPDATE( fastlane );
