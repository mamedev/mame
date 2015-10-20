// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    Gradius 3 (GX945) (c) 1989 Konami

    driver by Nicola Salmoria

    This board uses the well known 052109 051962 custom gfx chips, however unlike
    all other games they fetch gfx data from RAM. The gfx ROMs are memory mapped
    on cpu B and the needed parts are copied to RAM at run time.

    There's also something wrong in the way tile banks are implemented in
    k052109.c. They don't seem to be used by this game.

    2009-03:
    Added dsw locations and verified factory setting based on Guru's notes

    2015-05:
    gradius3js set added, same as normal gradius3j set in content but with
    some ROMs split and populated differently.

***************************************************************************/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "sound/2151intf.h"
#include "includes/konamipt.h"
#include "includes/gradius3.h"

READ16_MEMBER(gradius3_state::k052109_halfword_r)
{
	return m_k052109->read(space, offset);
}

WRITE16_MEMBER(gradius3_state::k052109_halfword_w)
{
	if (ACCESSING_BITS_0_7)
		m_k052109->write(space, offset, data & 0xff);

	/* is this a bug in the game or something else? */
	if (!ACCESSING_BITS_0_7)
		m_k052109->write(space, offset, (data >> 8) & 0xff);
//      logerror("%06x half %04x = %04x\n",space.device().safe_pc(),offset,data);
}

READ16_MEMBER(gradius3_state::k051937_halfword_r)
{
	return m_k051960->k051937_r(space, offset);
}

WRITE16_MEMBER(gradius3_state::k051937_halfword_w)
{
	if (ACCESSING_BITS_0_7)
		m_k051960->k051937_w(space, offset, data & 0xff);
}

READ16_MEMBER(gradius3_state::k051960_halfword_r)
{
	return m_k051960->k051960_r(space, offset);
}

WRITE16_MEMBER(gradius3_state::k051960_halfword_w)
{
	if (ACCESSING_BITS_0_7)
		m_k051960->k051960_w(space, offset, data & 0xff);
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
		m_subcpu->set_input_line(INPUT_LINE_RESET, (data & 0x08) ? CLEAR_LINE : ASSERT_LINE);

		/* bit 5 enables irq */
		m_irqAen = data & 0x20;

		/* other bits unknown */
	//logerror("%06x: write %04x to c0000\n",space.device().safe_pc(),data);
	}
}

WRITE16_MEMBER(gradius3_state::cpuB_irqenable_w)
{
	if (ACCESSING_BITS_8_15)
		m_irqBmask = (data >> 8) & 0x07;
}

INTERRUPT_GEN_MEMBER(gradius3_state::cpuA_interrupt)
{
	if (m_irqAen)
		device.execute().set_input_line(2, HOLD_LINE);
}


TIMER_DEVICE_CALLBACK_MEMBER(gradius3_state::gradius3_sub_scanline)
{
	int scanline = param;

	if(scanline == 240 && m_irqBmask & 1) // vblank-out irq
		m_subcpu->set_input_line(1, HOLD_LINE);

	if(scanline ==  16 && m_irqBmask & 2) // sprite end DMA irq
		m_subcpu->set_input_line(2, HOLD_LINE);
}

WRITE16_MEMBER(gradius3_state::cpuB_irqtrigger_w)
{
	if (m_irqBmask & 4)
	{
		logerror("%04x trigger cpu B irq 4 %02x\n",space.device().safe_pc(),data);
		m_subcpu->set_input_line(4, HOLD_LINE);
	}
	else
		logerror("%04x MISSED cpu B irq 4 %02x\n",space.device().safe_pc(),data);
}

WRITE16_MEMBER(gradius3_state::sound_irq_w)
{
	m_audiocpu->set_input_line_and_vector(0, HOLD_LINE, 0xff);
}

WRITE8_MEMBER(gradius3_state::sound_bank_w)
{
	int bank_A, bank_B;

	/* banks # for the 007232 (chip 1) */
	bank_A = ((data >> 0) & 0x03);
	bank_B = ((data >> 2) & 0x03);
	m_k007232->set_bank(bank_A, bank_B);
}



static ADDRESS_MAP_START( gradius3_map, AS_PROGRAM, 16, gradius3_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x040000, 0x043fff) AM_RAM
	AM_RANGE(0x080000, 0x080fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x0c0000, 0x0c0001) AM_WRITE(cpuA_ctrl_w)  /* halt cpu B, irq enable, priority, coin counters, other? */
	AM_RANGE(0x0c8000, 0x0c8001) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x0c8002, 0x0c8003) AM_READ_PORT("P1")
	AM_RANGE(0x0c8004, 0x0c8005) AM_READ_PORT("P2")
	AM_RANGE(0x0c8006, 0x0c8007) AM_READ_PORT("DSW3")
	AM_RANGE(0x0d0000, 0x0d0001) AM_READ_PORT("DSW1")
	AM_RANGE(0x0d0002, 0x0d0003) AM_READ_PORT("DSW2")
	AM_RANGE(0x0d8000, 0x0d8001) AM_WRITE(cpuB_irqtrigger_w)
	AM_RANGE(0x0e0000, 0x0e0001) AM_WRITE(watchdog_reset16_w)
	AM_RANGE(0x0e8000, 0x0e8001) AM_WRITE8(soundlatch_byte_w, 0xff00)
	AM_RANGE(0x0f0000, 0x0f0001) AM_WRITE(sound_irq_w)
	AM_RANGE(0x100000, 0x103fff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x14c000, 0x153fff) AM_READWRITE(k052109_halfword_r, k052109_halfword_w)
	AM_RANGE(0x180000, 0x19ffff) AM_RAM_WRITE(gradius3_gfxram_w) AM_SHARE("k052109")
ADDRESS_MAP_END


static ADDRESS_MAP_START( gradius3_map2, AS_PROGRAM, 16, gradius3_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x100000, 0x103fff) AM_RAM
	AM_RANGE(0x140000, 0x140001) AM_WRITE(cpuB_irqenable_w)
	AM_RANGE(0x200000, 0x203fff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x24c000, 0x253fff) AM_READWRITE(k052109_halfword_r, k052109_halfword_w)
	AM_RANGE(0x280000, 0x29ffff) AM_RAM_WRITE(gradius3_gfxram_w) AM_SHARE("k052109")
	AM_RANGE(0x2c0000, 0x2c000f) AM_READWRITE(k051937_halfword_r, k051937_halfword_w)
	AM_RANGE(0x2c0800, 0x2c0fff) AM_READWRITE(k051960_halfword_r, k051960_halfword_w)
	AM_RANGE(0x400000, 0x5fffff) AM_READ(gradius3_gfxrom_r)     /* gfx ROMs are mapped here, and copied to RAM */
ADDRESS_MAP_END


static ADDRESS_MAP_START( gradius3_s_map, AS_PROGRAM, 8, gradius3_state )
	AM_RANGE(0x0000, 0xefff) AM_ROM
	AM_RANGE(0xf000, 0xf000) AM_WRITE(sound_bank_w)             /* 007232 bankswitch */
	AM_RANGE(0xf010, 0xf010) AM_READ(soundlatch_byte_r)
	AM_RANGE(0xf020, 0xf02d) AM_DEVREADWRITE("k007232", k007232_device, read, write)
	AM_RANGE(0xf030, 0xf031) AM_DEVREADWRITE("ymsnd", ym2151_device, read, write)
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
	KONAMI8_MONO_B123_UNK                       // button1 = power-up, button2 = shoot, button3 = missile

	PORT_START("P2")
	KONAMI8_COCKTAIL_B123_UNK

	PORT_START("DSW1")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "Invalid", SW1)
	/* "Invalid" = both coin slots disabled */

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "20k and every 70k" )
	PORT_DIPSETTING(    0x10, "100k and every 100k" )
	PORT_DIPSETTING(    0x08, "50k only" )
	PORT_DIPSETTING(    0x00, "100k only" )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Upright Controls" )          PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Dual ) )
	PORT_SERVICE_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW3:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW3:4" )           /* Manual says it's unused */
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


WRITE8_MEMBER(gradius3_state::volume_callback)
{
	m_k007232->set_volume(0, (data >> 4) * 0x11, 0);
	m_k007232->set_volume(1, 0, (data & 0x0f) * 0x11);
}

void gradius3_state::machine_start()
{
	save_item(NAME(m_irqAen));
	save_item(NAME(m_irqBmask));
	save_item(NAME(m_priority));
}

void gradius3_state::machine_reset()
{
	/* start with cpu B halted */
	m_subcpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_irqAen = 0;
	m_irqBmask = 0;
	m_priority = 0;

}

static MACHINE_CONFIG_START( gradius3, gradius3_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_10MHz)
	MCFG_CPU_PROGRAM_MAP(gradius3_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", gradius3_state,  cpuA_interrupt)

	MCFG_CPU_ADD("sub", M68000, XTAL_10MHz)
	MCFG_CPU_PROGRAM_MAP(gradius3_map2)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", gradius3_state, gradius3_sub_scanline, "screen", 0, 1)
																				/* 4 is triggered by cpu A, the others are unknown but */
																				/* required for the game to run. */

	MCFG_CPU_ADD("audiocpu", Z80, 3579545)
	MCFG_CPU_PROGRAM_MAP(gradius3_s_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(12*8, (64-12)*8-1, 2*8, 30*8-1 )
	MCFG_SCREEN_UPDATE_DRIVER(gradius3_state, screen_update_gradius3)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 2048)
	MCFG_PALETTE_FORMAT(xRRRRRGGGGGBBBBB)
	MCFG_PALETTE_ENABLE_SHADOWS()

	MCFG_DEVICE_ADD("k052109", K052109, 0)
	MCFG_GFX_PALETTE("palette")
	MCFG_K052109_CB(gradius3_state, tile_callback)
	MCFG_K052109_CHARRAM(true)

	MCFG_DEVICE_ADD("k051960", K051960, 0)
	MCFG_GFX_PALETTE("palette")
	MCFG_K051960_SCREEN_TAG("screen")
	MCFG_K051960_CB(gradius3_state, sprite_callback)
	MCFG_K051960_PLANEORDER(K051960_PLANEORDER_GRADIUS3)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_YM2151_ADD("ymsnd", 3579545)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)

	MCFG_SOUND_ADD("k007232", K007232, 3579545)
	MCFG_K007232_PORT_WRITE_HANDLER(WRITE8(gradius3_state, volume_callback))
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
	ROM_LOAD16_BYTE( "945_r13.f15", 0x00000, 0x20000, CRC(cffd103f) SHA1(6bd15e8c2e6e5223d7de9b0b375f36f3e81f60ba) )
	ROM_LOAD16_BYTE( "945_r12.e15", 0x00001, 0x20000, CRC(0b968ef6) SHA1(ba28d16d94b13aac791b11d3d91df26f78e2e477) )

	ROM_REGION( 0x100000, "sub", 0 )
	ROM_LOAD16_BYTE( "945_m09.r17",  0x000000, 0x20000, CRC(b4a6df25) SHA1(85533cf140d28f6f81c0b49b8061bda0924a613a) )
	ROM_LOAD16_BYTE( "945_m08.n17",  0x000001, 0x20000, CRC(74e981d2) SHA1(e7b47a2da01ff73293d2100c48fdf00b33125af5) )
	ROM_LOAD16_BYTE( "945_l06b.r11", 0x040000, 0x20000, CRC(83772304) SHA1(a90c75a3de670b6ec5e0fc201876d463b4a76766) )
	ROM_LOAD16_BYTE( "945_l06a.n11", 0x040001, 0x20000, CRC(e1fd75b6) SHA1(6160d80a2f1bf550e85d6253cf521a96f5a644cc) )
	ROM_LOAD16_BYTE( "945_l07c.r15", 0x080000, 0x20000, CRC(c1e399b6) SHA1(e95bd478dd3beea0175bf9ee4cededb111c4ace1) )
	ROM_LOAD16_BYTE( "945_l07a.n15", 0x080001, 0x20000, CRC(96222d04) SHA1(b55700f683a556b0e73dbac9c7b4ce485420d21c) )
	ROM_LOAD16_BYTE( "945_l07d.r13", 0x0c0000, 0x20000, CRC(4c16d4bd) SHA1(01dcf169b78a1e495214b10181401d1920b0c924) )
	ROM_LOAD16_BYTE( "945_l07b.n13", 0x0c0001, 0x20000, CRC(5e209d01) SHA1(0efa1bbfdc7e2ba1e0bb96245e2bfe961258b446) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "945_r05.d9", 0x00000, 0x10000, CRC(c8c45365) SHA1(b9a7b736b52bca42c7b8c8ed64c8df73e0116158) ) /* Same as 945 M05, but different label */

	ROM_REGION( 0x200000, "k051960", 0 )   /* graphics (addressable by the main CPU) */
	ROM_LOAD32_WORD( "945_a02.l3",  0x000000, 0x80000, CRC(4dfffd74) SHA1(588210bac27448240ef08961f70b714b69cb3ffd) )
	ROM_LOAD32_WORD( "945_a01.h3",  0x000002, 0x80000, CRC(339d6dd2) SHA1(6a52b826aba92c75fc6a5926184948735dc20812) )
	ROM_LOAD32_BYTE( "945_l04a.k6", 0x100000, 0x20000, CRC(884e21ee) SHA1(ce86dd3a06775e5b1aa09db010dcb674e67828e7) )
	ROM_LOAD32_BYTE( "945_l04c.m6", 0x100001, 0x20000, CRC(45bcd921) SHA1(e51a8a71362a6fb55124aa1dce74519c0a3c6e3f) )
	ROM_LOAD32_BYTE( "945_l03a.e6", 0x100002, 0x20000, CRC(a67ef087) SHA1(fd63474f3bbde5dfc53ed4c1db25d6411a8b54d2) )
	ROM_LOAD32_BYTE( "945_l03c.h6", 0x100003, 0x20000, CRC(a56be17a) SHA1(1d387736144c30fcb5de54235331ab1ff70c356e) )
	ROM_LOAD32_BYTE( "945_l04b.k8", 0x180000, 0x20000, CRC(843bc67d) SHA1(cdf8421083f24ab27867ed5d08d8949da192b2b9) )
	ROM_LOAD32_BYTE( "945_l04d.m8", 0x180001, 0x20000, CRC(0a98d08e) SHA1(1e0ca51a2d45c01fa3f11950ddd387f41ddae691) )
	ROM_LOAD32_BYTE( "945_l03b.e8", 0x180002, 0x20000, CRC(933e68b9) SHA1(f3a39446ca77d17fdbd938bd5f718ae9d5570879) )
	ROM_LOAD32_BYTE( "945_l03d.h8", 0x180003, 0x20000, CRC(f375e87b) SHA1(6427b966795c907c8e516244872fe52217da62c4) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "945l14.j28", 0x0000, 0x0100, CRC(c778c189) SHA1(847eaf379ba075c25911c6f83dd63ff390534f60) )  /* priority encoder (not used) */

	ROM_REGION( 0x80000, "k007232", 0 ) /* 007232 samples */
	ROM_LOAD( "945_a10.b15",  0x00000, 0x40000, CRC(1d083e10) SHA1(b116f133a7647ef7a6c373aff00e9622d9954b61) )
	ROM_LOAD( "945_l11a.c18", 0x40000, 0x20000, CRC(6043f4eb) SHA1(1c2e9ace1cfdde504b7b6158e3c3f54dc5ae33d4) )
	ROM_LOAD( "945_l11b.c20", 0x60000, 0x20000, CRC(89ea3baf) SHA1(8edcbaa7969185cfac48c02559826d1b8b081f3f) )
ROM_END

ROM_START( gradius3j )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "945_s13.f15", 0x00000, 0x20000, CRC(70c240a2) SHA1(82dc391572e1f61b0182cb031654d71adcdd5f6e) )
	ROM_LOAD16_BYTE( "945_s12.e15", 0x00001, 0x20000, CRC(bbc300d4) SHA1(e1ca98bc591575285d7bd2d4fefdf35fed10dcb6) )

	ROM_REGION( 0x100000, "sub", 0 )
	ROM_LOAD16_BYTE( "945_m09.r17",  0x000000, 0x20000, CRC(b4a6df25) SHA1(85533cf140d28f6f81c0b49b8061bda0924a613a) )
	ROM_LOAD16_BYTE( "945_m08.n17",  0x000001, 0x20000, CRC(74e981d2) SHA1(e7b47a2da01ff73293d2100c48fdf00b33125af5) )
	ROM_LOAD16_BYTE( "945_l06b.r11", 0x040000, 0x20000, CRC(83772304) SHA1(a90c75a3de670b6ec5e0fc201876d463b4a76766) )
	ROM_LOAD16_BYTE( "945_l06a.n11", 0x040001, 0x20000, CRC(e1fd75b6) SHA1(6160d80a2f1bf550e85d6253cf521a96f5a644cc) )
	ROM_LOAD16_BYTE( "945_l07c.r15", 0x080000, 0x20000, CRC(c1e399b6) SHA1(e95bd478dd3beea0175bf9ee4cededb111c4ace1) )
	ROM_LOAD16_BYTE( "945_l07a.n15", 0x080001, 0x20000, CRC(96222d04) SHA1(b55700f683a556b0e73dbac9c7b4ce485420d21c) )
	ROM_LOAD16_BYTE( "945_l07d.r13", 0x0c0000, 0x20000, CRC(4c16d4bd) SHA1(01dcf169b78a1e495214b10181401d1920b0c924) )
	ROM_LOAD16_BYTE( "945_l07b.n13", 0x0c0001, 0x20000, CRC(5e209d01) SHA1(0efa1bbfdc7e2ba1e0bb96245e2bfe961258b446) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "945_m05.d9", 0x00000, 0x10000, CRC(c8c45365) SHA1(b9a7b736b52bca42c7b8c8ed64c8df73e0116158) )

	ROM_REGION( 0x200000, "k051960", 0 )   /* graphics (addressable by the main CPU) */
	ROM_LOAD32_WORD( "945_a02.l3",  0x000000, 0x80000, CRC(4dfffd74) SHA1(588210bac27448240ef08961f70b714b69cb3ffd) )
	ROM_LOAD32_WORD( "945_a01.h3",  0x000002, 0x80000, CRC(339d6dd2) SHA1(6a52b826aba92c75fc6a5926184948735dc20812) )
	ROM_LOAD32_BYTE( "945_l04a.k6", 0x100000, 0x20000, CRC(884e21ee) SHA1(ce86dd3a06775e5b1aa09db010dcb674e67828e7) )
	ROM_LOAD32_BYTE( "945_l04c.m6", 0x100001, 0x20000, CRC(45bcd921) SHA1(e51a8a71362a6fb55124aa1dce74519c0a3c6e3f) )
	ROM_LOAD32_BYTE( "945_l03a.e6", 0x100002, 0x20000, CRC(a67ef087) SHA1(fd63474f3bbde5dfc53ed4c1db25d6411a8b54d2) )
	ROM_LOAD32_BYTE( "945_l03c.h6", 0x100003, 0x20000, CRC(a56be17a) SHA1(1d387736144c30fcb5de54235331ab1ff70c356e) )
	ROM_LOAD32_BYTE( "945_l04b.k8", 0x180000, 0x20000, CRC(843bc67d) SHA1(cdf8421083f24ab27867ed5d08d8949da192b2b9) )
	ROM_LOAD32_BYTE( "945_l04d.m8", 0x180001, 0x20000, CRC(0a98d08e) SHA1(1e0ca51a2d45c01fa3f11950ddd387f41ddae691) )
	ROM_LOAD32_BYTE( "945_l03b.e8", 0x180002, 0x20000, CRC(933e68b9) SHA1(f3a39446ca77d17fdbd938bd5f718ae9d5570879) )
	ROM_LOAD32_BYTE( "945_l03d.h8", 0x180003, 0x20000, CRC(f375e87b) SHA1(6427b966795c907c8e516244872fe52217da62c4) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "945l14.j28", 0x0000, 0x0100, CRC(c778c189) SHA1(847eaf379ba075c25911c6f83dd63ff390534f60) )  /* priority encoder (not used) */

	ROM_REGION( 0x80000, "k007232", 0 ) /* 007232 samples */
	ROM_LOAD( "945_a10.b15",  0x00000, 0x40000, CRC(1d083e10) SHA1(b116f133a7647ef7a6c373aff00e9622d9954b61) )
	ROM_LOAD( "945_l11a.c18", 0x40000, 0x20000, CRC(6043f4eb) SHA1(1c2e9ace1cfdde504b7b6158e3c3f54dc5ae33d4) )
	ROM_LOAD( "945_l11b.c20", 0x60000, 0x20000, CRC(89ea3baf) SHA1(8edcbaa7969185cfac48c02559826d1b8b081f3f) )
ROM_END

ROM_START( gradius3js )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "945_s13.f15", 0x00000, 0x20000, CRC(70c240a2) SHA1(82dc391572e1f61b0182cb031654d71adcdd5f6e) )
	ROM_LOAD16_BYTE( "945_s12.e15", 0x00001, 0x20000, CRC(bbc300d4) SHA1(e1ca98bc591575285d7bd2d4fefdf35fed10dcb6) )

	ROM_REGION( 0x100000, "sub", 0 )
	ROM_LOAD16_BYTE( "945_m09.r17",  0x000000, 0x20000, CRC(b4a6df25) SHA1(85533cf140d28f6f81c0b49b8061bda0924a613a) )
	ROM_LOAD16_BYTE( "945_m08.n17",  0x000001, 0x20000, CRC(74e981d2) SHA1(e7b47a2da01ff73293d2100c48fdf00b33125af5) )
	ROM_LOAD16_BYTE( "945_l06b.r11", 0x040000, 0x20000, CRC(83772304) SHA1(a90c75a3de670b6ec5e0fc201876d463b4a76766) )
	ROM_LOAD16_BYTE( "945_l06a.n11", 0x040001, 0x20000, CRC(e1fd75b6) SHA1(6160d80a2f1bf550e85d6253cf521a96f5a644cc) )
	ROM_LOAD16_BYTE( "945_l07c.r15", 0x080000, 0x20000, CRC(c1e399b6) SHA1(e95bd478dd3beea0175bf9ee4cededb111c4ace1) )
	ROM_LOAD16_BYTE( "945_l07a.n15", 0x080001, 0x20000, CRC(96222d04) SHA1(b55700f683a556b0e73dbac9c7b4ce485420d21c) )
	ROM_LOAD16_BYTE( "945_l07d.r13", 0x0c0000, 0x20000, CRC(4c16d4bd) SHA1(01dcf169b78a1e495214b10181401d1920b0c924) )
	ROM_LOAD16_BYTE( "945_l07b.n13", 0x0c0001, 0x20000, CRC(5e209d01) SHA1(0efa1bbfdc7e2ba1e0bb96245e2bfe961258b446) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "945_m05.d9", 0x00000, 0x10000, CRC(c8c45365) SHA1(b9a7b736b52bca42c7b8c8ed64c8df73e0116158) )

	ROM_REGION( 0x200000, "k051960", 0 )   /* graphics (addressable by the main CPU) */
	ROM_LOAD32_BYTE( "945_A02A.K2", 0x000000, 0x20000, CRC(fbb81511) SHA1(e7988d52e323e46117f5080c469daf9b119f28a0) )
	ROM_LOAD32_BYTE( "945_A02C.M2", 0x000001, 0x20000, CRC(031b55e8) SHA1(64ed8dee60bf012df7c1ed496af1c75263c052a6) )
	ROM_LOAD32_BYTE( "945_A01A.E2", 0x000002, 0x20000, CRC(bace5abb) SHA1(b32df63294c0730f463335b1b760494389c60062) )
	ROM_LOAD32_BYTE( "945_A01C.H2", 0x000003, 0x20000, CRC(d91b29a6) SHA1(0c3027a08996f4c2b86dd88695241b21c8dffd64) )
	ROM_LOAD32_BYTE( "945_A02B.K4", 0x080000, 0x20000, CRC(c0fed4ab) SHA1(f01975b13759cae7c8dfd24f9b3f4ac960d32957) )
	ROM_LOAD32_BYTE( "945_A02D.M4", 0x080001, 0x20000, CRC(d462817c) SHA1(00137e38454e7c3548a1a9553c5ee644916b3959) )
	ROM_LOAD32_BYTE( "945_A01B.E4", 0x080002, 0x20000, CRC(b426090e) SHA1(06a671a648e3255146fe0c325d5451d4f75f08aa) )
	ROM_LOAD32_BYTE( "945_A01D.H4", 0x080003, 0x20000, CRC(3990c09a) SHA1(1f6a089c1d03fb95d4d96fecc0379bde26ee2b9d) )

	ROM_LOAD32_BYTE( "945_l04a.k6", 0x100000, 0x20000, CRC(884e21ee) SHA1(ce86dd3a06775e5b1aa09db010dcb674e67828e7) )
	ROM_LOAD32_BYTE( "945_l04c.m6", 0x100001, 0x20000, CRC(45bcd921) SHA1(e51a8a71362a6fb55124aa1dce74519c0a3c6e3f) )
	ROM_LOAD32_BYTE( "945_l03a.e6", 0x100002, 0x20000, CRC(a67ef087) SHA1(fd63474f3bbde5dfc53ed4c1db25d6411a8b54d2) )
	ROM_LOAD32_BYTE( "945_l03c.h6", 0x100003, 0x20000, CRC(a56be17a) SHA1(1d387736144c30fcb5de54235331ab1ff70c356e) )
	ROM_LOAD32_BYTE( "945_l04b.k8", 0x180000, 0x20000, CRC(843bc67d) SHA1(cdf8421083f24ab27867ed5d08d8949da192b2b9) )
	ROM_LOAD32_BYTE( "945_l04d.m8", 0x180001, 0x20000, CRC(0a98d08e) SHA1(1e0ca51a2d45c01fa3f11950ddd387f41ddae691) )
	ROM_LOAD32_BYTE( "945_l03b.e8", 0x180002, 0x20000, CRC(933e68b9) SHA1(f3a39446ca77d17fdbd938bd5f718ae9d5570879) )
	ROM_LOAD32_BYTE( "945_l03d.h8", 0x180003, 0x20000, CRC(f375e87b) SHA1(6427b966795c907c8e516244872fe52217da62c4) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "945l14.j28", 0x0000, 0x0100, CRC(c778c189) SHA1(847eaf379ba075c25911c6f83dd63ff390534f60) )  /* priority encoder (not used) */

	ROM_REGION( 0x80000, "k007232", 0 ) /* 007232 samples */
	ROM_LOAD( "945_A10A.C14", 0x00000, 0x20000, CRC(ec717414) SHA1(8c63d5fe01d0833529fca91bc80cdbd8a04174c0) )
	ROM_LOAD( "945_A10B.C16", 0x20000, 0x20000, CRC(709e30e4) SHA1(27fcea720cd2498f1870c9290d30dcb3dd81d5e5) )
	ROM_LOAD( "945_l11a.c18", 0x40000, 0x20000, CRC(6043f4eb) SHA1(1c2e9ace1cfdde504b7b6158e3c3f54dc5ae33d4) )
	ROM_LOAD( "945_l11b.c20", 0x60000, 0x20000, CRC(89ea3baf) SHA1(8edcbaa7969185cfac48c02559826d1b8b081f3f) )
ROM_END

ROM_START( gradius3a )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "945_13.f15", 0x00000, 0x20000, CRC(9974fe6b) SHA1(c18ad8d7c93bf58d886715d8e210177cf49f220b) )
	ROM_LOAD16_BYTE( "945_12.e15", 0x00001, 0x20000, CRC(e9771b91) SHA1(c9f4610b897c13742b44b546e2bed8ee21945f61) )

	ROM_REGION( 0x100000, "sub", 0 )
	ROM_LOAD16_BYTE( "945_m09.r17",  0x000000, 0x20000, CRC(b4a6df25) SHA1(85533cf140d28f6f81c0b49b8061bda0924a613a) )
	ROM_LOAD16_BYTE( "945_m08.n17",  0x000001, 0x20000, CRC(74e981d2) SHA1(e7b47a2da01ff73293d2100c48fdf00b33125af5) )
	ROM_LOAD16_BYTE( "945_l06b.r11", 0x040000, 0x20000, CRC(83772304) SHA1(a90c75a3de670b6ec5e0fc201876d463b4a76766) )
	ROM_LOAD16_BYTE( "945_l06a.n11", 0x040001, 0x20000, CRC(e1fd75b6) SHA1(6160d80a2f1bf550e85d6253cf521a96f5a644cc) )
	ROM_LOAD16_BYTE( "945_l07c.r15", 0x080000, 0x20000, CRC(c1e399b6) SHA1(e95bd478dd3beea0175bf9ee4cededb111c4ace1) )
	ROM_LOAD16_BYTE( "945_l07a.n15", 0x080001, 0x20000, CRC(96222d04) SHA1(b55700f683a556b0e73dbac9c7b4ce485420d21c) )
	ROM_LOAD16_BYTE( "945_l07d.r13", 0x0c0000, 0x20000, CRC(4c16d4bd) SHA1(01dcf169b78a1e495214b10181401d1920b0c924) )
	ROM_LOAD16_BYTE( "945_l07b.n13", 0x0c0001, 0x20000, CRC(5e209d01) SHA1(0efa1bbfdc7e2ba1e0bb96245e2bfe961258b446) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "945_m05.d9", 0x00000, 0x10000, CRC(c8c45365) SHA1(b9a7b736b52bca42c7b8c8ed64c8df73e0116158) )

	ROM_REGION( 0x200000, "k051960", 0 )   /* graphics (addressable by the main CPU) */
	ROM_LOAD32_WORD( "945_a02.l3",  0x000000, 0x80000, CRC(4dfffd74) SHA1(588210bac27448240ef08961f70b714b69cb3ffd) )
	ROM_LOAD32_WORD( "945_a01.h3",  0x000002, 0x80000, CRC(339d6dd2) SHA1(6a52b826aba92c75fc6a5926184948735dc20812) )
	ROM_LOAD32_BYTE( "945_l04a.k6", 0x100000, 0x20000, CRC(884e21ee) SHA1(ce86dd3a06775e5b1aa09db010dcb674e67828e7) )
	ROM_LOAD32_BYTE( "945_l04c.m6", 0x100001, 0x20000, CRC(45bcd921) SHA1(e51a8a71362a6fb55124aa1dce74519c0a3c6e3f) )
	ROM_LOAD32_BYTE( "945_l03a.e6", 0x100002, 0x20000, CRC(a67ef087) SHA1(fd63474f3bbde5dfc53ed4c1db25d6411a8b54d2) )
	ROM_LOAD32_BYTE( "945_l03c.h6", 0x100003, 0x20000, CRC(a56be17a) SHA1(1d387736144c30fcb5de54235331ab1ff70c356e) )
	ROM_LOAD32_BYTE( "945_l04b.k8", 0x180000, 0x20000, CRC(843bc67d) SHA1(cdf8421083f24ab27867ed5d08d8949da192b2b9) )
	ROM_LOAD32_BYTE( "945_l04d.m8", 0x180001, 0x20000, CRC(0a98d08e) SHA1(1e0ca51a2d45c01fa3f11950ddd387f41ddae691) )
	ROM_LOAD32_BYTE( "945_l03b.e8", 0x180002, 0x20000, CRC(933e68b9) SHA1(f3a39446ca77d17fdbd938bd5f718ae9d5570879) )
	ROM_LOAD32_BYTE( "945_l03d.h8", 0x180003, 0x20000, CRC(f375e87b) SHA1(6427b966795c907c8e516244872fe52217da62c4) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "945l14.j28", 0x0000, 0x0100, CRC(c778c189) SHA1(847eaf379ba075c25911c6f83dd63ff390534f60) )  /* priority encoder (not used) */

	ROM_REGION( 0x80000, "k007232", 0 ) /* 007232 samples */
	ROM_LOAD( "945_a10.b15",  0x00000, 0x40000, CRC(1d083e10) SHA1(b116f133a7647ef7a6c373aff00e9622d9954b61) )
	ROM_LOAD( "945_l11a.c18", 0x40000, 0x20000, CRC(6043f4eb) SHA1(1c2e9ace1cfdde504b7b6158e3c3f54dc5ae33d4) )
	ROM_LOAD( "945_l11b.c20", 0x60000, 0x20000, CRC(89ea3baf) SHA1(8edcbaa7969185cfac48c02559826d1b8b081f3f) )
ROM_END



GAME( 1989, gradius3,  0,        gradius3, gradius3, driver_device, 0, ROT0, "Konami", "Gradius III (World, program code R)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, gradius3j, gradius3, gradius3, gradius3, driver_device, 0, ROT0, "Konami", "Gradius III (Japan, program code S)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, gradius3js, gradius3, gradius3, gradius3, driver_device, 0, ROT0, "Konami", "Gradius III (Japan, program code S, split)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, gradius3a, gradius3, gradius3, gradius3, driver_device, 0, ROT0, "Konami", "Gradius III (Asia)", MACHINE_SUPPORTS_SAVE )
