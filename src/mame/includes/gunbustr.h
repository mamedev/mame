struct tempsprite
{
	int gfx;
	int code,color;
	int flipx,flipy;
	int x,y;
	int zoomx,zoomy;
	int primask;
};

class gunbustr_state : public driver_device
{
public:
	gunbustr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT16 m_coin_word;
	UINT32 *m_ram;
	struct tempsprite *m_spritelist;
	UINT32 m_mem[2];
	UINT32 *m_spriteram;
	size_t m_spriteram_size;
	DECLARE_WRITE32_MEMBER(gunbustr_palette_w);
	DECLARE_WRITE32_MEMBER(gunbustr_input_w);
	DECLARE_WRITE32_MEMBER(motor_control_w);
	DECLARE_READ32_MEMBER(gunbustr_gun_r);
	DECLARE_WRITE32_MEMBER(gunbustr_gun_w);
	DECLARE_READ32_MEMBER(main_cycle_r);
};


/*----------- defined in video/gunbustr.c -----------*/

VIDEO_START( gunbustr );
SCREEN_UPDATE_IND16( gunbustr );
