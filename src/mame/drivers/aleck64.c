/* 'Aleck64' and similar boards */
/* N64 based hardware */
/*

If you want to boot eleven beat on any n64 emu ?(tested on nemu, 1964
and project64) patch the rom :
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
       |R|TA8139S                                      |
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
                   NUS-ZHAJ.U3 : 64Mbit 28 pin DIP serial MASKROM

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


On bootup, they also mention 'T.L.S' (Temporary Landing System), which seems
to be the hardware system, designed by Arika Co. Ltd.


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
      *          : Unpopulated socket for 8M - 32M 42 pin DIP MASKROM
      O          : Push-button reset switch
      X          : Connectors for special (Aleck64?) digital joysticks
      CPU-NUS A  : Labelled on the PCB as "VR4300"
      BK4D-NUS   : Similar to the save chip used in N64 console carts

      ROMs
      ----
      TET-01M.U5 : 8Mbit 42 pin MASKROM
      NUS-CZAJ.U4: 128Mbit 28 pin DIP serial MASKROM
      AT24C01.U34: 128bytes x 8 bit serial EEPROM

      - RCA audio plugs output stereo sound. Regular mono sound is output
        via the standard JAMMA connector also.

      - ALL components are listed for completeness. However, many are power or
        logic devices that most people need not be concerned about :-)

      - The Seta/N64 Aleck64 hardware is similar also, but instead of the hich capacity
        serial MASKROM being on the main board, it's in a cart that plugs into a slot.

*/

#include "emu.h"
#include "streams.h"
#include "cpu/rsp/rsp.h"
#include "cpu/mips/mips3.h"
#include "sound/dmadac.h"
#include "includes/n64.h"

static READ32_HANDLER( aleck_dips_r )
{
	if (offset == 0)
		return (input_port_read(space->machine, "IN0"));	/* mtetrisc has regular inputs here */
	else if (offset == 1)
		return (input_port_read(space->machine, "IN1"));

	return 0;
}

static ADDRESS_MAP_START( n64_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x007fffff) AM_RAM	AM_BASE(&rdram)				// RDRAM
	AM_RANGE(0x04000000, 0x04000fff) AM_RAM AM_SHARE("dmem")					// RSP DMEM
	AM_RANGE(0x04001000, 0x04001fff) AM_RAM AM_SHARE("imem")					// RSP IMEM
	AM_RANGE(0x04040000, 0x040fffff) AM_DEVREADWRITE("rsp", n64_sp_reg_r, n64_sp_reg_w)	// RSP
	AM_RANGE(0x04100000, 0x041fffff) AM_DEVREADWRITE("rsp", n64_dp_reg_r, n64_dp_reg_w)	// RDP
	AM_RANGE(0x04300000, 0x043fffff) AM_READWRITE(n64_mi_reg_r, n64_mi_reg_w)	// MIPS Interface
	AM_RANGE(0x04400000, 0x044fffff) AM_READWRITE(n64_vi_reg_r, n64_vi_reg_w)	// Video Interface
	AM_RANGE(0x04500000, 0x045fffff) AM_READWRITE(n64_ai_reg_r, n64_ai_reg_w)	// Audio Interface
	AM_RANGE(0x04600000, 0x046fffff) AM_READWRITE(n64_pi_reg_r, n64_pi_reg_w)	// Peripheral Interface
	AM_RANGE(0x04700000, 0x047fffff) AM_READWRITE(n64_ri_reg_r, n64_ri_reg_w)	// RDRAM Interface
	AM_RANGE(0x04800000, 0x048fffff) AM_READWRITE(n64_si_reg_r, n64_si_reg_w)	// Serial Interface
	AM_RANGE(0x10000000, 0x13ffffff) AM_ROM AM_REGION("user2", 0)	// Cartridge
	AM_RANGE(0x1fc00000, 0x1fc007bf) AM_ROM AM_REGION("user1", 0)	// PIF ROM
	AM_RANGE(0x1fc007c0, 0x1fc007ff) AM_READWRITE(n64_pif_ram_r, n64_pif_ram_w)

	AM_RANGE(0xc0800000, 0xc08fffff) AM_READ(aleck_dips_r) AM_WRITENOP
	AM_RANGE(0xd0000000, 0xd0000fff) AM_RAM
	AM_RANGE(0xd0010000, 0xd00109ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( rsp_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x00000fff) AM_RAM AM_SHARE("dmem")
	AM_RANGE(0x00001000, 0x00001fff) AM_RAM AM_SHARE("imem")
	AM_RANGE(0x04000000, 0x04000fff) AM_RAM AM_BASE(&rsp_dmem) AM_SHARE("dmem")
	AM_RANGE(0x04001000, 0x04001fff) AM_RAM AM_BASE(&rsp_imem) AM_SHARE("imem")
ADDRESS_MAP_END

static INPUT_PORTS_START( aleck64 )
	PORT_START("P1")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)			// Button A
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)			// Button B
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)			// Button Z
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_START1 )							// Start
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1)		// Joypad Up
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)	// Joypad Down
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)	// Joypad Left
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)	// Joypad Right
	PORT_BIT( 0x00c0, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(1)			// Pan Left
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(1)			// Pan Right
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER(1)			// C Button Up
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_PLAYER(1)			// C Button Down
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_PLAYER(1)			// C Button Left
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_BUTTON9 ) PORT_PLAYER(1)			// C Button Right

	PORT_START("P1_ANALOG_X")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_PLAYER(1)

	PORT_START("P1_ANALOG_Y")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0xff,0x00) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_PLAYER(1)

	PORT_START("P2")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)			// Button A
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)			// Button B
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)			// Button Z
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_START2 )							// Start
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2)		// Joypad Up
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)	// Joypad Down
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)	// Joypad Left
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)	// Joypad Right
	PORT_BIT( 0x00c0, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(2)			// Pan Left
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(2)			// Pan Right
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER(2)			// C Button Up
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_PLAYER(2)			// C Button Down
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_PLAYER(2)			// C Button Left
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_BUTTON9 ) PORT_PLAYER(2)			// C Button Right

	PORT_START("P2_ANALOG_X")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_PLAYER(2)

	PORT_START("P2_ANALOG_Y")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0xff,0x00) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_PLAYER(2)

	PORT_START("IN0")
	PORT_DIPNAME( 0x80000000, 0x80000000, "DIPSW1 #8" )
	PORT_DIPSETTING( 0x80000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x40000000, 0x40000000, "DIPSW1 #7" )
	PORT_DIPSETTING( 0x40000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x20000000, 0x20000000, "DIPSW1 #6" )
	PORT_DIPSETTING( 0x20000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x10000000, 0x10000000, "DIPSW1 #5" )
	PORT_DIPSETTING( 0x10000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x08000000, 0x08000000, "DIPSW1 #4" )
	PORT_DIPSETTING( 0x08000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x04000000, 0x04000000, "DIPSW1 #3" )
	PORT_DIPSETTING( 0x04000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x02000000, 0x02000000, "DIPSW1 #2" )
	PORT_DIPSETTING( 0x02000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x01000000, 0x01000000, "DIPSW1 #1" )
	PORT_DIPSETTING( 0x01000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00800000, 0x00800000, "Test Mode" )
	PORT_DIPSETTING( 0x00800000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00400000, 0x00400000, "DIPSW2 #7" )
	PORT_DIPSETTING( 0x00400000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00200000, 0x00200000, "DIPSW2 #6" )
	PORT_DIPSETTING( 0x00200000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00100000, 0x00100000, "DIPSW2 #5" )
	PORT_DIPSETTING( 0x00100000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00080000, 0x00080000, "DIPSW2 #4" )
	PORT_DIPSETTING( 0x00080000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00040000, 0x00040000, "DIPSW2 #3" )
	PORT_DIPSETTING( 0x00040000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00020000, 0x00020000, "DIPSW2 #2" )
	PORT_DIPSETTING( 0x00020000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00010000, 0x00010000, "DIPSW2 #1" )
	PORT_DIPSETTING( 0x00010000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_BIT(0x0000ffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0xff00ffff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE_NO_TOGGLE( 0x00200000, IP_ACTIVE_LOW )
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Service Button") PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_COIN1 )
INPUT_PORTS_END

static INPUT_PORTS_START( 11beat )
	PORT_START("P1_ANALOG_X")
	PORT_START("P1_ANALOG_Y")
	PORT_START("P2_ANALOG_X")
	PORT_START("P2_ANALOG_Y")

	PORT_START("P1")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)			// Button A
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)			// Button B
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNUSED )							// Button Z
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_START1 )							// Start
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1)		// Joypad Up
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)	// Joypad Down
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)	// Joypad Left
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)	// Joypad Right
	PORT_BIT( 0x00c0, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNUSED )							// Pan Left
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)			// Pan Right
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNUSED )							// C Button Up
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNUSED )							// C Button Down
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNUSED )							// C Button Left
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(1)			// C Button Right

	PORT_START("P2")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)			// Button A
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)			// Button B
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNUSED )							// Button Z
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_START2 )							// Start
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2)		// Joypad Up
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)	// Joypad Down
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)	// Joypad Left
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)	// Joypad Right
	PORT_BIT( 0x00c0, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNUSED )							// Pan Left
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)			// Pan Right (Button C here)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNUSED )							// C Button Up
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNUSED )							// C Button Down
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNUSED )							// C Button Left
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(2)			// C Button Right (Button D here)

	PORT_START("IN0")
	PORT_DIPNAME( 0x80000000, 0x80000000, "DIPSW1 #8" )
	PORT_DIPSETTING( 0x80000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x40000000, 0x40000000, "DIPSW1 #7" )
	PORT_DIPSETTING( 0x40000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x20000000, 0x20000000, "DIPSW1 #6" )
	PORT_DIPSETTING( 0x20000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x10000000, 0x10000000, "DIPSW1 #5" )
	PORT_DIPSETTING( 0x10000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x08000000, 0x08000000, "DIPSW1 #4" )
	PORT_DIPSETTING( 0x08000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x04000000, 0x04000000, "DIPSW1 #3" )
	PORT_DIPSETTING( 0x04000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x02000000, 0x02000000, "DIPSW1 #2" )
	PORT_DIPSETTING( 0x02000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x01000000, 0x01000000, "DIPSW1 #1" )
	PORT_DIPSETTING( 0x01000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00800000, 0x00800000, "Test Mode" )
	PORT_DIPSETTING( 0x00800000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00400000, 0x00400000, "DIPSW2 #7" )
	PORT_DIPSETTING( 0x00400000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00200000, 0x00200000, "DIPSW2 #6" )
	PORT_DIPSETTING( 0x00200000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00100000, 0x00100000, "DIPSW2 #5" )
	PORT_DIPSETTING( 0x00100000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00080000, 0x00080000, "DIPSW2 #4" )
	PORT_DIPSETTING( 0x00080000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00040000, 0x00040000, "DIPSW2 #3" )
	PORT_DIPSETTING( 0x00040000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00020000, 0x00020000, "DIPSW2 #2" )
	PORT_DIPSETTING( 0x00020000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00010000, 0x00010000, "DIPSW2 #1" )
	PORT_DIPSETTING( 0x00010000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_BIT(0x0000ffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0xff00ffff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE_NO_TOGGLE( 0x00200000, IP_ACTIVE_LOW )
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Service Button") PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_COIN1 )
INPUT_PORTS_END

static INPUT_PORTS_START( mtetrisc )
	// The basic N64 controls are unused in this game
	PORT_START("P1")
	PORT_START("P1_ANALOG_X")
	PORT_START("P1_ANALOG_Y")
	PORT_START("P2")
	PORT_START("P2_ANALOG_X")
	PORT_START("P2_ANALOG_Y")

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
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Service Button") PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
INPUT_PORTS_END

static INPUT_PORTS_START( starsldr )
	PORT_START("P1")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)			// Button A
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)			// Button B
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_START1 )							// Start
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1)		// Joypad Up
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)	// Joypad Down
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)	// Joypad Left
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)	// Joypad Right
	PORT_BIT( 0x00c0, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)			// Button C
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(1)			// Button D

	PORT_START("P1_ANALOG_X")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_PLAYER(1)

	PORT_START("P1_ANALOG_Y")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0xff,0x00) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_PLAYER(1)

	PORT_START("P2")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)			// Button A
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)			// Button B
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_START1 )							// Start
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1)		// Joypad Up
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)	// Joypad Down
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)	// Joypad Left
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)	// Joypad Right
	PORT_BIT( 0x00c0, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)			// Button C
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(1)			// Button D

	PORT_START("P2_ANALOG_X")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_PLAYER(1)

	PORT_START("P2_ANALOG_Y")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0xff,0x00) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_PLAYER(1)


	PORT_START("IN0")
	PORT_DIPNAME( 0x80000000, 0x00000000, DEF_STR(Joystick) )	// DIPSW1 #8
	PORT_DIPSETTING( 0x00000000, DEF_STR(Joystick) )
	PORT_DIPSETTING( 0x80000000, "3D" )
	PORT_DIPNAME( 0x60000000, 0x60000000, "Auto Level" )		// DIPSW1 #6, #7
	PORT_DIPSETTING( 0x60000000, DEF_STR(Normal) )
	PORT_DIPSETTING( 0x40000000, "Slow" )
	PORT_DIPSETTING( 0x20000000, "Fast1" )
	PORT_DIPSETTING( 0x00000000, "Fast2" )
	PORT_DIPNAME( 0x18000000, 0x18000000, "Player" )			// DIPSW1 #4, #5
	PORT_DIPSETTING( 0x18000000, "3" )
	PORT_DIPSETTING( 0x10000000, "4" )
	PORT_DIPSETTING( 0x08000000, "2" )
	PORT_DIPSETTING( 0x00000000, "1" )
	PORT_DIPNAME( 0x07000000, 0x07000000, DEF_STR(Coinage) )	// DIPSW1 #1, #2, #3
	PORT_DIPSETTING( 0x00000000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING( 0x01000000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING( 0x02000000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING( 0x03000000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING( 0x07000000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING( 0x06000000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING( 0x05000000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING( 0x04000000, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x00800000, 0x00800000, "DIPSW2 #8" )
	PORT_DIPSETTING( 0x00800000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00400000, 0x00400000, DEF_STR(Language) )	// DIPSW2 #7
	PORT_DIPSETTING( 0x00400000, DEF_STR(English) )
	PORT_DIPSETTING( 0x00000000, DEF_STR(Japanese) )
	PORT_DIPNAME( 0x00200000, 0x00000000, "Demosound" )			// DIPSW2 #6
	PORT_DIPSETTING( 0x00200000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00100000, 0x00100000, "Rapid" )				// DIPSW2 #5
	PORT_DIPSETTING( 0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00100000, DEF_STR( On ) )
	PORT_DIPNAME( 0x000c0000, 0x000c0000, "Extend" )			// DIPSW2 #3, #4
	PORT_DIPSETTING( 0x000c0000, "Every 30000000" )
	PORT_DIPSETTING( 0x00080000, "Every 50000000" )
	PORT_DIPSETTING( 0x00040000, "Every 70000000" )
	PORT_DIPSETTING( 0x00000000, "Non" )
	PORT_DIPNAME( 0x00030000, 0x00030000, DEF_STR(Difficulty) )	// DIPSW2 #1, #2
	PORT_DIPSETTING( 0x00030000, DEF_STR(Normal) )
	PORT_DIPSETTING( 0x00020000, DEF_STR(Easy) )
	PORT_DIPSETTING( 0x00010000, "Hard1" )
	PORT_DIPSETTING( 0x00000000, "Hard2" )
	PORT_BIT(0x0000ffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0xff00ffff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE_NO_TOGGLE( 0x00200000, IP_ACTIVE_LOW )
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Service Button") PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_COIN1 )
INPUT_PORTS_END

/* ?? */
static const mips3_config config =
{
	16384,				/* code cache size */
	8192,				/* data cache size */
	62500000			/* system clock */
};

static INTERRUPT_GEN( n64_vblank )
{
	signal_rcp_interrupt(device->machine, VI_INTERRUPT);
}

static MACHINE_DRIVER_START( aleck64 )

	MDRV_DRIVER_DATA(_n64_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", VR4300BE, 93750000)
	MDRV_CPU_CONFIG(config)
	MDRV_CPU_PROGRAM_MAP(n64_map)
	MDRV_CPU_VBLANK_INT("screen", n64_vblank)

	MDRV_CPU_ADD("rsp", RSP, 62500000)
	MDRV_CPU_CONFIG(n64_rsp_config)
	MDRV_CPU_PROGRAM_MAP(rsp_map)

	MDRV_MACHINE_START( n64 )
	MDRV_MACHINE_RESET( n64 )

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(640, 525)
	MDRV_SCREEN_VISIBLE_AREA(0, 639, 0, 239)
	MDRV_PALETTE_LENGTH(0x1000)

	MDRV_VIDEO_START(n64)
	MDRV_VIDEO_UPDATE(n64)

	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("dac1", DMADAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MDRV_SOUND_ADD("dac2", DMADAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
MACHINE_DRIVER_END

static DRIVER_INIT( aleck64 )
{
	UINT8 *rom = memory_region(machine, "user2");

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
	ROM_LOAD16_WORD_SWAP( "nus-zhaj.u3", 0x000000, 0x0800000,  CRC(95258ba2) SHA1(0299b8fb9a8b1b24428d0f340f6bf1cfaf99c672) )

	ROM_REGION16_BE( 0x80, "normpoint", 0 )
    ROM_LOAD( "normpnt.rom", 0x00, 0x80, CRC(e7f2a005) SHA1(c27b4a364a24daeee6e99fd286753fd6216362b4) )

	ROM_REGION16_BE( 0x80, "normslope", 0 )
    ROM_LOAD( "normslp.rom", 0x00, 0x80, CRC(4f2ae525) SHA1(eab43f8cc52c8551d9cff6fced18ef80eaba6f05) )
ROM_END

ROM_START( mtetrisc )
	ROM_REGION32_BE( 0x800, "user1", ROMREGION_ERASE00 )
	PIF_BOOTROM

	ROM_REGION32_BE( 0x4000000, "user2", 0 )
	ROM_LOAD16_WORD_SWAP( "nus-zcaj.u4", 0x000000, 0x1000000,  CRC(ec4563fc) SHA1(4d5a30873a5850cf4cd1c0bdbe24e1934f163cd0) )

	ROM_REGION32_BE( 0x100000, "user3", 0 )
	ROM_LOAD ( "tet-01m.u5", 0x000000, 0x100000,  CRC(f78f859b) SHA1(b07c85e0453869fe43792f42081f64a5327e58e6) )

	ROM_REGION32_BE( 0x80, "user4", 0 )
	ROM_LOAD ( "at24c01.u34", 0x000000, 0x80,  CRC(ba7e503f) SHA1(454aa4fdde7d8694d1affaf25cd750fa678686bb) )

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
	ROM_LOAD16_WORD_SWAP( "nus-zhbj-0.u3", 0x000000, 0xc00000,  CRC(a4edac93) SHA1(3794606c008fb69f5d16dcccece94d03da23bf8a) )

	ROM_REGION16_BE( 0x80, "normpoint", 0 )
    ROM_LOAD( "normpnt.rom", 0x00, 0x80, CRC(e7f2a005) SHA1(c27b4a364a24daeee6e99fd286753fd6216362b4) )

	ROM_REGION16_BE( 0x80, "normslope", 0 )
    ROM_LOAD( "normslp.rom", 0x00, 0x80, CRC(4f2ae525) SHA1(eab43f8cc52c8551d9cff6fced18ef80eaba6f05) )

ROM_END

ROM_START( srmvs )
	ROM_REGION32_BE( 0x800, "user1", ROMREGION_ERASE00 )
	PIF_BOOTROM

	ROM_REGION32_BE( 0x4000000, "user2", 0 )
	ROM_LOAD16_WORD_SWAP( "nus-zsej-0.u2", 0x000000, 0x2000000,  CRC(44f40102) SHA1(a78de955f2fcd99dda14e782984368b320eb5415) )

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
	ROM_LOAD16_WORD_SWAP( "nus-zsaj-0.u3", 0x000000, 0x800000,  CRC(f3220e29) SHA1(06d8b808cc19378b046803f4dc75c7d791b7767f) )

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
	ROM_LOAD16_WORD_SWAP( "ua3012--all02.u3", 0x000000, 0x1000000,  CRC(904a91a7) SHA1(7dfa3447d2c489c0448c4004dc12d3037c05a0f3) )

	ROM_REGION32_BE( 0x800000, "user3", 0 )
	ROM_LOAD16_WORD_SWAP( "nus-zsij-0.u1", 0x000000, 0x800000,  CRC(2389576f) SHA1(dc22b2eab4d7a02cb918827a62e6c120b3a84e6c) )

	ROM_REGION16_BE( 0x80, "normpoint", 0 )
    ROM_LOAD( "normpnt.rom", 0x00, 0x80, CRC(e7f2a005) SHA1(c27b4a364a24daeee6e99fd286753fd6216362b4) )

	ROM_REGION16_BE( 0x80, "normslope", 0 )
    ROM_LOAD( "normslp.rom", 0x00, 0x80, CRC(4f2ae525) SHA1(eab43f8cc52c8551d9cff6fced18ef80eaba6f05) )
ROM_END


ROM_START( hipai )
	ROM_REGION32_BE( 0x800, "user1", ROMREGION_ERASE00 )
	PIF_BOOTROM

	ROM_REGION32_BE( 0x4000000, "user2", 0 )
	ROM_LOAD16_WORD_SWAP( "ua2011-all02.u3", 0x0000000, 0x1000000,  CRC(eb4b96d0) SHA1(e909ea5b71b81087da07821c4f57244576363678) )
	ROM_LOAD16_WORD_SWAP( "ua2011-alh02.u4", 0x1000000, 0x1000000,  CRC(b8e35ddf) SHA1(7c3e59f6520dc3f0aa592e682fa82e30ffd1f4d0) )

	ROM_REGION32_BE( 0x800000, "user3", 0 )
	ROM_LOAD16_WORD_SWAP( "nus-nsij-0.u1", 0x000000, 0x800000,  CRC(94cf9f8d) SHA1(cd624d1f5de2be3bec3ece06556a2e39bef66d77) )

	ROM_REGION16_BE( 0x80, "normpoint", 0 )
    ROM_LOAD( "normpnt.rom", 0x00, 0x80, CRC(e7f2a005) SHA1(c27b4a364a24daeee6e99fd286753fd6216362b4) )

	ROM_REGION16_BE( 0x80, "normslope", 0 )
    ROM_LOAD( "normslp.rom", 0x00, 0x80, CRC(4f2ae525) SHA1(eab43f8cc52c8551d9cff6fced18ef80eaba6f05) )
ROM_END


ROM_START( doncdoon )
	ROM_REGION32_BE( 0x800, "user1", ROMREGION_ERASE00 )
	PIF_BOOTROM

	ROM_REGION32_BE( 0x4000000, "user2", 0 )
	ROM_LOAD16_WORD_SWAP( "ua3088-all01.u3", 0x0000000, 0x1000000,  CRC(00db4dbc) SHA1(824fdce01fffdfcbcc9b1fbda4ab389a10b2b418) )
	ROM_LOAD16_WORD_SWAP( "ua3088-alh04.u4", 0x1000000, 0x1000000,  CRC(c96bc7c0) SHA1(2b6ca1a769dee74e112c2b287dacd0bf46dda091) )

	ROM_REGION32_BE( 0x800000, "user3", 0 )
	ROM_LOAD16_WORD_SWAP( "nus-zsij-0.u1", 0x000000, 0x800000,   CRC(2389576f) SHA1(dc22b2eab4d7a02cb918827a62e6c120b3a84e6c) ) // same as tower & shaft

	ROM_REGION16_BE( 0x80, "normpoint", 0 )
    ROM_LOAD( "normpnt.rom", 0x00, 0x80, CRC(e7f2a005) SHA1(c27b4a364a24daeee6e99fd286753fd6216362b4) )

	ROM_REGION16_BE( 0x80, "normslope", 0 )
    ROM_LOAD( "normslp.rom", 0x00, 0x80, CRC(4f2ae525) SHA1(eab43f8cc52c8551d9cff6fced18ef80eaba6f05) )
ROM_END

ROM_START( kurufev )
	ROM_REGION32_BE( 0x800, "user1", ROMREGION_ERASE00 )
	PIF_BOOTROM

	ROM_REGION32_BE( 0x4000000, "user2", 0 )
	ROM_LOAD16_WORD_SWAP( "ua3003-all01.u3", 0x0000000, 0x1000000,  CRC(f362fa82) SHA1(4f41ee23edc18110be1218ba333d1c58376ab175) )
	ROM_LOAD16_WORD_SWAP( "ua3003-alh01.u4", 0x1000000, 0x1000000,  CRC(47c56387) SHA1(c8cc6c0a456b593aef711d0a75b2342ba2f8203f) )

	ROM_REGION32_BE( 0x800000, "user3", 0 )
	ROM_LOAD16_WORD_SWAP( "nus-zsij-0.u1", 0x000000, 0x800000, CRC(547d8122) SHA1(347f0785767265acb0f0c21646e06cbe6f561821) )

	ROM_REGION16_BE( 0x80, "normpoint", 0 )
    ROM_LOAD( "normpnt.rom", 0x00, 0x80, CRC(e7f2a005) SHA1(c27b4a364a24daeee6e99fd286753fd6216362b4) )

	ROM_REGION16_BE( 0x80, "normslope", 0 )
    ROM_LOAD( "normslp.rom", 0x00, 0x80, CRC(4f2ae525) SHA1(eab43f8cc52c8551d9cff6fced18ef80eaba6f05) )
ROM_END


ROM_START( mayjin3 )
	ROM_REGION32_BE( 0x800, "user1", ROMREGION_ERASE00 )
	PIF_BOOTROM

	ROM_REGION32_BE( 0x4000000, "user2", 0 )
	ROM_LOAD16_WORD_SWAP( "nus-zscj.u3", 0x000000, 0x800000,  CRC(8b36eb91) SHA1(179745625c16c6813d5f8d29bfd7628783d55806) )

	ROM_REGION16_BE( 0x80, "normpoint", 0 )
    ROM_LOAD( "normpnt.rom", 0x00, 0x80, CRC(e7f2a005) SHA1(c27b4a364a24daeee6e99fd286753fd6216362b4) )

	ROM_REGION16_BE( 0x80, "normslope", 0 )
    ROM_LOAD( "normslp.rom", 0x00, 0x80, CRC(4f2ae525) SHA1(eab43f8cc52c8551d9cff6fced18ef80eaba6f05) )

ROM_END




// BIOS
GAME( 1998, aleck64,        0,  aleck64, aleck64,  aleck64, ROT0, "Nintendo/Seta", "Aleck64 PIF BIOS", GAME_IS_BIOS_ROOT)

// games
GAME( 1998, 11beat,   aleck64,  aleck64, 11beat,  aleck64, ROT0, "Hudson", "Eleven Beat", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 1998, mtetrisc, aleck64,  aleck64, mtetrisc, aleck64, ROT0, "Capcom", "Magical Tetris Challenge (981009 Japan)", GAME_NOT_WORKING|GAME_IMPERFECT_GRAPHICS )
GAME( 1998, starsldr, aleck64,  aleck64, starsldr, aleck64, ROT0, "Hudson/Seta", "Star Soldier: Vanishing Earth", GAME_IMPERFECT_GRAPHICS )
GAME( 1998, vivdolls, aleck64,  aleck64, aleck64,  aleck64, ROT0, "Visco", "Vivid Dolls", GAME_IMPERFECT_GRAPHICS )
GAME( 1999, srmvs,    aleck64,  aleck64, aleck64,  aleck64, ROT0, "Seta", "Super Real Mahjong VS", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 2003, twrshaft, aleck64,  aleck64, aleck64,  aleck64, ROT0, "Aruze", "Tower & Shaft", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 2003, hipai,    aleck64,  aleck64, aleck64,  aleck64, ROT0, "Aruze/Seta", "Hi Pai Paradise", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 2003, doncdoon, aleck64,  aleck64, aleck64,  aleck64, ROT0, "Aruze/Takumi", "Donchan no Hanabi de Doon", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 2003, kurufev,  aleck64,  aleck64, aleck64,  aleck64, ROT0, "Aruze/Takumi", "Kurukuru Fever", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 2000, mayjin3,  aleck64,  aleck64, aleck64,  aleck64, ROT0, "Seta/Able Corporation", "Mayjinsen 3", GAME_NOT_WORKING|GAME_NO_SOUND )

