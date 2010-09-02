/***************************************************************************

    Finalizer

***************************************************************************/

class finalizr_state : public driver_device
{
public:
	finalizr_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *       videoram;
	UINT8 *       colorram;
	UINT8 *       videoram2;
	UINT8 *       colorram2;
	UINT8 *       scroll;
	UINT8 *       spriteram;
	UINT8 *       spriteram_2;
	size_t        videoram_size;
	size_t        spriteram_size;

	/* video-related */
	tilemap_t       *fg_tilemap, *bg_tilemap;
	int           spriterambank, charbank;

	/* misc */
	int           T1_line;
	UINT8         nmi_enable, irq_enable;

	/* devices */
	running_device *audio_cpu;
};


/*----------- defined in video/finalizr.c -----------*/

WRITE8_HANDLER( finalizr_videoctrl_w );

PALETTE_INIT( finalizr );
VIDEO_START( finalizr );
VIDEO_UPDATE( finalizr );
