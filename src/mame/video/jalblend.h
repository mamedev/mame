extern UINT8 *jal_blend_table;
rgb_t jal_blend_func(rgb_t dest, rgb_t addMe, UINT8 alpha);
void jal_blend_drawgfx(running_machine *machine,bitmap_t *dest_bmp,const gfx_element *gfx,
							UINT32 code,UINT32 color,int flipx,int flipy,int offsx,int offsy,
							const rectangle *clip,int transparency,int transparent_color);
