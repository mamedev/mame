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
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_ram(*this,"ram"),
		m_spriteram(*this,"spriteram")
	{
		m_coin_lockout = true;
	}

	required_device<cpu_device> m_maincpu;
	required_shared_ptr<UINT32> m_ram;
	required_shared_ptr<UINT32> m_spriteram;

	bool m_coin_lockout;
	UINT16 m_coin_word;
	struct tempsprite *m_spritelist;
	UINT32 m_mem[2];

	DECLARE_WRITE32_MEMBER(gunbustr_palette_w);
	DECLARE_WRITE32_MEMBER(gunbustr_input_w);
	DECLARE_WRITE32_MEMBER(motor_control_w);
	DECLARE_READ32_MEMBER(gunbustr_gun_r);
	DECLARE_WRITE32_MEMBER(gunbustr_gun_w);
	DECLARE_READ32_MEMBER(main_cycle_r);
	DECLARE_CUSTOM_INPUT_MEMBER(coin_word_r);
	DECLARE_DRIVER_INIT(gunbustrj);
	DECLARE_DRIVER_INIT(gunbustr);
	virtual void video_start();
	UINT32 screen_update_gunbustr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(gunbustr_interrupt);
};
