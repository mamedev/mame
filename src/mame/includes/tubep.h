class tubep_state : public driver_device
{
public:
	tubep_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 sound_latch;
	UINT8 ls74;
	UINT8 ls377;
	emu_timer *interrupt_timer;
	int curr_scanline;
	UINT8 *rjammer_backgroundram;
	UINT8 *backgroundram;
	UINT8 *textram;
	UINT8 *sprite_colorsharedram;
	UINT8 *spritemap;
	UINT8 prom2[32];
	UINT32 romD_addr;
	UINT32 romEF_addr;
	UINT32 E16_add_b;
	UINT32 HINV;
	UINT32 VINV;
	UINT32 XSize;
	UINT32 YSize;
	UINT32 mark_1;
	UINT32 mark_2;
	UINT32 colorram_addr_hi;
	UINT32 ls273_g6;
	UINT32 ls273_j6;
	UINT32 romHI_addr_mid;
	UINT32 romHI_addr_msb;
	UINT8 DISP;
	UINT8 background_romsel;
	UINT8 color_A4;
	UINT8 ls175_b7;
	UINT8 ls175_e8;
	UINT8 ls377_data;
	UINT32 page;
};


/*----------- defined in video/tubep.c -----------*/

void tubep_vblank_end(running_machine &machine);
PALETTE_INIT( tubep );
SCREEN_UPDATE( tubep );
PALETTE_INIT( rjammer );
SCREEN_UPDATE( rjammer );
VIDEO_START( tubep );
VIDEO_RESET( tubep );


extern WRITE8_HANDLER( tubep_textram_w );
extern WRITE8_HANDLER( rjammer_background_LS377_w );
extern WRITE8_HANDLER( rjammer_background_page_w );

extern WRITE8_HANDLER( tubep_colorproms_A4_line_w );
extern WRITE8_HANDLER( tubep_background_romselect_w );
extern WRITE8_HANDLER( tubep_background_a000_w );
extern WRITE8_HANDLER( tubep_background_c000_w );

extern WRITE8_HANDLER( tubep_sprite_control_w );

