/*----------- defined in drivers/tumbleb.c -----------*/

extern UINT16* jumppop_control;
extern UINT16* suprtrio_control;


/*----------- defined in video/tumbleb.c -----------*/

extern UINT16 *tumblepb_pf1_data,*tumblepb_pf2_data;

VIDEO_START( tumblepb );
VIDEO_START( fncywld );
VIDEO_START( jumppop );
VIDEO_START( sdfight );
VIDEO_UPDATE( tumblepb );
VIDEO_UPDATE( jumpkids );
VIDEO_UPDATE( fncywld );
VIDEO_UPDATE( jumppop );
VIDEO_UPDATE( semicom );
VIDEO_UPDATE( semicom_altoffsets );
VIDEO_UPDATE( bcstory );
VIDEO_UPDATE(semibase );
VIDEO_START( suprtrio );
VIDEO_UPDATE( suprtrio );
VIDEO_START( pangpang );
VIDEO_UPDATE( pangpang );
VIDEO_UPDATE( sdfight );

WRITE16_HANDLER( tumblepb_pf1_data_w );
WRITE16_HANDLER( tumblepb_pf2_data_w );
WRITE16_HANDLER( fncywld_pf1_data_w );
WRITE16_HANDLER( fncywld_pf2_data_w );
WRITE16_HANDLER( tumblepb_control_0_w );
WRITE16_HANDLER( pangpang_pf1_data_w );
WRITE16_HANDLER( pangpang_pf2_data_w );

WRITE16_HANDLER( bcstory_tilebank_w );
WRITE16_HANDLER( suprtrio_tilebank_w );
WRITE16_HANDLER( chokchok_tilebank_w );
WRITE16_HANDLER( wlstar_tilebank_w );
