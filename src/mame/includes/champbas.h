/*************************************************************************

    Talbot - Champion Base Ball - Exciting Soccer

*************************************************************************/


#define CPUTAG_MCU "mcu"


class champbas_state : public driver_device
{
public:
	champbas_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *        m_bg_videoram;
	UINT8 *        m_spriteram;
	UINT8 *        m_spriteram_2;
	size_t         m_spriteram_size;

	/* video-related */
	tilemap_t        *m_bg_tilemap;
	UINT8          m_gfx_bank;
	UINT8          m_palette_bank;

	/* misc */
	int            m_watchdog_count;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	device_t *m_mcu;

	UINT8          m_irq_mask;
};


/*----------- defined in video/champbas.c -----------*/

WRITE8_HANDLER( champbas_bg_videoram_w );
WRITE8_HANDLER( champbas_gfxbank_w );
WRITE8_HANDLER( champbas_palette_bank_w );
WRITE8_HANDLER( champbas_flipscreen_w );

PALETTE_INIT( champbas );
PALETTE_INIT( exctsccr );
VIDEO_START( champbas );
VIDEO_START( exctsccr );
SCREEN_UPDATE( champbas );
SCREEN_UPDATE( exctsccr );


