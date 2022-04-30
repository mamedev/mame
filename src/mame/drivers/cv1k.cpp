// license:BSD-3-Clause
// copyright-holders:David Haywood, Luca Elia, MetalliC
/*

Cave CV1000 hardware

U2 flash rom note:

Cave often programmed the u2 roms onto defective flash chips, programming around the bad blocks.
As a result these are highly susceptible to failure, blocks around the known bad blocks appear to
decay at an alarming rate in some cases, and in others data has clearly been programmed over
blocks that were already going bad. - this is why the same game often has different u2 roms
in the different sets at the moment. - these will be reconstructed at some point.

The flash roms do contain a 'bad block' table, so it should be possible to rebuild a 'clean'
flash ROM for each game by comparing multiple dumps of each game and ensuring no other data has
decayed.  Naturally this is not an ideal situation for the less common games!

--------------------------------------------------------------------------------

Games by Cave ID number:
ID is labeled on U13 chip, not always
Serial is on a sticker on PCB, eg. "CAVE DS-10104"

ID     Serial   PCB  Title
-------------------------------------
CA011  -        B    Mushihime-Sama
CA???  MHN      B    Mushihime-Sama Cave Matsuri 1.5
CA012  -        B    Ibara
CA012B IB       B    Ibara Kuro Black Label
CA013  E        B    Espgaluda II
CA???  M        B    Puzzle! Mushihime-Tama
CA014  CA       B    Pink Sweets: Ibara Sorekara
CA015  CA       B    Mushihime-Sama Futari
CA015B MFBA/MMB B    Mushihime-Sama Futari Black Label
CA016  MP       B    Muchi Muchi Pork!
CA017  DS       B    Deathsmiles
CA017B DSB      D    Deathsmiles Black Label
CA019  DD       D    DoDonPachi DaiFukkatsu
CA019B DDB      D    DoDonPachi DaiFukkatsu Black Label
CA021  AK       D    Akai Katana
CA???  SDO      D    DoDonPachi SaiDaiOuJou

CMDL01 - Medal Mahjong Moukari Bancho
?????? - Pirates of Gappori: http://web.archive.org/web/20090907145501/http://www.cave.co.jp/gameonline/gappori/
?????? - Uhauha Ooku: http://web.archive.org/web/20141104001322/http://www.cave.co.jp/gameonline/oooku/

Note: CA018 - Deathsmiles II: Makai no Merry Christmas on PC based platform (see cavepc.cpp)
      CA020 - DoDonPachi DaiOuJou Tamashii on PGM2 platform


PCB CV1000-B / CV1000-D
+--------------------------------------------+
|                                            |
|                                            |
|                                            |
|                 VOL                        |
|                                            |
+-+        +-----+ +-----+       X3          |
  |        | U24 | | U23 |                   |
+-+        +-----+ +-----+    +------+       |
|                             |Yamaha|       |
|            U25*    U26*     |YMZ770|       |
|                             |      |       |
|J  D5                        +------+       |
|A  D2                                       |
|M  D3                    +-----+ +-----+    |
|M  D4         P2*        | U7  | | U6  |    |
|A  D1                    +-----+ +-----+    |
|          +-------+                         |
|C         |P4 JTAG|                         |
|o         +-------+       +-------+         |
|n                         |Altera |  +--+   |
|n          D6             |Cyclone|  |  |   |
|e     X1      S1          |       |  |U1|   |
|c    S3 +---------+       +-------+  |  |   |
|t       |         |                  +--+   |
|e       | Hitachi |                         |
|r       |         |     S2                  |
|        |   SH3   |   +-----------------+   |
|        |         |   |       P3        |   |
|        +---------+   +-----------------+   |
+-+    X2                                    |
  |       +--+  +--+    +---+  U27           |
+-+       |  |  |  |    |U13|                |
|+-+      |U2|  |U4|    +---+                |
||P|      |  |  |  |                 __      |
||8| U12* +--+  +--+     U10        /  \     |
|+-+                               |C126|    |
|     P5* P7*                       \__/     |
+--------------------------------------------+

* Denotes not populated

  CPU: Hitachi 6417709S SH3 clocked at 102.4MHz (12.800MHz * 8)
Sound: Yamaha YMZ770C-F clocked at 16.384MHz
Other: Altera Cyclone EPIC12 FPGA
       Altera EPM7032 (MAX 7000 Series) at U13

OSC:
 X1 12.800MHz (SH3 clock derived from this)
 X2 32.768kHz (Used by the RTC)
 X3 16.384MHz (Yamaha YMZ770C-F clock)

Memory:
 U6 (SDRAM)  MT46V16M16 ? 4 MBit x 16 x 4 banks, RAM (256 MBit)
 U7 (SDRAM)  MT46V16M16 ? 4 MBit x 16 x 4 banks, RAM (256 MBit)
 U1 (SDRAM)  MT48LC2M32 ? 512K x 32 x 4 banks, (64 MBit) for CV1000-B
 U1 (SDRAM)  IS42S32400 - 1024K x 32 x 4 banks, (128 MBit) for CV1000-D

Roms:
      U4 (FLASH)  29LV160BB 16M-Bit CMOS 3.0V, Boot device, FPGA bit file, main program code for CV1000-B
      U4 (FLASH)  S29JL032H 32M-Bit CMOS 3.0V, Boot device, FPGA bit file, main program code for CV1000-D
      U2 (FLASH)  K9F1G08U0M 128M x 8 Bit / 64M x 16 Bit NAND. Graphics data.
 U23-U24 (FLASH)  MBM 29DL321, 32M-Bit CMOS 3.0V. Sound data.
 U25-U26 (FLASH)  MBM 29DL321, not populated

Battery:
 C126 CR2450, Powers the RTC (Real Time Clock) U10. Look at the garden clock in Ibara. NOT present on CV1000-D

Dipswitches & Push Buttons:
 S1 (DIL SWITCH) Half Pitch DIL Switch x 1, function unknown
 S2 (DIL SWITCH) Half Pitch DIL Switch x 4, SW1=Setup, other switches unknown
 S3 (MICRO PUSH BUTTON) Test switch, same as on the JAMMA connector

Connectors:
 P2 (IDC CONNECTOR 20 PIN) function unknown, P2 is not always mounted
 P4 (IDC CONNECTOR 14 PIN) JTAG connector
 P8 (IDC CONNECTOR 10 PIN) Advanced User Debugger
 P3 (CONNECTOR) Most likely an expansion port, P3 is not always mounted
 P5 (CONNECTOR) D9 serial connector. Used for the mahjong Touchscreen titles.  Also mounted on early Mushihime-Sama PCB's
 P7 (CONNECTOR) Network port pinout. Never seen mounted on any PCB.

Misc:
   U27 (SUPERVISOR) MAX 690S 3.0V Microprocessor Supervisory Circuit.
   U10 (RTC & EEPROM) RTC 9701, Serial RTC Module with EEPROM 4 kbit (256x16 bit), controlled by Altera EPM7032 U13.
   U12 (RS-232 TRANSCEIVER) MAX 3244E RS-232 Transceiver, only mounted when P5 is mounted.
 D1-D6 (LED) Status LED's. D6 lights up at power on then shuts off, D2 indicates coinage.

Note: * The Altera EPM7032 usually stamped / labeled with the Cave game ID number as listed above.
      * Actual flash ROMs will vary by manufacturer but will be compatible with flash ROM listed.
      * There are two known CV1000-B PCB revisions. The newer one has some minor hardware differences
        and uses an updated FPGA firmware, they are not compatible with eachother.
      * The CV1000-D revision PCB has double the RAM at U1, double the ROM at U4 and no battery.
        The CV1000-D is used for Dodonpachi Daifukkatsu and later games. Commonly referred to as SH3B PCB.

Information by The Sheep, rtw, Ex-Cyber, BrianT & Guru

------------------------------------------------------

To enter service mode in most cases hold down Service (F2) for a few seconds
 (I believe it's the test button on the PCB)
Some games also use the test dipswitch as an alternative method.

Common game codes:
 - During boot hold P1 Right+A+B+C and P2 Left+A+B+C - Forcibly initialise non-volatile data (EEPROM or NAND settings area)
 - During boot hold P1 A and P2 A - Reset random numbers generator at each game start. Probably was used during testing or/and competition events.

TODO:

Improve Blending precision?
 - I'm not sure what precision the original HW mixes with, source data is 555 RGB with 1 bit transparency (16-bits)
   and the real VRAM is also clearly in this format.  The Alpha values supplied however are 8bpp, and the 'Tint'
   values use 0x20 for 'normal' (not 0x1f)

Overall screen brightness / contrast (see test mode)
 - Could convert ram back to 16-bit and use a palette lookup at the final blit.. probably easiest / quickest.

Touchscreen
 - Used for mmmbanc, needs SH3 serial support.

Remaining Video issues
 - mmpork startup screen flicker - the FOR USE IN JAPAN screen doesn't appear on the real PCB until after the graphics are fully loaded, it still displays 'please wait' until that point.
 - is the use of the 'scroll' registers 100% correct? (related to above?)
 - Sometimes the 'sprites' in mushisam lag by a frame vs the 'backgrounds' is this a timing problem, does the real game do it?

Blitter Timing
 - Correct slowdown emulation and flags (depends on blit mode, and speed of RAM) - could do with the recompiler or alt idle skips on the busy flag wait loops
 - End of Blit IRQ? (one game has a valid irq routine that looks like it was used for profiling, but nothing depends on it)

31/12/2021:
  Akai Katana and Dodonpachi Saidaioujou removed at the request of the
  current rightholder, exA-Arcadia (legal@exa.ac).

*/

#include "emu.h"

#include "cpu/sh/sh3comn.h"
#include "cpu/sh/sh4.h"
#include "machine/rtc9701.h"
#include "machine/serflash.h"
#include "sound/ymz770.h"
#include "video/epic12.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

namespace {

class cv1k_state : public driver_device
{
public:
	cv1k_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_blitter(*this, "blitter"),
		m_serflash(*this, "game"),
		m_eeprom(*this, "eeprom"),
		m_ram(*this, "mainram"),
		m_rombase(*this, "maincpu"),
		m_blitrate(*this, "BLITRATE"),
		m_eepromout(*this, "EEPROMOUT"),
		m_idleramoffs(0),
		m_idlepc(0)
	{ }

	void cv1k(machine_config &config);
	void cv1k_d(machine_config &config);

	void init_mushisam();
	void init_ibara();
	void init_espgal2();
	void init_mushitam();
	void init_pinkswts();
	void init_deathsml();
	void init_ddpdfk();

protected:
	virtual void machine_reset() override;

private:
	required_device<sh34_base_device> m_maincpu;
	required_device<epic12_device> m_blitter;
	required_device<serflash_device> m_serflash;
	required_device<rtc9701_device> m_eeprom;

	required_shared_ptr<uint64_t> m_ram;
	required_region_ptr<uint64_t> m_rombase;

	uint8_t flash_io_r(offs_t offset);
	void flash_io_w(offs_t offset, uint8_t data);
	uint8_t serial_rtc_eeprom_r(offs_t offset);
	void serial_rtc_eeprom_w(offs_t offset, uint8_t data);
	uint64_t flash_port_e_r();

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_ioport m_blitrate;
	required_ioport m_eepromout;

	uint32_t m_idleramoffs;
	uint32_t m_idlepc;

	uint64_t speedup_r();
	void install_speedups(uint32_t idleramoff, uint32_t idlepc, bool is_typed);

	void cv1k_d_map(address_map &map);
	void cv1k_map(address_map &map);
	void cv1k_port(address_map &map);
};


/**************************************************************************/

uint32_t cv1k_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_blitter->set_delay_scale(m_blitrate->read());

	m_blitter->draw_screen(bitmap,cliprect);
	return 0;
}


// FLASH interface

uint64_t cv1k_state::flash_port_e_r()
{
	return ((m_serflash->flash_ready_r() ? 0x20 : 0x00)) | 0xdf;
}


uint8_t cv1k_state::flash_io_r(offs_t offset)
{
	switch (offset)
	{
		default:
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x04:
		case 0x05:
		case 0x06:
		case 0x07:
			// logerror("flash_io_r offset %04x\n", offset);
			return 0xff;

		case 0x00:
			return m_serflash->flash_io_r();
	}
}

void cv1k_state::flash_io_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
		default:
		case 0x03:
			logerror("unknown flash_io_w offset %04x data %02x\n", offset, data); // 03 enable/disable fgpa access?
			break;

		case 0x00:
			m_serflash->flash_data_w( data);
			break;

		case 0x01:
			m_serflash->flash_cmd_w(data);
			break;

		case 0x2:
			m_serflash->flash_addr_w(data);
			break;
	}
}


// ibarablk uses the rtc to render the clock in the first attract demo
// if this code returns bad values it has gfx corruption.  the ibarablka set doesn't do this?!
uint8_t cv1k_state::serial_rtc_eeprom_r(offs_t offset)
{
	switch (offset)
	{
		case 0x01:
			return 0xfe | m_eeprom->read_bit();

		default:
			return 0;
	}
}

void cv1k_state::serial_rtc_eeprom_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0x01:
			m_eepromout->write(data, 0xff);
			break;
		case 0x03:
			m_serflash->flash_enab_w(data);
			break;

		default:
			logerror("unknown serial_rtc_eeprom_w access offset %02x data %02x\n", offset, data);
			break;
	}
}


void cv1k_state::cv1k_map(address_map &map)
{
	map(0x00000000, 0x003fffff).rom().region("maincpu", 0).nopw(); // mmmbanc writes here on startup for some reason..
	map(0x0c000000, 0x0c7fffff).ram().share("mainram");// work RAM
	map(0x10000000, 0x10000007).rw(FUNC(cv1k_state::flash_io_r), FUNC(cv1k_state::flash_io_w));
	map(0x10400000, 0x10400007).w("ymz770", FUNC(ymz770_device::write));
	map(0x10c00000, 0x10c00007).rw(FUNC(cv1k_state::serial_rtc_eeprom_r), FUNC(cv1k_state::serial_rtc_eeprom_w));
//  map(0x18000000, 0x18000057) // blitter, installed on reset
	map(0xf0000000, 0xf0ffffff).ram(); // mem mapped cache (sh3 internal?)
}

void cv1k_state::cv1k_d_map(address_map &map)
{
	map(0x00000000, 0x003fffff).rom().region("maincpu", 0).nopw(); // mmmbanc writes here on startup for some reason..
	map(0x0c000000, 0x0cffffff).ram().share("mainram"); // work RAM
	map(0x10000000, 0x10000007).rw(FUNC(cv1k_state::flash_io_r), FUNC(cv1k_state::flash_io_w));
	map(0x10400000, 0x10400007).w("ymz770", FUNC(ymz770_device::write));
	map(0x10c00000, 0x10c00007).rw(FUNC(cv1k_state::serial_rtc_eeprom_r), FUNC(cv1k_state::serial_rtc_eeprom_w));
//  map(0x18000000, 0x18000057) // blitter, installed on reset
	map(0xf0000000, 0xf0ffffff).ram(); // mem mapped cache (sh3 internal?)
}

void cv1k_state::cv1k_port(address_map &map)
{
	map(SH3_PORT_C, SH3_PORT_C+7).portr("PORT_C");
	map(SH3_PORT_D, SH3_PORT_D+7).portr("PORT_D");
	map(SH3_PORT_E, SH3_PORT_E+7).r(FUNC(cv1k_state::flash_port_e_r));
	map(SH3_PORT_F, SH3_PORT_F+7).portr("PORT_F");
	map(SH3_PORT_L, SH3_PORT_L+7).portr("PORT_L");
	map(SH3_PORT_J, SH3_PORT_J+7).rw(m_blitter, FUNC(epic12_device::fpga_r), FUNC(epic12_device::fpga_w));
}


static INPUT_PORTS_START( cv1k_base )
	PORT_START("DSW")       // 18000050.l (18000050.b + 3 i.e. MSB + 3, is shown as DIPSW)
	// note: physical switch have default/Off position marked as "ON" which is a bit confusing
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x00, "S2:1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x00, "S2:2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x00, "S2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x00, "S2:4" )
	PORT_BIT(0xfffffff0, IP_ACTIVE_HIGH, IPT_UNKNOWN)

	PORT_START("PORT_C")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 ) // Service coin
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE2 ) // Test Button on JAMMA Edge
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1  ) // TODO: IMPLEMENT COIN ERROR!
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2  )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("PORT_D")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1        ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2        ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3        ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4        ) PORT_PLAYER(1)

	PORT_START("PORT_F")
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_SERVICE3 ) // S3 Test Push Button
	PORT_BIT( 0xfd, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("PORT_L")    // 4000134.b, 4000136.b
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1        ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2        ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3        ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4        ) PORT_PLAYER(2)

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", rtc9701_device, write_bit)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", rtc9701_device, set_clock_line)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", rtc9701_device, set_cs_line)

	PORT_START("BLITCFG") // the Blitter Delay code isn't threadsafe, DO NOT turn on by default
	PORT_CONFNAME( 0x0001,  0x0000, "Use (unsafe) Blitter Delay (requires reset)" )
	PORT_CONFSETTING(       0x0000, DEF_STR( No ) )
	PORT_CONFSETTING(       0x0001, DEF_STR( Yes ) )

	PORT_START("BLITRATE")
	PORT_ADJUSTER(50, "Blitter Delay")
INPUT_PORTS_END

static INPUT_PORTS_START( cv1k )
	PORT_INCLUDE( cv1k_base )

	PORT_MODIFY("PORT_F")
	PORT_SERVICE_NO_TOGGLE( 0x02, IP_ACTIVE_LOW ) // S3 Test Push Button
INPUT_PORTS_END

static INPUT_PORTS_START( cv1ks )
	PORT_INCLUDE( cv1k_base )

	PORT_MODIFY("DSW")
	PORT_SERVICE_DIPLOC( 0x01, IP_ACTIVE_HIGH, "S2:1")
	PORT_DIPNAME( 0x02, 0x00, "Special Mode" ) PORT_DIPLOCATION("S2:2") // also require hold P1 A+B while booting
	PORT_DIPSETTING( 0x00, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x02, DEF_STR( On ) )
INPUT_PORTS_END

void cv1k_state::machine_reset()
{
	m_blitter->set_rambase(reinterpret_cast<uint16_t *>(m_ram.target()));
	m_blitter->set_is_unsafe(ioport("BLITCFG")->read());
	m_blitter->install_handlers( 0x18000000, 0x18000057 );
	m_blitter->reset();
}

void cv1k_state::cv1k(machine_config &config)
{
	/* basic machine hardware */
	SH3BE(config, m_maincpu, 12.8_MHz_XTAL*8); // 102.4MHz
	m_maincpu->set_md(0, 0); // none of this is verified
	m_maincpu->set_md(1, 0); // (the sh3 is different to the sh4 anyway, should be changed)
	m_maincpu->set_md(2, 0);
	m_maincpu->set_md(3, 0);
	m_maincpu->set_md(4, 0);
	m_maincpu->set_md(5, 1);
	m_maincpu->set_md(6, 0);
	m_maincpu->set_md(7, 1);
	m_maincpu->set_md(8, 0);
	m_maincpu->set_sh4_clock(12.8_MHz_XTAL*8); // 102.4MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &cv1k_state::cv1k_map);
	m_maincpu->set_addrmap(AS_IO, &cv1k_state::cv1k_port);
	m_maincpu->set_vblank_int("screen", FUNC(cv1k_state::irq2_line_hold)); // irq2 actually asserted at V-sync pulse, not at V-blank

	RTC9701(config, m_eeprom);
	SERFLASH(config, m_serflash, 0);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh(HZ_TO_ATTOSECONDS(60.024)); // measured from ibara PCB rates - 60.024Hz, 262 total lines
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(512, 512);
	screen.set_visarea(0, 320-1, 0, 240-1);
	screen.set_screen_update(FUNC(cv1k_state::screen_update));

	PALETTE(config, "palette").set_entries(0x10000);

	SPEAKER(config, "mono").front_center();
	YMZ770(config, "ymz770", 16.384_MHz_XTAL).add_route(1, "mono", 1.0); // only Right output used, Left is not connected

	EPIC12(config, m_blitter, 0);
	m_blitter->set_cpu(m_maincpu);
	m_blitter->port_r_callback().set_ioport("DSW");
	m_blitter->set_mainramsize(0x800000);
}

void cv1k_state::cv1k_d(machine_config &config)
{
	cv1k(config);

	/* basic machine hardware */
	SH3BE(config.replace(), m_maincpu, 12.8_MHz_XTAL*8); // 102.4MHz
	m_maincpu->set_md(0, 0); // none of this is verified
	m_maincpu->set_md(1, 0); // (the sh3 is different to the sh4 anyway, should be changed)
	m_maincpu->set_md(2, 0);
	m_maincpu->set_md(3, 0);
	m_maincpu->set_md(4, 0);
	m_maincpu->set_md(5, 1);
	m_maincpu->set_md(6, 0);
	m_maincpu->set_md(7, 1);
	m_maincpu->set_md(8, 0);
	m_maincpu->set_sh4_clock(12.8_MHz_XTAL*8); // 102.4MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &cv1k_state::cv1k_d_map);
	m_maincpu->set_addrmap(AS_IO, &cv1k_state::cv1k_port);
	m_maincpu->set_vblank_int("screen", FUNC(cv1k_state::irq2_line_hold));

	m_blitter->set_mainramsize(0x1000000);
}



/**************************************************

All roms are flash roms with no labels, so keep the
 version numbers attached to the roms that differ
 - roms which differ have also been prefixed with
   the MAME set names to aid readability and prevent
   accidental misloading of sets with the wrong
   CRCs which causes issues with the speedups.

**************************************************/

ROM_START( mushisam )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP("mushisam_u4", 0x000000, 0x200000, CRC(15321b30) SHA1(d2cd714ff2299eeab6f9a7c219dfb559c8f98b45) ) // (2004/10/12.MASTER VER.)
	ROM_RELOAD(0x200000,0x200000)

	ROM_REGION( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD("mushisam_u2", 0x000000, 0x8400000, CRC(4f0a842a) SHA1(33f3550ec676a7088b6348cd72c16cc6594afb84) ) // (2004/10/12.MASTER VER.)

	ROM_REGION( 0x800000, "ymz770", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP("u23", 0x000000, 0x400000, CRC(138e2050) SHA1(9e86489a4e65af5efb5495adf6d4b3e01d5b2816) )
	ROM_LOAD16_WORD_SWAP("u24", 0x400000, 0x400000, CRC(e3d05c9f) SHA1(130c3d62317da1729c85bd178bd51500edd73ada) )
ROM_END

ROM_START( mushisama )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP("mushisama_u4", 0x000000, 0x200000, CRC(0b5b30b2) SHA1(35fd1bb1561c30b311b4325bc8f4628f2fccd20b) ) // (2004/10/12 MASTER VER.)
	ROM_RELOAD(0x200000,0x200000)

	ROM_REGION( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD("mushisam_u2", 0x000000, 0x8400000, CRC(4f0a842a) SHA1(33f3550ec676a7088b6348cd72c16cc6594afb84) ) // (2004/10/12.MASTER VER.)

	ROM_REGION( 0x800000, "ymz770", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP("u23", 0x000000, 0x400000, CRC(138e2050) SHA1(9e86489a4e65af5efb5495adf6d4b3e01d5b2816) )
	ROM_LOAD16_WORD_SWAP("u24", 0x400000, 0x400000, CRC(e3d05c9f) SHA1(130c3d62317da1729c85bd178bd51500edd73ada) )
ROM_END

ROM_START( mushisamb )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP("mushisamb_u4", 0x000000, 0x200000, CRC(9f1c7f51) SHA1(f82ae72ec03687904ca7516887080be92365a5f3) ) // (2004/10/12 MASTER VER)
	ROM_RELOAD(0x200000,0x200000)

	ROM_REGION( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD("mushisam_u2", 0x000000, 0x8400000, CRC(4f0a842a) SHA1(33f3550ec676a7088b6348cd72c16cc6594afb84) ) // (2004/10/12.MASTER VER.)

	ROM_REGION( 0x800000, "ymz770", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP("u23", 0x000000, 0x400000, CRC(138e2050) SHA1(9e86489a4e65af5efb5495adf6d4b3e01d5b2816) )
	ROM_LOAD16_WORD_SWAP("u24", 0x400000, 0x400000, CRC(e3d05c9f) SHA1(130c3d62317da1729c85bd178bd51500edd73ada) )
ROM_END

ROM_START( espgal2 ) // newer CV1000-B PCB revision, updated FPGA firmware, no changes in game code or data
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "espgal2_u4", 0x000000, 0x200000, CRC(843608b8) SHA1(2f5fcd38e76df531a923cd9956104cef5185aaa9) ) // (2005/11/14 MASTER VER)
	ROM_RELOAD(0x200000,0x200000)

	ROM_REGION( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD( "u2", 0x000000, 0x8400000, CRC(222f58c7) SHA1(d47a5085a1debd9cb8c61d88cd39e4f5036d1797) ) // (2005/11/14 MASTER VER)

	ROM_REGION( 0x800000, "ymz770", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u23", 0x000000, 0x400000, CRC(b9a10c22) SHA1(4561f95c6018c9716077224bfe9660e61fb84681) )
	ROM_LOAD16_WORD_SWAP( "u24", 0x400000, 0x400000, CRC(c76b1ec4) SHA1(b98a53d41a995d968e0432ed824b0b06d93dcea8) )
ROM_END

ROM_START( espgal2a )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "espgal2a_u4", 0x000000, 0x200000, CRC(09c908bb) SHA1(7d6031fd3542b3e1d296ff218feb40502fd78694) ) // (2005/11/14 MASTER VER)
	ROM_RELOAD(0x200000,0x200000)

	ROM_REGION( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD( "u2", 0x000000, 0x8400000, CRC(222f58c7) SHA1(d47a5085a1debd9cb8c61d88cd39e4f5036d1797) ) // (2005/11/14 MASTER VER)

	ROM_REGION( 0x800000, "ymz770", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u23", 0x000000, 0x400000, CRC(b9a10c22) SHA1(4561f95c6018c9716077224bfe9660e61fb84681) )
	ROM_LOAD16_WORD_SWAP( "u24", 0x400000, 0x400000, CRC(c76b1ec4) SHA1(b98a53d41a995d968e0432ed824b0b06d93dcea8) )
ROM_END

ROM_START( mushitam )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP("mushitam_u4", 0x000000, 0x200000, CRC(c49eb6b1) SHA1(c40ee5de89e3f1cb49ac19687657dd2b42a88d81) ) // (2005/09/09.MASTER VER)
	ROM_RELOAD(0x200000,0x200000)

	ROM_REGION( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD("mushitam_u2", 0x000000, 0x8400000, CRC(8ba498ab) SHA1(459c0b4ab831bbe019bdd5b0ac56955948b9e3a6) ) // (2005/09/09.MASTER VER)

	ROM_REGION( 0x800000, "ymz770", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP("u23", 0x000000, 0x400000, CRC(701a912a) SHA1(85c198946fb693d99928ea2595c84ba4d9dc8157) )
	ROM_LOAD16_WORD_SWAP("u24", 0x400000, 0x400000, CRC(6feeb9a1) SHA1(992711c80e660c32f97b343c2ce8184fddd7364e) )
ROM_END

ROM_START( mushitama )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP("mushitama_u4", 0x000000, 0x200000, CRC(4a23e6c8) SHA1(d44c287bb88e6d413a8d35d75bc1b4928ad52cdf) ) // (2005/09/09 MASTER VER)
	ROM_RELOAD(0x200000,0x200000)

	ROM_REGION( 0x8400000, "game", ROMREGION_ERASEFF)
//  ROM_LOAD("mushitama_u2", 0x000000, 0x8400000, CRC(3f93ff82) SHA1(6f6c250aa7134016ffb288d056bc937ea311f538) ) // recycled ROM - only unused areas differ
	ROM_LOAD("mushitam_u2", 0x000000, 0x8400000, CRC(8ba498ab) SHA1(459c0b4ab831bbe019bdd5b0ac56955948b9e3a6) ) // (2005/09/09.MASTER VER)

	ROM_REGION( 0x800000, "ymz770", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP("u23", 0x000000, 0x400000, CRC(701a912a) SHA1(85c198946fb693d99928ea2595c84ba4d9dc8157) )
	ROM_LOAD16_WORD_SWAP("u24", 0x400000, 0x400000, CRC(6feeb9a1) SHA1(992711c80e660c32f97b343c2ce8184fddd7364e) )
ROM_END

ROM_START( futari15 )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP("futari15_u4", 0x000000, 0x200000, CRC(e8c5f128) SHA1(45fb8066fdbecb83fdc2e14555c460d0c652cd5f) ) // (2006/12/8.MAST VER. 1.54.)
	ROM_RELOAD(0x200000,0x200000)

	ROM_REGION( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD("futari15_u2", 0x000000, 0x8400000, CRC(b9eae1fc) SHA1(410f8e7cfcbfd271b41fb4f8d049a13a3191a1f9) ) // (2006/12/8.MAST VER. 1.54.)

	ROM_REGION( 0x800000, "ymz770", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP("u23", 0x000000, 0x400000, CRC(39f1e1f4) SHA1(53d12f59a56df35c705408c76e6e02118da656f1) )
	ROM_LOAD16_WORD_SWAP("u24", 0x400000, 0x400000, CRC(c631a766) SHA1(8bb6934a2f5b8a9841c3dcf85192b1743773dd8b) )
ROM_END

ROM_START( futari15a )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP("futari15a_u4", 0x000000, 0x200000, CRC(a609cf89) SHA1(56752fae9f42fa852af8ee2eae79e25ec7f17953) ) // (2006/12/8 MAST VER 1.54)
	ROM_RELOAD(0x200000,0x200000)

	ROM_REGION( 0x8400000, "game", ROMREGION_ERASEFF)
//  ROM_LOAD("futari15a_u2", 0x000000, 0x8400000, CRC(b9d815f9) SHA1(6b6f668b0bbb087ffac65e4f0d8bd9d5b28eeb28) )  // recycled ROM - only unused areas differ
	ROM_LOAD("futari15_u2", 0x000000, 0x8400000, CRC(b9eae1fc) SHA1(410f8e7cfcbfd271b41fb4f8d049a13a3191a1f9) ) // (2006/12/8.MAST VER. 1.54.)

	ROM_REGION( 0x800000, "ymz770", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP("u23", 0x000000, 0x400000, CRC(39f1e1f4) SHA1(53d12f59a56df35c705408c76e6e02118da656f1) )
	ROM_LOAD16_WORD_SWAP("u24", 0x400000, 0x400000, CRC(c631a766) SHA1(8bb6934a2f5b8a9841c3dcf85192b1743773dd8b) )
ROM_END

ROM_START( futari10 )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "futari10_u4", 0x000000, 0x200000, CRC(b127dca7) SHA1(e1f518bc72fc1cdf69aefa89eafa4edaf4e84778) ) // (2006/10/23 MASTER VER.)
	ROM_RELOAD(0x200000,0x200000)

	ROM_REGION( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD( "futari10_u2", 0x000000, 0x8400000, CRC(78ffcd0c) SHA1(0e2937edec15ce3f5741b72ebd3bbaaefffb556e) ) // (2006/10/23 MASTER VER.)

	ROM_REGION( 0x800000, "ymz770", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u23", 0x000000, 0x400000, CRC(39f1e1f4) SHA1(53d12f59a56df35c705408c76e6e02118da656f1) )
	ROM_LOAD16_WORD_SWAP( "u24", 0x400000, 0x400000, CRC(c631a766) SHA1(8bb6934a2f5b8a9841c3dcf85192b1743773dd8b) )
ROM_END

ROM_START( futaribl ) // Title screen shows (c) 2007 despite the 2009 "master" date - Also prints "Another Ver" to the title screen - reworked & re-released for the Chinese market
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "futaribli_u4", 0x000000, 0x200000, CRC(1971dd16) SHA1(e75993f2978cbaaf925b4b8bb33d094a5a7cebf0) ) // (2009/11/27 INTERNATIONAL BL)
	ROM_RELOAD(0x200000,0x200000)

	ROM_REGION( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD( "futariblk_u2", 0x000000, 0x8400000, CRC(08c6fd62) SHA1(e1fc386b2b0e41906c724287cbf82304297e0150) )

	ROM_REGION( 0x800000, "ymz770", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u23", 0x000000, 0x400000, CRC(39f1e1f4) SHA1(53d12f59a56df35c705408c76e6e02118da656f1) )
	ROM_LOAD16_WORD_SWAP( "u24", 0x400000, 0x400000, CRC(c631a766) SHA1(8bb6934a2f5b8a9841c3dcf85192b1743773dd8b) )
ROM_END

ROM_START( futariblj )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "futariblk_u4", 0x000000, 0x200000, CRC(b9467b6d) SHA1(64782807fe69acb4ae028e36c5c689d8bd9b7857) ) // (2007/12/11 BLACK LABEL VER)
	ROM_RELOAD(0x200000,0x200000)

	ROM_REGION( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD( "futariblk_u2", 0x000000, 0x8400000, CRC(08c6fd62) SHA1(e1fc386b2b0e41906c724287cbf82304297e0150) ) // (2007/12/11 BLACK LABEL VER)

	ROM_REGION( 0x800000, "ymz770", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u23", 0x000000, 0x400000, CRC(39f1e1f4) SHA1(53d12f59a56df35c705408c76e6e02118da656f1) )
	ROM_LOAD16_WORD_SWAP( "u24", 0x400000, 0x400000, CRC(c631a766) SHA1(8bb6934a2f5b8a9841c3dcf85192b1743773dd8b) )
ROM_END

ROM_START( ibara )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u4", 0x000000, 0x200000, CRC(8e6c155d) SHA1(38ac2107dc7824836e2b4e04c7180d5ae43c9b79) ) // (2005/03/22 MASTER VER..)
	ROM_RELOAD(0x200000,0x200000)

	ROM_REGION( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD( "u2", 0x000000, 0x8400000, CRC(55840976) SHA1(4982bdce84f9603adfed7a618f18bc80359ab81e) ) // (2005/03/22 MASTER VER..)

	ROM_REGION( 0x800000, "ymz770", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u23", 0x000000, 0x400000, CRC(ee5e585d) SHA1(7eeba4ee693060e927f8c46b16e39227c6a62392) )
	ROM_LOAD16_WORD_SWAP( "u24", 0x400000, 0x400000, CRC(f0aa3cb6) SHA1(f9d137cd879e718811b2d21a0af2a9c6b7dca2f9) )
ROM_END

ROM_START( ibarablk ) // Title screen shows (c) 2005 despite the 2006 "master" date
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "ibarablk_u4", 0x000000, 0x200000, CRC(ee1f1f77) SHA1(ac276f3955aa4dde2544af4912819a7ae6bcf8dd) ) // (2006/02/06. MASTER VER.)
	ROM_RELOAD(0x200000,0x200000)

	ROM_REGION( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD( "ibarablk_u2", 0x000000, 0x8400000, CRC(5e46be44) SHA1(bed5f1bf452f2cac58747ecabec3c4392566a3a7) ) // (2006/02/06. MASTER VER.)

	ROM_REGION( 0x800000, "ymz770", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u23", 0x000000, 0x400000, CRC(a436bb22) SHA1(0556e771cc02638bf8814315ba671c2d442594f1) ) // (2006/02/06 MASTER VER.)
	ROM_LOAD16_WORD_SWAP( "u24", 0x400000, 0x400000, CRC(d11ab6b6) SHA1(2132191cbe847e2560423e4545c969f21f8ff825) ) // (2006/02/06 MASTER VER.)
ROM_END

ROM_START( ibarablka ) // Title screen shows (c) 2005 despite the 2006 "master" date
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "ibarablka_u4", 0x000000, 0x200000, CRC(a9d43839) SHA1(507696e616608c05893c7ac2814b3365e9cb0720) ) // (2006/02/06 MASTER VER.)
	ROM_RELOAD(0x200000,0x200000)

	ROM_REGION( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD( "ibarablka_u2", 0x000000, 0x8400000, CRC(33400d96) SHA1(09c22b5431ac3726bf88c56efd970f56793f825a) ) // (2006/02/06 MASTER VER.)

	ROM_REGION( 0x800000, "ymz770", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u23", 0x000000, 0x400000, CRC(a436bb22) SHA1(0556e771cc02638bf8814315ba671c2d442594f1) ) // (2006/02/06 MASTER VER.)
	ROM_LOAD16_WORD_SWAP( "u24", 0x400000, 0x400000, CRC(d11ab6b6) SHA1(2132191cbe847e2560423e4545c969f21f8ff825) ) // (2006/02/06 MASTER VER.)
ROM_END

ROM_START( deathsml )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u4", 0x000000, 0x200000, CRC(1a7b98bf) SHA1(07798a4a846e5802756396b34df47d106895c1f1) ) // (2007/10/09 MASTER VER)
	ROM_RELOAD(0x200000,0x200000)

	ROM_REGION( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD( "u2", 0x000000, 0x8400000, CRC(59ef5d78) SHA1(426e506b6d88948aa55aec71c0db6e91da3d490d) ) // (2007/10/09 MASTER VER)

	ROM_REGION( 0x800000, "ymz770", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u23", 0x000000, 0x400000, CRC(aab718c8) SHA1(0e636c46d06151abd6f73232bc479dafcafe5327) )
	ROM_LOAD16_WORD_SWAP( "u24", 0x400000, 0x400000, CRC(83881d84) SHA1(6e2294b247dfcbf0ced155dc45c706f29052775d) )
ROM_END

ROM_START( mmpork )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u4", 0x000000, 0x200000, CRC(d06cfa42) SHA1(5707feb4b3e5265daf5926f38c38612b24106f1f) ) // (2007/ 4/17 MASTER VER.)
	ROM_RELOAD(0x200000,0x200000)

	ROM_REGION( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD( "u2", 0x000000, 0x8400000, CRC(1ee961b8) SHA1(81a2eba704ac1cf7fc44fa7c6a3f50e3570c104f) ) // (2007/ 4/17 MASTER VER.)

	ROM_REGION( 0x800000, "ymz770", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u23", 0x000000, 0x400000, CRC(4a4b36df) SHA1(5db5ce6fa47e5ca3263d4bd19315890c6d29df66) )
	ROM_LOAD16_WORD_SWAP( "u24", 0x400000, 0x400000, CRC(ce83d07b) SHA1(a5947467c8f5b7c4b0ad8e32df2ee29b787e355f) )
ROM_END

ROM_START( mmmbanc )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u4", 0x0000, 0x200000, CRC(5589d8c6) SHA1(43fbdb0effe2bc0e7135698757b6ee50200aecde) ) // (2007/06/05 MASTER VER.)
	ROM_RELOAD(0x200000,0x200000)

	ROM_REGION( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD( "u2", 0x000000, 0x8400000, CRC(2e38965a) SHA1(2b58d1cd1a3dbc261d4a46805d2ea015fe22c444) ) // (2007/06/05 MASTER VER.)

	ROM_REGION( 0x800000, "ymz770", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u23", 0x000000, 0x400000, CRC(4caaa1bf) SHA1(9b92c13eac05601da4d9bb3eb727c156974e9f0c) )
	ROM_LOAD16_WORD_SWAP( "u24", 0x400000, 0x400000, CRC(8e3a51ba) SHA1(e34cf9acb13c3d8ca6cd1306b060b1d429872abd) )
ROM_END

ROM_START( pinkswts )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "pinkswts_u4", 0x0000, 0x200000, CRC(5d812c9e) SHA1(db821ec3892fd150513749d64a8b60bf147f3275) ) // (2006/04/06 MASTER VER....)
	ROM_RELOAD(0x200000,0x200000)

	ROM_REGION( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD( "pinkswts_u2", 0x000000, 0x8400000, CRC(a2fa5363) SHA1(5be327534840871592df523ac82ee1927bd79d67) ) // (2006/04/06 MASTER VER....)

	ROM_REGION( 0x800000, "ymz770", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u23", 0x000000, 0x400000, CRC(4b82d250) SHA1(ee98dbc3f791efb6d58f3945bcb2044667ae7978) )
	ROM_LOAD16_WORD_SWAP( "u24", 0x400000, 0x400000, CRC(e93f0627) SHA1(6f5ec0ade87f7fc42a58a8f125557a4d1f3f187d) )
ROM_END

ROM_START( pinkswtsa )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "pnkswtsa_u4", 0x0000, 0x200000, CRC(ee3339b2) SHA1(995988d370731a7074b49ce8752525dadf06a954) ) // (2006/04/06 MASTER VER...)
	ROM_RELOAD(0x200000,0x200000)

	ROM_REGION( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD( "pnkswtsa_u2", 0x000000, 0x8400000, CRC(829a862e) SHA1(8c0ee2a0eb33b68869252fd68aed74820a904287) ) // (2006/04/06 MASTER VER...)

	ROM_REGION( 0x800000, "ymz770", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u23", 0x000000, 0x400000, CRC(4b82d250) SHA1(ee98dbc3f791efb6d58f3945bcb2044667ae7978) )
	ROM_LOAD16_WORD_SWAP( "u24", 0x400000, 0x400000, CRC(e93f0627) SHA1(6f5ec0ade87f7fc42a58a8f125557a4d1f3f187d) )
ROM_END

ROM_START( pinkswtsb )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "pnkswtsb_u4", 0x0000, 0x200000, CRC(68bcc009) SHA1(2fef544b93c61161a37365f868b431d8262e4b21) ) // (2006/04/06 MASTER VER.)
	ROM_RELOAD(0x200000,0x200000)

	ROM_REGION( 0x8400000, "game", ROMREGION_ERASEFF)
//  ROM_LOAD( "pnkswtsb_u2", 0x000000, 0x8400000, BAD_DUMP CRC(a5666ed9) SHA1(682e06c84990225bc6bb0c9f38b5f46c4e36b430) ) // (2006/04/06 MASTER VER.)
	ROM_LOAD( "pnkswtsx_u2", 0x000000, 0x8400000, CRC(91e4deb2) SHA1(893cb10d6f805df7cb4a1bb709a3ea6de147b7e9) ) // (2006/xx/xx MASTER VER.) and (2006/04/06 MASTER VER.)

	ROM_REGION( 0x800000, "ymz770", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u23", 0x000000, 0x400000, CRC(4b82d250) SHA1(ee98dbc3f791efb6d58f3945bcb2044667ae7978) )
	ROM_LOAD16_WORD_SWAP( "u24", 0x400000, 0x400000, CRC(e93f0627) SHA1(6f5ec0ade87f7fc42a58a8f125557a4d1f3f187d) )
ROM_END

ROM_START( pinkswtsx )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "pnkswtsx_u4", 0x0000, 0x200000, CRC(8fe05bf0) SHA1(5cd45ac0e74690787c83d3fb383a65ed7cd47104) ) // (2006/xx/xx MASTER VER.)
	ROM_RELOAD(0x200000,0x200000)

	ROM_REGION( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD( "pnkswtsx_u2", 0x000000, 0x8400000, CRC(91e4deb2) SHA1(893cb10d6f805df7cb4a1bb709a3ea6de147b7e9) ) // (2006/xx/xx MASTER VER.) and (2006/04/06 MASTER VER.)

	ROM_REGION( 0x800000, "ymz770", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u23", 0x000000, 0x400000, CRC(4b82d250) SHA1(ee98dbc3f791efb6d58f3945bcb2044667ae7978) )
	ROM_LOAD16_WORD_SWAP( "u24", 0x400000, 0x400000, CRC(e93f0627) SHA1(6f5ec0ade87f7fc42a58a8f125557a4d1f3f187d) )
ROM_END

// modification based on 2006/04/06 MASTER VER....
// This set is dumped from bootleg PCB with pre-patched Suicide Club hack, These CRC32 checksums are matches compares to authors homepage's one.
ROM_START( pinkswtssc )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "suicideclub.u4", 0x0000, 0x200000, CRC(5e03662f) SHA1(b974204b8dcd55fc1b7775f7c1806150919caff3) ) // (2017/10/31 SUICIDECLUB VER.)
	ROM_RELOAD(0x200000,0x200000)

	ROM_REGION( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD( "suicideclub.u2", 0x000000, 0x8400000, CRC(32324608) SHA1(cec1416c943520cb3f91eb295e2ba864a0db7d45) )

	ROM_REGION( 0x800000, "ymz770", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u23", 0x000000, 0x400000, CRC(4b82d250) SHA1(ee98dbc3f791efb6d58f3945bcb2044667ae7978) )
	ROM_LOAD16_WORD_SWAP( "u24", 0x400000, 0x400000, CRC(e93f0627) SHA1(6f5ec0ade87f7fc42a58a8f125557a4d1f3f187d) )
ROM_END

ROM_START( ddpdfk )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "ddpdfk_u4", 0x0000, 0x400000, CRC(9976d699) SHA1(9dfe9d1daf6f638cafce8cdc5230209e2bcb7522) ) // (2008/06/23  MASTER VER 1.5)

	ROM_REGION( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD( "ddpdfk_u2", 0x000000, 0x8400000, CRC(84a51a4f) SHA1(291a6279c0746d2eb8630e7d6d886043f0cfdd94) ) // (2008/06/23  MASTER VER 1.5)

	ROM_REGION( 0x800000, "ymz770", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u23", 0x000000, 0x400000, CRC(27032cde) SHA1(5b58d0140d72b91db4e763ca4af93060d36ac74d) )
	ROM_LOAD16_WORD_SWAP( "u24", 0x400000, 0x400000, CRC(a6178c2c) SHA1(7742ad1de2e4b0d0206ad11d467ea35da36352df) )
ROM_END

ROM_START( ddpdfk10 )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "ddpdfk10_u4", 0x0000, 0x400000, CRC(a3d650b2) SHA1(46a7551760e18c2cecd372c3f4be16f6600efc2c) ) // (2008/05/16  MASTER VER)

	ROM_REGION( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD( "ddpdfk10_u2", 0x000000, 0x8400000, CRC(d349cb2a) SHA1(c364c36b69b93f8f62390f185d044f51056669ff) ) // (2008/05/16  MASTER VER)

	ROM_REGION( 0x800000, "ymz770", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u23", 0x000000, 0x400000, CRC(27032cde) SHA1(5b58d0140d72b91db4e763ca4af93060d36ac74d) )
	ROM_LOAD16_WORD_SWAP( "u24", 0x400000, 0x400000, CRC(a6178c2c) SHA1(7742ad1de2e4b0d0206ad11d467ea35da36352df) )
ROM_END

ROM_START( dsmbl )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u4", 0x000000, 0x400000, CRC(77fc5ad1) SHA1(afe044fc16e9494143c876879b033caccd08cf22) ) // (2008/10/06 MEGABLACK LABEL VER)

	ROM_REGION( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD( "u2", 0x000000, 0x8400000, CRC(d6b85b7a) SHA1(4674f6ad07f6a03904ca3d05060816b8fe061add) ) // (2008/10/06 MEGABLACK LABEL VER)

	ROM_REGION( 0x800000, "ymz770", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u23", 0x000000, 0x400000, CRC(a9536a6a) SHA1(4b9dcaf6803b1fcfdf73ae9daabc4508fec71631) )
	ROM_LOAD16_WORD_SWAP( "u24", 0x400000, 0x400000, CRC(3b673326) SHA1(1ae847eb4e752fef1d72081d82344f0ad0537c31) )
ROM_END

ROM_START( dfkbl )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u4", 0x000000, 0x400000, CRC(8092ca9d) SHA1(75e16cd7c8d0f9c715115ce12da5c245fbcd2416) ) // (2010/1/18 BLACK LABEL)

	ROM_REGION( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD( "u2", 0x000000, 0x8400000, CRC(29f9d73a) SHA1(ed978ab5e3ad8c05e7778a91bfb5aaa17b0f72d9) )

	ROM_REGION( 0x800000, "ymz770", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u23", 0x000000, 0x400000, CRC(36d4093b) SHA1(4aed7e2f7c0d2c9bceeb110a9907d8d99d55f4c3) )
	ROM_LOAD16_WORD_SWAP( "u24", 0x400000, 0x400000, CRC(31f9eb0a) SHA1(322158779e969bb321241065dd49c1167b91ff6c) )
ROM_END

// ROM_START( akatana )
//  ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF)
//  ROM_LOAD16_WORD_SWAP( "u4", 0x000000, 0x400000, CRC(613fd380) SHA1(6e28480eef3b483d00b42d811a9d2c7fa1097924) ) // (2010/ 8/13 MASTER VER.)

//  ROM_REGION( 0x8400000, "game", ROMREGION_ERASEFF)
//  ROM_LOAD( "u2", 0x000000, 0x8400000, CRC(89a2e1a5) SHA1(e6f4ec974406283665697fdd52bd606d0337dd11) )

//  ROM_REGION( 0x800000, "ymz770", ROMREGION_ERASEFF)
//  ROM_LOAD16_WORD_SWAP( "u23", 0x000000, 0x400000, CRC(34a67e24) SHA1(78a7e82123b86311f1116a80c39f147b8b695549) )
//  ROM_LOAD16_WORD_SWAP( "u24", 0x400000, 0x400000, CRC(10760fed) SHA1(b70f4506c00f3901ff38f5efd4b897af1afc7a0c) )
// ROM_END

// ROM_START( ddpsdoj )
//  ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF)
//  ROM_LOAD16_WORD_SWAP("u4", 0x000000, 0x400000, CRC(e2a4411c) SHA1(f8b2b6326dd8eeae99b8b1ab2bd5a3f0b9c7f027) )
//  ROM_IGNORE( 0x000100 ) // Flash extra bytes

//  ROM_REGION( 0x8400000, "game", ROMREGION_ERASEFF)
//  ROM_LOAD("u2", 0x000000, 0x8400000, CRC(668e4cd6) SHA1(da0b10865df3a3e46cf8a109ca88a551faba4483) )

//  ROM_REGION( 0x800000, "ymz770", ROMREGION_ERASEFF)
//  ROM_LOAD16_WORD_SWAP("u23", 0x000000, 0x400000, CRC(ac94801c) SHA1(cbcc6d5d89860bc961967e1d3b7c329adaf200c5) )
//  ROM_IGNORE( 0x000100 ) // Flash extra bytes
//  ROM_LOAD16_WORD_SWAP("u24", 0x400000, 0x400000, CRC(f593045b) SHA1(91b92870d0dd2a7817cb0059cc750e2393686f4c) )
//  ROM_IGNORE( 0x000100 ) // Flash extra bytes
// ROM_END

uint64_t cv1k_state::speedup_r()
{
	offs_t pc = m_maincpu->pc();

	if (pc == m_idlepc || pc == m_idlepc + 2) m_maincpu->spin_until_time(attotime::from_usec(10));

	return m_ram[m_idleramoffs / 8];
}

void cv1k_state::install_speedups(uint32_t idleramoff, uint32_t idlepc, bool is_typed)
{
	m_idleramoffs = idleramoff;
	m_idlepc = idlepc;

	m_maincpu->sh2drc_set_options(SH2DRC_FASTEST_OPTIONS);

	m_maincpu->sh2drc_add_pcflush(idlepc+2);

	m_maincpu->space(AS_PROGRAM).install_read_handler(0xc000000+m_idleramoffs, 0xc000000+m_idleramoffs+7, read64smo_delegate(*this, FUNC(cv1k_state::speedup_r)));

	m_maincpu->sh2drc_add_fastram(0x00000000, 0x003fffff, true, m_rombase);

	m_maincpu->sh2drc_add_fastram(0x0c000000, 0x0c000000+m_idleramoffs-1, false, m_ram);
	m_maincpu->sh2drc_add_fastram(0x0c000000+m_idleramoffs+8, is_typed ? 0x0cffffff : 0x0c7fffff, false, m_ram + ((m_idleramoffs+8)/8));
}


void cv1k_state::init_mushisam()
{
	install_speedups(0x024d8, 0xc04a2aa, false);
}

void cv1k_state::init_ibara()
{
	install_speedups(0x022f0, 0xc04a0aa, false);
}

void cv1k_state::init_espgal2()
{
	install_speedups(0x02310, 0xc05177a, false);
}

void cv1k_state::init_mushitam()
{
	install_speedups(0x0022f0, 0xc04a0da, false);
}

void cv1k_state::init_pinkswts()
{
	install_speedups(0x02310, 0xc05176a, false);
}

void cv1k_state::init_deathsml()
{
	install_speedups(0x02310, 0xc0519a2, false);
}

void cv1k_state::init_ddpdfk()
{
	install_speedups(0x02310, 0xc1d1346, true);
}


} // anonymous namespace


// The black label versions are intentionally not set as clones, they were re-releases with different game codes, not bugfixes.

// CA011  Mushihime-Sama
GAME( 2004, mushisam,   0,        cv1k,   cv1k, cv1k_state, init_mushisam, ROT270, "Cave (AMI license)",   "Mushihime-Sama (2004/10/12.MASTER VER.)",                                       MACHINE_IMPERFECT_TIMING )
GAME( 2004, mushisama,  mushisam, cv1k,   cv1k, cv1k_state, init_ibara,    ROT270, "Cave (AMI license)",   "Mushihime-Sama (2004/10/12 MASTER VER.)",                                       MACHINE_IMPERFECT_TIMING )
GAME( 2004, mushisamb,  mushisam, cv1k,   cv1k, cv1k_state, init_mushisam, ROT270, "Cave (AMI license)",   "Mushihime-Sama (2004/10/12 MASTER VER)",                                        MACHINE_IMPERFECT_TIMING )

// CA012  Ibara
GAME( 2005, ibara,      0,        cv1k,   cv1ks,cv1k_state, init_ibara,    ROT270, "Cave (AMI license)",   "Ibara (2005/03/22 MASTER VER..)",                                               MACHINE_IMPERFECT_TIMING )

// CA012B Ibara Kuro Black Label
GAME( 2006, ibarablk,   0,        cv1k,   cv1ks,cv1k_state, init_pinkswts, ROT270, "Cave (AMI license)",   "Ibara Kuro Black Label (2006/02/06. MASTER VER.)",                              MACHINE_IMPERFECT_TIMING )
GAME( 2006, ibarablka,  ibarablk, cv1k,   cv1ks,cv1k_state, init_pinkswts, ROT270, "Cave (AMI license)",   "Ibara Kuro Black Label (2006/02/06 MASTER VER.)",                               MACHINE_IMPERFECT_TIMING )

// CA013  Espgaluda II
GAME( 2005, espgal2,    0,        cv1k,   cv1k, cv1k_state, init_espgal2,  ROT270, "Cave (AMI license)",   "Espgaluda II (2005/11/14 MASTER VER, newer CV1000-B PCB)",                      MACHINE_IMPERFECT_TIMING )
GAME( 2005, espgal2a,   espgal2,  cv1k,   cv1k, cv1k_state, init_espgal2,  ROT270, "Cave (AMI license)",   "Espgaluda II (2005/11/14 MASTER VER, original CV1000-B PCB)",                   MACHINE_IMPERFECT_TIMING )

// CA???  Puzzle! Mushihime-Tama
GAME( 2005, mushitam,   0,        cv1k,   cv1k, cv1k_state, init_mushitam, ROT0,   "Cave (AMI license)",   "Puzzle! Mushihime-Tama (2005/09/09.MASTER VER)",                                MACHINE_IMPERFECT_TIMING )
GAME( 2005, mushitama,  mushitam, cv1k,   cv1k, cv1k_state, init_mushitam, ROT0,   "Cave (AMI license)",   "Puzzle! Mushihime-Tama (2005/09/09 MASTER VER)",                                MACHINE_IMPERFECT_TIMING )

// CA014  Pink Sweets: Ibara Sorekara
GAME( 2006, pinkswts,   0,        cv1k,   cv1ks,cv1k_state, init_pinkswts, ROT270, "Cave (AMI license)",   "Pink Sweets: Ibara Sorekara (2006/04/06 MASTER VER....)",                       MACHINE_IMPERFECT_TIMING )
GAME( 2006, pinkswtsa,  pinkswts, cv1k,   cv1ks,cv1k_state, init_pinkswts, ROT270, "Cave (AMI license)",   "Pink Sweets: Ibara Sorekara (2006/04/06 MASTER VER...)",                        MACHINE_IMPERFECT_TIMING )
GAME( 2006, pinkswtsb,  pinkswts, cv1k,   cv1ks,cv1k_state, init_pinkswts, ROT270, "Cave (AMI license)",   "Pink Sweets: Ibara Sorekara (2006/04/06 MASTER VER.)",                          MACHINE_IMPERFECT_TIMING )
GAME( 2006, pinkswtsx,  pinkswts, cv1k,   cv1ks,cv1k_state, init_pinkswts, ROT270, "Cave (AMI license)",   "Pink Sweets: Ibara Sorekara (2006/xx/xx MASTER VER.)",                          MACHINE_IMPERFECT_TIMING ) // defaults to freeplay, possibly bootlegged from show/dev version?
GAME( 2017, pinkswtssc, pinkswts, cv1k,   cv1ks,cv1k_state, init_pinkswts, ROT270, "bootleg (Four Horsemen)", "Pink Sweets: Suicide Club (2017/10/31 SUICIDECLUB VER., bootleg)",           MACHINE_IMPERFECT_TIMING ) // dumped from bootleg with pre-patched hack

// CA015  Mushihime-Sama Futari
GAME( 2006, futari15,   0,        cv1k,   cv1k, cv1k_state, init_pinkswts, ROT270, "Cave (AMI license)",   "Mushihime-Sama Futari Ver 1.5 (2006/12/8.MASTER VER. 1.54.)",                   MACHINE_IMPERFECT_TIMING )
GAME( 2006, futari15a,  futari15, cv1k,   cv1k, cv1k_state, init_pinkswts, ROT270, "Cave (AMI license)",   "Mushihime-Sama Futari Ver 1.5 (2006/12/8 MASTER VER 1.54)",                     MACHINE_IMPERFECT_TIMING )
GAME( 2006, futari10,   futari15, cv1k,   cv1k, cv1k_state, init_pinkswts, ROT270, "Cave (AMI license)",   "Mushihime-Sama Futari Ver 1.0 (2006/10/23 MASTER VER.)",                        MACHINE_IMPERFECT_TIMING )

// CA015B Mushihime-Sama Futari Black Label
GAME( 2007, futaribl,   0,        cv1k,   cv1k, cv1k_state, init_pinkswts, ROT270, "Cave (AMI license)",   "Mushihime-Sama Futari Black Label - Another Ver (2009/11/27 INTERNATIONAL BL)", MACHINE_IMPERFECT_TIMING ) // re-released for Chinese market
GAME( 2007, futariblj,  futaribl, cv1k,   cv1k, cv1k_state, init_pinkswts, ROT270, "Cave (AMI license)",   "Mushihime-Sama Futari Black Label (2007/12/11 BLACK LABEL VER)",                MACHINE_IMPERFECT_TIMING )

// CA016  Muchi Muchi Pork!
GAME( 2007, mmpork,     0,        cv1k,   cv1ks,cv1k_state, init_pinkswts, ROT270, "Cave (AMI license)",   "Muchi Muchi Pork! (2007/ 4/17 MASTER VER.)",                                    MACHINE_IMPERFECT_TIMING )

// CA017  Deathsmiles
GAME( 2007, deathsml,   0,        cv1k,   cv1k, cv1k_state, init_deathsml, ROT0,   "Cave (AMI license)",   "Deathsmiles (2007/10/09 MASTER VER)",                                           MACHINE_IMPERFECT_TIMING )

// CA017B Deathsmiles Black Label
GAME( 2008, dsmbl,      0,        cv1k_d, cv1k, cv1k_state, init_ddpdfk,   ROT0,   "Cave (AMI license)",   "Deathsmiles MegaBlack Label (2008/10/06 MEGABLACK LABEL VER)",                  MACHINE_IMPERFECT_TIMING )

// CA019  Do-Don-Pachi Dai-Fukkatsu
GAME( 2008, ddpdfk,     0,        cv1k_d, cv1k, cv1k_state, init_ddpdfk,   ROT270, "Cave (AMI license)",   "DoDonPachi Dai-Fukkatsu Ver 1.5 (2008/06/23 MASTER VER 1.5)",                   MACHINE_IMPERFECT_TIMING )
GAME( 2008, ddpdfk10,   ddpdfk,   cv1k_d, cv1k, cv1k_state, init_ddpdfk,   ROT270, "Cave (AMI license)",   "DoDonPachi Dai-Fukkatsu Ver 1.0 (2008/05/16 MASTER VER)",                       MACHINE_IMPERFECT_TIMING )

// CA019B Do-Don-Pachi Dai-Fukkatsu Black Label
GAME( 2010, dfkbl,      0,        cv1k_d, cv1k, cv1k_state, init_ddpdfk,   ROT270, "Cave",                 "DoDonPachi Dai-Fukkatsu Black Label (2010/1/18 BLACK LABEL)",                   MACHINE_IMPERFECT_TIMING )

// CA021  Akai Katana
//GAME( 2010, akatana,    0,        cv1k_d, cv1k, cv1k_state, init_ddpdfk,   ROT0,   "Cave",               "Akai Katana (2010/ 8/13 MASTER VER.)",                            MACHINE_IMPERFECT_TIMING )

// CA??? DoDonPachi SaiDaiOuJou
//GAME( 2012, ddpsdoj,    0,        cv1k_d, cv1k, cv1k_state, init_ddpdfk,   ROT270, "Cave",               "DoDonPachi SaiDaiOuJou (2012/ 4/20)",                             MACHINE_IMPERFECT_TIMING )

// CMDL01 Medal Mahjong Moukari Bancho
GAME( 2007, mmmbanc,    0,        cv1k,   cv1k, cv1k_state, init_pinkswts, ROT0,   "Cave (AMI license)",   "Medal Mahjong Moukari Bancho (2007/06/05 MASTER VER.)",                         MACHINE_NOT_WORKING )
