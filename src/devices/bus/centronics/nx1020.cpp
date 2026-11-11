// license:BSD-3-Clause
// copyright-holders:Keith Monahan
/***************************************************************************

    Star Micronics NX-1020 (Rainbow) 9-pin dot matrix printer

    A Mitsubishi M50734 (740 family) CPU at 10 MHz runs firmware v3.2 from
    IC6, a 32K EPROM at 8000-ffff.  IC7 is a 128K mask ROM with the fonts and
    system code: 04000-07fff, the upper half of its first 32K, appears directly
    in program space at 4000-7fff and holds the interrupt handlers, and the
    rest is reached as a 32K window at 8000-ffff in the data space, banked by
    the gate array.  IC5 is a 32K SRAM; the CPU's DME output (P0.5) selects
    which 16K half is visible at 0000-3fff, so each of the CPU's two spaces
    sees one half.  The electronic DIP switch settings are kept in a BR93C46
    serial EEPROM (IC9) bit-banged on port P1.

    The NEC D65006 gate array sits at 4000-4003 of the data space, which the
    firmware only reaches through indirect addressing (lda/sta [$ae],y with
    $ae:$af = $4000).  It latches the print head wires, holds the panel
    lamps, banks the font ROM window and contains the Centronics receiver;
    BUSY comes from its byte-waiting flag, not from the M50734's BUSYout.

    The print head solenoids double as the beeper.  Color printing (ribbon
    shift) and the print head thermistor are not emulated.

***************************************************************************/

#include "emu.h"
#include "nx1020.h"

#include "cpu/m6502/m50734.h"
#include "machine/bitmap_printer.h"
#include "machine/eepromser.h"
#include "machine/steppers.h"
#include "sound/beep.h"

#include "screen.h"
#include "speaker.h"

#include "nx1020.lh"

#define LOG_GA      (1U << 1)
#define LOG_PORT    (1U << 2)
#define LOG_HEAD    (1U << 3)
#define LOG_HOST    (1U << 4)   // Centronics host interface

#define VERBOSE (0)
#include "logmacro.h"

#define LOGGA(...)   LOGMASKED(LOG_GA, __VA_ARGS__)
#define LOGPORT(...) LOGMASKED(LOG_PORT, __VA_ARGS__)
#define LOGHEAD(...) LOGMASKED(LOG_HEAD, __VA_ARGS__)
#define LOGHOST(...) LOGMASKED(LOG_HOST, __VA_ARGS__)


namespace {

// one carriage step is one 240 dpi dot; one paper feed step is 1/432 in,
// half a 216 dpi row
constexpr int PAPER_HDPI = 240;
constexpr int PAPER_VDPI = 216;
constexpr int PAPER_WIDTH = PAPER_HDPI * 17 / 2;   // 8.5 in
constexpr int PAPER_HEIGHT = PAPER_VDPI * 11;      // fanfold, one 11 in page per lap
constexpr int WIRE_PITCH = PAPER_VDPI / 72;        // 1/72 in, wire 1 at the top
constexpr int PF_STEPS_PER_ROW = 2;                // 1/432 in feed motor steps

// position of each phase pattern in the 1-2 phase sequence 9,1,3,2,6,4,c,8, so a
// 2-2 phase step is a difference of two; -1 for patterns outside the sequence
constexpr s8 PHASE_INDEX[16] =
{
	-1, 1, 3, 2, 5, -1, 4, -1,
	 7, 0, -1, -1, 6, -1, -1, -1
};


class star_nx1020_device : public device_t, public device_centronics_peripheral_interface
{
public:
	// color printing is not emulated
	static constexpr feature_type imperfect_features() { return feature::PRINTER; }

	star_nx1020_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_reset_after_children() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device_centronics_peripheral_interface implementation
	virtual void input_strobe(int state) override;
	virtual void input_data0(int state) override { host_data_w(0, state); }
	virtual void input_data1(int state) override { host_data_w(1, state); }
	virtual void input_data2(int state) override { host_data_w(2, state); }
	virtual void input_data3(int state) override { host_data_w(3, state); }
	virtual void input_data4(int state) override { host_data_w(4, state); }
	virtual void input_data5(int state) override { host_data_w(5, state); }
	virtual void input_data6(int state) override { host_data_w(6, state); }
	virtual void input_data7(int state) override { host_data_w(7, state); }
	virtual void input_init(int state) override;
	// AUTOFEED and SELECT IN are not connected

private:
	// Home position sensor: a photo-interrupter worked by a shield plate on the
	// carriage, in carriage steps from the left stop.  The plate is not
	// documented: solid out to SHIELD_RIGHT, with two slots.
	static constexpr s32 SHIELD_RIGHT = 390;
	static constexpr s32 SLOT_OUT_L = 366;
	static constexpr s32 SLOT_OUT_R = 374;
	static constexpr s32 SLOT_IN_L = 342;
	static constexpr s32 SLOT_IN_R = 350;

	static constexpr s32 CR_REST = 550;       // power-on carriage position, clear of the sensor
	static constexpr s32 CR_TRAVEL = 2700;    // right end stop
	// the 8 in print width (1920 carriage steps from 392) centered on 8.5 in paper
	static constexpr s32 CR_DOT_ORIGIN = 392 - 60;

	// The firmware times the head strobe with Timer S at 1.6 us per count:
	// about 330 us to print, 390 us dark, 190 us for the power-on head test
	// and 60 us for the beeper.  Only the print pulses mark the paper.
	static constexpr attotime BEEP_MAX_PULSE = attotime::from_usec(154);
	static constexpr attotime INK_MIN_PULSE = attotime::from_usec(256);
	// a beep is a train of those pulses every 490 us
	static constexpr u32 BEEP_HZ = 2040;
	static constexpr attotime BEEP_HOLD = attotime::from_msec(2);

	void prog_map(address_map &map) ATTR_COLD;
	void data_map(address_map &map) ATTR_COLD;

	u8 status_r();
	void head_data_w(u8 data);
	u8 host_data_r();
	void pitch_font_lamps_w(u8 data);
	u8 control_r();
	void control_w(u8 data);
	u8 lamps_r();
	void lamps_w(u8 data);

	u8 p0_r();
	void p0_w(u8 data);
	u8 p1_r();
	void p1_w(u8 data);
	void p2_w(u8 data);
	u8 p3_r();
	void p3_w(u8 data);

	void host_data_w(int bit, int state);
	TIMER_CALLBACK_MEMBER(strobe_w);
	TIMER_CALLBACK_MEMBER(init_w);
	void update_host_lines(bool force = false);

	void step_carriage(u8 phase);
	void step_paper(u8 phase);
	void head_strobe_start();
	void fire_head();
	void update_head_position();
	int head_column() const { return m_cr_pos - CR_DOT_ORIGIN; }
	TIMER_CALLBACK_MEMBER(beep_off);
	void update_lamps();

	static bool home_flag(s32 pos)
	{
		// blocked left of the plate's right edge, except in its two slots
		return pos < SHIELD_RIGHT
				&& !(pos >= SLOT_OUT_L && pos < SLOT_OUT_R)
				&& !(pos >= SLOT_IN_L && pos < SLOT_IN_R);
	}

	required_device<m50734_device> m_maincpu;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<bitmap_printer_device> m_printer;
	required_device<stepper_device> m_pf_stepper;
	required_device<beep_device> m_beeper;
	required_region_ptr<u8> m_fontrom;
	required_region_ptr<u8> m_firmware;
	memory_bank_creator m_font_bank;
	required_ioport m_paper;
	required_ioport m_panel;
	output_finder<11> m_lamps;    // reg1 bits 0-7, then reg3 bits 0-2

	emu_timer *m_beep_off_timer;

	// gate array
	u8 m_pitch_font_lamps;  // reg1
	u8 m_control;           // reg2
	u8 m_lamp_reg;          // reg3: POWER, ON LINE, QUIET
	u16 m_head_data;        // print head latch

	u8 m_p0_out;
	u8 m_p1_out;
	u8 m_p2_out;
	u8 m_p3_out;

	// head strobe in progress (P0.4 low): when it began, and the wires and dot it fires
	bool m_fire_pending;
	attotime m_fire_time;
	u16 m_fire_data;
	s32 m_fire_x;
	s32 m_fire_y;

	// Centronics interface (in the gate array)
	u8 m_host_data;         // data pins
	u8 m_host_latch;        // byte latched by /STROBE
	bool m_ia7;             // gate array reg0 bit 7: byte waiting
	u8 m_strobe_in;
	u8 m_init_in;           // also read on P0.1
	u8 m_lines_out;         // BUSY, /ACK, /FAULT, SELECT, PE as last driven, bits 0-4

	// mechanism
	s32 m_cr_pos;           // carriage steps from the left stop
	bool m_pf_synced;       // feed stepper phase follows the coils
};


star_nx1020_device::star_nx1020_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, STAR_NX1020, tag, owner, clock),
	device_centronics_peripheral_interface(mconfig, *this),
	m_maincpu(*this, "maincpu"),
	m_eeprom(*this, "eeprom"),
	m_printer(*this, "printer"),
	m_pf_stepper(*this, "printer:pf_stepper"),
	m_beeper(*this, "beeper"),
	m_fontrom(*this, "fontrom"),
	m_firmware(*this, "maincpu"),
	m_font_bank(*this, "font_bank"),
	m_paper(*this, "PAPER"),
	m_panel(*this, "PANEL"),
	m_lamps(*this, "lamp%u", 0U),
	m_p2_out(0),
	m_fire_time(attotime::zero),
	m_fire_data(0),
	m_fire_x(0),
	m_fire_y(0),
	m_host_data(0),
	m_strobe_in(1),
	m_init_in(1),
	m_cr_pos(CR_REST)
{
}


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void star_nx1020_device::prog_map(address_map &map)
{
	// IC5 SRAM, upper half: A14 is /DME (R79 pull-up), so zero page, the
	// stack and all absolute accesses land here
	map(0x0000, 0x3fff).ram();
	// IC7 04000-07fff: system code and interrupt handlers; writes are ignored
	map(0x4000, 0x7fff).rom().region(m_fontrom, 0x4000).nopw();
	map(0x8000, 0xffff).rom().region(m_firmware, 0);
}

void star_nx1020_device::data_map(address_map &map)
{
	// IC5 SRAM, lower half (host receive buffer, line records)
	map(0x0000, 0x3fff).ram();
	// D65006 gate array
	map(0x4000, 0x4000).rw(FUNC(star_nx1020_device::status_r), FUNC(star_nx1020_device::head_data_w));
	map(0x4001, 0x4001).rw(FUNC(star_nx1020_device::host_data_r), FUNC(star_nx1020_device::pitch_font_lamps_w));
	map(0x4002, 0x4002).rw(FUNC(star_nx1020_device::control_r), FUNC(star_nx1020_device::control_w));
	map(0x4003, 0x4003).rw(FUNC(star_nx1020_device::lamps_r), FUNC(star_nx1020_device::lamps_w));
	map(0x4004, 0x7fff).rom().region(m_fontrom, 0x4004);
	// banked window onto the font ROM (control bits 7-6)
	map(0x8000, 0xffff).bankr(m_font_bank);
}


//**************************************************************************
//  GATE ARRAY
//**************************************************************************

// reg0 read: bit 7 (IA7) is set while a host byte is waiting
u8 star_nx1020_device::status_r()
{
	u8 const data = m_ia7 ? 0x80 : 0x00;
	if (!machine().side_effects_disabled())
		LOGGA("%s: status read -> %02x\n", machine().describe_context(), data);
	return data;
}

// reg0 write: print head shift register, two writes per column, wire 9 in
// bit 7 of the first, wires 1-8 in bits 7-0 of the second
void star_nx1020_device::head_data_w(u8 data)
{
	m_head_data = (m_head_data << 8) | data;
	LOGHEAD("%s: print head data %02x (latch %04x)\n", machine().describe_context(), data, m_head_data);
}

// reg1 read: the host byte; reading it does not clear IA7, /ACK does
u8 star_nx1020_device::host_data_r()
{
	if (!machine().side_effects_disabled())
		LOGGA("%s: host data read -> %02x\n", machine().describe_context(), m_host_latch);
	return m_host_latch;
}

// reg1 write: panel lamps 10CPI, 12CPI, PROP, COND, HS DRAFT, DRAFT, COURIER, SCRIPT
void star_nx1020_device::pitch_font_lamps_w(u8 data)
{
	LOGGA("%s: pitch/font lamps <- %02x\n", machine().describe_context(), data);
	m_pitch_font_lamps = data;
	update_lamps();
}

u8 star_nx1020_device::control_r()
{
	return m_control;
}

// reg2 write: bit 0 interface enable, bit 1 strobe edge (EDS B-4: 0 falling,
// 1 rising), bit 2 /ACK, bit 3 fault, bit 4 SELECT, bit 5 paper empty,
// bits 7-6 font window bank
void star_nx1020_device::control_w(u8 data)
{
	LOGGA("%s: control <- %02x\n", machine().describe_context(), data);
	u8 const prev = m_control;
	m_control = data;

	m_font_bank->set_entry(BIT(data, 6, 2));

	if (BIT(data, 2) && !BIT(prev, 2))
	{
		// /ACK clears the byte-waiting flag
		m_ia7 = false;
		LOGHOST("%s: /ACK\n", machine().describe_context());
	}
	update_host_lines();
}

// reg3 read: the write latch (the firmware reads it back to toggle single lamps)
u8 star_nx1020_device::lamps_r()
{
	return m_lamp_reg;
}

// reg3 write: bits 0-2 POWER, ON LINE and QUIET lamps.  Fatal error traps
// blink them by XORing an error code in here.
void star_nx1020_device::lamps_w(u8 data)
{
	LOGGA("%s: lamps <- %02x\n", machine().describe_context(), data);
	m_lamp_reg = data;
	update_lamps();
}

void star_nx1020_device::update_lamps()
{
	for (int i = 0; i < 8; i++)
		m_lamps[i] = BIT(m_pitch_font_lamps, i);
	// bit 0 switches the POWER lamp off, which is how it blinks for paper out
	m_lamps[8] = !BIT(m_lamp_reg, 0);
	m_lamps[9] = BIT(m_lamp_reg, 1);
	m_lamps[10] = BIT(m_lamp_reg, 2);
}


//**************************************************************************
//  CENTRONICS HOST INTERFACE
//
//  In the D65006.  The firmware polls reg0 bit 7 (IA7) for a byte, reads
//  it from reg1 and pulses /ACK with reg2 bit 2, which clears IA7.  Values
//  passed to output_x()/input_x() are pin levels.
//**************************************************************************

void star_nx1020_device::host_data_w(int bit, int state)
{
	if (state)
		m_host_data |= u8(1U << bit);
	else
		m_host_data &= u8(~(1U << bit));
}

void star_nx1020_device::update_host_lines(bool force)
{
	// BUSY while a byte is waiting, the interface is disabled (reg2 bit 0) or
	// the printer is off line or faulted (reg2 bit 3)
	u8 const reg2 = m_control;
	bool const busy = m_ia7 || !BIT(reg2, 0) || BIT(reg2, 3);
	u8 const lines = (busy ? 0x01 : 0x00)
			| (BIT(reg2, 2) ? 0x00 : 0x02)      // /ACK
			| (BIT(reg2, 3) ? 0x00 : 0x04)      // /FAULT
			| (BIT(reg2, 4) ? 0x08 : 0x00)      // SELECT
			| (BIT(reg2, 5) ? 0x10 : 0x00);     // PE
	u8 const changed = force ? 0x1f : (lines ^ m_lines_out);
	m_lines_out = lines;

	if (BIT(changed, 0))
	{
		LOGHOST("BUSY %d (ia7 %d reg2 %02x)\n", busy, m_ia7, reg2);
		output_busy(BIT(lines, 0));
	}
	if (BIT(changed, 1))
		output_ack(BIT(lines, 1));
	if (BIT(changed, 2))
		output_fault(BIT(lines, 2));
	if (BIT(changed, 3))
		output_select(BIT(lines, 3));
	if (BIT(changed, 4))
		output_perror(BIT(lines, 4));
}

void star_nx1020_device::input_strobe(int state)
{
	// the host drives /STROBE from its own timeslice: take the level and the
	// data pins now, latch in the printer's time
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(star_nx1020_device::strobe_w), this), (state ? 0x100 : 0) | m_host_data);
}

TIMER_CALLBACK_MEMBER(star_nx1020_device::strobe_w)
{
	// latched on the edge EDS B-4 selects (reg2 bit 1: 0 falling, 1 rising);
	// a byte strobed while BUSY is dropped
	u8 const level = BIT(param, 8);
	bool const latch = (level != m_strobe_in) && (level == BIT(m_control, 1));
	m_strobe_in = level;
	if (latch)
	{
		if (!BIT(m_lines_out, 0))
		{
			m_host_latch = u8(param);
			m_ia7 = true;
			LOGHOST("host byte %02x latched\n", m_host_latch);
			update_host_lines();
		}
		else
		{
			LOGHOST("host byte %02x dropped, BUSY\n", u8(param));
		}
	}
}

void star_nx1020_device::input_init(int state)
{
	// from the host's timeslice, as /STROBE
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(star_nx1020_device::init_w), this), state ? 1 : 0);
}

TIMER_CALLBACK_MEMBER(star_nx1020_device::init_w)
{
	u8 const level = u8(param);
	if (level == m_init_in)
		return;

	m_init_in = level;
	LOGHOST("/INIT %d\n", level);

	// /INIT is _INT2 (P0.1); the handler waits for it to go high, then restarts
	m_maincpu->set_input_line(m50734_device::M50734_INT2_LINE, level ? CLEAR_LINE : ASSERT_LINE);
}


//**************************************************************************
//  PORTS
//**************************************************************************

u8 star_nx1020_device::p0_r()
{
	// release lever (0) and tractor (2) read as fanfold on the push tractor;
	// paper out (6, high = out)
	u8 data = m_paper->read() & 0x40;

	// /INIT
	if (m_init_in)
		data |= 0x02;

	if (home_flag(m_cr_pos))
		data |= 0x80;                       // P0.7 HP: high = carriage at home

	return data;
}

void star_nx1020_device::p0_w(u8 data)
{
	// P0.4 is the head strobe, low for the solenoid on-time (Timer S)
	if (BIT(m_p0_out, 4) && !BIT(data, 4))
		head_strobe_start();
	else if (!BIT(m_p0_out, 4) && BIT(data, 4))
		fire_head();

	m_p0_out = data;
}

u8 star_nx1020_device::p1_r()
{
	// P1.4 = BR93C46 DO
	return (m_p1_out & 0x07) | (m_eeprom->do_read() ? 0x10 : 0x00);
}

void star_nx1020_device::p1_w(u8 data)
{
	m_eeprom->cs_write(BIT(data, 0));
	m_eeprom->di_write(BIT(data, 2));
	m_eeprom->clk_write(BIT(data, 1));

	m_p1_out = data;
}

void star_nx1020_device::p2_w(u8 data)
{
	// four phase outputs: P2.0-3 carriage, P2.4-7 line feed
	if ((data ^ m_p2_out) & 0x0f)
		step_carriage(data & 0x0f);
	if ((data ^ m_p2_out) & 0xf0)
		step_paper(data >> 4);

	m_p2_out = data;
}

u8 star_nx1020_device::p3_r()
{
	// panel buttons; held at power on they select the self tests and settings
	// modes
	return (m_p3_out & 0x07) | (m_panel->read() & 0xf8);
}

void star_nx1020_device::p3_w(u8 data)
{
	m_p3_out = data;
}


//**************************************************************************
//  MECHANISM
//
//  The carriage position is counted here, for the end stops and the home
//  sensor; the paper feed is bitmap_printer's stepper.
//**************************************************************************

void star_nx1020_device::step_carriage(u8 phase)
{
	s8 const to = PHASE_INDEX[phase];
	s8 const from = PHASE_INDEX[m_p2_out & 0x0f];

	if (to < 0 || from < 0)
		return;

	bool const was_home = home_flag(m_cr_pos);

	// one step forward is +1 in 1-2 phase mode and +2 in 2-2 phase mode
	s32 delta = (to - from) & 7;
	if (delta > 4)
		delta -= 8;
	m_cr_pos += delta;

	// mechanical end stops
	if (m_cr_pos < 0)
		m_cr_pos = 0;
	else if (m_cr_pos > CR_TRAVEL)
		m_cr_pos = CR_TRAVEL;

	bool const home = home_flag(m_cr_pos);
	if (home != was_home)
		LOGPORT("carriage %s home at step %d\n", home ? "entered" : "left", m_cr_pos);

	update_head_position();
}

void star_nx1020_device::step_paper(u8 phase)
{
	// the same 1-2 phase sequence in stepper_device's coil order; a half step
	// is one paper feed step
	u8 const pattern = bitswap<4>(phase, 3, 1, 0, 2);

	// the stepper restarts its phase on reset but the rotor stays put, so the
	// first energized pattern only sets the phase
	if (!m_pf_synced && PHASE_INDEX[phase] >= 0)
	{
		s32 const pos = m_pf_stepper->get_absolute_position();
		m_pf_stepper->update(pattern);
		m_pf_stepper->set_absolute_position(pos);
		m_pf_synced = true;
		return;
	}

	m_printer->update_pf_stepper(pattern);
}

void star_nx1020_device::update_head_position()
{
	m_printer->setheadpos(head_column(), m_printer->m_ypos);
}

// P0.4 falling: the gate array starts driving the latched wires
void star_nx1020_device::head_strobe_start()
{
	m_fire_pending = true;
	m_fire_time = machine().time();
	m_fire_data = m_head_data;
	m_fire_x = head_column();
	m_fire_y = m_printer->m_ypos;
}

// P0.4 rising: the strobe's length decides what the solenoids did
void star_nx1020_device::fire_head()
{
	if (!m_fire_pending)
		return;
	m_fire_pending = false;

	attotime const width = machine().time() - m_fire_time;

	LOGHEAD("%s: head fired %04x for %s at carriage %d (dot %d), paper row %d\n",
			machine().describe_context(), m_fire_data, width.to_string(), m_cr_pos, m_fire_x, m_fire_y);

	if (m_fire_data == 0)
		return;

	// pulses too short to mark the paper sound the beeper
	if (width < BEEP_MAX_PULSE)
	{
		m_beeper->set_state(1);
		m_beep_off_timer->adjust(BEEP_HOLD);
		return;
	}

	if (width < INK_MIN_PULSE || m_fire_x < 0 || m_fire_x >= PAPER_WIDTH)
		return;

	m_printer->check_new_page();
	for (int wire = 1; wire <= 9; wire++)
	{
		// wire 1 (top) is bit 7, wire 9 (bottom) is bit 15
		bool const fired = (wire == 9) ? BIT(m_fire_data, 15) : BIT(m_fire_data, 8 - wire);
		if (!fired)
			continue;

		// a dot is about 0.35 mm, 3x3 pixels
		int const wy = m_fire_y + (wire - 1) * WIRE_PITCH;
		for (int dy = 0; dy < 3; dy++)
		{
			for (int dx = 0; dx < 3 && m_fire_x + dx < PAPER_WIDTH; dx++)
				m_printer->draw_pixel(m_fire_x + dx, wy + dy, 0x000000);
		}
	}
}

TIMER_CALLBACK_MEMBER(star_nx1020_device::beep_off)
{
	m_beeper->set_state(0);
}


//**************************************************************************
//  DEVICE
//**************************************************************************

void star_nx1020_device::device_start()
{
	m_beep_off_timer = timer_alloc(FUNC(star_nx1020_device::beep_off), this);

	// control bits 7-6 drive MA15 and /MA16, so the ROM quarter is two ahead of the code:
	//   0 -> 10000 font descriptors and metrics
	//   1 -> 18000 draft glyphs
	//   2 -> 00000 NLQ glyphs
	//   3 -> 08000 v3.0 system code, no font data
	for (int i = 0; i < 4; i++)
		m_font_bank->configure_entry(i, &m_fontrom[((i + 2) & 3) << 15]);

	save_item(NAME(m_pitch_font_lamps));
	save_item(NAME(m_control));
	save_item(NAME(m_lamp_reg));
	save_item(NAME(m_head_data));
	save_item(NAME(m_p0_out));
	save_item(NAME(m_p1_out));
	save_item(NAME(m_p2_out));
	save_item(NAME(m_p3_out));
	save_item(NAME(m_fire_pending));
	save_item(NAME(m_fire_time));
	save_item(NAME(m_fire_data));
	save_item(NAME(m_fire_x));
	save_item(NAME(m_fire_y));
	save_item(NAME(m_host_data));
	save_item(NAME(m_host_latch));
	save_item(NAME(m_ia7));
	save_item(NAME(m_strobe_in));
	save_item(NAME(m_init_in));
	save_item(NAME(m_lines_out));
	save_item(NAME(m_cr_pos));
	save_item(NAME(m_pf_synced));
}

// the gate array and port latches restart with the CPU; the carriage and the
// host's input levels are kept
void star_nx1020_device::device_reset()
{
	m_pitch_font_lamps = 0;
	m_control = 0;
	m_lamp_reg = 0;
	m_font_bank->set_entry(0);
	update_lamps();
	m_head_data = 0;
	m_host_latch = 0;
	m_ia7 = false;
	m_p0_out = 0; // as the CPU drives it on reset
	m_p1_out = 0;
	m_p3_out = 0;
	m_fire_pending = false;
	m_beep_off_timer->adjust(attotime::never);
	m_beeper->set_state(0);

	update_host_lines(true);
}

void star_nx1020_device::device_reset_after_children()
{
	// after bitmap_printer has reset its head position: start the feed motor
	// at the top margin
	m_pf_stepper->set_absolute_position(m_printer->get_top_margin() * PF_STEPS_PER_ROW);
	m_pf_synced = false;
	update_head_position();
}


INPUT_PORTS_START( star_nx1020 )
	PORT_START("PAPER")
	PORT_CONFNAME(0x40, 0x00, "Paper")
	PORT_CONFSETTING(0x00, "Loaded")
	PORT_CONFSETTING(0x40, "Out of paper")

	// clickable in the layout; no default keys, so a host's keys never press them.
	// Held at power on: ON LINE short self test, PAPER FEED long self test,
	// SET print area test, PAPER FEED + SET hexadecimal dump.
	PORT_START("PANEL")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("FONT")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("PITCH")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("SET/EJECT/PARK")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("PAPER FEED")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("ON LINE")
INPUT_PORTS_END

ioport_constructor star_nx1020_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(star_nx1020);
}


void star_nx1020_device::device_add_mconfig(machine_config &config)
{
	M50734(config, m_maincpu, 10_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &star_nx1020_device::prog_map);
	m_maincpu->set_addrmap(AS_DATA, &star_nx1020_device::data_map);
	m_maincpu->p0_in_cb().set(FUNC(star_nx1020_device::p0_r));
	m_maincpu->p0_out_cb().set(FUNC(star_nx1020_device::p0_w));
	m_maincpu->p1_in_cb().set(FUNC(star_nx1020_device::p1_r));
	m_maincpu->p1_out_cb().set(FUNC(star_nx1020_device::p1_w));
	m_maincpu->p2_out_cb().set(FUNC(star_nx1020_device::p2_w));
	m_maincpu->p3_in_cb().set(FUNC(star_nx1020_device::p3_r));
	m_maincpu->p3_out_cb().set(FUNC(star_nx1020_device::p3_w));
	// print head thermistor (>= d1 throttles printing, >= fc or < 04 is a sensor fault)
	m_maincpu->an3_in_cb().set_constant(0x60);

	EEPROM_93C46_16BIT(config, m_eeprom);

	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beeper, BEEP_HZ);
	m_beeper->add_route(ALL_OUTPUTS, "mono", 0.25);

	BITMAP_PRINTER(config, m_printer, PAPER_WIDTH, PAPER_HEIGHT, PAPER_HDPI, PAPER_VDPI);
	m_printer->set_continuous_feed(true);
	m_printer->set_pf_stepper_ratio(1, PF_STEPS_PER_ROW);

	// show the paper at its physical shape (240 x 216 dpi pixels), not 4:3
	screen_device &paper = *m_printer->subdevice<screen_device>("screen");
	paper.set_physical_aspect(paper.visible_area().width() * PAPER_VDPI, paper.visible_area().height() * PAPER_HDPI);

	config.set_default_layout(layout_nx1020);
}


ROM_START( star_nx1020 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "zx_3.2.ic6", 0x0000, 0x8000, CRC(294e1027) SHA1(be12cf6468e190a713e7b3c42ad8812658385cb0) ) // "*** VER 3.2 ***"

	ROM_REGION( 0x20000, "fontrom", 0 )
	ROM_LOAD( "08223072.ic7", 0x00000, 0x20000, CRC(6f77f09f) SHA1(9eb97242659674e6c5cdda976750272327213696) ) // "*** VER 3.0 ***"
ROM_END

const tiny_rom_entry *star_nx1020_device::device_rom_region() const
{
	return ROM_NAME(star_nx1020);
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(STAR_NX1020, device_centronics_peripheral_interface, star_nx1020_device, "star_nx1020", "Star NX-1020 Dot Matrix Printer")
