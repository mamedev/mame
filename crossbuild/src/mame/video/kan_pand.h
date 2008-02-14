/* kan_pand.h */

void pandora_start(UINT8 region, int x, int y);
void pandora_update(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect);
void pandora_eof(running_machine *machine);
void pandora_set_clear_bitmap(int clear);

WRITE8_HANDLER ( pandora_spriteram_w );
READ8_HANDLER( pandora_spriteram_r );
WRITE16_HANDLER( pandora_spriteram_LSB_w );
READ16_HANDLER( pandora_spriteram_LSB_r );
