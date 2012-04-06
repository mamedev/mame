/*************************************************************************

    Express Raider

*************************************************************************/


class exprraid_state : public driver_device
{
public:
	exprraid_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *        m_main_ram;
	UINT8 *        m_videoram;
	UINT8 *        m_colorram;
	UINT8 *        m_spriteram;
	size_t         m_spriteram_size;

	/* video-related */
	tilemap_t        *m_bg_tilemap;
	tilemap_t        *m_fg_tilemap;
	int            m_bg_index[4];

	/* misc */
	//int          m_coin;    // used in the commented out INTERRUPT_GEN - can this be removed?

	/* devices */
	device_t *m_maincpu;
	device_t *m_slave;
	DECLARE_READ8_MEMBER(exprraid_protection_r);
	DECLARE_WRITE8_MEMBER(sound_cpu_command_w);
	DECLARE_READ8_MEMBER(vblank_r);
	DECLARE_WRITE8_MEMBER(exprraid_videoram_w);
	DECLARE_WRITE8_MEMBER(exprraid_colorram_w);
	DECLARE_WRITE8_MEMBER(exprraid_flipscreen_w);
	DECLARE_WRITE8_MEMBER(exprraid_bgselect_w);
	DECLARE_WRITE8_MEMBER(exprraid_scrollx_w);
	DECLARE_WRITE8_MEMBER(exprraid_scrolly_w);
};


/*----------- defined in video/exprraid.c -----------*/


extern VIDEO_START( exprraid );
extern SCREEN_UPDATE_IND16( exprraid );
