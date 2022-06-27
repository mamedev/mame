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

    The games use flash memory for resources, however only Enchanted Lamp has its flash data dumped thus the others can't boot.

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
    IFU2 Failure
    LCD Display Disconnected (when enchlamp is set to Wild Fire jackpot mode)
*/

#include "emu.h"
#include "k057714.h"

#include "cpu/h8/h83006.h"
#include "cpu/powerpc/ppc.h"
#include "machine/eepromser.h"
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
		, m_rtc(*this, "rtc")
		, m_dpram(*this, "dpram")
	{ }

	void init_konendev();
	void init_enchlamp();

	void konendev(machine_config &config);

	virtual void machine_start() override
	{
		m_dpram_base = (uint16_t *)m_dpram.target();
	}

private:
	uint32_t mcu2_r(offs_t offset, uint32_t mem_mask = ~0);
	uint32_t ifu2_r(offs_t offset, uint32_t mem_mask = ~0);
	void eeprom_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t sound_data_r();
	void sound_data_w(uint32_t data);

	uint16_t ifu_unk_r();
	uint16_t ifu_dpram_r(offs_t offset);
	void ifu_dpram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	DECLARE_WRITE_LINE_MEMBER(gcu_interrupt);
	INTERRUPT_GEN_MEMBER(vbl_interrupt);

	uint8_t rtc_dev_r(uint32_t reg);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void konendev_map(address_map &map);
	void ifu_map(address_map &map);

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<h83007_device> m_ifu;
	required_device<k057714_device> m_gcu;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<rtc62423_device> m_rtc;
	required_shared_ptr<uint32_t> m_dpram;

	uint16_t *m_dpram_base = nullptr;
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
		r |= ioport("DSW")->read() & 0xff;
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

uint32_t konendev_state::sound_data_r()
{
	return 0xffffffff;
}

void konendev_state::sound_data_w(uint32_t data)
{
}

void konendev_state::konendev_map(address_map &map)
{
	map(0x00000000, 0x00ffffff).ram();
	map(0x78000000, 0x78000003).r(FUNC(konendev_state::mcu2_r));
	map(0x78080000, 0x7808000f).rw(m_rtc, FUNC(rtc62423_device::read), FUNC(rtc62423_device::write));
	map(0x780c0000, 0x780c0003).rw(FUNC(konendev_state::sound_data_r), FUNC(konendev_state::sound_data_w));
	map(0x78100000, 0x78100003).w(FUNC(konendev_state::eeprom_w));
	map(0x78800000, 0x78800003).r(FUNC(konendev_state::ifu2_r));
	map(0x78800004, 0x78800007).portr("IN1"); // doors, switches
	map(0x78a00000, 0x78a00003).portr("IN2"); // hard meter access, hopper
	map(0x78a00004, 0x78a00007).portr("IN3"); // main door optic
	map(0x78c00000, 0x78c003ff).ram().share("dpram");
	map(0x78e00000, 0x78e00003).portr("IN0"); // buttons
	map(0x79000000, 0x79000003).w(m_gcu, FUNC(k057714_device::fifo_w));
	map(0x79800000, 0x798000ff).rw(m_gcu, FUNC(k057714_device::read), FUNC(k057714_device::write));
	map(0x7a000000, 0x7a01ffff).ram().share("nvram0");
	map(0x7a100000, 0x7a11ffff).ram().share("nvram1");
	map(0x7e000000, 0x7f7fffff).rom().region("flash", 0);
	map(0x7ff00000, 0x7fffffff).rom().region("program", 0);
}

uint16_t konendev_state::ifu_unk_r()
{
	return 0xc3c3;  // H8 program crashes immediately if it doesn't see
}

uint16_t konendev_state::ifu_dpram_r(offs_t offset)
{
	return m_dpram_base[offset];
}

void konendev_state::ifu_dpram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_dpram_base[offset]);
}

void konendev_state::ifu_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom().region("ifu", 0);
	map(0x200000, 0x20ffff).ram();
	map(0x210000, 0x217fff).ram();
	map(0x800000, 0x8003ff).rw(FUNC(konendev_state::ifu_dpram_r), FUNC(konendev_state::ifu_dpram_w));
	map(0xa40000, 0xa40001).nopw();
	map(0xfee010, 0xfee011).r(FUNC(konendev_state::ifu_unk_r));
};

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


WRITE_LINE_MEMBER(konendev_state::gcu_interrupt)
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ1, state);
	m_maincpu->set_input_line(INPUT_LINE_IRQ3, state);
}


INTERRUPT_GEN_MEMBER(konendev_state::vbl_interrupt)
{
	device.execute().set_input_line(INPUT_LINE_IRQ1, ASSERT_LINE);
	device.execute().set_input_line(INPUT_LINE_IRQ3, ASSERT_LINE);
}

void konendev_state::konendev(machine_config &config)
{
	/* basic machine hardware */
	PPC403GCX(config, m_maincpu, 32000000); // Clock unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &konendev_state::konendev_map);
	m_maincpu->set_vblank_int("screen", FUNC(konendev_state::vbl_interrupt));

	H83007(config, m_ifu, 8000000); // Clock unknown
	m_ifu->set_addrmap(AS_PROGRAM, &konendev_state::ifu_map);

	/* video hardware */
	PALETTE(config, "palette", palette_device::RGB_555);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(25.175_MHz_XTAL, 800, 0, 640, 525, 0, 480); // Based on Firebeat settings
	screen.set_screen_update(FUNC(konendev_state::screen_update));
	screen.set_palette("palette");

	K057714(config, m_gcu, 0).set_screen("screen");
	m_gcu->irq_callback().set(FUNC(konendev_state::gcu_interrupt));

	RTC62423(config, m_rtc, XTAL(32'768));

	NVRAM(config, "nvram0", nvram_device::DEFAULT_ALL_0);
	NVRAM(config, "nvram1", nvram_device::DEFAULT_ALL_0);

	EEPROM_93C56_16BIT(config, "eeprom");

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	ymz280b_device &ymz(YMZ280B(config, "ymz", 16934400)); // Clock unknown
	ymz.add_route(0, "lspeaker", 1.0);
	ymz.add_route(1, "rspeaker", 1.0);
}


#define ENDEAVOUR_BIOS \
	ROM_REGION( 0x200000, "ifu", 0 ) \
	ROM_LOAD( "2v02s502_ifu.u190", 0x000000, 0x080000, CRC(36122a98) SHA1(3d2c40c9d504358d890364e26c9562e40314d8a4) )

ROM_START( konendev )
	ENDEAVOUR_BIOS

	ROM_REGION32_BE( 0x200000, "program", ROMREGION_ERASE00 )
	ROM_REGION32_BE( 0x1800000, "flash", ROMREGION_ERASE00 )
	ROM_REGION16_BE( 0x100, "eeprom", ROMREGION_ERASE00 )
ROM_END

/* Complete sets */

// Enchanted Lamp (Russia)
ROM_START( enchlamp )
	ENDEAVOUR_BIOS

	ROM_REGION32_BE( 0x200000, "program", 0 )
	ROM_LOAD32_WORD_SWAP( "enl5rg26_01h.u75", 0x00000, 0x100000, CRC(fed5b988) SHA1(49442decd9b40f0a382c4fc7b231958f526ddbd1) )
	ROM_LOAD32_WORD_SWAP( "enl5rg26_02l.u66", 0x00002, 0x100000, CRC(d0e42c9f) SHA1(10ff944ec0a626d47ec12be291ff5fe001342ed4) )

	ROM_REGION32_BE( 0x1800000, "flash", ROMREGION_ERASE00 )
	ROM_LOAD( "enl5r211.fmu", 0x0000, 0x1800000, CRC(592c3c7f) SHA1(119b3c6223d656981c399c399d7edccfdbb50dc7) )

	ROM_REGION16_BE( 0x100, "eeprom", 0 )
	ROM_LOAD( "93c56.u98", 0x00, 0x100, CRC(b2521a6a) SHA1(f44711545bee7e9c772a3dc23b79f0ea8059ec50) ) // empty eeprom with Konami header
ROM_END

/* No flash dumped */

// African Adventure (Russia)
ROM_START( aadvent )
	ENDEAVOUR_BIOS

	ROM_REGION32_BE( 0x200000, "program", 0 )
	ROM_LOAD32_WORD_SWAP( "afa5re26_01h.u75", 0x00000, 0x100000, CRC(65ce6f7a) SHA1(018742f13fea4c52f822e7f12e8efd0aff61a713) )
	ROM_LOAD32_WORD_SWAP( "afa5re26_02l.u66", 0x00002, 0x100000, CRC(73945b3a) SHA1(5ace9c439048f3555fe631917c15bee76362e784) )

	ROM_REGION32_BE( 0x1800000, "flash", ROMREGION_ERASE00 )
	ROM_LOAD( "flash", 0x0000, 0x1800000, NO_DUMP )
ROM_END

// Black Rose Rapid Fire Grand Prix (Queensland)
ROM_START( blkrose )
	ROM_REGION32_BE( 0x200000, "program", 0 )
	ROM_LOAD32_WORD_SWAP( "blr8qb16_01h.u75", 0x00000, 0x080000, CRC(693bbc64) SHA1(2988ef414b0a4aa11b20709a497265d8c74343b3) )
	ROM_LOAD32_WORD_SWAP( "blr8qb16_02l.u66", 0x00002, 0x080000, CRC(3999a94e) SHA1(72d4dd2aa15dcff266b0bf7c5dfb54c34b17cb4e) )

	ROM_REGION( 0x200000, "ifu", 0 )
	ROM_LOAD( "2q14prog_ifu.u190", 0x00000, 0x080000, CRC(00e4eb51) SHA1(38c7c28da6d980f9c7447ad31416ccb321c20e25) )

	ROM_REGION32_BE( 0x1800000, "flash", ROMREGION_ERASE00 )
	ROM_LOAD( "blr8q211.fmu", 0x0000, 0x1800000, NO_DUMP ) // BLR8Q211.FMU Chk-GR: 13BB, SD: E1F4 BROSERGP 6 x 2M Konami
ROM_END

// Dragonfly (Russia)
ROM_START( dragnfly )
	ENDEAVOUR_BIOS

	ROM_REGION32_BE( 0x200000, "program", 0 )
	ROM_LOAD32_WORD_SWAP( "drf5re26_01h.u75", 0x00000, 0x100000, CRC(ef6f1b69) SHA1(007a41cd1b08705184f69ce3e0e6c63bc2301e25) )
	ROM_LOAD32_WORD_SWAP( "drf5re26_02l.u66", 0x00002, 0x100000, CRC(00e00c29) SHA1(a92d7220bf46655222ddc5d1c276dc469343f4c5) )

	ROM_REGION32_BE( 0x1800000, "flash", ROMREGION_ERASE00 )
	ROM_LOAD( "flash", 0x0000, 0x1800000, NO_DUMP )
ROM_END

// Gypsy Magic (Russia)
ROM_START( gypmagic )
	ENDEAVOUR_BIOS

	ROM_REGION32_BE( 0x200000, "program", 0 )
	ROM_LOAD32_WORD_SWAP( "gym5rc26_01h.u75", 0x00000, 0x080000, CRC(8643be94) SHA1(fc63872a55ac2229652566bd9795ce9bf8442fee) )
	ROM_LOAD32_WORD_SWAP( "gym5rc26_02l.u66", 0x00002, 0x080000, CRC(4ee33c46) SHA1(9e0ef66e9d53a47827d04e6a89d13d37429e0c16) )

	ROM_REGION32_BE( 0x1800000, "flash", ROMREGION_ERASE00 )
	ROM_LOAD( "flash", 0x0000, 0x1800000, NO_DUMP )
ROM_END

// Incan Pyramid (Russia)
ROM_START( incanp )
	ENDEAVOUR_BIOS

	ROM_REGION32_BE( 0x200000, "program", 0 )
	ROM_LOAD32_WORD_SWAP( "inp5rg26_01h.u75", 0x00000, 0x100000, CRC(8434222e) SHA1(d03710e18f5b9e45db32685778a21a5dc598d043) )
	ROM_LOAD32_WORD_SWAP( "inp5rg26_02l.u66", 0x00002, 0x100000, CRC(50c37109) SHA1(a638587f37f63b3f63ee51f541d991c3784c09f7) )

	ROM_REGION32_BE( 0x1800000, "flash", ROMREGION_ERASE00 )
	ROM_LOAD( "flash", 0x0000, 0x1800000, NO_DUMP )
ROM_END

// Jester Magic (Russia)
ROM_START( jestmagi )
	ENDEAVOUR_BIOS

	ROM_REGION32_BE( 0x200000, "program", 0 )
	ROM_LOAD32_WORD_SWAP( "jem5rc26_01h.u75", 0x00000, 0x080000, CRC(9145324c) SHA1(366baa22bde1b8da19dba756829305d0fd69b4ff) )
	ROM_LOAD32_WORD_SWAP( "jem5rc26_02l.u66", 0x00002, 0x080000, CRC(cb49f466) SHA1(e3987de2e640fe8116d66d2c1755e6500dedf8a5) )

	ROM_REGION32_BE( 0x1800000, "flash", ROMREGION_ERASE00 )
	ROM_LOAD( "flash", 0x0000, 0x1800000, NO_DUMP )
ROM_END

// Lucky Fountain (Russia)
ROM_START( luckfoun )
	ENDEAVOUR_BIOS

	ROM_REGION32_BE( 0x200000, "program", 0 )
	ROM_LOAD32_WORD_SWAP( "luf5rd26_01h.u75", 0x00000, 0x080000, CRC(68b3d50a) SHA1(9b3d2a9f5d72db091e79b036017bd5d07f9fed00) )
	ROM_LOAD32_WORD_SWAP( "luf5rd26_02l.u66", 0x00002, 0x080000, CRC(e7e9b8cd) SHA1(d8c421b0d58775f5a0ccae6395a604091b0acf1d) )

	ROM_REGION32_BE( 0x1800000, "flash", ROMREGION_ERASE00 )
	ROM_LOAD( "flash", 0x0000, 0x1800000, NO_DUMP )
ROM_END

// Mohican Sun (Russia)
ROM_START( mohicans )
	ENDEAVOUR_BIOS

	ROM_REGION32_BE( 0x200000, "program", 0 )
	ROM_LOAD32_WORD_SWAP( "moh5rf26_01h.u75", 0x00000, 0x100000, CRC(527dda20) SHA1(0a71484421738517c17d76e9bf92943b57cc4cc8) )
	ROM_LOAD32_WORD_SWAP( "moh5rf26_02l.u66", 0x00002, 0x100000, CRC(a9bd3846) SHA1(02d80ff6c20e3732ae582de5d4392d4d6d8ba955) )

	ROM_REGION32_BE( 0x1800000, "flash", ROMREGION_ERASE00 )
	ROM_LOAD( "flash", 0x0000, 0x1800000, NO_DUMP )
ROM_END

// The Monster Show (Russia)
ROM_START( monshow )
	ENDEAVOUR_BIOS

	ROM_REGION32_BE( 0x200000, "program", 0 )
	ROM_LOAD32_WORD_SWAP( "tms5rc26_01h.u75", 0x00000, 0x100000, CRC(8209aafe) SHA1(e48a0524ad93a9b657d3efe67f7b5e1067b37e48) )
	ROM_LOAD32_WORD_SWAP( "tms5rc26_02l.u66", 0x00002, 0x100000, CRC(78de8c59) SHA1(ad73bc926f5874d257171dfa6b727cb31e33bce9) )

	ROM_REGION32_BE( 0x1800000, "flash", ROMREGION_ERASE00 )
	ROM_LOAD( "flash", 0x0000, 0x1800000, NO_DUMP )
ROM_END

// Rapid Fire 5 (NSW)
// Same game as Black Rose
ROM_START( rapfire5 )
	ROM_REGION32_BE( 0x200000, "program", 0 )
	ROM_LOAD32_WORD_SWAP( "r056na12_01h.u75", 0x00000, 0x080000, CRC(bed72d6c) SHA1(2bc4d88ed62aaed9fb0c75ee7153c81d8e6f38d9) )
	ROM_LOAD32_WORD_SWAP( "r056na12_02l.u66", 0x00002, 0x080000, CRC(bf17c88f) SHA1(b9b3448b4accf676c9d60643270c045ffe3c59f9) )

	ROM_REGION( 0x200000, "ifu", 0 )
	ROM_LOAD( "2n12prog_ifu.u190", 0x00000, 0x080000, CRC(c9c4ac89) SHA1(8eebda327892d00951355a86e927fa2e4ad3c9a0) )

	ROM_REGION32_BE( 0x1800000, "flash", ROMREGION_ERASE00 )
	ROM_LOAD( "r0561111.fmu", 0x0000, 0x1800000, NO_DUMP ) // R0561111.FMU Chk-GR: 1CC6, SD: 6FEE R_FIRE05 6 x 2M Konami
ROM_END

// Roman Legion (Russia)
ROM_START( romanl )
	ENDEAVOUR_BIOS

	ROM_REGION32_BE( 0x200000, "program", 0 )
	ROM_LOAD32_WORD_SWAP( "rol5rg26_01h.u75", 0x00000, 0x100000, CRC(d441d30c) SHA1(025111699a7e29781bbb4d0f4151c808e3d06235) )
	ROM_LOAD32_WORD_SWAP( "rol5rg26_02l.u66", 0x00002, 0x100000, CRC(08bd72ca) SHA1(a082cffeb1bccc8ec468a618eaabba7dac89882c) )

	ROM_REGION32_BE( 0x1800000, "flash", ROMREGION_ERASE00 )
	ROM_LOAD( "flash", 0x0000, 0x1800000, NO_DUMP )
ROM_END

// Safe Money (Russia)
ROM_START( safemon )
	ENDEAVOUR_BIOS

	ROM_REGION32_BE( 0x200000, "program", 0 )
	ROM_LOAD32_WORD_SWAP( "sam5rj26_01h.u75", 0x00000, 0x080000, CRC(7f82693f) SHA1(1c8540d209ab17f4fca5ff74bc687c83ec315208) )
	ROM_LOAD32_WORD_SWAP( "sam5rj26_02l.u66", 0x00002, 0x080000, CRC(73bd981e) SHA1(f01b97201bd877c601cf3c742a6e0963de8e48dc) )

	ROM_REGION32_BE( 0x1800000, "flash", ROMREGION_ERASE00 )
	ROM_LOAD( "flash", 0x0000, 0x1800000, NO_DUMP )
ROM_END

// Show Queen (Russia)
ROM_START( showqn )
	ENDEAVOUR_BIOS

	ROM_REGION32_BE( 0x200000, "program", 0 )
	ROM_LOAD32_WORD_SWAP( "shq5rc26_01h.u75", 0x00000, 0x080000, CRC(3fc44415) SHA1(f0be1b90a2a374f9fb9e059e834bbdbf714b6607) )
	ROM_LOAD32_WORD_SWAP( "shq5rc26_02l.u66", 0x00002, 0x080000, CRC(38a03281) SHA1(1b4552b0ce347df4d87e398111bbf72f126a8ec1) )

	ROM_REGION32_BE( 0x1800000, "flash", ROMREGION_ERASE00 )
	ROM_LOAD( "flash", 0x0000, 0x1800000, NO_DUMP )
ROM_END

// Spice It Up (Russia)
ROM_START( spiceup )
	ENDEAVOUR_BIOS

	ROM_REGION32_BE( 0x200000, "program", 0 )
	ROM_LOAD32_WORD_SWAP( "siu5rc26_01h.u75", 0x00000, 0x100000, CRC(373bc2b1) SHA1(af3740fdcd028f162440701c952a3a87805bc65b) )
	ROM_LOAD32_WORD_SWAP( "siu5rc26_02l.u66", 0x00002, 0x100000, CRC(2e584321) SHA1(ca98092dde76338117e989e774db2db672d87bfa) )

	ROM_REGION32_BE( 0x1800000, "flash", ROMREGION_ERASE00 )
	ROM_LOAD( "flash", 0x0000, 0x1800000, NO_DUMP )
ROM_END

// Sultan's Wish (Russia)
ROM_START( sultanw )
	ENDEAVOUR_BIOS

	ROM_REGION32_BE( 0x200000, "program", 0 )
	ROM_LOAD32_WORD_SWAP( "suw5rc26_01h.u75", 0x00000, 0x100000, CRC(27760529) SHA1(b8970a706df52ee5792bbd7a4e719f2be87662ac) )
	ROM_LOAD32_WORD_SWAP( "suw5rc26_02l.u66", 0x00002, 0x100000, CRC(1c98fd4d) SHA1(58ff948c0deba0bffb8866b15f46518524516501) )

	ROM_REGION32_BE( 0x1800000, "flash", ROMREGION_ERASE00 )
	ROM_LOAD( "flash", 0x0000, 0x1800000, NO_DUMP )
ROM_END

// White Russia (Russia)
ROM_START( whiterus )
	ENDEAVOUR_BIOS

	ROM_REGION32_BE( 0x200000, "program", 0 )
	ROM_LOAD32_WORD_SWAP( "whr5ra26_01h.u75", 0x00000, 0x080000, CRC(d5a1ebb6) SHA1(14a8d1d8f8ae8919eaa878660c7e97e7ea7a02d8) )
	ROM_LOAD32_WORD_SWAP( "whr5ra26_02l.u66", 0x00002, 0x080000, CRC(48a2277c) SHA1(965d1da31e3bcde6fda4e15e8980a69e8bce5a84) )

	ROM_REGION32_BE( 0x1800000, "flash", ROMREGION_ERASE00 )
	ROM_LOAD( "flash", 0x0000, 0x1800000, NO_DUMP )
ROM_END

// "Zero" is not the actual name, the ROMs are clear chips and have no name string
ROM_START( konzero )
	ROM_REGION32_BE( 0x200000, "program", 0 )
	ROM_LOAD32_WORD_SWAP( "low2na35_01h.u75", 0x00000, 0x080000, CRC(b9237061) SHA1(0eb311e8e1c872d6a9c38726efb17ddf4713bc7d) )
	ROM_LOAD32_WORD_SWAP( "low2na35_02l.u66", 0x00002, 0x080000, CRC(2806299c) SHA1(a069f4477b310f99ff1ff48f622dc30862589127) )

	ROM_REGION32_BE( 0x1800000, "flash", ROMREGION_ERASE00 )
	// unsure whether this set actually has flash data, given that these chips would be replacing the game chips when resetting the memory

	ROM_REGION( 0x200000, "ifu", 0 )
	ROM_LOAD( "2n11prog_ifu.u190", 0x000000, 0x080000, NO_DUMP )

	ROM_REGION16_BE( 0x100, "eeprom", 0 )
	ROM_LOAD( "93c56.u98", 0x00, 0x100, CRC(b2521a6a) SHA1(f44711545bee7e9c772a3dc23b79f0ea8059ec50) ) // empty eeprom with Konami header
ROM_END

void konendev_state::init_konendev()
{
}

void konendev_state::init_enchlamp()
{
	uint32_t *rom = (uint32_t*)memregion("program")->base();
	rom[0x24/4] = 0x00002743;       // patch flash checksum for now

	// patch sound data checksums
	rom[0x2d924/4] = 0x00000000;
	rom[0x2d928/4] = 0x00000000;
	rom[0x2d934/4] = 0x00000000;
	rom[0x2d938/4] = 0x00000000;

	rom[0] = 0x5782b930;                    // new checksum for program rom
}

} // anonymous namespace


// BIOS
GAME( 200?, konendev, 0,        konendev, konendev, konendev_state, init_enchlamp, ROT0,  "Konami", "Konami Endeavour BIOS", MACHINE_NOT_WORKING|MACHINE_IS_BIOS_ROOT )

// has flash dump
GAME( 200?, enchlamp, konendev, konendev, konendev, konendev_state, init_enchlamp, ROT0,  "Konami", "Enchanted Lamp (Konami Endeavour, Russia)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )

// missing flash
GAME( 200?, aadvent,  konendev, konendev, konendev, konendev_state, init_konendev, ROT0,  "Konami", "African Adventure (Konami Endeavour, Russia)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 200?, blkrose,  0,        konendev, konendev, konendev_state, init_konendev, ROT0,  "Konami", "Black Rose Rapid Fire Grand Prix (Konami Endeavour, Queensland)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 200?, dragnfly, konendev, konendev, konendev, konendev_state, init_konendev, ROT0,  "Konami", "Dragonfly (Konami Endeavour, Russia)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 200?, gypmagic, konendev, konendev, konendev, konendev_state, init_konendev, ROT0,  "Konami", "Gypsy Magic (Konami Endeavour, Russia)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 200?, incanp,   konendev, konendev, konendev, konendev_state, init_konendev, ROT0,  "Konami", "Incan Pyramid (Konami Endeavour, Russia)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 200?, jestmagi, konendev, konendev, konendev, konendev_state, init_konendev, ROT0,  "Konami", "Jester Magic (Konami Endeavour, Russia)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 200?, luckfoun, konendev, konendev, konendev, konendev_state, init_konendev, ROT0,  "Konami", "Lucky Fountain (Konami Endeavour, Russia)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 200?, mohicans, konendev, konendev, konendev, konendev_state, init_konendev, ROT0,  "Konami", "Mohican Sun (Konami Endeavour, Russia)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 200?, monshow,  konendev, konendev, konendev, konendev_state, init_konendev, ROT0,  "Konami", "The Monster Show (Konami Endeavour, Russia)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 200?, rapfire5, 0,        konendev, konendev, konendev_state, init_konendev, ROT0,  "Konami", "Rapid Fire 5 (Konami Endeavour, NSW)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 200?, romanl,   konendev, konendev, konendev, konendev_state, init_konendev, ROT0,  "Konami", "Roman Legions (Konami Endeavour, Russia)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 200?, safemon,  konendev, konendev, konendev, konendev_state, init_konendev, ROT0,  "Konami", "Safe Money (Konami Endeavour, Russia)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 200?, showqn,   konendev, konendev, konendev, konendev_state, init_konendev, ROT0,  "Konami", "Show Queen (Konami Endeavour, Russia)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 200?, spiceup,  konendev, konendev, konendev, konendev_state, init_konendev, ROT0,  "Konami", "Spice It Up (Konami Endeavour, Russia)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 200?, sultanw,  konendev, konendev, konendev, konendev_state, init_konendev, ROT0,  "Konami", "Sultan's Wish (Konami Endeavour, Russia)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 200?, whiterus, konendev, konendev, konendev, konendev_state, init_konendev, ROT0,  "Konami", "White Russia (Konami Endeavour, Russia)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )

// clear chip (not a game)
GAME( 200?, konzero,  0,        konendev, konendev, konendev_state, init_konendev, ROT0,  "Konami", "Zero (Konami Endeavour)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
