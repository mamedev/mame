#ifndef __SEGAIC_H
#define __SEGAIC_H

/* system24temp_ functions / variables are from shared rewrite files,
   once the rest of the rewrite is complete they can be removed, I
   just made a copy & renamed them for now to avoid any conflicts
*/

//#include "vmixer.h"

WRITE16_HANDLER (system24temp_sys16_paletteram1_w);

// Tilemaps
//  System24
void sys24_tile_vh_start(running_machine *machine, UINT16 tile_mask);
void sys24_tile_update(running_machine *machine);
void sys24_tile_draw(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect, int layer, int pri, int flags);

READ16_HANDLER(sys24_tile_r);
READ16_HANDLER(sys24_char_r);
WRITE16_HANDLER(sys24_tile_w);
WRITE16_HANDLER(sys24_char_w);

READ32_HANDLER(sys24_tile32_r);
READ32_HANDLER(sys24_char32_r);
WRITE32_HANDLER(sys24_tile32_w);
WRITE32_HANDLER(sys24_char32_w);

// Sprites
//  System24
void sys24_sprite_vh_start(void);
void sys24_sprite_draw(mame_bitmap *bitmap, const rectangle *cliprect, const int *spri);

WRITE16_HANDLER (sys24_sprite_w);
READ16_HANDLER (sys24_sprite_r);

// Programmable mixers
//  System24
void sys24_mixer_vh_start(void);
int sys24_mixer_get_reg(int reg);

WRITE16_HANDLER (sys24_mixer_w);
READ16_HANDLER (sys24_mixer_r);

#endif
