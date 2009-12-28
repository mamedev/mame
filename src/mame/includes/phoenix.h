#include "devcb.h"
#include "sound/discrete.h"


/*----------- video timing  -----------*/

#define MASTER_CLOCK			XTAL_11MHz

#define PIXEL_CLOCK				(MASTER_CLOCK/2)
#define CPU_CLOCK				(PIXEL_CLOCK)
#define HTOTAL					(512-160)
#define HBSTART					(256)
#define HBEND					(0)
#define VTOTAL					(256)
#define VBSTART					(208)
#define VBEND					(0)

/*----------- defined in audio/phoenix.c -----------*/

SOUND_START( phoenix );

DISCRETE_SOUND_EXTERN( phoenix );

WRITE8_DEVICE_HANDLER( phoenix_sound_control_a_w );
WRITE8_DEVICE_HANDLER( phoenix_sound_control_b_w );

DEVICE_GET_INFO( phoenix_sound );
#define SOUND_PHOENIX DEVICE_GET_INFO_NAME(phoenix_sound)

/*----------- defined in audio/pleiads.c -----------*/

WRITE8_HANDLER( pleiads_sound_control_a_w );
WRITE8_HANDLER( pleiads_sound_control_b_w );
WRITE8_HANDLER( pleiads_sound_control_c_w );

DEVICE_GET_INFO( pleiads_sound );
#define SOUND_PLEIADS DEVICE_GET_INFO_NAME(pleiads_sound)

DEVICE_GET_INFO( naughtyb_sound );
#define SOUND_NAUGHTYB DEVICE_GET_INFO_NAME(naughtyb_sound)

DEVICE_GET_INFO( popflame_sound );
#define SOUND_POPFLAME DEVICE_GET_INFO_NAME(popflame_sound)

/*----------- defined in video/naughtyb.c -----------*/

extern UINT8 *naughtyb_videoram2;
extern UINT8 *naughtyb_scrollreg;
extern int naughtyb_cocktail;

WRITE8_HANDLER( naughtyb_videoreg_w );
WRITE8_HANDLER( popflame_videoreg_w );

VIDEO_START( naughtyb );
PALETTE_INIT( naughtyb );
VIDEO_UPDATE( naughtyb );


/*----------- defined in video/phoenix.c -----------*/

PALETTE_INIT( phoenix );
PALETTE_INIT( survival );
PALETTE_INIT( pleiads );
VIDEO_START( phoenix );
VIDEO_UPDATE( phoenix );

WRITE8_HANDLER( phoenix_videoram_w );
WRITE8_HANDLER( phoenix_videoreg_w );
WRITE8_HANDLER( pleiads_videoreg_w );
WRITE8_HANDLER( phoenix_scroll_w );

CUSTOM_INPUT( player_input_r );
CUSTOM_INPUT( pleiads_protection_r );
READ8_HANDLER( survival_input_port_0_r );
READ8_DEVICE_HANDLER( survival_protection_r );

READ_LINE_DEVICE_HANDLER( survival_sid_callback );

