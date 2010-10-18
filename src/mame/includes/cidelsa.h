#ifndef __CIDELSA__
#define __CIDELSA__

#include "cpu/cosmac/cosmac.h"

#define SCREEN_TAG	"screen"
#define CDP1802_TAG	"cdp1802"
#define CDP1869_TAG	"cdp1869"
#define COP402N_TAG	"cop402n"
#define AY8910_TAG	"ay8910"

#define DESTRYER_CHR1	3579000.0 // unverified
#define DESTRYER_CHR2	XTAL_5_7143MHz
#define ALTAIR_CHR1		3579000.0 // unverified
#define ALTAIR_CHR2		CDP1869_DOT_CLK_PAL // unverified
#define DRACO_CHR1		XTAL_4_43361MHz
#define DRACO_CHR2		CDP1869_DOT_CLK_PAL // unverified
#define DRACO_SND_CHR1	XTAL_2_01216MHz

#define CIDELSA_PAGERAM_SIZE	0x400
#define DRACO_PAGERAM_SIZE		0x800
#define CIDELSA_CHARRAM_SIZE	0x800

#define CIDELSA_PAGERAM_MASK	0x3ff
#define DRACO_PAGERAM_MASK		0x7ff
#define CIDELSA_CHARRAM_MASK	0x7ff

class cidelsa_state : public driver_device
{
public:
	cidelsa_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* cpu state */
	int reset;

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
	running_device *cdp1802;
	running_device *cdp1869;
};

/*----------- defined in video/cidelsa.c -----------*/

MACHINE_CONFIG_EXTERN( destryer_video );
MACHINE_CONFIG_EXTERN( altair_video );
MACHINE_CONFIG_EXTERN( draco_video );

#endif
