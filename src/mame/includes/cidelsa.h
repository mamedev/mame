#ifndef __CIDELSA__
#define __CIDELSA__

#include "cpu/cdp1802/cdp1802.h"

#define SCREEN_TAG	"screen"
#define CDP1802_TAG	"cdp1802"
#define CDP1869_TAG	"cdp1869"
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

class cidelsa_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, cidelsa_state(machine)); }

	cidelsa_state(running_machine &machine)
		: driver_data_t(machine) { }

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
	running_device *cdp1802;
	running_device *cdp1869;
};

/*----------- defined in video/cidelsa.c -----------*/

MACHINE_DRIVER_EXTERN( destryer_video );
MACHINE_DRIVER_EXTERN( altair_video );
MACHINE_DRIVER_EXTERN( draco_video );

#endif
