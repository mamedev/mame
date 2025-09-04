// license:BSD-3-Clause
// copyright-holders:Alex Pasadyn, Zsolt Vasvari, Ernesto Corvi, Aaron Giles
// thanks-to:Kurt Mahan
/*************************************************************************

    Midway Wolf-unit system

    driver by Ernesto Corvi, Aaron Giles
    based on Y/Z-unit driver by Alex Pasadyn, Zsolt Vasvari, Kurt Mahan

    Games supported:
        * Mortal Kombat 3
        * Ultimate Mortal Kombat 3
        * NBA Hangtime
        * NBA Maximum Hangtime
        * 2 On 2 Open Ice Challenge
        * WWF Wrestlemania
        * Rampage World Tour

    TODO:
        * WWF has an unimplemented and not Y2K compatible real-time clock

    BTANB:
        * umk3 Scorpion's "Get Over Here" sample gets cut off, ROM dumps confirmed good

    NOTE: There is known to exist a Wrestlemania PCB with the following labels:
          Wrestlemania 1.0 U63 #B549    &    Wrestlemania 1.0 U54 #40C7
          These checksums match the version 1.1 dump.

***************************************************************************

MK3/ Ultimate Mortal Kombat 3 / Williams-Midway Wolf hardware
Midway, 1995

PCB Layout
----------

5770-14086-05
|---------------------------------------------------------------------------------------------------|
| 42C8128 42C8128 42C8128 42C8128                       *U102       *U103       *U104       *U105   |
| 42C8128 42C8128 42C8128 42C8128                                                                   |
|                                                       *U106       *U107       *U108       *U109   |
|                                                                                                   |
|                |---------|                             U110        U111        U112        U113   |
| |-------|      |5410-    |                                                                        |
| |5410-  |      |12862-   |                            *U114       *U115       *U116       *U117   |
| |14120-0|      |00       |          |------|                                                      |
| |-------|      |WILLIAMS |          |ALTERA|           U118        U119        U120        U121   |
| 84256  84256   |---------|          |FPGA3 |                                                      |
|                                     |------|           U122        U123        U124        U125   |
|                                        RESET_SW                                                   |
|                                                        U126        U127        U128        U129   |
| 8MHz      |------|                                                                                |
|           |ALTERA|              BATTERY                U130        U131        U132        U133   |
||------|   |FPGA2 |                                                                                |
||ALTERA|   |      |      42S4260                                                                   |
||FPGA1 |   |------|                            10MHz                                               |
||      |50MHz                               GAL       |-----|               ROM.U5         *U9     |
||------|                  ROM.U54                     |ADSP-|                                      |
|              LH5268                                  |2105 |               ROM.U4         *U8     |
|   |------|                                           |-----|                                      |
|   |TMS   |               ROM.U63      PIC16C57.U64                         ROM.U3         *U7     |
|   |34010 |                                             65764                                      |
|   |      | LED LED                                     65764               ROM.U2         *U6     |
|   |------|                                 P6          65764                                      |
|                                                                                                   |
|                                                                                                   |
|    TL084                                                    MAX693               DSW2    DSW1     |
|                AD1851                                       ULN2064                               |
|TDA7240                                                         LED                                |
|            |----|         JAMMA           |----|             P9    P10     P11      P12           |
|------------|    |-------------------------|    |--------------------------------------------------|
Notes:
      *            - Empty DIP32 socket (for UMK3)
      P9/10/11/12  - connector for extra controls
      P6           - connector possibly tied to the PIC via some logic?
      GAL          - labelled 'MKIII-U57 A-20093 (C)1995 MIDWAY MFG. CO.'
      PIC.U64      - labelled 'MK3 ULTIMATE U64'
      ALTERA FPGA1 - labelled 'MKIII-U45 A-20096 (C)1995 MIDWAY MFG. CO.'
      ALTERA FPGA2 - labelled 'MKIII-U47 A-20249 (C)1995 MIDWAY MFG. CO.'
      ALTERA FPGA3 - labelled 'MKIII-U35 A-20095 (C)1995 MIDWAY MFG. CO.'
      TMS34010     - input clock (pin5) 50.000MHz
      ADSP-2105    - clock input 10.000MHz
      PIC16C57     - clock input 4.000MHz [8/2]
      TDA7240      - Sound Amplifier IC
      RAM          - 42C8128 (x8), 42S4260 (x1), 84256 (x2), LH5268 (x1), 65764 (x3)
      VSync        - 54.5Hz (measured on pin 31 of TMS34010)
      HSync        - 15.81kHz (measured on pin 30 of TMS34010)

      All ROMs from U102 to U133 are labelled 'MORTAL KOMBAT III ULTIMATE (C) MIDWAY MFG U' + U# as 3 digits + 'VIDEO IMAGE'
      U3 labelled 'MORTAL KOMBAT 3 U3 MUSIC/SPCH'
      U2 labelled 'MORTAL KOMBAT 3 U2 ULTIMATE'

**************************************************************************/


#include "emu.h"
#include "midwunit.h"

#include "cpu/tms34010/tms34010.h"
#include "machine/nvram.h"

#include "screen.h"
#include "speaker.h"


/*************************************
 *
 *  Memory maps
 *
 *************************************/

void midwunit_state::main_map(address_map &map)
{
	map(0x00000000, 0x003fffff).rw(m_video, FUNC(midwunit_video_device::midtunit_vram_r), FUNC(midwunit_video_device::midtunit_vram_w));
	map(0x01000000, 0x013fffff).ram().share(m_mainram);
	map(0x01400000, 0x0145ffff).rw(FUNC(midwunit_state::cmos_r), FUNC(midwunit_state::cmos_w)).share(m_nvram);
	map(0x01480000, 0x014fffff).w(FUNC(midwunit_state::cmos_enable_w));
	map(0x01600000, 0x0160001f).rw(FUNC(midwunit_state::security_r), FUNC(midwunit_state::security_w));
	map(0x01680000, 0x0168001f).rw(FUNC(midwunit_state::sound_r), FUNC(midwunit_state::sound_w));
	map(0x01800000, 0x0187ffff).rw(FUNC(midwunit_state::io_r), FUNC(midwunit_state::io_w));
	map(0x01880000, 0x018fffff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x01a00000, 0x01a000ff).mirror(0x00080000).rw(m_video, FUNC(midwunit_video_device::dma_r), FUNC(midwunit_video_device::dma_w));
	map(0x01b00000, 0x01b0001f).rw(m_video, FUNC(midwunit_video_device::midwunit_control_r), FUNC(midwunit_video_device::midwunit_control_w));
	map(0x02000000, 0x06ffffff).r(m_video, FUNC(midwunit_video_device::midwunit_gfxrom_r));
	map(0xff800000, 0xffffffff).rom().region("maincpu", 0);
}


/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( mk3 )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 High Punch") PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Block") PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P1 High Kick") PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P2 High Punch") PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P2 Block") PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P2 High Kick") PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P1 Low Punch") PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("P1 Low Kick") PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("P1 Run") PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P2 Low Punch") PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("P2 Low Kick") PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("P2 Run") PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0001, 0x0001, "Test Switch" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x0002, 0x0000, "Counters" )
	PORT_DIPSETTING(      0x0002, "One" )
	PORT_DIPSETTING(      0x0000, "Two" )
	PORT_DIPNAME( 0x007c, 0x007c, DEF_STR( Coinage ))
	PORT_DIPSETTING(      0x007c, "USA-1" )
	PORT_DIPSETTING(      0x003c, "USA-2" )
	PORT_DIPSETTING(      0x005c, "USA-3" )
	PORT_DIPSETTING(      0x001c, "USA-4" )
	PORT_DIPSETTING(      0x006c, "USA-ECA" )
	PORT_DIPSETTING(      0x000c, "USA-Free Play" )
	PORT_DIPSETTING(      0x0074, "German-1" )
	PORT_DIPSETTING(      0x0034, "German-2" )
	PORT_DIPSETTING(      0x0054, "German-3" )
	PORT_DIPSETTING(      0x0014, "German-4" )
	PORT_DIPSETTING(      0x0064, "German-5" )
	PORT_DIPSETTING(      0x0024, "German-ECA" )
	PORT_DIPSETTING(      0x0004, "German-Free Play" )
	PORT_DIPSETTING(      0x0078, "French-1" )
	PORT_DIPSETTING(      0x0038, "French-2" )
	PORT_DIPSETTING(      0x0058, "French-3" )
	PORT_DIPSETTING(      0x0018, "French-4" )
	PORT_DIPSETTING(      0x0068, "French-ECA" )
	PORT_DIPSETTING(      0x0008, "French-Free Play" )
	PORT_DIPNAME( 0x0080, 0x0000, "Coinage Source" )
	PORT_DIPSETTING(      0x0080, "Dipswitch" )
	PORT_DIPSETTING(      0x0000, "CMOS" )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0200, 0x0000, "Powerup Test" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0200, DEF_STR( On ))
	PORT_DIPNAME( 0x0400, 0x0400, "Bill Validator" )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x1000, 0x1000, "Attract Sound" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ))
	PORT_DIPSETTING(      0x1000, DEF_STR( On ))
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x4000, 0x4000, "Blood" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ))
	PORT_DIPSETTING(      0x4000, DEF_STR( On ))
	PORT_DIPNAME( 0x8000, 0x8000, "Violence" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ))
	PORT_DIPSETTING(      0x8000, DEF_STR( On ))

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT ) // Slam Switch
	PORT_SERVICE_NO_TOGGLE( 0x0010, IP_ACTIVE_LOW )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_VOLUME_DOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_VOLUME_UP )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_INTERLOCK )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BILL1 )
INPUT_PORTS_END


static INPUT_PORTS_START( openice )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Shoot / Block") PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P1 Pass / Steal") PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Turbo") PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P2 Shoot / Block") PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P2 Pass / Steal") PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P2 Turbo") PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P3 Shoot / Block") PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P3 Pass / Steal") PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P3 Turbo") PORT_PLAYER(3)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P4 Shoot / Block") PORT_PLAYER(4)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P4 Pass / Steal")PORT_PLAYER(4)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P4 Turbo")PORT_PLAYER(4)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0001, 0x0000, "Coinage Source" )
	PORT_DIPSETTING(      0x0001, "Dipswitch" )
	PORT_DIPSETTING(      0x0000, "CMOS" )
	PORT_DIPNAME( 0x003e, 0x003e, DEF_STR( Coinage ))
	PORT_DIPSETTING(      0x003e, "USA-1" )
	PORT_DIPSETTING(      0x003c, "USA-2" )
	PORT_DIPSETTING(      0x003a, "USA-3" )
	PORT_DIPSETTING(      0x0038, "USA-4" )
	PORT_DIPSETTING(      0x0034, "USA-9" )
	PORT_DIPSETTING(      0x0032, "USA-10" )
	PORT_DIPSETTING(      0x0036, "USA-ECA" )
	PORT_DIPSETTING(      0x0030, "USA-Free Play" )
	PORT_DIPSETTING(      0x002e, "German-1" )
	PORT_DIPSETTING(      0x002c, "German-2" )
	PORT_DIPSETTING(      0x002a, "German-3" )
	PORT_DIPSETTING(      0x0028, "German-4" )
	PORT_DIPSETTING(      0x0024, "German-5" )
	PORT_DIPSETTING(      0x0026, "German-ECA" )
	PORT_DIPSETTING(      0x0020, "German-Free Play" )
	PORT_DIPSETTING(      0x001e, "French-1" )
	PORT_DIPSETTING(      0x001c, "French-2" )
	PORT_DIPSETTING(      0x001a, "French-3" )
	PORT_DIPSETTING(      0x0018, "French-4" )
	PORT_DIPSETTING(      0x0014, "French-11" )
	PORT_DIPSETTING(      0x0012, "French-12" )
	PORT_DIPSETTING(      0x0016, "French-ECA" )
	PORT_DIPSETTING(      0x0010, "French-Free Play" )
	PORT_DIPNAME( 0x0040, 0x0000, "Counters" )
	PORT_DIPSETTING(      0x0040, "One" )
	PORT_DIPSETTING(      0x0000, "Two" )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0100, 0x0100, "Bill Validator" )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x0200, 0x0200, "Attract Sound" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0200, DEF_STR( On ))
	PORT_DIPNAME( 0x0400, 0x0000, "Powerup Test" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0400, DEF_STR( On ))
	PORT_DIPNAME( 0x0800, 0x0800, "Head Size" )
	PORT_DIPSETTING(      0x0800, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, "Large" )
	PORT_DIPNAME( 0x1000, 0x0000, DEF_STR( Cabinet ))
	PORT_DIPSETTING(      0x0000, "2-player" )
	PORT_DIPSETTING(      0x1000, "4-player" )
	PORT_BIT( 0x6000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x8000, 0x8000, "Test Switch" )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT ) // Slam Switch
	PORT_SERVICE_NO_TOGGLE( 0x0010, IP_ACTIVE_LOW )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_VOLUME_DOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_VOLUME_UP )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_INTERLOCK )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BILL1 )
INPUT_PORTS_END


static INPUT_PORTS_START( nbahangt )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Shoot / Block") PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P1 Pass / Steal") PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Turbo") PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P2 Shoot / Block") PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P2 Pass / Steal") PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P2 Turbo") PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P3 Shoot / Block") PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P3 Pass / Steal") PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P3 Turbo") PORT_PLAYER(3)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P4 Shoot / Block") PORT_PLAYER(4)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P4 Pass / Steal") PORT_PLAYER(4)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P4 Turbo") PORT_PLAYER(4)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0001, 0x0001, "Test Switch" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x0002, 0x0000, "Powerup Test" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0002, DEF_STR( On ))
	PORT_BIT( 0x003c, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0040, 0x0040, "Bill Validator" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Cabinet ))
	PORT_DIPSETTING(      0x0000, "2-player" )
	PORT_DIPSETTING(      0x0080, "4-player" )
	PORT_DIPNAME( 0x0300, 0x0300, "Counters" )
	PORT_DIPSETTING(      0x0300, "One, 1/1" )
	PORT_DIPSETTING(      0x0200, "One, Totalizing" )
	PORT_DIPSETTING(      0x0100, "Two, 1/1" )
	PORT_DIPNAME( 0x0c00, 0x0c00, "Country" )
	PORT_DIPSETTING(      0x0c00, DEF_STR( USA ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( French ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( German ) )
//  PORT_DIPSETTING(      0x0000, DEF_STR( Unused ))
	PORT_DIPNAME( 0x7000, 0x7000, DEF_STR( Coinage ))
	PORT_DIPSETTING(      0x7000, "1" )
	PORT_DIPSETTING(      0x3000, "2" )
	PORT_DIPSETTING(      0x5000, "3" )
	PORT_DIPSETTING(      0x1000, "4" )
	PORT_DIPSETTING(      0x6000, "ECA" )
//  PORT_DIPSETTING(      0x4000, DEF_STR( Unused ))
//  PORT_DIPSETTING(      0x2000, DEF_STR( Unused ))
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ))
	PORT_DIPNAME( 0x8000, 0x0000, "Coinage Source" )
	PORT_DIPSETTING(      0x8000, "Dipswitch" )
	PORT_DIPSETTING(      0x0000, "CMOS" )

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT ) // Slam Switch
	PORT_SERVICE_NO_TOGGLE( 0x0010, IP_ACTIVE_LOW )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_VOLUME_DOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_VOLUME_UP )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_INTERLOCK )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BILL1 )
INPUT_PORTS_END


static INPUT_PORTS_START( rmpgwt )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Punch") PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P1 Kick") PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Jump") PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P2 Punch") PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P2 Kick") PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P2 Jump") PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P3 Punch") PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P3 Kick") PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P3 Jump") PORT_PLAYER(3)
	PORT_BIT( 0xff80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0001, 0x0000, "Coinage Source" )
	PORT_DIPSETTING(      0x0001, "Dipswitch" )
	PORT_DIPSETTING(      0x0000, "CMOS" )
	PORT_DIPNAME( 0x003e, 0x003e, DEF_STR( Coinage ))
	PORT_DIPSETTING(      0x003e, "USA-1" )
	PORT_DIPSETTING(      0x003c, "USA-2" )
	PORT_DIPSETTING(      0x003a, "USA-3" )
	PORT_DIPSETTING(      0x0038, "USA-4" )
	PORT_DIPSETTING(      0x0034, "USA-9" )
	PORT_DIPSETTING(      0x0032, "USA-10" )
	PORT_DIPSETTING(      0x0036, "USA-ECA" )
	PORT_DIPSETTING(      0x0030, "USA-Free Play" )
	PORT_DIPSETTING(      0x002e, "German-1" )
	PORT_DIPSETTING(      0x002c, "German-2" )
	PORT_DIPSETTING(      0x002a, "German-3" )
	PORT_DIPSETTING(      0x0028, "German-4" )
	PORT_DIPSETTING(      0x0024, "German-5" )
	PORT_DIPSETTING(      0x0026, "German-ECA" )
	PORT_DIPSETTING(      0x0020, "German-Free Play" )
	PORT_DIPSETTING(      0x001e, "French-1" )
	PORT_DIPSETTING(      0x001c, "French-2" )
	PORT_DIPSETTING(      0x001a, "French-3" )
	PORT_DIPSETTING(      0x0018, "French-4" )
	PORT_DIPSETTING(      0x0014, "French-11" )
	PORT_DIPSETTING(      0x0012, "French-12" )
	PORT_DIPSETTING(      0x0016, "French-ECA" )
	PORT_DIPSETTING(      0x0010, "French-Free Play" )
	PORT_DIPNAME( 0x0040, 0x0000, "Counters" )
	PORT_DIPSETTING(      0x0040, "One" )
	PORT_DIPSETTING(      0x0000, "Two" )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0100, 0x0100, "Bill Validator" )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0400, 0x0000, "Powerup Test" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0400, DEF_STR( On ))
	PORT_BIT( 0x7800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x8000, 0x8000, "Test Switch" )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT ) // Slam Switch
	PORT_SERVICE_NO_TOGGLE( 0x0010, IP_ACTIVE_LOW )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_VOLUME_DOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_VOLUME_UP )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_INTERLOCK )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BILL1 )
INPUT_PORTS_END


static INPUT_PORTS_START( wwfmania )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Punch") PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Defense") PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P1 Power Punch") PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P2 Punch") PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P2 Defense") PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P2 Power Punch") PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P1 Kick") PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("P1 Power Kick") PORT_PLAYER(1)
	PORT_BIT( 0x000c, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P2 Kick") PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("P2 Power Kick") PORT_PLAYER(2)
	PORT_BIT( 0x00c0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0001, 0x0001, "Test Switch" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x0002, 0x0000, "Powerup Test" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0002, DEF_STR( On ))
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0008, 0x0008, "Realtime Clock" )
	PORT_DIPSETTING(      0x0008, DEF_STR( No ))
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ))
	PORT_BIT( 0x0030, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0040, 0x0040, "Bill Validator" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0300, 0x0300, "Counters" )
	PORT_DIPSETTING(      0x0300, "One, 1/1" )
	PORT_DIPSETTING(      0x0200, "One, Totalizing" )
	PORT_DIPSETTING(      0x0100, "Two, 1/1" )
	PORT_DIPNAME( 0x0c00, 0x0c00, "Country" )
	PORT_DIPSETTING(      0x0c00, DEF_STR( USA ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( French ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( German ) )
//  PORT_DIPSETTING(      0x0000, DEF_STR( Unused ))
	PORT_DIPNAME( 0x7000, 0x7000, DEF_STR( Coinage ))
	PORT_DIPSETTING(      0x7000, "1" )
	PORT_DIPSETTING(      0x3000, "2" )
	PORT_DIPSETTING(      0x5000, "3" )
	PORT_DIPSETTING(      0x1000, "4" )
	PORT_DIPSETTING(      0x6000, "ECA" )
//  PORT_DIPSETTING(      0x4000, DEF_STR( Unused ))
//  PORT_DIPSETTING(      0x2000, DEF_STR( Unused ))
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ))
	PORT_DIPNAME( 0x8000, 0x0000, "Coinage Source" )
	PORT_DIPSETTING(      0x8000, "Dipswitch" )
	PORT_DIPSETTING(      0x0000, "CMOS" )

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT ) // Slam Switch
	PORT_SERVICE_NO_TOGGLE( 0x0010, IP_ACTIVE_LOW )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_VOLUME_DOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_VOLUME_UP )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_INTERLOCK ) // Coin Door
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BILL1 )
INPUT_PORTS_END



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

void midwunit_state::wunit(machine_config &config)
{
	constexpr XTAL PIXEL_CLOCK = 8_MHz_XTAL;

	MIDWUNIT_VIDEO(config, m_video, m_palette);
	m_video->dma_irq_cb().set_inputline(m_maincpu, 0);

	TMS34010(config, m_maincpu, 50_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &midwunit_state::main_map);
	m_maincpu->set_halt_on_reset(false);     // halt on reset
	m_maincpu->set_pixel_clock(PIXEL_CLOCK); // pixel clock
	m_maincpu->set_pixels_per_clock(1);      // pixels per clock
	m_maincpu->set_scanline_ind16_callback(m_video, FUNC(midtunit_video_device::scanline_update));  // scanline updater (indexed16)
	m_maincpu->set_shiftreg_in_callback(m_video, FUNC(midtunit_video_device::to_shiftreg));         // write to shiftreg function
	m_maincpu->set_shiftreg_out_callback(m_video, FUNC(midtunit_video_device::from_shiftreg));      // read from shiftreg function
	m_maincpu->set_screen("screen");

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// video hardware
	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 32768);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	// from TMS340 registers
	screen.set_raw(PIXEL_CLOCK, 506, 101, 501, 289, 20, 274);
	screen.set_screen_update(m_maincpu, FUNC(tms34010_device::tms340x0_ind16));
	screen.set_palette(m_palette);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	DCS_AUDIO_8K(config, m_dcs, 0);
	m_dcs->set_maincpu_tag(m_maincpu);
	m_dcs->add_route(0, "mono", 1.0);
}

void midwunit_state::wunit_picsim(machine_config &config)
{
	wunit(config);
	MIDWAY_SERIAL_PIC(config, m_midway_serial_pic, 0);
	m_midway_serial_pic->set_upper(528); // this is actually development PIC code, all games check for in addition to their own game specific code!
}

void midwunit_state::wunit_picemu(machine_config &config)
{
	wunit(config);
	MIDWAY_SERIAL_PIC_EMU(config, m_midway_serial_pic_emu, 0);
}



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

/*
A Mortal Kombat 3 PCB with V2.1 program ROMs was seen with different styled labels:

L1.0  3/29/95 9200  MORTAL KOMBAT III SOUND  U2
 - Labels missing for U3 through U5 -

MORTAL KOMBAT 3  U54  REV 2.1 5/3/95  CCFB  FM.0
MORTAL KOMBAT 3  U63  REV 2.1 5/3/95  015E  FM.1

PROTO  991A 3/20/95  MORTAL KOMBAT III  U133  2M.0
PROTO  9FA5 3/20/95  MORTAL KOMBAT III  U132  2M.1
PROTO  056F 3/20/95  MORTAL KOMBAT III  U131  2M.2
PROTO  775D 3/20/95  MORTAL KOMBAT III  U130  2M.3

PROTO  DE07 3/20/95  MORTAL KOMBAT III  U129  4M.0   <these labels were missing, except for one hand written label.  Following format of U133 - U130>
PROTO  EEE2 3/20/95  MORTAL KOMBAT III  U128  4M.1
PROTO  D3C6 3/20/95  MORTAL KOMBAT III  U127  4M.2
PROTO  0F4D 3/20/95  MORTAL KOMBAT III  U126  4M.3

PROTO  985D 3/20/95  MORTAL KOMBAT III  U125  6M.0  <these labels were missing, except for one hand written label.  Following format of U133 - U130>
PROTO  F576 3/20/95  MORTAL KOMBAT III  U124  6M.1
PROTO  7C81 3/20/95  MORTAL KOMBAT III  U123  6M.2
PROTO  49FE 3/20/95  MORTAL KOMBAT III  U122  6M.3

0435 3/29/95  MORTAL KOMBAT III  U121  8M.0  <these labels were missing, except for one hand written label>
E667 3/29/95  MORTAL KOMBAT III  U120  8M.1
0493 3/29/95  MORTAL KOMBAT III  U119  8M.2
B037 3/29/95  MORTAL KOMBAT III  U118  8M.3

A83E 4/6/95  MORTAL KOMBAT III  U117  AM.0
7B1E 4/6/95  MORTAL KOMBAT III  U116  AM.1
94A7 4/6/95  MORTAL KOMBAT III  U115  AM.2
7F0B 4/6/95  MORTAL KOMBAT III  U114  AM.3
*/
ROM_START( mk3 )
	ROM_REGION16_LE( 0x800000, "dcs", ROMREGION_ERASEFF )   // sound data
	ROM_LOAD16_BYTE( "l1_mortal_kombat_3_u2_music_spch.u2", 0x000000, 0x100000, CRC(5273436f) SHA1(e1735842a0159eafe79d878d44e3828df9bfa5bb) )
	ROM_LOAD16_BYTE( "l1_mortal_kombat_3_u3_music_spch.u3", 0x200000, 0x100000, CRC(856fe411) SHA1(6165ebecfce7500e948d84492ffa19eed7f47091) )
	ROM_LOAD16_BYTE( "l1_mortal_kombat_3_u4_music_spch.u4", 0x400000, 0x100000, CRC(428a406f) SHA1(e70ec83cd054de0da1e178720ed0035b8887f797) )
	ROM_LOAD16_BYTE( "l1_mortal_kombat_3_u5_music_spch.u5", 0x600000, 0x100000, CRC(3b98a09f) SHA1(edf1d02a56dcf3349e6b4bb4097acfe7592305f4) )

	ROM_REGION16_LE( 0x100000, "maincpu", 0 )   // 34010 code
	ROM_LOAD16_BYTE( "mk321u54.bin",  0x00000, 0x80000, CRC(9e344401) SHA1(5760b355f0a5c27c9746f33abfdedf4302f1af38) )
	ROM_LOAD16_BYTE( "mk321u63.bin",  0x00001, 0x80000, CRC(64d34776) SHA1(d8f09e1e946dc13fec5e9f83fdaf61d4076ba9ea) )

	ROM_REGION( 0x2000000, "video", 0 )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u133_game_rom.u133",  0x0000000, 0x100000, CRC(79b94667) SHA1(31bba640c351fdccc6685cadb74dd79a3f910ce8) ) // all GAME ROMs here are also known to be labeled as P1.0
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u132_game_rom.u132",  0x0000001, 0x100000, CRC(13e95228) SHA1(405b05f5a5a55667c2be17d4b399129bdacefd90) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u131_game_rom.u131",  0x0000002, 0x100000, CRC(41001e30) SHA1(2cec91116771951c0380cec5debf4cbb40c14c61) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u130_game_rom.u130",  0x0000003, 0x100000, CRC(49379dd7) SHA1(e6dfab4e23d9cc38ae56c1bbf10ccd160e8fad5e) )

	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u129_game_rom.u129",  0x0400000, 0x100000, CRC(a8b41803) SHA1(9697e35e8bb51d6d36b1d7ae47377b446e57682f) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u128_game_rom.u128",  0x0400001, 0x100000, CRC(b410d72f) SHA1(ac5c1c6f744186540f4ab100d9bd4ce6007e600b) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u127_game_rom.u127",  0x0400002, 0x100000, CRC(bd985be7) SHA1(f5183abea2e5eb2c2c8cefa72c9ed321679f5128) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u126_game_rom.u126",  0x0400003, 0x100000, CRC(e7c32cf4) SHA1(94ea7b2eed7dae66f5dd676c20d6b360140e3e0e) )

	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u125_game_rom.u125",  0x0800000, 0x100000, CRC(9a52227e) SHA1(0474a14fa8dbfea0b0889c1d1756b86391683558) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u124_game_rom.u124",  0x0800001, 0x100000, CRC(5c750ebc) SHA1(45d68af1a56994376e086d840502453c8d6be700) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u123_game_rom.u123",  0x0800002, 0x100000, CRC(f0ab88a8) SHA1(cdc9dc12e162255845c6627b1e35182b7e8502d0) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u122_game_rom.u122",  0x0800003, 0x100000, CRC(9b87cdac) SHA1(a5f8db559293978f23e6f105543d8b2e170a2e0d) )

	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u121_game_rom.u121",  0x0c00000, 0x100000, CRC(b6c6296a) SHA1(7b92a92d65493bb201daf5ece6f00140f017ac51) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u120_game_rom.u120",  0x0c00001, 0x100000, CRC(8d1ccc3b) SHA1(35d91c00113718a08a9d56eb04366f8cf4069ba6) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u119_game_rom.u119",  0x0c00002, 0x100000, CRC(63215b59) SHA1(709bce15fba1520bcba40f0a5cb614542f1b460f) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u118_game_rom.u118",  0x0c00003, 0x100000, CRC(8b681e34) SHA1(524104ba2eb2deb3cfae9760e6b2125ce6b6633e) )

	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u117_game_rom.u117",  0x1000000, 0x080000, CRC(1ab20377) SHA1(0c0d14464d8b23a60e0693669af2ddb82655eff8) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u116_game_rom.u116",  0x1000001, 0x080000, CRC(ba246ad0) SHA1(ec6bdd4b9cd3007bb66bb8de36d148abb30e7f11) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u115_game_rom.u115",  0x1000002, 0x080000, CRC(3ee8b124) SHA1(1523d51e36d3c336b134a562da36a29ba137c0f6) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u114_game_rom.u114",  0x1000003, 0x080000, CRC(a8d99922) SHA1(04ff8d85448701070672f44dbf5bcfd744f1bc8a) )
ROM_END


ROM_START( mk3r20 )
	ROM_REGION16_LE( 0x800000, "dcs", ROMREGION_ERASEFF )   // sound data
	ROM_LOAD16_BYTE( "l1_mortal_kombat_3_u2_music_spch.u2", 0x000000, 0x100000, CRC(5273436f) SHA1(e1735842a0159eafe79d878d44e3828df9bfa5bb) )
	ROM_LOAD16_BYTE( "l1_mortal_kombat_3_u3_music_spch.u3", 0x200000, 0x100000, CRC(856fe411) SHA1(6165ebecfce7500e948d84492ffa19eed7f47091) )
	ROM_LOAD16_BYTE( "l1_mortal_kombat_3_u4_music_spch.u4", 0x400000, 0x100000, CRC(428a406f) SHA1(e70ec83cd054de0da1e178720ed0035b8887f797) )
	ROM_LOAD16_BYTE( "l1_mortal_kombat_3_u5_music_spch.u5", 0x600000, 0x100000, CRC(3b98a09f) SHA1(edf1d02a56dcf3349e6b4bb4097acfe7592305f4) )

	ROM_REGION16_LE( 0x100000, "maincpu", 0 )   // 34010 code
	ROM_LOAD16_BYTE( "mk320u54.bin",  0x00000, 0x80000, CRC(453da302) SHA1(d9a4814e7abb49ac0eb306ad05adcceac68df6a5) )
	ROM_LOAD16_BYTE( "mk320u63.bin",  0x00001, 0x80000, CRC(f8dc0600) SHA1(6eb689d92619c751252155b40af119ad47e94cfa) )

	ROM_REGION( 0x2000000, "video", 0 )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u133_game_rom.u133",  0x0000000, 0x100000, CRC(79b94667) SHA1(31bba640c351fdccc6685cadb74dd79a3f910ce8) ) // all GAME ROMs here are also known to be labeled as P1.0
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u132_game_rom.u132",  0x0000001, 0x100000, CRC(13e95228) SHA1(405b05f5a5a55667c2be17d4b399129bdacefd90) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u131_game_rom.u131",  0x0000002, 0x100000, CRC(41001e30) SHA1(2cec91116771951c0380cec5debf4cbb40c14c61) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u130_game_rom.u130",  0x0000003, 0x100000, CRC(49379dd7) SHA1(e6dfab4e23d9cc38ae56c1bbf10ccd160e8fad5e) )

	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u129_game_rom.u129",  0x0400000, 0x100000, CRC(a8b41803) SHA1(9697e35e8bb51d6d36b1d7ae47377b446e57682f) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u128_game_rom.u128",  0x0400001, 0x100000, CRC(b410d72f) SHA1(ac5c1c6f744186540f4ab100d9bd4ce6007e600b) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u127_game_rom.u127",  0x0400002, 0x100000, CRC(bd985be7) SHA1(f5183abea2e5eb2c2c8cefa72c9ed321679f5128) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u126_game_rom.u126",  0x0400003, 0x100000, CRC(e7c32cf4) SHA1(94ea7b2eed7dae66f5dd676c20d6b360140e3e0e) )

	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u125_game_rom.u125",  0x0800000, 0x100000, CRC(9a52227e) SHA1(0474a14fa8dbfea0b0889c1d1756b86391683558) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u124_game_rom.u124",  0x0800001, 0x100000, CRC(5c750ebc) SHA1(45d68af1a56994376e086d840502453c8d6be700) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u123_game_rom.u123",  0x0800002, 0x100000, CRC(f0ab88a8) SHA1(cdc9dc12e162255845c6627b1e35182b7e8502d0) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u122_game_rom.u122",  0x0800003, 0x100000, CRC(9b87cdac) SHA1(a5f8db559293978f23e6f105543d8b2e170a2e0d) )

	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u121_game_rom.u121",  0x0c00000, 0x100000, CRC(b6c6296a) SHA1(7b92a92d65493bb201daf5ece6f00140f017ac51) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u120_game_rom.u120",  0x0c00001, 0x100000, CRC(8d1ccc3b) SHA1(35d91c00113718a08a9d56eb04366f8cf4069ba6) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u119_game_rom.u119",  0x0c00002, 0x100000, CRC(63215b59) SHA1(709bce15fba1520bcba40f0a5cb614542f1b460f) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u118_game_rom.u118",  0x0c00003, 0x100000, CRC(8b681e34) SHA1(524104ba2eb2deb3cfae9760e6b2125ce6b6633e) )

	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u117_game_rom.u117",  0x1000000, 0x080000, CRC(1ab20377) SHA1(0c0d14464d8b23a60e0693669af2ddb82655eff8) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u116_game_rom.u116",  0x1000001, 0x080000, CRC(ba246ad0) SHA1(ec6bdd4b9cd3007bb66bb8de36d148abb30e7f11) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u115_game_rom.u115",  0x1000002, 0x080000, CRC(3ee8b124) SHA1(1523d51e36d3c336b134a562da36a29ba137c0f6) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u114_game_rom.u114",  0x1000003, 0x080000, CRC(a8d99922) SHA1(04ff8d85448701070672f44dbf5bcfd744f1bc8a) )
ROM_END


ROM_START( mk3r10 )
	ROM_REGION16_LE( 0x800000, "dcs", ROMREGION_ERASEFF )   // sound data
	ROM_LOAD16_BYTE( "l1_mortal_kombat_3_u2_music_spch.u2", 0x000000, 0x100000, CRC(5273436f) SHA1(e1735842a0159eafe79d878d44e3828df9bfa5bb) )
	ROM_LOAD16_BYTE( "l1_mortal_kombat_3_u3_music_spch.u3", 0x200000, 0x100000, CRC(856fe411) SHA1(6165ebecfce7500e948d84492ffa19eed7f47091) )
	ROM_LOAD16_BYTE( "l1_mortal_kombat_3_u4_music_spch.u4", 0x400000, 0x100000, CRC(428a406f) SHA1(e70ec83cd054de0da1e178720ed0035b8887f797) )
	ROM_LOAD16_BYTE( "l1_mortal_kombat_3_u5_music_spch.u5", 0x600000, 0x100000, CRC(3b98a09f) SHA1(edf1d02a56dcf3349e6b4bb4097acfe7592305f4) )

	ROM_REGION16_LE( 0x100000, "maincpu", 0 )   // 34010 code
	ROM_LOAD16_BYTE( "mk310u54.bin",  0x00000, 0x80000, CRC(41829228) SHA1(5686b50a08b528d41b28ef578cfb171da9905c45) )
	ROM_LOAD16_BYTE( "mk310u63.bin",  0x00001, 0x80000, CRC(b074e1e8) SHA1(fe1a6f622614b1ebd8edc3edeec442d39ba2924c) )

	ROM_REGION( 0x2000000, "video", 0 )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u133_game_rom.u133",  0x0000000, 0x100000, CRC(79b94667) SHA1(31bba640c351fdccc6685cadb74dd79a3f910ce8) ) // all GAME ROMs here are also known to be labeled as P1.0
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u132_game_rom.u132",  0x0000001, 0x100000, CRC(13e95228) SHA1(405b05f5a5a55667c2be17d4b399129bdacefd90) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u131_game_rom.u131",  0x0000002, 0x100000, CRC(41001e30) SHA1(2cec91116771951c0380cec5debf4cbb40c14c61) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u130_game_rom.u130",  0x0000003, 0x100000, CRC(49379dd7) SHA1(e6dfab4e23d9cc38ae56c1bbf10ccd160e8fad5e) )

	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u129_game_rom.u129",  0x0400000, 0x100000, CRC(a8b41803) SHA1(9697e35e8bb51d6d36b1d7ae47377b446e57682f) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u128_game_rom.u128",  0x0400001, 0x100000, CRC(b410d72f) SHA1(ac5c1c6f744186540f4ab100d9bd4ce6007e600b) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u127_game_rom.u127",  0x0400002, 0x100000, CRC(bd985be7) SHA1(f5183abea2e5eb2c2c8cefa72c9ed321679f5128) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u126_game_rom.u126",  0x0400003, 0x100000, CRC(e7c32cf4) SHA1(94ea7b2eed7dae66f5dd676c20d6b360140e3e0e) )

	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u125_game_rom.u125",  0x0800000, 0x100000, CRC(9a52227e) SHA1(0474a14fa8dbfea0b0889c1d1756b86391683558) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u124_game_rom.u124",  0x0800001, 0x100000, CRC(5c750ebc) SHA1(45d68af1a56994376e086d840502453c8d6be700) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u123_game_rom.u123",  0x0800002, 0x100000, CRC(f0ab88a8) SHA1(cdc9dc12e162255845c6627b1e35182b7e8502d0) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u122_game_rom.u122",  0x0800003, 0x100000, CRC(9b87cdac) SHA1(a5f8db559293978f23e6f105543d8b2e170a2e0d) )

	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u121_game_rom.u121",  0x0c00000, 0x100000, CRC(b6c6296a) SHA1(7b92a92d65493bb201daf5ece6f00140f017ac51) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u120_game_rom.u120",  0x0c00001, 0x100000, CRC(8d1ccc3b) SHA1(35d91c00113718a08a9d56eb04366f8cf4069ba6) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u119_game_rom.u119",  0x0c00002, 0x100000, CRC(63215b59) SHA1(709bce15fba1520bcba40f0a5cb614542f1b460f) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u118_game_rom.u118",  0x0c00003, 0x100000, CRC(8b681e34) SHA1(524104ba2eb2deb3cfae9760e6b2125ce6b6633e) )

	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u117_game_rom.u117",  0x1000000, 0x080000, CRC(1ab20377) SHA1(0c0d14464d8b23a60e0693669af2ddb82655eff8) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u116_game_rom.u116",  0x1000001, 0x080000, CRC(ba246ad0) SHA1(ec6bdd4b9cd3007bb66bb8de36d148abb30e7f11) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u115_game_rom.u115",  0x1000002, 0x080000, CRC(3ee8b124) SHA1(1523d51e36d3c336b134a562da36a29ba137c0f6) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u114_game_rom.u114",  0x1000003, 0x080000, CRC(a8d99922) SHA1(04ff8d85448701070672f44dbf5bcfd744f1bc8a) )
ROM_END


ROM_START( mk3p40 )
	ROM_REGION16_LE( 0x800000, "dcs", ROMREGION_ERASEFF )   // sound data
	ROM_LOAD16_BYTE( "l1_mortal_kombat_3_u2_music_spch.u2", 0x000000, 0x100000, CRC(5273436f) SHA1(e1735842a0159eafe79d878d44e3828df9bfa5bb) )
	ROM_LOAD16_BYTE( "l1_mortal_kombat_3_u3_music_spch.u3", 0x200000, 0x100000, CRC(856fe411) SHA1(6165ebecfce7500e948d84492ffa19eed7f47091) )
	ROM_LOAD16_BYTE( "l1_mortal_kombat_3_u4_music_spch.u4", 0x400000, 0x100000, CRC(428a406f) SHA1(e70ec83cd054de0da1e178720ed0035b8887f797) )
	ROM_LOAD16_BYTE( "l1_mortal_kombat_3_u5_music_spch.u5", 0x600000, 0x100000, CRC(3b98a09f) SHA1(edf1d02a56dcf3349e6b4bb4097acfe7592305f4) )

	ROM_REGION16_LE( 0x100000, "maincpu", 0 )   // 34010 code
	ROM_LOAD16_BYTE( "mk3p40.u54",  0x00000, 0x80000, CRC(4dfb0748) SHA1(8c628a51642c940de8abb795be36123e4008ce15) )
	ROM_LOAD16_BYTE( "mk3p40.u63",  0x00001, 0x80000, CRC(f25a8083) SHA1(ff11462d23d9e16f6ee0d77bf85caa996df32618) )

	ROM_REGION( 0x2000000, "video", 0 )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u133_game_rom.u133",  0x0000000, 0x100000, CRC(79b94667) SHA1(31bba640c351fdccc6685cadb74dd79a3f910ce8) ) // all GAME ROMs here are also known to be labeled as P1.0
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u132_game_rom.u132",  0x0000001, 0x100000, CRC(13e95228) SHA1(405b05f5a5a55667c2be17d4b399129bdacefd90) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u131_game_rom.u131",  0x0000002, 0x100000, CRC(41001e30) SHA1(2cec91116771951c0380cec5debf4cbb40c14c61) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u130_game_rom.u130",  0x0000003, 0x100000, CRC(49379dd7) SHA1(e6dfab4e23d9cc38ae56c1bbf10ccd160e8fad5e) )

	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u129_game_rom.u129",  0x0400000, 0x100000, CRC(a8b41803) SHA1(9697e35e8bb51d6d36b1d7ae47377b446e57682f) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u128_game_rom.u128",  0x0400001, 0x100000, CRC(b410d72f) SHA1(ac5c1c6f744186540f4ab100d9bd4ce6007e600b) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u127_game_rom.u127",  0x0400002, 0x100000, CRC(bd985be7) SHA1(f5183abea2e5eb2c2c8cefa72c9ed321679f5128) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u126_game_rom.u126",  0x0400003, 0x100000, CRC(e7c32cf4) SHA1(94ea7b2eed7dae66f5dd676c20d6b360140e3e0e) )

	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u125_game_rom.u125",  0x0800000, 0x100000, CRC(9a52227e) SHA1(0474a14fa8dbfea0b0889c1d1756b86391683558) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u124_game_rom.u124",  0x0800001, 0x100000, CRC(5c750ebc) SHA1(45d68af1a56994376e086d840502453c8d6be700) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u123_game_rom.u123",  0x0800002, 0x100000, CRC(f0ab88a8) SHA1(cdc9dc12e162255845c6627b1e35182b7e8502d0) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u122_game_rom.u122",  0x0800003, 0x100000, CRC(9b87cdac) SHA1(a5f8db559293978f23e6f105543d8b2e170a2e0d) )

	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u121_game_rom.u121",  0x0c00000, 0x100000, CRC(b6c6296a) SHA1(7b92a92d65493bb201daf5ece6f00140f017ac51) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u120_game_rom.u120",  0x0c00001, 0x100000, CRC(8d1ccc3b) SHA1(35d91c00113718a08a9d56eb04366f8cf4069ba6) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u119_game_rom.u119",  0x0c00002, 0x100000, CRC(63215b59) SHA1(709bce15fba1520bcba40f0a5cb614542f1b460f) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u118_game_rom.u118",  0x0c00003, 0x100000, CRC(8b681e34) SHA1(524104ba2eb2deb3cfae9760e6b2125ce6b6633e) )

	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u117_game_rom.u117",  0x1000000, 0x080000, CRC(1ab20377) SHA1(0c0d14464d8b23a60e0693669af2ddb82655eff8) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u116_game_rom.u116",  0x1000001, 0x080000, CRC(ba246ad0) SHA1(ec6bdd4b9cd3007bb66bb8de36d148abb30e7f11) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u115_game_rom.u115",  0x1000002, 0x080000, CRC(3ee8b124) SHA1(1523d51e36d3c336b134a562da36a29ba137c0f6) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u114_game_rom.u114",  0x1000003, 0x080000, CRC(a8d99922) SHA1(04ff8d85448701070672f44dbf5bcfd744f1bc8a) )
ROM_END


ROM_START( umk3 )
	ROM_REGION16_LE( 0x800000, "dcs", ROMREGION_ERASEFF )   // sound data
	ROM_LOAD16_BYTE( "l2.0_mortal_kombat_3_u2_ultimate.u2", 0x000000, 0x100000, CRC(3838cfe5) SHA1(e3d2901f3bae1362742fc6ee0aa31c9f63b4dfa3) ) // verified labeled as L2.0
	ROM_LOAD16_BYTE( "l1_mortal_kombat_3_u3_music_spch.u3", 0x200000, 0x100000, CRC(856fe411) SHA1(6165ebecfce7500e948d84492ffa19eed7f47091) )
	ROM_LOAD16_BYTE( "l1_mortal_kombat_3_u4_music_spch.u4", 0x400000, 0x100000, CRC(428a406f) SHA1(e70ec83cd054de0da1e178720ed0035b8887f797) )
	ROM_LOAD16_BYTE( "l1_mortal_kombat_3_u5_music_spch.u5", 0x600000, 0x100000, CRC(3b98a09f) SHA1(edf1d02a56dcf3349e6b4bb4097acfe7592305f4) )

	ROM_REGION16_LE( 0x100000, "maincpu", 0 )   // 34010 code
	ROM_LOAD16_BYTE( "l1.2_mortal_kombat_3_u54_ultimate.u54",  0x00000, 0x80000, CRC(712b4db6) SHA1(7015a55f3d745c6aeb8630903e2d5cd9554b2766) )
	ROM_LOAD16_BYTE( "l1.2_mortal_kombat_3_u63_ultimate.u63",  0x00001, 0x80000, CRC(6d301faf) SHA1(18a8e29cc3e8ce5cc0e10f8386d43e7f44fd7b75) )

	ROM_REGION( 0x1009, "serial_security:pic", 0 )   // security PIC (provides game ID code and serial number)
	ROM_LOAD( "463_mk3_ultimate.u64",  0x0000, 0x1009, CRC(4f425218) SHA1(7f26045ed2c9ca94fadcb673ce10f28208aa720e) )

	ROM_REGION( 0x2000000, "video", 0 )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u133_game_rom.u133",  0x0000000, 0x100000, CRC(79b94667) SHA1(31bba640c351fdccc6685cadb74dd79a3f910ce8) ) // all GAME ROMs here are also known to be labeled as P1.0
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u132_game_rom.u132",  0x0000001, 0x100000, CRC(13e95228) SHA1(405b05f5a5a55667c2be17d4b399129bdacefd90) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u131_game_rom.u131",  0x0000002, 0x100000, CRC(41001e30) SHA1(2cec91116771951c0380cec5debf4cbb40c14c61) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u130_game_rom.u130",  0x0000003, 0x100000, CRC(49379dd7) SHA1(e6dfab4e23d9cc38ae56c1bbf10ccd160e8fad5e) )

	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u129_game_rom.u129",  0x0400000, 0x100000, CRC(a8b41803) SHA1(9697e35e8bb51d6d36b1d7ae47377b446e57682f) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u128_game_rom.u128",  0x0400001, 0x100000, CRC(b410d72f) SHA1(ac5c1c6f744186540f4ab100d9bd4ce6007e600b) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u127_game_rom.u127",  0x0400002, 0x100000, CRC(bd985be7) SHA1(f5183abea2e5eb2c2c8cefa72c9ed321679f5128) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u126_game_rom.u126",  0x0400003, 0x100000, CRC(e7c32cf4) SHA1(94ea7b2eed7dae66f5dd676c20d6b360140e3e0e) )

	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u125_game_rom.u125",  0x0800000, 0x100000, CRC(9a52227e) SHA1(0474a14fa8dbfea0b0889c1d1756b86391683558) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u124_game_rom.u124",  0x0800001, 0x100000, CRC(5c750ebc) SHA1(45d68af1a56994376e086d840502453c8d6be700) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u123_game_rom.u123",  0x0800002, 0x100000, CRC(f0ab88a8) SHA1(cdc9dc12e162255845c6627b1e35182b7e8502d0) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u122_game_rom.u122",  0x0800003, 0x100000, CRC(9b87cdac) SHA1(a5f8db559293978f23e6f105543d8b2e170a2e0d) )

	ROM_LOAD32_BYTE( "umk-u121.bin",  0x0c00000, 0x100000, CRC(cc4b95db) SHA1(3d53180eec649e9616c4b87db55573f12d9bfee3) ) // PCB pictures show EPROMs labeled as:
	ROM_LOAD32_BYTE( "umk-u120.bin",  0x0c00001, 0x100000, CRC(1c8144cd) SHA1(77cdc1eaf630ccb7233f5532f8b08191d00f0816) ) // L2.0 MORTAL KOMBAT 3 Uxxx ULTIMATE
	ROM_LOAD32_BYTE( "umk-u119.bin",  0x0c00002, 0x100000, CRC(5f10c543) SHA1(24dc83b7aa531ebd399258ffa7b2e028f1c4a28e) ) // currently unverified if these are the same as for v1.0 & v1.1 UMK3
	ROM_LOAD32_BYTE( "umk-u118.bin",  0x0c00003, 0x100000, CRC(de0c4488) SHA1(227cab34798c440b2a45223567113df5f17d913f) )

	// sockets U114 through U117 are left empty for this version

	ROM_LOAD32_BYTE( "umk-u113.bin",  0x1400000, 0x100000, CRC(99d74a1e) SHA1(ed3068afa98287ea290d1f537f5009d3b6d683da) )
	ROM_LOAD32_BYTE( "umk-u112.bin",  0x1400001, 0x100000, CRC(b5a46488) SHA1(dbf22e55d200eb9ff550f48b223cf0c6114a9357) )
	ROM_LOAD32_BYTE( "umk-u111.bin",  0x1400002, 0x100000, CRC(a87523c8) SHA1(e70b7599fef82001f762fc2c48f7b85474431ccc) )
	ROM_LOAD32_BYTE( "umk-u110.bin",  0x1400003, 0x100000, CRC(0038f205) SHA1(059c1c71a2d92ee6db36c09831d213a48a7e81d0) )
ROM_END


ROM_START( umk3r11 )
	ROM_REGION16_LE( 0x800000, "dcs", ROMREGION_ERASEFF )   // sound data
	ROM_LOAD16_BYTE( "l2.0_mortal_kombat_3_u2_ultimate.u2", 0x000000, 0x100000, CRC(3838cfe5) SHA1(e3d2901f3bae1362742fc6ee0aa31c9f63b4dfa3) ) // verified labeled as L2.0
	ROM_LOAD16_BYTE( "l1_mortal_kombat_3_u3_music_spch.u3", 0x200000, 0x100000, CRC(856fe411) SHA1(6165ebecfce7500e948d84492ffa19eed7f47091) )
	ROM_LOAD16_BYTE( "l1_mortal_kombat_3_u4_music_spch.u4", 0x400000, 0x100000, CRC(428a406f) SHA1(e70ec83cd054de0da1e178720ed0035b8887f797) )
	ROM_LOAD16_BYTE( "l1_mortal_kombat_3_u5_music_spch.u5", 0x600000, 0x100000, CRC(3b98a09f) SHA1(edf1d02a56dcf3349e6b4bb4097acfe7592305f4) )

	ROM_REGION16_LE( 0x100000, "maincpu", 0 )   // 34010 code
	ROM_LOAD16_BYTE( "l1.1_mortal_kombat_3_u54_ultimate.u54",  0x00000, 0x80000, CRC(8bb27659) SHA1(a3ffe3d8f21c261b36c7510d620d691a8bbf665b) )
	ROM_LOAD16_BYTE( "l1.1_mortal_kombat_3_u63_ultimate.u63",  0x00001, 0x80000, CRC(ea731783) SHA1(2915626090650c4b5adf5b26e736c3ec91ce81a6) )

	ROM_REGION( 0x1009, "serial_security:pic", 0 )   // security PIC (provides game ID code and serial number)
	ROM_LOAD( "463_mk3_ultimate.u64",  0x0000, 0x1009, CRC(4f425218) SHA1(7f26045ed2c9ca94fadcb673ce10f28208aa720e) )

	ROM_REGION( 0x2000000, "video", 0 )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u133_game_rom.u133",  0x0000000, 0x100000, CRC(79b94667) SHA1(31bba640c351fdccc6685cadb74dd79a3f910ce8) ) // all GAME ROMs here are also known to be labeled as P1.0
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u132_game_rom.u132",  0x0000001, 0x100000, CRC(13e95228) SHA1(405b05f5a5a55667c2be17d4b399129bdacefd90) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u131_game_rom.u131",  0x0000002, 0x100000, CRC(41001e30) SHA1(2cec91116771951c0380cec5debf4cbb40c14c61) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u130_game_rom.u130",  0x0000003, 0x100000, CRC(49379dd7) SHA1(e6dfab4e23d9cc38ae56c1bbf10ccd160e8fad5e) )

	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u129_game_rom.u129",  0x0400000, 0x100000, CRC(a8b41803) SHA1(9697e35e8bb51d6d36b1d7ae47377b446e57682f) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u128_game_rom.u128",  0x0400001, 0x100000, CRC(b410d72f) SHA1(ac5c1c6f744186540f4ab100d9bd4ce6007e600b) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u127_game_rom.u127",  0x0400002, 0x100000, CRC(bd985be7) SHA1(f5183abea2e5eb2c2c8cefa72c9ed321679f5128) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u126_game_rom.u126",  0x0400003, 0x100000, CRC(e7c32cf4) SHA1(94ea7b2eed7dae66f5dd676c20d6b360140e3e0e) )

	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u125_game_rom.u125",  0x0800000, 0x100000, CRC(9a52227e) SHA1(0474a14fa8dbfea0b0889c1d1756b86391683558) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u124_game_rom.u124",  0x0800001, 0x100000, CRC(5c750ebc) SHA1(45d68af1a56994376e086d840502453c8d6be700) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u123_game_rom.u123",  0x0800002, 0x100000, CRC(f0ab88a8) SHA1(cdc9dc12e162255845c6627b1e35182b7e8502d0) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u122_game_rom.u122",  0x0800003, 0x100000, CRC(9b87cdac) SHA1(a5f8db559293978f23e6f105543d8b2e170a2e0d) )

	ROM_LOAD32_BYTE( "mortal_kombat_iii_ultimate_u121_video_image.u121",  0x0c00000, 0x100000, CRC(cc4b95db) SHA1(3d53180eec649e9616c4b87db55573f12d9bfee3) ) // Both v1.0 & v1.1 have been found with mask roms
	ROM_LOAD32_BYTE( "mortal_kombat_iii_ultimate_u120_video_image.u120",  0x0c00001, 0x100000, CRC(1c8144cd) SHA1(77cdc1eaf630ccb7233f5532f8b08191d00f0816) )
	ROM_LOAD32_BYTE( "mortal_kombat_iii_ultimate_u119_video_image.u119",  0x0c00002, 0x100000, CRC(5f10c543) SHA1(24dc83b7aa531ebd399258ffa7b2e028f1c4a28e) )
	ROM_LOAD32_BYTE( "mortal_kombat_iii_ultimate_u118_video_image.u118",  0x0c00003, 0x100000, CRC(de0c4488) SHA1(227cab34798c440b2a45223567113df5f17d913f) )

	// sockets U114 through U117 are left empty for this version

	ROM_LOAD32_BYTE( "mortal_kombat_iii_ultimate_u113_video_image.u113",  0x1400000, 0x100000, CRC(99d74a1e) SHA1(ed3068afa98287ea290d1f537f5009d3b6d683da) )
	ROM_LOAD32_BYTE( "mortal_kombat_iii_ultimate_u112_video_image.u112",  0x1400001, 0x100000, CRC(b5a46488) SHA1(dbf22e55d200eb9ff550f48b223cf0c6114a9357) )
	ROM_LOAD32_BYTE( "mortal_kombat_iii_ultimate_u111_video_image.u111",  0x1400002, 0x100000, CRC(a87523c8) SHA1(e70b7599fef82001f762fc2c48f7b85474431ccc) )
	ROM_LOAD32_BYTE( "mortal_kombat_iii_ultimate_u110_video_image.u110",  0x1400003, 0x100000, CRC(0038f205) SHA1(059c1c71a2d92ee6db36c09831d213a48a7e81d0) )
ROM_END


ROM_START( umk3r10 )
	ROM_REGION16_LE( 0x800000, "dcs", ROMREGION_ERASEFF )   // sound data
	ROM_LOAD16_BYTE( "l1.0_mortal_kombat_3_u2_ultimate.u2", 0x000000, 0x100000, CRC(3838cfe5) SHA1(e3d2901f3bae1362742fc6ee0aa31c9f63b4dfa3) ) // verified labeled as L1.0
	ROM_LOAD16_BYTE( "l1_mortal_kombat_3_u3_music_spch.u3", 0x200000, 0x100000, CRC(856fe411) SHA1(6165ebecfce7500e948d84492ffa19eed7f47091) )
	ROM_LOAD16_BYTE( "l1_mortal_kombat_3_u4_music_spch.u4", 0x400000, 0x100000, CRC(428a406f) SHA1(e70ec83cd054de0da1e178720ed0035b8887f797) )
	ROM_LOAD16_BYTE( "l1_mortal_kombat_3_u5_music_spch.u5", 0x600000, 0x100000, CRC(3b98a09f) SHA1(edf1d02a56dcf3349e6b4bb4097acfe7592305f4) )

	ROM_REGION16_LE( 0x100000, "maincpu", 0 )   // 34010 code
	ROM_LOAD16_BYTE( "l1.0_mortal_kombat_3_u54_ultimate.u54",  0x00000, 0x80000, CRC(dfd735da) SHA1(bcb6d80dbde407d0042ec2f225b2f98740a79203) )
	ROM_LOAD16_BYTE( "l1.0_mortal_kombat_3_u63_ultimate.u63",  0x00001, 0x80000, CRC(2dff0c83) SHA1(8942ffa3addf134085ea8d77d56e82593312e7a5) )

	ROM_REGION( 0x1009, "serial_security:pic", 0 )   // security PIC (provides game ID code and serial number)
	ROM_LOAD( "463_mk3_ultimate.u64",  0x0000, 0x1009, CRC(4f425218) SHA1(7f26045ed2c9ca94fadcb673ce10f28208aa720e) )

	ROM_REGION( 0x2000000, "video", 0 )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u133_game_rom.u133",  0x0000000, 0x100000, CRC(79b94667) SHA1(31bba640c351fdccc6685cadb74dd79a3f910ce8) ) // all GAME ROMs here are also known to be labeled as P1.0
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u132_game_rom.u132",  0x0000001, 0x100000, CRC(13e95228) SHA1(405b05f5a5a55667c2be17d4b399129bdacefd90) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u131_game_rom.u131",  0x0000002, 0x100000, CRC(41001e30) SHA1(2cec91116771951c0380cec5debf4cbb40c14c61) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u130_game_rom.u130",  0x0000003, 0x100000, CRC(49379dd7) SHA1(e6dfab4e23d9cc38ae56c1bbf10ccd160e8fad5e) )

	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u129_game_rom.u129",  0x0400000, 0x100000, CRC(a8b41803) SHA1(9697e35e8bb51d6d36b1d7ae47377b446e57682f) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u128_game_rom.u128",  0x0400001, 0x100000, CRC(b410d72f) SHA1(ac5c1c6f744186540f4ab100d9bd4ce6007e600b) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u127_game_rom.u127",  0x0400002, 0x100000, CRC(bd985be7) SHA1(f5183abea2e5eb2c2c8cefa72c9ed321679f5128) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u126_game_rom.u126",  0x0400003, 0x100000, CRC(e7c32cf4) SHA1(94ea7b2eed7dae66f5dd676c20d6b360140e3e0e) )

	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u125_game_rom.u125",  0x0800000, 0x100000, CRC(9a52227e) SHA1(0474a14fa8dbfea0b0889c1d1756b86391683558) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u124_game_rom.u124",  0x0800001, 0x100000, CRC(5c750ebc) SHA1(45d68af1a56994376e086d840502453c8d6be700) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u123_game_rom.u123",  0x0800002, 0x100000, CRC(f0ab88a8) SHA1(cdc9dc12e162255845c6627b1e35182b7e8502d0) )
	ROM_LOAD32_BYTE( "l1_mortal_kombat_3_u122_game_rom.u122",  0x0800003, 0x100000, CRC(9b87cdac) SHA1(a5f8db559293978f23e6f105543d8b2e170a2e0d) )

	ROM_LOAD32_BYTE( "mortal_kombat_iii_ultimate_u121_video_image.u121",  0x0c00000, 0x100000, CRC(cc4b95db) SHA1(3d53180eec649e9616c4b87db55573f12d9bfee3) ) // Both v1.0 & v1.1 have been found with mask roms
	ROM_LOAD32_BYTE( "mortal_kombat_iii_ultimate_u120_video_image.u120",  0x0c00001, 0x100000, CRC(1c8144cd) SHA1(77cdc1eaf630ccb7233f5532f8b08191d00f0816) )
	ROM_LOAD32_BYTE( "mortal_kombat_iii_ultimate_u119_video_image.u119",  0x0c00002, 0x100000, CRC(5f10c543) SHA1(24dc83b7aa531ebd399258ffa7b2e028f1c4a28e) )
	ROM_LOAD32_BYTE( "mortal_kombat_iii_ultimate_u118_video_image.u118",  0x0c00003, 0x100000, CRC(de0c4488) SHA1(227cab34798c440b2a45223567113df5f17d913f) )

	// sockets U114 through U117 are left empty for this version

	ROM_LOAD32_BYTE( "mortal_kombat_iii_ultimate_u113_video_image.u113",  0x1400000, 0x100000, CRC(99d74a1e) SHA1(ed3068afa98287ea290d1f537f5009d3b6d683da) )
	ROM_LOAD32_BYTE( "mortal_kombat_iii_ultimate_u112_video_image.u112",  0x1400001, 0x100000, CRC(b5a46488) SHA1(dbf22e55d200eb9ff550f48b223cf0c6114a9357) )
	ROM_LOAD32_BYTE( "mortal_kombat_iii_ultimate_u111_video_image.u111",  0x1400002, 0x100000, CRC(a87523c8) SHA1(e70b7599fef82001f762fc2c48f7b85474431ccc) )
	ROM_LOAD32_BYTE( "mortal_kombat_iii_ultimate_u110_video_image.u110",  0x1400003, 0x100000, CRC(0038f205) SHA1(059c1c71a2d92ee6db36c09831d213a48a7e81d0) )
ROM_END


ROM_START( openice )
	ROM_REGION16_LE( 0x800000, "dcs", ROMREGION_ERASEFF )   // sound data
	ROM_LOAD16_BYTE( "open_ice_l1.2.u2", 0x000000, 0x100000, CRC(8adb5aab) SHA1(4c25bc051c90947f3366f83ac5ca8dc78e26b8a4) ) // This one labeled as L1.2
	ROM_LOAD16_BYTE( "open_ice_l1.u3",   0x200000, 0x100000, CRC(11c61ad6) SHA1(324621d6b486399b6d5ede1fed39d4e448cdeb32) ) // This one labeled as L1
	ROM_LOAD16_BYTE( "open_ice_l1.u4",   0x400000, 0x100000, CRC(04279290) SHA1(daf1e57137ae1c3434194054e69809bfe3ed1fc3) ) // This one labeled as L1
	ROM_LOAD16_BYTE( "open_ice_l1.u5",   0x600000, 0x100000, CRC(e90ad61f) SHA1(59eeabcae7e0e70cdb4472cde64b8a28b07ede98) ) // This one labeled as L1

	ROM_REGION16_LE( 0x100000, "maincpu", 0 )   // 34010 code
	ROM_LOAD16_BYTE( "open_ice_l1.21.u54", 0x00000, 0x80000, CRC(e4225284) SHA1(d5e267cf35826c106bb0a800363849ed4d489e56) ) // Labeled as L1.21
	ROM_LOAD16_BYTE( "open_ice_l1.21.u63", 0x00001, 0x80000, CRC(97d308a3) SHA1(0a517fab77bc2277884587c7e29e392bb360d27b) ) // Labeled as L1.21

	ROM_REGION( 0x2000000, "video", 0 )
	ROM_LOAD32_BYTE( "open_ice_l1.2.u133", 0x0000000, 0x100000, CRC(8a81605c) SHA1(cf397b8da242566b21579b90528857ccd2f93141) ) // These 4 are labeled as L1.2
	ROM_LOAD32_BYTE( "open_ice_l1.2.u132", 0x0000001, 0x100000, CRC(cfdd6702) SHA1(0198d2cc2de93a8aa345ba0af8d92713d798be8a) )
	ROM_LOAD32_BYTE( "open_ice_l1.2.u131", 0x0000002, 0x100000, CRC(cc428eb7) SHA1(ff2403077453f24bd1b176f57b17649b1b64bccf) )
	ROM_LOAD32_BYTE( "open_ice_l1.2.u130", 0x0000003, 0x100000, CRC(74c2d50c) SHA1(7880a28b003aa44878384efcb72b98833383f67e) )

	ROM_LOAD32_BYTE( "open_ice_l1.2.u129", 0x0400000, 0x100000, CRC(9e2ff012) SHA1(35160ab239f0d8efcb2dc67dee4bd8d204226e3d) ) // These 4 are labeled as L1.2
	ROM_LOAD32_BYTE( "open_ice_l1.2.u128", 0x0400001, 0x100000, CRC(35d2e610) SHA1(c4bd18f44592299f120344ecaf1464a8b31d80c8) )
	ROM_LOAD32_BYTE( "open_ice_l1.2.u127", 0x0400002, 0x100000, CRC(bcbf19fe) SHA1(e28f0238ef020b75b10318e5c3dd4c5472b3638a) )
	ROM_LOAD32_BYTE( "open_ice_l1.2.u126", 0x0400003, 0x100000, CRC(8e3106ae) SHA1(58d1fd097e23578195d28671f22cfa3ed161c0f5) )

	ROM_LOAD32_BYTE( "open_ice_l1.u125",   0x0800000, 0x100000, CRC(a7b54550) SHA1(83e3627c4e84466ec10023b0e2259ad86b791fd7) ) // Yes, these 4 are labeled as L1, NOT L1.2
	ROM_LOAD32_BYTE( "open_ice_l1.u124",   0x0800001, 0x100000, CRC(7c02cb50) SHA1(92d24bcfd66396c52c823b816118eed39c4ef9cd) )
	ROM_LOAD32_BYTE( "open_ice_l1.u123",   0x0800002, 0x100000, CRC(d543bd9d) SHA1(a9ff8589fe185ea058b549c2ed4e71f6c50e9638) )
	ROM_LOAD32_BYTE( "open_ice_l1.u122",   0x0800003, 0x100000, CRC(3744d291) SHA1(e4484f377a66c4c64b015ef461419d956b6e23e4) )

	ROM_LOAD32_BYTE( "open_ice_l1.2.u121", 0x0c00000, 0x100000, CRC(acd2f7c7) SHA1(82d6f09e63a825b118c36d668427011cd8892eaa) ) // These 4 are labeled as L1.2
	ROM_LOAD32_BYTE( "open_ice_l1.2.u120", 0x0c00001, 0x100000, CRC(4295686a) SHA1(2522e57335bb8cca6d76942d2fd62560f88e37a6) )
	ROM_LOAD32_BYTE( "open_ice_l1.2.u119", 0x0c00002, 0x100000, CRC(948b9b27) SHA1(62d031410f491d557e27ba055d3db9d36d5a153c) )
	ROM_LOAD32_BYTE( "open_ice_l1.2.u118", 0x0c00003, 0x100000, CRC(9eaaf93e) SHA1(56bd881df5282f659ac68ace960a3b085c13dd9d) )
ROM_END


ROM_START( openicea ) // PCB had alternate ROM labels showing the dates & checksums
	ROM_REGION16_LE( 0x800000, "dcs", ROMREGION_ERASEFF )   // sound data
	ROM_LOAD16_BYTE( "open_ice_l1.2.u2", 0x000000, 0x100000, CRC(8adb5aab) SHA1(4c25bc051c90947f3366f83ac5ca8dc78e26b8a4) ) // U2  OPEN ICE HOCKEY  R1.2  10/10/95  1900
	ROM_LOAD16_BYTE( "open_ice_l1.u3",   0x200000, 0x100000, CRC(11c61ad6) SHA1(324621d6b486399b6d5ede1fed39d4e448cdeb32) ) // U3  OPEN ICE HOCKEY  9/1/95  4D00
	ROM_LOAD16_BYTE( "open_ice_l1.u4",   0x400000, 0x100000, CRC(04279290) SHA1(daf1e57137ae1c3434194054e69809bfe3ed1fc3) ) // U4  OPEN ICE HOCKEY  9/1/95  4700
	ROM_LOAD16_BYTE( "open_ice_l1.u5",   0x600000, 0x100000, CRC(e90ad61f) SHA1(59eeabcae7e0e70cdb4472cde64b8a28b07ede98) ) // U5  OPEN ICE HOCKEY  9/1/95  C200

	ROM_REGION16_LE( 0x100000, "maincpu", 0 )   // 34010 code
	ROM_LOAD16_BYTE( "open_ice_781c_r1.2a.u54", 0x00000, 0x80000, CRC(63296053) SHA1(9f6fcb1f95a09165c211b569001563b56d06876c) ) // hand written as  781C  R1.21 - game reports as Revision 1.2A
	ROM_LOAD16_BYTE( "open_ice_6937_r1.2a.u63", 0x00001, 0x80000, CRC(04441034) SHA1(d0af6305749a26adddb17aabb512e0347fcac767) ) // hand written as  6937  R1.21 - game reports as Revision 1.2A

	ROM_REGION( 0x2000000, "video", 0 ) // label for U133 is likely for v1.1 with different data for that bank of 4 roms dated 9/14/95
	ROM_LOAD32_BYTE( "open_ice_l1.2.u133", 0x0000000, 0x100000, CRC(8a81605c) SHA1(cf397b8da242566b21579b90528857ccd2f93141) ) // 2M.0  U133  OPEN ICE HOCKEY  9/14/95  EE39 - however, this should be: 2M.0  U133  OPEN ICE HOCKEY  10/17/95  04E5 - Same as parent
	ROM_LOAD32_BYTE( "open_ice_l1.2.u132", 0x0000001, 0x100000, CRC(cfdd6702) SHA1(0198d2cc2de93a8aa345ba0af8d92713d798be8a) ) // 2M.1  U132  OPEN ICE HOCKEY  10/17/95  595E
	ROM_LOAD32_BYTE( "open_ice_l1.2.u131", 0x0000002, 0x100000, CRC(cc428eb7) SHA1(ff2403077453f24bd1b176f57b17649b1b64bccf) ) // 2M.2  U131  OPEN ICE HOCKEY  10/17/95  B2BF
	ROM_LOAD32_BYTE( "open_ice_l1.2.u130", 0x0000003, 0x100000, CRC(74c2d50c) SHA1(7880a28b003aa44878384efcb72b98833383f67e) ) // 2M.3  U130  OPEN ICE HOCKEY  10/17/95  D784

	ROM_LOAD32_BYTE( "open_ice_l1.2.u129", 0x0400000, 0x100000, CRC(9e2ff012) SHA1(35160ab239f0d8efcb2dc67dee4bd8d204226e3d) ) // 4M.0  U129  OPEN ICE HOCKEY  9/14/95  97E0
	ROM_LOAD32_BYTE( "open_ice_l1.2.u128", 0x0400001, 0x100000, CRC(35d2e610) SHA1(c4bd18f44592299f120344ecaf1464a8b31d80c8) ) // 4M.1  U128  OPEN ICE HOCKEY  9/14/95  96FC
	ROM_LOAD32_BYTE( "open_ice_l1.2.u127", 0x0400002, 0x100000, CRC(bcbf19fe) SHA1(e28f0238ef020b75b10318e5c3dd4c5472b3638a) ) // 4M.2  U127  OPEN ICE HOCKEY  9/14/95  6A67
	ROM_LOAD32_BYTE( "open_ice_l1.2.u126", 0x0400003, 0x100000, CRC(8e3106ae) SHA1(58d1fd097e23578195d28671f22cfa3ed161c0f5) ) // 4M.3  U126  OPEN ICE HOCKEY  9/14/95  E92F

	ROM_LOAD32_BYTE( "open_ice_l1.u125",   0x0800000, 0x100000, CRC(a7b54550) SHA1(83e3627c4e84466ec10023b0e2259ad86b791fd7) ) // 6M.0  U125  OPEN ICE HOCKEY  7/11/95  23F8
	ROM_LOAD32_BYTE( "open_ice_l1.u124",   0x0800001, 0x100000, CRC(7c02cb50) SHA1(92d24bcfd66396c52c823b816118eed39c4ef9cd) ) // 6M.1  U124  OPEN ICE HOCKEY  7/11/95  A90C
	ROM_LOAD32_BYTE( "open_ice_l1.u123",   0x0800002, 0x100000, CRC(d543bd9d) SHA1(a9ff8589fe185ea058b549c2ed4e71f6c50e9638) ) // 6M.2  U123  OPEN ICE HOCKEY  7/11/95  EA1C
	ROM_LOAD32_BYTE( "open_ice_l1.u122",   0x0800003, 0x100000, CRC(3744d291) SHA1(e4484f377a66c4c64b015ef461419d956b6e23e4) ) // 6M.3  U122  OPEN ICE HOCKEY  7/11/95  AA6B

	ROM_LOAD32_BYTE( "open_ice_l1.2.u121", 0x0c00000, 0x100000, CRC(acd2f7c7) SHA1(82d6f09e63a825b118c36d668427011cd8892eaa) ) // 8M.0  U121  OPEN ICE HOCKEY  9/14/95  0A39
	ROM_LOAD32_BYTE( "open_ice_l1.2.u120", 0x0c00001, 0x100000, CRC(4295686a) SHA1(2522e57335bb8cca6d76942d2fd62560f88e37a6) ) // 8M.1  U120  OPEN ICE HOCKEY  9/14/95  64AD
	ROM_LOAD32_BYTE( "open_ice_l1.2.u119", 0x0c00002, 0x100000, CRC(948b9b27) SHA1(62d031410f491d557e27ba055d3db9d36d5a153c) ) // 8M.2  U119  OPEN ICE HOCKEY  9/14/95  3446
	ROM_LOAD32_BYTE( "open_ice_l1.2.u118", 0x0c00003, 0x100000, CRC(9eaaf93e) SHA1(56bd881df5282f659ac68ace960a3b085c13dd9d) ) // 8M.3  U118  OPEN ICE HOCKEY  9/14/95  3FD9
ROM_END


ROM_START( nbahangt )
	ROM_REGION16_LE( 0x800000, "dcs", ROMREGION_ERASEFF )   // sound data
	ROM_LOAD16_BYTE( "l1.0_nba_hangtime_u_2_music_spch.u2", 0x000000, 0x100000, CRC(3f0b0d0a) SHA1(e3b8a264686ce7359d86e4926237d8cf17612991) ) // Labeled: L1.0  NBA HANGTIME  U 2 MUSIC/SPCH
	ROM_LOAD16_BYTE( "l1.0_nba_hangtime_u_3_music_spch.u3", 0x200000, 0x100000, CRC(ec1db988) SHA1(1cf06d0b75f20ded7db648070e85c056043765bb) )
	ROM_LOAD16_BYTE( "l1.0_nba_hangtime_u_4_music_spch.u4", 0x400000, 0x100000, CRC(c7f847a3) SHA1(c50175dffa3563ccd5792c59a6b44523f4014544) )
	ROM_LOAD16_BYTE( "l1.0_nba_hangtime_u_5_music_spch.u5", 0x600000, 0x100000, CRC(ef19316a) SHA1(d41ae87ab45630a37c73684de42f7f6e0ed8f13b) )

	ROM_REGION16_LE( 0x100000, "maincpu", 0 )   // 34010 code
	ROM_LOAD16_BYTE( "l1.3_nba_hangtime_u_54_game_rom.u54", 0x00000, 0x80000, CRC(fd9ccca2) SHA1(fc38d2440dd0712d7d5e2d2cca9635efd63a3d85) )
	ROM_LOAD16_BYTE( "l1.3_nba_hangtime_u_63_game_rom.u63", 0x00001, 0x80000, CRC(57de886f) SHA1(7cc127c7db7a68ea716914f7ddbbaf1356937f97) )

	ROM_REGION( 0x2000000, "video", 0 )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_133_image_rom.u133", 0x0000000, 0x100000, CRC(3163feed) SHA1(eb7f128de306933929a0933e36e57760459cb0a1) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_132_image_rom.u132", 0x0000001, 0x100000, CRC(428eaf44) SHA1(2897efef4ab1653870b5bebb2762ea85549da03a) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_131_image_rom.u131", 0x0000002, 0x100000, CRC(5f7c5111) SHA1(14337f50b7b98254b54250af00f8a4a46bd7ee8d) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_130_image_rom.u130", 0x0000003, 0x100000, CRC(c7c0c514) SHA1(49788ff885996d9c5909c0ecebe06b6abd4298ed) )

	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_129_image_rom.u129", 0x0400000, 0x100000, CRC(b3d0daa0) SHA1(302208c30f2b0c4aead8cf9201ae4c9501f7f952) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_128_image_rom.u128", 0x0400001, 0x100000, CRC(3704ee69) SHA1(e57846e96380af480fd6851c5359f88d432ac7cc) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_127_image_rom.u127", 0x0400002, 0x100000, CRC(4ea64d5a) SHA1(e7054c3946898ab0c5b4c27244c2eb6b24eaced7) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_126_image_rom.u126", 0x0400003, 0x100000, CRC(0c5c19b7) SHA1(802a05f53fcc2827960a63ef5c32a884fc96aaee) )

	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_125_image_rom.u125", 0x0800000, 0x100000, CRC(46c43d67) SHA1(13a4d924fed51c2db7f750436b0b7c6ef03d36a5) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_124_image_rom.u124", 0x0800001, 0x100000, CRC(ed495156) SHA1(40f67fb2ccdd185b444f1127b1ec2fa4b493d7d6) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_123_image_rom.u123", 0x0800002, 0x100000, CRC(b48aa5da) SHA1(d8ee259a63dd7d997f2b99b73d0f11a277ba961d) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_122_image_rom.u122", 0x0800003, 0x100000, CRC(b18cd181) SHA1(d86eb5c81b10ba112ed989ab581683ccb669c3b5) )

	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_121_image_rom.u121", 0x0c00000, 0x100000, CRC(5acb267a) SHA1(9c7e55991f795f0deae5f5cada3d5f49b7da578e) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_120_image_rom.u120", 0x0c00001, 0x100000, CRC(28e05f86) SHA1(ab642e7525b8fe55aab79597fcf84c4a8265463f) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_119_image_rom.u119", 0x0c00002, 0x100000, CRC(b4f604ea) SHA1(3997acfc856eead321e98584f7cb21953c95951a) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_118_image_rom.u118", 0x0c00003, 0x100000, CRC(a257b973) SHA1(31b4e3cf4e93be673d9b32dc85d7be0edcf6234c) )

	// U114 through U117 sockets not populated

	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_113_image_rom.u113", 0x1400000, 0x100000, CRC(d712a779) SHA1(ca0e25fbb570c28c9ac6674f35050152a9072a5b) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_112_image_rom.u112", 0x1400001, 0x100000, CRC(644e1bca) SHA1(447cc3b0a16aaff39b6cd095f1c255a91f235f7f) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_111_image_rom.u111", 0x1400002, 0x100000, CRC(10d3b768) SHA1(e22fcb02a17c78f963ee5d35a38a9ab14f3de450) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_110_image_rom.u110", 0x1400003, 0x100000, CRC(8575aeb2) SHA1(883acfc45416ab6e1ab77fc897638f89286bea7e) )
ROM_END


ROM_START( nbahangtm13 )
	ROM_REGION16_LE( 0x800000, "dcs", ROMREGION_ERASEFF )   // sound data
	ROM_LOAD16_BYTE( "l1.0_nba_hangtime_u_2_music_spch.u2", 0x000000, 0x100000, CRC(3f0b0d0a) SHA1(e3b8a264686ce7359d86e4926237d8cf17612991) ) // Labeled: L1.0  NBA HANGTIME  U 2 MUSIC/SPCH
	ROM_LOAD16_BYTE( "l1.0_nba_hangtime_u_3_music_spch.u3", 0x200000, 0x100000, CRC(ec1db988) SHA1(1cf06d0b75f20ded7db648070e85c056043765bb) )
	ROM_LOAD16_BYTE( "l1.0_nba_hangtime_u_4_music_spch.u4", 0x400000, 0x100000, CRC(c7f847a3) SHA1(c50175dffa3563ccd5792c59a6b44523f4014544) )
	ROM_LOAD16_BYTE( "l1.0_nba_hangtime_u_5_music_spch.u5", 0x600000, 0x100000, CRC(ef19316a) SHA1(d41ae87ab45630a37c73684de42f7f6e0ed8f13b) )

	ROM_REGION16_LE( 0x100000, "maincpu", 0 )   // 34010 code
	ROM_LOAD16_BYTE( "m1.3_nba_hangtime_u_54_game_rom.u54", 0x00000, 0x80000, CRC(3ee3a9f4) SHA1(e5c2ab23f03af5aa493fcc3250f6e9bf38040793) )
	ROM_LOAD16_BYTE( "m1.3_nba_hangtime_u_63_game_rom.u63", 0x00001, 0x80000, CRC(42e6aeca) SHA1(468ad4095ea54be77e59def04b78fd5fed0616e5) )

	ROM_REGION( 0x2000000, "video", 0 )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_133_image_rom.u133", 0x0000000, 0x100000, CRC(3163feed) SHA1(eb7f128de306933929a0933e36e57760459cb0a1) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_132_image_rom.u132", 0x0000001, 0x100000, CRC(428eaf44) SHA1(2897efef4ab1653870b5bebb2762ea85549da03a) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_131_image_rom.u131", 0x0000002, 0x100000, CRC(5f7c5111) SHA1(14337f50b7b98254b54250af00f8a4a46bd7ee8d) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_130_image_rom.u130", 0x0000003, 0x100000, CRC(c7c0c514) SHA1(49788ff885996d9c5909c0ecebe06b6abd4298ed) )

	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_129_image_rom.u129", 0x0400000, 0x100000, CRC(b3d0daa0) SHA1(302208c30f2b0c4aead8cf9201ae4c9501f7f952) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_128_image_rom.u128", 0x0400001, 0x100000, CRC(3704ee69) SHA1(e57846e96380af480fd6851c5359f88d432ac7cc) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_127_image_rom.u127", 0x0400002, 0x100000, CRC(4ea64d5a) SHA1(e7054c3946898ab0c5b4c27244c2eb6b24eaced7) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_126_image_rom.u126", 0x0400003, 0x100000, CRC(0c5c19b7) SHA1(802a05f53fcc2827960a63ef5c32a884fc96aaee) )

	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_125_image_rom.u125", 0x0800000, 0x100000, CRC(46c43d67) SHA1(13a4d924fed51c2db7f750436b0b7c6ef03d36a5) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_124_image_rom.u124", 0x0800001, 0x100000, CRC(ed495156) SHA1(40f67fb2ccdd185b444f1127b1ec2fa4b493d7d6) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_123_image_rom.u123", 0x0800002, 0x100000, CRC(b48aa5da) SHA1(d8ee259a63dd7d997f2b99b73d0f11a277ba961d) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_122_image_rom.u122", 0x0800003, 0x100000, CRC(b18cd181) SHA1(d86eb5c81b10ba112ed989ab581683ccb669c3b5) )

	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_121_image_rom.u121", 0x0c00000, 0x100000, CRC(5acb267a) SHA1(9c7e55991f795f0deae5f5cada3d5f49b7da578e) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_120_image_rom.u120", 0x0c00001, 0x100000, CRC(28e05f86) SHA1(ab642e7525b8fe55aab79597fcf84c4a8265463f) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_119_image_rom.u119", 0x0c00002, 0x100000, CRC(b4f604ea) SHA1(3997acfc856eead321e98584f7cb21953c95951a) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_118_image_rom.u118", 0x0c00003, 0x100000, CRC(a257b973) SHA1(31b4e3cf4e93be673d9b32dc85d7be0edcf6234c) )

	// U114 through U117 sockets not populated

	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_113_image_rom.u113", 0x1400000, 0x100000, CRC(d712a779) SHA1(ca0e25fbb570c28c9ac6674f35050152a9072a5b) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_112_image_rom.u112", 0x1400001, 0x100000, CRC(644e1bca) SHA1(447cc3b0a16aaff39b6cd095f1c255a91f235f7f) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_111_image_rom.u111", 0x1400002, 0x100000, CRC(10d3b768) SHA1(e22fcb02a17c78f963ee5d35a38a9ab14f3de450) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_110_image_rom.u110", 0x1400003, 0x100000, CRC(8575aeb2) SHA1(883acfc45416ab6e1ab77fc897638f89286bea7e) )
ROM_END


ROM_START( nbahangtl12 )
	ROM_REGION16_LE( 0x800000, "dcs", ROMREGION_ERASEFF )   // sound data
	ROM_LOAD16_BYTE( "l1.0_nba_hangtime_u_2_music_spch.u2", 0x000000, 0x100000, CRC(3f0b0d0a) SHA1(e3b8a264686ce7359d86e4926237d8cf17612991) ) // Labeled: L1.0  NBA HANGTIME  U 2 MUSIC/SPCH
	ROM_LOAD16_BYTE( "l1.0_nba_hangtime_u_3_music_spch.u3", 0x200000, 0x100000, CRC(ec1db988) SHA1(1cf06d0b75f20ded7db648070e85c056043765bb) )
	ROM_LOAD16_BYTE( "l1.0_nba_hangtime_u_4_music_spch.u4", 0x400000, 0x100000, CRC(c7f847a3) SHA1(c50175dffa3563ccd5792c59a6b44523f4014544) )
	ROM_LOAD16_BYTE( "l1.0_nba_hangtime_u_5_music_spch.u5", 0x600000, 0x100000, CRC(ef19316a) SHA1(d41ae87ab45630a37c73684de42f7f6e0ed8f13b) )

	ROM_REGION16_LE( 0x100000, "maincpu", 0 )   // 34010 code
	ROM_LOAD16_BYTE( "l1.2_nba_hangtime_u_54_game_rom.u54", 0x00000, 0x80000, CRC(c90dc3cd) SHA1(62d74e3f9ca290c2cdf0fdc7dbcd7f4004454d46) )
	ROM_LOAD16_BYTE( "l1.2_nba_hangtime_u_63_game_rom.u63", 0x00001, 0x80000, CRC(1883c461) SHA1(6e72b4d55041cc8d50f2591013b75dd75aa8a9dd) )

	ROM_REGION( 0x2000000, "video", 0 )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_133_image_rom.u133", 0x0000000, 0x100000, CRC(3163feed) SHA1(eb7f128de306933929a0933e36e57760459cb0a1) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_132_image_rom.u132", 0x0000001, 0x100000, CRC(428eaf44) SHA1(2897efef4ab1653870b5bebb2762ea85549da03a) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_131_image_rom.u131", 0x0000002, 0x100000, CRC(5f7c5111) SHA1(14337f50b7b98254b54250af00f8a4a46bd7ee8d) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_130_image_rom.u130", 0x0000003, 0x100000, CRC(c7c0c514) SHA1(49788ff885996d9c5909c0ecebe06b6abd4298ed) )

	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_129_image_rom.u129", 0x0400000, 0x100000, CRC(b3d0daa0) SHA1(302208c30f2b0c4aead8cf9201ae4c9501f7f952) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_128_image_rom.u128", 0x0400001, 0x100000, CRC(3704ee69) SHA1(e57846e96380af480fd6851c5359f88d432ac7cc) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_127_image_rom.u127", 0x0400002, 0x100000, CRC(4ea64d5a) SHA1(e7054c3946898ab0c5b4c27244c2eb6b24eaced7) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_126_image_rom.u126", 0x0400003, 0x100000, CRC(0c5c19b7) SHA1(802a05f53fcc2827960a63ef5c32a884fc96aaee) )

	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_125_image_rom.u125", 0x0800000, 0x100000, CRC(46c43d67) SHA1(13a4d924fed51c2db7f750436b0b7c6ef03d36a5) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_124_image_rom.u124", 0x0800001, 0x100000, CRC(ed495156) SHA1(40f67fb2ccdd185b444f1127b1ec2fa4b493d7d6) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_123_image_rom.u123", 0x0800002, 0x100000, CRC(b48aa5da) SHA1(d8ee259a63dd7d997f2b99b73d0f11a277ba961d) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_122_image_rom.u122", 0x0800003, 0x100000, CRC(b18cd181) SHA1(d86eb5c81b10ba112ed989ab581683ccb669c3b5) )

	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_121_image_rom.u121", 0x0c00000, 0x100000, CRC(5acb267a) SHA1(9c7e55991f795f0deae5f5cada3d5f49b7da578e) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_120_image_rom.u120", 0x0c00001, 0x100000, CRC(28e05f86) SHA1(ab642e7525b8fe55aab79597fcf84c4a8265463f) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_119_image_rom.u119", 0x0c00002, 0x100000, CRC(b4f604ea) SHA1(3997acfc856eead321e98584f7cb21953c95951a) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_118_image_rom.u118", 0x0c00003, 0x100000, CRC(a257b973) SHA1(31b4e3cf4e93be673d9b32dc85d7be0edcf6234c) )

	// U114 through U117 sockets not populated

	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_113_image_rom.u113", 0x1400000, 0x100000, CRC(d712a779) SHA1(ca0e25fbb570c28c9ac6674f35050152a9072a5b) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_112_image_rom.u112", 0x1400001, 0x100000, CRC(644e1bca) SHA1(447cc3b0a16aaff39b6cd095f1c255a91f235f7f) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_111_image_rom.u111", 0x1400002, 0x100000, CRC(10d3b768) SHA1(e22fcb02a17c78f963ee5d35a38a9ab14f3de450) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_110_image_rom.u110", 0x1400003, 0x100000, CRC(8575aeb2) SHA1(883acfc45416ab6e1ab77fc897638f89286bea7e) )
ROM_END


ROM_START( nbahangtm12 )
	ROM_REGION16_LE( 0x800000, "dcs", ROMREGION_ERASEFF )   // sound data
	ROM_LOAD16_BYTE( "l1.0_nba_hangtime_u_2_music_spch.u2", 0x000000, 0x100000, CRC(3f0b0d0a) SHA1(e3b8a264686ce7359d86e4926237d8cf17612991) ) // Labeled: L1.0  NBA HANGTIME  U 2 MUSIC/SPCH
	ROM_LOAD16_BYTE( "l1.0_nba_hangtime_u_3_music_spch.u3", 0x200000, 0x100000, CRC(ec1db988) SHA1(1cf06d0b75f20ded7db648070e85c056043765bb) )
	ROM_LOAD16_BYTE( "l1.0_nba_hangtime_u_4_music_spch.u4", 0x400000, 0x100000, CRC(c7f847a3) SHA1(c50175dffa3563ccd5792c59a6b44523f4014544) )
	ROM_LOAD16_BYTE( "l1.0_nba_hangtime_u_5_music_spch.u5", 0x600000, 0x100000, CRC(ef19316a) SHA1(d41ae87ab45630a37c73684de42f7f6e0ed8f13b) )

	ROM_REGION16_LE( 0x100000, "maincpu", 0 )   // 34010 code
	ROM_LOAD16_BYTE( "m1.2_nba_hangtime_u_54_game_rom.u54", 0x00000, 0x80000, CRC(3be47f64) SHA1(71b54037b89c11c031c1db0e3112ae08f7f28e8c) )
	ROM_LOAD16_BYTE( "m1.2_nba_hangtime_u_63_game_rom.u63", 0x00001, 0x80000, CRC(ba4344ae) SHA1(86557a21411c18136ac4383cc7e0da78b6f01235) )

	ROM_REGION( 0x2000000, "video", 0 )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_133_image_rom.u133", 0x0000000, 0x100000, CRC(3163feed) SHA1(eb7f128de306933929a0933e36e57760459cb0a1) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_132_image_rom.u132", 0x0000001, 0x100000, CRC(428eaf44) SHA1(2897efef4ab1653870b5bebb2762ea85549da03a) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_131_image_rom.u131", 0x0000002, 0x100000, CRC(5f7c5111) SHA1(14337f50b7b98254b54250af00f8a4a46bd7ee8d) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_130_image_rom.u130", 0x0000003, 0x100000, CRC(c7c0c514) SHA1(49788ff885996d9c5909c0ecebe06b6abd4298ed) )

	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_129_image_rom.u129", 0x0400000, 0x100000, CRC(b3d0daa0) SHA1(302208c30f2b0c4aead8cf9201ae4c9501f7f952) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_128_image_rom.u128", 0x0400001, 0x100000, CRC(3704ee69) SHA1(e57846e96380af480fd6851c5359f88d432ac7cc) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_127_image_rom.u127", 0x0400002, 0x100000, CRC(4ea64d5a) SHA1(e7054c3946898ab0c5b4c27244c2eb6b24eaced7) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_126_image_rom.u126", 0x0400003, 0x100000, CRC(0c5c19b7) SHA1(802a05f53fcc2827960a63ef5c32a884fc96aaee) )

	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_125_image_rom.u125", 0x0800000, 0x100000, CRC(46c43d67) SHA1(13a4d924fed51c2db7f750436b0b7c6ef03d36a5) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_124_image_rom.u124", 0x0800001, 0x100000, CRC(ed495156) SHA1(40f67fb2ccdd185b444f1127b1ec2fa4b493d7d6) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_123_image_rom.u123", 0x0800002, 0x100000, CRC(b48aa5da) SHA1(d8ee259a63dd7d997f2b99b73d0f11a277ba961d) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_122_image_rom.u122", 0x0800003, 0x100000, CRC(b18cd181) SHA1(d86eb5c81b10ba112ed989ab581683ccb669c3b5) )

	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_121_image_rom.u121", 0x0c00000, 0x100000, CRC(5acb267a) SHA1(9c7e55991f795f0deae5f5cada3d5f49b7da578e) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_120_image_rom.u120", 0x0c00001, 0x100000, CRC(28e05f86) SHA1(ab642e7525b8fe55aab79597fcf84c4a8265463f) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_119_image_rom.u119", 0x0c00002, 0x100000, CRC(b4f604ea) SHA1(3997acfc856eead321e98584f7cb21953c95951a) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_118_image_rom.u118", 0x0c00003, 0x100000, CRC(a257b973) SHA1(31b4e3cf4e93be673d9b32dc85d7be0edcf6234c) )

	// U114 through U117 sockets not populated

	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_113_image_rom.u113", 0x1400000, 0x100000, CRC(d712a779) SHA1(ca0e25fbb570c28c9ac6674f35050152a9072a5b) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_112_image_rom.u112", 0x1400001, 0x100000, CRC(644e1bca) SHA1(447cc3b0a16aaff39b6cd095f1c255a91f235f7f) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_111_image_rom.u111", 0x1400002, 0x100000, CRC(10d3b768) SHA1(e22fcb02a17c78f963ee5d35a38a9ab14f3de450) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_110_image_rom.u110", 0x1400003, 0x100000, CRC(8575aeb2) SHA1(883acfc45416ab6e1ab77fc897638f89286bea7e) )
ROM_END


ROM_START( nbahangtl11 )
	ROM_REGION16_LE( 0x800000, "dcs", ROMREGION_ERASEFF )   // sound data
	ROM_LOAD16_BYTE( "l1.0_nba_hangtime_u_2_music_spch.u2", 0x000000, 0x100000, CRC(3f0b0d0a) SHA1(e3b8a264686ce7359d86e4926237d8cf17612991) ) // Labeled: L1.0  NBA HANGTIME  U 2 MUSIC/SPCH
	ROM_LOAD16_BYTE( "l1.0_nba_hangtime_u_3_music_spch.u3", 0x200000, 0x100000, CRC(ec1db988) SHA1(1cf06d0b75f20ded7db648070e85c056043765bb) )
	ROM_LOAD16_BYTE( "l1.0_nba_hangtime_u_4_music_spch.u4", 0x400000, 0x100000, CRC(c7f847a3) SHA1(c50175dffa3563ccd5792c59a6b44523f4014544) )
	ROM_LOAD16_BYTE( "l1.0_nba_hangtime_u_5_music_spch.u5", 0x600000, 0x100000, CRC(ef19316a) SHA1(d41ae87ab45630a37c73684de42f7f6e0ed8f13b) )

	ROM_REGION16_LE( 0x100000, "maincpu", 0 )   // 34010 code
	ROM_LOAD16_BYTE( "l1.1_nba_hangtime_u_54_game_rom.u54", 0x00000, 0x80000, CRC(c2875d98) SHA1(3f88f6f5c15ae03bedda39f71a1deaf549a55516) )
	ROM_LOAD16_BYTE( "l1.1_nba_hangtime_u_63_game_rom.u63", 0x00001, 0x80000, CRC(6f4728c3) SHA1(c059f4aa72cc5c3edc41e72428b3ebba97cc9417) )

	ROM_REGION( 0x2000000, "video", 0 )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_133_image_rom.u133", 0x0000000, 0x100000, CRC(3163feed) SHA1(eb7f128de306933929a0933e36e57760459cb0a1) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_132_image_rom.u132", 0x0000001, 0x100000, CRC(428eaf44) SHA1(2897efef4ab1653870b5bebb2762ea85549da03a) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_131_image_rom.u131", 0x0000002, 0x100000, CRC(5f7c5111) SHA1(14337f50b7b98254b54250af00f8a4a46bd7ee8d) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_130_image_rom.u130", 0x0000003, 0x100000, CRC(c7c0c514) SHA1(49788ff885996d9c5909c0ecebe06b6abd4298ed) )

	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_129_image_rom.u129", 0x0400000, 0x100000, CRC(b3d0daa0) SHA1(302208c30f2b0c4aead8cf9201ae4c9501f7f952) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_128_image_rom.u128", 0x0400001, 0x100000, CRC(3704ee69) SHA1(e57846e96380af480fd6851c5359f88d432ac7cc) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_127_image_rom.u127", 0x0400002, 0x100000, CRC(4ea64d5a) SHA1(e7054c3946898ab0c5b4c27244c2eb6b24eaced7) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_126_image_rom.u126", 0x0400003, 0x100000, CRC(0c5c19b7) SHA1(802a05f53fcc2827960a63ef5c32a884fc96aaee) )

	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_125_image_rom.u125", 0x0800000, 0x100000, CRC(46c43d67) SHA1(13a4d924fed51c2db7f750436b0b7c6ef03d36a5) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_124_image_rom.u124", 0x0800001, 0x100000, CRC(ed495156) SHA1(40f67fb2ccdd185b444f1127b1ec2fa4b493d7d6) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_123_image_rom.u123", 0x0800002, 0x100000, CRC(b48aa5da) SHA1(d8ee259a63dd7d997f2b99b73d0f11a277ba961d) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_122_image_rom.u122", 0x0800003, 0x100000, CRC(b18cd181) SHA1(d86eb5c81b10ba112ed989ab581683ccb669c3b5) )

	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_121_image_rom.u121", 0x0c00000, 0x100000, CRC(5acb267a) SHA1(9c7e55991f795f0deae5f5cada3d5f49b7da578e) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_120_image_rom.u120", 0x0c00001, 0x100000, CRC(28e05f86) SHA1(ab642e7525b8fe55aab79597fcf84c4a8265463f) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_119_image_rom.u119", 0x0c00002, 0x100000, CRC(b4f604ea) SHA1(3997acfc856eead321e98584f7cb21953c95951a) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_118_image_rom.u118", 0x0c00003, 0x100000, CRC(a257b973) SHA1(31b4e3cf4e93be673d9b32dc85d7be0edcf6234c) )

	// U114 through U117 sockets not populated

	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_113_image_rom.u113", 0x1400000, 0x100000, CRC(d712a779) SHA1(ca0e25fbb570c28c9ac6674f35050152a9072a5b) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_112_image_rom.u112", 0x1400001, 0x100000, CRC(644e1bca) SHA1(447cc3b0a16aaff39b6cd095f1c255a91f235f7f) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_111_image_rom.u111", 0x1400002, 0x100000, CRC(10d3b768) SHA1(e22fcb02a17c78f963ee5d35a38a9ab14f3de450) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_110_image_rom.u110", 0x1400003, 0x100000, CRC(8575aeb2) SHA1(883acfc45416ab6e1ab77fc897638f89286bea7e) )
ROM_END


ROM_START( nbahangtm11 )
	ROM_REGION16_LE( 0x800000, "dcs", ROMREGION_ERASEFF )   // sound data
	ROM_LOAD16_BYTE( "l1.0_nba_hangtime_u_2_music_spch.u2", 0x000000, 0x100000, CRC(3f0b0d0a) SHA1(e3b8a264686ce7359d86e4926237d8cf17612991) ) // Labeled: L1.0  NBA HANGTIME  U 2 MUSIC/SPCH
	ROM_LOAD16_BYTE( "l1.0_nba_hangtime_u_3_music_spch.u3", 0x200000, 0x100000, CRC(ec1db988) SHA1(1cf06d0b75f20ded7db648070e85c056043765bb) )
	ROM_LOAD16_BYTE( "l1.0_nba_hangtime_u_4_music_spch.u4", 0x400000, 0x100000, CRC(c7f847a3) SHA1(c50175dffa3563ccd5792c59a6b44523f4014544) )
	ROM_LOAD16_BYTE( "l1.0_nba_hangtime_u_5_music_spch.u5", 0x600000, 0x100000, CRC(ef19316a) SHA1(d41ae87ab45630a37c73684de42f7f6e0ed8f13b) )

	ROM_REGION16_LE( 0x100000, "maincpu", 0 )   // 34010 code
	ROM_LOAD16_BYTE( "m1.1_nba_hangtime_u_54_game_rom.u54", 0x00000, 0x80000, CRC(113b37f4) SHA1(61fac820a6f6bf9ca74a52d7d4f718e08fc58a36) )
	ROM_LOAD16_BYTE( "m1.1_nba_hangtime_u_63_game_rom.u63", 0x00001, 0x80000, CRC(beaa3e92) SHA1(86b2c8278f200fea3df3f4b9e5ceea37cb0e6191) )

	ROM_REGION( 0x2000000, "video", 0 )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_133_image_rom.u133", 0x0000000, 0x100000, CRC(3163feed) SHA1(eb7f128de306933929a0933e36e57760459cb0a1) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_132_image_rom.u132", 0x0000001, 0x100000, CRC(428eaf44) SHA1(2897efef4ab1653870b5bebb2762ea85549da03a) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_131_image_rom.u131", 0x0000002, 0x100000, CRC(5f7c5111) SHA1(14337f50b7b98254b54250af00f8a4a46bd7ee8d) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_130_image_rom.u130", 0x0000003, 0x100000, CRC(c7c0c514) SHA1(49788ff885996d9c5909c0ecebe06b6abd4298ed) )

	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_129_image_rom.u129", 0x0400000, 0x100000, CRC(b3d0daa0) SHA1(302208c30f2b0c4aead8cf9201ae4c9501f7f952) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_128_image_rom.u128", 0x0400001, 0x100000, CRC(3704ee69) SHA1(e57846e96380af480fd6851c5359f88d432ac7cc) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_127_image_rom.u127", 0x0400002, 0x100000, CRC(4ea64d5a) SHA1(e7054c3946898ab0c5b4c27244c2eb6b24eaced7) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_126_image_rom.u126", 0x0400003, 0x100000, CRC(0c5c19b7) SHA1(802a05f53fcc2827960a63ef5c32a884fc96aaee) )

	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_125_image_rom.u125", 0x0800000, 0x100000, CRC(46c43d67) SHA1(13a4d924fed51c2db7f750436b0b7c6ef03d36a5) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_124_image_rom.u124", 0x0800001, 0x100000, CRC(ed495156) SHA1(40f67fb2ccdd185b444f1127b1ec2fa4b493d7d6) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_123_image_rom.u123", 0x0800002, 0x100000, CRC(b48aa5da) SHA1(d8ee259a63dd7d997f2b99b73d0f11a277ba961d) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_122_image_rom.u122", 0x0800003, 0x100000, CRC(b18cd181) SHA1(d86eb5c81b10ba112ed989ab581683ccb669c3b5) )

	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_121_image_rom.u121", 0x0c00000, 0x100000, CRC(5acb267a) SHA1(9c7e55991f795f0deae5f5cada3d5f49b7da578e) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_120_image_rom.u120", 0x0c00001, 0x100000, CRC(28e05f86) SHA1(ab642e7525b8fe55aab79597fcf84c4a8265463f) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_119_image_rom.u119", 0x0c00002, 0x100000, CRC(b4f604ea) SHA1(3997acfc856eead321e98584f7cb21953c95951a) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_118_image_rom.u118", 0x0c00003, 0x100000, CRC(a257b973) SHA1(31b4e3cf4e93be673d9b32dc85d7be0edcf6234c) )

	// U114 through U117 sockets not populated

	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_113_image_rom.u113", 0x1400000, 0x100000, CRC(d712a779) SHA1(ca0e25fbb570c28c9ac6674f35050152a9072a5b) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_112_image_rom.u112", 0x1400001, 0x100000, CRC(644e1bca) SHA1(447cc3b0a16aaff39b6cd095f1c255a91f235f7f) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_111_image_rom.u111", 0x1400002, 0x100000, CRC(10d3b768) SHA1(e22fcb02a17c78f963ee5d35a38a9ab14f3de450) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_110_image_rom.u110", 0x1400003, 0x100000, CRC(8575aeb2) SHA1(883acfc45416ab6e1ab77fc897638f89286bea7e) )
ROM_END


/*
The NBA Maximum Hangtime sets are a program ROM update for NBA Hangtime.  The MAX sets use all the same Music/SPCH ROMs,
  Image ROMs and even the same security PIC chip.

The new MAX sets boast a 1997 player line up and during the atrack mode state: "All NBA team Rosters accurate as of 11/1/96"

There are known "M" versions (EX: MAX HANGTIME - VER M1.0 11/08/96 ), but it's not known what the differences are between
  those and a standard "L" version.  In fact the ROM labels specifically state they are "L" Version ROMs
*/
ROM_START( nbamht )
	ROM_REGION16_LE( 0x800000, "dcs", ROMREGION_ERASEFF )   // sound data
	ROM_LOAD16_BYTE( "l1.0_nba_hangtime_u_2_music_spch.u2", 0x000000, 0x100000, CRC(3f0b0d0a) SHA1(e3b8a264686ce7359d86e4926237d8cf17612991) ) // Uses NBA Hangtime MUSIC/SPCH ROMs - verified correct
	ROM_LOAD16_BYTE( "l1.0_nba_hangtime_u_3_music_spch.u3", 0x200000, 0x100000, CRC(ec1db988) SHA1(1cf06d0b75f20ded7db648070e85c056043765bb) )
	ROM_LOAD16_BYTE( "l1.0_nba_hangtime_u_4_music_spch.u4", 0x400000, 0x100000, CRC(c7f847a3) SHA1(c50175dffa3563ccd5792c59a6b44523f4014544) )
	ROM_LOAD16_BYTE( "l1.0_nba_hangtime_u_5_music_spch.u5", 0x600000, 0x100000, CRC(ef19316a) SHA1(d41ae87ab45630a37c73684de42f7f6e0ed8f13b) )

	ROM_REGION16_LE( 0x100000, "maincpu", 0 )   // 34010 code
	ROM_LOAD16_BYTE( "l1.03_maximum_hangtime_u54_l_version.u54",  0x00000, 0x80000, CRC(21b0d9e1) SHA1(34fa928bdb222fba1fec2a9f37b853f77922250f) ) // Labeled: L1.03  Maximum Hangtime  U54 "L" Version
	ROM_LOAD16_BYTE( "l1.03_maximum_hangtime_u63_l_version.u63",  0x00001, 0x80000, CRC(c6fdbb97) SHA1(e6cf0c6a94441befdde40b620a182877c11582a5) ) // Labeled: L1.03  Maximum Hangtime  U63 "L" Version

	ROM_REGION( 0x2000000, "video", 0 )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_133_image_rom.u133", 0x0000000, 0x100000, CRC(3163feed) SHA1(eb7f128de306933929a0933e36e57760459cb0a1) ) // Uses NBA Hangtime IMAGE ROMs - verified correct
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_132_image_rom.u132", 0x0000001, 0x100000, CRC(428eaf44) SHA1(2897efef4ab1653870b5bebb2762ea85549da03a) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_131_image_rom.u131", 0x0000002, 0x100000, CRC(5f7c5111) SHA1(14337f50b7b98254b54250af00f8a4a46bd7ee8d) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_130_image_rom.u130", 0x0000003, 0x100000, CRC(c7c0c514) SHA1(49788ff885996d9c5909c0ecebe06b6abd4298ed) )

	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_129_image_rom.u129", 0x0400000, 0x100000, CRC(b3d0daa0) SHA1(302208c30f2b0c4aead8cf9201ae4c9501f7f952) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_128_image_rom.u128", 0x0400001, 0x100000, CRC(3704ee69) SHA1(e57846e96380af480fd6851c5359f88d432ac7cc) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_127_image_rom.u127", 0x0400002, 0x100000, CRC(4ea64d5a) SHA1(e7054c3946898ab0c5b4c27244c2eb6b24eaced7) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_126_image_rom.u126", 0x0400003, 0x100000, CRC(0c5c19b7) SHA1(802a05f53fcc2827960a63ef5c32a884fc96aaee) )

	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_125_image_rom.u125", 0x0800000, 0x100000, CRC(46c43d67) SHA1(13a4d924fed51c2db7f750436b0b7c6ef03d36a5) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_124_image_rom.u124", 0x0800001, 0x100000, CRC(ed495156) SHA1(40f67fb2ccdd185b444f1127b1ec2fa4b493d7d6) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_123_image_rom.u123", 0x0800002, 0x100000, CRC(b48aa5da) SHA1(d8ee259a63dd7d997f2b99b73d0f11a277ba961d) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_122_image_rom.u122", 0x0800003, 0x100000, CRC(b18cd181) SHA1(d86eb5c81b10ba112ed989ab581683ccb669c3b5) )

	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_121_image_rom.u121", 0x0c00000, 0x100000, CRC(5acb267a) SHA1(9c7e55991f795f0deae5f5cada3d5f49b7da578e) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_120_image_rom.u120", 0x0c00001, 0x100000, CRC(28e05f86) SHA1(ab642e7525b8fe55aab79597fcf84c4a8265463f) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_119_image_rom.u119", 0x0c00002, 0x100000, CRC(b4f604ea) SHA1(3997acfc856eead321e98584f7cb21953c95951a) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_118_image_rom.u118", 0x0c00003, 0x100000, CRC(a257b973) SHA1(31b4e3cf4e93be673d9b32dc85d7be0edcf6234c) )

	// U114 through U117 sockets not populated

	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_113_image_rom.u113", 0x1400000, 0x100000, CRC(d712a779) SHA1(ca0e25fbb570c28c9ac6674f35050152a9072a5b) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_112_image_rom.u112", 0x1400001, 0x100000, CRC(644e1bca) SHA1(447cc3b0a16aaff39b6cd095f1c255a91f235f7f) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_111_image_rom.u111", 0x1400002, 0x100000, CRC(10d3b768) SHA1(e22fcb02a17c78f963ee5d35a38a9ab14f3de450) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_110_image_rom.u110", 0x1400003, 0x100000, CRC(8575aeb2) SHA1(883acfc45416ab6e1ab77fc897638f89286bea7e) )
ROM_END


ROM_START( nbamhtl10 )
	ROM_REGION16_LE( 0x800000, "dcs", ROMREGION_ERASEFF )   // sound data
	ROM_LOAD16_BYTE( "l1.0_nba_hangtime_u_2_music_spch.u2", 0x000000, 0x100000, CRC(3f0b0d0a) SHA1(e3b8a264686ce7359d86e4926237d8cf17612991) ) // Uses NBA Hangtime MUSIC/SPCH ROMs - verified correct
	ROM_LOAD16_BYTE( "l1.0_nba_hangtime_u_3_music_spch.u3", 0x200000, 0x100000, CRC(ec1db988) SHA1(1cf06d0b75f20ded7db648070e85c056043765bb) )
	ROM_LOAD16_BYTE( "l1.0_nba_hangtime_u_4_music_spch.u4", 0x400000, 0x100000, CRC(c7f847a3) SHA1(c50175dffa3563ccd5792c59a6b44523f4014544) )
	ROM_LOAD16_BYTE( "l1.0_nba_hangtime_u_5_music_spch.u5", 0x600000, 0x100000, CRC(ef19316a) SHA1(d41ae87ab45630a37c73684de42f7f6e0ed8f13b) )

	ROM_REGION16_LE( 0x100000, "maincpu", 0 )   // 34010 code
	ROM_LOAD16_BYTE( "l1.0_maximum_hangtime_u54_l_version.u54", 0x00000, 0x80000, CRC(dfb6b3ae) SHA1(1dc59a2d89bf9764a47cebf71b9657c6ae7ce959) ) // Labeled: L1.0  Maximum Hangtime  U54 "L" Version
	ROM_LOAD16_BYTE( "l1.0_maximum_hangtime_u63_l_version.u63", 0x00001, 0x80000, CRC(78da472c) SHA1(b4573ff19dc0d8a99f1bceace872e4999d53317a) ) // Labeled: L1.0  Maximum Hangtime  U63 "L" Version

	ROM_REGION( 0x2000000, "video", 0 )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_133_image_rom.u133", 0x0000000, 0x100000, CRC(3163feed) SHA1(eb7f128de306933929a0933e36e57760459cb0a1) ) // Uses NBA Hangtime IMAGE ROMs - verified correct
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_132_image_rom.u132", 0x0000001, 0x100000, CRC(428eaf44) SHA1(2897efef4ab1653870b5bebb2762ea85549da03a) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_131_image_rom.u131", 0x0000002, 0x100000, CRC(5f7c5111) SHA1(14337f50b7b98254b54250af00f8a4a46bd7ee8d) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_130_image_rom.u130", 0x0000003, 0x100000, CRC(c7c0c514) SHA1(49788ff885996d9c5909c0ecebe06b6abd4298ed) )

	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_129_image_rom.u129", 0x0400000, 0x100000, CRC(b3d0daa0) SHA1(302208c30f2b0c4aead8cf9201ae4c9501f7f952) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_128_image_rom.u128", 0x0400001, 0x100000, CRC(3704ee69) SHA1(e57846e96380af480fd6851c5359f88d432ac7cc) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_127_image_rom.u127", 0x0400002, 0x100000, CRC(4ea64d5a) SHA1(e7054c3946898ab0c5b4c27244c2eb6b24eaced7) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_126_image_rom.u126", 0x0400003, 0x100000, CRC(0c5c19b7) SHA1(802a05f53fcc2827960a63ef5c32a884fc96aaee) )

	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_125_image_rom.u125", 0x0800000, 0x100000, CRC(46c43d67) SHA1(13a4d924fed51c2db7f750436b0b7c6ef03d36a5) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_124_image_rom.u124", 0x0800001, 0x100000, CRC(ed495156) SHA1(40f67fb2ccdd185b444f1127b1ec2fa4b493d7d6) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_123_image_rom.u123", 0x0800002, 0x100000, CRC(b48aa5da) SHA1(d8ee259a63dd7d997f2b99b73d0f11a277ba961d) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_122_image_rom.u122", 0x0800003, 0x100000, CRC(b18cd181) SHA1(d86eb5c81b10ba112ed989ab581683ccb669c3b5) )

	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_121_image_rom.u121", 0x0c00000, 0x100000, CRC(5acb267a) SHA1(9c7e55991f795f0deae5f5cada3d5f49b7da578e) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_120_image_rom.u120", 0x0c00001, 0x100000, CRC(28e05f86) SHA1(ab642e7525b8fe55aab79597fcf84c4a8265463f) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_119_image_rom.u119", 0x0c00002, 0x100000, CRC(b4f604ea) SHA1(3997acfc856eead321e98584f7cb21953c95951a) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_118_image_rom.u118", 0x0c00003, 0x100000, CRC(a257b973) SHA1(31b4e3cf4e93be673d9b32dc85d7be0edcf6234c) )

	// U114 through U117 sockets not populated

	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_113_image_rom.u113", 0x1400000, 0x100000, CRC(d712a779) SHA1(ca0e25fbb570c28c9ac6674f35050152a9072a5b) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_112_image_rom.u112", 0x1400001, 0x100000, CRC(644e1bca) SHA1(447cc3b0a16aaff39b6cd095f1c255a91f235f7f) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_111_image_rom.u111", 0x1400002, 0x100000, CRC(10d3b768) SHA1(e22fcb02a17c78f963ee5d35a38a9ab14f3de450) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_110_image_rom.u110", 0x1400003, 0x100000, CRC(8575aeb2) SHA1(883acfc45416ab6e1ab77fc897638f89286bea7e) )
ROM_END


ROM_START( nbamhtm10 )
	ROM_REGION16_LE( 0x800000, "dcs", ROMREGION_ERASEFF )   // sound data
	ROM_LOAD16_BYTE( "l1.0_nba_hangtime_u_2_music_spch.u2", 0x000000, 0x100000, CRC(3f0b0d0a) SHA1(e3b8a264686ce7359d86e4926237d8cf17612991) ) // Uses NBA Hangtime MUSIC/SPCH ROMs - verified correct
	ROM_LOAD16_BYTE( "l1.0_nba_hangtime_u_3_music_spch.u3", 0x200000, 0x100000, CRC(ec1db988) SHA1(1cf06d0b75f20ded7db648070e85c056043765bb) )
	ROM_LOAD16_BYTE( "l1.0_nba_hangtime_u_4_music_spch.u4", 0x400000, 0x100000, CRC(c7f847a3) SHA1(c50175dffa3563ccd5792c59a6b44523f4014544) )
	ROM_LOAD16_BYTE( "l1.0_nba_hangtime_u_5_music_spch.u5", 0x600000, 0x100000, CRC(ef19316a) SHA1(d41ae87ab45630a37c73684de42f7f6e0ed8f13b) )

	ROM_REGION16_LE( 0x100000, "maincpu", 0 )   // 34010 code
	ROM_LOAD16_BYTE( "m1.0_maximum_hangtime_u54_m_version.u54", 0x00000, 0x80000, CRC(e4e665d5) SHA1(8111536e041f69ec35284bf3cae40a85a48d7331) ) // Labeled: L1.0  Maximum Hangtime  U54 "M" Version
	ROM_LOAD16_BYTE( "m1.0_maximum_hangtime_u63_m_version.u63", 0x00001, 0x80000, CRC(51cfda55) SHA1(e8c8326fd57af9916a7fb8159b1d0901f30fd331) ) // Labeled: L1.0  Maximum Hangtime  U63 "M" Version

	ROM_REGION( 0x2000000, "video", 0 )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_133_image_rom.u133", 0x0000000, 0x100000, CRC(3163feed) SHA1(eb7f128de306933929a0933e36e57760459cb0a1) ) // Uses NBA Hangtime IMAGE ROMs - verified correct
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_132_image_rom.u132", 0x0000001, 0x100000, CRC(428eaf44) SHA1(2897efef4ab1653870b5bebb2762ea85549da03a) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_131_image_rom.u131", 0x0000002, 0x100000, CRC(5f7c5111) SHA1(14337f50b7b98254b54250af00f8a4a46bd7ee8d) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_130_image_rom.u130", 0x0000003, 0x100000, CRC(c7c0c514) SHA1(49788ff885996d9c5909c0ecebe06b6abd4298ed) )

	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_129_image_rom.u129", 0x0400000, 0x100000, CRC(b3d0daa0) SHA1(302208c30f2b0c4aead8cf9201ae4c9501f7f952) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_128_image_rom.u128", 0x0400001, 0x100000, CRC(3704ee69) SHA1(e57846e96380af480fd6851c5359f88d432ac7cc) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_127_image_rom.u127", 0x0400002, 0x100000, CRC(4ea64d5a) SHA1(e7054c3946898ab0c5b4c27244c2eb6b24eaced7) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_126_image_rom.u126", 0x0400003, 0x100000, CRC(0c5c19b7) SHA1(802a05f53fcc2827960a63ef5c32a884fc96aaee) )

	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_125_image_rom.u125", 0x0800000, 0x100000, CRC(46c43d67) SHA1(13a4d924fed51c2db7f750436b0b7c6ef03d36a5) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_124_image_rom.u124", 0x0800001, 0x100000, CRC(ed495156) SHA1(40f67fb2ccdd185b444f1127b1ec2fa4b493d7d6) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_123_image_rom.u123", 0x0800002, 0x100000, CRC(b48aa5da) SHA1(d8ee259a63dd7d997f2b99b73d0f11a277ba961d) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_122_image_rom.u122", 0x0800003, 0x100000, CRC(b18cd181) SHA1(d86eb5c81b10ba112ed989ab581683ccb669c3b5) )

	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_121_image_rom.u121", 0x0c00000, 0x100000, CRC(5acb267a) SHA1(9c7e55991f795f0deae5f5cada3d5f49b7da578e) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_120_image_rom.u120", 0x0c00001, 0x100000, CRC(28e05f86) SHA1(ab642e7525b8fe55aab79597fcf84c4a8265463f) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_119_image_rom.u119", 0x0c00002, 0x100000, CRC(b4f604ea) SHA1(3997acfc856eead321e98584f7cb21953c95951a) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_118_image_rom.u118", 0x0c00003, 0x100000, CRC(a257b973) SHA1(31b4e3cf4e93be673d9b32dc85d7be0edcf6234c) )

	// U114 through U117 sockets not populated

	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_113_image_rom.u113", 0x1400000, 0x100000, CRC(d712a779) SHA1(ca0e25fbb570c28c9ac6674f35050152a9072a5b) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_112_image_rom.u112", 0x1400001, 0x100000, CRC(644e1bca) SHA1(447cc3b0a16aaff39b6cd095f1c255a91f235f7f) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_111_image_rom.u111", 0x1400002, 0x100000, CRC(10d3b768) SHA1(e22fcb02a17c78f963ee5d35a38a9ab14f3de450) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_110_image_rom.u110", 0x1400003, 0x100000, CRC(8575aeb2) SHA1(883acfc45416ab6e1ab77fc897638f89286bea7e) )
ROM_END


ROM_START( nbamhtp )
	ROM_REGION16_LE( 0x800000, "dcs", ROMREGION_ERASEFF )   // sound data
	ROM_LOAD16_BYTE( "l1.0_nba_hangtime_u_2_music_spch.u2", 0x000000, 0x100000, CRC(3f0b0d0a) SHA1(e3b8a264686ce7359d86e4926237d8cf17612991) ) // Uses NBA Hangtime MUSIC/SPCH ROMs - verified correct
	ROM_LOAD16_BYTE( "l1.0_nba_hangtime_u_3_music_spch.u3", 0x200000, 0x100000, CRC(ec1db988) SHA1(1cf06d0b75f20ded7db648070e85c056043765bb) )
	ROM_LOAD16_BYTE( "l1.0_nba_hangtime_u_4_music_spch.u4", 0x400000, 0x100000, CRC(c7f847a3) SHA1(c50175dffa3563ccd5792c59a6b44523f4014544) )
	ROM_LOAD16_BYTE( "l1.0_nba_hangtime_u_5_music_spch.u5", 0x600000, 0x100000, CRC(ef19316a) SHA1(d41ae87ab45630a37c73684de42f7f6e0ed8f13b) )

	ROM_REGION16_LE( 0x100000, "maincpu", 0 )   // 34010 code
	ROM_LOAD16_BYTE( "l0.9_maximum_hangtime_u54_l_version.u54", 0x00000, 0x80000, CRC(0fbed60e) SHA1(a017d498a901c1608ffecfe0fb2ec82c7a23f4ea) )
	ROM_LOAD16_BYTE( "l0.9_maximum_hangtime_u63_l_version.u63", 0x00001, 0x80000, CRC(a064645a) SHA1(43dba6f64ef1d940f1d1b1764addf40359fcdb51) )

	ROM_REGION( 0x2000000, "video", 0 )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_133_image_rom.u133", 0x0000000, 0x100000, CRC(3163feed) SHA1(eb7f128de306933929a0933e36e57760459cb0a1) ) // Uses NBA Hangtime IMAGE ROMs - verified correct
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_132_image_rom.u132", 0x0000001, 0x100000, CRC(428eaf44) SHA1(2897efef4ab1653870b5bebb2762ea85549da03a) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_131_image_rom.u131", 0x0000002, 0x100000, CRC(5f7c5111) SHA1(14337f50b7b98254b54250af00f8a4a46bd7ee8d) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_130_image_rom.u130", 0x0000003, 0x100000, CRC(c7c0c514) SHA1(49788ff885996d9c5909c0ecebe06b6abd4298ed) )

	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_129_image_rom.u129", 0x0400000, 0x100000, CRC(b3d0daa0) SHA1(302208c30f2b0c4aead8cf9201ae4c9501f7f952) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_128_image_rom.u128", 0x0400001, 0x100000, CRC(3704ee69) SHA1(e57846e96380af480fd6851c5359f88d432ac7cc) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_127_image_rom.u127", 0x0400002, 0x100000, CRC(4ea64d5a) SHA1(e7054c3946898ab0c5b4c27244c2eb6b24eaced7) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_126_image_rom.u126", 0x0400003, 0x100000, CRC(0c5c19b7) SHA1(802a05f53fcc2827960a63ef5c32a884fc96aaee) )

	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_125_image_rom.u125", 0x0800000, 0x100000, CRC(46c43d67) SHA1(13a4d924fed51c2db7f750436b0b7c6ef03d36a5) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_124_image_rom.u124", 0x0800001, 0x100000, CRC(ed495156) SHA1(40f67fb2ccdd185b444f1127b1ec2fa4b493d7d6) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_123_image_rom.u123", 0x0800002, 0x100000, CRC(b48aa5da) SHA1(d8ee259a63dd7d997f2b99b73d0f11a277ba961d) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_122_image_rom.u122", 0x0800003, 0x100000, CRC(b18cd181) SHA1(d86eb5c81b10ba112ed989ab581683ccb669c3b5) )

	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_121_image_rom.u121", 0x0c00000, 0x100000, CRC(5acb267a) SHA1(9c7e55991f795f0deae5f5cada3d5f49b7da578e) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_120_image_rom.u120", 0x0c00001, 0x100000, CRC(28e05f86) SHA1(ab642e7525b8fe55aab79597fcf84c4a8265463f) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_119_image_rom.u119", 0x0c00002, 0x100000, CRC(b4f604ea) SHA1(3997acfc856eead321e98584f7cb21953c95951a) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_118_image_rom.u118", 0x0c00003, 0x100000, CRC(a257b973) SHA1(31b4e3cf4e93be673d9b32dc85d7be0edcf6234c) )

	// U114 through U117 sockets not populated

	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_113_image_rom.u113", 0x1400000, 0x100000, CRC(d712a779) SHA1(ca0e25fbb570c28c9ac6674f35050152a9072a5b) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_112_image_rom.u112", 0x1400001, 0x100000, CRC(644e1bca) SHA1(447cc3b0a16aaff39b6cd095f1c255a91f235f7f) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_111_image_rom.u111", 0x1400002, 0x100000, CRC(10d3b768) SHA1(e22fcb02a17c78f963ee5d35a38a9ab14f3de450) )
	ROM_LOAD32_BYTE( "l1.0_nba_hangtime_u_110_image_rom.u110", 0x1400003, 0x100000, CRC(8575aeb2) SHA1(883acfc45416ab6e1ab77fc897638f89286bea7e) )
ROM_END


ROM_START( rmpgwt )
	ROM_REGION16_LE( 0x800000, "dcs", ROMREGION_ERASEFF )   // sound data
	ROM_LOAD16_BYTE( "1.0_rampage_world_tour_u2_sound.u2",  0x000000, 0x100000, CRC(0e82f83d) SHA1(215eebb6c229ef9ad0fcbcbc6e4e07300c05654f) )
	ROM_LOAD16_BYTE( "1.0_rampage_world_tour_u3_sound.u3",  0x200000, 0x100000, CRC(3ff54d15) SHA1(827805602091313ec68ea1bccf667bd3b3fc6b8b) )
	ROM_LOAD16_BYTE( "1.0_rampage_world_tour_u4_sound.u4",  0x400000, 0x100000, CRC(5c7f5656) SHA1(6c9d692bad539fec8b5aa0bfb56de3ef3719c68a) )
	ROM_LOAD16_BYTE( "1.0_rampage_world_tour_u5_sound.u5",  0x600000, 0x100000, CRC(fd9aaf24) SHA1(d60dc076e72618c99ecac9d081d8c49d337b90c7) )

	ROM_REGION16_LE( 0x100000, "maincpu", 0 )   // 34010 code
	ROM_LOAD16_BYTE( "1.3_rampage_world_u54_game.u54",  0x00000, 0x80000, CRC(2a8f6e1e) SHA1(7a87ad37fa1d1228c4cdd4704ff0aee42e9c86cb) )
	ROM_LOAD16_BYTE( "1.3_rampage_world_u63_game.u63",  0x00001, 0x80000, CRC(403ae41e) SHA1(c08d9352efe63849f5d10c1bd1efe2b9dd7382e0) )

	ROM_REGION( 0x1009, "serial_security:pic", 0 )   // security PIC (provides game ID code and serial number)
	ROM_LOAD( "465_rampage_wt.u64",  0x0000, 0x1009, CRC(5c14d850) SHA1(f57aef8350e477252bff1fa0f930c1b5d0ceb03f) )

	ROM_REGION( 0x2000000, "video", 0 )
	ROM_LOAD32_BYTE( "1.0_rampage_world_tour_u133_image.u133",  0x0000000, 0x100000, CRC(5b5ac449) SHA1(1c01dde9a9dbd9f4a6cd30aea9f6410cab13c2c9) )
	ROM_LOAD32_BYTE( "1.0_rampage_world_tour_u132_image.u132",  0x0000001, 0x100000, CRC(7b3f09c6) SHA1(477658481ee96d5ce462d5e198d80faff4d4352c) )
	ROM_LOAD32_BYTE( "1.0_rampage_world_tour_u131_image.u131",  0x0000002, 0x100000, CRC(fdecf12e) SHA1(bcbd29009dabed484e2357dc75c38c7d7bade251) )
	ROM_LOAD32_BYTE( "1.0_rampage_world_tour_u130_image.u130",  0x0000003, 0x100000, CRC(4a983b05) SHA1(022753e6fa3b3d74eff5b2ec835b07787f5473ab) )

	ROM_LOAD32_BYTE( "1.0_rampage_world_tour_u129_image.u129",  0x0400000, 0x100000, CRC(dc495c6e) SHA1(7ec80d293f06a013c4f95bfb3fafe5b4a71cf170) )
	ROM_LOAD32_BYTE( "1.0_rampage_world_tour_u128_image.u128",  0x0400001, 0x100000, CRC(5545503d) SHA1(75298ca742339d70f86460645a6145070737a883) )
	ROM_LOAD32_BYTE( "1.0_rampage_world_tour_u127_image.u127",  0x0400002, 0x100000, CRC(6e1756ba) SHA1(6e88ee4f239d41430cdcacfbfe9dfe9e75c5e4ed) )
	ROM_LOAD32_BYTE( "1.0_rampage_world_tour_u126_image.u126",  0x0400003, 0x100000, CRC(c300eb1b) SHA1(e2fffa31c773737d5f5f3f053f9afee4690fcd3d) )

	ROM_LOAD32_BYTE( "1.0_rampage_world_tour_u125_image.u125",  0x0800000, 0x100000, CRC(7369bf5d) SHA1(edd84d4119d63263bc65adf953370a6e36c797d3) )
	ROM_LOAD32_BYTE( "1.0_rampage_world_tour_u124_image.u124",  0x0800001, 0x100000, CRC(c0bf88c8) SHA1(5ea348bcd208c1c1f5f5943f1e1ef81d25d2b95d) )
	ROM_LOAD32_BYTE( "1.0_rampage_world_tour_u123_image.u123",  0x0800002, 0x100000, CRC(ac4c712a) SHA1(ccf40d004ddf1b62870b99da359b00d9fd702944) )
	ROM_LOAD32_BYTE( "1.0_rampage_world_tour_u122_image.u122",  0x0800003, 0x100000, CRC(609862a2) SHA1(366c70d5de9135c28934ed6ccf4f373c5a76c748) )

	ROM_LOAD32_BYTE( "1.0_rampage_world_tour_u121_image.u121",  0x0c00000, 0x100000, CRC(f65119b7) SHA1(4c3110f3be370cad0b031a7aa8605559bdd14842) )
	ROM_LOAD32_BYTE( "1.0_rampage_world_tour_u120_image.u120",  0x0c00001, 0x100000, CRC(6d643dee) SHA1(923ad01a0eb5a26d2a913f09fde254605113c868) )
	ROM_LOAD32_BYTE( "1.0_rampage_world_tour_u119_image.u119",  0x0c00002, 0x100000, CRC(4e49c133) SHA1(0f83b0d645286f6c6196bcf316ac010378b96c43) )
	ROM_LOAD32_BYTE( "1.0_rampage_world_tour_u118_image.u118",  0x0c00003, 0x100000, CRC(43a6f51e) SHA1(cb9d698a6a2ab9134339264e851b57e599db135c) )
ROM_END


ROM_START( rmpgwt11 )
	ROM_REGION16_LE( 0x800000, "dcs", ROMREGION_ERASEFF )   // sound data
	ROM_LOAD16_BYTE( "1.0_rampage_world_tour_u2_sound.u2",  0x000000, 0x100000, CRC(0e82f83d) SHA1(215eebb6c229ef9ad0fcbcbc6e4e07300c05654f) )
	ROM_LOAD16_BYTE( "1.0_rampage_world_tour_u3_sound.u3",  0x200000, 0x100000, CRC(3ff54d15) SHA1(827805602091313ec68ea1bccf667bd3b3fc6b8b) )
	ROM_LOAD16_BYTE( "1.0_rampage_world_tour_u4_sound.u4",  0x400000, 0x100000, CRC(5c7f5656) SHA1(6c9d692bad539fec8b5aa0bfb56de3ef3719c68a) )
	ROM_LOAD16_BYTE( "1.0_rampage_world_tour_u5_sound.u5",  0x600000, 0x100000, CRC(fd9aaf24) SHA1(d60dc076e72618c99ecac9d081d8c49d337b90c7) )

	ROM_REGION16_LE( 0x100000, "maincpu", 0 )   // 34010 code
	ROM_LOAD16_BYTE( "1.1_rampage_world_u54_game.u54",  0x00000, 0x80000, CRC(3aa514eb) SHA1(4ed8db55f257da6d872586d0f9f0cdf1c30e0d22) )
	ROM_LOAD16_BYTE( "1.1_rampage_world_u63_game.u63",  0x00001, 0x80000, CRC(031c908f) SHA1(531669b13c33921ff199be1e841dd337c86fec50) )

	ROM_REGION( 0x1009, "serial_security:pic", 0 )   // security PIC (provides game ID code and serial number)
	ROM_LOAD( "465_rampage_wt.u64",  0x0000, 0x1009, CRC(5c14d850) SHA1(f57aef8350e477252bff1fa0f930c1b5d0ceb03f) )

	ROM_REGION( 0x2000000, "video", 0 )
	ROM_LOAD32_BYTE( "1.0_rampage_world_tour_u133_image.u133",  0x0000000, 0x100000, CRC(5b5ac449) SHA1(1c01dde9a9dbd9f4a6cd30aea9f6410cab13c2c9) )
	ROM_LOAD32_BYTE( "1.0_rampage_world_tour_u132_image.u132",  0x0000001, 0x100000, CRC(7b3f09c6) SHA1(477658481ee96d5ce462d5e198d80faff4d4352c) )
	ROM_LOAD32_BYTE( "1.0_rampage_world_tour_u131_image.u131",  0x0000002, 0x100000, CRC(fdecf12e) SHA1(bcbd29009dabed484e2357dc75c38c7d7bade251) )
	ROM_LOAD32_BYTE( "1.0_rampage_world_tour_u130_image.u130",  0x0000003, 0x100000, CRC(4a983b05) SHA1(022753e6fa3b3d74eff5b2ec835b07787f5473ab) )

	ROM_LOAD32_BYTE( "1.0_rampage_world_tour_u129_image.u129",  0x0400000, 0x100000, CRC(dc495c6e) SHA1(7ec80d293f06a013c4f95bfb3fafe5b4a71cf170) )
	ROM_LOAD32_BYTE( "1.0_rampage_world_tour_u128_image.u128",  0x0400001, 0x100000, CRC(5545503d) SHA1(75298ca742339d70f86460645a6145070737a883) )
	ROM_LOAD32_BYTE( "1.0_rampage_world_tour_u127_image.u127",  0x0400002, 0x100000, CRC(6e1756ba) SHA1(6e88ee4f239d41430cdcacfbfe9dfe9e75c5e4ed) )
	ROM_LOAD32_BYTE( "1.0_rampage_world_tour_u126_image.u126",  0x0400003, 0x100000, CRC(c300eb1b) SHA1(e2fffa31c773737d5f5f3f053f9afee4690fcd3d) )

	ROM_LOAD32_BYTE( "1.0_rampage_world_tour_u125_image.u125",  0x0800000, 0x100000, CRC(7369bf5d) SHA1(edd84d4119d63263bc65adf953370a6e36c797d3) )
	ROM_LOAD32_BYTE( "1.0_rampage_world_tour_u124_image.u124",  0x0800001, 0x100000, CRC(c0bf88c8) SHA1(5ea348bcd208c1c1f5f5943f1e1ef81d25d2b95d) )
	ROM_LOAD32_BYTE( "1.0_rampage_world_tour_u123_image.u123",  0x0800002, 0x100000, CRC(ac4c712a) SHA1(ccf40d004ddf1b62870b99da359b00d9fd702944) )
	ROM_LOAD32_BYTE( "1.0_rampage_world_tour_u122_image.u122",  0x0800003, 0x100000, CRC(609862a2) SHA1(366c70d5de9135c28934ed6ccf4f373c5a76c748) )

	ROM_LOAD32_BYTE( "1.0_rampage_world_tour_u121_image.u121",  0x0c00000, 0x100000, CRC(f65119b7) SHA1(4c3110f3be370cad0b031a7aa8605559bdd14842) )
	ROM_LOAD32_BYTE( "1.0_rampage_world_tour_u120_image.u120",  0x0c00001, 0x100000, CRC(6d643dee) SHA1(923ad01a0eb5a26d2a913f09fde254605113c868) )
	ROM_LOAD32_BYTE( "1.0_rampage_world_tour_u119_image.u119",  0x0c00002, 0x100000, CRC(4e49c133) SHA1(0f83b0d645286f6c6196bcf316ac010378b96c43) )
	ROM_LOAD32_BYTE( "1.0_rampage_world_tour_u118_image.u118",  0x0c00003, 0x100000, CRC(43a6f51e) SHA1(cb9d698a6a2ab9134339264e851b57e599db135c) )
ROM_END


/*
A WWF: Wrestlemania PCB with V1.3 program ROMs was seen with different styled labels:

SOUND  U2  WRESTLEMANIA 5/17/95  5D00 - crossed out and hand written 6-6-95 5500
SOUND  U3  WRESTLEMANIA 5/2/95  8300
SOUND  U4  WRESTLEMANIA 5/2/95  0F00
SOUND  U5  WRESTLEMANIA 5/2/95  3000

U54 WRESTLEMANIA Rev1.3 8/9/95 8A5D
U63 WRESTLEMANIA Rev1.3 8/9/95 9EFE

2M.0  U133  WRESTLEMANIA REV1.0 6/5/95  3A18
2M.1  U132  WRESTLEMANIA REV1.0 6/5/95  5B68
2M.2  U131  WRESTLEMANIA REV1.0 6/5/95  A478
2M.3  U130  WRESTLEMANIA REV1.0 6/5/95  E4EA

4M.0  U129  WRESTLEMANIA REV1.0 6/5/95  0B0E
4M.1  U128  WRESTLEMANIA REV1.0 6/5/95  E646
4M.2  U127  WRESTLEMANIA REV1.0 6/5/95  74A7
4M.3  U126  WRESTLEMANIA REV1.0 6/5/95  37FD

6M.0  U125  WRESTLEMANIA REV1.0 6/5/95  5EF3
6M.1  U124  WRESTLEMANIA REV1.0 6/5/95  8CD6
6M.2  U123  WRESTLEMANIA REV1.0 6/5/95  70EC
6M.3  U122  WRESTLEMANIA REV1.0 6/5/95  2B4E

8M.0  U121  WRESTLEMANIA REV1.0 6/5/95  5E15
8M.1  U120  WRESTLEMANIA REV1.0 6/5/95  0572
8M.2  U119  WRESTLEMANIA REV1.0 6/5/95  7FD4
8M.3  U118  WRESTLEMANIA REV1.0 6/5/95  E4F1
*/
ROM_START( wwfmania )
	ROM_REGION16_LE( 0x800000, "dcs", ROMREGION_ERASEFF )   // sound data
	ROM_LOAD16_BYTE( "wwf_music-spch_l1.u2", 0x000000, 0x100000, CRC(a9acb250) SHA1(c1a7773ffdb86dc2c1c90c220482ed6330fcbb55) ) // These 4 are labeled as L1
	ROM_LOAD16_BYTE( "wwf_music-spch_l1.u3", 0x200000, 0x100000, CRC(9442b6c9) SHA1(1f887c05ab9ca99078be584d7e9e6c59c8ec1818) )
	ROM_LOAD16_BYTE( "wwf_music-spch_l1.u4", 0x400000, 0x100000, CRC(cee78fac) SHA1(c37d3b4aef47dc80d864497b3013f03220d45482) )
	ROM_LOAD16_BYTE( "wwf_music-spch_l1.u5", 0x600000, 0x100000, CRC(5b31fd40) SHA1(35dcf19b223029e17616357d29dd04bbfeb83491) )

	ROM_REGION16_LE( 0x100000, "maincpu", 0 )   // 34010 code
	ROM_LOAD16_BYTE( "wwf_game_rom_l1.30.u54", 0x00000, 0x80000, CRC(eeb7bf58) SHA1(d93df59aed1672ab38af231d909d9df1a8e30f44) ) // Labeled as L1.30
	ROM_LOAD16_BYTE( "wwf_game_rom_l1.30.u63", 0x00001, 0x80000, CRC(09759529) SHA1(cf548ff199428a93b9bc5f4fc1347c4a3cbdf106) ) // Labeled as L1.30

	ROM_REGION( 0x2000000, "video", 0 )
	ROM_LOAD32_BYTE( "wwf_image_rom_l1.u133", 0x0000000, 0x100000, CRC(5e1b1e3d) SHA1(55f54e4b0dc775058699b1c0abdd7241ffca0e76) ) // All graphics roms labeled as L1
	ROM_LOAD32_BYTE( "wwf_image_rom_l1.u132", 0x0000001, 0x100000, CRC(5943b3b2) SHA1(8ba0b20e7993769736c961d0fda97b2850d1446b) )
	ROM_LOAD32_BYTE( "wwf_image_rom_l1.u131", 0x0000002, 0x100000, CRC(0815db22) SHA1(ebd6a8c4f0e8d979af7f173b3f139d91e4857f6b) )
	ROM_LOAD32_BYTE( "wwf_image_rom_l1.u130", 0x0000003, 0x100000, CRC(9ee9a145) SHA1(caeb8506e1414e8c58e3031d4a2e0619ef3922b7) )

	ROM_LOAD32_BYTE( "wwf_image_rom_l1.u129", 0x0400000, 0x100000, CRC(c644c2f4) SHA1(9094452eb37ec92932109ab2b209e12074111dd7) )
	ROM_LOAD32_BYTE( "wwf_image_rom_l1.u128", 0x0400001, 0x100000, CRC(fcda4e9a) SHA1(a05a12f606632034eae662cccfee5aaaffe0348b) )
	ROM_LOAD32_BYTE( "wwf_image_rom_l1.u127", 0x0400002, 0x100000, CRC(45be7428) SHA1(a5d3e37c64cac03139028fe998494b76e6b6a7ae) )
	ROM_LOAD32_BYTE( "wwf_image_rom_l1.u126", 0x0400003, 0x100000, CRC(eaa276a8) SHA1(d0c2f4d4409830355c6e112e3eafb4d3a1b8c22e) )

	ROM_LOAD32_BYTE( "wwf_image_rom_l1.u125", 0x0800000, 0x100000, CRC(a19ebeed) SHA1(cf51bca29fd39c6189c2b431eb718a6341781d1f) )
	ROM_LOAD32_BYTE( "wwf_image_rom_l1.u124", 0x0800001, 0x100000, CRC(dc7d3dbb) SHA1(8982d9a1babce57ae7465bce3f4863dd336c20ac) )
	ROM_LOAD32_BYTE( "wwf_image_rom_l1.u123", 0x0800002, 0x100000, CRC(e0ade56f) SHA1(a15c672a45f39c0232d678e71380d4f58c4659ae) )
	ROM_LOAD32_BYTE( "wwf_image_rom_l1.u122", 0x0800003, 0x100000, CRC(2800c78d) SHA1(8012785f1c1eaf8d533a98e0a521a5d31efc7a42) )

	ROM_LOAD32_BYTE( "wwf_image_rom_l1.u121", 0x0c00000, 0x100000, CRC(a28ffcba) SHA1(f66be0793b12a7f04e32d3db8311d1f33b0c3fbe) )
	ROM_LOAD32_BYTE( "wwf_image_rom_l1.u120", 0x0c00001, 0x100000, CRC(3a05d371) SHA1(4ed73e1c06ea7bd33e6c72a6a752960ba55d1975) )
	ROM_LOAD32_BYTE( "wwf_image_rom_l1.u119", 0x0c00002, 0x100000, CRC(97ffa659) SHA1(986f8ec57085b808d33c85ed55b35a5e1cadf3b6) )
	ROM_LOAD32_BYTE( "wwf_image_rom_l1.u118", 0x0c00003, 0x100000, CRC(46668e97) SHA1(282ca2e561f7553717d60b5a745f8e3fc1bda610) )
ROM_END


ROM_START( wwfmaniab )
	ROM_REGION16_LE( 0x800000, "dcs", ROMREGION_ERASEFF )   // sound data
	ROM_LOAD16_BYTE( "wwf_music-spch_l1.u2", 0x000000, 0x100000, CRC(a9acb250) SHA1(c1a7773ffdb86dc2c1c90c220482ed6330fcbb55) ) // These 4 are labeled as L1
	ROM_LOAD16_BYTE( "wwf_music-spch_l1.u3", 0x200000, 0x100000, CRC(9442b6c9) SHA1(1f887c05ab9ca99078be584d7e9e6c59c8ec1818) )
	ROM_LOAD16_BYTE( "wwf_music-spch_l1.u4", 0x400000, 0x100000, CRC(cee78fac) SHA1(c37d3b4aef47dc80d864497b3013f03220d45482) )
	ROM_LOAD16_BYTE( "wwf_music-spch_l1.u5", 0x600000, 0x100000, CRC(5b31fd40) SHA1(35dcf19b223029e17616357d29dd04bbfeb83491) )

	ROM_REGION16_LE( 0x100000, "maincpu", 0 )   // 34010 code
	ROM_LOAD16_BYTE( "wwf_game_rom_l1.20.u54",  0x00000, 0x80000, CRC(1b2dce48) SHA1(f70b6c5b56f9fc15cedfd8e0a95f983f3ea6dbb7) ) // Labeled as L1.20
	ROM_LOAD16_BYTE( "wwf_game_rom_l1.20.u63",  0x00001, 0x80000, CRC(1262f0bb) SHA1(e97a5939f10532f7815d08b1a7d63a7554d47d4f) ) // Labeled as L1.20

	ROM_REGION( 0x2000000, "video", 0 )
	ROM_LOAD32_BYTE( "wwf_image_rom_l1.u133", 0x0000000, 0x100000, CRC(5e1b1e3d) SHA1(55f54e4b0dc775058699b1c0abdd7241ffca0e76) ) // All graphics roms labeled as L1
	ROM_LOAD32_BYTE( "wwf_image_rom_l1.u132", 0x0000001, 0x100000, CRC(5943b3b2) SHA1(8ba0b20e7993769736c961d0fda97b2850d1446b) )
	ROM_LOAD32_BYTE( "wwf_image_rom_l1.u131", 0x0000002, 0x100000, CRC(0815db22) SHA1(ebd6a8c4f0e8d979af7f173b3f139d91e4857f6b) )
	ROM_LOAD32_BYTE( "wwf_image_rom_l1.u130", 0x0000003, 0x100000, CRC(9ee9a145) SHA1(caeb8506e1414e8c58e3031d4a2e0619ef3922b7) )

	ROM_LOAD32_BYTE( "wwf_image_rom_l1.u129", 0x0400000, 0x100000, CRC(c644c2f4) SHA1(9094452eb37ec92932109ab2b209e12074111dd7) )
	ROM_LOAD32_BYTE( "wwf_image_rom_l1.u128", 0x0400001, 0x100000, CRC(fcda4e9a) SHA1(a05a12f606632034eae662cccfee5aaaffe0348b) )
	ROM_LOAD32_BYTE( "wwf_image_rom_l1.u127", 0x0400002, 0x100000, CRC(45be7428) SHA1(a5d3e37c64cac03139028fe998494b76e6b6a7ae) )
	ROM_LOAD32_BYTE( "wwf_image_rom_l1.u126", 0x0400003, 0x100000, CRC(eaa276a8) SHA1(d0c2f4d4409830355c6e112e3eafb4d3a1b8c22e) )

	ROM_LOAD32_BYTE( "wwf_image_rom_l1.u125", 0x0800000, 0x100000, CRC(a19ebeed) SHA1(cf51bca29fd39c6189c2b431eb718a6341781d1f) )
	ROM_LOAD32_BYTE( "wwf_image_rom_l1.u124", 0x0800001, 0x100000, CRC(dc7d3dbb) SHA1(8982d9a1babce57ae7465bce3f4863dd336c20ac) )
	ROM_LOAD32_BYTE( "wwf_image_rom_l1.u123", 0x0800002, 0x100000, CRC(e0ade56f) SHA1(a15c672a45f39c0232d678e71380d4f58c4659ae) )
	ROM_LOAD32_BYTE( "wwf_image_rom_l1.u122", 0x0800003, 0x100000, CRC(2800c78d) SHA1(8012785f1c1eaf8d533a98e0a521a5d31efc7a42) )

	ROM_LOAD32_BYTE( "wwf_image_rom_l1.u121", 0x0c00000, 0x100000, CRC(a28ffcba) SHA1(f66be0793b12a7f04e32d3db8311d1f33b0c3fbe) )
	ROM_LOAD32_BYTE( "wwf_image_rom_l1.u120", 0x0c00001, 0x100000, CRC(3a05d371) SHA1(4ed73e1c06ea7bd33e6c72a6a752960ba55d1975) )
	ROM_LOAD32_BYTE( "wwf_image_rom_l1.u119", 0x0c00002, 0x100000, CRC(97ffa659) SHA1(986f8ec57085b808d33c85ed55b35a5e1cadf3b6) )
	ROM_LOAD32_BYTE( "wwf_image_rom_l1.u118", 0x0c00003, 0x100000, CRC(46668e97) SHA1(282ca2e561f7553717d60b5a745f8e3fc1bda610) )
ROM_END


ROM_START( wwfmaniac )
	ROM_REGION16_LE( 0x800000, "dcs", ROMREGION_ERASEFF )   // sound data
	ROM_LOAD16_BYTE( "wwf_music-spch_l1.u2", 0x000000, 0x100000, CRC(a9acb250) SHA1(c1a7773ffdb86dc2c1c90c220482ed6330fcbb55) ) // These 4 are labeled as L1
	ROM_LOAD16_BYTE( "wwf_music-spch_l1.u3", 0x200000, 0x100000, CRC(9442b6c9) SHA1(1f887c05ab9ca99078be584d7e9e6c59c8ec1818) )
	ROM_LOAD16_BYTE( "wwf_music-spch_l1.u4", 0x400000, 0x100000, CRC(cee78fac) SHA1(c37d3b4aef47dc80d864497b3013f03220d45482) )
	ROM_LOAD16_BYTE( "wwf_music-spch_l1.u5", 0x600000, 0x100000, CRC(5b31fd40) SHA1(35dcf19b223029e17616357d29dd04bbfeb83491) )

	ROM_REGION16_LE( 0x100000, "maincpu", 0 )   // 34010 code
	ROM_LOAD16_BYTE( "wwf_game_rom_l1.10.u54",  0x00000, 0x80000, CRC(ae1a3195) SHA1(89ce1e3dc46b4da2d723b61e868889d05f7d5162) ) // Labeled as L1.10, test menu shows REV 1.1
	ROM_LOAD16_BYTE( "wwf_game_rom_l1.10.u63",  0x00001, 0x80000, CRC(d809eb60) SHA1(9531009fb6e245548ab52ac1cbb6c736d6357cb5) ) // Labeled as L1.10, test menu shows REV 1.1

	ROM_REGION( 0x2000000, "video", 0 )
	ROM_LOAD32_BYTE( "wwf_image_rom_l1.u133", 0x0000000, 0x100000, CRC(5e1b1e3d) SHA1(55f54e4b0dc775058699b1c0abdd7241ffca0e76) ) // All graphics roms labeled as L1
	ROM_LOAD32_BYTE( "wwf_image_rom_l1.u132", 0x0000001, 0x100000, CRC(5943b3b2) SHA1(8ba0b20e7993769736c961d0fda97b2850d1446b) )
	ROM_LOAD32_BYTE( "wwf_image_rom_l1.u131", 0x0000002, 0x100000, CRC(0815db22) SHA1(ebd6a8c4f0e8d979af7f173b3f139d91e4857f6b) )
	ROM_LOAD32_BYTE( "wwf_image_rom_l1.u130", 0x0000003, 0x100000, CRC(9ee9a145) SHA1(caeb8506e1414e8c58e3031d4a2e0619ef3922b7) )

	ROM_LOAD32_BYTE( "wwf_image_rom_l1.u129", 0x0400000, 0x100000, CRC(c644c2f4) SHA1(9094452eb37ec92932109ab2b209e12074111dd7) )
	ROM_LOAD32_BYTE( "wwf_image_rom_l1.u128", 0x0400001, 0x100000, CRC(fcda4e9a) SHA1(a05a12f606632034eae662cccfee5aaaffe0348b) )
	ROM_LOAD32_BYTE( "wwf_image_rom_l1.u127", 0x0400002, 0x100000, CRC(45be7428) SHA1(a5d3e37c64cac03139028fe998494b76e6b6a7ae) )
	ROM_LOAD32_BYTE( "wwf_image_rom_l1.u126", 0x0400003, 0x100000, CRC(eaa276a8) SHA1(d0c2f4d4409830355c6e112e3eafb4d3a1b8c22e) )

	ROM_LOAD32_BYTE( "wwf_image_rom_l1.u125", 0x0800000, 0x100000, CRC(a19ebeed) SHA1(cf51bca29fd39c6189c2b431eb718a6341781d1f) )
	ROM_LOAD32_BYTE( "wwf_image_rom_l1.u124", 0x0800001, 0x100000, CRC(dc7d3dbb) SHA1(8982d9a1babce57ae7465bce3f4863dd336c20ac) )
	ROM_LOAD32_BYTE( "wwf_image_rom_l1.u123", 0x0800002, 0x100000, CRC(e0ade56f) SHA1(a15c672a45f39c0232d678e71380d4f58c4659ae) )
	ROM_LOAD32_BYTE( "wwf_image_rom_l1.u122", 0x0800003, 0x100000, CRC(2800c78d) SHA1(8012785f1c1eaf8d533a98e0a521a5d31efc7a42) )

	ROM_LOAD32_BYTE( "wwf_image_rom_l1.u121", 0x0c00000, 0x100000, CRC(a28ffcba) SHA1(f66be0793b12a7f04e32d3db8311d1f33b0c3fbe) )
	ROM_LOAD32_BYTE( "wwf_image_rom_l1.u120", 0x0c00001, 0x100000, CRC(3a05d371) SHA1(4ed73e1c06ea7bd33e6c72a6a752960ba55d1975) )
	ROM_LOAD32_BYTE( "wwf_image_rom_l1.u119", 0x0c00002, 0x100000, CRC(97ffa659) SHA1(986f8ec57085b808d33c85ed55b35a5e1cadf3b6) )
	ROM_LOAD32_BYTE( "wwf_image_rom_l1.u118", 0x0c00003, 0x100000, CRC(46668e97) SHA1(282ca2e561f7553717d60b5a745f8e3fc1bda610) )
ROM_END


ROM_START( wwfmaniap )
	ROM_REGION16_LE( 0x800000, "dcs", ROMREGION_ERASEFF )   // sound data
	ROM_LOAD16_BYTE( "wwf_music-spch_l1.u2", 0x000000, 0x100000, CRC(a9acb250) SHA1(c1a7773ffdb86dc2c1c90c220482ed6330fcbb55) ) // These 4 are labeled as L1
	ROM_LOAD16_BYTE( "wwf_music-spch_l1.u3", 0x200000, 0x100000, CRC(9442b6c9) SHA1(1f887c05ab9ca99078be584d7e9e6c59c8ec1818) )
	ROM_LOAD16_BYTE( "wwf_music-spch_l1.u4", 0x400000, 0x100000, CRC(cee78fac) SHA1(c37d3b4aef47dc80d864497b3013f03220d45482) )
	ROM_LOAD16_BYTE( "wwf_music-spch_l1.u5", 0x600000, 0x100000, CRC(5b31fd40) SHA1(35dcf19b223029e17616357d29dd04bbfeb83491) )

	ROM_REGION16_LE( 0x100000, "maincpu", 0 )   // 34010 code
	ROM_LOAD16_BYTE( "wwf_game_rom_p2.01.u54",  0x00000, 0x80000, CRC(a3d0b6d1) SHA1(974e0d40e3852b4c3233098079ded95110cca62e) ) // missing labels
	ROM_LOAD16_BYTE( "wwf_game_rom_p2.01.u63",  0x00001, 0x80000, CRC(22b80ae4) SHA1(4e160df9caf43fcf43ce002af4c88c2a324c4d86) ) // missing labels

	ROM_REGION( 0x2000000, "video", 0 )
	ROM_LOAD32_BYTE( "wwf_image_rom_l1.u133", 0x0000000, 0x100000, CRC(5e1b1e3d) SHA1(55f54e4b0dc775058699b1c0abdd7241ffca0e76) ) // All graphics roms labeled as L1
	ROM_LOAD32_BYTE( "wwf_image_rom_l1.u132", 0x0000001, 0x100000, CRC(5943b3b2) SHA1(8ba0b20e7993769736c961d0fda97b2850d1446b) )
	ROM_LOAD32_BYTE( "wwf_image_rom_l1.u131", 0x0000002, 0x100000, CRC(0815db22) SHA1(ebd6a8c4f0e8d979af7f173b3f139d91e4857f6b) )
	ROM_LOAD32_BYTE( "wwf_image_rom_l1.u130", 0x0000003, 0x100000, CRC(9ee9a145) SHA1(caeb8506e1414e8c58e3031d4a2e0619ef3922b7) )

	ROM_LOAD32_BYTE( "wwf_image_rom_l1.u129", 0x0400000, 0x100000, CRC(c644c2f4) SHA1(9094452eb37ec92932109ab2b209e12074111dd7) )
	ROM_LOAD32_BYTE( "wwf_image_rom_l1.u128", 0x0400001, 0x100000, CRC(fcda4e9a) SHA1(a05a12f606632034eae662cccfee5aaaffe0348b) )
	ROM_LOAD32_BYTE( "wwf_image_rom_l1.u127", 0x0400002, 0x100000, CRC(45be7428) SHA1(a5d3e37c64cac03139028fe998494b76e6b6a7ae) )
	ROM_LOAD32_BYTE( "wwf_image_rom_l1.u126", 0x0400003, 0x100000, CRC(eaa276a8) SHA1(d0c2f4d4409830355c6e112e3eafb4d3a1b8c22e) )

	ROM_LOAD32_BYTE( "wwf_image_rom_l1.u125", 0x0800000, 0x100000, CRC(a19ebeed) SHA1(cf51bca29fd39c6189c2b431eb718a6341781d1f) )
	ROM_LOAD32_BYTE( "wwf_image_rom_l1.u124", 0x0800001, 0x100000, CRC(dc7d3dbb) SHA1(8982d9a1babce57ae7465bce3f4863dd336c20ac) )
	ROM_LOAD32_BYTE( "wwf_image_rom_l1.u123", 0x0800002, 0x100000, CRC(e0ade56f) SHA1(a15c672a45f39c0232d678e71380d4f58c4659ae) )
	ROM_LOAD32_BYTE( "wwf_image_rom_l1.u122", 0x0800003, 0x100000, CRC(2800c78d) SHA1(8012785f1c1eaf8d533a98e0a521a5d31efc7a42) )

	ROM_LOAD32_BYTE( "wwf_image_rom_l1.u121", 0x0c00000, 0x100000, CRC(a28ffcba) SHA1(f66be0793b12a7f04e32d3db8311d1f33b0c3fbe) )
	ROM_LOAD32_BYTE( "wwf_image_rom_l1.u120", 0x0c00001, 0x100000, CRC(3a05d371) SHA1(4ed73e1c06ea7bd33e6c72a6a752960ba55d1975) )
	ROM_LOAD32_BYTE( "wwf_image_rom_l1.u119", 0x0c00002, 0x100000, CRC(97ffa659) SHA1(986f8ec57085b808d33c85ed55b35a5e1cadf3b6) )
	ROM_LOAD32_BYTE( "wwf_image_rom_l1.u118", 0x0c00003, 0x100000, CRC(46668e97) SHA1(282ca2e561f7553717d60b5a745f8e3fc1bda610) )
ROM_END



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1994, mk3,         0,        wunit_picsim, mk3,      midwunit_state, init_mk3,      ROT0, "Midway", "Mortal Kombat 3 (rev 2.1)", MACHINE_SUPPORTS_SAVE )
GAME( 1994, mk3r20,      mk3,      wunit_picsim, mk3,      midwunit_state, init_mk3r20,   ROT0, "Midway", "Mortal Kombat 3 (rev 2.0)", MACHINE_SUPPORTS_SAVE )
GAME( 1994, mk3r10,      mk3,      wunit_picsim, mk3,      midwunit_state, init_mk3r10,   ROT0, "Midway", "Mortal Kombat 3 (rev 1.0)", MACHINE_SUPPORTS_SAVE )
GAME( 1994, mk3p40,      mk3,      wunit_picsim, mk3,      midwunit_state, init_mk3r10,   ROT0, "Midway", "Mortal Kombat 3 (rev 1 chip label p4.0)", MACHINE_SUPPORTS_SAVE )

GAME( 1994, umk3,        0,        wunit_picemu, mk3,      midwunit_state, init_umk3,     ROT0, "Midway", "Ultimate Mortal Kombat 3 (rev 1.2)", MACHINE_SUPPORTS_SAVE )
GAME( 1994, umk3r11,     umk3,     wunit_picemu, mk3,      midwunit_state, init_umk3r11,  ROT0, "Midway", "Ultimate Mortal Kombat 3 (rev 1.1)", MACHINE_SUPPORTS_SAVE )
GAME( 1994, umk3r10,     umk3,     wunit_picemu, mk3,      midwunit_state, init_umk3r11,  ROT0, "Midway", "Ultimate Mortal Kombat 3 (rev 1.0)", MACHINE_SUPPORTS_SAVE )
// Ultimate Mortal Kombat 3 rev 2.0.35 (TE? Hack?) version known to exist

GAME( 1995, wwfmania,    0,        wunit_picsim, wwfmania, midwunit_state, init_wwfmania, ROT0, "Midway", "WWF: Wrestlemania (rev 1.30 08/10/95)", MACHINE_SUPPORTS_SAVE )
GAME( 1995, wwfmaniab,   wwfmania, wunit_picsim, wwfmania, midwunit_state, init_wwfmania, ROT0, "Midway", "WWF: Wrestlemania (rev 1.20 08/02/95)", MACHINE_SUPPORTS_SAVE )
GAME( 1995, wwfmaniac,   wwfmania, wunit_picsim, wwfmania, midwunit_state, init_wwfmania, ROT0, "Midway", "WWF: Wrestlemania (rev 1.1 07/11/95)", MACHINE_SUPPORTS_SAVE )
GAME( 1995, wwfmaniap,   wwfmania, wunit_picsim, wwfmania, midwunit_state, init_wwfmania, ROT0, "Midway", "WWF: Wrestlemania (proto 2.01 06/07/95)", MACHINE_SUPPORTS_SAVE )

GAME( 1995, openice,     0,        wunit_picsim, openice,  midwunit_state, init_openice,  ROT0, "Midway", "NHL Open Ice: 2 on 2 Challenge (rev 1.21)", MACHINE_SUPPORTS_SAVE )
GAME( 1995, openicea,    openice,  wunit_picsim, openice,  midwunit_state, init_openice,  ROT0, "Midway", "NHL Open Ice: 2 on 2 Challenge (rev 1.2A)", MACHINE_SUPPORTS_SAVE )

GAME( 1996, nbahangt,    0,        wunit_picsim, nbahangt, midwunit_state, init_nbahangt, ROT0, "Midway", "NBA Hangtime (ver L1.3 10/10/96)", MACHINE_SUPPORTS_SAVE )
GAME( 1996, nbahangtm13, nbahangt, wunit_picsim, nbahangt, midwunit_state, init_nbahangt, ROT0, "Midway", "NBA Hangtime (ver M1.3 10/10/96)", MACHINE_SUPPORTS_SAVE )
GAME( 1996, nbahangtl12, nbahangt, wunit_picsim, nbahangt, midwunit_state, init_nbahangt, ROT0, "Midway", "NBA Hangtime (ver L1.2 8/29/96)", MACHINE_SUPPORTS_SAVE )
GAME( 1996, nbahangtm12, nbahangt, wunit_picsim, nbahangt, midwunit_state, init_nbahangt, ROT0, "Midway", "NBA Hangtime (ver M1.2 8/29/96)", MACHINE_SUPPORTS_SAVE )
GAME( 1996, nbahangtl11, nbahangt, wunit_picsim, nbahangt, midwunit_state, init_nbahangt, ROT0, "Midway", "NBA Hangtime (ver L1.1 4/16/96)", MACHINE_SUPPORTS_SAVE )
GAME( 1996, nbahangtm11, nbahangt, wunit_picsim, nbahangt, midwunit_state, init_nbahangt, ROT0, "Midway", "NBA Hangtime (ver M1.1 4/16/96)", MACHINE_SUPPORTS_SAVE )

GAME( 1996, nbamht,      nbahangt, wunit_picsim, nbahangt, midwunit_state, init_nbahangt, ROT0, "Midway", "NBA Maximum Hangtime (ver L1.03 06/09/97)", MACHINE_SUPPORTS_SAVE )
GAME( 1996, nbamhtl10,   nbahangt, wunit_picsim, nbahangt, midwunit_state, init_nbahangt, ROT0, "Midway", "NBA Maximum Hangtime (ver L1.0 11/08/96)", MACHINE_SUPPORTS_SAVE )
GAME( 1996, nbamhtm10,   nbahangt, wunit_picsim, nbahangt, midwunit_state, init_nbahangt, ROT0, "Midway", "NBA Maximum Hangtime (ver M1.0 11/08/96)", MACHINE_SUPPORTS_SAVE )
GAME( 1996, nbamhtp,     nbahangt, wunit_picsim, nbahangt, midwunit_state, init_nbahangt, ROT0, "Midway", "NBA Maximum Hangtime (ver L0.9 10/30/96)", MACHINE_SUPPORTS_SAVE )

GAME( 1997, rmpgwt,      0,        wunit_picemu, rmpgwt,   midwunit_state, init_rmpgwt,   ROT0, "Midway", "Rampage: World Tour (rev 1.3)", MACHINE_SUPPORTS_SAVE )
GAME( 1997, rmpgwt11,    rmpgwt,   wunit_picemu, rmpgwt,   midwunit_state, init_rmpgwt,   ROT0, "Midway", "Rampage: World Tour (rev 1.1)", MACHINE_SUPPORTS_SAVE )
