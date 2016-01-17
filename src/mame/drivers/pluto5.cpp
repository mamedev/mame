// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

    Pluto 5

    Skeleton Driver - For note keeping

    THIS SYSTEM IS NOT EMULATED!!!

    68340 based system like MPU5/SC4?
    used by JPM (Post SEGA buyout)? + Others? Manufactured by Heber Ltd.

    this seems very close to astrafr.c
    (although these contain more text)

    Known games
    Club DNA?


    I'm not sure if the sets in this driver are actually this platform
    it's just based guessed on structure*, and some ebay notes which
    inidicate at least one or two should be..

    * 2 roms only, with what looks to be some modified address lines,
      and the roms containing a large amount of data, presmuably because
      they have the sound integrated, and the sound hw is just DAC style


  ---------------------------------------------------------------------

  Pluto 5 Technical Notes....
   (why is this information here? it seems to be c+p straight from a tech manual
    and is completely unneccessary)


  * Clocks...

  - Main Clock, EXTAL for MC68340 Processor @32.768kHz.
  - MC68340 Serial Module @3.6864MHz.
  - OKI MSM6585 devices, U8/39 @640KHz.


  * EPROMs...

  The 2 EPROM positions, U1 and U2, are configured such that 4 possible configurations
  of programme memory are possible (assuming no external memory expansion via P15):

  Possible EPROM Configurations...

     U1   |   U2   |  Mode  | Configuration | Total Size | Addresses Scrambled
  --------+--------+--------+---------------+------------+---------------------
   27C040 |  omit  | 8 bit  |    512k*8     |  512Kbyte  |        no
  --------+--------+--------+---------------+------------+---------------------
   27C040 | 27C040 | 16 bit |    512K*16    |   1Mbyte   |        yes
  --------+--------+--------+---------------+------------+---------------------
   27C801 |  omit  | 8 bit  |    1024k*8    |   1Mbyte   |        no
  --------+--------+--------+---------------+------------+---------------------
   27C801 | 27C801 | 16 bit |    1024k*16   |   2Mbyte   |        yes
  --------+--------+--------+---------------+------------+---------------------


  EPROM Address Line Scrambling in 16 Bit Mode
  2*27C040 EPROMs

  In 16 bit mode, running with 2 * 27C040 EPROMs, the scrambling of the address lines cause the
  following effect on the memory mapping in the EPROMs. Note that this table applies to the re-
  mapping that occurs to the EPROM contents, rather than the actual address lines.


  Re-Mapping of Address Lines in 2*27C040 Mode:

  68340 Address Bus |      EPROM Address
  ------------------+---------------------------
   A0               | (not used in 16 bit mode)
  ------------------+---------------------------
   A1-A18           | A2-A19
  ------------------+---------------------------
   A19              | A1
  ------------------+---------------------------


  Re-Mapping of EPROM Contents in 2*27C040 Mode:

  68340 Access Address | Will Read From This Location in EPROM
  ---------------------+---------------------------------------
   0000 0000           | 0000 0000
  ---------------------+---------------------------------------
   0000 0002           | 0000 0004
  ---------------------+---------------------------------------
   0000 0004           | 0000 0008
  ---------------------+---------------------------------------
   0000 0006           | 0000 000C
  ---------------------+---------------------------------------
   0000 0008           | 0000 0010
  ---------------------+---------------------------------------
  ---------------------+---------------------------------------
   0007 FFFC           | 000F FFF8
  ---------------------+---------------------------------------
   0007 FFFE           | 000F FFFC
  ---------------------+---------------------------------------
   0008 0000           | 0000 0002
  ---------------------+---------------------------------------
   0008 0002           | 0000 0006
  ---------------------+---------------------------------------
  ---------------------+---------------------------------------
   000F FFFC           | 000F FFFA
  ---------------------+---------------------------------------
   000F FFFE           | 000F FFFE
  ---------------------+---------------------------------------


  2*27C801 EPROMs

  In 16 bit mode, running with 2 * 27C801 EPROMs, the scrambling of the address lines cause the
  following effect on the memory mapping in the EPROMs. Note that this table applies to the re-
  mapping that occurs to the EPROM contents, rather than the actual address lines.

  Re-Mapping of Address Lines in 2*27C801 Mode:

  68340 Address Bus |      EPROM Address
  ------------------+---------------------------
   A0               | (not used in 16 bit mode)
  ------------------+---------------------------
   A1-A18           | A2-A19
  ------------------+---------------------------
   A19              | A1
  ------------------+---------------------------
   A20              | A20
  ------------------+---------------------------


  Re-Mapping of EPROM Contents in 2*27C801 Mode:

  68340 Access Address | Will Read From This Location in EPROM
  ---------------------+---------------------------------------
   0000 0000           | 0000 0000
  ---------------------+---------------------------------------
   0000 0002           | 0000 0004
  ---------------------+---------------------------------------
   0000 0004           | 0000 0008
  ---------------------+---------------------------------------
  ---------------------+---------------------------------------
   0007 FFFC           | 000F FFF8
  ---------------------+---------------------------------------
   0007 FFFE           | 000F FFFC
  ---------------------+---------------------------------------
   0008 0000           | 0000 0002
  ---------------------+---------------------------------------
   0008 0002           | 0000 0006
  ---------------------+---------------------------------------
  ---------------------+---------------------------------------
   000F FFFC           | 000F FFFA
  ---------------------+---------------------------------------
   000F FFFE           | 000F FFFA
  ---------------------+---------------------------------------
   0010 0000           | 0010 0000
  ---------------------+---------------------------------------
   0010 0002           | 0010 0004
  ---------------------+---------------------------------------
   0010 0004           | 0010 0008
  ---------------------+---------------------------------------
  ---------------------+---------------------------------------
   0017 FFFC           | 001F FFF8
  ---------------------+---------------------------------------
   0017 FFFE           | 001F FFFC
  ---------------------+---------------------------------------
   0018 0000           | 0010 0002
  ---------------------+---------------------------------------
   0018 0002           | 0010 0006
  ---------------------+---------------------------------------
  ---------------------+---------------------------------------
   001F FFFC           | 001F FFFA
  ---------------------+---------------------------------------
   001F FFFE           | 001F FFFA
  ---------------------+---------------------------------------

*/

#include "emu.h"
#include "machine/68340.h"

class pluto5_state : public driver_device
{
public:
	pluto5_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu")
	{ }

	UINT32* m_cpuregion;
	std::unique_ptr<UINT32[]> m_mainram;

	DECLARE_READ32_MEMBER(pluto5_mem_r);
	DECLARE_WRITE32_MEMBER(pluto5_mem_w);

protected:

	// devices
	required_device<m68340cpu_device> m_maincpu;
public:
	DECLARE_DRIVER_INIT(hb);
	virtual void machine_start() override;
};

READ32_MEMBER(pluto5_state::pluto5_mem_r)
{
	int pc = space.device().safe_pc();
	int cs = m68340_get_cs(m_maincpu, offset * 4);

	switch ( cs )
	{
		case 1:if (offset < 0x100000) // If reading beyond end of region, log error instead of crashing
			return m_cpuregion[offset];

		default:
			logerror("%08x maincpu read access offset %08x mem_mask %08x cs %d\n", pc, offset*4, mem_mask, cs);

	}

	return 0x0000;
}

WRITE32_MEMBER(pluto5_state::pluto5_mem_w)
{
	int pc = space.device().safe_pc();
	int cs = m68340_get_cs(m_maincpu, offset * 4);

	switch ( cs )
	{
		default:
			logerror("%08x maincpu write access offset %08x data %08x mem_mask %08x cs %d\n", pc, offset*4, data, mem_mask, cs);

	}

}


static ADDRESS_MAP_START( pluto5_map, AS_PROGRAM, 32, pluto5_state )
	AM_RANGE(0x00000000, 0xffffffff) AM_READWRITE(pluto5_mem_r, pluto5_mem_w)
ADDRESS_MAP_END

static INPUT_PORTS_START(  pluto5 )
INPUT_PORTS_END

void pluto5_state::machine_start()
{
	m_cpuregion = (UINT32*)memregion( "maincpu" )->base();
	m_mainram = make_unique_clear<UINT32[]>(0x10000);

}

static MACHINE_CONFIG_START( pluto5, pluto5_state )
	MCFG_CPU_ADD("maincpu", M68340, 16000000)
	MCFG_CPU_PROGRAM_MAP(pluto5_map)



	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	/* unknown sound */
MACHINE_CONFIG_END

ROM_START( hb_cr )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "80000504.p1", 0x00000, 0x080000, CRC(a982dd06) SHA1(f3de6ec7d538709961d8e1118c29ef3515415b25) )
	ROM_LOAD16_BYTE( "80000504.p2", 0x00001, 0x080000, CRC(76e867e3) SHA1(a088029c3a8fdbbc2b90ae3e33a07e41dcf7daee) )
ROM_END

ROM_START( hb_cra )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "80000550.p1", 0x00000, 0x080000, CRC(d800e216) SHA1(deb65ceec634a006dc510f908bccb383cbd3fadf) )
	ROM_LOAD16_BYTE( "80000550.p2", 0x00001, 0x080000, CRC(dede7764) SHA1(96ce329a6351c5d6a5d434cea67b3bb7a26d443e) )
ROM_END

ROM_START( hb_crb )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "80001504.p1", 0x00000, 0x080000, CRC(13d32b07) SHA1(aa263d17aab0e1155d020bcebfaa8dea31a525e9) )
	ROM_LOAD16_BYTE( "80001504.p2", 0x00001, 0x080000, CRC(8e3687b3) SHA1(d80c79c8283927e206437f4ff3208d1be00c2558) )
ROM_END


ROM_START( hb_bar7 )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "bar_seven-catcd-ver9.14-fpp5u1.bin", 0x00000, 0x080000, CRC(56156c92) SHA1(570a7d9c497a2b162fe833cf200afccc3b253598) )
	ROM_LOAD16_BYTE( "bar_seven-catcd-ver9.14-fpp5u2.bin", 0x00001, 0x080000, CRC(7182cd57) SHA1(00838ce0a11a30e0bb1a803cfbc3c8b4489a4fac) )
ROM_END

ROM_START( hb_bar7a )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "bar_seven-catcd-ver9.14p5u1.bin", 0x00000, 0x080000, CRC(6cbd4bf8) SHA1(0d40f37a16e5802fe5d6c9bdc51abf6842b98fdd) )
	ROM_LOAD16_BYTE( "bar_seven-catcd-ver9.14p5u2.bin", 0x00001, 0x080000, CRC(17e9408a) SHA1(85523c2730d573b6f4325ce0fbf5f0c719207f8f) )
ROM_END

ROM_START( hb_bigx )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "ccb9", 0x00001, 0x080000, CRC(6945622a) SHA1(d575b0083fe56694e818bb220d86ec5122dff8b3) )
	ROM_LOAD16_BYTE( "c2fe", 0x00000, 0x080000, CRC(080a7bc9) SHA1(0a9b41042236ac24df175b3577e38a4179b4ad5f) )
ROM_END

ROM_START( hb_bigxa )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "ccb9", 0x00001, 0x080000, CRC(6945622a) SHA1(d575b0083fe56694e818bb220d86ec5122dff8b3) )
	ROM_LOAD16_BYTE( "c302", 0x00000, 0x080000, CRC(ce4c57fe) SHA1(9ef22594352e37f9314f62d0dd85d2a16d4d4510) )
ROM_END

ROM_START( hb_bigxb )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "ccb9", 0x00001, 0x080000, CRC(6945622a) SHA1(d575b0083fe56694e818bb220d86ec5122dff8b3) )
	ROM_LOAD16_BYTE( "c304", 0x00000, 0x080000, CRC(40d7c2c5) SHA1(28ca00df9e714332993deb4963172c58b173e301) )
ROM_END

ROM_START( hb_bigxc )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "ccb9", 0x00001, 0x080000, CRC(6945622a) SHA1(d575b0083fe56694e818bb220d86ec5122dff8b3) )
	ROM_LOAD16_BYTE( "c305", 0x00000, 0x080000, CRC(ea228b78) SHA1(db59c1ef33038065e9c7b9da65e8173f479e6a63) )
ROM_END

ROM_START( hb_bigxd )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "ccb9", 0x00001, 0x080000, CRC(6945622a) SHA1(d575b0083fe56694e818bb220d86ec5122dff8b3) )
	ROM_LOAD16_BYTE( "c306", 0x00000, 0x080000, CRC(3aa14e2c) SHA1(ad1b74ac65b7a338abe966663b819f912a40b6bf) )
ROM_END

ROM_START( hb_ccow )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "80000013.p1", 0x00000, 0x080000, CRC(b347425b) SHA1(168e18a48ab69e02c9e35c5650558e2c47a8fb3e) )
	ROM_LOAD16_BYTE( "80000013.p2", 0x00001, 0x080000, CRC(cae6b86f) SHA1(c9704a0bdcb552505cd5184a73b1774cc3972f20) )
ROM_END

ROM_START( hb_ccowa )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "80000600.p1", 0x00000, 0x080000, CRC(a3c8b2b1) SHA1(b62e8dc5341590f7d7c9bbd72cfffd95a5504b51) )
	ROM_LOAD16_BYTE( "80000600.p2", 0x00001, 0x080000, CRC(4c7a8bf3) SHA1(6547c25217f1c50f6156d3809bbe53fcf0dec7d9) )
ROM_END

ROM_START( hb_ccowb )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "80001013.p1", 0x00000, 0x080000, CRC(5bb0f962) SHA1(01610111df939b4acb18838b4e38e8ec4dc7ffc5) )
	ROM_LOAD16_BYTE( "80001013.p2", 0x00001, 0x080000, CRC(cea8801f) SHA1(af6924917c4ea26d32a46f0257233a9912aefac5) )
ROM_END

ROM_START( hb_cashc )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "80000410.p1", 0x00000, 0x080000, CRC(6c65816d) SHA1(f9b9c755ec41209742bad688f02f0411631f5c03) )
	ROM_LOAD16_BYTE( "80000410.p2", 0x00001, 0x080000, CRC(1b9ec71f) SHA1(97ad99a10b65b6db73ef97694c7aadcd3fdcb874) )
ROM_END

ROM_START( hb_cashca )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "80000750.p1", 0x00000, 0x080000, CRC(e75ae304) SHA1(3e03f76c8777fc19bc2e968cc39ebb1731410dd9) )
	ROM_LOAD16_BYTE( "80000750.p2", 0x00001, 0x080000, CRC(260cc135) SHA1(b2e71314f077076d1e02586962db8be32c981c33) )
ROM_END

ROM_START( hb_cashcb )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "80001410.p1", 0x00000, 0x080000, CRC(044377a6) SHA1(14ef68e3c718c0c664c903f9f077cbe25b261a8b) )
	ROM_LOAD16_BYTE( "80001410.p2", 0x00001, 0x080000, CRC(05b7b38c) SHA1(da2dd2a7674242f26446b80970919b8905f36cda) )
ROM_END

ROM_START( hb_cashx )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "cashx-catcd-ver5.09-fpp5u1.bin", 0x00000, 0x080000, CRC(1d00384c) SHA1(dec9344907ae82309fd8f9db002c9ab562ddfa65) )
	ROM_LOAD16_BYTE( "cashx-catcd-ver5.09-fpp5u2.bin", 0x00001, 0x080000, CRC(82cb901b) SHA1(9c5b8f1d52242594cf3cf7939ddc2fd28903bca4) )
ROM_END

ROM_START( hb_cashxa )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "cashx-catcd-ver5.09p5u1.bin", 0x00000, 0x080000, CRC(af32ebbc) SHA1(15e4814e34072d5c9e95e325e98d480c37738115) )
	ROM_LOAD16_BYTE( "cashx-catcd-ver5.09p5u2.bin", 0x00001, 0x080000, CRC(e41d9e2f) SHA1(5e1633fb2f2a05dbd21471ae1fd0bf827c9e7229) )
ROM_END

ROM_START( hb_cwf )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "cherry_win_falls-catcd-ver9.08-fpp5u1.bin", 0x00000, 0x080000, CRC(4fc3fb09) SHA1(fde18838750081412770009e06afeadc36fc7c1f) )
	ROM_LOAD16_BYTE( "cherry_win_falls-catcd-ver9.08-fpp5u2.bin", 0x00001, 0x080000, CRC(3cfaa8a6) SHA1(f7ff5e4438e551c8793da33acf5c2101617cb589) )
ROM_END

ROM_START( hb_cwfa )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "cherry_win_falls-catcd-ver9.08p5u1.bin", 0x00000, 0x080000, CRC(8e17adc3) SHA1(8473cb5c3307089870231dc851eb9601964105f9) )
	ROM_LOAD16_BYTE( "cherry_win_falls-catcd-ver9.08p5u2.bin", 0x00001, 0x080000, CRC(5337ab1b) SHA1(a70f46984ea53301b4b2512366761ae1d939cb8b) )
ROM_END

ROM_START( hb_dac )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "80000156.p1", 0x00000, 0x100000, CRC(8f1a15ce) SHA1(9e2164bd098c8d34316e79c121769bff83dff3c3) )
	ROM_LOAD16_BYTE( "80000156.p2", 0x00001, 0x100000, CRC(284ac350) SHA1(c0289bbbdab09701d763245a0c3e1f320cfe1798) )
ROM_END

ROM_START( hb_daca )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "80000158.p1", 0x00000, 0x100000, CRC(b1c4bb9e) SHA1(056efaa9f0f792bfcc9f587c8bf48cb68cc582da) )
	ROM_LOAD16_BYTE( "80000158.p2", 0x00001, 0x100000, CRC(1b61d53d) SHA1(f5ce8a60fc9859676bd62a243fe773a11b9f2b41) )
ROM_END

ROM_START( hb_dacb )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "80000162.p1", 0x00000, 0x100000, CRC(958b714d) SHA1(428c9b79388e02e4f808e5e5b426805ee44404cd) )
	ROM_LOAD16_BYTE( "80000162.p2", 0x00001, 0x100000, CRC(789843c2) SHA1(ead228182177078f1cf3559a03ae0c04eb5e26e9) )
ROM_END

ROM_START( hb_dacc )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "80000164.p1", 0x00000, 0x100000, CRC(a5f7561b) SHA1(25e5e4d2ba84547a5b87ee4af46ebfb11076978b) )
	ROM_LOAD16_BYTE( "80000164.p2", 0x00001, 0x100000, CRC(ebd494e0) SHA1(fb1ba4e6fcf4d445593b27e402bf2b5f8826d988) )
ROM_END

ROM_START( hb_dacd )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "80000650.p1", 0x00000, 0x100000, CRC(f331bebf) SHA1(efe035a4e22a28f02dd15221236c99744e9e8d52) )
	ROM_LOAD16_BYTE( "80000650.p2", 0x00001, 0x100000, CRC(dcaaf2e6) SHA1(621ed3d1895b26c27ebc73cb0cbd7b4be1543ba8) )
ROM_END

ROM_START( hb_dace )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "80001156.p1", 0x00000, 0x100000, CRC(2542c2a4) SHA1(8cdad24fc0bb8a6600ba0689a7f6ed8079e6151c) )
	ROM_LOAD16_BYTE( "80001156.p2", 0x00001, 0x100000, CRC(587c013b) SHA1(39568445cea2775e928e5fc8eb7de3b1e717b471) )
ROM_END

ROM_START( hb_dacf )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "80001162.p1", 0x00000, 0x100000, CRC(7392dd9e) SHA1(62d2d08918eded99800b5611a61fa9deae558b76) )
	ROM_LOAD16_BYTE( "80001162.p2", 0x00001, 0x100000, CRC(8e850ddd) SHA1(4469d6a3ba36533053051e6e56300db618ff4030) )
ROM_END

ROM_START( hb_dacg )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "80001164.p1", 0x00000, 0x100000, CRC(a45a1322) SHA1(e97310d072cddef8e344ee3b32b3fe2d2902239b) )
	ROM_LOAD16_BYTE( "80001164.p2", 0x00001, 0x100000, CRC(01129a24) SHA1(8c567f1462b44d82da4dfa8a463e79586c446ed5) )
ROM_END

ROM_START( hb_dacz )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 ) // check if these match the start of other roms, if so, remove
	ROM_LOAD16_BYTE( "dough_cl.p1", 0x00000, 0x020000, BAD_DUMP CRC(c8833628) SHA1(ca797fd77c8c9993dc6411bc478d07b828afe34f) )
	ROM_LOAD16_BYTE( "dough_cl.p2", 0x00001, 0x020000, BAD_DUMP CRC(94a8141b) SHA1(968dfbe5dc1ec78f896e207beb390af70fccf9af) )
ROM_END

ROM_START( hb_frtcl )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "80000134.p1", 0x00000, 0x100000, CRC(5db61c65) SHA1(fdd354c74c43ab45e975ce14a5c8f4f2c2ecb30f) )
	ROM_LOAD16_BYTE( "80000134.p2", 0x00001, 0x100000, CRC(5bd74e30) SHA1(9c6f58ae0e5ddceb6412447a840f37711514e86f) )
ROM_END

ROM_START( hb_frtcla )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "80000136.p1", 0x00000, 0x100000, CRC(2c412e79) SHA1(c4099e89078ac71bbf640b0cdda20f8990b5025e) )
	ROM_LOAD16_BYTE( "80000136.p2", 0x00001, 0x100000, CRC(13ef57d1) SHA1(7be0494d4085a2146acb4a31b01942cccecb8174) )
ROM_END

ROM_START( hb_frtclb )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "80000138.p1", 0x00000, 0x100000, CRC(52493acf) SHA1(d8af43c960fe3012fb65ec5987bed1253250f9b1) )
	ROM_LOAD16_BYTE( "80000138.p2", 0x00001, 0x100000, CRC(06dc8a18) SHA1(d2f1a899bdb9d9d4485e4d1f28baf63e35c6824d) )
ROM_END

ROM_START( hb_frtclc )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "80000140.p1", 0x00000, 0x100000, CRC(95204a26) SHA1(24b3abcb9c0879ca90ffb0099a9f5d9cc31a6235) )
	ROM_LOAD16_BYTE( "80000140.p2", 0x00001, 0x100000, CRC(6e13b996) SHA1(5b6051fa0433ee49678cb1c5d4e4484a3603ced1) )
ROM_END

ROM_START( hb_frtcld )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "80000142.p1", 0x00000, 0x100000, CRC(dc4bc7f2) SHA1(3277e26a28428eb51d9b4a19cfad10137732f0cb) )
	ROM_LOAD16_BYTE( "80000142.p2", 0x00001, 0x100000, CRC(3d78700c) SHA1(fd8fb59b172200c3cb66cc65c6f138df8531304c) )
ROM_END

ROM_START( hb_frtcle )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "80000144.p1", 0x00000, 0x100000, CRC(b96ddcd7) SHA1(d0b34a906979efa9afd4d0c4c6008c8b1dbfeb50) )
	ROM_LOAD16_BYTE( "80000144.p2", 0x00001, 0x100000, CRC(861c8c53) SHA1(528735234ff9f6855961d1f61590e62176e98083) )
ROM_END

ROM_START( hb_frtclf )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "80000146.p1", 0x00000, 0x100000, CRC(e8ae2c44) SHA1(afc44a5ec1f696a0c1e2e266dfa2d6633802d7ee) ) // club-fruitopia(v5.1)p1.bin
	ROM_LOAD16_BYTE( "80000146.p2", 0x00001, 0x100000, CRC(0a8002fb) SHA1(bf994d69dbe49bc3cc26a5f0d85f59e1947e70bc) ) // club-fruitopia(v5.1)p2.bin
ROM_END

ROM_START( hb_frtclg )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "80000450.p1", 0x00000, 0x100000, CRC(448f47f5) SHA1(c236da3ed6bbac54c7419f0e2e0310d9cb1f6cd3) )
	ROM_LOAD16_BYTE( "80000450.p2", 0x00001, 0x100000, CRC(718c2385) SHA1(9c9ebdbafef95df7425ab57fa7e399281cbd59e8) )
ROM_END

ROM_START( hb_frtclh )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "80001132.p1", 0x00000, 0x100000, CRC(e0726a36) SHA1(1ede60980899bce463c787f091b8b9a0337fe7b6) )
	ROM_LOAD16_BYTE( "80001132.p2", 0x00001, 0x100000, CRC(d8083ea0) SHA1(198f84358e47eda33e377bb50050e52527bfd4a1) )
ROM_END

ROM_START( hb_frtcli )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "80001136.p1", 0x00000, 0x100000, CRC(e8bc986e) SHA1(e0ca394b849d4e672b0d9391c60b66290215f84c) )
	ROM_LOAD16_BYTE( "80001136.p2", 0x00001, 0x100000, CRC(db4ef616) SHA1(b3cda3a2cbc2a99a4720abbb923b33252011d52b) )
ROM_END

ROM_START( hb_frtclj )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "80001138.p1", 0x00000, 0x100000, CRC(5b6ade25) SHA1(8276b723516705099ae2f40a997be502c6717472) )
	ROM_LOAD16_BYTE( "80001138.p2", 0x00001, 0x100000, CRC(7777e0c5) SHA1(8e338bade694f28bddc7dc79d861215582644416) )
ROM_END

ROM_START( hb_frtclk )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "80001140.p1", 0x00000, 0x100000, CRC(46b2f9b2) SHA1(5875ee01bbd832d3a37b756f866970cbe88c07b9) )
	ROM_LOAD16_BYTE( "80001140.p2", 0x00001, 0x100000, CRC(30f07d62) SHA1(91d563e2d9a429d48dce6bb180f08fe8eafb1b95) )
ROM_END

ROM_START( hb_frtcll )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "80001144.p1", 0x00000, 0x100000, CRC(2647cbe1) SHA1(00073a6c0b2d59b63ebee4f1811f0c90dc9985fe) )
	ROM_LOAD16_BYTE( "80001144.p2", 0x00001, 0x100000, CRC(eea7b158) SHA1(bcabc848ea512eaf8e3fef87cd9831671dcccf82) )
ROM_END

ROM_START( hb_frtclm )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "80001146.p1", 0x00000, 0x100000, CRC(b743e34a) SHA1(6420b82ebcb9d1fe5913b82098c8928bcb129d30) )
	ROM_LOAD16_BYTE( "80001146.p2", 0x00001, 0x100000, CRC(a8e09f7e) SHA1(cc65079df900ea354cc67c6e0cc0cba149cfd051) )
ROM_END

ROM_START( hb_frtcln )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "qps club fruitopia v1.10 p1 std 4b38.bin", 0x00000, 0x100000, CRC(c4585e9d) SHA1(baa69d36ad236dfaf1f328897664d6ab6ca244a2) )
	ROM_LOAD16_BYTE( "qps club fruitopia v1.10 p2 std 81a7.bin", 0x00001, 0x100000, CRC(8743b034) SHA1(ca21b8af4ea04c9f0d71a1b9948fef10de762bf6) )
ROM_END

ROM_START( hb_gpal )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "80000094.p1", 0x00000, 0x080000, CRC(a745029a) SHA1(76337b1b075437f325fbc6e0d77e0965dfc9ca25) )
	ROM_LOAD16_BYTE( "80000094.p2", 0x00001, 0x080000, CRC(ae0b8ef7) SHA1(a26c9e9afca6de01b692d8e59f7c486a0c67e964) )
ROM_END

ROM_START( hb_gpala )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "80000096.p1", 0x00000, 0x080000, CRC(dff55c01) SHA1(046e97b23bbaa30d42a1bb8e3eb126ff9ddcda1d) )
	ROM_LOAD16_BYTE( "80000096.p2", 0x00001, 0x080000, CRC(8396b1c7) SHA1(0d5aafb8fb9abe38b23ff1bed53b7a2e7a6aa140) )
ROM_END

ROM_START( hb_gpalb )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "80000104.p1", 0x00000, 0x080000, CRC(9ad3218f) SHA1(5801c303a1a8c323079d6bea5aa5c10c4633f297) )
	ROM_LOAD16_BYTE( "80000104.p2", 0x00001, 0x080000, CRC(b31ef7b0) SHA1(4464764ed160608ec9a7074a93cc8f7296c7b890) )
ROM_END

ROM_START( hb_gpalc )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "80000120.p1", 0x00000, 0x080000, CRC(0b86cf02) SHA1(2f8dd0a65632bcfd42cc11881fe68b0496a55c42) )
	ROM_LOAD16_BYTE( "80000120.p2", 0x00001, 0x080000, CRC(763820e6) SHA1(25813972289c9a3671e448955df859ba10ed564b) )
ROM_END

ROM_START( hb_gpald )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "80000204.p1", 0x00000, 0x080000, CRC(8272b113) SHA1(c509143d15c451cefc99ed3484f699da33b7209c) )
	ROM_LOAD16_BYTE( "80000204.p2", 0x00001, 0x080000, CRC(d64dd22d) SHA1(a5ad7a6a7e8a7facce1269f9b47efc4834a6ab74) )
ROM_END

ROM_START( hb_gpale )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "80000206.p1", 0x00000, 0x080000, CRC(d1b4523c) SHA1(b9d2a2960e5b773f84cb7ce092a09f8db668067e) )
	ROM_LOAD16_BYTE( "80000206.p2", 0x00001, 0x080000, CRC(f27942a7) SHA1(8d41f4db88068e81a21b5d0a050f0ffa2e8de168) )
ROM_END

ROM_START( hb_gpalf )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "80000250.p1", 0x00000, 0x080000, CRC(4d20a315) SHA1(222ea67585808777cceb3438b201023949dd9aee) )
	ROM_LOAD16_BYTE( "80000250.p2", 0x00001, 0x080000, CRC(e3d07ca2) SHA1(1596606c7bf7bb7efa0cf797105faa7418699440) )
ROM_END

ROM_START( hb_gpalg )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "80000354.p1", 0x00000, 0x080000, CRC(2cc23812) SHA1(19911900f2cbb047b9c999ec28a7408a63609633) )
	ROM_LOAD16_BYTE( "80000354.p2", 0x00001, 0x080000, CRC(32aa236f) SHA1(85233a3dd13708b6379f5873b1bfa8bf53bb024e) )
ROM_END

ROM_START( hb_gpalh )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "80000356.p1", 0x00000, 0x080000, CRC(0662c3c8) SHA1(276113405d598ad94dff12400e39c63311944dab) )
	ROM_LOAD16_BYTE( "80000356.p2", 0x00001, 0x080000, CRC(d05bb01b) SHA1(f26fe381c3dd8b6b458d518374c45e4199da5d9e) )
ROM_END

ROM_START( hb_gpali )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "80000358.p1", 0x00000, 0x080000, CRC(6217c5c1) SHA1(351500f37a8c7e4d99d909ef408db13529825665) )
	ROM_LOAD16_BYTE( "80000358.p2", 0x00001, 0x080000, CRC(e9ad652d) SHA1(21d673f7f5fdfd56a145a8ba34acac75021615c7) )
ROM_END

ROM_START( hb_gldpl )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "80000362.p1", 0x00000, 0x080000, CRC(9a5b518c) SHA1(a716d8cf80b4ff4f02a5223cff03c49d8e03cbd5) )
	ROM_LOAD16_BYTE( "80000362.p2", 0x00001, 0x080000, CRC(a3502bc2) SHA1(9c8ae9e239f683c63baee9f9b7182a347890fddc) )
ROM_END

ROM_START( hb_gldpla )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "80000360.p1", 0x00000, 0x080000, CRC(94631ecc) SHA1(72dba58922e50909bbb4d2d8b96e129dbbb27840) )
	ROM_LOAD16_BYTE( "80000360.p2", 0x00001, 0x080000, CRC(ab3336d7) SHA1(fcdff8a48967dbe17a3d085872e2c1bc8390e710) )
ROM_END

ROM_START( hb_gldwn )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "golden_winner-catcd-ver1.05-fpp5u1.bin", 0x00000, 0x080000, CRC(ef036603) SHA1(0aa417c67bd80efadca0777cc2bf7352d57e226e) )
	ROM_LOAD16_BYTE( "golden_winner-catcd-ver1.05-fpp5u2.bin", 0x00001, 0x080000, CRC(402a1b4c) SHA1(19668ab237ad9ce369b340477cfabf79dc9d562d) )
ROM_END

ROM_START( hb_gldwna )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "golden_winner-catcd-ver1.05p5u1.bin", 0x00000, 0x080000, CRC(05cb9394) SHA1(f339f0fd4c04323ef95b2f66539b09e2827005fe) )
	ROM_LOAD16_BYTE( "golden_winner-catcd-ver1.05p5u2.bin", 0x00001, 0x080000, CRC(3cc32619) SHA1(cdb9d2bb493e880295630198c4e2153b32f413f5) )
ROM_END

ROM_START( hb_jailb )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "80000020.p1", 0x00000, 0x080000, CRC(aef3b90f) SHA1(1684004878f5879687f338f6ae316bd474fb238d) )
	ROM_LOAD16_BYTE( "80000020.p2", 0x00001, 0x080000, CRC(183d8013) SHA1(8a2d4247d4413bc090caadb60025232f6f429903) )
ROM_END

ROM_START( hb_jailba )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "80001020.p1", 0x00000, 0x080000, CRC(a0796db8) SHA1(274caf0db05e39bf708054115b36f4aec8cd1dbb) )
	ROM_LOAD16_BYTE( "80001020.p2", 0x00001, 0x080000, CRC(9e3874ba) SHA1(077f226355cb75d794e7e804567c6460bad10d58) )
ROM_END

ROM_START( hb_jkrwl )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "jokerwild-ver1.01-fpp5u1.bin", 0x00000, 0x080000, CRC(4c17331c) SHA1(a6e44e70c73993f4a96623a38ecad7f4606442a8) )
	ROM_LOAD16_BYTE( "jokerwild-ver1.01-fpp5u2.bin", 0x00001, 0x080000, CRC(e5f85c74) SHA1(898342364f75a2209f1b72574652012ca6e2364a) )
ROM_END

ROM_START( hb_jkrwla )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "jokerwild-ver1.01p5u1.bin", 0x00000, 0x080000, CRC(06f7ea55) SHA1(5c65f9308f13721b94aa66465da0bdd08171310f) )
	ROM_LOAD16_BYTE( "jokerwild-ver1.01p5u2.bin", 0x00001, 0x080000, CRC(e99d8e73) SHA1(0c7c4f588c213153ce4bd57ef53f09894fc67cc3) )
ROM_END

ROM_START( hb_mrmon )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "80000032.p1", 0x00000, 0x080000, CRC(3dc96f45) SHA1(1b4e36551f21ee8e579d6dd5a0709a20ab1905f3) )
	ROM_LOAD16_BYTE( "80000032.p2", 0x00001, 0x080000, CRC(8c1fd3fa) SHA1(795320edc0b1e0daeb86b7a3a440ae926d7de38d) )
ROM_END

ROM_START( hb_mrmona )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "80000034.p1", 0x00000, 0x080000, CRC(1f5e7ae4) SHA1(0d975edadbcb9f9bb289f8164c14e021587127c9) )
	ROM_LOAD16_BYTE( "80000034.p2", 0x00001, 0x080000, CRC(fdb3a2e1) SHA1(2225c626b4a14458159cf46bcfbe8d4c6561b170) )
ROM_END

ROM_START( hb_mrmonb )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "80001032.p1", 0x00000, 0x080000, CRC(fb513fda) SHA1(fac3d5b96e0f8c588b5b5d73eef25feb7cef601a) )
	ROM_LOAD16_BYTE( "80001032.p2", 0x00001, 0x080000, CRC(0fe7e8ca) SHA1(956747384f6cb488ed9b33abbaef212b41ce7019) )
ROM_END

ROM_START( hb_mrmonc )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "80001034.p1", 0x00000, 0x080000, CRC(8b4013e9) SHA1(66ce8e8954f93a123fa9c0b69b091a39c45082d5) )
	ROM_LOAD16_BYTE( "80001034.p2", 0x00001, 0x080000, CRC(f41facca) SHA1(d835d46feb2d270c4d4e9f5bb8500e48ca8c71c0) )
ROM_END

ROM_START( hb_rhv )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "80001604.p1", 0x00000, 0x080000, CRC(9318d95f) SHA1(6e7cfc0e70738020a5a7e06476120679d867a423) )
	ROM_LOAD16_BYTE( "80001604.p2", 0x00001, 0x080000, CRC(19d111a7) SHA1(dd355d454b405c68106ec5e4611f9ff4f8e4e37d) )
ROM_END

ROM_START( hb_rhva )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "80001606.p1", 0x00000, 0x080000, CRC(8b719a3a) SHA1(6607b88b4af4c764eb6e80714d3ffa2c4c3f4aa7) )
	ROM_LOAD16_BYTE( "80001606.p2", 0x00001, 0x080000, CRC(653173b4) SHA1(e4c50b0a2413e9a023c2eac8c45eaa6856e48953) )
ROM_END

ROM_START( hb_ringb )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "9dd5", 0x00001, 0x100000, CRC(697b8dc9) SHA1(9bc575023a7229fd3411ebcc5cb01ad2047bfa3f) )//r1
	ROM_LOAD16_BYTE( "6d9d", 0x00000, 0x100000, CRC(d61989b5) SHA1(76b37fe886e8e44f8d7fc59cabdefc722d6b2b90) )//r1
ROM_END

ROM_START( hb_ringba )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "9dd5", 0x00001, 0x100000, CRC(697b8dc9) SHA1(9bc575023a7229fd3411ebcc5cb01ad2047bfa3f) )//r1
	ROM_LOAD16_BYTE( "6d9e", 0x00000, 0x100000, CRC(1e58a92d) SHA1(945ddc435e488f4b021dac386eb83932c7e5dac7) )//r1
ROM_END

ROM_START( hb_ringbb )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "f682", 0x00001, 0x100000, CRC(d19f7cb1) SHA1(d943e25d88eeaddd90482ecb8f80f0fc3160161c) )//r2
	ROM_LOAD16_BYTE( "8579", 0x00000, 0x100000, CRC(005f585f) SHA1(9eba28632f1b48743198e0d85d1bc8d965e31356) )//r2
ROM_END

ROM_START( hb_ringbc )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "f682", 0x00001, 0x100000, CRC(d19f7cb1) SHA1(d943e25d88eeaddd90482ecb8f80f0fc3160161c) )//r2
	ROM_LOAD16_BYTE( "8578", 0x00000, 0x100000, CRC(c81e78c7) SHA1(214efe8c237b69b63ac1409810dcd0711780289f) )//r2
ROM_END

// these still contain a single ring-a-bell string, but are otherwise different, is it a 2nd CPU
// (video display?) for the same game.. the pairings are less obvious too, there are just small data
// differences near the end, so they'll need to be verified.

ROM_START( hb_ringbd )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "38fb", 0x00001, 0x100000, CRC(81cb92a6) SHA1(d2c829e8f4719c33a51d382570f00191ad03fa6a) )
	ROM_LOAD16_BYTE( "fb1f", 0x00000, 0x100000, CRC(42469e0a) SHA1(b1244b43dc661f9259f71d2f148e0a8d3a8e2fd1) )
ROM_END

ROM_START( hb_ringbe )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "37d0", 0x00001, 0x100000, CRC(9ab4e507) SHA1(086d7fd2288127da0aa4e4ce0e9d7971c855d196) )
	ROM_LOAD16_BYTE( "fc16", 0x00000, 0x100000, CRC(6faee0a9) SHA1(b90059d1a21b17e8b582693383e29c2acb7091ae) )
ROM_END

ROM_START( hb_rckrl )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "80000800.p1", 0x00000, 0x080000, CRC(c849ea6b) SHA1(3bbfca4e346467a1b5e582ff61949904145331ae) )
	ROM_LOAD16_BYTE( "80000800.p2", 0x00001, 0x080000, CRC(582ac89b) SHA1(997f0cb2e60abdfd56cc9539da9e1408926a0404) )
ROM_END

ROM_START( hb_rckrla )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "80000802.p1", 0x00000, 0x080000, CRC(0d953184) SHA1(a638d3df83d9ea57a2667d76edf07953955a9b72) )
	ROM_LOAD16_BYTE( "80000802.p2", 0x00001, 0x080000, CRC(b321bde4) SHA1(7655c73b609d765f50a8c46214ee046ee9f626ad) )
ROM_END

ROM_START( hb_rckrlb )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "80000804.p1", 0x00000, 0x080000, CRC(83ef85f6) SHA1(8b47cb572ed5ac04f75fae6f3d1a9cc99e60b9db) )
	ROM_LOAD16_BYTE( "80000804.p2", 0x00001, 0x080000, CRC(ec59a868) SHA1(32644834651e708448adccf5323e7f75ddff1914) )
ROM_END

ROM_START( hb_rckrlc )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "80000806.p1", 0x00000, 0x080000, CRC(0c67675e) SHA1(49b77f056ac13bcb9b0b7b4f24079389f2ddbc4d) )
	ROM_LOAD16_BYTE( "80000806.p2", 0x00001, 0x080000, CRC(ed1bd445) SHA1(9070e50e797071b794e6bb1db78decf42e3cf0e1) )
ROM_END

ROM_START( hb_rckrld )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "80000808.p1", 0x00000, 0x080000, CRC(d56bbb39) SHA1(cf7dafb73a170e2419d7e261e0bf666de9100edf) )
	ROM_LOAD16_BYTE( "80000808.p2", 0x00001, 0x080000, CRC(a0fd2416) SHA1(77ccd975a74478f044db03c77150fb0d083e0c44) )
ROM_END

ROM_START( hb_rckrle )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "80000810.p1", 0x00000, 0x080000, CRC(37e228a1) SHA1(877b3d562b28f1046c3b62b3acf1099743b3b655) )
	ROM_LOAD16_BYTE( "80000810.p2", 0x00001, 0x080000, CRC(3ae44531) SHA1(a90d53c95c85b694beaaa43ab711d067858e5588) )
ROM_END

ROM_START( hb_rckrlf )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "80000812.p1", 0x00000, 0x080000, CRC(42f0b206) SHA1(c8c8470359f8a2e9231b7cb984ac83f8a2298ea0) )
	ROM_LOAD16_BYTE( "80000812.p2", 0x00001, 0x080000, CRC(d3068f00) SHA1(57f5509dab41d5feea0e73c7dc3a6d11c1f8d13c) )
ROM_END

ROM_START( hb_rckrlg )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "80000814.p1", 0x00000, 0x080000, CRC(3f182316) SHA1(839c42e4004a231504b35590accf13fc2df0db90) )
	ROM_LOAD16_BYTE( "80000814.p2", 0x00001, 0x080000, CRC(2da91e78) SHA1(8ff3d22f6e0e2a676b29b839a91f4c05df574208) )
ROM_END

ROM_START( hb_ydd )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "80000040.p1", 0x00000, 0x080000, CRC(f0e4744a) SHA1(0eea373783108d1a6ae37ae81ed2e38d9cdba2e5) )
	ROM_LOAD16_BYTE( "80000040.p2", 0x00001, 0x080000, CRC(a24e01b4) SHA1(266f63ec113335bee88de61e0ceb1dc38e9fc883) )
ROM_END

ROM_START( hb_ydda )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "80001040.p1", 0x00000, 0x080000, CRC(7b353302) SHA1(24e800c05be9e06c9c2a9e5bfee3b835f9f770b9) )
	ROM_LOAD16_BYTE( "80001040.p2", 0x00001, 0x080000, CRC(046b5e10) SHA1(364024d5b8750d457c368ab2afd99a8118aaa309) )
ROM_END

ROM_START( hb_hotst )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "hstv0-04.u1", 0x000000, 0x080000, CRC(23043c19) SHA1(03c7b2eda9b4d975aeba3eea091b11016df35047) )
	ROM_LOAD16_BYTE( "hstv0-04.u2", 0x000001, 0x080000, CRC(941294dc) SHA1(359f3dac97c869f9cec5367d14aa95edced7c2de) )
ROM_END

ROM_START( hb_hotsta )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "hst0-04d.u1", 0x000000, 0x080000, CRC(2affa3aa) SHA1(ed3c5a0380f7d823e05164cbce85a85211fd2755) )
	ROM_LOAD16_BYTE( "hstv0-04.u2", 0x000001, 0x080000, CRC(941294dc) SHA1(359f3dac97c869f9cec5367d14aa95edced7c2de) )
ROM_END

ROM_START( hb_hotstb )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "hst0-04g.u1", 0x000000, 0x080000, CRC(4bcbd10b) SHA1(c7a9c0b3cec6dafd699b67b51823b86f354c665c) )
	ROM_LOAD16_BYTE( "hstv0-04.u2", 0x000001, 0x080000, CRC(941294dc) SHA1(359f3dac97c869f9cec5367d14aa95edced7c2de) )
ROM_END

ROM_START( hb_hotstc )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "hstv0-06.u1", 0x000000, 0x080000, CRC(e09f928c) SHA1(fafd8bb8c1f9e1904ef7a4dfe89eaf5a8ec6398e) )
	ROM_LOAD16_BYTE( "hstv0-06.u2", 0x000001, 0x080000, CRC(2f9a53cc) SHA1(63e7ec37c6ee177e3d33ff2c649f743a8585bce4) )
ROM_END

ROM_START( hb_hotstd )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "hstv006d.u1", 0x000000, 0x080000, CRC(e9640d3f) SHA1(a51a6c3a8301fc19d15a07a4f1d1c86d13ef9141) )
	ROM_LOAD16_BYTE( "hstv0-06.u2", 0x000001, 0x080000, CRC(2f9a53cc) SHA1(63e7ec37c6ee177e3d33ff2c649f743a8585bce4) )
ROM_END

ROM_START( hb_hotste )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "hstv006g.u1", 0x000000, 0x080000, CRC(88507f9e) SHA1(acf1e26d7ddc6270c4025079516ad58c85630b50) )
	ROM_LOAD16_BYTE( "hstv0-06.u2", 0x000001, 0x080000, CRC(2f9a53cc) SHA1(63e7ec37c6ee177e3d33ff2c649f743a8585bce4) )
ROM_END

ROM_START( hb_hotstf )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "hstv0-07.u1", 0x000000, 0x080000, CRC(b3376b55) SHA1(4c5b6f104318bea7efefb8535921a8527429392e) )
	ROM_LOAD16_BYTE( "hstv0-07.u2", 0x000001, 0x080000, CRC(adde3b86) SHA1(95b65b9ae2c2034370c895803eb5dcac0a69a77a) )
ROM_END

ROM_START( hb_hotstg )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "hst0-07d.u1", 0x000000, 0x080000, CRC(baccf4e6) SHA1(bd3f6c5e1a43779f278b481dd08860be22c565a2) )
	ROM_LOAD16_BYTE( "hstv0-07.u2", 0x000001, 0x080000, CRC(adde3b86) SHA1(95b65b9ae2c2034370c895803eb5dcac0a69a77a) )
ROM_END

ROM_START( hb_hotsth )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "hst0-07g.u1", 0x000000, 0x080000, CRC(dbf88647) SHA1(c2bdabae0e4234b245541983bb8e7d43636816c9) )
	ROM_LOAD16_BYTE( "hstv0-07.u2", 0x000001, 0x080000, CRC(adde3b86) SHA1(95b65b9ae2c2034370c895803eb5dcac0a69a77a) )
ROM_END

ROM_START( hb_medal )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "80000002.p1", 0x000000, 0x080000, CRC(4789c981) SHA1(597de01ef44621cf1223694338470fde2b4e527b) )
	ROM_LOAD16_BYTE( "80000002.p2", 0x000001, 0x080000, CRC(16b2cc93) SHA1(ea0dd43e19602791cde16816c699e6d70b5cbe41) )
ROM_END



extern void astra_addresslines( UINT16* src, size_t srcsize, int small );



DRIVER_INIT_MEMBER(pluto5_state,hb)
{
	astra_addresslines( (UINT16*)memregion( "maincpu" )->base(), memregion( "maincpu" )->bytes(), 0 );

	#if 0
	{
		UINT8* ROM = memregion( "maincpu" )->base();
		FILE *fp;
		char filename[256];
		sprintf(filename,"%s", machine().system().name);
		fp=fopen(filename, "w+b");
		if (fp)
		{
			fwrite(ROM,  memregion( "maincpu" )->bytes(), 1, fp);
			fclose(fp);
		}
	}
	#endif
}

GAME( 200?, hb_cr       ,0,         pluto5, pluto5, pluto5_state, hb, ROT0, "Qps","Cash Raker (Qps) (set 1)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_cra      ,hb_cr,     pluto5, pluto5, pluto5_state, hb, ROT0, "Qps","Cash Raker (Qps) (set 2)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_crb      ,hb_cr,     pluto5, pluto5, pluto5_state, hb, ROT0, "Qps","Cash Raker (Qps) (set 3)", MACHINE_IS_SKELETON_MECHANICAL )

GAME( 200?, hb_bar7     ,0,         pluto5, pluto5, pluto5_state, hb, ROT0, "Fairgames","Bar Seven (Fairgames) (set 1)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_bar7a    ,hb_bar7,   pluto5, pluto5, pluto5_state, hb, ROT0, "Fairgames","Bar Seven (Fairgames) (set 2)", MACHINE_IS_SKELETON_MECHANICAL )

GAME( 200?, hb_bigx     ,0,         pluto5, pluto5, pluto5_state, hb, ROT0, "JPM","Big X (JPM) (set 1)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_bigxa    ,hb_bigx,   pluto5, pluto5, pluto5_state, hb, ROT0, "JPM","Big X (JPM) (set 2)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_bigxb    ,hb_bigx,   pluto5, pluto5, pluto5_state, hb, ROT0, "JPM","Big X (JPM) (set 3)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_bigxc    ,hb_bigx,   pluto5, pluto5, pluto5_state, hb, ROT0, "JPM","Big X (JPM) (set 4)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_bigxd    ,hb_bigx,   pluto5, pluto5, pluto5_state, hb, ROT0, "JPM","Big X (JPM) (set 5)", MACHINE_IS_SKELETON_MECHANICAL )

GAME( 200?, hb_ccow     ,0,         pluto5, pluto5, pluto5_state, hb, ROT0, "Qps","Cash Cow (Qps) (set 1)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_ccowa    ,hb_ccow,   pluto5, pluto5, pluto5_state, hb, ROT0, "Qps","Cash Cow (Qps) (set 2)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_ccowb    ,hb_ccow,   pluto5, pluto5, pluto5_state, hb, ROT0, "Qps","Cash Cow (Qps) (set 3)", MACHINE_IS_SKELETON_MECHANICAL )

GAME( 200?, hb_cashc    ,0,         pluto5, pluto5, pluto5_state, hb, ROT0, "Qps","Cash Crusade (Qps) (set 1)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_cashca   ,hb_cashc,  pluto5, pluto5, pluto5_state, hb, ROT0, "Qps","Cash Crusade (Qps) (set 2)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_cashcb   ,hb_cashc,  pluto5, pluto5, pluto5_state, hb, ROT0, "Qps","Cash Crusade (Qps) (set 3)", MACHINE_IS_SKELETON_MECHANICAL )

GAME( 200?, hb_cashx    ,0,         pluto5, pluto5, pluto5_state, hb, ROT0, "Fairgames","Cash X (Fairgames) (set 1)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_cashxa   ,hb_cashx,  pluto5, pluto5, pluto5_state, hb, ROT0, "Fairgames","Cash X (Fairgames) (set 2)", MACHINE_IS_SKELETON_MECHANICAL )

GAME( 200?, hb_cwf      ,0,         pluto5, pluto5, pluto5_state, hb, ROT0, "Fairgames","Cherry Win Falls (Fairgames) (set 1)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_cwfa     ,hb_cwf,    pluto5, pluto5, pluto5_state, hb, ROT0, "Fairgames","Cherry Win Falls (Fairgames) (set 2)", MACHINE_IS_SKELETON_MECHANICAL )

GAME( 200?, hb_dac      ,0,         pluto5, pluto5, pluto5_state, hb, ROT0, "Qps","Dough & Arrow Club (Qps, set 1)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_daca     ,hb_dac,    pluto5, pluto5, pluto5_state, hb, ROT0, "Qps","Dough & Arrow Club (Qps, set 2)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_dacb     ,hb_dac,    pluto5, pluto5, pluto5_state, hb, ROT0, "Qps","Dough & Arrow Club (Qps, set 3)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_dacc     ,hb_dac,    pluto5, pluto5, pluto5_state, hb, ROT0, "Qps","Dough & Arrow Club (Qps, set 4)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_dacd     ,hb_dac,    pluto5, pluto5, pluto5_state, hb, ROT0, "Qps","Dough & Arrow Club (Qps, set 5)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_dace     ,hb_dac,    pluto5, pluto5, pluto5_state, hb, ROT0, "Qps","Dough & Arrow Club (Qps, set 6)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_dacf     ,hb_dac,    pluto5, pluto5, pluto5_state, hb, ROT0, "Qps","Dough & Arrow Club (Qps, set 7)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_dacg     ,hb_dac,    pluto5, pluto5, pluto5_state, hb, ROT0, "Qps","Dough & Arrow Club (Qps, set 8)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_dacz     ,hb_dac,    pluto5, pluto5, pluto5_state, hb, ROT0, "Qps","Dough & Arrow Club (Qps, set 9)", MACHINE_IS_SKELETON_MECHANICAL ) // bad dump

GAME( 200?, hb_frtcl    ,0,         pluto5, pluto5, pluto5_state, hb, ROT0, "Qps","Fruitopia Club (Qps) (set 1)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_frtcla   ,hb_frtcl,  pluto5, pluto5, pluto5_state, hb, ROT0, "Qps","Fruitopia Club (Qps) (set 2)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_frtclb   ,hb_frtcl,  pluto5, pluto5, pluto5_state, hb, ROT0, "Qps","Fruitopia Club (Qps) (set 3)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_frtclc   ,hb_frtcl,  pluto5, pluto5, pluto5_state, hb, ROT0, "Qps","Fruitopia Club (Qps) (set 4)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_frtcld   ,hb_frtcl,  pluto5, pluto5, pluto5_state, hb, ROT0, "Qps","Fruitopia Club (Qps) (set 5)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_frtcle   ,hb_frtcl,  pluto5, pluto5, pluto5_state, hb, ROT0, "Qps","Fruitopia Club (Qps) (set 6)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_frtclf   ,hb_frtcl,  pluto5, pluto5, pluto5_state, hb, ROT0, "Qps","Fruitopia Club (Qps) (set 7)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_frtclg   ,hb_frtcl,  pluto5, pluto5, pluto5_state, hb, ROT0, "Qps","Fruitopia Club (Qps) (set 8)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_frtclh   ,hb_frtcl,  pluto5, pluto5, pluto5_state, hb, ROT0, "Qps","Fruitopia Club (Qps) (set 9)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_frtcli   ,hb_frtcl,  pluto5, pluto5, pluto5_state, hb, ROT0, "Qps","Fruitopia Club (Qps) (set 10)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_frtclj   ,hb_frtcl,  pluto5, pluto5, pluto5_state, hb, ROT0, "Qps","Fruitopia Club (Qps) (set 11)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_frtclk   ,hb_frtcl,  pluto5, pluto5, pluto5_state, hb, ROT0, "Qps","Fruitopia Club (Qps) (set 12)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_frtcll   ,hb_frtcl,  pluto5, pluto5, pluto5_state, hb, ROT0, "Qps","Fruitopia Club (Qps) (set 13)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_frtclm   ,hb_frtcl,  pluto5, pluto5, pluto5_state, hb, ROT0, "Qps","Fruitopia Club (Qps) (set 14)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_frtcln   ,hb_frtcl,  pluto5, pluto5, pluto5_state, hb, ROT0, "Qps","Fruitopia Club (Qps) (set 15)", MACHINE_IS_SKELETON_MECHANICAL )

GAME( 200?, hb_gpal     ,0,         pluto5, pluto5, pluto5_state, hb, ROT0, "Qps","Golden Palace (Qps) (set 1)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_gpala    ,hb_gpal,   pluto5, pluto5, pluto5_state, hb, ROT0, "Qps","Golden Palace (Qps) (set 2)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_gpalb    ,hb_gpal,   pluto5, pluto5, pluto5_state, hb, ROT0, "Qps","Golden Palace (Qps) (set 3)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_gpalc    ,hb_gpal,   pluto5, pluto5, pluto5_state, hb, ROT0, "Qps","Golden Palace (Qps) (set 4)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_gpald    ,hb_gpal,   pluto5, pluto5, pluto5_state, hb, ROT0, "Qps","Golden Palace (Qps) (set 5)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_gpale    ,hb_gpal,   pluto5, pluto5, pluto5_state, hb, ROT0, "Qps","Golden Palace (Qps) (set 6)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_gpalf    ,hb_gpal,   pluto5, pluto5, pluto5_state, hb, ROT0, "Qps","Golden Palace (Qps) (set 7)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_gpalg    ,hb_gpal,   pluto5, pluto5, pluto5_state, hb, ROT0, "Qps","Golden Palace (Qps) (set 8)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_gpalh    ,hb_gpal,   pluto5, pluto5, pluto5_state, hb, ROT0, "Qps","Golden Palace (Qps) (set 9)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_gpali    ,hb_gpal,   pluto5, pluto5, pluto5_state, hb, ROT0, "Qps","Golden Palace (Qps) (set 10)", MACHINE_IS_SKELETON_MECHANICAL )

GAME( 200?, hb_gldpl    ,0,         pluto5, pluto5, pluto5_state, hb, ROT0, "Qps / Mazooma","Golden Palace (Qps / Mazooma) (set 1)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_gldpla   ,hb_gldpl,  pluto5, pluto5, pluto5_state, hb, ROT0, "Qps / Mazooma","Golden Palace (Qps / Mazooma) (set 2)", MACHINE_IS_SKELETON_MECHANICAL )

GAME( 200?, hb_gldwn    ,0,         pluto5, pluto5, pluto5_state, hb, ROT0, "Fairgames","Golden Winner (Fairgames) (set 1)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_gldwna   ,hb_gldwn,  pluto5, pluto5, pluto5_state, hb, ROT0, "Fairgames","Golden Winner (Fairgames) (set 2)", MACHINE_IS_SKELETON_MECHANICAL )

GAME( 200?, hb_jailb    ,0,         pluto5, pluto5, pluto5_state, hb, ROT0, "Qps","Jail Break (Qps) (set 1)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_jailba   ,hb_jailb,  pluto5, pluto5, pluto5_state, hb, ROT0, "Qps","Jail Break (Qps) (set 2)", MACHINE_IS_SKELETON_MECHANICAL )

GAME( 200?, hb_jkrwl    ,0,         pluto5, pluto5, pluto5_state, hb, ROT0, "Fairgames","Jokers Wild (Fairgames) (set 1)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_jkrwla   ,hb_jkrwl,  pluto5, pluto5, pluto5_state, hb, ROT0, "Fairgames","Jokers Wild (Fairgames) (set 2)", MACHINE_IS_SKELETON_MECHANICAL )

GAME( 200?, hb_mrmon    ,0,         pluto5, pluto5, pluto5_state, hb, ROT0, "Qps","Mr. Money (Qps) (set 1)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_mrmona   ,hb_mrmon,  pluto5, pluto5, pluto5_state, hb, ROT0, "Qps","Mr. Money (Qps) (set 2)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_mrmonb   ,hb_mrmon,  pluto5, pluto5, pluto5_state, hb, ROT0, "Qps","Mr. Money (Qps) (set 3)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_mrmonc   ,hb_mrmon,  pluto5, pluto5, pluto5_state, hb, ROT0, "Qps","Mr. Money (Qps) (set 4)", MACHINE_IS_SKELETON_MECHANICAL )

GAME( 200?, hb_rhv      ,0,         pluto5, pluto5, pluto5_state, hb, ROT0, "Qps","Red Hot Voucher (Qps) (set 1)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_rhva     ,hb_rhv,    pluto5, pluto5, pluto5_state, hb, ROT0, "Qps","Red Hot Voucher (Qps) (set 2)", MACHINE_IS_SKELETON_MECHANICAL )

GAME( 200?, hb_ringb    ,0,         pluto5, pluto5, pluto5_state, hb, ROT0, "JPM","Ring A Bell (JPM) (set 1)", MACHINE_IS_SKELETON_MECHANICAL ) // this game might be on Astra hardware, bigger roms, and a game of this name is known to exist there
GAME( 200?, hb_ringba   ,hb_ringb,  pluto5, pluto5, pluto5_state, hb, ROT0, "JPM","Ring A Bell (JPM) (set 2)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_ringbb   ,hb_ringb,  pluto5, pluto5, pluto5_state, hb, ROT0, "JPM","Ring A Bell (JPM) (set 3)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_ringbc   ,hb_ringb,  pluto5, pluto5, pluto5_state, hb, ROT0, "JPM","Ring A Bell (JPM) (set 4)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_ringbd   ,hb_ringb,  pluto5, pluto5, pluto5_state, hb, ROT0, "JPM","Ring A Bell (JPM) (set 5)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_ringbe   ,hb_ringb,  pluto5, pluto5, pluto5_state, hb, ROT0, "JPM","Ring A Bell (JPM) (set 6)", MACHINE_IS_SKELETON_MECHANICAL )

GAME( 200?, hb_rckrl    ,0,         pluto5, pluto5, pluto5_state, hb, ROT0, "Qps","Rock 'n' Roll (Qps) (set 1)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_rckrla   ,hb_rckrl,  pluto5, pluto5, pluto5_state, hb, ROT0, "Qps","Rock 'n' Roll (Qps) (set 2)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_rckrlb   ,hb_rckrl,  pluto5, pluto5, pluto5_state, hb, ROT0, "Qps","Rock 'n' Roll (Qps) (set 3)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_rckrlc   ,hb_rckrl,  pluto5, pluto5, pluto5_state, hb, ROT0, "Qps","Rock 'n' Roll (Qps) (set 4)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_rckrld   ,hb_rckrl,  pluto5, pluto5, pluto5_state, hb, ROT0, "Qps","Rock 'n' Roll (Qps) (set 5)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_rckrle   ,hb_rckrl,  pluto5, pluto5, pluto5_state, hb, ROT0, "Qps","Rock 'n' Roll (Qps) (set 6)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_rckrlf   ,hb_rckrl,  pluto5, pluto5, pluto5_state, hb, ROT0, "Qps","Rock 'n' Roll (Qps) (set 7)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_rckrlg   ,hb_rckrl,  pluto5, pluto5, pluto5_state, hb, ROT0, "Qps","Rock 'n' Roll (Qps) (set 8)", MACHINE_IS_SKELETON_MECHANICAL )

GAME( 200?, hb_ydd      ,0,         pluto5, pluto5, pluto5_state, hb, ROT0, "Qps","Yabba-Dabba-Dough (Qps) (set 1)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_ydda     ,hb_ydd,    pluto5, pluto5, pluto5_state, hb, ROT0, "Qps","Yabba-Dabba-Dough (Qps) (set 2)", MACHINE_IS_SKELETON_MECHANICAL )

GAME( 200?, hb_hotst    ,0,         pluto5, pluto5, pluto5_state, hb, ROT0, "JPM?","Hot Stuff (JPM?) (set 1)", MACHINE_IS_SKELETON_MECHANICAL ) // was in a Barcrest MPU5 set, but I doubt it is
GAME( 200?, hb_hotsta   ,hb_hotst,  pluto5, pluto5, pluto5_state, hb, ROT0, "JPM?","Hot Stuff (JPM?) (set 2)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_hotstb   ,hb_hotst,  pluto5, pluto5, pluto5_state, hb, ROT0, "JPM?","Hot Stuff (JPM?) (set 3)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_hotstc   ,hb_hotst,  pluto5, pluto5, pluto5_state, hb, ROT0, "JPM?","Hot Stuff (JPM?) (set 4)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_hotstd   ,hb_hotst,  pluto5, pluto5, pluto5_state, hb, ROT0, "JPM?","Hot Stuff (JPM?) (set 5)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_hotste   ,hb_hotst,  pluto5, pluto5, pluto5_state, hb, ROT0, "JPM?","Hot Stuff (JPM?) (set 6)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_hotstf   ,hb_hotst,  pluto5, pluto5, pluto5_state, hb, ROT0, "JPM?","Hot Stuff (JPM?) (set 7)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_hotstg   ,hb_hotst,  pluto5, pluto5, pluto5_state, hb, ROT0, "JPM?","Hot Stuff (JPM?) (set 8)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 200?, hb_hotsth   ,hb_hotst,  pluto5, pluto5, pluto5_state, hb, ROT0, "JPM?","Hot Stuff (JPM?) (set 9)", MACHINE_IS_SKELETON_MECHANICAL )

GAME( 200?, hb_medal    ,0,         pluto5, pluto5, pluto5_state, hb, ROT0, "Qps", "Medallion Job (Qps)", MACHINE_IS_SKELETON_MECHANICAL ) // was in an IMPACT set, strings indicate it's the same game, rebuild for this HW I guess
