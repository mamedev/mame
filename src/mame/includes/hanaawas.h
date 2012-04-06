/*************************************************************************

    Hana Awase

*************************************************************************/

class hanaawas_state : public driver_device
{
public:
	hanaawas_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *    m_videoram;
	UINT8 *    m_colorram;

	/* video-related */
	tilemap_t    *m_bg_tilemap;

	/* misc */
	int        m_mux;
	DECLARE_READ8_MEMBER(hanaawas_input_port_0_r);
	DECLARE_WRITE8_MEMBER(hanaawas_inputs_mux_w);
	DECLARE_WRITE8_MEMBER(hanaawas_videoram_w);
	DECLARE_WRITE8_MEMBER(hanaawas_colorram_w);
};


/*----------- defined in video/hanaawas.c -----------*/

WRITE8_DEVICE_HANDLER( hanaawas_portB_w );

PALETTE_INIT( hanaawas );
VIDEO_START( hanaawas );
SCREEN_UPDATE_IND16( hanaawas );
