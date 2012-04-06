
class matmania_state : public driver_device
{
public:
	matmania_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *         m_videoram;
	UINT8 *         m_videoram2;
	UINT8 *         m_videoram3;
	UINT8 *         m_colorram;
	UINT8 *         m_colorram2;
	UINT8 *         m_colorram3;
	UINT8 *         m_scroll;
	UINT8 *         m_pageselect;
	UINT8 *         m_spriteram;
	UINT8 *         m_paletteram;
	size_t          m_videoram_size;
	size_t          m_videoram2_size;
	size_t          m_videoram3_size;
	size_t          m_spriteram_size;

	/* video-related */
	bitmap_ind16        *m_tmpbitmap;
	bitmap_ind16        *m_tmpbitmap2;

	/* mcu */
	/* maniach 68705 protection */
	UINT8           m_port_a_in;
	UINT8           m_port_a_out;
	UINT8           m_ddr_a;
	UINT8           m_port_b_in;
	UINT8           m_port_b_out;
	UINT8           m_ddr_b;
	UINT8           m_port_c_in;
	UINT8           m_port_c_out;
	UINT8           m_ddr_c;
	UINT8           m_from_main;
	UINT8           m_from_mcu;
	int             m_mcu_sent;
	int             m_main_sent;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	device_t *m_mcu;
	DECLARE_WRITE8_MEMBER(matmania_sh_command_w);
	DECLARE_WRITE8_MEMBER(maniach_sh_command_w);
	DECLARE_WRITE8_MEMBER(matmania_paletteram_w);
};

/*----------- defined in machine/maniach.c -----------*/

READ8_HANDLER( maniach_68705_port_a_r );
WRITE8_HANDLER( maniach_68705_port_a_w );
READ8_HANDLER( maniach_68705_port_b_r );
WRITE8_HANDLER( maniach_68705_port_b_w );
READ8_HANDLER( maniach_68705_port_c_r );
WRITE8_HANDLER( maniach_68705_port_c_w );
WRITE8_HANDLER( maniach_68705_ddr_a_w );
WRITE8_HANDLER( maniach_68705_ddr_b_w );
WRITE8_HANDLER( maniach_68705_ddr_c_w );
WRITE8_HANDLER( maniach_mcu_w );
READ8_HANDLER( maniach_mcu_r );
READ8_HANDLER( maniach_mcu_status_r );


/*----------- defined in video/matmania.c -----------*/

PALETTE_INIT( matmania );
SCREEN_UPDATE_IND16( maniach );
VIDEO_START( matmania );
SCREEN_UPDATE_IND16( matmania );
