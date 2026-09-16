// license:BSD-3-Clause
// copyright-holders:Nathan Woods,R. Belmont
/*****************************************************************************
 *
 * includes/apple3.h
 *
 * Apple ///
 *
 ****************************************************************************/

#ifndef MAME_APPLE_APPLE3_H
#define MAME_APPLE_APPLE3_H

#pragma once

#include "cpu/m6502/m6502.h"
#include "machine/ram.h"
#include "machine/timer.h"
#include "bus/a2bus/a2bus.h"
#include "machine/mos6551.h"
#include "machine/6522via.h"
#include "machine/kb3600.h"
#include "machine/mm58167.h"
#include "sound/dac.h"
#include "machine/wozfdc.h"
#include "imagedev/floppy.h"
#include "emupal.h"
#include "screen.h"

#define VAR_VM0         0x0001
#define VAR_VM1         0x0002
#define VAR_VM2         0x0004
#define VAR_VM3         0x0008
#define VAR_EXTA0       0x0010
#define VAR_EXTA1       0x0020
#define VAR_EXTPOWER    0x0040
#define VAR_EXTSIDE     0x0080

class apple3_state : public driver_device
{
public:
	apple3_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ram(*this, RAM_TAG),
		m_via(*this, "via6522_%u", 0),
		m_acia(*this, "acia"),
		m_fdc(*this, "fdc"),
		m_ay3600(*this, "ay3600"),
		m_a2bus(*this, "a2bus"),
		m_rtc(*this, "rtc"),
		m_bell(*this, "bell"),
		m_dac(*this, "dac"),
		m_kbspecial(*this, "keyb_special"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen"),
		m_joy1x(*this, "joy_1_x"),
		m_joy1y(*this, "joy_1_y"),
		m_joy2x(*this, "joy_2_x"),
		m_joy2y(*this, "joy_2_y"),
		m_joybuttons(*this, "joy_buttons"),
		m_pdltimer(*this, "pdltimer"),
		floppy0(*this, "0"),
		floppy1(*this, "1"),
		floppy2(*this, "2"),
		floppy3(*this, "3"),
		m_repttimer(*this, "repttimer"),
		m_reset_latch(false),
		m_nmi_latch(false)
	{
	}

	required_device<m6502_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device_array<via6522_device, 2> m_via;
	required_device<mos6551_device> m_acia;
	required_device<appleiii_fdc_device> m_fdc;
	required_device<ay3600_device> m_ay3600;
	required_device<a2bus_device> m_a2bus;
	required_device<mm58167_device> m_rtc;
	required_device<dac_bit_interface> m_bell;
	required_device<dac_byte_interface> m_dac;
	required_ioport m_kbspecial;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	required_ioport m_joy1x, m_joy1y, m_joy2x, m_joy2y, m_joybuttons;
	required_device<timer_device> m_pdltimer;
	required_device<floppy_connector> floppy0;
	required_device<floppy_connector> floppy1;
	required_device<floppy_connector> floppy2;
	required_device<floppy_connector> floppy3;
	required_device<timer_device> m_repttimer;

	uint8_t apple3_memory_r(offs_t offset);
	void apple3_memory_w(offs_t offset, uint8_t data);
	void apple3_sync_w(int state);
	uint8_t apple3_c0xx_r(offs_t offset);
	void apple3_c0xx_w(offs_t offset, uint8_t data);
	void init_apple3();
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(scanstart_cb);
	TIMER_CALLBACK_MEMBER(scanend_cb);
	void apple3_via_0_out_a(uint8_t data);
	void apple3_via_0_out_b(uint8_t data);
	void apple3_via_1_out_a(uint8_t data);
	void apple3_via_1_out_b(uint8_t data);
	void apple3_write_charmem();
	void text40(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void text80(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void graphics_hgr(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void graphics_chgr(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void graphics_shgr(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void graphics_chires(bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint8_t *apple3_bankaddr(uint16_t bank, offs_t offset);
	uint8_t *apple3_get_zpa_addr(offs_t offset);
	void apple3_update_memory();
	void apple3_via_out(uint8_t *var, uint8_t data);
	uint8_t *apple3_get_indexed_addr(offs_t offset);
	TIMER_DEVICE_CALLBACK_MEMBER(apple3_c040_tick);
	void palette_init(palette_device &palette) const;
	int ay3600_shift_r();
	int ay3600_control_r();
	void ay3600_data_ready_w(int state);
	void ay3600_ako_w(int state);
	TIMER_DEVICE_CALLBACK_MEMBER(ay3600_repeat);
	virtual void device_post_load() override;
	TIMER_DEVICE_CALLBACK_MEMBER(paddle_timer);
	void pdl_handler(int offset);
	void a2bus_irq_w(int state);
	void a2bus_nmi_w(int state);
	void vbl_w(int state);
	void a2bus_inh_w(int state);
	DECLARE_INPUT_CHANGED_MEMBER(keyb_special_changed);

	// these need to be public for now
	uint32_t m_flags = 0;
	int m_enable_mask = 0;

	void apple3(machine_config &config);
	void apple3_map(address_map &map) ATTR_COLD;
private:
	bool m_reset_latch;
	bool m_nmi_latch;
	uint8_t m_via_0_a = 0;
	uint8_t m_via_0_b = 0;
	uint8_t m_via_1_a = 0;
	uint8_t m_via_1_b = 0;
	offs_t m_zpa = 0;
	uint8_t m_last_n = 0;
	uint8_t m_char_mem[0x800]{};
	std::unique_ptr<uint32_t[]> m_hgr_map;

	bool m_sync = false;
	bool m_rom_has_been_disabled = false;
	int m_cnxx_slot = 0;
	uint8_t m_indir_bank = 0;

	uint8_t *m_bank2 = nullptr, *m_bank3 = nullptr, *m_bank4 = nullptr, *m_bank5 = nullptr, *m_bank8 = nullptr, *m_bank9 = nullptr;
	uint8_t *m_bank10 = nullptr, *m_bank11 = nullptr;
	uint8_t *m_bank6 = nullptr, *m_bank7rd = nullptr, *m_bank7wr = nullptr;
	int m_bell_state = 0;
	int m_c040_time = 0;
	uint16_t m_lastchar = 0, m_strobe = 0;
	uint8_t m_transchar = 0;
	bool m_anykeydown;
	bool m_charwrt = false;

	emu_timer *m_scanstart = nullptr, *m_scanend = nullptr;

	int m_analog_sel = 0;
	bool m_ramp_active = false;
	int m_pdl_charge = 0;
	int m_va = 0, m_vb = 0, m_vc = 0;
	int m_smoothscr = 0;

	int m_inh_state = 0;
	bool m_flash = false;
};

#endif // MAME_APPLE_APPLE3_H
