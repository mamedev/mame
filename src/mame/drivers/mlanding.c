/* Taito Midnight Landing
  Dual 68k + 2xZ80
  no other hardware info..but it doesn't seem related to taitoair.c at all

    TODO:
    - Fix "sprite" emulation, it's probably a blitter/buffer with commands etc.;
    - Comms between the four CPUs;
    - understand how to display the "dots", my guess is that they are at 0x200000-0x203fff of the sub cpu (this might need a side-by-side);
    - Fix "sound cpu error" msg;
    - Inputs, particularly needed for a game like this one;
*/

#include "driver.h"
#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "deprecat.h"
#include "audio/taitosnd.h"
#include "sound/2151intf.h"
#include "sound/msm5205.h"


static UINT16 * ml_tileram;
static UINT16 * ml_spriteram;
static bitmap_t *ml_bitmap[8];
#define ML_CHARS 0x2000
static int status_bit;
static int adpcm_pos;
static int adpcm_data;



static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 16, 24, 0, 8 },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static WRITE16_HANDLER(ml_tileram_w)
{
	COMBINE_DATA(&ml_tileram[offset]);
	gfx_element_mark_dirty(space->machine->gfx[0], offset>>4);
}

static READ16_HANDLER(ml_tileram_r)
{
	return ml_tileram[offset];
}



static READ16_HANDLER( io1_r ) //240006
{
	/*
    fedcba9876543210
                   x  - mecha driver status
                  x   - ???
                 x    - test 2
                x     - ???
    x                 - video status
        other bits = language(german, japan, english), video test
    */

	int retval= (status_bit|1|0x07fff);
	status_bit=0x8000;
	return retval;
}

static WRITE16_HANDLER(ml_subreset_w)
{
	//wrong
	if(cpu_get_pc(space->cpu) == 0x822)
		cputag_set_input_line(space->machine, "sub", INPUT_LINE_RESET, PULSE_LINE);
}

static WRITE8_DEVICE_HANDLER( sound_bankswitch_w )
{
	data=0;
	memory_set_bankptr(device->machine,  1, memory_region(device->machine, "audiocpu") + ((data) & 0x03) * 0x4000 + 0x10000 );
}

static void ml_msm5205_vck(const device_config *device)
{
	if (adpcm_data != -1)
	{
		msm5205_data_w(device, adpcm_data & 0x0f);
		adpcm_data = -1;
	}
	else
	{
		adpcm_data = memory_region(device->machine, "adpcm")[adpcm_pos];
		adpcm_pos = (adpcm_pos + 1) & 0xffff;
		msm5205_data_w(device, adpcm_data >> 4);
	}
}

static WRITE8_HANDLER( ml_msm5205_address_w )
{
	adpcm_pos = (adpcm_pos & 0x00ff) | (data << 8);
}

static WRITE8_DEVICE_HANDLER( ml_msm5205_start_w )
{
	msm5205_reset_w(device, 0);
}

static WRITE8_DEVICE_HANDLER( ml_msm5205_stop_w )
{
	msm5205_reset_w(device, 1);
	adpcm_pos &= 0xff00;
}

/* TODO: check this */
static WRITE16_HANDLER( sound_reset_w )
{
	cputag_set_input_line(space->machine, "audiocpu", INPUT_LINE_RESET, PULSE_LINE);
}

static ADDRESS_MAP_START( mlanding_mem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x05ffff) AM_ROM
	AM_RANGE(0x080000, 0x08ffff) AM_RAM

	AM_RANGE(0x100000, 0x17ffff) AM_RAM
	AM_RANGE(0x180000, 0x1bffff) AM_READWRITE(ml_tileram_r, ml_tileram_w) AM_BASE(&ml_tileram)
	AM_RANGE(0x1c0000, 0x1c1fff) AM_RAM AM_BASE(&ml_spriteram)
	AM_RANGE(0x1c2000, 0x1c3fff) AM_RAM
	AM_RANGE(0x1c4000, 0x1cffff) AM_RAM AM_SHARE(1)

	AM_RANGE(0x1d0000, 0x1d0001) AM_WRITENOP
	AM_RANGE(0x1d0002, 0x1d0003) AM_WRITE(sound_reset_w) //sound reset ??

	AM_RANGE(0x2d0000, 0x2d0001) AM_READNOP AM_WRITE8(taitosound_port_w, 0x00ff)
//  AM_RANGE(0x2d0002, 0x2d0003) AM_READ8(taitosound_comm_r, 0xff00) AM_WRITE8(taitosound_comm_w, 0x00ff)
	AM_RANGE(0x2d0002, 0x2d0003) AM_READ8(taitosound_comm_r, 0x00ff) AM_WRITE8(taitosound_comm_w, 0x00ff)

	AM_RANGE(0x200000, 0x20ffff) AM_RAM_WRITE(paletteram16_xBBBBBGGGGGRRRRR_word_w) AM_BASE(&paletteram16)//AM_SHARE(2)
	AM_RANGE(0x280000, 0x2807ff) AM_RAM // is it shared with mecha ? tested around $940

	AM_RANGE(0x290000, 0x290001) AM_READ_PORT("IN1")
	AM_RANGE(0x290002, 0x290003) AM_READ_PORT("IN0")

	AM_RANGE(0x240004, 0x240005) AM_NOP //watchdog ??
	AM_RANGE(0x240006, 0x240007) AM_READ(io1_r) // vblank ?
	AM_RANGE(0x2a0000, 0x2a0001) AM_WRITE(ml_subreset_w)

	/* are we sure that these are for an analog stick? */
	AM_RANGE(0x2b0000, 0x2b0001) AM_READ_PORT("STICKX")		//-40 .. 40 analog controls ?
	AM_RANGE(0x2b0004, 0x2b0005) AM_READ_PORT("STICKY")		//-40 .. 40 analog controls ?
	AM_RANGE(0x2b0006, 0x2b0007) AM_READNOP // tested in service mode, dips?
	AM_RANGE(0x2c0000, 0x2c0001) AM_READ_PORT("STICKZ")		//-60 .. 60 analog controls ?
	AM_RANGE(0x2c0002, 0x2c0003) AM_READ_PORT("IN3")
	AM_RANGE(0x2b0002, 0x2b0003) AM_READ_PORT("IN2")		// IN2/IN3 could be switched
ADDRESS_MAP_END


/* Sub CPU Map */

static ADDRESS_MAP_START( mlanding_sub_mem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x01ffff) AM_ROM
	AM_RANGE(0x040000, 0x043fff) AM_RAM
	AM_RANGE(0x050000, 0x0503ff) AM_RAM // palette?
	AM_RANGE(0x1c0000, 0x1c1fff) AM_RAM
	AM_RANGE(0x1c4000, 0x1cffff) AM_RAM AM_SHARE(1)
	AM_RANGE(0x200000, 0x203fff) AM_RAM //AM_SHARE(2)
ADDRESS_MAP_END

static ADDRESS_MAP_START( mlanding_z80_mem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK(1)
	AM_RANGE(0x8000, 0x8fff) AM_RAM
	AM_RANGE(0x9000, 0x9001) AM_MIRROR(0x00fe) AM_DEVREADWRITE("ym", ym2151_r, ym2151_w)
	AM_RANGE(0xa000, 0xa000) AM_WRITE(taitosound_slave_port_w)
	AM_RANGE(0xa001, 0xa001) AM_READ(taitosound_slave_comm_r) AM_WRITE(taitosound_slave_comm_w)

	AM_RANGE(0xb000, 0xb000) AM_WRITE(ml_msm5205_address_w) //guess
	AM_RANGE(0xc000, 0xc000) AM_DEVWRITE("msm", ml_msm5205_start_w)
	AM_RANGE(0xd000, 0xd000) AM_DEVWRITE("msm", ml_msm5205_stop_w)

	AM_RANGE(0xf400, 0xf400) AM_WRITENOP
	AM_RANGE(0xf600, 0xf600) AM_WRITENOP
ADDRESS_MAP_END

//mecha driver ?
static ADDRESS_MAP_START( mlanding_z80_sub_mem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM //AM_SHARE(2)
	AM_RANGE(0x8800, 0x8fff) AM_RAM

	AM_RANGE(0x9000, 0x9001) AM_RAM
	AM_RANGE(0x9800, 0x9803) AM_RAM

ADDRESS_MAP_END

static VIDEO_START(mlanding)
{
	int i;

	gfx_element_set_source(machine->gfx[0], (UINT8 *) ml_tileram);

	for	(i=0;i<8;i++)
		ml_bitmap[i] = video_screen_auto_bitmap_alloc(machine->primary_screen);
}

static VIDEO_UPDATE(mlanding)
{
	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));

	{
		int i,dx,dy,j,k,num;
		for(i=0;i<0x1000;i+=4)
		{
			int x,y,code,color;
			code=ml_spriteram[i];
			x=ml_spriteram[i+1];
			y=ml_spriteram[i+2];
			color=ml_spriteram[i+3];

			dx=x>>11;
			dy=y>>11;
			dx&=0x1f;
			dy&=0x1f;
			dx++;
			dy++;

			x&=0x1ff;
			y&=0x1ff;

			num=code>>14;
			code&=0x1fff;


			for(j=0;j<dx;j++)
			{
				for(k=0;k<dy;k++)
				{
				//test
					if(code)
					{
						drawgfx_opaque(ml_bitmap[num],cliprect,screen->machine->gfx[0],
							code++,
							color,
							0,0,
							x+j*8,y+k*8);
					}
					else
					{
						 int xx,yy;
						 for(yy=0;yy<8;yy++)
						 	for(xx=0;xx<8;xx++)
							{
								*BITMAP_ADDR16(ml_bitmap[num], y+yy+k*8, x+xx+j*8) = 0; //test only .. ugly
							}
					}
				}
			}
		}
	}

	{
		int i;
		for(i=0;i<7;i++)
		{
			copybitmap_trans(bitmap,ml_bitmap[i], 0, 0, 0, 0, cliprect, 0);
		}
	}
	status_bit=0; //FIXME: emulation variable in VIDEO_UPDATE()
	{
	/*  int i;
        for(i=0;i<0x8000;i++)
        {
            *BITMAP_ADDR16(bitmap, 156+( ml_unk[i]>>8), ml_unk[i]&0xff) = 0x207;
        }
        */
	}
	return 0;
}


static INPUT_PORTS_START( mlanding )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Door") PORT_TOGGLE
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, "SYSTEM" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x01, "SYSTEM" ) //high bits of counter 2
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Slot Down")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Slot Up")
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN3")
	PORT_DIPNAME( 0x01, 0x01, "SYSTEM" ) //high bits of counter 3
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("STICKX")		/* Stick 1 (3) */
	PORT_BIT( 0xffff, 0x0000, IPT_AD_STICK_X ) PORT_MINMAX(0xffd8,0x28) PORT_SENSITIVITY(30) PORT_KEYDELTA(1) PORT_PLAYER(1)

	PORT_START("STICKY")	/* Stick 2 (4) */
	PORT_BIT( 0xffff, 0x0000, IPT_AD_STICK_Y ) PORT_MINMAX(0xffd8,0x28) PORT_SENSITIVITY(30) PORT_KEYDELTA(1) PORT_PLAYER(1)

	PORT_START("STICKZ")	/* Stick 3 (5) */
	PORT_BIT( 0xffff, 0x0000, IPT_AD_STICK_Z ) PORT_MINMAX(0xffc4,0x3c) PORT_SENSITIVITY(30) PORT_KEYDELTA(1) PORT_PLAYER(1)
INPUT_PORTS_END

static void irq_handler(const device_config *device, int irq)
{
	cputag_set_input_line(device->machine, "audiocpu", 0, irq ? ASSERT_LINE : CLEAR_LINE);
}

static GFXDECODE_START( mlanding )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout, 0, 16*16 )
GFXDECODE_END

static const msm5205_interface msm5205_config =
{
	ml_msm5205_vck,	/* VCK function */
	MSM5205_S48_4B		/* 8 kHz */
};

static const ym2151_interface ym2151_config =
{
	irq_handler,
	sound_bankswitch_w
};

static MACHINE_RESET( mlanding )
{
	status_bit = 0;
	adpcm_pos = 0;
	adpcm_data = -1;
}

static MACHINE_DRIVER_START( mlanding )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 12000000 )		/* 12 MHz ??? (guess) */
	MDRV_CPU_PROGRAM_MAP(mlanding_mem)
	MDRV_CPU_VBLANK_INT("screen", irq6_line_hold)

	MDRV_CPU_ADD("audiocpu", Z80, 4000000 )		/* 4 MHz ??? (guess) */
	MDRV_CPU_PROGRAM_MAP(mlanding_z80_mem)

	MDRV_CPU_ADD("sub", M68000, 12000000 )		/* 12 MHz ??? (guess) */
	MDRV_CPU_PROGRAM_MAP(mlanding_sub_mem)
	MDRV_CPU_VBLANK_INT_HACK(irq6_line_hold,7)

	MDRV_CPU_ADD("z80sub", Z80, 4000000 )		/* 4 MHz ??? (guess) */
	MDRV_CPU_PROGRAM_MAP(mlanding_z80_sub_mem)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	MDRV_QUANTUM_TIME(HZ(600))

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_SIZE(512, 512)
	MDRV_SCREEN_VISIBLE_AREA(0, 511, 14*8, 511)

	MDRV_GFXDECODE(mlanding)
	MDRV_PALETTE_LENGTH(512*16)

	MDRV_VIDEO_START(mlanding)
	MDRV_VIDEO_UPDATE(mlanding)

	MDRV_MACHINE_RESET(mlanding)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ym", YM2151, 4000000)
	MDRV_SOUND_CONFIG(ym2151_config)
	MDRV_SOUND_ROUTE(0, "mono", 0.50)
	MDRV_SOUND_ROUTE(1, "mono", 0.50)

	MDRV_SOUND_ADD("msm", MSM5205, 384000)
	MDRV_SOUND_CONFIG(msm5205_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END

ROM_START( mlanding )
	ROM_REGION( 0x60000, "maincpu", 0 )	/* 68000 */
	ROM_LOAD16_BYTE( "ml_b0929.epr", 0x00000, 0x10000, CRC(ab3f38f3) SHA1(4357112ca11a8e7bfe08ba99ac3bddac046c230a))
	ROM_LOAD16_BYTE( "ml_b0928.epr", 0x00001, 0x10000, CRC(21e7a8f6) SHA1(860d3861d4375866cd27d426d546ddb2894a6629) )
	ROM_LOAD16_BYTE( "ml_b0927.epr", 0x20000, 0x10000, CRC(b02f1805) SHA1(b8050f955c7070dc9b962db329b5b0ee8b2acb70) )
	ROM_LOAD16_BYTE( "ml_b0926.epr", 0x20001, 0x10000, CRC(d57ff428) SHA1(8ff1ab666b06fb873f1ba9b25edf4cd49b9861a1) )
	ROM_LOAD16_BYTE( "ml_b0925.epr", 0x40000, 0x10000, CRC(ff59f049) SHA1(aba490a28aba03728415f34d321fd599c31a5fde) )
	ROM_LOAD16_BYTE( "ml_b0924.epr", 0x40001, 0x10000, CRC(9bc3e1b0) SHA1(6d86804327df11a513a0f06dceb57b83b34ac007) )

	ROM_REGION( 0x20000, "audiocpu", 0 )	/* z80 */
	ROM_LOAD( "ml_b0935.epr", 0x00000, 0x4000, CRC(b85915c5) SHA1(656e97035ae304f84e90758d0dd6f0616c40f1db) )
	ROM_CONTINUE(             0x10000, 0x04000 )	/* banked stuff */
	ROM_LOAD( "ml_b0936.epr", 0x14000, 0x02000, CRC(51fd3a77) SHA1(1fcbadf1877e25848a1d1017322751560a4823c0) )

	ROM_REGION( 0x40000, "gfx1", ROMREGION_ERASE00 )

	ROM_REGION( 0x20000, "sub", 0 )	/* 68000 */
	ROM_LOAD16_BYTE( "ml_b0923.epr", 0x00000, 0x10000, CRC(81b2c871) SHA1(a085bc528c63834079469db6ae263a5b9b984a7c) )
	ROM_LOAD16_BYTE( "ml_b0922.epr", 0x00001, 0x10000, CRC(36923b42) SHA1(c31d7c45a563cfc4533379f69f32889c79562534) )

	ROM_REGION( 0x10000, "z80sub", 0 )	/* z80 */
	ROM_LOAD( "ml_b0937.epr", 0x00000, 0x08000, CRC(4bdf15ed) SHA1(b960208e63cede116925e064279a6cf107aef81c) )

	ROM_REGION( 0x50000, "adpcm", 0 )
	ROM_LOAD( "ml_b0930.epr", 0x00000, 0x10000, CRC(214a30e2) SHA1(3dcc3a89ed52e4dbf232d2a92a3e64975b46c2dd) )
	ROM_LOAD( "ml_b0931.epr", 0x10000, 0x10000, CRC(9c4a82bf) SHA1(daeac620c636013a36595ce9f37e84e807f88977) )
	ROM_LOAD( "ml_b0932.epr", 0x20000, 0x10000, CRC(4721dc59) SHA1(faad75d577344e9ba495059040a2cf0647567426) )
	ROM_LOAD( "ml_b0933.epr", 0x30000, 0x10000, CRC(f5cac954) SHA1(71abdc545e0196ad4d357af22dd6312d10a1323f) )
	ROM_LOAD( "ml_b0934.epr", 0x40000, 0x10000, CRC(0899666f) SHA1(032e3ddd4caa48f82592570616e16c084de91f3e) )
ROM_END

static DRIVER_INIT(mlanding)
{
	UINT8 *rom = memory_region(machine, "sub");
	rom[0x88b]=0x4e;
	rom[0x88a]=0x71;
}

GAME( 1990, mlanding, 0,        mlanding,   mlanding, mlanding,        ROT0,    "Taito Corporation", "Midnight Landing", GAME_NOT_WORKING|GAME_NO_SOUND )
