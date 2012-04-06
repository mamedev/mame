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
	DECLARE_WRITE8_MEMBER(beast_data_w);
	DECLARE_READ8_MEMBER(beast_data_r);
	DECLARE_READ8_MEMBER(beast_status_r);
	DECLARE_WRITE8_MEMBER(trigger_nmi_on_cpu0);
	DECLARE_WRITE8_MEMBER(cpu0_bankswitch_w);
	DECLARE_WRITE8_MEMBER(cpu1_bankswitch_w);
	DECLARE_WRITE8_MEMBER(coin_count_w);
	DECLARE_WRITE8_MEMBER(trigger_nmi_on_sound_cpu2);
	DECLARE_WRITE8_MEMBER(cpu2_bankswitch_w);
	DECLARE_READ8_MEMBER(beast_p0_r);
	DECLARE_WRITE8_MEMBER(beast_p0_w);
	DECLARE_READ8_MEMBER(beast_p1_r);
	DECLARE_WRITE8_MEMBER(beast_p1_w);
	DECLARE_READ8_MEMBER(beast_p2_r);
	DECLARE_WRITE8_MEMBER(beast_p2_w);
	DECLARE_READ8_MEMBER(beast_p3_r);
	DECLARE_WRITE8_MEMBER(beast_p3_w);
	DECLARE_WRITE8_MEMBER(djboy_scrollx_w);
	DECLARE_WRITE8_MEMBER(djboy_scrolly_w);
	DECLARE_WRITE8_MEMBER(djboy_videoram_w);
	DECLARE_WRITE8_MEMBER(djboy_paletteram_w);
};


/*----------- defined in video/djboy.c -----------*/


VIDEO_START( djboy );
SCREEN_UPDATE_IND16( djboy );
SCREEN_VBLANK( djboy );
