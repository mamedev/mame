/*

    Sand Scorpion

(C) 1992 FACE

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
#include "includes/kaneko16.h"
#include "sound/2203intf.h"
#include "sound/2151intf.h"
#include "sound/okim6295.h"
#include "video/kan_pand.h"


class sandscrp_state : public kaneko16_state
{
public:
	sandscrp_state(const machine_config &mconfig, device_type type, const char *tag)
		: kaneko16_state(mconfig, type, tag) { }

	UINT8 m_sprite_irq;
	UINT8 m_unknown_irq;
	UINT8 m_vblank_irq;
	UINT8 m_latch1_full;
	UINT8 m_latch2_full;
	DECLARE_READ16_MEMBER(sandscrp_irq_cause_r);
	DECLARE_WRITE16_MEMBER(sandscrp_irq_cause_w);
	DECLARE_WRITE16_MEMBER(sandscrp_coin_counter_w);
	DECLARE_READ16_MEMBER(sandscrp_latchstatus_word_r);
	DECLARE_WRITE16_MEMBER(sandscrp_latchstatus_word_w);
	DECLARE_READ16_MEMBER(sandscrp_soundlatch_word_r);
	DECLARE_WRITE16_MEMBER(sandscrp_soundlatch_word_w);
	DECLARE_WRITE8_MEMBER(sandscrp_bankswitch_w);
	DECLARE_READ8_MEMBER(sandscrp_latchstatus_r);
	DECLARE_READ8_MEMBER(sandscrp_soundlatch_r);
	DECLARE_WRITE8_MEMBER(sandscrp_soundlatch_w);
};


static MACHINE_RESET( sandscrp )
{
	sandscrp_state *state = machine.driver_data<sandscrp_state>();
	state->m_sprite_type  = 0;

	state->m_sprite_xoffs = 0;
	state->m_sprite_yoffs = 0;

	state->m_priority.sprite[0] = 1;	// above tile[0],   below the others
	state->m_priority.sprite[1] = 2;	// above tile[0-1], below the others
	state->m_priority.sprite[2] = 3;	// above tile[0-2], below the others
	state->m_priority.sprite[3] = 8;	// above all
	state->m_sprite_type = 3;	// "different" sprites layout
}

/* Sand Scorpion */



/* Update the IRQ state based on all possible causes */
static void update_irq_state(running_machine &machine)
{
	sandscrp_state *state = machine.driver_data<sandscrp_state>();
	if (state->m_vblank_irq || state->m_sprite_irq || state->m_unknown_irq)
		cputag_set_input_line(machine, "maincpu", 1, ASSERT_LINE);
	else
		cputag_set_input_line(machine, "maincpu", 1, CLEAR_LINE);
}



/* Called once/frame to generate the VBLANK interrupt */
static INTERRUPT_GEN( sandscrp_interrupt )
{
	sandscrp_state *state = device->machine().driver_data<sandscrp_state>();
	state->m_vblank_irq = 1;
	update_irq_state(device->machine());
}


static SCREEN_VBLANK( sandscrp )
{
	// rising edge
	if (vblank_on)
	{
		sandscrp_state *state = screen.machine().driver_data<sandscrp_state>();
		device_t *pandora = screen.machine().device("pandora");
		state->m_sprite_irq = 1;
		update_irq_state(screen.machine());
		pandora_eof(pandora);
	}
}

/* Reads the cause of the interrupt */
READ16_MEMBER(sandscrp_state::sandscrp_irq_cause_r)
{

	return	( m_sprite_irq  ?  0x08  : 0 ) |
			( m_unknown_irq ?  0x10  : 0 ) |
			( m_vblank_irq  ?  0x20  : 0 ) ;
}


/* Clear the cause of the interrupt */
WRITE16_MEMBER(sandscrp_state::sandscrp_irq_cause_w)
{

	if (ACCESSING_BITS_0_7)
	{
		m_sprite_flipx	=	data & 1;
		m_sprite_flipy	=	data & 1;

		if (data & 0x08)	m_sprite_irq  = 0;
		if (data & 0x10)	m_unknown_irq = 0;
		if (data & 0x20)	m_vblank_irq  = 0;
	}

	update_irq_state(machine());
}



/***************************************************************************
                                Sand Scorpion
***************************************************************************/

WRITE16_MEMBER(sandscrp_state::sandscrp_coin_counter_w)
{
	if (ACCESSING_BITS_0_7)
	{
		coin_counter_w(machine(), 0,   data  & 0x0001);
		coin_counter_w(machine(), 1,   data  & 0x0002);
	}
}


READ16_MEMBER(sandscrp_state::sandscrp_latchstatus_word_r)
{

	return	(m_latch1_full ? 0x80 : 0) |
			(m_latch2_full ? 0x40 : 0) ;
}

WRITE16_MEMBER(sandscrp_state::sandscrp_latchstatus_word_w)
{

	if (ACCESSING_BITS_0_7)
	{
		m_latch1_full = data & 0x80;
		m_latch2_full = data & 0x40;
	}
}

READ16_MEMBER(sandscrp_state::sandscrp_soundlatch_word_r)
{

	m_latch2_full = 0;
	return soundlatch2_r(*&space,0);
}

WRITE16_MEMBER(sandscrp_state::sandscrp_soundlatch_word_w)
{

	if (ACCESSING_BITS_0_7)
	{
		m_latch1_full = 1;
		soundlatch_w(*&space, 0, data & 0xff);
		cputag_set_input_line(machine(), "audiocpu", INPUT_LINE_NMI, PULSE_LINE);
		device_spin_until_time(&space.device(), attotime::from_usec(100));	// Allow the other cpu to reply
	}
}

static ADDRESS_MAP_START( sandscrp, AS_PROGRAM, 16, sandscrp_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM		// ROM
	AM_RANGE(0x100000, 0x100001) AM_WRITE(sandscrp_irq_cause_w)	// IRQ Ack

	AM_RANGE(0x700000, 0x70ffff) AM_RAM		// RAM
	AM_RANGE(0x200000, 0x20001f) AM_READWRITE(galpanib_calc_r,galpanib_calc_w)	// Protection
	AM_RANGE(0x300000, 0x30000f) AM_RAM_WRITE(kaneko16_layers_0_regs_w) AM_BASE(m_layers_0_regs)	// Layers 0 Regs
	AM_RANGE(0x400000, 0x400fff) AM_RAM_WRITE(kaneko16_vram_1_w) AM_BASE(m_vram[1])	// Layers 0
	AM_RANGE(0x401000, 0x401fff) AM_RAM_WRITE(kaneko16_vram_0_w) AM_BASE(m_vram[0])	//
	AM_RANGE(0x402000, 0x402fff) AM_RAM AM_BASE(m_vscroll[1])									//
	AM_RANGE(0x403000, 0x403fff) AM_RAM AM_BASE(m_vscroll[0])									//
	AM_RANGE(0x500000, 0x501fff) AM_DEVREADWRITE_LEGACY("pandora", pandora_spriteram_LSB_r, pandora_spriteram_LSB_w ) // sprites
	AM_RANGE(0x600000, 0x600fff) AM_RAM_WRITE(paletteram16_xGGGGGRRRRRBBBBB_word_w) AM_SHARE("paletteram")	// Palette
	AM_RANGE(0xa00000, 0xa00001) AM_WRITE(sandscrp_coin_counter_w)	// Coin Counters (Lockout unused)
	AM_RANGE(0xb00000, 0xb00001) AM_READ_PORT("P1")
	AM_RANGE(0xb00002, 0xb00003) AM_READ_PORT("P2")
	AM_RANGE(0xb00004, 0xb00005) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xb00006, 0xb00007) AM_READ_PORT("UNK")
	AM_RANGE(0xec0000, 0xec0001) AM_READ(watchdog_reset16_r)	//
	AM_RANGE(0x800000, 0x800001) AM_READ(sandscrp_irq_cause_r)	// IRQ Cause
	AM_RANGE(0xe00000, 0xe00001) AM_READWRITE(sandscrp_soundlatch_word_r, sandscrp_soundlatch_word_w)	// From/To Sound CPU
	AM_RANGE(0xe40000, 0xe40001) AM_READWRITE(sandscrp_latchstatus_word_r, sandscrp_latchstatus_word_w)	//
ADDRESS_MAP_END



/***************************************************************************
                                Sand Scorpion
***************************************************************************/

WRITE8_MEMBER(sandscrp_state::sandscrp_bankswitch_w)
{
	UINT8 *RAM = machine().region("maincpu")->base();
	int bank = data & 0x07;

	if ( bank != data )	logerror("CPU #1 - PC %04X: Bank %02X\n",cpu_get_pc(&space.device()),data);

	if (bank < 3)	RAM = &RAM[0x4000 * bank];
	else			RAM = &RAM[0x4000 * (bank-3) + 0x10000];

	memory_set_bankptr(machine(), "bank1", RAM);
}

READ8_MEMBER(sandscrp_state::sandscrp_latchstatus_r)
{

	return	(m_latch2_full ? 0x80 : 0) |	// swapped!?
			(m_latch1_full ? 0x40 : 0) ;
}

READ8_MEMBER(sandscrp_state::sandscrp_soundlatch_r)
{

	m_latch1_full = 0;
	return soundlatch_r(*&space,0);
}

WRITE8_MEMBER(sandscrp_state::sandscrp_soundlatch_w)
{

	m_latch2_full = 1;
	soundlatch2_w(*&space,0,data);
}

static ADDRESS_MAP_START( sandscrp_soundmem, AS_PROGRAM, 8, sandscrp_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM		// ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")	// Banked ROM
	AM_RANGE(0xc000, 0xdfff) AM_RAM		// RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sandscrp_soundport, AS_IO, 8, sandscrp_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(sandscrp_bankswitch_w)	// ROM Bank
	AM_RANGE(0x02, 0x03) AM_DEVREADWRITE_LEGACY("ymsnd", ym2203_r, ym2203_w)		// PORTA/B read
	AM_RANGE(0x04, 0x04) AM_DEVWRITE("oki", okim6295_device, write)		// OKIM6295
	AM_RANGE(0x06, 0x06) AM_WRITE(sandscrp_soundlatch_w)	//
	AM_RANGE(0x07, 0x07) AM_READ(sandscrp_soundlatch_r)		//
	AM_RANGE(0x08, 0x08) AM_READ(sandscrp_latchstatus_r)	//
ADDRESS_MAP_END


/***************************************************************************
                                Sand Scorpion
***************************************************************************/

static INPUT_PORTS_START( sandscrp )
	PORT_START("P1")	/* $b00000.w */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN	 ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")	/* $b00002.w */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN	 ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")	/* $b00004.w */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1   )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START2   )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN1    )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN2    )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_TILT     )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("UNK")	/* $b00006.w */
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")	/* read by the Z80 through the sound chip */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x02, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x0c, 0x0c, "Bombs" )
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Easy )    )
	PORT_DIPSETTING(    0x20, DEF_STR( Normal )  )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard )    )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x80, "100K, 300K" )
	PORT_DIPSETTING(    0xc0, "200K, 500K" )
	PORT_DIPSETTING(    0x40, "500K, 1000K" )
	PORT_DIPSETTING(    0x00, "1000K, 3000K" )

	PORT_START("DSW2")	/* read by the Z80 through the sound chip */
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coinage ) )
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
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )
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

/* YM3014B + YM2203C */

static void irq_handler(device_t *device, int irq)
{
	cputag_set_input_line(device->machine(), "audiocpu", 0, irq ? ASSERT_LINE : CLEAR_LINE);
}

static const ym2203_interface ym2203_intf_sandscrp =
{
	{
		AY8910_LEGACY_OUTPUT,
		AY8910_DEFAULT_LOADS,
		DEVCB_INPUT_PORT("DSW1"), /* Port A Read */
		DEVCB_INPUT_PORT("DSW2"), /* Port B Read */
		DEVCB_NULL,	/* Port A Write */
		DEVCB_NULL,	/* Port B Write */
	},
	irq_handler	/* IRQ handler */
};


static const kaneko_pandora_interface sandscrp_pandora_config =
{
	"screen",	/* screen tag */
	0,	/* gfx_region */
	0, 0	/* x_offs, y_offs */
};

static MACHINE_CONFIG_START( sandscrp, sandscrp_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000,12000000)	/* TMP68HC000N-12 */
	MCFG_CPU_PROGRAM_MAP(sandscrp)
	MCFG_CPU_VBLANK_INT("screen", sandscrp_interrupt)

	MCFG_CPU_ADD("audiocpu", Z80,4000000)	/* Z8400AB1, Reads the DSWs: it can't be disabled */
	MCFG_CPU_PROGRAM_MAP(sandscrp_soundmem)
	MCFG_CPU_IO_MAP(sandscrp_soundport)

	MCFG_WATCHDOG_TIME_INIT(attotime::from_seconds(3))	/* a guess, and certainly wrong */

	MCFG_MACHINE_RESET(sandscrp)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME( ATTOSECONDS_IN_USEC(2500) /* not accurate */ )
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 256-1, 0+16, 256-16-1)
	MCFG_SCREEN_UPDATE_STATIC(sandscrp)
	MCFG_SCREEN_VBLANK_STATIC(sandscrp)

	MCFG_GFXDECODE(sandscrp)
	MCFG_PALETTE_LENGTH(2048)

	MCFG_KANEKO_PANDORA_ADD("pandora", sandscrp_pandora_config)

	MCFG_VIDEO_START(sandscrp_1xVIEW2)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_OKIM6295_ADD("oki", 12000000/6, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.25)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.25)

	MCFG_SOUND_ADD("ymsnd", YM2203, 4000000)
	MCFG_SOUND_CONFIG(ym2203_intf_sandscrp)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.25)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.25)
MACHINE_CONFIG_END



/***************************************************************************

                                Sand Scorpion

***************************************************************************/

ROM_START( sandscrp ) /* Z03VA-003 PCB */
	ROM_REGION( 0x080000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "11.bin", 0x000000, 0x040000, CRC(9b24ab40) SHA1(3187422dbe8b15d8053be4cb20e56d3e6afbd5f2) ) /* Location is IC4 */
	ROM_LOAD16_BYTE( "12.bin", 0x000001, 0x040000, CRC(ad12caee) SHA1(83267445b89c3cf4dc317106aa68763d2f29eff7) ) /* Location is IC5 */

	ROM_REGION( 0x24000, "audiocpu", 0 )		/* Z80 Code */
	ROM_LOAD( "8.ic51", 0x00000, 0x0c000, CRC(6f3e9db1) SHA1(06a04fa17f44319986913bff70433510c89e38f1) )
	ROM_CONTINUE(       0x10000, 0x14000 )

	ROM_REGION( 0x100000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD( "5.ic16", 0x000000, 0x080000, CRC(9bb675f6) SHA1(c3f6768cfd99a0e19ca2224fff9aa4e27ec0da24) )
	ROM_LOAD( "6.ic17", 0x080000, 0x080000, CRC(7df2f219) SHA1(e2a59e201bfededa92d6c86f8dc1b212527ef66f) )

	ROM_REGION( 0x100000, "gfx2", 0 )	/* Layers */
	ROM_LOAD16_BYTE( "4.ic32", 0x000000, 0x080000, CRC(b9222ff2) SHA1(a445da3f7f5dea5ff64bb0b048f624f947875a39) )
	ROM_LOAD16_BYTE( "3.ic33", 0x000001, 0x080000, CRC(adf20fa0) SHA1(67a7a2be774c86916cbb97e4c9b16c2e48125780) )

	ROM_REGION( 0x040000, "oki", 0 )	/* Samples */
	ROM_LOAD( "7.ic55", 0x000000, 0x040000, CRC(9870ab12) SHA1(5ea3412cbc57bfaa32a1e2552b2eb46f4ceb5fa8) )
ROM_END

ROM_START( sandscrpa ) /* Z03VA-003 PCB, earlier program version */
	ROM_REGION( 0x080000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "1.ic4", 0x000000, 0x040000, CRC(c0943ae2) SHA1(04dac4e1f116cd96d6292daa61ef40efc7eba919) )
	ROM_LOAD16_BYTE( "2.ic5", 0x000001, 0x040000, CRC(6a8e0012) SHA1(2350b11c9bd545c8ba4b3c25cd6547ba2ad474b5) )

	ROM_REGION( 0x24000, "audiocpu", 0 )		/* Z80 Code */
	ROM_LOAD( "8.ic51", 0x00000, 0x0c000, CRC(6f3e9db1) SHA1(06a04fa17f44319986913bff70433510c89e38f1) )
	ROM_CONTINUE(       0x10000, 0x14000 )

	ROM_REGION( 0x100000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD( "5.ic16", 0x000000, 0x080000, CRC(9bb675f6) SHA1(c3f6768cfd99a0e19ca2224fff9aa4e27ec0da24) )
	ROM_LOAD( "6.ic17", 0x080000, 0x080000, CRC(7df2f219) SHA1(e2a59e201bfededa92d6c86f8dc1b212527ef66f) )

	ROM_REGION( 0x100000, "gfx2", 0 )	/* Layers */
	ROM_LOAD16_BYTE( "4.ic32", 0x000000, 0x080000, CRC(b9222ff2) SHA1(a445da3f7f5dea5ff64bb0b048f624f947875a39) )
	ROM_LOAD16_BYTE( "3.ic33", 0x000001, 0x080000, CRC(adf20fa0) SHA1(67a7a2be774c86916cbb97e4c9b16c2e48125780) )

	ROM_REGION( 0x040000, "oki", 0 )	/* Samples */
	ROM_LOAD( "7.ic55", 0x000000, 0x040000, CRC(9870ab12) SHA1(5ea3412cbc57bfaa32a1e2552b2eb46f4ceb5fa8) )
ROM_END


ROM_START( sandscrpb ) /* Different rev PCB */
	ROM_REGION( 0x080000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "11.ic4", 0x000000, 0x040000, CRC(80020cab) SHA1(4f1f4d8ea07ad745f2d6d3f800686f07fe4bf20f) ) /* Chinese title screen */
	ROM_LOAD16_BYTE( "12.ic5", 0x000001, 0x040000, CRC(8df1d42f) SHA1(2a9db5c4b99a8a3f62bffa9ddd96a95e2042602b) ) /* Game & test menu in English */
	/* internet translators come up with "fighter lion king" and / or "Hits lion Emperor Quickly" */

	ROM_REGION( 0x24000, "audiocpu", 0 )		/* Z80 Code */
	ROM_LOAD( "8.ic51", 0x00000, 0x0c000, CRC(6f3e9db1) SHA1(06a04fa17f44319986913bff70433510c89e38f1) )
	ROM_CONTINUE(       0x10000, 0x14000 )

	ROM_REGION( 0x100000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD( "ss502.ic16", 0x000000, 0x100000, CRC(d8012ebb) SHA1(975bbb3b57a09e41d2257d4fa3a64097144de554) )

	ROM_REGION( 0x100000, "gfx2", 0 )	/* Layers */
	ROM_LOAD16_WORD_SWAP( "ss501.ic30", 0x000000, 0x100000, CRC(0cf9f99d) SHA1(47f7f120d2bc075bedaff0a44306a8f46a1d848c) )

	ROM_REGION( 0x040000, "oki", 0 )	/* Samples */
	ROM_LOAD( "7.ic55", 0x000000, 0x040000, CRC(9870ab12) SHA1(5ea3412cbc57bfaa32a1e2552b2eb46f4ceb5fa8) )
ROM_END


GAME( 1992, sandscrp,  0,        sandscrp, sandscrp, 0,          ROT90, "Face",   "Sand Scorpion", 0 )
GAME( 1992, sandscrpa, sandscrp, sandscrp, sandscrp, 0,          ROT90, "Face",   "Sand Scorpion (Earlier)", 0 )
GAME( 1992, sandscrpb, sandscrp, sandscrp, sandscrp, 0,          ROT90, "Face",   "Sand Scorpion (Chinese Title Screen, Revised Hardware)", 0 )
