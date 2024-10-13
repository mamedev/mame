// license:BSD-3-Clause
// copyright-holders:Dirk Verwiebe, Cowering, Sandro Ronco, hap
/*******************************************************************************

Hegener + Glaser Mephisto chesscomputers with plugin modules
3rd generation (2nd gen is Glasgow/Amsterdam, 1st gen is MM series)

After Roma, H+G started naming the different versions 16 Bit/32 Bit instead of 68000/68020.
With Genius and the TM versions, they still applied "68030".

The London program (1994 competition) is not a dedicated module, but an EPROM upgrade
released by Richard Lang for Almeria, Lyon, Portorose and Vancouver modules, and also
available as upgrades for Berlin/Berlin Pro and Genius. The engine is the same as
Saitek's 1996 Mephisto London 68030 (limited release TM version).

Hardware notes:

Almeria 16 Bit, Lyon 16 Bit:
- MC68HC000FN12, 12.288MHz XTAL
- 128KB ROM (2*27C512)
- 512KB RAM (4*TC514256AP-10, or equivalent)

Portorose 32 Bit:
- MC68020RC12E, 12.288MHz XTAL
- 128KB ROM (TC5710000-20)
- 1MB RAM (8*TC514256AP-70)

Genius 68030:
- M68EC030RP40B, 33.3330MHz XTAL, 6.144MHz XTAL
- 256KB ROM (M27C2001)
- 512+256 KB RAM (TC518512PL-10, 8*TC55465P-20), note: chess engine resides in the
  256KB RAM, probably because the EPROM is too slow for this CPU at full speed.

Display Modul:
- HD44780, 2-line LCD display
- 8KB RAM (TC5565APL-15L), battery
- piezo speaker

For the dedicated tournament machines, see modular_tm.cpp

Undocumented buttons:
- holding ENTER and LEFT cursor on boot runs diagnostics
- holding CLEAR on boot will clear the battery backed RAM

TODO:
- verify IRQ level for 32bit modules, and how IRQ is acknowledged
- match I/S= diag speed test with real hardware (good test for proper waitstates?)
- gen32 waitstates emulation is preliminary (without it, sound pitch is way too high
  and lcd write speed too fast). Real gen32 sound is a bit lower pitched than MAME.

================================================================================

Bavaria piece recognition board:
 _______________________________________________
|                                               |
| 74HC21                      74HC74    74HC238 |
| 74HC4040   74HC574          74HC173   74HC374 |
| ROM                  XTAL   74HC368   74HC374 |
| 74HC4024   74HC32           74HC139   74HC374 |
|_______________________________________________|

XTAL = 7.37280MHz
ROM = TC57256AD-12, sine table (not used in MAME)

Only usable with Weltmeister modules, Portorose until London (aka this MAME driver).
It does not work on Portorose 1.01, even though it detects the Bavaria board.
See German patent DE4207534 for detailed description.

Each piece has a Tank circuit, and in each square of the board there is a coil.
By scanning all the squares at different frequencies, the resonance frequency
of every piece is obtained in order to identify it.

Coil resonance frequency:
wJ,  bJ,  wK,  bK,  wQ,  bQ,  wP,  bP,  wB,  bB,  wN,  bN,  wR,  bR  (J = Joker)
460, 421, 381, 346, 316, 289, 259, 238, 217, 203, 180, 167, 154, 138 kHz
14,  13,  12,  11,  10,  9,   8,   7,   6,   5,   4,   3,   2,   1   piece ID

In MAME, to switch between the magnets chessboard and the Bavaria board, there's
a configuration port. It's also doable by clicking on the sensorboard UI header
(where it says "SB.MAGNETS" or "SB.BAVARIA"). The board gets detected at boot,
so restart or reset(F3) afterwards.

Reminder: unsupported on Almeria and Portorose 1.01, this is not a bug.

*******************************************************************************/

#include "emu.h"

#include "mmboard.h"
#include "mmdisplay2.h"

#include "cpu/m68000/m68000.h"
#include "cpu/m68000/m68020.h"
#include "cpu/m68000/m68030.h"
#include "machine/nvram.h"
#include "machine/timer.h"

// internal artwork
#include "mephisto_alm16.lh"
#include "mephisto_alm32.lh"
#include "mephisto_gen32.lh"


namespace {

class mmodular_state : public driver_device
{
public:
	mmodular_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_rom(*this, "maincpu"),
		m_nvram(*this, "nvram", 0x2000, ENDIANNESS_BIG),
		m_board(*this, "board"),
		m_bav_busy(*this, "bav_busy"),
		m_fake(*this, "FAKE")
	{ }

	// machine configs
	void alm16(machine_config &config);
	void alm32(machine_config &config);
	void port16(machine_config &config);
	void port32(machine_config &config);
	void van16(machine_config &config);
	void van32(machine_config &config);
	void gen32(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(switch_sensor_type) { set_sbtype(newval); }

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	optional_region_ptr<u32> m_rom;
	memory_share_creator<u8> m_nvram;
	required_device<mephisto_board_device> m_board;
	required_device<timer_device> m_bav_busy;
	optional_ioport m_fake;

	u8 m_bav_data = 0;

	// address maps
	void alm16_mem(address_map &map) ATTR_COLD;
	void alm32_mem(address_map &map) ATTR_COLD;
	void port16_mem(address_map &map) ATTR_COLD;
	void port32_mem(address_map &map) ATTR_COLD;
	void van16_mem(address_map &map) ATTR_COLD;
	void van32_mem(address_map &map) ATTR_COLD;
	void gen32_mem(address_map &map) ATTR_COLD;

	// I/O handlers
	u32 rom_r(offs_t offset);
	void bavaria_w(u8 data);
	u8 bavaria1_r();
	u8 bavaria2_r();

	u8 nvram_r(offs_t offset) { return m_nvram[offset]; }
	void nvram_w(offs_t offset, u8 data) { m_nvram[offset] = data; }

	u8 spawn_cb(offs_t offset);
	void set_sbtype(ioport_value newval);
};

void mmodular_state::machine_start()
{
	save_item(NAME(m_bav_data));
}

void mmodular_state::machine_reset()
{
	set_sbtype(m_fake.read_safe(0) & 1);
}



/*******************************************************************************
    I/O
*******************************************************************************/

u32 mmodular_state::rom_r(offs_t offset)
{
	// waitstates for gen32 (preliminary)
	if (!machine().side_effects_disabled())
		m_maincpu->adjust_icount(-20);

	return m_rom[offset];
}


// Bavaria board

void mmodular_state::set_sbtype(ioport_value newval)
{
	m_bav_data = 0;
	m_board->get()->set_type(newval ? sensorboard_device::INDUCTIVE : sensorboard_device::MAGNETS);

	if (machine().phase() == machine_phase::RUNNING)
	{
		m_board->get()->cancel_hand();
		m_board->get()->refresh();
	}
}

u8 mmodular_state::spawn_cb(offs_t offset)
{
	// ignore jokers
	return (!m_board->get()->is_inductive() && offset > 12) ? 0 : offset;
}

void mmodular_state::bavaria_w(u8 data)
{
	if (!m_board->get()->is_inductive())
		return;

	// d0-d5: select square
	// d6: no function?
	// d7: start search
	if (m_bav_data & ~data & 0x80)
		m_bav_busy->adjust(attotime::from_usec(3000));

	m_bav_data = data;
}

u8 mmodular_state::bavaria1_r()
{
	if (!m_board->get()->is_inductive())
		return 0;

	// d0-d3: piece id
	// other: unused?
	static const u8 piece_id[0x10] =
		{ 0, 8, 4, 6, 2, 10, 12, 7, 3, 5, 1, 9, 11, 14, 13, 0 };

	int x = m_bav_data >> 3 & 7;
	int y = m_bav_data & 7;

	return piece_id[m_board->get()->read_sensor(x, y) & 0xf];
}

u8 mmodular_state::bavaria2_r()
{
	if (!m_board->get()->is_inductive())
		return 0;

	// d7: busy signal
	// other: unused?
	return m_bav_busy->enabled() ? 0x80 : 0;
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void mmodular_state::alm16_mem(address_map &map)
{
	map(0x000000, 0x01ffff).rom();
	map(0x400000, 0x47ffff).ram();
	map(0x800000, 0x803fff).rw(FUNC(mmodular_state::nvram_r), FUNC(mmodular_state::nvram_w)).umask16(0xff00);
	map(0xc00000, 0xc00000).r("board", FUNC(mephisto_board_device::input_r)); // 0xff on Bavaria
	map(0xc80000, 0xc80000).w("board", FUNC(mephisto_board_device::mux_w));
	map(0xd00000, 0xd00000).w("board", FUNC(mephisto_board_device::led_w));
	map(0xd00000, 0xd00001).nopr(); // clr.b
	map(0xf00000, 0xf00003).portr("KEY1");
	map(0xf00004, 0xf00007).portr("KEY2");
	map(0xf00008, 0xf0000b).portr("KEY3");
	map(0xd80000, 0xd80000).w("display", FUNC(mephisto_display2_device::latch_w));
	map(0xd80008, 0xd80008).w("display", FUNC(mephisto_display2_device::io_w));
}

void mmodular_state::port16_mem(address_map &map)
{
	alm16_mem(map);

	map(0xe80002, 0xe80002).r(FUNC(mmodular_state::bavaria1_r));
	map(0xe80004, 0xe80004).w(FUNC(mmodular_state::bavaria_w));
	map(0xe80006, 0xe80006).r(FUNC(mmodular_state::bavaria2_r));
}

void mmodular_state::van16_mem(address_map &map)
{
	port16_mem(map);

	map(0x020000, 0x03ffff).rom();
}


void mmodular_state::alm32_mem(address_map &map)
{
	map(0x00000000, 0x0001ffff).rom();
	map(0x40000000, 0x400fffff).ram();
	map(0x800000ec, 0x800000ef).portr("KEY1");
	map(0x800000f4, 0x800000f7).portr("KEY2");
	map(0x800000f8, 0x800000fb).portr("KEY3");
	map(0x800000fc, 0x800000fc).r("board", FUNC(mephisto_board_device::input_r));
	map(0x88000000, 0x88000007).w("board", FUNC(mephisto_board_device::mux_w)).umask32(0xff000000);
	map(0x90000000, 0x90000007).w("board", FUNC(mephisto_board_device::led_w)).umask32(0xff000000);
	map(0xa0000000, 0xa0000000).w("display", FUNC(mephisto_display2_device::latch_w));
	map(0xa0000010, 0xa0000010).w("display", FUNC(mephisto_display2_device::io_w));
	map(0xa8000000, 0xa8007fff).rw(FUNC(mmodular_state::nvram_r), FUNC(mmodular_state::nvram_w)).umask32(0xff000000);
}

void mmodular_state::port32_mem(address_map &map)
{
	alm32_mem(map);

	map(0x98000004, 0x98000004).r(FUNC(mmodular_state::bavaria1_r));
	map(0x98000008, 0x98000008).w(FUNC(mmodular_state::bavaria_w));
	map(0x9800000c, 0x9800000c).r(FUNC(mmodular_state::bavaria2_r));
}

void mmodular_state::van32_mem(address_map &map)
{
	port32_mem(map);

	map(0x00020000, 0x0003ffff).rom();
}


void mmodular_state::gen32_mem(address_map &map)
{
	map(0x00000000, 0x0003ffff).r(FUNC(mmodular_state::rom_r));
	map(0x40000000, 0x4007ffff).ram();
	map(0x80000000, 0x8003ffff).ram();
	map(0xc0000000, 0xc0000000).r("board", FUNC(mephisto_board_device::input_r));
	map(0xc8000000, 0xc8000003).nopw();
	map(0xc8000004, 0xc8000004).w("board", FUNC(mephisto_board_device::mux_w));
	map(0xd0000000, 0xd0000003).nopw();
	map(0xd0000004, 0xd0000004).w("board", FUNC(mephisto_board_device::led_w));
	map(0xd8000004, 0xd8000004).r(FUNC(mmodular_state::bavaria1_r));
	map(0xd8000008, 0xd8000008).w(FUNC(mmodular_state::bavaria_w));
	map(0xd800000c, 0xd800000c).r(FUNC(mmodular_state::bavaria2_r));
	map(0xe0000000, 0xe0000000).w("display", FUNC(mephisto_display2_device::latch_w));
	map(0xe0000010, 0xe0000010).w("display", FUNC(mephisto_display2_device::io_w));
	map(0xe8000000, 0xe8007fff).rw(FUNC(mmodular_state::nvram_r), FUNC(mmodular_state::nvram_w)).umask32(0xff000000);
	map(0xf0000004, 0xf0000007).portr("KEY1");
	map(0xf0000008, 0xf000000b).portr("KEY2");
	map(0xf0000010, 0xf0000013).portr("KEY3");
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( bavaria )
	PORT_START("FAKE")
	PORT_CONFNAME( 0x01, 0x00, "Board Sensors" ) PORT_CHANGED_MEMBER(DEVICE_SELF, mmodular_state, switch_sensor_type, 0)
	PORT_CONFSETTING(    0x00, "Magnets (Exclusive)" ) // or Muenchen/Modular
	PORT_CONFSETTING(    0x01, "Induction (Bavaria)" )
INPUT_PORTS_END

static INPUT_PORTS_START( gen32 )
	PORT_INCLUDE( bavaria )

	PORT_START("KEY1")
	PORT_BIT(0x01000000, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_NAME("ENT")    PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x02000000, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_NAME("LEFT")   PORT_CODE(KEYCODE_LEFT) PORT_CODE(KEYCODE_4_PAD)

	PORT_START("KEY2")
	PORT_BIT(0x01000000, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_NAME("UP")     PORT_CODE(KEYCODE_UP) PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x02000000, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_NAME("DOWN")   PORT_CODE(KEYCODE_DOWN) PORT_CODE(KEYCODE_2_PAD)

	PORT_START("KEY3")
	PORT_BIT(0x01000000, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_NAME("CL")     PORT_CODE(KEYCODE_BACKSPACE) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT(0x02000000, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_NAME("RIGHT")  PORT_CODE(KEYCODE_RIGHT) PORT_CODE(KEYCODE_6_PAD)
INPUT_PORTS_END

static INPUT_PORTS_START( alm16 )
	PORT_START("KEY1")
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYPAD)      PORT_NAME("LEFT")   PORT_CODE(KEYCODE_LEFT) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYPAD)      PORT_NAME("ENT")    PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CODE(KEYCODE_7_PAD)

	PORT_START("KEY2")
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYPAD)      PORT_NAME("RIGHT")  PORT_CODE(KEYCODE_RIGHT) PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYPAD)      PORT_NAME("UP")     PORT_CODE(KEYCODE_UP) PORT_CODE(KEYCODE_8_PAD)

	PORT_START("KEY3")
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYPAD)      PORT_NAME("DOWN")   PORT_CODE(KEYCODE_DOWN) PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYPAD)      PORT_NAME("CL")     PORT_CODE(KEYCODE_BACKSPACE) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_9_PAD)
INPUT_PORTS_END

static INPUT_PORTS_START( alm32 )
	PORT_START("KEY1")
	PORT_BIT(0x4000, IP_ACTIVE_LOW, IPT_KEYPAD)       PORT_NAME("RIGHT")  PORT_CODE(KEYCODE_RIGHT) PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x8000, IP_ACTIVE_LOW, IPT_KEYPAD)       PORT_NAME("CL")     PORT_CODE(KEYCODE_BACKSPACE) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_9_PAD)

	PORT_START("KEY2")
	PORT_BIT(0x4000, IP_ACTIVE_LOW, IPT_KEYPAD)       PORT_NAME("DOWN")   PORT_CODE(KEYCODE_DOWN) PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x8000, IP_ACTIVE_LOW, IPT_KEYPAD)       PORT_NAME("UP")     PORT_CODE(KEYCODE_UP) PORT_CODE(KEYCODE_8_PAD)

	PORT_START("KEY3")
	PORT_BIT(0x4000, IP_ACTIVE_LOW, IPT_KEYPAD)       PORT_NAME("LEFT")   PORT_CODE(KEYCODE_LEFT) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x8000, IP_ACTIVE_LOW, IPT_KEYPAD)       PORT_NAME("ENT")    PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CODE(KEYCODE_7_PAD)
INPUT_PORTS_END

static INPUT_PORTS_START( port16 )
	PORT_INCLUDE( bavaria )
	PORT_INCLUDE( alm16 )
INPUT_PORTS_END

static INPUT_PORTS_START( port32 )
	PORT_INCLUDE( bavaria )
	PORT_INCLUDE( alm32 )
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void mmodular_state::alm16(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 12.288_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &mmodular_state::alm16_mem);

	const attotime irq_period = attotime::from_hz(12.288_MHz_XTAL / 10 / 0x800); // 600Hz
	m_maincpu->set_periodic_int(FUNC(mmodular_state::irq3_line_hold), irq_period);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	MEPHISTO_SENSORS_BOARD(config, m_board);
	subdevice<sensorboard_device>("board:board")->set_spawnpoints(12+2); // +2 jokers
	subdevice<sensorboard_device>("board:board")->spawn_cb().set(FUNC(mmodular_state::spawn_cb));
	subdevice<sensorboard_device>("board:board")->set_nvram_enable(true);

	TIMER(config, "bav_busy").configure_generic(nullptr);

	// video hardware
	MEPHISTO_DISPLAY_MODULE2(config, "display");
	config.set_default_layout(layout_mephisto_alm16);
}

void mmodular_state::port16(machine_config &config)
{
	alm16(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &mmodular_state::port16_mem);
}

void mmodular_state::van16(machine_config &config)
{
	port16(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &mmodular_state::van16_mem);
}

void mmodular_state::alm32(machine_config &config)
{
	alm16(config);

	// basic machine hardware
	M68020(config.replace(), m_maincpu, 12.288_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &mmodular_state::alm32_mem);

	const attotime irq_period = attotime::from_hz(12.288_MHz_XTAL / 0x4000); // 750Hz
	m_maincpu->set_periodic_int(FUNC(mmodular_state::irq3_line_hold), irq_period);

	config.set_default_layout(layout_mephisto_alm32);
}

void mmodular_state::port32(machine_config &config)
{
	alm32(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &mmodular_state::port32_mem);
}

void mmodular_state::van32(machine_config &config)
{
	port32(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &mmodular_state::van32_mem);
}

void mmodular_state::gen32(machine_config &config)
{
	van32(config);

	// basic machine hardware
	M68EC030(config.replace(), m_maincpu, 33.333_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &mmodular_state::gen32_mem);

	const attotime irq_period = attotime::from_hz(6.144_MHz_XTAL / 0x4000); // through 4060, 375Hz
	m_maincpu->set_periodic_int(FUNC(mmodular_state::irq3_line_hold), irq_period);

	config.set_default_layout(layout_mephisto_gen32);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

// sinus table lookup ROM for Mephisto Bavaria chessboard (unused on MAME)
#define BAVARIA_BOARD_ROM() \
	ROM_REGION( 0x8000, "bavaria", 0 ) \
	ROM_LOAD("sinus_15_bavaria", 0x0000, 0x8000, CRC(84421306) SHA1(5aab13bf38d80a4233c11f6eb5657f2749c14547) )

ROM_START( alm32 ) // U012 D21A 2FCE
	ROM_REGION32_BE( 0x20000, "maincpu", 0 )
	ROM_LOAD("alm32.bin", 0x00000, 0x20000, CRC(38f4b305) SHA1(43459a057ff29248c74d656a036ac325202b9c15) )
ROM_END

ROM_START( alm16 ) // U013 65CE 2FCE
	ROM_REGION16_BE( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE("almeria_16bit_v0.13_even", 0x00000, 0x10000, CRC(ee5b6ec4) SHA1(30920c1b9e16ffae576da5afa0b56da59ada3dbb) ) // AT26C512
	ROM_LOAD16_BYTE("almeria_16bit_v0.13_odd",  0x00001, 0x10000, CRC(d0be4ee4) SHA1(d36c074802d2c9099cd44e75f9de3fc7d1fd9908) ) // "
ROM_END

ROM_START( alm16a ) // U012 737C 2FCE
	ROM_REGION16_BE( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE("almeria_16bit_v0.121_even", 0x00000, 0x10000, CRC(3ab8fd3b) SHA1(0147f2f7aa57a5afab656d05be77bda2d35deb92) ) // TMS27C512-2JL
	ROM_LOAD16_BYTE("almeria_16bit_v0.121_odd",  0x00001, 0x10000, CRC(436c1d85) SHA1(b141789c2be0a22bab58532d7fb8e57131811547) ) // "
ROM_END

ROM_START( port32 ) // V104 3F63 1CD7
	ROM_REGION32_BE( 0x20000, "maincpu", 0 )
	ROM_LOAD("portorose_32bit_v104", 0x00000, 0x20000, CRC(66a6c84c) SHA1(b71035f91a452901e1765e351fb36c3f18888e42) )

	BAVARIA_BOARD_ROM()
ROM_END

ROM_START( port32a ) // V103 C734 1CD7
	ROM_REGION32_BE( 0x20000, "maincpu", 0 )
	ROM_LOAD("portorose_32bit_v103", 0x00000, 0x20000, CRC(02c091b3) SHA1(f1d48e73b24093288dbb8a06617bb62420c07508) )

	BAVARIA_BOARD_ROM()
ROM_END

ROM_START( port32b ) // V101 7805 1CD7
	ROM_REGION32_BE( 0x20000, "maincpu", 0 )
	ROM_LOAD("portorose_32bit_v101", 0x00000, 0x20000, CRC(405bd668) SHA1(8c6eacff7f6784fa1d38344d594c7e52ac828a23) )

	BAVARIA_BOARD_ROM()
ROM_END

ROM_START( port16 ) // V101 630D 1CD7
	ROM_REGION16_BE( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE("port16ev.bin", 0x00000, 0x10000, CRC(68e4a37d) SHA1(33e7216db664174a8448e455bba97738a29c0f31) )
	ROM_LOAD16_BYTE("port16od.bin", 0x00001, 0x10000, CRC(cae77a05) SHA1(9a0ca8bb37325698f8d208f64a340690b9a933b5) )

	BAVARIA_BOARD_ROM()
ROM_END

ROM_START( lyon32 ) // V207 AE64 5805
	ROM_REGION32_BE( 0x20000, "maincpu", 0 )
	ROM_LOAD("lyon32.bin", 0x00000, 0x20000, CRC(5c128b06) SHA1(954c8f0d3fae29900cb1e9c14a41a9a07a8e185f) )

	BAVARIA_BOARD_ROM()
ROM_END

ROM_START( lyon16 ) // V207 EC82 5805
	ROM_REGION16_BE( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE("lyon_16bit_even_v_207", 0x00000, 0x10000, CRC(497bd41a) SHA1(3ffefeeac694f49997c10d248ec6a7aa932898a4) )
	ROM_LOAD16_BYTE("lyon_16bit_odd_v_207",  0x00001, 0x10000, CRC(f9de3f54) SHA1(4060e29566d2f40122ccde3c1f84c94a9c1ed54f) )

	BAVARIA_BOARD_ROM()
ROM_END

ROM_START( van32 ) // V309 3FD3 18D3
	ROM_REGION32_BE( 0x40000, "maincpu", 0 )
	ROM_LOAD("vanc32.bin", 0x00000, 0x40000, CRC(f872beb5) SHA1(9919f207264f74e2b634b723b048ae9ca2cefbc7) )

	BAVARIA_BOARD_ROM()
ROM_END

ROM_START( van16 ) // V309 C8F3 18D3
	ROM_REGION16_BE( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE("vancouver_16_even_v_309", 0x00000, 0x20000, CRC(e87602d5) SHA1(90cb2767b4ae9e1b265951eb2569b9956b9f7f44) )
	ROM_LOAD16_BYTE("vancouver_16_odd_v_309",  0x00001, 0x20000, CRC(585f3bdd) SHA1(90bb94a12d3153a91e3760020e1ea2a9eaa7ec0a) )

	BAVARIA_BOARD_ROM()
ROM_END

ROM_START( gen32 ) // V401 D1BB 5A88
	ROM_REGION32_BE( 0x40000, "maincpu", 0 )
	ROM_LOAD("genius_68030_version_4.01", 0x00000, 0x40000, CRC(ea9938c0) SHA1(645cf0b5b831b48104ad6cec8d78c63dbb6a588c) )

	BAVARIA_BOARD_ROM()
ROM_END

ROM_START( gen32a ) // V400 3B95 5A88
	ROM_REGION32_BE( 0x40000, "maincpu", 0 )
	ROM_LOAD("gen32_4.bin", 0x00000, 0x40000, CRC(6cc4da88) SHA1(ea72acf9c67ed17c6ac8de56a165784aa629c4a1) )

	BAVARIA_BOARD_ROM()
ROM_END

ROM_START( gen32l ) // V500 EDC1 B0D1
	ROM_REGION32_BE( 0x40000, "maincpu", 0 )
	ROM_LOAD("gen32l.bin", 0x00000, 0x40000, CRC(853baa4e) SHA1(946951081d4e91e5bdd9e93d0769568a7fe79bad) )

	BAVARIA_BOARD_ROM()
ROM_END

ROM_START( lond32 ) // V500 DF8B B0D1
	ROM_REGION32_BE( 0x40000, "maincpu", 0 )
	ROM_LOAD("london_program_68020_module", 0x00000, 0x40000, CRC(3225b8da) SHA1(fd8f6f4e9c03b6cdc86d8405e856c26041bfad12) )

	BAVARIA_BOARD_ROM()
ROM_END

ROM_START( lond16 ) // V500 5ED1 B0D1
	ROM_REGION16_BE( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE("london_program_68000_module_even", 0x00000, 0x20000, CRC(68cfc2de) SHA1(93b551180f01f8ed6991c082795cd9ead922179a) )
	ROM_LOAD16_BYTE("london_program_68000_module_odd",  0x00001, 0x20000, CRC(2d75e2cf) SHA1(2ec9222c95f4be9667fb3b4be1b6f90fd4ad11c4) )

	BAVARIA_BOARD_ROM()
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME     PARENT  COMPAT  MACHINE INPUT   CLASS           INIT        COMPANY, FULLNAME, FLAGS
SYST( 1988, alm32,   0,      0,      alm32,  alm32,  mmodular_state, empty_init, "Hegener + Glaser", "Mephisto Almeria 32 Bit", MACHINE_SUPPORTS_SAVE ) // 0.12 or 0.121?
SYST( 1988, alm16,   alm32,  0,      alm16,  alm16,  mmodular_state, empty_init, "Hegener + Glaser", "Mephisto Almeria 16 Bit (v0.13)", MACHINE_SUPPORTS_SAVE )
SYST( 1988, alm16a,  alm32,  0,      alm16,  alm16,  mmodular_state, empty_init, "Hegener + Glaser", "Mephisto Almeria 16 Bit (v0.121)", MACHINE_SUPPORTS_SAVE )

SYST( 1989, port32,  0,      0,      port32, port32, mmodular_state, empty_init, "Hegener + Glaser", "Mephisto Portorose 32 Bit (v1.04)", MACHINE_SUPPORTS_SAVE )
SYST( 1989, port32a, port32, 0,      port32, port32, mmodular_state, empty_init, "Hegener + Glaser", "Mephisto Portorose 32 Bit (v1.03)", MACHINE_SUPPORTS_SAVE )
SYST( 1989, port32b, port32, 0,      port32, port32, mmodular_state, empty_init, "Hegener + Glaser", "Mephisto Portorose 32 Bit (v1.01)", MACHINE_SUPPORTS_SAVE )
SYST( 1989, port16,  port32, 0,      port16, port16, mmodular_state, empty_init, "Hegener + Glaser", "Mephisto Portorose 16 Bit (v1.01)", MACHINE_SUPPORTS_SAVE )

SYST( 1990, lyon32,  0,      0,      port32, port32, mmodular_state, empty_init, "Hegener + Glaser", "Mephisto Lyon 32 Bit", MACHINE_SUPPORTS_SAVE )
SYST( 1990, lyon16,  lyon32, 0,      port16, port16, mmodular_state, empty_init, "Hegener + Glaser", "Mephisto Lyon 16 Bit", MACHINE_SUPPORTS_SAVE )

SYST( 1991, van32,   0,      0,      van32,  port32, mmodular_state, empty_init, "Hegener + Glaser", "Mephisto Vancouver 32 Bit", MACHINE_SUPPORTS_SAVE )
SYST( 1991, van16,   van32,  0,      van16,  port16, mmodular_state, empty_init, "Hegener + Glaser", "Mephisto Vancouver 16 Bit", MACHINE_SUPPORTS_SAVE )

SYST( 1993, gen32,   0,      0,      gen32,  gen32,  mmodular_state, empty_init, "Hegener + Glaser", "Mephisto Genius 68030 (v4.01)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_TIMING )
SYST( 1993, gen32a,  gen32,  0,      gen32,  gen32,  mmodular_state, empty_init, "Hegener + Glaser", "Mephisto Genius 68030 (v4.00)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_TIMING )
SYST( 1996, gen32l,  gen32,  0,      gen32,  gen32,  mmodular_state, empty_init, "Richard Lang", "Mephisto Genius 68030 (London upgrade)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_TIMING )

SYST( 1996, lond32,  0,      0,      van32,  port32, mmodular_state, empty_init, "Richard Lang", "Mephisto London 32 Bit", MACHINE_SUPPORTS_SAVE ) // for alm32/port32/lyon32/van32
SYST( 1996, lond16,  lond32, 0,      van16,  port16, mmodular_state, empty_init, "Richard Lang", "Mephisto London 16 Bit", MACHINE_SUPPORTS_SAVE ) // for alm16/port16/lyon16/van16
