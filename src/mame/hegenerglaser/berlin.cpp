// license:BSD-3-Clause
// copyright-holders:Sandro Ronco, hap
/*******************************************************************************

Mephisto Berlin 68000 / Berlin Professional 68020
Berlin Professional has the same engine as Mephisto Genius.

Hardware notes:

Berlin 68000:
- MC68HC000FN12 @ 12.288MHz
- 128KB ROM (2*M27C512-15XF1)
- 512KB DRAM (4*TC514256AP-80)
- 8KB SRAM (HM6264ALP-12 or equivalent), CR2032 battery
- HD44780, 2-line LCD display (small daughterboard)
- 8*8 chessboard buttons, 64 leds, piezo

Berlin Professional 68020:
- MC68EC020RP25 @ 24.576MHz
- 256KB ROM (D27C020-200V10)
- 1MB DRAM (8*MCM514256AP70)
- rest is similar to Berlin 68000

Undocumented buttons:
- holding ENTER and LEFT cursor on boot runs diagnostics
- holding CLEAR on boot clears battery backed RAM

TODO:
- verify IRQ level and acknowledge
- does it have ROM waitstates like modular.cpp?

*******************************************************************************/

#include "emu.h"

#include "mmboard.h"
#include "mmdisplay2.h"

#include "cpu/m68000/m68000.h"
#include "cpu/m68000/m68020.h"
#include "machine/nvram.h"

// internal artwork
#include "mephisto_berlin.lh"


namespace {

class berlin_state : public driver_device
{
public:
	berlin_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_nvram(*this, "nvram", 0x2000, ENDIANNESS_BIG),
		m_board(*this, "board"),
		m_display(*this, "display"),
		m_keys(*this, "KEY")
	{ }

	void berlin(machine_config &config);
	void berlinp(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	memory_share_creator<u8> m_nvram;
	required_device<mephisto_board_device> m_board;
	required_device<mephisto_display2_device> m_display;
	required_ioport m_keys;

	void berlin_mem(address_map &map);
	void berlinp_mem(address_map &map);

	u8 nvram_r(offs_t offset) { return m_nvram[offset]; }
	void nvram_w(offs_t offset, u8 data) { m_nvram[offset] = data; }

	u8 input_r();
};



/*******************************************************************************
    I/O
*******************************************************************************/

u8 berlin_state::input_r()
{
	// display i/o d7 selects keypad
	u8 data = (m_display->io_r() & 0x80) ? 0 : m_keys->read();
	return ~m_board->input_r() | data;
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void berlin_state::berlin_mem(address_map &map)
{
	map(0x000000, 0x01ffff).rom();
	map(0x800000, 0x87ffff).ram();
	map(0x900000, 0x903fff).rw(FUNC(berlin_state::nvram_r), FUNC(berlin_state::nvram_w)).umask16(0xff00);
	map(0xa00000, 0xa00000).r(FUNC(berlin_state::input_r));
	map(0xb00000, 0xb00000).w(m_board, FUNC(mephisto_board_device::mux_w));
	map(0xc00000, 0xc00000).w(m_display, FUNC(mephisto_display2_device::latch_w));
	map(0xd00008, 0xd00008).w(m_display, FUNC(mephisto_display2_device::io_w));
	map(0xe00000, 0xe00000).w(m_board, FUNC(mephisto_board_device::led_w));
	map(0xe00000, 0xe00001).nopr(); // clr.b
}

void berlin_state::berlinp_mem(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x400000, 0x4fffff).ram();
	map(0x800000, 0x800000).r(FUNC(berlin_state::input_r));
	map(0x900000, 0x900000).w(m_board, FUNC(mephisto_board_device::mux_w));
	map(0xa00000, 0xa00000).w(m_board, FUNC(mephisto_board_device::led_w));
	map(0xb00000, 0xb00000).w(m_display, FUNC(mephisto_display2_device::io_w));
	map(0xc00000, 0xc00000).w(m_display, FUNC(mephisto_display2_device::latch_w));
	map(0xd00000, 0xd07fff).rw(FUNC(berlin_state::nvram_r), FUNC(berlin_state::nvram_w)).umask32(0xff000000);
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( berlin )
	PORT_START("KEY")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("Enter")           PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("Clear")           PORT_CODE(KEYCODE_BACKSPACE) PORT_CODE(KEYCODE_DEL)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("Up")              PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("Down")            PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("Left")            PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("Right")           PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("New Game (1/2)")  PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_F1) // combine for NEW GAME
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("New Game (2/2)")  PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_F1) // "
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void berlin_state::berlin(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 12.288_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &berlin_state::berlin_mem);

	const attotime irq_period = attotime::from_hz(12.288_MHz_XTAL / 0x4000); // 750Hz
	m_maincpu->set_periodic_int(FUNC(berlin_state::irq2_line_hold), irq_period);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	MEPHISTO_BUTTONS_BOARD(config, m_board); // internal
	subdevice<sensorboard_device>("board:board")->set_nvram_enable(true);

	MEPHISTO_DISPLAY_MODULE2(config, m_display); // internal
	config.set_default_layout(layout_mephisto_berlin);
}

void berlin_state::berlinp(machine_config &config)
{
	berlin(config);

	// basic machine hardware
	M68EC020(config.replace(), m_maincpu, 24.576_MHz_XTAL); // M68EC020RP25
	m_maincpu->set_addrmap(AS_PROGRAM, &berlin_state::berlinp_mem);

	const attotime irq_period = attotime::from_hz(24.576_MHz_XTAL / 0x8000); // 750Hz
	m_maincpu->set_periodic_int(FUNC(berlin_state::irq2_line_hold), irq_period);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( berl16 ) // B003 8C60 CA47
	ROM_REGION16_BE( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE("berlin_68000_even.bin",     0x00000, 0x10000, CRC(31337f15) SHA1(0dcacb153a6f8376e6f1c2f3e57e60aad4370740) )
	ROM_LOAD16_BYTE("berlin_68000_odd_b003.bin", 0x00001, 0x10000, CRC(cc146819) SHA1(e4b2c6e496eff4a657a0718be292f563fb4e5688) )
ROM_END

ROM_START( berl16a ) // B002 8C59 CA47
	ROM_REGION16_BE( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE("berlin_68000_even.bin",     0x00000, 0x10000, CRC(31337f15) SHA1(0dcacb153a6f8376e6f1c2f3e57e60aad4370740) )
	ROM_LOAD16_BYTE("berlin_68000_odd_b002.bin", 0x00001, 0x10000, CRC(513a95f2) SHA1(cbaef0078a119163577e76a78b2110939b17be6b) )
ROM_END

ROM_START( berl16l ) // B500 ABD5 CA47
	ROM_REGION16_BE( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE("berlin_68000_london_even.bin", 0x00000, 0x10000, CRC(0ccddbc6) SHA1(90effdc9f2811a24d450b74ccfb24995ce896b86) )
	ROM_LOAD16_BYTE("berlin_68000_london_odd.bin",  0x00001, 0x10000, CRC(5edac658) SHA1(18ebebc5ceffd9a01798d8a3709875120bd096f7) )
ROM_END

ROM_START( berlinp ) // B400 8AA1 E785
	ROM_REGION32_BE( 0x40000, "maincpu", 0 )
	ROM_LOAD("berlin_020_v400.u2", 0x00000, 0x40000, CRC(82fbaf6e) SHA1(729b7cef3dfaecc4594a6178fc4ba6015afa6202) )
ROM_END

ROM_START( berlinpl ) // B500 53CA 3DCE
	ROM_REGION32_BE( 0x40000, "maincpu", 0 )
	ROM_LOAD("berlin_020_london.u2", 0x00000, 0x40000, CRC(d75e170f) SHA1(ac0ebdaa114abd4fef87361a03df56928768b1ae) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME      PARENT   COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY, FULLNAME, FLAGS
SYST( 1992, berl16,   0,       0,      berlin,  berlin, berlin_state, empty_init, "Hegener + Glaser", "Mephisto Berlin 68000 (v0.03)", MACHINE_SUPPORTS_SAVE )
SYST( 1992, berl16a,  berl16,  0,      berlin,  berlin, berlin_state, empty_init, "Hegener + Glaser", "Mephisto Berlin 68000 (v0.02)", MACHINE_SUPPORTS_SAVE )
SYST( 1996, berl16l,  berl16,  0,      berlin,  berlin, berlin_state, empty_init, "Richard Lang", "Mephisto Berlin 68000 (London upgrade)", MACHINE_SUPPORTS_SAVE )

SYST( 1994, berlinp,  0,       0,      berlinp, berlin, berlin_state, empty_init, "Hegener + Glaser", "Mephisto Berlin Professional 68020", MACHINE_SUPPORTS_SAVE )
SYST( 1996, berlinpl, berlinp, 0,      berlinp, berlin, berlin_state, empty_init, "Richard Lang", "Mephisto Berlin Professional 68020 (London upgrade)", MACHINE_SUPPORTS_SAVE )
