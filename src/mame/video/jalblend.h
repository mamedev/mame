void jal_blend_init(running_machine &machine, int enable);
void jal_blend_set(int color, UINT8 val);

rgb_t jal_blend_func(rgb_t dest, rgb_t addMe, UINT8 alpha);
void jal_blend_drawgfx(palette_device &palette,bitmap_ind16 &dest_bmp,const rectangle &clip,gfx_element *gfx,
							UINT32 code,UINT32 color,int flipx,int flipy,int offsx,int offsy,
							int transparent_color);
void jal_blend_drawgfx(palette_device &palette,bitmap_rgb32 &dest_bmp,const rectangle &clip,gfx_element *gfx,
							UINT32 code,UINT32 color,int flipx,int flipy,int offsx,int offsy,
							int transparent_color);
