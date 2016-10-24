// license:BSD-3-Clause
// copyright-holders:Brad Oliver

#include "machine/gen_latch.h"

class matmania_state : public driver_device
{
public:
	matmania_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_videoram2(*this, "videoram2"),
		m_videoram3(*this, "videoram3"),
		m_colorram(*this, "colorram"),
		m_colorram2(*this, "colorram2"),
		m_colorram3(*this, "colorram3"),
		m_scroll(*this, "scroll"),
		m_pageselect(*this, "pageselect"),
		m_spriteram(*this, "spriteram"),
		m_paletteram(*this, "paletteram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_mcu(*this, "mcu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch")
	{ }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_videoram2;
	required_shared_ptr<uint8_t> m_videoram3;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_colorram2;
	required_shared_ptr<uint8_t> m_colorram3;
	required_shared_ptr<uint8_t> m_scroll;
	required_shared_ptr<uint8_t> m_pageselect;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_paletteram;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<cpu_device> m_mcu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	/* video-related */
	std::unique_ptr<bitmap_ind16> m_tmpbitmap;
	std::unique_ptr<bitmap_ind16> m_tmpbitmap2;

	/* maniach 68705 protection */
	uint8_t m_port_a_in;
	uint8_t m_port_a_out;
	uint8_t m_ddr_a;
	uint8_t m_port_b_in;
	uint8_t m_port_b_out;
	uint8_t m_ddr_b;
	uint8_t m_port_c_in;
	uint8_t m_port_c_out;
	uint8_t m_ddr_c;
	uint8_t m_from_main;
	uint8_t m_from_mcu;
	int m_mcu_sent;
	int m_main_sent;

	uint8_t maniach_68705_port_a_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void maniach_68705_port_a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t maniach_68705_port_b_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void maniach_68705_port_b_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t maniach_68705_port_c_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void maniach_68705_port_c_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void maniach_68705_ddr_a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void maniach_68705_ddr_b_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void maniach_68705_ddr_c_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void maniach_mcu_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t maniach_mcu_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t maniach_mcu_status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	void matmania_sh_command_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void maniach_sh_command_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void matmania_paletteram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	virtual void video_start() override;
	void palette_init_matmania(palette_device &palette);
	void machine_start_maniach();
	void machine_reset_maniach();
	uint32_t screen_update_matmania(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_maniach(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
