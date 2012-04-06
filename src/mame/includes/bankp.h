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
	DECLARE_WRITE8_MEMBER(bankp_scroll_w);
	DECLARE_WRITE8_MEMBER(bankp_videoram_w);
	DECLARE_WRITE8_MEMBER(bankp_colorram_w);
	DECLARE_WRITE8_MEMBER(bankp_videoram2_w);
	DECLARE_WRITE8_MEMBER(bankp_colorram2_w);
	DECLARE_WRITE8_MEMBER(bankp_out_w);
};


/*----------- defined in video/bankp.c -----------*/


PALETTE_INIT( bankp );
VIDEO_START( bankp );
SCREEN_UPDATE_IND16( bankp );


