/***************************************************************************

    Blockout

***************************************************************************/

class blockout_state : public driver_device
{
public:
	blockout_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"),
		m_frontvideoram(*this, "frontvideoram"),
		m_paletteram(*this, "paletteram"){ }

	/* memory pointers */
	required_shared_ptr<UINT16> m_videoram;
	required_shared_ptr<UINT16> m_frontvideoram;
	required_shared_ptr<UINT16> m_paletteram;

	/* video-related */
	bitmap_ind16 m_tmpbitmap;
	UINT16   m_color;

	/* devices */
	cpu_device *m_maincpu;
	cpu_device *m_audiocpu;
	DECLARE_WRITE16_MEMBER(blockout_sound_command_w);
	DECLARE_WRITE16_MEMBER(blockout_irq6_ack_w);
	DECLARE_WRITE16_MEMBER(blockout_irq5_ack_w);
	DECLARE_WRITE16_MEMBER(blockout_paletteram_w);
	DECLARE_WRITE16_MEMBER(blockout_frontcolor_w);
	DECLARE_WRITE16_MEMBER(blockout_videoram_w);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_blockout(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(blockout_scanline);
};
