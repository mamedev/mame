// license:BSD-3-Clause
// copyright-holders:AJR, Pietro Gagliardi
/*******************************************************************************

Yamaha DX27 and DX100 digital synthesizers

The DX27 and DX100 are mid-tier professional synthesizers released by Yamaha
around 1985. The DX27 is a full-size keyboard with 61 full-size keys that can
only run on AC power. The DX100 is a smaller, wearable keyboard with only 49
small-size keys and can run on either AC power or batteries. Both keyboards have
full MIDI in/out/thru, and can also hook up to a Yamaha foot pedal and breath
controller.

Apart from the differences listed above and a few less significant differences,
these two keyboards are largely identical. In fact, they use the exact same
mainboard, with both DX27 and DX100 printed on the silkscreen; a marker was used
to cross out the irrelevant model number during assembly. The front panel and
keyboard decoding circuits differ, as do the LCD/LED screen assemblies.

These two appear to be based on the DX21, a slightly higher-end keyboard released
by Yamaha earlier in 1985 that features a hardware chorus effect (implemented
independently of the FM synthesis chip) and some additional voice parameters (which
appear to be implemented in software). There is also the DX27S, which appears to
be based on the DX27 but adds a few extra features. Neither of these two models
are covered here as of this writing.

The main CPU is a Hitachi HD6303; its I/O circuitry is used extensively.
Some of the controls and inputs are analog; a M58990 ADC chip is used for these.

The FM synthesis chip is the YM2164 "OPP", a proprietary variant of the YM2151
that changes the rate of Timer B, moves the test/LFO reset register, and adds
an additional set of 8 registers used by the breath controller code. This chip
was also used in the DX21 (and possibly also the DX27S), but is perhaps most
famously used by the SFG-05 expansion module for the MSX and the FB-01 standalone
desktop MIDI synthesizer. As these two don't support the breath controller, the
extra registers were totally unused (set to 0 by the firmware), and as a result
sound and behave entirely identically to the YM2151. (Figuring out what these
registers did was part of the impetus for me (Pietro Gagliardi) actually building
this emulation.)

In addition to the independent instruction manuals for the DX27 and DX100, there is
also a single service manual that covers both keyboards (but not the DX27S, curiously
enough). The DX21 service manual comes with an "Overall Circuit Diagram" for that
keyboard; it is likely that the DX27/DX100 service manual also did, but PDFs available
online are missing it. A scan of the DX27/DX100 Overall Circuit Diagram is available
online separately; it does not appear to have spread as far and wide as the rest of the
service manual, but is still readily available.

*** Currently imperfectly emulated

    - The cassette interface.
      This uses an 8-pin DIN with what appears to be the same pinout as the MSX.
      However, the remote lines are completely unused, and the tape player has
      to be manually operated. The "dxconvert" project by rarepixel does claim to
      support DX100 tape dumps, and does so by using the "castools" project by
      joyrex2001, which includes a program to convert wav files storing the raw
      tape audio to MSX .cas files. dxconvert uses the command-line parameters
      "-p -e 4 -t 10"; I'm not sure how to interpret this, even after reading
      the code.

      The constant used below for testing against m_cassette->input() was guessed
      through trial and error until I could get a successful load at least somewhat
      consistently. It still isn't perfect, but I don't know how to refine this
      further.

      "dxconvert": https://github.com/rarepixel/dxconvert
      "castools": https://github.com/joyrex2001/castools
      The sample voices that I used to test can be found at
      http://robertgomez.org/blog/2017/08/02/some-yamaha-dx100-dx27-synth-patches/
      I loaded the wav download here, saved it to another wav,
      verified that wav (the DX100 will ask to verify data you just
      saved), and then converted both to .cas as described and compared
      the two; they are identical.

*** Currently unemulated

    - Bit 5 of port 6 is tied to the /G2A and /G2B pins of the two
      TC40H138P chips (~~ 74138?) that sit between the panel switches and the CPU.
      I don't yet understand how these lines are actually used, but I can still use
      the full functions of the keyboard if I just have it return 0 on read, so.

*** CPU ports

    Port 2 is arranged as so:
    Bit(s)    Connection
    0         Controls the state of the power LED on the DX100.
              The firmware will blink this LED when battery power is low.
              This is unused on the DX27.
    1         0 (pulled to chassis ground by 220 ohm resistor)
    2         500khz clock
    3         MIDI In data bit
    4         MIDI Out data bit
    5         0 if the foot pedal is connected, 1 otherwise.
              The manual says to use either the Yamaha FC-4 or Yamaha FC-5 foot pedals
              with the DX100.
    6         Pulled TODO if the foot pedal is pressed.
    7         0 (pulled to chassis ground by 220 ohm resistor)

    Port 5 is connected to the panel switches and keyboard keys.
    Which sets of buttons are exposed is determined by the low four bits
    of port 6. The exact matrix differs between the DX27 and DX100.

    Port 6 is arranged as so:
    Bit(s)    Connection
    0-3       Which set of buttons to expose on port 5.
    4         Connected to the EOC line of the ADC.
    5         (see /G2A and /G2B comment in Currently unemulated)
    6         Connected to the REC (TS) line of the cassette interface.
    7         Connected to the PLAY (TL) line of the cassette interface.

*** M58990 ports

    Port 0 is connected to the pitch wheel.

    Port 1 is connected to the mod wheel.

    Port 2 is connected to the breath controller.
    The manual says to use the Yamaha BC-1 breath controller with the DX100.

    Port 3 is connected to the data entry slider.

    (TODO port 4 appears to be connected to chassis ground by 220 ohm resistor?)

    Port 5 is a voltmeter for the RAM battery.

    Port 6 is a voltmeter for system power.

    (TODO port 7 appears to be tied to port 6?)

*** Default keyboard layout

    For quick testing, the default keyboard button layout will be:
    1 2 3 4 5 6 7 8 9 0 - = -> 1 2 3 4 5 6 7 8 9 10 11 12
    Q W  T Y  O P -> PBend KeyShift  Store Func      BankA BankB
    E R  U I  [ ] -> -1    +1        Edit  Internal  BankC BankD
    Octave 3 will be available over [Z S X D C  V G B H N J M]
    Pitch bend will be ' /
    Mod wheel will be ; .
    Data entry slider will be L ,

    This leaves the breath controller, switches 13-24, and the other
    keyboard octaves unmapped by default.

    When first starting with a fresh configuration and no NVRAM,
    press Y and then O (Func and then BankA) to switch to the
    internal voices. Note that some voices may not respond to
    the pitch or mod wheels as much as other voices do.

    To disable memory protection, press Y, ], and then E (Func,
    12 to select the Memory Protection setting, and -1 to turn
    it off). Then you can press U (Edit) to edit voice
    parameters.

    IPT_KEYBOARD is used, so use the UI mode key (default
    Scroll Lock) to re-enable the MAME UI.

    No, I don't know why some voices (such as Pianobells)
    seem to not let you turn off modulation even if you set
    the mod wheel to its lowest point. You can set the pitch
    sensitivity value ("PMS", Edit/Compare key 9) to 0 to
    force it off. This might have to do with the YM2164
    moving the LFO Reset register; if not, it might have to
    do with the default pitch wheel position not being 0
    (see TODO below).

*** Test mode

    To enter test mode, hold 1 and 2 on the panel while powering the system
    on. You'll see the version number, and then a prompt asking if you want
    to enter test mode; press +1 to enter test mode.

    If 1 and 3 or 1 and 4 are held instead of 1 and 2, different subsets of the
    test mode will run instead. Furthermore, some tests will only be run on
    the DX100.

    For more details on the individual tests, refer to the service manual.

    To make developing this driver easier I wrote an init function for the
    DX100 1.1 driver which allows me to pick and choose which tests to actually
    run. I've disabled it in the final version of the driver, but here it is
    in this comment if you want to run it yourself:

void yamaha_dx100_state::init_dx100()
{
    // Since test mode forcibly halts on the first failed test, and we can only choose to not run certain tests, use these blocks of code to skip specific tests.
    // The numbering and naming below is from the service manual, which, surprise, doesn't quite line up with the way the tests are written in the code.
    u8 *rom = (u8 *) (memregion("program")->base());
    auto skip = [rom](u16 whichAddr, u16 jumpTo, u8 extraInst = 0, u16 nop2 = 0) {
        if (extraInst != 0) {
            rom[whichAddr & 0x7fff] = extraInst;
            whichAddr++;
        }
        rom[whichAddr & 0x7fff] = 0x7e;
        rom[(whichAddr + 1) & 0x7fff] = (u8) ((jumpTo >> 8) & 0xff);
        rom[(whichAddr + 2) & 0x7fff] = (u8) (jumpTo & 0xff);
        if (nop2 != 0) {
            rom[nop2 & 0x7fff] = 0x01;
            rom[(nop2 + 1) & 0x7fff] = 0x01;
        }
    };
    // 1. Output level, RAM battery voltage
    skip(0xc464, 0xc4b3);
    // 2. RAM, cassette interface, MIDI in/out, MIDI thru
    // This is hard to skip as a unit in the code, so you need to skip them individually:
        // Or to only disable a subset of these tests:
        // 2a. RAM test
        rom[0xc55f & 0x7fff] = 0x39;
        // 2b. Cassette interface test (I'm not entirely sure MAME's cassette interface
        //     supports the direct writing; I *think* this is MSX format?)
        rom[0xc66e & 0x7fff] = 0x39;
        // 2c. MIDI in/out (I'm not sure why this is failing, but something's not right)
        rom[0xc631 & 0x7fff] = 0x39;
        // 2d. MIDI thru
        skip(0xc6b0, 0xc52a, 0, 0xc4dd);
    // 3. LCD test
    // This is actually two tests in the code, so you need to skip them individually:
        // 2a. LCD solid flashing test
        skip(0xc6e1, 0xc52a, 0, 0xc4e6);
        // 2b. LCD checkerboard test
        skip(0xc6d1, 0xc52a, 0, 0xc4ef);
    // 4. Pitch wheel, mod wheel, data entry slider, breath controller, foot switch
    // AND (run as part of 4 in the code)
    // 5. Foot switch detecting circuit
    skip(0xc70f, 0xc7a9);
        // Or to only disable a subset of these tests:
        // 4a/5. Foot switch tests
        skip(0xc7b3, 0xc7a9, 0x38);
    // 6. Keyboard
    skip(0xc86e, 0xc927);
    // 7. LCD contrast
    // AND (run as part of 7 in the code)
    // 8. Main power, LED flash on low power
    skip(0xc927, 0xc98f);
    // 9. Panel switches
    skip(0xc9d3, 0xca50);
}

*******************************************************************************/

#include "emu.h"

#include "bus/midi/midi.h"
#include "cpu/m6800/m6801.h"
#include "imagedev/cassette.h"
#include "machine/adc0808.h"
#include "machine/clock.h"
#include "machine/nvram.h"
#include "sound/ymopm.h"
#include "video/hd44780.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "dx100.lh"

namespace {

class yamaha_dx100_state : public driver_device
{
public:
	yamaha_dx100_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_adc(*this, "adc")
		, m_keysbuttons(*this, "P6_%d", 0)
		, m_midi_in(true)
		, m_led(*this, "LED")
		, m_cassette(*this, "cassette")
	{
	}

	void dx100(machine_config &config);

	void led_w(int state)                  { m_led = state; }
	ioport_value midi_in_r() { return m_midi_in; }

protected:
	virtual void driver_start() override;
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	HD44780_PIXEL_UPDATE(lcd_pixel_update);
	void palette_init(palette_device &palette);

	void p22_w(int state);

	void mem_map(address_map &map) ATTR_COLD;

	required_device<hd6303x_cpu_device> m_maincpu;
	required_device<m58990_device> m_adc;
	required_ioport_array<16> m_keysbuttons;

	bool m_midi_in;

	u8 m_port6_val;

	// for hooking up the LED to layouts
	output_finder<> m_led;

	required_device<cassette_image_device> m_cassette;
};

void yamaha_dx100_state::driver_start()
{
	m_led.resolve();
}

void yamaha_dx100_state::machine_start()
{
	save_item(NAME(m_midi_in));
}

void yamaha_dx100_state::machine_reset()
{
	// TODO: figure out the actual power-on state
	m_port6_val = 0;
}

HD44780_PIXEL_UPDATE(yamaha_dx100_state::lcd_pixel_update)
{
	if (x < 5 && y < 8 && line < 2 && pos < 8)
		bitmap.pix(y + 1 + ((y == 7) ? 1 : 0), (line * 8 + pos) * 6 + x + 1) = state ? 1 : 2;
}

void yamaha_dx100_state::palette_init(palette_device &palette)
{
	palette.set_pen_color(0, rgb_t(0xff, 0xff, 0xff)); // background
	palette.set_pen_color(1, rgb_t(0x00, 0x00, 0x00)); // lcd pixel on
	palette.set_pen_color(2, rgb_t(0xe7, 0xe7, 0xe7)); // lcd pixel off
}

void yamaha_dx100_state::p22_w(int state)
{
	if (state)
		m_maincpu->clock_serial();
}

void yamaha_dx100_state::mem_map(address_map &map)
{
	map(0x0800, 0x0fff).ram().share("nvram");
	map(0x1000, 0x17ff).ram();
	map(0x2000, 0x2001).rw("lcdc", FUNC(hd44780_device::read), FUNC(hd44780_device::write));
	map(0x2800, 0x2800).r("adc", FUNC(m58990_device::data_r));
	map(0x3000, 0x3000).w("adc", FUNC(m58990_device::address_data_start_w));
	map(0x3800, 0x3801).rw("ymsnd", FUNC(ym2164_device::read), FUNC(ym2164_device::write));
	map(0x8000, 0xffff).rom().region("program", 0);
}

static INPUT_PORTS_START(dx100)
	PORT_START("P2")
	// TODO: Should 0x02, 0x04, 0x10, and 0x80 be listed here?
	// They should be handled by the other interconnections in this file.
	// If so, verify the active states of the MIDI ports.
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OUTPUT )   PORT_NAME("LED") PORT_WRITE_LINE_MEMBER(yamaha_dx100_state, led_w)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )  // tied to ground
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM )  // 500khz clock
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM )  PORT_CUSTOM_MEMBER(yamaha_dx100_state, midi_in_r)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OUTPUT )  // MIDI out
	PORT_CONFNAME( 0x20, 0x00, "Foot Switch" )
	PORT_CONFSETTING( 0x00, "Connected" )
	PORT_CONFSETTING( 0x20, "Disconnected" )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER )   PORT_NAME("Foot Switch")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )  // tied to ground

	PORT_START("P6_0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER )  PORT_NAME("Data Entry +1/Yes/On") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER )  PORT_NAME("Data Entry -1/No/Off") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER )  PORT_NAME("Store/EG Copy") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER )  PORT_NAME("Function/Compare") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER )  PORT_NAME("Edit/Compare") PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER )  PORT_NAME("Internal/Play") PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER )  PORT_NAME("Pitch B Mode/Operator Select/Mode Set") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER )  PORT_NAME("Key Shift/Key Set") PORT_CODE(KEYCODE_W)

	PORT_START("P6_1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER )  PORT_NAME("1/Algorithm/Master Tune Adj") PORT_CODE(KEYCODE_1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER )  PORT_NAME("2/Feedback/MIDI On/Off") PORT_CODE(KEYCODE_2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER )  PORT_NAME("3/LFO Wave/MIDI Channel") PORT_CODE(KEYCODE_3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER )  PORT_NAME("4/LFO Speed/MIDI Ch Info") PORT_CODE(KEYCODE_4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER )  PORT_NAME("5/LFO Delay/MIDI Sys Info") PORT_CODE(KEYCODE_5)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER )  PORT_NAME("6/LFO PMD/Recall Edit") PORT_CODE(KEYCODE_6)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER )  PORT_NAME("7/LFO AMD/Init Voice") PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER )  PORT_NAME("8/LFO Sync/Edit Bank") PORT_CODE(KEYCODE_8)

	PORT_START("P6_2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER )  PORT_NAME("9/Modulation Sensitivity Pitch/Cassette Save/Verify") PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER )  PORT_NAME("10/Modulation Sensitivity Velocity/Cassette Load") PORT_CODE(KEYCODE_0)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER )  PORT_NAME("11/Modulation Sensitivity EG Bias/Cassette Load Single") PORT_CODE(KEYCODE_MINUS)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER )  PORT_NAME("12/Key Velocity/Memory Protect") PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER )  PORT_NAME("13/Oscillator Freq Ratio/Poly/Mono")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER )  PORT_NAME("14/Oscillator Detune/Pitch Bend Range")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER )  PORT_NAME("15/Envelope Generator AR/Portamento Mode")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER )  PORT_NAME("16/Envelope Generator D1R/Portamento Time")

	PORT_START("P6_3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER )  PORT_NAME("17/Envelope Generator D1L/Foot Sw Assign")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER )  PORT_NAME("18/Envelope Generator D2R/Wheel Range Pitch")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER )  PORT_NAME("19/Envelope Generator RR/Wheel Range Amplitude")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER )  PORT_NAME("20/Operator Out Level/Breath Range Pitch")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER )  PORT_NAME("21/Keyboard Scaling Rate/Breath Range Amplitude")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER )  PORT_NAME("22/Keyboard Scaling Level/Breath Range Pitch Bias")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER )  PORT_NAME("23/Transpose/Breath Range EG Bias")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER )  PORT_NAME("24/Voice Name Cursor >")

	PORT_START("P6_4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )  PORT_NAME("C#1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )  PORT_NAME("C#2")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )  PORT_NAME("C#3") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )  PORT_NAME("C#4")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER )  PORT_NAME("Bank D/Operator/AMS On/Off 4/Preset Search 401~424") PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER )  PORT_NAME("Bank C/Operator/AMS On/Off 3/Preset Search 301~324") PORT_CODE(KEYCODE_OPENBRACE)

	PORT_START("P6_5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )  PORT_NAME("D1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )  PORT_NAME("D2")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )  PORT_NAME("D3") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )  PORT_NAME("D4")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER )  PORT_NAME("Bank B/Operator/AMS On/Off 2/Preset Search 201~224") PORT_CODE(KEYCODE_P)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER )  PORT_NAME("Bank A/Operator/AMS On/Off 1/Preset Search 101~124") PORT_CODE(KEYCODE_O)

	PORT_START("P6_6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )  PORT_NAME("D#1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )  PORT_NAME("D#2")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )  PORT_NAME("D#3") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )  PORT_NAME("D#4")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P6_7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )  PORT_NAME("E1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )  PORT_NAME("E2")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )  PORT_NAME("E3") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )  PORT_NAME("E4")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P6_8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )  PORT_NAME("F1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )  PORT_NAME("F2")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )  PORT_NAME("F3") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )  PORT_NAME("F4")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P6_9")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )  PORT_NAME("F#1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )  PORT_NAME("F#2")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )  PORT_NAME("F#3") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )  PORT_NAME("F#4")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P6_10")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )  PORT_NAME("G1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )  PORT_NAME("G2")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )  PORT_NAME("G3") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )  PORT_NAME("G4")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P6_11")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )  PORT_NAME("G#1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )  PORT_NAME("G#2")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )  PORT_NAME("G#3") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )  PORT_NAME("G#4")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P6_12")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )  PORT_NAME("A1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )  PORT_NAME("A2")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )  PORT_NAME("A3") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )  PORT_NAME("A4")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P6_13")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )  PORT_NAME("A#1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )  PORT_NAME("A#2")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )  PORT_NAME("A#3") PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )  PORT_NAME("A#4")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P6_14")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )  PORT_NAME("B1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )  PORT_NAME("B2")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )  PORT_NAME("B3") PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )  PORT_NAME("B4")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P6_15")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )  PORT_NAME("C1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )  PORT_NAME("C2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )  PORT_NAME("C3") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )  PORT_NAME("C4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )  PORT_NAME("C5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("AN0")
	// The pitch wheel returns to center once released.
	PORT_BIT( 0xff, 0x7f, IPT_AD_STICK_Y ) PORT_NAME("Pitch Wheel") PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_MINMAX(0x00, 0xff) PORT_CODE_INC(KEYCODE_QUOTE) PORT_CODE_DEC(KEYCODE_SLASH)

	PORT_START("AN1")
	// The mod wheel stays in place to wherever it's set.
	// TODO: This should have its initial state be 0, not halfway.
	PORT_BIT( 0xff, 0, IPT_PADDLE_V ) PORT_NAME("Modulation Wheel") PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_MINMAX(0x00, 0xff) PORT_CENTERDELTA(0) PORT_CODE_INC(KEYCODE_COLON) PORT_CODE_DEC(KEYCODE_STOP)

	PORT_START("AN2")
	// TODO: This appears to be inverted; if this is set to 255 it behaves as if
	// there was no breath controller? or at least seems to? on instruments like
	// 112 Pianobrass if this is set to 0 it acts as if the mod wheel had been turned
	// all the way up and enables LFO -- and there's probably a better way we could
	// simulate not having a breath controller attached at all???
	PORT_BIT( 0xff, 0, IPT_PEDAL ) PORT_NAME("Breath Controller") PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_MINMAX(0x00, 0xff)

	PORT_START("AN3")
	// The data entry slider stays in place to wherever it's set.
	PORT_BIT( 0xff, 0, IPT_POSITIONAL_V ) PORT_NAME("Data Entry Slider") PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_MINMAX(0x00, 0xff) PORT_CENTERDELTA(0) PORT_CODE_INC(KEYCODE_L) PORT_CODE_DEC(KEYCODE_COMMA)

	PORT_START("AN5")
	PORT_CONFNAME( 0xff, 0x80, "Internal RAM Battery Level" )
	// "CNG RAM BATTERY!" displayed unless value is between 0x70 and 0xcc
	PORT_CONFSETTING( 0x6f, "Too Low" )
	PORT_CONFSETTING( 0x70, "Lowest Allowed" )
	PORT_CONFSETTING( 0x80, "Normal" ) // for some arbitrary definition of "normal"
	PORT_CONFSETTING( 0xcb, "Highest Allowed" )
	PORT_CONFSETTING( 0xcc, "Too High" )

	PORT_START("AN6")
	PORT_CONFNAME( 0xff, 0x00, "Battery Power Level" )
	// Yes, higher values mean lower voltages.
	// I think this is opposite to how the RAM battery voltmeter works.
	// The weird granularity here is due to the buggy implementation of the
	// test in the test mode; all 7V values should LED flash, but some don't.
	PORT_CONFSETTING( 0x00, "9V (Normal)" ) // for some arbitrary definition of "normal"
	PORT_CONFSETTING( 0x4b, "9V (Lowest Allowed)" )
	PORT_CONFSETTING( 0x67, "7V (Highest Allowed Without LED Flash)" )
	PORT_CONFSETTING( 0x6b, "7V (Lowest Allowed Without LED Flash)" )
	PORT_CONFSETTING( 0x6c, "7V (Highest Allowed With LED Flash)" )
	PORT_CONFSETTING( 0x6f, "7V (Lowest Allowed With LED Flash)" )
	PORT_CONFSETTING( 0x70, "Less Than 7V" )
INPUT_PORTS_END

void yamaha_dx100_state::dx100(machine_config &config)
{
	HD6303X(config, m_maincpu, 7.15909_MHz_XTAL / 2); // HD6303XP
	m_maincpu->set_addrmap(AS_PROGRAM, &yamaha_dx100_state::mem_map);
	m_maincpu->in_p2_cb().set_ioport("P2");
	m_maincpu->out_p2_cb().set_ioport("P2");
	m_maincpu->in_p6_cb().set([this]() -> u8 {
		// TODO: The threshold here and the output values below should be double-checked.
		u8 casplay = (m_cassette->input() > 0.009) ? 0x80 : 0;
		// TODO: Need to verify if this bit is held on output or not
		u8 casrec = m_port6_val & 0x40;
		u8 adcval = ((u8) (m_adc->eoc_r() << 4)) & 0x10;
		u8 port5line = (m_port6_val & 0x2f);
		return casplay | casrec | adcval | port5line;
	});
	m_maincpu->out_p6_cb().set([this](u8 val) {
		m_port6_val = val;
		m_cassette->output(((m_port6_val & 0x40) != 0) ? -1.0 : 1.0);
	});
	m_maincpu->in_p5_cb().set([this]() -> u8 {
		u8 line = m_port6_val & 0x2f;
		// TODO: figure out how the /G2A and /G2B pins are connected
		if ((line & 0x20) != 0)
			return 0x00;
		return (u8) (m_keysbuttons[line]->read() & 0xff);
	});
	m_maincpu->out_ser_tx_cb().set("mdout", FUNC(midi_port_device::write_txd));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // TC5518BPL + CR2032T battery

	M58990(config, m_adc, 7.15909_MHz_XTAL / 8); // M58990P-1 (clocked by E)
	m_adc->in_callback<0>().set_ioport("AN0");   // pitch wheel
	m_adc->in_callback<1>().set_ioport("AN1");   // mod wheel
	m_adc->in_callback<2>().set_ioport("AN2");   // breath controller
	m_adc->in_callback<3>().set_ioport("AN3");   // data entry slider
	m_adc->in_callback<5>().set_ioport("AN5");   // internal RAM battery voltmeter
	m_adc->in_callback<6>().set_ioport("AN6");   // battery power voltmeter

	CLOCK(config, "subclock", 500_kHz_XTAL).signal_handler().set(FUNC(yamaha_dx100_state::p22_w));

	MIDI_PORT(config, "mdin", midiin_slot, "midiin").rxd_handler().set([this](int state) { m_midi_in = state; });
	MIDI_PORT(config, "mdout", midiout_slot, "midiout");

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update("lcdc", FUNC(hd44780_device::screen_update));
	screen.set_size(6*16+1, 10*1+1);
	screen.set_visarea_full();
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(yamaha_dx100_state::palette_init), 3);

	hd44780_device &lcdc(HD44780(config, "lcdc", 270'000)); // HD44780RA00, 91K resistor
	lcdc.set_lcd_size(1, 16);
	lcdc.set_pixel_update_cb(FUNC(yamaha_dx100_state::lcd_pixel_update));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	ym2164_device &ymsnd(YM2164(config, "ymsnd", 7.15909_MHz_XTAL / 2)); // with YM3014 DAC
	ymsnd.add_route(0, "lspeaker", 0.60);
	ymsnd.add_route(1, "rspeaker", 0.60);

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED);
	m_cassette->set_interface("ymdx100_cass");

	config.set_default_layout(layout_dx100);
}

ROM_START(dx100)
	ROM_REGION(0x8000, "program", 0)
	ROM_LOAD("dx100 v1.1.bin", 0x0000, 0x8000, CRC(c3ed7c86) SHA1(5b003db1bb5c1909907153f6446b63b07f5b41d6))
ROM_END

} // anonymous namespace

SYST(1985, dx100, 0, 0, dx100, dx100, yamaha_dx100_state, empty_init, "Yamaha", "DX100 Digital Programmable Algorithm Synthesizer", MACHINE_IMPERFECT_SOUND)

