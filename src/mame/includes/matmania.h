
class matmania_state : public driver_device
{
public:
	matmania_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"),
		m_videoram2(*this, "videoram2"),
		m_videoram3(*this, "videoram3"),
		m_colorram(*this, "colorram"),
		m_colorram2(*this, "colorram2"),
		m_colorram3(*this, "colorram3"),
		m_scroll(*this, "scroll"),
		m_pageselect(*this, "pageselect"),
		m_spriteram(*this, "spriteram"),
		m_paletteram(*this, "paletteram"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_videoram2;
	required_shared_ptr<UINT8> m_videoram3;
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_colorram2;
	required_shared_ptr<UINT8> m_colorram3;
	required_shared_ptr<UINT8> m_scroll;
	required_shared_ptr<UINT8> m_pageselect;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_paletteram;

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
	cpu_device *m_maincpu;
	cpu_device *m_audiocpu;
	device_t *m_mcu;
	DECLARE_WRITE8_MEMBER(matmania_sh_command_w);
	DECLARE_WRITE8_MEMBER(maniach_sh_command_w);
	DECLARE_WRITE8_MEMBER(matmania_paletteram_w);
	virtual void video_start();
	virtual void palette_init();
	DECLARE_MACHINE_START(matmania);
	DECLARE_MACHINE_START(maniach);
	DECLARE_MACHINE_RESET(maniach);
};

/*----------- defined in machine/maniach.c -----------*/

DECLARE_READ8_HANDLER( maniach_68705_port_a_r );
DECLARE_WRITE8_HANDLER( maniach_68705_port_a_w );
DECLARE_READ8_HANDLER( maniach_68705_port_b_r );
DECLARE_WRITE8_HANDLER( maniach_68705_port_b_w );
DECLARE_READ8_HANDLER( maniach_68705_port_c_r );
DECLARE_WRITE8_HANDLER( maniach_68705_port_c_w );
DECLARE_WRITE8_HANDLER( maniach_68705_ddr_a_w );
DECLARE_WRITE8_HANDLER( maniach_68705_ddr_b_w );
DECLARE_WRITE8_HANDLER( maniach_68705_ddr_c_w );
DECLARE_WRITE8_HANDLER( maniach_mcu_w );
DECLARE_READ8_HANDLER( maniach_mcu_r );
DECLARE_READ8_HANDLER( maniach_mcu_status_r );


/*----------- defined in video/matmania.c -----------*/


SCREEN_UPDATE_IND16( maniach );

SCREEN_UPDATE_IND16( matmania );
