// license:BSD-3-Clause
// copyright-holders:Mike Coates, Pierpaolo Prazzoli
/***************************************************************************

    Zaccaria Quasar

****************************************************************************/

#include "includes/cvs.h"

class quasar_state : public cvs_state
{
public:
	quasar_state(const machine_config &mconfig, device_type type, const char *tag)
		: cvs_state(mconfig, type, tag) { }

	std::unique_ptr<uint8_t[]>    m_effectram;
	uint8_t      m_effectcontrol;
	uint8_t      m_page;
	uint8_t      m_io_page;
	void video_page_select_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void io_page_select_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void quasar_video_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t quasar_IO_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void quasar_bullet_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void quasar_sh_command_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t quasar_sh_command_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t audio_t1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void machine_start_quasar();
	void machine_reset_quasar();
	void video_start_quasar();
	void palette_init_quasar(palette_device &palette);
	uint32_t screen_update_quasar(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void quasar_interrupt(device_t &device);
};
