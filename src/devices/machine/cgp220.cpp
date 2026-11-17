// license:BSD-3-Clause
// copyright-holders:tim lindner
/**********************************************************************

    Radio Shack TRS-80 Color Graphics Printer (CGP-220, 26-1268)

    A Canon based ink jet printer with one nozzle for each of black,
    cyan, magenta and yellow. It prints one dot row per carriage pass,
    in both directions, and steps the paper between passes.

    Hardware:
        MC6809 with a 4 MHz crystal
        MC6821 PIA ($0010)
            PA0-PA3  out  ink jet drive: black, cyan, magenta, yellow
            PA4-PA6  in   front panel FEED, ON LINE, RESET (active low)
            PA7      in   TR photo detector, carriage position
            CA1      in   TP photo detector, one pulse per dot column
            PB0      out  carriage forward (active low)
            PB1      out  carriage reverse (active low)
            PB2      out  PON, carriage motor full power
            PB3      out  paper motor enable
            PB4-PB7  out  paper feed stepper phases
            CB1      in   parallel STROBE
            CB2      in   serial data
            IRQA     FIRQ
            IRQB     IRQ
        MC6840 PTM ($0020)
            O1       PSTB, gated with PA0-PA3 to drive the nozzles
            G1       pulsed low by a write to $0080
            G2, G3, C1-C3 tied low
            IRQ      IRQ
        74LS374 parallel data latch, read at $0040
        74LS273 control latch, written at $0040
            D0       ACKNLG (inverted)
            D1       data input of the BUSY flip-flop
            D2       clocks the BUSY flip-flop
            D3       P.E (inverted)
            D4       BUSY and FAULT
            D5       serial STATUS (high = busy)
            D6       READY lamp (active low)
            D7       ALERT lamp (active low)
        74LS244 switches, read at $0041
            D0-D2    DSW1 ink jet drive delay switches
            D3       paper empty switch
            D4       SSW baud rate switch (600/2400)
            D5       serial plug inserted (a switch in the DIN socket)
            D6       PUMP button (active low)
            D7       serial data
        RAM $1000-$1FFF, EPROM $2000-$3FFF, A14 and A15 not decoded

    A STROBE presets a 74LS74 that holds parallel BUSY until the firmware
    clocks D1 low into it.

    The carriage is driven by a DC motor through an H bridge. PON
    switches the bridge to full power, otherwise an NE555 drives it at
    reduced power. The firmware regulates the speed by switching PON from
    the encoder pulse rate. The TR position sensor is dark over the home
    area to the left of the paper.
    The firmware finds where to start each right to left pass from a
    short mark on the sensor strip, so marks are placed to match the
    line widths the firmware prints: 112 dot multiples plus 30 for the
    nozzle spread, and 670 for a full line.

    Holding FEED at power on prints the self test.

    TODO:
    - The real sensor strip layout, motor characteristics and vertical
      dot pitch are not known.
    - ORD, a read strobe at $0100, leaves the sheet and is not emulated.

**********************************************************************/

#include "emu.h"
#include "cgp220.h"

#include "machine/input_merger.h"

#include "cgp220.lh"

#include <cmath>

#define LOG_CARRIAGE (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_CARRIAGE)
#include "logmacro.h"

#define LOGCARRIAGE(...) LOGMASKED(LOG_CARRIAGE, __VA_ARGS__)


namespace {

constexpr int DPI = 252;                      // 1 paper step, 42 steps per text line
constexpr int PAPER_WIDTH = DPI * 17 / 2;     // 8.5 inch
constexpr int PAPER_HEIGHT = DPI * 11;
constexpr int DOTS = 680;                     // 640 printable plus the nozzle spread
constexpr int DOT_MARGIN = (PAPER_WIDTH - 640 * DPI / 80) / 2;
constexpr int DOT_ROWS = 4;                   // paper steps covered by one dot
constexpr int NOZZLE_SPACING = 10;            // dot columns between nozzles

constexpr u32 INK_COLOUR[4] = { 0x000000, 0x00ffff, 0xff00ff, 0xffff00 };

// carriage model, in encoder pulses
constexpr double CR_LEFT_STOP = -200.0;
constexpr double CR_RIGHT_STOP = 760.0;
constexpr double CR_VHIGH = 4000.0;           // pulses per second with full power
constexpr double CR_VLOW = 400.0;             // pulses per second with reduced power
constexpr double CR_TAU_ON = 0.040;
constexpr double CR_TAU_OFF = 0.030;
constexpr double CR_TICK = 20e-6;

constexpr int CR_MARKS[] = { 144, 256, 368, 480, 592, 672 };
constexpr int CR_MARK_WIDTH = 2;

int dot_x(int dot)
{
	return DOT_MARGIN + dot * DPI / 80;
}

} // anonymous namespace


DEFINE_DEVICE_TYPE(CGP220, cgp220_device, "cgp220", "Radio Shack Color Graphics Printer CGP-220")


cgp220_device::cgp220_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, CGP220, tag, owner, clock),
	m_maincpu(*this, "maincpu"),
	m_pia(*this, "pia"),
	m_ptm(*this, "ptm"),
	m_bitmap_printer(*this, "bitmap"),
	m_buttons(*this, "BUTTONS"),
	m_switches(*this, "SWITCHES"),
	m_ready_led(*this, "cgp220_ready_led"),
	m_alert_led(*this, "cgp220_alert_led"),
	m_busy_cb(*this),
	m_ack_cb(*this),
	m_perror_cb(*this),
	m_fault_cb(*this),
	m_select_cb(*this),
	m_carriage_timer(nullptr),
	m_serial(true),
	m_rxd(1),
	m_strobe(1),
	m_data(0),
	m_latch(0),
	m_control(0xfe),
	m_busy_ff(false),
	m_heads(0),
	m_motor(0xff),
	m_paper_phase(0),
	m_paper_steps(0),
	m_cr_pos(CR_LEFT_STOP),
	m_cr_vel(0.0),
	m_cr_cell(int(CR_LEFT_STOP))
{
}


ROM_START( cgp220 )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "cgp220_v004e.ic402", 0x0000, 0x2000, CRC(9102e6b6) SHA1(17c50ead76133c156918e69303a169f475c8d59c) )
ROM_END

const tiny_rom_entry *cgp220_device::device_rom_region() const
{
	return ROM_NAME( cgp220 );
}


static INPUT_PORTS_START( cgp220 )
	PORT_START("BUTTONS")
	PORT_BIT(0x0f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Feed") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("On Line") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Reset") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("SWITCHES")
	PORT_DIPNAME(0x07, 0x00, "Ink Jet Drive Delay")
	PORT_DIPSETTING(   0x00, "0")
	PORT_DIPSETTING(   0x01, "1")
	PORT_DIPSETTING(   0x02, "2")
	PORT_DIPSETTING(   0x03, "3")
	PORT_DIPSETTING(   0x04, "4")
	PORT_DIPSETTING(   0x05, "5")
	PORT_DIPSETTING(   0x06, "6")
	PORT_DIPSETTING(   0x07, "7")
	PORT_CONFNAME(0x08, 0x00, "Paper")
	PORT_CONFSETTING(  0x00, "Loaded")
	PORT_CONFSETTING(  0x08, "Empty")
	PORT_DIPNAME(0x10, 0x00, "Serial Baud Rate")
	PORT_DIPSETTING(   0x00, "600")
	PORT_DIPSETTING(   0x10, "2400")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Pump") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

ioport_constructor cgp220_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( cgp220 );
}


void cgp220_device::mem_map(address_map &map)
{
	map.global_mask(0x3fff);
	map(0x0010, 0x0013).rw(m_pia, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0020, 0x0027).rw(m_ptm, FUNC(ptm6840_device::read), FUNC(ptm6840_device::write));
	map(0x0040, 0x0040).rw(FUNC(cgp220_device::data_r), FUNC(cgp220_device::control_w));
	map(0x0041, 0x0041).r(FUNC(cgp220_device::status_r));
	map(0x0080, 0x0080).w(FUNC(cgp220_device::head_fire_w));
	map(0x1000, 0x1fff).ram();
	map(0x2000, 0x3fff).rom().region("maincpu", 0);
}


void cgp220_device::device_add_mconfig(machine_config &config)
{
	MC6809(config, m_maincpu, 4_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &cgp220_device::mem_map);

	INPUT_MERGER_ANY_HIGH(config, "irq").output_handler().set_inputline(m_maincpu, M6809_IRQ_LINE);

	PIA6821(config, m_pia);
	m_pia->readpa_handler().set(FUNC(cgp220_device::pia_pa_r));
	m_pia->writepa_handler().set(FUNC(cgp220_device::pia_pa_w));
	m_pia->writepb_handler().set(FUNC(cgp220_device::pia_pb_w));
	m_pia->irqa_handler().set_inputline(m_maincpu, M6809_FIRQ_LINE);
	m_pia->irqb_handler().set("irq", FUNC(input_merger_device::in_w<0>));

	PTM6840(config, m_ptm, 4_MHz_XTAL / 4);
	m_ptm->irq_callback().set("irq", FUNC(input_merger_device::in_w<1>));

	BITMAP_PRINTER(config, m_bitmap_printer, PAPER_WIDTH, PAPER_HEIGHT, DPI, DPI);
	m_bitmap_printer->set_continuous_feed(true);

	// paper pixels are square, not the default 4:3 screen
	screen_device &paper = *m_bitmap_printer->subdevice<screen_device>("screen");
	paper.set_physical_aspect(paper.visible_area().width(), paper.visible_area().height());

	config.set_default_layout(layout_cgp220);
}


void cgp220_device::device_start()
{
	m_carriage_timer = timer_alloc(FUNC(cgp220_device::carriage_tick), this);

	save_item(NAME(m_rxd));
	save_item(NAME(m_strobe));
	save_item(NAME(m_data));
	save_item(NAME(m_latch));
	save_item(NAME(m_control));
	save_item(NAME(m_busy_ff));
	save_item(NAME(m_heads));
	save_item(NAME(m_motor));
	save_item(NAME(m_paper_phase));
	save_item(NAME(m_paper_steps));
	save_item(NAME(m_cr_pos));
	save_item(NAME(m_cr_vel));
	save_item(NAME(m_cr_cell));
}


void cgp220_device::device_reset()
{
	m_control = 0xfe;
	m_busy_ff = false;
	m_heads = 0;
	m_motor = 0xff;
	m_cr_vel = 0.0;

	m_pia->ca1_w(1);
	m_ptm->set_g1(1);
	m_pia->cb1_w(m_strobe);
	m_pia->cb2_w(m_rxd);

	m_carriage_timer->enable(false);
	update_head();
	update_outputs();
}


void cgp220_device::rxd_w(int state)
{
	m_rxd = state ? 1 : 0;
	m_pia->cb2_w(m_rxd);
}


void cgp220_device::strobe_w(int state)
{
	state = state ? 1 : 0;

	if (m_strobe && !state)
	{
		m_latch = m_data;
		m_busy_ff = true;
		update_outputs();
	}

	m_strobe = state;
	m_pia->cb1_w(state);
}


u8 cgp220_device::pia_pa_r()
{
	return (m_buttons->read() & 0x70) | (sensor(m_cr_cell) << 7);
}


void cgp220_device::pia_pa_w(u8 data)
{
	m_heads = data & 0x0f;
}


void cgp220_device::pia_pb_w(u8 data)
{
	if ((data ^ m_motor) & 0xf0)
		step_paper(data >> 4);

	bool const changed = (data ^ m_motor) & 0x07;
	if (changed && BIT(data ^ m_motor, 0, 2))
		LOGCARRIAGE("%s motor %c%c%c pos %d\n", machine().describe_context(), BIT(data, 2) ? 'P' : '-', BIT(data, 0) ? '-' : 'F', BIT(data, 1) ? '-' : 'R', m_cr_cell);

	m_motor = data;

	if (changed && !m_carriage_timer->enabled())
		m_carriage_timer->adjust(attotime::from_double(CR_TICK), 0, attotime::from_double(CR_TICK));
}


u8 cgp220_device::data_r()
{
	return m_latch;
}


void cgp220_device::control_w(u8 data)
{
	// BUSY flip-flop: D2 rising clocks in D1
	if (BIT(data & ~m_control, 2))
		m_busy_ff = BIT(data, 1);

	m_control = data;

	update_outputs();
}


u8 cgp220_device::status_r()
{
	return (m_switches->read() & 0x5f) | (m_serial ? 0x20 : 0x00) | (m_rxd << 7);
}


//-------------------------------------------------
//  head_fire_w - fire the selected nozzles at the
//  carriage position of the last encoder pulse
//-------------------------------------------------

void cgp220_device::head_fire_w(u8 data)
{
	m_ptm->set_g1(0);
	m_ptm->set_g1(1);

	if (!m_heads)
		return;

	m_bitmap_printer->check_new_page();

	for (int ink = 0; ink < 4; ink++)
	{
		if (!BIT(m_heads, ink))
			continue;

		// spitting into the home area misses the paper
		int const dot = m_cr_cell - 1 - ink * NOZZLE_SPACING;
		if (dot < 0 || dot >= DOTS)
			continue;

		u32 const colour = INK_COLOUR[ink];
		int const x1 = std::min(dot_x(dot + 1), PAPER_WIDTH);
		for (int y = m_paper_steps; y < m_paper_steps + DOT_ROWS; y++)
		{
			for (int x = dot_x(dot); x < x1; x++)
			{
				// inks are transparent and mix on the paper
				u32 const paper = m_bitmap_printer->get_pixel(x, y);
				u32 const mixed =
						((((paper >> 16) & 0xff) * ((colour >> 16) & 0xff) / 0xff) << 16) |
						((((paper >> 8) & 0xff) * ((colour >> 8) & 0xff) / 0xff) << 8) |
						(((paper >> 0) & 0xff) * ((colour >> 0) & 0xff) / 0xff);
				m_bitmap_printer->draw_pixel(x, y, mixed);
			}
		}
	}
}


//-------------------------------------------------
//  carriage_tick - integrate the carriage motor
//  and emit encoder pulses as it moves
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(cgp220_device::carriage_tick)
{
	// the governor drops to reduced power rather than cutting the motor
	// off, the firmware relies on that to start moving after a stale
	// encoder pulse
	bool const forward = !BIT(m_motor, 0);
	bool const reverse = !BIT(m_motor, 1);
	double const speed = BIT(m_motor, 2) ? CR_VHIGH : CR_VLOW;

	double const target = (forward == reverse) ? 0.0 : forward ? speed : -speed;
	double const tau = (target != 0.0) ? CR_TAU_ON : CR_TAU_OFF;
	double const decay = std::exp(-CR_TICK / tau);

	m_cr_pos += target * CR_TICK + (m_cr_vel - target) * tau * (1.0 - decay);
	m_cr_vel = target + (m_cr_vel - target) * decay;

	if (m_cr_pos < CR_LEFT_STOP)
	{
		m_cr_pos = CR_LEFT_STOP;
		m_cr_vel = 0.0;
	}
	else if (m_cr_pos > CR_RIGHT_STOP)
	{
		m_cr_pos = CR_RIGHT_STOP;
		m_cr_vel = 0.0;
	}

	int const cell = int(std::floor(m_cr_pos));
	if (cell != m_cr_cell)
	{
		while (cell != m_cr_cell)
		{
			m_cr_cell += (cell > m_cr_cell) ? 1 : -1;
			m_pia->ca1_w(0);
			m_pia->ca1_w(1);
	m_ptm->set_g1(1);
		}
		update_head();
	}

	// stop ticking when at rest or held against an end stop
	bool const held = (m_cr_pos == CR_LEFT_STOP && target <= 0.0) || (m_cr_pos == CR_RIGHT_STOP && target >= 0.0);
	if ((target == 0.0 && std::abs(m_cr_vel) < 1.0) || (held && m_cr_vel == 0.0))
	{
		m_cr_vel = 0.0;
		m_carriage_timer->enable(false);
	}
}


//-------------------------------------------------
//  sensor - position sensor, high over the home
//  area and the right to left start marks
//-------------------------------------------------

int cgp220_device::sensor(int pos) const
{
	if (pos < 0)
		return 1;

	for (int mark : CR_MARKS)
		if (pos >= mark && pos < mark + CR_MARK_WIDTH)
			return 1;

	return 0;
}


//-------------------------------------------------
//  step_paper - two phase on full stepping, the
//  firmware only ever feeds forward
//-------------------------------------------------

void cgp220_device::step_paper(u8 phases)
{
	static constexpr s8 PHASE_INDEX[16] = { -1, -1, -1, 1, -1, -1, 2, -1, -1, 0, -1, -1, 3, -1, -1, -1 };

	int const now = PHASE_INDEX[phases & 0x0f];
	int const last = PHASE_INDEX[m_paper_phase & 0x0f];
	m_paper_phase = phases & 0x0f;

	if (now < 0 || last < 0)
		return;

	switch ((now - last) & 3)
	{
	case 1: m_paper_steps++; break;
	case 3: m_paper_steps--; break;
	default: return;
	}

	update_head();
	m_bitmap_printer->check_new_page();
}


void cgp220_device::update_head()
{
	int const dot = std::clamp(m_cr_cell - 1, 0, DOTS);
	m_bitmap_printer->setheadpos(dot_x(dot), m_paper_steps);
}


void cgp220_device::update_outputs()
{
	if (m_serial)
		m_busy_cb(BIT(m_control, 5));
	else
		m_busy_cb(m_busy_ff || BIT(m_control, 4));
	m_ack_cb(!BIT(m_control, 0));
	m_perror_cb(!BIT(m_control, 3));
	m_fault_cb(!BIT(m_control, 4));
	m_select_cb(!BIT(m_control, 6));

	m_bitmap_printer->set_led_state(bitmap_printer_device::LED_READY, !BIT(m_control, 6));
	m_bitmap_printer->set_led_state(bitmap_printer_device::LED_ERROR, !BIT(m_control, 7));
	m_ready_led = !BIT(m_control, 6);
	m_alert_led = !BIT(m_control, 7);
}
