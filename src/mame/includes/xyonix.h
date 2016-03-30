// license:BSD-3-Clause
// copyright-holders:David Haywood
class xyonix_state : public driver_device
{
public:
	xyonix_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_vidram(*this, "vidram") { }

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;

	required_shared_ptr<UINT8> m_vidram;

	tilemap_t *m_tilemap;

	int m_e0_data;
	int m_credits;
	int m_coins;
	int m_prev_coin;

	DECLARE_WRITE8_MEMBER(irqack_w);
	DECLARE_READ8_MEMBER(io_r);
	DECLARE_WRITE8_MEMBER(io_w);
	DECLARE_WRITE8_MEMBER(vidram_w);

	virtual void machine_start() override;
	virtual void video_start() override;
	TILE_GET_INFO_MEMBER(get_tile_info);
	DECLARE_PALETTE_INIT(xyonix);
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void handle_coins(int coin);
};
