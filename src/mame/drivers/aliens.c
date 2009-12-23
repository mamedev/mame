/***************************************************************************

Aliens (c) 1990 Konami Co. Ltd

Preliminary driver by:
    Manuel Abadia <manu@teleline.es>

***************************************************************************/

#include "driver.h"
#include "cpu/z80/z80.h"
#include "cpu/konami/konami.h" /* for the callback and the firq irq definition */
#include "sound/k007232.h"
#include "sound/2151intf.h"
#include "video/konicdev.h"
#include "includes/konamipt.h"

/* prototypes */
static MACHINE_RESET( aliens );
static KONAMI_SETLINES_CALLBACK( aliens_banking );

extern void aliens_tile_callback(running_machine *machine, int layer,int bank,int *code,int *color, int *flags, int *priority);
extern void aliens_sprite_callback(running_machine *machine, int *code,int *color,int *priority_mask,int *shadow);

VIDEO_START( aliens );
VIDEO_UPDATE( aliens );


static int palette_selected;
static UINT8 *ram;


static INTERRUPT_GEN( aliens_interrupt )
{
	const device_config *k051960 = devtag_get_device(device->machine, "k051960");

	if (k051960_is_irq_enabled(k051960))
		cpu_set_input_line(device, KONAMI_IRQ_LINE, HOLD_LINE);
}

static READ8_HANDLER( bankedram_r )
{
	if (palette_selected)
		return space->machine->generic.paletteram.u8[offset];
	else
		return ram[offset];
}

static WRITE8_HANDLER( bankedram_w )
{
	if (palette_selected)
		paletteram_xBBBBBGGGGGRRRRR_be_w(space,offset,data);
	else
		ram[offset] = data;
}

static WRITE8_HANDLER( aliens_coin_counter_w )
{
	const device_config *k052109 = devtag_get_device(space->machine, "k052109");

	/* bits 0-1 = coin counters */
	coin_counter_w(space->machine, 0,data & 0x01);
	coin_counter_w(space->machine, 1,data & 0x02);

	/* bit 5 = select work RAM or palette */
	palette_selected = data & 0x20;

	/* bit 6 = enable char ROM reading through the video RAM */
	k052109_set_rmrd_line(k052109, (data & 0x40) ? ASSERT_LINE : CLEAR_LINE);

	/* other bits unknown */
#if 0
{
	char baf[40];
	sprintf(baf,"%02x",data);
	popmessage(baf);
}
#endif
}

static WRITE8_HANDLER( aliens_sh_irqtrigger_w )
{
	soundlatch_w(space,offset,data);
	cputag_set_input_line_and_vector(space->machine, "audiocpu", 0, HOLD_LINE, 0xff);
}

static WRITE8_DEVICE_HANDLER( aliens_snd_bankswitch_w )
{
	/* b1: bank for chanel A */
	/* b0: bank for chanel B */

	int bank_A = ((data >> 1) & 0x01);
	int bank_B = ((data) & 0x01);

	k007232_set_bank( devtag_get_device(device->machine, "konami"), bank_A, bank_B );
}


static READ8_HANDLER( k052109_051960_r )
{
	const device_config *k052109 = devtag_get_device(space->machine, "k052109");
	const device_config *k051960 = devtag_get_device(space->machine, "k051960");

	if (k052109_get_rmrd_line(k052109) == CLEAR_LINE)
	{
		if (offset >= 0x3800 && offset < 0x3808)
			return k051937_r(k051960, offset - 0x3800);
		else if (offset < 0x3c00)
			return k052109_r(k052109, offset);
		else
			return k051960_r(k051960, offset - 0x3c00);
	}
	else
		return k052109_r(k052109, offset);
}

static WRITE8_HANDLER( k052109_051960_w )
{
	const device_config *k052109 = devtag_get_device(space->machine, "k052109");
	const device_config *k051960 = devtag_get_device(space->machine, "k051960");

	if (offset >= 0x3800 && offset < 0x3808)
		k051937_w(k051960, offset - 0x3800, data);
	else if (offset < 0x3c00)
		k052109_w(k052109, offset, data);
	else
		k051960_w(k051960, offset - 0x3c00, data);
}

static ADDRESS_MAP_START( aliens_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x03ff) AM_READWRITE(bankedram_r, bankedram_w) AM_BASE(&ram)		/* palette + work RAM */
	AM_RANGE(0x0400, 0x1fff) AM_RAM
	AM_RANGE(0x2000, 0x3fff) AM_ROMBANK("bank1")												/* banked ROM */
	AM_RANGE(0x5f80, 0x5f80) AM_READ_PORT("DSW3")
	AM_RANGE(0x5f81, 0x5f81) AM_READ_PORT("P1")
	AM_RANGE(0x5f82, 0x5f82) AM_READ_PORT("P2")
	AM_RANGE(0x5f83, 0x5f83) AM_READ_PORT("DSW2")
	AM_RANGE(0x5f84, 0x5f84) AM_READ_PORT("DSW1")
	AM_RANGE(0x5f88, 0x5f88) AM_READWRITE(watchdog_reset_r, aliens_coin_counter_w)		/* coin counters */
	AM_RANGE(0x5f8c, 0x5f8c) AM_WRITE(aliens_sh_irqtrigger_w)							/* cause interrupt on audio CPU */
	AM_RANGE(0x4000, 0x7fff) AM_READWRITE(k052109_051960_r, k052109_051960_w)
	AM_RANGE(0x8000, 0xffff) AM_ROM														/* ROM e24_j02.bin */
ADDRESS_MAP_END

static ADDRESS_MAP_START( aliens_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM								/* ROM g04_b03.bin */
	AM_RANGE(0x8000, 0x87ff) AM_RAM								/* RAM */
	AM_RANGE(0xa000, 0xa001) AM_DEVREADWRITE("ymsnd", ym2151_r, ym2151_w)
	AM_RANGE(0xc000, 0xc000) AM_READ(soundlatch_r)				/* soundlatch_r */
	AM_RANGE(0xe000, 0xe00d) AM_DEVREADWRITE("konami", k007232_r, k007232_w)
ADDRESS_MAP_END


/***************************************************************************

    Input Ports

***************************************************************************/

static INPUT_PORTS_START( aliens )
	PORT_START("DSW1")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "No Credits", SW1)
	/* "No Credits" = both coin slots open, but no effect on coin counters */

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )		PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPUNUSED_DIPLOC( 0x0004, 0x0004, "SW2:3" )	/* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0008, 0x0008, "SW2:4" )	/* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0010, "SW2:5" )	/* Listed as "Unused" */
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )	PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )	PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )	PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW3:2" )		/* Listed as "Unused" */
	PORT_SERVICE_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW3:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW3:4" )		/* Listed as "Unused" */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1")
	KONAMI8_B12_COIN_START(1)

	PORT_START("P2")
	KONAMI8_B12_COIN_START(2)
INPUT_PORTS_END


/***************************************************************************

    Machine Driver

***************************************************************************/

static void volume_callback(const device_config *device, int v)
{
	k007232_set_volume(device,0,(v & 0x0f) * 0x11,0);
	k007232_set_volume(device,1,0,(v >> 4) * 0x11);
}

static const k007232_interface k007232_config =
{
	volume_callback	/* external port callback */
};

static const ym2151_interface ym2151_config =
{
	0,
	aliens_snd_bankswitch_w
};


static const k052109_interface aliens_k052109_intf =
{
	"gfx1", 0,
	NORMAL_PLANE_ORDER,
	KONAMI_ROM_DEINTERLEAVE_2,
	aliens_tile_callback
};

static const k051960_interface aliens_k051960_intf =
{
	"gfx2", 1,
	NORMAL_PLANE_ORDER,
	KONAMI_ROM_DEINTERLEAVE_2,
	aliens_sprite_callback
};

static MACHINE_DRIVER_START( aliens )

	/* basic machine hardware */

	/* external clock should be 12MHz probably, CPU internal divider and precise cycle timings */
	/* are unknown though. 3MHz is too low, sprites flicker in the pseudo-3D levels */
	MDRV_CPU_ADD("maincpu", KONAMI, 6000000)		/* ? */
	MDRV_CPU_PROGRAM_MAP(aliens_map)
	MDRV_CPU_VBLANK_INT("screen", aliens_interrupt)

	MDRV_CPU_ADD("audiocpu", Z80, 3579545)
	MDRV_CPU_PROGRAM_MAP(aliens_sound_map)

	MDRV_MACHINE_RESET(aliens)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_HAS_SHADOWS)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(14*8, (64-14)*8-1, 2*8, 30*8-1 )

	MDRV_PALETTE_LENGTH(512)

	MDRV_VIDEO_START(aliens)
	MDRV_VIDEO_UPDATE(aliens)

	MDRV_K052109_ADD("k052109", aliens_k052109_intf)
	MDRV_K051960_ADD("k051960", aliens_k051960_intf)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ymsnd", YM2151, 3579545)
	MDRV_SOUND_CONFIG(ym2151_config)
	MDRV_SOUND_ROUTE(0, "mono", 0.60)
	MDRV_SOUND_ROUTE(1, "mono", 0.60)

	MDRV_SOUND_ADD("konami", K007232, 3579545)
	MDRV_SOUND_CONFIG(k007232_config)
	MDRV_SOUND_ROUTE(0, "mono", 0.20)
	MDRV_SOUND_ROUTE(1, "mono", 0.20)
MACHINE_DRIVER_END


/***************************************************************************

  Game ROMs

***************************************************************************/

ROM_START( aliens )
	ROM_REGION( 0x38000, "maincpu", 0 ) /* code + banked roms */
	ROM_LOAD( "875_j02.e24", 0x10000, 0x08000, CRC(56c20971) SHA1(af272e146705e97342466a208c64d823ebc83d83) )
	ROM_CONTINUE(            0x08000, 0x08000 )
	ROM_LOAD( "875_j01.c24", 0x18000, 0x20000, CRC(6a529cd6) SHA1(bff6dee33141d8ed2b2c28813cf49f52dceac364) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 64k for the sound CPU */
	ROM_LOAD( "875_b03.g04", 0x00000, 0x08000, CRC(1ac4d283) SHA1(2253f1f39c7edb6cef438b3d97f3af67a1f491ff) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* graphics */
	ROM_LOAD( "875b11.k13", 0x000000, 0x80000, CRC(89c5c885) SHA1(02a1581579b6ef816e04bec312a7b3ae7c7e84f8) )	/* characters (set 1) */
	ROM_LOAD( "875b07.j13", 0x080000, 0x40000, CRC(e9c56d66) SHA1(1f58949d5391aef002a6e1ee7034e57bf99cee61) )	/* characters (set 2) */
	/* second half empty */
	ROM_LOAD( "875b12.k19", 0x100000, 0x80000, CRC(ea6bdc17) SHA1(a7c22370f8adc5b479283f1ff831f493df78282f) )	/* characters (set 1) */
	ROM_LOAD( "875b08.j19", 0x180000, 0x40000, CRC(f9387966) SHA1(470ecc4a5a3edd08d5e0ab10b0c590db1968fb0a) )	/* characters (set 2) */
	/* second half empty */

	ROM_REGION( 0x200000, "gfx2", 0 ) /* graphics */
	ROM_LOAD( "875b10.k08", 0x000000, 0x80000, CRC(0b1035b1) SHA1(db04020761386e79249762cd1540208375c38c7f) )	/* sprites (set 1) */
	ROM_LOAD( "875b06.j08", 0x080000, 0x40000, CRC(081a0566) SHA1(3a4aa14178fe76a030224743c9e9cd974e08bd79) )	/* sprites (set 2) */
	/* second half empty */
	ROM_LOAD( "875b09.k02", 0x100000, 0x80000, CRC(e76b3c19) SHA1(6838e07460b3eaaeb129208ad0696c8019bd63d9) )	/* sprites (set 1) */
	ROM_LOAD( "875b05.j02", 0x180000, 0x40000, CRC(19a261f2) SHA1(b0518fad833b3e613e0201d5d9cab73dc5e78e1d) )	/* sprites (set 2) */
	/* second half empty */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "821a08.h14",  0x0000, 0x0100, CRC(7da55800) SHA1(3826f73569c8ae0431510a355bdfa082152b74a5) )	/* priority encoder (not used) */

	ROM_REGION( 0x40000, "konami", 0 ) /* samples for 007232 */
	ROM_LOAD( "875b04.e05",  0x00000, 0x40000, CRC(4e209ac8) SHA1(09d9eaae61bfd04bf318555ccd44d7371571d86d) )
ROM_END

ROM_START( aliens2 )
	ROM_REGION( 0x38000, "maincpu", 0 ) /* code + banked roms */
	ROM_LOAD( "875_p02.e24", 0x10000, 0x08000, CRC(4edd707d) SHA1(02b39068e5fd99ecb5b35a586335b65a20fde490) )
	ROM_CONTINUE(            0x08000, 0x08000 )
	ROM_LOAD( "875_n01.c24", 0x18000, 0x20000, CRC(106cf59c) SHA1(78622adc02055d31cd587c83b23a6cde30c9bc22) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 64k for the sound CPU */
	ROM_LOAD( "875_b03.g04", 0x00000, 0x08000, CRC(1ac4d283) SHA1(2253f1f39c7edb6cef438b3d97f3af67a1f491ff) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* graphics */
	ROM_LOAD( "875b11.k13", 0x000000, 0x80000, CRC(89c5c885) SHA1(02a1581579b6ef816e04bec312a7b3ae7c7e84f8) )	/* characters (set 1) */
	ROM_LOAD( "875b07.j13", 0x080000, 0x40000, CRC(e9c56d66) SHA1(1f58949d5391aef002a6e1ee7034e57bf99cee61) )	/* characters (set 2) */
	/* second half empty */
	ROM_LOAD( "875b12.k19", 0x100000, 0x80000, CRC(ea6bdc17) SHA1(a7c22370f8adc5b479283f1ff831f493df78282f) )	/* characters (set 1) */
	ROM_LOAD( "875b08.j19", 0x180000, 0x40000, CRC(f9387966) SHA1(470ecc4a5a3edd08d5e0ab10b0c590db1968fb0a) )	/* characters (set 2) */
	/* second half empty */

	ROM_REGION( 0x200000, "gfx2", 0 ) /* graphics */
	ROM_LOAD( "875b10.k08", 0x000000, 0x80000, CRC(0b1035b1) SHA1(db04020761386e79249762cd1540208375c38c7f) )	/* sprites (set 1) */
	ROM_LOAD( "875b06.j08", 0x080000, 0x40000, CRC(081a0566) SHA1(3a4aa14178fe76a030224743c9e9cd974e08bd79) )	/* sprites (set 2) */
	/* second half empty */
	ROM_LOAD( "875b09.k02", 0x100000, 0x80000, CRC(e76b3c19) SHA1(6838e07460b3eaaeb129208ad0696c8019bd63d9) )	/* sprites (set 1) */
	ROM_LOAD( "875b05.j02", 0x180000, 0x40000, CRC(19a261f2) SHA1(b0518fad833b3e613e0201d5d9cab73dc5e78e1d) )	/* sprites (set 2) */
	/* second half empty */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "821a08.h14",  0x0000, 0x0100, CRC(7da55800) SHA1(3826f73569c8ae0431510a355bdfa082152b74a5) )	/* priority encoder (not used) */

	ROM_REGION( 0x40000, "konami", 0 ) /* samples for 007232 */
	ROM_LOAD( "875b04.e05",  0x00000, 0x40000, CRC(4e209ac8) SHA1(09d9eaae61bfd04bf318555ccd44d7371571d86d) )
ROM_END

ROM_START( aliens3 )
	ROM_REGION( 0x38000, "maincpu", 0 ) /* code + banked roms */
	ROM_LOAD( "875_w3_2.e24", 0x10000, 0x08000, CRC(f917f7b5) SHA1(ab95ad40c82f11572bbaa03d76dae35f76bacf0c) ) /* Needs correct rom label */
	ROM_CONTINUE(             0x08000, 0x08000 )
	ROM_LOAD( "875_w3_1.c24", 0x18000, 0x20000, CRC(3c0006fb) SHA1(e8730d50c358e7321dd676c74368fe44b9bbe5b2) ) /* Needs correct rom label */

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 64k for the sound CPU */
	ROM_LOAD( "875_b03.g04", 0x00000, 0x08000, CRC(1ac4d283) SHA1(2253f1f39c7edb6cef438b3d97f3af67a1f491ff) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* graphics */
	ROM_LOAD( "875b11.k13", 0x000000, 0x80000, CRC(89c5c885) SHA1(02a1581579b6ef816e04bec312a7b3ae7c7e84f8) )	/* characters (set 1) */
	ROM_LOAD( "875b07.j13", 0x080000, 0x40000, CRC(e9c56d66) SHA1(1f58949d5391aef002a6e1ee7034e57bf99cee61) )	/* characters (set 2) */
	/* second half empty */
	ROM_LOAD( "875b12.k19", 0x100000, 0x80000, CRC(ea6bdc17) SHA1(a7c22370f8adc5b479283f1ff831f493df78282f) )	/* characters (set 1) */
	ROM_LOAD( "875b08.j19", 0x180000, 0x40000, CRC(f9387966) SHA1(470ecc4a5a3edd08d5e0ab10b0c590db1968fb0a) )	/* characters (set 2) */
	/* second half empty */

	ROM_REGION( 0x200000, "gfx2", 0 ) /* graphics */
	ROM_LOAD( "875b10.k08", 0x000000, 0x80000, CRC(0b1035b1) SHA1(db04020761386e79249762cd1540208375c38c7f) )	/* sprites (set 1) */
	ROM_LOAD( "875b06.j08", 0x080000, 0x40000, CRC(081a0566) SHA1(3a4aa14178fe76a030224743c9e9cd974e08bd79) )	/* sprites (set 2) */
	/* second half empty */
	ROM_LOAD( "875b09.k02", 0x100000, 0x80000, CRC(e76b3c19) SHA1(6838e07460b3eaaeb129208ad0696c8019bd63d9) )	/* sprites (set 1) */
	ROM_LOAD( "875b05.j02", 0x180000, 0x40000, CRC(19a261f2) SHA1(b0518fad833b3e613e0201d5d9cab73dc5e78e1d) )	/* sprites (set 2) */
	/* second half empty */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "821a08.h14", 0x0000, 0x0100, CRC(7da55800) SHA1(3826f73569c8ae0431510a355bdfa082152b74a5) )	/* priority encoder (not used) */

	ROM_REGION( 0x40000, "konami", 0 ) /* samples for 007232 */
	ROM_LOAD( "875b04.e05",  0x00000, 0x40000, CRC(4e209ac8) SHA1(09d9eaae61bfd04bf318555ccd44d7371571d86d) )
ROM_END

ROM_START( aliensu )
	ROM_REGION( 0x38000, "maincpu", 0 ) /* code + banked roms */
	ROM_LOAD( "875_n02.e24", 0x10000, 0x08000, CRC(24dd612e) SHA1(35bceb3045cd0bd9d107312b371fb60dcf3f1272) )
	ROM_CONTINUE(            0x08000, 0x08000 )
	ROM_LOAD( "875_n01.c24", 0x18000, 0x20000, CRC(106cf59c) SHA1(78622adc02055d31cd587c83b23a6cde30c9bc22) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 64k for the sound CPU */
	ROM_LOAD( "875_b03.g04", 0x00000, 0x08000, CRC(1ac4d283) SHA1(2253f1f39c7edb6cef438b3d97f3af67a1f491ff) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* graphics */
	ROM_LOAD( "875b11.k13", 0x000000, 0x80000, CRC(89c5c885) SHA1(02a1581579b6ef816e04bec312a7b3ae7c7e84f8) )	/* characters (set 1) */
	ROM_LOAD( "875b07.j13", 0x080000, 0x40000, CRC(e9c56d66) SHA1(1f58949d5391aef002a6e1ee7034e57bf99cee61) )	/* characters (set 2) */
	/* second half empty */
	ROM_LOAD( "875b12.k19", 0x100000, 0x80000, CRC(ea6bdc17) SHA1(a7c22370f8adc5b479283f1ff831f493df78282f) )	/* characters (set 1) */
	ROM_LOAD( "875b08.j19", 0x180000, 0x40000, CRC(f9387966) SHA1(470ecc4a5a3edd08d5e0ab10b0c590db1968fb0a) )	/* characters (set 2) */
	/* second half empty */

	ROM_REGION( 0x200000, "gfx2", 0 ) /* graphics */
	ROM_LOAD( "875b10.k08", 0x000000, 0x80000, CRC(0b1035b1) SHA1(db04020761386e79249762cd1540208375c38c7f) )	/* sprites (set 1) */
	ROM_LOAD( "875b06.j08", 0x080000, 0x40000, CRC(081a0566) SHA1(3a4aa14178fe76a030224743c9e9cd974e08bd79) )	/* sprites (set 2) */
	/* second half empty */
	ROM_LOAD( "875b09.k02", 0x100000, 0x80000, CRC(e76b3c19) SHA1(6838e07460b3eaaeb129208ad0696c8019bd63d9) )	/* sprites (set 1) */
	ROM_LOAD( "875b05.j02", 0x180000, 0x40000, CRC(19a261f2) SHA1(b0518fad833b3e613e0201d5d9cab73dc5e78e1d) )	/* sprites (set 2) */
	/* second half empty */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "821a08.h14", 0x0000, 0x0100, CRC(7da55800) SHA1(3826f73569c8ae0431510a355bdfa082152b74a5) )	/* priority encoder (not used) */

	ROM_REGION( 0x40000, "konami", 0 ) /* samples for 007232 */
	ROM_LOAD( "875b04.e05",  0x00000, 0x40000, CRC(4e209ac8) SHA1(09d9eaae61bfd04bf318555ccd44d7371571d86d) )
ROM_END

ROM_START( aliensj )
	ROM_REGION( 0x38000, "maincpu", 0 ) /* code + banked roms */
	ROM_LOAD( "875_m02.e24",  0x10000, 0x08000, CRC(54a774e5) SHA1(b6413b2199f863cae1c6fcef766989162cd4b95e) )
	ROM_CONTINUE(             0x08000, 0x08000 )
	ROM_LOAD( "875_m01.c24",  0x18000, 0x20000, CRC(1663d3dc) SHA1(706bdf3daa3bda372d94263f3405d67a7ef8dc69) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 64k for the sound CPU */
	ROM_LOAD( "875_k03.g04", 0x00000, 0x08000, CRC(bd86264d) SHA1(345fd666daf8a29ef314b14306c1a976cb159bed) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* graphics */
	ROM_LOAD( "875b11.k13", 0x000000, 0x80000, CRC(89c5c885) SHA1(02a1581579b6ef816e04bec312a7b3ae7c7e84f8) )	/* characters (set 1) */
	ROM_LOAD( "875b07.j13", 0x080000, 0x40000, CRC(e9c56d66) SHA1(1f58949d5391aef002a6e1ee7034e57bf99cee61) )	/* characters (set 2) */
	/* second half empty */
	ROM_LOAD( "875b12.k19", 0x100000, 0x80000, CRC(ea6bdc17) SHA1(a7c22370f8adc5b479283f1ff831f493df78282f) )	/* characters (set 1) */
	ROM_LOAD( "875b08.j19", 0x180000, 0x40000, CRC(f9387966) SHA1(470ecc4a5a3edd08d5e0ab10b0c590db1968fb0a) )	/* characters (set 2) */
	/* second half empty */

	ROM_REGION( 0x200000, "gfx2", 0 ) /* graphics */
	ROM_LOAD( "875b10.k08", 0x000000, 0x80000, CRC(0b1035b1) SHA1(db04020761386e79249762cd1540208375c38c7f) )	/* sprites (set 1) */
	ROM_LOAD( "875b06.j08", 0x080000, 0x40000, CRC(081a0566) SHA1(3a4aa14178fe76a030224743c9e9cd974e08bd79) )	/* sprites (set 2) */
	/* second half empty */
	ROM_LOAD( "875b09.k02", 0x100000, 0x80000, CRC(e76b3c19) SHA1(6838e07460b3eaaeb129208ad0696c8019bd63d9) )	/* sprites (set 1) */
	ROM_LOAD( "875b05.j02", 0x180000, 0x40000, CRC(19a261f2) SHA1(b0518fad833b3e613e0201d5d9cab73dc5e78e1d) )	/* sprites (set 2) */
	/* second half empty */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "821a08.h14", 0x0000, 0x0100, CRC(7da55800) SHA1(3826f73569c8ae0431510a355bdfa082152b74a5) )	/* priority encoder (not used) */

	ROM_REGION( 0x40000, "konami", 0 ) /* samples for 007232 */
	ROM_LOAD( "875b04.e05",  0x00000, 0x40000, CRC(4e209ac8) SHA1(09d9eaae61bfd04bf318555ccd44d7371571d86d) )
ROM_END

ROM_START( aliensj2 )
	ROM_REGION( 0x38000, "maincpu", 0 ) /* code + banked roms */
	ROM_LOAD( "875_j2_2.e24", 0x10000, 0x08000, CRC(4bb84952) SHA1(ca40a7181f11d6c34c26b65f8d4a1d1df2c7fb48) ) /* Needs correct rom label */
	ROM_CONTINUE(             0x08000, 0x08000 )
	ROM_LOAD( "875_m01.c24",  0x18000, 0x20000, CRC(1663d3dc) SHA1(706bdf3daa3bda372d94263f3405d67a7ef8dc69) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 64k for the sound CPU */
	ROM_LOAD( "875_k03.g04", 0x00000, 0x08000, CRC(bd86264d) SHA1(345fd666daf8a29ef314b14306c1a976cb159bed) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* graphics */
	ROM_LOAD( "875b11.k13", 0x000000, 0x80000, CRC(89c5c885) SHA1(02a1581579b6ef816e04bec312a7b3ae7c7e84f8) )	/* characters (set 1) */
	ROM_LOAD( "875b07.j13", 0x080000, 0x40000, CRC(e9c56d66) SHA1(1f58949d5391aef002a6e1ee7034e57bf99cee61) )	/* characters (set 2) */
	/* second half empty */
	ROM_LOAD( "875b12.k19", 0x100000, 0x80000, CRC(ea6bdc17) SHA1(a7c22370f8adc5b479283f1ff831f493df78282f) )	/* characters (set 1) */
	ROM_LOAD( "875b08.j19", 0x180000, 0x40000, CRC(f9387966) SHA1(470ecc4a5a3edd08d5e0ab10b0c590db1968fb0a) )	/* characters (set 2) */
	/* second half empty */

	ROM_REGION( 0x200000, "gfx2", 0 ) /* graphics */
	ROM_LOAD( "875b10.k08", 0x000000, 0x80000, CRC(0b1035b1) SHA1(db04020761386e79249762cd1540208375c38c7f) )	/* sprites (set 1) */
	ROM_LOAD( "875b06.j08", 0x080000, 0x40000, CRC(081a0566) SHA1(3a4aa14178fe76a030224743c9e9cd974e08bd79) )	/* sprites (set 2) */
	/* second half empty */
	ROM_LOAD( "875b09.k02", 0x100000, 0x80000, CRC(e76b3c19) SHA1(6838e07460b3eaaeb129208ad0696c8019bd63d9) )	/* sprites (set 1) */
	ROM_LOAD( "875b05.j02", 0x180000, 0x40000, CRC(19a261f2) SHA1(b0518fad833b3e613e0201d5d9cab73dc5e78e1d) )	/* sprites (set 2) */
	/* second half empty */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "821a08.h14", 0x0000, 0x0100, CRC(7da55800) SHA1(3826f73569c8ae0431510a355bdfa082152b74a5) )	/* priority encoder (not used) */

	ROM_REGION( 0x40000, "konami", 0 ) /* samples for 007232 */
	ROM_LOAD( "875b04.e05",  0x00000, 0x40000, CRC(4e209ac8) SHA1(09d9eaae61bfd04bf318555ccd44d7371571d86d) )
ROM_END

ROM_START( aliensa )
	ROM_REGION( 0x38000, "maincpu", 0 ) /* code + banked roms */
	ROM_LOAD( "875_r02.e24", 0x10000, 0x08000, CRC(973e4f11) SHA1(a4f65ef4c84b1dcac591dc348ebbb96d35ef5f93) )
	ROM_CONTINUE(            0x08000, 0x08000 )
	ROM_LOAD( "875_m01.c24",  0x18000, 0x20000, CRC(1663d3dc) SHA1(706bdf3daa3bda372d94263f3405d67a7ef8dc69) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 64k for the sound CPU */
	ROM_LOAD( "875_k03.g04", 0x00000, 0x08000, CRC(bd86264d) SHA1(345fd666daf8a29ef314b14306c1a976cb159bed) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* graphics */
	ROM_LOAD( "875b11.k13", 0x000000, 0x80000, CRC(89c5c885) SHA1(02a1581579b6ef816e04bec312a7b3ae7c7e84f8) )	/* characters (set 1) */
	ROM_LOAD( "875b07.j13", 0x080000, 0x40000, CRC(e9c56d66) SHA1(1f58949d5391aef002a6e1ee7034e57bf99cee61) )	/* characters (set 2) */
	/* second half empty */
	ROM_LOAD( "875b12.k19", 0x100000, 0x80000, CRC(ea6bdc17) SHA1(a7c22370f8adc5b479283f1ff831f493df78282f) )	/* characters (set 1) */
	ROM_LOAD( "875b08.j19", 0x180000, 0x40000, CRC(f9387966) SHA1(470ecc4a5a3edd08d5e0ab10b0c590db1968fb0a) )	/* characters (set 2) */
	/* second half empty */

	ROM_REGION( 0x200000, "gfx2", 0 ) /* graphics */
	ROM_LOAD( "875b10.k08", 0x000000, 0x80000, CRC(0b1035b1) SHA1(db04020761386e79249762cd1540208375c38c7f) )	/* sprites (set 1) */
	ROM_LOAD( "875b06.j08", 0x080000, 0x40000, CRC(081a0566) SHA1(3a4aa14178fe76a030224743c9e9cd974e08bd79) )	/* sprites (set 2) */
	/* second half empty */
	ROM_LOAD( "875b09.k02", 0x100000, 0x80000, CRC(e76b3c19) SHA1(6838e07460b3eaaeb129208ad0696c8019bd63d9) )	/* sprites (set 1) */
	ROM_LOAD( "875b05.j02", 0x180000, 0x40000, CRC(19a261f2) SHA1(b0518fad833b3e613e0201d5d9cab73dc5e78e1d) )	/* sprites (set 2) */
	/* second half empty */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "821a08.h14", 0x0000, 0x0100, CRC(7da55800) SHA1(3826f73569c8ae0431510a355bdfa082152b74a5) )	/* priority encoder (not used) */

	ROM_REGION( 0x40000, "konami", 0 ) /* samples for 007232 */
	ROM_LOAD( "875b04.e05",  0x00000, 0x40000, CRC(4e209ac8) SHA1(09d9eaae61bfd04bf318555ccd44d7371571d86d) )
ROM_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

static KONAMI_SETLINES_CALLBACK( aliens_banking )
{
	UINT8 *RAM = memory_region(device->machine, "maincpu");
	int offs = 0x18000;


	if (lines & 0x10) offs -= 0x8000;

	offs += (lines & 0x0f)*0x2000;
	memory_set_bankptr(device->machine,  "bank1", &RAM[offs] );
}

static MACHINE_RESET( aliens )
{
	UINT8 *RAM = memory_region(machine, "maincpu");

	konami_configure_set_lines(cputag_get_cpu(machine, "maincpu"), aliens_banking);

	/* init the default bank */
	memory_set_bankptr(machine,  "bank1", &RAM[0x10000]);
}



static DRIVER_INIT( aliens )
{
	state_save_register_global(machine, palette_selected);
}


GAME( 1990, aliens,   0,      aliens, aliens, aliens, ROT0, "Konami", "Aliens (World set 1)", GAME_SUPPORTS_SAVE )
GAME( 1990, aliens2,  aliens, aliens, aliens, aliens, ROT0, "Konami", "Aliens (World set 2)", GAME_SUPPORTS_SAVE )
GAME( 1990, aliens3,  aliens, aliens, aliens, aliens, ROT0, "Konami", "Aliens (World set 3)", GAME_SUPPORTS_SAVE )
GAME( 1990, aliensu,  aliens, aliens, aliens, aliens, ROT0, "Konami", "Aliens (US)",          GAME_SUPPORTS_SAVE )
GAME( 1990, aliensj,  aliens, aliens, aliens, aliens, ROT0, "Konami", "Aliens (Japan set 1)", GAME_SUPPORTS_SAVE )
GAME( 1990, aliensj2, aliens, aliens, aliens, aliens, ROT0, "Konami", "Aliens (Japan set 2)", GAME_SUPPORTS_SAVE )
GAME( 1990, aliensa,  aliens, aliens, aliens, aliens, ROT0, "Konami", "Aliens (Asia)",        GAME_SUPPORTS_SAVE )
