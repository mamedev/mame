/*----------- defined in drivers/changela.c -----------*/

extern UINT8 changela_tree0_col;
extern UINT8 changela_tree1_col;
extern UINT8 changela_left_bank_col;
extern UINT8 changela_right_bank_col;
extern UINT8 changela_boat_shore_col;
extern UINT8 changela_collision_reset;
extern UINT8 changela_tree_collision_reset;


/*----------- defined in video/changela.c -----------*/

VIDEO_START( changela );
VIDEO_UPDATE( changela );

WRITE8_HANDLER( changela_colors_w );
WRITE8_HANDLER( changela_mem_device_select_w );
WRITE8_HANDLER( changela_mem_device_w );
READ8_HANDLER( changela_mem_device_r );
WRITE8_HANDLER( changela_slope_rom_addr_hi_w );
WRITE8_HANDLER( changela_slope_rom_addr_lo_w );
