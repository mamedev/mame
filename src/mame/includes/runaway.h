class runaway_state : public driver_device
{
public:
	runaway_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_video_ram(*this, "video_ram"),
		m_sprite_ram(*this, "sprite_ram"){ }

	emu_timer *m_interrupt_timer;
	required_shared_ptr<UINT8> m_video_ram;
	required_shared_ptr<UINT8> m_sprite_ram;
	tilemap_t *m_bg_tilemap;
	int m_tile_bank;
	DECLARE_READ8_MEMBER(runaway_input_r);
	DECLARE_WRITE8_MEMBER(runaway_led_w);
	DECLARE_WRITE8_MEMBER(runaway_irq_ack_w);
	DECLARE_WRITE8_MEMBER(runaway_paletteram_w);
	DECLARE_WRITE8_MEMBER(runaway_video_ram_w);
	DECLARE_WRITE8_MEMBER(runaway_tile_bank_w);
	DECLARE_READ8_MEMBER(runaway_pot_r);
	TILE_GET_INFO_MEMBER(runaway_get_tile_info);
	TILE_GET_INFO_MEMBER(qwak_get_tile_info);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	DECLARE_VIDEO_START(qwak);
};


/*----------- defined in video/runaway.c -----------*/



SCREEN_UPDATE_IND16( runaway );
SCREEN_UPDATE_IND16( qwak );

