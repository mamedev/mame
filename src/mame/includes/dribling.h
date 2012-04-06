/*************************************************************************

    Model Racing Dribbling hardware

*************************************************************************/



class dribling_state : public driver_device
{
public:
	dribling_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *  m_videoram;
	UINT8 *  m_colorram;

	/* misc */
	UINT8    m_abca;
	UINT8    m_dr;
	UINT8    m_ds;
	UINT8    m_sh;
	UINT8    m_input_mux;
	UINT8    m_di;

	/* devices */
	device_t *m_maincpu;
	device_t *m_ppi_0;
	device_t *m_ppi_1;
	DECLARE_READ8_MEMBER(ioread);
	DECLARE_WRITE8_MEMBER(iowrite);
	DECLARE_WRITE8_MEMBER(dribling_colorram_w);
};


/*----------- defined in video/dribling.c -----------*/

PALETTE_INIT( dribling );
SCREEN_UPDATE_IND16( dribling );
