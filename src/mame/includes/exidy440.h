// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Exidy 440 hardware

*************************************************************************/

#include "audio/exidy440.h"

#define EXIDY440_MASTER_CLOCK       (XTAL_12_9792MHz)


class exidy440_state : public driver_device
{
public:
	exidy440_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_imageram(*this, "imageram"),
		m_spriteram(*this, "spriteram"),
		m_scanline(*this, "scanline"),
		m_maincpu(*this, "maincpu"),
		m_custom(*this, "custom"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	required_shared_ptr<uint8_t> m_imageram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_scanline;

	required_device<cpu_device> m_maincpu;
	required_device<exidy440_sound_device> m_custom;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	uint8_t m_bank;
	const uint8_t *m_showdown_bank_data[2];
	int8_t m_showdown_bank_select;
	uint8_t m_showdown_bank_offset;
	uint8_t m_firq_vblank;
	uint8_t m_firq_beam;
	uint8_t m_topsecex_yscroll;
	uint8_t m_latched_x;
	std::unique_ptr<uint8_t[]> m_local_videoram;
	std::unique_ptr<uint8_t[]> m_local_paletteram;
	uint8_t m_firq_enable;
	uint8_t m_firq_select;
	uint8_t m_palettebank_io;
	uint8_t m_palettebank_vis;
	void bankram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t exidy440_input_port_3_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t sound_command_ack_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void sound_command_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void exidy440_input_port_3_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void exidy440_coin_counter_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t showdown_bank0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t claypign_protection_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t topsecex_input_port_5_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void topsecex_yscroll_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t exidy440_videoram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void exidy440_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t exidy440_paletteram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void exidy440_paletteram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t exidy440_horizontal_pos_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t exidy440_vertical_pos_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void exidy440_spriteram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void exidy440_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void exidy440_interrupt_clear_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	ioport_value firq_beam_r(ioport_field &field, void *param);
	ioport_value firq_vblank_r(ioport_field &field, void *param);
	ioport_value hitnmiss_button1_r(ioport_field &field, void *param);
	void coin_inserted(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);
	void init_showdown();
	void init_topsecex();
	void init_yukon();
	void init_exidy440();
	void init_claypign();
	virtual void machine_start() override;
	virtual void machine_reset() override;
	void video_start_exidy440();
	void video_start_topsecex();
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int scroll_offset, int check_collision);
	void update_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect,  int scroll_offset, int check_collision);
	uint32_t screen_update_exidy440(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_topsecex(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void exidy440_vblank_interrupt(device_t &device);
	void delayed_sound_command_w(void *ptr, int32_t param);
	void beam_firq_callback(void *ptr, int32_t param);
	void collide_firq_callback(void *ptr, int32_t param);
	void exidy440_update_firq();
	void exidy440_bank_select(uint8_t bank);
};

/*----------- defined in video/exidy440.c -----------*/

MACHINE_CONFIG_EXTERN( exidy440_video );
MACHINE_CONFIG_EXTERN( topsecex_video );
