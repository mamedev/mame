/*************************************************************************

    Exidy 6502 hardware

*************************************************************************/

#include "devlegcy.h"

#define EXIDY_MASTER_CLOCK				(XTAL_11_289MHz)
#define EXIDY_CPU_CLOCK					(EXIDY_MASTER_CLOCK / 16)
#define EXIDY_PIXEL_CLOCK				(EXIDY_MASTER_CLOCK / 2)
#define EXIDY_HTOTAL					(0x150)
#define EXIDY_HBEND						(0x000)
#define EXIDY_HBSTART					(0x100)
#define EXIDY_HSEND						(0x140)
#define EXIDY_HSSTART					(0x120)
#define EXIDY_VTOTAL					(0x118)
#define EXIDY_VBEND						(0x000)
#define EXIDY_VBSTART					(0x100)
#define EXIDY_VSEND						(0x108)
#define EXIDY_VSSTART					(0x100)


/*----------- defined in audio/exidy.c -----------*/

DECLARE_LEGACY_SOUND_DEVICE(EXIDY, exidy_sound);
DECLARE_LEGACY_SOUND_DEVICE(EXIDY_VENTURE, venture_sound);
DECLARE_LEGACY_SOUND_DEVICE(EXIDY_VICTORY, victory_sound);

READ8_DEVICE_HANDLER( exidy_sh6840_r );
WRITE8_DEVICE_HANDLER( exidy_sh6840_w );
WRITE8_DEVICE_HANDLER( exidy_sfxctrl_w );

MACHINE_CONFIG_EXTERN( venture_audio );

MACHINE_CONFIG_EXTERN( mtrap_cvsd_audio );

MACHINE_CONFIG_EXTERN( victory_audio );
READ8_DEVICE_HANDLER( victory_sound_response_r );
READ8_DEVICE_HANDLER( victory_sound_status_r );
WRITE8_DEVICE_HANDLER( victory_sound_command_w );



/*----------- defined in video/exidy.c -----------*/

extern UINT8 *exidy_videoram;
extern UINT8 *exidy_characterram;
extern UINT8 *exidy_color_latch;
extern UINT8 *exidy_sprite1_xpos;
extern UINT8 *exidy_sprite1_ypos;
extern UINT8 *exidy_sprite2_xpos;
extern UINT8 *exidy_sprite2_ypos;
extern UINT8 *exidy_spriteno;
extern UINT8 *exidy_sprite_enable;

void exidy_video_config(UINT8 _collision_mask, UINT8 _collision_invert, int _is_2bpp);
VIDEO_START( exidy );
VIDEO_UPDATE( exidy );

INTERRUPT_GEN( exidy_vblank_interrupt );
INTERRUPT_GEN( teetert_vblank_interrupt );

READ8_HANDLER( exidy_interrupt_r );
