class system1_state : public driver_device
{
public:
	system1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_videoram;
	void (*m_videomode_custom)(running_machine &machine, UINT8 data, UINT8 prevdata);
	UINT8 m_mute_xor;
	UINT8 *m_ram;
	UINT8 m_dakkochn_mux_data;
	UINT8 m_videomode_prev;
	UINT8 m_mcu_control;
	UINT8 *m_nob_mcu_status;
	UINT8 *m_nob_mcu_latch;
	UINT8 m_nob_maincpu_latch;
	int m_nobb_inport23_step;
	UINT8 *m_mix_collide;
	UINT8 m_mix_collide_summary;
	UINT8 *m_sprite_collide;
	UINT8 m_sprite_collide_summary;
	bitmap_ind16 *m_sprite_bitmap;
	UINT8 m_video_mode;
	UINT8 m_videoram_bank;
	tilemap_t *m_tilemap_page[8];
	UINT8 m_tilemap_pages;
	UINT8 *m_spriteram;
	DECLARE_WRITE8_MEMBER(videomode_w);
	DECLARE_READ8_MEMBER(sound_data_r);
	DECLARE_WRITE8_MEMBER(soundport_w);
	DECLARE_WRITE8_MEMBER(mcu_control_w);
	DECLARE_WRITE8_MEMBER(mcu_io_w);
	DECLARE_READ8_MEMBER(mcu_io_r);
	DECLARE_WRITE8_MEMBER(nob_mcu_control_p2_w);
	DECLARE_READ8_MEMBER(nob_maincpu_latch_r);
	DECLARE_WRITE8_MEMBER(nob_maincpu_latch_w);
	DECLARE_READ8_MEMBER(nob_mcu_status_r);
	DECLARE_READ8_MEMBER(nobb_inport1c_r);
	DECLARE_READ8_MEMBER(nobb_inport22_r);
	DECLARE_READ8_MEMBER(nobb_inport23_r);
	DECLARE_WRITE8_MEMBER(nobb_outport24_w);
	DECLARE_READ8_MEMBER(nob_start_r);
	DECLARE_WRITE8_MEMBER(system1_videomode_w);
	DECLARE_READ8_MEMBER(system1_mixer_collision_r);
	DECLARE_WRITE8_MEMBER(system1_mixer_collision_w);
	DECLARE_WRITE8_MEMBER(system1_mixer_collision_reset_w);
	DECLARE_READ8_MEMBER(system1_sprite_collision_r);
	DECLARE_WRITE8_MEMBER(system1_sprite_collision_w);
	DECLARE_WRITE8_MEMBER(system1_sprite_collision_reset_w);
	DECLARE_READ8_MEMBER(system1_videoram_r);
	DECLARE_WRITE8_MEMBER(system1_videoram_w);
	DECLARE_WRITE8_MEMBER(system1_paletteram_w);
};


/*----------- defined in video/system1.c -----------*/

VIDEO_START( system1 );
VIDEO_START( system2 );


WRITE8_DEVICE_HANDLER( system1_videoram_bank_w );



SCREEN_UPDATE_IND16( system1 );
SCREEN_UPDATE_IND16( system2 );
SCREEN_UPDATE_IND16( system2_rowscroll );
