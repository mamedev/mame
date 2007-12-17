/*************************************************************************

    VIC Dual Game board

*************************************************************************/

#include "sound/discrete.h"


#define VICDUAL_MASTER_CLOCK				(15468480)
#define VICDUAL_MAIN_CPU_CLOCK				(VICDUAL_MASTER_CLOCK/8)
#define VICDUAL_PIXEL_CLOCK					(VICDUAL_MASTER_CLOCK/3)
#define VICDUAL_HTOTAL						(0x148)
#define VICDUAL_HBEND						(0x000)
#define VICDUAL_HBSTART						(0x100)
#define VICDUAL_HSSTART						(0x110)
#define VICDUAL_HSEND						(0x130)
#define VICDUAL_VTOTAL						(0x106)
#define VICDUAL_VBEND						(0x000)
#define VICDUAL_VBSTART						(0x0e0)
#define VICDUAL_VSSTART						(0x0ec)
#define VICDUAL_VSEND						(0x0f0)


/*----------- defined in drivers/vicdual.c -----------*/

UINT8 vicdual_videoram_r(offs_t offset);
UINT8 vicdual_characterram_r(offs_t offset);

int vicdual_is_cabinet_color(void);


/*----------- defined in video/vicdual.c -----------*/

WRITE8_HANDLER( vicdual_palette_bank_w );

VIDEO_UPDATE( vicdual_bw );
VIDEO_UPDATE( vicdual_color );
VIDEO_UPDATE( vicdual_bw_or_color );


/*----------- defined in audio/vicdual.c -----------*/

MACHINE_START( frogs_audio );
MACHINE_DRIVER_EXTERN( frogs_audio );
MACHINE_DRIVER_EXTERN( headon_audio );
WRITE8_HANDLER( frogs_audio_w );
WRITE8_HANDLER( headon_audio_w );
WRITE8_HANDLER( invho2_audio_w );


/*----------- defined in audio/depthch.c -----------*/

MACHINE_DRIVER_EXTERN( depthch_audio );
WRITE8_HANDLER( depthch_audio_w );


/*----------- defined in audio/carnival.c -----------*/

MACHINE_DRIVER_EXTERN( carnival_audio );
WRITE8_HANDLER( carnival_audio_1_w );
WRITE8_HANDLER( carnival_audio_2_w );


/*----------- defined in audio/invinco.c -----------*/

MACHINE_DRIVER_EXTERN( invinco_audio );
WRITE8_HANDLER( invinco_audio_w );


/*----------- defined in audio/pulsar.c -----------*/

MACHINE_DRIVER_EXTERN( pulsar_audio );
WRITE8_HANDLER( pulsar_audio_1_w );
WRITE8_HANDLER( pulsar_audio_2_w );
