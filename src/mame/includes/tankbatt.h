class tankbatt_state : public driver_device
{
public:
	tankbatt_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_bulletsram(*this, "bulletsram"),
		m_videoram(*this, "videoram"){ }

	required_shared_ptr<UINT8> m_bulletsram;
	required_shared_ptr<UINT8> m_videoram;
	int m_nmi_enable;
	int m_sound_enable;

	tilemap_t *m_bg_tilemap;
	DECLARE_WRITE8_MEMBER(tankbatt_led_w);
	DECLARE_READ8_MEMBER(tankbatt_in0_r);
	DECLARE_READ8_MEMBER(tankbatt_in1_r);
	DECLARE_READ8_MEMBER(tankbatt_dsw_r);
	DECLARE_WRITE8_MEMBER(tankbatt_interrupt_enable_w);
	DECLARE_WRITE8_MEMBER(tankbatt_demo_interrupt_enable_w);
	DECLARE_WRITE8_MEMBER(tankbatt_sh_expl_w);
	DECLARE_WRITE8_MEMBER(tankbatt_sh_engine_w);
	DECLARE_WRITE8_MEMBER(tankbatt_sh_fire_w);
	DECLARE_WRITE8_MEMBER(tankbatt_irq_ack_w);
	DECLARE_WRITE8_MEMBER(tankbatt_coin_counter_w);
	DECLARE_WRITE8_MEMBER(tankbatt_coin_lockout_w);
	DECLARE_WRITE8_MEMBER(tankbatt_videoram_w);
	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void video_start();
	virtual void palette_init();
	UINT32 screen_update_tankbatt(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
