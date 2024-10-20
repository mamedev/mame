// license:BSD-3-Clause
// copyright-holders:Nathan Woods,Frank Palazzolo
/*****************************************************************************
 *
 * includes/intv.h
 *
 ****************************************************************************/
#ifndef MAME_MATTEL_INTV_H
#define MAME_MATTEL_INTV_H

#pragma once

#include "sound/ay8910.h"
#include "stic.h"
#include "video/tms9927.h"

#include "bus/intv/ecs.h"
#include "bus/intv/slot.h"
#include "bus/intv/voice.h"
//#include "bus/intv/keycomp.h"

#include "bus/intv_ctrl/ctrl.h"
#include "bus/intv_ctrl/handctrl.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include "emupal.h"


class intv_state : public driver_device
{
public:
	intv_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_sound(*this, "ay8914"),
		m_stic(*this, "stic"),
		m_crtc(*this, "crtc"),
		m_cart(*this, "cartslot"),
		m_intvkbd_dualport_ram(*this, "dualport_ram"),
		m_videoram(*this, "videoram"),
		m_keyboard(*this, "keyboard"),
		m_iocart1(*this, "ioslot1"),
		m_iocart2(*this, "ioslot2"),
		m_region_maincpu(*this, "maincpu"),
		m_region_keyboard(*this, "keyboard"),
		m_io_test(*this, "TEST"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_intv_keyboard(*this, "ROW%X", 0U)
	{ }

	void intvkbd(machine_config &config);
	void intv2(machine_config &config);
	void intvoice(machine_config &config);
	void intvecs(machine_config &config);
	void intv(machine_config &config);

	void init_intvecs();
	void init_intvkbd();
	void init_intv();

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	emu_timer *m_int_complete_timer;
	emu_timer *m_int2_complete_timer;
	emu_timer *m_btb_fill_timers[stic_device::BACKTAB_HEIGHT];

	required_device<cpu_device> m_maincpu;
	required_device<ay8914_device> m_sound;
	required_device<stic_device> m_stic;
	optional_device<tms9927_device> m_crtc;
	optional_device<intv_cart_slot_device> m_cart;
	optional_shared_ptr<uint16_t> m_intvkbd_dualport_ram;
	optional_shared_ptr<uint8_t> m_videoram;

	uint16_t stic_r(offs_t offset);
	void stic_w(offs_t offset, uint16_t data);
	uint16_t gram_r(offs_t offset);
	void gram_w(offs_t offset, uint16_t data);
	uint16_t ram8_r(offs_t offset);
	void ram8_w(offs_t offset, uint16_t data);
	uint16_t ram16_r(offs_t offset);
	void ram16_w(offs_t offset, uint16_t data);
	uint8_t intvkb_iocart_r(offs_t offset);

	uint8_t m_bus_copy_mode = 0;
	uint8_t m_backtab_row = 0;
	uint16_t m_ram16[0x160]{};
	int m_sr1_int_pending = 0;
	uint8_t m_ram8[256]{};
	bool m_maincpu_reset = false;

	// Keyboard Component
	void intvkbd_dualport16_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t intvkbd_dualport8_lsb_r(offs_t offset);
	void intvkbd_dualport8_lsb_w(offs_t offset, uint8_t data);
	uint8_t intvkbd_dualport8_msb_r(offs_t offset);
	void intvkbd_dualport8_msb_w(offs_t offset, uint8_t data);
	uint8_t intvkbd_io_r(offs_t offset);
	void intvkbd_io_w(offs_t offset, uint8_t data);
	uint8_t intvkbd_periph_r(offs_t offset);
	void intvkbd_periph_w(offs_t offset, uint8_t data);

	uint16_t iab_r();

	bool m_printer_not_busy = false;        // printer state
	bool m_printer_no_paper = false;        // printer state
	bool m_printer_not_busy_enable = false; // printer interface state

	int m_intvkbd_text_blanked = 0;
	int m_intvkbd_keyboard_col = 0;
	int m_tape_int_pending = 0;
	int m_tape_interrupts_enabled = 0;
	int m_tape_motor_mode = 0;

	void intv_palette(palette_device &palette) const;
	uint32_t screen_update_intv(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_intvkbd(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(interrupt2);
	INTERRUPT_GEN_MEMBER(interrupt);
	TIMER_CALLBACK_MEMBER(interrupt2_complete);
	TIMER_CALLBACK_MEMBER(interrupt_complete);
	TIMER_CALLBACK_MEMBER(btb_fill);

	void intv2_mem(address_map &map) ATTR_COLD;
	void intv_mem(address_map &map) ATTR_COLD;
	void intvecs_mem(address_map &map) ATTR_COLD;
	void intvkbd2_mem(address_map &map) ATTR_COLD;
	void intvkbd_mem(address_map &map) ATTR_COLD;
	void intvoice_mem(address_map &map) ATTR_COLD;

	int m_is_keybd = 0;

	optional_device<cpu_device> m_keyboard;
	optional_device<generic_slot_device> m_iocart1;
	optional_device<generic_slot_device> m_iocart2;
	required_memory_region m_region_maincpu;
	optional_memory_region m_region_keyboard;
	optional_ioport m_io_test;

	optional_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	optional_ioport_array<10> m_intv_keyboard;
};

#endif // MAME_MATTEL_INTV_H
