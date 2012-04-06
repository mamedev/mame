/***************************************************************************

    Gradius 3 (GX945) (c) 1989 Konami

    driver by Nicola Salmoria

    This board uses the well known 052109 051962 custom gfx chips, however unlike
    all other games they fetch gfx data from RAM. The gfx ROMs are memory mapped
    on cpu B and the needed parts are copied to RAM at run time.
    To handle this efficiently in MAME, some changes would be required to the
    tilemap system and to video/konamiic.c. For the time being, I'm kludging
    my way in.
    There's also something wrong in the way tile banks are implemented in
    konamiic.c. They don't seem to be used by this game.

    2009-03:
    Added dsw locations and verified factory setting based on Guru's notes

***************************************************************************/

#include "emu.h"
#include "video/konicdev.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "sound/2151intf.h"
#include "sound/k007232.h"
#include "includes/konamipt.h"
#include "includes/gradius3.h"

READ16_MEMBER(gradius3_state::k052109_halfword_r)
{
	return k052109_r(m_k052109, offset);
}

WRITE16_MEMBER(gradius3_state::k052109_halfword_w)
{

	if (ACCESSING_BITS_0_7)
		k052109_w(m_k052109, offset, data & 0xff);

	/* is this a bug in the game or something else? */
	if (!ACCESSING_BITS_0_7)
		k052109_w(m_k052109, offset, (data >> 8) & 0xff);
//      logerror("%06x half %04x = %04x\n",cpu_get_pc(&space.device()),offset,data);
}

READ16_MEMBER(gradius3_state::k051937_halfword_r)
{
	return k051937_r(m_k051960, offset);
}

WRITE16_MEMBER(gradius3_state::k051937_halfword_w)
{

	if (ACCESSING_BITS_0_7)
		k051937_w(m_k051960, offset, data & 0xff);
}

READ16_MEMBER(gradius3_state::k051960_halfword_r)
{
	return k051960_r(m_k051960, offset);
}

WRITE16_MEMBER(gradius3_state::k051960_halfword_w)
{
	if (ACCESSING_BITS_0_7)
		k051960_w(m_k051960, offset, data & 0xff);
}

WRITE16_MEMBER(gradius3_state::cpuA_ctrl_w)
{

	if (ACCESSING_BITS_8_15)
	{
		data >>= 8;

		/* bits 0-1 are coin counters */
		coin_counter_w(machine(), 0, data & 0x01);
		coin_counter_w(machine(), 1, data & 0x02);

		/* bit 2 selects layer priority */
		m_priority = data & 0x04;

		/* bit 3 enables cpu B */
		device_set_input_line(m_subcpu, INPUT_LINE_RESET, (data & 0x08) ? CLEAR_LINE : ASSERT_LINE);

		/* bit 5 enables irq */
		m_irqAen = data & 0x20;

		/* other bits unknown */
	//logerror("%06x: write %04x to c0000\n",cpu_get_pc(&space.device()),data);
	}
}

WRITE16_MEMBER(gradius3_state::cpuB_irqenable_w)
{

	if (ACCESSING_BITS_8_15)
		m_irqBmask = (data >> 8) & 0x07;
}

static INTERRUPT_GEN( cpuA_interrupt )
{
	gradius3_state *state = device->machine().driver_data<gradius3_state>();
	if (state->m_irqAen)
		device_set_input_line(device, 2, HOLD_LINE);
}


static TIMER_DEVICE_CALLBACK( gradius3_sub_scanline )
{
	gradius3_state *state = timer.machine().driver_data<gradius3_state>();
	int scanline = param;

	if(scanline == 240 && state->m_irqBmask & 1) // vblank-out irq
		cputag_set_input_line(timer.machine(), "sub", 1, HOLD_LINE);

	if(scanline ==  16 && state->m_irqBmask & 2) // sprite end DMA irq
		cputag_set_input_line(timer.machine(), "sub", 2, HOLD_LINE);
}

WRITE16_MEMBER(gradius3_state::cpuB_irqtrigger_w)
{

	if (m_irqBmask & 4)
	{
		logerror("%04x trigger cpu B irq 4 %02x\n",cpu_get_pc(&space.device()),data);
		device_set_input_line(m_subcpu, 4, HOLD_LINE);
	}
	else
		logerror("%04x MISSED cpu B irq 4 %02x\n",cpu_get_pc(&space.device()),data);
}

WRITE16_MEMBER(gradius3_state::sound_command_w)
{
	if (ACCESSING_BITS_8_15)
		soundlatch_w(space, 0, (data >> 8) & 0xff);
}

WRITE16_MEMBER(gradius3_state::sound_irq_w)
{
	device_set_input_line_and_vector(m_audiocpu, 0, HOLD_LINE, 0xff);
}

static WRITE8_DEVICE_HANDLER( sound_bank_w )
{
	int bank_A, bank_B;

	/* banks # for the 007232 (chip 1) */
	bank_A = ((data >> 0) & 0x03);
	bank_B = ((data >> 2) & 0x03);
	k007232_set_bank(device, bank_A, bank_B);
}



static ADDRESS_MAP_START( gradius3_map, AS_PROGRAM, 16, gradius3_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x040000, 0x043fff) AM_RAM
	AM_RANGE(0x080000, 0x080fff) AM_RAM_WRITE(paletteram16_xRRRRRGGGGGBBBBB_word_w) AM_SHARE("paletteram")
	AM_RANGE(0x0c0000, 0x0c0001) AM_WRITE(cpuA_ctrl_w)	/* halt cpu B, irq enable, priority, coin counters, other? */
	AM_RANGE(0x0c8000, 0x0c8001) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x0c8002, 0x0c8003) AM_READ_PORT("P1")
	AM_RANGE(0x0c8004, 0x0c8005) AM_READ_PORT("P2")
	AM_RANGE(0x0c8006, 0x0c8007) AM_READ_PORT("DSW3")
	AM_RANGE(0x0d0000, 0x0d0001) AM_READ_PORT("DSW1")
	AM_RANGE(0x0d0002, 0x0d0003) AM_READ_PORT("DSW2")
	AM_RANGE(0x0d8000, 0x0d8001) AM_WRITE(cpuB_irqtrigger_w)
	AM_RANGE(0x0e0000, 0x0e0001) AM_WRITE(watchdog_reset16_w)
	AM_RANGE(0x0e8000, 0x0e8001) AM_WRITE(sound_command_w)
	AM_RANGE(0x0f0000, 0x0f0001) AM_WRITE(sound_irq_w)
	AM_RANGE(0x100000, 0x103fff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x14c000, 0x153fff) AM_READWRITE(k052109_halfword_r, k052109_halfword_w)
	AM_RANGE(0x180000, 0x19ffff) AM_RAM_WRITE(gradius3_gfxram_w) AM_BASE(m_gfxram) AM_SHARE("share2")
ADDRESS_MAP_END


static ADDRESS_MAP_START( gradius3_map2, AS_PROGRAM, 16, gradius3_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x100000, 0x103fff) AM_RAM
	AM_RANGE(0x140000, 0x140001) AM_WRITE(cpuB_irqenable_w)
	AM_RANGE(0x200000, 0x203fff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x24c000, 0x253fff) AM_READWRITE(k052109_halfword_r, k052109_halfword_w)
	AM_RANGE(0x280000, 0x29ffff) AM_RAM_WRITE(gradius3_gfxram_w) AM_SHARE("share2")
	AM_RANGE(0x2c0000, 0x2c000f) AM_READWRITE(k051937_halfword_r, k051937_halfword_w)
	AM_RANGE(0x2c0800, 0x2c0fff) AM_READWRITE(k051960_halfword_r, k051960_halfword_w)
	AM_RANGE(0x400000, 0x5fffff) AM_READ(gradius3_gfxrom_r)		/* gfx ROMs are mapped here, and copied to RAM */
ADDRESS_MAP_END


static ADDRESS_MAP_START( gradius3_s_map, AS_PROGRAM, 8, gradius3_state )
	AM_RANGE(0x0000, 0xefff) AM_ROM
	AM_RANGE(0xf000, 0xf000) AM_DEVWRITE_LEGACY("k007232", sound_bank_w)				/* 007232 bankswitch */
	AM_RANGE(0xf010, 0xf010) AM_READ(soundlatch_r)
	AM_RANGE(0xf020, 0xf02d) AM_DEVREADWRITE_LEGACY("k007232", k007232_r, k007232_w)
	AM_RANGE(0xf030, 0xf031) AM_DEVREADWRITE_LEGACY("ymsnd", ym2151_r, ym2151_w)
	AM_RANGE(0xf800, 0xffff) AM_RAM
ADDRESS_MAP_END



static INPUT_PORTS_START( gradius3 )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1")
	KONAMI8_MONO_B123_UNK						// button1 = power-up, button2 = shoot, button3 = missile

	PORT_START("P2")
	KONAMI8_COCKTAIL_B123_UNK

	PORT_START("DSW1")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "Invalid", SW1)
	/* "Invalid" = both coin slots disabled */

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )			PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )			PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Bonus_Life ) )		PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "20k and every 70k" )
	PORT_DIPSETTING(    0x10, "100k and every 100k" )
	PORT_DIPSETTING(    0x08, "50k only" )
	PORT_DIPSETTING(    0x00, "100k only" )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )		PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )		PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )		PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Upright Controls" )			PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Dual ) )
	PORT_SERVICE_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW3:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW3:4" )			/* Manual says it's unused */
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static void volume_callback(device_t *device, int v)
{
	k007232_set_volume(device, 0, (v >> 4) * 0x11, 0);
	k007232_set_volume(device, 1, 0, (v & 0x0f) * 0x11);
}

static const k007232_interface k007232_config =
{
	volume_callback	/* external port callback */
};



static const k052109_interface gradius3_k052109_intf =
{
	"gfx1", 0,
	GRADIUS3_PLANE_ORDER,
	KONAMI_ROM_DEINTERLEAVE_NONE,
	gradius3_tile_callback
};

static const k051960_interface gradius3_k051960_intf =
{
	"gfx2", 1,
	GRADIUS3_PLANE_ORDER,
	KONAMI_ROM_DEINTERLEAVE_2,
	gradius3_sprite_callback
};

static MACHINE_START( gradius3 )
{
	gradius3_state *state = machine.driver_data<gradius3_state>();

	state->m_maincpu = machine.device("maincpu");
	state->m_audiocpu = machine.device("audiocpu");
	state->m_subcpu = machine.device("sub");
	state->m_k007232 = machine.device("k007232");
	state->m_k052109 = machine.device("k052109");
	state->m_k051960 = machine.device("k051960");

	state->save_item(NAME(state->m_irqAen));
	state->save_item(NAME(state->m_irqBmask));
	state->save_item(NAME(state->m_priority));
}

static MACHINE_RESET( gradius3 )
{
	gradius3_state *state = machine.driver_data<gradius3_state>();

	/* start with cpu B halted */
	cputag_set_input_line(machine, "sub", INPUT_LINE_RESET, ASSERT_LINE);
	state->m_irqAen = 0;
	state->m_irqBmask = 0;
	state->m_priority = 0;

}

static MACHINE_CONFIG_START( gradius3, gradius3_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 10000000)	/* 10 MHz */
	MCFG_CPU_PROGRAM_MAP(gradius3_map)
	MCFG_CPU_VBLANK_INT("screen", cpuA_interrupt)

	MCFG_CPU_ADD("sub", M68000, 10000000)	/* 10 MHz */
	MCFG_CPU_PROGRAM_MAP(gradius3_map2)
	MCFG_TIMER_ADD_SCANLINE("scantimer", gradius3_sub_scanline, "screen", 0, 1) /* has three interrupt vectors, 1 2 and 4 */
																				/* 4 is triggered by cpu A, the others are unknown but */
																				/* required for the game to run. */

	MCFG_CPU_ADD("audiocpu", Z80, 3579545)
	MCFG_CPU_PROGRAM_MAP(gradius3_s_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))

	MCFG_MACHINE_START(gradius3)
	MCFG_MACHINE_RESET(gradius3)

	/* video hardware */
	MCFG_VIDEO_ATTRIBUTES(VIDEO_HAS_SHADOWS)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(12*8, (64-12)*8-1, 2*8, 30*8-1 )
	MCFG_SCREEN_UPDATE_STATIC(gradius3)

	MCFG_PALETTE_LENGTH(2048)

	MCFG_VIDEO_START(gradius3)

	MCFG_K052109_ADD("k052109", gradius3_k052109_intf)
	MCFG_K051960_ADD("k051960", gradius3_k051960_intf)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("ymsnd", YM2151, 3579545)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)

	MCFG_SOUND_ADD("k007232", K007232, 3579545)
	MCFG_SOUND_CONFIG(k007232_config)
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.20)
	MCFG_SOUND_ROUTE(0, "rspeaker", 0.20)
	MCFG_SOUND_ROUTE(1, "lspeaker", 0.20)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.20)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( gradius3 )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "945_s13.f15",		0x00000, 0x20000, CRC(70c240a2) SHA1(82dc391572e1f61b0182cb031654d71adcdd5f6e) )
	ROM_LOAD16_BYTE( "945_s12.e15",		0x00001, 0x20000, CRC(bbc300d4) SHA1(e1ca98bc591575285d7bd2d4fefdf35fed10dcb6) )

	ROM_REGION( 0x100000, "sub", 0 )
	ROM_LOAD16_BYTE( "945_m09.r17",		0x000000, 0x20000, CRC(b4a6df25) SHA1(85533cf140d28f6f81c0b49b8061bda0924a613a) )
	ROM_LOAD16_BYTE( "945_m08.n17",		0x000001, 0x20000, CRC(74e981d2) SHA1(e7b47a2da01ff73293d2100c48fdf00b33125af5) )
	ROM_LOAD16_BYTE( "945_l06b.r11",	0x040000, 0x20000, CRC(83772304) SHA1(a90c75a3de670b6ec5e0fc201876d463b4a76766) )
	ROM_LOAD16_BYTE( "945_l06a.n11",	0x040001, 0x20000, CRC(e1fd75b6) SHA1(6160d80a2f1bf550e85d6253cf521a96f5a644cc) )
	ROM_LOAD16_BYTE( "945_l07c.r15",	0x080000, 0x20000, CRC(c1e399b6) SHA1(e95bd478dd3beea0175bf9ee4cededb111c4ace1) )
	ROM_LOAD16_BYTE( "945_l07a.n15",	0x080001, 0x20000, CRC(96222d04) SHA1(b55700f683a556b0e73dbac9c7b4ce485420d21c) )
	ROM_LOAD16_BYTE( "945_l07d.r13",	0x0c0000, 0x20000, CRC(4c16d4bd) SHA1(01dcf169b78a1e495214b10181401d1920b0c924) )
	ROM_LOAD16_BYTE( "945_l07b.n13",	0x0c0001, 0x20000, CRC(5e209d01) SHA1(0efa1bbfdc7e2ba1e0bb96245e2bfe961258b446) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "945_m05.d9",				0x00000, 0x10000, CRC(c8c45365) SHA1(b9a7b736b52bca42c7b8c8ed64c8df73e0116158) )

	ROM_REGION( 0x20000, "gfx1", ROMREGION_ERASE00 )	/* fake */
	/* gfx data is dynamically generated in RAM */

	ROM_REGION( 0x200000, "gfx2", 0 )	/* graphics (addressable by the main CPU) */
	ROM_LOAD( "945_a02.l3",				0x000000, 0x80000, CRC(4dfffd74) SHA1(588210bac27448240ef08961f70b714b69cb3ffd) )
	ROM_LOAD16_BYTE( "945_l04a.k6",		0x080000, 0x20000, CRC(884e21ee) SHA1(ce86dd3a06775e5b1aa09db010dcb674e67828e7) )
	ROM_LOAD16_BYTE( "945_l04c.m6",		0x080001, 0x20000, CRC(45bcd921) SHA1(e51a8a71362a6fb55124aa1dce74519c0a3c6e3f) )
	ROM_LOAD16_BYTE( "945_l04b.k8",		0x0c0000, 0x20000, CRC(843bc67d) SHA1(cdf8421083f24ab27867ed5d08d8949da192b2b9) )
	ROM_LOAD16_BYTE( "945_l04d.m8",		0x0c0001, 0x20000, CRC(0a98d08e) SHA1(1e0ca51a2d45c01fa3f11950ddd387f41ddae691) )
	ROM_LOAD( "945_a01.h3",				0x100000, 0x80000, CRC(339d6dd2) SHA1(6a52b826aba92c75fc6a5926184948735dc20812) )
	ROM_LOAD16_BYTE( "945_l03a.e6",		0x180000, 0x20000, CRC(a67ef087) SHA1(fd63474f3bbde5dfc53ed4c1db25d6411a8b54d2) )
	ROM_LOAD16_BYTE( "945_l03c.h6",		0x180001, 0x20000, CRC(a56be17a) SHA1(1d387736144c30fcb5de54235331ab1ff70c356e) )
	ROM_LOAD16_BYTE( "945_l03b.e8",		0x1c0000, 0x20000, CRC(933e68b9) SHA1(f3a39446ca77d17fdbd938bd5f718ae9d5570879) )
	ROM_LOAD16_BYTE( "945_l03d.h8",		0x1c0001, 0x20000, CRC(f375e87b) SHA1(6427b966795c907c8e516244872fe52217da62c4) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "945l14.j28",				0x0000, 0x0100, CRC(c778c189) SHA1(847eaf379ba075c25911c6f83dd63ff390534f60) )	/* priority encoder (not used) */

	ROM_REGION( 0x80000, "k007232", 0 )	/* 007232 samples */
	ROM_LOAD( "945_a10.b15",			0x00000, 0x40000, CRC(1d083e10) SHA1(b116f133a7647ef7a6c373aff00e9622d9954b61) )
	ROM_LOAD( "945_l11a.c18",			0x40000, 0x20000, CRC(6043f4eb) SHA1(1c2e9ace1cfdde504b7b6158e3c3f54dc5ae33d4) )
	ROM_LOAD( "945_l11b.c20",			0x60000, 0x20000, CRC(89ea3baf) SHA1(8edcbaa7969185cfac48c02559826d1b8b081f3f) )
ROM_END

ROM_START( gradius3a )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "945_13.f15",		0x00000, 0x20000, CRC(9974fe6b) SHA1(c18ad8d7c93bf58d886715d8e210177cf49f220b) )
	ROM_LOAD16_BYTE( "945_12.e15",		0x00001, 0x20000, CRC(e9771b91) SHA1(c9f4610b897c13742b44b546e2bed8ee21945f61) )

	ROM_REGION( 0x100000, "sub", 0 )
	ROM_LOAD16_BYTE( "945_m09.r17",		0x000000, 0x20000, CRC(b4a6df25) SHA1(85533cf140d28f6f81c0b49b8061bda0924a613a) )
	ROM_LOAD16_BYTE( "945_m08.n17",		0x000001, 0x20000, CRC(74e981d2) SHA1(e7b47a2da01ff73293d2100c48fdf00b33125af5) )
	ROM_LOAD16_BYTE( "945_l06b.r11",	0x040000, 0x20000, CRC(83772304) SHA1(a90c75a3de670b6ec5e0fc201876d463b4a76766) )
	ROM_LOAD16_BYTE( "945_l06a.n11",	0x040001, 0x20000, CRC(e1fd75b6) SHA1(6160d80a2f1bf550e85d6253cf521a96f5a644cc) )
	ROM_LOAD16_BYTE( "945_l07c.r15",	0x080000, 0x20000, CRC(c1e399b6) SHA1(e95bd478dd3beea0175bf9ee4cededb111c4ace1) )
	ROM_LOAD16_BYTE( "945_l07a.n15",	0x080001, 0x20000, CRC(96222d04) SHA1(b55700f683a556b0e73dbac9c7b4ce485420d21c) )
	ROM_LOAD16_BYTE( "945_l07d.r13",	0x0c0000, 0x20000, CRC(4c16d4bd) SHA1(01dcf169b78a1e495214b10181401d1920b0c924) )
	ROM_LOAD16_BYTE( "945_l07b.n13",	0x0c0001, 0x20000, CRC(5e209d01) SHA1(0efa1bbfdc7e2ba1e0bb96245e2bfe961258b446) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "945_m05.d9",				0x00000, 0x10000, CRC(c8c45365) SHA1(b9a7b736b52bca42c7b8c8ed64c8df73e0116158) )

	ROM_REGION( 0x20000, "gfx1", ROMREGION_ERASE00 )	/* fake */
	/* gfx data is dynamically generated in RAM */

	ROM_REGION( 0x200000, "gfx2", 0 )	/* graphics (addressable by the main CPU) */
	ROM_LOAD( "945_a02.l3",				0x000000, 0x80000, CRC(4dfffd74) SHA1(588210bac27448240ef08961f70b714b69cb3ffd) )
	ROM_LOAD16_BYTE( "945_l04a.k6",		0x080000, 0x20000, CRC(884e21ee) SHA1(ce86dd3a06775e5b1aa09db010dcb674e67828e7) )
	ROM_LOAD16_BYTE( "945_l04c.m6",		0x080001, 0x20000, CRC(45bcd921) SHA1(e51a8a71362a6fb55124aa1dce74519c0a3c6e3f) )
	ROM_LOAD16_BYTE( "945_l04b.k8",		0x0c0000, 0x20000, CRC(843bc67d) SHA1(cdf8421083f24ab27867ed5d08d8949da192b2b9) )
	ROM_LOAD16_BYTE( "945_l04d.m8",		0x0c0001, 0x20000, CRC(0a98d08e) SHA1(1e0ca51a2d45c01fa3f11950ddd387f41ddae691) )
	ROM_LOAD( "945_a01.h3",				0x100000, 0x80000, CRC(339d6dd2) SHA1(6a52b826aba92c75fc6a5926184948735dc20812) )
	ROM_LOAD16_BYTE( "945_l03a.e6",		0x180000, 0x20000, CRC(a67ef087) SHA1(fd63474f3bbde5dfc53ed4c1db25d6411a8b54d2) )
	ROM_LOAD16_BYTE( "945_l03c.h6",		0x180001, 0x20000, CRC(a56be17a) SHA1(1d387736144c30fcb5de54235331ab1ff70c356e) )
	ROM_LOAD16_BYTE( "945_l03b.e8",		0x1c0000, 0x20000, CRC(933e68b9) SHA1(f3a39446ca77d17fdbd938bd5f718ae9d5570879) )
	ROM_LOAD16_BYTE( "945_l03d.h8",		0x1c0001, 0x20000, CRC(f375e87b) SHA1(6427b966795c907c8e516244872fe52217da62c4) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "945l14.j28",				0x0000, 0x0100, CRC(c778c189) SHA1(847eaf379ba075c25911c6f83dd63ff390534f60) )	/* priority encoder (not used) */

	ROM_REGION( 0x80000, "k007232", 0 )	/* 007232 samples */
	ROM_LOAD( "945_a10.b15",			0x00000, 0x40000, CRC(1d083e10) SHA1(b116f133a7647ef7a6c373aff00e9622d9954b61) )
	ROM_LOAD( "945_l11a.c18",			0x40000, 0x20000, CRC(6043f4eb) SHA1(1c2e9ace1cfdde504b7b6158e3c3f54dc5ae33d4) )
	ROM_LOAD( "945_l11b.c20",			0x60000, 0x20000, CRC(89ea3baf) SHA1(8edcbaa7969185cfac48c02559826d1b8b081f3f) )
ROM_END

ROM_START( gradius3e )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "945_r13.f15",		0x00000, 0x20000, CRC(cffd103f) SHA1(6bd15e8c2e6e5223d7de9b0b375f36f3e81f60ba) )
	ROM_LOAD16_BYTE( "945_r12.e15",		0x00001, 0x20000, CRC(0b968ef6) SHA1(ba28d16d94b13aac791b11d3d91df26f78e2e477) )

	ROM_REGION( 0x100000, "sub", 0 )
	ROM_LOAD16_BYTE( "945_m09.r17",		0x000000, 0x20000, CRC(b4a6df25) SHA1(85533cf140d28f6f81c0b49b8061bda0924a613a) )
	ROM_LOAD16_BYTE( "945_m08.n17",		0x000001, 0x20000, CRC(74e981d2) SHA1(e7b47a2da01ff73293d2100c48fdf00b33125af5) )
	ROM_LOAD16_BYTE( "945_l06b.r11",	0x040000, 0x20000, CRC(83772304) SHA1(a90c75a3de670b6ec5e0fc201876d463b4a76766) )
	ROM_LOAD16_BYTE( "945_l06a.n11",	0x040001, 0x20000, CRC(e1fd75b6) SHA1(6160d80a2f1bf550e85d6253cf521a96f5a644cc) )
	ROM_LOAD16_BYTE( "945_l07c.r15",	0x080000, 0x20000, CRC(c1e399b6) SHA1(e95bd478dd3beea0175bf9ee4cededb111c4ace1) )
	ROM_LOAD16_BYTE( "945_l07a.n15",	0x080001, 0x20000, CRC(96222d04) SHA1(b55700f683a556b0e73dbac9c7b4ce485420d21c) )
	ROM_LOAD16_BYTE( "945_l07d.r13",	0x0c0000, 0x20000, CRC(4c16d4bd) SHA1(01dcf169b78a1e495214b10181401d1920b0c924) )
	ROM_LOAD16_BYTE( "945_l07b.n13",	0x0c0001, 0x20000, CRC(5e209d01) SHA1(0efa1bbfdc7e2ba1e0bb96245e2bfe961258b446) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "945_m05.d9",				0x00000, 0x10000, CRC(c8c45365) SHA1(b9a7b736b52bca42c7b8c8ed64c8df73e0116158) ) /* 945_r05.d9 */

	ROM_REGION( 0x20000, "gfx1", ROMREGION_ERASE00 )	/* fake */
	/* gfx data is dynamically generated in RAM */

	ROM_REGION( 0x200000, "gfx2", 0 )	/* graphics (addressable by the main CPU) */
	ROM_LOAD( "945_a02.l3",				0x000000, 0x80000, CRC(4dfffd74) SHA1(588210bac27448240ef08961f70b714b69cb3ffd) )
	ROM_LOAD16_BYTE( "945_l04a.k6",		0x080000, 0x20000, CRC(884e21ee) SHA1(ce86dd3a06775e5b1aa09db010dcb674e67828e7) )
	ROM_LOAD16_BYTE( "945_l04c.m6",		0x080001, 0x20000, CRC(45bcd921) SHA1(e51a8a71362a6fb55124aa1dce74519c0a3c6e3f) )
	ROM_LOAD16_BYTE( "945_l04b.k8",		0x0c0000, 0x20000, CRC(843bc67d) SHA1(cdf8421083f24ab27867ed5d08d8949da192b2b9) )
	ROM_LOAD16_BYTE( "945_l04d.m8",		0x0c0001, 0x20000, CRC(0a98d08e) SHA1(1e0ca51a2d45c01fa3f11950ddd387f41ddae691) )
	ROM_LOAD( "945_a01.h3",				0x100000, 0x80000, CRC(339d6dd2) SHA1(6a52b826aba92c75fc6a5926184948735dc20812) )
	ROM_LOAD16_BYTE( "945_l03a.e6",		0x180000, 0x20000, CRC(a67ef087) SHA1(fd63474f3bbde5dfc53ed4c1db25d6411a8b54d2) )
	ROM_LOAD16_BYTE( "945_l03c.h6",		0x180001, 0x20000, CRC(a56be17a) SHA1(1d387736144c30fcb5de54235331ab1ff70c356e) )
	ROM_LOAD16_BYTE( "945_l03b.e8",		0x1c0000, 0x20000, CRC(933e68b9) SHA1(f3a39446ca77d17fdbd938bd5f718ae9d5570879) )
	ROM_LOAD16_BYTE( "945_l03d.h8",		0x1c0001, 0x20000, CRC(f375e87b) SHA1(6427b966795c907c8e516244872fe52217da62c4) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "945l14.j28",				0x0000, 0x0100, CRC(c778c189) SHA1(847eaf379ba075c25911c6f83dd63ff390534f60) )	/* priority encoder (not used) */

	ROM_REGION( 0x80000, "k007232", 0 )	/* 007232 samples */
	ROM_LOAD( "945_a10.b15",			0x00000, 0x40000, CRC(1d083e10) SHA1(b116f133a7647ef7a6c373aff00e9622d9954b61) )
	ROM_LOAD( "945_l11a.c18",			0x40000, 0x20000, CRC(6043f4eb) SHA1(1c2e9ace1cfdde504b7b6158e3c3f54dc5ae33d4) )
	ROM_LOAD( "945_l11b.c20",			0x60000, 0x20000, CRC(89ea3baf) SHA1(8edcbaa7969185cfac48c02559826d1b8b081f3f) )
ROM_END



GAME( 1989, gradius3,  0,        gradius3, gradius3, 0, ROT0, "Konami", "Gradius III (Japan)", GAME_SUPPORTS_SAVE )
GAME( 1989, gradius3a, gradius3, gradius3, gradius3, 0, ROT0, "Konami", "Gradius III (Asia)", GAME_SUPPORTS_SAVE )
GAME( 1989, gradius3e, gradius3, gradius3, gradius3, 0, ROT0, "Konami", "Gradius III (World ?)", GAME_SUPPORTS_SAVE )
