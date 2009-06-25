extern UINT8 *jal_blend_table;
rgb_t jal_blend_func(rgb_t dest, rgb_t addMe, UINT8 alpha);
void jal_blend_drawgfx(bitmap_t *dest_bmp,const rectangle *clip,const gfx_element *gfx,
							UINT32 code,UINT32 color,int flipx,int flipy,int offsx,int offsy,
							int transparency,int transparent_color);
