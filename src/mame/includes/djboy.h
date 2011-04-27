/*************************************************************************

    DJ Boy

*************************************************************************/

#define PROT_OUTPUT_BUFFER_SIZE 8

class djboy_state : public driver_device
{
public:
	djboy_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8		*m_videoram;
	UINT8		*m_paletteram;

	/* ROM banking */
	UINT8		m_bankxor;

	/* video-related */
	tilemap_t	*m_background;
	UINT8		m_videoreg;
	UINT8       m_scrollx;
	UINT8       m_scrolly;

	/* Kaneko BEAST state */
	UINT8		m_data_to_beast;
	UINT8		m_data_to_z80;
	UINT8		m_beast_to_z80_full;
	UINT8		m_z80_to_beast_full;
	UINT8		m_beast_int0_l;
	UINT8		m_beast_p0;
	UINT8		m_beast_p1;
	UINT8		m_beast_p2;
	UINT8		m_beast_p3;

	/* devices */
	device_t *m_maincpu;
	device_t *m_cpu1;
	device_t *m_cpu2;
	device_t *m_pandora;
	device_t *m_beast;
};


/*----------- defined in video/djboy.c -----------*/

WRITE8_HANDLER( djboy_scrollx_w );
WRITE8_HANDLER( djboy_scrolly_w );
WRITE8_HANDLER( djboy_videoram_w );
WRITE8_HANDLER( djboy_paletteram_w );

VIDEO_START( djboy );
SCREEN_UPDATE( djboy );
SCREEN_EOF( djboy );
