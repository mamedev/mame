// license:BSD-3-Clause
// copyright-holders: 68bit
/******************************************************************************

Micro Chroma 68
Motorola evaluation board for the MC6847 VDG and MC1372 RF modulator.

The Micro Chroma was supplied as a kit that included the PCB and the major
parts and is reported to have been designed by the "applications group" in
Austin including Tim Ahrens, Jack Brown, and Hunter Scales. The micro Chroma
68 uses the TVBUG monitor which is a 2k monitor ROM that is included in the
MC6846 timer and is reported to have been written by John Dumas with parts
taken from MIKBUG with the help of Mike Wiles the author of MIKBUG.

References:

1. "Micro Chroma 68: The new 'bug' from Motorola TVBUG 1.2", Motorola
   Semiconductor Products Inc. Microprocessor Operations., 1978

G - Go to user program.
G <addr> - Go to user program, starting at the given address 'addr'.
L - Load tape. The encoding uses the Kansas City Standard and the format is
   largely JBUG compatible. Enter an offset, the difference between the
   existing start address and the target start address. If the data can not be
   stored in the memory then 'Memory Bad' is printed and it exits. The load
   can be aborted by pressing the Escape key.
P - Punch tape. Enter the start address, the end address, a name up to 31
   characters long, and enter return to start. The format is largely JBUG
   compatible, just avoid using 'B' or 'G' in the name. The punch can be
   aborted by pressing the Escape key.
V - Verify Tape. Enter an offset, a difference between the existing start
   address and the target start address. If there is an error then 'Memory
   Bad' is printed and it exits. The verfiy can be aborted by pressing the
   Escape key.
M <addr> - Memory examine and change at address 'addr'.
  <hh><linefeed> - Enter new data for the address and increase the address.
    If the memory can not be changed the 'Memory Bad' is printed that the
    memory examine is exited.
  '^' - decrease the address.
  <carriage return> - exit the memory examine function.
E - Examine a memory block. Enter the start address, the end address, and then
    a space to print the next line.
Q - Quick load. Enter the start address, the end address, then the hex bytes
    to load separate by a space.
F - Fill memory. Enter the start address, the end address, and the fill byte.
O - Offset calculation. Enter the start address and then the end address. The
    offset is printed as a 16 bit number, the lower 8 bits being the offset.
    If the offset is too large then 'TOO FAR' is printed.

R - Register display: CC B A X PC SP. The stack pointer (SP) is stored at
    0xf382 to 0xf383 and may be modifed via the memory examine 'M' function
    there. The other registers are on the stack at:
    SP + 1 : CC
    SP + 2 : B
    SP + 3 : A
    SP + 4,5 : X
    SP + 6,7 : P
Z - Clear screen.
S <addr> - Set a breakpoint at address 'addr'. Up to eight breakpoints may be
    defined. A breakpoint can not be set at 0x0000.
U <addr> - Unset the breakpoint at address 'addr'.
D - Remove all breakpoints.
B - Print all breakpoints.
C - Contine user program, until next breakpoint.
N - Traces one instruction.
T <n> - Traces 'n' instructions.

! - User defined code from 0xf396 to 0xf398, a jump instruction.
" - User defined code from 0xf399 to 0xf39a, a jump instruction.
# - User defined code from 0xf39c to 0xf39d, a jump instruction.

A user input function can be defined at 0xf390 to 0xf392, which is room for a
jump instruction, and this is called by the monitor input loop. This function
should set the carry bit on return if there is user input.

A user output function can be defined at 0xf393 to 0xf395 which is room for a
jump instruction.

The board has a "Reset" button, and a "Break" button that toggled the NMI
line. TODO implement these.

The documentation includes hardware suggestions for memory expansion. This
expands the RAM to use the address range 0x0000 to 0xcfff.

The address range 0xe800 to 0xefff is for user expansion. The documentation
gives examples using this address range for two 1k or one 2k EPROM.

The documentation includes position independent ROMable code for driving a
printer via the ACIA, and it also adds S19 'load', 'punch', and 'verify' tape
support for compatibility with MIKBUG, EXBUG, and MINIBUG. The documentation
suggests using this as an optional ROM at 0xe800 to 0xea73. The printer
support implements a user output function definable at 0xf393 to direct screen
output to the ACIA and with a hardware patch this can be directed to a RS232
printer. The entry point for the printer support is 0xe800. The entry point
for the S19 support is 0xe802, so type 'G E802 <CR>' to enter this option and
it displays a choice of operations. It prints a 'B' for each record 'punched',
and a 'S' for each record loaded or verified, and it reports the address of
any bad memory. The S19 support writes a 0x11 (DC1) to start the tape playback
and a 0x12 (DC2) to start the tape recording as might be expected for the
Texas Instruments 733 ASR twin tape cassette terminal with automatic device
control, but it does not appear to issue DC3 or DC4 to turn the tape off.

The documentation includes patches for a range or TSC software which is
generally supplied in Kansas City S19 format, so might need the above extra
EPROM support. The omission of any mention of support for Motorola software is
notable.

The documentation includes instructions on using the Micro Chroma 68 as a dumb
terminal, requiring some minor hardware modifications to reuse the MC6850 ACIA
for a RS-232 interface, and it includes ROMable code at 0xfc00 to 0xffff code
to implement this, a substitute for the TVBUG monitor. TODO might consider
implementing this, but it was hardly a good terminal for the time.

Address decoding was available in the range 0xc000 to 0xcfff in 1k blocks for
expansion.

******************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "machine/6821pia.h"
#include "machine/6850acia.h"
#include "machine/mc6846.h"
#include "machine/clock.h"
#include "machine/timer.h"
#include "video/mc6847.h"
#include "imagedev/cassette.h"
#include "sound/wave.h"
#include "speaker.h"

#include "machine/terminal.h"
#include "video/mc6845.h"
#include "screen.h"


namespace {

#define XTAL_UCHROMA68 3.579545_MHz_XTAL

class uchroma68_state : public driver_device
{
public:
	uchroma68_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_mc6846(*this, "mc6846")
		, m_mc6847(*this, "mc6847")
		, m_pia(*this, "pia")
		, m_acia(*this, "acia")
		, m_acia_tx_clock(*this, "acia_tx_clock")
		, m_screen(*this, "screen")
		, m_video_ram(*this, "videoram")
		, m_cass(*this, "cassette")
		, m_semi_graphics_six_mod(*this, "SEMI_GRAPHICS_SIX_MOD")
	{ }

	void uchroma68(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void uchroma68_mem(address_map &map) ATTR_COLD;

	TIMER_CALLBACK_MEMBER(kbd_strobe);

	uint8_t mc6847_videoram_r(offs_t offset);
	uint8_t pia_pa_r();
	uint8_t pia_pb_r();
	void mc6846_out_w(uint8_t data);
	void kbd_put(uint8_t data);
	emu_timer *m_kbd_strobe_timer;
	bool m_kbd_strobe;
	uint8_t m_kbd_data;
	bool m_video_inv;

	TIMER_DEVICE_CALLBACK_MEMBER(kansas_r);
	void kansas_w(int state);

	required_device<cpu_device> m_maincpu;
	required_device<mc6846_device> m_mc6846;
	required_device<mc6847_ntsc_device> m_mc6847;
	required_device<pia6821_device> m_pia;
	required_device<acia6850_device> m_acia;
	required_device<clock_device> m_acia_tx_clock;
	required_device<screen_device> m_screen;
	required_shared_ptr<uint8_t> m_video_ram;
	required_device<cassette_image_device> m_cass;
	required_ioport m_semi_graphics_six_mod;

	uint8_t m_cass_rx_clock_count;
	uint8_t m_cass_rx_period;
	uint8_t m_cass_txcount;
	bool m_cass_in;
	bool m_cass_inbit;
	bool m_cass_txbit;
	bool m_cass_last_txbit;
};


/***********************************************************

    Address Map

************************************************************/

void uchroma68_state::uchroma68_mem(address_map &map)
{
	// On board user RAM
	map(0x0000, 0x1fff).ram();

	// Off board RAM expansion, as documented.
	map(0x2000, 0xcfff).ram();

	// Display RAM
	map(0xd000, 0xe7ff).ram().share(m_video_ram);

	// Optional EPROM, with the supplied S19 tape format support.
	map(0xe800, 0xebff).rom();

	// Stack RAM
	map(0xf000, 0xf3ff).ram();

	map(0xf408, 0xf409).mirror(0x03b0).rw(m_acia, FUNC(acia6850_device::read), FUNC(acia6850_device::write));

	map(0xf404, 0xf407).mirror(0x03b0).rw(m_pia, FUNC(pia6821_device::read), FUNC(pia6821_device::write));

	// Programmable counter/timer and I/O.
	map(0xf440, 0xf447).mirror(0x0380).rw(m_mc6846, FUNC(mc6846_device::read), FUNC(mc6846_device::write));

	// TVBUG 1.2 ROM
	map(0xf800, 0xffff).rom();
}

/***********************************************************

    Keys

************************************************************/

static INPUT_PORTS_START(uchroma68)

	// The documentation notes a hardware modification to allow the use of
	// the Semi-graphics 6 mode of the MC6847.
	PORT_START("SEMI_GRAPHICS_SIX_MOD")
	PORT_CONFNAME(0x01, 0x00, "Semi-graphics 6 mode modification")
	PORT_CONFSETTING(0x00, "No")
	PORT_CONFSETTING(0x01, "Yes")

INPUT_PORTS_END


/***********************************************************

************************************************************/

TIMER_CALLBACK_MEMBER(uchroma68_state::kbd_strobe)
{
	m_kbd_strobe = 1;
	m_kbd_data = 0;
}

void uchroma68_state::mc6846_out_w(uint8_t data)
{
	m_mc6847->css_w(!BIT(data, 0));
	m_mc6847->intext_w(!BIT(data, 1));
	m_mc6847->ag_w(!BIT(data, 2));
	m_mc6847->gm0_w(BIT(data, 3));
	m_mc6847->gm1_w(BIT(data, 4));
	m_mc6847->gm2_w(BIT(data, 5));
	m_video_inv = BIT(data, 6);
	// P7 is NC.
}


uint8_t uchroma68_state::mc6847_videoram_r(offs_t offset)
{
	offset &= 0x1fff;
	if (offset > 0x17ff) return 0xff;

	uint8_t code = m_video_ram[offset];

	if (m_semi_graphics_six_mod->read())
		m_mc6847->inv_w(m_video_inv);
	else
		m_mc6847->inv_w(m_video_inv | BIT(code, 6));

	m_mc6847->as_w(BIT(code, 7));

	return code;
}

// TVBUG polls the parallel keyboard strobe every four video frames - it
// blinks the cursor for four frames between polling. At a 60Hz frame rate
// that gives polling at about every 67ms. A 70ms keyboard strobe period
// appears to be more than adequate.
void uchroma68_state::kbd_put(uint8_t data)
{
	m_kbd_data = data;
	m_kbd_strobe_timer->adjust(attotime::from_msec(70));
	m_kbd_strobe = 0;
}

uint8_t uchroma68_state::pia_pa_r()
{
	uint8_t data = m_kbd_data;
	return (m_kbd_strobe << 7) | data;
}

uint8_t uchroma68_state::pia_pb_r()
{
	// PB0 to PB4 are Up, Down, Left, Right, Home.
	// PB5 is NC
	// PB6 is V VDG
	// PB7 is H VDG
	return (m_mc6847->hs_r() << 7) | (m_mc6847->fs_r() << 6);
}


TIMER_DEVICE_CALLBACK_MEMBER(uchroma68_state::kansas_r)
{
	// Turn 1200/2400Hz to a bit
	uint8_t cassin = (m_cass->input() > +0.04) ? 1 : 0;
	uint8_t inbit = m_cass_inbit;

	m_cass_rx_period++;

	// The RX clock is recovered from the input data, and synchronized to
	// the bit transitions. The software expects a 1x RX clock.
	if (++m_cass_rx_clock_count > 133)
		m_cass_rx_clock_count = 0;

	if (cassin != m_cass_in)
	{
		// A transition, so now check the period.
		inbit = (m_cass_rx_period < 12) ? 1 : 0;
		m_cass_in = cassin;
		m_cass_rx_period = 0;
	}
	else if (m_cass_rx_period > 32)
	{
		// Idle the ACIA if there is no data.
		m_cass_rx_period = 32;
		inbit = 1;
	}

	if (inbit != m_cass_inbit)
	{
		m_acia->write_rxd(inbit);
		m_cass_inbit = inbit;
		// Sync the RX clock with the data edges.
		m_cass_rx_clock_count = 0;
	}

	if (m_cass_rx_clock_count == 0)
		m_acia->write_rxc(0);
	else if (m_cass_rx_clock_count == 67)
		m_acia->write_rxc(1);
}

void uchroma68_state::kansas_w(int state)
{
	// The Kansas City cassette format encodes a '0' bit by four cycles of
	// a 1200 Hz sine wave, and a '1' bit as eight cycles of 2400 Hz,
	// giving a 300 baud rate.
	//
	// The clock rate to the ACIA is 16x the baud rate, or 4800Hz, and is
	// divided by 2 to get the 2400 Hz rate, or divided by 4 to get the
	// 1200 Hz rate.

	// Sync the period phase on TX bit transitions.
	if (m_cass_txbit != m_cass_last_txbit)
	{
		m_cass_txcount = 0;
		m_cass_last_txbit = m_cass_txbit;
	}

	if (m_cass_txbit)
		m_cass->output(BIT(m_cass_txcount, 1) ? +1.0 : -1.0); // 2400Hz
	else
		m_cass->output(BIT(m_cass_txcount, 2) ? +1.0 : -1.0); // 1200Hz

	m_cass_txcount++;
	m_acia->write_txc(state);
}

void uchroma68_state::machine_start()
{
	m_kbd_strobe_timer = timer_alloc(FUNC(uchroma68_state::kbd_strobe), this);
	m_kbd_strobe = 1;

	save_item(NAME(m_kbd_strobe));
	save_item(NAME(m_kbd_data));
	save_item(NAME(m_video_inv));
	save_item(NAME(m_cass_rx_clock_count));
	save_item(NAME(m_cass_rx_period));
	save_item(NAME(m_cass_txcount));
	save_item(NAME(m_cass_in));
	save_item(NAME(m_cass_inbit));
	save_item(NAME(m_cass_txbit));
	save_item(NAME(m_cass_last_txbit));
}

void uchroma68_state::machine_reset()
{
	m_mc6846->reset();
	m_pia->reset();
	m_kbd_strobe_timer->reset();
	m_kbd_strobe = 1;
	m_kbd_data = 0;

	// These MC6846 I/O outputs are pulled high. The high impedance state
	// of these lines is not yet emulated by the mc6846 driver, and TVBUG
	// does not drive them. On reset these are high impedance anyway, and
	// pulled high, so their lines going to the VDG are initialized
	// here. The first three lines are inverted.
	m_mc6847->css_w(0);
	m_mc6847->intext_w(0);
	m_mc6847->ag_w(0);
	m_mc6847->gm0_w(1);
	m_mc6847->gm1_w(1);
	m_mc6847->gm2_w(1);
	m_video_inv = 1;

	m_cass_rx_clock_count = 0;
	m_cass_rx_period = 0;
	m_cass_txcount = 0;
	m_cass_in = 0;
	m_cass_inbit = 0;
	m_cass_txbit = 0;
	m_cass_last_txbit = 0;
}

/***********************************************************

    Machine

************************************************************/

void uchroma68_state::uchroma68(machine_config &config)
{
	M6808(config, m_maincpu, XTAL_UCHROMA68);        // 894.8 kHz clock
	m_maincpu->set_addrmap(AS_PROGRAM, &uchroma68_state::uchroma68_mem);

	MC6846(config, m_mc6846, XTAL_UCHROMA68 / 4);  // Same as the cpu clock
	m_mc6846->out_port().set(FUNC(uchroma68_state::mc6846_out_w));

	SPEAKER(config, "mono").front_center();

	CASSETTE(config, m_cass);
	m_cass->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);

	MC6847_NTSC(config, m_mc6847, XTAL_UCHROMA68);
	m_mc6847->set_screen(m_screen);
	m_mc6847->input_callback().set(FUNC(uchroma68_state::mc6847_videoram_r));

	PIA6821(config, m_pia);
	m_pia->readpa_handler().set(FUNC(uchroma68_state::pia_pa_r));
	m_pia->readpb_handler().set(FUNC(uchroma68_state::pia_pb_r));

	ACIA6850(config, m_acia, 0);
	m_acia->txd_handler().set([this] (bool state) { m_cass_txbit = state; });

	CLOCK(config, m_acia_tx_clock, 4800);
	m_acia_tx_clock->signal_handler().set(FUNC(uchroma68_state::kansas_w));

	TIMER(config, "kansas_r").configure_periodic(FUNC(uchroma68_state::kansas_r), attotime::from_hz(40000));

	generic_keyboard_device &keyboard(GENERIC_KEYBOARD(config, "keyboard", 0));
	keyboard.set_keyboard_callback(FUNC(uchroma68_state::kbd_put));
}

/***********************************************************

    ROMS

************************************************************/

ROM_START(uchroma68)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD("tvbug.rom", 0xf800, 0x0800, CRC(47f721e4) SHA1(31ea8d596f1f99ee26c4bea694245448bfdc3ee6))
	ROM_LOAD("uch68s19.rom", 0xe800, 0x0400, CRC(8932484f) SHA1(231098ab10185d400032415cf9b7273d736d3b87))
ROM_END

} // anonymous namespace


/***************************************************************************

  Game driver(s)

***************************************************************************/

//    YEAR  NAME    PARENT  COMPAT  MACHINE   INPUT  CLASS        INIT        COMPANY     FULLNAME      FLAGS
COMP( 1980, uchroma68,  0,      0,      uchroma68,    uchroma68, uchroma68_state, empty_init, "Motorola", "Micro Chroma 68" , MACHINE_NO_SOUND_HW )
