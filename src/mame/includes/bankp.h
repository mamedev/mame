/***************************************************************************

    Bank Panic

***************************************************************************/


#define BANKP_MASTER_CLOCK 15468000
#define BANKP_CPU_CLOCK (BANKP_MASTER_CLOCK/6)
#define BANKP_SN76496_CLOCK (BANKP_MASTER_CLOCK/6)

class bankp_state : public driver_device
{
public:
	bankp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 * m_videoram;
	UINT8 * m_colorram;
	UINT8 * m_videoram2;
	UINT8 * m_colorram2;

	/* video-related */
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	int     m_scroll_x;
	int     m_priority;

	UINT8 m_nmi_mask;
};


/*----------- defined in video/bankp.c -----------*/

WRITE8_HANDLER( bankp_videoram_w );
WRITE8_HANDLER( bankp_colorram_w );
WRITE8_HANDLER( bankp_videoram2_w );
WRITE8_HANDLER( bankp_colorram2_w );
WRITE8_HANDLER( bankp_scroll_w );
WRITE8_HANDLER( bankp_out_w );

PALETTE_INIT( bankp );
VIDEO_START( bankp );
SCREEN_UPDATE( bankp );


