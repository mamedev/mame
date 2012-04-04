struct tempsprite
{
	int gfx;
	int code,color;
	int flipx,flipy;
	int x,y;
	int zoomx,zoomy;
	int pri;
};

class groundfx_state : public driver_device
{
public:
	groundfx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT16 m_coin_word;
	UINT16 m_frame_counter;
	UINT16 m_port_sel;
	UINT32 *m_ram;
	struct tempsprite *m_spritelist;
	UINT16 m_rotate_ctrl[8];
	rectangle m_hack_cliprect;
	UINT32 *m_spriteram;
	size_t m_spriteram_size;
	DECLARE_WRITE32_MEMBER(color_ram_w);
	DECLARE_WRITE32_MEMBER(groundfx_input_w);
	DECLARE_READ32_MEMBER(groundfx_adc_r);
	DECLARE_WRITE32_MEMBER(groundfx_adc_w);
	DECLARE_WRITE32_MEMBER(rotate_control_w);
	DECLARE_WRITE32_MEMBER(motor_control_w);
	DECLARE_READ32_MEMBER(irq_speedup_r_groundfx);
};


/*----------- defined in video/groundfx.c -----------*/

VIDEO_START( groundfx );
SCREEN_UPDATE_IND16( groundfx );
