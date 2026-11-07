// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  Heathkit Terminal Logic Board (TLB)

    Base class shared by all of the TLB variants. This header is private
    to bus/heathzenith/h19, drivers should only include tlb.h and cards.h.

****************************************************************************/

#ifndef MAME_BUS_HEATHZENITH_H19_TLBBASE_H
#define MAME_BUS_HEATHZENITH_H19_TLBBASE_H

#pragma once

#include "tlb.h"

#include "machine/clock.h"
#include "machine/ins8250.h"
#include "machine/mm5740.h"
#include "sound/beep.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"


namespace bus::heathzenith::h19 {

// Clocks
constexpr XTAL MASTER_CLOCK = XTAL(12'288'000);

// Standard H19 used a 2.048 MHz clock for Z80
constexpr XTAL H19_CLOCK    = MASTER_CLOCK / 6;
constexpr XTAL H19_3MHZ     = MASTER_CLOCK / 4;
constexpr XTAL H19_4MHZ     = MASTER_CLOCK / 3;
constexpr XTAL H19_6MHZ     = MASTER_CLOCK / 2;

INPUT_PORTS_EXTERN( tlb );

/**
 * Standard Heath Terminal logic board
 */
class heath_tlb_device : public device_t,
						 public device_heath_tlb_card_interface
{
public:
	heath_tlb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// 8250 UART
	virtual void serial_in_w(int state) override;
	virtual void rlsd_in_w(int state) override;
	virtual void dsr_in_w(int state) override;
	virtual void cts_in_w(int state) override;
	void serial_irq_w(int state);

	// Keyboard
	void reset_key_w(int state);
	void right_shift_w(int state);
	void repeat_key_w(int state);
	void break_key_w(int state);

protected:
	heath_tlb_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock = 0);

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;


	// interrupt related
	virtual void set_irq_line();

	// reset
	void check_for_reset();

	// sound/speaker
	void check_beep_state();
	void key_click_w(u8 data);
	void bell_w(u8 data);
	TIMER_CALLBACK_MEMBER(key_click_off);
	TIMER_CALLBACK_MEMBER(bell_off);

	// keyboard
	u8   kbd_key_r();
	u8   kbd_flags_r();
	u16  translate_mm5740_b(u16 b);
	int  mm5740_shift_r();
	int  mm5740_control_r();
	void mm5740_data_ready_w(int state);

	// serial port
	void serial_out_b(int data);
	void dtr_out(int data);
	void rts_out(int data);

	// crtc
	void crtc_addr_w(offs_t reg, u8 val);
	virtual u8 crtc_reg_r(offs_t reg);
	virtual void crtc_reg_w(offs_t reg, u8 val);
	void crtc_vsync_w(int val);
	virtual MC6845_UPDATE_ROW(crtc_update_row);

	required_device<cpu_device>     m_maincpu;
	required_device<screen_device>  m_screen;
	required_device<palette_device> m_palette;
	required_device<mc6845_device>  m_crtc;
	required_shared_ptr<u8>         m_p_videoram;
	required_region_ptr<u8>         m_p_chargen;
	required_ioport                 m_config;
	required_device<ins8250_device> m_ace;
	required_device<beep_device>    m_beep;
	required_device<mm5740_device>  m_mm5740;
	required_memory_region          m_kbdrom;
	required_ioport                 m_kbspecial;
	required_device<clock_device>   m_repeat_clock;

	emu_timer *m_key_click_timer;
	emu_timer *m_bell_timer;

	u8   m_transchar;
	bool m_strobe;
	bool m_key_click_active;
	bool m_bell_active;
	bool m_reset_pending;
	bool m_right_shift;
	bool m_reset_key;
	bool m_keyboard_irq_raised;
	bool m_serial_irq_raised;
	bool m_break_key_irq_raised;
	bool m_allow_vsync_nmi;
};

} // namespace bus::heathzenith::h19

DECLARE_DEVICE_TYPE(HEATH_TLB, device_heath_tlb_card_interface)

#endif // MAME_BUS_HEATHZENITH_H19_TLBBASE_H
