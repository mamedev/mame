/* This it the best way to allow game specific kludges until the system is fully understood */
enum {
	ARKUNK = 0,  /* unknown bootlegs for inclusion of possible new sets */
	ARKANGC,
	ARKANGC2,
	BLOCK2,
	ARKBLOCK,
	ARKBLOC2,
	ARKGCBL,
	PADDLE2
};

class arkanoid_state : public driver_device
{
public:
	arkanoid_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this,"videoram"),
		m_spriteram(*this,"spriteram") { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;
	optional_shared_ptr<UINT8> m_spriteram;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	UINT8    m_gfxbank;
	UINT8    m_palettebank;

	/* input-related */
	UINT8    m_paddle_select;

	/* misc */
	int      m_bootleg_id;
	UINT8    m_z80write;
	UINT8    m_fromz80;
	UINT8    m_m68705write;
	UINT8    m_toz80;
	UINT8    m_port_a_in;
	UINT8    m_port_a_out;
	UINT8    m_ddr_a;
	UINT8    m_port_c_out;
	UINT8    m_ddr_c;
	UINT8    m_bootleg_cmd;

	/* devices */
	device_t *m_mcu;
	DECLARE_READ8_MEMBER(arkanoid_Z80_mcu_r);
	DECLARE_WRITE8_MEMBER(arkanoid_Z80_mcu_w);
	DECLARE_READ8_MEMBER(arkanoid_68705_port_a_r);
	DECLARE_WRITE8_MEMBER(arkanoid_68705_port_a_w);
	DECLARE_WRITE8_MEMBER(arkanoid_68705_ddr_a_w);
	DECLARE_READ8_MEMBER(arkanoid_68705_port_c_r);
	DECLARE_WRITE8_MEMBER(arkanoid_68705_port_c_w);
	DECLARE_WRITE8_MEMBER(arkanoid_68705_ddr_c_w);
	DECLARE_READ8_MEMBER(arkanoid_bootleg_f000_r);
	DECLARE_READ8_MEMBER(arkanoid_bootleg_f002_r);
	DECLARE_WRITE8_MEMBER(arkanoid_bootleg_d018_w);
	DECLARE_READ8_MEMBER(block2_bootleg_f000_r);
	DECLARE_READ8_MEMBER(arkanoid_bootleg_d008_r);
	DECLARE_WRITE8_MEMBER(arkanoid_videoram_w);
	DECLARE_WRITE8_MEMBER(arkanoid_d008_w);
	DECLARE_WRITE8_MEMBER(tetrsark_d008_w);
	DECLARE_WRITE8_MEMBER(hexa_d008_w);
	DECLARE_CUSTOM_INPUT_MEMBER(arkanoid_68705_input_r);
	DECLARE_CUSTOM_INPUT_MEMBER(arkanoid_input_mux);
};



/*----------- defined in video/arkanoid.c -----------*/



extern VIDEO_START( arkanoid );
extern SCREEN_UPDATE_IND16( arkanoid );
extern SCREEN_UPDATE_IND16( hexa );

