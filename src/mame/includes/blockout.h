/***************************************************************************

    Blockout

***************************************************************************/

class blockout_state : public driver_device
{
public:
	blockout_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT16 * m_videoram;
	UINT16 * m_frontvideoram;
	UINT16 * m_paletteram;

	/* video-related */
	bitmap_ind16 m_tmpbitmap;
	UINT16   m_color;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	DECLARE_WRITE16_MEMBER(blockout_sound_command_w);
	DECLARE_WRITE16_MEMBER(blockout_irq6_ack_w);
	DECLARE_WRITE16_MEMBER(blockout_irq5_ack_w);
	DECLARE_WRITE16_MEMBER(blockout_paletteram_w);
	DECLARE_WRITE16_MEMBER(blockout_frontcolor_w);
	DECLARE_WRITE16_MEMBER(blockout_videoram_w);
};


/*----------- defined in video/blockout.c -----------*/


VIDEO_START( blockout );
SCREEN_UPDATE_IND16( blockout );
