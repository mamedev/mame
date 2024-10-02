// license: GPL-2.0+
// copyright-holders: Angelo Salese, Jonathan Edwards, Christopher Edwards, Robbbert

#ifndef MAME_HITACHI_BML3_H
#define MAME_HITACHI_BML3_H

#pragma once

#include "bus/bml3/bml3bus.h"
#include "imagedev/cassette.h"
#include "machine/6850acia.h"
#include "sound/spkrdev.h"
#include "sound/ymopn.h"
#include "machine/timer.h"
#include "video/mc6845.h"

#include "emupal.h"


// System clock definitions, from the MB-6890 servce manual, p.48:

#define MASTER_CLOCK ( 32.256_MHz_XTAL )   // Master clock crystal (X1) frequency, 32.256 MHz.  "fx" in the manual.

#define D80_CLOCK ( MASTER_CLOCK / 2 )  // Graphics dot clock in 80-column mode. ~16 MHz.
#define D40_CLOCK ( D80_CLOCK / 2 )     // Graphics dot clock in 40-column mode.  ~8 MHz.

#define CPU_EXT_CLOCK ( D40_CLOCK / 2 ) // IC37, 4.032 MHz signal to EXTAL on the 6809 MPU.
#define CPU_CLOCK     ( CPU_EXT_CLOCK / 4 ) // Actual MPU clock frequency ("fE" in the manual). The division yielding CPU_CLOCK is done in the 6809 itself.  Also used as the 40-column character clock.

#define C80_CLOCK ( CPU_EXT_CLOCK / 2 ) // 80-column character mode.  IC37, "80CHRCK"; ~2 MHz.
#define C40_CLOCK ( CPU_CLOCK )         // 40-column character mode; same as MPU clock.  "40CHRCK";       ~1 MHz.

// Video signal clocks from the HD4650SSP CRTC (IC36)
// In the real hardware, the 80-/40-column Mode switch changes the CRTC character clock input source.  However, it just uses a different divider, so the end result is the same horizontal vsync frequency.
#define H_CLOCK ( C80_CLOCK / 128 ) // in 80-column mode
//#define H_CLOCK ( C40_CLOCK / 64 )    // in 40-column mode (either way the frequency is the same: 15.75 kHz)
#define V_CLOCK ( 2 * H_CLOCK / 525 )   // Vertical refresh rate comes out at exactly 60 Hz.

// TODO: ACIA RS-232C and cassette interface clocks (service manual p.67); perhaps reverse the order and simply divide by 2 from each previous frequency.
// IC111 (74LS157P) takes 16.128 MHz and 1.008 MHz TTL clock inputs.  The RS/C SW input line determines the mode (high -> RS-232C).

// Frequencies for RS-232C mode:
// D80_CLOCK / 840.0    // 19200 Hz
// D80_CLOCK / 420  // 38400 Hz
// D80_CLOCK / 210  // 76800 Hz
// D80_CLOCK / 105.0    // 153600 Hz

// Frequencies for cassette mode:
// / 13440  // 1200
// / 6720   // 2400
// / 3360   // 4800
// / 1680   // 9600


class bml3_state : public driver_device
{
public:
	bml3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_bml3bus(*this, "bml3bus")
		, m_banka(*this, "banka")
		, m_bankc(*this, "bankc")
		, m_banke(*this, "banke")
		, m_bankf(*this, "bankf")
		, m_bankg(*this, "bankg")
		, m_rom_view(*this, "rom")
		, m_palette(*this, "palette")
		, m_p_chargen(*this, "chargen")
		, m_crtc(*this, "crtc")
		, m_cassette(*this, "cassette")
		, m_speaker(*this, "speaker")
		, m_ym2203(*this, "ym2203")
		, m_acia(*this, "acia")
		, m_io_keyboard(*this, "X%u", 0U)
	{ }

	void bml3(machine_config &config);
	DECLARE_INPUT_CHANGED_MEMBER(nmi_button);

protected:
	virtual void main_map(address_map &map) ATTR_COLD;
	virtual void system_io(address_map &map) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<bml3bus_device> m_bml3bus;
	memory_view m_banka;
	memory_view m_bankc;
	memory_view m_banke;
	memory_view m_bankf;
	memory_view m_bankg;
	memory_view m_rom_view;
	required_device<palette_device> m_palette;
	required_region_ptr<u8> m_p_chargen;

	virtual u8 get_attr_mask() { return 0x1f; }
	virtual u8 get_ig_mode(u8 attr) { return 0; }
	virtual u8 get_ig_dots(u8 tile, u8 ra, u8 xi) { return 0; }

	uint8_t vram_r(offs_t offset);
	void vram_w(offs_t offset, u8 data);

	virtual MC6845_UPDATE_ROW(crtc_update_row);

private:
	required_device<mc6845_device> m_crtc;
	required_device<cassette_image_device> m_cassette;
	required_device<speaker_sound_device> m_speaker;
	optional_device<ym2203_device> m_ym2203;
	required_device<acia6850_device> m_acia;
	//memory_view m_bank4;
	required_ioport_array<4> m_io_keyboard;

	uint8_t mc6845_r(offs_t offset);
	void mc6845_w(offs_t offset, u8 data);
	uint8_t kb_sel_r();
	void kb_sel_w(u8 data);
	void mode_sel_w(u8 data);
	void interlace_sel_w(u8 data);
	[[maybe_unused]] uint8_t psg_latch_r();
	[[maybe_unused]] void psg_latch_w(u8 data);
	uint8_t c_reg_sel_r();
	void c_reg_sel_w(u8 data);
	uint8_t music_sel_r();
	void music_sel_w(u8 data);
	void piaA_w(uint8_t data);
	uint8_t kbnmi_r();
	void time_mask_w(u8 data);
	uint8_t timer_r();
	void remote_w(u8 data);
	void acia_rts_w(int state);
	void acia_irq_w(int state);

	INTERRUPT_GEN_MEMBER(timer_firq);
	TIMER_DEVICE_CALLBACK_MEMBER(kansas_r);
	TIMER_DEVICE_CALLBACK_MEMBER(kansas_w);
	TIMER_DEVICE_CALLBACK_MEMBER(keyboard_callback);
	[[maybe_unused]] uint8_t ym2203_r();
	[[maybe_unused]] void ym2203_w(u8 data);

	u8 m_hres_reg = 0U;
	u8 m_crtc_vreg[0x100]{};
	u8 m_psg_latch = 0U;
	u8 m_attr_latch = 0U;
	u8 m_vres_reg = 0U;
	bool m_keyb_interrupt_enabled = 0;
	bool m_keyb_nmi_disabled = 0; // not used yet
	bool m_keyb_counter_operation_disabled = 0;
	u8 m_keyb_empty_scan = 0U;
	u8 m_keyb_scancode = 0U;
	u16 m_kbt = 0U;
	bool m_keyb_capslock_led_on = 0;
	bool m_keyb_hiragana_led_on = 0;
	bool m_keyb_katakana_led_on = 0;
	bool m_cassbit = 0;
	bool m_cassold = 0;
	u8 m_cass_data[4]{};
	void crtc_change_clock();
	u8 m_crtc_index = 0U;
	std::unique_ptr<u8[]> m_vram;
	std::unique_ptr<u8[]> m_aram;
	u8 m_firq_mask = 0U;
	u8 m_firq_status = 0U;
	u8 m_nmi = 0U;
};

class bml3mk2_state : public bml3_state
{
public:
	bml3mk2_state(const machine_config &mconfig, device_type type, const char *tag)
		: bml3_state(mconfig, type, tag)
	{ }

	void bml3mk2(machine_config &config);
};

class bml3mk5_state : public bml3mk2_state
{
public:
	bml3mk5_state(const machine_config &mconfig, device_type type, const char *tag)
		: bml3mk2_state(mconfig, type, tag)
		, m_ig_view(*this, "ig_view")
		, m_gfxdecode(*this, "gfxdecode")
	{ }

	void bml3mk5(machine_config &config);
protected:
	virtual void main_map(address_map &map) override ATTR_COLD;
	virtual void system_io(address_map &map) override ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	virtual u8 get_attr_mask() override { return 0x3f; }
	virtual u8 get_ig_mode(u8 attr) override { return BIT(attr, 5); }
	// NOTE: if IG attribute is enabled then the rest of attribute byte is ignored (no reverse etc.).
	// TODO: if IGMODREG is 1 then the resulting tile will be a white square
	virtual u8 get_ig_dots(u8 tile, u8 ra, u8 xi) override {
		u16 base_offset = tile << 3;
		u8 res = 0;
		for (int i = 0; i < 3; i++)
		{
			if (BIT(m_ig_ram[base_offset + ra + i * 0x800], xi))
				res |= 1 << i;
		}
		return res;
	}

	void ig_ram_w(offs_t offset, u8 data);

	memory_view m_ig_view;

private:
	required_device<gfxdecode_device> m_gfxdecode;
	std::unique_ptr<u8[]> m_ig_ram;

	u8 m_igen = 0;
};

#endif // MAME_HITACHI_BML3_H
