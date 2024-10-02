// license:BSD-3-Clause
// copyright-holders:hap
/*******************************************************************************

Mephisto Turniermaschinen (dedicated in-house chesscomputers used at tournaments),
and their limited-release home versions. These are mephisto_modular hardware
generation, see that driver for more information.

V versions were sold in limited quantities by Hobby Computer Centrale.
T versions were the ones used in actual tournaments, some of them sold to fans.

The Bavaria board does not work on these. Not only does it not have the connector
for it, but no software 'driver' either.

Mephisto TM Almeria is not on this hardware.

Mephisto TM Berlin is an unreleased dev version, an update to TM Vancouver, but
not used in any tournament. Internal string and version matches TM Vancouver,
but ROM has many differences.

BTANB:
- lyon32t8 still says "2048Kbyte" even though it uses 8MB RAM

================================================================================

Hardware notes:

V(Verkauf?) home version:
- XC68030RC33B @ 36MHz
- 256KB SRAM (8*TC55465P-25), 128KB or 256KB ROM
- 2MB DRAM (16*TC514256AP-70)
- 8KB battery-backed SRAM (TC5565PL-15)
- 8*8 LEDs, magnets chessboard

T(Turnier) tournament version: (differences)
- XC68030RC50B, CPU frequency tuned for tournament (see change_cpu_freq)
- 3 more 2MB DRAM rows

After boot, it copies ROM to RAM, probably to circumvent waitstates on slow ROM.

*******************************************************************************/

#include "emu.h"

#include "mmboard.h"
#include "mmdisplay2.h"

#include "cpu/m68000/m68030.h"
#include "machine/nvram.h"

// internal artwork
#include "mephisto_modular_tm.lh"


namespace {

class mmtm_state : public driver_device
{
public:
	mmtm_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_boot_view(*this, "boot_view"),
		m_nvram(*this, "nvram", 0x2000, ENDIANNESS_BIG)
	{ }

	// machine configs
	void mmtm_v(machine_config &config);
	void mmtm_t(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(change_cpu_freq);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	memory_view m_boot_view;
	memory_share_creator<u8> m_nvram;

	emu_timer *m_boot_timer;

	// address maps
	void mmtm_2m_map(address_map &map) ATTR_COLD;
	void mmtm_8m_map(address_map &map) ATTR_COLD;

	u8 nvram_r(offs_t offset) { return m_nvram[offset]; }
	void nvram_w(offs_t offset, u8 data) { m_nvram[offset] = data; }

	TIMER_CALLBACK_MEMBER(disable_bootrom) { m_boot_view.select(1); }
};



/*******************************************************************************
    Initialization
*******************************************************************************/

void mmtm_state::machine_start()
{
	m_boot_timer = timer_alloc(FUNC(mmtm_state::disable_bootrom), this);
}

void mmtm_state::machine_reset()
{
	// disable bootrom after reset
	m_boot_view.select(0);
	m_boot_timer->adjust(m_maincpu->cycles_to_attotime(50));
}

INPUT_CHANGED_MEMBER(mmtm_state::change_cpu_freq)
{
	// "Mephisto X" were usually overclocked at tournaments
	// rare versions sold to fans seen overclocked at 60MHz or 66MHz
	// default frequency of TM version is 50MHz (also matches beeper pitch with V version)
	static const XTAL xtal[3] = { 50_MHz_XTAL, 60_MHz_XTAL, 66_MHz_XTAL };
	m_maincpu->set_unscaled_clock(xtal[newval % 3]);

	// lcd busy flag timing problem when overclocked
	subdevice<hd44780_device>("display:hd44780")->set_clock_scale((newval == 0) ? 1.0 : 1.32);
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void mmtm_state::mmtm_2m_map(address_map &map)
{
	map(0x00000000, 0x0003ffff).view(m_boot_view);
	m_boot_view[0](0x00000000, 0x0003ffff).rom().region("maincpu", 0);
	m_boot_view[1](0x00000000, 0x0003ffff).ram();

	map(0x80000000, 0x801fffff).ram();
	map(0xf0000000, 0xf003ffff).rom().region("maincpu", 0);
	map(0xfc000000, 0xfc001fff).rw(FUNC(mmtm_state::nvram_r), FUNC(mmtm_state::nvram_w)).umask32(0xffffffff);
	map(0xfc020004, 0xfc020007).portr("KEY1");
	map(0xfc020008, 0xfc02000b).portr("KEY2");
	map(0xfc020010, 0xfc020013).portr("KEY3");
	map(0xfc040000, 0xfc040000).w("display", FUNC(mephisto_display2_device::latch_w));
	map(0xfc060000, 0xfc060000).w("display", FUNC(mephisto_display2_device::io_w));
	map(0xfc080000, 0xfc080000).w("board", FUNC(mephisto_board_device::mux_w));
	map(0xfc0a0000, 0xfc0a0000).w("board", FUNC(mephisto_board_device::led_w));
	map(0xfc0c0000, 0xfc0c0000).r("board", FUNC(mephisto_board_device::input_r));
}

void mmtm_state::mmtm_8m_map(address_map &map)
{
	mmtm_2m_map(map);
	map(0x80000000, 0x807fffff).ram();
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( mmtm_v )
	PORT_START("KEY1")
	PORT_BIT(0x01000000, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_NAME("LEFT")   PORT_CODE(KEYCODE_LEFT) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x02000000, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_NAME("ENT")    PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CODE(KEYCODE_7_PAD)

	PORT_START("KEY2")
	PORT_BIT(0x01000000, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_NAME("RIGHT")  PORT_CODE(KEYCODE_RIGHT) PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x02000000, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_NAME("UP")     PORT_CODE(KEYCODE_UP) PORT_CODE(KEYCODE_8_PAD)

	PORT_START("KEY3")
	PORT_BIT(0x01000000, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_NAME("DOWN")   PORT_CODE(KEYCODE_DOWN) PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x02000000, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_NAME("CL")     PORT_CODE(KEYCODE_BACKSPACE) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_9_PAD)
INPUT_PORTS_END

static INPUT_PORTS_START( mmtm_t )
	PORT_INCLUDE( mmtm_v )

	PORT_START("CPU")
	PORT_CONFNAME( 0x03, 0x00, "CPU Frequency" ) PORT_CHANGED_MEMBER(DEVICE_SELF, mmtm_state, change_cpu_freq, 0)
	PORT_CONFSETTING(    0x00, "50MHz" )
	PORT_CONFSETTING(    0x01, "60MHz" )
	PORT_CONFSETTING(    0x02, "66MHz" )
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void mmtm_state::mmtm_v(machine_config &config)
{
	// basic machine hardware
	M68030(config, m_maincpu, 36_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &mmtm_state::mmtm_2m_map);

	const attotime irq_period = attotime::from_hz(12.288_MHz_XTAL / 0x8000); // through 4060, 375Hz
	m_maincpu->set_periodic_int(FUNC(mmtm_state::irq3_line_hold), irq_period);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	MEPHISTO_SENSORS_BOARD(config, "board");
	subdevice<sensorboard_device>("board:board")->set_nvram_enable(true);

	MEPHISTO_DISPLAY_MODULE2(config, "display");
	config.set_default_layout(layout_mephisto_modular_tm);
}

void mmtm_state::mmtm_t(machine_config &config)
{
	mmtm_v(config);

	// basic machine hardware
	m_maincpu->set_clock(50_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &mmtm_state::mmtm_8m_map);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( port32t ) // V101 FA1D 1CD7
	ROM_REGION32_BE( 0x40000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD("portorose_68030_tournament.bin", 0x00000, 0x20000, CRC(31f4c916) SHA1(d22572d224b7308d0d03617997a1ad90e63403c5) )
ROM_END

ROM_START( lyon32t ) // V207 6D28 5805
	ROM_REGION32_BE( 0x40000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD("lyon_68030_tournament.bin", 0x00000, 0x20000, CRC(f07856af) SHA1(a1d5191a4ab4518f2df22f10e6e1305bea8afb37) )
ROM_END

ROM_START( lyon32t8 ) // T207 3C9F 5805
	ROM_REGION32_BE( 0x40000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD("lyon_68030_tournament_8mb.bin", 0x00000, 0x20000, CRC(7b3e6db5) SHA1(28ce87c2d12d92e1a534aaa8dd5489fcf1b42964) )
ROM_END

ROM_START( van32t ) // V309 15C8 18D3
	ROM_REGION32_BE( 0x40000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD("vancouver_68030_tournament.bin", 0x00000, 0x40000, CRC(7b25c9ec) SHA1(bf955b9521f52341754814a7c0bf5941989c8be6) )
ROM_END

ROM_START( van32t8 ) // T309 E04C 18D3
	ROM_REGION32_BE( 0x40000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD("vancouver_68030_tournament_8mb.bin", 0x00000, 0x40000, CRC(d9f0190b) SHA1(7631d2499baad7b8075a1ca2c49f6544d7020c95) )
ROM_END

ROM_START( berl32t8p ) // T309 B138 C981
	ROM_REGION32_BE( 0x40000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD("berlin_68030_tournament_8mb.bin", 0x00000, 0x40000, CRC(4fa6d99d) SHA1(4cda1ce11136e5055f5307f3de3a29b1d98d4f00) )
ROM_END

ROM_START( lond32t ) // V500 BAC6 B0D1
	ROM_REGION32_BE( 0x40000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD("london_68030_tournament.bin", 0x00000, 0x40000, CRC(cc7a1a19) SHA1(860b84ac354280cec1cfcc36627e0a6e1d80c108) )
ROM_END

ROM_START( lond32t8 ) // T500 854A B0D1
	ROM_REGION32_BE( 0x40000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD("london_68030_tournament_8mb.bin", 0x00000, 0x40000, CRC(1ef51242) SHA1(7d4ffec7d80789aaf0a9e796baa8dac7ef2c2f1b) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME       PARENT  COMPAT  MACHINE  INPUT   CLASS       INIT        COMPANY, FULLNAME, FLAGS
SYST( 1989, port32t,   port32, 0,      mmtm_v,  mmtm_v, mmtm_state, empty_init, "Hegener + Glaser", "Mephisto Portorose 68030", MACHINE_SUPPORTS_SAVE )

SYST( 1990, lyon32t,   lyon32, 0,      mmtm_v,  mmtm_v, mmtm_state, empty_init, "Hegener + Glaser", "Mephisto Lyon 68030", MACHINE_SUPPORTS_SAVE )
SYST( 1990, lyon32t8,  lyon32, 0,      mmtm_t,  mmtm_t, mmtm_state, empty_init, "Hegener + Glaser", "Mephisto TM Lyon", MACHINE_SUPPORTS_SAVE )

SYST( 1991, van32t,    van32,  0,      mmtm_v,  mmtm_v, mmtm_state, empty_init, "Hegener + Glaser", "Mephisto Vancouver 68030", MACHINE_SUPPORTS_SAVE )
SYST( 1991, van32t8,   van32,  0,      mmtm_t,  mmtm_t, mmtm_state, empty_init, "Hegener + Glaser", "Mephisto TM Vancouver", MACHINE_SUPPORTS_SAVE )
SYST( 1991, berl32t8p, van32,  0,      mmtm_t,  mmtm_t, mmtm_state, empty_init, "Hegener + Glaser", "Mephisto TM Berlin (prototype)", MACHINE_SUPPORTS_SAVE )

SYST( 1996, lond32t,   lond32, 0,      mmtm_v,  mmtm_v, mmtm_state, empty_init, "Saitek", "Mephisto London 68030", MACHINE_SUPPORTS_SAVE ) // after Saitek took over H+G
SYST( 1996, lond32t8,  lond32, 0,      mmtm_t,  mmtm_t, mmtm_state, empty_init, "Saitek", "Mephisto TM London", MACHINE_SUPPORTS_SAVE ) // "
