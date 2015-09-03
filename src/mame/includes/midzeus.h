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

	required_shared_ptr<UINT32> m_nvram;
	required_shared_ptr<UINT32> m_ram_base;
	optional_shared_ptr<UINT32> m_linkram;
	required_shared_ptr<UINT32> m_tms32031_control;
	required_shared_ptr<UINT32> m_zeusbase;
	optional_device<timekeeper_device> m_m48t35;
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	optional_device<palette_device> m_palette;

	DECLARE_WRITE32_MEMBER(cmos_w);
	DECLARE_READ32_MEMBER(cmos_r);
	DECLARE_WRITE32_MEMBER(cmos_protect_w);
	DECLARE_READ32_MEMBER(zpram_r);
	DECLARE_WRITE32_MEMBER(zpram_w);
	DECLARE_READ32_MEMBER(bitlatches_r);
	DECLARE_WRITE32_MEMBER(bitlatches_w);
	DECLARE_READ32_MEMBER(crusnexo_leds_r);
	DECLARE_WRITE32_MEMBER(crusnexo_leds_w);
	DECLARE_READ32_MEMBER(linkram_r);
	DECLARE_WRITE32_MEMBER(linkram_w);
	DECLARE_READ32_MEMBER(tms32031_control_r);
	DECLARE_WRITE32_MEMBER(tms32031_control_w);
	DECLARE_WRITE32_MEMBER(keypad_select_w);
	DECLARE_READ32_MEMBER(analog_r);
	DECLARE_WRITE32_MEMBER(analog_w);
	DECLARE_WRITE32_MEMBER(invasn_gun_w);
	DECLARE_READ32_MEMBER(invasn_gun_r);
	DECLARE_READ_LINE_MEMBER(PIC16C5X_T0_clk_r);
	DECLARE_READ32_MEMBER(zeus_r);
	DECLARE_WRITE32_MEMBER(zeus_w);
	DECLARE_CUSTOM_INPUT_MEMBER(custom_49way_r);
	DECLARE_CUSTOM_INPUT_MEMBER(keypad_r);
	DECLARE_READ32_MEMBER(zeus2_timekeeper_r);
	DECLARE_WRITE32_MEMBER(zeus2_timekeeper_w);
	DECLARE_DRIVER_INIT(invasn);
	DECLARE_DRIVER_INIT(mk4);
	DECLARE_DRIVER_INIT(thegrid);
	DECLARE_DRIVER_INIT(crusnexo);
	DECLARE_MACHINE_START(midzeus);
	DECLARE_MACHINE_RESET(midzeus);
	DECLARE_VIDEO_START(midzeus);
	UINT32 screen_update_midzeus(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(display_irq);
	TIMER_CALLBACK_MEMBER(display_irq_off);
	TIMER_CALLBACK_MEMBER(invasn_gun_callback);
private:
	void exit_handler();
	void zeus_pointer_w(UINT32 which, UINT32 data, int logit);
	void zeus_register16_w(offs_t offset, UINT16 data, int logit);
	void zeus_register32_w(offs_t offset, UINT32 data, int logit);
	void zeus_register_update(offs_t offset);
	int zeus_fifo_process(const UINT32 *data, int numwords);
	void zeus_draw_model(UINT32 texdata, int logit);

	void log_fifo_command(const UINT32 *data, int numwords, const char *suffix);
	void log_waveram(UINT32 length_and_base);
	void update_gun_irq();
};


class midzeus2_state : public midzeus_state
{
public:
	midzeus2_state(const machine_config &mconfig, device_type type, const char *tag)
				: midzeus_state(mconfig, type, tag) { }

	DECLARE_VIDEO_START(midzeus2);
	UINT32 screen_update_midzeus2(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECLARE_READ32_MEMBER( zeus2_r );
	DECLARE_WRITE32_MEMBER( zeus2_w );
private:
	TIMER_CALLBACK_MEMBER(int_timer_callback);
	void exit_handler2();
	void zeus2_register32_w(offs_t offset, UINT32 data, int logit);
	void zeus2_register_update(offs_t offset, UINT32 oldval, int logit);
	int zeus2_fifo_process(const UINT32 *data, int numwords);
	void zeus2_pointer_write(UINT8 which, UINT32 value);
	void zeus2_draw_model(UINT32 baseaddr, UINT16 count, int logit);
	void log_fifo_command(const UINT32 *data, int numwords, const char *suffix);
};
