// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, David Graves
/***************************************************************************

    Underfire                           (c) 1993 Taito
    Chase Bombers                       (c) 1994 Taito

    Driver by Bryan McPhail & David Graves.

    Board Info (Underfire):

        TC0470LIN : object line buffer?
        TC0480SCP : known tilemap chip
        TC0510NIO : known input chip
        TC0570SPC : must be the object chip (next to spritemap and OBJ roms)
        TC0590PIV : object related???
        TC0620SCC : tilemap chip (6bpp version of TC0100SCN)
        TC0650FDA : palette ? (Slapshot and F3 games also have one)

    M43E0278A
    K1100744A Main Board

    2018 2088           43256    43256   68020-25
    2018 2088           D67-23   D67-17                    93C46
    2018 2088           43256    43256
    2018                D67-18   D67-19                TC0510NIO
    2018
    2018 TC0570 SPC                      43256
                                             43256
     D67-13                              43256  TC0650FDA
              D67-07                            2018
              D67-06
    TC0470LIN D67-05
              D67-04                      43256
    TC0590PIV D67-03    43256    D67-10   43256
                        43256    D67-11
       D67-09    TC0480SCP       D67-12   TC0620SCC
       D67-08

      MB8421
      MB8421   43256             EnsoniqOTIS
               D67-20    D67-01
               43256                             EnsoniqESP-R6
     68000-12  D67-21    D67-02     EnsoniqSuperGlu

               40MHz            16MHz   30.476MHz    68681


    Under Fire combines the sprite system used in Taito Z games with
    the TC0480SCP tilemap chip plus some features from the Taito F3 system.
    It has an extra TC0620SCC tilemap chip which is a 6bpp version of the
    TC0100SCN (check the inits).


    Game misbehaviours
    ------------------

    (i) Sprites on some rounds had sprite/tile priority issues.
    Solved by upping sprite priority while TC0480SCP row zoom is
    enabled - kludge.


    Todo
    ----

    What does the 0xb00000 area do... alpha blending ??

    What is the unknown hardware at 0x600000... an alternative
    or legacy lightgun hookup?

    Pivot port which may be used for rotation: but not
    seen changing except in game inits. Perhaps only used
    in later levels?

    Chase Bombers title screen has wrong Taito logo;

    Chase Bombers proto sports lots of gfx bugs;

    Sprites are delayed, but not yet implemented

    Gun calibration
    ---------------

    The values below work well (set speed down to 4 so you can enter
    them). They give a little reloading margin all around screen.

    Use X=0x2000  Y=0x100 for top center
    Use X=0x3740  (same Y) top left
    Use X=0x8c0   (same Y) top right
    Then for the points from left to right near bottom (all Y=0x3400):
    X=0x3f00,0x3740,0x2f80,0x27c0,0x2000,0x1840,0x1080,0x8c0,0x100


Code documentation
------------------

$17b6e2: loop which keeps displaying the trail of aim dots in test mode

$181826: routine which calls subs to derive aim coords for P1 and P2 and
pokes them in the game's internal sprite table format into RAM - along
with the nine blue "flag point" sprites on the calibration screen
which seem to have fixed coordinates. [It also refreshes some green
text on screen - the calls to $1bfa.]

$18141a sub appears to be doing all the calculations - including an
indirected subroutine so there may be quite a lot of possible code.
It is called with parameter of 1 for player 2 (and 0 for player 1).

$1821c8 sub is called just after - this is simpler and seems to be
copying the calculation results (modified slightly by 3 pixels in each
direction - to adjust for size of aim sprite?) into the table in ram.

(Subsequently a standard conversion routine turns the table on the fly
into dwords that are actually poked into spriteram. To locate the code
do a watchpoint on the first sprite in spriteram - the P1 aim point.)

In-game: $18141a is called 3 times when you hit fire - 3 bullets - and
once when you hit shotgun. Like Spacegun it is only doing the aim
calculations when it needs to, so to provide an artificial target we
need to reproduce the $18141a calculations.


Info (Chase Bombers)

Chase Bombers
Taito, 1994

Runs on hardware similar to Ground Effects


PCB Layout
----------

MAIN PCB-D
K1100809A
J1100342A
|----------------------------------------------------------------------------------------------|
|           C5                              C6                             SMC_COM20020  LANOUT|
| 68EC020   61256     68EC000  61256            68EC000      61256      MB8421                 |
|           61256              61256                         61256                       LANIN |
|           61256                     61256                  PAL                               |
|           61256    PAL              61256                  PAL                               |
|                    PAL                                     PAL        MB8421                 |
|                    40MHz                                                                     |
|                                                                  MC68681                     |
|                                           TC511664-80     MB3771                             |
|                                                                                            P1|
|  MACH120   MACH120                                                                           |
|                                     ENSONIQ         30.4761MHz  16MHz   ADC0809              |
|                                     ESP-R6                                                   |
|                                                       ENSONIQ                                |
|                                     ENSONIQ           5701     DSW1(8)  TC0510NIO            |
|                                     OTIS-R2                                                 Z|
|                                                         93C46                                |
|           C3                              C4                                                 |
|                                                                                              |
|  61256                                                                                     |-|
|  61256                                                                                     |
|                                                                                            |-|
|                TC0480SCP        TC0620SCC           TC0360PRI                                |
|                                                                                              |
|                                                                                TC0650FDA     |
|  2018                                                                  61256                 |
|                                                                        61256                 |
|  2018     2088             61256                                       61256                G|
|                            61256                    TC0580PIV                                |
|  2018     2088                                                                               |
|                                                                                              |
|  2018                                                             TL074    TL074   TDA1543   |
|           TC0570SPC         TC0470LIN               514256  514256                           |
|  2018                                               514256  514256        TA8221   TD62064 |-|
|                                                     514256  514256                 TD62064 |
|  2018                                                               MB87078        TD62064 |-|
|           C1                              C2                                                 |
|----------------------------------------------------------------------------------------------|
Notes:
      ROM board plugs into C* connectors
      No clocks for now, PCB has light corrosion and will need extensive cleaning before it can be powered up.

ROM Board
---------

PCB Numbers - ROM.PCB
              K9100508A
              J9100367A

Board contains only 29 ROMs and not much else.



***************************************************************************/

#include "emu.h"
#include "undrfire.h"
#include "taito_en.h"

#include "cpu/m68000/m68020.h"
#include "machine/adc0808.h"
#include "machine/eepromser.h"
#include "taitoio.h"
#include "machine/watchdog.h"
#include "sound/es5506.h"
#include "screen.h"
#include "speaker.h"

#include "cbombers.lh"


void undrfire_state::machine_start()
{
	m_lamp_start.resolve();
	m_gun_recoil.resolve();
	m_lamp.resolve();
	m_wheel_vibration.resolve();
}

/**********************************************************
            GAME INPUTS
**********************************************************/

int undrfire_state::frame_counter_r()
{
	return m_frame_counter;
}

void undrfire_state::coin_word_w(u8 data)
{
	machine().bookkeeping().coin_lockout_w(0,~data & 0x01);
	machine().bookkeeping().coin_lockout_w(1,~data & 0x02);
	machine().bookkeeping().coin_counter_w(0, data & 0x04);
	machine().bookkeeping().coin_counter_w(1, data & 0x08);
}


u16 undrfire_state::shared_ram_r(offs_t offset)
{
	return m_shared_ram[offset];
}

void undrfire_state::shared_ram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_shared_ram[offset]);
}

u32 undrfire_state::undrfire_lightgun_r(offs_t offset)
{
	switch (offset)
	{
		/* NB we are raising the raw inputs by an arbitrary amount,
		   but presumably the guns on the original will not have had
		   full 0-0xffff travel. We don't center around 0x8000... but
		   who knows if the real machine does. */

		case 0x00:  /* P1 */
		case 0x01:  /* P2 */
		{
			int x = m_in_gunx[offset & 1]->read() << 6;
			int y = m_in_guny[offset & 1]->read() << 6;

			return ((x << 24) &0xff000000) | ((x << 8) &0xff0000)
					| ((y << 8) &0xff00) | ((y >> 8) &0xff) ;
		}
	}

logerror("CPU #0 PC %06x: warning - read unmapped lightgun offset %06x\n",m_maincpu->pc(),offset);

	return 0x0;
}


void undrfire_state::rotate_control_w(offs_t offset, u16 data)/* only a guess that it's rotation */
{
	if (offset & 1)
	{
		m_rotate_ctrl[m_port_sel] = data;
	}
	else
	{
		m_port_sel = data & 0x7;
	}
}


void undrfire_state::motor_control_w(u8 data)
{
/*
    Standard value poked is 0x00910200 (we ignore lsb and msb
    which seem to be always zero)

    0x0, 0x8000 and 0x9100 are written at startup

    Two bits are written in test mode to this middle word
    to test gun vibration:

    ........ .x......   P1 gun vibration
    ........ x.......   P2 gun vibration
*/

	m_lamp_start[0] = BIT(data, 4); //p1 start
	m_lamp_start[1] = BIT(data, 5); //p2 start
	m_gun_recoil[0] = BIT(data, 6); //p1 recoil
	m_gun_recoil[1] = BIT(data, 7); //p2 recoil
}

void undrfire_state::cbombers_cpua_ctrl_w(u32 data)
{
/*
    ........ ..xxxxxx   Lamp 1-6 enables
    ........ .x......   Vibration
*/
	for (int i = 0; i < 6; i++)
		m_lamp[i] = BIT(data, i);

	m_wheel_vibration = BIT(data, 6);

	m_subcpu->set_input_line(INPUT_LINE_RESET, BIT(data, 12) ? CLEAR_LINE : ASSERT_LINE);
}


/***********************************************************
             MEMORY STRUCTURES
***********************************************************/

void undrfire_state::undrfire_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom();
	map(0x200000, 0x21ffff).ram().share("ram");
	map(0x300000, 0x303fff).ram().share("spriteram");
//  map(0x304000, 0x304003).ram(); // debugging - doesn't change ???
//  map(0x304400, 0x304403).ram(); // debugging - doesn't change ???
	map(0x400002, 0x400002).w(FUNC(undrfire_state::motor_control_w));      /* gun vibration */
	map(0x500000, 0x500007).rw("tc0510nio", FUNC(tc0510nio_device::read), FUNC(tc0510nio_device::write));
	map(0x600000, 0x600007).noprw(); // space for ADC0809, not fitted on pcb
	map(0x700000, 0x7007ff).rw("taito_en:dpram", FUNC(mb8421_device::left_r), FUNC(mb8421_device::left_w));
	map(0x800000, 0x80ffff).rw(m_tc0480scp, FUNC(tc0480scp_device::ram_r), FUNC(tc0480scp_device::ram_w));        /* tilemaps */
	map(0x830000, 0x83002f).rw(m_tc0480scp, FUNC(tc0480scp_device::ctrl_r), FUNC(tc0480scp_device::ctrl_w));
	map(0x900000, 0x90ffff).rw(m_tc0620scc, FUNC(tc0620scc_device::ram_r), FUNC(tc0620scc_device::ram_w));        /* 6bpp tilemaps */
	map(0x920000, 0x92000f).rw(m_tc0620scc, FUNC(tc0620scc_device::ctrl_r), FUNC(tc0620scc_device::ctrl_w));
	map(0xa00000, 0xa0ffff).ram().w(m_palette, FUNC(palette_device::write32)).share("palette");
	map(0xb00000, 0xb003ff).ram();                         /* single bytes, blending ??? */
	map(0xd00000, 0xd00003).w(FUNC(undrfire_state::rotate_control_w));     /* perhaps port based rotate control? */
	map(0xf00000, 0xf00007).r(FUNC(undrfire_state::undrfire_lightgun_r));   /* stick coords read at $11b2-bc */
}


void undrfire_state::cbombers_cpua_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom();
	map(0x200000, 0x21ffff).ram();
	map(0x300000, 0x303fff).ram().share("spriteram");
	map(0x400000, 0x400003).w(FUNC(undrfire_state::cbombers_cpua_ctrl_w));
	map(0x500000, 0x500007).rw("tc0510nio", FUNC(tc0510nio_device::read), FUNC(tc0510nio_device::write));
	map(0x600000, 0x600007).rw("adc", FUNC(adc0808_device::data_r), FUNC(adc0808_device::address_offset_start_w)).umask32(0xffffffff);
	map(0x700000, 0x7007ff).rw("taito_en:dpram", FUNC(mb8421_device::left_r), FUNC(mb8421_device::left_w));
	map(0x800000, 0x80ffff).rw(m_tc0480scp, FUNC(tc0480scp_device::ram_r), FUNC(tc0480scp_device::ram_w));        /* tilemaps */
	map(0x830000, 0x83002f).rw(m_tc0480scp, FUNC(tc0480scp_device::ctrl_r), FUNC(tc0480scp_device::ctrl_w));
	map(0x900000, 0x90ffff).rw(m_tc0620scc, FUNC(tc0620scc_device::ram_r), FUNC(tc0620scc_device::ram_w));        /* 6bpp tilemaps */
	map(0x920000, 0x92000f).rw(m_tc0620scc, FUNC(tc0620scc_device::ctrl_r), FUNC(tc0620scc_device::ctrl_w));
	map(0xa00000, 0xa0ffff).ram().w(m_palette, FUNC(palette_device::write32)).share("palette");
	map(0xb00000, 0xb0000f).rw(m_tc0360pri, FUNC(tc0360pri_device::read), FUNC(tc0360pri_device::write)); /* TC0360PRI */
	map(0xc00000, 0xc00007).ram(); /* LAN controller? */
	map(0xd00000, 0xd00003).w(FUNC(undrfire_state::rotate_control_w));     /* perhaps port based rotate control? */
	map(0xe00000, 0xe0ffff).rw(FUNC(undrfire_state::shared_ram_r), FUNC(undrfire_state::shared_ram_w));
}

void undrfire_state::cbombers_cpub_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x400000, 0x40ffff).ram(); /* local ram */
//  map(0x600000, 0x60ffff).w(m_tc0480scp, FUNC(tc0480scp_device::ram_w)); /* Only written upon errors */
	map(0x800000, 0x80ffff).ram().share("shared_ram");
//  map(0xa00000, 0xa001ff).ram(); /* Extra road control?? */
}


/***********************************************************
             INPUT PORTS (dips in eprom)
***********************************************************/

static INPUT_PORTS_START( undrfire )
	PORT_START("INPUTS0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("INPUTS1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)  /* ? where is freeze input */
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(2)

	PORT_START("INPUTS2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_SERVICE_NO_TOGGLE(0x01, IP_ACTIVE_LOW)
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	/* Gun inputs (real range is 0-0xffff: we use standard 0-255 and shift later) */
	PORT_START("GUNX1")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, -1.0, 0.0, 0) PORT_SENSITIVITY(20) PORT_KEYDELTA(25) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("GUNY1")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(20) PORT_KEYDELTA(25) PORT_PLAYER(1)

	PORT_START("GUNX2")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, -1.0, 0.0, 0) PORT_SENSITIVITY(20) PORT_KEYDELTA(25) PORT_REVERSE PORT_PLAYER(2)

	PORT_START("GUNY2")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(20) PORT_KEYDELTA(25) PORT_PLAYER(2)
INPUT_PORTS_END



static INPUT_PORTS_START( cbombers )
	PORT_START("INPUTS0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_BUTTON4 ) PORT_NAME("Gear Shift") PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_BUTTON3 ) PORT_NAME("Nitro")
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_NAME("Accelerator")
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_NAME("Brake")
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("INPUTS1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON5 ) /* ? where is freeze input */
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("INPUTS2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_SERVICE_NO_TOGGLE(0x01, IP_ACTIVE_LOW)
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("STEER") /* IN 3, steering wheel */
	PORT_BIT( 0xff, 0x7f, IPT_AD_STICK_X ) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_REVERSE PORT_PLAYER(1)
INPUT_PORTS_END


/**********************************************************
                GFX DECODING
**********************************************************/

static const gfx_layout tile16x16_layout =
{
	16,16,  /* 16*16 sprites */
	RGN_FRAC(1,5),
	5,  /* 5 bits per pixel */
	{ RGN_FRAC(4,5), RGN_FRAC(3,5), RGN_FRAC(2,5), RGN_FRAC(1,5), RGN_FRAC(0,5) },
	{ STEP16(0,1) },
	{ STEP16(0,16) },
	16*16   /* every sprite takes 128 consecutive bytes */
};

static GFXDECODE_START( gfx_undrfire )
	GFXDECODE_ENTRY( "sprites", 0x0, tile16x16_layout, 0, 512 )
GFXDECODE_END


/***********************************************************
                 MACHINE DRIVERS
***********************************************************/

INTERRUPT_GEN_MEMBER(undrfire_state::undrfire_interrupt)
{
	m_frame_counter ^= 1;
	device.execute().set_input_line(4, HOLD_LINE);
}

void undrfire_state::undrfire(machine_config &config)
{
	/* basic machine hardware */
	M68EC020(config, m_maincpu, XTAL(40'000'000)/2); /* 20 MHz - NOT verified */
	m_maincpu->set_addrmap(AS_PROGRAM, &undrfire_state::undrfire_map);
	m_maincpu->set_vblank_int("screen", FUNC(undrfire_state::undrfire_interrupt));

	EEPROM_93C46_16BIT(config, "eeprom");

	tc0510nio_device &tc0510nio(TC0510NIO(config, "tc0510nio", 0));
	tc0510nio.read_0_callback().set_ioport("INPUTS0");
	tc0510nio.read_1_callback().set_ioport("INPUTS1");
	tc0510nio.read_2_callback().set_ioport("INPUTS2");
	tc0510nio.read_3_callback().set(m_eeprom, FUNC(eeprom_serial_93cxx_device::do_read)).lshift(7);
	tc0510nio.read_3_callback().append(FUNC(undrfire_state::frame_counter_r)).lshift(0);
	tc0510nio.write_3_callback().set(m_eeprom, FUNC(eeprom_serial_93cxx_device::clk_write)).bit(5);
	tc0510nio.write_3_callback().append(m_eeprom, FUNC(eeprom_serial_93cxx_device::di_write)).bit(6);
	tc0510nio.write_3_callback().append(m_eeprom, FUNC(eeprom_serial_93cxx_device::cs_write)).bit(4);
	tc0510nio.write_4_callback().set(FUNC(undrfire_state::coin_word_w));
	tc0510nio.read_7_callback().set_ioport("SYSTEM");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(40*8, 32*8);
	screen.set_visarea(0, 40*8-1, 3*8, 32*8-1);
	screen.set_screen_update(FUNC(undrfire_state::screen_update_undrfire));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_undrfire);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_888, 16384);

	TC0620SCC(config, m_tc0620scc, 0);
	m_tc0620scc->set_offsets(50, 8);
	m_tc0620scc->set_palette(m_palette);

	TC0480SCP(config, m_tc0480scp, 0);
	m_tc0480scp->set_palette(m_palette);
	m_tc0480scp->set_offsets(0x24, 0);
	m_tc0480scp->set_offsets_tx(-1, 0);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	taito_en_device &taito_en(TAITO_EN(config, "taito_en", 0));
	taito_en.add_route(0, "lspeaker", 1.0);
	taito_en.add_route(1, "rspeaker", 1.0);
}


void undrfire_state::cbombers(machine_config &config)
{
	/* basic machine hardware */
	M68EC020(config, m_maincpu, XTAL(40'000'000)/2); /* 20 MHz - NOT verified */
	m_maincpu->set_addrmap(AS_PROGRAM, &undrfire_state::cbombers_cpua_map);
	m_maincpu->set_vblank_int("screen", FUNC(undrfire_state::irq4_line_hold));

	M68000(config, m_subcpu, XTAL(32'000'000)/2);   /* 16 MHz */
	m_subcpu->set_addrmap(AS_PROGRAM, &undrfire_state::cbombers_cpub_map);
	m_subcpu->set_vblank_int("screen", FUNC(undrfire_state::irq4_line_hold));

	config.set_maximum_quantum(attotime::from_hz(480));   /* CPU slices - Need to interleave Cpu's 1 & 3 */

	EEPROM_93C46_16BIT(config, "eeprom");

	adc0809_device &adc(ADC0809(config, "adc", 500000)); // unknown clock
	adc.eoc_ff_callback().set_inputline("maincpu", 5);
	adc.in_callback<0>().set_ioport("STEER");

	tc0510nio_device &tc0510nio(TC0510NIO(config, "tc0510nio", 0));
	tc0510nio.read_0_callback().set_ioport("INPUTS0");
	tc0510nio.read_1_callback().set_ioport("INPUTS1");
	tc0510nio.read_2_callback().set_ioport("INPUTS2");
	tc0510nio.read_3_callback().set(m_eeprom, FUNC(eeprom_serial_93cxx_device::do_read)).lshift(7);
	tc0510nio.read_3_callback().append(FUNC(undrfire_state::frame_counter_r)).lshift(0);
	tc0510nio.write_3_callback().set(m_eeprom, FUNC(eeprom_serial_93cxx_device::clk_write)).bit(5);
	tc0510nio.write_3_callback().append(m_eeprom, FUNC(eeprom_serial_93cxx_device::di_write)).bit(6);
	tc0510nio.write_3_callback().append(m_eeprom, FUNC(eeprom_serial_93cxx_device::cs_write)).bit(4);
	tc0510nio.write_4_callback().set(FUNC(undrfire_state::coin_word_w));
	tc0510nio.read_7_callback().set_ioport("SYSTEM");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(40*8, 32*8);
	screen.set_visarea(0, 40*8-1, 3*8, 32*8-1);
	screen.set_screen_update(FUNC(undrfire_state::screen_update_cbombers));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_undrfire);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_888, 16384);

	TC0620SCC(config, m_tc0620scc, 0);
	m_tc0620scc->set_offsets(50, 8);
	m_tc0620scc->set_palette(m_palette);

	TC0480SCP(config, m_tc0480scp, 0);
	m_tc0480scp->set_palette(m_palette);
	m_tc0480scp->set_offsets(0x24, 0);
	m_tc0480scp->set_offsets_tx(-1, 0);
	m_tc0480scp->set_col_base(4096);

	TC0360PRI(config, m_tc0360pri, 0);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	taito_en_device &taito_en(TAITO_EN(config, "taito_en", 0));
	taito_en.add_route(0, "lspeaker", 1.0);
	taito_en.add_route(1, "rspeaker", 1.0);
}


/***************************************************************************
                    DRIVERS
***************************************************************************/

ROM_START( undrfire )
	ROM_REGION( 0x200000, "maincpu", 0 )    /* 2048K for 68020 code (CPU A) */
	ROM_LOAD32_BYTE( "d67-19", 0x00000, 0x80000, CRC(1d88fa5a) SHA1(5e498efb9535a8f4e82b5525390b8bde7c45c07e) )
	ROM_LOAD32_BYTE( "d67-18", 0x00001, 0x80000, CRC(f41ae7fd) SHA1(bdd0df01b11205c263d2fa280746826b831d58bc) )
	ROM_LOAD32_BYTE( "d67-17", 0x00002, 0x80000, CRC(34e030b7) SHA1(62c270c817199a56e647ea74849fe5c07717ac18) )
	ROM_LOAD32_BYTE( "d67-23", 0x00003, 0x80000, CRC(28e84e0a) SHA1(74c73c6df07d33ef4c0a29f8c1ee1a33eee922da) )

	ROM_REGION( 0x140000, "taito_en:audiocpu", 0 )
	ROM_LOAD16_BYTE( "d67-20", 0x100000, 0x20000,  CRC(974ebf69) SHA1(8a5de503c514bf0da0c956e2dfdf0cfb83ea1f72) )
	ROM_LOAD16_BYTE( "d67-21", 0x100001, 0x20000,  CRC(8fc6046f) SHA1(28522ce5c5900f74d3faa86710256a7201b32500) )

	ROM_REGION( 0x400000, "tc0480scp", 0 )
	ROM_LOAD32_WORD( "d67-08", 0x000000, 0x200000, CRC(56730d44) SHA1(110872714b3c26a82473c7b80c120918b91b1b4b) )   /* SCR 16x16 tiles */
	ROM_LOAD32_WORD( "d67-09", 0x000002, 0x200000, CRC(3c19f9e3) SHA1(7ba8475d37cbf8bf38029124afdf62c915c8668d) )

	ROM_REGION( 0xa00000, "sprites", 0 )
	ROM_LOAD16_WORD_SWAP( "d67-03", 0x000000, 0x200000, CRC(3b6e99a9) SHA1(1e0e66763ddfa18a2d291626b245633555092959) )   /* OBJ 16x16 tiles */
	ROM_LOAD16_WORD_SWAP( "d67-04", 0x200000, 0x200000, CRC(8f2934c9) SHA1(ead95b34eec3a6df27199edcbdd5595bc6555a50) )
	ROM_LOAD16_WORD_SWAP( "d67-05", 0x400000, 0x200000, CRC(e2e7dcf3) SHA1(185dbd0489931123a295139dc0a045ad239018fb) )
	ROM_LOAD16_WORD_SWAP( "d67-06", 0x600000, 0x200000, CRC(a2a63488) SHA1(a1ed140cc3757c3c05a0a822089c6efc83bf4805) )
	ROM_LOAD16_WORD_SWAP( "d67-07", 0x800000, 0x200000, CRC(189c0ee5) SHA1(de85b39dc67f31ef80800ff6ec9a391652eb12e4) )

	ROM_REGION( 0x200000, "tc0620scc", 0 )
	ROM_LOAD16_BYTE( "d67-10", 0x000001, 0x100000, CRC(d79e6ce9) SHA1(8b38302971816d599cdaa3279cb6395441373c6f) )   /* PIV 8x8 tiles, 4bpp */
	ROM_LOAD16_BYTE( "d67-11", 0x000000, 0x100000, CRC(7a401bb3) SHA1(47257a6a4b37ec1ceb4e974b776ee3ea30db06fa) )

	ROM_REGION( 0x100000, "tc0620scc:hi_gfx", 0 )
	ROM_LOAD       ( "d67-12", 0x000000, 0x100000, CRC(67b16fec) SHA1(af0f9f50516331780ef6cfab1e12a23edf87daa7) )   /* PIV 8x8 tiles, 2bpp */

	ROM_REGION16_LE( 0x80000, "spritemap", 0 )
	ROM_LOAD16_WORD( "d67-13", 0x00000,  0x80000,  CRC(42e7690d) SHA1(5f00f3f814653733bf9a5cb010675799de02fa76) )   /* STY, spritemap */

	ROM_REGION16_BE( 0x1000000, "taito_en:ensoniq", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "d67-01", 0x000000, 0x200000, CRC(a2f18122) SHA1(640014c6e6d66c59fe0accf370ad3bab9f40429a) )   /* Ensoniq samples */
	ROM_LOAD16_BYTE( "d67-02", 0xc00000, 0x200000, CRC(fceb715e) SHA1(9326513acb0696669d4f2345649ab37c8c6ed171) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "eeprom-undrfire.bin", 0x0000, 0x0080, CRC(9f7368f4) SHA1(4bb28e6eb3a72a06341199f0d744ed0ce13bce2c) )
ROM_END


ROM_START( undrfireu )
	ROM_REGION( 0x200000, "maincpu", 0 )    /* 2048K for 68020 code (CPU A) */
	ROM_LOAD32_BYTE( "d67-19", 0x00000, 0x80000, CRC(1d88fa5a) SHA1(5e498efb9535a8f4e82b5525390b8bde7c45c07e) )
	ROM_LOAD32_BYTE( "d67-18", 0x00001, 0x80000, CRC(f41ae7fd) SHA1(bdd0df01b11205c263d2fa280746826b831d58bc) )
	ROM_LOAD32_BYTE( "d67-17", 0x00002, 0x80000, CRC(34e030b7) SHA1(62c270c817199a56e647ea74849fe5c07717ac18) )
	ROM_LOAD32_BYTE( "d67-22", 0x00003, 0x80000, CRC(5fef7e9c) SHA1(03a6ea0715ce8705d74550186b22940f8a49c088) )

	ROM_REGION( 0x140000, "taito_en:audiocpu", 0 )
	ROM_LOAD16_BYTE( "d67-20", 0x100000, 0x20000,  CRC(974ebf69) SHA1(8a5de503c514bf0da0c956e2dfdf0cfb83ea1f72) )
	ROM_LOAD16_BYTE( "d67-21", 0x100001, 0x20000,  CRC(8fc6046f) SHA1(28522ce5c5900f74d3faa86710256a7201b32500) )

	ROM_REGION( 0x400000, "tc0480scp", 0 )
	ROM_LOAD32_WORD( "d67-08", 0x000000, 0x200000, CRC(56730d44) SHA1(110872714b3c26a82473c7b80c120918b91b1b4b) )   /* SCR 16x16 tiles */
	ROM_LOAD32_WORD( "d67-09", 0x000002, 0x200000, CRC(3c19f9e3) SHA1(7ba8475d37cbf8bf38029124afdf62c915c8668d) )

	ROM_REGION( 0xa00000, "sprites", 0 )
	ROM_LOAD16_WORD_SWAP( "d67-03", 0x000000, 0x200000, CRC(3b6e99a9) SHA1(1e0e66763ddfa18a2d291626b245633555092959) )   /* OBJ 16x16 tiles */
	ROM_LOAD16_WORD_SWAP( "d67-04", 0x200000, 0x200000, CRC(8f2934c9) SHA1(ead95b34eec3a6df27199edcbdd5595bc6555a50) )
	ROM_LOAD16_WORD_SWAP( "d67-05", 0x400000, 0x200000, CRC(e2e7dcf3) SHA1(185dbd0489931123a295139dc0a045ad239018fb) )
	ROM_LOAD16_WORD_SWAP( "d67-06", 0x600000, 0x200000, CRC(a2a63488) SHA1(a1ed140cc3757c3c05a0a822089c6efc83bf4805) )
	ROM_LOAD16_WORD_SWAP( "d67-07", 0x800000, 0x200000, CRC(189c0ee5) SHA1(de85b39dc67f31ef80800ff6ec9a391652eb12e4) )

	ROM_REGION( 0x200000, "tc0620scc", 0 )
	ROM_LOAD16_BYTE( "d67-10", 0x000001, 0x100000, CRC(d79e6ce9) SHA1(8b38302971816d599cdaa3279cb6395441373c6f) )   /* PIV 8x8 tiles, 4bpp */
	ROM_LOAD16_BYTE( "d67-11", 0x000000, 0x100000, CRC(7a401bb3) SHA1(47257a6a4b37ec1ceb4e974b776ee3ea30db06fa) )

	ROM_REGION( 0x100000, "tc0620scc:hi_gfx", 0 )
	ROM_LOAD       ( "d67-12", 0x000000, 0x100000, CRC(67b16fec) SHA1(af0f9f50516331780ef6cfab1e12a23edf87daa7) )   /* PIV 8x8 tiles, 2bpp */

	ROM_REGION16_LE( 0x80000, "spritemap", 0 )
	ROM_LOAD16_WORD( "d67-13", 0x00000,  0x80000,  CRC(42e7690d) SHA1(5f00f3f814653733bf9a5cb010675799de02fa76) )   /* STY, spritemap */

	ROM_REGION16_BE( 0x1000000, "taito_en:ensoniq", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "d67-01", 0x000000, 0x200000, CRC(a2f18122) SHA1(640014c6e6d66c59fe0accf370ad3bab9f40429a) )   /* Ensoniq samples */
	ROM_LOAD16_BYTE( "d67-02", 0xc00000, 0x200000, CRC(fceb715e) SHA1(9326513acb0696669d4f2345649ab37c8c6ed171) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "eeprom-undrfire.bin", 0x0000, 0x0080, CRC(9f7368f4) SHA1(4bb28e6eb3a72a06341199f0d744ed0ce13bce2c) )
ROM_END

ROM_START( undrfirej )
	ROM_REGION( 0x200000, "maincpu", 0 )    /* 2048K for 68020 code (CPU A) */
	ROM_LOAD32_BYTE( "d67-19", 0x00000, 0x80000, CRC(1d88fa5a) SHA1(5e498efb9535a8f4e82b5525390b8bde7c45c07e) )
	ROM_LOAD32_BYTE( "d67-18", 0x00001, 0x80000, CRC(f41ae7fd) SHA1(bdd0df01b11205c263d2fa280746826b831d58bc) )
	ROM_LOAD32_BYTE( "d67-17", 0x00002, 0x80000, CRC(34e030b7) SHA1(62c270c817199a56e647ea74849fe5c07717ac18) )
	ROM_LOAD32_BYTE( "d67-16", 0x00003, 0x80000, CRC(c6e62f26) SHA1(6a430916f829a4b0240ccf8477dcbb1f39a26e90) )

	ROM_REGION( 0x140000, "taito_en:audiocpu", 0 )
	ROM_LOAD16_BYTE( "d67-20", 0x100000, 0x20000,  CRC(974ebf69) SHA1(8a5de503c514bf0da0c956e2dfdf0cfb83ea1f72) )
	ROM_LOAD16_BYTE( "d67-21", 0x100001, 0x20000,  CRC(8fc6046f) SHA1(28522ce5c5900f74d3faa86710256a7201b32500) )

	ROM_REGION( 0x400000, "tc0480scp", 0 )
	ROM_LOAD32_WORD( "d67-08", 0x000000, 0x200000, CRC(56730d44) SHA1(110872714b3c26a82473c7b80c120918b91b1b4b) )   /* SCR 16x16 tiles */
	ROM_LOAD32_WORD( "d67-09", 0x000002, 0x200000, CRC(3c19f9e3) SHA1(7ba8475d37cbf8bf38029124afdf62c915c8668d) )

	ROM_REGION( 0xa00000, "sprites", 0 )
	ROM_LOAD16_WORD_SWAP( "d67-03", 0x000000, 0x200000, CRC(3b6e99a9) SHA1(1e0e66763ddfa18a2d291626b245633555092959) )   /* OBJ 16x16 tiles */
	ROM_LOAD16_WORD_SWAP( "d67-04", 0x200000, 0x200000, CRC(8f2934c9) SHA1(ead95b34eec3a6df27199edcbdd5595bc6555a50) )
	ROM_LOAD16_WORD_SWAP( "d67-05", 0x400000, 0x200000, CRC(e2e7dcf3) SHA1(185dbd0489931123a295139dc0a045ad239018fb) )
	ROM_LOAD16_WORD_SWAP( "d67-06", 0x600000, 0x200000, CRC(a2a63488) SHA1(a1ed140cc3757c3c05a0a822089c6efc83bf4805) )
	ROM_LOAD16_WORD_SWAP( "d67-07", 0x800000, 0x200000, CRC(189c0ee5) SHA1(de85b39dc67f31ef80800ff6ec9a391652eb12e4) )

	ROM_REGION( 0x200000, "tc0620scc", 0 )
	ROM_LOAD16_BYTE( "d67-10", 0x000001, 0x100000, CRC(d79e6ce9) SHA1(8b38302971816d599cdaa3279cb6395441373c6f) )   /* PIV 8x8 tiles, 4bpp */
	ROM_LOAD16_BYTE( "d67-11", 0x000000, 0x100000, CRC(7a401bb3) SHA1(47257a6a4b37ec1ceb4e974b776ee3ea30db06fa) )

	ROM_REGION( 0x100000, "tc0620scc:hi_gfx", 0 )
	ROM_LOAD       ( "d67-12", 0x000000, 0x100000, CRC(67b16fec) SHA1(af0f9f50516331780ef6cfab1e12a23edf87daa7) )   /* PIV 8x8 tiles, 2bpp */

	ROM_REGION16_LE( 0x80000, "spritemap", 0 )
	ROM_LOAD16_WORD( "d67-13", 0x00000,  0x80000,  CRC(42e7690d) SHA1(5f00f3f814653733bf9a5cb010675799de02fa76) )   /* STY, spritemap */

	ROM_REGION16_BE( 0x1000000, "taito_en:ensoniq", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "d67-01", 0x000000, 0x200000, CRC(a2f18122) SHA1(640014c6e6d66c59fe0accf370ad3bab9f40429a) )   /* Ensoniq samples */
	ROM_LOAD16_BYTE( "d67-02", 0xc00000, 0x200000, CRC(fceb715e) SHA1(9326513acb0696669d4f2345649ab37c8c6ed171) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "eeprom-undrfire.bin", 0x0000, 0x0080, CRC(9f7368f4) SHA1(4bb28e6eb3a72a06341199f0d744ed0ce13bce2c) )
ROM_END

ROM_START( cbombers )
	ROM_REGION( 0x200000, "maincpu", 0 )    /* 2048K for 68020 code (CPU A) */
	ROM_LOAD32_BYTE( "d83_39.ic17", 0x00000, 0x80000, CRC(b9f48284) SHA1(acc5d412e8900dda483a89a1ac1febd6d5735f3c) )
	ROM_LOAD32_BYTE( "d83_41.ic4",  0x00001, 0x80000, CRC(a2f4c8be) SHA1(0f8f3b5ecff34d8c35af1ab11bb5528b52e30109) )
	ROM_LOAD32_BYTE( "d83_40.ic3",  0x00002, 0x80000, CRC(b05f59ea) SHA1(e46a31737f44be2a3d478b8010fe0d6383290e03) )
	ROM_LOAD32_BYTE( "d83_38.ic16", 0x00003, 0x80000, CRC(0a10616c) SHA1(c9cfc8c870f8a989f004d2db4f6fb76e5b7b7f9b) )

	ROM_REGION( 0x140000, "taito_en:audiocpu", 0 )   /* Sound cpu */
	ROM_LOAD16_BYTE( "d83_26.ic37", 0x100000, 0x20000, CRC(4f49b484) SHA1(96daa3cb7fa4aae3aedc91ec27d85945311dfcc9) )
	ROM_LOAD16_BYTE( "d83_27.ic38", 0x100001, 0x20000, CRC(2aa1a237) SHA1(b809f75bbbbb4eb5d0df725aaa31aae8a6fba552) )

	ROM_REGION( 0x40000, "sub", 0 ) /* 256K for 68000 code (CPU B) */
	ROM_LOAD16_BYTE( "d83_28.ic26", 0x00001, 0x20000, CRC(06328ef7) SHA1(90a14649e56221e47b87958896f6eae4556265c2) )
	ROM_LOAD16_BYTE( "d83_29.ic27", 0x00000, 0x20000, CRC(771b4080) SHA1(a47c3a6abc07a6a61b694d32baa0ad4c25045841) )

	ROM_REGION( 0x400000, "tc0480scp", 0 )
	ROM_LOAD32_WORD( "d83_04.ic8", 0x000000, 0x200000, CRC(79f36cce) SHA1(2c8dc4cd5c4aa335c1e45888f5947acf94fa628a) )
	ROM_LOAD32_WORD( "d83_05.ic7", 0x000002, 0x200000, CRC(7787e495) SHA1(1758de5fdd1d12727368d08d7d4752c3756fc23e) )

	ROM_REGION( 0xf00000, "sprites", 0 )
	ROM_LOAD16_WORD_SWAP( "d83_06.ic28", 0x000000, 0x200000, CRC(4b71944e) SHA1(e8ed190280c7378fb4edcb192cef0d4d62582ad5) )
	ROM_LOAD16_WORD_SWAP( "d83_07.ic30", 0x300000, 0x200000, CRC(29861b61) SHA1(76562b0243c1bc38623c0ef9d20de7572a979e37) )
	ROM_LOAD16_WORD_SWAP( "d83_08.ic32", 0x600000, 0x200000, CRC(a0e81e01) SHA1(96ad8cfc849caaf85350cfc7cf23ad23635a3813) )
	ROM_LOAD16_WORD_SWAP( "d83_09.ic45", 0x900000, 0x200000, CRC(7e4dec50) SHA1(4d8c1be739d425d8ded07774094b775f35a915bf) )
	ROM_LOAD16_WORD_SWAP( "d83_10.ic43", 0xc00000, 0x200000, CRC(36c440a0) SHA1(31685d3cdf4e39e1365df7e6a588c28f95d7e0a8) )
	ROM_LOAD16_WORD_SWAP( "d83_11.ic41", 0x200000, 0x100000, CRC(a790e490) SHA1(9c57405ef2ef3368eb0958a3e43601110c1cc90d) )
	ROM_LOAD16_WORD_SWAP( "d83_12.ic29", 0x500000, 0x100000, CRC(2f237b0d) SHA1(2ecb947671d263a77510bfebda03f883b55b8df4) )
	ROM_LOAD16_WORD_SWAP( "d83_13.ic31", 0x800000, 0x100000, CRC(c2cceeb6) SHA1(3ec932655326caed13a40394bbf8e8baf836de2a) )
	ROM_LOAD16_WORD_SWAP( "d83_14.ic44", 0xb00000, 0x100000, CRC(8b6f4f12) SHA1(6a28004d287f00627622376aa3d6704f2684a6f3) )
	ROM_LOAD16_WORD_SWAP( "d83_15.ic42", 0xe00000, 0x100000, CRC(1b71175e) SHA1(60ad38ce97fd7995ff2f29d6b1a3b873dc2f0eb3) )

	ROM_REGION( 0x200000, "tc0620scc", 0 )
	ROM_LOAD16_BYTE( "d83_16.ic19", 0x000001, 0x100000, CRC(d364cf1e) SHA1(ee43f50edf50ec840acfb98b1314140ee9693839) )
	ROM_LOAD16_BYTE( "d83_17.ic5",  0x000000, 0x100000, CRC(0ffe737c) SHA1(5923a4edf9d0c8339f793840c2bdc691e2c651e6) )

	ROM_REGION( 0x100000, "tc0620scc:hi_gfx", 0 )
	ROM_LOAD       ( "d83_18.ic6",  0x000000, 0x100000, CRC(87979155) SHA1(0ffafa970f9f9c98f8938104b97e63d2b5757804) )

	ROM_REGION16_LE( 0x80000, "spritemap", 0 )
	ROM_LOAD16_BYTE( "d83_31.ic10", 0x000001, 0x40000, CRC(85c37961) SHA1(15ea5c4904d910575e984e146c8941dff913d45f) )
	ROM_LOAD16_BYTE( "d83_32.ic11", 0x000000, 0x40000, CRC(b0db2559) SHA1(2bfae2dbe164b42e95d0a93fab82b7040c3fbc56) )

	ROM_REGION( 0x40000, "spritemaphi", 0 )
	ROM_LOAD( "d83_30.ic9", 0x00000,  0x40000,  CRC(eb86dc67) SHA1(31c7b6f30ff912fafed4b87ce8bf603ee17d1664) )

	ROM_REGION16_BE( 0x1000000, "taito_en:ensoniq" , ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "d83_01.ic40", 0xc00000, 0x200000, CRC(912799f4) SHA1(22f69e61519d2cddcfc4e4c9601e78a9d5265d5b) )
	ROM_LOAD16_BYTE( "d83_02.ic39", 0x000000, 0x200000, CRC(2abca020) SHA1(3491a95651ca89b7fe6d040b8576fa7646bfe84b) )
	ROM_RELOAD     (                0x400000, 0x200000 )
	ROM_LOAD16_BYTE( "d83_03.ic18", 0x800000, 0x200000, CRC(1b2d9ec3) SHA1(ead6b5542ad3987ef0f9ea01ce7f960abc9119b3) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "eeprom-cbombers.bin", 0x0000, 0x0080, CRC(9f7368f4) SHA1(4bb28e6eb3a72a06341199f0d744ed0ce13bce2c) )
ROM_END

ROM_START( cbombersj )
	ROM_REGION( 0x200000, "maincpu", 0 )    /* 2048K for 68020 code (CPU A) */
	ROM_LOAD32_BYTE( "d83_34.ic17", 0x00000, 0x80000, CRC(78e85cb1) SHA1(96219ee6eba04f6b490bb46dab99147036104791) )
	ROM_LOAD32_BYTE( "d83_36.ic4",  0x00001, 0x80000, CRC(fdc936cf) SHA1(834c5a50387a63098d3f9b8ff0baf5d72d3c319c) )
	ROM_LOAD32_BYTE( "d83_35.ic3",  0x00002, 0x80000, CRC(b0379b1e) SHA1(fc398f5f67d1347b594563a3703db4c0e01d5dff) )
	ROM_LOAD32_BYTE( "d83_33.ic16", 0x00003, 0x80000, CRC(72fb42a7) SHA1(7c0b02be604b8b90c444a973962d4b29a5b8b047) )

	ROM_REGION( 0x140000, "taito_en:audiocpu", 0 )   /* Sound cpu */
	ROM_LOAD16_BYTE( "d83_26.ic37", 0x100000, 0x20000, CRC(4f49b484) SHA1(96daa3cb7fa4aae3aedc91ec27d85945311dfcc9) )
	ROM_LOAD16_BYTE( "d83_27.ic38", 0x100001, 0x20000, CRC(2aa1a237) SHA1(b809f75bbbbb4eb5d0df725aaa31aae8a6fba552) )

	ROM_REGION( 0x40000, "sub", 0 ) /* 256K for 68000 code (CPU B) */
	ROM_LOAD16_BYTE( "d83_28.ic26", 0x00001, 0x20000, CRC(06328ef7) SHA1(90a14649e56221e47b87958896f6eae4556265c2) )
	ROM_LOAD16_BYTE( "d83_29.ic27", 0x00000, 0x20000, CRC(771b4080) SHA1(a47c3a6abc07a6a61b694d32baa0ad4c25045841) )

	ROM_REGION( 0x400000, "tc0480scp", 0 )
	ROM_LOAD32_WORD( "d83_04.ic8", 0x000000, 0x200000, CRC(79f36cce) SHA1(2c8dc4cd5c4aa335c1e45888f5947acf94fa628a) )
	ROM_LOAD32_WORD( "d83_05.ic7", 0x000002, 0x200000, CRC(7787e495) SHA1(1758de5fdd1d12727368d08d7d4752c3756fc23e) )

	ROM_REGION( 0xf00000, "sprites", 0 )
	ROM_LOAD16_WORD_SWAP( "d83_06.ic28", 0x000000, 0x200000, CRC(4b71944e) SHA1(e8ed190280c7378fb4edcb192cef0d4d62582ad5) )
	ROM_LOAD16_WORD_SWAP( "d83_07.ic30", 0x300000, 0x200000, CRC(29861b61) SHA1(76562b0243c1bc38623c0ef9d20de7572a979e37) )
	ROM_LOAD16_WORD_SWAP( "d83_08.ic32", 0x600000, 0x200000, CRC(a0e81e01) SHA1(96ad8cfc849caaf85350cfc7cf23ad23635a3813) )
	ROM_LOAD16_WORD_SWAP( "d83_09.ic45", 0x900000, 0x200000, CRC(7e4dec50) SHA1(4d8c1be739d425d8ded07774094b775f35a915bf) )
	ROM_LOAD16_WORD_SWAP( "d83_10.ic43", 0xc00000, 0x200000, CRC(36c440a0) SHA1(31685d3cdf4e39e1365df7e6a588c28f95d7e0a8) )
	ROM_LOAD16_WORD_SWAP( "d83_11.ic41", 0x200000, 0x100000, CRC(a790e490) SHA1(9c57405ef2ef3368eb0958a3e43601110c1cc90d) )
	ROM_LOAD16_WORD_SWAP( "d83_12.ic29", 0x500000, 0x100000, CRC(2f237b0d) SHA1(2ecb947671d263a77510bfebda03f883b55b8df4) )
	ROM_LOAD16_WORD_SWAP( "d83_13.ic31", 0x800000, 0x100000, CRC(c2cceeb6) SHA1(3ec932655326caed13a40394bbf8e8baf836de2a) )
	ROM_LOAD16_WORD_SWAP( "d83_14.ic44", 0xb00000, 0x100000, CRC(8b6f4f12) SHA1(6a28004d287f00627622376aa3d6704f2684a6f3) )
	ROM_LOAD16_WORD_SWAP( "d83_15.ic42", 0xe00000, 0x100000, CRC(1b71175e) SHA1(60ad38ce97fd7995ff2f29d6b1a3b873dc2f0eb3) )

	ROM_REGION( 0x200000, "tc0620scc", 0 )
	ROM_LOAD16_BYTE( "d83_16.ic19", 0x000001, 0x100000, CRC(d364cf1e) SHA1(ee43f50edf50ec840acfb98b1314140ee9693839) )
	ROM_LOAD16_BYTE( "d83_17.ic5",  0x000000, 0x100000, CRC(0ffe737c) SHA1(5923a4edf9d0c8339f793840c2bdc691e2c651e6) )

	ROM_REGION( 0x100000, "tc0620scc:hi_gfx", 0 )
	ROM_LOAD       ( "d83_18.ic6",  0x000000, 0x100000, CRC(87979155) SHA1(0ffafa970f9f9c98f8938104b97e63d2b5757804) )

	ROM_REGION16_LE( 0x80000, "spritemap", 0 )
	ROM_LOAD16_BYTE( "d83_31.ic10", 0x000001, 0x40000, CRC(85c37961) SHA1(15ea5c4904d910575e984e146c8941dff913d45f) )
	ROM_LOAD16_BYTE( "d83_32.ic11", 0x000000, 0x40000, CRC(b0db2559) SHA1(2bfae2dbe164b42e95d0a93fab82b7040c3fbc56) )

	ROM_REGION( 0x40000, "spritemaphi", 0 )
	ROM_LOAD( "d83_30.ic9", 0x00000,  0x40000,  CRC(eb86dc67) SHA1(31c7b6f30ff912fafed4b87ce8bf603ee17d1664) )

	ROM_REGION16_BE( 0x1000000, "taito_en:ensoniq" , ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "d83_01.ic40", 0xc00000, 0x200000, CRC(912799f4) SHA1(22f69e61519d2cddcfc4e4c9601e78a9d5265d5b) )
	ROM_LOAD16_BYTE( "d83_02.ic39", 0x000000, 0x200000, CRC(2abca020) SHA1(3491a95651ca89b7fe6d040b8576fa7646bfe84b) )
	ROM_RELOAD     (                0x400000, 0x200000 )
	ROM_LOAD16_BYTE( "d83_03.ic18", 0x800000, 0x200000, CRC(1b2d9ec3) SHA1(ead6b5542ad3987ef0f9ea01ce7f960abc9119b3) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "eeprom-cbombers.bin", 0x0000, 0x0080, CRC(9f7368f4) SHA1(4bb28e6eb3a72a06341199f0d744ed0ce13bce2c) )
ROM_END



ROM_START( cbombersp )
	ROM_REGION( 0x200000, "maincpu", 0 )    // 2048K for 68020 code (CPU A)
	ROM_LOAD32_BYTE( "hh.bin",  0x00000, 0x80000, CRC(8ee7b6c2) SHA1(033bc2c207b458113ff9f81cb90d22b7984f49de) )
	ROM_LOAD32_BYTE( "hl.bin",  0x00001, 0x80000, CRC(63683ed8) SHA1(e6b57300f712fac3cf54f70ca255e514018a1c81) )
	ROM_LOAD32_BYTE( "lh.bin",  0x00002, 0x80000, CRC(77ad9acb) SHA1(73bc156c377d42e7c752d71686ca5ad3dc506053) )
	ROM_LOAD32_BYTE( "ll.bin",  0x00003, 0x80000, CRC(54c060a5) SHA1(7623eb418734d2adc4e913b5a99d4caeb857b3ac) )

	ROM_REGION( 0x140000, "taito_en:audiocpu", 0 )   // Sound cpu
	ROM_LOAD16_BYTE( "ic47_eprg_h_up_c5c5.bin", 0x100000, 0x20000, CRC(cb0d11b1) SHA1(0ac192f31f531defe088ed899aadbaa0798df4d6) )
	ROM_LOAD16_BYTE( "ic46_eprg_l_low_d5e.bin", 0x100001, 0x20000, CRC(567ae215) SHA1(32d40d1c24e014ffb52a81df4a0d5adb20f0048c) )

	ROM_REGION( 0x40000, "sub", 0 ) // 256K for 68000 code (CPU B)
	ROM_LOAD16_BYTE( "5-l.bin", 0x00001, 0x20000, CRC(aed4c3c0) SHA1(004f83ce0739cb2839022eb4d83f82e54776914f) )
	ROM_LOAD16_BYTE( "5-h.bin", 0x00000, 0x20000, CRC(c6ec60e4) SHA1(554f19926e050ff2b8c56c30f174aca5a3fff845) )

	ROM_REGION( 0x400000, "tc0480scp", 0 )
	ROM_LOAD32_BYTE( "scp0aa_2b04_ic35.bin",    0x000003, 0x80000, CRC(b8ec56bd) SHA1(00191fd4b2e315a5a18f2e690a45b6f6a6ebb3d2) )
	ROM_LOAD32_BYTE( "scp0hl_ic9.bin",          0x000002, 0x80000, CRC(5b6e413e) SHA1(7eaee158e985a20b5c228b476ee102f88311423a) )
	ROM_LOAD32_BYTE( "scp0lh_ic22.bin",         0x000001, 0x80000, CRC(d5109bca) SHA1(c4c5b8dbc1139718d2aa73413b1b206f9df10fed) )
	ROM_LOAD32_BYTE( "scp0ll_ic7.bin",          0x000000, 0x80000, CRC(b1af439d) SHA1(a13fb4242808d1f3fc629988912c8186a99fb878) )
	ROM_LOAD32_BYTE( "scp1hh_ic36.bin",         0x200003, 0x80000, CRC(24f545d8) SHA1(c5ae0e714ed4765f3416cb58bc9cfccfbf78081c) )
	ROM_LOAD32_BYTE( "scp1hl_ic24.bin",         0x200002, 0x80000, CRC(46d198ba) SHA1(d9c9ddb23ad8f2abbd0ab2322d31d929085f0591) )
	ROM_LOAD32_BYTE( "scp1lh_ic23.bin",         0x200001, 0x80000, CRC(7c9f0035) SHA1(a5632bd11426ba2cf0016847a3c08a2b90498271) )
	ROM_LOAD32_BYTE( "scp1ll_ic8.bin",          0x200000, 0x80000, CRC(eaa5839a) SHA1(80c7bb1151253a23934b65110db973641f7a073e) )

	ROM_REGION( 0xf00000, "sprites", 0 )
	// tiles 0x00000 - 0x07fff
	ROM_LOAD16_BYTE( "obj0l_ic29.bin",   0x0000001, 0x80000, CRC(4b954950) SHA1(cafd9ba3128aa2e7dbde959a705aff8db6c311fa) ) // bp 1
	ROM_LOAD16_BYTE( "obj16l_ic20.bin",  0x0300001, 0x80000, CRC(b53932c0) SHA1(94ea6ccc29bd7b7e94d7494aaf0cc19b67c4ce72) ) // bp 2
	ROM_LOAD16_BYTE( "obj32l_ic51.bin",  0x0600001, 0x80000, CRC(f23f7253) SHA1(cbff5aee79d1b4990a35d5fc55e348aa81b2b5d3) ) // bp 3
	ROM_LOAD16_BYTE( "obj48l_ic53.bin",  0x0900001, 0x80000, CRC(85bb6b75) SHA1(ccff9c331b54e3c3618d205ffd4b999fcf861e09) ) // bp 4
	ROM_LOAD16_BYTE( "obj64l_ic55.bin",  0x0c00001, 0x80000, CRC(18c976f1) SHA1(4fa53bbfaacd8ca9c7bec3aeff13481302aeefdc) ) // bp 5
	ROM_LOAD16_BYTE( "obj8l_ic4.bin",    0x0000000, 0x80000, CRC(d26140bb) SHA1(c882150c2914f23c5a27b43a228c62401cdb1a73) ) // bp 1
	ROM_LOAD16_BYTE( "obj24l_ic50.bin",  0x0300000, 0x80000, CRC(27c76f27) SHA1(c3e96015abc042420632e489e202684c358a5f42) ) // bp 2
	ROM_LOAD16_BYTE( "obj40l_ic52.bin",  0x0600000, 0x80000, CRC(09aaf6c5) SHA1(f2838fbf5ede241939562a65ecb908d882f267e2) ) // bp 3
	ROM_LOAD16_BYTE( "obj56l_ic54.bin",  0x0900000, 0x80000, CRC(8b6bacdf) SHA1(fe22f700147f641339ef754b3e5896279b5c2800) ) // bp 4
	ROM_LOAD16_BYTE( "obj72l_ic56.bin",  0x0c00000, 0x80000, CRC(6a1b5ebc) SHA1(632d19d73c8cf75e3c428f1870e75c07b6846be5) ) // bp 5
	// tiles 0x08000 - 0x0ffff
	ROM_LOAD16_BYTE( "obj0h_ic30.bin",   0x0100001, 0x80000, CRC(4c436ad2) SHA1(34fb6bb5aa7b131dda2d167a5dacc09090698dd1) ) // bp 1
	ROM_LOAD16_BYTE( "obj16h_ic21.bin",  0x0400001, 0x80000, CRC(5406b71e) SHA1(5e10db4d9052b903987629aa1df34bd4380ad29b) ) // bp 2
	ROM_LOAD16_BYTE( "ic65_4b57.bin",    0x0700001, 0x80000, CRC(6a1a8054) SHA1(20a001b1957fa67297959b86e20a0c2a417c2452) ) // bp 3
	ROM_LOAD16_BYTE( "ic67_0956.bin",    0x0a00001, 0x80000, CRC(abe445dd) SHA1(22ad1ace2ee80a2297e4a004b6f5cd52c7c9b187) ) // bp 4
	ROM_LOAD16_BYTE( "ic69_43b4.bin",    0x0d00001, 0x80000, CRC(d08643be) SHA1(aacc324276c92b5786298b474ad6277fb802c470) ) // bp 5
	ROM_LOAD16_BYTE( "obj8l_ic5.bin",    0x0100000, 0x80000, CRC(46b028eb) SHA1(02bb36108636695bc6db523991b34713599b2716) ) // bp 1
	ROM_LOAD16_BYTE( "ic64_5aba.bin",    0x0400000, 0x80000, CRC(0912b766) SHA1(62a8c0f2341b7e1f7b7ba0e194eda39f502209e6) ) // bp 2
	ROM_LOAD16_BYTE( "ic66_4ae9.bin",    0x0700000, 0x80000, CRC(77aafe1a) SHA1(35392526d5b6d974bff3bda316677fa0890b76a8) ) // bp 3
	ROM_LOAD16_BYTE( "ic68_1429.bin",    0x0a00000, 0x80000, CRC(2e5857e5) SHA1(04bdd2e7cc89b14c7d7d72de45a7cce74cbcf7bd) ) // bp 4
	ROM_LOAD16_BYTE( "ic70_4504.bin",    0x0d00000, 0x80000, CRC(3cf5d9d7) SHA1(35cc8b403550fb9928003308139de05ceb7bd5bb) ) // bp 5
	// tiles 0x10000 - 0x17fff
	ROM_LOAD16_BYTE( "obj0l_ic31.bin",   0x0200001, 0x80000, CRC(9a20d601) SHA1(f3e561ed8687b65c6328c10d69c74615e693a83f) ) // bp 1
	ROM_LOAD16_BYTE( "obj16l_ic33.bin",  0x0500001, 0x80000, CRC(ea9df360) SHA1(4b0b50679ae4438f117b631b76b02a5170cc292a) ) // bp 2
	ROM_LOAD16_BYTE( "ic77_36ac.bin",    0x0800001, 0x80000, CRC(75628014) SHA1(62f8ba94be047e54c3577e6ecd68b3e613905a9f) ) // bp 3
	ROM_LOAD16_BYTE( "ic79_ef40.bin",    0x0b00001, 0x80000, CRC(6af34bbf) SHA1(7bd021fc041465abe8dc6ac1ada634ae7314fec9) ) // bp 4
	ROM_LOAD16_BYTE( "ic81_e150.bin",    0x0e00001, 0x80000, CRC(48dbc4fb) SHA1(acec207d05a8ea615f27216fbfd567cc630e5191) ) // bp 5
	ROM_LOAD16_BYTE( "obj8l_ic6.bin",    0x0200000, 0x80000, CRC(2037aad5) SHA1(0c837a39d92e99536bcd949c80fbdd4c37ad7e34) ) // bp 1
	ROM_LOAD16_BYTE( "ic76_443a.bin",    0x0500000, 0x80000, CRC(e5820610) SHA1(0eefb098766187983d88017863ce3a3418807428) ) // bp 2
	ROM_LOAD16_BYTE( "ic78_989c.bin",    0x0800000, 0x80000, CRC(23ec2896) SHA1(b5fcc88d2428ff9341690a0e0a1fc25e42684680) ) // bp 3
	ROM_LOAD16_BYTE( "ic80_d511.bin",    0x0b00000, 0x80000, CRC(37da5baf) SHA1(a78ac413de08a1ff70ab14561b75df633a9e5be8) ) // bp 4
	ROM_LOAD16_BYTE( "ic82_3d3d.bin",    0x0e00000, 0x80000, CRC(3e62970e) SHA1(82970accb4ce29034e7b97b74c831ec0314c5a8f) ) // bp 5

	ROM_REGION( 0x200000, "tc0620scc", 0 )
	ROM_LOAD16_BYTE( "ic44_scc1.bin",  0x000000, 0x080000, CRC(868d0d3d) SHA1(29251d545548856296b5ae32a96f2eeef2418dc4) )
	ROM_LOAD16_BYTE( "ic43_scc4.bin",  0x000001, 0x080000, CRC(2f170ee4) SHA1(2b8f07186c9f7589e1af131b8c377443a29bd149) )
	ROM_LOAD16_BYTE( "ic58_f357.bin",  0x100000, 0x080000, CRC(16486967) SHA1(c2fd6c9f21232656b52ab589ac61f94aa728524e) )
	ROM_LOAD16_BYTE( "ic57_1a62.bin",  0x100001, 0x080000, CRC(afd45e35) SHA1(6d7c0729c7d2b204473679b97923130e289f429d) )

	ROM_REGION( 0x100000, "tc0620scc:hi_gfx", 0 )
	ROM_LOAD       ( "ic45_5cc2.bin",  0x000000, 0x080000, CRC(7ae48d63) SHA1(2a8b291f0a683ed5b0c39d221737956b6fc72fa5) )
	ROM_LOAD       ( "ic59_7cce.bin",  0x080000, 0x080000, CRC(ee762199) SHA1(d56e96feeedba8b77f8f18cb380d2902ca3f1e50) )

	ROM_REGION16_LE( 0x80000, "spritemap", 0 )
	ROM_LOAD16_BYTE( "st8_ic2.bin", 0x000001, 0x40000, CRC(d74254d8) SHA1(f4a4f9d95f70edf74d937be067d6a9f68a955ea7) )
	ROM_LOAD16_BYTE( "st0_ic1.bin", 0x000000, 0x40000, CRC(c414c479) SHA1(e585502fcfa6ae2a36a66927ba2c49e49317f149) )

	ROM_REGION( 0x40000, "spritemaphi", 0 )
	ROM_LOAD( "st16_ic3.bin", 0x00000,  0x40000, CRC(c4ff6b2f) SHA1(65795bcb3749cce9c291204cd64fafa529317e14) )

	ROM_REGION16_BE( 0x1000000, "taito_en:ensoniq" , ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE("ic84_0816_wave0.bin", 0x000000, 0x080000, CRC(c30c71fd) SHA1(4240a23120b85f9daf6a462770185614fa758e4d) )
	ROM_LOAD16_BYTE("ic85_8058_wave1.bin", 0x100000, 0x080000, CRC(fe37d544) SHA1(75c23b4e4e2efbbda1724a557f68c3bf1e5016c6) )
	ROM_LOAD16_BYTE("ic86_9e88_wave2.bin", 0x200000, 0x080000, CRC(d6dcb45d) SHA1(ec69fb0a9fc6f7e72850775656e9fcd185889825) )
	ROM_LOAD16_BYTE("ic87_42e7_wave3.bin", 0x300000, 0x080000, CRC(fe52856b) SHA1(fc68301d8b514a142cb38c3aa7a081674bdba6ca) )
	ROM_LOAD16_BYTE("ic88_2704_wave4.bin", 0x400000, 0x080000, CRC(cba55d36) SHA1(944188bb3d9466ba246cabc57db6b965f2a902a0) )
	// wave 5 not populated
	// wave 6 not populated
	// wave 7 not populated
	// wave 8 not populated
	// wave 9 not populated
	// wave 10 not populated
	// wave 11 not populated
	// wave 12 not populated
	// wave 13 not populated
	ROM_LOAD16_BYTE("ic107_3a9c_wave14.bin", 0xe00000, 0x080000, CRC(26312451) SHA1(9f947a11592fd8420fc581914bf16e7ade75390c) )    // -std-
	ROM_LOAD16_BYTE("ic108_a148_wave15.bin", 0xf00000, 0x080000, CRC(2edaa9dc) SHA1(72fead505c4f44e5736ff7d545d72dfa37d613e2) )    // -std-
ROM_END


GAME( 1993, undrfire,  0,        undrfire, undrfire, undrfire_state, empty_init, ROT0, "Taito Corporation Japan",   "Under Fire (World)",              0 )
GAME( 1993, undrfireu, undrfire, undrfire, undrfire, undrfire_state, empty_init, ROT0, "Taito America Corporation", "Under Fire (US)",                 0 )
GAME( 1993, undrfirej, undrfire, undrfire, undrfire, undrfire_state, empty_init, ROT0, "Taito Corporation",         "Under Fire (Japan)",              0 )
GAMEL(1994, cbombers,  0,        cbombers, cbombers, undrfire_state, empty_init, ROT0, "Taito Corporation Japan",   "Chase Bombers (World)",           MACHINE_IMPERFECT_GRAPHICS | MACHINE_NODEVICE_LAN, layout_cbombers )
GAMEL(1994, cbombersj, cbombers, cbombers, cbombers, undrfire_state, empty_init, ROT0, "Taito Corporation",         "Chase Bombers (Japan)",           MACHINE_IMPERFECT_GRAPHICS | MACHINE_NODEVICE_LAN, layout_cbombers )
GAMEL(1994, cbombersp, cbombers, cbombers, cbombers, undrfire_state, empty_init, ROT0, "Taito Corporation",         "Chase Bombers (Japan Prototype)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NODEVICE_LAN, layout_cbombers )
