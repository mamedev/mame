// license:BSD-3-Clause
// copyright-holders:Nathan Woods,Frank Palazzolo
/*****************************************************************************
 *
 * includes/intv.h
 *
 ****************************************************************************/
#ifndef MAME_INCLUDES_INTV_H
#define MAME_INCLUDES_INTV_H

#pragma once

#include "sound/ay8910.h"
#include "video/stic.h"
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
		m_palette(*this, "palette")
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
	enum
	{
		TIMER_INTV_INTERRUPT2_COMPLETE,
		TIMER_INTV_INTERRUPT_COMPLETE,
		TIMER_INTV_BTB_FILL
	};

	required_device<cpu_device> m_maincpu;
	required_device<ay8914_device> m_sound;
	required_device<stic_device> m_stic;
	optional_device<tms9927_device> m_crtc;
	optional_device<intv_cart_slot_device> m_cart;
	optional_shared_ptr<uint16_t> m_intvkbd_dualport_ram;
	optional_shared_ptr<uint8_t> m_videoram;

	DECLARE_READ16_MEMBER(intv_stic_r);
	DECLARE_WRITE16_MEMBER(intv_stic_w);
	DECLARE_READ16_MEMBER(intv_gram_r);
	DECLARE_WRITE16_MEMBER(intv_gram_w);
	DECLARE_READ16_MEMBER(intv_ram8_r);
	DECLARE_WRITE16_MEMBER(intv_ram8_w);
	DECLARE_READ16_MEMBER(intv_ram16_r);
	DECLARE_WRITE16_MEMBER(intv_ram16_w);
	DECLARE_READ8_MEMBER(intvkb_iocart_r);

	DECLARE_READ8_MEMBER(intv_right_control_r);
	DECLARE_READ8_MEMBER(intv_left_control_r);

	uint8_t m_bus_copy_mode;
	uint8_t m_backtab_row;
	uint16_t m_ram16[0x160];
	int m_sr1_int_pending;
	uint8_t m_ram8[256];

	// Keyboard Component
	DECLARE_WRITE16_MEMBER(intvkbd_dualport16_w);
	DECLARE_READ8_MEMBER(intvkbd_dualport8_lsb_r);
	DECLARE_WRITE8_MEMBER(intvkbd_dualport8_lsb_w);
	DECLARE_READ8_MEMBER(intvkbd_dualport8_msb_r);
	DECLARE_WRITE8_MEMBER(intvkbd_dualport8_msb_w);
	DECLARE_READ8_MEMBER(intvkbd_io_r);
	DECLARE_WRITE8_MEMBER(intvkbd_io_w);
	DECLARE_READ8_MEMBER(intvkbd_periph_r);
	DECLARE_WRITE8_MEMBER(intvkbd_periph_w);

	bool m_printer_not_busy;        // printer state
	bool m_printer_no_paper;        // printer state
	bool m_printer_not_busy_enable; // printer interface state

	int m_intvkbd_text_blanked;
	int m_intvkbd_keyboard_col;
	int m_tape_int_pending;
	int m_tape_interrupts_enabled;
	int m_tape_motor_mode;

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void intv_palette(palette_device &palette) const;
	uint32_t screen_update_intv(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_intvkbd(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(intv_interrupt2);
	INTERRUPT_GEN_MEMBER(intv_interrupt);
	TIMER_CALLBACK_MEMBER(intv_interrupt2_complete);
	TIMER_CALLBACK_MEMBER(intv_interrupt_complete);
	TIMER_CALLBACK_MEMBER(intv_btb_fill);

	void intv2_mem(address_map &map);
	void intv_mem(address_map &map);
	void intvecs_mem(address_map &map);
	void intvkbd2_mem(address_map &map);
	void intvkbd_mem(address_map &map);
	void intvoice_mem(address_map &map);

	int m_is_keybd;

	optional_device<cpu_device> m_keyboard;
	optional_device<generic_slot_device> m_iocart1;
	optional_device<generic_slot_device> m_iocart2;
	required_memory_region m_region_maincpu;
	optional_memory_region m_region_keyboard;
	optional_ioport m_io_test;

	optional_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	ioport_port *m_intv_keyboard[10];

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};

#endif // MAME_INCLUDES_INTV_H
