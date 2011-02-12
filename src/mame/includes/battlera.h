class battlera_state : public driver_device
{
public:
	battlera_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	int control_port_select;
	int msm5205next;
	int toggle;
	int HuC6270_registers[20];
	int VDC_register;
	int vram_ptr;
	UINT8 *HuC6270_vram;
	UINT8 *vram_dirty;
	bitmap_t *tile_bitmap;
	bitmap_t *front_bitmap;
	UINT32 tile_dirtyseq;
	int current_scanline;
	int inc_value;
	int irq_enable;
	int rcr_enable;
	int sb_enable;
	int bb_enable;
	int bldwolf_vblank;
	UINT8 blank_tile[32];
};


/*----------- defined in video/battlera.c -----------*/

VIDEO_UPDATE( battlera );
VIDEO_START( battlera );
INTERRUPT_GEN( battlera_interrupt );

READ8_HANDLER( HuC6270_register_r );
WRITE8_HANDLER( HuC6270_register_w );
//READ8_HANDLER( HuC6270_data_r );
WRITE8_HANDLER( HuC6270_data_w );
WRITE8_HANDLER( battlera_palette_w );

READ8_HANDLER( HuC6270_debug_r );
WRITE8_HANDLER( HuC6270_debug_w );
