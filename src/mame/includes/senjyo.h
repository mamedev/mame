#include "sound/samples.h"
#include "cpu/z80/z80daisy.h"
#include "machine/z80pio.h"
#include "machine/z80ctc.h"

/*----------- defined in audio/senjyo.c -----------*/

extern const z80_daisy_config senjyo_daisy_chain[];
extern const z80pio_interface senjyo_pio_intf;
extern const z80ctc_interface senjyo_ctc_intf;

SAMPLES_START( senjyo_sh_start );
WRITE8_HANDLER( senjyo_volume_w );


/*----------- defined in video/senjyo.c -----------*/

extern UINT8 *senjyo_fgscroll;
extern UINT8 *senjyo_scrollx1,*senjyo_scrolly1;
extern UINT8 *senjyo_scrollx2,*senjyo_scrolly2;
extern UINT8 *senjyo_scrollx3,*senjyo_scrolly3;
extern UINT8 *senjyo_fgvideoram,*senjyo_fgcolorram;
extern UINT8 *senjyo_bg1videoram,*senjyo_bg2videoram,*senjyo_bg3videoram;
extern UINT8 *senjyo_radarram;
extern UINT8 *senjyo_bgstripesram;
extern int is_senjyo, senjyo_scrollhack;

WRITE8_HANDLER( senjyo_fgvideoram_w );
WRITE8_HANDLER( senjyo_fgcolorram_w );
WRITE8_HANDLER( senjyo_bg1videoram_w );
WRITE8_HANDLER( senjyo_bg2videoram_w );
WRITE8_HANDLER( senjyo_bg3videoram_w );
WRITE8_HANDLER( senjyo_bgstripes_w );

VIDEO_START( senjyo );
VIDEO_UPDATE( senjyo );
