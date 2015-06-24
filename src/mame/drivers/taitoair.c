// license:LGPL-2.1+
// copyright-holders:Angelo Salese, Olivier Galibert
/***************************************************************************

Taito Air System
----------------

Top Landing             (c) 1988 Taito
Air Inferno             (c) 1990 Taito


(Thanks to Raine team for their preliminary drivers)

Controls:

    P2 y analogue = throttle
    P1 analogue = pitch/yaw control

Can someone with flight sim stick confirm this is sensible.
I think we need OSD display for P1 l/r.


System specs    (from TaitoH: incorrect!)
------------

 CPU   : MC68000 (12 MHz) x 1, Z80 (4 MHz?, sound CPU) x 1
 Sound : YM2610, YM3016?
 OSC   : 20.000 MHz, 8.000 MHz, 24.000 MHz
 Chips : TC0070RGB (Palette?)
         TC0220IOC (Input)
         TC0140SYT (Sound communication)
         TC0130LNB (???)
         TC0160ROM (???)
         TC0080VCO (Video?)

From Ainferno readme
--------------------

Location     Type       File ID
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CPU IC5       9016*     C45-01
CPU IC4       9016*     C45-02
CPU IC3       9016*     C45-03
CPU IC2       9016*     C45-04
CPU IC1       9016*     C45-05
CPU IC31      9016*     C45-06
VID IC28     27C010     C45-11
VID IC29     27C010     C45-12
VID IC30     27C010     C45-13
VID IC31     27C010     C45-14
VID IC40     27C010     C45-15
VID IC41     27C010     C45-16
VID IC42     27C010     C45-17
VID IC43     27C010     C45-18
CPU IC14     27C010     C45-20
CPU IC42     27C010     C45-21
CPU IC43     27C010     C45-22
CPU IC43     27C010     C45-23
CPU IC6      LH5763     C45-24
CPU IC35     LH5763     C45-25
CPU IC13     27C010     C45-28

VID IC6    PAL16L8B     C45-07
VID IC62   PAL16L8B     C45-08
VID IC63   PAL16L8B     C45-09
VID IC2    PAL20L8B     C45-10
CPU IC76   PAL16L8B     C45-26
CPU IC114  PAL16L8B     C45-27
CPU IC60   PAL20L8B     B62-02
CPU IC62   PAL20L8B     B62-03
CPU IC63   PAL20L8B     B62-04
CPU IC82   PAL16L8B     B62-07
VID IC23   PAL16L8B     B62-08
VID IC26   PAL16L8B     B62-11
VID IC27   PAL16L8B     B62-12


Notes:  CPU - CPU PCB      K1100586A  M4300186A
        VID - Video PCB    K1100576A  M4300186A


Known TC0080VCO issues  (from TaitoH driver)
----------------------

 - Y coordinate of sprite zooming is non-linear, so currently implemented
   hand-tuned value and this is used for only Record Breaker.
 - Sprite and BG1 priority bit is not understood. It is defined by sprite
   priority in Record Breaker and by zoom value and some scroll value in
   Dynamite League. So, some priority problems still remain.
 - Background zoom effect is not working in flip screen mode.
 - Sprite zoom is a bit wrong.


Stephh's notes (based on the game M68000 code and some tests) :

1) 'topland'

  - Region stored at 0x03fffe.w
  - Sets :
      * 'topland' : region = 0x0002
  - Coinage relies on the region (code at 0x0016e8) :
      * 0x0000 (Japan) and 0x0001 (US) use TAITO_COINAGE_JAPAN_OLD
      * 0x0002 (World) uses TAITO_COINAGE_WORLD
  - Notice screen only if region = 0x0000


2) 'ainferno'

  - Region stored at 0x07fffe.w
  - Sets :
      * 'ainferno' : region = 0x0002
  - Coinage relies on the region (code at 0x000cec) :
      * 0x0001 (Japan) uses TAITO_COINAGE_JAPAN_OLD
      * 0x0002 (US) uses TAITO_COINAGE_US
      * 0x0003 (World) uses TAITO_COINAGE_WORLD
  - Notice screen only if region = 0x0001 or region = 0x0002
  - FBI logo only if region = 0x0002



TODO    (TC0080VCO issues shared with TaitoH driver)
----

 - Need to implement BG1 : sprite priority. Currently not clear how this works.
 - Fix sprite coordinates.
 - Improve zoom y coordinate.


TODO
----

Video section hung off TaitoH driver, it should be separate.

3d graphics h/w: do the gradiation ram and line ram map to
hardware which creates the 3d background scenes? It seems
the TMS320C25 is being used as a co-processor to relieve the
68000 of 3d calculations... it has direct access to line ram
along with the 68000.

Gradiation RAM is used to display a rotatable gradient background.
The rotation is handled by the TC0430GRW ROZ chip which outputs
coordinates for a X=a1+b1*x+c1*y, Y=a2+b2*x+c2*y mapping.  The
coordinates are used unconventionally as indices in a color palette.

"Power common ram" is for communication with a processor
controlling the sit-in-cabinet (deluxe mechanized version only).
The interface is similar to that used by Midnight Landing
and though undumped, the motor CPU program may be identical.

[Offer dip-selectable kludge of the analogue stick inputs so that
keyboard play is possible?]

Unknown control bits remain in the 0x140000 write.


DIPs
----

They're now correct (including locations) according to the
manuals. Nevertherless, ainferno manual states that the coinage
DIPs are the same as topland, which is clearly wrong if you try
them ("SWB:7,8" do not set Coin B to multiple credits for each
coin!)

Therefore, some verification could still be needed, once the
emulation is complete.


Topland
-------

Sprite/tile priority bad.

After demo game in attract, palette seems too dark for a while.
Palette corruption has occurred with areas not restored after a fade.
Don't know why. (Perhaps 68000 relies on feedback from co-processor
in determining what parts of palette ram to write... but this would
then be fixed by hookup of 32025 core, which it isn't.)

Mechanized cabinet has a problem with test mode: there is
code at $d72 calling a sub which tests a silly amount of "power
common ram"; $80000 words (only one byte per word used).
Probably the address map wraps, and only $400 separate words
are actually accessed ?

TMS320C25 emulation: one unmapped read which appears to be
discarded. But the cpu waits for a bit to be zero... some
sort of frame flag or some "ready" message from the 3d h/w
perhaps? The two writes seem to take only two values.


Ainferno
--------

Sprite/tile priority bad.

More unmapped 320C25 reads and writes. This could be some sort of
I/O device?? The MCU program is longer than the Topland one.

cpu #2 (PC=000000C3): unmapped memory word write to 00006808 = 00FD & FFFF
cpu #2 (PC=000000C8): unmapped memory word write to 00006810 = FF38 & FFFF
cpu #2 (PC=000005A0): unmapped memory word write to 00006836 = 804E & FFFF
cpu #2 (PC=000005B2): unmapped memory word write to 00006830 = FFFF & FFFF
cpu #2 (PC=000005B5): unmapped memory word write to 00006832 = FFFE & FFFF
cpu #2 (PC=000005B8): unmapped memory word write to 00006834 = FBCA & FFFF
cpu #2 (PC=000005B9): unmapped memory word read from 00006836 & FFFF
cpu #2 (PC=000005CC): unmapped memory word write to 00006830 = FFFF & FFFF
cpu #2 (PC=000005CF): unmapped memory word write to 00006832 = FFFE & FFFF
cpu #2 (PC=000005D2): unmapped memory word write to 00006834 = FBCA & FFFF
cpu #2 (PC=000005D3): unmapped memory word read from 00006836 & FFFF
cpu #2 (PC=000005E6): unmapped memory word write to 00006830 = FFFF & FFFF
cpu #2 (PC=000005E9): unmapped memory word write to 00006832 = FFFE & FFFF
cpu #2 (PC=000005EC): unmapped memory word write to 00006834 = FC8F & FFFF
cpu #2 (PC=000005ED): unmapped memory word read from 00006836 & FFFF
cpu #2 (PC=00000600): unmapped memory word write to 00006830 = FFFF & FFFF
cpu #2 (PC=00000603): unmapped memory word write to 00006832 = FFFE & FFFF
cpu #2 (PC=00000606): unmapped memory word write to 00006834 = FC8F & FFFF
cpu #2 (PC=00000607): unmapped memory word read from 00006836 & FFFF
cpu #2 (PC=00000609): unmapped memory word read from 00006838 & FFFF
cpu #2 (PC=0000060E): unmapped memory word read from 0000683A & FFFF

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "includes/taitoipt.h"
#include "includes/taitoair.h"
#include "audio/taitosnd.h"
#include "cpu/tms32025/tms32025.h"
#include "sound/2610intf.h"

/***********************************************************
                MEMORY handlers
***********************************************************/

WRITE16_MEMBER(taitoair_state::system_control_w)
{
	if ((ACCESSING_BITS_0_7 == 0) && ACCESSING_BITS_8_15)
		data >>= 8;

	m_dsp_hold_signal = (data & 4) ? CLEAR_LINE : ASSERT_LINE;

	m_dsp->set_input_line(INPUT_LINE_RESET, (data & 1) ? CLEAR_LINE : ASSERT_LINE);

	logerror("68K:%06x writing %04x to TMS32025.  %s HOLD , %s RESET\n", space.device().safe_pcbase(), data, ((data & 4) ? "Clear" : "Assert"), ((data & 1) ? "Clear" : "Assert"));
}

READ16_MEMBER(taitoair_state::lineram_r)
{
	return m_line_ram[offset];
}

WRITE16_MEMBER(taitoair_state::lineram_w)
{
	if (ACCESSING_BITS_8_15 && ACCESSING_BITS_0_7)
		m_line_ram[offset] = data;

	//if(offset == 0x3fff)
	//  printf("LineRAM go %d\n",(int)m_screen->frame_number());
}

READ16_MEMBER(taitoair_state::dspram_r)
{
	return m_dsp_ram[offset];
}

WRITE16_MEMBER(taitoair_state::dspram_w)
{
	if (ACCESSING_BITS_8_15 && ACCESSING_BITS_0_7)
		m_dsp_ram[offset] = data;
}

READ16_MEMBER(taitoair_state::dsp_HOLD_signal_r)
{
	/* HOLD signal is active low */
	//  logerror("TMS32025:%04x Reading %01x level from HOLD signal\n", space.device().safe_pcbase(), m_dsp_hold_signal);

	return m_dsp_hold_signal;
}

WRITE16_MEMBER(taitoair_state::dsp_HOLDA_signal_w)
{
	if (offset)
		logerror("TMS32025:%04x Writing %01x level to HOLD-Acknowledge signal\n", space.device().safe_pcbase(), data);
}


WRITE16_MEMBER(taitoair_state::airsys_paletteram16_w)/* xxBBBBxRRRRxGGGG */
{
	int a;

	COMBINE_DATA(&m_paletteram[offset]);

	a = m_paletteram[offset];
	m_palette->set_pen_color(offset, pal4bit(a >> 0), pal4bit(a >> 5), pal4bit(a >> 10));
}

WRITE16_MEMBER(taitoair_state::airsys_gradram_w)
{
	UINT32 pen;
	int r,g,b;
	//int pal_r,pal_g,pal_b;

	COMBINE_DATA(&m_gradram[offset]);
	offset &= 0x1fff;

	pen = (m_gradram[offset])|(m_gradram[(offset+0x2000)]<<16);
	/* TODO: correct? */
	r = (pen & 0x00007f) >> 0;
	g = (pen & 0x007f00) >> (8);
	b = (pen & 0x7f0000) >> (16);

	r = (r << 1) | (r & 1);
	g = (g << 1) | (g & 1);
	b = (b << 1) | (b & 1);

	/* TODO: I'm sure that normal paletteram and gradiation ram mixes in some way ... */
	//pal_r = ((m_paletteram[(offset >> 7) + 0x300] & 0x000f) >> 0) * 0x11;
	//pal_g = ((m_paletteram[(offset >> 7) + 0x300] & 0x01e0) >> 5) * 0x11;
	//pal_b = ((m_paletteram[(offset >> 7) + 0x300] & 0x7c00) >> 10) * 0x11;

	//if(r == 0) { r = (pal_r); }
	//if(g == 0) { g = (pal_g); }
	//if(b == 0) { b = (pal_b); }

	m_palette->set_pen_color(offset+0x2000, r, g, b);
}


/***********************************************************
                INPUTS
***********************************************************/

#define STICK1_PORT_TAG  "STICK1"
#define STICK2_PORT_TAG  "STICK2"
#define STICK3_PORT_TAG  "STICK3"

READ16_MEMBER(taitoair_state::stick_input_r)
{
	switch( offset )
	{
		case 0x00:  /* "counter 1" lo */
			return ioport(STICK1_PORT_TAG)->read();

		case 0x01:  /* "counter 2" lo */
			return ioport(STICK2_PORT_TAG)->read();

		case 0x02:  /* "counter 1" hi */
			if(ioport(STICK1_PORT_TAG)->read() & 0x80)
				return 0xff;

			return 0;

		case 0x03:  /* "counter 2" hi */
			return (ioport(STICK2_PORT_TAG)->read() & 0xff00) >> 8;
	}

	return 0;
}

READ16_MEMBER(taitoair_state::stick2_input_r)
{
	switch( offset )
	{
		case 0x00:  /* "counter 3" lo */
			return ioport(STICK3_PORT_TAG)->read();

		case 0x02:  /* "counter 3" hi */
			return (ioport(STICK3_PORT_TAG)->read() & 0xff00) >> 8;
	}

	return 0;
}

WRITE8_MEMBER(taitoair_state::sound_bankswitch_w)
{
	membank("z80bank")->set_entry(data & 3);
}


/***********************************************************
             MEMORY STRUCTURES
***********************************************************/

static ADDRESS_MAP_START( airsys_map, AS_PROGRAM, 16, taitoair_state )
	AM_RANGE(0x000000, 0x0bffff) AM_ROM
	AM_RANGE(0x0c0000, 0x0cffff) AM_RAM AM_SHARE("m68000_mainram")
	AM_RANGE(0x140000, 0x140001) AM_WRITE(system_control_w) /* Pause the TMS32025 */
	AM_RANGE(0x180000, 0x187fff) AM_RAM_WRITE(airsys_gradram_w) AM_SHARE("gradram")                 /* "gradiation ram (0/1)" */
	AM_RANGE(0x188000, 0x189fff) AM_MIRROR(0x2000) AM_RAM_WRITE(airsys_paletteram16_w) AM_SHARE("paletteram")
	AM_RANGE(0x800000, 0x820fff) AM_DEVREADWRITE("tc0080vco", tc0080vco_device, word_r, word_w)    /* tilemaps, sprites */
	AM_RANGE(0x906000, 0x906007) AM_RAM // DMA?
	AM_RANGE(0x908000, 0x90ffff) AM_RAM AM_SHARE("line_ram")    /* "line ram" */
	AM_RANGE(0x910000, 0x91ffff) AM_RAM AM_SHARE("dsp_ram") /* "dsp common ram" (TMS320C25) */
	AM_RANGE(0x980000, 0x98000f) AM_RAM AM_SHARE("tc0430grw") /* TC0430GRW roz transform coefficients */
	AM_RANGE(0xa00000, 0xa00007) AM_READ(stick_input_r)
	AM_RANGE(0xa00100, 0xa00107) AM_READ(stick2_input_r)
	AM_RANGE(0xa00200, 0xa0020f) AM_DEVREADWRITE8("tc0220ioc", tc0220ioc_device, read, write, 0x00ff) /* other I/O */
	AM_RANGE(0xa80000, 0xa80001) AM_READNOP AM_DEVWRITE8("tc0140syt", tc0140syt_device, master_port_w, 0x00ff)
	AM_RANGE(0xa80002, 0xa80003) AM_DEVREADWRITE8("tc0140syt", tc0140syt_device, master_comm_r, master_comm_w, 0x00ff)
	AM_RANGE(0xb00000, 0xb007ff) AM_RAM                     /* "power common ram" (mecha drive) */
ADDRESS_MAP_END

/************************** Z80 ****************************/

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, taitoair_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("z80bank")
	AM_RANGE(0xc000, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xe003) AM_DEVREADWRITE("ymsnd", ym2610_device, read, write)
	AM_RANGE(0xe200, 0xe200) AM_READNOP AM_DEVWRITE("tc0140syt", tc0140syt_device, slave_port_w)
	AM_RANGE(0xe201, 0xe201) AM_DEVREADWRITE("tc0140syt", tc0140syt_device, slave_comm_r, slave_comm_w)
	AM_RANGE(0xe400, 0xe403) AM_WRITENOP        /* pan control */
	AM_RANGE(0xea00, 0xea00) AM_READNOP
	AM_RANGE(0xee00, 0xee00) AM_WRITENOP        /* ? */
	AM_RANGE(0xf000, 0xf000) AM_WRITENOP        /* ? */
	AM_RANGE(0xf200, 0xf200) AM_WRITE(sound_bankswitch_w)
ADDRESS_MAP_END

/********************************** TMS32025 ********************************/

/*
Air Inferno:

write to 0x3404 - almost always 0x00fd / 0xff38  (253, -200)
write to 0x3408 /

write to 0x341b - May not be numeric - it's weird.  stays stable,
                  then freaks out just before "quad: unknown value 0066"
                  This function seems to break things up into different polygon
                  'classes'

write to 0x3418 - X value
write to 0x3419 - Y value
write to 0x341a - Z value
read to 0x341b, puts data to internal RAM 0x380 - 0x384 - 0x388 - 0x38c

checks 0x341c - if != to 0 then skip ... ?
checks 0x341d - if == to 0 then skip ... ?

write to 0x3405 ; X value
write to 0x3409 ; Y value
write to 0x3406 ; Z value
write to 0x340a ; Z value
read to 0x340b, puts to line RAM (y) with offset + 0x160
read to 0x3407, puts to line RAM (x) with offset + 0x5d

*/

static ADDRESS_MAP_START( DSP_map_program, AS_PROGRAM, 16, taitoair_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( DSP_map_data, AS_DATA, 16, taitoair_state )
	AM_RANGE(0x2003, 0x2003) AM_READNOP //bit 0 DMA status flag or vblank
	AM_RANGE(0x3000, 0x3002) AM_WRITE(dsp_flags_w)
	AM_RANGE(0x3404, 0x3404) AM_WRITE(dsp_frustum_left_w)
	AM_RANGE(0x3405, 0x3405) AM_WRITE(dsp_x_eyecoord_w)
	AM_RANGE(0x3406, 0x3406) AM_WRITE(dsp_z_eyecoord_w)
	AM_RANGE(0x3407, 0x3407) AM_READ(dsp_x_return_r)
	AM_RANGE(0x3408, 0x3408) AM_WRITE(dsp_frustum_bottom_w)
	AM_RANGE(0x3409, 0x3409) AM_WRITE(dsp_y_eyecoord_w)
	AM_RANGE(0x340a, 0x340a) AM_WRITE(dsp_rasterize_w)      /* Just a (lame) guess */
	AM_RANGE(0x340b, 0x340b) AM_READ(dsp_y_return_r)
//  AM_RANGE(0x3418, 0x341a) AM_WRITE(dsp_sqrt_w)
//  AM_RANGE(0x341b, 0x341b) AM_WRITE(dsp_sqrt_r)
//  AM_RANGE(0x341c, 0x341c) AM_READ(dsp_sqrt_flags1_r)
//  AM_RANGE(0x341d, 0x341d) AM_READ(dsp_sqrt_flags2_r)
	AM_RANGE(0x4000, 0x7fff) AM_READWRITE(lineram_r, lineram_w)
	AM_RANGE(0x8000, 0xffff) AM_READWRITE(dspram_r, dspram_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( DSP_map_io, AS_IO, 16, taitoair_state )
	AM_RANGE(TMS32025_HOLD, TMS32025_HOLD) AM_READ(dsp_HOLD_signal_r)
	AM_RANGE(TMS32025_HOLDA, TMS32025_HOLDA) AM_WRITE(dsp_HOLDA_signal_w)
ADDRESS_MAP_END


/************************************************************
               INPUT PORTS & DIPS
************************************************************/

static INPUT_PORTS_START( topland )
	/* 0xa00200 -> 0x0c0d7c (-$7285,A5) */
	PORT_START("DSWA")
	PORT_DIPUNUSED_DIPLOC( 0x01, IP_ACTIVE_LOW, "SWA:1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, IP_ACTIVE_LOW, "SWA:2" )
	TAITO_DSWA_BITS_2_TO_3_LOC(SWA)
	TAITO_COINAGE_WORLD_LOC(SWA)

	/* 0xa00202 -> 0x0c0d7e (-$7283,A5) */
	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SWB)
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_LOW, "SWB:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_LOW, "SWB:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, IP_ACTIVE_LOW, "SWB:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_LOW, "SWB:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SWB:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, IP_ACTIVE_LOW, "SWB:8" )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_TILT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_BUTTON3 ) PORT_PLAYER(1)    /* "door" (!) */

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(1)    /* slot down */
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(1)    /* slot up */
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)    /* handle */
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) // DMA status flag
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW,  IPT_UNUSED )

	PORT_START(STICK1_PORT_TAG)
	PORT_BIT( 0x00ff, 0x0000, IPT_AD_STICK_Z ) PORT_MINMAX(0x0080,0x007f) PORT_SENSITIVITY(30) PORT_KEYDELTA(40) PORT_PLAYER(1) PORT_REVERSE

	PORT_START(STICK2_PORT_TAG)
	PORT_BIT( 0xffff, 0x0000, IPT_AD_STICK_X ) PORT_MINMAX(0xf800,0x07ff) PORT_SENSITIVITY(30) PORT_KEYDELTA(40) PORT_PLAYER(1)

	PORT_START(STICK3_PORT_TAG)
	PORT_BIT( 0xffff, 0x0000, IPT_AD_STICK_Y ) PORT_MINMAX(0xf800,0x07ff) PORT_SENSITIVITY(30) PORT_KEYDELTA(40) PORT_PLAYER(1)
INPUT_PORTS_END

static INPUT_PORTS_START( ainferno )
	/* 0xa00200 -> 0x0c0003.b (-$7ffd,A5) */
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, "Moving Control" )        PORT_DIPLOCATION("SWA:1")
	PORT_DIPSETTING(    0x01, "Upright/Cockpit" )
	PORT_DIPSETTING(    0x00, "DX Moving Only" )
	PORT_DIPNAME( 0x02, 0x02, "Motion Test Mode" )      PORT_DIPLOCATION("SWA:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	TAITO_DSWA_BITS_2_TO_3_LOC(SWA)
	TAITO_COINAGE_US_LOC(SWA)

	/* 0xa00202 -> 0x0c0004.b (-$7ffc,A5) */
	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SWB)
	PORT_DIPNAME( 0x0c, 0x0c, "Timer Length" )      PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x10, 0x10, "Rudder Pedal" )      PORT_DIPLOCATION("SWB:5")
	PORT_DIPSETTING(    0x10, "Without (Upright)" )
	PORT_DIPSETTING(    0x00, "With (Cockpit / DX)" )
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_LOW, "SWB:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SWB:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, IP_ACTIVE_LOW, "SWB:8" )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_TILT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_START2 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(1)    /* lever */
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(1)    /* handle x */
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_BUTTON3 ) PORT_PLAYER(1)    /* handle y */
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_BUTTON4 ) PORT_PLAYER(1)    /* fire */
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_BUTTON6 ) PORT_PLAYER(1)    /* pedal r */
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_BUTTON5 ) PORT_PLAYER(1)    /* pedal l */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) // DMA status flag
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW,  IPT_UNUSED )

	PORT_START(STICK1_PORT_TAG)
	PORT_BIT( 0x00ff, 0x0000, IPT_AD_STICK_Z ) PORT_MINMAX(0x0080,0x007f) PORT_SENSITIVITY(30) PORT_KEYDELTA(40) PORT_PLAYER(1) PORT_REVERSE

	PORT_START(STICK2_PORT_TAG)
	PORT_BIT( 0xffff, 0x0000, IPT_AD_STICK_X ) PORT_MINMAX(0xf800,0x7ff) PORT_SENSITIVITY(30) PORT_KEYDELTA(40) PORT_PLAYER(1)

	PORT_START(STICK3_PORT_TAG)
	PORT_BIT( 0xffff, 0x0000, IPT_AD_STICK_Y ) PORT_MINMAX(0xf800,0x7ff) PORT_SENSITIVITY(30) PORT_KEYDELTA(40) PORT_PLAYER(1)
INPUT_PORTS_END


/************************************************************
                GFX DECODING
************************************************************/

static const gfx_layout tilelayout =
{
	16,16,  /* 16x16 pixels */
	RGN_FRAC(1,4),
	4,
	{ 0, 1, 2, 3 },
	{ 4, 0, 12, 8,
		RGN_FRAC(1,4)+4, RGN_FRAC(1,4), RGN_FRAC(1,4)+12, RGN_FRAC(1,4)+8,
		RGN_FRAC(2,4)+4, RGN_FRAC(2,4), RGN_FRAC(2,4)+12, RGN_FRAC(2,4)+8,
		RGN_FRAC(3,4)+4, RGN_FRAC(3,4), RGN_FRAC(3,4)+12, RGN_FRAC(3,4)+8 },
	{ 0*16, 1*16, 2*16,  3*16,  4*16,  5*16,  6*16,  7*16,
		8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	16*16
};

static GFXDECODE_START( airsys )
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout, 0, 32*16 )
GFXDECODE_END


/************************************************************
                YM2610 (SOUND)
************************************************************/

/* Handler called by the YM2610 emulator when the internal timers cause an IRQ */
WRITE_LINE_MEMBER(taitoair_state::irqhandler)
{
	m_audiocpu->set_input_line(0, state ? ASSERT_LINE : CLEAR_LINE);
}


/************************************************************
                MACHINE DRIVERS
************************************************************/

void taitoair_state::machine_start()
{
	membank("z80bank")->configure_entries(0, 4, memregion("audiocpu")->base(), 0x4000);

	save_item(NAME(m_q.col));
	save_item(NAME(m_q.pcount));

	for (int i = 0; i < TAITOAIR_POLY_MAX_PT; i++)
	{
		save_item(NAME(m_q.p[i].x), i);
		save_item(NAME(m_q.p[i].y), i);
	}
}

void taitoair_state::machine_reset()
{
	int i;

	m_dsp_hold_signal = ASSERT_LINE;

	for (i = 0; i < TAITOAIR_POLY_MAX_PT; i++)
	{
		m_q.p[i].x = 0;
		m_q.p[i].y = 0;
	}
}

static MACHINE_CONFIG_START( airsys, taitoair_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_12MHz) // MC68000P12
	MCFG_CPU_PROGRAM_MAP(airsys_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", taitoair_state,  irq5_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_16MHz / 4)   // Z8400AB1
	MCFG_CPU_PROGRAM_MAP(sound_map)

	MCFG_CPU_ADD("dsp", TMS32025, XTAL_36MHz) // Unverified
	MCFG_CPU_PROGRAM_MAP(DSP_map_program)
	MCFG_CPU_DATA_MAP(DSP_map_data)
	MCFG_CPU_IO_MAP(DSP_map_io)

	MCFG_QUANTUM_PERFECT_CPU("maincpu")

	MCFG_DEVICE_ADD("tc0220ioc", TC0220IOC, 0)
	MCFG_TC0220IOC_READ_0_CB(IOPORT("DSWA"))
	MCFG_TC0220IOC_READ_1_CB(IOPORT("DSWB"))
	MCFG_TC0220IOC_READ_2_CB(IOPORT("IN0"))
	MCFG_TC0220IOC_READ_3_CB(IOPORT("IN1"))
	MCFG_TC0220IOC_READ_7_CB(IOPORT("IN2"))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*16, 64*16)
	MCFG_SCREEN_VISIBLE_AREA(0*16, 32*16-1, 3*16, 28*16-1)
	MCFG_SCREEN_UPDATE_DRIVER(taitoair_state, screen_update_taitoair)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", airsys)

	MCFG_PALETTE_ADD_INIT_BLACK("palette", 512*16+512*16)

	MCFG_DEVICE_ADD("tc0080vco", TC0080VCO, 0)
	MCFG_TC0080VCO_GFX_REGION(0)
	MCFG_TC0080VCO_TX_REGION(1)
	MCFG_TC0080VCO_OFFSETS(1, 1)
	MCFG_TC0080VCO_BGFLIP_OFFS(-2)
	MCFG_TC0080VCO_GFXDECODE("gfxdecode")
	MCFG_TC0080VCO_PALETTE("palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2610, XTAL_16MHz / 2)
	MCFG_YM2610_IRQ_HANDLER(WRITELINE(taitoair_state, irqhandler))
	MCFG_SOUND_ROUTE(0, "mono", 0.30)
	MCFG_SOUND_ROUTE(1, "mono", 0.60)
	MCFG_SOUND_ROUTE(2, "mono", 0.60)

	MCFG_DEVICE_ADD("tc0140syt", TC0140SYT, 0)
	MCFG_TC0140SYT_MASTER_CPU("maincpu")
	MCFG_TC0140SYT_SLAVE_CPU("audiocpu")
MACHINE_CONFIG_END


/*************************************************************
                   DRIVERS

Both games use near-identical CPU boards but different video
boards. Top Landing has a video board ROM (b62-28.22) which is
not present on Air Inferno.

Air Inferno video customs:

TC0460LRN - 3D related?
TC0440ENZ - 3D related?

[Used also by F2/H/Z System games]
TC0430GRW - Rotation/Zoom
TC0300FLA
TC0080VCO
TC0130LNB
TC0130LNB
TC0160ROM
TC0270MOD

*************************************************************/

ROM_START( topland )
	ROM_REGION( 0xc0000, "maincpu", 0 ) /* 68000 */
	ROM_LOAD16_BYTE( "b62_41.43",  0x00000, 0x20000, CRC(28264798) SHA1(72e4441ad468f37cff69c36699867119ad28274c) )
	ROM_LOAD16_BYTE( "b62_40.14",  0x00001, 0x20000, CRC(db872f7d) SHA1(6932c62d8051b1811c30139dbd0375115305c731) )
	ROM_LOAD16_BYTE( "b62_25.42",  0x40000, 0x20000, CRC(1bd53a72) SHA1(ada679198739cd6a419d3fa4311bb92dc385099c) )
	ROM_LOAD16_BYTE( "b62_24.13",  0x40001, 0x20000, CRC(845026c5) SHA1(ab8d8f5f6597bfcde4e9ccf9e0181b8b6e769ada) )
	ROM_LOAD16_BYTE( "b62_23.41",  0x80000, 0x20000, CRC(ef3a971c) SHA1(0840668dda48f4c9a85410361bfba3ae9580a71f) )
	ROM_LOAD16_BYTE( "b62_22.12",  0x80001, 0x20000, CRC(94279201) SHA1(8518d8e722d4f2516f75224d9a21ab20d8ee6c78) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Z80 */
	ROM_LOAD( "b62-42.34", 0x00000, 0x10000, CRC(389230e0) SHA1(3a336987aad7bf4df658f924de4bbe6f0fff6d59) )

	ROM_REGION( 0x20000, "dsp", 0 ) /* TMS320C25 */
	ROM_LOAD16_BYTE( "b62-21.35", 0x00000, 0x02000, CRC(5f38460d) SHA1(0593718d15b30b10f7686959932e2c934de2a529) )  // cpu board
	ROM_LOAD16_BYTE( "b62-20.6",  0x00001, 0x02000, CRC(a4afe958) SHA1(7593a327f4ea0cc9e28fd3269278871f62fb0598) )  // cpu board

	ROM_REGION( 0x100000, "gfx1", 0 )   /* 16x16 tiles */
	ROM_LOAD16_BYTE( "b62-33.39",  0x000000, 0x20000, CRC(38786867) SHA1(7292e3fa69cad6494f2e8e7efa9c3f989bdf958d) )
	ROM_LOAD16_BYTE( "b62-36.48",  0x000001, 0x20000, CRC(4259e76a) SHA1(eb0dc5d0a6f875e3b8335fb30d4c2ad3880c31b9) )
	ROM_LOAD16_BYTE( "b62-29.27",  0x040000, 0x20000, CRC(efdd5c51) SHA1(6df3e9782946cf6f4a21ee3d335548c53cd21e3a) )
	ROM_LOAD16_BYTE( "b62-34.40",  0x040001, 0x20000, CRC(a7e10ca4) SHA1(862c23c095f96f9e0cae00d70947782d5f4e45e6) )
	ROM_LOAD16_BYTE( "b62-35.47",  0x080000, 0x20000, CRC(cba7bac5) SHA1(5305c84abcbcc23281744454803b849853b26632) )
	ROM_LOAD16_BYTE( "b62-30.28",  0x080001, 0x20000, CRC(30e37cb8) SHA1(6bc777bdf1a56952dbfbe2f595279a43e2fa98fd) )
	ROM_LOAD16_BYTE( "b62-31.29",  0x0c0000, 0x20000, CRC(3feebfe3) SHA1(5b014d7d6fa1daf400ac1a437f551281debfdba6) )
	ROM_LOAD16_BYTE( "b62-32.30",  0x0c0001, 0x20000, CRC(66806646) SHA1(d8e0c37b5227d8583d523164ffc6828b4508d5a3) )

	ROM_REGION( 0xa0000, "ymsnd", 0 )   /* ADPCM samples */
	ROM_LOAD( "b62-17.5",  0x00000, 0x20000, CRC(36447066) SHA1(91c8cc4e99534b2d533895a342abb22766a20090) )
	ROM_LOAD( "b62-16.4",  0x20000, 0x20000, CRC(203a5c27) SHA1(f6fc9322dea8d82bfec3be3fdc8616dc6adf666e) )
	ROM_LOAD( "b62-15.3",  0x40000, 0x20000, CRC(e35ffe81) SHA1(f35afdd7cfd4c09907fb062beb5ae46c2286a381) )
	ROM_LOAD( "b62-14.2",  0x60000, 0x20000, CRC(617948a3) SHA1(4660570fa6263c28cfae7ccdf154763cc6144896) )
	ROM_LOAD( "b62-13.1",  0x80000, 0x20000, CRC(b37dc3ea) SHA1(198d4f828132316c624da998e49b1873b9886bf0) )

	ROM_REGION( 0x20000, "ymsnd.deltat", 0 )    /* Delta-T samples */
	ROM_LOAD( "b62-18.31", 0x00000, 0x20000, CRC(3a4e687a) SHA1(43f07fe19dec351e851defdf9c7810fb9df04736) )

	ROM_REGION( 0x02000, "user1", 0 )   /* unknown */
	ROM_LOAD( "b62-28.22", 0x00000, 0x02000, CRC(c4be68a6) SHA1(2c07a0e71d11bca67427331217c507d849500ec1) ) // video board

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "pal20l8b-b62-02.bin", 0x0000, 0x0144, CRC(c43ab9d8) SHA1(38542b10e9206a25669534ee26a0472e5f2d6257) )
	ROM_LOAD( "pal20l8b-b62-03.bin", 0x0200, 0x0144, CRC(904753fa) SHA1(87f7414c3eab5740b188276b06c5b898ed07c1cd) )
	ROM_LOAD( "pal20l8b-b62-04.bin", 0x0400, 0x0144, CRC(80512abc) SHA1(0e87e59df3c4d3b4adba295dbd5a2c27b9d5fefd) )
	ROM_LOAD( "pal16l8a-b62-10.bin", 0x0600, 0x0104, CRC(6c1e3fc4) SHA1(8953d82ed94741fdfacb0465415915ca398678d4) )
ROM_END

ROM_START( ainferno )
	ROM_REGION( 0xc0000, "maincpu", 0 ) /* 68000 */
	ROM_LOAD16_BYTE( "c45_22.43", 0x00000, 0x20000, CRC(50300926) SHA1(9c2a60282d3f9f115b94cb5b6d64bbfc9d726d1d) )
	ROM_LOAD16_BYTE( "c45_20.14", 0x00001, 0x20000, CRC(39b189d9) SHA1(002013c02b546d3f5a9f3a3149971975a73cc8ce) )
	ROM_LOAD16_BYTE( "c45_21.42", 0x40000, 0x20000, CRC(1b687241) SHA1(309e42f79cbd48ceae58a15afb648aef838822f0) )
	ROM_LOAD16_BYTE( "c45_28.13", 0x40001, 0x20000, CRC(c7cd2567) SHA1(cf1f163ec252e9986132095f22bca8d061bfdf9a) )

	/* 0x80000 to 0xbffff is empty for this game */

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Z80 */
	ROM_LOAD( "c45-23.34", 0x00000, 0x10000, CRC(d0750c78) SHA1(63232c2acef86e8c8ffaad36ab0b6c4cc1eb48f8) )

	ROM_REGION( 0x20000, "dsp", 0 ) /* TMS320C25 */
	ROM_LOAD16_BYTE( "c45-25.35", 0x00000, 0x02000, CRC(c0d39f95) SHA1(542aa6e2af510aea00db40bf803cb6653d4e7747) )
	ROM_LOAD16_BYTE( "c45-24.6",  0x00001, 0x02000, CRC(1013d937) SHA1(817769d21583f5281ba044ce8c134c9239d1e83e) )

	ROM_REGION( 0x100000, "gfx1", 0 )   /* 16x16 tiles */
	ROM_LOAD16_BYTE( "c45-11.28", 0x000000, 0x20000, CRC(d9b4b77c) SHA1(69d570efa8146fb0a712ff45e77bda6fd85769f8) )
	ROM_LOAD16_BYTE( "c45-15.40", 0x000001, 0x20000, CRC(d4610698) SHA1(5de519a23300d5b3b09ce7cf8c02a1a6b2fb985c) )
	ROM_LOAD16_BYTE( "c45-12.29", 0x040000, 0x20000, CRC(4ae305b8) SHA1(2bbb981853a7abbba90afb8eb58f6869357551d3) )
	ROM_LOAD16_BYTE( "c45-16.41", 0x040001, 0x20000, CRC(c6eb93b0) SHA1(d0b1adfce5c1f4e21c5d84527d22ace14578f2d7) )
	ROM_LOAD16_BYTE( "c45-13.30", 0x080000, 0x20000, CRC(69b82af6) SHA1(13c035e84affa59734c6dd1b07963c08654b5f5a) )
	ROM_LOAD16_BYTE( "c45-17.42", 0x080001, 0x20000, CRC(0dbee000) SHA1(41073d5cf20df12d5ba1c424c9d9f0b2d9836d5d) )
	ROM_LOAD16_BYTE( "c45-14.31", 0x0c0000, 0x20000, CRC(481b6f29) SHA1(0b047e805663b144dc2388c86438950fcdc29658) )
	ROM_LOAD16_BYTE( "c45-18.43", 0x0c0001, 0x20000, CRC(ba7ecf3b) SHA1(dd073b7bfbf2f88432337027ae9fb6c4f02a538f) )

	ROM_REGION( 0xa0000, "ymsnd", 0 )   /* ADPCM samples */
	ROM_LOAD( "c45-01.5",  0x00000, 0x20000, CRC(052997b2) SHA1(3aa8b4f759a1c196de39754a9ccdf4fabdbab388) )
	ROM_LOAD( "c45-02.4",  0x20000, 0x20000, CRC(2fc0a88e) SHA1(6a635671fa2518f74015429ce580d7b7f00299ad) )
	ROM_LOAD( "c45-03.3",  0x40000, 0x20000, CRC(0e1e5b5f) SHA1(a53d5ba01825f825e31a014cb4808f59ef86f0c9) )
	ROM_LOAD( "c45-04.2",  0x60000, 0x20000, CRC(6d081044) SHA1(2d98bde55621762509dfc645d9ca5e267b1757ae) )
	ROM_LOAD( "c45-05.1",  0x80000, 0x20000, CRC(6c59a808) SHA1(6264bbe4d7ad3070c6441859eb704a42910a82f0) )

	ROM_REGION( 0x20000, "ymsnd.deltat", 0 )    /* Delta-T samples */
	ROM_LOAD( "c45-06.31", 0x00000, 0x20000, CRC(6a7976d4) SHA1(a465f9bb874b1eff08742b33cc3c364703b281ca) )

	ROM_REGION( 0x1c00, "plds", 0 )
	ROM_LOAD( "pal16l8b-c45-07.ic6",   0x0000, 0x0104, CRC(a139114f) SHA1(d21f0c02c34a59b2cea925a9a417d5c2db27a30e) )
	ROM_LOAD( "pal16l8b-c45-08.ic62",  0x0200, 0x0104, CRC(6f8ec860) SHA1(25161f6e5a5a76c35e697312567abe995b08b945) )
	ROM_LOAD( "pal16l8b-c45-09.ic63",  0x0400, 0x0104, CRC(6703d122) SHA1(8636ee19cf850461e95318b2b82ace036d92225d) )
	ROM_LOAD( "pal20l8b-c45-10.ic2",   0x0600, 0x0144, CRC(c41c2a1b) SHA1(33ef3449bea145d6b6a5b7067587ea91795f8383) )
	ROM_LOAD( "pal16l8b-c45-26.ic76",  0x0800, 0x0104, CRC(23b59efc) SHA1(20965dcf73d4f98f38788b01891b64a756bd823c) )
	ROM_LOAD( "pal16l8b-c45-27.ic114", 0x0a00, 0x0104, CRC(2bdc4831) SHA1(dcf4845e7f793a4233af6131638267fea0d864b9) )
	ROM_LOAD( "pal20l8b-b62-02.ic60",  0x0c00, 0x0144, CRC(c43ab9d8) SHA1(38542b10e9206a25669534ee26a0472e5f2d6257) )
	ROM_LOAD( "pal20l8b-b62-03.ic62",  0x0e00, 0x0144, CRC(904753fa) SHA1(87f7414c3eab5740b188276b06c5b898ed07c1cd) )
	ROM_LOAD( "pal20l8b-b62-04.ic63",  0x1000, 0x0144, CRC(80512abc) SHA1(0e87e59df3c4d3b4adba295dbd5a2c27b9d5fefd) )
	ROM_LOAD( "pal16l8b-b62-07.ic82",  0x1200, 0x0104, CRC(dc524371) SHA1(d7529d812fc37043ad302380adc8bb6172fb837c) )
	ROM_LOAD( "pal16l8b-b62-08.ic23",  0x1400, 0x0104, CRC(b2d7ec83) SHA1(4e2e4af4b77ab1482520cc9644fec009beff014c) )
	ROM_LOAD( "pal16l8b-b62-11.ic26",  0x1600, 0x0104, CRC(44e9a034) SHA1(9d7e916baf797994469ddbcfe21fa0fff24b5acb) )
	ROM_LOAD( "pal16l8b-b62-12.ic27",  0x1800, 0x0104, CRC(f1182381) SHA1(3a76064f2bf322324575bbc111f93c9990da8ac1) )
ROM_END

ROM_START( ainfernoj )
	ROM_REGION( 0xc0000, "maincpu", 0 ) /* 68000 */
	ROM_LOAD16_BYTE( "c45_22.43", 0x00000, 0x20000, CRC(50300926) SHA1(9c2a60282d3f9f115b94cb5b6d64bbfc9d726d1d) )
	ROM_LOAD16_BYTE( "c45_20.14", 0x00001, 0x20000, CRC(39b189d9) SHA1(002013c02b546d3f5a9f3a3149971975a73cc8ce) )
	ROM_LOAD16_BYTE( "c45_21.42", 0x40000, 0x20000, CRC(1b687241) SHA1(309e42f79cbd48ceae58a15afb648aef838822f0) )
	ROM_LOAD16_BYTE( "c45_19.ic13",0x40001, 0x20000, CRC(5ec474dd) SHA1(7b436ba60628a410a5053095dafaee0bd7932daf) )

	/* 0x80000 to 0xbffff is empty for this game */

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Z80 */
	ROM_LOAD( "c45-23.34", 0x00000, 0x10000, CRC(d0750c78) SHA1(63232c2acef86e8c8ffaad36ab0b6c4cc1eb48f8) )

	ROM_REGION( 0x20000, "dsp", 0 ) /* TMS320C25 */
	ROM_LOAD16_BYTE( "c45-25.35", 0x00000, 0x02000, CRC(c0d39f95) SHA1(542aa6e2af510aea00db40bf803cb6653d4e7747) )
	ROM_LOAD16_BYTE( "c45-24.6",  0x00001, 0x02000, CRC(1013d937) SHA1(817769d21583f5281ba044ce8c134c9239d1e83e) )

	ROM_REGION( 0x100000, "gfx1", 0 )   /* 16x16 tiles */
	ROM_LOAD16_BYTE( "c45-11.28", 0x000000, 0x20000, CRC(d9b4b77c) SHA1(69d570efa8146fb0a712ff45e77bda6fd85769f8) )
	ROM_LOAD16_BYTE( "c45-15.40", 0x000001, 0x20000, CRC(d4610698) SHA1(5de519a23300d5b3b09ce7cf8c02a1a6b2fb985c) )
	ROM_LOAD16_BYTE( "c45-12.29", 0x040000, 0x20000, CRC(4ae305b8) SHA1(2bbb981853a7abbba90afb8eb58f6869357551d3) )
	ROM_LOAD16_BYTE( "c45-16.41", 0x040001, 0x20000, CRC(c6eb93b0) SHA1(d0b1adfce5c1f4e21c5d84527d22ace14578f2d7) )
	ROM_LOAD16_BYTE( "c45-13.30", 0x080000, 0x20000, CRC(69b82af6) SHA1(13c035e84affa59734c6dd1b07963c08654b5f5a) )
	ROM_LOAD16_BYTE( "c45-17.42", 0x080001, 0x20000, CRC(0dbee000) SHA1(41073d5cf20df12d5ba1c424c9d9f0b2d9836d5d) )
	ROM_LOAD16_BYTE( "c45-14.31", 0x0c0000, 0x20000, CRC(481b6f29) SHA1(0b047e805663b144dc2388c86438950fcdc29658) )
	ROM_LOAD16_BYTE( "c45-18.43", 0x0c0001, 0x20000, CRC(ba7ecf3b) SHA1(dd073b7bfbf2f88432337027ae9fb6c4f02a538f) )

	ROM_REGION( 0xa0000, "ymsnd", 0 )   /* ADPCM samples */
	ROM_LOAD( "c45-01.5",  0x00000, 0x20000, CRC(052997b2) SHA1(3aa8b4f759a1c196de39754a9ccdf4fabdbab388) )
	ROM_LOAD( "c45-02.4",  0x20000, 0x20000, CRC(2fc0a88e) SHA1(6a635671fa2518f74015429ce580d7b7f00299ad) )
	ROM_LOAD( "c45-03.3",  0x40000, 0x20000, CRC(0e1e5b5f) SHA1(a53d5ba01825f825e31a014cb4808f59ef86f0c9) )
	ROM_LOAD( "c45-04.2",  0x60000, 0x20000, CRC(6d081044) SHA1(2d98bde55621762509dfc645d9ca5e267b1757ae) )
	ROM_LOAD( "c45-05.1",  0x80000, 0x20000, CRC(6c59a808) SHA1(6264bbe4d7ad3070c6441859eb704a42910a82f0) )

	ROM_REGION( 0x20000, "ymsnd.deltat", 0 )    /* Delta-T samples */
	ROM_LOAD( "c45-06.31", 0x00000, 0x20000, CRC(6a7976d4) SHA1(a465f9bb874b1eff08742b33cc3c364703b281ca) )

	ROM_REGION( 0x1c00, "plds", 0 )
	ROM_LOAD( "pal16l8b-c45-07.ic6",   0x0000, 0x0104, CRC(a139114f) SHA1(d21f0c02c34a59b2cea925a9a417d5c2db27a30e) )
	ROM_LOAD( "pal16l8b-c45-08.ic62",  0x0200, 0x0104, CRC(6f8ec860) SHA1(25161f6e5a5a76c35e697312567abe995b08b945) )
	ROM_LOAD( "pal16l8b-c45-09.ic63",  0x0400, 0x0104, CRC(6703d122) SHA1(8636ee19cf850461e95318b2b82ace036d92225d) )
	ROM_LOAD( "pal20l8b-c45-10.ic2",   0x0600, 0x0144, CRC(c41c2a1b) SHA1(33ef3449bea145d6b6a5b7067587ea91795f8383) )
	ROM_LOAD( "pal16l8b-c45-26.ic76",  0x0800, 0x0104, CRC(23b59efc) SHA1(20965dcf73d4f98f38788b01891b64a756bd823c) )
	ROM_LOAD( "pal16l8b-c45-27.ic114", 0x0a00, 0x0104, CRC(2bdc4831) SHA1(dcf4845e7f793a4233af6131638267fea0d864b9) )
	ROM_LOAD( "pal20l8b-b62-02.ic60",  0x0c00, 0x0144, CRC(c43ab9d8) SHA1(38542b10e9206a25669534ee26a0472e5f2d6257) )
	ROM_LOAD( "pal20l8b-b62-03.ic62",  0x0e00, 0x0144, CRC(904753fa) SHA1(87f7414c3eab5740b188276b06c5b898ed07c1cd) )
	ROM_LOAD( "pal20l8b-b62-04.ic63",  0x1000, 0x0144, CRC(80512abc) SHA1(0e87e59df3c4d3b4adba295dbd5a2c27b9d5fefd) )
	ROM_LOAD( "pal16l8b-b62-07.ic82",  0x1200, 0x0104, CRC(dc524371) SHA1(d7529d812fc37043ad302380adc8bb6172fb837c) )
	ROM_LOAD( "pal16l8b-b62-08.ic23",  0x1400, 0x0104, CRC(b2d7ec83) SHA1(4e2e4af4b77ab1482520cc9644fec009beff014c) )
	ROM_LOAD( "pal16l8b-b62-11.ic26",  0x1600, 0x0104, CRC(44e9a034) SHA1(9d7e916baf797994469ddbcfe21fa0fff24b5acb) )
	ROM_LOAD( "pal16l8b-b62-12.ic27",  0x1800, 0x0104, CRC(f1182381) SHA1(3a76064f2bf322324575bbc111f93c9990da8ac1) )
ROM_END


/*   ( YEAR  NAME      PARENT    MACHINE   INPUT     INIT      MONITOR  COMPANY  FULLNAME */
GAME( 1988, topland,  0,        airsys,   topland, driver_device,  0,        ROT0,    "Taito Corporation Japan", "Top Landing (World)", GAME_NOT_WORKING )
GAME( 1990, ainferno, 0,        airsys,   ainferno, driver_device, 0,        ROT0,    "Taito America Corporation", "Air Inferno (US)", GAME_NOT_WORKING )
GAME( 1990, ainfernoj,ainferno, airsys,   ainferno, driver_device, 0,        ROT0,    "Taito Corporation Japan", "Air Inferno (Japan)", GAME_NOT_WORKING )
