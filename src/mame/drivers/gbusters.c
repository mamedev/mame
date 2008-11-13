/***************************************************************************

Gangbusters(GX878) (c) 1988 Konami

Preliminary driver by:
    Manuel Abadia <manu@teleline.es>

***************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "cpu/konami/konami.h" /* for the callback and the firq irq definition */
#include "video/konamiic.h"
#include "sound/2151intf.h"
#include "sound/k007232.h"

/* prototypes */
static MACHINE_RESET( gbusters );
static void gbusters_banking( int lines );


extern int gbusters_priority;

VIDEO_START( gbusters );
VIDEO_UPDATE( gbusters );

static int palette_selected;
static UINT8 *ram;

static INTERRUPT_GEN( gbusters_interrupt )
{
	if (K052109_is_IRQ_enabled())
		cpu_set_input_line(device, KONAMI_IRQ_LINE, HOLD_LINE);
}

static READ8_HANDLER( bankedram_r )
{
	if (palette_selected)
		return paletteram[offset];
	else
		return ram[offset];
}

static WRITE8_HANDLER( bankedram_w )
{
	if (palette_selected)
		paletteram_xBBBBBGGGGGRRRRR_be_w(machine,offset,data);
	else
		ram[offset] = data;
}

static WRITE8_HANDLER( gbusters_1f98_w )
{

	/* bit 0 = enable char ROM reading through the video RAM */
	K052109_set_RMRD_line((data & 0x01) ? ASSERT_LINE : CLEAR_LINE);

	/* bit 7 used (during gfx rom tests), but unknown */

	/* other bits unused/unknown */
	if (data & 0xfe){
		//logerror("%04x: (1f98) write %02x\n",cpu_get_pc(machine->activecpu), data);
		//popmessage("$1f98 = %02x", data);
	}
}

static WRITE8_HANDLER( gbusters_coin_counter_w )
{
	/* bit 0 select palette RAM  or work RAM at 5800-5fff */
	palette_selected = ~data & 0x01;

	/* bits 1 & 2 = coin counters */
	coin_counter_w(0,data & 0x02);
	coin_counter_w(1,data & 0x04);

	/* bits 3 selects tilemap priority */
	gbusters_priority = data & 0x08;

	/* bit 7 is used but unknown */

	/* other bits unused/unknown */
	if (data & 0xf8)
	{
#if 0
		char baf[40];
		sprintf(baf,"ccnt = %02x", data);
		popmessage(baf);
#endif
		logerror("%04x: (ccount) write %02x\n",cpu_get_pc(machine->activecpu), data);
	}
}

static WRITE8_HANDLER( gbusters_unknown_w )
{
	logerror("%04x: write %02x to 0x1f9c\n",cpu_get_pc(machine->activecpu), data);

{
char baf[40];
	sprintf(baf,"??? = %02x", data);
//  popmessage(baf);
}
}

static WRITE8_HANDLER( gbusters_sh_irqtrigger_w )
{
	cpu_set_input_line_and_vector(machine->cpu[1],0,HOLD_LINE,0xff);
}

static WRITE8_HANDLER( gbusters_snd_bankswitch_w )
{
	int bank_B = ((data >> 2) & 0x01);	/* ?? */
	int bank_A = ((data) & 0x01);		/* ?? */
	k007232_set_bank( 0, bank_A, bank_B );

#if 0
	{
		char baf[40];
		sprintf(baf,"snd_bankswitch = %02x", data);
		popmessage(baf);
	}
#endif
}

static ADDRESS_MAP_START( gbusters_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x1f90, 0x1f90) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x1f91, 0x1f91) AM_READ_PORT("P1")
	AM_RANGE(0x1f92, 0x1f92) AM_READ_PORT("P2")
	AM_RANGE(0x1f93, 0x1f93) AM_READ_PORT("DSW3")
	AM_RANGE(0x1f94, 0x1f94) AM_READ_PORT("DSW1")
	AM_RANGE(0x1f95, 0x1f95) AM_READ_PORT("DSW2")
	AM_RANGE(0x0000, 0x3fff) AM_READ(K052109_051960_r)	/* tiles + sprites (RAM H21, G21 & H6) */
	AM_RANGE(0x4000, 0x57ff) AM_READ(SMH_RAM)			/* RAM I12 */
	AM_RANGE(0x5800, 0x5fff) AM_READ(bankedram_r)		/* palette + work RAM (RAM D16 & C16) */
	AM_RANGE(0x6000, 0x7fff) AM_READ(SMH_BANK1)			/* banked ROM */
	AM_RANGE(0x8000, 0xffff) AM_READ(SMH_ROM)			/* ROM 878n02.rom */
ADDRESS_MAP_END

static ADDRESS_MAP_START( gbusters_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x1f80, 0x1f80) AM_WRITE(gbusters_coin_counter_w)	/* coin counters */
	AM_RANGE(0x1f84, 0x1f84) AM_WRITE(soundlatch_w)				/* sound code # */
	AM_RANGE(0x1f88, 0x1f88) AM_WRITE(gbusters_sh_irqtrigger_w)	/* cause interrupt on audio CPU */
	AM_RANGE(0x1f8c, 0x1f8c) AM_WRITE(watchdog_reset_w)			/* watchdog reset */
	AM_RANGE(0x1f98, 0x1f98) AM_WRITE(gbusters_1f98_w)			/* enable gfx ROM read through VRAM */
	AM_RANGE(0x1f9c, 0x1f9c) AM_WRITE(gbusters_unknown_w)			/* ??? */
	AM_RANGE(0x0000, 0x3fff) AM_WRITE(K052109_051960_w)			/* tiles + sprites (RAM H21, G21 & H6) */
	AM_RANGE(0x4000, 0x57ff) AM_WRITE(SMH_RAM)					/* RAM I12 */
	AM_RANGE(0x5800, 0x5fff) AM_WRITE(bankedram_w) AM_BASE(&ram)			/* palette + work RAM (RAM D16 & C16) */
	AM_RANGE(0x6000, 0x7fff) AM_WRITE(SMH_ROM)					/* banked ROM */
	AM_RANGE(0x8000, 0xffff) AM_WRITE(SMH_ROM)					/* ROM 878n02.rom */
ADDRESS_MAP_END

static ADDRESS_MAP_START( gbusters_readmem_sound, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_READ(SMH_ROM)				/* ROM 878h01.rom */
	AM_RANGE(0x8000, 0x87ff) AM_READ(SMH_RAM)				/* RAM */
	AM_RANGE(0xa000, 0xa000) AM_READ(soundlatch_r)			/* soundlatch_r */
	AM_RANGE(0xb000, 0xb00d) AM_READ(k007232_read_port_0_r)	/* 007232 registers */
	AM_RANGE(0xc001, 0xc001) AM_READ(ym2151_status_port_0_r)	/* YM 2151 */
ADDRESS_MAP_END

static ADDRESS_MAP_START( gbusters_writemem_sound, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_WRITE(SMH_ROM)					/* ROM 878h01.rom */
	AM_RANGE(0x8000, 0x87ff) AM_WRITE(SMH_RAM)					/* RAM */
	AM_RANGE(0xb000, 0xb00d) AM_WRITE(k007232_write_port_0_w)		/* 007232 registers */
	AM_RANGE(0xc000, 0xc000) AM_WRITE(ym2151_register_port_0_w)	/* YM 2151 */
	AM_RANGE(0xc001, 0xc001) AM_WRITE(ym2151_data_port_0_w)		/* YM 2151 */
	AM_RANGE(0xf000, 0xf000) AM_WRITE(gbusters_snd_bankswitch_w)	/* 007232 bankswitch? */
ADDRESS_MAP_END

/***************************************************************************

    Input Ports

***************************************************************************/

static INPUT_PORTS_START( gbusters )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_7C ) )
//  PORT_DIPSETTING(    0x00, "Invalid" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x04, 0x04, "Bullets" )
	PORT_DIPSETTING(    0x04, "50" )
	PORT_DIPSETTING(    0x00, "60" )
	PORT_DIPNAME( 0x18, 0x10, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x18, "50k, 200k & 400k" )
	PORT_DIPSETTING(    0x10, "70k, 250k & 500k" )
	PORT_DIPSETTING(    0x08, "50k" )
	PORT_DIPSETTING(    0x00, "70k" )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, "Difficult" )
	PORT_DIPSETTING(    0x00, "Very Difficult" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x04, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


/***************************************************************************

    Machine Driver

***************************************************************************/

static void volume_callback(int v)
{
	k007232_set_volume(0,0,(v >> 4) * 0x11,0);
	k007232_set_volume(0,1,0,(v & 0x0f) * 0x11);
}

static const k007232_interface k007232_config =
{
	volume_callback	/* external port callback */
};

static MACHINE_DRIVER_START( gbusters )

	/* basic machine hardware */
	MDRV_CPU_ADD("main", KONAMI, 3000000)	/* Konami custom 052526 */
	MDRV_CPU_PROGRAM_MAP(gbusters_readmem,gbusters_writemem)
	MDRV_CPU_VBLANK_INT("main", gbusters_interrupt)

	MDRV_CPU_ADD("audio", Z80, 3579545)		/* ? */
	MDRV_CPU_PROGRAM_MAP(gbusters_readmem_sound,gbusters_writemem_sound)

	MDRV_MACHINE_RESET(gbusters)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_HAS_SHADOWS)

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(14*8, (64-14)*8-1, 2*8, 30*8-1 )

	MDRV_PALETTE_LENGTH(1024)

	MDRV_VIDEO_START(gbusters)
	MDRV_VIDEO_UPDATE(gbusters)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ym", YM2151, 3579545)
	MDRV_SOUND_ROUTE(0, "mono", 0.60)
	MDRV_SOUND_ROUTE(1, "mono", 0.60)

	MDRV_SOUND_ADD("konami", K007232, 3579545)
	MDRV_SOUND_CONFIG(k007232_config)
	MDRV_SOUND_ROUTE(0, "mono", 0.30)
	MDRV_SOUND_ROUTE(1, "mono", 0.30)
MACHINE_DRIVER_END


/***************************************************************************

  Game ROMs

***************************************************************************/

ROM_START( gbusters )
	ROM_REGION( 0x30800, "main", 0 ) /* code + banked roms + space for banked RAM */
	ROM_LOAD( "878n02.k13", 0x10000, 0x08000, CRC(51697aaa) SHA1(1e6461e2e5e871d44085623a890158a4c1c4c404) )
	ROM_CONTINUE(           0x08000, 0x08000 )
	ROM_LOAD( "878j03.k15", 0x20000, 0x10000, CRC(3943a065) SHA1(6b0863f4182e6c973adfaa618f096bd4cc9b7b6d) )

	ROM_REGION( 0x10000, "audio", 0 ) /* 64k for the sound CPU */
	ROM_LOAD( "878h01.f8", 0x00000, 0x08000, CRC(96feafaa) SHA1(8b6547e610cb4fa1c1f5bf12cb05e9a12a353903) )

	ROM_REGION( 0x80000, "gfx1", 0 ) /* graphics (addressable by the main CPU) */
	ROM_LOAD( "878c07.h27", 0x00000, 0x40000, CRC(eeed912c) SHA1(b2e27610b38f3fc9c2cdad600b03c8bae4fb9138) ) /* tiles */
	ROM_LOAD( "878c08.k27", 0x40000, 0x40000, CRC(4d14626d) SHA1(226b1d83fb82586302be0a67737a427475856537) ) /* tiles */

	ROM_REGION( 0x80000, "gfx2", 0 ) /* graphics (addressable by the main CPU) */
	ROM_LOAD( "878c05.h5", 0x00000, 0x40000, CRC(01f4aea5) SHA1(124123823be6bd597805484539d821aaaadde2c0) ) /* sprites */
	ROM_LOAD( "878c06.k5", 0x40000, 0x40000, CRC(edfaaaaf) SHA1(67468c4ce47e8d43d58de8d3b50b048c66508156) ) /* sprites */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "878a09.f20",   0x0000, 0x0100, CRC(e2d09a1b) SHA1(a9651e137486b2df367c39eb43f52d0833589e87) ) /* priority encoder (not used) */

	ROM_REGION( 0x40000, "konami", 0 ) /* samples for 007232 */
	ROM_LOAD( "878c04.d5",  0x00000, 0x40000, CRC(9e982d1c) SHA1(a5b611c67b0f2ac50c679707931ee12ebbf72ebe) )
ROM_END

ROM_START( gbustera )
	ROM_REGION( 0x30800, "main", 0 ) /* code + banked roms + space for banked RAM */
	ROM_LOAD( "878_02.k13", 0x10000, 0x08000, CRC(57178414) SHA1(89b1403158f6ce18706c8a941109554d03cf77d9) ) /* unknown region/version leter */
	ROM_CONTINUE(           0x08000, 0x08000 )
	ROM_LOAD( "878_03.k15", 0x20000, 0x10000, CRC(6c59e660) SHA1(66a92eb8a93c9f542489fa31bec6ed1819d174da) ) /* unknown region/version leter */

	ROM_REGION( 0x10000, "audio", 0 ) /* 64k for the sound CPU */
	ROM_LOAD( "878h01.f8", 0x00000, 0x08000, CRC(96feafaa) SHA1(8b6547e610cb4fa1c1f5bf12cb05e9a12a353903) )

	ROM_REGION( 0x80000, "gfx1", 0 ) /* graphics (addressable by the main CPU) */
	ROM_LOAD( "878c07.h27", 0x00000, 0x40000, CRC(eeed912c) SHA1(b2e27610b38f3fc9c2cdad600b03c8bae4fb9138) ) /* tiles */
	ROM_LOAD( "878c08.k27", 0x40000, 0x40000, CRC(4d14626d) SHA1(226b1d83fb82586302be0a67737a427475856537) ) /* tiles */

	ROM_REGION( 0x80000, "gfx2", 0 ) /* graphics (addressable by the main CPU) */
	ROM_LOAD( "878c05.h5", 0x00000, 0x40000, CRC(01f4aea5) SHA1(124123823be6bd597805484539d821aaaadde2c0) ) /* sprites */
	ROM_LOAD( "878c06.k5", 0x40000, 0x40000, CRC(edfaaaaf) SHA1(67468c4ce47e8d43d58de8d3b50b048c66508156) ) /* sprites */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "878a09.f20",   0x0000, 0x0100, CRC(e2d09a1b) SHA1(a9651e137486b2df367c39eb43f52d0833589e87) ) /* priority encoder (not used) */

	ROM_REGION( 0x40000, "konami", 0 ) /* samples for 007232 */
	ROM_LOAD( "878c04.d5",  0x00000, 0x40000, CRC(9e982d1c) SHA1(a5b611c67b0f2ac50c679707931ee12ebbf72ebe) )
ROM_END

ROM_START( crazycop )
	ROM_REGION( 0x30800, "main", 0 ) /* code + banked roms + space for banked RAM */
	ROM_LOAD( "878m02.k13", 0x10000, 0x08000, CRC(9c1c9f52) SHA1(7a60ad20aac92da8258b43b04f8c7f27bb71f1df) )
	ROM_CONTINUE(           0x08000, 0x08000 )
	ROM_LOAD( "878j03.k15", 0x20000, 0x10000, CRC(3943a065) SHA1(6b0863f4182e6c973adfaa618f096bd4cc9b7b6d) )

	ROM_REGION( 0x10000, "audio", 0 ) /* 64k for the sound CPU */
	ROM_LOAD( "878h01.f8", 0x00000, 0x08000, CRC(96feafaa) SHA1(8b6547e610cb4fa1c1f5bf12cb05e9a12a353903) )

	ROM_REGION( 0x80000, "gfx1", 0 ) /* graphics (addressable by the main CPU) */
	ROM_LOAD( "878c07.h27", 0x00000, 0x40000, CRC(eeed912c) SHA1(b2e27610b38f3fc9c2cdad600b03c8bae4fb9138) ) /* tiles */
	ROM_LOAD( "878c08.k27", 0x40000, 0x40000, CRC(4d14626d) SHA1(226b1d83fb82586302be0a67737a427475856537) ) /* tiles */

	ROM_REGION( 0x80000, "gfx2", 0 ) /* graphics (addressable by the main CPU) */
	ROM_LOAD( "878c05.h5", 0x00000, 0x40000, CRC(01f4aea5) SHA1(124123823be6bd597805484539d821aaaadde2c0) )	/* sprites */
	ROM_LOAD( "878c06.k5", 0x40000, 0x40000, CRC(edfaaaaf) SHA1(67468c4ce47e8d43d58de8d3b50b048c66508156) )	/* sprites */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "878a09.f20",   0x0000, 0x0100, CRC(e2d09a1b) SHA1(a9651e137486b2df367c39eb43f52d0833589e87) ) /* priority encoder (not used) */

	ROM_REGION( 0x40000, "konami", 0 ) /* samples for 007232 */
	ROM_LOAD( "878c04.d5",  0x00000, 0x40000, CRC(9e982d1c) SHA1(a5b611c67b0f2ac50c679707931ee12ebbf72ebe) )
ROM_END


static void gbusters_banking( int lines )
{
	UINT8 *RAM = memory_region(Machine, "main");
	int offs = 0x10000;

	/* bits 0-3 ROM bank */
	offs += (lines & 0x0f)*0x2000;
	memory_set_bankptr( 1, &RAM[offs] );

	if (lines & 0xf0){
		//logerror("%04x: (lines) write %02x\n",cpu_get_pc(machine->activecpu), lines);
		//popmessage("lines = %02x", lines);
	}

	/* other bits unknown */
}

static MACHINE_RESET( gbusters )
{
	UINT8 *RAM = memory_region(machine, "main");

	cpu_set_info_fct(machine->cpu[0], CPUINFO_PTR_KONAMI_SETLINES_CALLBACK, (genf *)gbusters_banking);

	/* mirror address for banked ROM */
	memcpy(&RAM[0x18000], &RAM[0x10000], 0x08000 );

	paletteram = &RAM[0x30000];
}


static DRIVER_INIT( gbusters )
{
	konami_rom_deinterleave_2("gfx1");
	konami_rom_deinterleave_2("gfx2");
}



GAME( 1988, gbusters, 0,        gbusters, gbusters, gbusters, ROT90, "Konami", "Gang Busters (set 1)", 0 ) /* N02 & J03 program roms */
GAME( 1988, gbustera, gbusters, gbusters, gbusters, gbusters, ROT90, "Konami", "Gang Busters (set 2)", 0 ) /* unknown region program roms */
GAME( 1988, crazycop, gbusters, gbusters, gbusters, gbusters, ROT90, "Konami", "Crazy Cop (Japan)", 0 )    /* M02 & J03 program roms */
