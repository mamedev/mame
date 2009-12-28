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
extern tilemap_t *megasys1_tmap[3];

extern UINT16 *megasys1_scrollram[3];
extern UINT16 *megasys1_objectram, *megasys1_vregs, *megasys1_ram;

extern int megasys1_scrollx[3], megasys1_scrolly[3];
extern int megasys1_active_layers;
//extern int megasys1_screen_flag, megasys1_sprite_flag;
extern int megasys1_bits_per_color_code;


/* Functions */
VIDEO_START( megasys1 );
VIDEO_UPDATE( megasys1 );
VIDEO_EOF( megasys1 );

PALETTE_INIT( megasys1 );

READ16_HANDLER( megasys1_vregs_C_r );

WRITE16_HANDLER( megasys1_vregs_A_w );
WRITE16_HANDLER( megasys1_vregs_C_w );
WRITE16_HANDLER( megasys1_vregs_D_w );

WRITE16_HANDLER( megasys1_scrollram_0_w );
WRITE16_HANDLER( megasys1_scrollram_1_w );
WRITE16_HANDLER( megasys1_scrollram_2_w );

void megasys1_set_vreg_flag(int which, int data);


/*----------- defined in drivers/megasys1.c -----------*/

void astyanax_rom_decode(running_machine *machine, const char *region);
void phantasm_rom_decode(running_machine *machine, const char *region);
void rodland_rom_decode (running_machine *machine, const char *region);
