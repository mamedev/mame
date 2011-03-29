void jal_blend_init(running_machine &machine, int enable);
void jal_blend_set(int color, UINT8 val);

rgb_t jal_blend_func(rgb_t dest, rgb_t addMe, UINT8 alpha);
void jal_blend_drawgfx(bitmap_t *dest_bmp,const rectangle *clip,const gfx_element *gfx,
							UINT32 code,UINT32 color,int flipx,int flipy,int offsx,int offsy,
							int transparent_color);
