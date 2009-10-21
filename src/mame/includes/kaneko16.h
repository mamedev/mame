/***************************************************************************

                            -= Kaneko 16 Bit Games =-

***************************************************************************/

/*----------- defined in machine/kaneko16.c -----------*/

extern UINT16 *kaneko16_mcu_ram; /* for calc3 and toybox */

READ16_HANDLER( galpanib_calc_r );
WRITE16_HANDLER( galpanib_calc_w );

READ16_HANDLER( bloodwar_calc_r );
WRITE16_HANDLER( bloodwar_calc_w );

void calc3_mcu_init(void);
WRITE16_HANDLER( calc3_mcu_ram_w );
WRITE16_HANDLER( calc3_mcu_com0_w );
WRITE16_HANDLER( calc3_mcu_com1_w );
WRITE16_HANDLER( calc3_mcu_com2_w );
WRITE16_HANDLER( calc3_mcu_com3_w );

void toybox_mcu_init(void);
WRITE16_HANDLER( toybox_mcu_com0_w );
WRITE16_HANDLER( toybox_mcu_com1_w );
WRITE16_HANDLER( toybox_mcu_com2_w );
WRITE16_HANDLER( toybox_mcu_com3_w );
READ16_HANDLER( toybox_mcu_status_r );

extern void (*toybox_mcu_run)(running_machine *machine);	/* one of the following */
void bloodwar_mcu_run(running_machine *machine);
void bonkadv_mcu_run(running_machine *machine);
void gtmr_mcu_run(running_machine *machine);
void calc3_mcu_run(running_machine *machine);

void toxboy_handle_04_subcommand(running_machine* machine,UINT8 mcu_subcmd, UINT16*mcu_ram);

DRIVER_INIT( decrypt_toybox_rom );
DRIVER_INIT( decrypt_toybox_rom_alt );
DRIVER_INIT( calc3_scantables );



/*----------- defined in drivers/kaneko16.c -----------*/

MACHINE_RESET( kaneko16 );

/*----------- defined in video/kaneko16.c -----------*/

WRITE16_HANDLER( kaneko16_display_enable );

/* Tile Layers: */

extern UINT16 *kaneko16_vram_0,    *kaneko16_vram_1,    *kaneko16_layers_0_regs;
extern UINT16 *kaneko16_vscroll_0, *kaneko16_vscroll_1;
extern UINT16 *kaneko16_vram_2,    *kaneko16_vram_3,    *kaneko16_layers_1_regs;
extern UINT16 *kaneko16_vscroll_2, *kaneko16_vscroll_3;

WRITE16_HANDLER( kaneko16_vram_0_w );
WRITE16_HANDLER( kaneko16_vram_1_w );
WRITE16_HANDLER( kaneko16_vram_2_w );
WRITE16_HANDLER( kaneko16_vram_3_w );

WRITE16_HANDLER( kaneko16_layers_0_regs_w );
WRITE16_HANDLER( kaneko16_layers_1_regs_w );


/* Sprites: */

extern int kaneko16_sprite_type;
extern int kaneko16_sprite_fliptype;
extern UINT16 kaneko16_sprite_xoffs, kaneko16_sprite_flipx;
extern UINT16 kaneko16_sprite_yoffs, kaneko16_sprite_flipy;
extern UINT16 *kaneko16_sprites_regs;

READ16_HANDLER ( kaneko16_sprites_regs_r );
WRITE16_HANDLER( kaneko16_sprites_regs_w );

void kaneko16_draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect);

/* Pixel Layer: */

extern UINT16 *kaneko16_bg15_select, *kaneko16_bg15_reg;

READ16_HANDLER ( kaneko16_bg15_select_r );
WRITE16_HANDLER( kaneko16_bg15_select_w );

READ16_HANDLER ( kaneko16_bg15_reg_r );
WRITE16_HANDLER( kaneko16_bg15_reg_w );

PALETTE_INIT( berlwall );


/* Priorities: */

typedef struct
{
	int VIEW2_2_pri;
	int sprite[4];
}	kaneko16_priority_t;

extern kaneko16_priority_t kaneko16_priority;


/* Machine */


VIDEO_START( kaneko16_sprites );
VIDEO_START( kaneko16_1xVIEW2_tilemaps );
VIDEO_START( kaneko16_1xVIEW2 );
VIDEO_START( kaneko16_2xVIEW2 );
VIDEO_START( berlwall );
VIDEO_START( sandscrp_1xVIEW2 );


VIDEO_UPDATE( kaneko16 );
VIDEO_UPDATE( sandscrp );
VIDEO_UPDATE( berlwall );
VIDEO_UPDATE( jchan_view2 );

VIDEO_START( galsnew );
VIDEO_UPDATE( galsnew );

extern UINT16* galsnew_bg_pixram;
extern UINT16* galsnew_fg_pixram;


/*----------- defined in video/galpani2.c -----------*/

extern UINT16 *galpani2_bg8_0,         *galpani2_bg8_1;
extern UINT16 *galpani2_palette_0,     *galpani2_palette_1;
extern UINT16 *galpani2_bg8_0_scrollx, *galpani2_bg8_1_scrollx;
extern UINT16 *galpani2_bg8_0_scrolly, *galpani2_bg8_1_scrolly;

extern UINT16 *galpani2_bg15;

PALETTE_INIT( galpani2 );
VIDEO_START( galpani2 );
VIDEO_UPDATE( galpani2 );

WRITE16_HANDLER( galpani2_palette_0_w );
WRITE16_HANDLER( galpani2_palette_1_w );

WRITE16_HANDLER( galpani2_bg8_0_w );
WRITE16_HANDLER( galpani2_bg8_1_w );

WRITE16_HANDLER( galpani2_bg15_w );
