// license:BSD-3-Clause
// copyright-holders:Ville Linde
/* 'Aleck64' and similar boards */
/* N64 based hardware */
/*

TODO:
- eeprom (tested failure in mtetrisc and srmvs)
- mtetrisc: finish E90 hookup, understand how it actually draws gfxs.
- kurufev: column scrolling garbage on map and matchup screens. (btanb?)
- mayjin3: static noise during gameplay.
- starsldr: credit display is busted, it displays a 0 if credit is between 0 and 9. Silly protection/core bug? <- fixed as per 060916


If you want to boot eleven beat on any n64 emu ?(tested on nemu, 1964 and project64) patch the rom :
write 0 to offset $67b,$67c,$67d,$67e

*/

/*

Eleven Beat World Tournament
Hudson Soft, 1998

This game runs on Nintendo 64-based hardware which is manufactured by Seta.
It's very similar to the hardware used for 'Magical Tetris Featuring Mickey'
(Seta E90 main board) except the game software is contained in a cart that
plugs into a slot on the main board. The E92 board also has more RAM than
the E90 board.
The carts are not compatible with standard N64 console carts.

PCB Layout
----------

          Seta E92 Mother PCB
         |---------------------------------------------|
       --|     VOL_POT                                 |
       |R|TA8139S                                      |-
  RCA  --|  TA8201         BU9480                      |
 AUDIO   |                                             |
 PLUGS --|           AMP-NUS                           |
       |L|                                             |
       --|                 BU9480                      |
         |  TD62064                                    |
         |           UPD555  4050                      |
         |                                             |
         |    AD813  DSW1    TC59S1616AFT-10           |
         |J          DSW2    TC59S1616AFT-10           |
         |A       4.9152MHz                            |
         |M                                            |
         |M                                            |
         |A                    SETA                    |
         |                     ST-0043                 |
         |      SETA                          NINTENDO |
         |      ST-0042                       CPU-NUS A|
         |                                             |
         |                                             |
         |                                             |
         |                     14.31818MHz             |
         |X                                            |
         |   MAX232                            NINTENDO|
         |X          RDRAM18-NUS  RDRAM18-NUS  RCP-NUS |
         |           RDRAM18-NUS  RDRAM18-NUS          |
         |X   LVX125                                   |
         |                     14.705882MHz            |
         |X  PIF-NUS                                   |
         |            -------------------------------  |
         |   O        |                             |  |
         |            -------------------------------  |
         |---------------------------------------------|

Notes:
      Hsync      : 15.73kHz
      VSync      : 60Hz
      O          : Push-button reset switch
      X          : Connectors for special (Aleck64) digital joysticks
      CPU-NUS A  : Labelled on the PCB as "VR4300"

The cart contains:
                   CIC-NUS-5101: Boot protection chip
                   BK4D-NUS    : Similar to the save chip used in N64 console carts
                   NUS-ZHAJ.U3 : 64Mbit 28 pin DIP serial mask ROM

      - RCA audio plugs output stereo sound. Regular mono sound is output
        via the standard JAMMA connector also.

      - ALL components are listed for completeness. However, many are power or
        logic devices that most people need not be concerned about :-)

*/



/*

Magical Tetris Challenge Featuring Mickey
Capcom, 1998

This game runs on Nintendo 64-based hardware which is manufactured
by Seta. On bootup, it has the usual Capcom message....


Magical Tetris Challenge
        Featuring Mickey

       981009

        JAPAN




PCB Layout
----------

          Seta E90 Main PCB Rev. B
         |--------------------------------------------|
       --|     VOL_POT                       *        |
       |R|TA8139S                        TET-01M.U5   |
  RCA  --|  TA8201         BU9480                     |
 AUDIO   |                                SETA        |
 PLUGS --|           AMP-NUS              ST-0039     |
       |L|                      42.95454MHz           |
       --|                 BU9480                     |
         |  TD62064                   QS32X384        |
         |           UPD555  4050            QS32X384 |
         |                                            |
         |    AD813                                   |
         |J                                           |
         |A            4.9152MHz                      |
         |M                                           |
         |M                                           |
         |A                    SETA                   |
         |      SETA           ST-0035                |
         |      ST-0042                    NINTENDO   |
         |                     MX8330      CPU-NUS A  |
         |                     14.31818MHz            |
         |                                            |
         |X       AT24C01.U34  NINTENDO               |
         |                     RDRAM18-NUS            |
         |X                                           |
         |   MAX232            NINTENDO     NINTENDO  |
         |X          LT1084    RDRAM18-NUS  RCP-NUS   |
         |    LVX125     MX8330                       |
         |X  PIF-NUS       14.31818MHz                |
         |   O   CIC-NUS-5101  BK4D-NUS   NUS-CZAJ.U4 |
         |--------------------------------------------|

Notes:
      Hsync      : 15.73kHz
      VSync      : 60Hz
      *          : Unpopulated socket for 8M - 32M 42 pin DIP mask ROM
      O          : Push-button reset switch
      X          : Connectors for special (Aleck64?) digital joysticks
      CPU-NUS A  : Labelled on the PCB as "VR4300"
      BK4D-NUS   : Similar to the save chip used in N64 console carts

      ROMs
      ----
      TET-01M.U5 : 8Mbit 42 pin mask ROM
      NUS-CZAJ.U4: 128Mbit 28 pin DIP serial mask ROM
      AT24C01.U34: 128bytes x 8 bit serial EEPROM

      - RCA audio plugs output stereo sound. Regular mono sound is output
        via the standard JAMMA connector also.

      - ALL components are listed for completeness. However, many are power or
        logic devices that most people need not be concerned about :-)

      - The Seta/N64 Aleck64 hardware is similar also, but instead of the high capacity
        serial mask ROM being on the main board, it's in a cart that plugs into a slot.

*/

#include "emu.h"
#include "n64.h"

#include "cpu/rsp/rsp.h"
#include "cpu/mips/mips3.h"
#include "sound/dmadac.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class aleck64_state : public n64_state
{
public:
	aleck64_state(const machine_config &mconfig, device_type type, const char *tag)
		: n64_state(mconfig, type, tag)
		, m_e90_vram(*this,"e90vram")
		, m_e90_pal(*this,"e90pal")
		, m_dip_read_offset(0)
	{ }

	void aleck64(machine_config &config);
	void a64_e90(machine_config &config);

	void init_aleck64();

private:
	void aleck_dips_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t aleck_dips_r(offs_t offset, uint32_t mem_mask = ~0);
	uint16_t e90_prot_r();
	void e90_prot_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint32_t screen_update_e90(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void e90_map(address_map &map) ATTR_COLD;
	void n64_map(address_map &map) ATTR_COLD;
	void rsp_imem_map(address_map &map) ATTR_COLD;
	void rsp_dmem_map(address_map &map) ATTR_COLD;
	optional_shared_ptr<uint32_t> m_e90_vram;
	optional_shared_ptr<uint32_t> m_e90_pal;

	uint32_t m_dip_read_offset;
};


void aleck64_state::aleck_dips_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	/*
	    mtetrisc uses offset 0x1c and 0x03 a good bit in conjunction with reading INMJ.
	    (See Test menu GAME DATA and MEMORY CHECK/S2D Ram test, also CONFIGURATION/save & exit. eeprom? Serial2DRAM?)

	    srmvs uses 0x40, communications?
	 */

	switch( offset )
	{
		case 2:
			m_dip_read_offset = data;
			break;

		default:
			logerror("Unknown aleck_dips_w(0x%08x, 0x%08x, %08x) @ 0x%08x PC=%08x\n", offset, data, mem_mask, 0xc0800000 + offset*4, m_vr4300->pc());
	}
}

uint32_t aleck64_state::aleck_dips_r(offs_t offset, uint32_t mem_mask)
{
	// srmvs uses 0x40, communications?

	switch( offset )
	{
	case 0:
		return (ioport("IN0")->read());   /* mtetrisc has regular inputs here */
	case 1:
		return (ioport("IN1")->read());
	case 2:
		{
			uint32_t const val = ioport("INMJ")->read();

			switch( m_dip_read_offset >> 8 & 0xff )
			{
			case 1:
				return  val;

			case 2:
				return val << 8;

			case 4:
				return val << 16;

			case 8:
				return val >> 8;

			default:
				logerror("Unexpected read from INMJ with no dip_read_offset set.\n");
				return 0;
			}
		}
	default:
		logerror("Unknown aleck_dips_r(0x%08x, 0x%08x) @ 0x%08x PC=%08x\n", offset, 0xc0800000 + offset*4, mem_mask, m_vr4300->pc());
		return 0;
	}
}

/*

    The TLB entries made by all games show that the programmers put in extra mappings.
    Some of these are big, every game adds a mapping for 4MB at 0xc0000000 physical.
    Some add more than one 4MB mapping. As some of the boards are only 4MB RAM,
    it seems likely that the programmers expected main RAM to be mirrored at 0xc0000000.
    Some of the games have a further 4MB mapping to a physical segment contiguous with
    the first at 0xc0000000, so if these games are on the E92 8MB boards, this would
    add a bit of support to the mirror idea.

    Adding the mirror makes everything go pretty well,
    it makes doncdoon, hipai, kurufev, srmvs, twrshaft boot.

    There's still a couple of problems though:
    - srmvs and vivdolls system test screen actually check "SDRAM" at 0xc0000000.
    This all goes well until they overwrite the RAM test code which is running from
    virtual memory backed by the same physical memory as it's testing.
    (maybe this is related to the fact that the check code is executing from a cacheable kseg0 range,
    and the TLB entry for 0xc0000000 is in kuseg and flagged as non-cache?)
    - srmvs reads a jump vector from memory mapped to physical segment at 0xc0000000 that's been overwritten
    by a boot up memory test of that segment, so it crashes with a bad access during the intro.

    Both these problems go away by adding new RAM at 0xc0000000 physical, but maybe they are related to
    caching?

    Here's a full list of the physical addresses mapped by tlb entries, along with the names of the games
    that make the mappings and which games actually use them.

    4MB @ 0xc0000000 - 0xc03fffff ALL : used by doncdoon, hipai, kurufev, srmvs, twrshaft
    4MB @ 0xc0400000 - 0xc07fffff mayjin3, vivdolls, 11beat, starsldr, mtetrisc : unused
    4MB @ 0xc0401000 - 0xc0800fff 11beat, starsldr (odd-page entry, overlaps inputs) : unused
    4KB @ 0xc0800000 - 0xc0800fff ALL (inputs)
    4KB @ 0xc0801000 - 0xc0801fff mayjin3, vivdolls, 11beat, starsldr, mtetrisc : unused
    4KB @ 0xc0810000 - 0xc0810fff 11beat, starsldr, mtetrisc : unused
    4KB @ 0xc0811000 - 0xc0811fff 11beat, starsldr, mtetrisc : unused
    4KB @ 0xc0c00000 - 0xc0c00fff doncdoon, hipai, kurufev, twrshaft, srmvs, mayjin3, vivdolls : unused
    4KB @ 0xc0c01000 - 0xc0c01fff mayjin3, vivdolls : unused
    1MB @ 0xd0000000 - 0xd00fffff mtetrisc : used but write only
    1MB @ 0xd0100000 - 0xd01fffff mtetrisc : unused
    4MB @ 0xd0800000 - 0xd0bfffff doncdoon, hipai, kurufev, twrshaft, srmvs : unused
 */

void aleck64_state::n64_map(address_map &map)
{
	map(0x00000000, 0x007fffff).ram() /*.mirror(0xc0000000)*/ .share("rdram");             // RDRAM

	map(0x03f00000, 0x03f00027).rw("rcp", FUNC(n64_periphs::rdram_reg_r), FUNC(n64_periphs::rdram_reg_w));
	map(0x04000000, 0x04000fff).ram().share("rsp_dmem");                    // RSP DMEM
	map(0x04001000, 0x04001fff).ram().share("rsp_imem");                    // RSP IMEM
	map(0x04040000, 0x040fffff).rw("rcp", FUNC(n64_periphs::sp_reg_r), FUNC(n64_periphs::sp_reg_w));  // RSP
	map(0x04100000, 0x041fffff).rw("rcp", FUNC(n64_periphs::dp_reg_r), FUNC(n64_periphs::dp_reg_w));  // RDP
	map(0x04300000, 0x043fffff).rw("rcp", FUNC(n64_periphs::mi_reg_r), FUNC(n64_periphs::mi_reg_w));    // MIPS Interface
	map(0x04400000, 0x044fffff).rw("rcp", FUNC(n64_periphs::vi_reg_r), FUNC(n64_periphs::vi_reg_w));    // Video Interface
	map(0x04500000, 0x045fffff).rw("rcp", FUNC(n64_periphs::ai_reg_r), FUNC(n64_periphs::ai_reg_w));    // Audio Interface
	map(0x04600000, 0x046fffff).rw("rcp", FUNC(n64_periphs::pi_reg_r), FUNC(n64_periphs::pi_reg_w));    // Peripheral Interface
	map(0x04700000, 0x047fffff).rw("rcp", FUNC(n64_periphs::ri_reg_r), FUNC(n64_periphs::ri_reg_w));    // RDRAM Interface
	map(0x04800000, 0x048fffff).rw("rcp", FUNC(n64_periphs::si_reg_r), FUNC(n64_periphs::si_reg_w));    // Serial Interface
	map(0x10000000, 0x13ffffff).rom().region("user2", 0);   // Cartridge
	map(0x1fc00000, 0x1fc007bf).rom().region("user1", 0);   // PIF ROM
	map(0x1fc007c0, 0x1fc007ff).rw("rcp", FUNC(n64_periphs::pif_ram_r), FUNC(n64_periphs::pif_ram_w));

	map(0xc0000000, 0xc07fffff).ram(); // SDRAM, Aleck 64 specific

	map(0xc0800000, 0xc0800fff).rw(FUNC(aleck64_state::aleck_dips_r), FUNC(aleck64_state::aleck_dips_w));
}

/*
 E90 protection handlers
*/

uint16_t aleck64_state::e90_prot_r()
{
// offset 0 $800 = status ready, active high
	return 0;
}

void aleck64_state::e90_prot_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	switch(offset*2)
	{
		case 0x16:
			if(data != 6 && data != 7)
				logerror("E90: ! %04x %04x %08x\n",offset*2,data,mem_mask);

			if(data & 1) // 0 -> 1 transition
			{
				//for(int i=0;i<0x1000;i+=4)
				//  space.write_dword(0x007502f4+i,space.read_dword(0xd0000000+i));
			}
			break;
		//0x1e bit 0 probably enables the chip
		default:
			logerror("E90: unmapped %04x %04x %08x\n",offset*2,data,mem_mask);
			break;
	}
}

void aleck64_state::e90_map(address_map &map)
{
	n64_map(map);
	map(0xd0000000, 0xd0000fff).ram().share("e90vram");// x/y offsets
	map(0xd0010000, 0xd0010fff).ram().share("e90pal");// RGB555 palette
	map(0xd0030000, 0xd003001f).rw(FUNC(aleck64_state::e90_prot_r), FUNC(aleck64_state::e90_prot_w));
}

void aleck64_state::rsp_imem_map(address_map &map)
{
	map(0x00000000, 0x00000fff).ram().share("rsp_imem");
}

void aleck64_state::rsp_dmem_map(address_map &map)
{
	map(0x00000000, 0x00000fff).ram().share("rsp_dmem");
}

static INPUT_PORTS_START( aleck64 )
	PORT_START("input")
	PORT_BIT( 0xff, 0x05, IPT_CUSTOM )                                     // Tell base driver to expect two gamepads

	PORT_START("P1")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)          // Button A
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)          // Button B
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)          // Button Z
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_START1 )                          // Start
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1)      // Joypad Up
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)    // Joypad Down
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)    // Joypad Left
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)   // Joypad Right
	PORT_BIT( 0x00c0, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(1)          // Pan Left
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(1)          // Pan Right
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER(1)          // C Button Up
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_PLAYER(1)          // C Button Down
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_PLAYER(1)          // C Button Left
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_BUTTON9 ) PORT_PLAYER(1)          // C Button Right

	PORT_START("P1_ANALOG_X")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_PLAYER(1)

	PORT_START("P1_ANALOG_Y")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_PLAYER(1) PORT_REVERSE

	PORT_START("P2")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)          // Button A
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)          // Button B
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)          // Button Z
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_START2 )                          // Start
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2)      // Joypad Up
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)    // Joypad Down
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)    // Joypad Left
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)   // Joypad Right
	PORT_BIT( 0x00c0, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(2)          // Pan Left
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(2)          // Pan Right
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER(2)          // C Button Up
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_PLAYER(2)          // C Button Down
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_PLAYER(2)          // C Button Left
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_BUTTON9 ) PORT_PLAYER(2)          // C Button Right

	PORT_START("P2_ANALOG_X")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_PLAYER(2)

	PORT_START("P2_ANALOG_Y")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_PLAYER(2) PORT_REVERSE

	PORT_START("IN0")
	PORT_DIPNAME( 0x80000000, 0x80000000, "DIPSW1 #8" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING( 0x80000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x40000000, 0x40000000, "DIPSW1 #7" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING( 0x40000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x20000000, 0x20000000, "DIPSW1 #6" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING( 0x20000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x10000000, 0x10000000, "DIPSW1 #5" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING( 0x10000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x08000000, 0x08000000, "DIPSW1 #4" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING( 0x08000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x04000000, 0x04000000, "DIPSW1 #3" ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING( 0x04000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x02000000, 0x02000000, "DIPSW1 #2" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING( 0x02000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x01000000, 0x01000000, "DIPSW1 #1" ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING( 0x01000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00800000, 0x00800000, "Test Mode" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING( 0x00800000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00400000, 0x00400000, "DIPSW2 #7" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING( 0x00400000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00200000, 0x00200000, "DIPSW2 #6" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING( 0x00200000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00100000, 0x00100000, "DIPSW2 #5" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING( 0x00100000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00080000, 0x00080000, "DIPSW2 #4" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING( 0x00080000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00040000, 0x00040000, "DIPSW2 #3" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING( 0x00040000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00020000, 0x00020000, "DIPSW2 #2" ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING( 0x00020000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00010000, 0x00010000, "DIPSW2 #1" ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING( 0x00010000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_BIT(0x0000ffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0xff00ffff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE_NO_TOGGLE( 0x00200000, IP_ACTIVE_LOW )
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Service Button")
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_COIN1 )
INPUT_PORTS_END

static INPUT_PORTS_START( vivdolls )
	PORT_INCLUDE( aleck64 )

	PORT_MODIFY("P1")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_CONDITION("IN0", 0x00100000, EQUALS, 0x00100000)          // Button A
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_CONDITION("IN0", 0x00100000, EQUALS, 0x00100000)          // Button B
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_CONDITION("IN0", 0x00100000, EQUALS, 0x00100000)          // Button Z
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CONDITION("IN0", 0x00100000, EQUALS, 0x00100000)                          // Start
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_CONDITION("IN0", 0x00100000, EQUALS, 0x00100000)      // Joypad Up
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_CONDITION("IN0", 0x00100000, EQUALS, 0x00100000)    // Joypad Down
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_CONDITION("IN0", 0x00100000, EQUALS, 0x00100000)    // Joypad Left
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_CONDITION("IN0", 0x00100000, EQUALS, 0x00100000)   // Joypad Right
	PORT_BIT( 0x00c0, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_CONDITION("IN0", 0x00100000, EQUALS, 0x00100000)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_CONDITION("IN0", 0x00100000, EQUALS, 0x00100000)          // Pan Left
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_CONDITION("IN0", 0x00100000, EQUALS, 0x00100000)          // Pan Right
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER(1) PORT_CONDITION("IN0", 0x00100000, EQUALS, 0x00100000)          // C Button Up
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_PLAYER(1) PORT_CONDITION("IN0", 0x00100000, EQUALS, 0x00100000)          // C Button Down
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_PLAYER(1) PORT_CONDITION("IN0", 0x00100000, EQUALS, 0x00100000)          // C Button Left
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_BUTTON9 ) PORT_PLAYER(1) PORT_CONDITION("IN0", 0x00100000, EQUALS, 0x00100000)          // C Button Right

	PORT_MODIFY("P1_ANALOG_X")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_PLAYER(1) PORT_CONDITION("IN0", 0x00100000, EQUALS, 0x00100000)

	PORT_MODIFY("P1_ANALOG_Y")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_PLAYER(1) PORT_REVERSE PORT_CONDITION("IN0", 0x00100000, EQUALS, 0x00100000)

	PORT_MODIFY("P2")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_CONDITION("IN0", 0x00100000, EQUALS, 0x00100000)          // Button A
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_CONDITION("IN0", 0x00100000, EQUALS, 0x00100000)          // Button B
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_CONDITION("IN0", 0x00100000, EQUALS, 0x00100000)          // Button Z
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_START2 ) PORT_CONDITION("IN0", 0x00100000, EQUALS, 0x00100000)                          // Start
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2) PORT_CONDITION("IN0", 0x00100000, EQUALS, 0x00100000)      // Joypad Up
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2) PORT_CONDITION("IN0", 0x00100000, EQUALS, 0x00100000)    // Joypad Down
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) PORT_CONDITION("IN0", 0x00100000, EQUALS, 0x00100000)    // Joypad Left
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_CONDITION("IN0", 0x00100000, EQUALS, 0x00100000)   // Joypad Right
	PORT_BIT( 0x00c0, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_CONDITION("IN0", 0x00100000, EQUALS, 0x00100000)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_CONDITION("IN0", 0x00100000, EQUALS, 0x00100000)          // Pan Left
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(2) PORT_CONDITION("IN0", 0x00100000, EQUALS, 0x00100000)          // Pan Right
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER(2) PORT_CONDITION("IN0", 0x00100000, EQUALS, 0x00100000)          // C Button Up
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_PLAYER(2) PORT_CONDITION("IN0", 0x00100000, EQUALS, 0x00100000)          // C Button Down
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_PLAYER(2) PORT_CONDITION("IN0", 0x00100000, EQUALS, 0x00100000)          // C Button Left
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_BUTTON9 ) PORT_PLAYER(2) PORT_CONDITION("IN0", 0x00100000, EQUALS, 0x00100000)          // C Button Right

	PORT_MODIFY("P2_ANALOG_X")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_PLAYER(2) PORT_CONDITION("IN0", 0x00100000, EQUALS, 0x00100000)

	PORT_MODIFY("P2_ANALOG_Y")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_PLAYER(2) PORT_REVERSE PORT_CONDITION("IN0", 0x00100000, EQUALS, 0x00100000)

	PORT_MODIFY("IN0")
	PORT_DIPNAME( 0x80000000, 0x80000000, "Unused" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING( 0x80000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x40000000, 0x40000000, "Unused" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING( 0x40000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x20000000, 0x20000000, "Unused" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING( 0x20000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x10000000, 0x10000000, "Unused" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING( 0x10000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x08000000, 0x08000000, "Unused" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING( 0x08000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x07000000, 0x07000000, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:3,2,1")
	PORT_DIPSETTING( 0x07000000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING( 0x03000000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING( 0x05000000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING( 0x01000000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING( 0x06000000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING( 0x02000000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING( 0x04000000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( 5C_1C ) )
	PORT_DIPNAME( 0x00800000, 0x00800000, "Test Mode" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING( 0x00800000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00400000, 0x00400000, "Unused" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING( 0x00400000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00200000, 0x00200000, "Qualify Area" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING( 0x00200000, "70%" )
	PORT_DIPSETTING( 0x00000000, "80%" )
	PORT_DIPNAME( 0x00100000, 0x00100000, "Controls" ) PORT_DIPLOCATION("SW2:5") // Marked as unused always-off in manual, but off won't work on a JAMMA cabinet
	PORT_DIPSETTING( 0x00100000, "JOYSTICK" )
	PORT_DIPSETTING( 0x00000000, "JAMMASTICK" )
	PORT_DIPNAME( 0x000c0000, 0x000c0000, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:4,3")
	PORT_DIPSETTING( 0x000c0000, "4" )
	PORT_DIPSETTING( 0x00040000, "3" )
	PORT_DIPSETTING( 0x00080000, "2" )
	PORT_DIPSETTING( 0x00000000, "1" )
	PORT_DIPNAME( 0x00030000, 0x00030000, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:2,1")
	PORT_DIPSETTING( 0x00030000, DEF_STR( Normal ) )
	PORT_DIPSETTING( 0x00010000, DEF_STR( Easy ) )
	PORT_DIPSETTING( 0x00020000, DEF_STR( Hard ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( Hardest ) )
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("IN0", 0x00100000, NOTEQUALS, 0x00100000)
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("IN0", 0x00100000, NOTEQUALS, 0x00100000)
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("IN0", 0x00100000, NOTEQUALS, 0x00100000)
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_CONDITION("IN0", 0x00100000, NOTEQUALS, 0x00100000)
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_CONDITION("IN0", 0x00100000, NOTEQUALS, 0x00100000)
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) PORT_CONDITION("IN0", 0x00100000, NOTEQUALS, 0x00100000)
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2) PORT_CONDITION("IN0", 0x00100000, NOTEQUALS, 0x00100000)
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2) PORT_CONDITION("IN0", 0x00100000, NOTEQUALS, 0x00100000)
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("IN0", 0x00100000, NOTEQUALS, 0x00100000)
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("IN0", 0x00100000, NOTEQUALS, 0x00100000)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("IN0", 0x00100000, NOTEQUALS, 0x00100000)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_CONDITION("IN0", 0x00100000, NOTEQUALS, 0x00100000)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_CONDITION("IN0", 0x00100000, NOTEQUALS, 0x00100000)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_CONDITION("IN0", 0x00100000, NOTEQUALS, 0x00100000)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_CONDITION("IN0", 0x00100000, NOTEQUALS, 0x00100000)
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_CONDITION("IN0", 0x00100000, NOTEQUALS, 0x00100000)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_START2 ) PORT_CONDITION("IN0", 0x00100000, NOTEQUALS, 0x00100000)
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_START1 ) PORT_CONDITION("IN0", 0x00100000, NOTEQUALS, 0x00100000)
INPUT_PORTS_END

static INPUT_PORTS_START( 11beat )
	PORT_INCLUDE( aleck64 )

	PORT_MODIFY("P1")
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0f00, IP_ACTIVE_LOW, IPT_UNUSED ) // "joystick type error" happens because game expects D-PADs to be unconnected
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(1)

	PORT_MODIFY("P2")
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0f00, IP_ACTIVE_LOW, IPT_UNUSED ) // "joystick type error" happens because game expects D-PADs to be unconnected
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(2)
INPUT_PORTS_END

static INPUT_PORTS_START( mtetrisc )

	// The basic N64 controls are unused in this game
	PORT_START("input")
	PORT_BIT( 0xff, 0x00, IPT_CUSTOM )

	PORT_START("INMJ")

	PORT_START("IN0")
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)

	PORT_START("IN1")
	PORT_BIT( 0xffcc, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE_NO_TOGGLE( 0x0020, IP_ACTIVE_LOW )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Service Button")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
INPUT_PORTS_END

static INPUT_PORTS_START( starsldr )
	PORT_START("input")
	PORT_BIT( 0xff, 0x05, IPT_CUSTOM )                                     // Tell base driver to expect two gamepads

	PORT_START("P1")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)          // Button A
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)          // Button B
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_START1 )                          // Start
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1)      // Joypad Up
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)    // Joypad Down
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)    // Joypad Left
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)   // Joypad Right
	PORT_BIT( 0x00c0, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)          // Button C
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(1)          // Button D

	PORT_START("P1_ANALOG_X")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_PLAYER(1)

	PORT_START("P1_ANALOG_Y")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0xff,0x00) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_PLAYER(1)

	PORT_START("P2")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)          // Button A
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)          // Button B
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_START2 )                          // Start
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2)      // Joypad Up
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)    // Joypad Down
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)    // Joypad Left
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)   // Joypad Right
	PORT_BIT( 0x00c0, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)          // Button C
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(2)          // Button D

	PORT_START("P2_ANALOG_X")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_PLAYER(2)

	PORT_START("P2_ANALOG_Y")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0xff,0x00) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_PLAYER(2)


	PORT_START("IN0")
	PORT_DIPNAME( 0x80000000, 0x00000000, DEF_STR(Joystick) )   PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING( 0x00000000, DEF_STR(Joystick) )
	PORT_DIPSETTING( 0x80000000, "3D" )
	PORT_DIPNAME( 0x60000000, 0x60000000, "Auto Level" )        PORT_DIPLOCATION("SW1:6,7")
	PORT_DIPSETTING( 0x60000000, DEF_STR(Normal) )
	PORT_DIPSETTING( 0x40000000, "Slow" )
	PORT_DIPSETTING( 0x20000000, "Fast1" )
	PORT_DIPSETTING( 0x00000000, "Fast2" )
	PORT_DIPNAME( 0x18000000, 0x18000000, "Player" )            PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING( 0x18000000, "3" )
	PORT_DIPSETTING( 0x10000000, "4" )
	PORT_DIPSETTING( 0x08000000, "2" )
	PORT_DIPSETTING( 0x00000000, "1" )
	PORT_DIPNAME( 0x07000000, 0x07000000, DEF_STR(Coinage) )    PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING( 0x00000000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING( 0x01000000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING( 0x02000000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING( 0x03000000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING( 0x07000000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING( 0x06000000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING( 0x05000000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING( 0x04000000, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x00800000, 0x00800000, "DIPSW2 #8" )         PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING( 0x00800000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00400000, 0x00400000, DEF_STR(Language) )   PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING( 0x00400000, DEF_STR(English) )
	PORT_DIPSETTING( 0x00000000, DEF_STR(Japanese) )
	PORT_DIPNAME( 0x00200000, 0x00000000, "Demosound" )         PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING( 0x00200000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00100000, 0x00100000, "Rapid" )             PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING( 0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00100000, DEF_STR( On ) )
	PORT_DIPNAME( 0x000c0000, 0x000c0000, "Extend" )            PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING( 0x000c0000, "Every 30000000" )
	PORT_DIPSETTING( 0x00080000, "Every 50000000" )
	PORT_DIPSETTING( 0x00040000, "Every 70000000" )
	PORT_DIPSETTING( 0x00000000, "Non" )
	PORT_DIPNAME( 0x00030000, 0x00030000, DEF_STR(Difficulty) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING( 0x00030000, DEF_STR(Normal) )
	PORT_DIPSETTING( 0x00020000, DEF_STR(Easy) )
	PORT_DIPSETTING( 0x00010000, "Hard1" )
	PORT_DIPSETTING( 0x00000000, "Hard2" )
	PORT_BIT(0x0000ffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0xff00ffff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE_NO_TOGGLE( 0x00200000, IP_ACTIVE_LOW )
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Service Button")
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_COIN1 )
INPUT_PORTS_END

static INPUT_PORTS_START( doncdoon )
	PORT_START("input")
	PORT_BIT( 0xff, 0x00, IPT_CUSTOM ) // Disable standard N64 controls

	PORT_START("IN0")
	PORT_BIT(0xfcff8080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x02000000, 0x02000000, "DIP SW2" )
	PORT_DIPSETTING( 0x02000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x01000000, 0x01000000, "Test Mode" )
	PORT_DIPSETTING( 0x01000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)

	PORT_START("IN1")
	PORT_BIT(0xffc0ffff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Test Button")
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Service Button")
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_START1 )
INPUT_PORTS_END

static INPUT_PORTS_START( kurufev )
	PORT_INCLUDE( doncdoon )

	PORT_MODIFY("IN0")
	PORT_BIT(0x00004040, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( twrshaft )
	PORT_START("input")
	PORT_BIT( 0xff, 0x00, IPT_CUSTOM ) // Disable standard N64 controls

	PORT_START("INMJ")

	PORT_START("IN0")
	PORT_BIT(0xff7fffe0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_DIPNAME( 0x00800000, 0x00800000, "Test Mode" )
	PORT_DIPSETTING( 0x00800000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Test Button")
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Service Button")
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_START1 )
INPUT_PORTS_END

static INPUT_PORTS_START( hipai )
	PORT_START("input")
	PORT_BIT( 0xff, 0x00, IPT_CUSTOM ) // Disable standard N64 controls

	PORT_START("INMJ")
	PORT_BIT( 0xe1c1c0c1, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_MAHJONG_RON )

	PORT_START("IN0")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x80000000, 0x80000000, DEF_STR( Free_Play ) )        PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING( 0x80000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x40000000, 0x40000000, DEF_STR( Unused ) )           PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING( 0x40000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x20000000, 0x20000000, DEF_STR( Unused ) )           PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING( 0x20000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x10000000, 0x10000000, DEF_STR( Unused ) )           PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING( 0x10000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x08000000, 0x08000000, DEF_STR( Unused ) )           PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING( 0x08000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x07000000, 0x07000000, DEF_STR( Coinage ) )          PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING( 0x00000000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING( 0x01000000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING( 0x02000000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING( 0x03000000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING( 0x07000000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING( 0x06000000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING( 0x05000000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING( 0x04000000, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x00800000, 0x00800000, DEF_STR( Test ) )             PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING( 0x00800000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00400000, 0x00400000, DEF_STR( Unused ) )           PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING( 0x00400000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00200000, 0x00000000, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING( 0x00200000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00100000, 0x00100000, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING( 0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00100000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00080000, 0x00080000, "Kuitan" )                    PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING( 0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00080000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00070000, 0x00070000, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING( 0x00000000, DEF_STR( Hardest ) )
	PORT_DIPSETTING( 0x00010000, DEF_STR( Very_Hard ) )
	PORT_DIPSETTING( 0x00020000, DEF_STR( Hard ) )
	PORT_DIPSETTING( 0x00030000, "Normal+" )
	PORT_DIPSETTING( 0x00040000, DEF_STR( Easy ) )
	PORT_DIPSETTING( 0x00050000, DEF_STR( Very_Easy ) )
	PORT_DIPSETTING( 0x00060000, DEF_STR( Easiest ) )
	PORT_DIPSETTING( 0x00070000, DEF_STR( Normal ) )

	PORT_START("IN1")
	PORT_BIT( 0xffebffff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Service Button")
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_COIN1 )
INPUT_PORTS_END

static INPUT_PORTS_START( hipai2 )
	PORT_INCLUDE( hipai )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x80000000, 0x80000000, DEF_STR( Free_Play ) )        PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING( 0x80000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x40000000, 0x40000000, "Backup Settings" )           PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING( 0x40000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x20000000, 0x20000000, DEF_STR( Unused ) )           PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING( 0x20000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x10000000, 0x10000000, DEF_STR( Unused ) )           PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING( 0x10000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x08000000, 0x08000000, DEF_STR( Unused ) )           PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING( 0x08000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x07000000, 0x07000000, DEF_STR( Coinage ) )          PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING( 0x00000000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING( 0x01000000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING( 0x02000000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING( 0x03000000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING( 0x07000000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING( 0x06000000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING( 0x05000000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING( 0x04000000, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x00800000, 0x00800000, DEF_STR( Test ) )             PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING( 0x00800000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00400000, 0x00400000, DEF_STR( Unused ) )           PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING( 0x00400000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00200000, 0x00000000, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING( 0x00200000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00100000, 0x00100000, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING( 0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00100000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00080000, 0x00080000, "Kuitan" )                    PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING( 0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00080000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00070000, 0x00070000, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING( 0x00000000, DEF_STR( Hardest ) )
	PORT_DIPSETTING( 0x00010000, DEF_STR( Very_Hard ) )
	PORT_DIPSETTING( 0x00020000, DEF_STR( Hard ) )
	PORT_DIPSETTING( 0x00030000, "Normal+" )
	PORT_DIPSETTING( 0x00040000, DEF_STR( Easy ) )
	PORT_DIPSETTING( 0x00050000, DEF_STR( Very_Easy ) )
	PORT_DIPSETTING( 0x00060000, DEF_STR( Easiest ) )
	PORT_DIPSETTING( 0x00070000, DEF_STR( Normal ) )
INPUT_PORTS_END

static INPUT_PORTS_START( srmvs )
	PORT_INCLUDE( hipai )

	PORT_MODIFY("IN0")
	PORT_DIPNAME( 0x80000000, 0x80000000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING( 0x80000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x40000000, 0x40000000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING( 0x40000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x20000000, 0x20000000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING( 0x20000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x10000000, 0x10000000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING( 0x10000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x08000000, 0x08000000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING( 0x08000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00400000, 0x00400000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING( 0x00400000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00100000, 0x00100000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING( 0x00100000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00080000, 0x00080000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING( 0x00080000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00040000, 0x00040000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING( 0x00040000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00020000, 0x00020000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING( 0x00020000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00010000, 0x00010000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING( 0x00010000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
INPUT_PORTS_END


void aleck64_state::aleck64(machine_config &config)
{
	/* basic machine hardware */
	VR4300BE(config, m_vr4300, 93750000);
	m_vr4300->set_icache_size(16384);
	m_vr4300->set_dcache_size(8192);
	m_vr4300->set_system_clock(62500000);
	m_vr4300->set_addrmap(AS_PROGRAM, &aleck64_state::n64_map);
	m_vr4300->set_force_no_drc(false);

	RSP(config, m_rsp, 62500000);
	m_rsp->dp_reg_r().set(m_rcp_periphs, FUNC(n64_periphs::dp_reg_r));
	m_rsp->dp_reg_w().set(m_rcp_periphs, FUNC(n64_periphs::dp_reg_w));
	m_rsp->sp_reg_r().set(m_rcp_periphs, FUNC(n64_periphs::sp_reg_r));
	m_rsp->sp_reg_w().set(m_rcp_periphs, FUNC(n64_periphs::sp_reg_w));
	m_rsp->status_set().set(m_rcp_periphs, FUNC(n64_periphs::sp_set_status));
	m_rsp->set_addrmap(AS_PROGRAM, &aleck64_state::rsp_imem_map);
	m_rsp->set_addrmap(AS_DATA, &aleck64_state::rsp_dmem_map);
	m_rsp->set_force_no_drc(false);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(DACRATE_NTSC*2,3093,0,3093,525,0,525);
	m_screen->set_screen_update(FUNC(n64_state::screen_update));
	m_screen->screen_vblank().set(FUNC(n64_state::screen_vblank));

	PALETTE(config, "palette").set_entries(0x1000);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	DMADAC(config, "dac1").add_route(ALL_OUTPUTS, "lspeaker", 1.0);
	DMADAC(config, "dac2").add_route(ALL_OUTPUTS, "rspeaker", 1.0);

	N64PERIPH(config, m_rcp_periphs, 0);
}

uint32_t aleck64_state::screen_update_e90(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	n64_state::screen_update(screen, bitmap, cliprect);
	// TODO: extract from the real tables (maybe RLEd inside paletteram words 0xa - 0xf?)
	static constexpr u8 pal_table[8*8] =
	{
		8, 6, 6, 6, 6, 5, 4, 3,
		9, 7, 5, 6, 4, 1, 1, 4,
		9, 8, 6, 5, 1, 1, 1, 5,
		9, 8, 7, 5, 1, 1, 4, 6,
		9, 8, 7, 6, 5, 5, 6, 6,
		9, 8, 6, 7, 7, 6, 5, 6,
		9, 8, 8, 8, 8, 8, 7, 6,
		8, 9, 9, 9, 9, 9, 9, 8
	};

	for(int offs = 0; offs < 0x1000 / 4; offs+=2)
	{
		// 0x400 is another enable? end code if off?
		//uint16_t tile = m_e90_vram[offs] >> 16;

		// TODO: mask 0x300 on tile seems to be tile number
		// (active piece drawn with 0x100, special square blocks drawn with cycling 0x1c0, 0x200 & 0x240)
		uint16_t attr = m_e90_vram[offs] & 0xffff;
		// bit 5 disables?
		if(attr & 0x20)
			continue;

		// guess: 0x1000 entries / word / 4bpp = 0x7f, divided by two below
		// bits 5 and 6 of palette bank seems to be highlight and shadow, separated from this bank
		uint16_t pal = (attr & 0x06) >> 1;
		int16_t x = m_e90_vram[offs+1] >> 16;
		int16_t y = m_e90_vram[offs+1] & 0xffff;
		x>>=1;
		// ghost piece, probably enabled thru one bit of these (as weird as it sounds)
		if((y & 0xff00) == 0xbc00)
			pal |= (0x40 >> 1);

		// color banks
		// some pieces needs this color adjustment (see T piece / 5 block version of I piece)
		pal |= (attr & 0xc0) >> 4;

		for(int yi=0;yi<8;yi++)
		{
			for(int xi=0;xi<8;xi++)
			{
				int res_x = x+xi + 4;
				int res_y = (y & 0xff)+yi + 7;

				int pal_offs = (pal*0x10);
				pal_offs+= pal_table[xi+yi*8];
				int pal_shift = pal_offs & 1 ? 0 : 16;
				uint16_t raw_rgb = m_e90_pal[pal_offs>>1] >> pal_shift;
				int r = (raw_rgb & 0x001f) >> 0;
				int g = (raw_rgb & 0x03e0) >> 5;
				int b = (raw_rgb & 0x7c00) >> 10;
				r = pal5bit(r);
				g = pal5bit(g);
				b = pal5bit(b);

				if(cliprect.contains(res_x, res_y))
					bitmap.pix(res_y, res_x) = r << 16 | g << 8 | b;
			}
		}
	}
	return 0;
}

void aleck64_state::a64_e90(machine_config &config)
{
	aleck64(config);
	m_vr4300->set_addrmap(AS_PROGRAM, &aleck64_state::e90_map);

	subdevice<screen_device>("screen")->set_screen_update(FUNC(aleck64_state::screen_update_e90));
}

void aleck64_state::init_aleck64()
{
	uint8_t *rom = memregion("user2")->base();

	rom[0x67c] = 0;
	rom[0x67d] = 0;
	rom[0x67e] = 0;
	rom[0x67f] = 0;
}

#define PIF_BOOTROM ROM_LOAD( "pifdata.bin", 0x0000, 0x0800, CRC(5ec82be9) SHA1(9174eadc0f0ea2654c95fd941406ab46b9dc9bdd) )

ROM_START( aleck64 )
	ROM_REGION32_BE( 0x800, "user1", 0 )
	ROM_LOAD( "pifdata.bin", 0x0000, 0x0800, CRC(5ec82be9) SHA1(9174eadc0f0ea2654c95fd941406ab46b9dc9bdd) )

	ROM_REGION32_BE( 0x4000000, "user2", ROMREGION_ERASE00 )
	ROM_REGION32_BE( 0x100000, "user3", ROMREGION_ERASE00 )
	ROM_REGION32_BE( 0x80, "user4", ROMREGION_ERASE00 )

	ROM_REGION16_BE( 0x80, "normpoint", 0 )
	ROM_LOAD( "normpnt.rom", 0x00, 0x80, CRC(e7f2a005) SHA1(c27b4a364a24daeee6e99fd286753fd6216362b4) )

	ROM_REGION16_BE( 0x80, "normslope", 0 )
	ROM_LOAD( "normslp.rom", 0x00, 0x80, CRC(4f2ae525) SHA1(eab43f8cc52c8551d9cff6fced18ef80eaba6f05) )
ROM_END

ROM_START( 11beat )
	ROM_REGION32_BE( 0x800, "user1", ROMREGION_ERASE00 )
	PIF_BOOTROM

	ROM_REGION32_BE( 0x4000000, "user2", 0 )
	ROM_LOAD16_WORD_SWAP( "nus-zhaj.u3", 0x000000, 0x0800000, CRC(02faa8a7) SHA1(824911452639cedf6a8186c05cd046e61fc98896) )

	ROM_REGION16_BE( 0x80, "normpoint", 0 )
	ROM_LOAD( "normpnt.rom", 0x00, 0x80, CRC(e7f2a005) SHA1(c27b4a364a24daeee6e99fd286753fd6216362b4) )

	ROM_REGION16_BE( 0x80, "normslope", 0 )
	ROM_LOAD( "normslp.rom", 0x00, 0x80, CRC(4f2ae525) SHA1(eab43f8cc52c8551d9cff6fced18ef80eaba6f05) )
ROM_END

ROM_START( mtetrisc )
	ROM_REGION32_BE( 0x800, "user1", ROMREGION_ERASE00 )
	PIF_BOOTROM

	ROM_REGION32_BE( 0x4000000, "user2", 0 )
	ROM_LOAD16_WORD_SWAP( "nus-zcaj.u4", 0x000000, 0x1000000, CRC(c9de64db) SHA1(59932c70b43ff8e9264c670f37b3abbe939b7f95) )
	ROM_REGION32_BE( 0x100000, "user3", 0 )
	ROM_LOAD ( "tet-01m.u5", 0x000000, 0x100000, CRC(f78f859b) SHA1(b07c85e0453869fe43792f42081f64a5327e58e6) )

	ROM_REGION32_BE( 0x80, "user4", 0 )
	ROM_LOAD ( "at24c01.u34", 0x000000, 0x80, CRC(ba7e503f) SHA1(454aa4fdde7d8694d1affaf25cd750fa678686bb) )

	ROM_REGION16_BE( 0x80, "normpoint", 0 )
	ROM_LOAD( "normpnt.rom", 0x00, 0x80, CRC(e7f2a005) SHA1(c27b4a364a24daeee6e99fd286753fd6216362b4) )

	ROM_REGION16_BE( 0x80, "normslope", 0 )
	ROM_LOAD( "normslp.rom", 0x00, 0x80, CRC(4f2ae525) SHA1(eab43f8cc52c8551d9cff6fced18ef80eaba6f05) )
ROM_END


/*
Star Soldier Vanishing Earth for Aleck64
Uses standard PCB '3D ROM PCB-2A' containing... (PCB Labels in brackets)
1x Macronix MX23C9602CZ 96MBit Serial EEPROM @ U3 (NUS96M)
1x CIC-NUS-5101 Protection Chip (CIC-NUS)
1x BK4D-NUS (BU9850)
*/

ROM_START( starsldr )
	ROM_REGION32_BE( 0x800, "user1", ROMREGION_ERASE00 )
	PIF_BOOTROM

	ROM_REGION32_BE( 0x4000000, "user2", 0 )
	ROM_LOAD16_WORD_SWAP( "nus-zhbj-0.u3", 0x000000, 0xc00000, CRC(a4edac93) SHA1(3794606c008fb69f5d16dcccece94d03da23bf8a) )

	ROM_REGION16_BE( 0x80, "normpoint", 0 )
	ROM_LOAD( "normpnt.rom", 0x00, 0x80, CRC(e7f2a005) SHA1(c27b4a364a24daeee6e99fd286753fd6216362b4) )

	ROM_REGION16_BE( 0x80, "normslope", 0 )
	ROM_LOAD( "normslp.rom", 0x00, 0x80, CRC(4f2ae525) SHA1(eab43f8cc52c8551d9cff6fced18ef80eaba6f05) )
ROM_END

ROM_START( srmvs )
	ROM_REGION32_BE( 0x800, "user1", ROMREGION_ERASE00 )
	PIF_BOOTROM

	ROM_REGION32_BE( 0x4000000, "user2", 0 )
	ROM_LOAD16_WORD_SWAP( "nus-zsej-1.u2", 0x000000, 0x2000000, CRC(dd7dd81f) SHA1(ff020fa957a11bf0f18819d16a010f51b4a4cda4) )

	ROM_REGION16_BE( 0x80, "normpoint", 0 )
	ROM_LOAD( "normpnt.rom", 0x00, 0x80, CRC(e7f2a005) SHA1(c27b4a364a24daeee6e99fd286753fd6216362b4) )

	ROM_REGION16_BE( 0x80, "normslope", 0 )
	ROM_LOAD( "normslp.rom", 0x00, 0x80, CRC(4f2ae525) SHA1(eab43f8cc52c8551d9cff6fced18ef80eaba6f05) )
ROM_END

ROM_START( srmvsa )
	ROM_REGION32_BE( 0x800, "user1", ROMREGION_ERASE00 )
	PIF_BOOTROM

	ROM_REGION32_BE( 0x4000000, "user2", 0 )
	ROM_LOAD16_WORD_SWAP( "nus-zsej-0.u2", 0x000000, 0x2000000, CRC(44f40102) SHA1(a78de955f2fcd99dda14e782984368b320eb5415) )

	ROM_REGION16_BE( 0x80, "normpoint", 0 )
	ROM_LOAD( "normpnt.rom", 0x00, 0x80, CRC(e7f2a005) SHA1(c27b4a364a24daeee6e99fd286753fd6216362b4) )

	ROM_REGION16_BE( 0x80, "normslope", 0 )
	ROM_LOAD( "normslp.rom", 0x00, 0x80, CRC(4f2ae525) SHA1(eab43f8cc52c8551d9cff6fced18ef80eaba6f05) )
ROM_END


/*
Vivid Dolls for Aleck64
Uses standard PCB '3D ROM PCB-2A' containing... (PCB Labels in brackets)
1x CIC-NUS-5101 Protection Chip (U1 CIC-NUS)
1x BK4D-NUS (U2 BU9850)
1x Macronix MX23C6402CZ 64MBit Serial EEPROM (U3 NUS64M)
*/

ROM_START( vivdolls )
	ROM_REGION32_BE( 0x800, "user1", ROMREGION_ERASE00 )
	PIF_BOOTROM

	ROM_REGION32_BE( 0x4000000, "user2", 0 )
	ROM_LOAD16_WORD_SWAP( "nus-zsaj-0.u3", 0x000000, 0x800000, CRC(f3220e29) SHA1(06d8b808cc19378b046803f4dc75c7d791b7767f) )

	ROM_REGION16_BE( 0x80, "normpoint", 0 )
	ROM_LOAD( "normpnt.rom", 0x00, 0x80, CRC(e7f2a005) SHA1(c27b4a364a24daeee6e99fd286753fd6216362b4) )

	ROM_REGION16_BE( 0x80, "normslope", 0 )
	ROM_LOAD( "normslp.rom", 0x00, 0x80, CRC(4f2ae525) SHA1(eab43f8cc52c8551d9cff6fced18ef80eaba6f05) )
ROM_END


/*
Tower And Shaft for Aleck64
Uses custom PCB 'ALK-ROM01 ARUZE' containing... (PCB Labels in brackets)
1x Macronix MX23C6402CZ 64MBit Serial EEPROM (U1)  Note - This ROM is mostly empty.
1x Altera MAX EPM3128ATC144-10 CPLD/FPGA labelled AL01 (U2)
1x Fujitsu MSP55LV128 128MBit SOP44 FlashROM labelled UA3012-ALL02-U3 (U3)
1x BK4D-NUS BU9850 (U5)
1x CIC-NUS-5101 Protection Chip (U6)
1x 74F245 Logic Chip
Some transistors
*/

ROM_START( twrshaft )
	ROM_REGION32_BE( 0x800, "user1", ROMREGION_ERASE00 )
	PIF_BOOTROM

	ROM_REGION32_BE( 0x4000000, "user2", 0 )
	ROM_LOAD16_WORD_SWAP( "ua3012-all02.u3", 0x000000, 0x1000000, CRC(904a91a7) SHA1(7dfa3447d2c489c0448c4004dc12d3037c05a0f3) )

	ROM_REGION32_BE( 0x800000, "user3", 0 )
	ROM_LOAD16_WORD_SWAP( "nus-zsij-0.u1", 0x000000, 0x800000, CRC(2389576f) SHA1(dc22b2eab4d7a02cb918827a62e6c120b3a84e6c) )

	ROM_REGION16_BE( 0x80, "normpoint", 0 )
	ROM_LOAD( "normpnt.rom", 0x00, 0x80, CRC(e7f2a005) SHA1(c27b4a364a24daeee6e99fd286753fd6216362b4) )

	ROM_REGION16_BE( 0x80, "normslope", 0 )
	ROM_LOAD( "normslp.rom", 0x00, 0x80, CRC(4f2ae525) SHA1(eab43f8cc52c8551d9cff6fced18ef80eaba6f05) )
ROM_END


ROM_START( hipai )
	ROM_REGION32_BE( 0x800, "user1", ROMREGION_ERASE00 )
	PIF_BOOTROM

	ROM_REGION32_BE( 0x4000000, "user2", 0 )
	ROM_LOAD16_WORD_SWAP( "ua2011-all02.u3", 0x0000000, 0x1000000, CRC(eb4b96d0) SHA1(e909ea5b71b81087da07821c4f57244576363678) )
	ROM_LOAD16_WORD_SWAP( "ua2011-alh02.u4", 0x1000000, 0x1000000, CRC(b8e35ddf) SHA1(7c3e59f6520dc3f0aa592e682fa82e30ffd1f4d0) )

	ROM_REGION32_BE( 0x800000, "user3", 0 )
	ROM_LOAD16_WORD_SWAP( "nus-nsij-0.u1", 0x000000, 0x800000, CRC(94cf9f8d) SHA1(cd624d1f5de2be3bec3ece06556a2e39bef66d77) )

	ROM_REGION16_BE( 0x80, "normpoint", 0 )
	ROM_LOAD( "normpnt.rom", 0x00, 0x80, CRC(e7f2a005) SHA1(c27b4a364a24daeee6e99fd286753fd6216362b4) )

	ROM_REGION16_BE( 0x80, "normslope", 0 )
	ROM_LOAD( "normslp.rom", 0x00, 0x80, CRC(4f2ae525) SHA1(eab43f8cc52c8551d9cff6fced18ef80eaba6f05) )
ROM_END


ROM_START( hipai2 )
	ROM_REGION32_BE( 0x800, "user1", ROMREGION_ERASE00 )
	PIF_BOOTROM

	ROM_REGION32_BE( 0x4000000, "user2", 0 )
	ROM_LOAD16_WORD_SWAP( "ua3029-all01.u3", 0x0000000, 0x1000000, CRC(88b83e93) SHA1(279a006436069232383b2065e7416d8ccc76e3ca) )
	ROM_LOAD16_WORD_SWAP( "ua3029-alh01.u4", 0x1000000, 0x1000000, CRC(4f79fa94) SHA1(a6d2d805f96f0ad7294ebf41b13573320154b2a4) )

	ROM_REGION32_BE( 0x800000, "user3", 0 )
	ROM_LOAD16_WORD_SWAP( "nus-zsij-0.u1", 0x000000, 0x800000, CRC(2389576f) SHA1(dc22b2eab4d7a02cb918827a62e6c120b3a84e6c) )

	ROM_REGION16_BE( 0x80, "normpoint", 0 )
	ROM_LOAD( "normpnt.rom", 0x00, 0x80, CRC(e7f2a005) SHA1(c27b4a364a24daeee6e99fd286753fd6216362b4) )

	ROM_REGION16_BE( 0x80, "normslope", 0 )
	ROM_LOAD( "normslp.rom", 0x00, 0x80, CRC(4f2ae525) SHA1(eab43f8cc52c8551d9cff6fced18ef80eaba6f05) )
ROM_END


ROM_START( kurufev )
	ROM_REGION32_BE( 0x800, "user1", ROMREGION_ERASE00 )
	PIF_BOOTROM

	ROM_REGION32_BE( 0x4000000, "user2", 0 )
	ROM_LOAD16_WORD_SWAP( "ua3088-all01.u3", 0x0000000, 0x1000000, CRC(00db4dbc) SHA1(824fdce01fffdfcbcc9b1fbda4ab389a10b2b418) )
	ROM_LOAD16_WORD_SWAP( "ua3088-alh04.u4", 0x1000000, 0x1000000, CRC(c96bc7c0) SHA1(2b6ca1a769dee74e112c2b287dacd0bf46dda091) )

	ROM_REGION32_BE( 0x800000, "user3", 0 )
	ROM_LOAD16_WORD_SWAP( "nus-zsij-0.u1", 0x000000, 0x800000, CRC(2389576f) SHA1(dc22b2eab4d7a02cb918827a62e6c120b3a84e6c) ) // same as tower & shaft

	ROM_REGION16_BE( 0x80, "normpoint", 0 )
	ROM_LOAD( "normpnt.rom", 0x00, 0x80, CRC(e7f2a005) SHA1(c27b4a364a24daeee6e99fd286753fd6216362b4) )

	ROM_REGION16_BE( 0x80, "normslope", 0 )
	ROM_LOAD( "normslp.rom", 0x00, 0x80, CRC(4f2ae525) SHA1(eab43f8cc52c8551d9cff6fced18ef80eaba6f05) )
ROM_END

ROM_START( doncdoon )
	ROM_REGION32_BE( 0x800, "user1", ROMREGION_ERASE00 )
	PIF_BOOTROM

	ROM_REGION32_BE( 0x4000000, "user2", 0 )
	ROM_LOAD16_WORD_SWAP( "ua3003-all01.u3", 0x0000000, 0x1000000, CRC(f362fa82) SHA1(4f41ee23edc18110be1218ba333d1c58376ab175) )
	ROM_LOAD16_WORD_SWAP( "ua3003-alh01.u4", 0x1000000, 0x1000000, CRC(47c56387) SHA1(c8cc6c0a456b593aef711d0a75b2342ba2f8203f) )

	ROM_REGION32_BE( 0x800000, "user3", 0 )
	ROM_LOAD16_WORD_SWAP( "nus-zsij-0.u1", 0x000000, 0x800000, CRC(547d8122) SHA1(347f0785767265acb0f0c21646e06cbe6f561821) )

	ROM_REGION16_BE( 0x80, "normpoint", 0 )
	ROM_LOAD( "normpnt.rom", 0x00, 0x80, CRC(e7f2a005) SHA1(c27b4a364a24daeee6e99fd286753fd6216362b4) )

	ROM_REGION16_BE( 0x80, "normslope", 0 )
	ROM_LOAD( "normslp.rom", 0x00, 0x80, CRC(4f2ae525) SHA1(eab43f8cc52c8551d9cff6fced18ef80eaba6f05) )
ROM_END

/* Mayjinsen 3
 * PCB marking: "SeTa // 3D Rom PCB-2A // SETA CORPORATION // MADE IN JAPAN"
 * PCB has:
 * U1 CIC-NUS; CIC-NUS-5101 security chip (Sharp SM5K3 MCU)
 * U2 BU9850; BU9850 4k Serial EEPROM (Rohm, proprietary/custom part?)
 * U3 NUS64M: "MXC991789M // MX23L6402-35A // NUS-ZSCJ-0 // 1Q4787A2" Macronix custom latchable self-addressable mask rom
 * Internal checksum is verified to match rom dump
 */
ROM_START( mayjin3 )
	ROM_REGION32_BE( 0x800, "user1", ROMREGION_ERASE00 )
	PIF_BOOTROM

	ROM_REGION32_BE( 0x4000000, "user2", 0 )
	ROM_LOAD16_WORD_SWAP( "nus-zscj-0.u3", 0x000000, 0x800000, CRC(52a37340) SHA1(b5834bfde5b8a7e20415b2593abd76ec95ab27c7) ) // U3 NUS64M

	ROM_REGION16_BE( 0x80, "normpoint", 0 )
	ROM_LOAD( "normpnt.rom", 0x00, 0x80, CRC(e7f2a005) SHA1(c27b4a364a24daeee6e99fd286753fd6216362b4) )

	ROM_REGION16_BE( 0x80, "normslope", 0 )
	ROM_LOAD( "normslp.rom", 0x00, 0x80, CRC(4f2ae525) SHA1(eab43f8cc52c8551d9cff6fced18ef80eaba6f05) )
ROM_END

} // anonymous namespace




// BIOS
GAME( 1998, aleck64,  0,       aleck64, aleck64,  aleck64_state, init_aleck64, ROT0, "Nintendo / Seta", "Aleck64 PIF BIOS", MACHINE_IS_BIOS_ROOT)

// games
GAME( 1998, 11beat,   aleck64, aleck64, 11beat,   aleck64_state, init_aleck64, ROT0, "Hudson",                  "Eleven Beat", MACHINE_IMPERFECT_GRAPHICS ) // crashes at kick off / during attract with DRC
GAME( 1998, mtetrisc, aleck64, a64_e90, mtetrisc, aleck64_state, init_aleck64, ROT0, "Capcom",                  "Magical Tetris Challenge (981009 Japan)", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS ) // incomplete E90 gfxs (playfield), playable otherwise
GAME( 1998, starsldr, aleck64, aleck64, starsldr, aleck64_state, init_aleck64, ROT0, "Hudson / Seta",           "Star Soldier: Vanishing Earth", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1998, vivdolls, aleck64, aleck64, vivdolls, aleck64_state, init_aleck64, ROT0, "Visco",                   "Vivid Dolls", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1999, srmvs,    aleck64, aleck64, srmvs,    aleck64_state, init_aleck64, ROT0, "Seta",                    "Super Real Mahjong VS (Rev A)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1999, srmvsa,   srmvs,   aleck64, srmvs,    aleck64_state, init_aleck64, ROT0, "Seta",                    "Super Real Mahjong VS", MACHINE_IMPERFECT_GRAPHICS )
GAME( 2000, mayjin3,  aleck64, aleck64, aleck64,  aleck64_state, init_aleck64, ROT0, "Seta / Able Corporation", "Mayjinsen 3", MACHINE_IMPERFECT_SOUND|MACHINE_IMPERFECT_GRAPHICS )
GAME( 2003, twrshaft, aleck64, aleck64, twrshaft, aleck64_state, init_aleck64, ROT0, "Aruze",                   "Tower & Shaft", MACHINE_IMPERFECT_GRAPHICS )
GAME( 2003, hipai,    aleck64, aleck64, hipai,    aleck64_state, init_aleck64, ROT0, "Aruze / Seta",            "Hi Pai Paradise", MACHINE_IMPERFECT_GRAPHICS )
GAME( 2003, doncdoon, aleck64, aleck64, doncdoon, aleck64_state, init_aleck64, ROT0, "Aruze",                   "Hanabi de Doon! - Don-chan Puzzle", MACHINE_IMPERFECT_GRAPHICS )
GAME( 2003, kurufev,  aleck64, aleck64, kurufev,  aleck64_state, init_aleck64, ROT0, "Aruze / Takumi",          "Kurukuru Fever", MACHINE_IMPERFECT_GRAPHICS )
GAME( 2004, hipai2,   aleck64, aleck64, hipai2,   aleck64_state, init_aleck64, ROT0, "Aruze / Seta / Paon",     "Hi Pai Paradise 2", MACHINE_IMPERFECT_GRAPHICS )
