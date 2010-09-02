/*----------- defined in video/darkseal.c -----------*/

extern UINT16 *darkseal_pf12_row, *darkseal_pf34_row;
extern UINT16 *darkseal_pf1_data,*darkseal_pf2_data,*darkseal_pf3_data;

VIDEO_START( darkseal );
VIDEO_UPDATE( darkseal );

WRITE16_HANDLER( darkseal_pf1_data_w );
WRITE16_HANDLER( darkseal_pf2_data_w );
WRITE16_HANDLER( darkseal_pf3_data_w );
WRITE16_HANDLER( darkseal_pf3b_data_w );
WRITE16_HANDLER( darkseal_control_0_w );
WRITE16_HANDLER( darkseal_control_1_w );
WRITE16_HANDLER( darkseal_palette_24bit_rg_w );
WRITE16_HANDLER( darkseal_palette_24bit_b_w );
