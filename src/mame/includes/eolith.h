class eolith_state : public driver_device
{
public:
	eolith_state(const machine_config &mconfig, device_type type, const char *tag)
		:	driver_device(mconfig, type, tag),
			m_maincpu(*this,"maincpu")
			{ }

	int m_coin_counter_bit;
	int m_buffer;
	UINT32 *m_vram;

	required_device<cpu_device> m_maincpu;
	DECLARE_READ32_MEMBER(eolith_custom_r);
	DECLARE_WRITE32_MEMBER(systemcontrol_w);
	DECLARE_READ32_MEMBER(hidctch3_pen1_r);
	DECLARE_READ32_MEMBER(hidctch3_pen2_r);
	DECLARE_WRITE32_MEMBER(eolith_vram_w);
	DECLARE_READ32_MEMBER(eolith_vram_r);
};


/*----------- defined in video/eolith.c -----------*/

VIDEO_START( eolith );
SCREEN_UPDATE_IND16( eolith );
