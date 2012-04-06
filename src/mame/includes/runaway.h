class runaway_state : public driver_device
{
public:
	runaway_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	emu_timer *m_interrupt_timer;
	UINT8* m_video_ram;
	UINT8* m_sprite_ram;
	tilemap_t *m_bg_tilemap;
	int m_tile_bank;
	DECLARE_READ8_MEMBER(runaway_input_r);
	DECLARE_WRITE8_MEMBER(runaway_led_w);
	DECLARE_WRITE8_MEMBER(runaway_irq_ack_w);
	DECLARE_WRITE8_MEMBER(runaway_paletteram_w);
	DECLARE_WRITE8_MEMBER(runaway_video_ram_w);
	DECLARE_WRITE8_MEMBER(runaway_tile_bank_w);
};


/*----------- defined in video/runaway.c -----------*/

VIDEO_START( runaway );
VIDEO_START( qwak );
SCREEN_UPDATE_IND16( runaway );
SCREEN_UPDATE_IND16( qwak );

