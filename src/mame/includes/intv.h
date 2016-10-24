// license:BSD-3-Clause
// copyright-holders:Nathan Woods,Frank Palazzolo
/*****************************************************************************
 *
 * includes/intv.h
 *
 ****************************************************************************/

#ifndef INTV_H_
#define INTV_H_

#include "sound/ay8910.h"
#include "video/stic.h"
#include "bus/intv/slot.h"
#include "bus/intv/voice.h"
#include "bus/intv/ecs.h"
//#include "bus/intv/keycomp.h"

#include "bus/intv_ctrl/ctrl.h"
#include "bus/intv_ctrl/handctrl.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"


class intv_state : public driver_device
{
public:
	enum
	{
		TIMER_INTV_INTERRUPT2_COMPLETE,
		TIMER_INTV_INTERRUPT_COMPLETE,
		TIMER_INTV_BTB_FILL
	};

	intv_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_sound(*this, "ay8914"),
		m_stic(*this, "stic"),
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

	required_device<cpu_device> m_maincpu;
	required_device<ay8914_device> m_sound;
	required_device<stic_device> m_stic;
	optional_device<intv_cart_slot_device> m_cart;
	optional_shared_ptr<uint16_t> m_intvkbd_dualport_ram;
	optional_shared_ptr<uint8_t> m_videoram;

	uint16_t intv_stic_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void intv_stic_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t intv_gram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void intv_gram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t intv_ram8_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void intv_ram8_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t intv_ram16_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void intv_ram16_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint8_t intvkb_iocart_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	uint8_t intv_right_control_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t intv_left_control_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	uint8_t m_bus_copy_mode;
	uint8_t m_backtab_row;
	uint16_t m_ram16[0x160];
	int m_sr1_int_pending;
	uint8_t m_ram8[256];

	// Keyboard Component
	uint8_t intvkbd_tms9927_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void intvkbd_tms9927_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void intvkbd_dualport16_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint8_t intvkbd_dualport8_lsb_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void intvkbd_dualport8_lsb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t intvkbd_dualport8_msb_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void intvkbd_dualport8_msb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	uint8_t m_tms9927_num_rows;
	uint8_t m_tms9927_cursor_col;
	uint8_t m_tms9927_cursor_row;
	uint8_t m_tms9927_last_row;

	int m_intvkbd_text_blanked;
	int m_intvkbd_keyboard_col;
	int m_tape_int_pending;
	int m_tape_interrupts_enabled;
	int m_tape_unknown_write[6];
	int m_tape_motor_mode;
	void init_intvecs();
	void init_intvkbd();
	void init_intv();
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void palette_init_intv(palette_device &palette);
	uint32_t screen_update_intv(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_intvkbd(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void intv_interrupt2(device_t &device);
	void intv_interrupt(device_t &device);
	void intv_interrupt2_complete(void *ptr, int32_t param);
	void intv_interrupt_complete(void *ptr, int32_t param);
	void intv_btb_fill(void *ptr, int32_t param);

protected:
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

#endif /* INTV_H_ */
