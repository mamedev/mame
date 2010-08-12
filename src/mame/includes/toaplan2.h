/* vdp related */

// mixing debug, render each VDP to it's own screen - be sure to recompile both driver and video after changing
#define DUAL_SCREEN_VDPS

#include "video/gp9001.h"

// cache the vdps for faster access
class toaplan2_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, toaplan2_state(machine)); }

	toaplan2_state(running_machine &machine)
		: driver_data_t(machine)
	{
		vdp0 = NULL;
		vdp1 = NULL;
	}

	gp9001vdp_device* vdp0;
	gp9001vdp_device* vdp1;
};


/*----------- defined in audio/toaplan2.c -----------*/

void dogyuun_okisnd_w(running_device *device, int data);
void kbash_okisnd_w(running_device *device, int data);
void fixeight_okisnd_w(running_device *device, int data);
void batsugun_okisnd_w(running_device *device, int data);

/*----------- defined in drivers/toaplan2.c -----------*/

extern int toaplan2_sub_cpu;

/*----------- defined in video/toaplan2.c -----------*/

extern UINT16 *toaplan2_txvideoram16;
extern UINT16 *toaplan2_txvideoram16_offs;
extern UINT16 *toaplan2_txscrollram16;
extern UINT16 *toaplan2_tx_gfxram16;

extern  size_t toaplan2_tx_vram_size;
extern  size_t toaplan2_tx_offs_vram_size;
extern  size_t toaplan2_tx_scroll_vram_size;
extern  size_t batrider_paletteram16_size;

VIDEO_EOF( toaplan2 );
VIDEO_START( toaplan2 );
VIDEO_START( truxton2 );
VIDEO_START( fixeighb );
VIDEO_START( bgaregga );
VIDEO_START( batrider );

VIDEO_UPDATE( toaplan2 );
VIDEO_UPDATE( truxton2 );
VIDEO_UPDATE( batrider );
VIDEO_UPDATE( dogyuun );
VIDEO_UPDATE( batsugun );

/* non-vdp text layer */
READ16_HANDLER ( toaplan2_txvideoram16_r );
WRITE16_HANDLER( toaplan2_txvideoram16_w );
READ16_HANDLER ( toaplan2_txvideoram16_offs_r );
WRITE16_HANDLER( toaplan2_txvideoram16_offs_w );
READ16_HANDLER ( toaplan2_txscrollram16_r );
WRITE16_HANDLER( toaplan2_txscrollram16_w );
READ16_HANDLER ( toaplan2_tx_gfxram16_r );
WRITE16_HANDLER( toaplan2_tx_gfxram16_w );
READ16_HANDLER ( raizing_tx_gfxram16_r );
WRITE16_HANDLER( raizing_tx_gfxram16_w );

WRITE16_HANDLER( batrider_objectbank_w );
WRITE16_HANDLER( batrider_textdata_decode );





