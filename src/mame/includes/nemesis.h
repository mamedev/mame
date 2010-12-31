class nemesis_state : public driver_device
{
public:
	nemesis_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT16 *  videoram1;
	UINT16 *  videoram2;
	UINT16 *  colorram1;
	UINT16 *  colorram2;
	UINT16 *  charram;
	UINT16 *  spriteram;
	UINT16 *  paletteram;
	UINT16 *  xscroll1;
	UINT16 *  xscroll2;
	UINT16 *  yscroll1;
	UINT16 *  yscroll2;
	UINT8 *   gx400_shared_ram;

	size_t    charram_size;
	size_t    spriteram_size;

	/* video-related */
	tilemap_t *background, *foreground;
	int       spriteram_words;
	int       tilemap_flip;
	int       flipscreen;
	UINT8     irq_port_last;
	UINT8     blank_tile[8*8];

	/* misc */
	int       irq_on;
	int       irq1_on;
	int       irq2_on;
	int       irq4_on;
	UINT16    selected_ip; /* Copied from WEC Le Mans 24 driver, explicity needed for Hyper Crash */
	int       gx400_irq1_cnt;
	UINT8     frame_counter;

	/* devices */
	cpu_device *maincpu;
	cpu_device *audiocpu;
	device_t *vlm;
};


/*----------- defined in video/nemesis.c -----------*/

WRITE16_HANDLER( nemesis_gfx_flipx_word_w );
WRITE16_HANDLER( nemesis_gfx_flipy_word_w );
WRITE16_HANDLER( salamand_control_port_word_w );
WRITE16_HANDLER( salamander_palette_word_w );
WRITE16_HANDLER( nemesis_palette_word_w );

WRITE16_HANDLER( nemesis_videoram1_word_w );
WRITE16_HANDLER( nemesis_videoram2_word_w );
WRITE16_HANDLER( nemesis_colorram1_word_w );
WRITE16_HANDLER( nemesis_colorram2_word_w );
WRITE16_HANDLER( nemesis_charram_word_w );

VIDEO_START( nemesis );
VIDEO_UPDATE( nemesis );
