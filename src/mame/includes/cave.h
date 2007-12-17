/***************************************************************************

    Cave hardware

***************************************************************************/

/*----------- defined in video/cave.c -----------*/

extern int cave_spritetype;
extern int cave_kludge;

extern UINT16 *cave_videoregs;

extern UINT16 *cave_vram_0, *cave_vctrl_0;
extern UINT16 *cave_vram_1, *cave_vctrl_1;
extern UINT16 *cave_vram_2, *cave_vctrl_2;
extern UINT16 *cave_vram_3, *cave_vctrl_3;

WRITE16_HANDLER( cave_vram_0_w );
WRITE16_HANDLER( cave_vram_1_w );
WRITE16_HANDLER( cave_vram_2_w );
WRITE16_HANDLER( cave_vram_3_w );

WRITE16_HANDLER( cave_vram_0_8x8_w );
WRITE16_HANDLER( cave_vram_1_8x8_w );
WRITE16_HANDLER( cave_vram_2_8x8_w );
WRITE16_HANDLER( cave_vram_3_8x8_w );

PALETTE_INIT( ddonpach );
PALETTE_INIT( dfeveron );
PALETTE_INIT( mazinger );
PALETTE_INIT( sailormn );
PALETTE_INIT( pwrinst2 );
PALETTE_INIT( korokoro );

VIDEO_START( cave_1_layer );
VIDEO_START( cave_2_layers );
VIDEO_START( cave_3_layers );
VIDEO_START( cave_4_layers );

VIDEO_START( sailormn_3_layers );


VIDEO_UPDATE( cave );

void cave_get_sprite_info(running_machine *machine);

void sailormn_tilebank_w( int bank );
