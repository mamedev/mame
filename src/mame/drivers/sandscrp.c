// license:BSD-3-Clause
// copyright-holders:Luca Elia
/*

    Sand Scorpion    (C) 1992 FACE

driver by   Luca Elia (l.elia@tin.it)

PCB Number: Z03VA-001

      CPU: TMP68HC000N-12
Sound CPU: Z8400AB1 (Z80A)
    Sound: OKI6295, YM2203C & Y3014B
      OSC: 16.000mhz & 12.000mhz
     Dips: Two 8-way dipswitch banks

Kaneko custom (surfaced scratched):
   PX79C480FP-3 PANDORA-CHIP (160pin PQFP)  <- Sprites
   VIEW2-CHIP (144pin PQFP)                 <- Tilemaps
   HELP1-CHIP (64pin PQFP)
   CALC1-CHIP (40pin DIP)                   <- Collision Detection etc.


+-------------------------------------+
|  VOL   M6295 7.IC55   Z80A   HELP1  |
|        Y3014B YM2203C 8.IC51        |
|            DSW1 DSW2 LH5168D  4.IC32|
|        12MHz 16MHz   LH5168D  3.IC33|
|J                     LH5168D   IC30*|
|A                       VIEW2   IC31*|
|M       LH5116D             M51257AL |
|M       LH5116D        PAL  M51257AL |
|A                               2.IC5|
|         PAL                    1.IC4|
|         IC15*       PANDORA    68000|
|  IC14* 5.IC16   M41464 M41464  CALC1|
|  IC18* 6.IC17   M41464 M41464       |
+-------------------------------------+

IC18 unpopulated 28 pin socket (silkscreened EPROM)
IC14 & IC15 unpopulated 32 pin socket (silkscreened MASK)
IC30 & IC31 unpopulated 42 pin socket (silkscreened MASK)

RAM:
 OKI M41464C-10 (x4)
 OKI M51257AL-10 (x2)
 Sharp LH5116D-10 (x2)
 Sharp LH5168D-10L (x3)

PALS:
 AMI 18CV8PC-25 (Stamped IC25D located at IC25)
 AMI 18CV8PC-25 (Stamped IC26 located at IC26)

Chips simply labeled 1 through 8

ROM ID    ROM Type
===================
1.ic4     27C2001
2.ic5     27C2001
3.ic33    27C040
4.ic32    27C040
5.ic16    27C040
6.ic17    27C040
7.ic55    27C2001
8.ic51    27C1001

Alternate Program roms:
11.ic4    27C2001
12.ic5    27C2001

Is there another alt program rom set labeled 9 & 10?

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "sound/2203intf.h"
#include "sound/2151intf.h"
#include "sound/okim6295.h"
#include "video/kan_pand.h"
#include "machine/kaneko_hit.h"
#include "video/kaneko_tmap.h"


class sandscrp_state : public driver_device
{
public:
	sandscrp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_pandora(*this, "pandora"),
		m_view2_0(*this, "view2_0")
		{ }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<kaneko_pandora_device> m_pandora;
	optional_device<kaneko_view2_tilemap_device> m_view2_0;

	UINT8 m_sprite_irq;
	UINT8 m_unknown_irq;
	UINT8 m_vblank_irq;
	UINT8 m_latch1_full;
	UINT8 m_latch2_full;

	DECLARE_READ16_MEMBER(irq_cause_r);
	DECLARE_WRITE16_MEMBER(irq_cause_w);
	DECLARE_WRITE16_MEMBER(coincounter_w);
	DECLARE_READ16_MEMBER(latchstatus_word_r);
	DECLARE_WRITE16_MEMBER(latchstatus_word_w);
	DECLARE_READ16_MEMBER(soundlatch_word_r);
	DECLARE_WRITE16_MEMBER(soundlatch_word_w);
	DECLARE_WRITE8_MEMBER(bankswitch_w);
	DECLARE_READ8_MEMBER(latchstatus_r);
	DECLARE_READ8_MEMBER(soundlatch_r);
	DECLARE_WRITE8_MEMBER(soundlatch_w);

	virtual void machine_start();

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof(screen_device &screen, bool state);

	INTERRUPT_GEN_MEMBER(interrupt);
	void update_irq_state();
};



UINT32 sandscrp_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	screen.priority().fill(0, cliprect);

	m_view2_0->kaneko16_prepare(bitmap, cliprect);

	for ( int i = 0; i < 8; i++ )
	{
		m_view2_0->render_tilemap_chip(screen,bitmap,cliprect,i);
	}

	// copy sprite bitmap to screen
	m_pandora->update(bitmap, cliprect);
	return 0;
}




void sandscrp_state::machine_start()
{
	membank("bank1")->configure_entries(0, 8, memregion("audiocpu")->base(), 0x4000);

	save_item(NAME(m_sprite_irq));
	save_item(NAME(m_unknown_irq));
	save_item(NAME(m_vblank_irq));
	save_item(NAME(m_latch1_full));
	save_item(NAME(m_latch2_full));
}


/* Update the IRQ state based on all possible causes */
void sandscrp_state::update_irq_state()
{
	if (m_vblank_irq || m_sprite_irq || m_unknown_irq)
		m_maincpu->set_input_line(1, ASSERT_LINE);
	else
		m_maincpu->set_input_line(1, CLEAR_LINE);
}



/* Called once/frame to generate the VBLANK interrupt */
INTERRUPT_GEN_MEMBER(sandscrp_state::interrupt)
{
	m_vblank_irq = 1;
	update_irq_state();
}


void sandscrp_state::screen_eof(screen_device &screen, bool state)
{
	// rising edge
	if (state)
	{
		m_sprite_irq = 1;
		update_irq_state();
		m_pandora->eof();
	}
}

/* Reads the cause of the interrupt */
READ16_MEMBER(sandscrp_state::irq_cause_r)
{
	return  ( m_sprite_irq  ?  0x08  : 0 ) |
			( m_unknown_irq ?  0x10  : 0 ) |
			( m_vblank_irq  ?  0x20  : 0 ) ;
}


/* Clear the cause of the interrupt */
WRITE16_MEMBER(sandscrp_state::irq_cause_w)
{
	if (ACCESSING_BITS_0_7)
	{
//      m_sprite_flipx  =   data & 1;
//      m_sprite_flipy  =   data & 1;

		if (data & 0x08)    m_sprite_irq  = 0;
		if (data & 0x10)    m_unknown_irq = 0;
		if (data & 0x20)    m_vblank_irq  = 0;
	}

	update_irq_state();
}



/***************************************************************************
                                Sand Scorpion
***************************************************************************/

WRITE16_MEMBER(sandscrp_state::coincounter_w)
{
	if (ACCESSING_BITS_0_7)
	{
		coin_counter_w(machine(), 0,   data  & 0x0001);
		coin_counter_w(machine(), 1,   data  & 0x0002);
	}
}


READ16_MEMBER(sandscrp_state::latchstatus_word_r)
{
	return  (m_latch1_full ? 0x80 : 0) |
			(m_latch2_full ? 0x40 : 0) ;
}

WRITE16_MEMBER(sandscrp_state::latchstatus_word_w)
{
	if (ACCESSING_BITS_0_7)
	{
		m_latch1_full = data & 0x80;
		m_latch2_full = data & 0x40;
	}
}

READ16_MEMBER(sandscrp_state::soundlatch_word_r)
{
	m_latch2_full = 0;
	return soundlatch2_byte_r(space,0);
}

WRITE16_MEMBER(sandscrp_state::soundlatch_word_w)
{
	if (ACCESSING_BITS_0_7)
	{
		m_latch1_full = 1;
		soundlatch_byte_w(space, 0, data & 0xff);
		m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
		space.device().execute().spin_until_time(attotime::from_usec(100)); // Allow the other cpu to reply
	}
}

static ADDRESS_MAP_START( sandscrp, AS_PROGRAM, 16, sandscrp_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM     // ROM
	AM_RANGE(0x100000, 0x100001) AM_WRITE(irq_cause_w) // IRQ Ack

	AM_RANGE(0x700000, 0x70ffff) AM_RAM     // RAM
	AM_RANGE(0x200000, 0x20001f) AM_DEVREADWRITE("calc1_mcu", kaneko_hit_device, kaneko_hit_r,kaneko_hit_w)
	AM_RANGE(0x300000, 0x30001f) AM_DEVREADWRITE("view2_0", kaneko_view2_tilemap_device,  kaneko_tmap_regs_r, kaneko_tmap_regs_w)
	AM_RANGE(0x400000, 0x403fff) AM_DEVREADWRITE("view2_0", kaneko_view2_tilemap_device,  kaneko_tmap_vram_r, kaneko_tmap_vram_w )
	AM_RANGE(0x500000, 0x501fff) AM_DEVREADWRITE("pandora", kaneko_pandora_device, spriteram_LSB_r, spriteram_LSB_w ) // sprites
	AM_RANGE(0x600000, 0x600fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")    // Palette
	AM_RANGE(0xa00000, 0xa00001) AM_WRITE(coincounter_w)  // Coin Counters (Lockout unused)
	AM_RANGE(0xb00000, 0xb00001) AM_READ_PORT("P1")
	AM_RANGE(0xb00002, 0xb00003) AM_READ_PORT("P2")
	AM_RANGE(0xb00004, 0xb00005) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xb00006, 0xb00007) AM_READ_PORT("UNK")
	AM_RANGE(0xec0000, 0xec0001) AM_READ(watchdog_reset16_r)    //
	AM_RANGE(0x800000, 0x800001) AM_READ(irq_cause_r)  // IRQ Cause
	AM_RANGE(0xe00000, 0xe00001) AM_READWRITE(soundlatch_word_r, soundlatch_word_w)   // From/To Sound CPU
	AM_RANGE(0xe40000, 0xe40001) AM_READWRITE(latchstatus_word_r, latchstatus_word_w) //
ADDRESS_MAP_END



/***************************************************************************
                                Sand Scorpion
***************************************************************************/

WRITE8_MEMBER(sandscrp_state::bankswitch_w)
{
	membank("bank1")->set_entry(data & 7);
}

READ8_MEMBER(sandscrp_state::latchstatus_r)
{
	return  (m_latch2_full ? 0x80 : 0) |    // swapped!?
			(m_latch1_full ? 0x40 : 0) ;
}

READ8_MEMBER(sandscrp_state::soundlatch_r)
{
	m_latch1_full = 0;
	return soundlatch_byte_r(space,0);
}

WRITE8_MEMBER(sandscrp_state::soundlatch_w)
{
	m_latch2_full = 1;
	soundlatch2_byte_w(space,0,data);
}

static ADDRESS_MAP_START( sandscrp_soundmem, AS_PROGRAM, 8, sandscrp_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM     // ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")    // Banked ROM
	AM_RANGE(0xc000, 0xdfff) AM_RAM     // RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sandscrp_soundport, AS_IO, 8, sandscrp_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(bankswitch_w)    // ROM Bank
	AM_RANGE(0x02, 0x03) AM_DEVREADWRITE("ymsnd", ym2203_device, read, write)        // PORTA/B read
	AM_RANGE(0x04, 0x04) AM_DEVWRITE("oki", okim6295_device, write)     // OKIM6295
	AM_RANGE(0x06, 0x06) AM_WRITE(soundlatch_w)    //
	AM_RANGE(0x07, 0x07) AM_READ(soundlatch_r)     //
	AM_RANGE(0x08, 0x08) AM_READ(latchstatus_r)    //
ADDRESS_MAP_END


/***************************************************************************
                                Sand Scorpion
***************************************************************************/

static INPUT_PORTS_START( sandscrp )
	PORT_START("P1")    /* $b00000.w */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN   ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")    /* $b00002.w */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN   ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")    /* $b00004.w */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1   )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START2   )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN1    )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN2    )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_TILT     )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("UNK")   /* $b00006.w */
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")  /* read by the Z80 through the sound chip */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x02, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x0c, 0x0c, "Bombs" )         PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x30, 0x20, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x30, DEF_STR( Easy )    )
	PORT_DIPSETTING(    0x20, DEF_STR( Normal )  )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard )    )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x80, "100K, 300K" )
	PORT_DIPSETTING(    0xc0, "200K, 500K" )
	PORT_DIPSETTING(    0x40, "500K, 1000K" )
	PORT_DIPSETTING(    0x00, "1000K, 3000K" )

	PORT_START("DSW2")  /* read by the Z80 through the sound chip */
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW2:1,2,3,4")
	PORT_DIPSETTING(    0x0a, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 8C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 5C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x80, IP_ACTIVE_LOW, "SW2:8" )
INPUT_PORTS_END


/* 16x16x4 tiles (made of four 8x8 tiles) */
static const gfx_layout layout_16x16x4_2 =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	{ STEP4(8*8*4*0 + 3*4, -4), STEP4(8*8*4*0 + 7*4, -4),
		STEP4(8*8*4*1 + 3*4, -4), STEP4(8*8*4*1 + 7*4, -4) },
	{ STEP8(8*8*4*0, 8*4),     STEP8(8*8*4*2, 8*4) },
	16*16*4
};

static const gfx_layout layout_16x16x4 =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	{ STEP8(8*8*4*0,4),   STEP8(8*8*4*1,4)   },
	{ STEP8(8*8*4*0,8*4), STEP8(8*8*4*2,8*4) },
	16*16*4
};


static GFXDECODE_START( sandscrp )
	GFXDECODE_ENTRY( "gfx1", 0, layout_16x16x4,   0x000, 0x10 ) // [0] Sprites
	GFXDECODE_ENTRY( "gfx2", 0, layout_16x16x4_2, 0x400, 0x40 ) // [1] Layers
GFXDECODE_END



/***************************************************************************
                                Sand Scorpion
***************************************************************************/


static MACHINE_CONFIG_START( sandscrp, sandscrp_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000,12000000)    /* TMP68HC000N-12 */
	MCFG_CPU_PROGRAM_MAP(sandscrp)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", sandscrp_state,  interrupt)

	MCFG_CPU_ADD("audiocpu", Z80,4000000)   /* Z8400AB1, Reads the DSWs: it can't be disabled */
	MCFG_CPU_PROGRAM_MAP(sandscrp_soundmem)
	MCFG_CPU_IO_MAP(sandscrp_soundport)

	MCFG_WATCHDOG_TIME_INIT(attotime::from_seconds(3))  /* a guess, and certainly wrong */

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME( ATTOSECONDS_IN_USEC(2500) /* not accurate */ )
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 256-1, 0+16, 256-16-1)
	MCFG_SCREEN_UPDATE_DRIVER(sandscrp_state, screen_update)
	MCFG_SCREEN_VBLANK_DRIVER(sandscrp_state, screen_eof)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", sandscrp)
	MCFG_PALETTE_ADD("palette", 2048)
	MCFG_PALETTE_FORMAT(xGGGGGRRRRRBBBBB)

	MCFG_DEVICE_ADD("view2_0", KANEKO_TMAP, 0)
	kaneko_view2_tilemap_device::set_gfx_region(*device, 1);
	kaneko_view2_tilemap_device::set_offset(*device, 0x5b, 0, 256, 224);
	MCFG_KANEKO_TMAP_GFXDECODE("gfxdecode")

	MCFG_DEVICE_ADD("calc1_mcu", KANEKO_HIT, 0)
	kaneko_hit_device::set_type(*device, 0);

	MCFG_DEVICE_ADD("pandora", KANEKO_PANDORA, 0)
	MCFG_KANEKO_PANDORA_GFXDECODE("gfxdecode")
	MCFG_KANEKO_PANDORA_PALETTE("palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_OKIM6295_ADD("oki", 12000000/6, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.25)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.25)

	/* YM3014B + YM2203C */
	MCFG_SOUND_ADD("ymsnd", YM2203, 4000000)
	MCFG_YM2203_IRQ_HANDLER(INPUTLINE("audiocpu", 0))
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("DSW1"))
	MCFG_AY8910_PORT_B_READ_CB(IOPORT("DSW2"))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.25)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.25)
MACHINE_CONFIG_END


/***************************************************************************

                                Sand Scorpion

***************************************************************************/

ROM_START( sandscrp ) /* Z03VA-003 PCB */
	ROM_REGION( 0x080000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "11.bin", 0x000000, 0x040000, CRC(9b24ab40) SHA1(3187422dbe8b15d8053be4cb20e56d3e6afbd5f2) ) /* Location is IC4 */
	ROM_LOAD16_BYTE( "12.bin", 0x000001, 0x040000, CRC(ad12caee) SHA1(83267445b89c3cf4dc317106aa68763d2f29eff7) ) /* Location is IC5 */

	ROM_REGION( 0x20000, "audiocpu", 0 )        /* Z80 Code */
	ROM_LOAD( "8.ic51", 0x00000, 0x20000, CRC(6f3e9db1) SHA1(06a04fa17f44319986913bff70433510c89e38f1) )

	ROM_REGION( 0x100000, "gfx1", 0 )   /* Sprites */
	ROM_LOAD( "5.ic16", 0x000000, 0x080000, CRC(9bb675f6) SHA1(c3f6768cfd99a0e19ca2224fff9aa4e27ec0da24) )
	ROM_LOAD( "6.ic17", 0x080000, 0x080000, CRC(7df2f219) SHA1(e2a59e201bfededa92d6c86f8dc1b212527ef66f) )

	ROM_REGION( 0x100000, "gfx2", 0 )   /* Layers */
	ROM_LOAD16_BYTE( "4.ic32", 0x000000, 0x080000, CRC(b9222ff2) SHA1(a445da3f7f5dea5ff64bb0b048f624f947875a39) )
	ROM_LOAD16_BYTE( "3.ic33", 0x000001, 0x080000, CRC(adf20fa0) SHA1(67a7a2be774c86916cbb97e4c9b16c2e48125780) )

	ROM_REGION( 0x040000, "oki", 0 )    /* Samples */
	ROM_LOAD( "7.ic55", 0x000000, 0x040000, CRC(9870ab12) SHA1(5ea3412cbc57bfaa32a1e2552b2eb46f4ceb5fa8) )
ROM_END

ROM_START( sandscrpa ) /* Z03VA-003 PCB, earlier program version */
	ROM_REGION( 0x080000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "1.ic4", 0x000000, 0x040000, CRC(c0943ae2) SHA1(04dac4e1f116cd96d6292daa61ef40efc7eba919) )
	ROM_LOAD16_BYTE( "2.ic5", 0x000001, 0x040000, CRC(6a8e0012) SHA1(2350b11c9bd545c8ba4b3c25cd6547ba2ad474b5) )

	ROM_REGION( 0x20000, "audiocpu", 0 )        /* Z80 Code */
	ROM_LOAD( "8.ic51", 0x00000, 0x20000, CRC(6f3e9db1) SHA1(06a04fa17f44319986913bff70433510c89e38f1) )

	ROM_REGION( 0x100000, "gfx1", 0 )   /* Sprites */
	ROM_LOAD( "5.ic16", 0x000000, 0x080000, CRC(9bb675f6) SHA1(c3f6768cfd99a0e19ca2224fff9aa4e27ec0da24) )
	ROM_LOAD( "6.ic17", 0x080000, 0x080000, CRC(7df2f219) SHA1(e2a59e201bfededa92d6c86f8dc1b212527ef66f) )

	ROM_REGION( 0x100000, "gfx2", 0 )   /* Layers */
	ROM_LOAD16_BYTE( "4.ic32", 0x000000, 0x080000, CRC(b9222ff2) SHA1(a445da3f7f5dea5ff64bb0b048f624f947875a39) )
	ROM_LOAD16_BYTE( "3.ic33", 0x000001, 0x080000, CRC(adf20fa0) SHA1(67a7a2be774c86916cbb97e4c9b16c2e48125780) )

	ROM_REGION( 0x040000, "oki", 0 )    /* Samples */
	ROM_LOAD( "7.ic55", 0x000000, 0x040000, CRC(9870ab12) SHA1(5ea3412cbc57bfaa32a1e2552b2eb46f4ceb5fa8) )
ROM_END


ROM_START( sandscrpb ) /* Different rev PCB */
	ROM_REGION( 0x080000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "11.ic4", 0x000000, 0x040000, CRC(80020cab) SHA1(4f1f4d8ea07ad745f2d6d3f800686f07fe4bf20f) ) /* Chinese title screen */
	ROM_LOAD16_BYTE( "12.ic5", 0x000001, 0x040000, CRC(8df1d42f) SHA1(2a9db5c4b99a8a3f62bffa9ddd96a95e2042602b) ) /* Game & test menu in English */
	/* internet translators come up with "fighter lion king" and / or "Hits lion Emperor Quickly" */

	ROM_REGION( 0x20000, "audiocpu", 0 )        /* Z80 Code */
	ROM_LOAD( "8.ic51", 0x00000, 0x20000, CRC(6f3e9db1) SHA1(06a04fa17f44319986913bff70433510c89e38f1) )

	ROM_REGION( 0x100000, "gfx1", 0 )   /* Sprites */
	ROM_LOAD( "ss502.ic16", 0x000000, 0x100000, CRC(d8012ebb) SHA1(975bbb3b57a09e41d2257d4fa3a64097144de554) )

	ROM_REGION( 0x100000, "gfx2", 0 )   /* Layers */
	ROM_LOAD16_WORD_SWAP( "ss501.ic30", 0x000000, 0x100000, CRC(0cf9f99d) SHA1(47f7f120d2bc075bedaff0a44306a8f46a1d848c) )

	ROM_REGION( 0x040000, "oki", 0 )    /* Samples */
	ROM_LOAD( "7.ic55", 0x000000, 0x040000, CRC(9870ab12) SHA1(5ea3412cbc57bfaa32a1e2552b2eb46f4ceb5fa8) )
ROM_END


GAME( 1992, sandscrp,  0,        sandscrp, sandscrp, driver_device, 0,          ROT90, "Face",   "Sand Scorpion", MACHINE_SUPPORTS_SAVE )
GAME( 1992, sandscrpa, sandscrp, sandscrp, sandscrp, driver_device, 0,          ROT90, "Face",   "Sand Scorpion (Earlier)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, sandscrpb, sandscrp, sandscrp, sandscrp, driver_device, 0,          ROT90, "Face",   "Sand Scorpion (Chinese Title Screen, Revised Hardware)", MACHINE_SUPPORTS_SAVE )
