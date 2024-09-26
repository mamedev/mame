// license:BSD-3-Clause
// copyright-holders:David Haywood
/*
    LeapFrog - Leapster

    educational system from 2003, software is all developed in MXFlash

    hwspecs


    CPU:
      Custom ASIC (ARCTangent 5.1 CPU @ 96MHz)

    Memory:
      Leapster: 2MB onboard RAM, 256 bytes NVRAM.
      Leapster2: 16MB RAM, 128kbytes NVRAM

    Media type:
      Cartridges of 4-16MB with between 2 and 512kb NVRAM

    Graphics:
      4Mb ATI chip.

    Audio:
      Custom

    Screen:
      160x160 CSTN with touchscreen.


    The Leapster 2 also has
        USB 1.1 (client only) + full-sized SD slot.


    many magic numbers in the BIOS ROM match the [strings:VALID_FLAGS] table in
    https://github.com/tsbiberdorf/MqxSrc/blob/master/tools/tad/mqx.tad
    does this mean the System is running on the MQX RTOS?
    https://www.synopsys.com/dw/ipdir.php?ds=os_mqx_software
    indicates it was available for ARC processors

*/

/* Cartridge pinout - for games list see hash/leapster.xml

CARTRIDGE-PINOUT:
-----------------
Look at the Cartridge-Slot:

                   B        A
                  ------------
                  VCC |01| VCC
                   NC |02| VSS
                  D11 |03| D04
                  D03 |04| D12
                  D10 |05| D05
                  VSS |06| D02
                  D13 |07| D09
                   NC |08| NC
                  D06 |09| D01
                  D14 |10| D08
                  ----|--|----
                  D07 |11| VSS
                  D00 |12| D15
                 Byte |13| OE
                   NC |14| A22
                   NC |15| A23
                   CE |16| A16
                  A00 |17| A15
                  A01 |18| A14
                  A02 |19| A13
                  VSS |20| A03
                  A12 |21| A04
                  A11 |22| A05
                  A10 |23| A06
                  A09 |24| A07
                  A08 |25| A17
                  A19 |26| A18
                  A20 |27| A21
                   WE |28| VSS
PIN7 of 24LC02B <---| |29| |---> PIN7 of 24LC02B
                   NC |30| |---> PIN6 of 24LC02B
                  ----------




PCB - Handheld-Console:

               +-----------------------------+
               |                             |
  +------------|                             |------------+
  |            | C A R T R I D G E - S L O T |            |
  |            |                             |            |
  |            +-----------------------------+            |
  |                                                       |
  |ASY 310-10069    +-------------------+                 |
  |                 |                   |                 |
  |                 |                   |                 |
  |LEAPSTER MAIN    |                   |                 |
  |Leap Frog        |                 U3|                 |
  |(c) 2004         +-------------------+     +-----+     |
  |                                           |ISSI |     |
  |                                           |     |     |
  |                      +---------+          |IS42S|     |
  |                      |         |          |16100|     |
   \       +-+           | EPOXY   |          |AT-7T|    /
    \      |A|           |   BLOCK |          |     |   /
     \     +-+           |         |          |   U2|  /
      \                  |       U1|          +-----+ /
       \                 +---------+                 /
        \                                           /
         \                                         /
          \                                       /
           \                                     /
            \                                   /
             \                                 /
              \                               /
               \                             /
                \                           /
                 \-------------------------/


A = 24LC02B / SN0429


ETCHES ON THE BACK OF THE PCB:

"FAB-300-10069-C"

"702800254.01A"
"SW1208 Rev.5"



PCB - Cartridge:
FRONT:

+-------------------------------------+
| LEAPSTER ROM CARTRIDGE              |
|   +--+     +---------+    Leap Frog |
|   |B1|U2   |         |     (c) 2003 |
+-+ +--+     |E P O X Y|            +-+
  |          |       U3|            |
+-+          +---------+            +-+
|                       20232-003-1020|
| ASY 310-10028             REV:00    |
+-+                    +-+          +-+
  |||||||||||||||||||||| ||||||||||||
  +--------------------+ +----------+
 A30                                A01

B1: 24L002B

24L002B:

             +-----+
  (GND)<- A0-|     |-VCC
  (GND)<- A1-|     |-WP
  (GND)<- A2-|     |-SCL
         VSS-|     |-SDA
             +-----+



PCB - LEAPSTER-TV:

+-----------------------------------------------------------------------+
|                                                                       |
|                                                                       |
|           20300+003+0015                                              |
 \           REV:06                                                    /
  |                                                                   |
  |                                                                   |
  |                 +----+     +-------+                              |
   \                |    |     | EPOXY |                             /
    |   +----+      |    |     |       |                            |
    |   |EPOX|      |    |     |       |                            |
    |   |Y   |      |    |     |     U1|    +-+                     |
     \  +----+      |    |     +-------+    | |                    /
      |             |  U2|                  +-+U4                 |
      |             +----+                                        |
      |                     +--------------+   ROADRUNNER CONSOLE |
       \                    | AM29PL160CB  |   Leap Frog (c) 2006/
        |                   | -90SF        |   Asy 310-10378    |
        |                   |            U6|                    |
        |                   +--------------+                    |
         \                                                     /
          |          +-----------------------------+          |
          |          | C A R T R I D G E - S L O T |          |
          |          +-----------------------------+          |
           \                                                 /
            +-----------------------------------------------+

*/

#include "emu.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "cpu/arcompact/arcompact.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"


namespace {

class leapster_state : public driver_device
{
public:
	leapster_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_cart(*this, "cartslot"),
		m_palette(*this, "palette")
		{ }

	void leapster(machine_config &config);

	void init_leapster();

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	uint32_t screen_update_leapster(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);

	INTERRUPT_GEN_MEMBER(testirq);

	uint32_t leapster_1801000_r();
	uint32_t leapster_1801004_r();
	uint32_t leapster_1801008_r();
	uint32_t leapster_180100c_r();
	uint32_t leapster_1801018_r();
	uint32_t leapster_1809004_r();
	uint32_t leapster_1809008_r();
	uint32_t leapster_180b000_r();
	uint32_t leapster_180b004_r();
	uint32_t leapster_180b008_r();
	uint32_t leapster_180d400_r();
	uint32_t leapster_180d514_r();
	uint32_t leapster_180d800_r();

	void leapster_aux0047_w(uint32_t data);
	uint32_t leapster_aux0048_r();
	void leapster_aux0048_w(uint32_t data);
	void leapster_aux004b_w(uint32_t data);

	void leapster_aux0010_w(uint32_t data);
	uint32_t leapster_aux0011_r();
	void leapster_aux0011_w(uint32_t data);
	void leapster_aux001a_w(uint32_t data);
	uint32_t leapster_aux001b_r();

	void leapster_aux(address_map &map) ATTR_COLD;
	void leapster_map(address_map &map) ATTR_COLD;

	uint16_t m_1a_data[0x800];
	int m_1a_pointer;

	required_device<arcompact_device> m_maincpu;
	required_device<generic_slot_device> m_cart;
	required_device<palette_device> m_palette;

	memory_region *m_cart_rom = nullptr;
};


static INPUT_PORTS_START( leapster )
INPUT_PORTS_END

void leapster_state::leapster_aux0010_w(uint32_t data)
{
}

void leapster_state::leapster_aux0011_w(uint32_t data)
{
	// unknown, written with 1a
}

void leapster_state::leapster_aux001a_w(uint32_t data)
{
	// probably not palette, but it does load 0x1000 words of increasing value on startup, so could be?
	m_1a_data[m_1a_pointer & 0x7ff] = data;

	uint8_t r = (data >> 12) & 0x7;
	uint8_t g = (data >> 8) & 0xf;
	uint8_t b = (data >> 4) & 0xf;

	m_palette->set_pen_color(m_1a_pointer & 0x7ff, rgb_t(pal3bit(r), pal4bit(g), pal4bit(b)));
	m_1a_pointer++;
}

uint32_t leapster_state::leapster_aux0011_r()
{
	// unknown, read when 11/1a are being written
	logerror("%s: leapster_aux0011_r\n", machine().describe_context());
	return 0x00000000;
}

uint32_t leapster_state::leapster_aux001b_r()
{
	// unknown, read when 11/1a are being written
	logerror("%s: leapster_aux001b_r\n", machine().describe_context());
	return 0x00000000;
}

void leapster_state::leapster_aux0047_w(uint32_t data)
{
	logerror("%s: leapster_aux0047_w %08x\n", machine().describe_context(), data);
}

uint32_t leapster_state::leapster_aux0048_r()
{
	logerror("%s: leapster_aux0048_r\n", machine().describe_context());
	return 0x00000000;
}

void leapster_state::leapster_aux0048_w(uint32_t data)
{
	logerror("%s: leapster_aux0047_w %08x\n", machine().describe_context(), data);
}

void leapster_state::leapster_aux004b_w(uint32_t data)
{
	logerror("%s: leapster_aux004b_w %08x\n", machine().describe_context(), data);
}

uint32_t leapster_state::leapster_1801000_r()
{
	logerror("%s: leapster_1801000_r\n", machine().describe_context());
	return 0x00000000;
}

uint32_t leapster_state::leapster_1801004_r()
{
	logerror("%s: leapster_1801004_r\n", machine().describe_context());
	return 0x00000000;
}

uint32_t leapster_state::leapster_1801008_r()
{
	logerror("%s: leapster_1801004_r\n", machine().describe_context());
	return 0x00000000;
}

uint32_t leapster_state::leapster_180100c_r()
{
	logerror("%s: leapster_180100c_r\n", machine().describe_context());
	return 0x00000000;
}

uint32_t leapster_state::leapster_1801018_r()
{
	logerror("%s: leapster_1801018_r\n", machine().describe_context());
	return 0x00000000;
}

uint32_t leapster_state::leapster_1809004_r()
{
	logerror("%s: leapster_1809004_r (return usually checked against 0x00200000)\n", machine().describe_context());
	// does an AND with 0x00200000 and often jumps to dead loops if that fails
	return 0x00200000;
}

uint32_t leapster_state::leapster_1809008_r()
{
	logerror("%s: leapster_1809008_r\n", machine().describe_context());
	return 0x00000000;
}

uint32_t leapster_state::leapster_180b000_r()
{
	logerror("%s: leapster_180b000_r\n", machine().describe_context());
	return 0x00000000;
}

uint32_t leapster_state::leapster_180b004_r()
{
	// leapster2 BIOS checks if this is 0
	logerror("%s: leapster_180b004_r\n", machine().describe_context());
	return 0xffffffff;
}

uint32_t leapster_state::leapster_180b008_r()
{
	// checks bit 1 (using BMSK instruction and AND instruction)
	// writes back to same address?
	logerror("%s: leapster_180b008_r\n", machine().describe_context());
	return 0x00000001;
}

uint32_t leapster_state::leapster_180d400_r()
{
	logerror("%s: leapster_180d400_r (return usually checked against 0x0030d400)\n", machine().describe_context());
	// does a BRLO.ND against it
	// loops against 0x0030d400 (3,200,000) at 4003A52A for example
	return 0x0030d400;
}

uint32_t leapster_state::leapster_180d514_r()
{
	logerror("%s: leapster_180d514_r (return usually checked against 0x0030d400)\n", machine().describe_context());
	// leapster -bios 0 does a BRNE in a loop comparing with 0x80
	return 0x00000080;
}

uint32_t leapster_state::leapster_180d800_r()
{
	logerror("%s: leapster_180d800_r (return usually checked against 0x00027100 or 0x00003e80)\n", machine().describe_context());
	// does a BRLO.ND against it
	// loops against 0x00027100 (160,000)
	// loops against 0x00003e80 (16,000) in other places 4003A56C for example
	return 0x00027100;
}

uint32_t leapster_state::screen_update_leapster(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

DEVICE_IMAGE_LOAD_MEMBER( leapster_state::cart_load )
{
	uint32_t size = m_cart->common_get_size("rom");

	m_cart->rom_alloc(size, GENERIC_ROM32_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return std::make_pair(std::error_condition(), std::string());
}

void leapster_state::machine_start()
{
	std::string region_tag;
	m_cart_rom = memregion(region_tag.assign(m_cart->tag()).append(GENERIC_ROM_REGION_TAG).c_str());

	if (m_cart_rom)
	{
		m_maincpu->space(AS_PROGRAM).install_rom(0x80000000, 0x807fffff, m_cart_rom->base());
	}

	save_item(NAME(m_1a_data));
}

void leapster_state::machine_reset()
{
	m_1a_pointer = 0;
	for (int i = 0; i < 0x800; i++)
		m_1a_data[i] = 0;
}

void leapster_state::leapster_map(address_map &map)
{
//  A vector table is copied from 0x00000000 to 0x3c000000, but it is unclear if that is a BIOS mirror
//  or if it should be copying a different table.
	map(0x00000000, 0x007fffff).mirror(0x40000000).rom().region("maincpu", 0);
	//map(0x40000000, 0x407fffff).rom().region("maincpu", 0);

//  map(0x01800000, 0x0180ffff).ram();

	map(0x01801000, 0x01801003).r(FUNC(leapster_state::leapster_1801000_r));
	map(0x01801004, 0x01801007).r(FUNC(leapster_state::leapster_1801004_r));
	map(0x01801008, 0x0180100b).r(FUNC(leapster_state::leapster_1801008_r));
	map(0x0180100c, 0x0180100f).r(FUNC(leapster_state::leapster_180100c_r));
	map(0x01801018, 0x0180101b).r(FUNC(leapster_state::leapster_1801018_r));

	map(0x01809004, 0x01809007).r(FUNC(leapster_state::leapster_1809004_r));
	map(0x01809008, 0x0180900b).r(FUNC(leapster_state::leapster_1809008_r));

	map(0x0180b000, 0x0180b003).r(FUNC(leapster_state::leapster_180b000_r));
	map(0x0180b004, 0x0180b007).r(FUNC(leapster_state::leapster_180b004_r));
	map(0x0180b008, 0x0180b00b).r(FUNC(leapster_state::leapster_180b008_r));

	map(0x0180d400, 0x0180d403).r(FUNC(leapster_state::leapster_180d400_r));

	map(0x0180d514, 0x0180d517).r(FUNC(leapster_state::leapster_180d514_r));

	map(0x0180d800, 0x0180d803).r(FUNC(leapster_state::leapster_180d800_r));

	map(0x03000000, 0x030007ff).ram(); // puts stack here, writes a pointer @ 0x03000000 on startup
	map(0x03000800, 0x0300ffff).ram(); // some of the later models need to store stack values here (or code execution has gone wrong?)
	map(0x3c000000, 0x3c1fffff).ram(); // vector base gets moved here with new IRQ table, puts task stacks etc. here
	map(0x3c200000, 0x3fffffff).ram();
	// map(0x80000000, 0x807fffff).bankr("cartrom"); // game ROM pointers are all to the 80xxxxxx region, so I assume it maps here - installed if a cart is present
}


void leapster_state::leapster_aux(address_map &map)
{
	// addresses used here aren't known internal ARC addresses, so are presumed to be Leapster specific
	map(0x000000010, 0x000000010).w(FUNC(leapster_state::leapster_aux0010_w));
	map(0x000000011, 0x000000011).rw(FUNC(leapster_state::leapster_aux0011_r), FUNC(leapster_state::leapster_aux0011_w));
	map(0x00000001a, 0x00000001a).w(FUNC(leapster_state::leapster_aux001a_w));
	map(0x00000001b, 0x00000001b).r(FUNC(leapster_state::leapster_aux001b_r));

	map(0x000000047, 0x000000047).w(FUNC(leapster_state::leapster_aux0047_w));
	map(0x000000048, 0x000000048).rw(FUNC(leapster_state::leapster_aux0048_r), FUNC(leapster_state::leapster_aux0048_w));
	map(0x00000004b, 0x00000004b).w(FUNC(leapster_state::leapster_aux004b_w));
}

INTERRUPT_GEN_MEMBER(leapster_state::testirq)
{
	m_maincpu->set_input_line(0, ASSERT_LINE);
}

void leapster_state::leapster(machine_config &config)
{
	// Basic machine hardware
	// CPU is ArcTangent-A5 '5.1' (ARCompact core)
	ARCA5(config, m_maincpu, 96000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &leapster_state::leapster_map);
	m_maincpu->set_addrmap(AS_IO, &leapster_state::leapster_aux);
	m_maincpu->set_default_vector_base(0x40000000);
	m_maincpu->set_vblank_int("screen", FUNC(leapster_state::testirq));

	// Video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_size(160, 160);
	screen.set_visarea(0, 160-1, 0, 160-1);
	screen.set_screen_update(FUNC(leapster_state::screen_update_leapster));

	PALETTE(config, "palette").set_format(palette_device::xRGB_444, 0x800).set_endianness(ENDIANNESS_BIG);

	// Cartridge
	GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, "leapster_cart", "bin").set_device_load(FUNC(leapster_state::cart_load));

	// Software lists
	SOFTWARE_LIST(config, "cart_list").set_original("leapster");
}

#define ROM_LOAD_BIOS(bios,name,offset,length,hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_BIOS(bios))

/* There are various build dates and revisions for different parts of the code, the date listed is the newest on in each ROM.
   This is always in the same place relative to the rest of the data.

   V2.1 sets (except TV) are apparently larger because "Learning with Leap" was built in.
*/

ROM_START(leapster)
	ROM_REGION(0x800000, "maincpu", ROMREGION_ERASE00)
	ROM_SYSTEM_BIOS( 0, "uni15", "Universal v1.5" ) // 152-10346 Leapster BaseROM Universal v1.5 - Sep 04 2003 10:46:47
	ROM_LOAD_BIOS( 0, "155-10072-a.bin"   , 0x00000, 0x200000, CRC(af05e5a0) SHA1(d4468d060543ba7e44785041093bc98bcd9afa07) )

	// most (all?) ROMs below seem to be from LMAX devices, or Leapster 2 devices based on the boot logos contained within!
	// TODO: properly sort once they boot in the driver
	ROM_SYSTEM_BIOS( 1, "us21",  "USA v2.1" )       // 152-11265 Leapster BaseROM US v2.1        - Apr 13 2005 15:34:57
	ROM_LOAD_BIOS( 1, "152-11265_2.1.bin",  0x00000, 0x800000, CRC(9639b3ae) SHA1(002873b782e823c7a8159deed16c78c149f2afab) )
	ROM_SYSTEM_BIOS( 2, "uk21",  "UK v2.1" )        // 152-11452 Leapster BaseROM UK v2.1        - Aug 30 2005 16:01:46
	ROM_LOAD_BIOS( 2, "leapster2_1004.bin", 0x00000, 0x800000, CRC(b466e14d) SHA1(910c234f03e76b7de55b8aa0a0c62fd1daae4910) )
	ROM_SYSTEM_BIOS( 3, "ger21", "German v2.1" )    // 152-11435 Leapster BaseROM German v2.1    - Oct 21 2005 18:53:59
	ROM_LOAD_BIOS( 3, "leapster2_1006.bin", 0x00000, 0x800000, BAD_DUMP CRC(a69ed8ca) SHA1(e6aacba0c39b1465f344c2b07ff1cbd8a395adac) ) // BADADDR xxx-xxxxxxxxxxxxxxxxxxx
	ROM_SYSTEM_BIOS( 4, "sp10",  "Spanish v1.0" )   // 152-11546 Leapster Baserom SP v1.0        - Apr 03 2006 06:26:00
	ROM_LOAD_BIOS( 4, "leapster2_1008.bin", 0x00000, 0x800000, CRC(b43345e7) SHA1(31c27e79568115bf36e5ef668f528e3005054152) )
	ROM_SYSTEM_BIOS( 5, "connb5",  "Connected B5" )  // 152-12076 Leapster Connected Baserom B5   - Feb 29 2008 18:11:21
	ROM_LOAD_BIOS( 5, "152-12076_b5.bin",   0x00000, 0x800000, CRC(4d223022) SHA1(bdc10ad70aa7641716e16fbea16bd0ef35f6e85e) ) // is this a Leapster 2 BIOS or another device called "Leapster Connected"?
	ROM_DEFAULT_BIOS( "uni15" )
ROM_END

ROM_START(leapstertv)
	ROM_REGION(0x800000, "maincpu", ROMREGION_ERASE00)
	ROM_SYSTEM_BIOS( 0, "uni2111", "Universal v2.1.11" ) // 152-11594 LeapsterTv Baserom Universal.v2.1.11 - Apr 13 2006 16:36:08
	ROM_LOAD_BIOS( 0, "am29pl160cb-90sf.bin", 0x00000, 0x200000, CRC(194cc724) SHA1(000a79d75c19f2e43532ce0b31f0dca0bed49eab) )
	ROM_DEFAULT_BIOS( "uni2111" )
ROM_END

ROM_START(leapster2)
	ROM_REGION(0x800000, "maincpu", ROMREGION_ERASE00)
	ROM_SYSTEM_BIOS( 0, "2xcip3_m9", "2x CIP3 m9" ) // 152-12659 Leapster 2x CIP3 Baserom m9 - Mar 29 2011 14:13:45
	ROM_LOAD_BIOS( 0, "152-12659_m9.bin",  0x00000, 0x800000, CRC(57bde604) SHA1(4b5eaac1e40bc605eb4cf6d4ad212343334762fd) )
	ROM_SYSTEM_BIOS( 1, "2x_06",     "2x 0.6" )     // 152-12206 Leapster 2x Baserom 0.6     - Feb 02 2009 17:15:38
	ROM_LOAD_BIOS( 1, "152-12206_0.6.bin", 0x00000, 0x800000, CRC(fa94d9a7) SHA1(c5bd84146701dc4a7635b0e37adedb90747adf32) )
	ROM_DEFAULT_BIOS( "2xcip3_m9" )
ROM_END

ROM_START(leapsterlmx)
	ROM_REGION(0x800000, "maincpu", ROMREGION_ERASE00)
	ROM_SYSTEM_BIOS( 0, "lmax_2_2",    "v2.2" )     // 152-11476 LMAX Baserom v2.2    - Jan 12 2006 11:22:50
	ROM_LOAD_BIOS( 0, "152-11476_v2.2.bin",    0x00000, 0x800000, CRC(e1140475) SHA1(42089165db67005b6a0180e894ff8f36b97a081e) )
	ROM_SYSTEM_BIOS( 1, "lmax_us_2_1", "USA v2.1" ) // 152-11238 LMAX BaseROM US v2.1 - Mar 04 2005 12:01:01
	ROM_LOAD_BIOS( 1, "152-11238_us_v2.1.bin", 0x00000, 0x800000, CRC(80bb4e58) SHA1(7d8b1c23d08ce76a89cff1112957377c6a1d4b63) )
	ROM_DEFAULT_BIOS( "lmax_2_2" )
ROM_END

void leapster_state::init_leapster()
{
}

} // anonymous namespace


CONS( 2003, leapster,    0,        0, leapster, leapster, leapster_state, init_leapster, "LeapFrog", "Leapster",       MACHINE_IS_SKELETON )
CONS( 2005, leapstertv,  leapster, 0, leapster, leapster, leapster_state, init_leapster, "LeapFrog", "Leapster TV",    MACHINE_IS_SKELETON )
CONS( 2005, leapsterlmx, leapster, 0, leapster, leapster, leapster_state, init_leapster, "LeapFrog", "Leapster L-MAX", MACHINE_IS_SKELETON )
CONS( 2009, leapster2,   leapster, 0, leapster, leapster, leapster_state, init_leapster, "LeapFrog", "Leapster 2",     MACHINE_IS_SKELETON )
