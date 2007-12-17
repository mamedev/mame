/*************************************************************************

    Exidy 440 hardware

*************************************************************************/

#include "sound/custom.h"


#define EXIDY440_MASTER_CLOCK		(12979200)
#define EXIDY440_MAIN_CPU_CLOCK		(EXIDY440_MASTER_CLOCK / 8)
#define EXIDY440_PIXEL_CLOCK		(EXIDY440_MASTER_CLOCK / 2)
#define EXIDY440_HTOTAL				(0x1a0)
#define EXIDY440_HBEND				(0x000)
#define EXIDY440_HBSTART			(0x140)
#define EXIDY440_HSEND				(0x178)
#define EXIDY440_HSSTART			(0x160)
#define EXIDY440_VTOTAL				(0x104)
#define EXIDY440_VBEND				(0x000)
#define EXIDY440_VBSTART			(0x0f0)
#define EXIDY440_VSEND				(0x0f8)
#define EXIDY440_VSSTART			(0x0f0)

/* Top Secret has a larger VBLANK area */
#define TOPSECEX_VBSTART			(0x0ec)


/*----------- defined in drivers/exidy440.c -----------*/

extern UINT8 exidy440_topsecret;
void exidy440_bank_select(UINT8 bank);



/*----------- defined in audio/exidy440.c -----------*/

extern UINT8 exidy440_sound_command;
extern UINT8 exidy440_sound_command_ack;
extern UINT8 *exidy440_m6844_data;
extern UINT8 *exidy440_sound_banks;
extern UINT8 *exidy440_sound_volume;

void *exidy440_sh_start(int clock, const struct CustomSound_interface *config);
void exidy440_sh_stop(void *token);

READ8_HANDLER( exidy440_m6844_r );
WRITE8_HANDLER( exidy440_m6844_w );
READ8_HANDLER( exidy440_sound_command_r );
WRITE8_HANDLER( exidy440_sound_volume_w );
WRITE8_HANDLER( exidy440_sound_interrupt_clear_w );


/*----------- defined in video/exidy440.c -----------*/

extern UINT8 *exidy440_imageram;
extern UINT8 *exidy440_scanline;
extern UINT8 exidy440_firq_vblank;
extern UINT8 exidy440_firq_beam;
extern UINT8 topsecex_yscroll;

INTERRUPT_GEN( exidy440_vblank_interrupt );

VIDEO_START( exidy440 );
VIDEO_EOF( exidy440 );
VIDEO_UPDATE( exidy440 );

READ8_HANDLER( exidy440_videoram_r );
WRITE8_HANDLER( exidy440_videoram_w );
READ8_HANDLER( exidy440_paletteram_r );
WRITE8_HANDLER( exidy440_paletteram_w );
WRITE8_HANDLER( exidy440_spriteram_w );
WRITE8_HANDLER( exidy440_control_w );
READ8_HANDLER( exidy440_vertical_pos_r );
READ8_HANDLER( exidy440_horizontal_pos_r );
WRITE8_HANDLER( exidy440_interrupt_clear_w );
