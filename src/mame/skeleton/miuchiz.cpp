// license:BSD-3-Clause
// copyright-holders:Jonathan Gevaryahu
// thanks-to: ChrisMiuchiz
/*
    Driver for the Miuchiz handhelds
    CPU: ST2205U;
        XTAL: Y1 16MHz
        XTAL: Y2 32.768KHz
    LCDC: ST7626 (https://www.crystalfontz.com/controllers/Sitronix/ST7626/)
        the ST7626 is embedded into a epoxy part just below the screen glass with the flex cable attached to it
        it has internal 98x68x16bit ram

    Extremely preliminary
    The inputs are mapped in an input array, but aren't actually hooked anywhere yet.
    The screen is a dummy and just displays nothing for now.
*/

/* Core includes */
#include "emu.h"
#include "cpu/m6502/st2205u.h"
//include "video/st7626lcdc.h"
#include "screen.h"

// defines and logging
#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC printf
#include "logmacro.h"

namespace {

// class definition
class miuchiz_state : public driver_device
{
public:
	miuchiz_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
	{ }

	void miuchiz(machine_config &config);

private:
	void mem_map(address_map &map) ATTR_COLD;

	required_device<st2205u_device> m_maincpu;
	required_device<screen_device> m_screen;
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};


// address maps
void miuchiz_state::mem_map(address_map &map)
{
	map(0x0000000, 0x0003fff).rom().region("otp", 0);
	/*
	map(0x0600000, 0x0600000).w(m_lcdc, FUNC(st7626lcdc_device::lcdc_command_w));
	map(0x0600001, 0x0600001).w(m_lcdc, FUNC(st7626lcdc_device::lcdc_data_w));
	*/
	map(0x0800000, 0x09fffff).rom().region("flash", 0);
}

// flash map?
//  map(0x01000000, 0x011fffff).rom().region("flash", 0);


// input ports
static INPUT_PORTS_START( miuchiz )
	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_NAME("Up")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_NAME("Down")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_NAME("Left")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_NAME("Right")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Power")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START2 ) PORT_NAME("Menu")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Upside-up")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("Upside-down")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Screen-up-left")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Screen-up-right")
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Screen-low-left")
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Screen-low-right")
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Action")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("Mute/Pause")
	PORT_BIT( 0xc000, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

// machine specific functions
u32 miuchiz_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// todo
	return 0;
}

// machine driver
void miuchiz_state::miuchiz(machine_config &config)
{
	ST2205U(config, m_maincpu, XTAL(16'000'000)/2); // Y1 is a hynix HY16.000 crystal, divider is unknown. Y2 is a 32.768KHz xtal for clock
	m_maincpu->set_addrmap(AS_DATA, &miuchiz_state::mem_map);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(68, 98);
	m_screen->set_visarea(0, 68 - 1, 0, 98 - 1);
	m_screen->set_screen_update(FUNC(miuchiz_state::screen_update));

	/* serial hardware */

	/* sound hardware */
}

// rom definitions
ROM_START( miuchiz )
	//ROM_REGION(0x100000000, "maincpu", 0)
	ROM_REGION(0x4000, "otp", 0) // aka 'bootrom', in on-st2205u-chip mask or flash ROM, common for all versions
	ROM_LOAD( "otp.dat",            0x000000, 0x004000, CRC(2ff7ec96) SHA1(633365fd19a3d0f2ce56cb499b2577a5fb53e466) )

	ROM_REGION(0x200000, "flash", 0) // external SST39VF1681 flash chips
	// todo: should these be bioses? or separate drivers for each type (as the artwork would differ) with the alt versions being bioses within each?
	ROM_LOAD( "cloe_1.02.dat",      0x000000, 0x200000, CRC(569e14db) SHA1(5600c3a9cd945f53e7897bbad81b4dc0d7e9457d) )
	ROM_LOAD( "cloe_1.08.dat",      0x000000, 0x200000, CRC(6989e9d9) SHA1(70841a2aed9b567e049b93aab1e7c0b04db8c1d9) )
	ROM_LOAD( "cloe_1.09.03.dat",   0x000000, 0x200000, CRC(9f1c9414) SHA1(74c71b0e4cc8d18b0d6072558d593a3672299e95) )
	ROM_LOAD( "cloe_2.03.01.dat",   0x000000, 0x200000, CRC(c78640cf) SHA1(f58d0fb5c18e474a70ff5bbe529816fbb1a1d295) )
	ROM_LOAD( "creeper_1.04.dat",   0x000000, 0x200000, CRC(86411ef2) SHA1(4060050e2133550b61e21632c4f6237a2ce0c35b) )
	ROM_LOAD( "creeper_1.09.03.dat",0x000000, 0x200000, CRC(80b0a752) SHA1(99022d3256ba000582b8a429c9105b9564227392) )
	ROM_LOAD( "dash_1.02.dat",      0x000000, 0x200000, CRC(68d616a1) SHA1(6bd97f124a53b22143a015597aa3c7d545a53913) )
	ROM_LOAD( "dash_1.09.03.dat",   0x000000, 0x200000, CRC(e36047b3) SHA1(04bf0ce65305ca78d755a0a802d7af87882428ed) )
	ROM_LOAD( "inferno_1.04.dat",   0x000000, 0x200000, CRC(902b0443) SHA1(3cdaf2b2131132e20c5d63b079060553ff8304bb) )
	ROM_LOAD( "inferno_1.09.03.dat",0x000000, 0x200000, CRC(3c6c2bf8) SHA1(e049bc62902c755da1779c57fc7a6724f5210095) )
	ROM_LOAD( "inferno_2.00.04.dat",0x000000, 0x200000, CRC(f7f39596) SHA1(3d8d4310a76cff0420b3f885e4df4baa4ff9023f) )
	ROM_LOAD( "roc_1.04.dat",       0x000000, 0x200000, CRC(640f0d3a) SHA1(e68a187c4fb9674591cb1661de90bd2fb0464530) )
	ROM_LOAD( "roc_1.08.dat",       0x000000, 0x200000, CRC(bace7bc5) SHA1(8abb22c3ee136af0a95e110f55572d06064f64e3) )
	ROM_LOAD( "roc_1.09.03.dat",    0x000000, 0x200000, CRC(1d3bbbeb) SHA1(c3b0658019948c128e9f827027c98f649a9e61f8) )
	ROM_LOAD( "roc_2.00.04.dat",    0x000000, 0x200000, CRC(4c92ce75) SHA1(e00f37402d15cb166afdfd4132a4d48556eb3a04) )
	ROM_LOAD( "spike_1.02.dat",     0x000000, 0x200000, CRC(20c3e78e) SHA1(48c9ab622a32ea96abd6186bc4799fb021b9605b) )
	ROM_LOAD( "spike_1.09.03.dat",  0x000000, 0x200000, CRC(991e2f4d) SHA1(8f635fbd2a6a3a606cd7cbd19ac097974620fb21) )
	ROM_LOAD( "yasmin_1.02.dat",    0x000000, 0x200000, CRC(41749f71) SHA1(d388c977ccc87837b167395423196b480b569e0e) )
	ROM_LOAD( "yasmin_1.08.dat",    0x000000, 0x200000, CRC(56395608) SHA1(4ae9a7026faa7a01d49168336a86beeef56f8e5d) )
	ROM_LOAD( "yasmin_1.09.03.dat", 0x000000, 0x200000, CRC(8c4da665) SHA1(e6948324fd880bfb6b2fdaa282aa2e1841a7e6f1) )
	ROM_LOAD( "yasmin_2.03.01.dat", 0x000000, 0x200000, CRC(cdc4d525) SHA1(e17f280b54647ec76deb9dab8e5fbf35f1a48266) )
ROM_END

} // anonymous namespace

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY              FULLNAME                      FLAGS
COMP( 2006, miuchiz, 0,      0,      miuchiz, miuchiz, miuchiz_state, empty_init, "MGA Entertainment", "MIUCHIZ Virtual Companions", MACHINE_IS_SKELETON )
