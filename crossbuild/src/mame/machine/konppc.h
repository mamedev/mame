#ifndef _KONPPC_H
#define _KONPPC_H

#define CGBOARD_TYPE_ZR107		0
#define CGBOARD_TYPE_GTICLUB	1
#define CGBOARD_TYPE_NWKTR		2
#define CGBOARD_TYPE_HORNET		3
#define CGBOARD_TYPE_HANGPLT	4

void init_konami_cgboard(int board_id, int type);
void set_cgboard_id(int board_id);
int get_cgboard_id(void);
void set_cgboard_texture_bank(int board, int bank, UINT8 *rom);

READ32_HANDLER( cgboard_dsp_comm_r_ppc );
WRITE32_HANDLER( cgboard_dsp_comm_w_ppc );
READ32_HANDLER( cgboard_dsp_shared_r_ppc );
WRITE32_HANDLER( cgboard_dsp_shared_w_ppc );

READ32_HANDLER( cgboard_0_comm_sharc_r );
WRITE32_HANDLER( cgboard_0_comm_sharc_w );
READ32_HANDLER( cgboard_0_shared_sharc_r );
WRITE32_HANDLER( cgboard_0_shared_sharc_w );
READ32_HANDLER( cgboard_1_comm_sharc_r );
WRITE32_HANDLER( cgboard_1_comm_sharc_w );
READ32_HANDLER( cgboard_1_shared_sharc_r );
WRITE32_HANDLER( cgboard_1_shared_sharc_w );

void K033906_init(void);
READ32_HANDLER(K033906_0_r);
WRITE32_HANDLER(K033906_0_w);
READ32_HANDLER(K033906_1_r);
WRITE32_HANDLER(K033906_1_w);

WRITE32_HANDLER(nwk_fifo_0_w);
WRITE32_HANDLER(nwk_fifo_1_w);
READ32_HANDLER(nwk_voodoo_0_r);
READ32_HANDLER(nwk_voodoo_1_r);
WRITE32_HANDLER(nwk_voodoo_0_w);
WRITE32_HANDLER(nwk_voodoo_1_w);

void draw_7segment_led(mame_bitmap *bitmap, int x, int y, UINT8 value);

#endif
