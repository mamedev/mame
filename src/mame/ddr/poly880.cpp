// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

VEB Polytechnik Poly-Computer 880

http://www.kc85-museum.de/books/poly880/index.html

Initially the screen is blank. The CTC causes a NMI, this autoboots the system,
and then the PIO releases the NMI line.

Pasting:
    0-F : as is
    EXEC : X
    BACK : K
    MEM : M
    GO : G

Test Paste:
    M4000X11X22X33X44X55X66X77X88X99XM4000
    Now press X to confirm the data has been entered.


The SC1 version is a modification that turns it into a chesscomputer.
Not to be confused with the prequel to SC2, but more likely a different
version of SLC1, without the "Lern" part. Just like SLC1, the chess engine
was copied from Fidelity's Sensory Chess Challenger 8.

SC1-SLC1 Keypad Reference:
    1-8 = A1-H8
    C = C (back)
    D = O (option)
    E = St (clear)
    F = Z (enter)


TODO:
- STEP stays on the first user instruction: the CTC SEND pulse arrives a few
  clocks early (no timer start latency in z80ctc, NMI sampled at instruction
  boundary instead of the last T state)
- who made poly880s? slc1 is very similar, it's by the same person?

****************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/z80daisy.h"
#include "imagedev/cassette.h"
#include "machine/input_merger.h"
#include "machine/ram.h"
#include "machine/z80pio.h"
#include "machine/z80ctc.h"
#include "video/pwm.h"

#include "speaker.h"

// internal artwork
#include "poly880.lh"


namespace {

class poly880_state : public driver_device
{
public:
	poly880_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pio(*this, "pio%u", 0)
		, m_ctc(*this, "ctc")
		, m_display(*this, "display")
		, m_busled(*this, "busled")
		, m_cassette(*this, "cassette")
		, m_ram(*this, RAM_TAG)
		, m_rom(*this, "maincpu")
		, m_inputs(*this, "IN.%u", 0U)
		, m_special(*this, "SPECIAL")
		, m_addr_led(*this, "a%u", 0U)
		, m_data_led(*this, "d%u", 0U)
		, m_ctrl_led(*this, "c%u", 0U)
	{ }

	void poly880(machine_config &config);
	void poly880s(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(trigger_reset);
	DECLARE_INPUT_CHANGED_MEMBER(step_keys);
	DECLARE_INPUT_CHANGED_MEMBER(cycl_pressed);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	enum : u8
	{
		BUS_M1 = 0x01,
		BUS_MREQ = 0x02,
		BUS_IORQ = 0x04,
		BUS_RD = 0x08,
		BUS_WR = 0x10
	};

	required_device<z80_device> m_maincpu;
	required_device_array<z80pio_device, 2> m_pio;
	required_device<z80ctc_device> m_ctc;
	required_device<pwm_display_device> m_display;
	required_device<pwm_display_device> m_busled;
	required_device<cassette_image_device> m_cassette;
	required_device<ram_device> m_ram;
	required_region_ptr<u8> m_rom;
	required_ioport_array<3> m_inputs;
	required_ioport m_special;
	output_finder<16> m_addr_led;
	output_finder<8> m_data_led;
	output_finder<10> m_ctrl_led;

	emu_timer *m_cycl_timer = nullptr;

	u8 m_matrix = 0;
	bool m_send = false;
	bool m_scon = true;
	bool m_send_ff = false;
	bool m_nmi_ff = false;
	bool m_icycl = true;
	bool m_cycl_pulse = false;
	bool m_wait = false;
	bool m_halt = false;
	bool m_int = false;
	u16 m_bus_addr = 0;
	u8 m_bus_data = 0;
	u8 m_bus_ctrl = 0;
	u16 m_bus_sample = 1;

	void poly880_io(address_map &map) ATTR_COLD;
	void poly880_mem(address_map &map) ATTR_COLD;
	void poly880_opcodes(address_map &map) ATTR_COLD;
	void poly880s_mem(address_map &map) ATTR_COLD;
	void poly880s_opcodes(address_map &map) ATTR_COLD;

	template <bool M1, u16 RomMask> u8 mem_r(address_space &space, offs_t offset);
	void mem_w(offs_t offset, u8 data);
	u8 io_read(address_space &space, offs_t offset);
	u8 io_r(address_space &space, offs_t offset);
	void io_w(offs_t offset, u8 data);
	bool bus_cycle(u16 addr, u8 data, u8 ctrl);
	void update_busleds();
	void busled_w(offs_t offset, u8 data);
	void update_step();
	TIMER_CALLBACK_MEMBER(cycl_end);

	void cldig_w(u8 data);
	void int_w(int state);
	void halt_w(int state);
	void ctc_z0_w(int state);
	void pio1_pa_w(u8 data);
	u8 pio1_pb_r();
	void pio1_pb_w(u8 data);
};


// Read/Write Handlers

void poly880_state::cldig_w(u8 data)
{
	m_display->write_my(data);
	m_matrix = data;
}

inline ATTR_FORCE_INLINE bool poly880_state::bus_cycle(u16 addr, u8 data, u8 ctrl)
{
	if (machine().side_effects_disabled() || (!m_wait && --m_bus_sample))
		return false;

	m_bus_addr = addr;
	m_bus_data = data;
	m_bus_ctrl = ctrl;
	update_busleds();

	if (m_wait)
	{
		m_maincpu->defer_access();
		return true;
	}

	// sample at a prime interval so program loops don't alias the averaged LED brightness
	m_bus_sample = 509;

	return false;
}

template <bool M1, u16 RomMask>
u8 poly880_state::mem_r(address_space &space, offs_t offset)
{
	u8 data = space.unmap();

	if (!BIT(offset, 15))
		data = BIT(offset, 14) ? m_ram->pointer()[offset & 0x3ff] : m_rom[offset & RomMask];
	else if (m_ram->size() > 0x400)
		data = m_ram->pointer()[0x400 | (offset & 0x7fff)];

	bus_cycle(offset, data, (M1 ? BUS_M1 : 0) | BUS_MREQ | BUS_RD);

	return data;
}

void poly880_state::mem_w(offs_t offset, u8 data)
{
	if (bus_cycle(offset, data, BUS_MREQ | BUS_WR))
		return;

	if (!BIT(offset, 15))
	{
		if (BIT(offset, 14))
			m_ram->pointer()[offset & 0x3ff] = data;
	}
	else if (m_ram->size() > 0x400)
		m_ram->pointer()[0x400 | (offset & 0x7fff)] = data;
}

void poly880_state::poly880_mem(address_map &map)
{
	map(0x0000, 0xffff).rw(NAME((&poly880_state::mem_r<false, 0x33ff>)), FUNC(poly880_state::mem_w));
}

void poly880_state::poly880_opcodes(address_map &map)
{
	map(0x0000, 0xffff).r(NAME((&poly880_state::mem_r<true, 0x33ff>)));
}

void poly880_state::poly880s_mem(address_map &map)
{
	map(0x0000, 0xffff).rw(NAME((&poly880_state::mem_r<false, 0x3fff>)), FUNC(poly880_state::mem_w));
}

void poly880_state::poly880s_opcodes(address_map &map)
{
	map(0x0000, 0xffff).r(NAME((&poly880_state::mem_r<true, 0x3fff>)));
}

void poly880_state::poly880_io(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(poly880_state::io_r), FUNC(poly880_state::io_w));
}

u8 poly880_state::io_read(address_space &space, offs_t offset)
{
	if (BIT(offset, 7))
	{
		switch (BIT(offset, 2, 2))
		{
		case 0: return m_pio[0]->read_alt(offset & 3);
		case 1: return m_pio[1]->read_alt(offset & 3);
		case 2: return m_ctc->read(offset & 3);
		}
	}

	return space.unmap();
}

u8 poly880_state::io_r(address_space &space, offs_t offset)
{
	u8 data;

	if (m_wait && !machine().side_effects_disabled())
	{
		auto dis = machine().disable_side_effects();
		data = io_read(space, offset);
	}
	else
		data = io_read(space, offset);

	bus_cycle(offset, data, BUS_IORQ | BUS_RD);

	return data;
}

void poly880_state::io_w(offs_t offset, u8 data)
{
	if (bus_cycle(offset, data, BUS_IORQ | BUS_WR))
		return;

	if (BIT(offset, 7))
	{
		switch (BIT(offset, 2, 2))
		{
		case 0: m_pio[0]->write_alt(offset & 3, data); break;
		case 1: m_pio[1]->write_alt(offset & 3, data); break;
		case 2: m_ctc->write(offset & 3, data); break;
		}

		if (BIT(offset, 5))
			cldig_w(data);
	}
}

void poly880_state::update_busleds()
{
	u16 const ctrl = m_bus_ctrl
			| (m_halt ? 0x020 : 0)
			| (m_int ? 0x040 : 0)
			| (m_nmi_ff ? 0x080 : 0)
			| (m_wait ? 0x100 : 0)
			| (m_icycl ? 0 : 0x200);

	m_busled->write_row(0, m_bus_addr | (u64(m_bus_data) << 16) | (u64(ctrl) << 24));
}

void poly880_state::busled_w(offs_t offset, u8 data)
{
	u8 const x = offset >> 6;

	if (x < 16)
		m_addr_led[x] = data;
	else if (x < 24)
		m_data_led[x - 16] = data;
	else
		m_ctrl_led[x - 24] = data;
}


// Single Step Logic

void poly880_state::update_step()
{
	ioport_value const special = m_special->read();
	bool const reset = BIT(special, 0);
	bool const mon = BIT(special, 1);
	bool const mcycl = BIT(special, 2);

	if (reset)
		m_send_ff = m_nmi_ff = false;

	bool x = m_icycl;
	bool preset = false;
	bool y = false;

	for (;;)
	{
		preset = mon || (m_send_ff && x);
		y = !(x && !mcycl);

		bool const next = preset || reset || !y;

		if (next == x)
			break;

		x = next;
	}

	m_icycl = x;

	if (preset && !reset)
		m_nmi_ff = true;

	m_maincpu->set_input_line(INPUT_LINE_NMI, m_nmi_ff ? ASSERT_LINE : CLEAR_LINE);

	bool const wait = m_send_ff && y && !m_cycl_pulse;

	if (wait != m_wait)
	{
		m_wait = wait;
		update_busleds();
	}
}

TIMER_CALLBACK_MEMBER(poly880_state::cycl_end)
{
	m_cycl_pulse = false;
	update_step();
}


// Input Ports

INPUT_CHANGED_MEMBER(poly880_state::trigger_reset)
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? ASSERT_LINE : CLEAR_LINE);
	update_step();
}

INPUT_CHANGED_MEMBER(poly880_state::step_keys)
{
	update_step();
}

INPUT_CHANGED_MEMBER(poly880_state::cycl_pressed)
{
	if (newval)
	{
		// C9 differentiates the debounced key edge; /WAIT is released until the node recharges above the TTL threshold
		m_cycl_pulse = true;
		m_cycl_timer->adjust(attotime::from_nsec(2700));
		update_step();
	}
}

static INPUT_PORTS_START( poly880 )
	PORT_START("IN.0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("GO") PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("EXEC") PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("BACK") PORT_CODE(KEYCODE_BACKSPACE) PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("REG") PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("FCT") PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("STEP") PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("MEM") PORT_CODE(KEYCODE_M) PORT_CHAR('M')

	PORT_START("IN.1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_CHAR('0')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_CHAR('2')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_CHAR('3')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_CHAR('1')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_CHAR('8')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_CHAR('9')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')

	PORT_START("IN.2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_CHAR('4')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_CHAR('6')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_CHAR('7')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_CHAR('5')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')

	PORT_START("SPECIAL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("RES") PORT_CODE(KEYCODE_F1) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(poly880_state::trigger_reset), 0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("MON") PORT_CODE(KEYCODE_F2) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(poly880_state::step_keys), 0)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("MCYCL") PORT_CODE(KEYCODE_F3) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(poly880_state::step_keys), 0)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("CYCL") PORT_CODE(KEYCODE_F4) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(poly880_state::cycl_pressed), 0)
INPUT_PORTS_END


void poly880_state::int_w(int state)
{
	m_int = state;
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, state ? ASSERT_LINE : CLEAR_LINE);
}

void poly880_state::halt_w(int state)
{
	m_halt = state;
}


// Z80-CTC Interface

void poly880_state::ctc_z0_w(int state)
{
	// SEND
	m_send = state;

	if (state)
	{
		m_send_ff = true;
		update_step();
	}
}


// Z80-PIO Interface

void poly880_state::pio1_pa_w(u8 data)
{
	/*

	    bit     signal  description

	    PA0     SD0     segment E
	    PA1     SD1     segment D
	    PA2     SD2     segment C
	    PA3     SD3     segment DP
	    PA4     SD4     segment G
	    PA5     SD5     segment A
	    PA6     SD6     segment F
	    PA7     SD7     segment B

	*/

	m_display->write_mx(bitswap<8>(data, 3, 4, 6, 0, 1, 2, 7, 5));
}

u8 poly880_state::pio1_pb_r()
{
	/*

	    bit     signal  description

	    PB0
	    PB1     MIN     tape input
	    PB2
	    PB3     n/c
	    PB4     KI1     key row 1 input
	    PB5     KI2     key row 2 input
	    PB6
	    PB7     KI3     key row 3 input

	*/

	u8 data = 0x4c | ((m_cassette->input() < +0.0) << 1);

	for (int i = 0; i < 8; i++)
	{
		if (BIT(m_matrix, i))
		{
			if (BIT(m_inputs[0]->read(), i)) data |= 0x10;
			if (BIT(m_inputs[1]->read(), i)) data |= 0x20;
			if (BIT(m_inputs[2]->read(), i)) data |= 0x80;
		}
	}

	return data;
}

void poly880_state::pio1_pb_w(u8 data)
{
	/*

	    bit     signal  description

	    PB0     TTY     teletype
	    PB1
	    PB2     MOUT    tape output
	    PB3
	    PB4
	    PB5
	    PB6     SCON    release initial NMI
	    PB7

	*/

	// tape output
	m_cassette->output(BIT(data, 2) ? +1.0 : -1.0);

	bool const scon = BIT(data, 6);

	if (scon != m_scon)
	{
		m_scon = scon;

		if (scon)
		{
			if (!m_send)
				m_send_ff = false;
		}
		else
			m_nmi_ff = false;

		update_step();
	}
}


// Z80 Daisy Chain

static const z80_daisy_config poly880_daisy_chain[] =
{
	{ "pio0" },
	{ "pio1" },
	{ "ctc" },
	{ nullptr }
};


// Machine Initialization

void poly880_state::machine_start()
{

	m_cycl_timer = timer_alloc(FUNC(poly880_state::cycl_end), this);

	// register for state saving
	save_item(NAME(m_matrix));
	save_item(NAME(m_send));
	save_item(NAME(m_scon));
	save_item(NAME(m_send_ff));
	save_item(NAME(m_nmi_ff));
	save_item(NAME(m_icycl));
	save_item(NAME(m_cycl_pulse));
	save_item(NAME(m_wait));
	save_item(NAME(m_halt));
	save_item(NAME(m_int));
	save_item(NAME(m_bus_addr));
	save_item(NAME(m_bus_data));
	save_item(NAME(m_bus_ctrl));
	save_item(NAME(m_bus_sample));
}

void poly880_state::machine_reset()
{
	m_send_ff = false;
	m_nmi_ff = false;
	m_icycl = true;
	m_cycl_pulse = false;
	m_cycl_timer->adjust(attotime::never);

	update_step();
}


// Machine Driver

void poly880_state::poly880(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, XTAL(7'372'800)/8);
	m_maincpu->set_addrmap(AS_PROGRAM, &poly880_state::poly880_mem);
	m_maincpu->set_addrmap(AS_OPCODES, &poly880_state::poly880_opcodes);
	m_maincpu->set_addrmap(AS_IO, &poly880_state::poly880_io);
	m_maincpu->set_daisy_config(poly880_daisy_chain);
	m_maincpu->halt_cb().set(FUNC(poly880_state::halt_w));

	INPUT_MERGER_ANY_HIGH(config, "irq").output_handler().set(FUNC(poly880_state::int_w));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(8, 8);
	m_display->set_segmask(0xff, 0xff);

	PWM_DISPLAY(config, m_busled).set_size(1, 34);
	m_busled->set_bri_levels(0.01, 0.1, 0.4);
	m_busled->output_x().set(FUNC(poly880_state::busled_w));

	config.set_default_layout(layout_poly880);

	// devices
	Z80CTC(config, m_ctc, XTAL(7'372'800)/8);
	m_ctc->intr_callback().set("irq", FUNC(input_merger_device::in_w<2>));
	m_ctc->zc_callback<0>().set(FUNC(poly880_state::ctc_z0_w));
	m_ctc->zc_callback<2>().set(m_ctc, FUNC(z80ctc_device::trg3));

	Z80PIO(config, m_pio[0], XTAL(7'372'800)/8);
	m_pio[0]->out_int_callback().set("irq", FUNC(input_merger_device::in_w<0>));
	m_pio[0]->out_pa_callback().set(FUNC(poly880_state::pio1_pa_w));
	m_pio[0]->in_pb_callback().set(FUNC(poly880_state::pio1_pb_r));
	m_pio[0]->out_pb_callback().set(FUNC(poly880_state::pio1_pb_w));

	Z80PIO(config, m_pio[1], XTAL(7'372'800)/8);
	m_pio[1]->out_int_callback().set("irq", FUNC(input_merger_device::in_w<1>));

	RAM(config, m_ram).set_default_size("1K").set_extra_options("33K").set_default_value(0x00);

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	SPEAKER(config, "cass_output").front_center(); // on data recorder
	m_cassette->add_route(ALL_OUTPUTS, "cass_output", 0.05);
}

void poly880_state::poly880s(machine_config &config)
{
	poly880(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &poly880_state::poly880s_mem);
	m_maincpu->set_addrmap(AS_OPCODES, &poly880_state::poly880s_opcodes);
}


// ROMs

ROM_START( poly880 )
	ROM_REGION( 0x4000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "bm039.i5", 0x0000, 0x0400, CRC(b1c571e8) SHA1(85bfe53d39d6690e79999a1e1240789497e72db0) )
	ROM_LOAD( "bm040.i6", 0x1000, 0x0400, CRC(9efddf5b) SHA1(6ffa2f80b2c6f8ec9e22834f739c82f9754272b8) )
ROM_END

ROM_START( poly880s )
	ROM_REGION( 0x4000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "sc1.rom", 0x0000, 0x1000, CRC(26965b23) SHA1(01568911446eda9f05ec136df53da147b7c6f2bf) )
ROM_END

} // anonymous namespace


// System Drivers

//    YEAR  NAME      PARENT   COMPAT  MACHINE   INPUT    CLASS          INIT        COMPANY, FULLNAME, FLAGS
COMP( 1983, poly880,  0,       0,      poly880,  poly880, poly880_state, empty_init, "VEB Polytechnik Karl-Marx-Stadt", "Poly-Computer 880", MACHINE_SUPPORTS_SAVE )
COMP( 1983, poly880s, poly880, 0,      poly880s, poly880, poly880_state, empty_init, "hack", "Poly-Computer 880 (SC1)", MACHINE_SUPPORTS_SAVE )
