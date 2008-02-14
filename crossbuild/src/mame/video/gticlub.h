void K001005_draw(mame_bitmap *bitmap, const rectangle *cliprect);
void K001005_swap_buffers(void);
void K001005_init(void);
void K001005_preprocess_texture_data(UINT8 *rom, int length, int gticlub);

READ32_HANDLER(K001005_r);
WRITE32_HANDLER(K001005_w);

READ32_HANDLER(K001006_0_r);
WRITE32_HANDLER(K001006_0_w);
READ32_HANDLER(K001006_1_r);
WRITE32_HANDLER(K001006_1_w);
