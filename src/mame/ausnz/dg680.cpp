// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

DG680

2013-01-14 Driver created

All input must be in uppercase.

DG680 (ETI-680), using the DGOS-Z80 operating system.

This is a S100 card.

In some ways, this system is the ancestor of the original Microbee.

No schematic available, most of this is guesswork.

Port 0 is the input from an ascii keyboard.

Port 2 is the cassette interface.

Port 8 controls some kind of memory protection scheme.
The code indicates that B is the page to protect, and
A is the code (0x08 = inhibit; 0x0B = unprotect;
0x0C = enable; 0x0E = protect). There are 256 pages so
each page is 256 bytes.

The clock is controlled by the byte in D80D.

Monitor Commands:
C (compare)*
E (edit)*
F (fill)*
G - Go to address
I - Inhibit CTC
M (move)*
P (clear screen)*
R (read tape)*
S (search)*
T hhmm [ss] - Set the time
W (write tape)*
X - protection status
XC - clear ram
XD - same as X
XE - enable facilities
XF - disable facilities
XP - protect block
XU - unprotect block
Z - go to 0000.

* These commands are identical to the Microbee ones.

ToDo:
- dips
- leds
- need schematic to find out what else is missing

****************************************************************************/

#include "emu.h"
#include "machine/keyboard.h"
#include "machine/z80daisy.h"
#include "cpu/z80/z80.h"
#include "imagedev/cassette.h"
#include "machine/clock.h"
#include "machine/timer.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"
#include "bus/s100/s100.h"
#include "bus/s100/dg640.h"
#include "speaker.h"


namespace {

class dg680_state : public driver_device
{
public:
	dg680_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cass(*this, "cassette")
		, m_pio(*this, "pio")
		, m_ctc(*this, "ctc")
		, m_clock(*this, "cass_clock")
		, m_s100(*this, "s100")
	{ }

	void dg680(machine_config &config);

private:
	u8 porta_r();
	u8 portb_r();
	void portb_w(u8 data);
	u8 port08_r();
	void port08_w(u8 data);
	u8 mem_r(offs_t offset);
	void mem_w(offs_t offset, u8 data);
	void kansas_w(int state);
	TIMER_DEVICE_CALLBACK_MEMBER(kansas_r);
	void kbd_put(u8 data);

	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;
	void machine_reset() override ATTR_COLD;
	void machine_start() override ATTR_COLD;

	u8 m_pio_b = 0U;
	u8 m_term_data = 0U;
	u8 m_protection[0x100]{};
	u8 m_cass_data[4]{};
	bool m_cassold = 0, m_cassinbit = 0, m_cassoutbit = 0;

	required_device<cpu_device> m_maincpu;
	required_device<cassette_image_device> m_cass;
	required_device<z80pio_device> m_pio;
	required_device<z80ctc_device> m_ctc;
	required_device<clock_device> m_clock;
	required_device<s100_bus_device> m_s100;
};

void dg680_state::kansas_w(int state)
{
	if ((m_cass->get_state() & CASSETTE_MASK_UISTATE) != CASSETTE_RECORD)
		return;

	u8 twobit = m_cass_data[3] & 15;

	if (state)
	{
		if (twobit == 0)
			m_cassold = m_cassoutbit;

		if (m_cassold)
			m_cass->output(BIT(m_cass_data[3], 0) ? -1.0 : +1.0); // 2400Hz
		else
			m_cass->output(BIT(m_cass_data[3], 1) ? -1.0 : +1.0); // 1200Hz

		m_cass_data[3]++;
	}
}

TIMER_DEVICE_CALLBACK_MEMBER( dg680_state::kansas_r )
{
	// no tape - set to idle
	m_cass_data[1]++;
	if (m_cass_data[1] > 32)
	{
		m_cass_data[1] = 32;
		m_cassinbit = 1;
	}

	if ((m_cass->get_state() & CASSETTE_MASK_UISTATE) != CASSETTE_PLAY)
		return;

	/* cassette - turn 1200/2400Hz to a bit */
	u8 cass_ws = (m_cass->input() > +0.04) ? 1 : 0;

	if (cass_ws != m_cass_data[0])
	{
		m_cass_data[0] = cass_ws;
		m_cassinbit = (m_cass_data[1] < 12) ? 1 : 0;
		m_cass_data[1] = 0;
		m_pio->pb0_w(m_cassinbit);
	}
}

u8 dg680_state::mem_r(offs_t offset)
{
	return m_s100->smemr_r(offset + 0xf000);
}

void dg680_state::mem_w(offs_t offset, u8 data)
{
	m_s100->mwrt_w(offset + 0xf000, data);
}


void dg680_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xcfff).ram();
	map(0xd000, 0xd7ff).rom().region("maincpu", 0);
	map(0xd800, 0xefff).ram();
	map(0xf000, 0xffff).rw(FUNC(dg680_state::mem_r),FUNC(dg680_state::mem_w));
}

void dg680_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x03).rw(m_pio, FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
	map(0x04, 0x07).rw(m_ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x08, 0x08).rw(FUNC(dg680_state::port08_r), FUNC(dg680_state::port08_w)); //SWP Control and Status
	//map(0x09,0x09) parallel input port
	// Optional AM9519 Programmable Interrupt Controller (port c = data, port d = control)
	//map(0x0c,0x0d).rw("am9519", FUNC(am9519_device::read), FUNC(am9519_device::write));
}

void dg680_state::machine_start()
{
	save_item(NAME(m_pio_b));
	save_item(NAME(m_term_data));
	save_item(NAME(m_protection));
	save_item(NAME(m_cass_data));
	save_item(NAME(m_cassold));
	save_item(NAME(m_cassinbit));
	save_item(NAME(m_cassoutbit));
}

void dg680_state::machine_reset()
{
	m_maincpu->set_pc(0xd000);
	m_pio_b = 0xFF;
}

// this is a guess there is no information available
static const z80_daisy_config dg680_daisy_chain[] =
{
	{ "ctc" },
	{ "pio" },
	{ 0x00 }
};


/* Input ports */
static INPUT_PORTS_START( dg680 )
INPUT_PORTS_END

void dg680_state::kbd_put(u8 data)
{
	if (data == 8)
		data = 127;   // fix backspace
	m_term_data = data;
	/* strobe in keyboard data */
	m_pio->strobe_a(0);
	m_pio->strobe_a(1);
}

u8 dg680_state::porta_r()
{
	u8 data = m_term_data;
	m_term_data = 0;
	return data;
}

u8 dg680_state::portb_r()
{
	return m_pio_b | m_cassinbit;
}

// bit 1 = cassout; bit 2 = motor on
void dg680_state::portb_w(u8 data)
{
	if (BIT(m_pio_b ^ data, 2))
		m_cass->change_state(BIT(data, 2) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);
	m_pio_b = data & 0xfe;
	m_cassoutbit = BIT(data, 1);
}

u8 dg680_state::port08_r()
{
	u8 breg = m_maincpu->state_int(Z80_B);
	return m_protection[breg];
}

void dg680_state::port08_w(u8 data)
{
	u8 breg = m_maincpu->state_int(Z80_B);
	m_protection[breg] = data;
}


static void dg680_s100_devices(device_slot_interface &device)
{
	device.option_add("dg640", S100_DG640);
}

DEVICE_INPUT_DEFAULTS_START(dg680_dg640_f000)
	DEVICE_INPUT_DEFAULTS("DSW", 0x1f, 0x1e) // F000-F7FF
DEVICE_INPUT_DEFAULTS_END

void dg680_state::dg680(machine_config &config)
{
	SPEAKER(config, "mono").front_center();

	/* Cassette */
	CASSETTE(config, m_cass);
	m_cass->set_default_state(CASSETTE_PLAY | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);
	TIMER(config, "kansas_r").configure_periodic(FUNC(dg680_state::kansas_r), attotime::from_hz(40000));

	CLOCK(config, m_clock, 4'800); // 300 baud x 16(divider) = 4800
	m_clock->signal_handler().set(FUNC(dg680_state::kansas_w));
	m_clock->signal_handler().append(m_ctc, FUNC(z80ctc_device::trg2));
	m_clock->signal_handler().append(m_ctc, FUNC(z80ctc_device::trg3));

	/* basic machine hardware */
	z80_device& maincpu(Z80(config, m_maincpu, XTAL(8'000'000) / 4));
	maincpu.set_addrmap(AS_PROGRAM, &dg680_state::mem_map);
	maincpu.set_addrmap(AS_IO, &dg680_state::io_map);
	maincpu.set_daisy_config(dg680_daisy_chain);

	/* Keyboard */
	generic_keyboard_device &keyb(GENERIC_KEYBOARD(config, "keyb", 0));
	keyb.set_keyboard_callback(FUNC(dg680_state::kbd_put));

	/* Devices */
	Z80CTC(config, m_ctc, XTAL(8'000'000) / 4);
	m_ctc->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_ctc->set_clk<0>(200);
	m_ctc->zc_callback<0>().set(m_ctc, FUNC(z80ctc_device::trg1));

	Z80PIO(config, m_pio, XTAL(8'000'000) / 4);
	m_pio->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_pio->in_pa_callback().set(FUNC(dg680_state::porta_r));
	// OUT_ARDY - this activates to ask for kbd data but not known if actually used
	m_pio->in_pb_callback().set(FUNC(dg680_state::portb_r));
	m_pio->out_pb_callback().set(FUNC(dg680_state::portb_w));

	S100_BUS(config, m_s100, 1_MHz_XTAL);
	S100_SLOT(config, "s100:1", dg680_s100_devices, "dg640")
		.set_option_device_input_defaults("dg640", DEVICE_INPUT_DEFAULTS_NAME(dg680_dg640_f000));
}

/* ROM definition */
ROM_START( dg680 )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "dg680.rom", 0x0000, 0x0800, BAD_DUMP CRC(c1aaef6a) SHA1(1508ca8315452edfb984718e795ccbe79a0c0b58) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "82s123.bin", 0x0000, 0x0020, NO_DUMP )
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY            FULLNAME                   FLAGS
COMP( 1980, dg680, 0,      0,      dg680,   dg680, dg680_state, empty_init, "David Griffiths", "DG680 with DGOS-Z80 1.4", MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
