/*
    Mr. Game 1B11188/0
*/

#define ADDRESS_MAP_MODERN

#include "emu.h"
#include "cpu/m68000/m68000.h"

class mrgame_state : public driver_device
{
public:
	mrgame_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset();
};


static ADDRESS_MAP_START( mrgame_map, AS_PROGRAM, 16, mrgame_state )
	AM_RANGE(0x0000, 0xffffff) AM_NOP
ADDRESS_MAP_END

static INPUT_PORTS_START( mrgame )
INPUT_PORTS_END

void mrgame_state::machine_reset()
{
}

static DRIVER_INIT( mrgame )
{
}

static MACHINE_CONFIG_START( mrgame, mrgame_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 6000000)
	MCFG_CPU_PROGRAM_MAP(mrgame_map)
MACHINE_CONFIG_END

/*-------------------------------------------------------------------
/ Dakar (06/1988)
/-------------------------------------------------------------------*/
ROM_START(dakar)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD16_BYTE("cpu_ic13.rom", 0x000000, 0x8000, CRC(83183929) SHA1(977ac10a1e78c759eb0550794f2639fe0e2d1507))
	ROM_LOAD16_BYTE("cpu_ic14.rom", 0x000001, 0x8000, CRC(2010d28d) SHA1(d262dabd9298566df43df298cf71c974bee1434a))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("vid_ic14.rom", 0x00000, 0x8000, CRC(88a9ca81) SHA1(9660d416b2b8f1937cda7bca51bd287641c7730c))
	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD("vid_ic55.rom", 0x0000, 0x8000, CRC(3c68b448) SHA1(f416f00d2de0c71c021fec0e9702ba79b761d5e7))
	ROM_LOAD("vid_ic56.rom", 0x8000, 0x8000, CRC(0aac43e9) SHA1(28edfeddb2d54e40425488bad37e3819e4488b0b))
	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD("vid_ic66.rom", 0x0000, 0x0020, CRC(c8269b27) SHA1(daa83bfdb1e255b846bbade7f200abeaa9399c06))
	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("snd_ic06.rom", 0x0000, 0x8000, CRC(29e9417e) SHA1(24f465993da7c93d385ec453497f2af4d8abb6f4))
	ROM_LOAD("snd_ic07.rom", 0x8000, 0x8000, CRC(71ab15fe) SHA1(245842bb41410ea481539700f79c7ef94f8f8924))
	ROM_REGION(0x4000, "user1", 0)
	ROM_LOAD("snd_ic22.rom", 0x0000, 0x4000, CRC(e6c1098e) SHA1(06bf8917a27d5e46e4aab93e1f212918418e3a82))
	ROM_REGION(0x10000, "cpu4", 0)
	ROM_LOAD("snd_ic35.rom", 0x0000, 0x8000, CRC(7b2394d1) SHA1(f588f5105d75b54dd65bb6448a2d7774fb8477ec))
	ROM_LOAD("snd_ic36.rom", 0x8000, 0x8000, CRC(4039ea65) SHA1(390fce94d1e48b395157d8d9afaa485114c58d52))
ROM_END

/*-------------------------------------------------------------------
/ Motor Show (1988?)
/-------------------------------------------------------------------*/
ROM_START(motrshow)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD16_BYTE("cpu_ic13.rom", 0x000000, 0x8000, CRC(e862ca71) SHA1(b02e5f39f9427d58b70b7999a5ff6075beff05ae))
	ROM_LOAD16_BYTE("cpu_ic14.rom", 0x000001, 0x8000, CRC(c898ae25) SHA1(f0e1369284a1e0f394f1d40281fd46252016602e))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("vid_ic14.rom", 0x00000, 0x8000, CRC(1d4568e2) SHA1(bfc2bb59708ce3a09f9a1b3460ed8d5269840c97))
	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD("vid_ic55.rom", 0x0000, 0x8000, CRC(c27a4ded) SHA1(9c2c9b17f1e71afb74bdfbdcbabb99ef935d32db))
	ROM_LOAD("vid_ic56.rom", 0x8000, 0x8000, CRC(1664ec8d) SHA1(e7b15acdac7dfc51b668e908ca95f02a2b569737))
	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD("vid_ic66.rom", 0x0000, 0x0020, CRC(5b585252) SHA1(b88e56ebdce2c3a4b170aff4b05018e7c21a79b8))
	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("snd_ic06.rom", 0x0000, 0x8000, CRC(fba5a8f1) SHA1(ddf989abebe05c569c9ecdd498bd8ea409df88ac))
	ROM_REGION(0x4000, "user1", 0)
	ROM_LOAD("snd_ic22.rom", 0x0000, 0x4000, CRC(e6c1098e) SHA1(06bf8917a27d5e46e4aab93e1f212918418e3a82))
	ROM_REGION(0x10000, "cpu4", 0)
	ROM_LOAD("snd_ic35.rom", 0x0000, 0x8000, CRC(9dec153d) SHA1(8a0140257316aa19c0401456839e11b6896609b1))
	ROM_LOAD("snd_ic36.rom", 0x8000, 0x8000, CRC(4f42be6e) SHA1(684e988f413cd21c785ad5d60ef5eaddddaf72ab))
ROM_END

ROM_START(motrshowa)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD16_BYTE("cpuic13a.rom", 0x000000, 0x8000, CRC(2dbdd9d4) SHA1(b404814a4e83ead6da3c57818ae97f23d380f9da))
	ROM_LOAD16_BYTE("cpuic14b.rom", 0x000001, 0x8000, CRC(0bd98fec) SHA1(b90a7e997db59740398003ba94a69118b1ee70af))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("vid_ic14.rom", 0x00000, 0x8000, CRC(1d4568e2) SHA1(bfc2bb59708ce3a09f9a1b3460ed8d5269840c97))
	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD("vid_ic55.rom", 0x0000, 0x8000, CRC(c27a4ded) SHA1(9c2c9b17f1e71afb74bdfbdcbabb99ef935d32db))
	ROM_LOAD("vid_ic56.rom", 0x8000, 0x8000, CRC(1664ec8d) SHA1(e7b15acdac7dfc51b668e908ca95f02a2b569737))
	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD("vid_ic66.rom", 0x0000, 0x0020, CRC(5b585252) SHA1(b88e56ebdce2c3a4b170aff4b05018e7c21a79b8))
	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("snd_ic06.rom", 0x0000, 0x8000, CRC(fba5a8f1) SHA1(ddf989abebe05c569c9ecdd498bd8ea409df88ac))
	ROM_REGION(0x4000, "user1", 0)
	ROM_LOAD("snd_ic22.rom", 0x0000, 0x4000, CRC(e6c1098e) SHA1(06bf8917a27d5e46e4aab93e1f212918418e3a82))
	ROM_REGION(0x10000, "cpu4", 0)
	ROM_LOAD("snd_ic35.rom", 0x0000, 0x8000, CRC(9dec153d) SHA1(8a0140257316aa19c0401456839e11b6896609b1))
	ROM_LOAD("snd_ic36.rom", 0x8000, 0x8000, CRC(4f42be6e) SHA1(684e988f413cd21c785ad5d60ef5eaddddaf72ab))
ROM_END

/*-------------------------------------------------------------------
/ Mac Attack (1990)
/-------------------------------------------------------------------*/
ROM_START(macattck)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD16_BYTE("cpu_ic13.rom", 0x000000, 0x8000, NO_DUMP)
	ROM_LOAD16_BYTE("cpu_ic14.rom", 0x000001, 0x8000, NO_DUMP)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("vid_ic91.rom", 0x00000, 0x8000, CRC(42d2ba01) SHA1(c13d38c2798575760461912cef65dde57dfd938c))
	ROM_REGION( 0x30000, "gfx1", 0 )
	ROM_LOAD("vid_ic14.rom", 0x0000, 0x8000, CRC(f6e047fb) SHA1(6be712dda60257b9e7014315c8fee19812622bf6))
	ROM_LOAD("vid_ic15.rom", 0x8000, 0x8000, CRC(405a8f54) SHA1(4d58915763db3c3be2bfc166be1a12285ff2c38b))
	ROM_LOAD("vid_ic16.rom", 0x10000, 0x8000, CRC(063ea783) SHA1(385dbfcc8ecd3a784f9a8752d00e060b48d70d6a))
	ROM_LOAD("vid_ic17.rom", 0x18000, 0x8000, CRC(9f95abf8) SHA1(d71cf36c8bf27ad41b2d3cebd0af620a34ce0062) BAD_DUMP)
	ROM_LOAD("vid_ic18.rom", 0x20000, 0x8000, CRC(83ef25f8) SHA1(bab482badb8646b099dbb197ca9af3a126b274e3))
	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD("vid_ic61.rom", 0x0000, 0x0020, CRC(538c72ae) SHA1(f704492568257fcc4a4f1189207c6fb6526eb81c) BAD_DUMP)
	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("snd_ic06.rom", 0x0000, 0x8000, NO_DUMP)
	ROM_REGION(0x4000, "user1", 0)
	ROM_LOAD("snd_ic22.rom", 0x0000, 0x4000, NO_DUMP)
	ROM_REGION(0x10000, "cpu4", 0)
	ROM_LOAD("snd_ic35.rom", 0x0000, 0x8000, NO_DUMP)
	ROM_LOAD("snd_ic36.rom", 0x8000, 0x8000, NO_DUMP)
ROM_END

/*-------------------------------------------------------------------
/ World Cup 90 (1990)
/-------------------------------------------------------------------*/
ROM_START(wcup90)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD16_BYTE("cpu_ic13.rom", 0x000000, 0x8000, CRC(0e2edfb0) SHA1(862fb1f6509fb1f560d0b2bb8a5764f64b259f04))
	ROM_LOAD16_BYTE("cpu_ic14.rom", 0x000001, 0x8000, CRC(fdd03165) SHA1(6dc6e68197218f8808436098c26cd04fc3215b1c))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("vid_ic91.rom", 0x00000, 0x8000, CRC(3287ad20) SHA1(d5a453efc7292670073f157dca04897be857b8ed))
	ROM_REGION( 0x30000, "gfx1", 0 )
	ROM_LOAD("vid_ic14.rom", 0x0000, 0x8000, CRC(a101d562) SHA1(ad9ad3968f13169572ec60e22e84acf43382b51e))
	ROM_LOAD("vid_ic15.rom", 0x8000, 0x8000, CRC(40791e7a) SHA1(788760b8527df48d1825be88099491b6e94f0a19))
	ROM_LOAD("vid_ic16.rom", 0x10000, 0x8000, CRC(a7214157) SHA1(a4660180e8491a37028fec8533cf13daf839a7c4))
	ROM_LOAD("vid_ic17.rom", 0x18000, 0x8000, CRC(caf4fb04) SHA1(81784a4dc7c671090cf39cafa7d34a6b34523168))
	ROM_LOAD("vid_ic18.rom", 0x20000, 0x8000, CRC(83ad2a10) SHA1(37664e5872e6322ee6bb61ec9385876626598152))
	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD("vid_ic61.rom", 0x0000, 0x0020, CRC(538c72ae) SHA1(f704492568257fcc4a4f1189207c6fb6526eb81c))
	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("snd_ic06.rom", 0x0000, 0x8000, CRC(19a66331) SHA1(fbd71bc378b5a04247fd1754529c66b086eb33d8))
	ROM_REGION(0x4000, "user1", 0)
	ROM_LOAD("snd_ic21.rom", 0x0000, 0x4000, CRC(e6c1098e) SHA1(06bf8917a27d5e46e4aab93e1f212918418e3a82))
	ROM_REGION(0x30000, "user2", 0)
	ROM_LOAD("snd_ic45.rom", 0x00000, 0x10000, CRC(265aa979) SHA1(9ca10c41526a2d227c21f246273ca14bec7f1bc7))
	ROM_LOAD("snd_ic46.rom", 0x10000, 0x10000, CRC(7edb321e) SHA1(b242e94c24e996d2de803d339aa9bf6e93586a4c))
	ROM_REGION(0x10000, "cpu4", 0)
	ROM_LOAD("snd_ic44.rom", 0x00000, 0x8000, CRC(00946570) SHA1(83e7dd89844679571ab2a803295c8ca8941a4ac7))
ROM_END


GAME(1988,  dakar,     0,         mrgame,  mrgame,  mrgame,  ROT0,  "Mr Game",    "Dakar",                      GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1988,  motrshow,  0,         mrgame,  mrgame,  mrgame,  ROT0,  "Mr Game",    "Motor Show",                 GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1988,  motrshowa, motrshow,  mrgame,  mrgame,  mrgame,  ROT0,  "Mr Game",    "Motor Show (alternate set)", GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1990,  macattck,  0,         mrgame,  mrgame,  mrgame,  ROT0,  "Mr Game",    "Mac Attack",                 GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1990,  wcup90,    0,         mrgame,  mrgame,  mrgame,  ROT0,  "Mr Game",    "World Cup 90",               GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
