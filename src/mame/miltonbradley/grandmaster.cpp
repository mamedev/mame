// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/*******************************************************************************

Milton Bradley (Electronic) Grand Master (stylized as Grand·Master) (model 4243)
aka Phantom Chess Computer in the UK, and Milton in the rest of Europe

It's a Chess computer with plotter style motor + magnet hidden underneath it. When
it's the computer's turn, it will automatically do the move. Programmed by Intelligent
Software, the chess engine is weaker than the one in SciSys Mark V. The hardware design
and technology was sold to Fidelity a couple of years later (fphantom in MAME).

At boot-up, the computer will do a self-test. Although the user can start playing
immediately, it may be a big distracting. So, just fast forward MAME for a while
(hold INS key on Windows) before starting a new game.

After the user captures a piece, select the captured piece from the MAME sensorboard
spawn block and place it anywhere on a free spot at the designated box at the edge
of the chessboard.

Hardware notes:
- PCB label: ASSY 1274243001 PN 7924243001
- SY6502A @ 1.79MHz (3.58MHz resonator)
- 16KB ROM (2*2364), 2KB RAM (4*MM2114N-25)
- 2 DC motors under chessboard, electromagnet for automatically moving chess pieces
- piezo speaker, 16 LEDs, 12*8 chessboard buttons

There's also a newer revision with mask ROM labels C19679 7830043002 and C19680
7830043001, ROM contents is confirmed to be the same.

*******************************************************************************/

#include "emu.h"

#include "cpu/m6502/m6502.h"
#include "machine/7474.h"
#include "machine/clock.h"
#include "machine/gmboard.h"
#include "machine/input_merger.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "speaker.h"

// internal artwork
#include "grandmaster.lh"


namespace {

class grandmas_state : public driver_device
{
public:
	grandmas_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_irq_ff(*this, "irq_ff%u", 0),
		m_board(*this, "board"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.0")
	{ }

	void grandmas(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device_array<ttl7474_device, 3> m_irq_ff;
	required_device<gmboard_device> m_board;
	required_device<pwm_display_device> m_display;
	required_device<dac_1bit_device> m_dac;
	required_ioport m_inputs;

	u8 m_inp_mux = 0;

	void main_map(address_map &map) ATTR_COLD;

	u8 irq_clear_r(offs_t offset);
	void irq_clear_w(offs_t offset, u8 data = 0);

	u8 status_r();
	void control_w(u8 data);
	void leds_w(u8 data);
	u8 input_r(offs_t offset);
};

void grandmas_state::machine_start()
{
	save_item(NAME(m_inp_mux));
}

void grandmas_state::machine_reset()
{
	// make sure 7474 is not in an indeterminate state
	for (int i = 0; i < 3; i++)
		irq_clear_w(i);
}



/*******************************************************************************
    I/O
*******************************************************************************/

void grandmas_state::irq_clear_w(offs_t offset, u8 data)
{
	// any read or write here clears IRQ F/F
	m_irq_ff[offset]->clear_w(0);
	m_irq_ff[offset]->clear_w(1);
}

u8 grandmas_state::irq_clear_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
		irq_clear_w(offset);

	return 0xff;
}

u8 grandmas_state::status_r()
{
	// d0: magnet sensor
	u8 data = m_board->magnet_r() ^ 1;

	// d1,d3,d5: IRQ F/F Q
	for (int i = 0; i < 3; i++)
		data |= m_irq_ff[2 - i]->output_r() << (i * 2 + 1);

	// d2,d4: motor y/x quadrature state
	for (int i = 0; i < 2; i++)
		data |= ((m_board->quad_r(i) ^ m_board->quad_r(i) >> 1) & 1) << (i ? 2 : 4);

	return data | 0xc0;
}

void grandmas_state::control_w(u8 data)
{
	// d0: vertical motor down
	// d1: vertical motor up
	// d2: horizontal motor left
	// d3: horizontal motor right
	m_board->motor_w(1, bitswap<2>(data, 0, 1));
	m_board->motor_w(0, data >> 2 & 3);

	// d4: electromagnet
	m_board->magnet_w(BIT(data, 4));

	// d5: speaker out
	m_dac->write(BIT(data, 5));
}

void grandmas_state::leds_w(u8 data)
{
	// d0-d3: 74145 A-D
	// 74145 0-8: input mux, led data
	m_inp_mux = data & 0xf;

	// d4,d5: led select
	m_display->matrix(data >> 4 & 3, 1 << m_inp_mux);
}

u8 grandmas_state::input_r(offs_t offset)
{
	u16 data = 0;

	// read chessboard
	if (m_inp_mux < 8)
		data = m_board->read_rank(m_inp_mux);

	// read buttons
	else if (m_inp_mux == 8)
		data = m_inputs->read();

	data = bitswap<4>(data,10,11,8,9) << 8 | (data & 0xff);
	return ~data >> ((offset ^ 1) * 6) | 0xc0;
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void grandmas_state::main_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0x7fff);

	map(0x0000, 0x07ff).mirror(0x1800).ram();

	map(0x2000, 0x2002).mirror(0x1ff8).rw(FUNC(grandmas_state::irq_clear_r), FUNC(grandmas_state::irq_clear_w));
	map(0x2003, 0x2003).mirror(0x1ff8).r(FUNC(grandmas_state::status_r));
	map(0x2004, 0x2004).mirror(0x1ff8).w(FUNC(grandmas_state::control_w));
	map(0x2005, 0x2006).mirror(0x1ff8).r(FUNC(grandmas_state::input_r));
	map(0x2007, 0x2007).mirror(0x1ff8).w(FUNC(grandmas_state::leds_w));

	map(0x4000, 0x7fff).rom().region("maincpu", 0);
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( grandmas )
	PORT_START("IN.0")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Level / Stop")
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("Hint / New")
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Change / Replay")
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("Forward / Back")
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("Verify / Setup")
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("Game / Manual")
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Auto / Problem")
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("Sound / Legal")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x800, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Shift")
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void grandmas_state::grandmas(machine_config &config)
{
	// basic machine hardware
	M6502(config, m_maincpu, 3.58_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &grandmas_state::main_map);

	INPUT_MERGER_ANY_HIGH(config, "mainirq").output_handler().set_inputline(m_maincpu, m6502_device::IRQ_LINE);

	TTL7474(config, m_irq_ff[0]).output_cb().set("mainirq", FUNC(input_merger_device::in_w<0>));
	TTL7474(config, m_irq_ff[1]).output_cb().set("mainirq", FUNC(input_merger_device::in_w<1>));
	TTL7474(config, m_irq_ff[2]).output_cb().set("mainirq", FUNC(input_merger_device::in_w<2>));

	auto &irq_clock(CLOCK(config, "irq_clock"));
	irq_clock.set_period(attotime::from_ticks(0x1000, 3.58_MHz_XTAL / 2)); // CD4020 Q12
	irq_clock.signal_handler().set(m_irq_ff[0], FUNC(ttl7474_device::clock_w));

	MB_GMBOARD(config, m_board);
	m_board->set_size(750, 575, 64);
	m_board->set_offsets(10, 75);
	m_board->quad_cb<0>().set(m_irq_ff[1], FUNC(ttl7474_device::clock_w)).bit(1);
	m_board->quad_cb<1>().set(m_irq_ff[2], FUNC(ttl7474_device::clock_w)).bit(1);

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(2, 9);
	config.set_default_layout(layout_grandmaster);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( grandmas )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD("c19660_7834243004_reva.u3", 0x0000, 0x2000, CRC(c7a91b34) SHA1(43f9caf8a13ae1a3274851fbcc411bc50c21fe1d) )
	ROM_LOAD("c19661_7834243003_reva.u2", 0x2000, 0x2000, CRC(b47dda81) SHA1(02c946fa47db1c7edfb59e11ad3c802b92f4d3a1) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY, FULLNAME, FLAGS
SYST( 1983, grandmas, 0,      0,      grandmas, grandmas, grandmas_state, empty_init, "Milton Bradley", "Grand Master (Milton Bradley)", MACHINE_SUPPORTS_SAVE | MACHINE_MECHANICAL | MACHINE_IMPERFECT_CONTROLS )
