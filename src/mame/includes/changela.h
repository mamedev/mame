

class changela_state : public driver_device
{
public:
	changela_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *  videoram;
	UINT8 *  colorram;
	UINT8 *  spriteram;

	/* video-related */
	bitmap_t *obj0_bitmap, *river_bitmap, *tree0_bitmap, *tree1_bitmap;
	UINT8*   tree_ram;
	UINT8*   memory_devices;
	UINT32   mem_dev_selected;	/* an offset within memory_devices area */
	UINT32   slopeROM_bank;
	UINT8    tree_en;
	UINT8    horizon;
	UINT8    v_count_river;
	UINT8    v_count_tree;
	int      tree_on[2];
	emu_timer* scanline_timer;

	/* misc */
	UINT8    tree0_col;
	UINT8    tree1_col;
	UINT8    left_bank_col;
	UINT8    right_bank_col;
	UINT8    boat_shore_col;
	UINT8    collision_reset;
	UINT8    tree_collision_reset;
	UINT8    prev_value_31;
	int      dir_31;

	/* mcu-related */
	UINT8    port_a_in, port_a_out, ddr_a;
	UINT8    port_b_out, ddr_b;
	UINT8    port_c_in, port_c_out, ddr_c;

	UINT8    mcu_out;
	UINT8    mcu_in;
	UINT8    mcu_pc_1;
	UINT8    mcu_pc_0;

	/* devices */
	running_device *mcu;
};

/*----------- defined in video/changela.c -----------*/

VIDEO_START( changela );
VIDEO_UPDATE( changela );

WRITE8_HANDLER( changela_colors_w );
WRITE8_HANDLER( changela_mem_device_select_w );
WRITE8_HANDLER( changela_mem_device_w );
READ8_HANDLER( changela_mem_device_r );
WRITE8_HANDLER( changela_slope_rom_addr_hi_w );
WRITE8_HANDLER( changela_slope_rom_addr_lo_w );
