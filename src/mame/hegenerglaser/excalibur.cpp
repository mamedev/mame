// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/*******************************************************************************

Mephisto Excalibur

It's a very rare chess computer, the chess engine was programmed by the same guys
that did Mephisto I/II/III and Mephisto III-S Glasgow.

Hardware notes:

Motherboard:
- PCB label: FORCE COMPUTERS
- HD68000-8 @ 8 MHz (16MHz XTAL)
- 128KB DRAM (16*MB8264-20), half unused
- 4 EPROM sockets are unpopulated
- HD6840 PTM @ 800kHz, HD6821P PIA
- MC14411P (set to 9600 baud), 1.8432MHz XTAL
- 2*HD6850P ACIA looped back to itself, both unused

Interface board:
- PCB label: HGS 15 101 00 B
- edge connector to module with 64KB ROM (4*27128)

Brikett:
- stripped down, only the LCD/keypad PCB
- 4-digit 7seg LCD
- keypad, piezo

The wooden ESB type chessboard is the same as the one for ESB II/Mephisto III,
except the interface is more similar to Mephisto Exclusive.

The motherboard is a 1983 Force Computers 68000 kit with lots of patches and wire
mods. It's an older version of the VME in src/devices/bus/vme/sys68k_cpu1.cpp.

The 68000 is clocked at 8MHz, but due to wait states and bus requests, it runs
much slower, closer to around 5MHz.

There's a 555 timer (R1=11K, R2=22K, C=1nf: ~26kHz) that clocks 68000 BR. Tests
on real hardware showed that the chess computer is about 30% faster if the timer
interval is reduced by a factor 100. It still worked fine, so it's probably a
leftover for VME comms, not the DRAM refresh circuit.

Other possible sources of wait states are periodic DRAM refresh and user/system
area access time.

See glasgow.cpp on how to verify CPU speed. On the real thing after 6 minutes,
number of positions is 2026 for excal, and 2028 for excaltm.

TODO:
- verify CPU speed, see notes above

*******************************************************************************/

#include "emu.h"

#include "mmboard.h"
#include "mmdisplay1.h"

#include "cpu/m68000/m68000.h"
#include "machine/6821pia.h"
#include "machine/6840ptm.h"
#include "sound/dac.h"

#include "speaker.h"

// internal artwork
#include "mephisto_excalibur.lh"


namespace {

class excal_state : public driver_device
{
public:
	excal_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ptm(*this, "ptm"),
		m_pia(*this, "pia"),
		m_board(*this, "board"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	DECLARE_INPUT_CHANGED_MEMBER(reset_button);

	void excal(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<ptm6840_device> m_ptm;
	required_device<pia6821_device> m_pia;
	required_device<mephisto_board_device> m_board;
	required_device<mephisto_display1_device> m_display;
	required_device<dac_1bit_device> m_dac;
	required_ioport_array<4> m_inputs;

	void excal_mem(address_map &map) ATTR_COLD;

	u8 input_r(offs_t offset);
};

void excal_state::machine_start()
{
	// HACK: slow down CPU to account for bus arbiter and wait states
	m_maincpu->set_clock_scale(5.0 / 8.0);
}



/*******************************************************************************
    I/O
*******************************************************************************/

INPUT_CHANGED_MEMBER(excal_state::reset_button)
{
	// reset system (same function as SW1 on the motherboard)
	if (newval)
	{
		m_ptm->reset();
		m_pia->reset();
		m_maincpu->reset();
	}
}

u8 excal_state::input_r(offs_t offset)
{
	u8 data = 0;

	// read keypad
	for (int i = 0; i < 4; i++)
		if (BIT(~offset, i))
			data |= m_inputs[i]->read();

	return data;
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void excal_state::excal_mem(address_map &map)
{
	map(0x000000, 0x01ffff).ram();
	map(0x000000, 0x000007).rom();
	map(0x020000, 0x02ffff).rom().region("maincpu", 0);
	map(0x04c000, 0x04c00f).mirror(0x003ff0).rw(m_ptm, FUNC(ptm6840_device::read), FUNC(ptm6840_device::write)).umask16(0x00ff);
	map(0x05c000, 0x05c007).mirror(0x003ff8).rw(m_pia, FUNC(pia6821_device::read), FUNC(pia6821_device::write)).umask16(0x00ff);

	map(0x100000, 0x100001).mirror(0x0ffffe).r(m_board, FUNC(mephisto_board_device::input_r)).umask16(0x00ff);
	map(0x200000, 0x200001).mirror(0x0ffffe).w(m_board, FUNC(mephisto_board_device::mux_w)).umask16(0xff00);
	map(0x300000, 0x300001).mirror(0x0ffffe).w(m_board, FUNC(mephisto_board_device::led_w)).umask16(0xff00);
	map(0x400000, 0x40001f).mirror(0x0fffe0).r(FUNC(excal_state::input_r)).umask16(0x00ff);
	map(0x800000, 0x800001).mirror(0x0ffffe).w(m_display, FUNC(mephisto_display1_device::data_w)).umask16(0xff00);
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( excal )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSPACE) PORT_CODE(KEYCODE_DEL) PORT_NAME("CL")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("A / 1")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("ENT")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("B / 2 / Pawn")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("INFO")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("C / 3 / Knight")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("LEV")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("D / 4 / Bishop")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_P) PORT_NAME("POS")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("E / 5 / Rook")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Left / Black / 9")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("F / 6 / Queen")

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Right / White / 0")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("G / 7 / King")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("MEM")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("H / 8")

	PORT_START("RESET")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("RES") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(excal_state::reset_button), 0)
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void excal_state::excal(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 16_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &excal_state::excal_mem);

	PTM6840(config, m_ptm, 16_MHz_XTAL / 20);
	m_ptm->irq_callback().set_inputline(m_maincpu, M68K_IRQ_4);

	PIA6821(config, m_pia);
	m_pia->writepa_handler().set(m_display, FUNC(mephisto_display1_device::strobe_w)).bit(0);
	m_pia->writepa_handler().append(m_dac, FUNC(dac_1bit_device::write)).bit(1);

	MEPHISTO_SENSORS_BOARD(config, m_board);
	m_board->set_delay(attotime::from_msec(200));

	// video hardware
	MEPHISTO_DISPLAY_MODULE1(config, m_display);
	config.set_default_layout(layout_mephisto_excalibur);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( excal )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD16_BYTE("1u_215_22.10.u1", 0x00000, 0x04000, CRC(f4b16b35) SHA1(778b1ece0fc5db88b667d550a55cb757c1280d4d) )
	ROM_LOAD16_BYTE("1l_215_22.10.l1", 0x00001, 0x04000, CRC(372639b0) SHA1(fb56a19689e164175a3db10faf24ab3360264b7c) )
	ROM_LOAD16_BYTE("2u_215_22.10.u2", 0x08000, 0x04000, CRC(ad039672) SHA1(4dc80600bcc7ea450102f2d0eb25be644e5e542c) )
	ROM_LOAD16_BYTE("2l_215_22.10.l2", 0x08001, 0x04000, CRC(08dc7409) SHA1(6f7a336c615ff40dd4018a2150c3213bc7e7e1dc) )
ROM_END

ROM_START( excaltm )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD16_BYTE("excalibur_wccc_1983_1u.u1", 0x00000, 0x04000, CRC(70c420c0) SHA1(11130d910dbc062da224ca24d020f34e95e299a9) )
	ROM_LOAD16_BYTE("excalibur_wccc_1983_1l.l1", 0x00001, 0x04000, CRC(5ff3c9f7) SHA1(6d408f9709a7b763215a6bf1c7199d9b54f89d79) )
	ROM_LOAD16_BYTE("excalibur_wccc_1983_2u.u2", 0x08000, 0x04000, CRC(5d899bcc) SHA1(d9dbdc1b05a86664bb22489c812ca347b44c6395) )
	ROM_LOAD16_BYTE("excalibur_wccc_1983_2l.l2", 0x08001, 0x04000, CRC(576d2e6e) SHA1(58b8018996442b4b957d64631c0701be64dfc382) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY, FULLNAME, FLAGS
SYST( 1983, excal,   0,      0,      excal,   excal, excal_state, empty_init, "Hegener + Glaser", "Mephisto Excalibur", MACHINE_SUPPORTS_SAVE )
SYST( 1983, excaltm, excal,  0,      excal,   excal, excal_state, empty_init, "Hegener + Glaser", "Mephisto Excalibur (WCCC 1983 New York TM)", MACHINE_SUPPORTS_SAVE )
