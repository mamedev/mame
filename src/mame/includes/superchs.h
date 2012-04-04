struct tempsprite
{
	int gfx;
	int code,color;
	int flipx,flipy;
	int x,y;
	int zoomx,zoomy;
	int primask;
};

class superchs_state : public driver_device
{
public:
	superchs_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT16 m_coin_word;
	UINT32 *m_ram;
	UINT32 *m_shared_ram;
	int m_steer;
	struct tempsprite *m_spritelist;
	UINT32 m_mem[2];
	UINT32 *m_spriteram;
	size_t m_spriteram_size;
	DECLARE_READ16_MEMBER(shared_ram_r);
	DECLARE_WRITE16_MEMBER(shared_ram_w);
	DECLARE_WRITE32_MEMBER(cpua_ctrl_w);
	DECLARE_WRITE32_MEMBER(superchs_palette_w);
	DECLARE_READ32_MEMBER(superchs_input_r);
	DECLARE_WRITE32_MEMBER(superchs_input_w);
	DECLARE_READ32_MEMBER(superchs_stick_r);
	DECLARE_WRITE32_MEMBER(superchs_stick_w);
	DECLARE_READ32_MEMBER(main_cycle_r);
	DECLARE_READ16_MEMBER(sub_cycle_r);
};


/*----------- defined in video/superchs.c -----------*/

VIDEO_START( superchs );
SCREEN_UPDATE_IND16( superchs );
