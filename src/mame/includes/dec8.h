
class dec8_state : public driver_device
{
public:
	dec8_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *  videoram;
	UINT8 *  bg_data;
	UINT8 *  pf1_data;
	UINT8 *  row;
//  UINT8 *  paletteram;    // currently this uses generic palette handling
//  UINT8 *  paletteram_2;  // currently this uses generic palette handling
//  UINT8 *  spriteram; // currently this uses buffered_spriteram in some games
	size_t   videoram_size;
	UINT16   buffered_spriteram16[0x800/2]; // for the mxc06 sprite chip emulation (oscar, cobra)

	/* video-related */
	tilemap_t  *bg_tilemap, *pf1_tilemap, *fix_tilemap;
	//int      scroll1[4];
	int      scroll2[4];
	int      bg_control[0x20], pf1_control[0x20];
	int      game_uses_priority;

	/* misc */
	int      i8751_port0, i8751_port1;
	int      nmi_enable;
	int      i8751_return, i8751_value;
	int      coin1, coin2, latch, snd;
	int      msm5205next;
	int      toggle;

	/* devices */
	device_t *maincpu;
	device_t *subcpu;
	device_t *audiocpu;
	device_t *mcu;
};

/*----------- defined in video/dec8.c -----------*/


PALETTE_INIT( ghostb );
SCREEN_UPDATE( cobracom );
SCREEN_UPDATE( ghostb );
SCREEN_UPDATE( srdarwin );
SCREEN_UPDATE( gondo );
SCREEN_UPDATE( garyoret );
SCREEN_UPDATE( lastmisn );
SCREEN_UPDATE( shackled );
SCREEN_UPDATE( oscar );
VIDEO_START( cobracom );
VIDEO_START( oscar );
VIDEO_START( ghostb );
VIDEO_START( lastmisn );
VIDEO_START( shackled );
VIDEO_START( srdarwin );
VIDEO_START( gondo );
VIDEO_START( garyoret );

WRITE8_HANDLER( dec8_bac06_0_w );
WRITE8_HANDLER( dec8_bac06_1_w );
WRITE8_HANDLER( dec8_bg_data_w );
WRITE8_HANDLER( dec8_pf1_data_w );
READ8_HANDLER( dec8_bg_data_r );
READ8_HANDLER( dec8_pf1_data_r );
WRITE8_HANDLER( srdarwin_videoram_w );
WRITE8_HANDLER( dec8_scroll2_w );
WRITE8_HANDLER( srdarwin_control_w );
WRITE8_HANDLER( gondo_scroll_w );
WRITE8_HANDLER( shackled_control_w );
WRITE8_HANDLER( lastmisn_control_w );
WRITE8_HANDLER( lastmisn_scrollx_w );
WRITE8_HANDLER( lastmisn_scrolly_w );
WRITE8_HANDLER( dec8_videoram_w );
