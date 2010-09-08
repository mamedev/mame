class playch10_state : public driver_device
{
public:
	playch10_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *videoram;
};


/*----------- defined in machine/playch10.c -----------*/

MACHINE_RESET( pc10 );
MACHINE_START( pc10 );
MACHINE_START( playch10_hboard );
DRIVER_INIT( playch10 );	/* standard games */
DRIVER_INIT( pc_gun );	/* gun games */
DRIVER_INIT( pc_hrz );	/* horizontal games */
DRIVER_INIT( pcaboard );	/* a-board games */
DRIVER_INIT( pcbboard );	/* b-board games */
DRIVER_INIT( pccboard );	/* c-board games */
DRIVER_INIT( pcdboard );	/* d-board games */
DRIVER_INIT( pcdboard_2 );	/* d-board games with extra ram */
DRIVER_INIT( pceboard );	/* e-board games */
DRIVER_INIT( pcfboard );	/* f-board games */
DRIVER_INIT( pcfboard_2 );	/* f-board games with extra ram */
DRIVER_INIT( pcgboard );	/* g-board games */
DRIVER_INIT( pcgboard_type2 ); /* g-board games with 4 screen mirror */
DRIVER_INIT( pchboard );	/* h-board games */
DRIVER_INIT( pciboard );	/* i-board games */
DRIVER_INIT( pckboard );	/* k-board games */
CUSTOM_INPUT( pc10_int_detect_r );
READ8_HANDLER( pc10_prot_r );
READ8_HANDLER( pc10_detectclr_r );
READ8_HANDLER( pc10_in0_r );
READ8_HANDLER( pc10_in1_r );
WRITE8_HANDLER( pc10_SDCS_w );
WRITE8_HANDLER( pc10_CNTRLMASK_w );
WRITE8_HANDLER( pc10_DISPMASK_w );
WRITE8_HANDLER( pc10_SOUNDMASK_w );
WRITE8_HANDLER( pc10_NMIENABLE_w );
WRITE8_HANDLER( pc10_DOGDI_w );
WRITE8_HANDLER( pc10_GAMERES_w );
WRITE8_HANDLER( pc10_GAMESTOP_w );
WRITE8_HANDLER( pc10_PPURES_w );
WRITE8_HANDLER( pc10_prot_w );
WRITE8_HANDLER( pc10_CARTSEL_w );
WRITE8_HANDLER( pc10_in0_w );

extern int pc10_nmi_enable;
extern int pc10_dog_di;
extern int pc10_sdcs;			/* ShareD Chip Select */
extern int pc10_dispmask;		/* Display Mask */
extern int pc10_int_detect;
extern int pc10_game_mode;
extern int pc10_dispmask_old;


/*----------- defined in video/playch10.c -----------*/

extern const ppu2c0x_interface playch10_ppu_interface;
extern const ppu2c0x_interface playch10_ppu_interface_hboard;

WRITE8_HANDLER( playch10_videoram_w );
PALETTE_INIT( playch10 );
VIDEO_START( playch10 );
VIDEO_START( playch10_hboard );
VIDEO_UPDATE( playch10 );
