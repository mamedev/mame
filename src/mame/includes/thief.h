// license:BSD-3-Clause
// copyright-holders:Victor Trucco, Mike Balfour, Phil Stroffolino
#include "sound/samples.h"
#include "video/tms9927.h"

struct coprocessor_t {
	std::unique_ptr<uint8_t[]> context_ram;
	uint8_t bank;
	std::unique_ptr<uint8_t[]> image_ram;
	uint8_t param[0x9];
};

class thief_state : public driver_device
{
public:
	thief_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu"),
		m_samples(*this, "samples"),
		m_tms(*this, "tms"),
		m_palette(*this, "palette") { }

	std::unique_ptr<uint8_t[]> m_videoram;
	uint8_t m_input_select;
	uint8_t m_read_mask;
	uint8_t m_write_mask;
	uint8_t m_video_control;
	coprocessor_t m_coprocessor;
	void thief_input_select_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t thief_io_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t thief_context_ram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void thief_context_ram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void thief_context_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void thief_video_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void thief_color_map_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void thief_color_plane_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t thief_videoram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void thief_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void thief_blit_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t thief_coprocessor_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void thief_coprocessor_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tape_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_thief();
	virtual void video_start() override;
	uint32_t screen_update_thief(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void thief_interrupt(device_t &device);
	uint16_t fetch_image_addr( coprocessor_t &thief_coprocessor );
	void tape_set_audio( int track, int bOn );
	void tape_set_motor( int bOn );
	required_device<cpu_device> m_maincpu;
	required_device<samples_device> m_samples;
	required_device<tms9927_device> m_tms;
	required_device<palette_device> m_palette;
};
