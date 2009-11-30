
typedef struct _flstory_state flstory_state;
struct _flstory_state
{
	/* memory pointers */
	UINT8 *  videoram;
	UINT8 *  workram;
	UINT8 *  scrlram;
	UINT8 *  spriteram;
//  UINT8 *  paletteram;    // currently this uses generic palette handling
//  UINT8 *  paletteram_2;  // currently this uses generic palette handling
	size_t   videoram_size;
	size_t   spriteram_size;

	/* video-related */
	tilemap  *bg_tilemap;
	int      char_bank, palette_bank, flipscreen, gfxctrl;

	/* sound-related */
	UINT8    snd_data;
	UINT8    snd_flag;
	int      sound_nmi_enable, pending_nmi;
	int      vol_ctrl[16];
	UINT8    snd_ctrl0;
	UINT8    snd_ctrl1;
	UINT8    snd_ctrl2;
	UINT8    snd_ctrl3;

	/* protection */
	UINT8    from_main, from_mcu;
	int      mcu_sent, main_sent;
	UINT8    port_a_in, port_a_out, ddr_a;
	UINT8    port_b_in, port_b_out, ddr_b;
	UINT8    port_c_in, port_c_out, ddr_c;
	int      mcu_select;

	/* devices */
	const device_config *maincpu;
	const device_config *audiocpu;
	const device_config *mcu;
};


/*----------- defined in machine/flstory.c -----------*/

READ8_HANDLER( flstory_68705_port_a_r );
WRITE8_HANDLER( flstory_68705_port_a_w );
READ8_HANDLER( flstory_68705_port_b_r );
WRITE8_HANDLER( flstory_68705_port_b_w );
READ8_HANDLER( flstory_68705_port_c_r );
WRITE8_HANDLER( flstory_68705_port_c_w );
WRITE8_HANDLER( flstory_68705_ddr_a_w );
WRITE8_HANDLER( flstory_68705_ddr_b_w );
WRITE8_HANDLER( flstory_68705_ddr_c_w );
WRITE8_HANDLER( flstory_mcu_w );
READ8_HANDLER( flstory_mcu_r );
READ8_HANDLER( flstory_mcu_status_r );
WRITE8_HANDLER( onna34ro_mcu_w );
READ8_HANDLER( onna34ro_mcu_r );
READ8_HANDLER( onna34ro_mcu_status_r );
WRITE8_HANDLER( victnine_mcu_w );
READ8_HANDLER( victnine_mcu_r );
READ8_HANDLER( victnine_mcu_status_r );


/*----------- defined in video/flstory.c -----------*/

VIDEO_START( flstory );
VIDEO_UPDATE( flstory );
VIDEO_START( victnine );
VIDEO_UPDATE( victnine );

WRITE8_HANDLER( flstory_videoram_w );
READ8_HANDLER( flstory_palette_r );
WRITE8_HANDLER( flstory_palette_w );
WRITE8_HANDLER( flstory_gfxctrl_w );
WRITE8_HANDLER( flstory_scrlram_w );
READ8_HANDLER( victnine_gfxctrl_r );
WRITE8_HANDLER( victnine_gfxctrl_w );
