#ifndef __CIDELSA__
#define __CIDELSA__

#include "cpu/cdp1802/cdp1802.h"
#include "video/cdp1869.h"

#define MAIN_CPU_TAG	"main"
#define DESTRYER_CHR1	3579000.0 // unverified
#define DESTRYER_CHR2	XTAL_5_7143MHz
#define ALTAIR_CHR1		3579000.0 // unverified
#define ALTAIR_CHR2		CDP1869_DOT_CLK_PAL // unverified
#define DRACO_CHR1		XTAL_4_43361MHz
#define DRACO_CHR2		CDP1869_DOT_CLK_PAL // unverified
#define DRACO_SND_CHR1	XTAL_2_01216MHz

typedef struct _cidelsa_state cidelsa_state;

struct _cidelsa_state
{
	/* cpu state */
	cdp1802_control_mode cdp1802_mode;

	/* video state */
	int cdp1802_q;
	int cdp1869_prd;
	int cdp1869_pcb;

	UINT8 *pageram;
	UINT8 *pcbram;
	UINT8 *charram;

	/* sound state */
	int draco_sound;
	int draco_ay_latch;

	/* devices */
	const device_config *cdp1869;
};

/*----------- defined in video/cidelsa.c -----------*/

MACHINE_DRIVER_EXTERN( destryer_video );
MACHINE_DRIVER_EXTERN( altair_video );
MACHINE_DRIVER_EXTERN( draco_video );

#endif
