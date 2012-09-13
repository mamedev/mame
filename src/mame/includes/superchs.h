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
		: driver_device(mconfig, type, tag),
		m_ram(*this,"ram"),
		m_spriteram(*this,"spriteram"),
		m_shared_ram(*this,"shared_ram")
	{ }

	UINT16 m_coin_word;
	required_shared_ptr<UINT32> m_ram;
	required_shared_ptr<UINT32> m_spriteram;
	required_shared_ptr<UINT32> m_shared_ram;

	struct tempsprite *m_spritelist;
	UINT32 m_mem[2];

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
	DECLARE_DRIVER_INIT(superchs);
	virtual void video_start();
};


/*----------- defined in video/superchs.c -----------*/


SCREEN_UPDATE_IND16( superchs );
