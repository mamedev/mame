class tutankhm_state : public driver_device
{
public:
	tutankhm_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"),
		m_paletteram(*this, "paletteram"),
		m_scroll(*this, "scroll"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_paletteram;
	required_shared_ptr<UINT8> m_scroll;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	UINT8     m_flip_x;
	UINT8     m_flip_y;

	/* misc */
	UINT8    m_irq_toggle;
	UINT8    m_irq_enable;

	/* devices */
	cpu_device *m_maincpu;
	DECLARE_WRITE8_MEMBER(irq_enable_w);
	DECLARE_WRITE8_MEMBER(tutankhm_bankselect_w);
	DECLARE_WRITE8_MEMBER(sound_mute_w);
	DECLARE_WRITE8_MEMBER(tutankhm_coin_counter_w);
	DECLARE_WRITE8_MEMBER(tutankhm_flip_screen_x_w);
	DECLARE_WRITE8_MEMBER(tutankhm_flip_screen_y_w);
};


/*----------- defined in video/tutankhm.c -----------*/


SCREEN_UPDATE_RGB32( tutankhm );
