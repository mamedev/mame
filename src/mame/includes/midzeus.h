// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Driver for Midway Zeus games

**************************************************************************/

#define MIDZEUS_VIDEO_CLOCK     XTAL(66'666'700)

#include "machine/timekpr.h"
#include "screen.h"

class midzeus_state : public driver_device
{
public:
	midzeus_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_nvram(*this, "nvram"),
		m_ram_base(*this, "ram_base"),
		m_firewire(*this, "firewire"),
		m_tms32031_control(*this, "tms32031_ctl"),
		m_zeusbase(*this, "zeusbase") ,
		m_m48t35(*this, "m48t35"),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	required_shared_ptr<uint32_t> m_nvram;
	required_shared_ptr<uint32_t> m_ram_base;
	optional_shared_ptr<uint32_t> m_firewire;
	required_shared_ptr<uint32_t> m_tms32031_control;
	optional_shared_ptr<uint32_t> m_zeusbase;
	optional_device<timekeeper_device> m_m48t35;
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	optional_device<palette_device> m_palette;

	emu_timer *m_display_irq_off_timer;

	DECLARE_WRITE32_MEMBER(cmos_w);
	DECLARE_READ32_MEMBER(cmos_r);
	DECLARE_WRITE32_MEMBER(cmos_protect_w);
	DECLARE_READ32_MEMBER(zpram_r);
	DECLARE_WRITE32_MEMBER(zpram_w);
	DECLARE_READ32_MEMBER(disk_asic_r);
	DECLARE_WRITE32_MEMBER(disk_asic_w);
	DECLARE_READ32_MEMBER(disk_asic_jr_r);
	DECLARE_WRITE32_MEMBER(disk_asic_jr_w);
	DECLARE_READ32_MEMBER(crusnexo_leds_r);
	DECLARE_WRITE32_MEMBER(crusnexo_leds_w);
	DECLARE_READ32_MEMBER(firewire_r);
	DECLARE_WRITE32_MEMBER(firewire_w);
	DECLARE_READ32_MEMBER(tms32031_control_r);
	DECLARE_WRITE32_MEMBER(tms32031_control_w);
	DECLARE_WRITE32_MEMBER(keypad_select_w);
	DECLARE_READ32_MEMBER(analog_r);
	DECLARE_WRITE32_MEMBER(analog_w);
	DECLARE_WRITE32_MEMBER(invasn_gun_w);
	DECLARE_READ32_MEMBER(invasn_gun_r);
	DECLARE_READ32_MEMBER(zeus_r);
	DECLARE_WRITE32_MEMBER(zeus_w);
	DECLARE_CUSTOM_INPUT_MEMBER(custom_49way_r);
	DECLARE_CUSTOM_INPUT_MEMBER(keypad_r);
	DECLARE_READ32_MEMBER(grid_keypad_r);
	DECLARE_READ32_MEMBER(trackball_r);
	DECLARE_DRIVER_INIT(invasn);
	DECLARE_DRIVER_INIT(mk4);
	DECLARE_DRIVER_INIT(thegrid);
	DECLARE_DRIVER_INIT(crusnexo);
	DECLARE_MACHINE_START(midzeus);
	DECLARE_MACHINE_RESET(midzeus);
	DECLARE_VIDEO_START(midzeus);
	uint32_t screen_update_midzeus(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(display_irq);
	TIMER_CALLBACK_MEMBER(display_irq_off);
	TIMER_CALLBACK_MEMBER(invasn_gun_callback);
	void midzeus(machine_config &config);
	void invasn(machine_config &config);
	void mk4(machine_config &config);
	void zeus_map(address_map &map);
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
