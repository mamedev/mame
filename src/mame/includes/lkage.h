
class lkage_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, lkage_state(machine)); }

	lkage_state(running_machine &machine) { }

	UINT8 *      scroll;
	UINT8 *      vreg;
	UINT8 *      videoram;
	UINT8 *      spriteram;
//  UINT8 *    paletteram;    // currently this uses generic palette handling

	/* video-related */
	tilemap_t *bg_tilemap, *fg_tilemap, *tx_tilemap;
	UINT8 bg_tile_bank, fg_tile_bank;

	/* misc */
	int sound_nmi_enable, pending_nmi;

	/* mcu */
	UINT8 from_main, from_mcu;
	int mcu_sent, main_sent;
	UINT8 port_a_in, port_a_out, ddr_a;
	UINT8 port_b_in, port_b_out, ddr_b;
	UINT8 port_c_in, port_c_out, ddr_c;

	/* lkageb fake mcu */
	UINT8 mcu_val;
	int mcu_ready;	/* cpu data/mcu ready status */

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
	running_device *mcu;
};

/*----------- defined in machine/lkage.c -----------*/

READ8_HANDLER( lkage_68705_port_a_r );
WRITE8_HANDLER( lkage_68705_port_a_w );
READ8_HANDLER( lkage_68705_port_b_r );
WRITE8_HANDLER( lkage_68705_port_b_w );
READ8_HANDLER( lkage_68705_port_c_r );
WRITE8_HANDLER( lkage_68705_port_c_w );
WRITE8_HANDLER( lkage_68705_ddr_a_w );
WRITE8_HANDLER( lkage_68705_ddr_b_w );
WRITE8_HANDLER( lkage_68705_ddr_c_w );
WRITE8_HANDLER( lkage_mcu_w );
READ8_HANDLER( lkage_mcu_r );
READ8_HANDLER( lkage_mcu_status_r );


/*----------- defined in video/lkage.c -----------*/

WRITE8_HANDLER( lkage_videoram_w );
VIDEO_START( lkage );
VIDEO_UPDATE( lkage );

