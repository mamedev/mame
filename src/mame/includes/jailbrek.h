/***************************************************************************

    Jailbreak

***************************************************************************/

#define MASTER_CLOCK        XTAL_18_432MHz
#define VOICE_CLOCK         XTAL_3_579545MHz

class jailbrek_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, jailbrek_state(machine)); }

	jailbrek_state(running_machine &machine)
		: driver_data_t(machine) { }

	/* memory pointers */
	UINT8 *      videoram;
	UINT8 *      colorram;
	UINT8 *      spriteram;
	UINT8 *      scroll_x;
	UINT8 *      scroll_dir;
	size_t       spriteram_size;

	/* video-related */
	tilemap_t      *bg_tilemap;

	/* misc */
	UINT8        irq_enable, nmi_enable;
};


/*----------- defined in video/jailbrek.c -----------*/

WRITE8_HANDLER( jailbrek_videoram_w );
WRITE8_HANDLER( jailbrek_colorram_w );

PALETTE_INIT( jailbrek );
VIDEO_START( jailbrek );
VIDEO_UPDATE( jailbrek );
