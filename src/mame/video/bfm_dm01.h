/************************************************************************************

    Bellfruit dotmatrix driver, (under heavy construction !!!)

*************************************************************************************/
#ifndef BFM_DM01
#define BFM_DM01

ADDRESS_MAP_EXTERN( bfm_dm01_memmap,8 );

INTERRUPT_GEN( bfm_dm01_vbl );

VIDEO_START(  bfm_dm01 );
VIDEO_UPDATE( bfm_dm01 );
PALETTE_INIT( bfm_dm01 );

void BFM_dm01_reset(void);

void BFM_dm01_writedata(running_machine *machine,UINT8 data);

int BFM_dm01_busy(void);

#endif
