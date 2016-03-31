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
#include "softlist.h"

class leapster_state : public driver_device
{
public:
	leapster_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_cart(*this, "cartslot")
		{ }


	virtual void machine_start() override;
	virtual void machine_reset() override;

	UINT32 screen_update_leapster(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(leapster_cart);
	DECLARE_DRIVER_INIT(leapster);

	DECLARE_READ32_MEMBER(leapster_random_r)
	{
		return rand() | (rand()<<16); // there is a loop checking that this is above a certain value
	}

	DECLARE_WRITE32_MEMBER(leapster_aux004b_w)
	{
		printf("leapster_aux004b_w %04x\n", data);
	}

protected:
	required_device<cpu_device> m_maincpu;
	required_device<generic_slot_device> m_cart;

	memory_region *m_cart_rom;
};




static INPUT_PORTS_START( leapster )
INPUT_PORTS_END



UINT32 leapster_state::screen_update_leapster(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

DEVICE_IMAGE_LOAD_MEMBER( leapster_state, leapster_cart )
{
	UINT32 size = m_cart->common_get_size("rom");

	m_cart->rom_alloc(size, GENERIC_ROM32_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return IMAGE_INIT_PASS;
}

void leapster_state::machine_start()
{
	std::string region_tag;
	m_cart_rom = memregion(region_tag.assign(m_cart->tag()).append(GENERIC_ROM_REGION_TAG).c_str());

	if (m_cart_rom)
	{
		address_space &space = m_maincpu->space(AS_PROGRAM);

		space.install_readwrite_bank(0x80000000, 0x807fffff, "cartrom");
		membank("cartrom")->set_base(m_cart_rom->base());
	}
}

void leapster_state::machine_reset()
{
}

static ADDRESS_MAP_START( leapster_map, AS_PROGRAM, 32, leapster_state )
	AM_RANGE(0x00000000, 0x001fffff) AM_ROM AM_MIRROR(0x40000000) // pointers in the bios region seem to be to the 40xxxxxx region, either we mirror there or something (real bios?) is acutally missing
	AM_RANGE(0x0180D800, 0x0180D803) AM_READ(leapster_random_r)
	AM_RANGE(0x03000000, 0x030007ff) AM_RAM // puts stack here, writes a pointer @ 0x03000000 on startup
	AM_RANGE(0x3c000000, 0x3c1fffff) AM_RAM // really ram, or has our code execution gone wrong?
//  AM_RANGE(0x80000000, 0x807fffff) AM_ROMBANK("cartrom") // game ROM pointers are all to the 80xxxxxx region, so I assume it maps here - installed if a cart is present
ADDRESS_MAP_END

static ADDRESS_MAP_START( leapster_aux, AS_IO, 32, leapster_state )
	AM_RANGE(0x00000012c, 0x00000012f) AM_WRITE(leapster_aux004b_w) // this address isn't used by ARC internal stuff afaik, so probably leapster specific
ADDRESS_MAP_END

static MACHINE_CONFIG_START( leapster, leapster_state )
	/* basic machine hardware */
	// CPU is ArcTangent-A5 '5.1' (ARCompact core)
	MCFG_CPU_ADD("maincpu", ARCA5, 96000000/10)
	MCFG_CPU_PROGRAM_MAP(leapster_map)
	MCFG_CPU_IO_MAP(leapster_aux)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(160, 160)
	MCFG_SCREEN_VISIBLE_AREA(0, 160-1, 0, 160-1)
	MCFG_SCREEN_UPDATE_DRIVER(leapster_state, screen_update_leapster)

	/* cartridge */
	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_plain_slot, "leapster_cart")
	MCFG_GENERIC_EXTENSIONS("bin")
	MCFG_GENERIC_LOAD(leapster_state, leapster_cart)

	/* Software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list", "leapster")
MACHINE_CONFIG_END

ROM_START(leapster)
	ROM_REGION(0x200000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD( "155-10072-a.bin", 0x00000, 0x200000, CRC(af05e5a0) SHA1(d4468d060543ba7e44785041093bc98bcd9afa07) )
ROM_END

ROM_START(leapstertv)
	ROM_REGION(0x200000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD( "am29pl160cb-90sf.bin", 0x00000, 0x200000, CRC(194cc724) SHA1(000a79d75c19f2e43532ce0b31f0dca0bed49eab) )
ROM_END

DRIVER_INIT_MEMBER(leapster_state,leapster)
{
}

CONS(2003,  leapster,    0,         0,  leapster,    leapster, leapster_state, leapster,    "LeapFrog",   "Leapster (Germany)",    MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IS_SKELETON )
CONS(2005,  leapstertv,  leapster,  0,  leapster,    leapster, leapster_state, leapster,    "LeapFrog",   "Leapster TV (Germany)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IS_SKELETON )
