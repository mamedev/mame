class xyonix_state : public driver_device
{
public:
	xyonix_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_vidram(*this, "vidram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode") { }

	required_shared_ptr<UINT8> m_vidram;
	tilemap_t *m_tilemap;

	int m_e0_data;
	int m_credits;
	int m_coins;
	int m_prev_coin;
	DECLARE_WRITE8_MEMBER(xyonix_irqack_w);
	DECLARE_READ8_MEMBER(xyonix_io_r);
	DECLARE_WRITE8_MEMBER(xyonix_io_w);
	DECLARE_WRITE8_MEMBER(xyonix_vidram_w);
	TILE_GET_INFO_MEMBER(get_xyonix_tile_info);
	virtual void video_start();
	DECLARE_PALETTE_INIT(xyonix);
	UINT32 screen_update_xyonix(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void handle_coins(int coin);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
};
