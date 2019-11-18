// license:GPL-2.0+
// copyright-holders:Angelo Salese, Jonathan Edwards, Christopher Edwards,Robbbert
/**************************************************************************************

    Basic Master Level 3 (MB-689x) (c) 1980 Hitachi

    Driver by Angelo Salese, Jonathan Edwards and Christopher Edwards

    TODO:
    - implement sound as a bml3bus slot device
    - account for hardware differences between MB-6890, MB-6891 and MB-6892
      (e.g. custom font support on the MB-6892)

**************************************************************************************/

#include "emu.h"

#include "cpu/m6809/m6809.h"
#include "imagedev/cassette.h"
#include "machine/6821pia.h"
#include "machine/6850acia.h"
#include "machine/clock.h"
#include "machine/timer.h"
#include "sound/2203intf.h"
#include "sound/spkrdev.h"
#include "video/mc6845.h"
#include "emupal.h"

#include "bus/bml3/bml3bus.h"
#include "bus/bml3/bml3mp1802.h"
#include "bus/bml3/bml3mp1805.h"
#include "bus/bml3/bml3kanji.h"

#include "screen.h"
#include "speaker.h"

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
// / 13440  //1200
// / 6720   // 2400
// / 3360   // 4800
// / 1680   // 9600


class bml3_state : public driver_device
{
public:
	bml3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_p_videoram(*this, "vram")
		, m_p_chargen(*this, "chargen")
		, m_bml3bus(*this, "bml3bus")
		, m_crtc(*this, "crtc")
		, m_cass(*this, "cassette")
		, m_speaker(*this, "speaker")
		, m_ym2203(*this, "ym2203")
		, m_acia(*this, "acia")
		, m_palette(*this, "palette")
	{
	}

	DECLARE_READ8_MEMBER(mc6845_r);
	DECLARE_WRITE8_MEMBER(mc6845_w);
	DECLARE_READ8_MEMBER(keyboard_r);
	DECLARE_WRITE8_MEMBER(keyboard_w);
	DECLARE_WRITE8_MEMBER(hres_reg_w);
	DECLARE_WRITE8_MEMBER(vres_reg_w);
	DECLARE_READ8_MEMBER(vram_r);
	DECLARE_WRITE8_MEMBER(vram_w);
	DECLARE_READ8_MEMBER(psg_latch_r);
	DECLARE_WRITE8_MEMBER(psg_latch_w);
	DECLARE_READ8_MEMBER(vram_attr_r);
	DECLARE_WRITE8_MEMBER(vram_attr_w);
	DECLARE_READ8_MEMBER(beep_r);
	DECLARE_WRITE8_MEMBER(beep_w);
	DECLARE_WRITE8_MEMBER(piaA_w);
	DECLARE_READ8_MEMBER(keyb_nmi_r);
	DECLARE_WRITE8_MEMBER(firq_mask_w);
	DECLARE_READ8_MEMBER(firq_status_r);
	DECLARE_WRITE8_MEMBER(relay_w);
	DECLARE_WRITE_LINE_MEMBER(acia_rts_w);
	DECLARE_WRITE_LINE_MEMBER(acia_irq_w);

	DECLARE_READ8_MEMBER(a000_r); DECLARE_WRITE8_MEMBER(a000_w);
	DECLARE_READ8_MEMBER(c000_r); DECLARE_WRITE8_MEMBER(c000_w);
	DECLARE_READ8_MEMBER(e000_r); DECLARE_WRITE8_MEMBER(e000_w);
	DECLARE_READ8_MEMBER(f000_r); DECLARE_WRITE8_MEMBER(f000_w);
	DECLARE_READ8_MEMBER(fff0_r); DECLARE_WRITE8_MEMBER(fff0_w);

	MC6845_UPDATE_ROW(crtc_update_row);

	// INTERRUPT_GEN_MEMBER(irq);
	INTERRUPT_GEN_MEMBER(timer_firq);
	TIMER_DEVICE_CALLBACK_MEMBER(kansas_r);
	TIMER_DEVICE_CALLBACK_MEMBER(kansas_w);
	TIMER_DEVICE_CALLBACK_MEMBER(keyboard_callback);
	DECLARE_READ8_MEMBER(ym2203_r);
	DECLARE_WRITE8_MEMBER(ym2203_w);

	void bml3mk2(machine_config &config);
	void bml3mk5(machine_config &config);
	void bml3(machine_config &config);
	void bml3_common(machine_config &config);
	void bml3_mem(address_map &map);
	void bml3mk2_mem(address_map &map);
	void bml3mk5_mem(address_map &map);
private:
	u8 m_hres_reg;
	u8 m_crtc_vreg[0x100];
	u8 m_psg_latch;
	u8 m_attr_latch;
	u8 m_vres_reg;
	bool m_keyb_interrupt_disabled;
	bool m_keyb_nmi_disabled; // not used yet
	bool m_keyb_counter_operation_disabled;
	u8 m_keyb_empty_scan;
	u8 m_keyb_scancode;
	bool m_keyb_capslock_led_on;
	bool m_keyb_hiragana_led_on;
	bool m_keyb_katakana_led_on;
	bool m_cassbit;
	bool m_cassold;
	u8 m_cass_data[4];
	virtual void machine_reset() override;
	virtual void machine_start() override;
	void m6845_change_clock(u8 setting);
	u8 m_crtc_index;
	std::unique_ptr<u8[]> m_extram;
	u8 m_firq_mask;
	u8 m_firq_status;
	required_device<cpu_device> m_maincpu;
	required_region_ptr<u8> m_p_videoram;
	required_region_ptr<u8> m_p_chargen;
	required_device<bml3bus_device> m_bml3bus;
	required_device<mc6845_device> m_crtc;
	required_device<cassette_image_device> m_cass;
	required_device<speaker_sound_device> m_speaker;
	optional_device<ym2203_device> m_ym2203;
	required_device<acia6850_device> m_acia;
	required_device<palette_device> m_palette;
};

READ8_MEMBER( bml3_state::mc6845_r )
{
	if (offset)
		return m_crtc->register_r();
	else
		return m_crtc->status_r();
}

WRITE8_MEMBER( bml3_state::mc6845_w )
{
	if(offset == 0)
	{
		m_crtc_index = data;
		m_crtc->address_w(data);
	}
	else
	{
		m_crtc_vreg[m_crtc_index] = data;
		m_crtc->register_w(data);
	}
}

READ8_MEMBER( bml3_state::keyboard_r )
{
	u8 ret = m_keyb_scancode;
	if (!machine().side_effects_disabled())
		m_keyb_scancode &= 0x7f;
	return ret;
}

WRITE8_MEMBER( bml3_state::keyboard_w )
{
	m_keyb_katakana_led_on = BIT(data, 0);
	m_keyb_hiragana_led_on = BIT(data, 1);
	m_keyb_capslock_led_on = BIT(data, 2);
	m_keyb_counter_operation_disabled = BIT(data, 3);
	m_keyb_interrupt_disabled = !BIT(data, 6);
	m_keyb_nmi_disabled = !BIT(data, 7);
}

void bml3_state::m6845_change_clock(u8 setting)
{
	int m6845_clock = CPU_CLOCK.value();    // CRTC and MPU are synchronous by default

	switch(setting & 0x88)
	{
		case 0x00: m6845_clock = C40_CLOCK.value(); break; //320 x 200
		case 0x08: m6845_clock = C40_CLOCK.value(); break; //320 x 200, interlace
		case 0x80: m6845_clock = C80_CLOCK.value(); break; //640 x 200
		case 0x88: m6845_clock = C80_CLOCK.value(); break; //640 x 200, interlace
	}

	m_crtc->set_unscaled_clock(m6845_clock);
}

WRITE8_MEMBER( bml3_state::hres_reg_w )
{
	// MODE SEL register (see service manual p.43).
	/*
	x--- ---- "W" bit: 0 = 40 columns, 1 = 80 columns
	-x-- ---- "HR" bit: 0 = high resolution, 1 = normal
	--x- ---- "C" bit - ACIA mode: 0 = cassette, 1 = RS-232C
	---- -RGB Background colour
	*/

	m_hres_reg = data;

	m6845_change_clock((m_hres_reg & 0x80) | (m_vres_reg & 0x08));
}

WRITE8_MEMBER( bml3_state::vres_reg_w )
{
	// The MB-6890 had an interlaced video mode which was used for displaying Japanese (Hiragana and Katakana) text (8x16 character glyph bitmaps).
	/*
	---- x--- Interlace select: 0 = non-interlace, 1 = interlace
	*/
	m_vres_reg = data;

	m6845_change_clock((m_hres_reg & 0x80) | (m_vres_reg & 0x08));
}


READ8_MEMBER( bml3_state::vram_r )
{
	// Bit 7 masks reading back to the latch
	if (!BIT(m_attr_latch, 7))
	{
		m_attr_latch = m_p_videoram[offset+0x4000];
	}

	return m_p_videoram[offset];
}

WRITE8_MEMBER( bml3_state::vram_w )
{
	m_p_videoram[offset] = data;
	// color ram is 5-bit
	m_p_videoram[offset+0x4000] = m_attr_latch & 0x1F;
}

READ8_MEMBER( bml3_state::psg_latch_r)
{
	return 0x7f;
}

WRITE8_MEMBER( bml3_state::psg_latch_w)
{
	m_psg_latch = data;
}

READ8_MEMBER(bml3_state::ym2203_r)
{
	u8 dev_offs = ((m_psg_latch & 3) != 3);

	return m_ym2203->read(dev_offs);
}

WRITE8_MEMBER(bml3_state::ym2203_w)
{
	u8 dev_offs = ((m_psg_latch & 3) != 3);

	m_ym2203->write(dev_offs, data);
}

READ8_MEMBER( bml3_state::vram_attr_r)
{
	// C-REG-SELECT register
	// Reads from a VRAM address copy the corresponding 'colour RAM' address to the low-order 5 bits of this register as a side-effect
	// (unless MK bit indicates 'prohibit write')
	return m_attr_latch;
}

WRITE8_MEMBER( bml3_state::vram_attr_w)
{
	// C-REG-SELECT register
	// Writes to a VRAM address copy the low-order 5 bits of this register to the corresponding 'colour RAM' address as a side-effect
	/*
	x--- ---- "MK" bit: 0 = enable write, 1 = prohibit write
	---x ---- "GC" bit: 0 = character, 1 = graphic
	---- x--- "RV" bit: 0 = normal, 1 - reverse
	---- -RGB Foreground colour
	*/
	m_attr_latch = data;
}

READ8_MEMBER( bml3_state::beep_r)
{
	return -1; // BEEP status read?
}

WRITE8_MEMBER( bml3_state::beep_w)
{
	m_speaker->level_w(BIT(data, 7));
}

WRITE8_MEMBER( bml3_state::relay_w )
{
	m_cass->change_state(
		BIT(data,7) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);
}

READ8_MEMBER( bml3_state::a000_r) { return m_extram[offset + 0xa000]; }
WRITE8_MEMBER( bml3_state::a000_w) { m_extram[offset + 0xa000] = data; }
READ8_MEMBER( bml3_state::c000_r) { return m_extram[offset + 0xc000]; }
WRITE8_MEMBER( bml3_state::c000_w) { m_extram[offset + 0xc000] = data; }
READ8_MEMBER( bml3_state::e000_r) { return m_extram[offset + 0xe000]; }
WRITE8_MEMBER( bml3_state::e000_w) { m_extram[offset + 0xe000] = data; }
READ8_MEMBER( bml3_state::f000_r) { return m_extram[offset + 0xf000]; }
WRITE8_MEMBER( bml3_state::f000_w) { m_extram[offset + 0xf000] = data; }
READ8_MEMBER( bml3_state::fff0_r) { return m_extram[offset + 0xfff0]; }
WRITE8_MEMBER( bml3_state::fff0_w) { m_extram[offset + 0xfff0] = data; }

READ8_MEMBER( bml3_state::keyb_nmi_r)
{
	return 0; // bit 7 used to signal a BREAK key pressure
}

WRITE8_MEMBER( bml3_state::firq_mask_w)
{
	m_firq_mask = data & 0x80;
	if(m_firq_mask)
	{
		m_firq_status = 0; // clear pending firq
		m_maincpu->set_input_line(M6809_FIRQ_LINE, CLEAR_LINE);
	}
}

READ8_MEMBER( bml3_state::firq_status_r )
{
	u8 res = m_firq_status << 7;
	m_firq_status = 0;
	m_maincpu->set_input_line(M6809_FIRQ_LINE, CLEAR_LINE);
	return res;
}


void bml3_state::bml3_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x03ff).ram();
	map(0x0400, 0x43ff).rw(FUNC(bml3_state::vram_r), FUNC(bml3_state::vram_w));
	map(0x4400, 0x9fff).ram();
	map(0xff40, 0xff46).noprw(); // lots of unknown reads and writes
	map(0xffc0, 0xffc3).rw("pia", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xffc4, 0xffc5).rw(m_acia, FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0xffc6, 0xffc7).rw(FUNC(bml3_state::mc6845_r), FUNC(bml3_state::mc6845_w));
	// KBNMI - Keyboard "Break" key non-maskable interrupt
	map(0xffc8, 0xffc8).r(FUNC(bml3_state::keyb_nmi_r)); // keyboard nmi
	// DIPSW - DIP switches on system mainboard
	map(0xffc9, 0xffc9).portr("DSW");
	// TIMER - System timer enable
	map(0xffca, 0xffca).r(FUNC(bml3_state::firq_status_r)); // timer irq
	// LPFLG - Light pen interrupt
//  map(0xffcb, 0xffcb)
	// MODE_SEL - Graphics mode select
	map(0xffd0, 0xffd0).w(FUNC(bml3_state::hres_reg_w));
	// TRACE - Trace counter
//  map(0xffd1, 0xffd1)
	// REMOTE - Remote relay control for cassette - bit 7
	map(0xffd2, 0xffd2).w(FUNC(bml3_state::relay_w));
	// MUSIC_SEL - Music select: toggle audio output level when rising
	map(0xffd3, 0xffd3).rw(FUNC(bml3_state::beep_r), FUNC(bml3_state::beep_w));
	// TIME_MASK - Prohibit timer IRQ
	map(0xffd4, 0xffd4).w(FUNC(bml3_state::firq_mask_w));
	// LPENBL - Light pen operation enable
	map(0xffd5, 0xffd5).noprw();
	// INTERLACE_SEL - Interlaced video mode (manual has "INTERACE SEL"!)
	map(0xffd6, 0xffd6).w(FUNC(bml3_state::vres_reg_w));
//  map(0xffd7, 0xffd7) baud select
	// C_REG_SEL - Attribute register (character/video mode and colours)
	map(0xffd8, 0xffd8).rw(FUNC(bml3_state::vram_attr_r), FUNC(bml3_state::vram_attr_w));
	// KB - Keyboard mode register, interrupt control, keyboard LEDs
	map(0xffe0, 0xffe0).rw(FUNC(bml3_state::keyboard_r), FUNC(bml3_state::keyboard_w));
//  map(0xffe8, 0xffe8) bank register
//  map(0xffe9, 0xffe9) IG mode register
//  map(0xffea, 0xffea) IG enable register
	map(0xa000, 0xfeff).rom().region("maincpu", 0xa000);
	map(0xfff0, 0xffff).rom().region("maincpu", 0xfff0);
	map(0xa000, 0xbfff).w(FUNC(bml3_state::a000_w));
	map(0xc000, 0xdfff).w(FUNC(bml3_state::c000_w));
	map(0xe000, 0xefff).w(FUNC(bml3_state::e000_w));
	map(0xf000, 0xfeff).w(FUNC(bml3_state::f000_w));
	map(0xfff0, 0xffff).w(FUNC(bml3_state::fff0_w));

#if 0
	map(0xff00, 0xff00).rw(FUNC(bml3_state::ym2203_r), FUNC(bml3_state::ym2203_w));
	map(0xff02, 0xff02).rw(FUNC(bml3_state::psg_latch_r), FUNC(bml3_state::psg_latch_w)); // PSG address/data select
#endif
}

void bml3_state::bml3mk2_mem(address_map &map)
{
	bml3_mem(map);
	// TODO: anything to add here?

}

void bml3_state::bml3mk5_mem(address_map &map)
{
	bml3_mem(map);
	// TODO: anything to add here?

}

/* Input ports */

static INPUT_PORTS_START( bml3 )

	// DIP switches (service manual p.88)
	// Note the NEWON command reboots with a soft override for the DIP switch
	PORT_START("DSW")
	PORT_DIPNAME(     0x01, 0x01, "BASIC/terminal mode")
		PORT_DIPSETTING(0x00, "Terminal mode")
		PORT_DIPSETTING(0x01, "BASIC mode")
	PORT_DIPNAME(     0x02, 0x02, "Interlaced video")
		PORT_DIPSETTING(0x00, DEF_STR( Off ) )
		PORT_DIPSETTING(0x02, DEF_STR( On ))
	// This is overridden by the 'Mode' toggle button
	PORT_DIPNAME(     0x04, 0x04, "40-/80-column")
		PORT_DIPSETTING(0x00, "40 chars/line")
		PORT_DIPSETTING(0x04, "80 chars/line")
	PORT_DIPNAME(     0x08, 0x00, "Video resolution")
		PORT_DIPSETTING(0x00, "High")
		PORT_DIPSETTING(0x08, "Low")
	PORT_DIPNAME(     0x10, 0x00, "Show PF key content")
		PORT_DIPSETTING(0x00, DEF_STR ( Off ) )
		PORT_DIPSETTING(0x10, DEF_STR ( On ))
	PORT_DIPNAME(     0x20, 0x00, "Terminal duplex")
		PORT_DIPSETTING(0x00, "Full duplex")
		PORT_DIPSETTING(0x20, "Half duplex")
	PORT_DIPNAME(     0x40, 0x00, "Terminal bits")
		PORT_DIPSETTING(0x00, "8 bits/char")
		PORT_DIPSETTING(0x40, "7 bits/char")
	PORT_DIPNAME(     0x80, 0x00, "Hiragana->Katakana")
		PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
		PORT_DIPSETTING(    0x80, DEF_STR( On ) )

// TODO: also model the CS jumper block?

	// RAM expansion configurations (see service manual p.76)
/*
    PORT_START("RAM")
    PORT_DIPNAME( 0x0003, 0x0002, "RAM size" )
    PORT_DIPSETTING(   0x0000, "32 kiB (standard)" )
    PORT_DIPSETTING(   0x0001, "40 kiB (32 kiB + 8 kiB)" )
    PORT_DIPSETTING(   0x0002, "60 kiB (32 kiB + 28 kiB)" )
*/

	PORT_START("key1") //0x00-0x1f
	PORT_BIT(0x00000001,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x00000002,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x00000004,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("? PAD") PORT_CHAR('?')
	PORT_BIT(0x00000008,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x00000010,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x00000020,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x00000040,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Control") PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0x00000080,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Shift")PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT(0x00000100,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("X2")  // ???
	PORT_BIT(0x00000200,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Caps Lock") PORT_CODE(KEYCODE_CAPSLOCK)
	PORT_BIT(0x00000400,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Kana Lock") PORT_CODE(KEYCODE_NUMLOCK)
	PORT_BIT(0x00000800,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Kana Shift")  PORT_CODE(KEYCODE_LALT)
	PORT_BIT(0x00001000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC)
	PORT_BIT(0x00002000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("8 PAD") PORT_CODE(KEYCODE_8_PAD) PORT_CHAR('8')
	PORT_BIT(0x00004000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("9 PAD") PORT_CODE(KEYCODE_9_PAD) PORT_CHAR('9')
	PORT_BIT(0x00008000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("*") PORT_CODE(KEYCODE_ASTERISK) PORT_CHAR('*')
	PORT_BIT(0x00010000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT(0x00020000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT(0x00040000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT(0x00080000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT(0x00100000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x00200000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("^") PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT(0x00400000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("-") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')
	PORT_BIT(0x00800000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT(0x01000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Delete") PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x02000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT(0x04000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT(0x08000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT(0x10000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT(0x20000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("7 PAD") PORT_CODE(KEYCODE_7_PAD) PORT_CHAR('7')
	PORT_BIT(0x40000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Delete PAD") //backspace
	PORT_BIT(0x80000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("\xC2\xA5") PORT_CODE(KEYCODE_TAB) // yen sign

	PORT_START("key2") //0x20-0x3f
	PORT_BIT(0x00000001,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U)
	PORT_BIT(0x00000002,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x00000004,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x00000008,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I)
	PORT_BIT(0x00000010,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P)
	PORT_BIT(0x00000020,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("[") PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x00000040,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("@") PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x00000080,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0 PAD") PORT_CODE(KEYCODE_0_PAD) PORT_CHAR('0')
	PORT_BIT(0x00000100,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x00000200,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T)
	PORT_BIT(0x00000400,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x00000800,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E)
	PORT_BIT(0x00001000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O)
	PORT_BIT(0x00002000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(". PAD")  PORT_CODE(KEYCODE_DEL_PAD) PORT_CHAR('.')
	PORT_BIT(0x00004000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("HOME") PORT_CODE(KEYCODE_HOME) //or cls?
	PORT_BIT(0x00008000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("ENTER") PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x00010000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J)
	PORT_BIT(0x00020000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F)
	PORT_BIT(0x00040000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H)
	PORT_BIT(0x00080000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K)
	PORT_BIT(0x00100000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(";") PORT_CODE(KEYCODE_COLON)
	PORT_BIT(0x00200000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("]") PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT(0x00400000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(":") PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT(0x00800000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("4 PAD") PORT_CODE(KEYCODE_4_PAD) PORT_CHAR('4')
	PORT_BIT(0x01000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x02000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G)
	PORT_BIT(0x04000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x08000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D)
	PORT_BIT(0x10000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L)
	PORT_BIT(0x20000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("5 PAD") PORT_CODE(KEYCODE_5_PAD) PORT_CHAR('5')
	PORT_BIT(0x40000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("6 PAD") PORT_CODE(KEYCODE_6_PAD) PORT_CHAR('6')
	PORT_BIT(0x80000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("- PAD") PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR('-')

	PORT_START("key3") //0x40-0x5f
	PORT_BIT(0x00000001,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M)
	PORT_BIT(0x00000002,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V)
	PORT_BIT(0x00000004,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N)
	PORT_BIT(0x00000008,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(",") PORT_CODE(KEYCODE_COMMA)
	PORT_BIT(0x00000010,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("/") PORT_CODE(KEYCODE_SLASH)
	PORT_BIT(0x00000020,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("/ PAD") PORT_CODE(KEYCODE_SLASH_PAD) PORT_CHAR('/')
	PORT_BIT(0x00000040,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("_") PORT_CODE(KEYCODE_TILDE)
	PORT_BIT(0x00000080,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1 PAD") PORT_CODE(KEYCODE_1_PAD) PORT_CHAR('1')
	PORT_BIT(0x00000100,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x00000200,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B)
	PORT_BIT(0x00000400,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X)
	PORT_BIT(0x00000800,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C)
	PORT_BIT(0x00001000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(".") PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x00002000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2 PAD") PORT_CODE(KEYCODE_2_PAD) PORT_CHAR('2')
	PORT_BIT(0x00004000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3 PAD") PORT_CODE(KEYCODE_3_PAD) PORT_CHAR('3')
	PORT_BIT(0x00008000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("+") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x00010000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF1") PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x00020000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF2") PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x00040000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF3") PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x00080000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF4") PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x00100000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF5") PORT_CODE(KEYCODE_F5)
	PORT_BIT(0xffe00000,IP_ACTIVE_HIGH,IPT_UNKNOWN)
INPUT_PORTS_END

MC6845_UPDATE_ROW( bml3_state::crtc_update_row )
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	// The MB-6890 has a 5-bit colour RAM region.  The meaning of the bits are:
	// 0: blue
	// 1: red
	// 2: green
	// 3: reverse/inverse video
	// 4: graphic (not character)

	u8 x=0,hf=0,xi=0,interlace=0,bgcolor=0,rawbits=0,dots[2],color=0,pen=0;
	bool reverse=0,graphic=0,lowres=0;
	u16 mem=0;

	interlace = (m_crtc_vreg[8] & 3) ? 1 : 0;
	lowres = BIT(m_hres_reg, 6);
	bgcolor = m_hres_reg & 7;

	if (interlace)
	{
		ra >>= 1;
		if (y > 0x191) return;
	}

	// redundant initializers to keep compiler happy
	dots[0] = dots[1] = 0;

	for(x=0; x<x_count; x++)
	{
		if (lowres)
			mem = (ma + x - 0x400) & 0x3fff;
		else
			mem = (ma + x + ra * x_count/40 * 0x400 -0x400) & 0x3fff;

		color = m_p_videoram[mem|0x4000] & 7;
		reverse = BIT(m_p_videoram[mem|0x4000], 3) ^ (x == cursor_x);
		graphic = BIT(m_p_videoram[mem|0x4000], 4);

		rawbits = m_p_videoram[mem];

		if (graphic)
		{
			if (lowres)
			{
				// low-res graphics, each tile has 8 bits arranged as follows:
				// 4 0
				// 5 1
				// 6 2
				// 7 3
				dots[0] = dots[1] = (rawbits >> ra/2 & 0x11) * 0xf;
			}
			else
			{
				dots[0] = dots[1] = rawbits;
			}
		}
		else
		{
			// character mode
			int tile = rawbits & 0x7f;
			int tile_bank = BIT(rawbits, 7);
			if (interlace)
			{
				dots[0] = m_p_chargen[(tile_bank<<11)|(tile<<4)|(ra<<1)];
				dots[1] = m_p_chargen[(tile_bank<<11)|(tile<<4)|(ra<<1)|tile_bank];
			}
			else
			{
				dots[0] = dots[1] = m_p_chargen[(tile<<4)|(ra<<1)|tile_bank];
			}
		}

		for(hf=0;hf<=interlace;hf++)
		{
			for(xi=0;xi<8;xi++)
			{
				if(reverse)
					pen = (dots[hf] >> (7-xi) & 1) ? bgcolor : color;
				else
					pen = (dots[hf] >> (7-xi) & 1) ? color : bgcolor;

				bitmap.pix32(y, x*8+xi) = palette[pen];
				// when the mc6845 device gains full interlace&video support, replace the line above with the line below
				// bitmap.pix32(y*(interlace+1)+hf, x*8+xi) = palette[pen];
			}
		}
	}
}


TIMER_DEVICE_CALLBACK_MEMBER(bml3_state::keyboard_callback)
{
	static const char *const portnames[3] = { "key1","key2","key3" };
	int i,port_i;
	bool trigger = false;

	if(!(m_keyb_scancode & 0x80))
	{
		m_keyb_scancode = (m_keyb_scancode + 1) & 0x7F;
		if (m_keyb_counter_operation_disabled)
		{
			m_keyb_scancode &= 0x7;
		}

		if (m_keyb_scancode == 0x7F)
		{
			if (m_keyb_empty_scan == 1)
			{
				// full scan completed with no keypress
				trigger = true;
			}
			if (m_keyb_empty_scan > 0)
				m_keyb_empty_scan--;
		}
		else if (m_keyb_scancode < 32*3)
		{
			port_i = m_keyb_scancode / 32;
			i = m_keyb_scancode % 32;
			if((ioport(portnames[port_i])->read()>>i) & 1)
			{
				m_keyb_empty_scan = 2;
				trigger = true;
			}
		}
		if (trigger)
		{
			m_keyb_scancode |= 0x80;
			if (!m_keyb_interrupt_disabled)
				m_maincpu->set_input_line(M6809_IRQ_LINE, HOLD_LINE);
		}
		/* Don't need this apparently...
		else
		{
		    m_maincpu->set_input_line(M6809_IRQ_LINE, CLEAR_LINE);
		}
		*/
	}
}

TIMER_DEVICE_CALLBACK_MEMBER( bml3_state::kansas_r )
{
	/* cassette - turn 1200/2400Hz to a bit */
	m_cass_data[1]++;
	u8 cass_ws = (m_cass->input() > +0.03) ? 1 : 0;

	if (cass_ws != m_cass_data[0])
	{
		m_cass_data[0] = cass_ws;
		m_acia->write_rxd((m_cass_data[1] < 12) ? 1 : 0);
		m_cass_data[1] = 0;
	}
}

#if 0
INTERRUPT_GEN_MEMBER(bml3_state::irq)
{
	m_maincpu->set_input_line(M6809_IRQ_LINE, HOLD_LINE);
}
#endif


INTERRUPT_GEN_MEMBER(bml3_state::timer_firq)
{
	if(!m_firq_mask)
	{
		m_maincpu->set_input_line(M6809_FIRQ_LINE, ASSERT_LINE);
		m_firq_status = 1;
	}
}

void bml3_state::machine_start()
{
	m_extram = std::make_unique<u8[]>(0x10000);
	m_psg_latch = 0;
	m_attr_latch = 0;
	m_vres_reg = 0;
	m_keyb_interrupt_disabled = 0;
	m_keyb_nmi_disabled = 0;
	m_keyb_counter_operation_disabled = 0;
	m_keyb_empty_scan = 0;
	m_keyb_scancode = 0;
	m_keyb_capslock_led_on = 0;
	m_keyb_hiragana_led_on = 0;
	m_keyb_katakana_led_on = 0;
	m_crtc_index = 0;
	m_firq_status = 0;
	m_cassbit = 0;
	m_cassold = 0;
}

void bml3_state::machine_reset()
{
	address_space &mem = m_maincpu->space(AS_PROGRAM);

	/* defaults */
	mem.install_rom(0xa000, 0xfeff,memregion("maincpu")->base() + 0xa000);
	mem.install_rom(0xfff0, 0xffff,memregion("maincpu")->base() + 0xfff0);
	mem.install_write_handler(0xa000, 0xbfff, write8_delegate(*this, FUNC(bml3_state::a000_w)), 0);
	mem.install_write_handler(0xc000, 0xdfff, write8_delegate(*this, FUNC(bml3_state::c000_w)), 0);
	mem.install_write_handler(0xe000, 0xefff, write8_delegate(*this, FUNC(bml3_state::e000_w)), 0);
	mem.install_write_handler(0xf000, 0xfeff, write8_delegate(*this, FUNC(bml3_state::f000_w)), 0);
	mem.install_write_handler(0xfff0, 0xffff, write8_delegate(*this, FUNC(bml3_state::fff0_w)), 0);

	m_firq_mask = -1; // disable firq
}

WRITE8_MEMBER(bml3_state::piaA_w)
{
	address_space &mem = m_maincpu->space(AS_PROGRAM);
	/* ROM banking:
	-0-- --0- 0xa000 - 0xbfff ROM R RAM W
	-1-- --0- 0xa000 - 0xbfff RAM R/W
	-x-- --1- 0xa000 - 0xbfff no change
	0--- -0-- 0xc000 - 0xdfff ROM R RAM W
	1--- -0-- 0xc000 - 0xdfff RAM R/W
	x--- -1-- 0xc000 - 0xdfff no change
	0--- 0--- 0xe000 - 0xefff ROM R RAM W
	1--- 0--- 0xe000 - 0xefff RAM R/W
	x--- 1--- 0xe000 - 0xefff no change
	---- ---x 0xf000 - 0xfeff (0) ROM R RAM W (1) RAM R/W
	---- --x- 0xfff0 - 0xffff (0) ROM R RAM W (1) RAM R/W
	*/
	logerror("Check banking PIA A -> %02x\n",data);

	if(!(data & 0x2))
	{
		if(data & 0x40)
		{
			mem.install_readwrite_handler(0xa000, 0xbfff,
					read8_delegate(*this, FUNC(bml3_state::a000_r)),
					write8_delegate(*this, FUNC(bml3_state::a000_w)),
					0);
		}
		else
		{
			mem.install_rom(0xa000, 0xbfff,
					memregion("maincpu")->base() + 0xa000);
			mem.install_write_handler(0xa000, 0xbfff,
					write8_delegate(*this, FUNC(bml3_state::a000_w)),
					0);
		}
	}

	if(!(data & 0x4))
	{
		if(data & 0x40)
		{
			mem.install_readwrite_handler(0xc000, 0xdfff,
					read8_delegate(*this, FUNC(bml3_state::c000_r)),
					write8_delegate(*this, FUNC(bml3_state::c000_w)),
					0);
		}
		else
		{
			mem.install_rom(0xc000, 0xdfff,
					memregion("maincpu")->base() + 0xc000);
			mem.install_write_handler(0xc000, 0xdfff,
					write8_delegate(*this, FUNC(bml3_state::c000_w)),
					0);
		}
	}

	if(!(data & 0x8))
	{
		if(data & 0x80)
		{
			mem.install_readwrite_handler(0xe000, 0xefff,
					read8_delegate(*this, FUNC(bml3_state::e000_r)),
					write8_delegate(*this, FUNC(bml3_state::e000_w)),
					0);
		}
		else
		{
			mem.install_rom(0xe000, 0xefff,
					memregion("maincpu")->base() + 0xe000);
			mem.install_write_handler(0xe000, 0xefff,
					write8_delegate(*this, FUNC(bml3_state::e000_w)),
					0);
		}
	}

	if(data & 1)
	{
		mem.install_readwrite_handler(0xf000, 0xfeff,
				read8_delegate(*this, FUNC(bml3_state::f000_r)),
				write8_delegate(*this, FUNC(bml3_state::f000_w)),
				0);
	}
	else
	{
		mem.install_rom(0xf000, 0xfeff,
				memregion("maincpu")->base() + 0xf000);
		mem.install_write_handler(0xf000, 0xfeff,
				write8_delegate(*this, FUNC(bml3_state::f000_w)),
				0);
	}

	if(data & 2)
	{
		mem.install_readwrite_handler(0xfff0, 0xffff,
				read8_delegate(*this, FUNC(bml3_state::fff0_r)),
				write8_delegate(*this, FUNC(bml3_state::fff0_w)),
				0);
	}
	else
	{
		mem.install_rom(0xfff0, 0xffff,
				memregion("maincpu")->base() + 0xfff0);
		mem.install_write_handler(0xfff0, 0xffff,
				write8_delegate(*this, FUNC(bml3_state::fff0_w)),
				0);
	}
}

WRITE_LINE_MEMBER( bml3_state::acia_rts_w )
{
	logerror("%02x TAPE RTS\n",state);
}

WRITE_LINE_MEMBER( bml3_state::acia_irq_w )
{
	logerror("%02x TAPE IRQ\n",state);
}

TIMER_DEVICE_CALLBACK_MEMBER( bml3_state::kansas_w )
{
	m_cass_data[3]++;

	if (m_cassbit != m_cassold)
	{
		m_cass_data[3] = 0;
		m_cassold = m_cassbit;
	}

	if (m_cassbit)
		m_cass->output(BIT(m_cass_data[3], 0) ? -1.0 : +1.0); // 2400Hz
	else
		m_cass->output(BIT(m_cass_data[3], 1) ? -1.0 : +1.0); // 1200Hz
}

static void bml3_cards(device_slot_interface &device)
{
	device.option_add("bml3mp1802", BML3BUS_MP1802); // MP-1802 Floppy Controller Card
	device.option_add("bml3mp1805", BML3BUS_MP1805); // MP-1805 Floppy Controller Card
	device.option_add("bml3kanji",  BML3BUS_KANJI);
}


void bml3_state::bml3_common(machine_config &config)
{
	/* basic machine hardware */
	MC6809(config, m_maincpu, CPU_EXT_CLOCK);
	m_maincpu->set_vblank_int("screen", FUNC(bml3_state::timer_firq));
//  m_maincpu->set_periodic_int(FUNC(bml3_state::firq), attotime::fromhz(45));

//  MCFG_MACHINE_RESET_OVERRIDE(bml3_state,bml3)

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2400)); /* Service manual specifies "Raster return period" as 2.4 ms (p.64), although the total vertical non-displaying time seems to be 4 ms. */
	screen.set_size(640, 400);
	screen.set_visarea(0, 320-1, 0, 200-1);
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));
	PALETTE(config, m_palette, palette_device::BRG_3BIT);

	/* Devices */
	// CRTC clock should be synchronous with the CPU clock.
	HD6845S(config, m_crtc, CPU_CLOCK); // HD46505SP
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(bml3_state::crtc_update_row));

	// fire once per scan of an individual key
	// According to the service manual (p.65), the keyboard timer is driven by the horizontal video sync clock.
	TIMER(config, "keyboard_timer").configure_periodic(FUNC(bml3_state::keyboard_callback), attotime::from_hz(H_CLOCK/2));
	TIMER(config, "kansas_w").configure_periodic(FUNC(bml3_state::kansas_w), attotime::from_hz(4800));
	TIMER(config, "kansas_r").configure_periodic(FUNC(bml3_state::kansas_r), attotime::from_hz(40000));

	pia6821_device &pia(PIA6821(config, "pia", 0));
	pia.writepa_handler().set(FUNC(bml3_state::piaA_w));

	ACIA6850(config, m_acia, 0);
	m_acia->txd_handler().set([this] (bool state) { m_cassbit = state; });
	m_acia->rts_handler().set(FUNC(bml3_state::acia_rts_w));
	m_acia->irq_handler().set(FUNC(bml3_state::acia_irq_w));

	/* Audio */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, "mono", 0.50);

	clock_device &acia_clock(CLOCK(config, "acia_clock", 9'600)); // 600 baud x 16(divider) = 9600
	acia_clock.signal_handler().set(m_acia, FUNC(acia6850_device::write_txc));
	acia_clock.signal_handler().append(m_acia, FUNC(acia6850_device::write_rxc));

	CASSETTE(config, m_cass);
	m_cass->set_default_state(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_ENABLED);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);

	/* slot devices */
	BML3BUS(config, m_bml3bus, 0);
	m_bml3bus->set_space(m_maincpu, AS_PROGRAM);
	m_bml3bus->nmi_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
	m_bml3bus->irq_callback().set_inputline(m_maincpu, M6809_IRQ_LINE);
	m_bml3bus->firq_callback().set_inputline(m_maincpu, M6809_FIRQ_LINE);
	/* Default to MP-1805 disk (3" or 5.25" SS/SD), as our MB-6892 ROM dump includes the MP-1805 ROM.
	   User may want to switch this to MP-1802 (5.25" DS/DD).
	   Note it isn't feasible to use both, as they each place boot ROM at F800.
	 */
	BML3BUS_SLOT(config, "sl1", m_bml3bus, bml3_cards, "bml3mp1805");
	BML3BUS_SLOT(config, "sl2", m_bml3bus, bml3_cards, nullptr);
	BML3BUS_SLOT(config, "sl3", m_bml3bus, bml3_cards, nullptr);
	BML3BUS_SLOT(config, "sl4", m_bml3bus, bml3_cards, nullptr);
	BML3BUS_SLOT(config, "sl5", m_bml3bus, bml3_cards, nullptr);
	BML3BUS_SLOT(config, "sl6", m_bml3bus, bml3_cards, "bml3kanji");
}

void bml3_state::bml3(machine_config &config)
{
	bml3_common(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &bml3_state::bml3_mem);

#if 0
	// TODO: slot device for sound card
	// audio
	YM2203(config, m_ym2203, 2000000); //unknown clock / divider
	m_ym2203->set_flags(AY8910_LEGACY_OUTPUT);
	m_ym2203->add_route(0, "mono", 0.25);
	m_ym2203->add_route(1, "mono", 0.25);
	m_ym2203->add_route(2, "mono", 0.50);
	m_ym2203->add_route(3, "mono", 0.50);
#endif
}

void bml3_state::bml3mk2(machine_config &config)
{
	bml3_common(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &bml3_state::bml3mk2_mem);

	// TODO: anything to add here?
}

void bml3_state::bml3mk5(machine_config &config)
{
	bml3_common(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &bml3_state::bml3mk5_mem);

	// TODO: anything to add here?
}



/* ROM definition */
ROM_START( bml3 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
//  ROM_LOAD( "l3bas.rom", 0xa000, 0x6000, BAD_DUMP CRC(d81baa07) SHA1(a8fd6b29d8c505b756dbf5354341c48f9ac1d24d)) //original, 24k isn't a proper rom size!
	// TODO: replace with MB-6890 ROMs (these are from an MB-6892)
	ROM_LOAD( "598 p16611.ic3", 0xa000, 0x2000, BAD_DUMP CRC(954b9bad) SHA1(047948fac6808717c60a1d0ac9205a5725362430))
	ROM_LOAD( "599 p16561.ic4", 0xc000, 0x2000, BAD_DUMP CRC(b27a48f5) SHA1(94cb616df4caa6415c5076f9acdf675acb7453e2))
	// TODO: Replace checksums with a ROM dump without a disk controller patch (checksums here are inclusive of the MP1805 patch)
	ROM_LOAD( "600 p16681.ic5", 0xe000, 0x2000, BAD_DUMP CRC(fe3988a5) SHA1(edc732f1cd421e0cf45ffcfc71c5589958ceaae7))

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD("font.rom", 0x00000, 0x1000, BAD_DUMP CRC(0b6f2f10) SHA1(dc411b447ca414e94843636d8b5f910c954581fb) ) // handcrafted

	ROM_REGION( 0x8000, "vram", ROMREGION_ERASEFF )
ROM_END

ROM_START( bml3mk2 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
//  ROM_LOAD( "l3bas.rom", 0xa000, 0x6000, BAD_DUMP CRC(d81baa07) SHA1(a8fd6b29d8c505b756dbf5354341c48f9ac1d24d)) //original, 24k isn't a proper rom size!
	// TODO: replace with MB-6891 ROMs (these are from an MB-6892)
	ROM_LOAD( "598 p16611.ic3", 0xa000, 0x2000, BAD_DUMP CRC(954b9bad) SHA1(047948fac6808717c60a1d0ac9205a5725362430))
	ROM_LOAD( "599 p16561.ic4", 0xc000, 0x2000, BAD_DUMP CRC(b27a48f5) SHA1(94cb616df4caa6415c5076f9acdf675acb7453e2))
	ROM_LOAD( "600 p16681.ic5", 0xe000, 0x2000, BAD_DUMP CRC(fe3988a5) SHA1(edc732f1cd421e0cf45ffcfc71c5589958ceaae7))

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD("font.rom", 0x00000, 0x1000, BAD_DUMP CRC(0b6f2f10) SHA1(dc411b447ca414e94843636d8b5f910c954581fb) ) // handcrafted

	ROM_REGION( 0x8000, "vram", ROMREGION_ERASEFF )
ROM_END

ROM_START( bml3mk5 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
//  ROM_LOAD( "l3bas.rom", 0xa000, 0x6000, BAD_DUMP CRC(d81baa07) SHA1(a8fd6b29d8c505b756dbf5354341c48f9ac1d24d)) //original, 24k isn't a proper rom size!
	/* Handcrafted ROMs, rom labels and contents might not match */
	ROM_LOAD( "598 p16611.ic3", 0xa000, 0x2000, BAD_DUMP CRC(954b9bad) SHA1(047948fac6808717c60a1d0ac9205a5725362430))
	ROM_LOAD( "599 p16561.ic4", 0xc000, 0x2000, BAD_DUMP CRC(b27a48f5) SHA1(94cb616df4caa6415c5076f9acdf675acb7453e2))
	// TODO: Replace checksums with a ROM dump without a disk controller patch (checksums here are inclusive of the MP1805 patch)
	ROM_LOAD( "600 p16681.ic5", 0xe000, 0x2000, BAD_DUMP CRC(fe3988a5) SHA1(edc732f1cd421e0cf45ffcfc71c5589958ceaae7))

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD("font.rom", 0x00000, 0x1000, BAD_DUMP CRC(0b6f2f10) SHA1(dc411b447ca414e94843636d8b5f910c954581fb) ) // handcrafted

	ROM_REGION( 0x8000, "vram", ROMREGION_ERASEFF )
ROM_END

/* Driver */

/*    YEAR  NAME     PARENT COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY    FULLNAME                               FLAGS */
COMP( 1980, bml3,    0,     0,      bml3,    bml3,  bml3_state, empty_init, "Hitachi", "MB-6890 Basic Master Level 3",        MACHINE_NOT_WORKING)
COMP( 1982, bml3mk2, bml3,  0,      bml3mk2, bml3,  bml3_state, empty_init, "Hitachi", "MB-6891 Basic Master Level 3 Mark 2", MACHINE_NOT_WORKING)
COMP( 1983, bml3mk5, bml3,  0,      bml3mk5, bml3,  bml3_state, empty_init, "Hitachi", "MB-6892 Basic Master Level 3 Mark 5", MACHINE_NOT_WORKING)
