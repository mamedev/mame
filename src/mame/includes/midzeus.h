// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Driver for Midway Zeus games

**************************************************************************/

#define MIDZEUS_VIDEO_CLOCK     XTAL_66_6667MHz

#include "machine/timekpr.h"

class midzeus_state : public driver_device
{
public:
	midzeus_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_nvram(*this, "nvram"),
			m_ram_base(*this, "ram_base"),
			m_linkram(*this, "linkram"),
			m_tms32031_control(*this, "tms32031_ctl"),
			m_zeusbase(*this, "zeusbase") ,
		m_m48t35(*this, "m48t35"),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	required_shared_ptr<uint32_t> m_nvram;
	required_shared_ptr<uint32_t> m_ram_base;
	optional_shared_ptr<uint32_t> m_linkram;
	required_shared_ptr<uint32_t> m_tms32031_control;
	optional_shared_ptr<uint32_t> m_zeusbase;
	optional_device<timekeeper_device> m_m48t35;
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	optional_device<palette_device> m_palette;

	void cmos_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t cmos_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void cmos_protect_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t zpram_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void zpram_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t bitlatches_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void bitlatches_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t crusnexo_leds_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void crusnexo_leds_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t linkram_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void linkram_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t tms32031_control_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void tms32031_control_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void keypad_select_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t analog_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void analog_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void invasn_gun_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t invasn_gun_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	int PIC16C5X_T0_clk_r();
	uint32_t zeus_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void zeus_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	ioport_value custom_49way_r(ioport_field &field, void *param);
	ioport_value keypad_r(ioport_field &field, void *param);
	uint32_t zeus2_timekeeper_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void zeus2_timekeeper_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void init_invasn();
	void init_mk4();
	void init_thegrid();
	void init_crusnexo();
	void machine_start_midzeus();
	void machine_reset_midzeus();
	void video_start_midzeus();
	uint32_t screen_update_midzeus(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void display_irq(device_t &device);
	void display_irq_off(void *ptr, int32_t param);
	void invasn_gun_callback(void *ptr, int32_t param);
private:
	void exit_handler();
	void zeus_pointer_w(uint32_t which, uint32_t data, bool logit);
	void zeus_register16_w(offs_t offset, uint16_t data, bool logit);
	void zeus_register32_w(offs_t offset, uint32_t data, bool logit);
	void zeus_register_update(offs_t offset);
	int zeus_fifo_process(const uint32_t *data, int numwords);
	void zeus_draw_model(uint32_t texdata, bool logit);

	void log_fifo_command(const uint32_t *data, int numwords, const char *suffix);
	void log_waveram(uint32_t length_and_base);
	void update_gun_irq();
};
