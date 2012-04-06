class tankbatt_state : public driver_device
{
public:
	tankbatt_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_videoram;
	int m_nmi_enable;
	int m_sound_enable;
	UINT8 *m_bulletsram;
	size_t m_bulletsram_size;
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
};


/*----------- defined in video/tankbatt.c -----------*/


PALETTE_INIT( tankbatt );
VIDEO_START( tankbatt );
SCREEN_UPDATE_IND16( tankbatt );
