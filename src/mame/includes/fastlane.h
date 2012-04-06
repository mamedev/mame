/*************************************************************************

    Fast Lane

*************************************************************************/

class fastlane_state : public driver_device
{
public:
	fastlane_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu")
		{ }

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

	required_device<cpu_device> m_maincpu;
	DECLARE_WRITE8_MEMBER(k007121_registers_w);
	DECLARE_WRITE8_MEMBER(fastlane_bankswitch_w);
	DECLARE_WRITE8_MEMBER(fastlane_vram1_w);
	DECLARE_WRITE8_MEMBER(fastlane_vram2_w);
};




/*----------- defined in video/fastlane.c -----------*/


PALETTE_INIT( fastlane );
VIDEO_START( fastlane );
SCREEN_UPDATE_IND16( fastlane );
