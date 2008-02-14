/****************************************************************************

    Super Chase                         (c) 1992 Taito

    Driver by Bryan McPhail & David Graves.

    Board Info:

        CPU board:
        68000
        68020
        TC0570SPC (Taito custom)
        TC0470LIN (Taito custom)
        TC0510NIO (Taito custom)
        TC0480SCP (Taito custom)
        TC0650FDA (Taito custom)
        ADC0809CCN

        X2=26.686MHz
        X1=40MHz
        X3=32MHz

        Sound board:
        68000
        68681
        MB8421 (x2)
        MB87078
        Ensoniq 5510
        Ensoniq 5505

    (Acknowledgments and thanks to Richard Bush and the Raine team
    for their preliminary Super Chase driver.)

***************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "cpu/m68000/m68000.h"
#include "video/taitoic.h"
#include "audio/taitosnd.h"
#include "machine/eeprom.h"
#include "sound/es5506.h"
#include "includes/taito_f3.h"
#include "audio/taito_en.h"

VIDEO_START( superchs );
VIDEO_UPDATE( superchs );

static UINT16 coin_word;
static UINT32 *superchs_ram;
static UINT32 *shared_ram;

static int steer=0;

/*********************************************************************/

static READ16_HANDLER( shared_ram_r )
{
	if ((offset&1)==0) return (shared_ram[offset/2]&0xffff0000)>>16;
	return (shared_ram[offset/2]&0x0000ffff);
}

static WRITE16_HANDLER( shared_ram_w )
{
	if ((offset&1)==0) {
		if (ACCESSING_MSB)
			shared_ram[offset/2]=(shared_ram[offset/2]&0x00ffffff)|((data&0xff00)<<16);
		if (ACCESSING_LSB)
			shared_ram[offset/2]=(shared_ram[offset/2]&0xff00ffff)|((data&0x00ff)<<16);
	} else {
		if (ACCESSING_MSB)
			shared_ram[offset/2]=(shared_ram[offset/2]&0xffff00ff)|((data&0xff00)<< 0);
		if (ACCESSING_LSB)
			shared_ram[offset/2]=(shared_ram[offset/2]&0xffffff00)|((data&0x00ff)<< 0);
	}
}

static WRITE32_HANDLER( cpua_ctrl_w )
{
	/*
    CPUA writes 0x00, 22, 72, f2 in that order.
    f2 seems to be the standard in-game value.
    ..x...x.
    .xxx..x.
    xxxx..x.
    is there an irq enable in the top nibble?
    */

	if (ACCESSING_MSB)
	{
		cpunum_set_input_line(Machine, 2, INPUT_LINE_RESET, (data &0x200) ? CLEAR_LINE : ASSERT_LINE);
		if (data&0x8000) cpunum_set_input_line(Machine, 0,3,HOLD_LINE); /* Guess */
	}

	if (ACCESSING_LSB32)
	{
		/* Lamp control bits of some sort in the lsb */
	}
}

static WRITE32_HANDLER( superchs_palette_w )
{
	int a,r,g,b;
	COMBINE_DATA(&paletteram32[offset]);

	a = paletteram32[offset];
	r = (a &0xff0000) >> 16;
	g = (a &0xff00) >> 8;
	b = (a &0xff);

	palette_set_color(Machine,offset,MAKE_RGB(r,g,b));
}

static READ32_HANDLER( superchs_input_r )
{
	switch (offset)
	{
		case 0x00:
			return (input_port_0_word_r(0,0) << 16) | input_port_1_word_r(0,0) |
				  (EEPROM_read_bit() << 7);

		case 0x01:
			return coin_word<<16;
 	}

	return 0xffffffff;
}

static WRITE32_HANDLER( superchs_input_w )
{

	#if 0
	{
	char t[64];
	static UINT32 mem[2];
	COMBINE_DATA(&mem[offset]);
	sprintf(t,"%08x %08x",mem[0],mem[1]);
	//popmessage(t);
	}
	#endif

	switch (offset)
	{
		case 0x00:
		{
			if (ACCESSING_MSB32)	/* $300000 is watchdog */
			{
				watchdog_reset_w(0,data >> 24);
			}

			if (ACCESSING_LSB32)
			{
				EEPROM_set_clock_line((data & 0x20) ? ASSERT_LINE : CLEAR_LINE);
				EEPROM_write_bit(data & 0x40);
				EEPROM_set_cs_line((data & 0x10) ? CLEAR_LINE : ASSERT_LINE);
				return;
			}

			return;
		}

		/* there are 'vibration' control bits somewhere! */

		case 0x01:
		{
			if (ACCESSING_MSB32)
			{
				coin_lockout_w(0,~data & 0x01000000);
				coin_lockout_w(1,~data & 0x02000000);
				coin_counter_w(0, data & 0x04000000);
				coin_counter_w(1, data & 0x08000000);
				coin_word=(data >> 16) &0xffff;
			}
		}
	}
}

static READ32_HANDLER( superchs_stick_r )
{
	int fake = input_port_6_word_r(0,0);
	int accel;

	if (!(fake &0x10))	/* Analogue steer (the real control method) */
	{
		steer = input_port_2_word_r(0,0);
	}
	else	/* Digital steer, with smoothing - speed depends on how often stick_r is called */
	{
		int delta;
		int goal = 0x80;
		if (fake &0x04) goal = 0xff;		/* pressing left */
		if (fake &0x08) goal = 0x0;		/* pressing right */

		if (steer!=goal)
		{
			delta = goal - steer;
			if (steer < goal)
			{
				if (delta >2) delta = 2;
			}
			else
			{
				if (delta < (-2)) delta = -2;
			}
			steer += delta;
		}
	}

	/* Accelerator is an analogue input but the game treats it as digital (on/off) */
	if (input_port_6_word_r(0,0) & 0x1)	/* pressing B1 */
		accel = 0x0;
	else
		accel = 0xff;

	/* Todo: Verify brake - and figure out other input */
	return (steer << 24) | (accel << 16) | (input_port_4_word_r(0,0) << 8) | input_port_5_word_r(0,0);
}

static WRITE32_HANDLER( superchs_stick_w )
{
	/* This is guess work - the interrupts are in groups of 4, with each writing to a
        different byte in this long word before the RTE.  I assume all but the last
        (top) byte cause an IRQ with the final one being an ACK.  (Total guess but it works). */
	if (mem_mask!=0x00ffffff)
		cpunum_set_input_line(Machine, 0,3,HOLD_LINE);
}

/***********************************************************
             MEMORY STRUCTURES
***********************************************************/

static ADDRESS_MAP_START( superchs_readmem, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x000000, 0x0fffff) AM_READ(MRA32_ROM)
	AM_RANGE(0x100000, 0x11ffff) AM_READ(MRA32_RAM)	/* main CPUA ram */
	AM_RANGE(0x140000, 0x141fff) AM_READ(MRA32_RAM)	/* Sprite ram */
	AM_RANGE(0x180000, 0x18ffff) AM_READ(TC0480SCP_long_r)
	AM_RANGE(0x1b0000, 0x1b002f) AM_READ(TC0480SCP_ctrl_long_r)
	AM_RANGE(0x200000, 0x20ffff) AM_READ(MRA32_RAM)	/* Shared ram */
	AM_RANGE(0x280000, 0x287fff) AM_READ(MRA32_RAM)	/* Palette ram */
	AM_RANGE(0x2c0000, 0x2c07ff) AM_READ(MRA32_RAM)	/* Sound shared ram */
	AM_RANGE(0x300000, 0x300007) AM_READ(superchs_input_r)
	AM_RANGE(0x340000, 0x340003) AM_READ(superchs_stick_r)	/* stick coord read */
ADDRESS_MAP_END

static ADDRESS_MAP_START( superchs_writemem, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x000000, 0x0fffff) AM_WRITE(MWA32_ROM)
	AM_RANGE(0x100000, 0x11ffff) AM_WRITE(MWA32_RAM) AM_BASE(&superchs_ram)
	AM_RANGE(0x140000, 0x141fff) AM_WRITE(MWA32_RAM) AM_BASE(&spriteram32) AM_SIZE(&spriteram_size)
	AM_RANGE(0x180000, 0x18ffff) AM_WRITE(TC0480SCP_long_w)
	AM_RANGE(0x1b0000, 0x1b002f) AM_WRITE(TC0480SCP_ctrl_long_w)
	AM_RANGE(0x200000, 0x20ffff) AM_WRITE(MWA32_RAM) AM_BASE(&shared_ram)
	AM_RANGE(0x240000, 0x240003) AM_WRITE(cpua_ctrl_w)
	AM_RANGE(0x280000, 0x287fff) AM_WRITE(superchs_palette_w) AM_BASE(&paletteram32)
	AM_RANGE(0x2c0000, 0x2c07ff) AM_WRITE(MWA32_RAM) AM_BASE(&f3_shared_ram)
	AM_RANGE(0x300000, 0x300007) AM_WRITE(superchs_input_w)	/* eerom etc. */
	AM_RANGE(0x340000, 0x340003) AM_WRITE(superchs_stick_w)	/* stick int request */
ADDRESS_MAP_END

static ADDRESS_MAP_START( superchs_cpub_readmem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_READ(MRA16_ROM)
	AM_RANGE(0x200000, 0x20ffff) AM_READ(MRA16_RAM)	/* local ram */
	AM_RANGE(0x800000, 0x80ffff) AM_READ(shared_ram_r)
	AM_RANGE(0xa00000, 0xa001ff) AM_READ(MRA16_RAM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( superchs_cpub_writemem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_WRITE(MWA16_ROM)
	AM_RANGE(0x200000, 0x20ffff) AM_WRITE(MWA16_RAM)
	AM_RANGE(0x600000, 0x60ffff) AM_WRITE(TC0480SCP_word_w) /* Only written upon errors */
	AM_RANGE(0x800000, 0x80ffff) AM_WRITE(shared_ram_w)
	AM_RANGE(0xa00000, 0xa001ff) AM_WRITE(MWA16_RAM)	/* Extra road control?? */
ADDRESS_MAP_END

/***********************************************************/

static INPUT_PORTS_START( superchs )
	PORT_START      /* IN0 */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER(1)	/* Freeze input */
	PORT_SERVICE_NO_TOGGLE( 0x0010, IP_ACTIVE_LOW )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW,  IPT_SERVICE1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START      /* IN1 */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_SPECIAL )	/* reserved for EEROM */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW,  IPT_BUTTON5 ) PORT_PLAYER(1)	/* seat center (cockpit only) */
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT(0x1000, IP_ACTIVE_LOW,  IPT_BUTTON3 ) PORT_NAME("Nitro")
	PORT_BIT(0x2000, IP_ACTIVE_LOW,  IPT_BUTTON4 ) PORT_NAME("Gear Shift")
	PORT_BIT(0x4000, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_NAME("Brake")
	PORT_BIT( 0x8000, IP_ACTIVE_LOW,  IPT_START1 )

	PORT_START	/* IN 2, steering wheel */
	PORT_BIT( 0xff, 0x7f, IPT_AD_STICK_X ) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_REVERSE PORT_PLAYER(1)

	PORT_START	/* IN 3, accel [effectively also brake for the upright] */
	PORT_BIT( 0xff, 0x00, IPT_AD_STICK_Y ) PORT_SENSITIVITY(20) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START	/* IN 4, sound volume */
	PORT_BIT( 0xff, 0x00, IPT_AD_STICK_X ) PORT_SENSITIVITY(20) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(2)

	PORT_START	/* IN 5, unknown */
	PORT_BIT( 0xff, 0x00, IPT_AD_STICK_Y ) PORT_SENSITIVITY(20) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START	/* IN 6, inputs and DSW all fake */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(1)
	PORT_DIPNAME( 0x10, 0x00, "Steering type" )
	PORT_DIPSETTING(    0x10, "Digital" )
	PORT_DIPSETTING(    0x00, "Analogue" )
INPUT_PORTS_END

/***********************************************************
                GFX DECODING
**********************************************************/

static const gfx_layout tile16x16_layout =
{
	16,16,	/* 16*16 sprites */
	RGN_FRAC(1,1),
	4,	/* 4 bits per pixel */
	{ 0, 8, 16, 24 },
	{ 32, 33, 34, 35, 36, 37, 38, 39, 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*64, 1*64,  2*64,  3*64,  4*64,  5*64,  6*64,  7*64,
	  8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	64*16	/* every sprite takes 128 consecutive bytes */
};

static const gfx_layout charlayout =
{
	16,16,    /* 16*16 characters */
	RGN_FRAC(1,1),
	4,        /* 4 bits per pixel */
	{ 0, 1, 2, 3 },
	{ 1*4, 0*4, 5*4, 4*4, 3*4, 2*4, 7*4, 6*4, 9*4, 8*4, 13*4, 12*4, 11*4, 10*4, 15*4, 14*4 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64, 8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	128*8     /* every sprite takes 128 consecutive bytes */
};

static GFXDECODE_START( superchs )
	GFXDECODE_ENTRY( REGION_GFX2, 0x0, tile16x16_layout,  0, 512 )
	GFXDECODE_ENTRY( REGION_GFX1, 0x0, charlayout,        0, 512 )
GFXDECODE_END


/***********************************************************
                 MACHINE DRIVERS
***********************************************************/

static MACHINE_RESET( superchs )
{
	taito_f3_soundsystem_reset();

	f3_68681_reset();
}

static const struct EEPROM_interface superchs_eeprom_interface =
{
	6,				/* address bits */
	16,				/* data bits */
	"0110",			/* read command */
	"0101",			/* write command */
	"0111",			/* erase command */
	"0100000000",	/* unlock command */
	"0100110000",	/* lock command */
};

static const UINT8 default_eeprom[128]={
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x53,0x00,0x2e,0x00,0x43,0x00,0x00,
	0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0xff,0xff,0xff,0xff,0x00,0x01,
	0x00,0x01,0x00,0x01,0x00,0x00,0x00,0x01,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x80,0xff,0xff,0xff,0xff,
	0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff
};

static NVRAM_HANDLER( superchs )
{
	if (read_or_write)
		EEPROM_save(file);
	else
	{
		EEPROM_init(&superchs_eeprom_interface);

		if (file)
			EEPROM_load(file);
		else
			EEPROM_set_data(default_eeprom,128);  /* Default the wheel setup values */
	}
}

static MACHINE_DRIVER_START( superchs )

	/* basic machine hardware */
	MDRV_CPU_ADD(M68EC020, 16000000)	/* 16 MHz */
	MDRV_CPU_PROGRAM_MAP(superchs_readmem,superchs_writemem)
	MDRV_CPU_VBLANK_INT(irq2_line_hold,1)/* VBL */

	TAITO_F3_SOUND_SYSTEM_CPU(16000000)


	MDRV_CPU_ADD(M68000, 16000000)	/* 16 MHz */
	MDRV_CPU_PROGRAM_MAP(superchs_cpub_readmem,superchs_cpub_writemem)
	MDRV_CPU_VBLANK_INT(irq4_line_hold,1)/* VBL */

	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(DEFAULT_60HZ_VBLANK_DURATION)
	MDRV_INTERLEAVE(8)	/* CPU slices - Need to interleave Cpu's 1 & 3 */

	MDRV_MACHINE_RESET(superchs)
	MDRV_NVRAM_HANDLER(superchs)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(40*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0, 40*8-1, 2*8, 32*8-1)
	MDRV_GFXDECODE(superchs)
	MDRV_PALETTE_LENGTH(8192)

	MDRV_VIDEO_START(superchs)
	MDRV_VIDEO_UPDATE(superchs)

	/* sound hardware */
	TAITO_F3_SOUND_SYSTEM_ES5505(13343000)
MACHINE_DRIVER_END

/***************************************************************************/

ROM_START( superchs )
	ROM_REGION( 0x100000, REGION_CPU1, 0 )	/* 1024K for 68020 code (CPU A) */
	ROM_LOAD32_BYTE( "d46-35.27", 0x00000, 0x40000, CRC(1575c9a7) SHA1(e3441d6018ed3315c62c5e5c4534d8712b025ae2) )
	ROM_LOAD32_BYTE( "d46-34.25", 0x00001, 0x40000, CRC(c72a4d2b) SHA1(6ef64de15e52007406ce3255071a1f856e0e8b49) )
	ROM_LOAD32_BYTE( "d46-33.23", 0x00002, 0x40000, CRC(3094bcd0) SHA1(b6779b81a3ebec440a9359868dc43fc3a631ee11) )
	ROM_LOAD32_BYTE( "d46-31.21", 0x00003, 0x40000, CRC(38b983a3) SHA1(c4859cecc2f3506b7090c462cecd3e4eaabe85aa) )

	ROM_REGION( 0x140000, REGION_CPU2, 0 )	/* Sound cpu */
	ROM_LOAD16_BYTE( "d46-37.8up", 0x100000, 0x20000, CRC(60b51b91) SHA1(0d0b017808e0a3bdabe8ef5a726bbe16428db06b) )
	ROM_LOAD16_BYTE( "d46-36.7lo", 0x100001, 0x20000, CRC(8f7aa276) SHA1(b3e330e33099d3cbf4cdc43063119b041e9eea3a) )

	ROM_REGION( 0x40000, REGION_CPU3, 0 )	/* 256K for 68000 code (CPU B) */
	ROM_LOAD16_BYTE( "d46-24.127", 0x00000, 0x20000, CRC(a006baa1) SHA1(e691ddab6cb79444bd6c3fc870e0dff3051d8cf9) )
	ROM_LOAD16_BYTE( "d46-23.112", 0x00001, 0x20000, CRC(9a69dbd0) SHA1(13eca492f1db834c599656750864e7003514f3d4) )

	ROM_REGION( 0x200000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD16_BYTE( "d46-05.87", 0x00000, 0x100000, CRC(150d0e4c) SHA1(9240b32900be733b8f44868ed5d64f5f1aaadb47) )	/* SCR 16x16 tiles */
	ROM_LOAD16_BYTE( "d46-06.88", 0x00001, 0x100000, CRC(321308be) SHA1(17e724cce39b1331650c1f08d693d057dcd43a3f) )

	ROM_REGION( 0x800000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD32_BYTE( "d46-01.64", 0x000003, 0x200000, CRC(5c2ae92d) SHA1(bee2caed4729a27fa0569d952d6d12170c2aa2a8) )	/* OBJ 16x16 tiles: each rom has 1 bitplane */
	ROM_LOAD32_BYTE( "d46-02.65", 0x000002, 0x200000, CRC(a83ca82e) SHA1(03759be87a8d62c0044e8a44e90c47308e32d3e5) )
	ROM_LOAD32_BYTE( "d46-03.66", 0x000001, 0x200000, CRC(e0e9cbfd) SHA1(b7deb2c58320af9d1b4273ad2758ce927d2e279c) )
	ROM_LOAD32_BYTE( "d46-04.67", 0x000000, 0x200000, CRC(832769a9) SHA1(136ead19edeee90b5be91a6e2f434193dc670fd8) )

	ROM_REGION16_LE( 0x80000, REGION_USER1, 0 )
	ROM_LOAD16_WORD( "d46-07.34", 0x00000, 0x80000, CRC(c3b8b093) SHA1(f34364248ca7fdaaa1a0f8f6f795f9b4bc935fb9) )	/* STY, used to create big sprites on the fly */

	ROM_REGION16_BE( 0x1000000, REGION_SOUND1 , ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "d46-10.2", 0xc00000, 0x200000, CRC(306256be) SHA1(e6e5d4a4c0b98470f2aff2e94624dd19af73ec5d) )
	ROM_LOAD16_BYTE( "d46-12.4", 0x000000, 0x200000, CRC(a24a53a8) SHA1(5d5fb87a94ceabda89360064d7d9b6d23c4c606b) )
	ROM_RELOAD     (             0x400000, 0x200000 )
	ROM_LOAD16_BYTE( "d46-11.5", 0x800000, 0x200000, CRC(d4ea0f56) SHA1(dc8d2ed3c11d0b6f9ebdfde805188884320235e6) )
ROM_END

static READ32_HANDLER( main_cycle_r )
{
	if (activecpu_get_pc()==0x702)
		cpu_spinuntil_int();

	return superchs_ram[0];
}

static READ16_HANDLER( sub_cycle_r )
{
	if (activecpu_get_pc()==0x454)
		cpu_spinuntil_int();

	return superchs_ram[2]&0xffff;
}

static DRIVER_INIT( superchs )
{
	/* Speedup handlers */
	memory_install_read32_handler(0, ADDRESS_SPACE_PROGRAM, 0x100000, 0x100003, 0, 0, main_cycle_r);
	memory_install_read16_handler(2, ADDRESS_SPACE_PROGRAM, 0x80000a, 0x80000b, 0, 0, sub_cycle_r);
}

GAME( 1992, superchs, 0, superchs, superchs, superchs, ROT0, "Taito America Corporation", "Super Chase - Criminal Termination (US)", 0 )
