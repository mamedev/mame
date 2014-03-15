void K001005_draw(bitmap_rgb32 &bitmap, const rectangle &cliprect);
void K001005_swap_buffers(running_machine &machine);
void K001005_init(running_machine &machine);
void K001005_preprocess_texture_data(UINT8 *rom, int length, int gticlub);

DECLARE_READ32_HANDLER(K001005_r);
DECLARE_WRITE32_HANDLER(K001005_w);

void K001006_init(running_machine &machine, palette_device *palette);
DECLARE_READ32_HANDLER(K001006_0_r);
DECLARE_WRITE32_HANDLER(K001006_0_w);
DECLARE_READ32_HANDLER(K001006_1_r);
DECLARE_WRITE32_HANDLER(K001006_1_w);
