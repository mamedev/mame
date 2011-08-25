/*************************************************************************

    Epos games

**************************************************************************/

class epos_state : public driver_device
{
public:
	epos_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *  m_videoram;
	size_t   m_videoram_size;

	/* video-related */
	UINT8    m_palette;

	/* misc */
	int      m_counter;
};


/*----------- defined in video/epos.c -----------*/

WRITE8_HANDLER( epos_port_1_w );
SCREEN_UPDATE( epos );
