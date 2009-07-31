/*----------- defined in video/vsnes.c -----------*/

VIDEO_START( vsnes );
PALETTE_INIT( vsnes );
VIDEO_UPDATE( vsnes );
VIDEO_START( vsdual );
VIDEO_UPDATE( vsdual );
PALETTE_INIT( vsdual );

extern const ppu2c0x_interface vsnes_ppu_interface_1;
extern const ppu2c0x_interface vsnes_ppu_interface_2;


/*----------- defined in machine/vsnes.c -----------*/

MACHINE_RESET( vsnes );
MACHINE_RESET( vsdual );
MACHINE_START( vsnes );
MACHINE_START( vsdual );
DRIVER_INIT( suprmrio );
DRIVER_INIT( excitebk );
DRIVER_INIT( excitbkj );
DRIVER_INIT( vsnormal );
DRIVER_INIT( duckhunt );
DRIVER_INIT( hogalley );
DRIVER_INIT( goonies );
DRIVER_INIT( machridr );
DRIVER_INIT( vsslalom );
DRIVER_INIT( cstlevna );
DRIVER_INIT( drmario );
DRIVER_INIT( rbibb );
DRIVER_INIT( tkoboxng );
DRIVER_INIT( topgun );
DRIVER_INIT( vsgradus );
DRIVER_INIT( vspinbal );
DRIVER_INIT( MMC3 );
DRIVER_INIT( platoon );
DRIVER_INIT( vstennis );
DRIVER_INIT( wrecking );
DRIVER_INIT( balonfgt );
DRIVER_INIT( vsbball );
DRIVER_INIT( iceclmrj );
DRIVER_INIT( supxevs );
DRIVER_INIT( btlecity );
DRIVER_INIT( vstetris );
DRIVER_INIT( bnglngby );
DRIVER_INIT( jajamaru);
DRIVER_INIT( vsgshoe );
DRIVER_INIT( vsfdf );
DRIVER_INIT( mightybj);

READ8_HANDLER( vsnes_in0_r );
READ8_HANDLER( vsnes_in1_r );
READ8_HANDLER( vsnes_in0_1_r );
READ8_HANDLER( vsnes_in1_1_r );
WRITE8_HANDLER( vsnes_in0_w );
WRITE8_HANDLER( vsnes_in0_1_w );
