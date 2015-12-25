// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

                            -= Jaleco Driving Games =-

                    driver by   Luca Elia (l.elia@tin.it)

- Note: press F2 to enter service mode -

---------------------------------------------------------------------------
Year + Game     Hardware:   Main    Sub#1   Sub#2   Sound   Sound Chips
---------------------------------------------------------------------------
[89 Big Run]                68000   68000   68000   68000   YM2151 2xM6295

    BOARD #1    BR8950c
    BOARD #2    8951
    BOARD #3    8952A
    BOARD #4    8953

[90 Cisco Heat]             68000   68000   68000   68000   YM2151 2xM6295

    BOARD #1 CH-9072 EB90001-20024
    BOARD #2 CH-9071 EB90001-20023
    BOARD #3 CH-9073 EB90001-20025

[91 F1 GP Star]             68000   68000   68000   68000   YM2151 2xM6295

    TB - Top board     (audio & I/O)       GP-9190A EB90015-20039-1
    MB - Middle board  (GFX)               GP-9189  EB90015-20038
    LB - Lower board   (CPU/GFX)           GP-9188A EB90015-20037-1

    Chips:

        GS90015-02 (100 pin PQFP)   x 3     [Tilemaps]
        GS-9000406 (80 pin PQFP)    x 3

        GS900151   (44 pin PQFP) (too small for the full part No.)
        GS90015-03 (80 pin PQFP)    x 3  + 2x LH52258D-45 (32kx8 SRAM)
        GS90015-06 (100 pin PQFP)   x 2  + 2x LH52250AD-90L (32kx8 SRAM)
        GS90015-07 (64 pin PQFP)
        GS90015-08 (64 pin PQFP)
        GS90015-09 (64 pin PQFP)  + 2x MCM2018AN45 (2kx8 SRAM)
        GS90015-10 (64 pin PQFP)
        GS90015-12 (80 pin PQFP)  + 2x MCM2018AN45 (2kx8 SRAM)
        GS90015-11 (100 pin PQFP)

        CS90015-04 x 2 (64 pin PQFP)        [Road]
        GS90015-05 x 2 (100 pin PQFP)

[92 Wild Pilot]             68000   68000   68000   68000   YM2151 2xM6295 + another 68000

        TB - Top board     (audio & I/O)       WP-92116 EB92020-20053
        MB - Middle board  (GFX)               GP-9189  EB90015-20038
        LB - Lower board   (CPU/GFX)           GP-9188A EB90015-20037-1

[92 Arm Champs II]          68000   -       -       -       2xM6295

    Top board   EB91042-20048-3 AC-91106B
    Lower board EB91015-20038 GP-9189
    Same chips as Scud Hammer

[94 Scud Hammer]            68000   -       -       -       2xM6295

    Board CF-92128B Chips:
        GS9001501
        GS90015-02 x 2  GS600406   x 2      [Tilemaps]
        GS90015-03

    Board GP-9189 Chips:
        GS90015-03 x 2                      [Sprites]
        GS90015-06 x 2
        GS90015-07
        GS90015-08
        GS90015-09
        GS90015-10
        GS90015-11
        MR90015-35 x 2
---------------------------------------------------------------------------

----------------------------------------------------------------------------------
Main CPU                    [Cisco Heat]        [F1 GP Star]        [Scud Hammer]
----------------------------------------------------------------------------------
ROM                 R       000000-07ffff       <                   <
                            100000-17ffff       <                   RW : I / O + Sound
Work RAM + Sprites  RW      0f0000-0fffff       <                   <
Hardware Regs       RW      080000-087fff       <                   <
Units Linking RAM   RW      088000-88ffff       <                   -
Shared RAM #2       RW      090000-097fff       <                   -
Shared RAM #1       RW      098000-09ffff       <                   -
Scroll RAM 0        RW      0a0000-0a7fff       <                   <
Scroll RAM 1        RW      0a8000-0affff       <                   -
Scroll RAM 2        RW      0b0000-0b7fff       <                   <
Palette RAM         RW      0b8000-0bffff       <                   <
    Palette Scroll 0        0b9c00-0b9fff       0b9e00-0b9fff       <
    Palette Scroll 1        0bac00-0bafff       0bae00-0bafff       -
    Palette Road 0          0bb800-0bbfff       <                   -
    Palette Road 1          0bc800-0bcfff       <                   -
    Palette Sprites         0bd000-0bdfff       <                   0bb000-0bbfff
    Palette Scroll 2        0bec00-0befff0      0bee00-0befff       0bce00-0bcfff
----------------------------------------------------------------------------------

----------------------------------------------------------------
Sub CPU's                   [Cisco Heat]        [F1 GP Star]
----------------------------------------------------------------
ROM                 R       000000-03ffff       <
                            200000-23ffff       -
Work RAM            RW      0c0000-0c3fff       180000-183fff
Shared RAM          RW      040000-047fff       080000-087fff
Road RAM            RW      080000-0807ff       100000-1007ff
Whatchdog                   100000-100001       200000-200001
----------------------------------------------------------------

----------------------------------------------------------------
Sound CPU                   [Cisco Heat]        [F1 GP Star]
----------------------------------------------------------------
ROM                 R       000000-03ffff       <
Work RAM            RW      0f0000-0fffff       0e0000-0fffff
M6295 #1 Banking     W      040002-040003       040004-040005
M6295 #2 Banking     W      040004-040005       040008-040009
Sound Latch          W      060002-060003       060000-060001
Sound Latch         R       060004-060005       060000-060001
YM2151 Reg Sel       W      080001-080001       <
YM2151 Data          W      080003-080003       <
YM2151 Status       R       080003-080003       <
M6295 #1 Status     R       0a0001-0a0001       <
M6295 #1 Data        W      0a0000-0a0003       <
M6295 #2 Status     R       0c0001-0c0001       <
M6295 #2 Data        W      0c0000-0c0003       <
----------------------------------------------------------------

Cheats:

[cischeat]
-   f011a.w     *** stage - 1 ***
-   f0190.l     *** score / 10 (BCD) ***
-   f0280.w     *** time * 10 (seconds) ***
-   f61Xa.w     car X data

[f1gpstar]
-   Note: This game has some leftover code from Cisco Heat, it seems.
-   f9088.w     *** lap - 1 ***
-   fa008.w     ($fa000 + $08) *** time (seconds) ***
-   fa2aa.l     ($fa200 + $aa) speed << 16

Common Issues:

- Some ROMs aren't used (priorities?)
- Screen control register (priorities, layers enabling etc.) - Where is it?
- In cischeat & bigrun, at the start of some levels, you can see the empty
  scrolling layers as they are filled. In f1gpstar, I'm unsure whether they
  are correct in a few places (e.g. in the attract mode, where cars move
  horizontally, the wheels don't follow for this reason, I think
- Sound communication not quite right: see Test Mode

To Do:

- Priorities!
- Use the Tilemap Manager for the road layers (when this kind of layers
  will be supported) for performance and better priority support.
  A line based zooming is additionally needed for f1gpstar.
- Force feedback :)


2008-08
Dip locations verified for Big Run with the manual. Also added missing dips
and locations from Service mode to: Scud Hammer, F1 GP Star 1 & 2 and
Cisco Heat.

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "sound/2151intf.h"
#include "sound/okim6295.h"
#include "machine/jalcrpt.h"
#include "includes/cischeat.h"

#include "cischeat.lh"
#include "f1gpstar.lh"

/**************************************************************************


                        Memory Maps - Main CPU (#1)


**************************************************************************/


/**************************************************************************
                                Big Run
**************************************************************************/


static ADDRESS_MAP_START( bigrun_map, AS_PROGRAM, 16, cischeat_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM                                                                 // ROM
	AM_RANGE(0x080000, 0x083fff) AM_READWRITE(bigrun_vregs_r, bigrun_vregs_w) AM_SHARE("vregs") // Vregs
	AM_RANGE(0x084000, 0x087fff) AM_RAM                                                 // Linking with other units
	AM_RANGE(0x088000, 0x08bfff) AM_RAM AM_SHARE("share2") // Sharedram with sub CPU#2
	AM_RANGE(0x08c000, 0x08ffff) AM_RAM AM_SHARE("share1") // Sharedram with sub CPU#1

	/* Only writes to the first 0x40000 bytes affect the tilemaps:             */
	/* either these games support larger tilemaps or have more ram than needed */
	AM_RANGE(0x090000, 0x093fff) AM_WRITE(cischeat_scrollram_0_w) AM_SHARE("scrollram.0")       // Scroll ram 0
	AM_RANGE(0x094000, 0x097fff) AM_WRITE(cischeat_scrollram_1_w) AM_SHARE("scrollram.1")       // Scroll ram 1
	AM_RANGE(0x098000, 0x09bfff) AM_WRITE(cischeat_scrollram_2_w) AM_SHARE("scrollram.2")       // Scroll ram 2

	AM_RANGE(0x09c000, 0x09ffff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")            // Palettes
	AM_RANGE(0x0f0000, 0x0fffff) AM_RAM AM_SHARE("ram")                                         // RAM
	AM_RANGE(0x100000, 0x13ffff) AM_ROM AM_REGION("user1",0)                                                        // ROM
ADDRESS_MAP_END


/**************************************************************************
                                Cisco Heat
**************************************************************************/

/*  CISCO HEAT
    [  Test  ]      [  Real  ]
    b9c00-b9fff     <               scroll 0
    bac00-bafff     <               scroll 1
    bb800-bbbff     bb800-bbfff     road 0
    bc800-bcbff     bc800-bcfff     road 1
    bd000-bd3ff     bd000-bdfff     sprites
    bec00-befff     <               text        */


static ADDRESS_MAP_START( cischeat_map, AS_PROGRAM, 16, cischeat_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM                                                                     // ROM
	AM_RANGE(0x080000, 0x087fff) AM_READWRITE(cischeat_vregs_r, cischeat_vregs_w)  AM_SHARE("vregs")    // Vregs
	AM_RANGE(0x088000, 0x088fff) AM_RAM                                                                     // Linking with other units

/*  Only the first 0x800 bytes are tested but:
    CPU #0 PC 0000278c: warning - write 68c0 to unmapped memory address 0009c7fe
    CPU #0 PC 0000dd58: warning - read unmapped memory address 000945ac
    No mem access error from the other CPU's, though.. */

	/* this is the right order of sharedram's */
	AM_RANGE(0x090000, 0x097fff) AM_RAM AM_SHARE("share2") // Sharedram with sub CPU#2
	AM_RANGE(0x098000, 0x09ffff) AM_RAM AM_SHARE("share1") // Sharedram with sub CPU#1

	/* Only writes to the first 0x40000 bytes affect the tilemaps:             */
	/* either these games support larger tilemaps or have more ram than needed */
	AM_RANGE(0x0a0000, 0x0a7fff) AM_RAM_WRITE(cischeat_scrollram_0_w) AM_SHARE("scrollram.0")       // Scroll ram 0
	AM_RANGE(0x0a8000, 0x0affff) AM_RAM_WRITE(cischeat_scrollram_1_w) AM_SHARE("scrollram.1")       // Scroll ram 1
	AM_RANGE(0x0b0000, 0x0b7fff) AM_RAM_WRITE(cischeat_scrollram_2_w) AM_SHARE("scrollram.2")       // Scroll ram 2

	AM_RANGE(0x0b8000, 0x0bffff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")              // Palettes

	AM_RANGE(0x0f0000, 0x0fffff) AM_RAM AM_SHARE("ram")                                             // RAM
	AM_RANGE(0x100000, 0x17ffff) AM_ROM AM_REGION("user1",0)                                                            // ROM
ADDRESS_MAP_END


/**************************************************************************
                            F1 GrandPrix Star
**************************************************************************/


/*  F1 GP Star tests:
    0A0000-0B8000
    0F0000-100000
    0B8000-0C0000
    090800-091000
    098800-099000
    0F8000-0F9000   */

static ADDRESS_MAP_START( f1gpstar_map, AS_PROGRAM, 16, cischeat_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM                                                                     // ROM
	AM_RANGE(0x080000, 0x087fff) AM_READWRITE(f1gpstar_vregs_r, f1gpstar_vregs_w) AM_SHARE("vregs") // Vregs
	AM_RANGE(0x088000, 0x088fff) AM_RAM                                                                     // Linking with other units

	AM_RANGE(0x090000, 0x097fff) AM_RAM AM_SHARE("share2") // Sharedram with sub CPU#2
	AM_RANGE(0x098000, 0x09ffff) AM_RAM AM_SHARE("share1") // Sharedram with sub CPU#1

	/* Only writes to the first 0x40000 bytes affect the tilemaps:             */
	/* either these games support larger tilemaps or have more ram than needed */
	AM_RANGE(0x0a0000, 0x0a7fff) AM_RAM_WRITE(cischeat_scrollram_0_w) AM_SHARE("scrollram.0")       // Scroll ram 0
	AM_RANGE(0x0a8000, 0x0affff) AM_RAM_WRITE(cischeat_scrollram_1_w) AM_SHARE("scrollram.1")       // Scroll ram 1
	AM_RANGE(0x0b0000, 0x0b7fff) AM_RAM_WRITE(cischeat_scrollram_2_w) AM_SHARE("scrollram.2")       // Scroll ram 2

	AM_RANGE(0x0b8000, 0x0bffff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")              // Palettes

	AM_RANGE(0x0f0000, 0x0fffff) AM_RAM AM_SHARE("ram")                                             // RAM
	AM_RANGE(0x100000, 0x17ffff) AM_ROM AM_REGION("user1",0)                                                            // ROM
ADDRESS_MAP_END


/**************************************************************************
                            F1 GrandPrix Star II
**************************************************************************/

// Same as f1gpstar, but vregs are slightly different:
static ADDRESS_MAP_START( f1gpstr2_map, AS_PROGRAM, 16, cischeat_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM                                                                     // ROM
	AM_RANGE(0x080000, 0x087fff) AM_READWRITE(f1gpstr2_vregs_r, f1gpstr2_vregs_w) AM_SHARE("vregs") // Vregs (slightly different from f1gpstar)
	AM_RANGE(0x088000, 0x088fff) AM_RAM                                                                     // Linking with other units

	AM_RANGE(0x090000, 0x097fff) AM_RAM AM_SHARE("share2") // Sharedram with sub CPU#2
	AM_RANGE(0x098000, 0x09ffff) AM_RAM AM_SHARE("share1") // Sharedram with sub CPU#1

	/* Only writes to the first 0x40000 bytes affect the tilemaps:             */
	/* either these games support larger tilemaps or have more ram than needed */
	AM_RANGE(0x0a0000, 0x0a7fff) AM_RAM_WRITE(cischeat_scrollram_0_w) AM_SHARE("scrollram.0")       // Scroll ram 0
	AM_RANGE(0x0a8000, 0x0affff) AM_RAM_WRITE(cischeat_scrollram_1_w) AM_SHARE("scrollram.1")       // Scroll ram 1
	AM_RANGE(0x0b0000, 0x0b7fff) AM_RAM_WRITE(cischeat_scrollram_2_w) AM_SHARE("scrollram.2")       // Scroll ram 2

	AM_RANGE(0x0b8000, 0x0bffff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")               // Palettes

	AM_RANGE(0x0f0000, 0x0fffff) AM_RAM AM_SHARE("ram")                                             // RAM
	AM_RANGE(0x100000, 0x17ffff) AM_ROM AM_REGION("user1",0)                                                            // ROM
ADDRESS_MAP_END


/**************************************************************************
                            Scud Hammer
**************************************************************************/



/*  Motor Status.

    f--- ---- ---- ----     Rotation Limit (R?)
    -e-- ---- ---- ----     Rotation Limit (L?)
    --dc ba98 7654 32--
    ---- ---- ---- --1-     Up Limit
    ---- ---- ---- ---0     Down Limit  */

READ16_MEMBER(cischeat_state::scudhamm_motor_status_r)
{
	return m_scudhamm_motor_command;    // Motor Status
}


READ16_MEMBER(cischeat_state::scudhamm_motor_pos_r)
{
	return 0x00 << 8;
}


/*  Move the motor.

    fedc ba98 7654 32--
    ---- ---- ---- --1-     Move Up
    ---- ---- ---- ---0     Move Down

    Within $20 vblanks the motor must reach the target. */

WRITE16_MEMBER(cischeat_state::scudhamm_motor_command_w)
{
	COMBINE_DATA( &m_scudhamm_motor_command );
}


READ16_MEMBER(cischeat_state::scudhamm_analog_r)
{
	int i=ioport("IN1")->read(),j;

	if ((i^m_prev)&0x4000) {
		if (i<m_prev) m_prev-=0x8000;
		else m_prev+=0x8000;
	}

	j=i-m_prev;
	m_prev=i;

	/* effect of hammer collision 'accelerometer':
	$00 - $09 - no hit
	$0A - $3F - soft hit
	$40 - $FF - hard hit */
	if (j<0) return 0;
	else if (j>0xff) return 0xff;
	return j;
}

/*
    I don't know how many leds are there, but each bit in the buttons input
    port (coins, tilt, buttons, select etc.) triggers the corresponding bit
    in this word. I mapped the 3 buttons to the first 3 led.
*/
WRITE16_MEMBER(cischeat_state::scudhamm_leds_w)
{
	if (ACCESSING_BITS_8_15)
	{
		set_led_status(machine(), 0, data & 0x0100);    // 3 buttons
		set_led_status(machine(), 1, data & 0x0200);
		set_led_status(machine(), 2, data & 0x0400);
	}

	if (ACCESSING_BITS_0_7)
	{
//      set_led_status(machine(), 3, data & 0x0010);   // if we had more leds..
//      set_led_status(machine(), 4, data & 0x0020);
	}
}


/*
    $FFFC during self test, $FFFF onwards.
    It could be audio(L/R) or layers(0/2) enable.
*/
WRITE16_MEMBER(cischeat_state::scudhamm_enable_w)
{
}


WRITE16_MEMBER(cischeat_state::scudhamm_oki_bank_w)
{
	if (ACCESSING_BITS_0_7)
	{
		m_oki1->set_bank_base(0x40000 * ((data >> 0) & 0x3) );
		m_oki2->set_bank_base(0x40000 * ((data >> 4) & 0x3) );
	}
}

static ADDRESS_MAP_START( scudhamm_map, AS_PROGRAM, 16, cischeat_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM                                                                 // ROM
	AM_RANGE(0x082000, 0x082fff) AM_RAM_WRITE(scudhamm_vregs_w) AM_SHARE("vregs")               // Video Registers + RAM
	AM_RANGE(0x0a0000, 0x0a3fff) AM_RAM_WRITE(cischeat_scrollram_0_w) AM_SHARE("scrollram.0")   // Scroll RAM 0
	AM_RANGE(0x0b0000, 0x0b3fff) AM_RAM_WRITE(cischeat_scrollram_2_w) AM_SHARE("scrollram.2")   // Scroll RAM 2
	AM_RANGE(0x0b8000, 0x0bffff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")          // Palette
	AM_RANGE(0x0f0000, 0x0fffff) AM_RAM AM_SHARE("ram")                                         // Work RAM + Spriteram
	AM_RANGE(0x100000, 0x100001) AM_WRITE(scudhamm_oki_bank_w)                                          // Sound
	AM_RANGE(0x100008, 0x100009) AM_READ_PORT("IN0") AM_WRITE(scudhamm_leds_w)                          // Buttons
	AM_RANGE(0x100014, 0x100015) AM_DEVREADWRITE8("oki1", okim6295_device, read, write, 0x00ff)             // Sound
	AM_RANGE(0x100018, 0x100019) AM_DEVREADWRITE8("oki2", okim6295_device, read, write, 0x00ff)             //
	AM_RANGE(0x10001c, 0x10001d) AM_WRITE(scudhamm_enable_w)                                            // ?
	AM_RANGE(0x100040, 0x100041) AM_READ(scudhamm_analog_r) AM_WRITENOP                         // A / D
	AM_RANGE(0x100044, 0x100045) AM_READ(scudhamm_motor_pos_r)                                  // Motor Position
	AM_RANGE(0x100050, 0x100051) AM_READ(scudhamm_motor_status_r) AM_WRITE(scudhamm_motor_command_w)        // Motor Limit Switches
	AM_RANGE(0x10005c, 0x10005d) AM_READ_PORT("IN2")                                                    // 2 x DSW
ADDRESS_MAP_END


/**************************************************************************
                            Arm Champs II
**************************************************************************/

READ16_MEMBER(cischeat_state::armchmp2_motor_status_r)
{
	return 0x11;
}

WRITE16_MEMBER(cischeat_state::armchmp2_motor_command_w)
{
	COMBINE_DATA( &m_scudhamm_motor_command );
}

READ16_MEMBER(cischeat_state::armchmp2_analog_r)
{
	int armdelta;

	armdelta = ioport("IN1")->read() - m_armold;
	m_armold = ioport("IN1")->read();

	return ~( m_scudhamm_motor_command + armdelta );    // + x : x<=0 and player loses, x>0 and player wins
}

READ16_MEMBER(cischeat_state::armchmp2_buttons_r)
{
	int arm_x = ioport("IN1")->read();

	UINT16 ret = ioport("IN0")->read();

	if (arm_x < 0x40)       ret &= ~1;
	else if (arm_x > 0xc0)  ret &= ~2;
	else if ((arm_x > 0x60) && (arm_x < 0xa0))  ret &= ~4;

	return ret;
}

/*
    f--- ---- ---- ----
    -edc ---- ---- ----     Leds
    ---- ba9- ---- ----
    ---- ---8 ---- ----     Start button led
    ---- ---- 76-- ----     Coin counters
    ---- ---- --54 3210
*/
WRITE16_MEMBER(cischeat_state::armchmp2_leds_w)
{
	if (ACCESSING_BITS_8_15)
	{
		set_led_status(machine(), 0, data & 0x0100);
		set_led_status(machine(), 1, data & 0x1000);
		set_led_status(machine(), 2, data & 0x2000);
		set_led_status(machine(), 3, data & 0x4000);
	}

	if (ACCESSING_BITS_0_7)
	{
		coin_counter_w(machine(), 0, data & 0x0040);
		coin_counter_w(machine(), 1, data & 0x0080);
	}
}

static ADDRESS_MAP_START( armchmp2_map, AS_PROGRAM, 16, cischeat_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM                                                                 // ROM
	AM_RANGE(0x082000, 0x082fff) AM_RAM_WRITE(scudhamm_vregs_w) AM_SHARE("vregs")               // Video Registers + RAM
	AM_RANGE(0x0a0000, 0x0a3fff) AM_RAM_WRITE(cischeat_scrollram_0_w) AM_SHARE("scrollram.0")   // Scroll RAM 0
	AM_RANGE(0x0b0000, 0x0b3fff) AM_RAM_WRITE(cischeat_scrollram_2_w) AM_SHARE("scrollram.2")   // Scroll RAM 2
	AM_RANGE(0x0b8000, 0x0bffff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")           // Palette
	AM_RANGE(0x0f0000, 0x0fffff) AM_RAM AM_SHARE("ram")                                         // Work RAM + Spriteram
	AM_RANGE(0x100000, 0x100001) AM_READ_PORT("IN2") AM_WRITE(scudhamm_oki_bank_w)                      // DSW + Sound
	AM_RANGE(0x100004, 0x100005) AM_READ_PORT("IN3")                                                    // DSW
	AM_RANGE(0x100008, 0x100009) AM_READWRITE(armchmp2_buttons_r, armchmp2_leds_w)                      // Leds + Coin Counters + Buttons + Sensors
	AM_RANGE(0x10000c, 0x10000d) AM_READ(armchmp2_analog_r) AM_WRITENOP                         // A / D
	AM_RANGE(0x100010, 0x100011) AM_READWRITE(armchmp2_motor_status_r, armchmp2_motor_command_w)        // Motor Limit Switches?
	AM_RANGE(0x100014, 0x100015) AM_DEVREADWRITE8("oki1", okim6295_device, read, write, 0x00ff  )           // Sound
	AM_RANGE(0x100018, 0x100019) AM_DEVREADWRITE8("oki2", okim6295_device, read, write, 0x00ff  )           //
ADDRESS_MAP_END


/**************************************************************************


                    Memory Maps - Road CPUs (#2 & #3)


**************************************************************************/

/**************************************************************************
                                Big Run
**************************************************************************/

static ADDRESS_MAP_START( bigrun_map2, AS_PROGRAM, 16, cischeat_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM                                                 // ROM
	AM_RANGE(0x040000, 0x047fff) AM_RAM AM_SHARE("share1")                              // Shared RAM (with Main CPU)
	AM_RANGE(0x080000, 0x0807ff) AM_RAM AM_SHARE("roadram.0")   // Road RAM
	AM_RANGE(0x0c0000, 0x0c3fff) AM_RAM                                                 // RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( bigrun_map3, AS_PROGRAM, 16, cischeat_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM                                                 // ROM
	AM_RANGE(0x040000, 0x047fff) AM_RAM AM_SHARE("share2")                              // Shared RAM (with Main CPU)
	AM_RANGE(0x080000, 0x0807ff) AM_RAM AM_SHARE("roadram.1")   // Road RAM
	AM_RANGE(0x0c0000, 0x0c3fff) AM_RAM                                                 // RAM
ADDRESS_MAP_END


/**************************************************************************
                                Cisco Heat
**************************************************************************/

static ADDRESS_MAP_START( cischeat_map2, AS_PROGRAM, 16, cischeat_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM                                                 // ROM
	AM_RANGE(0x040000, 0x047fff) AM_RAM AM_SHARE("share1")                              // Shared RAM (with Main CPU)
	AM_RANGE(0x080000, 0x0807ff) AM_RAM AM_SHARE("roadram.0")   // Road RAM
	AM_RANGE(0x0c0000, 0x0c3fff) AM_RAM                                                 // RAM
	AM_RANGE(0x100000, 0x100001) AM_WRITENOP                                            // watchdog
	AM_RANGE(0x200000, 0x23ffff) AM_ROM AM_REGION("cpu2",0x40000)                                       // ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( cischeat_map3, AS_PROGRAM, 16, cischeat_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM                                                 // ROM
	AM_RANGE(0x040000, 0x047fff) AM_RAM AM_SHARE("share2")                              // Shared RAM (with Main CPU)
	AM_RANGE(0x080000, 0x0807ff) AM_RAM AM_SHARE("roadram.1")   // Road RAM
	AM_RANGE(0x0c0000, 0x0c3fff) AM_RAM                                                 // RAM
	AM_RANGE(0x100000, 0x100001) AM_WRITENOP                                            // watchdog
	AM_RANGE(0x200000, 0x23ffff) AM_ROM AM_REGION("cpu3",0x40000)                                       // ROM
ADDRESS_MAP_END



/**************************************************************************
                            F1 GrandPrix Star
**************************************************************************/

static ADDRESS_MAP_START( f1gpstar_map2, AS_PROGRAM, 16, cischeat_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM                                                 // ROM
	AM_RANGE(0x080000, 0x0807ff) AM_RAM AM_SHARE("share1")                              // Shared RAM (with Main CPU)
	AM_RANGE(0x100000, 0x1007ff) AM_RAM AM_SHARE("roadram.0")   // Road RAM
	AM_RANGE(0x180000, 0x183fff) AM_RAM                                                 // RAM
	AM_RANGE(0x200000, 0x200001) AM_WRITENOP                                            // watchdog
ADDRESS_MAP_END

static ADDRESS_MAP_START( f1gpstar_map3, AS_PROGRAM, 16, cischeat_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM                                                 // ROM
	AM_RANGE(0x080000, 0x0807ff) AM_RAM AM_SHARE("share2")                              // Shared RAM (with Main CPU)
	AM_RANGE(0x100000, 0x1007ff) AM_RAM AM_SHARE("roadram.1")   // Road RAM
	AM_RANGE(0x180000, 0x183fff) AM_RAM                                                 // RAM
	AM_RANGE(0x200000, 0x200001) AM_WRITENOP                                            // watchdog
ADDRESS_MAP_END


/**************************************************************************


                        Memory Maps - Sound CPU (#4)


**************************************************************************/

/* Music tempo driven by the YM2151 timers (status reg polled) */


/**************************************************************************
                                Big Run
**************************************************************************/

WRITE16_MEMBER(cischeat_state::bigrun_soundbank_w)
{
	if (ACCESSING_BITS_0_7)
	{
		m_oki1->set_bank_base(0x40000 * ((data >> 0) & 1) );
		m_oki2->set_bank_base(0x40000 * ((data >> 4) & 1) );
	}
}

static ADDRESS_MAP_START( bigrun_sound_map, AS_PROGRAM, 16, cischeat_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM                                                 // ROM
	AM_RANGE(0x040000, 0x040001) AM_READ(soundlatch_word_r) AM_WRITE(bigrun_soundbank_w)    // From Main CPU
	AM_RANGE(0x060000, 0x060001) AM_WRITE(soundlatch2_word_w)                           // To Main CPU
	AM_RANGE(0x080000, 0x080003) AM_DEVREADWRITE8("ymsnd", ym2151_device, read, write, 0x00ff)
	AM_RANGE(0x0a0000, 0x0a0003) AM_DEVREADWRITE8("oki1", okim6295_device, read, write, 0x00ff)
	AM_RANGE(0x0c0000, 0x0c0003) AM_DEVREADWRITE8("oki2", okim6295_device, read, write, 0x00ff)
	AM_RANGE(0x0f0000, 0x0fffff) AM_RAM                                                 // RAM
ADDRESS_MAP_END


/**************************************************************************
                                Cisco Heat
**************************************************************************/

WRITE16_MEMBER(cischeat_state::cischeat_soundbank_1_w)
{
	if (ACCESSING_BITS_0_7) m_oki1->set_bank_base(0x40000 * (data & 1) );
}

WRITE16_MEMBER(cischeat_state::cischeat_soundbank_2_w)
{
	if (ACCESSING_BITS_0_7) m_oki2->set_bank_base(0x40000 * (data & 1) );
}
static ADDRESS_MAP_START( cischeat_sound_map, AS_PROGRAM, 16, cischeat_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM                                                 // ROM
	AM_RANGE(0x040002, 0x040003) AM_WRITE(cischeat_soundbank_1_w)               // Sample Banking
	AM_RANGE(0x040004, 0x040005) AM_WRITE(cischeat_soundbank_2_w)               // Sample Banking
	AM_RANGE(0x060002, 0x060003) AM_WRITE(soundlatch2_word_w)                           // To Main CPU
	AM_RANGE(0x060004, 0x060005) AM_READ(soundlatch_word_r)                             // From Main CPU
	AM_RANGE(0x080000, 0x080003) AM_DEVREADWRITE8("ymsnd", ym2151_device, read, write, 0x00ff)
	AM_RANGE(0x0a0000, 0x0a0003) AM_DEVREADWRITE8("oki1", okim6295_device, read, write, 0x00ff)
	AM_RANGE(0x0c0000, 0x0c0003) AM_DEVREADWRITE8("oki2", okim6295_device, read, write, 0x00ff)
	AM_RANGE(0x0f0000, 0x0fffff) AM_RAM                                                 // RAM
ADDRESS_MAP_END


/**************************************************************************
                            F1 GrandPrix Star
**************************************************************************/

static ADDRESS_MAP_START( f1gpstar_sound_map, AS_PROGRAM, 16, cischeat_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM                                                 // ROM
	AM_RANGE(0x040004, 0x040005) AM_WRITE(cischeat_soundbank_1_w)               // Sample Banking   (cischeat: 40002)
	AM_RANGE(0x040008, 0x040009) AM_WRITE(cischeat_soundbank_2_w)               // Sample Banking   (cischeat: 40004)
	AM_RANGE(0x060000, 0x060001) AM_READWRITE(soundlatch_word_r, soundlatch2_word_w)    // From Main CPU    (cischeat: 60004)
	AM_RANGE(0x080000, 0x080003) AM_DEVREADWRITE8("ymsnd", ym2151_device, read, write, 0x00ff)
	AM_RANGE(0x0a0000, 0x0a0003) AM_DEVREADWRITE8("oki1", okim6295_device, read, write, 0x00ff)
	AM_RANGE(0x0c0000, 0x0c0003) AM_DEVREADWRITE8("oki2", okim6295_device, read, write, 0x00ff)
	AM_RANGE(0x0e0000, 0x0fffff) AM_RAM                                                 // RAM              (cischeat: f0000-fffff)
ADDRESS_MAP_END


/**************************************************************************
                            F1 GrandPrix Star II
**************************************************************************/

static ADDRESS_MAP_START( f1gpstr2_sound_map, AS_PROGRAM, 16, cischeat_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM // ROM
	AM_RANGE(0x040004, 0x040005) AM_WRITE(cischeat_soundbank_1_w)                   // Sample Banking
	AM_RANGE(0x040008, 0x040009) AM_WRITE(cischeat_soundbank_2_w)                   // Sample Banking
	AM_RANGE(0x04000e, 0x04000f) AM_WRITENOP                                            // ? 0              (f1gpstar: no)
	AM_RANGE(0x060004, 0x060005) AM_READWRITE(soundlatch_word_r, soundlatch2_word_w)        // From Main CPU    (f1gpstar: 60000)
	AM_RANGE(0x080000, 0x080003) AM_DEVREADWRITE8("ymsnd", ym2151_device, read, write, 0x00ff)
	AM_RANGE(0x0a0000, 0x0a0003) AM_DEVREADWRITE8("oki1", okim6295_device, read, write, 0x00ff)
	AM_RANGE(0x0c0000, 0x0c0003) AM_DEVREADWRITE8("oki2", okim6295_device, read, write, 0x00ff)
	AM_RANGE(0x0e0000, 0x0fffff) AM_RAM                                                     // RAM
ADDRESS_MAP_END

/**************************************************************************


                        Memory Maps - IO CPU (#5)


**************************************************************************/

READ16_MEMBER(cischeat_state::f1gpstr2_io_r)
{
	return m_vregs[offset + 0x1000/2];
}

WRITE16_MEMBER(cischeat_state::f1gpstr2_io_w)
{
	COMBINE_DATA(&m_vregs[offset + 0x1000/2]);
}

static ADDRESS_MAP_START( f1gpstr2_io_map, AS_PROGRAM, 16, cischeat_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM                                         // ROM
	AM_RANGE(0x080000, 0x080fff) AM_READWRITE(f1gpstr2_io_r, f1gpstr2_io_w)     //
	AM_RANGE(0x100000, 0x100001) AM_WRITEONLY AM_SHARE("ioready")   //
	AM_RANGE(0x180000, 0x183fff) AM_RAM                                         // RAM
	AM_RANGE(0x200000, 0x200001) AM_WRITENOP                                //
ADDRESS_MAP_END


/***************************************************************************


                                Input Ports


***************************************************************************/

/* Fake input port to read the status of five buttons: used to
   implement the shift using 2 buttons, and the accelerator in
   f1gpstar */

/**************************************************************************
                                Big Run
**************************************************************************/

//  Input Ports:    [0] Fake: Buttons Status
//                  [1] Coins       [2] Controls    [3] Unknown
//                  [4] DSW 1 & 2   [5] DSW 3       [6] Driving Wheel

static INPUT_PORTS_START( bigrun )
	PORT_START("IN1")   // Coins - $80000.w
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x08, IP_ACTIVE_LOW )   // called "Test"
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")   //Controls - $80002.w
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Brake")  // Brake
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Gear Shift") PORT_TOGGLE // Shift
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P1 Horn")   // Horn

	PORT_START("IN3")   // Motor Control? - $80004.w
	PORT_DIPNAME( 0x01, 0x01, "Up Limit SW" )   // Limit the Cockpit movements?
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Down Limit SW" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
//  PORT_DIPNAME( 0x10, 0x10, "Right Limit SW" )
//  PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( On ) )
//  PORT_DIPNAME( 0x20, 0x20, "Left Limit SW" )
//  PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN4")   // DSW 2 & 3 - $80006.w
	// DSW 3
	PORT_DIPNAME( 0x0003, 0x0003, "Extra Setting For Coin B" ) PORT_DIPLOCATION("SW302:8,7")    /* 'Not used' (and must be OFF) according to the manual */
	PORT_DIPSETTING(      0x0003, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 1C_5C ) )    PORT_CONDITION("IN4", 0x1c00, NOTEQUALS, 0x0000)
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_6C ) )    PORT_CONDITION("IN4", 0x1c00, NOTEQUALS, 0x0000)
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_7C ) )    PORT_CONDITION("IN4", 0x1c00, NOTEQUALS, 0x0000)
	PORT_DIPSETTING(      0x0001, DEF_STR( Unused ) )   PORT_CONDITION("IN4", 0x1c00, EQUALS, 0x0000)
	PORT_DIPSETTING(      0x0002, DEF_STR( Unused ) )   PORT_CONDITION("IN4", 0x1c00, EQUALS, 0x0000)
	PORT_DIPSETTING(      0x0000, DEF_STR( Unused ) )   PORT_CONDITION("IN4", 0x1c00, EQUALS, 0x0000)
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW302:6")
	PORT_DIPSETTING(      0x0004, DEF_STR( No )  )
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Region ) ) PORT_DIPLOCATION("SW302:5")   // If you try to change Unit ID the game show the error message in different language
	PORT_DIPSETTING(      0x0008, DEF_STR( Japanese ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( English) )
	PORT_DIPNAME( 0x0010, 0x0010, "Move Cabinet" ) PORT_DIPLOCATION("SW302:4")
	PORT_DIPSETTING(      0x0000, DEF_STR( No )  )
	PORT_DIPSETTING(      0x0010, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0060, 0x0060, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW302:3,2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Automatic Game Start" ) PORT_DIPLOCATION("SW302:1")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0080, "After 15 Seconds" )

	// DSW 2
	PORT_DIPNAME( 0x0100, 0x0100, "Invulnerability" ) PORT_DIPLOCATION("SW301:8")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW301:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( On ) )
	PORT_DIPNAME( 0x1c00, 0x1c00, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW301:6,5,4") PORT_CONDITION("IN4", 0x0003, EQUALS, 0x0003)
	PORT_DIPSETTING(      0x1000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x1c00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x1400, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x1c00, 0x1c00, "Set Coin B" ) PORT_DIPLOCATION("SW301:6,5,4") PORT_CONDITION("IN4", 0x0003, NOTEQUALS, 0x0003)
	PORT_DIPSETTING(      0x1c00, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0xe000, 0xe000, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW301:3,2,1")
	PORT_DIPSETTING(      0x8000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0xe000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0xa000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )

	PORT_START("IN5")   // DSW 3 (4 bits, Cabinet Linking) - $82200.w
	/* According to Manual: "When Machines are linked for simultaneous racing throught the
	Com-Link System, SW1 should be set as shown", i.e. ON for Master Machine, OFF for others */
	PORT_DIPNAME( 0x01, 0x00, "Link ID" ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, "Master Unit" )
	PORT_DIPSETTING(    0x01, "Other Units" )
	PORT_DIPNAME( 0x06, 0x00, "Unit ID" ) PORT_DIPLOCATION("SW1:2,3")
	PORT_DIPSETTING(    0x00, "1 (Blue-White Car)" )
	PORT_DIPSETTING(    0x02, "2 (Green-White Car)" )
	PORT_DIPSETTING(    0x04, "3 (Red-White Car)" )
	PORT_DIPSETTING(    0x06, "4 (Yellow Car)" )
	PORT_DIPUNKNOWN_DIPLOC(  0x08, 0x00, "SW1:4")       /* Always ON according to manual */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN6")   // Driving Wheel - $80010.w(0)
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_SENSITIVITY(30) PORT_KEYDELTA(30)

	PORT_START("PEDAL") // Accelerator Pedal
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(100) PORT_KEYDELTA(40)
INPUT_PORTS_END


/**************************************************************************
                                Cisco Heat
**************************************************************************/

//  Input Ports:    [0] Fake: Buttons Status
//                  [1] Coins       [2] Controls    [3] Unknown
//                  [4] DSW 1 & 2   [5] DSW 3       [6] Driving Wheel

static INPUT_PORTS_START( cischeat )
	PORT_START("IN1")   // Coins - $80000.w
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x08, IP_ACTIVE_LOW)    // called "Test"
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")   // Controls - $80002.w
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Brake")  // Brake
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P1 Gear Shift") PORT_TOGGLE // Shift
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Accelerator")    // Accel
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P1 Horn")   // Horn

	PORT_START("IN3")   // Motor Control? - $80004.w
	PORT_DIPNAME( 0x01, 0x01, "Up Limit SW" )   // Limit the Cockpit movements?
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Down Limit SW" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x10, 0x10, "Right Limit SW" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Left Limit SW" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN4")   // DSW 1 & 2 - $80006.w -> !f000a.w(hi byte) !f0008.w(low byte)
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW301:1,2,3")
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW301:4,5,6")
	PORT_DIPSETTING(    0x08, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW301:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW301:8" )
	// DSW 2
	PORT_DIPNAME( 0x0300, 0x0300, "Unit ID" ) PORT_DIPLOCATION("SW302:1,2") // -> !f0020 (ID of this unit, when linked)
	PORT_DIPSETTING(      0x0300, "0 (Red Car)" )
	PORT_DIPSETTING(      0x0200, "1 (Blue Car)" )
	PORT_DIPSETTING(      0x0100, "2 (Yellow Car)" )
	PORT_DIPSETTING(      0x0000, "3 (Green Car)" )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW302:3,4") // -> !f0026
	PORT_DIPSETTING(      0x0000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x1000, 0x1000, "Infinite Time (Cheat)" )  PORT_DIPLOCATION("SW302:5")    // -> !f0028
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW302:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Country" ) PORT_DIPLOCATION("SW302:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Japan ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( USA ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW302:8")   // -> !f00c0
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("IN5")   // DSW 3 (4 bits, Cabinet Linking) - $82200.w
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW1:1" )
	PORT_DIPNAME( 0x06, 0x06, "Unit ID (2)" ) PORT_DIPLOCATION("SW1:2,3")   // -> f0020 (like DSW2 !!)
	PORT_DIPSETTING(    0x06, "Use other" )
	PORT_DIPSETTING(    0x00, "0 (Red Car)" )
	PORT_DIPSETTING(    0x02, "1 (Blue Car)" )
	PORT_DIPSETTING(    0x04, "2 (Yellow Car)" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW1:4" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN6")   // IN6 - Driving Wheel - $80010.w(0)
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_SENSITIVITY(30) PORT_KEYDELTA(30)
INPUT_PORTS_END


/**************************************************************************
                                F1 GrandPrix Star
**************************************************************************/

//  Input Ports:    [0] Fake: Buttons Status
//                  [1] DSW 1 & 2       [2] Controls        [3] Unknown
//                  [4] DSW 3           [5] Driving Wheel

static INPUT_PORTS_START( f1gpstar )
/*  [Country]
    Japan       "race together" in Test Mode, Always Choose Race
                Japanese, Km/h, "handle shock"  , "(c)1991",
    USA         English,  Mph , "steering shock", "(c)1992 North America Only"
    Europe      English,  Mph , "steering shock", "(c)1992"
    France      French,   Km/h, "steering shock", "(c)1992" */

	PORT_START("IN1")   // DSW 1 & 2 - $80000.w -> !f9012
	// DSW 1
	// Coinage Japan & USA (it changes with Country)
	PORT_DIPNAME( 0x0007, 0x0007, "Coin A (JP US)" ) PORT_DIPLOCATION("SW01:1,2,3") PORT_CONDITION("IN1", 0x0300, GREATERTHAN, 0x0100)
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0038, 0x0038, "Coin B (JP US)" ) PORT_DIPLOCATION("SW01:4,5,6") PORT_CONDITION("IN1", 0x0300, GREATERTHAN, 0x0100)
	PORT_DIPSETTING(      0x0008, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW01:7") PORT_CONDITION("IN1", 0x0300, GREATERTHAN, 0x0100)    // unused?
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	// Coinage Europe & France (it changes with Country)
	PORT_DIPNAME( 0x0007, 0x0007, "Coin A (EU FR)" ) PORT_DIPLOCATION("SW01:1,2,3") PORT_CONDITION("IN1", 0x0300, NOTGREATERTHAN, 0x0100)
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 1C_7C ) )
	PORT_DIPNAME( 0x0038, 0x0038, "Coin B (EU FR)" ) PORT_DIPLOCATION("SW01:4,5,6") PORT_CONDITION("IN1", 0x0300, NOTGREATERTHAN, 0x0100)
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Free Play (EU & FR)" ) PORT_DIPLOCATION("SW01:7") PORT_CONDITION("IN1", 0x0300, NOTGREATERTHAN, 0x0100)
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_DIPUNKNOWN_DIPLOC( 0x0080, 0x0080, "SW01:8" )  // unused?

	// DSW 2
	PORT_DIPNAME( 0x0300, 0x0100, DEF_STR( Region ) ) PORT_DIPLOCATION("SW02:1,2")  // -> !f901e
	PORT_DIPSETTING(      0x0300, DEF_STR( Japan ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( USA ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Europe ) )
	PORT_DIPSETTING(      0x0000, "France" )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW02:3,4")  // -> !f9026
	PORT_DIPSETTING(      0x0000, DEF_STR( Easy ) )         // 58 <- Initial Time (seconds, Germany)
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal ) )       // 51
	PORT_DIPSETTING(      0x0800, DEF_STR( Hard ) )         // 48
	PORT_DIPSETTING(      0x0400, DEF_STR( Very_Hard ) )    // 46
	PORT_DIPNAME( 0x1000, 0x1000, "Infinite Time (Cheat)" ) PORT_DIPLOCATION("SW02:5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW02:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW02:7") PORT_CONDITION("IN1", 0x0300, EQUALS, 0x0300) // unused?
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Choose Race (US EU FR)" ) PORT_DIPLOCATION("SW02:7") PORT_CONDITION("IN1", 0x0300, NOTEQUALS, 0x0300)    // -> f0020
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, "Vibrations" ) PORT_DIPLOCATION("SW02:8")
	PORT_DIPSETTING(      0x8000, "Torque" )
	PORT_DIPSETTING(      0x0000, "Shake" )

	PORT_START("IN2")   // Controls - $80004.w -> !f9016
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x0008, IP_ACTIVE_LOW) // -> f0100 (called "Test")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Gear Shift") PORT_TOGGLE // Shift
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Brake")// Brake -> !f9010
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2 ) // "Race Together"

	PORT_START("IN3")   // ? Read at boot only - $80006.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

/*  DSW3-2&1 (Country: JP)  Effect
    OFF-OFF                 Red-White Car
    OFF- ON                 Red Car
    ON-OFF                  Blue-White Car
    ON- ON                  Blue Car, "equipped with communication link"    */

	PORT_START("IN4")   // DSW 3 (4 bits, Cabinet Linking) - $8000c.w -> !f9014
	PORT_DIPNAME( 0x01, 0x01, "This Unit Is" ) PORT_DIPLOCATION("SW03:1")
	PORT_DIPSETTING(    0x01, "Slave" )
	PORT_DIPSETTING(    0x00, "Master" )
	PORT_DIPNAME( 0x06, 0x06, "Unit ID" ) PORT_DIPLOCATION("SW03:2,3")          // -> !f901c
	PORT_DIPSETTING(    0x06, "0 (Red-White Car)" )
	PORT_DIPSETTING(    0x04, "1 (Red Car)" )
	PORT_DIPSETTING(    0x02, "2 (Blue-White Car)" )
	PORT_DIPSETTING(    0x00, "3 (Blue Car)" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW03:4" )  // Redundant: Invert Unit ID
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW03:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW03:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW03:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW03:8" )

						//       Accelerator   - $80010.b ->  !f9004.w
	PORT_START("IN5")   // Driving Wheel - $80011.b ->  !f9008.w
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_SENSITIVITY(30) PORT_KEYDELTA(30)

	PORT_START("PEDAL") // Accelerator Pedal
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(100) PORT_KEYDELTA(40) PORT_REVERSE
INPUT_PORTS_END


/**************************************************************************
                                Wild Pilot
**************************************************************************/

static INPUT_PORTS_START( wildplt )
	PORT_START("IN0")   // DSW 1 & 2
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coinage ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x2000, "2" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPNAME( 0xc000, 0xc000, DEF_STR( Region ) )
	PORT_DIPSETTING(      0x4000, "Europe?" )
	PORT_DIPSETTING(      0x8000, DEF_STR( USA ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( Japan ) )
	PORT_DIPSETTING(      0x0000, "France?" )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 ) //service 1 too
	PORT_SERVICE_NO_TOGGLE( 0x0008, IP_ACTIVE_LOW ) //start 2 too
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN ) // Up Limit SW.
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN ) // Dow Limit SW.
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN ) // Center SW.
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN ) // Senser SW. #1
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN ) // Senser SW. #2
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Emergency Button") //E Stop for motors? ( Senser SW. #3 )

	PORT_START("IN2")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_SENSITIVITY(35) PORT_KEYDELTA(15) PORT_REVERSE

	PORT_START("IN3")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_SENSITIVITY(35) PORT_KEYDELTA(15)
INPUT_PORTS_END


/**************************************************************************
                                Scud Hammer
**************************************************************************/

static INPUT_PORTS_START( scudhamm )
	PORT_START("IN0")   // Buttons
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN  ) // GAME OVER if pressed on the selection screen
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x0008, IP_ACTIVE_LOW )     // called "Test"
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON4 ) // Select
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_BUTTON1 ) // Gu (rock)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON2 ) // Choki (scissors)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_BUTTON3 ) // Pa (paper)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("IN1")   // A/D
	PORT_BIT( 0x7fff, 0x0000, IPT_DIAL_V ) PORT_SENSITIVITY(50) PORT_KEYDELTA(50)

	PORT_START("IN2")   // DSW
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW3:1,2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x000c, 0x000c, "Time To Hit" ) PORT_DIPLOCATION("SW3:3,4")
	PORT_DIPSETTING(      0x000c, "2 s" )
	PORT_DIPSETTING(      0x0008, "3 s" )
	PORT_DIPSETTING(      0x0004, "4 s" )
	PORT_DIPSETTING(      0x0000, "5 s" )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(      0x0020, "3" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0040, 0x0040, "SW3:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0080, 0x0080, "SW3:8" )

	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW4:1,2,3")
	PORT_DIPSETTING(      0x0100, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x0800, 0x0800, "SW4:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x1000, 0x1000, "SW4:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x2000, 0x2000, "SW4:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x4000, 0x4000, "SW4:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x8000, 0x8000, "SW4:8" )
INPUT_PORTS_END


/**************************************************************************
                            Arm Champs II
**************************************************************************/

static INPUT_PORTS_START( armchmp2 )
	PORT_START("IN0")   // Buttons + Sensors
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_SPECIAL ) // left   sensor
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_SPECIAL ) // right  sensor
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SPECIAL ) // center sensor
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x0010, IP_ACTIVE_LOW )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON1 ) // hard
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON2 ) // easy
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON3 ) // elbow (it always complains though)

	PORT_START("IN1")   // A/D
	PORT_BIT( 0x00ff, 0x0000, IPT_DIAL ) PORT_MINMAX(0x0000,0x00ff) PORT_SENSITIVITY(100) PORT_KEYDELTA(10)

	PORT_START("IN2")   // DSW
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x000c, "2" )
	PORT_DIPSETTING(      0x0008, "3" )
	PORT_DIPSETTING(      0x0004, "4" )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Game_Time ) )
	PORT_DIPSETTING(      0x0000, "15 s" )
	PORT_DIPSETTING(      0x0030, "20 s" )
	PORT_DIPSETTING(      0x0020, "25 s" )
	PORT_DIPSETTING(      0x0010, "30 s" )
	PORT_DIPNAME( 0x00c0, 0x0000, DEF_STR( Region ) )   // some text is in japanese regardless!
	PORT_DIPSETTING(      0x00c0, DEF_STR( Japan ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( USA ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Europe ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( World ) )

	PORT_START("IN3")   // DSW
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Demo Arm" )
	PORT_DIPSETTING(    0x80, "Center" )
	PORT_DIPSETTING(    0x00, "Side" )
INPUT_PORTS_END


/**************************************************************************


                                Gfx Layouts


**************************************************************************/

/* 8x8x4, straightforward layout */
static const gfx_layout tiles_8x8 =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	{ STEP8(0,4) },
	{ STEP8(0,4*8) },
	8*8*4
};

/* 16x16x4, straightforward layout */
static const gfx_layout tiles_16x16 =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	{ STEP16(0,4) },
	{ STEP16(0,4*16) },
	16*16*4
};

/* 16x16x4, made of four 8x8 tiles */
static const gfx_layout tiles_16x16_quad =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	{ STEP8(8*8*4*0,4), STEP8(8*8*4*2,4) },
	{ STEP16(0,4*8) },
	16*16*4
};

static const UINT32 road_layout_xoffset[64] =
{
	STEP16(16*4*0,4),STEP16(16*4*1,4),
	STEP16(16*4*2,4),STEP16(16*4*3,4)
};

/* Road: 64 x 1 x 4 */
static const gfx_layout road_layout =
{
	64,1,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	EXTENDED_XOFFS,
	{ 0 },
	64*1*4,
	road_layout_xoffset,
	nullptr
};

/**************************************************************************
                                Big Run
**************************************************************************/

static GFXDECODE_START( bigrun )
	GFXDECODE_ENTRY( "gfx1", 0, tiles_8x8,  0x0e00/2 , 16 ) // [0] Scroll 0
	GFXDECODE_ENTRY( "gfx2", 0, tiles_8x8,  0x1600/2 , 16 ) // [1] Scroll 1
	GFXDECODE_ENTRY( "gfx3", 0, tiles_8x8,  0x3600/2 , 16 ) // [2] Scroll 2
	GFXDECODE_ENTRY( "gfx4", 0, tiles_16x16,0x2800/2 , 64 ) // [3] Sprites
	GFXDECODE_ENTRY( "gfx5", 0, road_layout,0x1800/2 , 64 ) // [4] Road 0
	GFXDECODE_ENTRY( "gfx6", 0, road_layout,0x2000/2 , 64 ) // [5] Road 1
GFXDECODE_END

/**************************************************************************
                                Cisco Heat
**************************************************************************/

static GFXDECODE_START( cischeat )
	GFXDECODE_ENTRY( "gfx1", 0, tiles_8x8,  0x1c00/2, 32  ) // [0] Scroll 0
	GFXDECODE_ENTRY( "gfx2", 0, tiles_8x8,  0x2c00/2, 32  ) // [1] Scroll 1
	GFXDECODE_ENTRY( "gfx3", 0, tiles_8x8,  0x6c00/2, 32  ) // [2] Scroll 2
	GFXDECODE_ENTRY( "gfx4", 0, tiles_16x16,0x5000/2, 128 ) // [3] Sprites
	GFXDECODE_ENTRY( "gfx5", 0, road_layout,0x3800/2, 64  ) // [4] Road 0
	GFXDECODE_ENTRY( "gfx6", 0, road_layout,0x4800/2, 64  ) // [5] Road 1
GFXDECODE_END

/**************************************************************************
                            F1 GrandPrix Star
**************************************************************************/

static GFXDECODE_START( f1gpstar )
	GFXDECODE_ENTRY( "gfx1", 0, tiles_8x8,  0x1e00/2, 16  ) // [0] Scroll 0
	GFXDECODE_ENTRY( "gfx2", 0, tiles_8x8,  0x2e00/2, 16  ) // [1] Scroll 1
	GFXDECODE_ENTRY( "gfx3", 0, tiles_8x8,  0x6e00/2, 16  ) // [2] Scroll 2
	GFXDECODE_ENTRY( "gfx4", 0, tiles_16x16,0x5000/2, 128 ) // [3] Sprites
	GFXDECODE_ENTRY( "gfx5", 0, road_layout,0x3800/2, 64  ) // [4] Road 0
	GFXDECODE_ENTRY( "gfx6", 0, road_layout,0x4800/2, 64  ) // [5] Road 1
GFXDECODE_END

/**************************************************************************
                                Scud Hammer
**************************************************************************/

static GFXDECODE_START( scudhamm )
	GFXDECODE_ENTRY( "gfx1", 0, tiles_8x8,          0x1e00/2, 16  )   // [0] Scroll 0
	GFXDECODE_ENTRY( "gfx1", 0, tiles_8x8,          0x0000/2, 16  )   // [1] UNUSED
	GFXDECODE_ENTRY( "gfx3", 0, tiles_8x8,          0x4e00/2, 16  )   // [2] Scroll 2
	GFXDECODE_ENTRY( "gfx4", 0, tiles_16x16_quad,   0x3000/2, 128 )   // [3] sprites
	// No Road Layers
GFXDECODE_END


/***************************************************************************


                                Machine Drivers


**************************************************************************/

/**************************************************************************
                    Big Run, Cisco Heat, F1 GrandPrix Star
**************************************************************************/

/* TODO: this is hackish */
TIMER_DEVICE_CALLBACK_MEMBER(cischeat_state::bigrun_scanline)
{
	int scanline = param;

	if(scanline == 240) // vblank-out irq
		m_cpu1->set_input_line(4, HOLD_LINE);

	if(scanline == 154)
		m_cpu1->set_input_line(2, HOLD_LINE);

	if(scanline == 69)
		m_cpu1->set_input_line(1, HOLD_LINE);
}



#define STD_FM_CLOCK    3000000
#define STD_OKI_CLOCK     (1584000)




static MACHINE_CONFIG_START( bigrun, cischeat_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("cpu1", M68000, 10000000)
	MCFG_CPU_PROGRAM_MAP(bigrun_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", cischeat_state, bigrun_scanline, "screen", 0, 1)

	MCFG_CPU_ADD("cpu2", M68000, 10000000)
	MCFG_CPU_PROGRAM_MAP(bigrun_map2)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", cischeat_state,  irq4_line_hold)

	MCFG_CPU_ADD("cpu3", M68000, 10000000)
	MCFG_CPU_PROGRAM_MAP(bigrun_map3)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", cischeat_state,  irq4_line_hold)

	MCFG_CPU_ADD("soundcpu", M68000, 6000000)
	MCFG_CPU_PROGRAM_MAP(bigrun_sound_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(cischeat_state, irq4_line_hold, 16*30)

	MCFG_QUANTUM_TIME(attotime::from_hz(1200))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_AFTER_VBLANK)
	MCFG_SCREEN_REFRESH_RATE(30) //TODO: wrong!
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 256-1,  0+16, 256-16-1)
	MCFG_SCREEN_UPDATE_DRIVER(cischeat_state, screen_update_bigrun)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", bigrun)
	MCFG_PALETTE_ADD("palette", 0x4000/2)
	MCFG_PALETTE_ENABLE_SHADOWS()
	MCFG_PALETTE_FORMAT(RRRRGGGGBBBBRGBx)


	MCFG_VIDEO_START_OVERRIDE(cischeat_state,bigrun)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_YM2151_ADD("ymsnd", STD_FM_CLOCK)
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.50)

	MCFG_OKIM6295_ADD("oki1", STD_OKI_CLOCK, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.25)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.25)

	MCFG_OKIM6295_ADD("oki2", STD_OKI_CLOCK, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.25)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.25)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( cischeat, bigrun )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("cpu1")
	MCFG_CPU_PROGRAM_MAP(cischeat_map)

	MCFG_CPU_MODIFY("cpu2")
	MCFG_CPU_PROGRAM_MAP(cischeat_map2)

	MCFG_CPU_MODIFY("cpu3")
	MCFG_CPU_PROGRAM_MAP(cischeat_map3)

	MCFG_CPU_MODIFY("soundcpu")
	MCFG_CPU_PROGRAM_MAP(cischeat_sound_map)

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(0, 256-1,  0+16, 256-16-8-1)
	MCFG_SCREEN_UPDATE_DRIVER(cischeat_state, screen_update_cischeat)

	MCFG_GFXDECODE_MODIFY("gfxdecode", cischeat)
	MCFG_PALETTE_MODIFY("palette")
	MCFG_PALETTE_ENTRIES(0x8000/2)
	MCFG_PALETTE_FORMAT(RRRRGGGGBBBBRGBx)


	MCFG_VIDEO_START_OVERRIDE(cischeat_state,cischeat)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( f1gpstar, bigrun )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("cpu1")
	MCFG_CPU_CLOCK(12000000)
	MCFG_CPU_PROGRAM_MAP(f1gpstar_map)

	MCFG_CPU_MODIFY("cpu2")
	MCFG_CPU_CLOCK(12000000)
	MCFG_CPU_PROGRAM_MAP(f1gpstar_map2)

	MCFG_CPU_MODIFY("cpu3")
	MCFG_CPU_CLOCK(12000000)
	MCFG_CPU_PROGRAM_MAP(f1gpstar_map3)

	MCFG_CPU_MODIFY("soundcpu")
	MCFG_CPU_PROGRAM_MAP(f1gpstar_sound_map)

	/* video hardware */
	MCFG_GFXDECODE_MODIFY("gfxdecode", f1gpstar)
	MCFG_PALETTE_MODIFY("palette")
	MCFG_PALETTE_ENTRIES(0x8000/2)
	MCFG_PALETTE_FORMAT(RRRRGGGGBBBBRGBx)

	MCFG_VIDEO_START_OVERRIDE(cischeat_state,f1gpstar)
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(cischeat_state, screen_update_f1gpstar)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( f1gpstr2, f1gpstar )

	/* basic machine hardware */

	MCFG_CPU_MODIFY("cpu1")
	MCFG_CPU_PROGRAM_MAP(f1gpstr2_map)

	MCFG_CPU_MODIFY("soundcpu")
	MCFG_CPU_PROGRAM_MAP(f1gpstr2_sound_map)

	MCFG_CPU_ADD("cpu5", M68000, 10000000)
	MCFG_CPU_PROGRAM_MAP(f1gpstr2_io_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(12000))
MACHINE_CONFIG_END


/**************************************************************************
                                Scud Hammer
**************************************************************************/

/*
    1, 5-7]     busy loop
    2]          clr.w   $fc810.l + rte
    3]          game
    4]          == 3
*/

TIMER_DEVICE_CALLBACK_MEMBER(cischeat_state::scudhamm_scanline)
{
	int scanline = param;

	if(scanline == 240) // vblank-out irq
		m_maincpu->set_input_line(3, HOLD_LINE);

	if(scanline == 120) // timer irq (clears a flag, presumably sprite DMA end)
		m_maincpu->set_input_line(2, HOLD_LINE);
}

static MACHINE_CONFIG_START( scudhamm, cischeat_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",M68000, 12000000)
	MCFG_CPU_PROGRAM_MAP(scudhamm_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", cischeat_state, scudhamm_scanline, "screen", 0, 1)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_AFTER_VBLANK)
	MCFG_SCREEN_REFRESH_RATE(30) //TODO: wrong!
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500 * 3) /* not accurate */)
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 256-1, 0 +16, 256-1 -16)
	MCFG_SCREEN_UPDATE_DRIVER(cischeat_state, screen_update_scudhamm)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", scudhamm)
	MCFG_PALETTE_ADD("palette", 0x8000/2)
	MCFG_PALETTE_FORMAT(RRRRGGGGBBBBRGBx)
	MCFG_PALETTE_ENABLE_SHADOWS()

	MCFG_VIDEO_START_OVERRIDE(cischeat_state,f1gpstar)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_OKIM6295_ADD("oki1", 2112000, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.5)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.5)

	MCFG_OKIM6295_ADD("oki2", 2112000, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.5)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.5)
MACHINE_CONFIG_END


/**************************************************************************
                            Arm Champs II
**************************************************************************/

TIMER_DEVICE_CALLBACK_MEMBER(cischeat_state::armchamp2_scanline)
{
	int scanline = param;

	if(scanline == 240) // vblank-out irq
		m_maincpu->set_input_line(2, HOLD_LINE);

	if(scanline == 120) // timer irq (TODO: timing)
		m_maincpu->set_input_line(4, HOLD_LINE);
}

static MACHINE_CONFIG_DERIVED( armchmp2, scudhamm )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(armchmp2_map)
	MCFG_TIMER_MODIFY("scantimer")
	MCFG_TIMER_DRIVER_CALLBACK(cischeat_state, armchamp2_scanline)
MACHINE_CONFIG_END


/***************************************************************************


                                ROMs Loading


**************************************************************************/

/*
    Sprite data is stored like this:

    Sprite 0
        Line 0-15 (left half)
        Line 0-15 (right half)
    Sprite 1
    ..

    We need to untangle it
*/
void cischeat_state::cischeat_untangle_sprites(const char *region)
{
	UINT8       *src = memregion(region)->base();
	const UINT8 *end = src + memregion(region)->bytes();

	while (src < end)
	{
		UINT8 sprite[16*8];
		int i;

		for (i = 0; i < 16 ; i++)
		{
			memcpy(&sprite[i*8+0], &src[i*4+0],    4);
			memcpy(&sprite[i*8+4], &src[i*4+16*4], 4);
		}
		memcpy(src, sprite, 16*8);
		src += 16*8;
	}
}



/***************************************************************************

                                    Big Run

Jaleco 1989

BR8950c
-------

 20MHz
                                    6264 6264
68000-10  E1 3 58257        SCPT              VCTR    5   6
          E2 4 58257
                                    6264 6264
                            SCPT              VCTR    7   8

                                    6264 6264
                            SCPT              VCTR    9


        PR88004D
                                    2088  2088


    2018
                                   PR88004P


8951
------
                          22
                          21     23
                    19    T48    T49   20

                    15    T42    T44   16

                     7    T41    T43   8

8952A
-----

                              18  T46   T45  14

68000


68000


                              18  T46   T45  4


8953
----

    68000                  YM2151
    D65006C
    BR8953C.1 58257
    BR8953C.2 58257
                                6295    5  T50
                                6295    8  T51

***************************************************************************/

ROM_START( bigrun )
	ROM_REGION( 0x80000, "cpu1", 0 )
	ROM_LOAD16_BYTE( "br8950b.e1",  0x000000, 0x040000, CRC(bfb54a62) SHA1(49f78e162e8bc19a75c62029737acd665b9b124b) )
	ROM_LOAD16_BYTE( "br8950b.e2",  0x000001, 0x040000, CRC(c0483e81) SHA1(e8fd8860191c7d8cb4dda44c69ce05cd58174a07) )

	ROM_REGION( 0x40000, "cpu2", 0 )
	ROM_LOAD16_BYTE( "br8952a.19", 0x000000, 0x020000, CRC(fcf6b70c) SHA1(34f7ca29241925251a043c63062596d1c220b730) ) // 1xxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD16_BYTE( "br8952a.20", 0x000001, 0x020000, CRC(c43d367b) SHA1(86242ba03cd172e95bf42eac369fb3acf42c7c45) ) // 1xxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x40000, "cpu3", 0 )
	ROM_LOAD16_BYTE( "br8952a.9",  0x000000, 0x020000, CRC(f803f0d9) SHA1(6d2e90e74bb15cd443d901ad0fc827117432daa6) ) // 1xxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD16_BYTE( "br8952a.10", 0x000001, 0x020000, CRC(8c0df287) SHA1(3a2e689e2d841e2089b18267a8462f1acc40ae99) ) // 1xxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x40000, "soundcpu", 0 )
	ROM_LOAD16_BYTE( "br8953c.2", 0x000000, 0x020000, CRC(b690d8d9) SHA1(317c0b4e9bef30c84daf16765af66f5a6f9d1c54) )
	ROM_LOAD16_BYTE( "br8953c.1", 0x000001, 0x020000, CRC(79fc7bc0) SHA1(edb6cf626f93417cdc9525627d85a0ca9bcd1b1c) )

	ROM_REGION16_BE( 0x40000, "user1", 0 )  /* second halves of program ROMs */
	ROM_LOAD16_BYTE( "br8950b.3",   0x000000, 0x020000, CRC(864cebf7) SHA1(4e63106cd5832901688dfce2e450f536a6927c81) )
	ROM_LOAD16_BYTE( "br8950b.4",   0x000001, 0x020000, CRC(702c2a6e) SHA1(cf3327919e24b7206d404b008cb00117e4308f94) )

	ROM_REGION( 0x040000, "gfx1", 0 ) // scroll 0
	ROM_LOAD( "br8950b.5",  0x000000, 0x020000, CRC(0530764e) SHA1(0a89eab2ce9bd5df574a46bb6ea884c33407f122) )
	ROM_LOAD( "br8950b.6",  0x020000, 0x020000, CRC(cf40ecfa) SHA1(96e1bfb7a33603a1eaaeb31386bba947fc005060) )

	ROM_REGION( 0x040000, "gfx2", 0 ) // scroll 1
	ROM_LOAD( "br8950b.7",  0x000000, 0x020000, CRC(bd5fd061) SHA1(e5e0afb0a3a1d5f7a27bd04b724c48ea65872233) )
	ROM_LOAD( "br8950b.8",  0x020000, 0x020000, CRC(7d66f418) SHA1(f6564ac9d65d40af17f5dd422c435aeb6a385256) )

	ROM_REGION( 0x020000, "gfx3", 0 ) // scroll 2
	ROM_LOAD( "br8950b.9",  0x000000, 0x020000, CRC(be0864c4) SHA1(e0f1c0f09b30a731f0e062b1acb1b3a3a772a5cc) )

	ROM_REGION( 0x280000, "gfx4", 0 )   /* sprites */
	ROM_LOAD16_BYTE( "mr88004a.t41",  0x000000, 0x080000, CRC(c781d8c5) SHA1(52b23842a20443b51490d701dca72fb0a3118033) )
	ROM_LOAD16_BYTE( "mr88004d.t43",  0x000001, 0x080000, CRC(e4041208) SHA1(f5bd21b42f627b01bca2324082aecee7852a37e6) )
	ROM_LOAD16_BYTE( "mr88004b.t42",  0x100000, 0x080000, CRC(2df2e7b9) SHA1(5772e0dc2f842077ea70a558b55ec5ceea693f00) )
	ROM_LOAD16_BYTE( "mr88004e.t44",  0x100001, 0x080000, CRC(7af7fbf6) SHA1(5b8e2d3bb0c9f29f2c96f68c6d4c7fbf63e640c9) )
	ROM_LOAD16_BYTE( "mb88004c.t48",  0x200000, 0x040000, CRC(02e2931d) SHA1(754b38929a2dd10d39634fb9cf737e3175a8b1ec) )
	ROM_LOAD16_BYTE( "mb88004f.t49",  0x200001, 0x040000, CRC(4f012dab) SHA1(35f756b1c7b41f2e81ccbefb2075608a5d663152) )

	ROM_REGION( 0x100000, "gfx5", 0 ) // Road 0
	ROM_LOAD( "mr88004g.t45",  0x000000, 0x080000, CRC(bb397bae) SHA1(c67d33bde6e8de2ea7581faadb96acd977adc13c) )
	ROM_LOAD( "mb88004h.t46",  0x080000, 0x080000, CRC(6b31a1ba) SHA1(71a956f0f51a63bddedfef0febdc95108ed42226) )

	ROM_REGION( 0x100000, "gfx6", 0 ) // Road 1
	ROM_LOAD( "mr88004g.t45",  0x000000, 0x080000, CRC(bb397bae) SHA1(c67d33bde6e8de2ea7581faadb96acd977adc13c) )
	ROM_LOAD( "mb88004h.t46",  0x080000, 0x080000, CRC(6b31a1ba) SHA1(71a956f0f51a63bddedfef0febdc95108ed42226) )

	/* t50 & t51: 40000-5ffff = 60000-7ffff */
	ROM_REGION( 0x80000, "oki1", 0 )    /* samples */
	ROM_LOAD( "mb88004l.t50", 0x000000, 0x020000, CRC(6b11fb10) SHA1(eb6e9614bb50b8fc332ada61882da484d34d727f) )
	ROM_CONTINUE(             0x040000, 0x020000             )
	ROM_CONTINUE(             0x020000, 0x020000             )
	ROM_CONTINUE(             0x060000, 0x020000             )

	ROM_REGION( 0x80000, "oki2", 0 )    /* samples */
	ROM_LOAD( "mb88004m.t51", 0x000000, 0x020000, CRC(ee52f04d) SHA1(fc45bd1d3a7552433e40846c358573c6988127c3) )
	ROM_CONTINUE(             0x040000, 0x020000             )
	ROM_CONTINUE(             0x020000, 0x020000             )
	ROM_CONTINUE(             0x060000, 0x020000             )

	ROM_REGION( 0x20000, "user2", 0 )       /* ? Unused ROMs ? */
	ROM_LOAD( "br8951b.21",  0x000000, 0x020000, CRC(59b9c26b) SHA1(09fea3b77b045d9c1ed62bf53efa8b5242a33a10) ) // x00xxxxxxxxxxxxx, mask=0001e0
	ROM_LOAD( "br8951b.22",  0x000000, 0x020000, CRC(c112a803) SHA1(224a2ed690b78caef266958a93524211ff4a8e70) ) // x00xxxxxxxxxxxxx
	ROM_LOAD( "br8951b.23",  0x000000, 0x010000, CRC(b9474fec) SHA1(f1f0eab014e8f52572484b83f56189e0ff6f2b0d) ) // 000xxxxxxxxxxxxx
ROM_END

DRIVER_INIT_MEMBER(cischeat_state,bigrun)
{
	cischeat_untangle_sprites("gfx4");   // Untangle sprites
	phantasm_rom_decode(machine(), "soundcpu");                 // Decrypt sound cpu code
}


/***************************************************************************

                                Cisco Heat

From "ARCADE ROMS FROM JAPAN (ARFJ)"'s readme:

 -BOARD #1 CH-9072 EB90001-20024-
|                [9]r15 [10]r16  |  EP: [1]ch9072.01    [2]ch9072.02
|               [11]r17 [12]r18  |      [3]ch9072.03
|               [13]r25 [14]r26  |MASK: [9]ch9072.r15   [10]ch9072.r16
|               [15]r19 [16]r20  |      [11]ch9072.r17  [12]ch9072.r18
|     [1]01                      |      [13]ch9072.r25  [14]ch9072.r26
|[2]02[3]03                      |      [15]ch9072.r19  [16]ch9072.r20
|[4] [5] [6]                     |
|[7]                             |  ([4][5][6][8]:27cx322  [7]:82S135)
|[8]                             |
 --------------------------------

Video                        Sound
 -BOARD #2 CH-9071 EB90001-20023-   X1:12MHz  X2:4MHz  X3:20MHz  X4:7MHz
|68000             [9]      68000|  YM2151x1 OKI M6295 x2 ([8]82S147 [9]:82S185)
|[1]01 X3               X4 [11]11|  EP: [1]ch9071v2.01 "CH-9071 Ver.2  1"
|[2]02                     [10]10|      [2]ch9071.02
|[3]03                           |      [3]ch9071v2.03 "CH-9071 Ver.2  3"
|[4]04            X1 X2   [12]r23|      [4]ch9071.04
|           [8]     YM2151[13]r24|      [7]ch9071.07
|[7]07[5]a14[6]t74               |      [10]ch9071.10   [11]ch9071.11
|                                |MASK: [5]ch9071.a14   [6]ch9071.t74
 --------------------------------       [12]ch9071.r23  [13]ch9071.r24

 -BOARD #3 CH-9073 EB90001-20025-
|           [5]r21 [6]r22   68000|
|    [9]    [1]01  [2]02         | EP:  [1]ch9073.01    [2]ch9073.02
|                                |      [3]ch9073v1.03 "CH-9073 Ver.1  3"
|           [7]r21 [8]r22   68000|      [4]ch9073v1.04 "CH-9073 Ver.1  4"
|           [3]03  [4]04         |MASK: [5][7]ch9073.r21
|            [10]    [11]        |      [6][8]ch9073.r22
|                                |      ([9][10][11]:82S129)
 --------------------------------

DIP SW:8BITx2 , 4BITx1

According to KLOV:

Controls:   Steering: Wheel - A 'judder' motor is attached to the wheel.
            Pedals: 2 - Both foot controls are simple switches.
Sound:      Amplified Stereo (two channel)

***************************************************************************/

ROM_START( cischeat )
	ROM_REGION( 0x080000, "cpu1", 0 )
	ROM_LOAD16_BYTE( "ch9071v2.03", 0x000000, 0x040000, CRC(dd1bb26f) SHA1(2b9330b45edcc3291ad4ac935558c1f070ab5bd9) )
	ROM_LOAD16_BYTE( "ch9071v2.01", 0x000001, 0x040000, CRC(7b65276a) SHA1(e0075b6d09da12ab7c84b888ffe65cd33ec7c6b6) )

	ROM_REGION( 0x80000, "cpu2", 0 )
	ROM_LOAD16_BYTE( "ch9073.01",  0x000000, 0x040000, CRC(ba331526) SHA1(bed5182c0d63a64cc44f9c127d1b8ce55a701184) )
	ROM_LOAD16_BYTE( "ch9073.02",  0x000001, 0x040000, CRC(b45ff10f) SHA1(11fb2d9a3b5cc4f997816e0d3c9cf47bf749db17) )

	ROM_REGION( 0x80000, "cpu3", 0 )
	ROM_LOAD16_BYTE( "ch9073v1.03", 0x000000, 0x040000, CRC(bf1d1cbf) SHA1(0892ce5f35864393a6f899f02f811b8fdba03978) )
	ROM_LOAD16_BYTE( "ch9073v1.04", 0x000001, 0x040000, CRC(1ec8a597) SHA1(311d4aa8bd92dd2eea0a64f881f64d19b7ba7d12) )

	ROM_REGION( 0x40000, "soundcpu", 0 )
	ROM_LOAD16_BYTE( "ch9071.11", 0x000000, 0x020000, CRC(bc137bea) SHA1(ca6d781a617c797aec87e6ce0a002280aa62aebc) )
	ROM_LOAD16_BYTE( "ch9071.10", 0x000001, 0x020000, CRC(bf7b634d) SHA1(29186c41a397df322cc2c40decd1c19963f89d36) )

	ROM_REGION16_BE( 0x100000, "user1", 0 ) /* second halves of program ROMs */
	ROM_LOAD16_BYTE( "ch9071.04",   0x000000, 0x040000, CRC(7fb48cbc) SHA1(7f0442ce37b39e830fe8bcb8230cf7da2103059d) )  // cpu #1
	ROM_LOAD16_BYTE( "ch9071.02",   0x000001, 0x040000, CRC(a5d0f4dc) SHA1(2e7aaa915e27ab31e38ca6759301ffe33a12b427) )
	ROM_COPY( "cpu2", 0x40000, 0x80000, 0x40000 )
	ROM_COPY( "cpu3", 0x40000, 0xc0000, 0x40000 )

	ROM_REGION( 0x040000, "gfx1", 0 )
	ROM_LOAD( "ch9071.a14",  0x000000, 0x040000, CRC(7a6d147f) SHA1(8f52e012d9699311c2a2409130c6200c6d2e1c51) ) // scroll 0

	ROM_REGION( 0x040000, "gfx2", 0 )
	ROM_LOAD( "ch9071.t74",  0x000000, 0x040000, CRC(735a2e25) SHA1(51f528db207283c0d2b70acd5037ffafbe24f6f3) ) // scroll 1

	ROM_REGION( 0x010000, "gfx3", 0 )
	ROM_LOAD( "ch9071.07",   0x000000, 0x010000, CRC(3724ccc3) SHA1(3797ea49156362467ba948c51ac7b52610d1b9de) ) // scroll 2

	ROM_REGION( 0x400000, "gfx4", 0 )   /* sprites */
	ROM_LOAD16_BYTE( "ch9072.r15", 0x000000, 0x080000, CRC(38af4aea) SHA1(ea27ab44b33776984adaa9b26d85165d6700a12c) )
	ROM_LOAD16_BYTE( "ch9072.r16", 0x000001, 0x080000, CRC(71388dad) SHA1(0d2451e36cfbf7400ade849b4c8a1e8f56fc089c) )
	ROM_LOAD16_BYTE( "ch9072.r17", 0x100000, 0x080000, CRC(9d052cf3) SHA1(d6bd30965316104cb03e62a69df61eb60eb84741) )
	ROM_LOAD16_BYTE( "ch9072.r18", 0x100001, 0x080000, CRC(fe402a56) SHA1(c64aa0d83b77ce0fea5072a9332bbcbbd94a1be4) )
	ROM_LOAD16_BYTE( "ch9072.r25", 0x200000, 0x080000, CRC(be8cca47) SHA1(4e64cbbdec9b55721e420b50f0a563684e93f739) )
	ROM_LOAD16_BYTE( "ch9072.r26", 0x200001, 0x080000, CRC(2f96f47b) SHA1(045842428849bc4afb8b59f7f0594b3f537f9e12) )
	ROM_LOAD16_BYTE( "ch9072.r19", 0x300000, 0x080000, CRC(4e996fa8) SHA1(c74d761e0c8d17b3fb5d33b06136c4d0ba87c2e1) )
	ROM_LOAD16_BYTE( "ch9072.r20", 0x300001, 0x080000, CRC(fa70b92d) SHA1(01b5f7309c9c7cd6d41c0f46678772dda45344e1) )

	ROM_REGION( 0x100000, "gfx5", 0 )
	ROM_LOAD( "ch9073.r21",  0x000000, 0x080000, CRC(2943d2f6) SHA1(ae8a25c1d76d3c36aa326d0171acb7dce93c4d87) ) // Road
	ROM_LOAD( "ch9073.r22",  0x080000, 0x080000, CRC(2dd44f85) SHA1(5f20f75e96e14389187d3471bc7f2ceb0758eec4) )

	ROM_REGION( 0x100000, "gfx6", 0 )
	ROM_LOAD( "ch9073.r21",  0x000000, 0x080000, CRC(2943d2f6) SHA1(ae8a25c1d76d3c36aa326d0171acb7dce93c4d87) ) // Road
	ROM_LOAD( "ch9073.r22",  0x080000, 0x080000, CRC(2dd44f85) SHA1(5f20f75e96e14389187d3471bc7f2ceb0758eec4) )

	ROM_REGION( 0x80000, "oki1", 0 )    /* samples */
	ROM_LOAD( "ch9071.r23", 0x000000, 0x080000, CRC(c7dbb992) SHA1(9fa802830947f4e5d41447b2ac9637daf1fd8193) ) // 2 x 0x40000

	ROM_REGION( 0x80000, "oki2", 0 )    /* samples */
	ROM_LOAD( "ch9071.r24", 0x000000, 0x080000, CRC(e87ca4d7) SHA1(b2a2edd701324640e438d1e84dd033ec212917b5) ) // 2 x 0x40000 (FIRST AND SECOND HALF IDENTICAL)

	ROM_REGION( 0x40000, "user2", 0 )       /* ? Unused ROMs ? */
	ROM_LOAD( "ch9072.01",  0x000000, 0x020000, CRC(b2efed33) SHA1(3b347d4bc866aaa6cb53bd0991b4fb6a22e40a5c) ) // FIXED BITS (xxxxxxxx0xxxxxxx)
	ROM_LOAD( "ch9072.02",  0x000000, 0x040000, CRC(536edde4) SHA1(45ebd2add357275177fcd7b6d9ea748c6756f1c0) )
	ROM_LOAD( "ch9072.03",  0x000000, 0x040000, CRC(7e79151a) SHA1(5a305cff8600446be426641ce112208b379094b9) )
ROM_END

DRIVER_INIT_MEMBER(cischeat_state,cischeat)
{
	cischeat_untangle_sprites("gfx4");   // Untangle sprites
	astyanax_rom_decode(machine(), "soundcpu");                 // Decrypt sound cpu code
}


/***************************************************************************

                            F1 GrandPrix Star

From malcor's readme:

Location     Device      File ID     Checksum      PROM Label
-----------------------------------------------------------------
LB IC2       27C010    9188A-1.V10     46C6     GP-9188A 1  Ver1.0
LB IC27      27C010    9188A-6.V10     DB84     GP-9188A 6  Ver1.0
LB IC46      27C010    9188A-11.V10    DFEA     GP-9188A 11 Ver1.0
LB IC70      27C010    9188A-16.V10    1034     GP-9188A 16 Ver1.0
LB IC91      27C020    9188A-21.V10    B510     GP-9188A 21 Ver1.0
LB IC92      27C020    9188A-22.V20    A9BA     GP-9188A 22 Ver2.0
LB IC124     27C020    9188A-26.V10    AA81     GP-9188A 26 Ver1.0
LB IC125     27C020    9188A-27.V20    F34D     GP-9188A 27 Ver2.0
LB IC174    23C1000    9188A-30.V10    0AA5     GP-9188A 30 Ver1.0
TB IC2       27C010    9190A-1.V11     B05A     GP-9190A 1  Ver1.1
TB IC4       27C010    9190A-2.V11     ED7A     GP-9190A 2  Ver1.1
LB IC37     23C4001    90015-01.W06    F10F     MR90015-01-W06      *
LB IC79     23C4001    90015-01.W06    F10F     MR90015-01-W06      *
LB IC38     23C4001    90015-02.W07    F901     MR90015-02-W07      *
LB IC80     23C4001    90015-02.W07    F901     MR90015-02-W07      *
LB IC39     23C4001    90015-03.W08    F100     MR90015-03-W08      *
LB IC81     23C4001    90015-03.W08    F100     MR90015-03-W08      *
LB IC40     23C4001    90015-04.W09    FA00     MR90015-04-W09      *
LB IC82     23C4001    90015-04.W09    FA00     MR90015-04-W09      *
LB IC64     23C4001    90015-05.W10    5FF9     MR90015-05-W10
LB IC63     23C4001    90015-06.W11    6EDA     MR90015-06-W11
LB IC62     23C4001    90015-07.W12    E9B4     MR90015-07-W12
LB IC61     23C4001    90015-08.W14    5107     MR90015-08-W14
LB IC17     23C4001    90015-09.W13    71EE     MR90015-09-W13
LB IC16     23C4001    90015-10.W15    EFEF     MR90015-10-W15
MB IC54     23C4001    90015-20.R45    7890     MR90015-20-R45      *
MB IC67     23C4001    90015-20.R45    7890     MR90015-20-R45      *
MB IC1      23C4001    90015-21.R46    C73C     MR90015-21-R46
MB IC2      23C4001    90015-22.R47    5D58     MR90015-22-R47
MB IC5      23C4001    90015-23.R48    4E7B     MR90015-23-R48
MB IC6      23C4001    90015-24.R49    F6A0     MR90015-24-R49
MB IC11     23C4001    90015-25.R50    9FC0     MR90015-25-R50
MB IC12     23C4001    90015-26.R51    13E4     MR90015-26-R51
MB IC15     23C4001    90015-27.R52    8D5D     MR90015-27-R52
MB IC16     23C4001    90015-28.R53    E0B8     MR90015-28-R53
MB IC21     23C4001    90015-29.R54    DF33     MR90015-29-R54
MB IC22     23C4001    90015-30.R55    DA2D     MR90015-30-R55
LB IC123    23C4001    90015-31.R56    BE57     MR90015-31-R56
LB IC152    23C4001    90015-32.R57    8B57     MR90015-32-R57
TB IC12     23C4001    90015-33.W31    7C0E     MR90015-33-W31
TB IC11     23C4001    90015-34.W32    B203     MR90015-34-W32
MB IC39     27CX322    CH9072-4        085F     CH9072 4
MB IC33     27CX322    CH9072-5        641D     CH9072 5
MB IC35     27CX322    CH9072-6        EAE1     CH9072 6
MB IC59     27CX322    CH9072-8        AB60     CH9072 8
LB IC105    N82S147    PR88004Q        FCFC     PR88004Q
LB IC66     N82S135    PR88004W        20C8     PR88004W
LB IC117    N82S185    PR90015A        3326     PR90015A
LB IC153    N82S135    PR90015B        1E52     PR90015B

Notes:  TB - Top board     (audio & I/O)       GP-9190A EB90015-20039-1
        MB - Middle board  (GFX)               GP-9189  EB90015-20038
        LB - Lower board   (CPU/GFX)           GP-9188A EB90015-20037-1

         * - These ROMs are found twice on the PCB
           - There are two linked boards per cabinet (two player cabinet)
             (attract mode displays across both monitors)

Brief hardware overview:
------------------------

Main processor   - 68000
                 - program ROMs: 9188A-22.V20 (odd), 9188A-27.V20 (even) bank 1
                                 9188A-21.V10 (odd), 9188A-26.V10 (even) bank 2
                 - Processor RAM 2x LH52250AD-90L (32kx8 SRAM)

Slave processor1 - 68000
                 - program ROMs: 9188A-11.V10 (odd), 9188A-16.V10 (even)
                 - Processor RAM 2x LH5168D-10L (8kx8 SRAM)
                 - CS90015-04 (64 pin PQFP)  + 2x MCM2018AN45 (2kx8 SRAM)
                 - GS90015-05 (100 pin PQFP) + 2x MCM2018AN45 (2kx8 SRAM)
                 - uses ROMs: 90015-08.W14, 90015-07.W12, 90015-06.W11
                              90015-05.W10, 90015-01.W06, 90015-02.W07
                              90015-03.W08, 90015-04.W09

Slave processor2 - 68000
                 - Program ROMs: 9188A-1.V10 (odd), 9188A-6.V10 (even)
                 - Processor RAM 2x LH5168D-10L (8kx8 SRAM)
                 - CS90015-04 (64 pin PQFP)  + 2x MCM2018AN45 (2kx8 SRAM)
                 - GS90015-05 (100 pin PQFP) + 2x MCM2018AN45 (2kx8 SRAM)
                 - uses ROMs: 90015-01.W06, 90015-02.W07, 90015-03.W08
                              90015-10.W15, 90015-09.W13, 90015-04.W09

Sound processor  - 68000
                 - Program ROMs: 9190A-1.V11 (odd), 9190A-2.V11 (even)
                 - Processor RAM 2x LH52250AD-90L (32kx8 SRAM)
                 - M6295,  uses ROM  90015-34.W32
                 - M6295,  uses ROM  90015-33.W31
                 - YM2151

GFX & Misc       - GS90015-02 (100 pin PQFP),  uses ROM 90015-31-R56
                   GS-9000406 (80 pin PQFP)  + 2x LH5168D-10L (8kx8 SRAM)

                 - GS90015-02 (100 pin PQFP),  uses ROM 90015-32-R57
                   GS-9000406 (80 pin PQFP)  + 2x LH5168D-10L (8kx8 SRAM)

                 - GS90015-02 (100 pin PQFP),  uses ROM 9188A-30-V10
                   GS-9000406 (80 pin PQFP)  + 2x LH5168D-10L (8kx8 SRAM)

                 - GS900151   (44 pin PQFP) (too small for the full part No.)
             3x  - GS90015-03 (80 pin PQFP)  + 2x LH52258D-45 (32kx8 SRAM)
             2x  - GS90015-06 (100 pin PQFP) + 2x LH52250AD-90L (32kx8 SRAM)
                 - GS90015-07 (64 pin PQFP)
                 - GS90015-08 (64 pin PQFP)
                 - GS90015-09 (64 pin PQFP)  + 2x MCM2018AN45 (2kx8 SRAM)
                 - GS90015-10 (64 pin PQFP)
                 - GS90015-12 (80 pin PQFP)  + 2x MCM2018AN45 (2kx8 SRAM)
                 - GS90015-11 (100 pin PQFP)
                   uses ROMs 90015-30-R55, 90015-25-R50, 90015-24-R49
                             90015-29-R54, 90015-23-R48, 90015-22-R47
                             90015-28-R53, 90015-21-R46, 90015-27-R52
                             90015-26-R51

***************************************************************************/

ROM_START( f1gpstar )
	ROM_REGION( 0x100000, "cpu1", 0 )
	ROM_LOAD16_BYTE( "9188a-27.v20", 0x000000, 0x040000, CRC(0a9d3896) SHA1(5e3332a1b779dead1e4f9ef274a2f168721db0ed) )
	ROM_LOAD16_BYTE( "9188a-22.v20", 0x000001, 0x040000, CRC(de15c9ca) SHA1(f356b02ca66b7e8ab0293e6e28fcd3f7996c80c8) )

	ROM_REGION( 0x80000, "cpu2", 0 )
	/* Should Use ROMs: 90015-01.W06, 90015-02.W07, 90015-03.W08, 90015-04.W09 */
	ROM_LOAD16_BYTE( "9188a-16.v10",  0x000000, 0x020000, CRC(ef0f7ca9) SHA1(98ad687fdab67dd9f54b50cf21fd10ac34b61e7a) )
	ROM_LOAD16_BYTE( "9188a-11.v10",  0x000001, 0x020000, CRC(de292ea3) SHA1(04ed19045edb4edfff2b8fedac37c4a3352dfa76) )

	ROM_REGION( 0x80000, "cpu3", 0 )
	/* Should Use ROMs: 90015-01.W06, 90015-02.W07, 90015-03.W08, 90015-04.W09 */
	ROM_LOAD16_BYTE( "9188a-6.v10",  0x000000, 0x020000, CRC(18ba0340) SHA1(e46e10a350f18cf3a46c0d3a0cb08fc369fced6d) )
	ROM_LOAD16_BYTE( "9188a-1.v10",  0x000001, 0x020000, CRC(109d2913) SHA1(e117556481e801d51b8526a143bc202dda222f7f) )

	ROM_REGION( 0x40000, "soundcpu", 0 )
	ROM_LOAD16_BYTE( "9190a-2.v11", 0x000000, 0x020000, CRC(acb2fd80) SHA1(bbed505ce745490ae11df8efdd3633181cfd4dec) )
	ROM_LOAD16_BYTE( "9190a-1.v11", 0x000001, 0x020000, CRC(7cccadaf) SHA1(d1b79fbd0e27e8d479ef533fa00b18d1f2982dda) )

	ROM_REGION16_BE( 0x80000, "user1", 0 )  /* second halves of program ROMs */
	ROM_LOAD16_BYTE( "9188a-26.v10", 0x000000, 0x040000, CRC(0b76673f) SHA1(cf29333ffb51250ae2d5363d612260f536cd15af) ) // cpu #1
	ROM_LOAD16_BYTE( "9188a-21.v10", 0x000001, 0x040000, CRC(3e098d77) SHA1(0bf7e8ca36086a7ae3d44a10b4ca43f869403eb0) )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "90015-31.r56",  0x000000, 0x080000, CRC(0c8f0e2b) SHA1(6b0917a632c6beaca018146b6be66a3561b863b3) ) // scroll 0

	ROM_REGION( 0x080000, "gfx2", 0 )
	ROM_LOAD( "90015-32.r57",  0x000000, 0x080000, CRC(9c921cfb) SHA1(006d4af6dbbc34bee05f3620ba0a947a568a2400) ) // scroll 1

	ROM_REGION( 0x020000, "gfx3", 0 )
	ROM_LOAD( "9188a-30.v10",  0x000000, 0x020000, CRC(0ef1fbf1) SHA1(28fa0b677e70833954a5fc2fdce233d0dec4f43c) ) // scroll 2

	ROM_REGION( 0x500000, "gfx4", 0 )   /* sprites */
	ROM_LOAD16_BYTE( "90015-21.r46",  0x000000, 0x080000, CRC(6f30211f) SHA1(aedba39fc6aab7847a3a2314e152bc00615cbd72) )
	ROM_LOAD16_BYTE( "90015-22.r47",  0x000001, 0x080000, CRC(05a9a5da) SHA1(807c43c3ee76bce8e4874fa51d2453917b1e4f3b) )
	ROM_LOAD16_BYTE( "90015-23.r48",  0x100000, 0x080000, CRC(58e9c6d2) SHA1(b81208819dbc5887183855001c72d0d91d32fc4b) )
	ROM_LOAD16_BYTE( "90015-24.r49",  0x100001, 0x080000, CRC(abd6c91d) SHA1(ccbf47a37008a0ec64d7058225e6ba991b559a39) )
	ROM_LOAD16_BYTE( "90015-25.r50",  0x200000, 0x080000, CRC(7ded911f) SHA1(d0083c17266f03f70f2d4b2953237fed0cb0696c) )
	ROM_LOAD16_BYTE( "90015-26.r51",  0x200001, 0x080000, CRC(18a6c663) SHA1(b39cbb4b6d09150c7d1a8cf2cd3a96b61c265d83) )
	ROM_LOAD16_BYTE( "90015-27.r52",  0x300000, 0x080000, CRC(7378c82f) SHA1(3e65064a36393b5d6ecb118a560f3fccc5b3c3c2) )
	ROM_LOAD16_BYTE( "90015-28.r53",  0x300001, 0x080000, CRC(9944dacd) SHA1(722a0c152ef97830d5ab6251d5447293d951261f) )
	ROM_LOAD16_BYTE( "90015-29.r54",  0x400000, 0x080000, CRC(2cdec370) SHA1(9fd8e8d6783a6c820d1f580a8872b5cc59641aa9) )
	ROM_LOAD16_BYTE( "90015-30.r55",  0x400001, 0x080000, CRC(47e37604) SHA1(114eb01d3258bf481c01a8378f5f08b2bdeffbba) )

	ROM_REGION( 0x200000, "gfx5", 0 )
	ROM_LOAD( "90015-05.w10",  0x000000, 0x080000, CRC(8eb48a23) SHA1(e394eb013dd1fdc1c30616ce356bebd187453d08) ) // Road 0
	ROM_LOAD( "90015-06.w11",  0x080000, 0x080000, CRC(32063a68) SHA1(587d35edec2755df11f4d63ff7bfd134a0f9fb36) )
	ROM_LOAD( "90015-07.w12",  0x100000, 0x080000, CRC(0d0d54f3) SHA1(8040945ea8f9487f0527140c90d6a66965c27ff4) )
	ROM_LOAD( "90015-08.w14",  0x180000, 0x080000, CRC(f48a42c5) SHA1(5caf50fbde682d7d1e4ec0cceacf0db7682b72a9) )

	ROM_REGION( 0x100000, "gfx6", 0 )
	ROM_LOAD( "90015-09.w13",  0x000000, 0x080000, CRC(55f49315) SHA1(ad338cb53149ccea2dbe5ad890433c9f09a8211c) ) // Road 1
	ROM_LOAD( "90015-10.w15",  0x080000, 0x080000, CRC(678be0cb) SHA1(3857b549170b62b29644cf5ebdd4aac1afa9e420) )

	ROM_REGION( 0x80000, "oki1", 0 )    /* samples */
	ROM_LOAD( "90015-34.w32", 0x000000, 0x080000, CRC(2ca9b062) SHA1(c01b8020b409d826c0ae69c153fdc5d89241771e) ) // 2 x 0x40000

	ROM_REGION( 0x80000, "oki2", 0 )    /* samples */
	ROM_LOAD( "90015-33.w31", 0x000000, 0x080000, CRC(6121d247) SHA1(213c7c45bc3d57c09778b1d58dbb5fe26d0b2477) ) // 2 x 0x40000

	ROM_REGION( 0x80000, "user2", 0 )       /* ? Unused ROMs ? */
// "I know that one of the ROM images in the archive looks bad (90015-04.W09)
//  however, it is good as far as I can tell. There were two of those ROMs
// (soldered) onto the board and I checked them both against each other. "

	ROM_LOAD( "90015-04.w09",  0x000000, 0x080000, CRC(5b324c81) SHA1(ce61f2ea29086a74bdcf9f4df8e2edb749e41da5) )   // x 2 xxxxxxxxx0xxxxxxxxx = 0x00
	ROM_LOAD( "90015-03.w08",  0x000000, 0x080000, CRC(ccf5b158) SHA1(06250762646e0da1fb71fd7b638492eaab3f5b7f) )   // x 2 FIXED BITS (000x000x)
	ROM_LOAD( "90015-02.w07",  0x000000, 0x080000, CRC(fcbecc9b) SHA1(0670c276730ee282ef8c9599c00571b8d97725ab) )   // x 2
	ROM_LOAD( "90015-01.w06",  0x000000, 0x080000, CRC(ce4bfe6e) SHA1(d428eb3d5da3bd080957c585c5b72b94a7849fca) )   // x 2 FIXED BITS (000x000x)

	ROM_LOAD( "90015-20.r45",  0x000000, 0x080000, CRC(9d428fb7) SHA1(02f72938d73db932bd217620a175a05215f6016a) ) // x 2

	ROM_LOAD( "ch9072-4",  0x000000, 0x001000, CRC(5bc23535) SHA1(2fd1b7184175c416b19e6570de7ecb0d897deb9a) )   // FIXED BITS (0000000x)
	ROM_LOAD( "ch9072-5",  0x000000, 0x001000, CRC(0efac5b4) SHA1(a3e945aaf142bb62e0e791b8ca49a34891f31077) )   // FIXED BITS (xxxx0xxx)
	ROM_LOAD( "ch9072-6",  0x000000, 0x001000, CRC(76ff63c5) SHA1(652754533cc14773f4d7590a65183349eed9eb62) )
	ROM_LOAD( "ch9072-8",  0x000000, 0x001000, CRC(ca04bace) SHA1(3771ef4bf7983e97e3346309fcb0271e17a6d359) )   // FIXED BITS (0xxx0xxx)

	ROM_LOAD( "pr88004q",  0x000000, 0x000200, CRC(9327dc37) SHA1(cfe7b144cdcd76170d47f1c4e0f72b6d4fca0c8d) )   // FIXED BITS (1xxxxxxx1111x1xx)
	ROM_LOAD( "pr88004w",  0x000000, 0x000100, CRC(3d648467) SHA1(bf8dbaa2176c801f7370313425c87f0eefe8a3a4) )   // FIXED BITS (00xxxxxx)

	ROM_LOAD( "pr90015a",  0x000000, 0x000800, CRC(777583db) SHA1(8fd060a68fbb6156feb55afcfc5afd95999a8a62) )   // FIXED BITS (00000xxx0000xxxx)
	ROM_LOAD( "pr90015b",  0x000000, 0x000100, CRC(be240dac) SHA1(6203b73c1a5e09e525380a78b555c3818929d5eb) )   // FIXED BITS (000xxxxx000xxxx1)
ROM_END

DRIVER_INIT_MEMBER(cischeat_state,f1gpstar)
{
	cischeat_untangle_sprites("gfx4");
}


/***************************************************************************

                            Wild Pilot


Location   Device          File ID             Checksum
-------------------------------------------------------
LB IC124   27C020     GP-9188A_21_Ver1-0.bin     765B  [ CPU A PROG 1 ]
LB IC91    27C020     GP-9188A_26_Ver1-0.bin     5614  [ CPU A PROG 1 ]
LB IC92    27C020     GP-9188A_22_Ver1-4.bin     1907  [ CPU A PROG 2 ]
LB IC125   27C020     GP-9188A_27_Ver1-4.bin     A118  [ CPU A PROG 2 ]
LB IC46    27C010     GP-9188A_11_Ver1-0.bin     D98A  [ CPU B PROG   ]
LB IC70    27C010     GP-9188A_16_Ver1-0.bin     41E6  [ CPU B PROG   ]
LB IC2     27C010     GP-9188A_01_Ver1-0.bin     D714  [ CPU C PROG   ]
LB IC27    27C010     GP-9188A_06_Ver1-0.bin     3937  [ CPU C PROG   ]
LB IC23   27C4001     GP-9188A_25_Ver1-0.bin     90BE
LB IC152  27C4001     GP-9188A_28_Ver1-0.bin     6BBC
LB IC174  27C1001     GP-9188A_30_Ver1-0.bin     1740
LB IC14   27C4001         MR92020-01_C46.bin     E83D
LB IC15   27C4001         MR92020-02_C53.bin     064F
LB IC16   27C4001         MR92020-03_C52.bin     B77C
LB IC17   27C4001         MR92020-04_C47.bin     D768
LB IC61   27C4001         MR92020-05_C48.bin     7858
LB IC62   27C4001         MR92020-06_C51.bin     8B52
LB IC63   27C4001         MR92020-07_C49.bin     B2B6
LB IC64   27C4001         MR92020-08_C50.bin     37B7
LB IC37   27C4001         MR90015-01_w06.bin     F10F *
LB IC38   27C4001         MR90015-02_w07.bin     F901 *
LB IC39   27C4001         MR90015-03_w08.bin     F100 *
LB IC40   27C4001         MR90015-04_w09.bin     FA00 *
LB IC79   27C4001         MR90015-01_w06.bin     F10F *
LB IC80   27C4001         MR90015-02_w07.bin     F901 *
LB IC81   27C4001         MR90015-03_w08.bin     F100 *
LB IC82   27C4001         MR90015-04_w09.bin     FA00 *
MB IC1    27C4001         MR92020-11_W65.bin     80F8
MB IC2    27C4001         MR92020-12_W66.bin     2775
MB IC5    27C4001         MR92020-13_W67.bin     00D3
MB IC6    27C4001         MR92020-14_W68.bin     5645
MB IC11   27C4001         MR92020-15_W69.bin     F0BD
MB IC12   27C4001         MR92020-16_W70.bin     9F01
MB IC15    27C040      GP-9189_07_Ver1-0.bin     909A
MB IC16    27C040      GP-9189_08_Ver1-0.bin     2FBE
MB IC21    27C040      GP-9189_09_Ver1-0.bin     A33A
MB IC22    27C040      GP-9189_10_Ver1-0.bin     CE2E
MB IC54   27C4001         MR90015-35_W33.bin     7890 *
MB IC67   27C4001         MR90015-35_W33.bin     7890 *
TB IC4    27C4001      WP-92116_1_Ver1-0.bin     7485  [ CPU D SND    ]
TB IC18   27C4001      WP-92116_2_Ver1-0.bin     599D  [ CPU D SND    ]
TB IC37    27C010      WP-92116_3_Ver1-1.bin     506F  [ CPU D PROG   ]
TB IC38    27C010      WP-92116_4_Ver1-1.bin     F032  [ CPU D PROG   ]
TB IC116   27C010      WP-92116_5_Ver1-0.bin     A8E2  [ CPU E PROG   ]
TB IC117   27C010      WP-92116_6_Ver1-0.bin     253D  [ CPU E PROG   ]
MB IC39   27CX642               CH9072-4.bin     10BE #
MB IC33   27CX642               CH9072-5.bin     C83A #
MB IC35   27CX642               CH9072-6.bin     D5C2 #
MB IC59   27CX642               CH9072-8.bin     56C0 #
LB IC105  N82S147               PR88004Q.bin     FCFC
MB IC66   N82S135               PR88004W.bin     20C8
LB IC117  N82S185               PR90015A.bin     3326
LB IC153  N82S135               PR90015B.bin     1E52


Notes:  TB - Top board     (audio & I/O)       WP-92116 EB92020-20053
        MB - Middle board  (GFX)               GP-9189  EB90015-20038
        LB - Lower board   (CPU/GFX)           GP-9188A EB90015-20037-1

         * - These ROMs are found twice on the PCB
         # - Device is larger than it needs to be. A 27CX322 could be substituted.

Brief hardware overview:
------------------------

Main processor(A)  - 68000
                   - program ROMs: GP-9188A_21_Ver1-0.bin & GP-9188A_26_Ver1-0.bin
                                   GP-9188A_22_Ver1-4.bin & GP-9188A_27_Ver1-4.bin
                   - Processor RAM 2x LH52250AD-90L (32kx8 SRAM)

Slave processor(B) - 68000
                   - program ROMs: GP-9188A_11_Ver1-0.bin & , GP-9188A_16_Ver1-0.bin
                   - Processor RAM 2x LH5168D-10L (8kx8 SRAM)
                   - CS90015-04 (64 pin PQFP)  + 2x MCM2018AN45 (2kx8 SRAM)
                   - GS90015-05 (100 pin PQFP) + 2x MCM2018AN45 (2kx8 SRAM)

Slave processor(C) - 68000
                   - Program ROMs: GP-9188A_01_Ver1-0.bin & GP-9188A_06_Ver1-0.bin
                   - Processor RAM 2x LH5168D-10L (8kx8 SRAM)
                   - CS90015-04 (64 pin PQFP)  + 2x MCM2018AN45 (2kx8 SRAM)
                   - GS90015-05 (100 pin PQFP) + 2x MCM2018AN45 (2kx8 SRAM)

Sound processor(D) - 68000
                   - Program ROMs: WP-92116_3_Ver1-1.bin & WP-92116_4_Ver1-1.bin
                   - Processor RAM 2x LH52256ALSP-10 (32kx8 SRAM)
                   - M6295,  uses ROM  WP-92116_1_Ver1-0.bin
                   - M6295,  uses ROM  WP-92116_2_Ver1-0.bin
                   - YM2151
                   - YM3012

I/O Processor(E)   - 68000
                   - Program ROMs: WP-92116_5_Ver1-0.bin & WP-92116_6_Ver1-0.bin
                   - Processor RAM 2x LH5168D-10L (8kx8 SRAM)
                   - GS90015-03 (80 PIN PQFP) + 2X 6116 SRAM

GFX & Misc         - GS90015-02 (100 pin PQFP)
                     GS-9000406 (80 pin PQFP)  + 2x LH5168D-10L (8kx8 SRAM)

                   - GS90015-02 (100 pin PQFP)
                     GS-9000406 (80 pin PQFP)  + 2x LH5168D-10L (8kx8 SRAM)

                   - GS90015-02 (100 pin PQFP)
                     GS-9000406 (80 pin PQFP)  + 2x LH5168D-10L (8kx8 SRAM)

                   - GS900151   (44 pin PQFP) (too small for the full part No.)
               3x  - GS90015-03 (80 pin PQFP)  + 2x LH52258D-45 (32kx8 SRAM)
               2x  - GS90015-06 (100 pin PQFP) + 2x LH52250AD-90L (32kx8 SRAM)
                   - GS90015-07 (64 pin PQFP)
                   - GS90015-08 (64 pin PQFP)
                   - GS90015-09 (64 pin PQFP)  + 2x MCM2018AN45 (2kx8 SRAM)
                   - GS90015-10 (64 pin PQFP)
                   - GS90015-12 (80 pin PQFP)  + 2x MCM2018AN45 (2kx8 SRAM)
                   - GS90015-11 (100 pin PQFP)

***************************************************************************/

ROM_START( wildplt )
	ROM_REGION( 0x100000, "cpu1", 0 )
	ROM_LOAD16_BYTE( "gp-9188a_27_ver1-4.bin", 0x000000, 0x40000, CRC(4754f023) SHA1(32865343d633fef7d3e081d52b423fbf4e5f2053) )
	ROM_LOAD16_BYTE( "gp-9188a_22_ver1-4.bin", 0x000001, 0x40000, CRC(9e099111) SHA1(11b17d77dfe6315105e8297b8f5f7c8d93b18ac0) )

	ROM_REGION( 0x80000, "cpu2", 0 )
	ROM_LOAD16_BYTE( "gp-9188a_16_ver1-0.bin", 0x000000, 0x20000, CRC(75e48920) SHA1(2344c1f394aa1199169e75b2f998e89648a39100) )
	ROM_LOAD16_BYTE( "gp-9188a_11_ver1-0.bin", 0x000001, 0x20000, CRC(8174e65b) SHA1(2c35ae0d04fc4a075c297fe69c429f57457348e1) )

	ROM_REGION( 0x80000, "cpu3", 0 )
	ROM_LOAD16_BYTE( "gp-9188a_06_ver1-0.bin", 0x000000, 0x20000, CRC(99139696) SHA1(4f8c96012a8214ed952aa44d65ff92e9b3adcf73) )
	ROM_LOAD16_BYTE( "gp-9188a_01_ver1-0.bin", 0x000001, 0x20000, CRC(2c3d7ec4) SHA1(30215b2ea106aa3c8069fef9411e5f41be5645f6) )

	ROM_REGION( 0x40000, "soundcpu", 0 )
	ROM_LOAD16_BYTE( "wp-92116_3_ver1-1.bin", 0x000000, 0x20000, CRC(51dd594b) SHA1(3fbec13c1d2942797267820157180d374b38347c) )
	ROM_LOAD16_BYTE( "wp-92116_4_ver1-1.bin", 0x000001, 0x20000, CRC(9ba3cb7b) SHA1(6f55e2675bde2756a2996e6ab6aaef4c9d3f4724) )

	ROM_REGION( 0x80000, "cpu5", 0 )
	ROM_LOAD16_BYTE( "wp-92116_5_ver1-0.bin", 0x000000, 0x20000, CRC(8952e07c) SHA1(9025954c09c20dfe910a83283cfe010a5e898f38) )
	ROM_LOAD16_BYTE( "wp-92116_6_ver1-0.bin", 0x000001, 0x20000, CRC(2c8108c2) SHA1(759ab884a09669c8cb532f72aa14a3df2806c5f2) )

	ROM_REGION16_BE( 0x80000, "user1", 0 )  /* second halves of program ROMs */
	ROM_LOAD16_BYTE( "gp-9188a_26_ver1-0.bin", 0x000000, 0x40000, CRC(bc48db69) SHA1(d66fa43347b991b899e086bfcf9ceb6277b859a9) )
	ROM_LOAD16_BYTE( "gp-9188a_21_ver1-0.bin", 0x000001, 0x40000, CRC(c3192fbe) SHA1(c4a82a9174f6dc48946925ab94f81162632f58b0) )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "gp-9188a_25_ver1-0.bin", 0x000000, 0x80000, CRC(e69d3ccc) SHA1(10ab3d1980c571a478625ec4e505d711d90670cf) )

	ROM_REGION( 0x080000, "gfx2", 0 )
	ROM_LOAD( "gp-9188a_28_ver1-0.bin", 0x000000, 0x80000, CRC(2166c803) SHA1(14b3446fbd9e6d65cdcc20e8e0820008d561894b) )

	ROM_REGION( 0x020000, "gfx3", 0 )
	ROM_LOAD( "gp-9188a_30_ver1-0.bin", 0x000000, 0x20000, CRC(35478ed9) SHA1(4090ad8c529c1295799a0fe6d7b05d68ec2cf584) )

	ROM_REGION( 0x500000, "gfx4", 0 )   /* sprites */
	ROM_LOAD16_BYTE( "mr92020-11_w65.bin",    0x000000, 0x80000, CRC(585448cb) SHA1(578d241b928cc5b531c63b118fd1e7f1b864d5f3) )
	ROM_LOAD16_BYTE( "mr92020-12_w66.bin",    0x000001, 0x80000, CRC(07104f78) SHA1(757fa1e0722533f42f12c7e030a283b7e90fc225) )
	ROM_LOAD16_BYTE( "mr92020-13_w67.bin",    0x100000, 0x80000, CRC(c4afa3dc) SHA1(deda215a72914c61d500fee82e108997990d4057) )
	ROM_LOAD16_BYTE( "mr92020-14_w68.bin",    0x100001, 0x80000, CRC(c8676eca) SHA1(268cb259a974ca3f6bece090a53b1334ce77ac17) )
	ROM_LOAD16_BYTE( "mr92020-15_w69.bin",    0x200000, 0x80000, CRC(cc24a3d7) SHA1(5cfce80677a6bd0d787e0d1b9175ceb2662c3ffd) )
	ROM_LOAD16_BYTE( "mr92020-16_w70.bin",    0x200001, 0x80000, CRC(6f4c9d4e) SHA1(56bd05f4f80c544dce919d548be0b05cd0454582) )
	ROM_LOAD16_BYTE( "gp-9189_7_ver1-0.bin",  0x300000, 0x80000, CRC(1ea6a85d) SHA1(2c8e7cf18219bb0a0f212234f967a89510c6e21f) )
	ROM_LOAD16_BYTE( "gp-9189_8_ver1-0.bin",  0x300001, 0x80000, CRC(d9822da0) SHA1(9a4059727d1ed5a2dd09bde5131ff8f47348109c) )
	ROM_LOAD16_BYTE( "gp-9189_9_ver1-0.bin",  0x400000, 0x80000, CRC(0d4f6b5e) SHA1(92412590d17b7297188678a2b64c1b0ee7b00622) )
	ROM_LOAD16_BYTE( "gp-9189_10_ver1-0.bin", 0x400001, 0x80000, CRC(9240969c) SHA1(1f7995349787792a759b1ba60673f28d0fe15cfe) )

	ROM_REGION( 0x200000, "gfx5", 0 )
	ROM_LOAD( "mr92020-08_c50.bin", 0x000000, 0x80000, CRC(5e840567) SHA1(26e0278f455013600d37fc89eb83ce8bf11bb39d) )
	ROM_LOAD( "mr92020-07_c49.bin", 0x080000, 0x80000, CRC(48d8ecb2) SHA1(dee5c274576c4463d33895a12e01b1b30a6daa58) )
	ROM_LOAD( "mr92020-06_c51.bin", 0x100000, 0x80000, CRC(c00f1245) SHA1(11b9f6acbdf7094debb5a7a897afeb6a63b84103) )
	ROM_LOAD( "mr92020-05_c48.bin", 0x180000, 0x80000, CRC(74ef3306) SHA1(9c22250df5bd14d50bb27728fe40b7f9ec283c24) )

	ROM_REGION( 0x200000, "gfx6", 0 )
	ROM_LOAD( "mr92020-04_c47.bin", 0x000000, 0x80000, CRC(c752f467) SHA1(9faa15567677dd5cc141727b182ba8d6de08329d) )
	ROM_LOAD( "mr92020-03_c52.bin", 0x080000, 0x80000, CRC(985b5fe0) SHA1(a28eb20d37f171241fd0be702f2db12be1329836) )
	ROM_LOAD( "mr92020-02_c53.bin", 0x100000, 0x80000, CRC(da961dd4) SHA1(fa36ee94d0a40a0e6e7201df2b74413f23e02ae0) )
	ROM_LOAD( "mr92020-01_c46.bin", 0x180000, 0x80000, CRC(f908c6e0) SHA1(40c50a01b8b3fb184b8fa9824f95652399d479e2) )

	ROM_REGION( 0x80000, "oki1", 0 )    /* samples */
	ROM_LOAD( "wp-92116_2_ver1-0.bin", 0x000000, 0x80000, CRC(95237bd0) SHA1(45780d587565edf8ee0b6443bdc44db72fe65a86) )

	ROM_REGION( 0x80000, "oki2", 0 )    /* samples */
	ROM_LOAD( "wp-92116_1_ver1-0.bin", 0x000000, 0x80000, CRC(559bafc3) SHA1(612fa66b02e72a93ea5e89b40426b6ffb2a2a373) )

	ROM_REGION( 0x80000, "user2", 0 )       /* ? Unused ROMs ? */
	ROM_LOAD( "ch9072_4.bin", 0x000000, 0x2000, CRC(b45b4dc0) SHA1(b9fae0c9ac2d40f0a202c538d866d5f2941ba8dd) )
	ROM_LOAD( "ch9072_5.bin", 0x000000, 0x2000, CRC(e122916b) SHA1(86d5ecc7ecc6f175ecb28459697ef33e1ee06860) )
	ROM_LOAD( "ch9072_6.bin", 0x000000, 0x2000, CRC(05d95bf7) SHA1(78181cf71f22c090a1e62823a43757353a9ef6ab) )
	ROM_LOAD( "ch9072_8.bin", 0x000000, 0x2000, CRC(6bf52596) SHA1(bf4e7e7df3daae4aa6a441b58b15a435aa45630e) )

	ROM_LOAD( "mr90015-01_w06.bin", 0x000000, 0x80000, CRC(ce4bfe6e) SHA1(d428eb3d5da3bd080957c585c5b72b94a7849fca) ) // x 2
	ROM_LOAD( "mr90015-02_w07.bin", 0x000000, 0x80000, CRC(fcbecc9b) SHA1(0670c276730ee282ef8c9599c00571b8d97725ab) ) // x 2
	ROM_LOAD( "mr90015-03_w08.bin", 0x000000, 0x80000, CRC(ccf5b158) SHA1(06250762646e0da1fb71fd7b638492eaab3f5b7f) ) // x 2
	ROM_LOAD( "mr90015-04_w09.bin", 0x000000, 0x80000, CRC(5b324c81) SHA1(ce61f2ea29086a74bdcf9f4df8e2edb749e41da5) ) // x 2
	ROM_LOAD( "mr90015_35_w33.bin", 0x000000, 0x80000, CRC(9d428fb7) SHA1(02f72938d73db932bd217620a175a05215f6016a) )

	ROM_LOAD( "pr88004q.bin", 0x000000, 0x0200, CRC(9327dc37) SHA1(cfe7b144cdcd76170d47f1c4e0f72b6d4fca0c8d) )
	ROM_LOAD( "pr88004w.bin", 0x000000, 0x0100, CRC(3d648467) SHA1(bf8dbaa2176c801f7370313425c87f0eefe8a3a4) )

	ROM_LOAD( "pr90015a.bin", 0x000000, 0x0800, CRC(777583db) SHA1(8fd060a68fbb6156feb55afcfc5afd95999a8a62) )
	ROM_LOAD( "pr90015b.bin", 0x000000, 0x0100, CRC(be240dac) SHA1(6203b73c1a5e09e525380a78b555c3818929d5eb) )
ROM_END

DRIVER_INIT_MEMBER(cischeat_state,wildplt)
{
	m_cpu1->space(AS_PROGRAM).install_read_handler(0x080000, 0x087fff, read16_delegate(FUNC(cischeat_state::wildplt_vregs_r),this));

	DRIVER_INIT_CALL(f1gpstar);
}


/***************************************************************************

                            F1 GrandPrix Star II

This game uses the same bottom and middle boards as Grand Prix Star, however the top board
(sound + I/O) is different (though I guess it has the same purpose.)
The game is mostly a simple ROM swap on a F1 Grand Prix Star board.
The swapped ROMs have been factory socketed. Some ROMs are soldered-in and match some in the
F1 Grand Prix Star archive (i.e. they are not swapped).

Top Board
---------
PCB No: WP-92116 EB92020-20053
CPUs  : MC68000P10, TMP68000P-12
SOUND : OKI M6295 (x2), YM2151, YM3012
XTALs : 4.000MHz, 7.000MHz, 24.000MHz
DIPs  : 8 position (x3)
RAM   : MCM2018AN45 (x4, 2kx8 SRAM), LH5168D-10L (x2, 8kx8 SRAM), HM62256ALSP-10 (x2, 32kx8 SRAM)
CUSTOM: GS90015-12 (QFP80)
        GS90015-03 (QFP80)
ROMs  :
        (extension = IC location on PCB)
        92116-1.4     27c040   \
        92116-2.18    27c040   |
        92116-3.37    27c010   |  Near MC68000P10 and YM2151
        92116-4.38    27c010   /

        92116-5.116   27c2001  \
        92116-6.117   27c2001  /  Near TMP68000P-12

Notes : Labels on all these ROMs say Ver 1.0



Middle Board
------------
PCB No: GP-9189 EB90015-90038
RAM   : HM62256ALSP-12 (x8, 32kx8 SRAM), MCM2018AN45 (x2, 2kx8 SRAM)
CUSTOM: GS90015-11 (QFP100)
        GS90015-08 (QFP64)
        GS90015-07 (QFP64)
        GS90015-10 (QFP64)
        GS90015-09 (QFP64)
        GS90015-03 (x2, QFP80)
        GS90015-06 (x2, QFP100)
ROMs  :
        (extension = IC location on PCB)
        92021-11.1    4M MASK   \
        92021-12.2       "      |
        92021-13.11      "      |  Socketed + Swapped
        92021-14.12      "      |
        92021-15.21      "      |
        92021-16.22      "      /

        90015-35.54   4M MASK   \  Matches 90015-20.R45 from f1gpstar archive! Mis-labelled filename?)
        90015-35.67      "      /  Soldered-in, not swapped. Both are identical, near GS90015-08

PROMs :
        (extension = IC location on PCB)
        CH9072_4.39 \
        CH9072_5.33 |  unusual type ICT 27CX322 with a clear window (re-programmable!), purpose = ?
        CH9072_6.35 |
        CH9072_8.59 /

        PR88004W.66    type 82s135, near 90015-35


Bottom Board
------------
PCB No: GP-9188A  EB90015-20037-1
CPU   : TMP68000P-12 (x3)
XTAL  : 24.000MHz
RAM   : HM62256ALSP-10 (x2, 32kx8 SRAM), LH5168D-10L (x10, 8kx8 SRAM)
        MCM2018AN45 (x8, 2kx8 SRAM), PDM41256SA20P (x2, 32kx8 SRAM)
CUSTOM: GS90015-03 (QFP80)
        GS9000406  (x3, QFP80)
        GS90015-02 (x3, QFP100)
        GS90015-01 (QFP44)
        GS90015-05 (x2, QFP100)
        GS90015-04 (x2, QFP64)

ROMs  :
        (extension = IC location on PCB)
        92021-01.14   4M MASK   \                                       \
        92021-02.15      "      |                                       |
        92021-03.16      "      |  Socketed + Swapped                   |
        92021-04.17      "      /                                       |
        90015-01.37   4M MASK   \                                       |
        90015-02.38      "      |   Soldered-in, not swapped, matches   |
        90015-03.39      "      |   same ROMs in F1 Grand Prix archive  |
        90015-04.40      "      /                                       | All these are
        92021-05.61   4M MASK   \                                       | grouped together
        92021-06.62      "      |                                       |
        92021-07.63      "      |  Socketed + Swapped                   |
        92021-08.64      "      /                                       |
        90015-01.79   4M MASK   \                                       |
        90015-02.80      "      |  Soldered-in, not swapped, matches    |
        90015-03.81      "      |  same ROMs in F1 Grand Prix archive   |
        90015-04.82      "      /                                       /

        9188A-25.123  27c040    Label says Ver 1.0   \
        9188A-28.152     "      Label says Ver 4.0   |  Near GS90015-02
        9188A-30.174  27c1001   Label says Ver 1.0   /

        9188A-21.91   27c2001   Label says Ver 4.0   \
        9188A-22.92      "      Label says Ver 4.0   |  Near a 68000
        9188A-26.124     "      Label says Ver 4.0   |
        9188A-27.125     "      Label says Ver 4.0   /

        9188A-11.46   27c1001   Label says Ver 1.0   \  Near a 68000
        9188A-16.70      "      Label says Ver 1.0   /

        9188A-1.2     27c1001   Label says Ver 1.0   \  Near a 68000
        9188A-6.27       "      Label says Ver 1.0   /

PROMs :
        (extension = IC location on PCB)
        PR90015A.117   type 82s185, near GS9000406
        PR90015B.153   type 82s135, near 9188A-26.124
        PR88004Q.105   type 82s147, near GS90015-01


***************************************************************************/

ROM_START( f1gpstr2 )
	ROM_REGION( 0x100000, "cpu1", 0 )
	ROM_LOAD16_BYTE( "9188a-27.125", 0x000000, 0x040000, CRC(ee60b894) SHA1(92247cd3b0e3fb2ed0ad27062d1cc13dadb21465) )
	ROM_LOAD16_BYTE( "9188a-22.92" , 0x000001, 0x040000, CRC(f229332b) SHA1(f7037515d77a1f42ce555b8baa15075d7009a5c6) )

	ROM_REGION( 0x80000, "cpu2", 0 )
	ROM_LOAD16_BYTE( "9188a-16.70",  0x000000, 0x020000, CRC(3b3d57a0) SHA1(2ad367fc938c385160a014b994fe45d1791d4ddb) )
	ROM_LOAD16_BYTE( "9188a-11.46",  0x000001, 0x020000, CRC(4d2afddf) SHA1(108ddbb6a008d390e43d88efd10d99b599c1d75d) )

	ROM_REGION( 0x80000, "cpu3", 0 )
	ROM_LOAD16_BYTE( "9188a-6.27",  0x000000, 0x020000, CRC(68faa23b) SHA1(b18c407cc1273295ec9fc30af191fccd94447ae8) )
	ROM_LOAD16_BYTE( "9188a-1.2",   0x000001, 0x020000, CRC(e4d83b4f) SHA1(4022521e43f6361c5e04a604f8a7fa1e60a2ac99) )

	ROM_REGION( 0x40000, "soundcpu", 0 )
	ROM_LOAD16_BYTE( "92116-3.37", 0x000000, 0x020000, CRC(2a541095) SHA1(934ef9b6bbe3f6e2e2649dde5671547b955ffc7c) )
	ROM_LOAD16_BYTE( "92116-4.38", 0x000001, 0x020000, CRC(70da1825) SHA1(7f2077e9b40d5acf4da3f6bcc5d7b92d6d08f861) )

	ROM_REGION( 0x80000, "cpu5", 0 )
	ROM_LOAD16_BYTE( "92116-5.116",  0x000000, 0x040000, CRC(da16db49) SHA1(a07fb706b0c93a83148a9fdaaca1bc5414bfe286) )
	ROM_LOAD16_BYTE( "92116-6.117",  0x000001, 0x040000, CRC(1f1e147a) SHA1(ebedcdad9cfda8fa3b5c2653232209da5be237e1) )

	ROM_REGION16_BE( 0x80000, "user1", 0 )  /* second halves of program ROMs */
	ROM_LOAD16_BYTE( "9188a-26.124", 0x000000, 0x040000, CRC(8690bb79) SHA1(8ef822cf8371cb209c30cfe5c4d5e8b36392f732) ) // cpu1
	ROM_LOAD16_BYTE( "9188a-21.91",  0x000001, 0x040000, CRC(c5a5807e) SHA1(15493030d154579d2095c7304dd843aed09a69ec) )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "9188a-25.123",  0x000000, 0x080000, CRC(000c83d2) SHA1(656e4553873ad945bbd770166cc3add287e525dd) ) // scroll 0

	ROM_REGION( 0x080000, "gfx2", 0 )
	ROM_LOAD( "9188a-28.152",  0x000000, 0x080000, CRC(e734f8ea) SHA1(efed2ce4a23d16a38872892c25fa9824ca0fed8e) ) // scroll 1

	ROM_REGION( 0x020000, "gfx3", 0 )
	ROM_LOAD( "9188a-30.174",  0x000000, 0x020000, CRC(c5906023) SHA1(6006c86995abef1442232ff5fbd68c2a37038d1f) ) // scroll 2

	ROM_REGION( 0x600000, "gfx4", 0 )   /* sprites */
	ROM_LOAD16_BYTE( "92021-11.1",  0x000000, 0x100000, CRC(fec883a7) SHA1(a17ca17fa37b35c5f28cede52efa91afe566a95a) )
	ROM_LOAD16_BYTE( "92021-12.2",  0x000001, 0x100000, CRC(df283a7e) SHA1(4e9eed9475186e6f94d9ef84a798fa807fb37903) )
	ROM_LOAD16_BYTE( "92021-13.11", 0x200000, 0x100000, CRC(1ceb593a) SHA1(e8428c42d10aa0d26717176b1bdea9a4a1d3e5f3) )
	ROM_LOAD16_BYTE( "92021-14.12", 0x200001, 0x100000, CRC(673c2c61) SHA1(5e04ac452ffd0747beaa42daca873a09cf179b18) )
	ROM_LOAD16_BYTE( "92021-15.21", 0x400000, 0x100000, CRC(66e8b197) SHA1(10f6c5beb4ab57fbd1940db0e15d07df486e4351) )
	ROM_LOAD16_BYTE( "92021-16.22", 0x400001, 0x100000, CRC(1f672dd8) SHA1(f75b8f3f9512e2ef085170888e621f54ee94f9d5) )

	ROM_REGION( 0x200000, "gfx5", 0 )
	ROM_LOAD( "92021-08.64",  0x000000, 0x080000, CRC(54ff00c4) SHA1(f86f16c77b211206fbe39efa278634db8a3eaf75) ) // Road 0
	ROM_LOAD( "92021-07.63",  0x080000, 0x080000, CRC(258d524a) SHA1(f2ba03b7fec81377b032476703cafe0fe79f6a2a) )
	ROM_LOAD( "92021-06.62",  0x100000, 0x080000, CRC(f1423efe) SHA1(bd45ba2b7908d10dc4df10b9c04dca6830894d2a) )
	ROM_LOAD( "92021-05.61",  0x180000, 0x080000, CRC(88bb6db1) SHA1(54413c41a4d02137aebc2a4866a38aadfe64825a) )

	ROM_REGION( 0x200000, "gfx6", 0 )
	ROM_LOAD( "92021-04.17",  0x000000, 0x080000, CRC(3a2e6b1e) SHA1(350465ade24c16e4fe39613f89bf3e7277cdd31e) ) // Road 1
	ROM_LOAD( "92021-03.16",  0x080000, 0x080000, CRC(1f041f65) SHA1(cc4defe3675b30e7de4b3e1eb580a71af4c36bc6) )
	ROM_LOAD( "92021-02.15",  0x100000, 0x080000, CRC(d0582ad8) SHA1(b343a6525bb9d7dbb288ddec392b23e85ae150bb) )
	ROM_LOAD( "92021-01.14",  0x180000, 0x080000, CRC(06e50be4) SHA1(60ced74d97ac5f641b7e721484abbe74522fe3ba) )

	ROM_REGION( 0x80000, "oki1", 0 )    /* samples */
	ROM_LOAD( "92116-2.18", 0x000000, 0x080000, CRC(8c06cded) SHA1(789d620dddfebc1343d424fccd03dd51b58c50fa) )

	ROM_REGION( 0x80000, "oki2", 0 )    /* samples */
	ROM_LOAD( "92116-1.4",  0x000000, 0x080000, CRC(8da37b9d) SHA1(ffc67901d087e63bfbb36d15d75c57f6847ef6ea) )

	ROM_REGION( 0x80000, "user2", 0 )       /* ? Unused ROMs ? */
	ROM_LOAD( "90015-04.17",  0x000000, 0x080000, CRC(5b324c81) SHA1(ce61f2ea29086a74bdcf9f4df8e2edb749e41da5) )    // x 2 xxxxxxxxx0xxxxxxxxx = 0x00
	ROM_LOAD( "90015-04.82",  0x000000, 0x080000, CRC(5b324c81) SHA1(ce61f2ea29086a74bdcf9f4df8e2edb749e41da5) )    //
	ROM_LOAD( "90015-03.16",  0x000000, 0x080000, CRC(ccf5b158) SHA1(06250762646e0da1fb71fd7b638492eaab3f5b7f) )    // x 2 FIXED BITS (000x000x)
	ROM_LOAD( "90015-03.81",  0x000000, 0x080000, CRC(ccf5b158) SHA1(06250762646e0da1fb71fd7b638492eaab3f5b7f) )    //
	ROM_LOAD( "90015-02.15",  0x000000, 0x080000, CRC(fcbecc9b) SHA1(0670c276730ee282ef8c9599c00571b8d97725ab) )    // x 2
	ROM_LOAD( "90015-02.80",  0x000000, 0x080000, CRC(fcbecc9b) SHA1(0670c276730ee282ef8c9599c00571b8d97725ab) )    //
	ROM_LOAD( "90015-01.14",  0x000000, 0x080000, CRC(ce4bfe6e) SHA1(d428eb3d5da3bd080957c585c5b72b94a7849fca) )    // x 2 FIXED BITS (000x000x)
	ROM_LOAD( "90015-01.79",  0x000000, 0x080000, CRC(ce4bfe6e) SHA1(d428eb3d5da3bd080957c585c5b72b94a7849fca) )    //

	ROM_LOAD( "90015-35.54",  0x000000, 0x080000, CRC(9d428fb7) SHA1(02f72938d73db932bd217620a175a05215f6016a) ) // x 2
	ROM_LOAD( "90015-35.67",  0x000000, 0x080000, CRC(9d428fb7) SHA1(02f72938d73db932bd217620a175a05215f6016a) ) //

	ROM_LOAD( "ch9072_4.39",  0x000000, 0x002000, CRC(b45b4dc0) SHA1(b9fae0c9ac2d40f0a202c538d866d5f2941ba8dd) )    // FIXED BITS (0000000x)
	ROM_LOAD( "ch9072_5.33",  0x000000, 0x002000, CRC(e122916b) SHA1(86d5ecc7ecc6f175ecb28459697ef33e1ee06860) )    // FIXED BITS (xxxx0xxx)
	ROM_LOAD( "ch9072_6.35",  0x000000, 0x002000, CRC(05d95bf7) SHA1(78181cf71f22c090a1e62823a43757353a9ef6ab) )
	ROM_LOAD( "ch9072_8.59",  0x000000, 0x002000, CRC(6bf52596) SHA1(bf4e7e7df3daae4aa6a441b58b15a435aa45630e) )    // FIXED BITS (0xxx0xxx)

	ROM_LOAD( "pr88004q.105", 0x000000, 0x000200, CRC(9327dc37) SHA1(cfe7b144cdcd76170d47f1c4e0f72b6d4fca0c8d) )    // FIXED BITS (1xxxxxxx1111x1xx)
	ROM_LOAD( "pr88004w.66",  0x000000, 0x000100, CRC(3d648467) SHA1(bf8dbaa2176c801f7370313425c87f0eefe8a3a4) )    // FIXED BITS (00xxxxxx)

	ROM_LOAD( "pr90015a.117", 0x000000, 0x000800, CRC(777583db) SHA1(8fd060a68fbb6156feb55afcfc5afd95999a8a62) )    // FIXED BITS (00000xxx0000xxxx)
	ROM_LOAD( "pr90015b.153", 0x000000, 0x000100, CRC(be240dac) SHA1(6203b73c1a5e09e525380a78b555c3818929d5eb) )    // FIXED BITS (000xxxxx000xxxx1)
ROM_END


/***************************************************************************

                                Scud Hammer

CF-92128B:

                                                      GS9001501
 2-H 2-L  6295            62256 62256
 1-H 1-L  6295  68000-12  3     4       6  GS90015-02 8464 8464 GS600406

                    24MHz               5  GS90015-02 8464 8464 GS900406

                                                       7C199
                                                       7C199 GS90015-03

GP-9189:

 1     2      62256                            62256
 3     4      62256    GS90015-06 GS90015-06   62256
 5     6      62256                            62256
 7     8      62256    GS90015-03 GS90015-03   62256
 9     10

                  GS90015-08            GS90015-07 GS90015-10

          GS90015-11


                      MR90015-35
                      MR90015-35              GS90015-09

***************************************************************************/

ROM_START( scudhamm )
	ROM_REGION( 0x080000, "maincpu", 0 )        /* Main CPU Code */
	ROM_LOAD16_BYTE( "3", 0x000000, 0x040000, CRC(a908e7bd) SHA1(be0a8f959ab5c19122eee6c3def6137f37f1a9c6) )
	ROM_LOAD16_BYTE( "4", 0x000001, 0x040000, CRC(981c8b02) SHA1(db6c8993bf1c3993ab31dd649022ab76169975e1) )

	ROM_REGION( 0x080000, "gfx1", 0 ) /* Scroll 0 */
	ROM_LOAD( "5", 0x000000, 0x080000, CRC(714c115e) SHA1(c3e88b3972e3926f37968f3e84b932e1ac177142) )

//  ROM_REGION( 0x080000, "gfx2", 0 ) /* Scroll 1 */
//  UNUSED

	ROM_REGION( 0x020000, "gfx3", 0 ) /* Scroll 2 */
	ROM_LOAD( "6", 0x000000, 0x020000, CRC(b39aab63) SHA1(88275cce8b1323b2d835390a8fc2380b90d50d95) ) // 1xxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x500000, "gfx4", 0 ) /* Sprites */
	ROM_LOAD16_BYTE( "1.bot",  0x000000, 0x080000, CRC(46450d73) SHA1(c9acdf1cef760e5194c346d721e859c61afbfce6) )
	ROM_LOAD16_BYTE( "2.bot",  0x000001, 0x080000, CRC(fb7b66dd) SHA1(ad6bbae4fa72f957e5c0fc7bf6199ac45f837dac) )
	ROM_LOAD16_BYTE( "3.bot",  0x100000, 0x080000, CRC(7d45960b) SHA1(abf59cf85f28c90d4c08e3a1e5408a9a700071cc) )
	ROM_LOAD16_BYTE( "4.bot",  0x100001, 0x080000, CRC(393b6a22) SHA1(0d002a8c09de2fb8aaa7f5f020badc6fc096fa41) )
	ROM_LOAD16_BYTE( "5.bot",  0x200000, 0x080000, CRC(7a3c33ad) SHA1(fe0e3722e15919ae3acfeeacae57716aae43647c) )
	ROM_LOAD16_BYTE( "6.bot",  0x200001, 0x080000, CRC(d19c4bf7) SHA1(b8aa21920d5a02f10a7ae65ade8a0a88ad23f373) )
	ROM_LOAD16_BYTE( "7.bot",  0x300000, 0x080000, CRC(9e5edf59) SHA1(fcd4136e39d40bcce365153c96e06181a24a480a) )
	ROM_LOAD16_BYTE( "8.bot",  0x300001, 0x080000, CRC(4980051e) SHA1(10de91239b5b4dab8e7fa4bf51d93356c5111ddf) )
	ROM_LOAD16_BYTE( "9.bot",  0x400000, 0x080000, CRC(c1b301f1) SHA1(776b9889703d73afc4fb0ff77498b98c943246d3) )
	ROM_LOAD16_BYTE( "10.bot", 0x400001, 0x080000, CRC(dab4528f) SHA1(f5ddc37a2d106d5438ad1b7d23a2bbbce07f2c89) )

	ROM_REGION( 0x100000, "oki1", 0 )       /* Samples (4x40000) */
	ROM_LOAD( "2.l",  0x000000, 0x080000, CRC(889311da) SHA1(fcaee3e6c98a784cfde06fc2e0e8f5abbfb4df6c) )
	ROM_LOAD( "2.h",  0x080000, 0x080000, CRC(347928fc) SHA1(36903c38b9f13594de40dfc697326327c7010d65) )

	ROM_REGION( 0x100000, "oki2", 0 )       /* Samples (4x40000) */
	ROM_LOAD( "1.l",  0x000000, 0x080000, CRC(3c94aa90) SHA1(f9278fec9d93dac0309f30e35c727bd481f347d4) )
	ROM_LOAD( "1.h",  0x080000, 0x080000, CRC(5caee787) SHA1(267f4d3c28e71e53180a5d0ff27a6555ac6fa4a0) )    // 1xxxxxxxxxxxxxxxxxx = 0xFF
ROM_END


/***************************************************************************

Arm Champs II
Jaleco, 1992

An arm wrestling game from Jaleco. Game is part mechanical, part video. The video part
is shown on a vertical screen, being your opponent you are wrestling. The mechanical
part is an arm you force against to pin your opponent's 'arm'. The mechanical arm is powered
by a motor. The arm probably has a 5k potentiometer connected to it to give feedback to the
hardware about the position of the mechanical arm.
The opponents get progressively harder and harder, it's almost impossible to beat the final
few opponents unless you have a few friends handy to swing on the arm ;-))
Arm Champs II appears to be a simple ROM upgrade on Arm Champs hardware.
The hardware appears to be similar to Jaleco's 'Grand Prix Star' etc, using many of
the same custom IC's, and even some of the same ROMs.


PCB Layouts
-----------

(top board)
EB91042-20048-3
AC-91106B
|---------------------------------------------------------------------------|
| uPC1318                                         GS9001501   TL7705        |
|          |-------------|                                                  |
|POWER     |   68000-12  |                                  PR88004Q_8.IC102|
|          |-------------|                                                  |
|                         24MHz                        AC91106_VER1.2_7.IC99|
|                                                                           |
|                                                 GS9000406    GS90015-02   |
|                                                                          |-|
|                                                   6264          6264     | |
|                                     AC91106_VER1.7_4.IC63                | |
|C                                                                         | |
|U                                    AC91106_VER1.7_3.IC62                | |
|S                                                                         | |
|T                                                    MR91042-07-R66_6.IC95| |
|O                                       62256                             |-|
|M                                                                          |
|7                                       62256    GS9000406    GS90015-02   |
|2                     ADC0804                                              |
|P                                                               6264       |
|I                                                                         |-|
|N                                                GS90015-03     6264      | |
|                                                                          | |
|                                 M6295                     PR91042_5.IC91 | |
|                                      MR91042-08_2.IC57                   | |
|                                                                          | |
|                                 M6295                                    | |
|                PMI_PM7528            AC91106_VER1.2_1.IC56               | |
|                                                     62256                | |
|                                                     62256                | |
| POWER     DSW1   DSW2            4MHz                                    |-|
|---------------------------------------------------------------------------|
Notes:
      68000 clock       - 12.0MHz [24/2]
      Jaleco Custom ICs -
                         GS9001501 (QFP44)
                         GS9000406 (QFP80, x2)
                         GS90015-02 (QFP100, x2)
                         GS90015-03 (QFP80)

      ROMs -
            PR88004Q_8.IC102      - 82S147 PROM
            AC91106_VER1.2_7.IC99 - 27C010 EPROM
            MR91042007-R66_6.IC95 - 4M MASKROM
            AC91106_VER1.7_4.IC63 - 27C010 EPROM
            AC91106_VER1.7_3.IC62 - 27C010 EPROM
            MR91042-08_2.IC57     - 4M MASKROM
            AC91106_VER1.2_1.IC56 - 27C4001 EPROM
            PR91042_5.IC91        - 82S129 PROM


(lower board)
GP-9189
EB91015-20038
|---------------------------------------------------------------------------|
|MR91042-01-R60_1.IC1                                                       |
|        MR91042-02-R61_2.IC2    62256                             62256    |
|                                       GS90015-06    GS90015-06            |
|MR91042-03-R62_3.IC5                                                       |
|        MR91042-04-R63_4.IC6    62256                             62256    |
|                                                                           |
|MR91042-05-R64_5.IC11                                                      |
|        MR91042-06-R65_6.IC12   62256                             62256   |-|
|                                       GS90015-03    GS90015-03           | |
|                                                                          | |
|                                62256                             62256   | |
|                                                                          | |
|                                                                          | |
|                        GS90015-08                                        | |
|                                               GS90015-07   GS90015-10    |-|
|                                                                           |
|                                                                           |
| CH9072-5_11.IC33                                                          |
|              GS90015-11              CH9072-6_12.IC35                     |
|                                      CH9072-4_13.IC39                    |-|
|                                                                          | |
|                                                                          | |
|                          MR90015-35-W33_14.IC54                          | |
|                                                                          | |
|      CH9072-8_15.IC59    MR90015-35-W33_17.IC67                          | |
|                                                    GS90015-09            | |
|              PR88004W_16.IC66                                            | |
|                                                                          | |
|                                                      6116                | |
| POWER                                                6116                |-|
|---------------------------------------------------------------------------|
Notes:
      Jaleco Custom ICs -
                         GS90015-03 (QFP80, x2)
                         GS90015-06 (QFP100, x2)
                         GS90015-07 (QFP64)
                         GS90015-08 (QFP64)
                         GS90015-09 (QFP64)
                         GS90015-10 (QFP64)
                         GS90015-11 (QFP100)

      ROMs -
            PR88004W_16.IC66      - 82S135 PROM
            CH9072-4_13.IC39      - Atmel AT27HC642R
            CH9072-5_11.IC33      - Atmel AT27HC642R
            CH9072-6_12.IC35      - Atmel AT27HC642R
            CH9072-8_15.IC59      - Atmel AT27HC642R
            MR90015-35-W33_14.IC54- 4M MASKROM  \
            MR90015-35-W33_17.IC67- 4M MASKROM  / Contain same data
            MR91042-01-R60_1.IC1  - 4M MASKROM
            MR91042-02-R61_2.IC2  - 4M MASKROM
            MR91042-03-R62_3.IC5  - 4M MASKROM
            MR91042-04-R63_4.IC6  - 4M MASKROM
            MR91042-05-R64_5.IC11 - 4M MASKROM
            MR91042-06-R65_6.IC12 - 4M MASKROM

***************************************************************************/

ROM_START( armchmp2 )
	ROM_REGION( 0x080000, "maincpu", 0 )        /* Main CPU Code */
	ROM_LOAD16_BYTE( "ac-91106v2.6_4.ic63", 0x000000, 0x020000, CRC(e0cec032) SHA1(743b022b6de3efb045c4f1cca49caed0259ccfff) )
	ROM_LOAD16_BYTE( "ac-91106v2.6_3.ic62", 0x000001, 0x020000, CRC(5de6da19) SHA1(1f46056596924789394ad2d99ec2d7fcb7845d3c) )

	ROM_REGION( 0x080000, "gfx1", 0 ) /* Scroll 0 */
	ROM_LOAD( "mr91042-07-r66_6.ic95",  0x000000, 0x080000, CRC(d1be8699) SHA1(67563761f95892b08c7113ab1c52ab5aa7118fb8) )

//  ROM_REGION( 0x080000, "gfx2", 0 ) /* Scroll 1 */
//  UNUSED

	ROM_REGION( 0x020000, "gfx3", 0 ) /* Scroll 2 */
	ROM_LOAD( "ac91106_ver1.2_7.ic99", 0x000000, 0x020000, CRC(09755aef) SHA1(39c901fb9408a0ba488f0112d7f48b929b092e3b) )

	ROM_REGION( 0x300000, "gfx4", 0 ) /* Sprites */
	ROM_LOAD16_BYTE( "mr91042-01-r60_1.ic1",  0x000000, 0x080000, CRC(fdfe6951) SHA1(ba6c5cd5d16fdca6f131302b19e621f8abe8136a) )
	ROM_LOAD16_BYTE( "mr91042-02-r61_2.ic2",  0x000001, 0x080000, CRC(2e6c8b30) SHA1(70503fec251606b37fea2c7f91e682aece252035) )
	ROM_LOAD16_BYTE( "mr91042-03-r62_3.ic5",  0x100000, 0x080000, CRC(07ba6d3a) SHA1(9c58e3a1931b593448c53a59e7f5b9aaac40ff88) )
	ROM_LOAD16_BYTE( "mr91042-04-r63_4.ic6",  0x100001, 0x080000, CRC(f37cb12c) SHA1(282ebbd795284d7efa335b797ca1eedc1110e9da) )
	ROM_LOAD16_BYTE( "mr91042-05-r64_5.ic11", 0x200000, 0x080000, CRC(7a3bb52d) SHA1(7f9d1dad4c89e6b55415b082363bc261115e9f96) )
	ROM_LOAD16_BYTE( "mr91042-06-r65_6.ic12", 0x200001, 0x080000, CRC(5312a4f2) SHA1(4dcd2839bb5acccecf1eb6c0e19e877a0cff6875) )

	ROM_REGION( 0x100000, "oki1", 0 )       /* Samples (4x40000) */
	ROM_LOAD( "mr91042-08_2.ic57",     0x000000, 0x080000, CRC(dc015f6c) SHA1(9d0677c50a25be1d11d43e54dbf3005f18b79b66) )
	ROM_RELOAD(                        0x080000, 0x080000 )

	ROM_REGION( 0x100000, "oki2", 0 )       /* Samples (4x40000) */
	ROM_LOAD( "ac-91106v2.0_1.ic56", 0x000000, 0x080000, CRC(0ff5cbcf) SHA1(25ef8d67749ca78afc4c13a31b3f7a87284947c1) )
	ROM_RELOAD(                        0x080000, 0x080000 )

	ROM_REGION( 0x80000, "user2", 0 )       /* ? Unused ROMs ? */
	ROM_LOAD( "ch9072-4_13.ic39", 0x000000, 0x002000, CRC(b45b4dc0) SHA1(b9fae0c9ac2d40f0a202c538d866d5f2941ba8dd) )
	ROM_LOAD( "ch9072-5_11.ic33", 0x000000, 0x002000, CRC(e122916b) SHA1(86d5ecc7ecc6f175ecb28459697ef33e1ee06860) )
	ROM_LOAD( "ch9072-6_12.ic35", 0x000000, 0x002000, CRC(05d95bf7) SHA1(78181cf71f22c090a1e62823a43757353a9ef6ab) )
	ROM_LOAD( "ch9072-8_15.ic59", 0x000000, 0x002000, CRC(6bf52596) SHA1(bf4e7e7df3daae4aa6a441b58b15a435aa45630e) )

	ROM_LOAD( "mr90015-35-w33_17.ic67", 0x000000, 0x080000, CRC(9d428fb7) SHA1(02f72938d73db932bd217620a175a05215f6016a) )
	ROM_LOAD( "mr90015-35-w33_14.ic54", 0x000000, 0x080000, CRC(9d428fb7) SHA1(02f72938d73db932bd217620a175a05215f6016a) )

	ROM_LOAD( "pr88004q_8.ic102", 0x000000, 0x000200, CRC(9327dc37) SHA1(cfe7b144cdcd76170d47f1c4e0f72b6d4fca0c8d) )
	ROM_LOAD( "pr88004w_16.ic66", 0x000000, 0x000100, CRC(3d648467) SHA1(bf8dbaa2176c801f7370313425c87f0eefe8a3a4) )

	ROM_LOAD( "pr91042_5.ic91", 0x000000, 0x000100, CRC(e71de4aa) SHA1(d06e5a35ad2127df2d6328cce153073380ee7819) )
ROM_END

ROM_START( armchmp2o )
	ROM_REGION( 0x080000, "maincpu", 0 )        /* Main CPU Code */
	ROM_LOAD16_BYTE( "ac91106_ver1.7_4.ic63", 0x000000, 0x020000, CRC(aaa11bc7) SHA1(ac6186f45a006074d3a86d7437c5a3411bf27188) )
	ROM_LOAD16_BYTE( "ac91106_ver1.7_3.ic62", 0x000001, 0x020000, CRC(a7965879) SHA1(556fecd6ea0f977b718d132c4180bb2160b9da7e) )

	ROM_REGION( 0x080000, "gfx1", 0 ) /* Scroll 0 */
	ROM_LOAD( "mr91042-07-r66_6.ic95",  0x000000, 0x080000, CRC(d1be8699) SHA1(67563761f95892b08c7113ab1c52ab5aa7118fb8) )

//  ROM_REGION( 0x080000, "gfx2", 0 ) /* Scroll 1 */
//  UNUSED

	ROM_REGION( 0x020000, "gfx3", 0 ) /* Scroll 2 */
	ROM_LOAD( "ac91106_ver1.2_7.ic99", 0x000000, 0x020000, CRC(09755aef) SHA1(39c901fb9408a0ba488f0112d7f48b929b092e3b) )

	ROM_REGION( 0x300000, "gfx4", 0 ) /* Sprites */
	ROM_LOAD16_BYTE( "mr91042-01-r60_1.ic1",  0x000000, 0x080000, CRC(fdfe6951) SHA1(ba6c5cd5d16fdca6f131302b19e621f8abe8136a) )
	ROM_LOAD16_BYTE( "mr91042-02-r61_2.ic2",  0x000001, 0x080000, CRC(2e6c8b30) SHA1(70503fec251606b37fea2c7f91e682aece252035) )
	ROM_LOAD16_BYTE( "mr91042-03-r62_3.ic5",  0x100000, 0x080000, CRC(07ba6d3a) SHA1(9c58e3a1931b593448c53a59e7f5b9aaac40ff88) )
	ROM_LOAD16_BYTE( "mr91042-04-r63_4.ic6",  0x100001, 0x080000, CRC(f37cb12c) SHA1(282ebbd795284d7efa335b797ca1eedc1110e9da) )
	ROM_LOAD16_BYTE( "mr91042-05-r64_5.ic11", 0x200000, 0x080000, CRC(7a3bb52d) SHA1(7f9d1dad4c89e6b55415b082363bc261115e9f96) )
	ROM_LOAD16_BYTE( "mr91042-06-r65_6.ic12", 0x200001, 0x080000, CRC(5312a4f2) SHA1(4dcd2839bb5acccecf1eb6c0e19e877a0cff6875) )

	ROM_REGION( 0x100000, "oki1", 0 )       /* Samples (4x40000) */
	ROM_LOAD( "mr91042-08_2.ic57",     0x000000, 0x080000, CRC(dc015f6c) SHA1(9d0677c50a25be1d11d43e54dbf3005f18b79b66) )
	ROM_RELOAD(                        0x080000, 0x080000 )

	ROM_REGION( 0x100000, "oki2", 0 )       /* Samples (4x40000) */
	ROM_LOAD( "ac91106_ver1.2_1.ic56", 0x000000, 0x080000, CRC(48208b69) SHA1(5dfcc7744f7cdd0326886a4a841755ab6ec5482b) )
	ROM_RELOAD(                        0x080000, 0x080000 )

	ROM_REGION( 0x80000, "user2", 0 )       /* ? Unused ROMs ? */
	ROM_LOAD( "ch9072-4_13.ic39", 0x000000, 0x002000, CRC(b45b4dc0) SHA1(b9fae0c9ac2d40f0a202c538d866d5f2941ba8dd) )
	ROM_LOAD( "ch9072-5_11.ic33", 0x000000, 0x002000, CRC(e122916b) SHA1(86d5ecc7ecc6f175ecb28459697ef33e1ee06860) )
	ROM_LOAD( "ch9072-6_12.ic35", 0x000000, 0x002000, CRC(05d95bf7) SHA1(78181cf71f22c090a1e62823a43757353a9ef6ab) )
	ROM_LOAD( "ch9072-8_15.ic59", 0x000000, 0x002000, CRC(6bf52596) SHA1(bf4e7e7df3daae4aa6a441b58b15a435aa45630e) )

	ROM_LOAD( "mr90015-35-w33_17.ic67", 0x000000, 0x080000, CRC(9d428fb7) SHA1(02f72938d73db932bd217620a175a05215f6016a) )
	ROM_LOAD( "mr90015-35-w33_14.ic54", 0x000000, 0x080000, CRC(9d428fb7) SHA1(02f72938d73db932bd217620a175a05215f6016a) )

	ROM_LOAD( "pr88004q_8.ic102", 0x000000, 0x000200, CRC(9327dc37) SHA1(cfe7b144cdcd76170d47f1c4e0f72b6d4fca0c8d) )
	ROM_LOAD( "pr88004w_16.ic66", 0x000000, 0x000100, CRC(3d648467) SHA1(bf8dbaa2176c801f7370313425c87f0eefe8a3a4) )

	ROM_LOAD( "pr91042_5.ic91", 0x000000, 0x000100, CRC(e71de4aa) SHA1(d06e5a35ad2127df2d6328cce153073380ee7819) )
ROM_END

/***************************************************************************


                                Game Drivers


***************************************************************************/

GAMEL( 1989, bigrun,   0,        bigrun,   bigrun, cischeat_state,   bigrun,   ROT0,   "Jaleco", "Big Run (11th Rallye version)", MACHINE_IMPERFECT_GRAPHICS, layout_cischeat )    // there's a 13th Rallye version (1991) (only on the SNES?)
GAMEL( 1990, cischeat, 0,        cischeat, cischeat, cischeat_state, cischeat, ROT0,   "Jaleco", "Cisco Heat",                    MACHINE_IMPERFECT_GRAPHICS, layout_cischeat )
GAMEL( 1991, f1gpstar, 0,        f1gpstar, f1gpstar, cischeat_state, f1gpstar, ROT0,   "Jaleco", "Grand Prix Star",               MACHINE_IMPERFECT_GRAPHICS, layout_f1gpstar )
GAME ( 1992, armchmp2, 0,        armchmp2, armchmp2, driver_device, 0,        ROT270, "Jaleco", "Arm Champs II v2.6",            MACHINE_IMPERFECT_GRAPHICS )
GAME ( 1992, armchmp2o,armchmp2, armchmp2, armchmp2, driver_device, 0,        ROT270, "Jaleco", "Arm Champs II v1.7",            MACHINE_IMPERFECT_GRAPHICS )
GAME ( 1992, wildplt,  0,        f1gpstr2, wildplt, cischeat_state,  wildplt,  ROT0,   "Jaleco", "Wild Pilot",                    MACHINE_IMPERFECT_GRAPHICS )
GAMEL( 1993, f1gpstr2, 0,        f1gpstr2, f1gpstar, cischeat_state, f1gpstar, ROT0,   "Jaleco", "F-1 Grand Prix Star II",        MACHINE_IMPERFECT_GRAPHICS, layout_f1gpstar )
GAME ( 1994, scudhamm, 0,        scudhamm, scudhamm, driver_device, 0,        ROT270, "Jaleco", "Scud Hammer",                   MACHINE_IMPERFECT_GRAPHICS )
