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
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *  m_videoram;
	UINT8 *  m_spriteram;
	size_t   m_spriteram_size;
	size_t   m_videoram_size;

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
};



/*----------- defined in video/arkanoid.c -----------*/

extern WRITE8_HANDLER( arkanoid_videoram_w );

extern WRITE8_HANDLER( arkanoid_d008_w );
extern WRITE8_HANDLER( tetrsark_d008_w );
extern WRITE8_HANDLER( hexa_d008_w );

extern VIDEO_START( arkanoid );
extern SCREEN_UPDATE( arkanoid );
extern SCREEN_UPDATE( hexa );


/*----------- defined in machine/arkanoid.c -----------*/

extern READ8_HANDLER( arkanoid_Z80_mcu_r );
extern WRITE8_HANDLER( arkanoid_Z80_mcu_w );

extern READ8_HANDLER( arkanoid_68705_port_a_r );
extern WRITE8_HANDLER( arkanoid_68705_port_a_w );
extern WRITE8_HANDLER( arkanoid_68705_ddr_a_w );

extern READ8_HANDLER( arkanoid_68705_port_c_r );
extern WRITE8_HANDLER( arkanoid_68705_port_c_w );
extern WRITE8_HANDLER( arkanoid_68705_ddr_c_w );

extern CUSTOM_INPUT( arkanoid_68705_input_r );
extern CUSTOM_INPUT( arkanoid_input_mux );

extern READ8_HANDLER( arkanoid_bootleg_f000_r );
extern READ8_HANDLER( arkanoid_bootleg_f002_r );
extern WRITE8_HANDLER( arkanoid_bootleg_d018_w );
extern READ8_HANDLER( arkanoid_bootleg_d008_r );
