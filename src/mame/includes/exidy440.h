/*************************************************************************

    Exidy 440 hardware

*************************************************************************/

#define EXIDY440_MASTER_CLOCK		(XTAL_12_9792MHz)


/*----------- defined in drivers/exidy440.c -----------*/

void exidy440_bank_select(running_machine *machine, UINT8 bank);


/*----------- defined in audio/exidy440.c -----------*/

extern UINT8 exidy440_sound_command;
extern UINT8 exidy440_sound_command_ack;

MACHINE_CONFIG_EXTERN( exidy440_audio );


/*----------- defined in video/exidy440.c -----------*/

extern UINT8 *exidy440_imageram;
extern UINT8 *exidy440_scanline;
extern UINT8  exidy440_firq_vblank;
extern UINT8  exidy440_firq_beam;
extern UINT8 *topsecex_yscroll;

INTERRUPT_GEN( exidy440_vblank_interrupt );

READ8_HANDLER( exidy440_videoram_r );
WRITE8_HANDLER( exidy440_videoram_w );
READ8_HANDLER( exidy440_paletteram_r );
WRITE8_HANDLER( exidy440_paletteram_w );
WRITE8_HANDLER( exidy440_spriteram_w );
WRITE8_HANDLER( exidy440_control_w );
READ8_HANDLER( exidy440_vertical_pos_r );
READ8_HANDLER( exidy440_horizontal_pos_r );
WRITE8_HANDLER( exidy440_interrupt_clear_w );

MACHINE_CONFIG_EXTERN( exidy440_video );
MACHINE_CONFIG_EXTERN( topsecex_video );
