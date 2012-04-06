class seta2_state : public driver_device
{
public:
	seta2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this,"maincpu"),
		  m_nvram(*this, "nvram") { }

	required_device<cpu_device> m_maincpu;
	optional_shared_ptr<UINT16> m_nvram;

	UINT16 *m_vregs;
	int m_xoffset;
	int m_yoffset;
	int m_keyboard_row;

	UINT16 *m_spriteram;
	size_t m_spriteram_size;
	UINT16 *m_buffered_spriteram;
	UINT32 *m_coldfire_regs;

	UINT8 *m_funcube_outputs;
	UINT8 *m_funcube_leds;

	UINT64 m_funcube_coin_start_cycles;
	UINT8 m_funcube_hopper_motor;
	UINT8 m_funcube_press;

	UINT8 m_funcube_serial_fifo[4];
	UINT8 m_funcube_serial_count;
	DECLARE_WRITE16_MEMBER(seta2_vregs_w);
};

/*----------- defined in video/seta2.c -----------*/


VIDEO_START( seta2 );
VIDEO_START( seta2_xoffset );
VIDEO_START( seta2_yoffset );
SCREEN_UPDATE_IND16( seta2 );
SCREEN_VBLANK( seta2 );
