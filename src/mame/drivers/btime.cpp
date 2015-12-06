// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari, Couriersud
/***************************************************************************

Burger Time

driver by Zsolt Vasvari

hardware description:

Actually Lock'n'Chase is (C)1981 while Burger Time is (C)1982, so it might
be more accurate to say 'Lock'n'Chase hardware'.

The bootleg called Cook Race runs on hardware similar but different. The fact
that it addresses the program ROMs in the range 0500-3fff instead of the usual
c000-ffff makes me suspect that it is a bootleg of the *tape system* version.
Little is known about that system, but it is quite likely that it would have
RAM in the range 0000-3fff and load the program there from tape.


This hardware is pretty straightforward, but has a couple of interesting
twists. There are two ports to the video and color RAMs, one normal access,
and one with X and Y coordinates swapped. The sprite RAM occupies the
first row of the swapped area, so it appears in the regular video RAM as
the first column of on the left side.

These games don't have VBLANK interrupts, but instead an IRQ or NMI
(depending on the particular board) is generated when a coin is inserted.

Some of the games also have a background playfield which, in the
case of Bump 'n' Jump and Zoar, can be scrolled vertically.

These boards use two 8910's for sound, controlled by a dedicated 6502. The
main processor triggers an IRQ request when writing a command to the sound
CPU.


PCB Layouts
-----------

Zoar
Data East, 1982

Top PCB

DE-0123
|---------------------------------|
|UPC1181H   SW2         Z17.15B   |
|   VOL     SW1                  |-|
|                       Z16.13B  | |
|          2128         Z15.12B  | |
|                         X      | |
|        AY-3-8910        X      |-|
|           AY-3-8910    DIP24    |
|                    6502         |
|1                      Z13.6B    |
|8                               |-|
|W                      Z12.4B   | |
|A                               | |
|Y                      Z11.3B   | |
|                                |-|
|  555                  Z10.1B    |
|---------------------------------|
Notes:
      6502      - clock 500.0kHz [12/24]
      AY-3-8910 - clock 3.00MHz(both) [12/4]
      2128      - 2k x8 SRAM == 6116
      X         - Position for a socket, but not populated with anything
      DIP24     - Empty socket. There are rumours that this socket would hold test mode code or something else.
                  It's possible a factory test ROM did exist for factory-only testing as this was common with
                  several manufacturers at the time. However the PCB came from the factory with this socket empty
                  so it would be extremely unlikely to find a PCB with that socket populated.
      SW1/2     - 8 position DIP switches
                  To set cocktail mode, set DIP#1 SW7 & 8 OFF. The player has 2 buttons only and the screen will flip between PL1 & PL2
                  To set upright mode, set DIP#1 SW7 & 8 ON. The player has 3 buttons and the screen will not flip between PL1 & PL2
                  DIP Notes:
                           SW1 #5 is unused
                           SW1 #6 must remain OFF otherwise the game will not boot-up and just displays garbage.
                           There is no TEST mode.
                           SW2 #5 is listed in the manual as "Panel B". This enables or removes the 2nd button.
                           There were two types of panels supplied, either cocktail or upright cabs. The cocktail panel
                           doesn't have 3 buttons. The 2 buttons are air-air missile and air-ground missile/bomb and
                           there's an extra button for accelerate on the 3 button panel. On the 2 button panel, button 1 is
                           the air-air and air-ground missile/bomb weapon button and is auto selected based on the enemies on
                           screen and the 2nd button is wired to accelerate. There is no button for manually selecting
                           the missiles/bombs.

Bottom PCB

DE-0122
|---------------------------------|
|    Z08.15L     PB3  PB0   2128  |
|    Z07.14L     2128  |-------| |-|
|                2128  |       | | |
|    Z06.12L           | CPU-7 | | |
|                      |       | | |
|    Z05.11L           |       | |-|
|                      |-------|  |
|    Z04.9L                       |
|    Z03.8L               Z19.7B  |
|                    AM93425     |-|
|1   Z02.6L  AM93425 AM93425     | |
|0   Z01.5L  AM93425 AM93425 PB2 | |
|W   Z00.3L  AM93425  PB4   PB1  | |
|A   Z21.2L    PB4           555 |-|
|Y   Z20.1L                12MHz  |
|---------------------------------|
Notes:
      CPU-7     - Epoxy block containing a 6502 clocked at 1.5MHz [12/8]
                  and possibly something else ^_^
      2128      - 2k x8 SRAM == 6116
      AM93425   - 1k x1 SRAM == 2125
      PB*       - PALs (not dumped, registered types)
      Z19/20/21 - PROMs, type Harris 7603 (32 bytes)
      VSync     - 57.4358Hz
      HSync     - 15.6235kHz


Note on Lock'n'Chase:

The watchdog test prints "WATCHDOG TEST ER". Just by looking at the code,
I can't see how it could print anything else, there is only one path it
can take. Should the game reset????


2008-07:
Verified dip locations for: btime (manual), bnj (dips listing), lnc (manual),
zoar (manual), disco (dips listing). Names of disco switches in DIPLOC are
not confirmed (manual needed, in the meanwhile I put generic SW1 & SW2).

A few notes:
* all the documents says that DSW1 bit 7 is related to the cocktail mode
    (either flipping the screen or changing the control panel)
* according to manuals, btime & bnj Service dips should have a different
    effect, using 2 bits to access different tests (see commented out
    settings below)
* how do country codes affect disco? are there other values other than
    the ones in the manual?

***************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "sound/ay8910.h"
#include "sound/discrete.h"
#include "includes/btime.h"
#include "machine/decocpu7.h"
#include "machine/deco222.h"

#define MASTER_CLOCK      XTAL_12MHz
#define HCLK             (MASTER_CLOCK/2)
#define HCLK1            (HCLK/2)
#define HCLK2            (HCLK1/2)
#define HCLK4            (HCLK2/2)

enum
{
	AUDIO_ENABLE_NONE,
	AUDIO_ENABLE_DIRECT,        /* via direct address in memory map */
	AUDIO_ENABLE_AY8910         /* via ay-8910 port A */
};


WRITE8_MEMBER(btime_state::audio_nmi_enable_w)
{
	/* for most games, this serves as the NMI enable for the audio CPU; however,
	   lnc and disco use bit 0 of the first AY-8910's port A instead; many other
	   games also write there in addition to this address */
	if (m_audio_nmi_enable_type == AUDIO_ENABLE_DIRECT)
	{
		m_audio_nmi_enabled = data & 1;
		m_audiocpu->set_input_line(INPUT_LINE_NMI, (m_audio_nmi_enabled && m_audio_nmi_state) ? ASSERT_LINE : CLEAR_LINE);
	}
}

WRITE8_MEMBER(btime_state::ay_audio_nmi_enable_w)
{
	/* port A bit 0, when 1, inhibits the NMI */
	if (m_audio_nmi_enable_type == AUDIO_ENABLE_AY8910)
	{
		m_audio_nmi_enabled = ~data & 1;
		m_audiocpu->set_input_line(INPUT_LINE_NMI, (m_audio_nmi_enabled && m_audio_nmi_state) ? ASSERT_LINE : CLEAR_LINE);
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(btime_state::audio_nmi_gen)
{
	int scanline = param;
	m_audio_nmi_state = scanline & 8;
	m_audiocpu->set_input_line(INPUT_LINE_NMI, (m_audio_nmi_enabled && m_audio_nmi_state) ? ASSERT_LINE : CLEAR_LINE);
}

static ADDRESS_MAP_START( btime_map, AS_PROGRAM, 8, btime_state )
	AM_RANGE(0x0000, 0x07ff) AM_RAM AM_SHARE("rambase")
	AM_RANGE(0x0c00, 0x0c0f) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x1000, 0x13ff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0x1400, 0x17ff) AM_RAM AM_SHARE("colorram")
	AM_RANGE(0x1800, 0x1bff) AM_READWRITE(btime_mirrorvideoram_r, btime_mirrorvideoram_w)
	AM_RANGE(0x1c00, 0x1fff) AM_READWRITE(btime_mirrorcolorram_r, btime_mirrorcolorram_w)
	AM_RANGE(0x4000, 0x4000) AM_READ_PORT("P1") AM_WRITENOP
	AM_RANGE(0x4001, 0x4001) AM_READ_PORT("P2")
	AM_RANGE(0x4002, 0x4002) AM_READ_PORT("SYSTEM") AM_WRITE(btime_video_control_w)
	AM_RANGE(0x4003, 0x4003) AM_READ_PORT("DSW1") AM_WRITE(audio_command_w)
	AM_RANGE(0x4004, 0x4004) AM_READ_PORT("DSW2") AM_WRITE(bnj_scroll1_w)
	AM_RANGE(0xb000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( cookrace_map, AS_PROGRAM, 8, btime_state )
	AM_RANGE(0x0000, 0x03ff) AM_RAM AM_SHARE("rambase")
	AM_RANGE(0x0500, 0x3fff) AM_ROM
	AM_RANGE(0xc000, 0xc3ff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0xc400, 0xc7ff) AM_RAM AM_SHARE("colorram")
	AM_RANGE(0xc800, 0xcbff) AM_READWRITE(btime_mirrorvideoram_r, btime_mirrorvideoram_w)
	AM_RANGE(0xcc00, 0xcfff) AM_READWRITE(btime_mirrorcolorram_r, btime_mirrorcolorram_w)
	AM_RANGE(0xd000, 0xd0ff) AM_RAM                         /* background? */
	AM_RANGE(0xd100, 0xd3ff) AM_RAM                         /* ? */
	AM_RANGE(0xd400, 0xd7ff) AM_RAM AM_SHARE("bnj_bgram")
	AM_RANGE(0xe000, 0xe000) AM_READ_PORT("DSW1") AM_WRITE(bnj_video_control_w)
	AM_RANGE(0xe300, 0xe300) AM_READ_PORT("DSW1")   /* mirror address used on high score name entry */
													/* screen */
	AM_RANGE(0xe001, 0xe001) AM_READ_PORT("DSW2") AM_WRITE(audio_command_w)
	AM_RANGE(0xe002, 0xe002) AM_READ_PORT("P1")
	AM_RANGE(0xe003, 0xe003) AM_READ_PORT("P2")
	AM_RANGE(0xe004, 0xe004) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xfff9, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( tisland_map, AS_PROGRAM, 8, btime_state )
	AM_RANGE(0x0000, 0x07ff) AM_RAM AM_SHARE("rambase")
	AM_RANGE(0x0c00, 0x0c0f) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x1000, 0x13ff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0x1400, 0x17ff) AM_RAM AM_SHARE("colorram")
	AM_RANGE(0x1800, 0x1bff) AM_READWRITE(btime_mirrorvideoram_r, btime_mirrorvideoram_w)
	AM_RANGE(0x1c00, 0x1fff) AM_READWRITE(btime_mirrorcolorram_r, btime_mirrorcolorram_w)
	AM_RANGE(0x4000, 0x4000) AM_READ_PORT("P1") AM_WRITENOP
	AM_RANGE(0x4001, 0x4001) AM_READ_PORT("P2")
	AM_RANGE(0x4002, 0x4002) AM_READ_PORT("SYSTEM") AM_WRITE(btime_video_control_w)
	AM_RANGE(0x4003, 0x4003) AM_READ_PORT("DSW1") AM_WRITE(audio_command_w)
	AM_RANGE(0x4004, 0x4004) AM_READ_PORT("DSW2") AM_WRITE(bnj_scroll1_w)
	AM_RANGE(0x4005, 0x4005) AM_WRITE(bnj_scroll2_w)
	AM_RANGE(0x9000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( zoar_map, AS_PROGRAM, 8, btime_state )
	AM_RANGE(0x0000, 0x07ff) AM_RAM AM_SHARE("rambase")
	AM_RANGE(0x8000, 0x83ff) AM_WRITEONLY AM_SHARE("videoram")
	AM_RANGE(0x8400, 0x87ff) AM_WRITEONLY AM_SHARE("colorram")
	AM_RANGE(0x8800, 0x8bff) AM_WRITE(btime_mirrorvideoram_w)
	AM_RANGE(0x8c00, 0x8fff) AM_WRITE(btime_mirrorcolorram_w)
	AM_RANGE(0x9000, 0x9000) AM_WRITE(zoar_video_control_w)
	AM_RANGE(0x9800, 0x9800) AM_READ(zoar_dsw1_read)
	AM_RANGE(0x9801, 0x9801) AM_READ_PORT("DSW2")
	AM_RANGE(0x9802, 0x9802) AM_READ_PORT("P1")
	AM_RANGE(0x9803, 0x9803) AM_READ_PORT("P2")
	AM_RANGE(0x9800, 0x9803) AM_WRITEONLY AM_SHARE("zoar_scrollram")
	AM_RANGE(0x9804, 0x9804) AM_READ_PORT("SYSTEM") AM_WRITE(bnj_scroll2_w)
	AM_RANGE(0x9805, 0x9805) AM_WRITE(bnj_scroll1_w)
	AM_RANGE(0x9806, 0x9806) AM_WRITE(audio_command_w)
	AM_RANGE(0xd000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( lnc_map, AS_PROGRAM, 8, btime_state )
	AM_RANGE(0x0000, 0x3bff) AM_RAM AM_SHARE("rambase")
	AM_RANGE(0x3c00, 0x3fff) AM_RAM_WRITE(lnc_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x7800, 0x7bff) AM_WRITEONLY AM_SHARE("colorram")  /* this is just here to initialize the pointer */
	AM_RANGE(0x7c00, 0x7fff) AM_READWRITE(btime_mirrorvideoram_r, lnc_mirrorvideoram_w)
	AM_RANGE(0x8000, 0x8000) AM_READ_PORT("DSW1") AM_WRITENOP     /* ??? */
	AM_RANGE(0x8001, 0x8001) AM_READ_PORT("DSW2") AM_WRITE(bnj_video_control_w)
	AM_RANGE(0x8003, 0x8003) AM_WRITEONLY AM_SHARE("lnc_charbank")
	AM_RANGE(0x9000, 0x9000) AM_READ_PORT("P1") AM_WRITENOP     /* IRQ ack??? */
	AM_RANGE(0x9001, 0x9001) AM_READ_PORT("P2")
	AM_RANGE(0x9002, 0x9002) AM_READ_PORT("SYSTEM") AM_WRITE(audio_command_w)
	AM_RANGE(0xb000, 0xb1ff) AM_RAM
	AM_RANGE(0xc000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( mmonkey_map, AS_PROGRAM, 8, btime_state )
	AM_RANGE(0x0000, 0x3bff) AM_RAM AM_SHARE("rambase")
	AM_RANGE(0x3c00, 0x3fff) AM_RAM_WRITE(lnc_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x7800, 0x7bff) AM_WRITEONLY AM_SHARE("colorram")      /* this is just here to initialize the pointer */
	AM_RANGE(0x7c00, 0x7fff) AM_READWRITE(btime_mirrorvideoram_r, lnc_mirrorvideoram_w)
	AM_RANGE(0x8000, 0x8000) AM_READ_PORT("DSW1")
	AM_RANGE(0x8001, 0x8001) AM_READ_PORT("DSW2") AM_WRITE(bnj_video_control_w)
	AM_RANGE(0x8003, 0x8003) AM_WRITEONLY AM_SHARE("lnc_charbank")
	AM_RANGE(0x9000, 0x9000) AM_READ_PORT("P1") AM_WRITENOP /* IRQ ack??? */
	AM_RANGE(0x9001, 0x9001) AM_READ_PORT("P2")
	AM_RANGE(0x9002, 0x9002) AM_READ_PORT("SYSTEM") AM_WRITE(audio_command_w)
	AM_RANGE(0xb000, 0xbfff) AM_READWRITE(mmonkey_protection_r, mmonkey_protection_w)
	AM_RANGE(0xc000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( bnj_map, AS_PROGRAM, 8, btime_state )
	AM_RANGE(0x0000, 0x07ff) AM_RAM AM_SHARE("rambase")
	AM_RANGE(0x1000, 0x1000) AM_READ_PORT("DSW1")
	AM_RANGE(0x1001, 0x1001) AM_READ_PORT("DSW2") AM_WRITE(bnj_video_control_w)
	AM_RANGE(0x1002, 0x1002) AM_READ_PORT("P1") AM_WRITE(audio_command_w)
	AM_RANGE(0x1003, 0x1003) AM_READ_PORT("P2")
	AM_RANGE(0x1004, 0x1004) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x4000, 0x43ff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0x4400, 0x47ff) AM_RAM AM_SHARE("colorram")
	AM_RANGE(0x4800, 0x4bff) AM_READWRITE(btime_mirrorvideoram_r, btime_mirrorvideoram_w)
	AM_RANGE(0x4c00, 0x4fff) AM_READWRITE(btime_mirrorcolorram_r, btime_mirrorcolorram_w)
	AM_RANGE(0x5000, 0x51ff) AM_RAM_WRITE(bnj_background_w) AM_SHARE("bnj_bgram")
	AM_RANGE(0x5200, 0x53ff) AM_RAM
	AM_RANGE(0x5400, 0x5400) AM_WRITE(bnj_scroll1_w)
	AM_RANGE(0x5800, 0x5800) AM_WRITE(bnj_scroll2_w)
	AM_RANGE(0x5c00, 0x5c0f) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0xa000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( disco_map, AS_PROGRAM, 8, btime_state )
	AM_RANGE(0x0000, 0x04ff) AM_RAM AM_SHARE("rambase")
	AM_RANGE(0x2000, 0x7fff) AM_RAM_WRITE(deco_charram_w) AM_SHARE("deco_charram")
	AM_RANGE(0x8000, 0x83ff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0x8400, 0x87ff) AM_RAM AM_SHARE("colorram")
	AM_RANGE(0x8800, 0x881f) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x9000, 0x9000) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x9200, 0x9200) AM_READ_PORT("P1")
	AM_RANGE(0x9400, 0x9400) AM_READ_PORT("P2")
	AM_RANGE(0x9800, 0x9800) AM_READ_PORT("DSW1")
	AM_RANGE(0x9a00, 0x9a00) AM_READ_PORT("DSW2") AM_WRITE(audio_command_w)
	AM_RANGE(0x9c00, 0x9c00) AM_READ_PORT("VBLANK") AM_WRITE(disco_video_control_w)
	AM_RANGE(0xa000, 0xffff) AM_ROM
ADDRESS_MAP_END



static ADDRESS_MAP_START( audio_map, AS_PROGRAM, 8, btime_state )
	AM_RANGE(0x0000, 0x03ff) AM_MIRROR(0x1c00) AM_RAM AM_SHARE("audio_rambase")
	AM_RANGE(0x2000, 0x3fff) AM_DEVWRITE("ay1", ay8910_device, data_w)
	AM_RANGE(0x4000, 0x5fff) AM_DEVWRITE("ay1", ay8910_device, address_w)
	AM_RANGE(0x6000, 0x7fff) AM_DEVWRITE("ay2", ay8910_device, data_w)
	AM_RANGE(0x8000, 0x9fff) AM_DEVWRITE("ay2", ay8910_device, address_w)
	AM_RANGE(0xa000, 0xbfff) AM_READ(audio_command_r)
	AM_RANGE(0xc000, 0xdfff) AM_WRITE(audio_nmi_enable_w)
	AM_RANGE(0xe000, 0xefff) AM_MIRROR(0x1000) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( disco_audio_map, AS_PROGRAM, 8, btime_state )
	AM_RANGE(0x0000, 0x03ff) AM_RAM
	AM_RANGE(0x4000, 0x4fff) AM_DEVWRITE("ay1", ay8910_device, data_w)
	AM_RANGE(0x5000, 0x5fff) AM_DEVWRITE("ay1", ay8910_device, address_w)
	AM_RANGE(0x6000, 0x6fff) AM_DEVWRITE("ay2", ay8910_device, data_w)
	AM_RANGE(0x7000, 0x7fff) AM_DEVWRITE("ay2", ay8910_device, address_w)
	AM_RANGE(0x8000, 0x8fff) AM_READ(soundlatch_byte_r) AM_WRITENOP /* ack ? */
	AM_RANGE(0xf000, 0xffff) AM_ROM
ADDRESS_MAP_END


INPUT_CHANGED_MEMBER(btime_state::coin_inserted_irq_hi)
{
	if (newval)
		m_maincpu->set_input_line(0, HOLD_LINE);
}

INPUT_CHANGED_MEMBER(btime_state::coin_inserted_irq_lo)
{
	if (!newval)
		m_maincpu->set_input_line(0, HOLD_LINE);
}

INPUT_CHANGED_MEMBER(btime_state::coin_inserted_nmi_lo)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, newval ? CLEAR_LINE : ASSERT_LINE);
}


WRITE8_MEMBER(btime_state::audio_command_w)
{
	soundlatch_byte_w(space, offset, data);
	m_audiocpu->set_input_line(0, ASSERT_LINE);
}

READ8_MEMBER(btime_state::audio_command_r)
{
	m_audiocpu->set_input_line(0, CLEAR_LINE);
	return soundlatch_byte_r(space, offset);
}

READ8_MEMBER(btime_state::zoar_dsw1_read)
{
	return (!m_screen->vblank() << 7) | (ioport("DSW1")->read() & 0x7f);
}

static INPUT_PORTS_START( btime )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, btime_state,coin_inserted_irq_hi, 0)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, btime_state,coin_inserted_irq_hi, 0)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("15D:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("15D:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	/* Manual gives the following Test settings, but they don't seem to correspond to the actual effect */
//  PORT_DIPNAME( 0x30, 0x30, "Test Mode" ) PORT_DIPLOCATION("15D:5,6")
//  PORT_DIPSETTING(    0x30, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x00, "Sound I/O Board Only" )
//  PORT_DIPSETTING(    0x10, "Normal Test" )
//  PORT_DIPSETTING(    0x20, "Cross Hatch Only" )
	PORT_SERVICE_DIPLOC( 0x10, IP_ACTIVE_LOW, "15D:5" )
	PORT_DIPNAME( 0x20, 0x20, "Cross Hatch Pattern" ) PORT_DIPLOCATION("15D:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Control Panel" ) PORT_DIPLOCATION("15D:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
//  PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("15D:8")
//  PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
//  PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM  ) PORT_VBLANK("screen")

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) ) PORT_DIPLOCATION("14D:1")
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x06, 0x02, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("14D:2,3")
	PORT_DIPSETTING(    0x06, "10000" )
	PORT_DIPSETTING(    0x04, "15000" )
	PORT_DIPSETTING(    0x02, "20000"  )
	PORT_DIPSETTING(    0x00, "30000"  )
	PORT_DIPNAME( 0x08, 0x08, "Enemies" ) PORT_DIPLOCATION("14D:4")
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x10, 0x00, "End of Level Pepper" ) PORT_DIPLOCATION("14D:5")
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "14D:6" )   /* it should be OFF according to the manual */
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "14D:7" )   /* it should be OFF according to the manual */
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "14D:8" )   /* it should be OFF according to the manual */
INPUT_PORTS_END

static INPUT_PORTS_START( cookrace )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, btime_state,coin_inserted_nmi_lo, 0)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, btime_state,coin_inserted_nmi_lo, 0)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM  ) PORT_VBLANK("screen")

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x06, "20000" )
	PORT_DIPSETTING(    0x04, "30000" )
	PORT_DIPSETTING(    0x02, "40000"  )
	PORT_DIPSETTING(    0x00, "50000"  )
	PORT_DIPNAME( 0x08, 0x08, "Enemies" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x10, 0x10, "End of Level Pepper" )
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x80, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
INPUT_PORTS_END

static INPUT_PORTS_START( zoar )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, btime_state,coin_inserted_irq_lo, 0)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, btime_state,coin_inserted_irq_lo, 0)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	/* Manual says bit 4,5 have to stay OFF */
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW1:5" )    /* almost certainly unused */
	/* Service mode doesn't work because of missing ROMs */
	PORT_SERVICE_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW1:6" )
	PORT_DIPNAME( 0x40, 0x00, "Control Panel" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
//  PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW1:8")
//  PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
//  PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )
	/* I can't use PORT_VBLANK as players would have almost no time to enter their initials */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(    0x06, "5000" )
	PORT_DIPSETTING(    0x04, "10000" )
	PORT_DIPSETTING(    0x02, "15000"  )
	PORT_DIPSETTING(    0x00, "20000"  )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x10, 0x00, "Weapon Select" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, "Manual" )
	PORT_DIPSETTING(    0x10, "Auto" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW2:6" )   /* These 3 switches have something to do with coinage  */
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW2:7" )   /* See code at $d234. Feel free to figure them out     */
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW2:8" )   /* Manual says to leave them OFF                       */
INPUT_PORTS_END

static INPUT_PORTS_START( lnc )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, btime_state,coin_inserted_nmi_lo, 0)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, btime_state,coin_inserted_nmi_lo, 0)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x30, 0x30, "Test Mode" ) PORT_DIPLOCATION("SW1:5,6") /* Manual says these bits are unused */
	PORT_DIPSETTING(    0x30, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, "RAM Test Only" )
	PORT_DIPSETTING(    0x20, "Watchdog Test Only" )
	PORT_DIPSETTING(    0x10, "All Tests" )
	PORT_DIPNAME( 0x40, 0x00, "Control Panel" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
//  PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW1:8")
//  PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
//  PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM  ) PORT_VBLANK("screen")

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(    0x06, "15000" )
	PORT_DIPSETTING(    0x04, "20000" )
	PORT_DIPSETTING(    0x02, "30000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x08, 0x08, "Game Speed" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, "Slow" )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x00, "SW2:5" )    /* it should be OFF according to the manual */
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x00, "SW2:6" )    /* it should be OFF according to the manual */
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x00, "SW2:7" )    /* it should be OFF according to the manual */
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x00, "SW2:8" )    /* it should be OFF according to the manual */
INPUT_PORTS_END

static INPUT_PORTS_START( wtennis )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, btime_state,coin_inserted_nmi_lo, 0)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, btime_state,coin_inserted_nmi_lo, 0)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM  ) PORT_VBLANK("screen")

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x06, "10000" )
	PORT_DIPSETTING(    0x04, "20000" )
	PORT_DIPSETTING(    0x02, "30000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )   /* definitely used */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )    /* These 3 switches     */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )        /* have to do with      */
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )         /* coinage.             */
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( mmonkey )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, btime_state,coin_inserted_nmi_lo, 0)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, btime_state,coin_inserted_nmi_lo, 0)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED( 0x20, 0x00 )    /* almost certainly unused */
	PORT_DIPNAME( 0x40, 0x00, "Control Panel" )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
//  PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
//  PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM  ) PORT_VBLANK("screen")

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x06, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x02, "Every 15000" )
	PORT_DIPSETTING(    0x04, "Every 30000" )
	PORT_DIPSETTING(    0x00, "20000" )
	PORT_DIPSETTING(    0x06, DEF_STR( None ) )
	PORT_DIPNAME( 0x18, 0x08, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x18, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, "Level Skip Mode (Cheat)")
	PORT_DIPUNUSED( 0x20, 0x00 )    /* almost certainly unused */
	PORT_DIPUNUSED( 0x40, 0x00 )    /* almost certainly unused */
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )
INPUT_PORTS_END

static INPUT_PORTS_START( bnj )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, btime_state,coin_inserted_nmi_lo, 0)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, btime_state,coin_inserted_nmi_lo, 0)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("8D:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("8D:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	/* Manual gives the following Test settings, but they don't seem to correspond to the actual effect */
//  PORT_DIPNAME( 0x30, 0x30, "Test Mode" ) PORT_DIPLOCATION("8D:5,6")
//  PORT_DIPSETTING(    0x30, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x00, "Video PC Board Only" )
//  PORT_DIPSETTING(    0x10, "Normal Test" )
//  PORT_DIPSETTING(    0x20, "No Effect" )
	PORT_SERVICE_DIPLOC( 0x10, IP_ACTIVE_LOW, "8D:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x00, "8D:6" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("8D:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	/* According to crazykong.com dips this should change the control layout  */
//  PORT_DIPNAME( 0x80, 0x00, "Control Panel" ) PORT_DIPLOCATION("8D:8")
//  PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
//  PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM  ) PORT_VBLANK("screen")

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) ) PORT_DIPLOCATION("7D:1")
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("7D:2,3")
	PORT_DIPSETTING(    0x06, "Every 30000" )
	PORT_DIPSETTING(    0x04, "Every 70000" )
	PORT_DIPSETTING(    0x02, "20000 Only"  )
	PORT_DIPSETTING(    0x00, "30000 Only"  )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("7D:4")
	PORT_DIPSETTING(    0x08, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("7D:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "7D:6" )    /* it should be OFF according to the manual */
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "7D:7" )    /* it should be OFF according to the manual */
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "7D:8" )    /* it should be OFF according to the manual */
INPUT_PORTS_END

static INPUT_PORTS_START( caractn2 ) /* 2/3 Lives Dip changes in this set */
		PORT_INCLUDE( bnj )

		PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) ) PORT_DIPLOCATION("7D:1")
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x00, "3" )
INPUT_PORTS_END

static INPUT_PORTS_START( disco )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x60, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x60, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x1f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH,IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, btime_state,coin_inserted_irq_hi, 0)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH,IPT_COIN2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, btime_state,coin_inserted_irq_hi, 0)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x0c, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_3C ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x00, "SW1:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x00, "SW1:6" )
	PORT_DIPNAME( 0x40, 0x40, "Control Panel" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPNAME( 0x06, 0x00, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x02, "20000" )
	PORT_DIPSETTING(    0x04, "30000" )
	PORT_DIPSETTING(    0x06, DEF_STR( None ) )
	PORT_DIPNAME( 0x08, 0x00, "Music Weapons" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x08, "8" )
	PORT_DIPNAME( 0x10, 0x00, "Game Speed" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, "Slow" )
	PORT_DIPSETTING(    0x10, "Fast" )
	PORT_DIPNAME( 0xe0, 0x00, "Country Code" ) PORT_DIPLOCATION("SW2:6,7,8")
	PORT_DIPSETTING(    0x00, "A" )
	PORT_DIPSETTING(    0x20, "B" )
	PORT_DIPSETTING(    0x40, "C" )
	PORT_DIPSETTING(    0x60, "D" )
	PORT_DIPSETTING(    0x80, "E" )
	PORT_DIPSETTING(    0xa0, "F" )

	PORT_START("VBLANK")
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
INPUT_PORTS_END

static INPUT_PORTS_START( sdtennis )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, btime_state,coin_inserted_nmi_lo, 0)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, btime_state,coin_inserted_nmi_lo, 0)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM  ) PORT_VBLANK("screen")

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x06, "1 Set won" )
	PORT_DIPSETTING(    0x04, "2 Sets won" )
	PORT_DIPSETTING(    0x02, "3 Sets won"  )
	PORT_DIPSETTING(    0x00, DEF_STR( None )  )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )      // Check code at 0xc55b
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )      // Check code at 0xc5af
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xe0, 0xe0, "Copyright" )
	PORT_DIPSETTING(    0xe0, "Data East Corporation" )
	PORT_DIPSETTING(    0xc0, "Data East USA" )
	/* Other values are the same as 0xe0 */
	/* 0x60 also gives a special coinage : COIN1 gives 3 credits and COIN2 gives 8 credits
	  whatever the coinage Dip Switch are (they are not read in this case) */
INPUT_PORTS_END

static const gfx_layout tile8layout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

static const gfx_layout disco_tile8layout =
{
	8,8,
	0x6000/3/8,
	3,
	{ 0x4000*8, 0x2000*8, 0x0000*8 },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};


static const gfx_layout tile16layout =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{ STEP8(16*8,1), STEP8(0,1) },
	{ STEP16(0,8) },
	32*8
};

static const gfx_layout disco_tile16layout =
{
	16,16,
	0x6000/3/32,
	3,
	{ 0x4000*8, 0x2000*8, 0x0000*8 },
	{ STEP8(16*8,1), STEP8(0,1) },
	{ STEP16(0,8) },
	32*8
};

static const gfx_layout bnj_tile16layout =
{
	16,16,
	RGN_FRAC(1,2),
	3,
	{ RGN_FRAC(1,2)+4, RGN_FRAC(0,2)+0, RGN_FRAC(0,2)+4 },
	{ STEP4(3*16*8,1), STEP4(2*16*8,1), STEP4(1*16*8,1), STEP4(0*16*8,1) },
	{ STEP16(0,8) },
	64*8
};

static GFXDECODE_START( btime )
	GFXDECODE_ENTRY( "gfx1", 0, tile8layout,     0, 1 ) /* char set #1 */
	GFXDECODE_ENTRY( "gfx1", 0, tile16layout,    0, 1 ) /* sprites */
	GFXDECODE_ENTRY( "gfx2", 0, tile16layout,    8, 1 ) /* background tiles */
GFXDECODE_END

static GFXDECODE_START( cookrace )
	GFXDECODE_ENTRY( "gfx1", 0, tile8layout,     0, 1 ) /* char set #1 */
	GFXDECODE_ENTRY( "gfx1", 0, tile16layout,    0, 1 ) /* sprites */
	GFXDECODE_ENTRY( "gfx2", 0, tile8layout,     8, 1 ) /* background tiles */
GFXDECODE_END

static GFXDECODE_START( lnc )
	GFXDECODE_ENTRY( "gfx1", 0, tile8layout,     0, 1 ) /* char set #1 */
	GFXDECODE_ENTRY( "gfx1", 0, tile16layout,    0, 1 ) /* sprites */
GFXDECODE_END

static GFXDECODE_START( bnj )
	GFXDECODE_ENTRY( "gfx1", 0, tile8layout,     0, 1 ) /* char set #1 */
	GFXDECODE_ENTRY( "gfx1", 0, tile16layout,    0, 1 ) /* sprites */
	GFXDECODE_ENTRY( "gfx2", 0, bnj_tile16layout,8, 1 ) /* background tiles */
GFXDECODE_END

static GFXDECODE_START( zoar )
	GFXDECODE_ENTRY( "gfx1", 0, tile8layout,     0, 8 ) /* char set #1 */
	GFXDECODE_ENTRY( "gfx3", 0, tile16layout,    0, 8 ) /* sprites */
	GFXDECODE_ENTRY( "gfx2", 0, tile16layout,    0, 8 ) /* background tiles */
GFXDECODE_END

static GFXDECODE_START( disco )
	GFXDECODE_ENTRY( 0x0000, 0, disco_tile8layout,  0, 4 ) /* char set #1 */
	GFXDECODE_ENTRY( 0x0000, 0, disco_tile16layout, 0, 4 ) /* sprites */
GFXDECODE_END

/***************************************************************************
  Discrete Filtering and Mixing

  All values taken from Burger Time Schematics.

 ****************************************************************************/

static const discrete_mixer_desc btime_sound_mixer_desc =
	{DISC_MIXER_IS_OP_AMP,
		{RES_K(100), RES_K(100)},
		{0,0},  /* no variable resistors   */
		{0,0},  /* no node capacitors      */
		0,      /* no RI */
		RES_K(10),
		CAP_P(150),
		0,      /* Modelled separately */
		0, 1};

/* R49 has 4.7k in schematics, but listed as 47k in bill of material
 * 47k gives proper low pass filtering
 *
 * Anoid measured R49 to R52 on a Burger Time pcb. These are
 * listed below
 */
#define BTIME_R49   RES_K(47)   /* pcb: 47.4k */

/* The input divider R51 R50 is not independent of R52, which
 * also depends on ay internal resistance.
 * FIXME: Develop proper model when I am retired.
 *
 * With R51 being 1K, the gain is way to high (23.5). Therefore R51
 * is set to 5k, but this is a hack. With the modification,
 * sound levels are in line with observations.
 * R51,R50,R52 and R49 verified on real pcb by Anoid.
 *
 * http://www.coinopvideogames.com/videogames01.php
 * There are two recordings from 1982 where the filtered sound is way louder
 * than the music. There is a later recording
 * http://www.coinopvideogames.com/videogames03.php
 * in which the filtered sounds have volumes closer to the music.
 *
 */

#define BTIME_R52   RES_K(1)    /* pcb: .912k = 1K || 11k */
#define BTIME_R51   RES_K(5)    /* pcb: .923k = 1k || 11k schematics 1k */
#define BTIME_R50   RES_K(10)   /* pcb: 1.667k = 10k || 2k */

static const discrete_op_amp_filt_info btime_opamp_desc =
	{BTIME_R51, 0, BTIME_R50, 0, BTIME_R49, CAP_U(0.068), CAP_U(0.068), 0, 0, 5.0, -5.0};

static DISCRETE_SOUND_START( btime_sound )

	DISCRETE_INPUTX_STREAM(NODE_01, 0, 5.0/32767.0, 0)
	DISCRETE_INPUTX_STREAM(NODE_02, 1, 5.0/32767.0, 0)
	DISCRETE_INPUTX_STREAM(NODE_03, 2, 5.0/32767.0, 0)

	DISCRETE_INPUTX_STREAM(NODE_04, 3, 5.0/32767.0, 0)
	DISCRETE_INPUTX_STREAM(NODE_05, 4, 5.0/32767.0, 0)
	DISCRETE_INPUTX_STREAM(NODE_06, 5, 5.0/32767.0, 0)

	/* Mix 5 channels 1A, 1B, 1C, 2B, 2C directly */
	DISCRETE_ADDER3(NODE_20, 1, NODE_01, NODE_02, NODE_03)
	DISCRETE_ADDER3(NODE_21, 1, NODE_20, NODE_05, NODE_06)
	DISCRETE_MULTIPLY(NODE_22, NODE_21, 0.2)

	/* Filter of channel 2A */
	DISCRETE_OP_AMP_FILTER(NODE_30, 1, NODE_04, NODE_NC, DISC_OP_AMP_FILTER_IS_BAND_PASS_1M, &btime_opamp_desc)

	DISCRETE_MIXER2(NODE_40, 1, NODE_22, NODE_30, &btime_sound_mixer_desc)
	DISCRETE_CRFILTER(NODE_41, NODE_40, RES_K(10), CAP_U(10))

	/* Amplifier is upc1181H3
	 *
	 * http://www.ic-ts-histo.de/fad/ics/upc1181/upc1181.htm
	 *
	 * A linear frequency response is mentioned as well as a lower
	 * edge frequency determined by cap on pin3, however no formula given.
	 *
	 * not modelled here
	 */

	/* Assuming a 4 Ohm impedance speaker */
	DISCRETE_CRFILTER(NODE_43, NODE_41, 3.0, CAP_U(100))

	DISCRETE_OUTPUT(NODE_43, 32767.0 / 5. * 35.0)

DISCRETE_SOUND_END


MACHINE_START_MEMBER(btime_state,btime)
{
	save_item(NAME(m_btime_palette));
	save_item(NAME(m_bnj_scroll1));
	save_item(NAME(m_bnj_scroll2));
	save_item(NAME(m_btime_tilemap));
	save_item(NAME(m_audio_nmi_enabled));
	save_item(NAME(m_audio_nmi_state));
}

MACHINE_START_MEMBER(btime_state,mmonkey)
{
	MACHINE_START_CALL_MEMBER(btime);

	save_item(NAME(m_protection_command));
	save_item(NAME(m_protection_status));
	save_item(NAME(m_protection_value));
	save_item(NAME(m_protection_ret));
}

MACHINE_RESET_MEMBER(btime_state,btime)
{
	/* by default, the audio NMI is disabled, except for bootlegs which don't use the enable */
	m_audio_nmi_enabled = (m_audio_nmi_enable_type == AUDIO_ENABLE_NONE);

	m_btime_palette = 0;
	m_bnj_scroll1 = 0;
	m_bnj_scroll2 = 0;
	m_btime_tilemap[0] = 0;
	m_btime_tilemap[1] = 0;
	m_btime_tilemap[2] = 0;
	m_btime_tilemap[3] = 0;
	m_audio_nmi_state = 0;
}

MACHINE_RESET_MEMBER(btime_state,lnc)
{
	*m_lnc_charbank = 1;

	MACHINE_RESET_CALL_MEMBER(btime);
}

MACHINE_RESET_MEMBER(btime_state,mmonkey)
{
	MACHINE_RESET_CALL_MEMBER(lnc);

	m_protection_command = 0;
	m_protection_status = 0;
	m_protection_value = 0;
	m_protection_ret = 0;
}

static MACHINE_CONFIG_START( btime, btime_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", DECO_CPU7, HCLK2)   /* seletable between H2/H4 via jumper */
	MCFG_CPU_PROGRAM_MAP(btime_map)

	MCFG_CPU_ADD("audiocpu", M6502, HCLK1/3/2)
	MCFG_CPU_PROGRAM_MAP(audio_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("audionmi", btime_state, audio_nmi_gen, "screen", 0, 8)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(HCLK, 384, 8, 248, 272, 8, 248)
	MCFG_SCREEN_UPDATE_DRIVER(btime_state, screen_update_btime)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_MACHINE_START_OVERRIDE(btime_state,btime)
	MCFG_MACHINE_RESET_OVERRIDE(btime_state,btime)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", btime)

	MCFG_PALETTE_ADD("palette", 16)
	MCFG_PALETTE_INIT_OWNER(btime_state,btime)
	MCFG_PALETTE_FORMAT(BBGGGRRR_inverted)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, HCLK2)
	MCFG_AY8910_OUTPUT_TYPE(AY8910_DISCRETE_OUTPUT)
	MCFG_AY8910_RES_LOADS(RES_K(5), RES_K(5), RES_K(5))
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(btime_state, ay_audio_nmi_enable_w))
	MCFG_SOUND_ROUTE_EX(0, "discrete", 1.0, 0)
	MCFG_SOUND_ROUTE_EX(1, "discrete", 1.0, 1)
	MCFG_SOUND_ROUTE_EX(2, "discrete", 1.0, 2)

	MCFG_SOUND_ADD("ay2", AY8910, HCLK2)
	MCFG_AY8910_OUTPUT_TYPE(AY8910_DISCRETE_OUTPUT)
	MCFG_AY8910_RES_LOADS(RES_K(1), RES_K(5), RES_K(5))
	MCFG_SOUND_ROUTE_EX(0, "discrete", 1.0, 3)
	MCFG_SOUND_ROUTE_EX(1, "discrete", 1.0, 4)
	MCFG_SOUND_ROUTE_EX(2, "discrete", 1.0, 5)

	MCFG_SOUND_ADD("discrete", DISCRETE, 0)
	MCFG_DISCRETE_INTF(btime_sound)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( cookrace, btime )

	/* basic machine hardware */
	MCFG_CPU_REPLACE("maincpu", DECO_C10707, HCLK2)
	MCFG_CPU_PROGRAM_MAP(cookrace_map)

	MCFG_CPU_MODIFY("audiocpu")
	MCFG_CPU_PROGRAM_MAP(audio_map)

	/* video hardware */
	MCFG_GFXDECODE_MODIFY("gfxdecode", cookrace)

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(btime_state, screen_update_cookrace)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( lnc, btime )

	/* basic machine hardware */
	MCFG_CPU_REPLACE("maincpu", DECO_C10707, HCLK2)
	MCFG_CPU_PROGRAM_MAP(lnc_map)

	MCFG_MACHINE_RESET_OVERRIDE(btime_state,lnc)

	/* video hardware */
	MCFG_GFXDECODE_MODIFY("gfxdecode", lnc)

	MCFG_PALETTE_MODIFY("palette")
	MCFG_PALETTE_ENTRIES(8)
	MCFG_PALETTE_INIT_OWNER(btime_state,lnc)

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(btime_state, screen_update_lnc)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( wtennis, lnc )

	/* basic machine hardware */

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(btime_state, screen_update_eggs)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( mmonkey, wtennis )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(mmonkey_map)

	MCFG_MACHINE_START_OVERRIDE(btime_state,mmonkey)
	MCFG_MACHINE_RESET_OVERRIDE(btime_state,mmonkey)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( bnj, btime )

	/* basic machine hardware */
	MCFG_CPU_REPLACE("maincpu", DECO_C10707, HCLK4)
	MCFG_CPU_CLOCK(HCLK4)
	MCFG_CPU_PROGRAM_MAP(bnj_map)

	/* video hardware */
	MCFG_GFXDECODE_MODIFY("gfxdecode", bnj)

	MCFG_VIDEO_START_OVERRIDE(btime_state,bnj)

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(btime_state, screen_update_bnj)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 1*8, 31*8-1) // 256 * 240, confirmed
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( sdtennis, bnj )

	/* basic machine hardware */
	MCFG_CPU_REPLACE("audiocpu", DECO_C10707, HCLK1/3/2)
	MCFG_CPU_PROGRAM_MAP(audio_map)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( zoar, btime )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(zoar_map)

	/* video hardware */
	MCFG_GFXDECODE_MODIFY("gfxdecode", zoar)

	MCFG_PALETTE_MODIFY("palette")
	MCFG_PALETTE_ENTRIES(64)

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(btime_state, screen_update_zoar)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 1*8, 31*8-1) // 256 * 240, confirmed

	/* sound hardware */
	MCFG_SOUND_REPLACE("ay1", AY8910, HCLK1)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.23)
	MCFG_AY8910_OUTPUT_TYPE(AY8910_DISCRETE_OUTPUT)
	MCFG_AY8910_RES_LOADS(RES_K(5), RES_K(5), RES_K(5))
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(btime_state, ay_audio_nmi_enable_w))

	MCFG_SOUND_REPLACE("ay2", AY8910, HCLK1)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.23)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( disco, btime )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_CLOCK(HCLK4)
	MCFG_CPU_PROGRAM_MAP(disco_map)

	MCFG_CPU_MODIFY("audiocpu")
	MCFG_CPU_PROGRAM_MAP(disco_audio_map)

	/* video hardware */
	MCFG_GFXDECODE_MODIFY("gfxdecode", disco)

	MCFG_PALETTE_MODIFY("palette")
	MCFG_PALETTE_ENTRIES(32)

	MCFG_VIDEO_START_OVERRIDE(btime_state,disco)

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(btime_state, screen_update_disco)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( tisland, btime )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(tisland_map)

	/* video hardware */
	MCFG_GFXDECODE_MODIFY("gfxdecode", zoar)
MACHINE_CONFIG_END


/***************************************************************************

    Game driver(s)

***************************************************************************/

ROM_START( btime )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "aa04.9b",      0xc000, 0x1000, CRC(368a25b5) SHA1(ed3f3712423979dcb351941fa85dce6a0a7bb16b) )
	ROM_LOAD( "aa06.13b",     0xd000, 0x1000, CRC(b4ba400d) SHA1(8c77397e934907bc47a739f263196a0f2f81ba3d) )
	ROM_LOAD( "aa05.10b",     0xe000, 0x1000, CRC(8005bffa) SHA1(d0da4e360039f6a8d8142a4e8e05c1f90c0af68a) )
	ROM_LOAD( "aa07.15b",     0xf000, 0x1000, CRC(086440ad) SHA1(4a32bc92f8ff5fbe112f56e62d2c03da8851a7b9) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ab14.12h",     0xe000, 0x1000, CRC(f55e5211) SHA1(27940026d0c6212d1138d2fd88880df697218627) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "aa12.7k",      0x0000, 0x1000, CRC(c4617243) SHA1(24204d591aa2c264a852ee9ba8c4be63efd97728) )    /* charset #1 */
	ROM_LOAD( "ab13.9k",      0x1000, 0x1000, CRC(ac01042f) SHA1(e64b6381a9298eaf74e79fa5f1ea8e9596c58a49) )
	ROM_LOAD( "ab10.10k",     0x2000, 0x1000, CRC(854a872a) SHA1(3d2ecfd54a5a9d68b53cf4b4ee1f2daa6aef2123) )
	ROM_LOAD( "ab11.12k",     0x3000, 0x1000, CRC(d4848014) SHA1(0a55b091cd4e7f317c35defe13d5051b26042eee) )
	ROM_LOAD( "aa8.13k",      0x4000, 0x1000, CRC(8650c788) SHA1(d9b1ee2d1f2fd66705d497c80252861b49aa9254) )
	ROM_LOAD( "ab9.15k",      0x5000, 0x1000, CRC(8dec15e6) SHA1(b72633de6268ce16742bba4dcba835df860d6c2f) )

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "ab00.1b",      0x0000, 0x0800, CRC(c7a14485) SHA1(6a0a8e6b7860859f22daa33634e34fbf91387659) )    /* charset #2 */
	ROM_LOAD( "ab01.3b",      0x0800, 0x0800, CRC(25b49078) SHA1(4abdcbd4f3362c3e4463a1274731289f1a72d2e6) )
	ROM_LOAD( "ab02.4b",      0x1000, 0x0800, CRC(b8ef56c3) SHA1(4a03bf011dc1fb2902f42587b1174b880cf06df1) )

	ROM_REGION( 0x0800, "bg_map", 0 )   /* background tilemaps */
	ROM_LOAD( "ab03.6b",      0x0000, 0x0800, CRC(d26bc1f3) SHA1(737af6e264183a1f151f277a07cf250d6abb3fd8) )
ROM_END


ROM_START( btime2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "aa04.9b2",     0xc000, 0x1000, CRC(a041e25b) SHA1(caaab3ae46619d0a87a8985d316411f23be0b696) )
	ROM_LOAD( "aa06.13b",     0xd000, 0x1000, CRC(b4ba400d) SHA1(8c77397e934907bc47a739f263196a0f2f81ba3d) )
	ROM_LOAD( "aa05.10b",     0xe000, 0x1000, CRC(8005bffa) SHA1(d0da4e360039f6a8d8142a4e8e05c1f90c0af68a) )
	ROM_LOAD( "aa07.15b",     0xf000, 0x1000, CRC(086440ad) SHA1(4a32bc92f8ff5fbe112f56e62d2c03da8851a7b9) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ab14.12h",     0xe000, 0x1000, CRC(f55e5211) SHA1(27940026d0c6212d1138d2fd88880df697218627) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "aa12.7k",      0x0000, 0x1000, CRC(c4617243) SHA1(24204d591aa2c264a852ee9ba8c4be63efd97728) )    /* charset #1 */
	ROM_LOAD( "ab13.9k",      0x1000, 0x1000, CRC(ac01042f) SHA1(e64b6381a9298eaf74e79fa5f1ea8e9596c58a49) )
	ROM_LOAD( "ab10.10k",     0x2000, 0x1000, CRC(854a872a) SHA1(3d2ecfd54a5a9d68b53cf4b4ee1f2daa6aef2123) )
	ROM_LOAD( "ab11.12k",     0x3000, 0x1000, CRC(d4848014) SHA1(0a55b091cd4e7f317c35defe13d5051b26042eee) )
	ROM_LOAD( "aa8.13k",      0x4000, 0x1000, CRC(8650c788) SHA1(d9b1ee2d1f2fd66705d497c80252861b49aa9254) )
	ROM_LOAD( "ab9.15k",      0x5000, 0x1000, CRC(8dec15e6) SHA1(b72633de6268ce16742bba4dcba835df860d6c2f) )

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "ab00.1b",      0x0000, 0x0800, CRC(c7a14485) SHA1(6a0a8e6b7860859f22daa33634e34fbf91387659) )    /* charset #2 */
	ROM_LOAD( "ab01.3b",      0x0800, 0x0800, CRC(25b49078) SHA1(4abdcbd4f3362c3e4463a1274731289f1a72d2e6) )
	ROM_LOAD( "ab02.4b",      0x1000, 0x0800, CRC(b8ef56c3) SHA1(4a03bf011dc1fb2902f42587b1174b880cf06df1) )

	ROM_REGION( 0x0800, "bg_map", 0 )   /* background tilemaps */
	ROM_LOAD( "ab03.6b",      0x0000, 0x0800, CRC(d26bc1f3) SHA1(737af6e264183a1f151f277a07cf250d6abb3fd8) )
ROM_END

ROM_START( btime3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ab05a-3.12b",  0xb000, 0x1000, CRC(12e9f58c) SHA1(c1a933c83255af431643451b4eb68dc755bf0f61) ) /* Revision 3 & Copyright 1982 DATA EAST USA. INC. */
	ROM_LOAD( "ab04-3.9b",    0xc000, 0x1000, CRC(5d90c696) SHA1(7b1674e7b6249a2d806d81abd967adeeb51111be) )
	ROM_LOAD( "ab06-3.13b",   0xd000, 0x1000, CRC(e0b993ad) SHA1(42674cc399a8281a9a6c6cdbe38f7e5a4b3e6cb9) )
	ROM_LOAD( "ab05-3.10b",   0xe000, 0x1000, CRC(c2b44b7f) SHA1(03c972f4ca0a31a2689d2f2d4064d82732fb19b9) )
	ROM_LOAD( "ab07-3.15b",   0xf000, 0x1000, CRC(91986594) SHA1(f163eb7b27b602ce61a2dee1ae221a6e1f84c43d) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ab14-1.12h",   0xe000, 0x1000, CRC(f55e5211) SHA1(27940026d0c6212d1138d2fd88880df697218627) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "ab12-1.7k",    0x0000, 0x1000, BAD_DUMP CRC(6c79f79f) SHA1(338009199b5889621693833d88c35abb8e9e38a2) ) /* ROM was damaged, not verified the same */   /* charset #1 */
	ROM_LOAD( "ab13-1.9k",    0x1000, 0x1000, BAD_DUMP CRC(ac01042f) SHA1(e64b6381a9298eaf74e79fa5f1ea8e9596c58a49) ) /* ROM was damaged, not verified the same */
	ROM_LOAD( "ab10-1.10k",   0x2000, 0x1000, CRC(854a872a) SHA1(3d2ecfd54a5a9d68b53cf4b4ee1f2daa6aef2123) )
	ROM_LOAD( "ab11-1.12k",   0x3000, 0x1000, CRC(d4848014) SHA1(0a55b091cd4e7f317c35defe13d5051b26042eee) )
	ROM_LOAD( "ab8-1.13k",    0x4000, 0x1000, BAD_DUMP CRC(70b35bbe) SHA1(ee8d70d6792ac4b8fe3de90c665457fedb94a7ba) ) /* ROM was damaged, not verified the same */
	ROM_LOAD( "ab9-1.15k",    0x5000, 0x1000, BAD_DUMP CRC(8dec15e6) SHA1(b72633de6268ce16742bba4dcba835df860d6c2f) ) /* ROM was damaged, not verified the same */

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "ab00-1.1b",    0x0000, 0x0800, CRC(c7a14485) SHA1(6a0a8e6b7860859f22daa33634e34fbf91387659) )
	ROM_LOAD( "ab01-1.3b",    0x0800, 0x0800, BAD_DUMP CRC(25b49078) SHA1(4abdcbd4f3362c3e4463a1274731289f1a72d2e6) ) /* ROM was damaged, not verified the same */
	ROM_LOAD( "ab02-1.4b",    0x1000, 0x0800, CRC(b8ef56c3) SHA1(4a03bf011dc1fb2902f42587b1174b880cf06df1) )

	ROM_REGION( 0x0800, "bg_map", 0 )   /* background tilemaps */
	ROM_LOAD( "ab03-3.6b",    0x0000, 0x0800, CRC(f699d797) SHA1(c09ba5e652f26683d90b6a5637e41adecc4f1afa) )
ROM_END

ROM_START( btimem )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ab05a1.12b",   0xb000, 0x1000, CRC(0a98b230) SHA1(aeee4f6f0aaa27575b80261d03c5453cc6ebd646) )
	ROM_LOAD( "ab04.9b",      0xc000, 0x1000, CRC(797e5f75) SHA1(35ea5fa4b8f3494adf7774b3946ed2540ac826ff) )
	ROM_LOAD( "ab06.13b",     0xd000, 0x1000, CRC(c77f3f64) SHA1(f283087fad0a102fe92be7ce80ed18e64dc93b67) )
	ROM_LOAD( "ab05.10b",     0xe000, 0x1000, CRC(b0d3640f) SHA1(6ba28971714ece6f1c04fa2dbf1f9f216ded7cfa) )
	ROM_LOAD( "ab07.15b",     0xf000, 0x1000, CRC(a142f862) SHA1(39d7ef172d18874885f1b1542e885cc4287dc344) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ab14.12h",     0xe000, 0x1000, CRC(f55e5211) SHA1(27940026d0c6212d1138d2fd88880df697218627) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "ab12.7k",      0x0000, 0x1000, CRC(6c79f79f) SHA1(338009199b5889621693833d88c35abb8e9e38a2) )    /* charset #1 */
	ROM_LOAD( "ab13.9k",      0x1000, 0x1000, CRC(ac01042f) SHA1(e64b6381a9298eaf74e79fa5f1ea8e9596c58a49) )
	ROM_LOAD( "ab10.10k",     0x2000, 0x1000, CRC(854a872a) SHA1(3d2ecfd54a5a9d68b53cf4b4ee1f2daa6aef2123) )
	ROM_LOAD( "ab11.12k",     0x3000, 0x1000, CRC(d4848014) SHA1(0a55b091cd4e7f317c35defe13d5051b26042eee) )
	ROM_LOAD( "ab8.13k",      0x4000, 0x1000, CRC(70b35bbe) SHA1(ee8d70d6792ac4b8fe3de90c665457fedb94a7ba) )
	ROM_LOAD( "ab9.15k",      0x5000, 0x1000, CRC(8dec15e6) SHA1(b72633de6268ce16742bba4dcba835df860d6c2f) )

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "ab00.1b",      0x0000, 0x0800, CRC(c7a14485) SHA1(6a0a8e6b7860859f22daa33634e34fbf91387659) )    /* charset #2 */
	ROM_LOAD( "ab01.3b",      0x0800, 0x0800, CRC(25b49078) SHA1(4abdcbd4f3362c3e4463a1274731289f1a72d2e6) )
	ROM_LOAD( "ab02.4b",      0x1000, 0x0800, CRC(b8ef56c3) SHA1(4a03bf011dc1fb2902f42587b1174b880cf06df1) )

	ROM_REGION( 0x0800, "bg_map", 0 )   /* background tilemaps */
	ROM_LOAD( "ab03.6b",      0x0000, 0x0800, CRC(d26bc1f3) SHA1(737af6e264183a1f151f277a07cf250d6abb3fd8) )
ROM_END

ROM_START( cookrace )
	ROM_REGION( 0x10000, "maincpu", 0 )
	/* code is in the range 0500-3fff, encrypted */
	ROM_LOAD( "1f.1",         0x0000, 0x2000, CRC(68759d32) SHA1(2112a6f17b871aefdb39739e47d4a9f368a2eb3c) )
	ROM_LOAD( "2f.2",         0x2000, 0x2000, CRC(be7d72d1) SHA1(232d108098cb490e7c828aa4524ad09d3866ae18) )
	ROM_LOAD( "2k",           0xffe0, 0x0020, CRC(e2553b3d) SHA1(0a38929cdb3f37c6e4bacc5c3f94c049b4352858) )    /* reset/interrupt vectors */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "6f.6",         0xe000, 0x1000, CRC(6b8e0272) SHA1(372a891b7b357aea0297ba9bcae752c3c9d8c1be) ) /* starts at 0000, not f000; 0000-01ff is RAM */

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "m8.7",         0x0000, 0x2000, CRC(a1a0d5a6) SHA1(e9583320e9c303407abfe02988b95403e5209c52) )  /* charset #1 */
	ROM_LOAD( "m7.8",         0x2000, 0x2000, CRC(1104f497) SHA1(60abd05c2549fe014660c169011480beb191f36d) )
	ROM_LOAD( "m6.9",         0x4000, 0x2000, CRC(d0d94477) SHA1(74ca9134a52cabe5769d714855b38a49632b9e40) )

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "2f.3",         0x0000, 0x0800, CRC(28609a75) SHA1(ab5d02bc0a771227db820a79b16aa662fb2140cf) )  /* garbage?? */
	ROM_CONTINUE(             0x0000, 0x0800 )              /* charset #2 */
	ROM_LOAD( "4f.4",         0x0800, 0x0800, CRC(7742e771) SHA1(c938c5714273bd4f2a1beb23d781ecbe7b023e6d) )  /* garbage?? */
	ROM_CONTINUE(             0x0800, 0x0800 )
	ROM_LOAD( "5f.5",         0x1000, 0x0800, CRC(611c686f) SHA1(e2c45061597d3d1a855a625a906b5a17a87deb2c) )  /* garbage?? */
	ROM_CONTINUE(             0x1000, 0x0800 )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "f9.clr",       0x0000, 0x0020, CRC(c2348c1d) SHA1(a7cc4b499b6c89c5966711f8bb922026c2978e1a) )    /* palette */
	ROM_LOAD( "b7",           0x0020, 0x0020, CRC(e4268fa6) SHA1(93f74e633c3a19755e78e0e2883109cd8ccde9a8) )    /* unknown */
ROM_END

ROM_START( tisland )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "t-04.b7",      0xa000, 0x1000, CRC(641af7f9) SHA1(50cd8f2372725356bb5a66024084363f5c5a870d) )
	ROM_RELOAD(               0x9000, 0x1000 )
	ROM_LOAD( "t-07.b11",     0xb000, 0x1000, CRC(6af00c8b) SHA1(e3948ca36642d3c2a1f94b017893d6e2fe178bb0) )
	ROM_LOAD( "t-05.b9",      0xc000, 0x1000, CRC(95b1a1d3) SHA1(5636580f26e839d1140838c7efc1cabc2cf06f6f) )
	ROM_LOAD( "t-08.b13",     0xd000, 0x1000, CRC(b7bbc008) SHA1(751491eac90f46985c83a6c06088638bcd0c0f20) )
	ROM_LOAD( "t-06.b10",     0xe000, 0x1000, CRC(5a6783cf) SHA1(f518290efec0fedb92432b4e3448aea2438b8448) )
	ROM_LOAD( "t-09.b14",     0xf000, 0x1000, CRC(5b26771a) SHA1(31d86acba4b6549fc08a3947d6d6d1a470fcb9da) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "t-0a.j11",     0xe000, 0x1000, CRC(807e1652) SHA1(ccfee616dc0e34d10a0e62b9864fd987291bf176) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_LOAD( "t-13.k14",     0x0000, 0x1000, CRC(95bdec2f) SHA1(201b9c53ea53a25535b619231d0d14e08c206ecf) )
	ROM_LOAD( "t-10.k10",     0x1000, 0x1000, CRC(3ba416cb) SHA1(90c968f963ba6f52f979f28f62eaccc0e2911508) )
	ROM_LOAD( "t-0d.k5",      0x2000, 0x1000, CRC(3d3e40b2) SHA1(90576c82500ce8eddbf4dd02e59ec4ccc3b13000) ) /* 8x8 tiles */

	ROM_REGION( 0x1800, "gfx2", 0 ) /* bg tiles */
	// also contains the (incomplete) bg tilemap data for 1 tilemap (0x400-0x7ff of every rom is same as bg_map region, leftover?) */
	ROM_LOAD( "t-00.b1",      0x0000, 0x0800, CRC(05eaf899) SHA1(b03a1b7d985b4d841d6bbb213a32a33e324dff89) )    /* charset #2 */
	ROM_LOAD( "t-01.b2",      0x0800, 0x0800, CRC(f692e9e0) SHA1(e07ef20de8e9387f1096412d42d14ed5e52bbbd9) )
	ROM_LOAD( "t-02.b4",      0x1000, 0x0800, CRC(88396cae) SHA1(47233d91e9c7b14091a0050524fa49e1bc69311d) )

	ROM_REGION( 0x6000, "gfx3", 0 )
	ROM_LOAD( "t-11.k11",     0x0000, 0x1000, CRC(779cc47c) SHA1(8921b81d460232252fd5a3c9bb2ad0befc1421da) ) /* 16x16 tiles*/
	ROM_LOAD( "t-12.k13",     0x1000, 0x1000, CRC(c804a8aa) SHA1(f8ce1da88443416b6cd276741a600104d36c3725) )
	ROM_LOAD( "t-0e.k6",      0x2000, 0x1000, CRC(63aa2b22) SHA1(765c405b1948191f5bdf1d8c1e7f20acb0894195) )
	ROM_LOAD( "t-0f.k8",      0x3000, 0x1000, CRC(3eeca392) SHA1(78deceea3628aed0a57cb4208d260a91a304695a) )
	ROM_LOAD( "t-0b.k2",      0x4000, 0x1000, CRC(ec416f20) SHA1(20852ef9753b103c5ec03d5eede778c0e25fc059) )
	ROM_LOAD( "t-0c.k4",      0x5000, 0x1000, CRC(428513a7) SHA1(aab97ee938dc743a2941f71f827c22b9dde8aef0) )

	ROM_REGION( 0x1000, "bg_map", 0 ) /* bg tilemap data */
	ROM_LOAD( "t-03.b5",      0x0000, 0x1000, CRC(68df6d50) SHA1(461acc39089faac36bf8a8d279fbb6c046ae0264) )
ROM_END

/* There is a flyer with a screen shot for Lock'n'Chase at:
   http://www.arcadeflyers.com/?page=flyer&db=videodb&id=608&image=1  */

ROM_START( lnc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "s3-3d",        0xc000, 0x1000, CRC(1ab4f2c2) SHA1(c5890b768172cd2e3912b84db5f71546969ad7e2) )
	ROM_LOAD( "s2-3c",        0xd000, 0x1000, CRC(5e46b789) SHA1(00b2510e07eb565cb373db798dd537191b0b7cc8) )
	ROM_LOAD( "s1-3b",        0xe000, 0x1000, CRC(1308a32e) SHA1(da64fe7b76f5ac8ac35460e6c789ab1e986c78ef) )
	ROM_LOAD( "s0-3a",        0xf000, 0x1000, CRC(beb4b1fc) SHA1(166a96b5757946231f3619844366218065412935) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "sa-1h",        0xe000, 0x1000, CRC(379387ec) SHA1(29d37f04c64ed53a2573962dfa9c0623b89e0045) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "s4-11l",       0x0000, 0x1000, CRC(a2162a9e) SHA1(2729cef805c8e863af540424faa1aca82d3525e2) )
	ROM_LOAD( "s5-11m",       0x1000, 0x1000, CRC(12f1c2db) SHA1(004e25a53ffa197e1238dfa53c530f128cf40516) )
	ROM_LOAD( "s6-13l",       0x2000, 0x1000, CRC(d21e2a57) SHA1(0462cd3a5be87da97ed1bd8b79f8822cd5a33cf1) )
	ROM_LOAD( "s7-13m",       0x3000, 0x1000, CRC(c4f247cd) SHA1(2c86bf479169981daf0378eb0b3e1a600937aaf2) )
	ROM_LOAD( "s8-15l",       0x4000, 0x1000, CRC(672a92d0) SHA1(1bc89f6a76873504aa0fcfa0c6a43e8546edde27) )
	ROM_LOAD( "s9-15m",       0x5000, 0x1000, CRC(87c8ee9a) SHA1(158019b18bc3e5104bebeb241c077a706bf72ff2) )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "sc-5m",        0x0000, 0x0020, CRC(2a976ebe) SHA1(f3c1b0d98f431f9cd0d5fa009fafa1115aabe6e5) )    /* palette */
	ROM_LOAD( "sb-4c",        0x0020, 0x0020, CRC(a29b4204) SHA1(7f15cae5c4aaa29638fb45029782dafd2b3d1484) )    /* RAS/CAS logic - not used */
ROM_END

/*This one doesn't have the (c) deco and the "pro" word at the title screen so I'm assuming it's a bootleg.*/
ROM_START( protennb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "t6.a1",        0xa000, 0x1000, CRC(e89cc295) SHA1(68f1578c4be816db6028a561d286b19553c87506) )
	ROM_LOAD( "t5.a3",        0xb000, 0x1000, CRC(9131ed87) SHA1(af2276a82e024bf00c6db02deb7f06ade89dd386) )
	ROM_LOAD( "t4.a4",        0xc000, 0x1000, CRC(01dc0e71) SHA1(a359468fb9dab9cfadcf8ec22a4d7ce9341f4324) )
	ROM_LOAD( "t3.a6",        0xd000, 0x1000, CRC(6253acec) SHA1(24aaac1cdea1c60f8ff05dff6c17ba3a0e732187) )
	ROM_LOAD( "t2.a8",        0xe000, 0x1000, CRC(6faf561c) SHA1(7fd5430af4b3f255e2c01e9b092b960ebdca8d13) )
	ROM_LOAD( "t1.a9",        0xf000, 0x1000, CRC(baa330ae) SHA1(b10c66d9a03b036d95926d0c0fe441bb7ca4015d) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "t7.b1",        0xf000, 0x1000, CRC(a6bcc2d1) SHA1(383cd170417256467dfce94939d6afa66518c6d2) )

	ROM_REGION( 0x6000, "gfx1", ROMREGION_ERASE00 )
	/* dynamically allocated */

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "82s123n.a8",    0x0000, 0x0020, CRC(6a0006ac) SHA1(72265bc472fb7610af190130560ef507244ce41c) )   /* palette */
	ROM_LOAD( "82s123n.j10",   0x0020, 0x0020, CRC(27b004e3) SHA1(4b9960b99130281a3b07f44816001e5eabf7a6fc) )   /* RAS/CAS logic - not used */
ROM_END

ROM_START( wtennis )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ten14.h4",     0xc000, 0x1000, CRC(f7b27303) SHA1(043476429bcc8def412f77d1f79a01586d984bfc) )
	ROM_LOAD( "ten4.d4",      0xd000, 0x1000, CRC(e465d82c) SHA1(c357dcf17539150425574985afa559db2e6ab834) ) // was t4
	ROM_LOAD( "ten3.c4",      0xe000, 0x1000, CRC(8f090eab) SHA1(baeef8ee05010bf44cf8865a22911f3d458df1b0) ) // was t3
	ROM_LOAD( "ten2.a4",      0xf000, 0x1000, CRC(d2f9dd30) SHA1(1faa088806e8627b5e561d8b99054d295045dcfb) ) // was t2

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ten1.h1",      0xe000, 0x1000, CRC(40737ea7) SHA1(27e8474028385574035d3982f9c576bb9bb3facd) ) /* was t1 - starts at 0000, not f000; 0000-01ff is RAM */

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "ten7.l11",     0x0000, 0x1000, CRC(aa935169) SHA1(965f41a9fcf35ac7c899e79acd0a85ab588d5831) ) // was t7
	ROM_LOAD( "ten10.m11",    0x1000, 0x1000, CRC(746be927) SHA1(a3361384437ac7c494fde92953c5aa5e3c104644) ) // was t10
	ROM_LOAD( "ten6.l13",     0x2000, 0x1000, CRC(4fb8565d) SHA1(6de865e41dcba45190af0753baebf5ab66e4eeb4) ) // was t6
	ROM_LOAD( "ten9.m13",     0x3000, 0x1000, CRC(4893286d) SHA1(f2c330286272b8d334b887bc4dd9608158249fc3) ) // was t9
	ROM_LOAD( "ten5.l14",     0x4000, 0x1000, CRC(ea1efa5d) SHA1(dd8ef1991d74778e6844a669e6de649e1130ec79) ) // was t5
	ROM_LOAD( "ten8.m14",     0x5000, 0x1000, CRC(542ace7b) SHA1(b1423d39302ad7d98c9223d8b1d6d062b7676dd9) ) // was t8

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "mb7051.m5",    0x0000, 0x0020, CRC(f051cb28) SHA1(6aebccd38ba7887caff248c8acddb8e14526f1e7) )    /* palette */
	ROM_LOAD( "sb-4c",        0x0020, 0x0020, CRC(a29b4204) SHA1(7f15cae5c4aaa29638fb45029782dafd2b3d1484) )    /* RAS/CAS logic - not used */
ROM_END

ROM_START( mmonkey )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mmonkey.e4",   0xc000, 0x1000, CRC(8d31bf6a) SHA1(77b44d8e2b4db148727e7bfc5162c7e9e9cfc662) )
	ROM_LOAD( "mmonkey.d4",   0xd000, 0x1000, CRC(e54f584a) SHA1(a03fef09f6a0bb6802b33b28c45548efb85cda5c) )
	ROM_LOAD( "mmonkey.b4",   0xe000, 0x1000, CRC(399a161e) SHA1(0eb3c5031a7d8c7b14019e215b18dac24a9e70dd) )
	ROM_LOAD( "mmonkey.a4",   0xf000, 0x1000, CRC(f7d3d1e3) SHA1(ff650a833e5e8975fe5b4a644ce6c35de5e04740) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "mmonkey.h1",   0xe000, 0x1000, CRC(5bcb2e81) SHA1(60fb8fd83c83b278e3aaf96f0b6dbefbc1eef0f7) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "mmonkey.l11",  0x0000, 0x1000, CRC(b6aa8566) SHA1(bc90d4cfa9a221477d1989fea532621ce3e76439) )
	ROM_LOAD( "mmonkey.m11",  0x1000, 0x1000, CRC(6cc4d0c4) SHA1(f43450e97dd0c6d0a269c06e4c4253d0814590e9) )
	ROM_LOAD( "mmonkey.l13",  0x2000, 0x1000, CRC(2a343b7e) SHA1(1dba32a83db933096b9a9fbcfd8e0290aba76483) )
	ROM_LOAD( "mmonkey.m13",  0x3000, 0x1000, CRC(0230b50d) SHA1(d62b5d1be35c8bf29483fb616cd7e3949a422e76) )
	ROM_LOAD( "mmonkey.l14",  0x4000, 0x1000, CRC(922bb3e1) SHA1(72d2017e80bea7700a3a61a06882839ecffcabe8) )
	ROM_LOAD( "mmonkey.m14",  0x5000, 0x1000, CRC(f943e28c) SHA1(6ff536a21f34cbb958f6d0f84791102938966ff3) )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "mmi6331.m5",   0x0000, 0x0020, CRC(55e28b32) SHA1(b73f85224738252dc8dbb38a54250dcfe1fc3ae3) )    /* palette */
	ROM_LOAD( "sb-4c",        0x0020, 0x0020, CRC(a29b4204) SHA1(7f15cae5c4aaa29638fb45029782dafd2b3d1484) )    /* RAS/CAS logic - not used */
ROM_END

ROM_START( brubber )
	ROM_REGION( 0x10000, "maincpu", 0 )
	/* a000-bfff space for the service ROM */
	ROM_LOAD( "brubber.12c",  0xc000, 0x2000, CRC(b5279c70) SHA1(5fb1c50040dc4e9444aed440e2c3cf4c79b72311) )
	ROM_LOAD( "brubber.12d",  0xe000, 0x2000, CRC(b2ce51f5) SHA1(5e38ea24bcafef1faba023def96532abd6f97d38) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "bnj6c.bin",    0xe000, 0x1000, CRC(8c02f662) SHA1(1279d564e65fd3ccac25b1f9fbb40d910de2b544) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "bnj4e.bin",    0x0000, 0x2000, CRC(b864d082) SHA1(cacf71fa6c0f7121d077381a0ff6222f534295ab) )
	ROM_LOAD( "bnj4f.bin",    0x2000, 0x2000, CRC(6c31d77a) SHA1(5e52554f594f569527af4768d244cc40a7b4460a) )
	ROM_LOAD( "bnj4h.bin",    0x4000, 0x2000, CRC(5824e6fb) SHA1(e98f0eb476b8f033f5cc70a6e503afc4e651fd45) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "bnj10e.bin",   0x0000, 0x1000, CRC(f4e9eb49) SHA1(b356512d2ebd4e2005e76496b434e5ecebadb251) )
	ROM_LOAD( "bnj10f.bin",   0x1000, 0x1000, CRC(a9ffacb4) SHA1(49d5f9c0b695f474197fbb761bacc065b6b5808a) )
ROM_END


/*
    Bump 'n Jump (Data East USA)

    Sound Board:
        CIS-1
        DATA EAST-0136

    Video Board:
        DSP-12
        DE-0135-2
*/

ROM_START( bnj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ad08.12b",     0xa000, 0x2000, CRC(8d649bd5) SHA1(83105718c2d18ef75ca18ae92b34545cb939bc02) )
	ROM_LOAD( "ad07.12c",     0xc000, 0x2000, CRC(7a27f5f4) SHA1(f62d752bb7a995e120ed4d642793c543f0ef13ca) )
	ROM_LOAD( "ad06.12d",     0xe000, 0x2000, CRC(f855a2d2) SHA1(f231ed008537aeeeacbec64f485e9a96ab3441e1) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ad05.6c",      0xe000, 0x1000, CRC(8c02f662) SHA1(1279d564e65fd3ccac25b1f9fbb40d910de2b544) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "ad00.4e",      0x0000, 0x2000, CRC(b864d082) SHA1(cacf71fa6c0f7121d077381a0ff6222f534295ab) )
	ROM_LOAD( "ad01.4f",      0x2000, 0x2000, CRC(6c31d77a) SHA1(5e52554f594f569527af4768d244cc40a7b4460a) )
	ROM_LOAD( "ad02.4h",      0x4000, 0x2000, CRC(5824e6fb) SHA1(e98f0eb476b8f033f5cc70a6e503afc4e651fd45) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "ad03.10e",     0x0000, 0x1000, CRC(f4e9eb49) SHA1(b356512d2ebd4e2005e76496b434e5ecebadb251) )
	ROM_LOAD( "ad04.10f",     0x1000, 0x1000, CRC(a9ffacb4) SHA1(49d5f9c0b695f474197fbb761bacc065b6b5808a) )

	ROM_REGION( 0x002d, "plds", 0 )
	ROM_LOAD( "pb-5.10k.bin", 0x0000, 0x002c, CRC(dc72a65f) SHA1(d61c149d4df93a2074debf7c5e46557c6b06d10d) ) /* PAL10L8 */
	ROM_LOAD( "pb-4.2d.bin",  0x002c, 0x0001, NO_DUMP ) /* PAL16R4CN - same as Car Action? */
ROM_END

ROM_START( bnjm )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bnj12b.bin",   0xa000, 0x2000, CRC(ba3e3801) SHA1(56284076d938c33c1492a07281b936681eb09808) )
	ROM_LOAD( "bnj12c.bin",   0xc000, 0x2000, CRC(fb3a2cdd) SHA1(4a964389cc8035b9264d4cb133eb6d3826e74b95) )
	ROM_LOAD( "bnj12d.bin",   0xe000, 0x2000, CRC(b88bc99e) SHA1(08a4ddea4037f9e14d0d9f4262a1746b0a3a140c) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "bnj6c.bin",    0xe000, 0x1000, CRC(8c02f662) SHA1(1279d564e65fd3ccac25b1f9fbb40d910de2b544) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "bnj4e.bin",    0x0000, 0x2000, CRC(b864d082) SHA1(cacf71fa6c0f7121d077381a0ff6222f534295ab) )
	ROM_LOAD( "bnj4f.bin",    0x2000, 0x2000, CRC(6c31d77a) SHA1(5e52554f594f569527af4768d244cc40a7b4460a) )
	ROM_LOAD( "bnj4h.bin",    0x4000, 0x2000, CRC(5824e6fb) SHA1(e98f0eb476b8f033f5cc70a6e503afc4e651fd45) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "bnj10e.bin",   0x0000, 0x1000, CRC(f4e9eb49) SHA1(b356512d2ebd4e2005e76496b434e5ecebadb251) )
	ROM_LOAD( "bnj10f.bin",   0x1000, 0x1000, CRC(a9ffacb4) SHA1(49d5f9c0b695f474197fbb761bacc065b6b5808a) )
ROM_END

ROM_START( caractn )
	ROM_REGION( 0x10000, "maincpu", 0 )
	/* a000-bfff space for the service ROM */
	ROM_LOAD( "c7.12c",  0xc000, 0x2000, CRC(b5279c70) SHA1(5fb1c50040dc4e9444aed440e2c3cf4c79b72311) )
	ROM_LOAD( "c6.12d",  0xe000, 0x2000, CRC(1d6957c4) SHA1(bd30f00187e56eef9adcc167dd752a3bb616454c) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "c5.6c",   0xe000, 0x1000, CRC(8c02f662) SHA1(1279d564e65fd3ccac25b1f9fbb40d910de2b544) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "c0.4e",   0x0000, 0x2000, CRC(bf3ea732) SHA1(d98970b2dda8c3435506656909e5e3aa70d45652) )
	ROM_LOAD( "c1.4f",   0x2000, 0x2000, CRC(9789f639) SHA1(77a4d494698718c052fa1967242a0e4fa263b6ad) )
	ROM_LOAD( "c2.4h",   0x4000, 0x2000, CRC(51dcc111) SHA1(9753d682ba2f4fb4d3b14783ac35ad214bf788b5) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "c3.10e",  0x0000, 0x1000, CRC(f4e9eb49) SHA1(b356512d2ebd4e2005e76496b434e5ecebadb251) )
	ROM_LOAD( "c4.10f",  0x1000, 0x1000, CRC(a9ffacb4) SHA1(49d5f9c0b695f474197fbb761bacc065b6b5808a) )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "tbp18s030.11a",   0x0000, 0x020, CRC(318d25b9) SHA1(9a82619c94f5911d01ddf6b85f7e30cdc6f1d0a3) )  /* palette */
	ROM_LOAD( "tbp18s030.cpu",   0x0020, 0x020, CRC(6b0c2942) SHA1(7d25acc753923b265792fc78f8fc70175c0e0ec2) )  /* RAS/CAS logic - not used */

	ROM_REGION( 0x0140, "plds", 0 )
	ROM_LOAD( "pal10l8.10k",   0x0000, 0x002c, CRC(dc72a65f) SHA1(d61c149d4df93a2074debf7c5e46557c6b06d10d) )
	ROM_LOAD( "pal16r4a.2d",   0x0030, 0x0104, CRC(fd1f3aa2) SHA1(67f1e74fcfc0e2301204ed58b8c6e35d4866a344) )
ROM_END

ROM_START( caractn2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	/* a000-bfff space for the service ROM */
	ROM_LOAD( "7.c12",   0xc000, 0x2000, CRC(406086aa) SHA1(711d547eeb73044930fb1fd15060dbd1e85339d6) ) /* 2 bytes difference, Lives DIP 2/3 instead of 3/5 */
	ROM_LOAD( "c6.12d",  0xe000, 0x2000, CRC(1d6957c4) SHA1(bd30f00187e56eef9adcc167dd752a3bb616454c) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "c5.6c",   0xe000, 0x1000, CRC(8c02f662) SHA1(1279d564e65fd3ccac25b1f9fbb40d910de2b544) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "c0.4e",   0x0000, 0x2000, CRC(bf3ea732) SHA1(d98970b2dda8c3435506656909e5e3aa70d45652) )
	ROM_LOAD( "c1.4f",   0x2000, 0x2000, CRC(9789f639) SHA1(77a4d494698718c052fa1967242a0e4fa263b6ad) )
	ROM_LOAD( "c2.4h",   0x4000, 0x2000, CRC(51dcc111) SHA1(9753d682ba2f4fb4d3b14783ac35ad214bf788b5) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "c3.10e",  0x0000, 0x1000, CRC(f4e9eb49) SHA1(b356512d2ebd4e2005e76496b434e5ecebadb251) )
	ROM_LOAD( "c4.10f",  0x1000, 0x1000, CRC(a9ffacb4) SHA1(49d5f9c0b695f474197fbb761bacc065b6b5808a) )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "tbp18s030.11a",   0x0000, 0x020, CRC(318d25b9) SHA1(9a82619c94f5911d01ddf6b85f7e30cdc6f1d0a3) )  /* palette */
	ROM_LOAD( "tbp18s030.cpu",   0x0020, 0x020, CRC(6b0c2942) SHA1(7d25acc753923b265792fc78f8fc70175c0e0ec2) )  /* RAS/CAS logic - not used */

	ROM_REGION( 0x0140, "plds", 0 )
	ROM_LOAD( "pal10l8.10k",   0x0000, 0x002c, CRC(dc72a65f) SHA1(d61c149d4df93a2074debf7c5e46557c6b06d10d) )
	ROM_LOAD( "pal16r4a.2d",   0x0030, 0x0104, CRC(fd1f3aa2) SHA1(67f1e74fcfc0e2301204ed58b8c6e35d4866a344) )
ROM_END

ROM_START( zoar )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "z15.12b", 0xd000, 0x1000, CRC(1f0cfdb7) SHA1(ce7e871f17c52b6eaf99cfb721e702e4f0e6bb25) )
	ROM_LOAD( "z16.13b", 0xe000, 0x1000, CRC(7685999c) SHA1(fabe38d71e797ae0b04b5d3aba228b4c85d96185) )
	ROM_LOAD( "z17.15b", 0xf000, 0x1000, CRC(619ea867) SHA1(0a3735384f03a1052d54ab799b5e37038d8ece2a) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "z09.13c", 0xe000, 0x1000, CRC(18d96ff1) SHA1(671d934a451e0b042450ea86d24c3751a39b38f8) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "z00.3l",  0x0000, 0x1000, CRC(fd2dcb64) SHA1(1a49a6ec6ffd354d872b1af83d55ec96e8215b2b) )
	ROM_LOAD( "z01.5l",  0x1000, 0x1000, CRC(74d3ca48) SHA1(2c75ea246f86a057467deb35ef6a6e72f667dd84) )
	ROM_LOAD( "z03.8l",  0x2000, 0x1000, CRC(77b7df14) SHA1(a1cbc214fc849b7e3417b1156d1e4440ab67f631) )
	ROM_LOAD( "z04.9l",  0x3000, 0x1000, CRC(9be786de) SHA1(480733a1438dffa4b0fac6f76bf84a0deec5d1fa) )
	ROM_LOAD( "z06.12l", 0x4000, 0x1000, CRC(07638c71) SHA1(1a7fc49657ac7ac0033bd60c86663bd615079230) )
	ROM_LOAD( "z07.14l", 0x5000, 0x1000, CRC(f4710f25) SHA1(08b4cc4252f83a689cded38d9a5a50f55ee6beee) )

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "z10.1b",  0x0000, 0x0800, CRC(aa8bcab8) SHA1(81f1a9fd754fd6f8030ff6b5aa80c7670be9d02e) )
	ROM_LOAD( "z11.3b",  0x0800, 0x0800, CRC(dcdad357) SHA1(d1569e1d38f14f5f457547e24df4f80f726c6157) )
	ROM_LOAD( "z12.4b",  0x1000, 0x0800, CRC(ed317e40) SHA1(db70889af5f233ca71acf734abfbdb74b6a393c0) )

	ROM_REGION( 0x3000, "gfx3", 0 )
	ROM_LOAD( "z02.6l",  0x0000, 0x1000, CRC(d8c3c122) SHA1(841006cc84622e851df462a64696b64bb8cb62a1) )
	ROM_LOAD( "z05.14l", 0x1000, 0x1000, CRC(05dc6b09) SHA1(197c720544a090e12980513b441a2b9cf04e212f) )
	ROM_LOAD( "z08.15l", 0x2000, 0x1000, CRC(9a148551) SHA1(db92dd7552c6f76a062910f37a3fe3524fdffd38) )

	ROM_REGION( 0x1000, "bg_map", 0 )   /* background tilemaps */
	ROM_LOAD( "z13.6b",  0x0000, 0x1000, CRC(8fefa960) SHA1(614026aa71703dd3898e470f45730e5c6934b31b) )

	ROM_REGION( 0x0060, "proms", 0 )
	ROM_LOAD( "z20.1l",  0x0000, 0x0020, CRC(a63f0a07) SHA1(16532d3ac0536ad4b712005fd722ee8c14d02e9b) )
	ROM_LOAD( "z21.2l",  0x0020, 0x0020, CRC(5e1e5788) SHA1(56068b209cc7c734bbcbb9858f40faa6474c8095) )
	ROM_LOAD( "z19.7b",  0x0040, 0x0020, CRC(03ee3a96) SHA1(4acb4061ef0d8a1fab50207fc81a54bfa4c7455d) )
ROM_END

ROM_START( disco )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "disco.w5",     0xa000, 0x1000, CRC(b2c87b78) SHA1(4095f0052ff0ac35ecd2ec1c1e99d21283d336e1) )
	ROM_LOAD( "disco.w4",     0xb000, 0x1000, CRC(ad7040ee) SHA1(287a4ff06edda4c66e2351e49a94212728aacb4e) )
	ROM_LOAD( "disco.w3",     0xc000, 0x1000, CRC(12fb4f08) SHA1(d6095f20d8676df89b1459134b5521ac311ddded) )
	ROM_LOAD( "disco.w2",     0xd000, 0x1000, CRC(73f6fb2f) SHA1(7b75b825d9bf7e512e054762500f79c18a276e1f) )
	ROM_LOAD( "disco.w1",     0xe000, 0x1000, CRC(ee7b536b) SHA1(b2de5da15cee1d80391eafd0a08361803f859c89) )
	ROM_LOAD( "disco.w0",     0xf000, 0x1000, CRC(7c26e76b) SHA1(952e91c4acc18d01b0e2c3efd764da8768f583da) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "disco.w6",     0xf000, 0x1000, CRC(d81e781e) SHA1(bde510bfed06a13bd56bf7ddbf220e7cf82f79b6) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "disco.clr",    0x0000, 0x0020, CRC(a393f913) SHA1(42dce159283427064b3f5ce3a6e2189744ecd943) )
ROM_END

ROM_START( discof )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "w5-f.1a",     0xa000, 0x1000, CRC(9d53c71c) SHA1(53c410cfa4fbbfd08e1c3cf7aeba1c9627171a71) )
	ROM_LOAD( "w4-f.2a",     0xb000, 0x1000, CRC(c1f8d747) SHA1(33f5fe73d1851ef4da670075d1aec1550e0417ce) )
	ROM_LOAD( "w3-f.4a",     0xc000, 0x1000, CRC(9aadd252) SHA1(c6da7ef46333d525e676c59f03ccc908108b41ba) )
	ROM_LOAD( "w2-f.6a",     0xd000, 0x1000, CRC(f131a5bb) SHA1(84b7dea112dce12e5cb235a13f6dc4edcfb18c06) )
	ROM_LOAD( "w1-f.9a",     0xe000, 0x1000, CRC(a6ce9a19) SHA1(e8f380e17a21fb33504d6efe9d01d0f903fa25e1) )
//  ROM_LOAD( "w1-f",        0xe000, 0x1000, CRC(c8ec57c5) SHA1(904a9ed0a7f1230c611bf473b9bc52e63eb56dbe) ) // 0x7d3 is 0x10 instead of 0x00, 1 bit different, looks out of place, bad?
	ROM_LOAD( "w0-f.9a",     0xf000, 0x1000, CRC(b3787a92) SHA1(7f40621dc739c1108a5df43142ab04709a380219) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "w6-.1b",     0xf000, 0x1000, CRC(d81e781e) SHA1(bde510bfed06a13bd56bf7ddbf220e7cf82f79b6) )

	ROM_REGION( 0x0020, "proms", 0 ) // board uses 2 proms, not 1
	ROM_LOAD( "disco.clr",    0x0000, 0x0020, CRC(a393f913) SHA1(42dce159283427064b3f5ce3a6e2189744ecd943) )
ROM_END

ROM_START( sdtennis )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ao_08.12b",  0xa000, 0x2000, CRC(6193724c) SHA1(97239c5aa8c8cd1812fba1b15be4d9a48eb0651a) )
	ROM_LOAD( "ao_07.12c",  0xc000, 0x2000, CRC(064888db) SHA1(f7bb728ab3408bb553191d9e131a441db1b39666) )
	ROM_LOAD( "ao_06.12d",  0xe000, 0x2000, CRC(413c984c) SHA1(1431df4db52d621ba39fd47dbd49da103b5c0bcf) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ao_05.6c",    0xe000, 0x1000, CRC(46833e38) SHA1(420831149a566199d6a3c74ef3df0687b4ddcbe4) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "ao_00.4e",    0x0000, 0x2000, CRC(f4e0cbd6) SHA1(a2ede0ce4a26957a5d3b62872a42b8979f5000aa) )
	ROM_LOAD( "ao_01.4f",    0x2000, 0x2000, CRC(f99029da) SHA1(45bc56ff6284d02371d5e1cd5239be665f9e56c7) )
	ROM_LOAD( "ao_02.4h",    0x4000, 0x2000, CRC(c3077555) SHA1(addfc67735dc22dfed9c4c4ec8d9dcf590c76737) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "ao_03.10e",   0x0000, 0x1000, CRC(1977db9b) SHA1(d175974967fdeb608df668089fa2a14b2d1609e6) )
	ROM_LOAD( "ao_04.10f",   0x1000, 0x1000, CRC(921952af) SHA1(4e9248f3493a5f4651278f27c11f507571242317) )
ROM_END

READ8_MEMBER(btime_state::wtennis_reset_hack_r)
{
	UINT8 *RAM = memregion("maincpu")->base();

	/* Otherwise the game goes into test mode and there is no way out that I
	   can see.  I'm not sure how it can work, it probably somehow has to do
	   with the tape system */

	RAM[0xfc30] = 0;

	return RAM[0xc15f];
}

DRIVER_INIT_MEMBER(btime_state,btime)
{
	m_audio_nmi_enable_type = AUDIO_ENABLE_DIRECT;
}

DRIVER_INIT_MEMBER(btime_state,zoar)
{
	UINT8 *rom = memregion("maincpu")->base();

	/* At location 0xD50A is what looks like an undocumented opcode. I tried
	   implementing it given what opcode 0x23 should do, but it still didn't
	   work in demo mode. So this could be another protection or a bad ROM read.
	   I'm NOPing it out for now. */
	memset(&rom[0xd50a],0xea,8);

	m_audio_nmi_enable_type = AUDIO_ENABLE_AY8910;
}

DRIVER_INIT_MEMBER(btime_state,tisland)
{
	UINT8 *rom = memregion("maincpu")->base();

	/* At location 0xa2b6 there's a strange RLA followed by a BPL that reads from an
	   unmapped area that causes the game to fail in several circumstances.On the Cassette
	   version the RLA (33) is in reality a BIT (24),so I'm guessing that there's something
	   wrong going on in the encryption scheme.*/
	memset(&rom[0xa2b6],0x24,1);

	m_audio_nmi_enable_type = AUDIO_ENABLE_DIRECT;
}

DRIVER_INIT_MEMBER(btime_state,lnc)
{
	m_audio_nmi_enable_type = AUDIO_ENABLE_AY8910;
}

DRIVER_INIT_MEMBER(btime_state,bnj)
{
	m_audio_nmi_enable_type = AUDIO_ENABLE_DIRECT;
}

DRIVER_INIT_MEMBER(btime_state,disco)
{
	DRIVER_INIT_CALL(btime);
	m_audio_nmi_enable_type = AUDIO_ENABLE_AY8910;
}

DRIVER_INIT_MEMBER(btime_state,cookrace)
{
	m_audiocpu->space(AS_PROGRAM).install_read_bank(0x0200, 0x0fff, "bank10");
	membank("bank10")->set_base(memregion("audiocpu")->base() + 0xe200);
	m_audio_nmi_enable_type = AUDIO_ENABLE_DIRECT;
}

DRIVER_INIT_MEMBER(btime_state,protennb)
{
	DRIVER_INIT_CALL(btime);
	m_audio_nmi_enable_type = AUDIO_ENABLE_AY8910;
}

DRIVER_INIT_MEMBER(btime_state,wtennis)
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xc15f, 0xc15f, read8_delegate(FUNC(btime_state::wtennis_reset_hack_r),this));

	m_audiocpu->space(AS_PROGRAM).install_read_bank(0x0200, 0x0fff, "bank10");
	membank("bank10")->set_base(memregion("audiocpu")->base() + 0xe200);
	m_audio_nmi_enable_type = AUDIO_ENABLE_AY8910;
}

DRIVER_INIT_MEMBER(btime_state,sdtennis)
{
	m_audio_nmi_enable_type = AUDIO_ENABLE_DIRECT;
}


GAME( 1982, btime,    0,       btime,    btime, btime_state,    btime,    ROT270, "Data East Corporation", "Burger Time (Data East set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, btime2,   btime,   btime,    btime, btime_state,    btime,    ROT270, "Data East Corporation", "Burger Time (Data East set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, btime3,   btime,   btime,    btime, btime_state,    btime,    ROT270, "Data East USA Inc.",    "Burger Time (Data East USA)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, btimem,   btime,   btime,    btime, btime_state,    btime,    ROT270, "Data East (Bally Midway license)", "Burger Time (Midway)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, cookrace, btime,   cookrace, cookrace, btime_state, cookrace, ROT270, "bootleg", "Cook Race", MACHINE_SUPPORTS_SAVE )
GAME( 1981, tisland,  0,       tisland,  btime, btime_state,    tisland,  ROT270, "Data East Corporation", "Treasure Island", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1981, lnc,      0,       lnc,      lnc, btime_state,      lnc,      ROT270, "Data East Corporation", "Lock'n'Chase", MACHINE_SUPPORTS_SAVE )
GAME( 1982, protennb, 0,       disco,    disco, btime_state,    protennb, ROT270, "bootleg", "Tennis (bootleg of Pro Tennis)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, wtennis,  0,       wtennis,  wtennis, btime_state,  wtennis,  ROT270, "bootleg", "World Tennis", MACHINE_SUPPORTS_SAVE )
GAME( 1982, mmonkey,  0,       mmonkey,  mmonkey, btime_state,  lnc,      ROT270, "Technos Japan / Roller Tron", "Minky Monkey", MACHINE_SUPPORTS_SAVE )
GAME( 1982, brubber,  0,       bnj,      bnj, btime_state,      bnj,      ROT270, "Data East", "Burnin' Rubber", MACHINE_SUPPORTS_SAVE )
GAME( 1982, bnj,      brubber, bnj,      bnj, btime_state,      bnj,      ROT270, "Data East USA", "Bump 'n' Jump", MACHINE_SUPPORTS_SAVE )
GAME( 1982, bnjm,     brubber, bnj,      bnj, btime_state,      bnj,      ROT270, "Data East USA (Bally Midway license)", "Bump 'n' Jump (Midway)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, caractn,  brubber, bnj,      bnj, btime_state,      bnj,      ROT270, "bootleg", "Car Action (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, caractn2, brubber, bnj,      caractn2, btime_state, bnj,      ROT270, "bootleg", "Car Action (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, zoar,     0,       zoar,     zoar, btime_state,     zoar,     ROT270, "Data East USA", "Zoar", MACHINE_SUPPORTS_SAVE )
GAME( 1982, disco,    0,       disco,    disco, btime_state,    disco,    ROT270, "Data East", "Disco No.1", MACHINE_SUPPORTS_SAVE )
GAME( 1982, discof,   disco,   disco,    disco, btime_state,    disco,    ROT270, "Data East", "Disco No.1 (Rev.F)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, sdtennis, 0,       sdtennis, sdtennis, btime_state, sdtennis, ROT270, "Data East Corporation", "Super Doubles Tennis", MACHINE_SUPPORTS_SAVE )
