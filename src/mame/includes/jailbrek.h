/***************************************************************************

    Jailbreak

***************************************************************************/

#define MASTER_CLOCK        XTAL_18_432MHz
#define VOICE_CLOCK         XTAL_3_579545MHz

class jailbrek_state : public driver_device
{
public:
	jailbrek_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *      m_videoram;
	UINT8 *      m_colorram;
	UINT8 *      m_spriteram;
	UINT8 *      m_scroll_x;
	UINT8 *      m_scroll_dir;
	size_t       m_spriteram_size;

	/* video-related */
	tilemap_t      *m_bg_tilemap;

	/* misc */
	UINT8        m_irq_enable;
	UINT8        m_nmi_enable;
};


/*----------- defined in video/jailbrek.c -----------*/

WRITE8_HANDLER( jailbrek_videoram_w );
WRITE8_HANDLER( jailbrek_colorram_w );

PALETTE_INIT( jailbrek );
VIDEO_START( jailbrek );
SCREEN_UPDATE( jailbrek );
