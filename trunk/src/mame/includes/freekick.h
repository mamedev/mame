

class freekick_state : public driver_device
{
public:
	freekick_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *    m_videoram;
	UINT8 *    m_spriteram;
	size_t     m_spriteram_size;

	/* video-related */
	tilemap_t    *m_freek_tilemap;

	/* misc */
	int        m_inval;
	int        m_outval;
	int        m_cnt;	// used by oigas
	int        m_romaddr;
	int        m_spinner;
	int        m_nmi_en;
	int        m_ff_data;
};


/*----------- defined in video/freekick.c -----------*/

VIDEO_START(freekick);
SCREEN_UPDATE(gigas);
SCREEN_UPDATE(pbillrd);
SCREEN_UPDATE(freekick);
WRITE8_HANDLER( freek_videoram_w );
