/* Tecmo Sprites */

void galspnbl_draw_sprites( screen_device &screen, gfxdecode_device *gfxdecode, bitmap_ind16 &bitmap, const rectangle &cliprect, int priority, UINT16* spriteram, int spriteram_bytes );
void tecmo16_draw_sprites(screen_device &screen, gfxdecode_device *gfxdecode, bitmap_ind16 &bitmap_bg, bitmap_ind16 &bitmap_fg, bitmap_ind16 &bitmap_sp, const rectangle &cliprect, UINT16* spriteram, UINT16 spriteram16_bytes, int game_is_riot, int flipscreen );
void gaiden_draw_sprites( screen_device &screen, gfxdecode_device *gfxdecode, bitmap_ind16 &bitmap_bg, bitmap_ind16 &bitmap_fg, bitmap_ind16 &bitmap_sp, const rectangle &cliprect, UINT16* spriteram, int sprite_sizey, int spr_offset_y, int flipscreen );
void raiga_draw_sprites( screen_device &screen, gfxdecode_device *gfxdecode, bitmap_ind16 &bitmap_bg, bitmap_ind16 &bitmap_fg, bitmap_ind16 &bitmap_sp, const rectangle &cliprect, UINT16* spriteram, int sprite_sizey, int spr_offset_y, int flipscreen );
int spbactn_draw_sprites(screen_device &screen, gfxdecode_device *gfxdecode, bitmap_ind16 &bitmap, const rectangle &cliprect, int priority, bool alt_sprites, UINT16* spriteram);
