class eolith_state : public driver_device
{
public:
	eolith_state(const machine_config &mconfig, device_type type, const char *tag)
		:	driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_soundcpu(*this, "soundcpu")
			{ }

	int m_coin_counter_bit;
	int m_buffer;
	UINT32 *m_vram;

	UINT8 m_sound_data;
	UINT8 m_data_to_qs1000;

	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_soundcpu;

	DECLARE_READ32_MEMBER(eolith_custom_r);
	DECLARE_WRITE32_MEMBER(systemcontrol_w);
	DECLARE_WRITE32_MEMBER(sound_w);
	DECLARE_READ32_MEMBER(hidctch3_pen1_r);
	DECLARE_READ32_MEMBER(hidctch3_pen2_r);
	DECLARE_WRITE32_MEMBER(eolith_vram_w);
	DECLARE_READ32_MEMBER(eolith_vram_r);
	DECLARE_CUSTOM_INPUT_MEMBER(eolith_speedup_getvblank);
	DECLARE_CUSTOM_INPUT_MEMBER(stealsee_speedup_getvblank);

	DECLARE_READ8_MEMBER(sound_cmd_r);
	DECLARE_WRITE8_MEMBER(sound_p1_w);

	DECLARE_READ8_MEMBER(qs1000_p1_r);
	DECLARE_WRITE8_MEMBER(qs1000_p1_w);
};


/*----------- defined in video/eolith.c -----------*/

VIDEO_START( eolith );
SCREEN_UPDATE_IND16( eolith );
