// license:BSD-3-Clause
// copyright-holders:David Haywood
/*
    Konami Endeavour hardware (gambling games)

    Hardware:

    1. Backplane PCB (GGAT2 PWB(A1) 10000094517)
       - VGA connector
       - RJ45 connector

    2. MCU2 PCB (GGAT2 PWB(B2) 0000093536)
       - U3, U18, U30, U97: Micron MT48LC4M16A2 SDRAM (other boards may have Hynix HY57V641620)
       - U6: Sound amplifier with heatsink
       - U12: Yamaha YMZ280B-F
       - U21, U22: NEC uPD000AGW-70LL SRAM (some boards may have Hynix SRAM)
       - U26: Texas Instruments LS245
       - U34: Epson RTC62423
       - U66, U75: EPROM sockets
       - U67: IBM PowerPC 403GCX
       - U92, U104: Elpida 5165165FLTT6
       - U98: 93C56 SRAM
       - U109: Konami 0000057714 with heatsink (same as Firebeat GCU)
       - U113: Fujitsu 40C950V
       - U152: D169K41 XTAL (16.9 MHz?)
       - U159: Xilinx XC9572XL, marked 101663
       - U9, U23: CR2032 batteries
       - C378, C379: 0.47F 5.5V supercaps

    3. IFU2 PCB (GGAT2 PWB(D2) 0000094516)
       - U37, U93, U149, U187, U191: HIN239CB
       - U53: Cypress CY7C131-55JC 1Kx8 DPRAM
       - U75: Hitachi H8/3007 HD6413007F20
       - U94, U168: National Semiconductor NS442AF
       - U130: Xilinx XC9536, marked 98475
       - U159, Xilinx XC9572, marked 98477
       - U166: Cypress CY62128BLL-70SC 128x8 SRAM
       - U184: Xilinx XC9572, marked 98476
       - U190: EPROM socket
       - 14 x LEDs
       - 3-pin jumper between backplane connectors: 1-2 JXU(DEFAULT); 2-3 ATU1
       - U147: B19.660 KDS5A XTAL (19.66 MHz?)

    4. FMU2 PCB (GGAT2 PWB(C) 0000094515)
       - 12 x Fujitsu 29F016A-90PFTN surface-mounted flash memory chips (not always populated e.g. rapfire5 only has 6 chips)

    The games use flash memory for resources. Several games are fully dumped but most of the Russian versions don't have the flash data dumped and can't boot.

    Games with progressive jackpots (such as Wild Fire) require a second display.
    The second display is a 15"(?) LCD which is noticeably smaller than the main display, which is a PC-style CRT (early models) or LCD (ES500 models or retrofitted).
    If a jackpot mode is enabled without the additional display, this will produce an "LCD display disconnected" error.

    Current issues:
    Door Open - Main
    Coin Validator Fault
    Coin In Error - Diverter Fault
    Hopper Disconnected
    Hard Meters Disconnected
    Logic Door Port
    IFU2 Failure (enchlamp only)
    LCD Display Disconnected (when enchlamp is set to Wild Fire jackpot mode)
*/

#include "emu.h"

#include "k057714.h"

#include "cpu/h8/h83006.h"
#include "cpu/powerpc/ppc.h"
#include "machine/eepromser.h"
#include "machine/intelfsh.h"
#include "machine/mb8421.h"
#include "machine/msm6242.h"
#include "machine/nvram.h"
#include "sound/ymz280b.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class konendev_state : public driver_device
{
public:
	konendev_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ifu(*this, "ifu")
		, m_gcu(*this, "gcu")
		, m_eeprom(*this, "eeprom")
		, m_dsw(*this, "DSW")
	{ }

	void konendev(machine_config &config) ATTR_COLD;

private:
	// devices
	required_device<cpu_device> m_maincpu;
	required_device<h83007_device> m_ifu;
	required_device<k057714_device> m_gcu;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_ioport m_dsw;

	uint32_t mcu2_r(offs_t offset, uint32_t mem_mask = ~0);
	uint32_t ifu2_r(offs_t offset, uint32_t mem_mask = ~0);
	void eeprom_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	uint16_t ifu_unk_r();

	void gcu_interrupt(int state);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;
	void ifu_map(address_map &map) ATTR_COLD;
	void ymz280b_map(address_map &map) ATTR_COLD;
};

uint32_t konendev_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return m_gcu->draw(screen, bitmap, cliprect);
}

uint32_t konendev_state::mcu2_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t r = 0;

	if (ACCESSING_BITS_24_31)
	{
		r |= 0x11000000;        // MCU2 version
	}
	if (ACCESSING_BITS_16_23)
	{
		r |= (m_eeprom->do_read() ? 0x2 : 0) << 16;
	}
	if (ACCESSING_BITS_8_15)
	{
		r &= ~0x4000;       // MCU2 presence
		r &= ~0x2000;       // IFU2 presence
		r &= ~0x1000;       // FMU2 presence
	}
	if (ACCESSING_BITS_0_7)
	{
		r |= m_dsw->read() & 0xff;
		r |= 0x04;          // battery 1 status
		r |= 0x10;          // battery 2 status
	}

	return r;
}

uint32_t konendev_state::ifu2_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t r = 0;

	if (ACCESSING_BITS_0_7)
	{
		r |= 0x11;          // IFU2 version
	}

	return r;
}

void konendev_state::eeprom_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		m_eeprom->di_write((data & 0x04) ? 1 : 0);
		m_eeprom->clk_write((data & 0x02) ? ASSERT_LINE : CLEAR_LINE);
		m_eeprom->cs_write((data & 0x01) ? ASSERT_LINE : CLEAR_LINE);
	}
}

void konendev_state::main_map(address_map &map)
{
	map(0x00000000, 0x00ffffff).ram();
	map(0x78000000, 0x78000003).r(FUNC(konendev_state::mcu2_r));
	map(0x78080000, 0x7808000f).rw("rtc", FUNC(rtc62423_device::read), FUNC(rtc62423_device::write));
	map(0x780c0000, 0x780c0001).rw("ymz", FUNC(ymz280b_device::read), FUNC(ymz280b_device::write));
	map(0x78100000, 0x78100003).w(FUNC(konendev_state::eeprom_w));
	map(0x78800000, 0x78800003).r(FUNC(konendev_state::ifu2_r));
	map(0x78800004, 0x78800007).portr("IN1"); // doors, switches
	map(0x78a00000, 0x78a00003).portr("IN2"); // hard meter access, hopper
	map(0x78a00004, 0x78a00007).portr("IN3"); // main door optic
	map(0x78c00000, 0x78c003ff).rw("dpram", FUNC(cy7c131_device::right_r), FUNC(cy7c131_device::right_w));
	map(0x78e00000, 0x78e00003).portr("IN0"); // buttons
	map(0x79000000, 0x79000003).w(m_gcu, FUNC(k057714_device::fifo_w));
	map(0x79800000, 0x798000ff).rw(m_gcu, FUNC(k057714_device::read), FUNC(k057714_device::write));
	map(0x7a000000, 0x7a01ffff).ram().share("nvram0");
	map(0x7a100000, 0x7a11ffff).ram().share("nvram1");
	map(0x7e800000, 0x7effffff).rw("prgflash1", FUNC(fujitsu_29f016a_device::read), FUNC(fujitsu_29f016a_device::write)).umask32(0x000000ff);
	map(0x7e800000, 0x7effffff).rw("prgflash2", FUNC(fujitsu_29f016a_device::read), FUNC(fujitsu_29f016a_device::write)).umask32(0x0000ff00);
	map(0x7e800000, 0x7effffff).rw("prgflash3", FUNC(fujitsu_29f016a_device::read), FUNC(fujitsu_29f016a_device::write)).umask32(0x00ff0000);
	map(0x7e800000, 0x7effffff).rw("prgflash4", FUNC(fujitsu_29f016a_device::read), FUNC(fujitsu_29f016a_device::write)).umask32(0xff000000);
	map(0x7f000000, 0x7f7fffff).rw("prgflash5", FUNC(fujitsu_29f016a_device::read), FUNC(fujitsu_29f016a_device::write)).umask32(0x000000ff);
	map(0x7f000000, 0x7f7fffff).rw("prgflash6", FUNC(fujitsu_29f016a_device::read), FUNC(fujitsu_29f016a_device::write)).umask32(0x0000ff00);
	map(0x7f000000, 0x7f7fffff).rw("prgflash7", FUNC(fujitsu_29f016a_device::read), FUNC(fujitsu_29f016a_device::write)).umask32(0x00ff0000);
	map(0x7f000000, 0x7f7fffff).rw("prgflash8", FUNC(fujitsu_29f016a_device::read), FUNC(fujitsu_29f016a_device::write)).umask32(0xff000000);
	map(0x7ff00000, 0x7fffffff).rom().region("program", 0);
}

uint16_t konendev_state::ifu_unk_r()
{
	return 0xc3c3;  // H8 program crashes immediately if it doesn't see
}

void konendev_state::ifu_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom().region("ifu", 0);
	map(0x200000, 0x20ffff).ram();
	map(0x210000, 0x217fff).ram();
	map(0x800000, 0x8003ff).rw("dpram", FUNC(cy7c131_device::left_r), FUNC(cy7c131_device::left_w));
	map(0xa40000, 0xa40001).nopw();
	map(0xfee010, 0xfee011).r(FUNC(konendev_state::ifu_unk_r));
};

void konendev_state::ymz280b_map(address_map &map)
{
	map(0x000000, 0x1fffff).r("sndflash1.u8", FUNC(fujitsu_29f016a_device::read));
	map(0x200000, 0x3fffff).r("sndflash2.u7", FUNC(fujitsu_29f016a_device::read));
	map(0x400000, 0x5fffff).r("sndflash3.u6", FUNC(fujitsu_29f016a_device::read));
	map(0x600000, 0x7fffff).r("sndflash4.u5", FUNC(fujitsu_29f016a_device::read));
}

/*******************************************************************************

Button layouts

[COLLECT] [RESERVE] [HELP]

The Collect/Reserve/Help buttons are located above the monitor rather than on the control panel.
On Australian machines outside NSW/ACT, the Help button may have an "i" logo instead.

[BET 1 ] [ BET 2 ] [ BET 3  ] [ BET 5  ] [ BET 10 ] [ BET 20 ]
[1 LINE] [5 LINES] [10 LINES] [20 LINES] [25 LINES] [30 LINES] [GAMBLE] [TAKE WIN]

Bet/line combinations differ per game/setup options. Some games have red/black double up, others may have a dice game.
The standard Bet/line buttons are square and are the same size as Aristocrat MK5/MVP and IGT Spectrum/GU4.
Some buttons have a hexagonal design however, matching the games with a unique hexagonal reel pattern layout (Egyptagon, Ashanti Queen and Go West are examples).
The Collect/Reserve/Help and Gamble/Take Win buttons are rectangular and smaller.

Bet button combinations:
3 Credits:       1, 2, 3,  N/A,N/A,N/A
4 Credits:       1, 2, 3,  4,  N/A,N/A
5 Credits:       1, 2, 3,  4,  5,  N/A
6 Credits:       1, 2, 3,  4,  5,  6
8 Credits:       1, 2, 3,  4,  5,  8
10 Credits:      1, 2, 3,  4,  5,  10
15 Credits:      1, 2, 3,  5,  10, 15
20 Credits:      1, 2, 3,  5,  10, 20
25 Credits:      1, 2, 3,  5,  10, 25
30 Credits (A):  1, 2, 3,  5,  10, 30
30 Credits (B):  1, 2, 5,  10, 20, 30
40 Credits (A):  1, 2, 3,  5,  10, 40  (10, 20 and 25 lines only)
40 Credits (B):  1, 2, 5,  10, 20, 40  (10, 20 and 25 lines only)
50 Credits (A):  1, 2, 3,  5,  10, 50  (10 and 20 lines only)
50 Credits (B):  1, 2, 5,  10, 20, 50  (10 and 20 lines only)
100 Credits (A): 1, 2, 3,  5,  10, 100 (10 lines only)
100 Credits (B): 1, 5, 10, 20, 50, 100 (10 lines only)

Line button combinations:
10 Lines: 1, 2, 3,  5,  7,  10
20 Lines: 1, 3, 5,  10, 15, 20
25 Lines: 1, 5, 10, 15, 20, 25
30 Lines: 1, 5, 10, 20, 25, 30

Dice-based gamble layout, if the machine supports it:
Bet buttons: 1, 2, 3, 4, 5 and 6 respectively (6:1 odds)
Play 1 Line: 1/3/5 (2:1 odds), red
Play 5 Lines: 1/2 (3:1 odds)
Play 10 Lines: 3/4 (3:1 odds)
Play 20 Lines: 5/6 (3:1 odds)
Play 25 Lines: No dice markings on the fifth line button since all dice combinations are used
Play 30 Lines: 2/4/6 (2:1 odds), black

*******************************************************************************/

static INPUT_PORTS_START( konendev )
	PORT_START("IN0")
	PORT_BIT( 0x000000ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_CODE(KEYCODE_3) PORT_NAME("Help")
	PORT_BIT( 0x0000fe00, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_Q) PORT_NAME("Bet 1 Credit")
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(KEYCODE_W) PORT_NAME("Bet 2 Credits")
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(KEYCODE_E) PORT_NAME("Bet 3 Credits")
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CODE(KEYCODE_R) PORT_NAME("Bet 5 Credits")
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_T) PORT_NAME("Bet 10 Credits")
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_CODE(KEYCODE_Y) PORT_NAME("Bet 20 Credits")
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) PORT_CODE(KEYCODE_1) PORT_NAME("Collect")
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_CODE(KEYCODE_2) PORT_NAME("Reserve")
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_CODE(KEYCODE_A) PORT_NAME("Play 1 Line / Red")
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_CODE(KEYCODE_S) PORT_NAME("Play 5 Lines")
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_CODE(KEYCODE_D) PORT_NAME("Play 10 Lines")
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_CODE(KEYCODE_F) PORT_NAME("Play 20 Lines")
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_BUTTON11 ) PORT_CODE(KEYCODE_G) PORT_NAME("Play 25 Lines")
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_BUTTON12 ) PORT_CODE(KEYCODE_H) PORT_NAME("Play 30 Lines / Black")
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_BUTTON13 ) PORT_CODE(KEYCODE_J) PORT_NAME("Gamble")
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_BUTTON14 ) PORT_CODE(KEYCODE_K) PORT_NAME("Take Win")

	PORT_START("IN1")
	PORT_BIT( 0x0001ffff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x00020000, 0x00020000, "BNA Power" )
	PORT_DIPSETTING( 0x00020000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_BIT( 0x000c0000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x00100000, 0x00100000, "BNA Stacker Exist" )
	PORT_DIPSETTING( 0x00100000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00200000, 0x00200000, "BNA Transport Exist" )
	PORT_DIPSETTING( 0x00200000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00400000, 0x00000000, "BNA Door" )
	PORT_DIPSETTING( 0x00400000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_BIT( 0x03800000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x04000000, 0x00000000, "Cashbox Door" )
	PORT_DIPSETTING( 0x04000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_GAMBLE_DOOR ) PORT_CODE(KEYCODE_M) PORT_TOGGLE PORT_NAME("Main Door")
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN ) PORT_CODE(KEYCODE_F1) PORT_NAME("Reset Key")
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN ) PORT_CODE(KEYCODE_F2) PORT_TOGGLE PORT_NAME("Audit Key")
	PORT_BIT( 0xc0000000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x0000000f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x00000010, 0x00000000, "Hard Meter Access" )
	PORT_DIPSETTING( 0x00000010, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_BIT( 0x00fffe0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x01000000, 0x00000000, "Hopper ?" ) // hopper runaway/hopper jammed
	PORT_DIPSETTING( 0x01000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_UNUSED ) // unused?
	PORT_DIPNAME( 0x04000000, 0x00000000, "Hopper Exist" )
	PORT_DIPSETTING( 0x04000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_BIT( 0xf8000000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT( 0x00ffffff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x01000000, IP_ACTIVE_HIGH, IPT_GAMBLE_DOOR ) PORT_CODE(KEYCODE_M) PORT_TOGGLE PORT_NAME("Main Door Optic")
	PORT_BIT( 0xfe000000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW")
	PORT_DIPNAME( 0x40, 0x00, "Logic Door" )
	PORT_DIPSETTING( 0x00, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "RAM Clear" )
	PORT_DIPSETTING( 0x80, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
INPUT_PORTS_END


void konendev_state::gcu_interrupt(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ1, state);
	m_maincpu->set_input_line(INPUT_LINE_IRQ3, state);
}

void konendev_state::konendev(machine_config &config)
{
	// basic machine hardware
	PPC403GCX(config, m_maincpu, 32'000'000); // Clock unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &konendev_state::main_map);

	H83007(config, m_ifu, 8'000'000); // Clock unknown
	m_ifu->set_addrmap(AS_PROGRAM, &konendev_state::ifu_map);

	FUJITSU_29F016A(config, "prgflash1");
	FUJITSU_29F016A(config, "prgflash2");
	FUJITSU_29F016A(config, "prgflash3");
	FUJITSU_29F016A(config, "prgflash4");
	FUJITSU_29F016A(config, "prgflash5");
	FUJITSU_29F016A(config, "prgflash6");
	FUJITSU_29F016A(config, "prgflash7");
	FUJITSU_29F016A(config, "prgflash8");
	FUJITSU_29F016A(config, "sndflash1.u8");
	FUJITSU_29F016A(config, "sndflash2.u7");
	FUJITSU_29F016A(config, "sndflash3.u6");
	FUJITSU_29F016A(config, "sndflash4.u5");

	CY7C131(config, "dpram");

	// video hardware
	PALETTE(config, "palette", palette_device::RGB_555);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(25.175_MHz_XTAL, 800, 0, 640, 525, 0, 480); // Based on Firebeat settings
	screen.set_screen_update(FUNC(konendev_state::screen_update));
	screen.set_palette("palette");
	screen.screen_vblank().set(m_gcu, FUNC(k057714_device::vblank_w));

	K057714(config, m_gcu, 0).set_screen("screen");
	m_gcu->irq_callback().set(FUNC(konendev_state::gcu_interrupt));

	RTC62423(config, "rtc", XTAL(32'768));

	NVRAM(config, "nvram0", nvram_device::DEFAULT_ALL_0);
	NVRAM(config, "nvram1", nvram_device::DEFAULT_ALL_0);

	EEPROM_93C56_16BIT(config, "eeprom");

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	ymz280b_device &ymz(YMZ280B(config, "ymz", 16'934'400)); // Clock unknown
	ymz.set_addrmap(0, &konendev_state::ymz280b_map);
	ymz.add_route(0, "speaker", 1.0, 0);
	ymz.add_route(1, "speaker", 1.0, 1);
}


#define ENDEAVOUR_BIOS \
	ROM_REGION( 0x200000, "ifu", 0 ) \
	ROM_LOAD( "2v02s502_ifu.u190", 0x000000, 0x080000, CRC(36122a98) SHA1(3d2c40c9d504358d890364e26c9562e40314d8a4) )

#define ENDEAVOUR_UNDUMPED_FLASH \
	ROM_REGION( 0x200000, "prgflash1",    ROMREGION_ERASE00 ) \
	ROM_REGION( 0x200000, "prgflash2",    ROMREGION_ERASE00 ) \
	ROM_REGION( 0x200000, "prgflash3",    ROMREGION_ERASE00 ) \
	ROM_REGION( 0x200000, "prgflash4",    ROMREGION_ERASE00 ) \
	ROM_REGION( 0x200000, "prgflash5",    ROMREGION_ERASE00 ) \
	ROM_REGION( 0x200000, "prgflash6",    ROMREGION_ERASE00 ) \
	ROM_REGION( 0x200000, "prgflash7",    ROMREGION_ERASE00 ) \
	ROM_REGION( 0x200000, "prgflash8",    ROMREGION_ERASE00 ) \
	ROM_REGION( 0x200000, "sndflash1.u8", ROMREGION_ERASE00 ) \
	ROM_REGION( 0x200000, "sndflash2.u7", ROMREGION_ERASE00 ) \
	ROM_REGION( 0x200000, "sndflash3.u6", ROMREGION_ERASE00 ) \
	ROM_REGION( 0x200000, "sndflash4.u5", ROMREGION_ERASE00 )

ROM_START( konendev )
	ENDEAVOUR_BIOS

	ROM_REGION32_BE( 0x200000, "program", ROMREGION_ERASE00 )
	ENDEAVOUR_UNDUMPED_FLASH
	ROM_REGION16_BE( 0x100, "eeprom", ROMREGION_ERASE00 )
ROM_END

// Complete sets

// Amazon Spirit (NSW)
ROM_START( amazonsp )
	ROM_REGION32_BE( 0x200000, "program", 0 )
	ROM_LOAD32_WORD_SWAP( "ams2nb25_01h.u75", 0x00000, 0x080000, CRC(7de70685) SHA1(c04b691fb290bab02e7aaf244da15db50f02319a) )
	ROM_LOAD32_WORD_SWAP( "ams2nb25_02l.u66", 0x00002, 0x080000, CRC(029d4328) SHA1(873bae41ec92b35008b919d12d7c05a4a508245a) )

	ROM_REGION( 0x200000, "ifu", 0 )
	ROM_LOAD( "ifu2.u190", 0x00000, 0x080000, CRC(a4bd533b) SHA1(ed6b0bbaa524a20c7848edcd5aab4a7bd203f63e) )

	ROM_REGION( 0x200000, "prgflash1", ROMREGION_ERASE00 )
	// not populated

	ROM_REGION( 0x200000, "prgflash2", ROMREGION_ERASE00 )
	// not populated

	ROM_REGION( 0x200000, "prgflash3", ROMREGION_ERASE00 )
	// not populated

	ROM_REGION( 0x200000, "prgflash4", ROMREGION_ERASE00 )
	// not populated

	// AMS21111.FMU Chk-GR: 4C3E, SD: D799 AMAZON_S 6 x 2M Konami
	ROM_REGION( 0x200000, "prgflash5", ROMREGION_ERASE00 )
	ROM_LOAD( "amazon_s.c4.u4", 0x000000, 0x200000, CRC(8e1142ab) SHA1(67e4ff742006d4566d55fade451f1cf3b744cecd) )

	ROM_REGION( 0x200000, "prgflash6", ROMREGION_ERASE00 )
	ROM_LOAD( "amazon_s.c3.u3", 0x000000, 0x200000, CRC(0841873e) SHA1(b8e23a17499897eaafabac4239da171f72cb6b89) )

	ROM_REGION( 0x200000, "prgflash7", ROMREGION_ERASE00 )
	ROM_LOAD( "amazon_s.c2.u2", 0x000000, 0x200000, CRC(f6b8a1e7) SHA1(d0ce554bd438b49a1d4cb81cef82db14f1c1904d) )

	ROM_REGION( 0x200000, "prgflash8", ROMREGION_ERASE00 )
	ROM_LOAD( "amazon_s.c1.u1", 0x000000, 0x200000, CRC(5d61cbf2) SHA1(9077705aa733a61e93ef6d6c5bdafb04066f25af) )

	ROM_REGION( 0x200000, "sndflash1.u8", ROMREGION_ERASE00 )
	ROM_LOAD( "amazon_s.a4.u8", 0x000000, 0x200000, CRC(b94d87bf) SHA1(d870db0e81f3289c48e70d500b2a2f9fa1d33cc9) )

	ROM_REGION( 0x200000, "sndflash2.u7", ROMREGION_ERASE00 )
	ROM_LOAD( "amazon_s.a3.u7", 0x000000, 0x200000, CRC(0a00bb2b) SHA1(a068f61d7fbbb4d20dcf468945dd3f5b77d9ec0c) )

	ROM_REGION( 0x200000, "sndflash3.u6", ROMREGION_ERASE00 )
	// not populated

	ROM_REGION( 0x200000, "sndflash4.u5", ROMREGION_ERASE00 )
	// not populated

	ROM_REGION16_BE( 0x100, "eeprom", 0 )
	ROM_LOAD( "93c56.u98", 0x00, 0x100, CRC(b2521a6a) SHA1(f44711545bee7e9c772a3dc23b79f0ea8059ec50) ) // empty EEPROM with Konami header
ROM_END

// Black Rose Rapid Fire Grand Prix (Queensland)
ROM_START( blkrose )
	ROM_REGION32_BE( 0x200000, "program", 0 )
	ROM_LOAD32_WORD_SWAP( "blr8qb16_01h.u75", 0x00000, 0x080000, CRC(693bbc64) SHA1(2988ef414b0a4aa11b20709a497265d8c74343b3) )
	ROM_LOAD32_WORD_SWAP( "blr8qb16_02l.u66", 0x00002, 0x080000, CRC(3999a94e) SHA1(72d4dd2aa15dcff266b0bf7c5dfb54c34b17cb4e) )

	ROM_REGION( 0x200000, "ifu", 0 )
	ROM_LOAD( "2q14prog_ifu.u190", 0x00000, 0x080000, CRC(00e4eb51) SHA1(38c7c28da6d980f9c7447ad31416ccb321c20e25) )

	ROM_REGION( 0x200000, "prgflash1", ROMREGION_ERASE00 )
	// not populated

	ROM_REGION( 0x200000, "prgflash2", ROMREGION_ERASE00 )
	// not populated

	ROM_REGION( 0x200000, "prgflash3", ROMREGION_ERASE00 )
	// not populated

	ROM_REGION( 0x200000, "prgflash4", ROMREGION_ERASE00 )
	// not populated

	// BLR8Q211.FMU Chk-GR: 13BB, SD: E1F4 BROSERGP 6 x 2M Konami
	ROM_REGION( 0x200000, "prgflash5", ROMREGION_ERASE00 )
	ROM_LOAD( "brosergp.c4.u4", 0x000000, 0x200000, CRC(6c82ff96) SHA1(4f0511e4b700180a72144a1609df52c288a58b53) )

	ROM_REGION( 0x200000, "prgflash6", ROMREGION_ERASE00 )
	ROM_LOAD( "brosergp.c3.u3", 0x000000, 0x200000, CRC(7a605e92) SHA1(365b2ad5df98c09644c998bbc7a8721b0a9524eb) )

	ROM_REGION( 0x200000, "prgflash7", ROMREGION_ERASE00 )
	ROM_LOAD( "brosergp.c2.u2", 0x000000, 0x200000, CRC(f3367411) SHA1(92840d07909e657a560d81783fd6daf2c353fae3) )

	ROM_REGION( 0x200000, "prgflash8", ROMREGION_ERASE00 )
	ROM_LOAD( "brosergp.c1.u1", 0x000000, 0x200000, CRC(44bb6808) SHA1(0e7e5837083bb51844c370e4e11e588f782d79b8) )

	ROM_REGION( 0x200000, "sndflash1.u8", ROMREGION_ERASE00 )
	ROM_LOAD( "brosergp.a4.u8", 0x000000, 0x200000, CRC(f89dbb3b) SHA1(08495770597cb91245251adc74d7a1597a95b0c9) ) // same as rapfire5

	ROM_REGION( 0x200000, "sndflash2.u7", ROMREGION_ERASE00 )
	ROM_LOAD( "brosergp.a3.u7", 0x000000, 0x200000, CRC(cf212581) SHA1(5959e73c36cb678de0a814bc699afcfd69199124) )

	ROM_REGION( 0x200000, "sndflash3.u6", ROMREGION_ERASE00 )
	// not populated

	ROM_REGION( 0x200000, "sndflash4.u5", ROMREGION_ERASE00 )
ROM_END

// Enchanted Lamp (Russia)
ROM_START( enchlamp ) // the flash dumps have been split from an aggregated dump. They pass the checksum so they are assumed good, but a redump wouldn't hurt, either.
	ENDEAVOUR_BIOS

	ROM_REGION32_BE( 0x200000, "program", 0 )
	ROM_LOAD32_WORD_SWAP( "enl5rg26_01h.u75", 0x00000, 0x100000, CRC(fed5b988) SHA1(49442decd9b40f0a382c4fc7b231958f526ddbd1) )
	ROM_LOAD32_WORD_SWAP( "enl5rg26_02l.u66", 0x00002, 0x100000, CRC(d0e42c9f) SHA1(10ff944ec0a626d47ec12be291ff5fe001342ed4) )

	ROM_REGION( 0x200000, "prgflash1", ROMREGION_ERASE00 )
	ROM_LOAD( "prgflash1", 0x000000, 0x200000, CRC(575327f8) SHA1(3a05e66c0323d92121d33b4d8b9072b81a25b053) )

	ROM_REGION( 0x200000, "prgflash2", ROMREGION_ERASE00 )
	ROM_LOAD( "prgflash2", 0x000000, 0x200000, CRC(6a3c0df4) SHA1(1c4fc6f6300ced8e22e79cff28627411b3b3b0f2) )

	ROM_REGION( 0x200000, "prgflash3", ROMREGION_ERASE00 )
	ROM_LOAD( "prgflash3", 0x000000, 0x200000, CRC(8a02ca3f) SHA1(4692cfa25b6eb2e3e055711bd6a741c4fb1a5b0d) )

	ROM_REGION( 0x200000, "prgflash4", ROMREGION_ERASE00 )
	ROM_LOAD( "prgflash4", 0x000000, 0x200000, CRC(04a37ed2) SHA1(5dc255d71588d39fe563fc5bb0c3e1f5b1ee7c89) )

	ROM_REGION( 0x200000, "prgflash5", ROMREGION_ERASE00 )
	// not populated

	ROM_REGION( 0x200000, "prgflash6", ROMREGION_ERASE00 )
	// not populated

	ROM_REGION( 0x200000, "prgflash7", ROMREGION_ERASE00 )
	// not populated

	ROM_REGION( 0x200000, "prgflash8", ROMREGION_ERASE00 )
	// not populated

	ROM_REGION( 0x200000, "sndflash1.u8", ROMREGION_ERASE00 )
	ROM_LOAD( "sndflash1", 0x000000, 0x200000, CRC(2b4dc7d4) SHA1(77c1d1baa20e29de22c0d710949987ef5fff8c78) )

	ROM_REGION( 0x200000, "sndflash2.u7", ROMREGION_ERASE00 )
	ROM_LOAD( "sndflash2", 0x000000, 0x200000, CRC(59ddc211) SHA1(5c6b630df10b3d49ce72a80b402355fa23fa7324) )

	ROM_REGION( 0x200000, "sndflash3.u6", ROMREGION_ERASE00 )
	// not populated

	ROM_REGION( 0x200000, "sndflash4.u5", ROMREGION_ERASE00 )
	// not populated

	ROM_REGION16_BE( 0x100, "eeprom", 0 )
	ROM_LOAD( "93c56.u98", 0x00, 0x100, CRC(b2521a6a) SHA1(f44711545bee7e9c772a3dc23b79f0ea8059ec50) ) // empty EEPROM with Konami header
ROM_END

// Incan Pyramid (Queensland)
ROM_START( incanpq )
	ROM_REGION32_BE( 0x200000, "program", 0 )
	ROM_LOAD32_WORD_SWAP( "es_inca_pyramids.u75", 0x00000, 0x080000, CRC(0a3d3243) SHA1(9016ddd1c100f8dc2f4713e90febf4081ac2d4db) )
	ROM_LOAD32_WORD_SWAP( "es_inca_pyramids.u66", 0x00002, 0x080000, CRC(a38f1c74) SHA1(7364e1257399571cf8145f047982ce0f6fb7ffb8) )

	ROM_REGION( 0x200000, "ifu", 0 )
	ROM_LOAD( "2q14prog_ifu.u190", 0x00000, 0x080000, CRC(00e4eb51) SHA1(38c7c28da6d980f9c7447ad31416ccb321c20e25) ) // same as blkrose

	ROM_REGION( 0x200000, "prgflash1", ROMREGION_ERASE00 )
	// not populated

	ROM_REGION( 0x200000, "prgflash2", ROMREGION_ERASE00 )
	// not populated

	ROM_REGION( 0x200000, "prgflash3", ROMREGION_ERASE00 )
	// not populated

	ROM_REGION( 0x200000, "prgflash4", ROMREGION_ERASE00 )
	// not populated

	ROM_REGION( 0x200000, "prgflash5", ROMREGION_ERASE00 )
	ROM_LOAD( "es_inca_pyramids.u4", 0x000000, 0x200000, CRC(c282a937) SHA1(41b44ff60bdfea410e197a882a7ab021aee874a4) )

	ROM_REGION( 0x200000, "prgflash6", ROMREGION_ERASE00 )
	ROM_LOAD( "es_inca_pyramids.u3", 0x000000, 0x200000, CRC(800fd71e) SHA1(c0899c713a4a3644c2f97a962706c6a1a60f692d) )

	ROM_REGION( 0x200000, "prgflash7", ROMREGION_ERASE00 )
	ROM_LOAD( "es_inca_pyramids.u2", 0x000000, 0x200000, CRC(148fd426) SHA1(c41de98d8df24d3fc1a613f83d1234da829eac73) )

	ROM_REGION( 0x200000, "prgflash8", ROMREGION_ERASE00 )
	ROM_LOAD( "es_inca_pyramids.u1", 0x000000, 0x200000, CRC(80bedf60) SHA1(ab756c340e7b13a4ba4a4035b5bbecb5bf40d4b9) )

	ROM_REGION( 0x200000, "sndflash1.u8", ROMREGION_ERASE00 ) // same as Amazon Spirit
	ROM_LOAD( "es_inca_pyramids.u8", 0x000000, 0x200000, CRC(b94d87bf) SHA1(d870db0e81f3289c48e70d500b2a2f9fa1d33cc9) )

	ROM_REGION( 0x200000, "sndflash2.u7", ROMREGION_ERASE00 ) // same as Amazon Spirit
	ROM_LOAD( "es_inca_pyramids.u7", 0x000000, 0x200000, CRC(0a00bb2b) SHA1(a068f61d7fbbb4d20dcf468945dd3f5b77d9ec0c) )

	ROM_REGION( 0x200000, "sndflash3.u6", ROMREGION_ERASE00 )
	// not populated

	ROM_REGION( 0x200000, "sndflash4.u5", ROMREGION_ERASE00 )
	// not populated

	ROM_REGION16_BE( 0x100, "eeprom", 0 )
	ROM_LOAD( "93c56.u98", 0x00, 0x100, CRC(b2521a6a) SHA1(f44711545bee7e9c772a3dc23b79f0ea8059ec50) ) // empty EEPROM with Konami header
ROM_END

// Rapid Fire 5 (NSW)
// Same game as Black Rose
ROM_START( rapfire5 )
	ROM_REGION32_BE( 0x200000, "program", 0 )
	ROM_LOAD32_WORD_SWAP( "r056na12_01h.u75", 0x00000, 0x080000, CRC(bed72d6c) SHA1(2bc4d88ed62aaed9fb0c75ee7153c81d8e6f38d9) )
	ROM_LOAD32_WORD_SWAP( "r056na12_02l.u66", 0x00002, 0x080000, CRC(bf17c88f) SHA1(b9b3448b4accf676c9d60643270c045ffe3c59f9) )

	ROM_REGION( 0x200000, "ifu", 0 )
	ROM_LOAD( "2n12prog_ifu.u190", 0x00000, 0x080000, CRC(c9c4ac89) SHA1(8eebda327892d00951355a86e927fa2e4ad3c9a0) )

	// R0561111.FMU Chk-GR: 1CC6, SD: 6FEE R_FIRE05 6 x 2M Konami
	// once concatenated, dumps below match the above checksum16s
	ROM_REGION( 0x200000, "prgflash1", ROMREGION_ERASE00 )
	// not populated

	ROM_REGION( 0x200000, "prgflash2", ROMREGION_ERASE00 )
	// not populated

	ROM_REGION( 0x200000, "prgflash3", ROMREGION_ERASE00 )
	// not populated

	ROM_REGION( 0x200000, "prgflash4", ROMREGION_ERASE00 )
	// not populated

	ROM_REGION( 0x200000, "prgflash5", ROMREGION_ERASE00 )
	ROM_LOAD( "r_fire05.c4.u4", 0x000000, 0x200000, CRC(08d31cb1) SHA1(1f0f05f078befcdb79c4d42d39a7ab5438b7bda3) )

	ROM_REGION( 0x200000, "prgflash6", ROMREGION_ERASE00 )
	ROM_LOAD( "r_fire05.c3.u3", 0x000000, 0x200000, CRC(1bf7ed0f) SHA1(664fe15c577f46ae1a17ad1d75c5c3c1cd3c01c9) )

	ROM_REGION( 0x200000, "prgflash7", ROMREGION_ERASE00 )
	ROM_LOAD( "r_fire05.c2.u2", 0x000000, 0x200000, CRC(c9282734) SHA1(11f5d30bfc6a971f6f8ad71f8f26582931442ec9) )

	ROM_REGION( 0x200000, "prgflash8", ROMREGION_ERASE00 )
	ROM_LOAD( "r_fire05.c1.u1", 0x000000, 0x200000, CRC(ad41d7a5) SHA1(f3ba22228e5699185a329508a1a3291e352e858d) )

	ROM_REGION( 0x200000, "sndflash1.u8", ROMREGION_ERASE00 )
	ROM_LOAD( "r_fire05.a4.u8", 0x000000, 0x200000, CRC(f89dbb3b) SHA1(08495770597cb91245251adc74d7a1597a95b0c9) )

	ROM_REGION( 0x200000, "sndflash2.u7", ROMREGION_ERASE00 )
	ROM_LOAD( "r_fire05.a3.u7", 0x000000, 0x200000, CRC(26d365d3) SHA1(7dfeeb0880d917b54b89694dfe434577d64fad90) )

	ROM_REGION( 0x200000, "sndflash3.u6", ROMREGION_ERASE00 )
	// not populated

	ROM_REGION( 0x200000, "sndflash4.u5", ROMREGION_ERASE00 )
	// not populated

	ROM_REGION16_BE( 0x100, "eeprom", 0 )
	ROM_LOAD( "93c56.u98", 0x00, 0x100, CRC(b2521a6a) SHA1(f44711545bee7e9c772a3dc23b79f0ea8059ec50) ) // empty EEPROM with Konami header
ROM_END

// Safe Money (NSW)
ROM_START( safemonn )
	ROM_REGION32_BE( 0x200000, "program", 0 )
	ROM_LOAD32_WORD_SWAP( "sam2ng12_01h.u75", 0x00000, 0x080000, CRC(33a21093) SHA1(e9f4257b70b17f29c2a91b85e0ff4af98fa98b63) )
	ROM_LOAD32_WORD_SWAP( "sam2ng12_02l.u66", 0x00002, 0x080000, CRC(4b69944c) SHA1(2e7205e3471c97b38e558c33c3b0bb1e593354ce) )

	ROM_REGION( 0x200000, "ifu", 0 )
	ROM_LOAD( "ifu2.u190", 0x00000, 0x080000, CRC(a4bd533b) SHA1(ed6b0bbaa524a20c7848edcd5aab4a7bd203f63e) )

	ROM_REGION( 0x200000, "prgflash1", ROMREGION_ERASE00 )
	// not populated

	ROM_REGION( 0x200000, "prgflash2", ROMREGION_ERASE00 )
	// not populated

	ROM_REGION( 0x200000, "prgflash3", ROMREGION_ERASE00 )
	// not populated

	ROM_REGION( 0x200000, "prgflash4", ROMREGION_ERASE00 )
	// not populated

	// SAM21111.FMU Chk-GR: 8FA4, SD: D85D SAFMONEY 6 x 2M Konami
	ROM_REGION( 0x200000, "prgflash5", ROMREGION_ERASE00 )
	ROM_LOAD( "safmoney.c4.u4", 0x000000, 0x200000, CRC(1706b876) SHA1(c3e53e2a8cab4bd6027eca2f5ccef00424b025ef) )

	ROM_REGION( 0x200000, "prgflash6", ROMREGION_ERASE00 )
	ROM_LOAD( "safmoney.c3.u3", 0x000000, 0x200000, CRC(6ac3a8e6) SHA1(3a1dcc67af92ed84a8e61f3b884364a77d10ad2d) )

	ROM_REGION( 0x200000, "prgflash7", ROMREGION_ERASE00 )
	ROM_LOAD( "safmoney.c2.u2", 0x000000, 0x200000, CRC(3fdbd6f4) SHA1(d864534370a6f636e6f011779fa8eddf5579f155) )

	ROM_REGION( 0x200000, "prgflash8", ROMREGION_ERASE00 )
	ROM_LOAD( "safmoney.c1.u1", 0x000000, 0x200000, CRC(193e47c1) SHA1(aeaa5d3aad784b2326cd8425118c8544f5c1fcc2) )

	ROM_REGION( 0x200000, "sndflash1.u8", ROMREGION_ERASE00 )
	ROM_LOAD( "safmoney.a4.u8", 0x000000, 0x200000, CRC(bbf4f9db) SHA1(b2e12d6b84c31cc9180c976134cbf2b602d540d2) )

	ROM_REGION( 0x200000, "sndflash2.u7", ROMREGION_ERASE00 )
	ROM_LOAD( "safmoney.a3.u7", 0x000000, 0x200000, CRC(a991cf14) SHA1(d0d9086c108e76441307f71d276d59681dc49853) )

	ROM_REGION( 0x200000, "sndflash3.u6", ROMREGION_ERASE00 )
	// not populated

	ROM_REGION( 0x200000, "sndflash4.u5", ROMREGION_ERASE00 )
	// not populated

	ROM_REGION16_BE( 0x100, "eeprom", 0 )
	ROM_LOAD( "93c56.u98", 0x00, 0x100, CRC(b2521a6a) SHA1(f44711545bee7e9c772a3dc23b79f0ea8059ec50) ) // empty EEPROM with Konami header
ROM_END

// No flash dumped

// African Adventure (Russia)
ROM_START( aadvent )
	ENDEAVOUR_BIOS

	ROM_REGION32_BE( 0x200000, "program", 0 )
	ROM_LOAD32_WORD_SWAP( "afa5re26_01h.u75", 0x00000, 0x100000, CRC(65ce6f7a) SHA1(018742f13fea4c52f822e7f12e8efd0aff61a713) )
	ROM_LOAD32_WORD_SWAP( "afa5re26_02l.u66", 0x00002, 0x100000, CRC(73945b3a) SHA1(5ace9c439048f3555fe631917c15bee76362e784) )

	ENDEAVOUR_UNDUMPED_FLASH
ROM_END

// Dragonfly (Russia)
ROM_START( dragnfly )
	ENDEAVOUR_BIOS

	ROM_REGION32_BE( 0x200000, "program", 0 )
	ROM_LOAD32_WORD_SWAP( "drf5re26_01h.u75", 0x00000, 0x100000, CRC(ef6f1b69) SHA1(007a41cd1b08705184f69ce3e0e6c63bc2301e25) )
	ROM_LOAD32_WORD_SWAP( "drf5re26_02l.u66", 0x00002, 0x100000, CRC(00e00c29) SHA1(a92d7220bf46655222ddc5d1c276dc469343f4c5) )

	ENDEAVOUR_UNDUMPED_FLASH
ROM_END

// Gypsy Magic (Russia)
ROM_START( gypmagic )
	ENDEAVOUR_BIOS

	ROM_REGION32_BE( 0x200000, "program", 0 )
	ROM_LOAD32_WORD_SWAP( "gym5rc26_01h.u75", 0x00000, 0x080000, CRC(8643be94) SHA1(fc63872a55ac2229652566bd9795ce9bf8442fee) )
	ROM_LOAD32_WORD_SWAP( "gym5rc26_02l.u66", 0x00002, 0x080000, CRC(4ee33c46) SHA1(9e0ef66e9d53a47827d04e6a89d13d37429e0c16) )

	ENDEAVOUR_UNDUMPED_FLASH
ROM_END

// Incan Pyramid (Russia)
ROM_START( incanp )
	ENDEAVOUR_BIOS

	ROM_REGION32_BE( 0x200000, "program", 0 )
	ROM_LOAD32_WORD_SWAP( "inp5rg26_01h.u75", 0x00000, 0x100000, CRC(8434222e) SHA1(d03710e18f5b9e45db32685778a21a5dc598d043) )
	ROM_LOAD32_WORD_SWAP( "inp5rg26_02l.u66", 0x00002, 0x100000, CRC(50c37109) SHA1(a638587f37f63b3f63ee51f541d991c3784c09f7) )

	ENDEAVOUR_UNDUMPED_FLASH
ROM_END

// Jester Magic (Russia)
ROM_START( jestmagi )
	ENDEAVOUR_BIOS

	ROM_REGION32_BE( 0x200000, "program", 0 )
	ROM_LOAD32_WORD_SWAP( "jem5rc26_01h.u75", 0x00000, 0x080000, CRC(9145324c) SHA1(366baa22bde1b8da19dba756829305d0fd69b4ff) )
	ROM_LOAD32_WORD_SWAP( "jem5rc26_02l.u66", 0x00002, 0x080000, CRC(cb49f466) SHA1(e3987de2e640fe8116d66d2c1755e6500dedf8a5) )

	ENDEAVOUR_UNDUMPED_FLASH
ROM_END

// Lucky Fountain (Russia)
ROM_START( luckfoun )
	ENDEAVOUR_BIOS

	ROM_REGION32_BE( 0x200000, "program", 0 )
	ROM_LOAD32_WORD_SWAP( "luf5rd26_01h.u75", 0x00000, 0x080000, CRC(68b3d50a) SHA1(9b3d2a9f5d72db091e79b036017bd5d07f9fed00) )
	ROM_LOAD32_WORD_SWAP( "luf5rd26_02l.u66", 0x00002, 0x080000, CRC(e7e9b8cd) SHA1(d8c421b0d58775f5a0ccae6395a604091b0acf1d) )

	ENDEAVOUR_UNDUMPED_FLASH
ROM_END

// Mohican Sun (Russia)
ROM_START( mohicans )
	ENDEAVOUR_BIOS

	ROM_REGION32_BE( 0x200000, "program", 0 )
	ROM_LOAD32_WORD_SWAP( "moh5rf26_01h.u75", 0x00000, 0x100000, CRC(527dda20) SHA1(0a71484421738517c17d76e9bf92943b57cc4cc8) )
	ROM_LOAD32_WORD_SWAP( "moh5rf26_02l.u66", 0x00002, 0x100000, CRC(a9bd3846) SHA1(02d80ff6c20e3732ae582de5d4392d4d6d8ba955) )

	ENDEAVOUR_UNDUMPED_FLASH
ROM_END

// The Monster Show (Russia)
ROM_START( monshow )
	ENDEAVOUR_BIOS

	ROM_REGION32_BE( 0x200000, "program", 0 )
	ROM_LOAD32_WORD_SWAP( "tms5rc26_01h.u75", 0x00000, 0x100000, CRC(8209aafe) SHA1(e48a0524ad93a9b657d3efe67f7b5e1067b37e48) )
	ROM_LOAD32_WORD_SWAP( "tms5rc26_02l.u66", 0x00002, 0x100000, CRC(78de8c59) SHA1(ad73bc926f5874d257171dfa6b727cb31e33bce9) )

	ENDEAVOUR_UNDUMPED_FLASH
ROM_END

// Roman Legion (Russia)
ROM_START( romanl )
	ENDEAVOUR_BIOS

	ROM_REGION32_BE( 0x200000, "program", 0 )
	ROM_LOAD32_WORD_SWAP( "rol5rg26_01h.u75", 0x00000, 0x100000, CRC(d441d30c) SHA1(025111699a7e29781bbb4d0f4151c808e3d06235) )
	ROM_LOAD32_WORD_SWAP( "rol5rg26_02l.u66", 0x00002, 0x100000, CRC(08bd72ca) SHA1(a082cffeb1bccc8ec468a618eaabba7dac89882c) )

	ENDEAVOUR_UNDUMPED_FLASH
ROM_END

// Safe Money (Russia)
ROM_START( safemon )
	ENDEAVOUR_BIOS

	ROM_REGION32_BE( 0x200000, "program", 0 )
	ROM_LOAD32_WORD_SWAP( "sam5rj26_01h.u75", 0x00000, 0x080000, CRC(7f82693f) SHA1(1c8540d209ab17f4fca5ff74bc687c83ec315208) )
	ROM_LOAD32_WORD_SWAP( "sam5rj26_02l.u66", 0x00002, 0x080000, CRC(73bd981e) SHA1(f01b97201bd877c601cf3c742a6e0963de8e48dc) )

	ENDEAVOUR_UNDUMPED_FLASH
ROM_END

// Show Queen (Russia)
ROM_START( showqn )
	ENDEAVOUR_BIOS

	ROM_REGION32_BE( 0x200000, "program", 0 )
	ROM_LOAD32_WORD_SWAP( "shq5rc26_01h.u75", 0x00000, 0x080000, CRC(3fc44415) SHA1(f0be1b90a2a374f9fb9e059e834bbdbf714b6607) )
	ROM_LOAD32_WORD_SWAP( "shq5rc26_02l.u66", 0x00002, 0x080000, CRC(38a03281) SHA1(1b4552b0ce347df4d87e398111bbf72f126a8ec1) )

	ENDEAVOUR_UNDUMPED_FLASH
ROM_END

// Spice It Up (Russia)
ROM_START( spiceup )
	ENDEAVOUR_BIOS

	ROM_REGION32_BE( 0x200000, "program", 0 )
	ROM_LOAD32_WORD_SWAP( "siu5rc26_01h.u75", 0x00000, 0x100000, CRC(373bc2b1) SHA1(af3740fdcd028f162440701c952a3a87805bc65b) )
	ROM_LOAD32_WORD_SWAP( "siu5rc26_02l.u66", 0x00002, 0x100000, CRC(2e584321) SHA1(ca98092dde76338117e989e774db2db672d87bfa) )

	ENDEAVOUR_UNDUMPED_FLASH
ROM_END

// Sultan's Wish (Russia)
ROM_START( sultanw )
	ENDEAVOUR_BIOS

	ROM_REGION32_BE( 0x200000, "program", 0 )
	ROM_LOAD32_WORD_SWAP( "suw5rc26_01h.u75", 0x00000, 0x100000, CRC(27760529) SHA1(b8970a706df52ee5792bbd7a4e719f2be87662ac) )
	ROM_LOAD32_WORD_SWAP( "suw5rc26_02l.u66", 0x00002, 0x100000, CRC(1c98fd4d) SHA1(58ff948c0deba0bffb8866b15f46518524516501) )

	ENDEAVOUR_UNDUMPED_FLASH
ROM_END

// White Russia (Russia)
ROM_START( whiterus )
	ENDEAVOUR_BIOS

	ROM_REGION32_BE( 0x200000, "program", 0 )
	ROM_LOAD32_WORD_SWAP( "whr5ra26_01h.u75", 0x00000, 0x080000, CRC(d5a1ebb6) SHA1(14a8d1d8f8ae8919eaa878660c7e97e7ea7a02d8) )
	ROM_LOAD32_WORD_SWAP( "whr5ra26_02l.u66", 0x00002, 0x080000, CRC(48a2277c) SHA1(965d1da31e3bcde6fda4e15e8980a69e8bce5a84) )

	ENDEAVOUR_UNDUMPED_FLASH
ROM_END

// "Zero" is not the actual name, the ROMs are clear chips and have no name string
ROM_START( konzero )
	ROM_REGION32_BE( 0x200000, "program", 0 )
	ROM_LOAD32_WORD_SWAP( "low2na35_01h.u75", 0x00000, 0x080000, CRC(b9237061) SHA1(0eb311e8e1c872d6a9c38726efb17ddf4713bc7d) )
	ROM_LOAD32_WORD_SWAP( "low2na35_02l.u66", 0x00002, 0x080000, CRC(2806299c) SHA1(a069f4477b310f99ff1ff48f622dc30862589127) )

	ENDEAVOUR_UNDUMPED_FLASH
	// unsure whether this set actually has flash data, given that these chips would be replacing the game chips when resetting the memory

	ROM_REGION( 0x200000, "ifu", 0 )
	ROM_LOAD( "2n11prog_ifu.u190", 0x000000, 0x080000, NO_DUMP )

	ROM_REGION16_BE( 0x100, "eeprom", 0 )
	ROM_LOAD( "93c56.u98", 0x00, 0x100, CRC(b2521a6a) SHA1(f44711545bee7e9c772a3dc23b79f0ea8059ec50) ) // empty EEPROM with Konami header
ROM_END

} // anonymous namespace


// BIOS
GAME( 200?, konendev, 0,        konendev, konendev, konendev_state, empty_init, ROT0, "Konami", "Konami Endeavour BIOS",                                           MACHINE_NOT_WORKING | MACHINE_IS_BIOS_ROOT )

// have flash dump
GAME( 200?, amazonsp, 0,        konendev, konendev, konendev_state, empty_init, ROT0, "Konami", "Amazon Spirit (Konami Endeavour, NSW)",                           MACHINE_NOT_WORKING )
GAME( 200?, blkrose,  0,        konendev, konendev, konendev_state, empty_init, ROT0, "Konami", "Black Rose Rapid Fire Grand Prix (Konami Endeavour, Queensland)", MACHINE_NOT_WORKING )
GAME( 200?, enchlamp, konendev, konendev, konendev, konendev_state, empty_init, ROT0, "Konami", "Enchanted Lamp (Konami Endeavour, Russia)",                       MACHINE_NOT_WORKING )
GAME( 200?, incanpq,  incanp,   konendev, konendev, konendev_state, empty_init, ROT0, "Konami", "Incan Pyramid (Konami Endeavour, Queensland)",                    MACHINE_NOT_WORKING )
GAME( 200?, rapfire5, 0,        konendev, konendev, konendev_state, empty_init, ROT0, "Konami", "Rapid Fire 5 (Konami Endeavour, NSW)",                            MACHINE_NOT_WORKING )
GAME( 200?, safemonn, safemon,  konendev, konendev, konendev_state, empty_init, ROT0, "Konami", "Safe Money (Konami Endeavour, NSW)",                              MACHINE_NOT_WORKING )

// missing flash
GAME( 200?, aadvent,  konendev, konendev, konendev, konendev_state, empty_init, ROT0, "Konami", "African Adventure (Konami Endeavour, Russia)",                    MACHINE_NOT_WORKING )
GAME( 200?, dragnfly, konendev, konendev, konendev, konendev_state, empty_init, ROT0, "Konami", "Dragonfly (Konami Endeavour, Russia)",                            MACHINE_NOT_WORKING )
GAME( 200?, gypmagic, konendev, konendev, konendev, konendev_state, empty_init, ROT0, "Konami", "Gypsy Magic (Konami Endeavour, Russia)",                          MACHINE_NOT_WORKING )
GAME( 200?, incanp,   konendev, konendev, konendev, konendev_state, empty_init, ROT0, "Konami", "Incan Pyramid (Konami Endeavour, Russia)",                        MACHINE_NOT_WORKING )
GAME( 200?, jestmagi, konendev, konendev, konendev, konendev_state, empty_init, ROT0, "Konami", "Jester Magic (Konami Endeavour, Russia)",                         MACHINE_NOT_WORKING )
GAME( 200?, luckfoun, konendev, konendev, konendev, konendev_state, empty_init, ROT0, "Konami", "Lucky Fountain (Konami Endeavour, Russia)",                       MACHINE_NOT_WORKING )
GAME( 200?, mohicans, konendev, konendev, konendev, konendev_state, empty_init, ROT0, "Konami", "Mohican Sun (Konami Endeavour, Russia)",                          MACHINE_NOT_WORKING )
GAME( 200?, monshow,  konendev, konendev, konendev, konendev_state, empty_init, ROT0, "Konami", "The Monster Show (Konami Endeavour, Russia)",                     MACHINE_NOT_WORKING )
GAME( 200?, romanl,   konendev, konendev, konendev, konendev_state, empty_init, ROT0, "Konami", "Roman Legions (Konami Endeavour, Russia)",                        MACHINE_NOT_WORKING )
GAME( 200?, safemon,  konendev, konendev, konendev, konendev_state, empty_init, ROT0, "Konami", "Safe Money (Konami Endeavour, Russia)",                           MACHINE_NOT_WORKING )
GAME( 200?, showqn,   konendev, konendev, konendev, konendev_state, empty_init, ROT0, "Konami", "Show Queen (Konami Endeavour, Russia)",                           MACHINE_NOT_WORKING )
GAME( 200?, spiceup,  konendev, konendev, konendev, konendev_state, empty_init, ROT0, "Konami", "Spice It Up (Konami Endeavour, Russia)",                          MACHINE_NOT_WORKING )
GAME( 200?, sultanw,  konendev, konendev, konendev, konendev_state, empty_init, ROT0, "Konami", "Sultan's Wish (Konami Endeavour, Russia)",                        MACHINE_NOT_WORKING )
GAME( 200?, whiterus, konendev, konendev, konendev, konendev_state, empty_init, ROT0, "Konami", "White Russia (Konami Endeavour, Russia)",                         MACHINE_NOT_WORKING )

// clear chip (not a game)
GAME( 200?, konzero,  0,        konendev, konendev, konendev_state, empty_init, ROT0, "Konami", "Zero (Konami Endeavour)",                                         MACHINE_NOT_WORKING )
