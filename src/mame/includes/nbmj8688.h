class nbmj8688_state : public driver_device
{
public:
	nbmj8688_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	int mjsikaku_scrolly;
	int blitter_destx;
	int blitter_desty;
	int blitter_sizex;
	int blitter_sizey;
	int blitter_direction_x;
	int blitter_direction_y;
	int blitter_src_addr;
	int mjsikaku_gfxrom;
	int mjsikaku_dispflag;
	int mjsikaku_gfxflag2;
	int mjsikaku_gfxflag3;
	int mjsikaku_flipscreen;
	int mjsikaku_screen_refresh;
	int mjsikaku_gfxmode;
	bitmap_t *mjsikaku_tmpbitmap;
	UINT16 *mjsikaku_videoram;
	UINT8 *clut;
	UINT8 *HD61830B_ram[2];
	int HD61830B_instr[2];
	int HD61830B_addr[2];
	int mjsikaku_flipscreen_old;
};


/*----------- defined in video/nbmj8688.c -----------*/

PALETTE_INIT( mbmj8688_8bit );
PALETTE_INIT( mbmj8688_12bit );
PALETTE_INIT( mbmj8688_16bit );
SCREEN_UPDATE( mbmj8688 );
SCREEN_UPDATE( mbmj8688_lcd0 );
SCREEN_UPDATE( mbmj8688_lcd1 );
VIDEO_START( mbmj8688_8bit );
VIDEO_START( mbmj8688_hybrid_12bit );
VIDEO_START( mbmj8688_pure_12bit );
VIDEO_START( mbmj8688_hybrid_16bit );
VIDEO_START( mbmj8688_pure_16bit );
VIDEO_START( mbmj8688_pure_16bit_LCD );

WRITE8_HANDLER( nbmj8688_clut_w );
WRITE8_HANDLER( nbmj8688_blitter_w );
WRITE8_HANDLER( mjsikaku_gfxflag2_w );
WRITE8_HANDLER( mjsikaku_scrolly_w );
WRITE8_HANDLER( mjsikaku_romsel_w );
WRITE8_HANDLER( secolove_romsel_w );
WRITE8_HANDLER( seiha_romsel_w );
WRITE8_HANDLER( crystalg_romsel_w );

WRITE8_HANDLER( nbmj8688_HD61830B_0_instr_w );
WRITE8_HANDLER( nbmj8688_HD61830B_0_data_w );
WRITE8_HANDLER( nbmj8688_HD61830B_1_instr_w );
WRITE8_HANDLER( nbmj8688_HD61830B_1_data_w );
WRITE8_HANDLER( nbmj8688_HD61830B_both_instr_w );
WRITE8_HANDLER( nbmj8688_HD61830B_both_data_w );
