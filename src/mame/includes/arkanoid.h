/* This it the best way to allow game specific kludges until the system is fully understood */
enum {
	ARKUNK=0,  /* unknown bootlegs for inclusion of possible new sets */
	ARKANGC,
	ARKANGC2,
	ARKBLOCK,
	ARKBLOC2,
	ARKGCBL,
	PADDLE2
};


/*----------- defined in drivers/arkanoid.c -----------*/

extern int arkanoid_bootleg_id;


/*----------- defined in video/arkanoid.c -----------*/

extern WRITE8_HANDLER( arkanoid_videoram_w );

extern WRITE8_HANDLER( arkanoid_d008_w );

extern VIDEO_START( arkanoid );
extern VIDEO_UPDATE( arkanoid );


/*----------- defined in machine/arkanoid.c -----------*/

extern MACHINE_START( arkanoid );
extern MACHINE_RESET( arkanoid );

extern READ8_HANDLER( arkanoid_Z80_mcu_r );
extern WRITE8_HANDLER( arkanoid_Z80_mcu_w );

extern READ8_HANDLER( arkanoid_68705_portA_r );
extern WRITE8_HANDLER( arkanoid_68705_portA_w );
extern WRITE8_HANDLER( arkanoid_68705_ddrA_w );

extern READ8_HANDLER( arkanoid_68705_portC_r );
extern WRITE8_HANDLER( arkanoid_68705_portC_w );
extern WRITE8_HANDLER( arkanoid_68705_ddrC_w );

extern READ8_HANDLER( arkanoid_68705_input_0_r );
extern READ8_HANDLER( arkanoid_input_2_r );

extern READ8_HANDLER( arkanoid_bootleg_f002_r );
extern WRITE8_HANDLER( arkanoid_bootleg_d018_w );
extern READ8_HANDLER( arkanoid_bootleg_d008_r );

