// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
#include "sound/sn76477.h"

class route16_state : public driver_device
{
public:
	route16_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_sn(*this, "snsnd"),
		m_sharedram(*this, "sharedram"),
		m_videoram1(*this, "videoram1"),
		m_videoram2(*this, "videoram2"),
		m_palette(*this, "palette") {}

	optional_device<sn76477_device> m_sn;

	required_shared_ptr<uint8_t> m_sharedram;
	required_shared_ptr<uint8_t> m_videoram1;
	required_shared_ptr<uint8_t> m_videoram2;
	required_device<palette_device> m_palette;

	uint8_t m_ttmahjng_port_select;
	int m_speakres_vrx;
	uint8_t m_flipscreen;
	uint8_t m_palette_1;
	uint8_t m_palette_2;

	void out0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void out1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void route16_sharedram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t routex_prot_read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ttmahjng_input_port_matrix_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t ttmahjng_input_port_matrix_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t speakres_in3_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void speakres_out2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void stratvox_sn76477_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void init_route16();
	void init_route16a();
	void init_route16c();
	void machine_start_speakres();
	void machine_start_ttmahjng();
	virtual void video_start() override;

	uint32_t screen_update_route16(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_ttmahjng(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};
