/***************************************************************************

                            -= Jaleco Mega System 1 =-

                    driver by   Luca Elia (l.elia@tin.it)


    This file contains definitions used across multiple megasys1
    and non megasys1 Jaleco games:

    * Scrolling layers handling
    * Code decryption handling

***************************************************************************/


/***************************************************************************

                            Scrolling Layers Handling

***************************************************************************/

/*----------- defined in video/megasys1.c -----------*/

/* Variables */
extern UINT16 *megasys1_scrollram[3];
extern UINT16 *megasys1_objectram, *megasys1_vregs, *megasys1_ram;


/* Functions */
VIDEO_START( megasys1 );
SCREEN_UPDATE( megasys1 );
SCREEN_EOF( megasys1 );

PALETTE_INIT( megasys1 );

READ16_HANDLER( megasys1_vregs_C_r );

WRITE16_HANDLER( megasys1_vregs_A_w );
WRITE16_HANDLER( megasys1_vregs_C_w );
WRITE16_HANDLER( megasys1_vregs_D_w );

WRITE16_HANDLER( megasys1_scrollram_0_w );
WRITE16_HANDLER( megasys1_scrollram_1_w );
WRITE16_HANDLER( megasys1_scrollram_2_w );
