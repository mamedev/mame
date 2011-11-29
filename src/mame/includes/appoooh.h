

class appoooh_state : public driver_device
{
public:
	appoooh_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *  m_bg_videoram;
	UINT8 *  m_bg_colorram;
	UINT8 *  m_fg_videoram;
	UINT8 *  m_fg_colorram;
	UINT8 *  m_spriteram;
	UINT8 *  m_spriteram_2;

	/* video-related */
	tilemap_t  *m_fg_tilemap;
	tilemap_t  *m_bg_tilemap;
	int m_scroll_x;
	int m_priority;

	/* sound-related */
	UINT32   m_adpcm_data;
	UINT32   m_adpcm_address;

	/* devices */
	device_t *m_adpcm;

	UINT8 m_nmi_mask;
};

#define CHR1_OFST   0x00  /* palette page of char set #1 */
#define CHR2_OFST   0x10  /* palette page of char set #2 */


/* ----------- defined in video/appoooh.c -----------*/

WRITE8_HANDLER( appoooh_fg_videoram_w );
WRITE8_HANDLER( appoooh_fg_colorram_w );
WRITE8_HANDLER( appoooh_bg_videoram_w );
WRITE8_HANDLER( appoooh_bg_colorram_w );
PALETTE_INIT( appoooh );
PALETTE_INIT( robowres );
WRITE8_HANDLER( appoooh_scroll_w );
WRITE8_HANDLER( appoooh_out_w );
VIDEO_START( appoooh );
SCREEN_UPDATE( appoooh );
SCREEN_UPDATE( robowres );
