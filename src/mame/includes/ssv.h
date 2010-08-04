class ssv_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, ssv_state(machine)); }

	ssv_state(running_machine &machine)
		: driver_data_t(machine) { }

	UINT16 *scroll;
	UINT16 *paletteram;
	UINT16 *spriteram;
	UINT16 *spriteram2;

	int tile_code[16];

	int enable_video;
	int sprites_offsx;
	int sprites_offsy;
	int tilemap_offsx;
	int tilemap_offsy;
	int shadow_pen_mask;
	int shadow_pen_shift;

	UINT8 requested_int;
	UINT16 *irq_vectors;
	UINT16 irq_enable;
	UINT16 *mainram;

	UINT16 *nvram;
	size_t nvram_size;
	UINT16 *dsp_ram;

	UINT16 *eaglshot_gfxram;
	UINT16 *gdfs_tmapram;
	UINT16 *gdfs_tmapscroll;

	tilemap_t *gdfs_tmap;

	int interrupt_ultrax;

	UINT16 *input_sel;
	int gdfs_gfxram_bank;
	int gdfs_lightgun_select;
	UINT16 *gdfs_blitram;

	UINT16 sxyreact_serial;
	int sxyreact_dial;
	UINT16 gdfs_eeprom_old;

	UINT32 latches[8];

	UINT8 trackball_select;
	UINT8 gfxrom_select;
};

/*----------- defined in video/ssv.c -----------*/

READ16_HANDLER( ssv_vblank_r );
WRITE16_HANDLER( ssv_scroll_w );
WRITE16_HANDLER( paletteram16_xrgb_swap_word_w );
WRITE16_HANDLER( gdfs_tmapram_w );
void ssv_enable_video(running_machine *machine, int enable);

VIDEO_START( ssv );
VIDEO_START( eaglshot );
VIDEO_START( gdfs );

VIDEO_UPDATE( ssv );
VIDEO_UPDATE( eaglshot );
VIDEO_UPDATE( gdfs );
