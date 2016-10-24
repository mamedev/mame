// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
class pk8000_base_state : public driver_device
{
public:
	pk8000_base_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu") { }

	uint8_t pk8000_video_color_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pk8000_video_color_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t pk8000_text_start_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pk8000_text_start_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t pk8000_chargen_start_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pk8000_chargen_start_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t pk8000_video_start_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pk8000_video_start_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t pk8000_color_start_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pk8000_color_start_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t pk8000_color_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pk8000_color_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t pk8000_84_porta_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pk8000_84_porta_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pk8000_84_portc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void palette_init_pk8000(palette_device &palette);

	uint32_t pk8000_video_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t *videomem);
protected:
	uint8_t m_pk8000_text_start;
	uint8_t m_pk8000_chargen_start;
	uint8_t m_pk8000_video_start;
	uint8_t m_pk8000_color_start;

	uint8_t m_pk8000_video_mode;
	uint8_t m_pk8000_video_color;
	uint8_t m_pk8000_color[32];
	uint8_t m_pk8000_video_enable;
	required_device<cpu_device> m_maincpu;
};
