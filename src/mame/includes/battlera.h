class battlera_state : public driver_device
{
public:
	battlera_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
		{ }

	int m_control_port_select;
	int m_msm5205next;
	int m_toggle;
	int m_HuC6270_registers[20];
	int m_VDC_register;
	int m_vram_ptr;
	UINT8 *m_HuC6270_vram;
	UINT8 *m_vram_dirty;
	bitmap_t *m_tile_bitmap;
	bitmap_t *m_front_bitmap;
	UINT32 m_tile_dirtyseq;
	int m_current_scanline;
	int m_inc_value;
	int m_irq_enable;
	int m_rcr_enable;
	int m_sb_enable;
	int m_bb_enable;
	int m_bldwolf_vblank;
	UINT8 m_blank_tile[32];

	required_device<cpu_device> m_maincpu;
};


/*----------- defined in video/battlera.c -----------*/

SCREEN_UPDATE( battlera );
VIDEO_START( battlera );
TIMER_DEVICE_CALLBACK( battlera_irq );

READ8_HANDLER( HuC6270_register_r );
WRITE8_HANDLER( HuC6270_register_w );
//READ8_HANDLER( HuC6270_data_r );
WRITE8_HANDLER( HuC6270_data_w );
WRITE8_HANDLER( battlera_palette_w );

READ8_HANDLER( HuC6270_debug_r );
WRITE8_HANDLER( HuC6270_debug_w );
