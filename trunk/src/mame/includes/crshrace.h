
class crshrace_state : public driver_device
{
public:
	crshrace_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT16 *  m_videoram1;
	UINT16 *  m_videoram2;
//  UINT16 *  m_spriteram1;   // currently this uses generic buffered spriteram
//  UINT16 *  m_spriteram2;   // currently this uses generic buffered spriteram
//      UINT16 *  m_paletteram;   // currently this uses generic palette handling

	/* video-related */
	tilemap_t   *m_tilemap1;
	tilemap_t   *m_tilemap2;
	int       m_roz_bank;
	int       m_gfxctrl;
	int       m_flipscreen;

	/* misc */
	int m_pending_command;

	/* devices */
	device_t *m_audiocpu;
	device_t *m_k053936;
};

/*----------- defined in video/crshrace.c -----------*/

WRITE16_HANDLER( crshrace_videoram1_w );
WRITE16_HANDLER( crshrace_videoram2_w );
WRITE16_HANDLER( crshrace_roz_bank_w );
WRITE16_HANDLER( crshrace_gfxctrl_w );

VIDEO_START( crshrace );
SCREEN_EOF( crshrace );
SCREEN_UPDATE( crshrace );
