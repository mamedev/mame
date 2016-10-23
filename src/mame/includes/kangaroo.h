// license:BSD-3-Clause
// copyright-holders:Ville Laitinen, Aaron Giles
/***************************************************************************

    Sun Electronics Kangaroo hardware

    driver by Ville Laitinen

***************************************************************************/

class kangaroo_state : public driver_device
{
public:
	kangaroo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_video_control(*this, "video_control"),
		m_maincpu(*this, "maincpu"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_video_control;

	/* video-related */
	std::unique_ptr<uint32_t[]>      m_videoram;

	/* misc */
	uint8_t        m_mcu_clock;
	uint8_t mcu_sim_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mcu_sim_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void kangaroo_coin_counter_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void kangaroo_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void kangaroo_video_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void machine_start_kangaroo_mcu();
	uint32_t screen_update_kangaroo(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void videoram_write( uint16_t offset, uint8_t data, uint8_t mask );
	void blitter_execute(  );
	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
};
