/*************************************************************************

    Irem M92 hardware

*************************************************************************/

/*----------- defined in drivers/m92.c -----------*/

extern void m92_sprite_interrupt(running_machine *machine);


/*----------- defined in video/m92.c -----------*/

extern UINT32 m92_raster_irq_position;
extern UINT16 *m92_vram_data, *m92_spritecontrol;

extern UINT8 m92_sprite_buffer_busy, m92_game_kludge;

WRITE16_HANDLER( m92_spritecontrol_w );
WRITE16_HANDLER( m92_videocontrol_w );
READ16_HANDLER( m92_paletteram_r );
WRITE16_HANDLER( m92_paletteram_w );
WRITE16_HANDLER( m92_vram_w );
WRITE16_HANDLER( m92_pf1_control_w );
WRITE16_HANDLER( m92_pf2_control_w );
WRITE16_HANDLER( m92_pf3_control_w );
WRITE16_HANDLER( m92_master_control_w );

VIDEO_START( m92 );
VIDEO_UPDATE( m92 );
