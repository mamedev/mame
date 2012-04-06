/*************************************************************************

    Mr. Flea

*************************************************************************/

class mrflea_state : public driver_device
{
public:
	mrflea_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *    m_videoram;
	UINT8 *    m_spriteram;
//  UINT8 *    paletteram;    // currently this uses generic palette handling

	/* video-related */
	int     m_gfx_bank;

	/* misc */
	int m_io;
	int m_main;
	int m_status;
	int m_select1;

	/* devices */
	device_t *m_maincpu;
	device_t *m_subcpu;
	DECLARE_WRITE8_MEMBER(mrflea_main_w);
	DECLARE_WRITE8_MEMBER(mrflea_io_w);
	DECLARE_READ8_MEMBER(mrflea_main_r);
	DECLARE_READ8_MEMBER(mrflea_io_r);
	DECLARE_READ8_MEMBER(mrflea_main_status_r);
	DECLARE_READ8_MEMBER(mrflea_io_status_r);
	DECLARE_READ8_MEMBER(mrflea_interrupt_type_r);
	DECLARE_WRITE8_MEMBER(mrflea_select1_w);
	DECLARE_READ8_MEMBER(mrflea_input1_r);
	DECLARE_WRITE8_MEMBER(mrflea_data1_w);
	DECLARE_WRITE8_MEMBER(mrflea_gfx_bank_w);
	DECLARE_WRITE8_MEMBER(mrflea_videoram_w);
	DECLARE_WRITE8_MEMBER(mrflea_spriteram_w);
};


/*----------- defined in video/mrflea.c -----------*/


SCREEN_UPDATE_IND16( mrflea );
