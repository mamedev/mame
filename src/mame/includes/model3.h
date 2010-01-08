/*----------- defined in drivers/model3.c -----------*/

extern UINT32 *model3_vrom;
extern int model3_step;

void model3_set_irq_line(running_machine *machine, UINT8 bit, int state);


/*----------- defined in machine/model3.c -----------*/

extern void model3_machine_init(int step);
extern int model3_tap_read(void);
extern void model3_tap_write(int tck, int tms, int tdi, int trst);
extern void model3_tap_reset(void);
extern READ32_HANDLER(rtc72421_r);
extern WRITE32_HANDLER(rtc72421_w);


/*----------- defined in video/model3.c -----------*/

extern UINT64 *paletteram64;

READ64_HANDLER(model3_char_r);
WRITE64_HANDLER(model3_char_w);
READ64_HANDLER(model3_tile_r);
WRITE64_HANDLER(model3_tile_w);
READ64_HANDLER(model3_vid_reg_r);
WRITE64_HANDLER(model3_vid_reg_w);
READ64_HANDLER(model3_palette_r);
WRITE64_HANDLER(model3_palette_w);

VIDEO_START(model3);
VIDEO_UPDATE(model3);

WRITE64_HANDLER(real3d_cmd_w);
WRITE64_HANDLER(real3d_display_list_w);
WRITE64_HANDLER(real3d_polygon_ram_w);
void real3d_display_list_end(running_machine *machine);
void real3d_display_list1_dma(const address_space *space, UINT32 src, UINT32 dst, int length, int byteswap);
void real3d_display_list2_dma(const address_space *space, UINT32 src, UINT32 dst, int length, int byteswap);
void real3d_vrom_texture_dma(const address_space *space, UINT32 src, UINT32 dst, int length, int byteswap);
void real3d_texture_fifo_dma(const address_space *space, UINT32 src, int length, int byteswap);
void real3d_polygon_ram_dma(const address_space *space, UINT32 src, UINT32 dst, int length, int byteswap);
