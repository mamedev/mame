// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

Crazy Climber memory map (preliminary)
as described by Lionel Theunissen (lionelth@ozemail.com.au)

Crazy Kong is very similar to Crazy Climber, there is an additional ROM at
5000-5fff and RAM is at 6000-6bff. Dip switches and input connections are
different as well.

Swimmer is similar but also different (e.g. it has two CPUs and two 8910,
graphics are 3bpp instead of 2)

0000h-4fffh ;20k program ROMs. ROM11=0000h
                               ROM10=1000h
                               ROM09=2000h
                               ROM08=3000h
                               ROM07=4000h

8000h-83ffh ;1k scratchpad RAM.
8800h-88ffh ;256 bytes Bigsprite RAM.
9000h-93ffh ;1k screen RAM.
9800h-981fh ;Column smooth scroll position. Corresponds to each char
             column.

9880h-989fh ;Sprite controls. 8 groups of 4 bytes:
  1st byte; code/attribute.
            Bits 0-5: sprite code.
            Bit    6: x invert.
            Bit    7: y invert.
  2nd byte ;color.
            Bits 0-3: colour. (palette scheme 0-15)
            Bit    4: 0=charset1, 1 =charset 2.
  3rd byte ;y position
  4th byte ;x position

98dc        bit 0  big sprite priority over sprites? (1 = less priority)
98ddh ;Bigsprite colour/attribute.
            Bit 0-2: Big sprite colour.
            bit 3  ??
            Bit   4: x invert.
            Bit   5: y invert.
98deh ;Bigsprite y position.
98dfh ;Bigsprite x position.

9c00h-9fffh ;1/2k colour RAM: Bits 0-3: colour. (palette scheme 0-15)
                              Bit    4: 0=charset1, 1=charset2.
                              Bit    5: (not used by CC)
                              Bit    6: x invert.
                              Bit    7: y invert. (not used by CC)

a000h ;RD: Player 1 controls.
            Bit 0: Left up
                1: Left down
                2: Left left
                3: Left right
                4: Right up
                5: Right down
                6: Right left
                7: Right right

a000h ;WR: Non Maskable interrupt.
            Bit 0: 0=NMI disable, 1=NMI enable.

a001h ;WR: Horizontal video direction (Crazy Kong sets it to 1).
            Bit 0: 0=Normal, 1=invert.

a002h ;WR: Vertical video direction (Crazy Kong sets it to 1).
            Bit 0: 0=Normal, 1=invert.

a004h ;WR: Sample trigger.
            Bit 0: 0=Trigger.

a800h ;RD: Player 2 controls (table model only).
            Bit 0: Left up
                1: Left down
                2: Left left
                3: Left right
                4: Right up
                5: Right down
                6: Right left
                7: Right right


a800h ;WR: Sample rate speed.
              Full byte value (0-255).

b000h ;RD: DIP switches.
            Bit 1,0: Number of climbers.
                     00=3, 01=4, 10=5, 11=6.
            Bit   2: Extra climber bonus.
                     0=30000, 1=50000.
            Bit   3: 1=Test Pattern
            Bit 5,4: Coins per credit.
                     00=1, 01=2, 10=3 11=4.
            Bit 7,6: Plays per credit.
                     00=1, 01=2, 10=3, 11=Freeplay.

b000h ;WR: Sample volume.
            Bits 0-5: Volume (0-31).

b800h ;RD: Machine switches.
            Bit 0: Coin 1.
            Bit 1: Coin 2.
            Bit 2: 1 Player start.
            Bit 3: 2 Player start.
            Bit 4: Upright/table select.
                   0=table, 1=upright.


I/O 8  ;AY-3-8910 Control Reg.
I/O 9  ;AY-3-8910 Data Write Reg.
I/O C  ;AY-3-8910 Data Read Reg.
        Port A of the 8910 selects the digital sample to play

Changes:
25 Jan 98 LBO
        * Added support for the real Swimmer bigsprite ROMs, courtesy of Gary Walton.
        * Increased the IRQs for the Swimmer audio CPU to 4 to make it more "jaunty".
          Not sure if this is accurate, but it should be closer.
3 Mar 98 LBO
        * Added alternate version of Swimmer.

TODO:
        * Verify timings of sound/music on Swimmer.
        * Add tms5110 support to bagmanf


-------------------------------------------------------------------

    T.S. 17.12.2005:

    Yamato:
    -------
     Added temporary bg gradient (bad colors/offset).

     Gradient table are stored in two(?) ROMs.
     Each table is 256 bytes long: 128 for normal
     and 128 bytes for flipped screen.
     Color format is unknown - probably direct RGB
     mapping of 8 or 16 (both roms) bits. Also table
     selection source is unknown.

     TODO:
      - bg gradient color decode & table selection


 Top Roller:
 ----------
     It's made by the same developers as Yamato and use
     probably the same encrypted SEGA cpu as Yamato.

     lives - $6155

     TODO:

       - COINB DSW is missing
       - few issues in cocktail mode
       - wrong colors (fg text layer) - game sometimes ("round" text , lives) updates only even columns of cell attribs...

-------------------------------------------------------------------


 Top Roller
 Jaleco

 Hardware : Original Jaleco board no 8307-B/8307-A(redump)

 Main CPU : Encrypted Z80 (probably 315-5018)
 Sound : AY-3-8910

 ROMS CRC32 + positions :

 [9894374d]  d5
 [ef789f00]  f5
 [d45494ba]  h5
 [1cb48ea0]  k5
 [84139f46]  l5
 [e30c1dd8]  m5
 [904fffb6]  d3
 [94371cfb]  f3
 [8a8032a7]  h3
 [1e8914a6]  k3
 [b20a9fa2]  l3
 [7f989dc9]  p3
 [89327329]  a4 bottom board 89327329
 [7a945733]  c4 bottom board
 [5f2c2a78]  h4 bottom board  bad dump / [1d9e3325] (8307-A)
 [ce3afe26]  j4 bottom board

----

2008-07
Dip location verified from manual for: cclimber, guzzler, swimmer

 Cannon Ball
 -----------

 The Cannon Ball bootlegs on this Falcon (Crazy Kong) hardware don't correctly
 handle the protection device found on the original pacman hardware conversion,
 this causes them to crash after the a few rounds - confirmed on an original PCB.
 They clearly weren't tested properly by the bootleggers.

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/segacrpt.h"
#include "sound/ay8910.h"
#include "sound/samples.h"
#include "audio/cclimber.h"
#include "includes/cclimber.h"


#define MASTER_CLOCK            XTAL_18_432MHz


void cclimber_state::machine_start()
{
	save_item(NAME(m_nmi_mask));
}

WRITE8_MEMBER(cclimber_state::swimmer_sh_soundlatch_w)
{
	soundlatch_byte_w(space,offset,data);
	m_audiocpu->set_input_line_and_vector(0, HOLD_LINE, 0xff);
}


WRITE8_MEMBER(cclimber_state::yamato_p0_w)
{
	m_yamato_p0 = data;
}

WRITE8_MEMBER(cclimber_state::yamato_p1_w)
{
	m_yamato_p1 = data;
}

READ8_MEMBER(cclimber_state::yamato_p0_r)
{
	return m_yamato_p0;
}

READ8_MEMBER(cclimber_state::yamato_p1_r)
{
	return m_yamato_p1;
}


WRITE8_MEMBER(cclimber_state::toprollr_rombank_w)
{
	m_toprollr_rombank &= ~(1 << offset);
	m_toprollr_rombank |= (data & 1) << offset;

	if (m_toprollr_rombank < 3) {
		membank("bank1")->set_entry(m_toprollr_rombank);
		membank("bank1d")->set_entry(m_toprollr_rombank);
	}
}

MACHINE_RESET_MEMBER(cclimber_state,cclimber)
{
	/* Disable interrupts, River Patrol / Silver Land needs this otherwise returns bad RAM on POST */
	m_nmi_mask = 0;

	m_toprollr_rombank = 0;
}


WRITE8_MEMBER(cclimber_state::nmi_mask_w)
{
	m_nmi_mask = data & 1;
}


/* Note that River Patrol reads/writes to a000-a4f0. This is a bug in the code.
   The instruction at 0x0593 should say LD DE,$8000 */

static ADDRESS_MAP_START( cclimber_map, AS_PROGRAM, 8, cclimber_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x6000, 0x6bff) AM_RAM             /* Crazy Kong only */
	AM_RANGE(0x8000, 0x83ff) AM_RAM
	AM_RANGE(0x8800, 0x88ff) AM_RAM AM_SHARE("bigspriteram")
	AM_RANGE(0x8900, 0x8bff) AM_RAM             /* not used, but initialized */
	AM_RANGE(0x9000, 0x93ff) AM_MIRROR(0x0400) AM_RAM AM_SHARE("videoram")
	/* 9800-9bff and 9c00-9fff share the same RAM, interleaved */
	/* (9800-981f for scroll, 9c20-9c3f for color RAM, and so on) */
	AM_RANGE(0x9800, 0x981f) AM_RAM AM_SHARE("column_scroll")
	AM_RANGE(0x9880, 0x989f) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x98dc, 0x98df) AM_RAM AM_SHARE("bigspritectrl")
	AM_RANGE(0x9800, 0x9bff) AM_RAM  /* not used, but initialized */
	AM_RANGE(0x9c00, 0x9fff) AM_RAM_WRITE(cclimber_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0xa000, 0xa000) AM_READ_PORT("P1") AM_WRITE(nmi_mask_w)
	AM_RANGE(0xa001, 0xa002) AM_WRITEONLY AM_SHARE("flip_screen")
	AM_RANGE(0xa003, 0xa003) AM_WRITE(nmi_mask_w) //used by Crazy Kong Bootleg with alt levels and speed up
	AM_RANGE(0xa004, 0xa004) AM_DEVWRITE("cclimber_audio", cclimber_audio_device, sample_trigger_w)
	AM_RANGE(0xa800, 0xa800) AM_READ_PORT("P2") AM_DEVWRITE("cclimber_audio", cclimber_audio_device, sample_rate_w)
	AM_RANGE(0xb000, 0xb000) AM_READ_PORT("DSW") AM_DEVWRITE("cclimber_audio", cclimber_audio_device, sample_volume_w)
	AM_RANGE(0xb800, 0xb800) AM_READ_PORT("SYSTEM")
ADDRESS_MAP_END

static ADDRESS_MAP_START( decrypted_opcodes_map, AS_DECRYPTED_OPCODES, 8, cclimber_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROM AM_SHARE("decrypted_opcodes")
ADDRESS_MAP_END

static ADDRESS_MAP_START( cannonb_map, AS_PROGRAM, 8, cclimber_state )
	AM_RANGE(0x5045, 0x505f) AM_WRITENOP        /* do not errorlog this */
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x6000, 0x6bff) AM_RAM
	AM_RANGE(0x8000, 0x83ff) AM_RAM
	AM_RANGE(0x8800, 0x88ff) AM_READNOP AM_WRITEONLY AM_SHARE("bigspriteram") /* must not return what's written (game will reset after coin insert if it returns 0xff)*/
//  AM_RANGE(0x8900, 0x8bff) AM_WRITEONLY  /* not used, but initialized */
	AM_RANGE(0x9000, 0x93ff) AM_MIRROR(0x0400) AM_RAM AM_SHARE("videoram")
	/* 9800-9bff and 9c00-9fff share the same RAM, interleaved */
	/* (9800-981f for scroll, 9c20-9c3f for color RAM, and so on) */
	AM_RANGE(0x9800, 0x981f) AM_RAM AM_SHARE("column_scroll")
	AM_RANGE(0x9880, 0x989f) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x98dc, 0x98df) AM_RAM AM_SHARE("bigspritectrl")
	AM_RANGE(0x9800, 0x9bff) AM_RAM  /* not used, but initialized */
	AM_RANGE(0x9c00, 0x9fff) AM_RAM_WRITE(cclimber_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0xa000, 0xa000) AM_READ_PORT("P1") AM_WRITE(nmi_mask_w)
	AM_RANGE(0xa001, 0xa002) AM_WRITE(cannonb_flip_screen_w) AM_SHARE("flip_screen")
	AM_RANGE(0xa004, 0xa004) AM_DEVWRITE("cclimber_audio", cclimber_audio_device, sample_trigger_w)
	AM_RANGE(0xa800, 0xa800) AM_READ_PORT("P2") AM_DEVWRITE("cclimber_audio", cclimber_audio_device, sample_rate_w)
	AM_RANGE(0xb000, 0xb000) AM_READ_PORT("DSW") AM_DEVWRITE("cclimber_audio", cclimber_audio_device, sample_volume_w)
	AM_RANGE(0xb800, 0xb800) AM_READ_PORT("SYSTEM")
ADDRESS_MAP_END

static ADDRESS_MAP_START( swimmer_map, AS_PROGRAM, 8, cclimber_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0x8800, 0x88ff) AM_MIRROR(0x0100) AM_RAM AM_SHARE("bigspriteram")
	AM_RANGE(0x9000, 0x93ff) AM_MIRROR(0x0400) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0x9800, 0x981f) AM_WRITEONLY AM_SHARE("column_scroll")
	AM_RANGE(0x9880, 0x989f) AM_WRITEONLY AM_SHARE("spriteram")
	AM_RANGE(0x98fc, 0x98ff) AM_WRITEONLY AM_SHARE("bigspritectrl")
	AM_RANGE(0x9c00, 0x9fff) AM_RAM_WRITE(cclimber_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0xa000, 0xa000) AM_READ_PORT("P2") AM_WRITE(nmi_mask_w)
	AM_RANGE(0xa001, 0xa002) AM_WRITEONLY AM_SHARE("flip_screen")
	AM_RANGE(0xa003, 0xa003) AM_WRITEONLY AM_SHARE("sidebg_enable")
	AM_RANGE(0xa004, 0xa004) AM_WRITEONLY AM_SHARE("palettebank")
	AM_RANGE(0xa800, 0xa800) AM_READ_PORT("P1") AM_WRITE(swimmer_sh_soundlatch_w)
	AM_RANGE(0xb000, 0xb000) AM_READ_PORT("DSW1")
	AM_RANGE(0xb800, 0xb800) AM_READ_PORT("DSW2") AM_WRITEONLY AM_SHARE("bgcolor")
	AM_RANGE(0xb880, 0xb880) AM_READ_PORT("SYSTEM")
ADDRESS_MAP_END

static ADDRESS_MAP_START( guzzler_map, AS_PROGRAM, 8, cclimber_state )
	AM_IMPORT_FROM(swimmer_map)
	AM_RANGE(0xc000, 0xc7ff) AM_RAM                 /* ??? used by Guzzler */
	AM_RANGE(0xe000, 0xffff) AM_ROM                 /* Guzzler only */
ADDRESS_MAP_END

static ADDRESS_MAP_START( yamato_map, AS_PROGRAM, 8, cclimber_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x6000, 0x6fff) AM_RAM
	AM_RANGE(0x7000, 0x7fff) AM_ROM
	AM_RANGE(0x8800, 0x88ff) AM_RAM AM_SHARE("bigspriteram")
	AM_RANGE(0x8900, 0x8bff) AM_RAM             /* not used, but initialized */
	AM_RANGE(0x9000, 0x93ff) AM_MIRROR(0x0400) AM_RAM AM_SHARE("videoram")
	/* 9800-9bff and 9c00-9fff share the same RAM, interleaved */
	/* (9800-981f for scroll, 9c20-9c3f for color RAM, and so on) */
	AM_RANGE(0x9800, 0x981f) AM_RAM AM_SHARE("column_scroll")
	AM_RANGE(0x9880, 0x989f) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x98dc, 0x98df) AM_RAM AM_SHARE("bigspritectrl")
	AM_RANGE(0x9800, 0x9bff) AM_RAM  /* not used, but initialized */
	AM_RANGE(0x9c00, 0x9fff) AM_RAM_WRITE(cclimber_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0xa000, 0xa000) AM_READ_PORT("P1") AM_WRITE(nmi_mask_w)
	AM_RANGE(0xa001, 0xa002) AM_WRITEONLY AM_SHARE("flip_screen")
	AM_RANGE(0xa800, 0xa800) AM_READ_PORT("P2")
	AM_RANGE(0xb000, 0xb000) AM_READ_PORT("DSW")
	AM_RANGE(0xb800, 0xb800) AM_READ_PORT("COIN")
	AM_RANGE(0xba00, 0xba00) AM_READ_PORT("START")  /* maybe a mirror of b800 */
ADDRESS_MAP_END

static ADDRESS_MAP_START( toprollr_map, AS_PROGRAM, 8, cclimber_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROMBANK("bank1")
	AM_RANGE(0x6000, 0x6bff) AM_RAM AM_SHARE("ram")
	AM_RANGE(0x8800, 0x88ff) AM_RAM AM_SHARE("bigspriteram")
	AM_RANGE(0x8c00, 0x8fff) AM_RAM AM_SHARE("bg_videoram")
	AM_RANGE(0x9000, 0x93ff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0x9400, 0x97ff) AM_RAM AM_SHARE("bg_coloram")
	AM_RANGE(0x9800, 0x987f) AM_RAM /* unused ? */
	AM_RANGE(0x9880, 0x995f) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x99dc, 0x99df) AM_RAM AM_SHARE("bigspritectrl")
	AM_RANGE(0x9c00, 0x9fff) AM_RAM AM_SHARE("colorram")
	AM_RANGE(0xa000, 0xa000) AM_READ_PORT("P1") AM_WRITE(nmi_mask_w)
	AM_RANGE(0xa001, 0xa002) AM_WRITEONLY AM_SHARE("flip_screen")
	AM_RANGE(0xa004, 0xa004) AM_DEVWRITE("cclimber_audio", cclimber_audio_device,  sample_trigger_w)
	AM_RANGE(0xa005, 0xa006) AM_WRITE(toprollr_rombank_w)
	AM_RANGE(0xa800, 0xa800) AM_READ_PORT("P2") AM_DEVWRITE("cclimber_audio", cclimber_audio_device, sample_rate_w)
	AM_RANGE(0xb000, 0xb000) AM_READ_PORT("DSW") AM_DEVWRITE("cclimber_audio", cclimber_audio_device, sample_volume_w)
	AM_RANGE(0xb800, 0xb800) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xc000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( toprollr_decrypted_opcodes_map, AS_DECRYPTED_OPCODES, 8, cclimber_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROMBANK("bank1d")
	AM_RANGE(0xc000, 0xffff) AM_ROM AM_REGION("maincpu", 0xc000)
ADDRESS_MAP_END

static ADDRESS_MAP_START( cclimber_portmap, AS_IO, 8, cclimber_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x08, 0x09) AM_DEVWRITE("cclimber_audio:aysnd", ay8910_device, address_data_w)
	AM_RANGE(0x0c, 0x0c) AM_DEVREAD("cclimber_audio:aysnd", ay8910_device, data_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( yamato_portmap, AS_IO, 8, cclimber_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(yamato_p0_w)  /* ??? */
	AM_RANGE(0x01, 0x01) AM_WRITE(yamato_p1_w)  /* ??? */
ADDRESS_MAP_END


static ADDRESS_MAP_START( swimmer_audio_map, AS_PROGRAM, 8, cclimber_state )
	AM_RANGE(0x0000, 0x0fff) AM_ROM
	AM_RANGE(0x2000, 0x23ff) AM_RAM
	AM_RANGE(0x3000, 0x3000) AM_READ(soundlatch_byte_r)
	AM_RANGE(0x4000, 0x4001) AM_RAM             /* ??? */
ADDRESS_MAP_END

static ADDRESS_MAP_START( yamato_audio_map, AS_PROGRAM, 8, cclimber_state )
	AM_RANGE(0x0000, 0x07ff) AM_ROM
	AM_RANGE(0x5000, 0x53ff) AM_RAM
ADDRESS_MAP_END


static ADDRESS_MAP_START( swimmer_audio_portmap, AS_IO, 8, cclimber_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_DEVWRITE("ay1", ay8910_device, data_address_w)
	AM_RANGE(0x80, 0x81) AM_DEVWRITE("ay2", ay8910_device, data_address_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( yamato_audio_portmap, AS_IO, 8, cclimber_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_DEVWRITE("ay1", ay8910_device, address_data_w)
	AM_RANGE(0x02, 0x03) AM_DEVWRITE("ay2", ay8910_device, address_data_w)
	AM_RANGE(0x04, 0x04) AM_READ(yamato_p0_r)   /* ??? */
	AM_RANGE(0x08, 0x08) AM_READ(yamato_p1_r)   /* ??? */
ADDRESS_MAP_END


static INPUT_PORTS_START( cclimber )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_UP ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_DOWN ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_LEFT ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_RIGHT ) PORT_8WAY

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_RIGHT ) PORT_8WAY PORT_COCKTAIL

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW:!1,!2")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x00, "SW:!3" )       // Look code at 0x03c4 : 0x8076 is never tested !
	PORT_DIPNAME( 0x08, 0x00, "Rack Test (Cheat)" ) PORT_CODE(KEYCODE_F1) PORT_DIPLOCATION("SW:!4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW:!5,!6")
	PORT_DIPSETTING(    0x30, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW:!7,!8")  // Also "Bonus Life" due to code at 0x03d4
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )                                // Bonus life : 30000 points
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )                                // Bonus life : 50000 points
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_3C ) )                                // Bonus life : 30000 points
	PORT_DIPSETTING(    0xc0, DEF_STR( Free_Play ) )                            // Bonus life : 50000 points

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

/* Same as 'cclimber' but correct "Bonus Life" Dip Switch */
static INPUT_PORTS_START( cclimberj )
	PORT_INCLUDE( cclimber )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW:!3")
	PORT_DIPSETTING(    0x00, "30000" )
	PORT_DIPSETTING(    0x04, "50000" )
INPUT_PORTS_END

static INPUT_PORTS_START( ckong )
	PORT_START("P1")
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY

	PORT_START("P2")
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "7000" )
	PORT_DIPSETTING(    0x04, "10000" )
	PORT_DIPSETTING(    0x08, "15000" )
	PORT_DIPSETTING(    0x0c, "20000" )
	PORT_DIPNAME( 0x70, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

/* Similar to normal Crazy Kong except for the lives per game */
static INPUT_PORTS_START( ckongb )
	PORT_INCLUDE( ckong )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x03, "4" )
INPUT_PORTS_END

static INPUT_PORTS_START( cannonb )
	PORT_INCLUDE( ckong )

	PORT_MODIFY("P1")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY

	PORT_MODIFY("P2")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x03, 0x03, "Display" )
	PORT_DIPSETTING(    0x03, "Scores and Progession Bars" )
	PORT_DIPSETTING(    0x01, "Scores only" )
	PORT_DIPSETTING(    0x02, "Progession Bars only" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x18, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x18, "6" )
	PORT_DIPUNUSED( 0x20, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME( "Start" )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 ) PORT_NAME( "Select" )
INPUT_PORTS_END

static INPUT_PORTS_START( rpatrol )
	PORT_START("P1")    /* P2 controls but we use cclimber tags */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x3e, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL

	PORT_START("P2")    /* P1 controls but we use cclimber tags */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x3e, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x0c, "6" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x20, 0x00, "Unknown 1" )  /* Probably unused */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Unknown 2" )  /* Probably unused */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Memory Test" )
	PORT_DIPSETTING(    0x00, "Retry on Error" )
	PORT_DIPSETTING(    0x80, "Stop on Error" )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( swimmer )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW A:!1,!2")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "Infinite (Cheat)")
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW A:!3,!4")
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x04, "20000" )
	PORT_DIPSETTING(    0x08, "30000" )
	PORT_DIPSETTING(    0x0c, DEF_STR( None ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW A:!5,!6")
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW A:!7,!8")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_6C ) )

	PORT_START("DSW2")
	PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW B:!1")
	PORT_DIPSETTING(    0x10, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW B:!2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW B:!3,!4")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )     // labeled this way for similarities with 'swimmerb'
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )     // labeled this way for similarities with 'swimmerb'
	PORT_DIPSETTING(    0x80, DEF_STR( Harder ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Hardest ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

/* Same as 'swimmer' but different "Difficulty" Dip Switch */
static INPUT_PORTS_START( swimmerb )
	PORT_INCLUDE( swimmer )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW B:!3")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )    // SW B:!4
INPUT_PORTS_END

static INPUT_PORTS_START( guzzler )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW A:!1,!2")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "Infinite (Cheat)")
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW A:!3,!4")
	PORT_DIPSETTING(    0x04, "20K, every 50K" )
	PORT_DIPSETTING(    0x00, "30K, every 100K" )
	PORT_DIPSETTING(    0x08, "30K only" )
	PORT_DIPSETTING(    0x0c, DEF_STR( None ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW A:!5,!6")
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW A:!7,!8")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_6C ) )

	PORT_START("DSW2")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW B:!1")
	PORT_DIPSETTING(    0x10, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x20, 0x00, "High Score Names" ) PORT_DIPLOCATION("SW B:!2")
	PORT_DIPSETTING(    0x20, "3 Letters" )
	PORT_DIPSETTING(    0x00, "10 Letters" )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW B:!3,!4")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Hardest ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( yamato )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x1c, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x14, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x1c, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "Every 30000" )
	PORT_DIPSETTING(    0x20, "Every 50000" )
	PORT_DIPNAME( 0x40, 0x00, "Speed" )
	PORT_DIPSETTING(    0x00, "Slow" )
	PORT_DIPSETTING(    0x40, "Fast" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN2 ) /* set 1 only */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN3 ) /* set 1 only */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("START")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( toprollr )
	PORT_START("P1")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY

	PORT_START("P2")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x1c, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x14, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x1c, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "Every 30000" )
	PORT_DIPSETTING(    0x20, "Every 50000" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN3 )
INPUT_PORTS_END



static const gfx_layout cclimber_charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	2,
	{ 0, RGN_FRAC(1,2) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout cclimber_spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	2,
	{ 0, RGN_FRAC(1,2) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },
	32*8
};

static const gfx_layout cannonb_charlayout =
{
	8,8,
	512,
	2,
	{ 0, 1024*8*8 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout cannonb_spritelayout =
{
	16,16,
	64,
	2,
	{ 0, 256*16*16 },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },
	32*8
};

static const gfx_layout swimmer_charlayout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ 0, RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },      /* characters are upside down */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8     /* every char takes 8 consecutive bytes */
};

static const gfx_layout swimmer_spritelayout =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{ 0, RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },
	32*8
};

static GFXDECODE_START( cclimber )
	GFXDECODE_ENTRY( "gfx1", 0x0000, cclimber_charlayout,      0, 16 ) /* characters */
	GFXDECODE_ENTRY( "gfx1", 0x0000, cclimber_spritelayout,    0, 16 ) /* sprites */
	GFXDECODE_ENTRY( "gfx2", 0x0000, cclimber_charlayout,   16*4,  8 ) /* big sprites */
GFXDECODE_END

static GFXDECODE_START( cannonb )
	GFXDECODE_ENTRY( "gfx1", 0x0000, cannonb_charlayout,       0, 16 ) /* characters */
	GFXDECODE_ENTRY( "gfx1", 0x1000, cannonb_spritelayout,     0, 16 ) /* sprites */
	GFXDECODE_ENTRY( "gfx2", 0x0000, cclimber_charlayout,   16*4,  8 ) /* big sprites */
GFXDECODE_END

static GFXDECODE_START( swimmer )
	GFXDECODE_ENTRY( "gfx1", 0x0000, swimmer_charlayout,       0, 32 ) /* characters */
	GFXDECODE_ENTRY( "gfx1", 0x0000, swimmer_spritelayout,     0, 32 ) /* sprites */
	GFXDECODE_ENTRY( "gfx2", 0x0000, swimmer_charlayout,    32*8,  4 ) /* big sprites */
GFXDECODE_END

static GFXDECODE_START( toprollr )
	GFXDECODE_ENTRY( "gfx1", 0x0000, cclimber_charlayout,      0, 16 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, cclimber_spritelayout,    0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0x0000, cclimber_charlayout,   16*4,  8 ) /* big sprites */
	GFXDECODE_ENTRY( "gfx3", 0x0000, cclimber_charlayout,   24*4, 16 )
GFXDECODE_END

INTERRUPT_GEN_MEMBER(cclimber_state::vblank_irq)
{
	if(m_nmi_mask)
		device.execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

static MACHINE_CONFIG_START( root, cclimber_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, MASTER_CLOCK/3/2)  /* 3.072 MHz */
	MCFG_CPU_PROGRAM_MAP(cclimber_map)
	MCFG_CPU_IO_MAP(cclimber_portmap)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", cclimber_state,  vblank_irq)

	MCFG_MACHINE_RESET_OVERRIDE(cclimber_state,cclimber)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(cclimber_state, screen_update_cclimber)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", cclimber)
	MCFG_PALETTE_ADD("palette", 16*4+8*4)

	MCFG_PALETTE_INIT_OWNER(cclimber_state,cclimber)
	MCFG_VIDEO_START_OVERRIDE(cclimber_state,cclimber)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( cclimber, root )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_CCLIMBER_AUDIO_ADD("cclimber_audio")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( cclimberx, cclimber )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_DECRYPTED_OPCODES_MAP(decrypted_opcodes_map)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( cannonb, cclimber )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(cannonb_map)

	/* video hardware */
	MCFG_GFXDECODE_MODIFY("gfxdecode", cannonb)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( yamato, root )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(yamato_map)
	MCFG_CPU_IO_MAP(yamato_portmap)
	MCFG_CPU_DECRYPTED_OPCODES_MAP(decrypted_opcodes_map)

	MCFG_CPU_ADD("audiocpu", Z80, 3072000) /* 3.072 MHz ? */
	MCFG_CPU_PROGRAM_MAP(yamato_audio_map)
	MCFG_CPU_IO_MAP(yamato_audio_portmap)

	/* video hardware */
	MCFG_PALETTE_MODIFY("palette")
	MCFG_PALETTE_ENTRIES(16*4+8*4+256)
	MCFG_PALETTE_INIT_OWNER(cclimber_state,yamato)
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(cclimber_state, screen_update_yamato)

	/* audio hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, 1536000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_SOUND_ADD("ay2", AY8910, 1536000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( toprollr, cclimber )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(toprollr_map)
	MCFG_CPU_DECRYPTED_OPCODES_MAP(toprollr_decrypted_opcodes_map)

	/* video hardware */
	MCFG_GFXDECODE_MODIFY("gfxdecode", toprollr)
	MCFG_PALETTE_MODIFY("palette")
	MCFG_PALETTE_ENTRIES(32*5)
	MCFG_PALETTE_INIT_OWNER(cclimber_state,toprollr)

	MCFG_VIDEO_START_OVERRIDE(cclimber_state,toprollr)
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(cclimber_state, screen_update_toprollr)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( swimmer, cclimber_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_18_432MHz/6)    /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(swimmer_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", cclimber_state,  vblank_irq)

	MCFG_CPU_ADD("audiocpu", Z80,XTAL_4MHz/2)  /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(swimmer_audio_map)
	MCFG_CPU_IO_MAP(swimmer_audio_portmap)
	MCFG_CPU_PERIODIC_INT_DRIVER(cclimber_state, nmi_line_pulse,  (double)4000000/16384) /* IRQs are triggered by the main CPU */

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60.57) /* verified on pcb */
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(cclimber_state, screen_update_swimmer)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", swimmer)
	MCFG_PALETTE_ADD("palette", 32*8+4*8+1)

	MCFG_PALETTE_INIT_OWNER(cclimber_state,swimmer)
	MCFG_VIDEO_START_OVERRIDE(cclimber_state,swimmer)

	/* audio hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("ay1", AY8910, XTAL_4MHz/2)  /* verified on pcb */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_SOUND_ADD("ay2", AY8910, XTAL_4MHz/2)  /* verified on pcb */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( guzzler, swimmer )
	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(guzzler_map)
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( cclimber )
	ROM_REGION( 0x6000, "maincpu", 0 )
	ROM_LOAD( "cc11",         0x0000, 0x1000, CRC(217ec4ff) SHA1(334604c3a051d57440a9d0bfc34b809418ef1d2d) )
	ROM_LOAD( "cc10",         0x1000, 0x1000, CRC(b3c26cef) SHA1(f52cb5482c12a9c5fb56e2e2aec7cab0ed23e5a5) )
	ROM_LOAD( "cc09",         0x2000, 0x1000, CRC(6db0879c) SHA1(c0ba1976c1dcd6edadd78073173a26851ae8dd4f) )
	ROM_LOAD( "cc08",         0x3000, 0x1000, CRC(f48c5fe3) SHA1(79072bbbf37387998ffd031afe8eb569a16fa9bd) )
	ROM_LOAD( "cc07",         0x4000, 0x1000, CRC(3e873baf) SHA1(8870dc5948cdd3c8d2fe9e54a20cf6c311c94e53) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "cc06",         0x0000, 0x0800, CRC(481b64cc) SHA1(3f35c545fc784ed4f969aba2d7be6e13a5ae32b7) )
	/* 0x0800-0x0fff - empty */
	ROM_LOAD( "cc05",         0x1000, 0x0800, CRC(2c33b760) SHA1(2edea8fe13376fbd51a5586d97aba3b30d78e94b) )
	/* 0x1800-0xffff - empty */
	ROM_LOAD( "cc04",         0x2000, 0x0800, CRC(332347cb) SHA1(4115ca32af73f1791635b7d9e093bf77088a8222) )
	/* 0x2800-0x2fff - empty */
	ROM_LOAD( "cc03",         0x3000, 0x0800, CRC(4e4b3658) SHA1(0d39a8cb5cd6cf06008be60707f9b277a8a32a2d) )
	/* 0x3800-0x3fff - empty */

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "cc02",         0x0000, 0x0800, CRC(14f3ecc9) SHA1(a1b5121abfbe8f07580eb3fa6384352d239a3d75) )
	ROM_LOAD( "cc01",         0x0800, 0x0800, CRC(21c0f9fb) SHA1(44fad56d302a439257216ddac9fd62b3666589f1) )

	ROM_REGION( 0x0060, "proms", 0 )
	ROM_LOAD( "cclimber.pr1", 0x0000, 0x0020, CRC(751c3325) SHA1(edce2bc883996c1d72dc6c1c9f62799b162d415a) )
	ROM_LOAD( "cclimber.pr2", 0x0020, 0x0020, CRC(ab1940fa) SHA1(8d98e05cbaa6f55770c12e0a9a8ed9c73cc54423) )
	ROM_LOAD( "cclimber.pr3", 0x0040, 0x0020, CRC(71317756) SHA1(1195f0a037e379cc1a3c0314cb746f5cd2bffe50) )

	ROM_REGION( 0x2000, "samples", 0 )  /* samples */
	ROM_LOAD( "cc13",         0x0000, 0x1000, CRC(e0042f75) SHA1(86cb31b110742a0f7ae33052c88f42d00deb5468) )
	ROM_LOAD( "cc12",         0x1000, 0x1000, CRC(5da13aaa) SHA1(b2d41e69435d09c456648a10e33f5e1fbb0bc64c) )
ROM_END

ROM_START( cclimberj )
	ROM_REGION( 0x6000, "maincpu", 0 )
	ROM_LOAD( "cc11j.bin",    0x0000, 0x1000, CRC(89783959) SHA1(948fa88fcb9e3797b9c10934d36cf6a55cb590fe) )
	ROM_LOAD( "cc10j.bin",    0x1000, 0x1000, CRC(14eda506) SHA1(4bc55b4c4ec197952b05ad32584f15f0383cc2df) )
	ROM_LOAD( "cc09j.bin",    0x2000, 0x1000, CRC(26489069) SHA1(9be4d4a22dd334e619416e6c846a05003c0d687e) )
	ROM_LOAD( "cc08j.bin",    0x3000, 0x1000, CRC(b33c96f8) SHA1(3974f4a60f37bed9e4faee7dafb565f553b9c201) )
	ROM_LOAD( "cc07j.bin",    0x4000, 0x1000, CRC(fbc9626c) SHA1(32be2d06321b2943718d0bec77ec9ebb806e4b93) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "cc06",         0x0000, 0x0800, CRC(481b64cc) SHA1(3f35c545fc784ed4f969aba2d7be6e13a5ae32b7) )
	/* 0x0800-0x0fff - empty */
	ROM_LOAD( "cc05",         0x1000, 0x0800, CRC(2c33b760) SHA1(2edea8fe13376fbd51a5586d97aba3b30d78e94b) )
	/* 0x1800-0xffff - empty */
	ROM_LOAD( "cc04",         0x2000, 0x0800, CRC(332347cb) SHA1(4115ca32af73f1791635b7d9e093bf77088a8222) )
	/* 0x2800-0x2fff - empty */
	ROM_LOAD( "cc03",         0x3000, 0x0800, CRC(4e4b3658) SHA1(0d39a8cb5cd6cf06008be60707f9b277a8a32a2d) )
	/* 0x3800-0x3fff - empty */

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "cc02",         0x0000, 0x0800, CRC(14f3ecc9) SHA1(a1b5121abfbe8f07580eb3fa6384352d239a3d75) )
	ROM_LOAD( "cc01",         0x0800, 0x0800, CRC(21c0f9fb) SHA1(44fad56d302a439257216ddac9fd62b3666589f1) )

	ROM_REGION( 0x0060, "proms", 0 )
	ROM_LOAD( "cclimber.pr1", 0x0000, 0x0020, CRC(751c3325) SHA1(edce2bc883996c1d72dc6c1c9f62799b162d415a) )
	ROM_LOAD( "cclimber.pr2", 0x0020, 0x0020, CRC(ab1940fa) SHA1(8d98e05cbaa6f55770c12e0a9a8ed9c73cc54423) )
	ROM_LOAD( "cclimber.pr3", 0x0040, 0x0020, CRC(71317756) SHA1(1195f0a037e379cc1a3c0314cb746f5cd2bffe50) )

	ROM_REGION( 0x2000, "samples", 0 )  /* samples */
	ROM_LOAD( "cc13j.bin",    0x0000, 0x1000, CRC(5f0bcdfb) SHA1(7f79bf6de117348f606696ed7ea1937bbf926612) )
	ROM_LOAD( "cc12j.bin",    0x1000, 0x1000, CRC(9003ffbd) SHA1(fd016056aabc23957643f37230f03842294f795e) )
ROM_END

ROM_START( ccboot )
	ROM_REGION( 0x6000, "maincpu", 0 )
	ROM_LOAD( "m11.bin",      0x0000, 0x1000, CRC(5efbe180) SHA1(e0c24f21d563da075eb5019d0e76cb01c2598c7a) )
	ROM_LOAD( "m10.bin",      0x1000, 0x1000, CRC(be2748c7) SHA1(ae66bc4e5e02bf9944a3ee4b0d2dec073f732260) )
	ROM_LOAD( "cc09j.bin",    0x2000, 0x1000, CRC(26489069) SHA1(9be4d4a22dd334e619416e6c846a05003c0d687e) )
	ROM_LOAD( "m08.bin",      0x3000, 0x1000, CRC(e3c542d6) SHA1(645cc4c94d1b1601c0083b156de67ec47fe2449f) )
	ROM_LOAD( "cc07j.bin",    0x4000, 0x1000, CRC(fbc9626c) SHA1(32be2d06321b2943718d0bec77ec9ebb806e4b93) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "cc06",         0x0000, 0x0800, CRC(481b64cc) SHA1(3f35c545fc784ed4f969aba2d7be6e13a5ae32b7) )
	/* 0x0800-0x0fff - empty */
	ROM_LOAD( "m05.bin",      0x1000, 0x0800, CRC(056af36b) SHA1(756a295bbf7ede201b2e4cb106ce67a127e008de) )
	/* 0x1800-0xffff - empty */
	ROM_LOAD( "m04.bin",      0x2000, 0x0800, CRC(6fb80538) SHA1(6ba5add5c0190e79191b3fa749a1b94e766e3950) )
	/* 0x2800-0x2fff - empty */
	ROM_LOAD( "m03.bin",      0x3800, 0x0800, CRC(67127253) SHA1(e27556ed74e73644a2578ce6645c312d64f484c6) )
	/* 0x3800-0x3fff - empty */

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "m02.bin",      0x0000, 0x0800, CRC(7f4877de) SHA1(c9aa9ff1b6cf907917fedfbd419b15ac337cf7bb) )
	ROM_LOAD( "m01.bin",      0x0800, 0x0800, CRC(49fab908) SHA1(9665d6e26f390afcbf0ed9fe8fea9be94fbb3a84) )

	ROM_REGION( 0x0160, "proms", 0 )
	ROM_LOAD( "cclimber.pr1", 0x0000, 0x0020, CRC(751c3325) SHA1(edce2bc883996c1d72dc6c1c9f62799b162d415a) )
	ROM_LOAD( "cclimber.pr2", 0x0020, 0x0020, CRC(ab1940fa) SHA1(8d98e05cbaa6f55770c12e0a9a8ed9c73cc54423) )
	ROM_LOAD( "cclimber.pr3", 0x0040, 0x0020, CRC(71317756) SHA1(1195f0a037e379cc1a3c0314cb746f5cd2bffe50) )
	ROM_LOAD( "ccboot.prm",   0x0060, 0x0100, CRC(9e11550d) SHA1(b8cba8e16e10e23fba1f11551102ab77b680bdf0) )    /* decryption table (not used) */

	ROM_REGION( 0x2000, "samples", 0 )  /* samples */
	ROM_LOAD( "cc13j.bin",    0x0000, 0x1000, CRC(5f0bcdfb) SHA1(7f79bf6de117348f606696ed7ea1937bbf926612) )
	ROM_LOAD( "cc12j.bin",    0x1000, 0x1000, CRC(9003ffbd) SHA1(fd016056aabc23957643f37230f03842294f795e) )
ROM_END

ROM_START( ccboot2 )
	ROM_REGION( 0x6000, "maincpu", 0 )
	ROM_LOAD( "11.4k",        0x0000, 0x1000, CRC(b2b17e24) SHA1(1242d64242b3a6fe099457d155ebc508e5482818) )
	ROM_LOAD( "10.4j",        0x1000, 0x1000, CRC(8382bc0f) SHA1(2390ee2ec08a074c7bc4b9c7750b979a1d3a8a67) )
	ROM_LOAD( "cc09j.bin",    0x2000, 0x1000, CRC(26489069) SHA1(9be4d4a22dd334e619416e6c846a05003c0d687e) )
	ROM_LOAD( "m08.bin",      0x3000, 0x1000, CRC(e3c542d6) SHA1(645cc4c94d1b1601c0083b156de67ec47fe2449f) )
	ROM_LOAD( "cc07j.bin",    0x4000, 0x1000, CRC(fbc9626c) SHA1(32be2d06321b2943718d0bec77ec9ebb806e4b93) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "cc06",         0x0000, 0x0800, CRC(481b64cc) SHA1(3f35c545fc784ed4f969aba2d7be6e13a5ae32b7) )
	/* 0x0800-0x0fff - empty */
	ROM_LOAD( "cc05",         0x1000, 0x0800, CRC(2c33b760) SHA1(2edea8fe13376fbd51a5586d97aba3b30d78e94b) )
	/* 0x1800-0xffff - empty */
	ROM_LOAD( "cc04",         0x2000, 0x0800, CRC(332347cb) SHA1(4115ca32af73f1791635b7d9e093bf77088a8222) )
	/* 0x2800-0x2fff - empty */
	ROM_LOAD( "cc03",         0x3000, 0x0800, CRC(4e4b3658) SHA1(0d39a8cb5cd6cf06008be60707f9b277a8a32a2d) )
	/* 0x3800-0x3fff - empty */

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "cc02",         0x0000, 0x0800, CRC(14f3ecc9) SHA1(a1b5121abfbe8f07580eb3fa6384352d239a3d75) )
	ROM_LOAD( "cc01",         0x0800, 0x0800, CRC(21c0f9fb) SHA1(44fad56d302a439257216ddac9fd62b3666589f1) )

	ROM_REGION( 0x0160, "proms", 0 )
	ROM_LOAD( "cclimber.pr1", 0x0000, 0x0020, CRC(751c3325) SHA1(edce2bc883996c1d72dc6c1c9f62799b162d415a) )
	ROM_LOAD( "cclimber.pr2", 0x0020, 0x0020, CRC(ab1940fa) SHA1(8d98e05cbaa6f55770c12e0a9a8ed9c73cc54423) )
	ROM_LOAD( "cclimber.pr3", 0x0040, 0x0020, CRC(71317756) SHA1(1195f0a037e379cc1a3c0314cb746f5cd2bffe50) )
	ROM_LOAD( "ccboot.prm",   0x0060, 0x0100, CRC(9e11550d) SHA1(b8cba8e16e10e23fba1f11551102ab77b680bdf0) )    /* decryption table (not used) */

	ROM_REGION( 0x2000, "samples", 0 )  /* samples */
	ROM_LOAD( "cc13j.bin",    0x0000, 0x1000, CRC(5f0bcdfb) SHA1(7f79bf6de117348f606696ed7ea1937bbf926612) )
	ROM_LOAD( "cc12j.bin",    0x1000, 0x1000, CRC(9003ffbd) SHA1(fd016056aabc23957643f37230f03842294f795e) )
ROM_END

ROM_START( ccbootmr )  /* Model Racing bootleg */
	ROM_REGION( 0x6000, "maincpu", 0 )
	ROM_LOAD( "211.k4",       0x0000, 0x1000, CRC(b2b17e24) SHA1(1242d64242b3a6fe099457d155ebc508e5482818) )
	ROM_LOAD( "210.j4",       0x1000, 0x1000, CRC(8382bc0f) SHA1(2390ee2ec08a074c7bc4b9c7750b979a1d3a8a67) )
	ROM_LOAD( "209.f4",       0x2000, 0x1000, CRC(26489069) SHA1(9be4d4a22dd334e619416e6c846a05003c0d687e) )
	ROM_LOAD( "208.e4",       0x3000, 0x1000, CRC(e3c542d6) SHA1(645cc4c94d1b1601c0083b156de67ec47fe2449f) )
	ROM_LOAD( "207.c4",       0x4000, 0x1000, CRC(fbc9626c) SHA1(32be2d06321b2943718d0bec77ec9ebb806e4b93) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "cc06",         0x0000, 0x0800, CRC(481b64cc) SHA1(3f35c545fc784ed4f969aba2d7be6e13a5ae32b7) ) // 206.n6
	/* 0x0800-0x0fff - empty */
	ROM_LOAD( "cc05",         0x1000, 0x0800, CRC(2c33b760) SHA1(2edea8fe13376fbd51a5586d97aba3b30d78e94b) ) // 205.l6
	/* 0x1800-0xffff - empty */
	ROM_LOAD( "cc04",         0x2000, 0x0800, CRC(332347cb) SHA1(4115ca32af73f1791635b7d9e093bf77088a8222) ) // 204.k6
	/* 0x2800-0x2fff - empty */
	ROM_LOAD( "cc03",         0x3000, 0x0800, CRC(4e4b3658) SHA1(0d39a8cb5cd6cf06008be60707f9b277a8a32a2d) ) // 203.h6
	/* 0x3800-0x3fff - empty */

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "202.c6",       0x0000, 0x0800, CRC(5ec87c50) SHA1(68317533800a06abb0454303443cdcd913866977) )
	ROM_LOAD( "201.a6",       0x0800, 0x0800, CRC(76d6d9a4) SHA1(3071dd65d5fe996b1b3a29e9a22d5c005cfd348d) )

	ROM_REGION( 0x0160, "proms", 0 )
	ROM_LOAD( "cclimber.pr1", 0x0000, 0x0020, CRC(751c3325) SHA1(edce2bc883996c1d72dc6c1c9f62799b162d415a) ) // 199-74288.n9
	ROM_LOAD( "cclimber.pr2", 0x0020, 0x0020, CRC(ab1940fa) SHA1(8d98e05cbaa6f55770c12e0a9a8ed9c73cc54423) ) // 210-74288.n9
	ROM_LOAD( "198-74288.c9", 0x0040, 0x0020, CRC(b4e827a5) SHA1(31a5a5ad54417a474d22bb16c473415d99a2b6f1) )
	ROM_LOAD( "214-74187.cpu",0x0060, 0x0100, CRC(9e11550d) SHA1(b8cba8e16e10e23fba1f11551102ab77b680bdf0) )    /* decryption table (not used) */

	ROM_REGION( 0x2000, "samples", 0 )  /* samples */
	ROM_LOAD( "213.r4",       0x0000, 0x1000, CRC(5f0bcdfb) SHA1(7f79bf6de117348f606696ed7ea1937bbf926612) )
	ROM_LOAD( "212.n4",       0x1000, 0x1000, CRC(9003ffbd) SHA1(fd016056aabc23957643f37230f03842294f795e) )
ROM_END


ROM_START( cclimbroper )
	ROM_REGION( 0x6000, "maincpu", 0 )
	ROM_LOAD( "cc5-2532.cpu",       0x0000, 0x1000, CRC(f94b96e8) SHA1(b13dceb0a73d1a4eeb9c3e2d2307f0c82c365393))
	ROM_LOAD( "cc4-2532.cpu",       0x1000, 0x1000, CRC(4b1abea6) SHA1(eae7c96fc0b64d313bed4a75bd6d397b37eaac7e) )
	ROM_LOAD( "cc3-2532.cpu",       0x2000, 0x1000, CRC(5612bb3c) SHA1(213846bb3393467260f401b00b821cbab7ac9636) )
	ROM_LOAD( "cc2-2532.cpu",       0x3000, 0x1000, CRC(653cebc4) SHA1(c0b664389f7a6f58e880ba0870118aa26c636a37) )
	ROM_LOAD( "cc1-2532.cpu",       0x4000, 0x1000, CRC(3fcf912b) SHA1(d540895018bc409ae011ce5841c8c5384bbbb1b9) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "cc13-2716.cpu",         0x0000, 0x0800, CRC(9324846d) SHA1(fc04635663ed9fb0f1b7924caff94fb3a1728050) )
	/* 0x0800-0x0fff - empty */
	ROM_LOAD( "cc12-2716.cpu",         0x1000, 0x0800, CRC(6d15ba36) SHA1(03d5b8866a27d70a8ddd9eb30717f42fe9164f4a) )
	/* 0x1800-0xffff - empty */
	ROM_LOAD( "cc11-2716.cpu",         0x2000, 0x0800, CRC(25886f13) SHA1(c83b133448f20b689d84509d4f0f01c38452ed51) )
	/* 0x2800-0x2fff - empty */
	ROM_LOAD( "cc10-2716.cpu",         0x3000, 0x0800, CRC(437471ec) SHA1(07be06d4d82e862a1e73f51e331b0cd6c9e7889b) )
	/* 0x3800-0x3fff - empty */

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "cc9-2716.cpu",         0x0000, 0x0800, CRC(a546a18f) SHA1(302ba08cef61b1badc361666fd559713037a2e43) )
	ROM_LOAD( "cc8-2716.cpu",         0x0800, 0x0800, CRC(0224e507) SHA1(c9b534246b6bb743294581a5e74608a295cf0734) )

	ROM_REGION( 0x2000, "samples", 0 )  /* samples */
	ROM_LOAD( "cc7-2532.cpu",    0x0000, 0x1000, CRC(5f0bcdfb) SHA1(7f79bf6de117348f606696ed7ea1937bbf926612) )
	ROM_LOAD( "cc6-2532.cpu",    0x1000, 0x1000, CRC(9003ffbd) SHA1(fd016056aabc23957643f37230f03842294f795e) )

	ROM_REGION( 0x0060, "proms", 0 ) // NOT verified on this board
	ROM_LOAD( "cclimber.pr1", 0x0000, 0x0020, CRC(751c3325) SHA1(edce2bc883996c1d72dc6c1c9f62799b162d415a) )
	ROM_LOAD( "cclimber.pr2", 0x0020, 0x0020, CRC(ab1940fa) SHA1(8d98e05cbaa6f55770c12e0a9a8ed9c73cc54423) )
	ROM_LOAD( "cclimber.pr3", 0x0040, 0x0020, CRC(71317756) SHA1(1195f0a037e379cc1a3c0314cb746f5cd2bffe50) )
ROM_END




/* Sets below are Crazy Kong Part II and have an extra screen in attract mode, showing a caged Kong and copyright */

ROM_START( ckongpt2 )
	ROM_REGION( 0x6000, "maincpu", 0 )
	ROM_LOAD( "7.5d",         0x0000, 0x1000, CRC(b27df032) SHA1(57f9be139c610405e3c2fddd7093dfb1277e450e) )
	ROM_LOAD( "8.5e",         0x1000, 0x1000, CRC(5dc1aaba) SHA1(42b9e5946ffce7c156d114bde68f37c2c34853c4) )
	ROM_LOAD( "9.5h",         0x2000, 0x1000, CRC(c9054c94) SHA1(1aa08d2501ee620759fd5c111e12f6d432c25294) )
	ROM_LOAD( "10.5k",        0x3000, 0x1000, CRC(069c4797) SHA1(03be185e6914ec7f3770ce3da4eb49cdb97adc85) )
	ROM_LOAD( "11.5l",        0x4000, 0x1000, CRC(ae159192) SHA1(d467256a3a366e246243e7828ff4a45d4c146e2c) )
	ROM_LOAD( "12.5n",        0x5000, 0x1000, CRC(966bc9ab) SHA1(4434fc620169ffea1b1f227b61674e1daf79b54b) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "6.11n",        0x0000, 0x1000, CRC(2dcedd12) SHA1(dfdcfc21bcba7c8e148ee54daae511ca78c58e70) )
	ROM_LOAD( "5.11l",        0x1000, 0x1000, CRC(fa7cbd91) SHA1(0208d2ebc59f3600005476b6987472685bc99d67) )
	ROM_LOAD( "4.11k",        0x2000, 0x1000, CRC(3375b3bd) SHA1(a00b3c31cff123aab6ac0833aabfdd663302971a) )
	ROM_LOAD( "3.11h",        0x3000, 0x1000, CRC(5655cc11) SHA1(5195e9b2a60c54280b48b32ee8248090904dbc51) )

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "2.11c",        0x0000, 0x0800, CRC(d1352c31) SHA1(da726a63a8be830d695afeddc1717749af8c9d47) )
	ROM_LOAD( "1.11a",        0x0800, 0x0800, CRC(a7a2fdbd) SHA1(529865f8bbfbdbbf34ac39c70ef17e6d5bd0f845) )

	ROM_REGION( 0x0060, "proms", 0 )
	ROM_LOAD( "prom.v6",      0x0000, 0x0020, CRC(b3fc1505) SHA1(5b94adde0428a26b815c7eb9b3f3716470d349c7) )
	ROM_LOAD( "prom.u6",      0x0020, 0x0020, CRC(26aada9e) SHA1(f59645e606ea4f0dd0fc4ea47dd03f526c534941) )
	ROM_LOAD( "prom.t6",      0x0040, 0x0020, CRC(676b3166) SHA1(29b9434cd34d43ea5664e436e2a24b54f8d88aac) )

	ROM_REGION( 0x2000, "samples", 0 )  /* samples */
	ROM_LOAD( "14.5s",        0x0000, 0x1000, CRC(5f0bcdfb) SHA1(7f79bf6de117348f606696ed7ea1937bbf926612) )
	ROM_LOAD( "13.5p",        0x1000, 0x1000, CRC(9003ffbd) SHA1(fd016056aabc23957643f37230f03842294f795e) )
ROM_END

ROM_START( ckongpt2a )
	ROM_REGION( 0x6000, "maincpu", 0 )
	ROM_LOAD( "7.5d",         0x0000, 0x1000, CRC(b27df032) SHA1(57f9be139c610405e3c2fddd7093dfb1277e450e) )
	ROM_LOAD( "8.5e",         0x1000, 0x1000, CRC(5dc1aaba) SHA1(42b9e5946ffce7c156d114bde68f37c2c34853c4) )
	ROM_LOAD( "9.5h",         0x2000, 0x1000, CRC(c9054c94) SHA1(1aa08d2501ee620759fd5c111e12f6d432c25294) )
	ROM_LOAD( "10.dat",       0x3000, 0x1000, CRC(c3beb501) SHA1(14f49c45fc7c91799034c5a51fca310f0a66b1d7) )
	ROM_LOAD( "11.5l",        0x4000, 0x1000, CRC(ae159192) SHA1(d467256a3a366e246243e7828ff4a45d4c146e2c) )
	ROM_LOAD( "12.5n",        0x5000, 0x1000, CRC(966bc9ab) SHA1(4434fc620169ffea1b1f227b61674e1daf79b54b) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "6.11n",        0x0000, 0x1000, CRC(2dcedd12) SHA1(dfdcfc21bcba7c8e148ee54daae511ca78c58e70) )
	ROM_LOAD( "5.11l",        0x1000, 0x1000, CRC(fa7cbd91) SHA1(0208d2ebc59f3600005476b6987472685bc99d67) )
	ROM_LOAD( "4.11k",        0x2000, 0x1000, CRC(3375b3bd) SHA1(a00b3c31cff123aab6ac0833aabfdd663302971a) )
	ROM_LOAD( "3.11h",        0x3000, 0x1000, CRC(5655cc11) SHA1(5195e9b2a60c54280b48b32ee8248090904dbc51) )

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "2.11c",        0x0000, 0x0800, CRC(d1352c31) SHA1(da726a63a8be830d695afeddc1717749af8c9d47) )
	ROM_LOAD( "1.11a",        0x0800, 0x0800, CRC(a7a2fdbd) SHA1(529865f8bbfbdbbf34ac39c70ef17e6d5bd0f845) )

	ROM_REGION( 0x0060, "proms", 0 )
	ROM_LOAD( "prom.v6",      0x0000, 0x0020, CRC(b3fc1505) SHA1(5b94adde0428a26b815c7eb9b3f3716470d349c7) )
	ROM_LOAD( "prom.u6",      0x0020, 0x0020, CRC(26aada9e) SHA1(f59645e606ea4f0dd0fc4ea47dd03f526c534941) )
	ROM_LOAD( "prom.t6",      0x0040, 0x0020, CRC(676b3166) SHA1(29b9434cd34d43ea5664e436e2a24b54f8d88aac) )

	ROM_REGION( 0x2000, "samples", 0 )  /* samples */
	ROM_LOAD( "14.5s",        0x0000, 0x1000, CRC(5f0bcdfb) SHA1(7f79bf6de117348f606696ed7ea1937bbf926612) )
	ROM_LOAD( "13.5p",        0x1000, 0x1000, CRC(9003ffbd) SHA1(fd016056aabc23957643f37230f03842294f795e) )
ROM_END

ROM_START( ckongpt2j )
	ROM_REGION( 0x6000, "maincpu", 0 )
	ROM_LOAD( "7.5d",         0x0000, 0x1000, CRC(b27df032) SHA1(57f9be139c610405e3c2fddd7093dfb1277e450e) )
	ROM_LOAD( "8.5e",         0x1000, 0x1000, CRC(5dc1aaba) SHA1(42b9e5946ffce7c156d114bde68f37c2c34853c4) )
	ROM_LOAD( "9.5h",         0x2000, 0x1000, CRC(c9054c94) SHA1(1aa08d2501ee620759fd5c111e12f6d432c25294) )
	ROM_LOAD( "10.dat",       0x3000, 0x1000, CRC(c3beb501) SHA1(14f49c45fc7c91799034c5a51fca310f0a66b1d7) )
	ROM_LOAD( "11.5l",        0x4000, 0x1000, CRC(4164eb4d) SHA1(ec95f913820375c3f6dd24776b4d3fd04163f5de) ) // sldh
	ROM_LOAD( "12.5n",        0x5000, 0x1000, CRC(966bc9ab) SHA1(4434fc620169ffea1b1f227b61674e1daf79b54b) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "6.11n",        0x0000, 0x1000, CRC(2dcedd12) SHA1(dfdcfc21bcba7c8e148ee54daae511ca78c58e70) )
	ROM_LOAD( "5.11l",        0x1000, 0x1000, CRC(fa7cbd91) SHA1(0208d2ebc59f3600005476b6987472685bc99d67) )
	ROM_LOAD( "4.11k",        0x2000, 0x1000, CRC(3375b3bd) SHA1(a00b3c31cff123aab6ac0833aabfdd663302971a) )
	ROM_LOAD( "3.11h",        0x3000, 0x1000, CRC(5655cc11) SHA1(5195e9b2a60c54280b48b32ee8248090904dbc51) )

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "2.11c",        0x0000, 0x0800, CRC(d1352c31) SHA1(da726a63a8be830d695afeddc1717749af8c9d47) )
	ROM_LOAD( "1.11a",        0x0800, 0x0800, CRC(a7a2fdbd) SHA1(529865f8bbfbdbbf34ac39c70ef17e6d5bd0f845) )

	ROM_REGION( 0x0060, "proms", 0 )
	ROM_LOAD( "prom.v6",      0x0000, 0x0020, CRC(b3fc1505) SHA1(5b94adde0428a26b815c7eb9b3f3716470d349c7) )
	ROM_LOAD( "prom.u6",      0x0020, 0x0020, CRC(26aada9e) SHA1(f59645e606ea4f0dd0fc4ea47dd03f526c534941) )
	ROM_LOAD( "prom.t6",      0x0040, 0x0020, CRC(676b3166) SHA1(29b9434cd34d43ea5664e436e2a24b54f8d88aac) )

	ROM_REGION( 0x2000, "samples", 0 )  /* samples */
	ROM_LOAD( "14.5s",        0x0000, 0x1000, CRC(5f0bcdfb) SHA1(7f79bf6de117348f606696ed7ea1937bbf926612) )
	ROM_LOAD( "13.5p",        0x1000, 0x1000, CRC(9003ffbd) SHA1(fd016056aabc23957643f37230f03842294f795e) )
ROM_END

ROM_START( ckongpt2jeu )
	ROM_REGION( 0x6000, "maincpu", 0 )
	ROM_LOAD( "7.5d",         0x0000, 0x1000, CRC(b27df032) SHA1(57f9be139c610405e3c2fddd7093dfb1277e450e) )
	ROM_LOAD( "8.5e",         0x1000, 0x1000, CRC(5dc1aaba) SHA1(42b9e5946ffce7c156d114bde68f37c2c34853c4) )
	ROM_LOAD( "9.5h",         0x2000, 0x1000, CRC(c9054c94) SHA1(1aa08d2501ee620759fd5c111e12f6d432c25294) )
	ROM_LOAD( "ckjeu10.dat",  0x3000, 0x1000, CRC(7e6eeec4) SHA1(98b283ea22bedc46710a24e65cfae48b87a57605) )
	ROM_LOAD( "11.5l",        0x4000, 0x1000, CRC(ae159192) SHA1(d467256a3a366e246243e7828ff4a45d4c146e2c) )
	ROM_LOAD( "ckjeu12.dat",  0x5000, 0x1000, CRC(0532f270) SHA1(a73680bd7939097bd821fb6834e8763cf1572b55) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "6.11n",        0x0000, 0x1000, CRC(2dcedd12) SHA1(dfdcfc21bcba7c8e148ee54daae511ca78c58e70) )
	ROM_LOAD( "5.11l",        0x1000, 0x1000, CRC(fa7cbd91) SHA1(0208d2ebc59f3600005476b6987472685bc99d67) )
	ROM_LOAD( "4.11k",        0x2000, 0x1000, CRC(3375b3bd) SHA1(a00b3c31cff123aab6ac0833aabfdd663302971a) )
	ROM_LOAD( "3.11h",        0x3000, 0x1000, CRC(5655cc11) SHA1(5195e9b2a60c54280b48b32ee8248090904dbc51) )

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "2.11c",        0x0000, 0x0800, CRC(d1352c31) SHA1(da726a63a8be830d695afeddc1717749af8c9d47) )
	ROM_LOAD( "1.11a",        0x0800, 0x0800, CRC(a7a2fdbd) SHA1(529865f8bbfbdbbf34ac39c70ef17e6d5bd0f845) )

	ROM_REGION( 0x0060, "proms", 0 )
	ROM_LOAD( "prom.v6",      0x0000, 0x0020, CRC(b3fc1505) SHA1(5b94adde0428a26b815c7eb9b3f3716470d349c7) )
	ROM_LOAD( "prom.u6",      0x0020, 0x0020, CRC(26aada9e) SHA1(f59645e606ea4f0dd0fc4ea47dd03f526c534941) )
	ROM_LOAD( "prom.t6",      0x0040, 0x0020, CRC(676b3166) SHA1(29b9434cd34d43ea5664e436e2a24b54f8d88aac) )

	ROM_REGION( 0x2000, "samples", 0 )  /* samples */
	ROM_LOAD( "14.5s",        0x0000, 0x1000, CRC(5f0bcdfb) SHA1(7f79bf6de117348f606696ed7ea1937bbf926612) )
	ROM_LOAD( "13.5p",        0x1000, 0x1000, CRC(9003ffbd) SHA1(fd016056aabc23957643f37230f03842294f795e) )
ROM_END


ROM_START( ckongpt2b )
	ROM_REGION( 0x6000, "maincpu", 0 )
	ROM_LOAD( "d05-7.rom",    0x0000, 0x1000, CRC(5d96ee9a) SHA1(f531d251fd3469edd3b5b5e7c26ff9cef7006ce8) )
	ROM_LOAD( "f05-8.rom",    0x1000, 0x1000, CRC(74a8435b) SHA1(465ad96009d3ba939eee13ba0d5fd6d9dec118bc) )
	ROM_LOAD( "h05-9.rom",    0x2000, 0x1000, CRC(e06ca575) SHA1(cd5a32fac614902e136e522ac188616c72d65571) )
	ROM_LOAD( "k05-10.rom",   0x3000, 0x1000, CRC(46d83a11) SHA1(de840994104bfc633a3640610966f087fbc3d749) )
	ROM_LOAD( "l05-11.rom",   0x4000, 0x1000, CRC(07c30f3d) SHA1(9b72f8a76c64ab22f8b8c1bd8e457c10b86d95a1) )
	ROM_LOAD( "n05-12.rom",   0x5000, 0x1000, CRC(151de90a) SHA1(5d063c4fe6767727d051815120d692818a30ee81) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "6.11n",        0x0000, 0x1000, CRC(2dcedd12) SHA1(dfdcfc21bcba7c8e148ee54daae511ca78c58e70) )
	ROM_LOAD( "5.11l",        0x1000, 0x1000, CRC(fa7cbd91) SHA1(0208d2ebc59f3600005476b6987472685bc99d67) )
	ROM_LOAD( "4.11k",        0x2000, 0x1000, CRC(3375b3bd) SHA1(a00b3c31cff123aab6ac0833aabfdd663302971a) )
	ROM_LOAD( "3.11h",        0x3000, 0x1000, CRC(5655cc11) SHA1(5195e9b2a60c54280b48b32ee8248090904dbc51) )

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "2.11c",        0x0000, 0x0800, CRC(d1352c31) SHA1(da726a63a8be830d695afeddc1717749af8c9d47) )
	ROM_LOAD( "1.11a",        0x0800, 0x0800, CRC(a7a2fdbd) SHA1(529865f8bbfbdbbf34ac39c70ef17e6d5bd0f845) )

	ROM_REGION( 0x0060, "proms", 0 )
	ROM_LOAD( "prom.v6",      0x0000, 0x0020, CRC(b3fc1505) SHA1(5b94adde0428a26b815c7eb9b3f3716470d349c7) )
	ROM_LOAD( "prom.u6",      0x0020, 0x0020, CRC(26aada9e) SHA1(f59645e606ea4f0dd0fc4ea47dd03f526c534941) )
	ROM_LOAD( "prom.t6",      0x0040, 0x0020, CRC(676b3166) SHA1(29b9434cd34d43ea5664e436e2a24b54f8d88aac) )

	ROM_REGION( 0x2000, "samples", 0 )  /* samples */
	ROM_LOAD( "14.5s",        0x0000, 0x1000, CRC(5f0bcdfb) SHA1(7f79bf6de117348f606696ed7ea1937bbf926612) )
	ROM_LOAD( "13.5p",        0x1000, 0x1000, CRC(9003ffbd) SHA1(fd016056aabc23957643f37230f03842294f795e) )
ROM_END



/* Sets below are 'Crazy Kong' without the extra Falcon screen or Pt. 2 subtitle, they also have worse colours */

ROM_START( ckong )
	ROM_REGION( 0x6000, "maincpu", 0 )
	ROM_LOAD( "falcon7",      0x0000, 0x1000, CRC(2171cac3) SHA1(7b18bfe44c32fb64b675bbbe2136344522c79b09) )
	ROM_LOAD( "falcon8",      0x1000, 0x1000, CRC(88b83ff7) SHA1(4afc494cc264aaa4614da6aed02ce062d9c20850) )
	ROM_LOAD( "falcon9",      0x2000, 0x1000, CRC(cff2af47) SHA1(1757428cefad13855a623162101ec01c04006c94) )
	ROM_LOAD( "falcon10",     0x3000, 0x1000, CRC(6b2ecf23) SHA1(75098de72f9b2966534b5c3d4bfaf4893c22150a) ) // differs from ckongalc
	ROM_LOAD( "falcon11",     0x4000, 0x1000, CRC(327dcadf) SHA1(17b2d3b9e2a82b5278a01cc972cb49705d113127) )
	/* no ROM at 5000 */

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "falcon6",      0x0000, 0x1000, CRC(a8916dc8) SHA1(472520aae3837e6026f2a7577d3b2aff371a316c) )
	ROM_LOAD( "falcon5",      0x1000, 0x1000, CRC(cd3b5dde) SHA1(2319a2be04d70989b01f4fc703756ba6e1c1f388) )
	ROM_LOAD( "falcon4",      0x2000, 0x1000, CRC(b62a0367) SHA1(8c285cbc714d7e6589bd63b3cef7c841ed1c2a4e) )
	ROM_LOAD( "falcon3",      0x3000, 0x1000, CRC(61122c5e) SHA1(978b6dbec35f3adc651fddf332db17625099a92e) )

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "falcon2",  0x0000, 0x0800, CRC(f67c80f1) SHA1(d1fbcce1b6242f810e106ff50812636e3168ebc1) )
	ROM_LOAD( "falcon1",  0x0800, 0x0800, CRC(80eb517d) SHA1(fef4111f656c58b28e7eac5aa5b5cc7e07ccb2fd) )

	ROM_REGION( 0x0060, "proms", 0 )
	ROM_LOAD( "ck6v.bin",     0x0000, 0x0020, CRC(751c3325) SHA1(edce2bc883996c1d72dc6c1c9f62799b162d415a) )
	ROM_LOAD( "ck6u.bin",     0x0020, 0x0020, CRC(ab1940fa) SHA1(8d98e05cbaa6f55770c12e0a9a8ed9c73cc54423) )
	ROM_LOAD( "ck6t.bin",     0x0040, 0x0020, CRC(b4e827a5) SHA1(31a5a5ad54417a474d22bb16c473415d99a2b6f1) )

	ROM_REGION( 0x2000, "samples", 0 )  /* samples */
	ROM_LOAD( "falcon13",    0x0000, 0x1000, CRC(5f0bcdfb) SHA1(7f79bf6de117348f606696ed7ea1937bbf926612) )
	ROM_LOAD( "falcon12",    0x1000, 0x1000, CRC(9003ffbd) SHA1(fd016056aabc23957643f37230f03842294f795e) ) // differs from ckongalc
ROM_END


ROM_START( ckongo )
	ROM_REGION( 0x6000, "maincpu", 0 )
	ROM_LOAD( "o55a-1",       0x0000, 0x1000, CRC(8bfb4623) SHA1(1b8e12d1f337756bbfa9c3d736db7513d571c1b3) )
	ROM_LOAD( "o55a-2",       0x1000, 0x1000, CRC(9ae8089b) SHA1(e50864bb77dce24ba6d10c4fc16ccaa593962442) )
	ROM_LOAD( "o55a-3",       0x2000, 0x1000, CRC(e82b33c8) SHA1(27befba696cd1a9453fb49e8e4ddd46eab41b30d) )
	ROM_LOAD( "o55a-4",       0x3000, 0x1000, CRC(f038f941) SHA1(02be92ef3bf8d36c9916b40109c738965a652a76) )
	ROM_LOAD( "o55a-5",       0x4000, 0x1000, CRC(5182db06) SHA1(f3e981dc3744aff7756f8e0bfd4d92583a02417d) )
	/* no ROM at 5000 */

	ROM_REGION( 0x4000, "gfx1", 0 )
	/* same as ckongpt2 but with halves switched */
	ROM_LOAD( "o50b-1",       0x0000, 0x0800, CRC(cae9e2bf) SHA1(bc62d98840b8b5b296de0f1379baabb1b4d25df0) )
	ROM_CONTINUE(             0x1000, 0x0800 )
	ROM_LOAD( "o50b-2",       0x0800, 0x0800, CRC(fba82114) SHA1(36b7c124edf73b01681f5d63867fafa38a31abbf) )
	ROM_CONTINUE(             0x1800, 0x0800 )
	ROM_LOAD( "o50b-3",       0x2000, 0x0800, CRC(1714764b) SHA1(b025fcc03d45b1ec29be7e292622745544ba891d) )
	ROM_CONTINUE(             0x3000, 0x0800 )
	ROM_LOAD( "o50b-4",       0x2800, 0x0800, CRC(b7008b57) SHA1(9328ff79947dbebdc3e2dd8bcc362667b8201476) )
	ROM_CONTINUE(             0x3800, 0x0800 )

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "c11-02.bin",   0x0000, 0x0800, CRC(d1352c31) SHA1(da726a63a8be830d695afeddc1717749af8c9d47) )
	ROM_LOAD( "a11-01.bin",   0x0800, 0x0800, CRC(a7a2fdbd) SHA1(529865f8bbfbdbbf34ac39c70ef17e6d5bd0f845) )

	ROM_REGION( 0x0060, "proms", 0 )
	ROM_LOAD( "prom.v6",      0x0000, 0x0020, CRC(b3fc1505) SHA1(5b94adde0428a26b815c7eb9b3f3716470d349c7) )
	ROM_LOAD( "prom.u6",      0x0020, 0x0020, CRC(26aada9e) SHA1(f59645e606ea4f0dd0fc4ea47dd03f526c534941) )
	ROM_LOAD( "prom.t6",      0x0040, 0x0020, CRC(676b3166) SHA1(29b9434cd34d43ea5664e436e2a24b54f8d88aac) )

	ROM_REGION( 0x2000, "samples", 0 )  /* samples */
	ROM_LOAD( "cc13j.bin",    0x0000, 0x1000, CRC(5f0bcdfb) SHA1(7f79bf6de117348f606696ed7ea1937bbf926612) )
	ROM_LOAD( "cc12j.bin",    0x1000, 0x1000, CRC(9003ffbd) SHA1(fd016056aabc23957643f37230f03842294f795e) )
ROM_END

ROM_START( ckongalc )
	ROM_REGION( 0x6000, "maincpu", 0 )
	ROM_LOAD( "ck7.bin",      0x0000, 0x1000, CRC(2171cac3) SHA1(7b18bfe44c32fb64b675bbbe2136344522c79b09) )
	ROM_LOAD( "ck8.bin",      0x1000, 0x1000, CRC(88b83ff7) SHA1(4afc494cc264aaa4614da6aed02ce062d9c20850) )
	ROM_LOAD( "ck9.bin",      0x2000, 0x1000, CRC(cff2af47) SHA1(1757428cefad13855a623162101ec01c04006c94) )
	ROM_LOAD( "ck10.bin",     0x3000, 0x1000, CRC(520fa4de) SHA1(6edbaf727756cd33bde94492d72654aa12dbd7e1) )
	ROM_LOAD( "ck11.bin",     0x4000, 0x1000, CRC(327dcadf) SHA1(17b2d3b9e2a82b5278a01cc972cb49705d113127) )
	/* no ROM at 5000 */

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "ck6.bin",      0x0000, 0x1000, CRC(a8916dc8) SHA1(472520aae3837e6026f2a7577d3b2aff371a316c) )
	ROM_LOAD( "ck5.bin",      0x1000, 0x1000, CRC(cd3b5dde) SHA1(2319a2be04d70989b01f4fc703756ba6e1c1f388) )
	ROM_LOAD( "ck4.bin",      0x2000, 0x1000, CRC(b62a0367) SHA1(8c285cbc714d7e6589bd63b3cef7c841ed1c2a4e) )
	ROM_LOAD( "ck3.bin",      0x3000, 0x1000, CRC(61122c5e) SHA1(978b6dbec35f3adc651fddf332db17625099a92e) )

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "alc_ck2.bin",  0x0000, 0x0800, CRC(f67c80f1) SHA1(d1fbcce1b6242f810e106ff50812636e3168ebc1) )
	ROM_LOAD( "alc_ck1.bin",  0x0800, 0x0800, CRC(80eb517d) SHA1(fef4111f656c58b28e7eac5aa5b5cc7e07ccb2fd) )

	ROM_REGION( 0x0060, "proms", 0 )
	ROM_LOAD( "ck6v.bin",     0x0000, 0x0020, CRC(751c3325) SHA1(edce2bc883996c1d72dc6c1c9f62799b162d415a) )
	ROM_LOAD( "ck6u.bin",     0x0020, 0x0020, CRC(ab1940fa) SHA1(8d98e05cbaa6f55770c12e0a9a8ed9c73cc54423) )
	ROM_LOAD( "ck6t.bin",     0x0040, 0x0020, CRC(b4e827a5) SHA1(31a5a5ad54417a474d22bb16c473415d99a2b6f1) )

	ROM_REGION( 0x2000, "samples", 0 )  /* samples */
	ROM_LOAD( "cc13j.bin",    0x0000, 0x1000, CRC(5f0bcdfb) SHA1(7f79bf6de117348f606696ed7ea1937bbf926612) )
	ROM_LOAD( "ck12.bin",     0x1000, 0x1000, CRC(2eb23b60) SHA1(c9e7dc584562aceb374193655fbacb7df6c9c731) )
ROM_END

ROM_START( bigkong )
	ROM_REGION( 0x6000, "maincpu", 0 )
	ROM_LOAD( "dk01f7_2532.d5",   0x0000, 0x1000, CRC(4c9102f1) SHA1(845b48fa1e6ad82dc797520f7ff7daffc1a47c39) )
	ROM_LOAD( "dk02f8_2532.f5",   0x1000, 0x1000, CRC(1683e9ae) SHA1(4690c8be70c0cc0e7d78d2ff205eed3f4ead7278) )
	ROM_LOAD( "dk03f9_2532.h5",   0x2000, 0x1000, CRC(073eea32) SHA1(de7889df04f8a279a0864748298e5ccdce0578f1) )
	ROM_LOAD( "dk04f10_2532.k5",   0x3000, 0x1000, CRC(0aab0334) SHA1(be4b5c121538dc3a82797475f3bb15918eb6d817) )
	ROM_LOAD( "dk05f11_2532.l5",   0x4000, 0x1000, CRC(45be1c6a) SHA1(3d45da4ab21586148a3608d085aa4c401bd257fe) )
	ROM_LOAD( "n05-12.bin",   0x5000, 0x1000, CRC(966bc9ab) SHA1(4434fc620169ffea1b1f227b61674e1daf79b54b) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "n11-06.bin",   0x0000, 0x1000, CRC(2dcedd12) SHA1(dfdcfc21bcba7c8e148ee54daae511ca78c58e70) )
	ROM_LOAD( "l11-05.bin",   0x1000, 0x1000, CRC(fa7cbd91) SHA1(0208d2ebc59f3600005476b6987472685bc99d67) )
	ROM_LOAD( "k11-04.bin",   0x2000, 0x1000, CRC(3375b3bd) SHA1(a00b3c31cff123aab6ac0833aabfdd663302971a) )
	ROM_LOAD( "h11-03.bin",   0x3000, 0x1000, CRC(5655cc11) SHA1(5195e9b2a60c54280b48b32ee8248090904dbc51) )

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "c11-02.bin",   0x0000, 0x0800, CRC(d1352c31) SHA1(da726a63a8be830d695afeddc1717749af8c9d47) )
	ROM_LOAD( "a11-01.bin",   0x0800, 0x0800, CRC(a7a2fdbd) SHA1(529865f8bbfbdbbf34ac39c70ef17e6d5bd0f845) )

	ROM_REGION( 0x0060, "proms", 0 )
	ROM_LOAD( "prom.v6",      0x0000, 0x0020, CRC(b3fc1505) SHA1(5b94adde0428a26b815c7eb9b3f3716470d349c7) )
	ROM_LOAD( "prom.u6",      0x0020, 0x0020, CRC(26aada9e) SHA1(f59645e606ea4f0dd0fc4ea47dd03f526c534941) )
	ROM_LOAD( "prom.t6",      0x0040, 0x0020, CRC(676b3166) SHA1(29b9434cd34d43ea5664e436e2a24b54f8d88aac) )

	ROM_REGION( 0x2000, "samples", 0 )  /* samples */
	ROM_LOAD( "cc13j.bin",    0x0000, 0x1000, CRC(5f0bcdfb) SHA1(7f79bf6de117348f606696ed7ea1937bbf926612) )
	ROM_LOAD( "cc12j.bin",    0x1000, 0x1000, CRC(9003ffbd) SHA1(fd016056aabc23957643f37230f03842294f795e) )
ROM_END

ROM_START( monkeyd )
	ROM_REGION( 0x6000, "maincpu", 0 )
	ROM_LOAD( "ck7.bin",      0x0000, 0x1000, CRC(2171cac3) SHA1(7b18bfe44c32fb64b675bbbe2136344522c79b09) )
	ROM_LOAD( "ck8.bin",      0x1000, 0x1000, CRC(88b83ff7) SHA1(4afc494cc264aaa4614da6aed02ce062d9c20850) )
	ROM_LOAD( "ck9.bin",      0x2000, 0x1000, CRC(cff2af47) SHA1(1757428cefad13855a623162101ec01c04006c94) )
	ROM_LOAD( "ck10.bin",     0x3000, 0x1000, CRC(520fa4de) SHA1(6edbaf727756cd33bde94492d72654aa12dbd7e1) )
	ROM_LOAD( "md5l.bin",     0x4000, 0x1000, CRC(d1db1bb0) SHA1(fe7d700c7f9eca9c389be3717ebebf3e7dc63aa2) )
	/* no ROM at 5000 */

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "ck6.bin",      0x0000, 0x1000, CRC(a8916dc8) SHA1(472520aae3837e6026f2a7577d3b2aff371a316c) )
	ROM_LOAD( "ck5.bin",      0x1000, 0x1000, CRC(cd3b5dde) SHA1(2319a2be04d70989b01f4fc703756ba6e1c1f388) )
	ROM_LOAD( "ck4.bin",      0x2000, 0x1000, CRC(b62a0367) SHA1(8c285cbc714d7e6589bd63b3cef7c841ed1c2a4e) )
	ROM_LOAD( "ck3.bin",      0x3000, 0x1000, CRC(61122c5e) SHA1(978b6dbec35f3adc651fddf332db17625099a92e) )

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "md_ck2.bin",   0x0000, 0x0800, CRC(f67c80f1) SHA1(d1fbcce1b6242f810e106ff50812636e3168ebc1) )
	ROM_LOAD( "md_ck1.bin",   0x0800, 0x0800, CRC(80eb517d) SHA1(fef4111f656c58b28e7eac5aa5b5cc7e07ccb2fd) )

	ROM_REGION( 0x0060, "proms", 0 )
	ROM_LOAD( "ck6v.bin",     0x0000, 0x0020, BAD_DUMP CRC(751c3325) SHA1(edce2bc883996c1d72dc6c1c9f62799b162d415a)  )
	ROM_LOAD( "ck6u.bin",     0x0020, 0x0020, BAD_DUMP CRC(ab1940fa) SHA1(8d98e05cbaa6f55770c12e0a9a8ed9c73cc54423)  )
	ROM_LOAD( "ck6t.bin",     0x0040, 0x0020, BAD_DUMP CRC(b4e827a5) SHA1(31a5a5ad54417a474d22bb16c473415d99a2b6f1)  )

	ROM_REGION( 0x2000, "samples", 0 )  /* samples */
	ROM_LOAD( "cc13j.bin",    0x0000, 0x1000, CRC(5f0bcdfb) SHA1(7f79bf6de117348f606696ed7ea1937bbf926612) )
	ROM_LOAD( "ck12.bin",     0x1000, 0x1000, CRC(2eb23b60) SHA1(c9e7dc584562aceb374193655fbacb7df6c9c731) )
ROM_END


/* Donkey King
1981 (bootleg)

This game runs on dedicated hardware.

Possibly bootlegged by Hafasonic?

CPU Board
---------

MTD-2
|-----------------------------------------|
|C1181  VOL             D5.1K   D7.1N     |
|      LM3900               D6.1M   D8.1R |
|                                         |
|                       6116    D10.2N    |
|                           D9.2M   D11.2R|
|   4066                                  |
|         AY3-8910                        |
|                                  PAL12L6|
|                                         |
|1                                        |
|8               2114 2114                |
|W                                        |
|A                                        |
|Y                                        |
|                                         |
|                                         |
|                    Z80A                 |
|                                         |
|                                         |
|                                         |
|   DSW(8)    82S129.5G                   |
|                                         |
|-----------------------------------------|
Notes:
      Z80 clock - 3.072MHz [18.432/6]
      AY3-8910 clock - 1.536MHz [18.432/12]
      HSync - 15.5065kHz
      VSync - 60.5608Hz


Video Board
-----------

MTD-2B
|-----------------------------------------|
| 18.432MHz                  82S123.1T    |
|          2114                82S123.1U  |
|          2114                  82S123.1V|
|                                         |
|                                         |
|                                         |
|                                         |
|                                         |
|                                         |
|                                         |
|                                         |
|                                         |
|                                         |
|                                         |
|2101 2101    2114 2114                   |
|                                         |
|D12.6A      D1.6H D3.6L    2115 2115 2115|
|   D13.6C      D2.6K  D4.6N              |
|                                         |
|                                         |
|                           2115 2115 2125|
|                                         |
|-----------------------------------------|


18-way Pinout
-------------

Parts          Solder
-------------------------
GND      1     GND
GND      2     GND
GND      3     GND
SPK-     4     SPK+
+12V     5     +12V
         6     P1 UP
         7     P2 UP
         8     VIDEO GND
+5V      9     +5V
P1 DOWN  10
P2 DOWN  11
         12    P2 START
COIN     13    P1 START
P1 JUMP  14    P1 RIGHT
RED      15    P1 LEFT
P2 RIGHT 16    BLUE
P2 LEFT  17    GREEN
P2 JUMP  18    SYNC


Dip Switch - Donkey King
+----------------+-----+-----+-----+-----+-----+-----+-----+-----+
|                |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |
+----------------+-----+-----+-----+-----+-----+-----+-----+-----+
|Life          3 | OFF | OFF |     |     |     |     |     |     |
|                +-----+-----+-----+-----+-----+-----+-----+-----+
|              4 | ON  | OFF |     |     |     |     |     |     |
|                +-----+-----+-----+-----+-----+-----+-----+-----+
|              5 | OFF | ON  |     |     |     |     |     |     |
|                +-----+-----+-----+-----+-----+-----+-----+-----+
|              6 | ON  | ON  |     |     |     |     |     |     |
+----------------+-----+-----+-----+-----+-----+-----+-----+-----+
|Bonus      7000 |     |     | OFF | OFF |     |     |     |     |
|                +-----+-----+-----+-----+-----+-----+-----+-----+
|          10000 |     |     | ON  | OFF |     |     |     |     |
|                +-----+-----+-----+-----+-----+-----+-----+-----+
|          15000 |     |     | OFF | ON  |     |     |     |     |
|                +-----+-----+-----+-----+-----+-----+-----+-----+
|          20000 |     |     | ON  | ON  |     |     |     |     |
+----------------+-----+-----+-----+-----+-----+-----+-----+-----+
|Credit    1C 1P |     |     |     |     | OFF | OFF | OFF |     |
|                +-----+-----+-----+-----+-----+-----+-----+-----+
|          1C 2P |     |     |     |     | OFF | ON  | OFF |     |
|                +-----+-----+-----+-----+-----+-----+-----+-----+
|          1C 3P |     |     |     |     | OFF | OFF | ON  |     |
|                +-----+-----+-----+-----+-----+-----+-----+-----+
|          1C 4P |     |     |     |     | OFF | ON  | ON  |     |
|                +-----+-----+-----+-----+-----+-----+-----+-----+
|          2C 1P |     |     |     |     | ON  | OFF | OFF |     |
|                +-----+-----+-----+-----+-----+-----+-----+-----+
|          3C 1P |     |     |     |     | ON  | ON  | OFF |     |
|                +-----+-----+-----+-----+-----+-----+-----+-----+
|          4C 1P |     |     |     |     | ON  | OFF | ON  |     |
|                +-----+-----+-----+-----+-----+-----+-----+-----+
|          5C 1P |     |     |     |     | ON  | ON  | ON  |     |
+----------------+-----+-----+-----+-----+-----+-----+-----+-----+
|Screen    Table |     |     |     |     |     |     |     | OFF |
|                +-----+-----+-----+-----+-----+-----+-----+-----+
|        Upright |     |     |     |     |     |     |     | ON  |
+----------------+-----+-----+-----+-----+-----+-----+-----+-----+ */




ROM_START( dking )
	ROM_REGION( 0x6000, "maincpu", 0 )
	ROM_LOAD( "d11.r2",       0x0800, 0x0800, CRC(f7cace41) SHA1(981dbb1cddd66a0cbc8fe147172ffe7eb5b7fa21) )
	ROM_CONTINUE( 0x0000, 0x800 )
	ROM_LOAD( "d7.1n",      0x1000, 0x1000, CRC(fe89dea4) SHA1(c39372ebe9950808ebc1ff7909c291496b206026) )
	ROM_LOAD( "d9.2m",      0x2000, 0x1000, CRC(b9c34e14) SHA1(dcfe45dede6aef52a2989978762df9c5463bbbf2) )
	ROM_LOAD( "d10.2n",     0x3000, 0x1000, CRC(243e458d) SHA1(de98fc90915913069b6802d5c662db18f56c36be) )
	ROM_LOAD( "d8.1r",        0x4800, 0x0800, CRC(7c66fb5c) SHA1(5eda9b0037f958433d96bc945c1273b66ef9cac5) )
	ROM_CONTINUE( 0x4000, 0x800 )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "falcon6",      0x0000, 0x1000, CRC(a8916dc8) SHA1(472520aae3837e6026f2a7577d3b2aff371a316c) ) // d4.6n
	ROM_LOAD( "falcon5",      0x1000, 0x1000, CRC(cd3b5dde) SHA1(2319a2be04d70989b01f4fc703756ba6e1c1f388) ) // d3.6l
	ROM_LOAD( "falcon4",      0x2000, 0x1000, CRC(b62a0367) SHA1(8c285cbc714d7e6589bd63b3cef7c841ed1c2a4e) ) // d2.6k
	ROM_LOAD( "falcon3",      0x3000, 0x1000, CRC(61122c5e) SHA1(978b6dbec35f3adc651fddf332db17625099a92e) ) // d1.6h

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "falcon2",  0x0000, 0x0800, CRC(f67c80f1) SHA1(d1fbcce1b6242f810e106ff50812636e3168ebc1) ) // d12.6a
	ROM_LOAD( "falcon1",  0x0800, 0x0800, CRC(80eb517d) SHA1(fef4111f656c58b28e7eac5aa5b5cc7e07ccb2fd) ) // d13.6c

	ROM_REGION( 0x0160, "proms", 0 )
	ROM_LOAD( "ck6v.bin",     0x0000, 0x0020, CRC(751c3325) SHA1(edce2bc883996c1d72dc6c1c9f62799b162d415a) ) // 82s123.1v
	ROM_LOAD( "ck6u.bin",     0x0020, 0x0020, CRC(ab1940fa) SHA1(8d98e05cbaa6f55770c12e0a9a8ed9c73cc54423) ) // 82s123.1u
	ROM_LOAD( "ck6t.bin",     0x0040, 0x0020, CRC(b4e827a5) SHA1(31a5a5ad54417a474d22bb16c473415d99a2b6f1) ) // 82s123.1t
	ROM_LOAD( "82s129.5g",    0x0060, 0x0100, CRC(9e11550d) SHA1(b8cba8e16e10e23fba1f11551102ab77b680bdf0) ) // Decryption Table?

	ROM_REGION( 0x2000, "samples", 0 )  /* samples */
	ROM_LOAD( "falcon13",    0x0000, 0x1000, CRC(5f0bcdfb) SHA1(7f79bf6de117348f606696ed7ea1937bbf926612) ) // d6.1m
	ROM_LOAD( "falcon12",    0x1000, 0x1000, CRC(9003ffbd) SHA1(fd016056aabc23957643f37230f03842294f795e) ) // d5.1k
ROM_END


ROM_START( ckongdks )
	ROM_REGION( 0x6000, "maincpu", 0 )
	ROM_LOAD( "ck13.bin",      0x0800, 0x0800, CRC(f97ba8ae) SHA1(ae4a578ad77a8d3252f2f99a1afa6f38bc00471e) ) // 97.509766%
	ROM_CONTINUE(              0x0000, 0x0800 )
	ROM_LOAD( "ck09.bin",      0x1000, 0x1000, CRC(fe89dea4) SHA1(c39372ebe9950808ebc1ff7909c291496b206026) )
	ROM_LOAD( "ck11.bin",      0x2000, 0x1000, CRC(b3947d06) SHA1(1c5e66e1f11313e11de760cda406c1fe237ce09a) ) // 99.975586%
	ROM_LOAD( "ck12.bin",      0x3000, 0x1000, CRC(23d0657d) SHA1(dfebf3902186a3ab3b36c6d07bdbc832885347b4) ) // 95.214844%
	ROM_LOAD( "ck10.bin",      0x4800, 0x0800, CRC(c27a13f1) SHA1(14f11976bc0e643829a4d4d2d5bb27971979be6f) ) // 94.921875%
	ROM_CONTINUE(              0x4000, 0x0800 )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "ck06.bin",      0x0000, 0x1000, CRC(a8916dc8) SHA1(472520aae3837e6026f2a7577d3b2aff371a316c) )
	ROM_LOAD( "ck05.bin",      0x1000, 0x1000, CRC(cd3b5dde) SHA1(2319a2be04d70989b01f4fc703756ba6e1c1f388) )
	ROM_LOAD( "ck04.bin",      0x2000, 0x1000, CRC(b62a0367) SHA1(8c285cbc714d7e6589bd63b3cef7c841ed1c2a4e) )
	ROM_LOAD( "ck03.bin",      0x3000, 0x1000, CRC(61122c5e) SHA1(978b6dbec35f3adc651fddf332db17625099a92e) )

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "ck02.bin",  0x0000, 0x0800, CRC(085b5f90) SHA1(cce771fbd76c2bc7749325d71c95810898e5b0d9) ) // 98.730469%
	ROM_LOAD( "ck01.bin",  0x0800, 0x0800, CRC(16fd47e2) SHA1(43e5ea70e99482db90681e401a7e1e2d2d36b6f4) ) // 98.339844%

	ROM_REGION( 0x0160, "proms", 0 ) // not dumped, assuming to be the same as dking
	ROM_LOAD( "ck6v.bin",     0x0000, 0x0020, CRC(751c3325) SHA1(edce2bc883996c1d72dc6c1c9f62799b162d415a) ) // 82s123.1v
	ROM_LOAD( "ck6u.bin",     0x0020, 0x0020, CRC(ab1940fa) SHA1(8d98e05cbaa6f55770c12e0a9a8ed9c73cc54423) ) // 82s123.1u
	ROM_LOAD( "ck6t.bin",     0x0040, 0x0020, CRC(b4e827a5) SHA1(31a5a5ad54417a474d22bb16c473415d99a2b6f1) ) // 82s123.1t
	ROM_LOAD( "82s129.5g",    0x0060, 0x0100, CRC(9e11550d) SHA1(b8cba8e16e10e23fba1f11551102ab77b680bdf0) ) // Decryption Table?

	ROM_REGION( 0x2000, "samples", 0 )  /* samples */
	ROM_LOAD( "ck08.bin",    0x0000, 0x1000, CRC(31c0a7de) SHA1(ace23fde4cb3c336b8377c1a1e940607d545e8c3) ) // 97.241211%
	ROM_LOAD( "ck07.bin",    0x1000, 0x1000, CRC(9003ffbd) SHA1(fd016056aabc23957643f37230f03842294f795e) )
ROM_END

/* This set came from a 'Silver Land' board with Silver Land GFX roms, however, the program roms are nearly
   the same as River Patrol but appear to have an original ORCA copyright

   I think the board was a half-converted board as 'Water Gage' and 'Bon Voyage' don't really fit the theme
   of Silver Land so I'm loading the River Patrol GFX instead as they fit better
*/
ROM_START( rpatrol )
	ROM_REGION( 0x6000, "maincpu", 0 )
	ROM_LOAD( "sci1.bin",       0x0000, 0x1000, CRC(33b01c90) SHA1(9c8da6dd963bfb0544ef99b8fdedcf86c32cdb6b) )
	ROM_LOAD( "sci2.bin",       0x1000, 0x1000, CRC(03f53340) SHA1(35336945f4b634fc4c7791ac9c9e6643c8cd8006) )
	ROM_LOAD( "sci3.bin",       0x2000, 0x1000, CRC(8fa300df) SHA1(5c3ba1ef6c1ce8df437b4fa464293208630b5e8d) )
	ROM_LOAD( "sci4.bin",       0x3000, 0x1000, CRC(74a8f1f4) SHA1(6bbc4944e4b31425a6b82f370b6760e5a4b36f56) )
	ROM_LOAD( "sci5.bin",       0x4000, 0x1000, CRC(d7ef6c87) SHA1(38e3b44b355907824919acc4f5064dcb98ebb1d0) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "rp6.6n",       0x0000, 0x0800, CRC(19f18e9e) SHA1(a5500ac36bcda772f3ba79d9e9d37b1eec7bfd13) )
	/* 0x0800-0x0fff - empty */
	ROM_LOAD( "rp7.6l",       0x1000, 0x0800, CRC(07f2070d) SHA1(39df286fda9e48eba6e770fe23a603b5e10d88b6) )
	/* 0x1800-0xffff - empty */
	ROM_LOAD( "rp8.6k",       0x2000, 0x0800, CRC(008738c7) SHA1(a66d9daf31b0d9cf087b591c74f0aaee3d7607b5) )
	/* 0x2800-0x2fff - empty */
	ROM_LOAD( "rp9.6h",       0x3000, 0x0800, CRC(ea5aafca) SHA1(d8f8fe270680ae261d63bd4702107961cd904699) )
	/* 0x3800-0x3fff - empty */

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "rp11.6c",      0x0000, 0x0800, CRC(065651a5) SHA1(5c2f9b44d8819d2f792525c06b5c341fe07329c0) )
	ROM_LOAD( "rp10.6a",      0x0800, 0x0800, CRC(59747c31) SHA1(92acf07489f3e17f0c1769a0df15b6ddb117830f) )

	ROM_REGION( 0x0060, "proms", 0 )
	ROM_LOAD( "bprom1.9n",    0x0000, 0x0020, CRC(f9a2383b) SHA1(4d88c177740efdb27708474c9ee0fcdca5a78c36) )
	ROM_LOAD( "bprom2.9p",    0x0020, 0x0020, CRC(1743bd26) SHA1(9bb50f6e24a7ac3c9ddf3923e57c5532603009e5) )
	ROM_LOAD( "bprom3.9c",    0x0040, 0x0020, CRC(ee03bc96) SHA1(45e33e750a536a904f30136d84dd7993d97e8e54) )

	// these are the GFX Roms found on the board, from Silver Land, but IMO not correct for this program
#if 0
	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "sci46.bin",    0x0000, 0x0800, CRC(affb804f) SHA1(9fc77804690e91773787e06f3329accef075f9f3) )
	/* 0x0800-0x0fff - empty */
	ROM_LOAD( "sci45.bin",    0x1000, 0x0800, CRC(ad4642e5) SHA1(f4de2d9ed0e69c002be07f47247e95167a3ffffb) )
	/* 0x1800-0xffff - empty */
	ROM_LOAD( "sci44.bin",    0x2000, 0x0800, CRC(e487579d) SHA1(aed59f15dbc904d73e19d914ccd0a86fda859085) )
	/* 0x2800-0x2fff - empty */
	ROM_LOAD( "sci43.bin",    0x3000, 0x0800, CRC(59125a1a) SHA1(37638fb690d6b4f11585f6a13586271c2f0e3743) )
	/* 0x3800-0x3fff - empty */

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "sci42.bin",         0x0000, 0x0800, CRC(c8d32b8e) SHA1(7d655d243ed13cf2537f3fdfde5bf34229f7cb84) )
	ROM_LOAD( "sci41.bin",         0x0800, 0x0800, CRC(ee333daf) SHA1(b02998dccec9a4f841838874221caabae8380fcc) )

	ROM_REGION( 0x0060, "proms", 0 )
	ROM_LOAD( "mb7051.1v",    0x0000, 0x0020, CRC(1d2343b1) SHA1(294f22178af4532abf767c1ffe2dc831bbe683bf) )
	ROM_LOAD( "mb7051.1u",    0x0020, 0x0020, CRC(c174753c) SHA1(303bfb1f470b525ccaeafa81a38a4bc3a7de5dbb) )
	ROM_LOAD( "mb7051.1t",    0x0040, 0x0020, CRC(04a1be01) SHA1(9c270c04d374d46752ec99bd4e79fed1e2896bc0) )
#endif
	/* no samples */
ROM_END

ROM_START( rpatrolb )
	ROM_REGION( 0x6000, "maincpu", 0 )
	ROM_LOAD( "rp1.4l",       0x0000, 0x1000, CRC(bfd7ae7a) SHA1(a06d1cc2674ed40d0bfa67dd6d724964c1e40600) )
	ROM_LOAD( "rp2.4j",       0x1000, 0x1000, CRC(03f53340) SHA1(35336945f4b634fc4c7791ac9c9e6643c8cd8006) )
	ROM_LOAD( "rp3.4f",       0x2000, 0x1000, CRC(8fa300df) SHA1(5c3ba1ef6c1ce8df437b4fa464293208630b5e8d) )
	ROM_LOAD( "rp4.4e",       0x3000, 0x1000, CRC(74a8f1f4) SHA1(6bbc4944e4b31425a6b82f370b6760e5a4b36f56) )
	ROM_LOAD( "rp5.4c",       0x4000, 0x1000, CRC(d7ef6c87) SHA1(38e3b44b355907824919acc4f5064dcb98ebb1d0) )
	/* no ROM at 5000 */

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "rp6.6n",       0x0000, 0x0800, CRC(19f18e9e) SHA1(a5500ac36bcda772f3ba79d9e9d37b1eec7bfd13) )
	/* 0x0800-0x0fff - empty */
	ROM_LOAD( "rp7.6l",       0x1000, 0x0800, CRC(07f2070d) SHA1(39df286fda9e48eba6e770fe23a603b5e10d88b6) )
	/* 0x1800-0xffff - empty */
	ROM_LOAD( "rp8.6k",       0x2000, 0x0800, CRC(008738c7) SHA1(a66d9daf31b0d9cf087b591c74f0aaee3d7607b5) )
	/* 0x2800-0x2fff - empty */
	ROM_LOAD( "rp9.6h",       0x3000, 0x0800, CRC(ea5aafca) SHA1(d8f8fe270680ae261d63bd4702107961cd904699) )
	/* 0x3800-0x3fff - empty */

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "rp11.6c",      0x0000, 0x0800, CRC(065651a5) SHA1(5c2f9b44d8819d2f792525c06b5c341fe07329c0) )
	ROM_LOAD( "rp10.6a",      0x0800, 0x0800, CRC(59747c31) SHA1(92acf07489f3e17f0c1769a0df15b6ddb117830f) )

	ROM_REGION( 0x0060, "proms", 0 )
	ROM_LOAD( "bprom1.9n",    0x0000, 0x0020, CRC(f9a2383b) SHA1(4d88c177740efdb27708474c9ee0fcdca5a78c36) )
	ROM_LOAD( "bprom2.9p",    0x0020, 0x0020, CRC(1743bd26) SHA1(9bb50f6e24a7ac3c9ddf3923e57c5532603009e5) )
	ROM_LOAD( "bprom3.9c",    0x0040, 0x0020, CRC(ee03bc96) SHA1(45e33e750a536a904f30136d84dd7993d97e8e54) )

	/* no samples */
ROM_END

ROM_START( silvland )
	ROM_REGION( 0x6000, "maincpu", 0 )
	ROM_LOAD( "7.2r",         0x0000, 0x1000, CRC(57e6be62) SHA1(c1d47970f8209256c9ccd6512b921dec6c276998) )
	ROM_LOAD( "8.1n",         0x1000, 0x1000, CRC(bbb2b287) SHA1(93cd4ebe238c189c80be8b8ab1ec2649256dd6ea) )
	ROM_LOAD( "rp3.4f",       0x2000, 0x1000, CRC(8fa300df) SHA1(5c3ba1ef6c1ce8df437b4fa464293208630b5e8d) )
	ROM_LOAD( "10.2n",        0x3000, 0x1000, CRC(5536a65d) SHA1(0bf2b9ea76fd6fd8c0475bf6f49a42f1c96d3906) )
	ROM_LOAD( "11.1r",        0x4000, 0x1000, CRC(6f23f66f) SHA1(3ca8075c28956ec473ccb0e9f05e9ad8669f743d) )
	ROM_LOAD( "12.2k",        0x5000, 0x1000, CRC(26f1537c) SHA1(0468352d49edec3a52e32612856735b78e11079b) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "6.6n",         0x0000, 0x0800, CRC(affb804f) SHA1(9fc77804690e91773787e06f3329accef075f9f3) )
	/* 0x0800-0x0fff - empty */
	ROM_LOAD( "5.6l",         0x1000, 0x0800, CRC(ad4642e5) SHA1(f4de2d9ed0e69c002be07f47247e95167a3ffffb) )
	/* 0x1800-0xffff - empty */
	ROM_LOAD( "4.6k",         0x2000, 0x0800, CRC(e487579d) SHA1(aed59f15dbc904d73e19d914ccd0a86fda859085) )
	/* 0x2800-0x2fff - empty */
	ROM_LOAD( "3.6h",         0x3000, 0x0800, CRC(59125a1a) SHA1(37638fb690d6b4f11585f6a13586271c2f0e3743) )
	/* 0x3800-0x3fff - empty */

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "2.6c",         0x0000, 0x0800, CRC(c8d32b8e) SHA1(7d655d243ed13cf2537f3fdfde5bf34229f7cb84) )
	ROM_LOAD( "1.6a",         0x0800, 0x0800, CRC(ee333daf) SHA1(b02998dccec9a4f841838874221caabae8380fcc) )

	ROM_REGION( 0x0060, "proms", 0 )
	ROM_LOAD( "mb7051.1v",    0x0000, 0x0020, CRC(1d2343b1) SHA1(294f22178af4532abf767c1ffe2dc831bbe683bf) )
	ROM_LOAD( "mb7051.1u",    0x0020, 0x0020, CRC(c174753c) SHA1(303bfb1f470b525ccaeafa81a38a4bc3a7de5dbb) )
	ROM_LOAD( "mb7051.1t",    0x0040, 0x0020, CRC(04a1be01) SHA1(9c270c04d374d46752ec99bd4e79fed1e2896bc0) )

	/* no samples */
ROM_END

/*This dump was a mess.  11n and 11k seem to be bad dumps, the second half should probably be sprite data
  Comparing to set 2 11l and 11h are unnecessary, and are actually from Le Bagnard(set1), as is 5m.
  5n ID'd as unknown, but it also is from bagnard with some patches. */
ROM_START( cannonb )
	ROM_REGION( 0x11000, "maincpu", 0 )
	ROM_LOAD( "canballs.5d", 0x10000, 0x1000, CRC(43ad0d16) SHA1(682f1ee15e41bb5a161287536bb97704c0d3be9c) ) /* only this one ROM is encrypted */
	ROM_LOAD( "canballs.5f",  0x1000, 0x1000, CRC(3e0dacdd) SHA1(cdd3684a6962f2fb582b8a415383c06a5e5059dd) )
	ROM_LOAD( "canballs.5h",  0x2000, 0x1000, CRC(e18a836b) SHA1(19b90a55db82914c5db18486e05d9f59aba1b442) )
	ROM_LOAD( "canballs.5k",  0x3000, 0x0800, CRC(6ed3cbf4) SHA1(070ba61dc97df6be8004f7e052a4cef836234888) )

	ROM_REGION( 0x4000, "gfx1", 0 ) // using gfx roms from other sets because the ones in this set were bad
	ROM_LOAD( "cb10.bin",   0x0000, 0x0800, CRC(602a6c2d) SHA1(788f83bcb0667d8a42c209f3d51708d496be58df) )
	/* 0x0800-0x0fff - empty */
	ROM_CONTINUE(           0x1000, 0x0800 )
	/* 0x1800-0x0fff - empty */
	ROM_LOAD( "cb9.bin",    0x2000, 0x0800, CRC(2d036026) SHA1(b6eada3e67edd7db59d9ca823b798cd20f0afca9) )
	/* 0x2800-0x0fff - empty */
	ROM_CONTINUE(           0x3000, 0x0800 )
	/* 0x3800-0x0fff - empty */

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "canballs.11c", 0x0000, 0x0800, CRC(d1352c31) SHA1(da726a63a8be830d695afeddc1717749af8c9d47) )
	ROM_LOAD( "canballs.11a", 0x0800, 0x0800, CRC(a7a2fdbd) SHA1(529865f8bbfbdbbf34ac39c70ef17e6d5bd0f845) )

	ROM_REGION( 0x0060, "proms", 0 )
	ROM_LOAD( "prom.v6",      0x0000, 0x0020, CRC(b3fc1505) SHA1(5b94adde0428a26b815c7eb9b3f3716470d349c7) )
	ROM_LOAD( "prom.u6",      0x0020, 0x0020, CRC(26aada9e) SHA1(f59645e606ea4f0dd0fc4ea47dd03f526c534941) )
	ROM_LOAD( "prom.t6",      0x0040, 0x0020, CRC(676b3166) SHA1(29b9434cd34d43ea5664e436e2a24b54f8d88aac) )

	ROM_REGION( 0x2000, "samples", 0 )  /* samples */
	ROM_LOAD( "canballs.5s",  0x0000, 0x1000, CRC(5f0bcdfb) SHA1(7f79bf6de117348f606696ed7ea1937bbf926612) )
	ROM_LOAD( "canballs.5p",  0x1000, 0x1000, CRC(9003ffbd) SHA1(fd016056aabc23957643f37230f03842294f795e) )

	ROM_REGION( 0x6000, "unused", ROMREGION_ERASEFF )   /* unused roms, don't seem to belong to this game - they come from the bagman on crazy kong conversion, see below */
	//ROM_LOAD( "canballs.5m",  0x0000, 0x1000, CRC(4f0088ab) SHA1(a8009f5b8517ba4d84fbc483b199f2514f24eae8) ) // patched 'le bagnard' cod rom
	//ROM_LOAD( "canballs.5n",  0x1000, 0x1000, CRC(91570033) SHA1(7cd7fe9541da36c3919324bc65e6db1d1ca635e0) ) // ?
	//ROM_LOAD( "canballs.11l", 0x2000, 0x1000, CRC(060b044c) SHA1(3121f07adb661663a2303085eea1b662968f8f98) ) // 'le bagnard gfx'
	//ROM_LOAD( "canballs.11h", 0x3000, 0x1000, CRC(8043bc1a) SHA1(bd2f3dfe26cf8d987d9ecaa41eac4bdc4e16a692) ) // 'le bagnard gfx'
	ROM_LOAD( "canballs.11n", 0x4000, 0x1000, CRC(a95b8e03) SHA1(e78125023e1af6de292292b875b45401b2173ca9) ) // probably just bad dump (missing sprite data)
	ROM_LOAD( "canballs.11k", 0x5000, 0x1000, CRC(dbbe8263) SHA1(efe4bba25a03261bc8309e6d83d5600def875b0c) ) // probably just bad dump (missing sprite data)
ROM_END

ROM_START( cannonb2 )
	ROM_REGION( 0x6000, "maincpu", 0 )
	ROM_LOAD( "cb1.bin",   0x0000, 0x1000, CRC(7a3cba7c) SHA1(08b8b356fdbe642e80d42b5ab4164a1bd6ad93ba) )
	ROM_LOAD( "cb2.bin",   0x1000, 0x1000, CRC(58ef3118) SHA1(51ae36c21147e99d4060034520f6eebf3210937c) )
	ROM_LOAD( "cb3.bin",   0x2000, 0x1000, CRC(e18a836b) SHA1(19b90a55db82914c5db18486e05d9f59aba1b442) )
	ROM_LOAD( "cb4.bin",   0x3000, 0x1000, CRC(696ebdb0) SHA1(0bff115e4710199641722ca12af4e16dc5b0ec13) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "cb10.bin",   0x0000, 0x0800, CRC(602a6c2d) SHA1(788f83bcb0667d8a42c209f3d51708d496be58df) )
	/* 0x0800-0x0fff - empty */
	ROM_CONTINUE(           0x1000, 0x0800 )
	/* 0x1800-0x0fff - empty */
	ROM_LOAD( "cb9.bin",    0x2000, 0x0800, CRC(2d036026) SHA1(b6eada3e67edd7db59d9ca823b798cd20f0afca9) )
	/* 0x2800-0x0fff - empty */
	ROM_CONTINUE(           0x3000, 0x0800 )
	/* 0x3800-0x0fff - empty */

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "cb7.bin",   0x0000, 0x0800, CRC(80eb517d) SHA1(fef4111f656c58b28e7eac5aa5b5cc7e07ccb2fd) )
	ROM_LOAD( "cb8.bin",   0x0800, 0x0800, CRC(f67c80f1) SHA1(d1fbcce1b6242f810e106ff50812636e3168ebc1) )

	ROM_REGION( 0x0060, "proms", 0 )
	ROM_LOAD( "v6.bin",      0x0000, 0x0020, CRC(751c3325) SHA1(edce2bc883996c1d72dc6c1c9f62799b162d415a) )
	ROM_LOAD( "u6.bin",      0x0020, 0x0020, CRC(c0539747) SHA1(1bc70057b59b8cb11299fb6b0d84a46da6c0a025) )
	ROM_LOAD( "t6.bin",      0x0040, 0x0020, CRC(b4e827a5) SHA1(31a5a5ad54417a474d22bb16c473415d99a2b6f1) )

	ROM_REGION( 0x2000, "samples", 0 )  /* samples */
	ROM_LOAD( "cb6.bin",    0x0000, 0x1000, CRC(5f0bcdfb) SHA1(7f79bf6de117348f606696ed7ea1937bbf926612) )
	ROM_LOAD( "cb5.bin",    0x1000, 0x1000, CRC(9003ffbd) SHA1(fd016056aabc23957643f37230f03842294f795e) )
ROM_END

ROM_START( cannonb3 )
	ROM_REGION( 0x6000, "maincpu", 0 )
	ROM_LOAD( "1 pos e5  2532.bin",  0x0000, 0x1000, CRC(4b482e80) SHA1(4c0e52016ed760d399e8d49f600d38c1b6ccf256) )
	ROM_LOAD( "2 pos f5  2532.bin",  0x1000, 0x1000, CRC(1fa050af) SHA1(00244e19aee14ce980697136cddb6cb72a4d80da) )
	ROM_LOAD( "3 pos h5  2532.bin",  0x2000, 0x1000, CRC(e18a836b) SHA1(19b90a55db82914c5db18486e05d9f59aba1b442) )
	ROM_LOAD( "4 pos k5  2716.bin",  0x3000, 0x0800, CRC(58e04f41) SHA1(d1d0adb36bd509928c0e1a3a0ee9ba296aa530ab) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "b pos n11  2532.bin",   0x0000, 0x0800, CRC(602a6c2d) SHA1(788f83bcb0667d8a42c209f3d51708d496be58df) )
	/* 0x0800-0x0fff - empty */
	ROM_CONTINUE(           0x1000, 0x0800 )
	/* 0x1800-0x0fff - empty */
	ROM_LOAD( "a pos k11  2532.bin",    0x2000, 0x0800, CRC(2d036026) SHA1(b6eada3e67edd7db59d9ca823b798cd20f0afca9) )
	/* 0x2800-0x0fff - empty */
	ROM_CONTINUE(           0x3000, 0x0800 )
	/* 0x3800-0x0fff - empty */

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "ck2 pos c11  2716.bin", 0x0000, 0x0800, CRC(d1352c31) SHA1(da726a63a8be830d695afeddc1717749af8c9d47) )
	ROM_LOAD( "ck1 pos a11  2716.bin", 0x0800, 0x0800, CRC(a7a2fdbd) SHA1(529865f8bbfbdbbf34ac39c70ef17e6d5bd0f845) )

	ROM_REGION( 0x0060, "proms", 0 )
	ROM_LOAD( "c pos v6  n82s123n.bin",      0x0000, 0x0020, CRC(b3fc1505) SHA1(5b94adde0428a26b815c7eb9b3f3716470d349c7) )
	ROM_LOAD( "b pos u6  n82s123n.bin",      0x0020, 0x0020, CRC(a758b567) SHA1(d188c90dba10fe3abaae92488786b555b35218c5) )
	ROM_LOAD( "a pos t6  n82s123n.bin",      0x0040, 0x0020, CRC(676b3166) SHA1(29b9434cd34d43ea5664e436e2a24b54f8d88aac) )

	ROM_REGION( 0x2000, "samples", 0 )  /* samples */
	ROM_LOAD( "ck14 pos s5  2532.bin",  0x0000, 0x1000, CRC(5f0bcdfb) SHA1(7f79bf6de117348f606696ed7ea1937bbf926612) )
	ROM_LOAD( "ck13 pos r5  2532.bin",  0x1000, 0x1000, CRC(9003ffbd) SHA1(fd016056aabc23957643f37230f03842294f795e) )
ROM_END


/* PCB FCK-00 (Falcon Crazy Kong) + Daughterboard
   running Bagman

   there are a lot of wire-mods on this board, it's possible that it's been hacked to work more like
   an original bagman board, a lot of the roms are the same

   this set also explains why the cannonball set above contained a number of bagman roms, it was clearly
   a half-converted board.

*/
ROM_START( bagmanf )
	ROM_REGION( 0x6000, "maincpu", 0 )
	ROM_LOAD( "01.d5.bin",  0x0000, 0x1000, CRC(e0156191) SHA1(bb5f16d49fbe48f3bac118acd1fea51ec4bc5355) )
	ROM_LOAD( "02.f5.bin",  0x1000, 0x1000, CRC(7b758982) SHA1(c8460023b43fed4aca9c6b987faea334832c5e30) )
	ROM_LOAD( "03.h5.bin",  0x2000, 0x1000, CRC(302a077b) SHA1(916c4a6ea1e631cc72bdb91ff9d263dcbaf08bb2) )
	ROM_LOAD( "04.k5.bin",  0x3000, 0x1000, CRC(b704d761) SHA1(60f5f84a7c43ef50cf8ea81d566a3b23ca4f57c4) )
	ROM_LOAD( "05.l5.bin",  0x4000, 0x1000, CRC(4f0088ab) SHA1(a8009f5b8517ba4d84fbc483b199f2514f24eae8) )
	ROM_LOAD( "2732 05 pos dboard.bin",  0x5000, 0x1000, CRC(91570033) SHA1(7cd7fe9541da36c3919324bc65e6db1d1ca635e0) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "09.l11.bin", 0x0000, 0x0800, CRC(060b044c) SHA1(3121f07adb661663a2303085eea1b662968f8f98) )
	/* 0x0800-0x0fff - empty */
	ROM_CONTINUE(           0x1000, 0x0800 )
	/* 0x1800-0x0fff - empty */
	ROM_LOAD( "11.h11.bin", 0x2000, 0x0800, CRC(8043bc1a) SHA1(bd2f3dfe26cf8d987d9ecaa41eac4bdc4e16a692) )
	/* 0x2800-0x0fff - empty */
	ROM_CONTINUE(           0x3000, 0x0800 )
	/* 0x3800-0x0fff - empty */

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "10.n11.bin",   0x0000, 0x1000, CRC(c680ef04) SHA1(79406bc786374abfcd9f548268c445b5c8d8858d) )
	ROM_LOAD( "12.k11.bin",   0x1000, 0x1000, CRC(4a0a6b55) SHA1(955f8bd4bd9b0fc3c6c359c25ba543ba26c04cbd) )

	ROM_REGION( 0x0060, "proms", 0 )
	ROM_LOAD( "v6",      0x0000, 0x0020, CRC(b3fc1505) SHA1(5b94adde0428a26b815c7eb9b3f3716470d349c7) )
	ROM_LOAD( "u6",      0x0020, 0x0020, CRC(26aada9e) SHA1(f59645e606ea4f0dd0fc4ea47dd03f526c534941) )
	ROM_LOAD( "t6",      0x0040, 0x0020, CRC(676b3166) SHA1(29b9434cd34d43ea5664e436e2a24b54f8d88aac) )

	/* as well as one of the program roms, the daughterboard contains the speech hardware + roms */
	ROM_REGION( 0x0020, "proms2", 0 )
	ROM_LOAD( "daughterboard.prom",  0x0000, 0x0020, CRC(c58a4f6a) SHA1(35ef244b3e94032df2610aa594ea5670b91e1449) ) /*state machine driving TMS5110*/

	ROM_REGION( 0x2000, "speech", 0 ) /* data for the TMS5110 speech chip */
	ROM_LOAD( "2732 07 pos dboard.bin",   0x0000, 0x1000, CRC(2e0057ff) SHA1(33e3ffa6418f86864eb81e5e9bda4bf540c143a6) )
	ROM_LOAD( "2732 08 pos dboard.bin",   0x1000, 0x1000, CRC(b2120edd) SHA1(52b89dbcc749b084331fa82b13d0876e911fce52) )

	ROM_REGION( 0x2000, "samples", ROMREGION_ERASE00 )  /* samples */
	/* unpopulated */
ROM_END


ROM_START( swimmer )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "sw1",          0x0000, 0x1000, CRC(f12481e7) SHA1(4e8ee509043fd57ec1579594f0b2c543f270bead) )
	ROM_LOAD( "sw2",          0x1000, 0x1000, CRC(a0b6fdd2) SHA1(7d3603de6c282224869824c7572868fc85599ea2) )
	ROM_LOAD( "sw3",          0x2000, 0x1000, CRC(ec93d7de) SHA1(e225c6b98eb3c32825c1cc1fcf69dec7e340460c) )
	ROM_LOAD( "sw4",          0x3000, 0x1000, CRC(0107927d) SHA1(419aeca37c7604f71f49e3dee36f477eee0ba53a) )
	ROM_LOAD( "sw5",          0x4000, 0x1000, CRC(ebd8a92c) SHA1(65401f8d39250f6ec61841e58ce4c21ddfe99842) )
	ROM_LOAD( "sw6",          0x5000, 0x1000, CRC(f8539821) SHA1(82f43ecbbb0a3771632eb26e10bc5453d74b65b1) )
	ROM_LOAD( "sw7",          0x6000, 0x1000, CRC(37efb64e) SHA1(0ed4d678895c17b37df605990acd096c538e3675) )
	ROM_LOAD( "sw8",          0x7000, 0x1000, CRC(33d6001e) SHA1(749b746d018e74e364fd6974e4522c8a18915774) )

	ROM_REGION( 0x1000, "audiocpu", 0 )
	ROM_LOAD( "sw12.4k",      0x0000, 0x1000, CRC(2eee9bcb) SHA1(ceafdf750a8af0c1c9abbbf437c3e9d9ae09f72b) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_LOAD( "sw15.18k",     0x0000, 0x1000, CRC(4f3608cb) SHA1(cebcad69c4ad5dacc0bf597fdaed6f8950ffdfe1) )  /* chars */
	ROM_LOAD( "sw14.18l",     0x1000, 0x1000, CRC(7181c8b4) SHA1(b22fa0ebac884002cf6f5651e4366f30d0ab09f5) )
	ROM_LOAD( "sw13.18m",     0x2000, 0x1000, CRC(2eb1af5c) SHA1(0105d03adfc5ce9ca478e678a1e1d8bae7c516e0) )

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "sw23.6c",      0x0000, 0x0800, CRC(9ca67e24) SHA1(86f561abc1a1c6b0c29c6017246d805c5a48b999) )  /* bigsprite data */
	ROM_RELOAD(               0x0800, 0x0800 )  /* Guzzler has larger ROMs */
	ROM_LOAD( "sw22.5c",      0x1000, 0x0800, CRC(02c10992) SHA1(8c383fdcd83aa9997e5802a58419b9d993a9b38d) )
	ROM_RELOAD(               0x1800, 0x0800 )  /* Guzzler has larger ROMs */
	ROM_LOAD( "sw21.4c",      0x2000, 0x0800, CRC(7f4993c1) SHA1(a5884b3af707109e810cf1f38bee3cb642e619f6) )
	ROM_RELOAD(               0x2800, 0x0800 )  /* Guzzler has larger ROMs */

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "24s10.13b",    0x0000, 0x100, CRC(8e35b97d) SHA1(2e2c254574660e01b9983f795a2adb5b9911d7f0) )
	ROM_LOAD( "24s10.13a",    0x0100, 0x100, CRC(c5f24909) SHA1(27f2c967d440f6387841aa3f7b116c64bb812af1) )
	ROM_LOAD( "18s030.12c",   0x0200, 0x020, CRC(3b2deb3a) SHA1(bb7b5c662454f5b355cc59cbdf8879e4664bed1d) )
ROM_END

ROM_START( swimmera )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "swa1",         0x0000, 0x1000, CRC(42c2b6c5) SHA1(13688e1ee08308b13ead5af7b4f65043dae4e40f) )
	ROM_LOAD( "swa2",         0x1000, 0x1000, CRC(49bac195) SHA1(a5d2cc2cdd10003f69014c4799f5f59e47a44260) )
	ROM_LOAD( "swa3",         0x2000, 0x1000, CRC(a6d8cb01) SHA1(80ab0ffaee6e0edf19b767229865722c2af6112c) )
	ROM_LOAD( "swa4",         0x3000, 0x1000, CRC(7be75182) SHA1(4fe7bc6382ea7311be1225fb0715aa2ff4ec084c) )
	ROM_LOAD( "swa5",         0x4000, 0x1000, CRC(78f79573) SHA1(6124fae47b3fa2e5965dffdfe9cbeb96acf08314) )
	ROM_LOAD( "swa6",         0x5000, 0x1000, CRC(fda9b311) SHA1(d9c914ad27f5988d0d4da5c942fb12bb5728cdfb) )
	ROM_LOAD( "swa7",         0x6000, 0x1000, CRC(7090e5ee) SHA1(d1e0ca38c3d1e4a7b7efa3696e47fb36ad3f8aa0) )
	ROM_LOAD( "swa8",         0x7000, 0x1000, CRC(ab86efa9) SHA1(5b5a80ae285c7e9f4c51e646116edf789d4dba39) )

	ROM_REGION( 0x1000, "audiocpu", 0 )
	ROM_LOAD( "sw12.4k",      0x0000, 0x1000, CRC(2eee9bcb) SHA1(ceafdf750a8af0c1c9abbbf437c3e9d9ae09f72b) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_LOAD( "sw15.18k",     0x0000, 0x1000, CRC(4f3608cb) SHA1(cebcad69c4ad5dacc0bf597fdaed6f8950ffdfe1) )  /* chars */
	ROM_LOAD( "sw14.18l",     0x1000, 0x1000, CRC(7181c8b4) SHA1(b22fa0ebac884002cf6f5651e4366f30d0ab09f5) )
	ROM_LOAD( "sw13.18m",     0x2000, 0x1000, CRC(2eb1af5c) SHA1(0105d03adfc5ce9ca478e678a1e1d8bae7c516e0) )

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "sw23.6c",      0x0000, 0x0800, CRC(9ca67e24) SHA1(86f561abc1a1c6b0c29c6017246d805c5a48b999) )  /* bigsprite data */
	ROM_RELOAD(               0x0800, 0x0800 )  /* Guzzler has larger ROMs */
	ROM_LOAD( "sw22.5c",      0x1000, 0x0800, CRC(02c10992) SHA1(8c383fdcd83aa9997e5802a58419b9d993a9b38d) )
	ROM_RELOAD(               0x1800, 0x0800 )  /* Guzzler has larger ROMs */
	ROM_LOAD( "sw21.4c",      0x2000, 0x0800, CRC(7f4993c1) SHA1(a5884b3af707109e810cf1f38bee3cb642e619f6) )
	ROM_RELOAD(               0x2800, 0x0800 )  /* Guzzler has larger ROMs */

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "24s10.13b",    0x0000, 0x100, CRC(8e35b97d) SHA1(2e2c254574660e01b9983f795a2adb5b9911d7f0) )
	ROM_LOAD( "24s10.13a",    0x0100, 0x100, CRC(c5f24909) SHA1(27f2c967d440f6387841aa3f7b116c64bb812af1) )
	ROM_LOAD( "18s030.12c",   0x0200, 0x020, CRC(3b2deb3a) SHA1(bb7b5c662454f5b355cc59cbdf8879e4664bed1d) )
ROM_END

ROM_START( swimmerb )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "sw1.9l",       0x0000, 0x1000, CRC(b045be08) SHA1(52187e1daebec521a98157f22960637393e40e62) )
	ROM_LOAD( "sw2.9k",       0x1000, 0x1000, CRC(163d65e5) SHA1(b505f05af96f241285f0f7082ed03fa07bbde7de) )
	ROM_LOAD( "sw3.9j",       0x2000, 0x1000, CRC(631d74e9) SHA1(b6adba9445264de80f5daf33dad1c90b23617648) )
	ROM_LOAD( "sw4.9f",       0x3000, 0x1000, CRC(d62634db) SHA1(c6d0d2cf7a3a19fac1752a30189f31eb3df8fa42) )
	ROM_LOAD( "sw5.9e",       0x4000, 0x1000, CRC(922d5d87) SHA1(e5f111d82a072e59c00b759eaada195f1fc06532) )
	ROM_LOAD( "sw6.9d",       0x5000, 0x1000, CRC(85478209) SHA1(df3c79ca25229fef2fe0f48d3c173e389628a68d) )
	ROM_LOAD( "sw7.9c",       0x6000, 0x1000, CRC(88266f2e) SHA1(4ad15f9ba7b45a6c1c3637f8d0fd8be9c04b495f) )
	ROM_LOAD( "sw8.9a",       0x7000, 0x1000, CRC(191a16e4) SHA1(75d3f49e2f4ea04d3a7cc88662c023768bf48365) )

	ROM_REGION( 0x1000, "audiocpu", 0 )
	ROM_LOAD( "sw12.4k",      0x0000, 0x1000, CRC(2eee9bcb) SHA1(ceafdf750a8af0c1c9abbbf437c3e9d9ae09f72b) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_LOAD( "sw15.18k",     0x0000, 0x1000, CRC(4f3608cb) SHA1(cebcad69c4ad5dacc0bf597fdaed6f8950ffdfe1) )  /* chars */
	ROM_LOAD( "sw14.18l",     0x1000, 0x1000, CRC(7181c8b4) SHA1(b22fa0ebac884002cf6f5651e4366f30d0ab09f5) )
	ROM_LOAD( "sw13.18m",     0x2000, 0x1000, CRC(2eb1af5c) SHA1(0105d03adfc5ce9ca478e678a1e1d8bae7c516e0) )

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "sw23.6c",      0x0000, 0x0800, CRC(9ca67e24) SHA1(86f561abc1a1c6b0c29c6017246d805c5a48b999) )  /* bigsprite data */
	ROM_RELOAD(               0x0800, 0x0800 )  /* Guzzler has larger ROMs */
	ROM_LOAD( "sw22.5c",      0x1000, 0x0800, CRC(02c10992) SHA1(8c383fdcd83aa9997e5802a58419b9d993a9b38d) )
	ROM_RELOAD(               0x1800, 0x0800 )  /* Guzzler has larger ROMs */
	ROM_LOAD( "sw21.4c",      0x2000, 0x0800, CRC(7f4993c1) SHA1(a5884b3af707109e810cf1f38bee3cb642e619f6) )
	ROM_RELOAD(               0x2800, 0x0800 )  /* Guzzler has larger ROMs */

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "24s10.13b",    0x0000, 0x100, CRC(8e35b97d) SHA1(2e2c254574660e01b9983f795a2adb5b9911d7f0) )
	ROM_LOAD( "24s10.13a",    0x0100, 0x100, CRC(c5f24909) SHA1(27f2c967d440f6387841aa3f7b116c64bb812af1) )
	ROM_LOAD( "18s030.12c",   0x0200, 0x020, CRC(3b2deb3a) SHA1(bb7b5c662454f5b355cc59cbdf8879e4664bed1d) )
ROM_END

ROM_START( guzzler )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "guzz-01.bin",  0x0000, 0x2000, CRC(58aaa1e9) SHA1(4ea9c85670a0d71483ac79564093043762a24b2c) )
	ROM_LOAD( "guzz-02.bin",  0x2000, 0x2000, CRC(f80ceb17) SHA1(eedff7355fb5aa18b82f0a3e39bba5521c359791) )
	ROM_LOAD( "guzz-03.bin",  0x4000, 0x2000, CRC(e63c65a2) SHA1(e2b888911330690faa3a041e1a17d838b46e6bbd) )
	ROM_LOAD( "guzz-04.bin",  0x6000, 0x2000, CRC(45be42f5) SHA1(578943afdb6ceca34ca7c19c2fd1164ca3aa57bd) )
	ROM_LOAD( "guzz-16.bin",  0xe000, 0x2000, CRC(61ee00b7) SHA1(ea8516c8dfb2de32a8034f94c7d0c086e3596740) )

	ROM_REGION( 0x1000, "audiocpu", 0 )
	ROM_LOAD( "guzz-12.bin",  0x0000, 0x1000, CRC(f3754d9e) SHA1(bb30832aba4e82ab0ecce40fc1223d9771ff7dd2) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_LOAD( "guzz-13.bin",  0x0000, 0x1000, CRC(afc464e2) SHA1(61730b5e5add24ba3d4e8903c5d71cf4df9b77e0) )   /* chars */
	ROM_LOAD( "guzz-14.bin",  0x1000, 0x1000, CRC(acbdfe1f) SHA1(ab7abe4bb321fc7dc4e73acab4b1a7133e6bcf20) )
	ROM_LOAD( "guzz-15.bin",  0x2000, 0x1000, CRC(66978c05) SHA1(2c8d5545f8b1d3cd7cd63448f8064fd3712d6fee) )

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "guzz-11.bin",  0x0000, 0x1000, CRC(ec2e9d86) SHA1(2fc631229e78db68777e74a03f98f660f324a885) )   /* big sprite */
	ROM_LOAD( "guzz-10.bin",  0x1000, 0x1000, CRC(bd3f0bf7) SHA1(c57aff05812801c22104a4afc8a8a6bca33dda96) )
	ROM_LOAD( "guzz-09.bin",  0x2000, 0x1000, CRC(18927579) SHA1(414676193ef1f6ce79a4cba73e4d017312f766f4) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "guzzler.003",  0x0000, 0x100, CRC(f86930c1) SHA1(58efc8cbef05e1612d12e2f0babddf15571d42bb) )
	ROM_LOAD( "guzzler.002",  0x0100, 0x100, CRC(b566ea9e) SHA1(345078af6a339fbe6cd966046acd9d04c8926b5c) )
	ROM_LOAD( "guzzler.001",  0x0200, 0x020, CRC(69089495) SHA1(96b067b22be14536bac748f8d61e5587a8a04e92) )
ROM_END

ROM_START( guzzlers ) /* Swimmer Conversion, 1k vs 2k romsize in maincpu */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "guzz1.l9",  0x0000, 0x1000, CRC(48f751ee) SHA1(a8ff19d150d382a43ad705fe2a470450e317aac3) )
	ROM_LOAD( "guzz2.k9",  0x1000, 0x1000, CRC(c13f23e6) SHA1(2cd31e0419875c50433f1763e35e32afcaf68fde) )
	ROM_LOAD( "guzz3.j9",  0x2000, 0x1000, CRC(7a523fd8) SHA1(683249d2ffdde21f74d80280e538645ac143d45c) )
	ROM_LOAD( "guzz4.f9",  0x3000, 0x1000, CRC(d2bb2204) SHA1(87f821f1cb92577e10beb67be29d9eecd9e8a04f) )
	ROM_LOAD( "guzz5.e9",  0x4000, 0x1000, CRC(09856fd0) SHA1(f2eeffe2c35f652a855502f808fd5056252ce7fd) )
	ROM_LOAD( "guzz6.d9",  0x5000, 0x1000, CRC(80990d1e) SHA1(282f5247b88f29ee6178c771ecddf2a5ed995913) )
	ROM_LOAD( "guzz7.c9",  0x6000, 0x1000, CRC(fe37b99d) SHA1(9219fe4506e81e574f5ae84ec10dc1df511f76a1) )
	ROM_LOAD( "guzz8.a9",  0x7000, 0x1000, CRC(8d44f5f8) SHA1(957f1b880f6f815ac31c1a37c40cdff75dd119cf) )
	ROM_LOAD( "guzz-16.bin",  0xe000, 0x2000, CRC(61ee00b7) SHA1(ea8516c8dfb2de32a8034f94c7d0c086e3596740) ) // 16.

	ROM_REGION( 0x1000, "audiocpu", 0 )
	ROM_LOAD( "guzz-12.bin",  0x0000, 0x1000, CRC(f3754d9e) SHA1(bb30832aba4e82ab0ecce40fc1223d9771ff7dd2) ) // GUZZ12.L4

	ROM_REGION( 0x3000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "guzz-13.bin",  0x0000, 0x1000, CRC(afc464e2) SHA1(61730b5e5add24ba3d4e8903c5d71cf4df9b77e0) ) // GUZZ13.JK18
	ROM_LOAD( "guzz-14.bin",  0x1000, 0x1000, CRC(acbdfe1f) SHA1(ab7abe4bb321fc7dc4e73acab4b1a7133e6bcf20) ) // GUZZ14.L18
	ROM_LOAD( "guzz-15.bin",  0x2000, 0x1000, CRC(66978c05) SHA1(2c8d5545f8b1d3cd7cd63448f8064fd3712d6fee) ) // GUZZ15.MN18

	ROM_REGION( 0x3000, "gfx2", 0 ) /* big sprite */
	ROM_LOAD( "guzz-11.bin",  0x0000, 0x1000, CRC(ec2e9d86) SHA1(2fc631229e78db68777e74a03f98f660f324a885) ) // 11.C6
	ROM_LOAD( "guzz-10.bin",  0x1000, 0x1000, CRC(bd3f0bf7) SHA1(c57aff05812801c22104a4afc8a8a6bca33dda96) ) // 10.C5
	ROM_LOAD( "guzz-09.bin",  0x2000, 0x1000, CRC(18927579) SHA1(414676193ef1f6ce79a4cba73e4d017312f766f4) ) // 9.C4

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "guzzler.003",  0x0000, 0x100, CRC(f86930c1) SHA1(58efc8cbef05e1612d12e2f0babddf15571d42bb) ) // B.B13
	ROM_LOAD( "guzzler.002",  0x0100, 0x100, CRC(b566ea9e) SHA1(345078af6a339fbe6cd966046acd9d04c8926b5c) ) // A.A13
	ROM_LOAD( "c.c12",        0x0200, 0x020, CRC(51cd9980) SHA1(9c4858a01c9b03ff8c87ba9f11049e0c1af5d519) )
ROM_END

ROM_START( yamato )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "2.5de",        0x0000, 0x2000, CRC(20895096) SHA1(af76786e3c519e710899f143d46c53087e9817c7) )
	ROM_LOAD( "3.5f",         0x2000, 0x2000, CRC(57a696f9) SHA1(28ea80fb100ac92295fc3eb318617d7cb014408d) )
	ROM_LOAD( "4.5jh",        0x4000, 0x2000, CRC(59a468e8) SHA1(a79cdee6efefd87a356cc8d710f8050bc12e07c3) )
	/* hole at 6000-6fff */
	ROM_LOAD( "11.5a",        0x7000, 0x1000, CRC(35987485) SHA1(1f0cb545bbd52982cbf801bc1dd2c4087af2f5f7) )

	ROM_REGION( 0x800, "audiocpu", 0 )
	ROM_LOAD( "1.5v",         0x0000, 0x0800, CRC(3aad9e3c) SHA1(37b0414b265397881bb45b166ecab85880d1358d) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "10.11k",       0x0000, 0x2000, CRC(161121f5) SHA1(017c5c6b773b0ae1d0be52e4bac90b699ea196dd) )
	ROM_LOAD( "9.11h",        0x2000, 0x2000, CRC(56e84cc4) SHA1(c48e0e5460376d6b34173c42a27907ef12218182) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "8.11c",        0x0000, 0x1000, CRC(28024d9a) SHA1(c871c4d74be72a8bfea99e89d43f91922f4b734b) )
	ROM_LOAD( "7.11a",        0x1000, 0x1000, CRC(4a179790) SHA1(7fb6b033de939ff8bd13055c073311dca2c1a6fe) )

	ROM_REGION( 0x2000, "user1", 0 )
	ROM_LOAD( "5.5lm",        0x0000, 0x1000, CRC(7761ad24) SHA1(98878b19addd142d35718080eece05eaaee0388d) )    /* ?? */
	ROM_LOAD( "6.5n",         0x1000, 0x1000, CRC(da48444c) SHA1(a43e672ce262eb817fb4e5715ef4fb304a6a2815) )

	ROM_REGION( 0x00a0, "proms", 0 )
	ROM_LOAD( "1.bpr",        0x0000, 0x0020, CRC(ef2053ab) SHA1(2006cbf003f90a8e75f39047a88a3bba85d78e80) )
	ROM_LOAD( "2.bpr",        0x0020, 0x0020, CRC(2281d39f) SHA1(e9b568bdacf7ab611801cf42ea5c7624f5440ef6) )
	ROM_LOAD( "3.bpr",        0x0040, 0x0020, CRC(9e6341e3) SHA1(2e7a4d3c1f40d6089735734b9d9de2ca57fb73c7) )
	ROM_LOAD( "4.bpr",        0x0060, 0x0020, CRC(1c97dc0b) SHA1(fe8e0a91172abdd2d14b199da144306a9b944372) )
	ROM_LOAD( "5.bpr",        0x0080, 0x0020, CRC(edd6c05f) SHA1(b95db8aaf74fe175d1179f0d85f79242b16f5fb4) )
ROM_END

ROM_START( yamato2 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "2-2.5de",      0x0000, 0x2000, CRC(93da1d52) SHA1(21b72856ebbd969e4e075b52719e6acdbd1bc4c5) )
	ROM_LOAD( "3-2.5f",       0x2000, 0x2000, CRC(31e73821) SHA1(e582c9fcea1b29d43f65b6aa67e1895c38d2736c) )
	ROM_LOAD( "4-2.5jh",      0x4000, 0x2000, CRC(fd7bcfc3) SHA1(5037170cb3a9824794e90d74def92b0b25d45caa) )
	/* hole at 6000-6fff */
	/* 7000-7fff not present here */

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "1.5v",         0x0000, 0x0800, CRC(3aad9e3c) SHA1(37b0414b265397881bb45b166ecab85880d1358d) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "10.11k",       0x0000, 0x2000, CRC(161121f5) SHA1(017c5c6b773b0ae1d0be52e4bac90b699ea196dd) )
	ROM_LOAD( "9.11h",        0x2000, 0x2000, CRC(56e84cc4) SHA1(c48e0e5460376d6b34173c42a27907ef12218182) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "8.11c",        0x0000, 0x1000, CRC(28024d9a) SHA1(c871c4d74be72a8bfea99e89d43f91922f4b734b) )
	ROM_LOAD( "7.11a",        0x1000, 0x1000, CRC(4a179790) SHA1(7fb6b033de939ff8bd13055c073311dca2c1a6fe) )

	ROM_REGION( 0x2000, "user1", 0 )
	ROM_LOAD( "5.5lm",        0x0000, 0x1000, CRC(7761ad24) SHA1(98878b19addd142d35718080eece05eaaee0388d) )    /* ?? */
	ROM_LOAD( "6.5n",         0x1000, 0x1000, CRC(da48444c) SHA1(a43e672ce262eb817fb4e5715ef4fb304a6a2815) )

	ROM_REGION( 0x00a0, "proms", 0 )
	ROM_LOAD( "1.bpr",        0x0000, 0x0020, CRC(ef2053ab) SHA1(2006cbf003f90a8e75f39047a88a3bba85d78e80) )
	ROM_LOAD( "2.bpr",        0x0020, 0x0020, CRC(2281d39f) SHA1(e9b568bdacf7ab611801cf42ea5c7624f5440ef6) )
	ROM_LOAD( "3.bpr",        0x0040, 0x0020, CRC(9e6341e3) SHA1(2e7a4d3c1f40d6089735734b9d9de2ca57fb73c7) )
	ROM_LOAD( "4.bpr",        0x0060, 0x0020, CRC(1c97dc0b) SHA1(fe8e0a91172abdd2d14b199da144306a9b944372) )
	ROM_LOAD( "5.bpr",        0x0080, 0x0020, CRC(edd6c05f) SHA1(b95db8aaf74fe175d1179f0d85f79242b16f5fb4) )
ROM_END

ROM_START( toprollr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "10.k3", 0xc000, 0x2000, CRC(1e8914a6) SHA1(ec17f185f890d04ce75a5d8edf8b32da60e7a8d8) )
	ROM_LOAD( "11.l3", 0xe000, 0x2000, CRC(b20a9fa2) SHA1(accd3296447eca002b0808e7b02832f5e35407e8) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "16.j4", 0x0000, 0x2000, CRC(ce3afe26) SHA1(7de00720f091537c64cc0fec687c061de3a8b1a3) )
	ROM_LOAD( "15.h4", 0x2000, 0x2000, CRC(1d9e3325) SHA1(e7f6863aa2ba2aeec40cfcc5cf6c69e947c185b5) )

	ROM_REGION( 0x04000, "gfx2", 0 )
	ROM_LOAD( "14.c4", 0x0000, 0x2000, CRC(7a945733) SHA1(14187ba303aecf0a812c425c34d8edda3deaa2b5) )
	ROM_LOAD( "13.a4", 0x2000, 0x2000, CRC(89327329) SHA1(555331a3136aa8c5bb35b97dd54bc59da067be57) )

	ROM_REGION( 0x2000, "gfx3", 0 )
	ROM_LOAD( "6.m5",  0x0000, 0x1000, CRC(e30c1dd8) SHA1(1777bf98625153c9b191020860e4e1839b46b998) )
	ROM_LOAD( "5.l5",  0x1000, 0x1000, CRC(84139f46) SHA1(976f781fb279dd540778708174b942a263f16443) )

	ROM_REGION( 0x12000, "user1", 0 )
	ROM_LOAD( "2.f5",   0x00000, 0x02000, CRC(ef789f00) SHA1(424d69584d391ee7b9ad5db7ee6ced97d69897d4) )
	ROM_LOAD( "8.f3",   0x02000, 0x02000, CRC(94371cfb) SHA1(cb501c36b213c995a4048b3a96c85848c556cd05) )
	ROM_LOAD( "4.k5",   0x04000, 0x02000, CRC(1cb48ea0) SHA1(fdc75075112042ec84a7d1b3e5b5a6db1d1cb871) )
	ROM_COPY( "user1", 0x04000, 0x0a000, 0x02000 )
	ROM_COPY( "user1", 0x04000, 0x10000, 0x02000 )
	ROM_LOAD( "3.h5",   0x06000, 0x02000, CRC(d45494ba) SHA1(6e235b34f9457acadad6d4e27799978bc2e3db08) )
	ROM_LOAD( "9.h3",   0x08000, 0x02000, CRC(8a8032a7) SHA1(d6642d72645c613c21f65bbbe1560d0437d41f43) )
	ROM_LOAD( "1.d5",   0x0c000, 0x02000, CRC(9894374d) SHA1(173de4abbc3fb5d522aa6d6d5caf8e4d54f2a598) )
	ROM_LOAD( "7.d3",   0x0e000, 0x02000, CRC(904fffb6) SHA1(5528bc2a4d2fe8672428fd4725644265f0d57ded) )

	ROM_REGION( 0x2000, "samples", 0 )
	ROM_LOAD( "12.p3",  0x0000, 0x2000, CRC(7f989dc9) SHA1(3b4d18cbb992872b3cf8f5eaf5381ed3a9468cc1) )

	ROM_REGION( 0x01a0, "proms", 0 )
	ROM_LOAD( "prom.p2",  0x0000, 0x0020, CRC(42e828fa) SHA1(81250b1f7c3956b3902324adbbaf3b5989e854ee) ) //08-0f sprites + fg (wrong?)
	ROM_LOAD( "prom.r2",  0x0020, 0x0020, CRC(99b87eed) SHA1(06c3164d681fe4aff0338c0dad1a921f7fe7369d) ) //10-17 sprites
	ROM_LOAD( "prom.a1",  0x0040, 0x0020, CRC(7d626d6c) SHA1(7c7202d0ec5bf0381e7104eef53afa5fa4596a29) ) //00-07 big sprites
	ROM_LOAD( "prom.p9",  0x0060, 0x0020, CRC(eb399c02) SHA1(bf3d6c6dd982cb54446cf8a010b7adb949514bdb) ) //18-1f bg
	ROM_LOAD( "prom.n9",  0x0080, 0x0020, CRC(fb03ea99) SHA1(4dcef86106cef713dfcbd965072bfa8fe4b68e15) ) //20-27 bg
	ROM_LOAD( "prom.s9",  0x00a0, 0x0100, CRC(abf4c5fb) SHA1(a953f14642d4b72328293b36bc3c65b13491ffff) ) //unknown prom (filled with 2 bit vals)

ROM_END


DRIVER_INIT_MEMBER(cclimber_state,yamato)
{
	static const UINT8 convtable[32][4] =
	{
		/*       opcode                   data                     address      */
		/*  A    B    C    D         A    B    C    D                           */
		{ 0x88,0xa8,0x08,0x28 }, { 0x88,0xa8,0x80,0xa0 },   /* ...0...0...0...0 */
		{ 0x20,0xa0,0x28,0xa8 }, { 0x88,0xa8,0x80,0xa0 },   /* ...0...0...0...1 */
		{ 0x88,0xa8,0x80,0xa0 }, { 0x88,0xa8,0x80,0xa0 },   /* ...0...0...1...0 */
		{ 0x88,0xa8,0x80,0xa0 }, { 0x20,0xa0,0x28,0xa8 },   /* ...0...0...1...1 */
		{ 0x88,0xa8,0x08,0x28 }, { 0x88,0xa8,0x08,0x28 },   /* ...0...1...0...0 */
		{ 0x88,0xa8,0x80,0xa0 }, { 0x88,0xa8,0x80,0xa0 },   /* ...0...1...0...1 */
		{ 0x20,0xa0,0x28,0xa8 }, { 0x20,0xa0,0x28,0xa8 },   /* ...0...1...1...0 */
		{ 0x88,0xa8,0x80,0xa0 }, { 0x88,0xa8,0x80,0xa0 },   /* ...0...1...1...1 */
		{ 0x20,0xa0,0x28,0xa8 }, { 0x88,0xa8,0x08,0x28 },   /* ...1...0...0...0 */
		{ 0x20,0xa0,0x28,0xa8 }, { 0x28,0x20,0xa8,0xa0 },   /* ...1...0...0...1 */
		{ 0xa0,0x20,0x80,0x00 }, { 0x20,0xa0,0x28,0xa8 },   /* ...1...0...1...0 */
		{ 0x28,0x20,0xa8,0xa0 }, { 0x20,0xa0,0x28,0xa8 },   /* ...1...0...1...1 */
		{ 0x20,0xa0,0x28,0xa8 }, { 0x88,0xa8,0x08,0x28 },   /* ...1...1...0...0 */
		{ 0x88,0xa8,0x08,0x28 }, { 0x88,0xa8,0x08,0x28 },   /* ...1...1...0...1 */
		{ 0xa0,0x20,0x80,0x00 }, { 0x88,0x08,0x80,0x00 },   /* ...1...1...1...0 */
		{ 0x20,0xa0,0x28,0xa8 }, { 0x00,0x08,0x20,0x28 }    /* ...1...1...1...1 */
	};

	sega_decode(memregion("maincpu")->base(), m_decrypted_opcodes, 0x6000, convtable);

	save_item(NAME(m_yamato_p0));
	save_item(NAME(m_yamato_p1));
}


DRIVER_INIT_MEMBER(cclimber_state,toprollr)
{
	static const UINT8 convtable[32][4] =
	{
		/*       opcode                   data                     address      */
		/*  A    B    C    D         A    B    C    D                           */
		{ 0x88,0xa8,0x08,0x28 }, { 0x88,0xa8,0x80,0xa0 },   /* ...0...0...0...0 */
		{ 0x20,0xa0,0x28,0xa8 }, { 0x88,0xa8,0x80,0xa0 },   /* ...0...0...0...1 */
		{ 0x88,0xa8,0x80,0xa0 }, { 0x88,0xa8,0x80,0xa0 },   /* ...0...0...1...0 */
		{ 0x88,0xa8,0x80,0xa0 }, { 0x20,0xa0,0x28,0xa8 },   /* ...0...0...1...1 */
		{ 0x88,0xa8,0x08,0x28 }, { 0x88,0xa8,0x08,0x28 },   /* ...0...1...0...0 */
		{ 0x88,0xa8,0x80,0xa0 }, { 0x88,0xa8,0x80,0xa0 },   /* ...0...1...0...1 */
		{ 0x20,0xa0,0x28,0xa8 }, { 0x20,0xa0,0x28,0xa8 },   /* ...0...1...1...0 */
		{ 0x88,0xa8,0x80,0xa0 }, { 0x88,0xa8,0x80,0xa0 },   /* ...0...1...1...1 */
		{ 0x20,0xa0,0x28,0xa8 }, { 0x88,0xa8,0x08,0x28 },   /* ...1...0...0...0 */
		{ 0x20,0xa0,0x28,0xa8 }, { 0x28,0x20,0xa8,0xa0 },   /* ...1...0...0...1 */
		{ 0xa0,0x20,0x80,0x00 }, { 0x20,0xa0,0x28,0xa8 },   /* ...1...0...1...0 */
		{ 0x28,0x20,0xa8,0xa0 }, { 0x20,0xa0,0x28,0xa8 },   /* ...1...0...1...1 */
		{ 0x20,0xa0,0x28,0xa8 }, { 0x88,0xa8,0x08,0x28 },   /* ...1...1...0...0 */
		{ 0x88,0xa8,0x08,0x28 }, { 0x88,0xa8,0x08,0x28 },   /* ...1...1...0...1 */
		{ 0xa0,0x20,0x80,0x00 }, { 0x88,0x08,0x80,0x00 },   /* ...1...1...1...0 */
		{ 0x20,0xa0,0x28,0xa8 }, { 0x00,0x08,0x20,0x28 }    /* ...1...1...1...1 */
	};

	UINT8 *opcodes = auto_alloc_array(machine(), UINT8, 0x6000*3);
	sega_decode(memregion("user1")->base(), opcodes, 0, convtable, 3, 0x6000);

	membank("bank1")->configure_entries(0, 3, memregion("user1")->base(), 0x6000);
	membank("bank1d")->configure_entries(0, 3, opcodes, 0x6000);

	membank("bank1")->set_entry(0);
	membank("bank1d")->set_entry(0);

	save_item(NAME(m_toprollr_rombank));
}

DRIVER_INIT_MEMBER(cclimber_state,dking)
{
	UINT8 *rom = memregion( "maincpu" )->base();
	int i;
	int j;

	for (j=0;j<0x5000;j+=0x1000)
	{
		for (i=0x0500;i<0x0800;i++)  rom[i+j] ^=0xff;
		for (i=0x0d00;i<0x1000;i++)  rom[i+j] ^=0xff;
	}

}


GAME( 1980, cclimber,    0,        cclimberx, cclimber, cclimber_state, cclimber, ROT0,   "Nichibutsu", "Crazy Climber (US)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, cclimberj,   cclimber, cclimberx, cclimberj, cclimber_state,cclimberj,ROT0,   "Nichibutsu", "Crazy Climber (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, ccboot,      cclimber, cclimberx, cclimber, cclimber_state, cclimberj,ROT0,   "bootleg", "Crazy Climber (bootleg set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, ccboot2,     cclimber, cclimberx, cclimber, cclimber_state, cclimberj,ROT0,   "bootleg", "Crazy Climber (bootleg set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, ccbootmr,    cclimber, cclimberx, cclimber, cclimber_state, cclimberj,ROT0,   "bootleg (Model Racing)", "Crazy Climber (Model Racing bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, cclimbroper, cclimber, cclimber,  cclimber, driver_device,  0,        ROT0,   "bootleg (Operamatic)", "Crazy Climber (Spanish, Operamatic bootleg)", MACHINE_SUPPORTS_SAVE )

/* these sets have ugly colours, no extra attract screen, and no graphics for the extra attract screen in the BG roms
  - there is a Falcon logo in the text roms which is unused
  - does the code to display the extra screen still exist in the roms?  */
GAME( 1981, ckong,       0,        cclimber, ckong, driver_device,    0,        ROT270, "Kyoei / Falcon", "Crazy Kong", MACHINE_SUPPORTS_SAVE ) // on a Falcon FCK-01 PCB, but doesn't display any Falcon copyright
GAME( 1981, ckongalc,    ckong,    cclimber, ckong, driver_device,    0,        ROT270, "bootleg (Alca)", "Crazy Kong (Alca bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, monkeyd,     ckong,    cclimber, ckong, driver_device,    0,        ROT270, "bootleg", "Monkey Donkey", MACHINE_SUPPORTS_SAVE )
GAME( 1981, dking,       ckong,    cclimber, ckong, cclimber_state,   dking,    ROT270, "bootleg", "Donkey King", MACHINE_SUPPORTS_SAVE ) // supposedly, possibly by Hafasonic?
GAME( 1981, ckongdks,    ckong,    cclimber, ckong, cclimber_state,   dking,    ROT270, "bootleg", "Donkey Kong (Spanish Crazy Kong bootleg)", MACHINE_SUPPORTS_SAVE )

/* these sets have correct colours, and also contain the graphics used for the extra attract screen in the BG roms, but it is unused
 - the Falcon logo in the text roms is still unused
 - does the code to display the extra screen still exist in the roms?  */
GAME( 1981, ckongo,      ckong,    cclimber, ckong, driver_device,    0,        ROT270, "bootleg (Orca)", "Crazy Kong (Orca bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, bigkong,     ckong,    cclimber, ckong, driver_device,    0,        ROT270, "bootleg", "Big Kong", MACHINE_SUPPORTS_SAVE )

/* these sets have correct colours, and the extra attract screen, they also make use of the Falcon logo, some sets hack out the Falcon
   text on the extra screen */
GAME( 1981, ckongpt2,    0,        cclimber, ckong, driver_device,    0,        ROT270, "Falcon", "Crazy Kong Part II (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, ckongpt2a,   ckongpt2, cclimber, ckong, driver_device,    0,        ROT270, "Falcon", "Crazy Kong Part II (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, ckongpt2j,   ckongpt2, cclimber, ckong, driver_device,    0,        ROT270, "Falcon", "Crazy Kong Part II (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, ckongpt2jeu, ckongpt2, cclimber, ckong, driver_device,    0,        ROT270, "bootleg (Jeutel)", "Crazy Kong Part II (Jeutel bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, ckongpt2b,   ckongpt2, cclimber, ckongb, cclimber_state,   ckongb,   ROT270, "bootleg", "Crazy Kong Part II (alternative levels)", MACHINE_SUPPORTS_SAVE )

// see bagman.c for parent
GAME( 1981, bagmanf,     bagman,   cclimber, ckong, driver_device,    0,        ROT270, "bootleg", "Bagman (bootleg on Crazy Kong hardware)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )

GAME( 1981, rpatrol,     0,        cclimber, rpatrol, driver_device,  0,        ROT0,   "Orca", "River Patrol (Orca)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, rpatrolb,    rpatrol,  cclimber, rpatrol, driver_device,  0,        ROT0,   "bootleg", "River Patrol (bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, silvland,    rpatrol,  cclimber, rpatrol, driver_device,  0,        ROT0,   "Falcon", "Silver Land", MACHINE_SUPPORTS_SAVE )

// see pacman.c for parent
GAME( 1985, cannonb,     cannonbp, cannonb,  cannonb, cclimber_state,  cannonb,  ROT90,  "bootleg (Soft)", "Cannon Ball (bootleg on Crazy Kong hardware) (set 1, buggy)" , MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE ) // bootleggers missed protection after bonus game
GAME( 1985, cannonb2,    cannonbp, cannonb,  cannonb, cclimber_state,  cannonb2, ROT90,  "bootleg (TV Game Gruenberg)", "Cannon Ball (bootleg on Crazy Kong hardware) (set 2, buggy)", MACHINE_SUPPORTS_SAVE ) // bootleggers missed protection after bonus game
GAME( 1985, cannonb3,    cannonbp, cannonb,  cannonb, cclimber_state,  cannonb2, ROT90,  "bootleg (Soft)", "Cannon Ball (bootleg on Crazy Kong hardware) (set 3, no bonus game)", MACHINE_SUPPORTS_SAVE ) // the bonus game is patched out, thus avoiding the protection issue

GAME( 1982, swimmer,     0,        swimmer,  swimmer, driver_device,  0,        ROT0,   "Tehkan", "Swimmer (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, swimmera,    swimmer,  swimmer,  swimmer, driver_device,  0,        ROT0,   "Tehkan", "Swimmer (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, swimmerb,    swimmer,  swimmer,  swimmerb, driver_device, 0,        ROT0,   "Tehkan", "Swimmer (set 3)", MACHINE_SUPPORTS_SAVE )

GAME( 1983, guzzler,     0,        guzzler,  guzzler, driver_device,  0,        ROT90,  "Tehkan", "Guzzler", MACHINE_SUPPORTS_SAVE )
GAME( 1983, guzzlers,    guzzler,  guzzler,  guzzler, driver_device,  0,        ROT90,  "Tehkan", "Guzzler (Swimmer Conversion)", MACHINE_SUPPORTS_SAVE )

GAME( 1983, yamato,      0,        yamato,   yamato, cclimber_state,   yamato,   ROT90,  "Sega", "Yamato (US)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1983, yamato2,     yamato,   yamato,   yamato, cclimber_state,   yamato,   ROT90,  "Sega", "Yamato (World?)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )

GAME( 1983, toprollr,    0,        toprollr, toprollr, cclimber_state, toprollr, ROT90,  "Jaleco", "Top Roller", MACHINE_IMPERFECT_COLORS | MACHINE_SUPPORTS_SAVE )
