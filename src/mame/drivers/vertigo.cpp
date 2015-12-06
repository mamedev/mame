// license:BSD-3-Clause
// copyright-holders:Mathis Rosenhauer
/***************************************************************************

    Exidy Vertigo hardware

    driver by Mathis Rosenhauer

    Games supported:
        * Top Gunner

***************************************************************************/

#include "emu.h"
#include "cpu/m6805/m6805.h"
#include "cpu/m68000/m68000.h"
#include "machine/pit8253.h"
#include "machine/nvram.h"
#include "includes/vertigo.h"



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

READ16_MEMBER(vertigo_state::vertigo_pit8254_lsb_r)
{
	return m_pit->read(space, offset);
}

WRITE16_MEMBER(vertigo_state::vertigo_pit8254_lsb_w)
{
	if (ACCESSING_BITS_0_7)
		m_pit->write(space, offset, data);
}

static ADDRESS_MAP_START( vertigo_map, AS_PROGRAM, 16, vertigo_state )
	AM_RANGE(0x000000, 0x000007) AM_ROM
	AM_RANGE(0x000008, 0x001fff) AM_RAM AM_MIRROR(0x010000)
	AM_RANGE(0x002000, 0x003fff) AM_RAM AM_SHARE("vectorram")
	AM_RANGE(0x004000, 0x00400f) AM_READ(vertigo_io_convert) AM_MIRROR(0x001000)
	AM_RANGE(0x004010, 0x00401f) AM_READ(vertigo_io_adc) AM_MIRROR(0x001000)
	AM_RANGE(0x004020, 0x00402f) AM_READ(vertigo_coin_r) AM_MIRROR(0x001000)
	AM_RANGE(0x004030, 0x00403f) AM_READ_PORT("GIO") AM_MIRROR(0x001000)
	AM_RANGE(0x004040, 0x00404f) AM_READ(vertigo_sio_r) AM_MIRROR(0x001000)
	AM_RANGE(0x004050, 0x00405f) AM_WRITE(vertigo_audio_w) AM_MIRROR(0x001000)
	AM_RANGE(0x004060, 0x00406f) AM_WRITE(vertigo_motor_w) AM_MIRROR(0x001000)
	AM_RANGE(0x004070, 0x00407f) AM_WRITE(vertigo_wsot_w) AM_MIRROR(0x001000)
	AM_RANGE(0x006000, 0x006007) AM_READWRITE(vertigo_pit8254_lsb_r, vertigo_pit8254_lsb_w)
	AM_RANGE(0x007000, 0x0073ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x800000, 0x81ffff) AM_ROM
ADDRESS_MAP_END



/*************************************
 *
 *  Motor CPU memory handlers
 *
 *************************************/

#if 0
static ADDRESS_MAP_START( vertigo_motor, AS_PROGRAM, 8, vertigo_state )
	AM_RANGE(0x010, 0x07f) AM_RAM
	AM_RANGE(0x080, 0x7ff) AM_ROM
ADDRESS_MAP_END
#endif



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( vertigo )
	PORT_START("P1X")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10)

	PORT_START("P1Y")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10)

	PORT_START("PADDLE")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_CODE_DEC(KEYCODE_Y) PORT_CODE_INC(KEYCODE_X)

	PORT_START("GIO")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_CONFIG_START( vertigo, vertigo_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 8000000)
	MCFG_CPU_PROGRAM_MAP(vertigo_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(vertigo_state, vertigo_interrupt, 60)

	MCFG_FRAGMENT_ADD(exidy440_audio)

	MCFG_DEVICE_ADD("pit8254", PIT8254, 0)
	MCFG_PIT8253_CLK0(240000)
	MCFG_PIT8253_OUT0_HANDLER(WRITELINE(vertigo_state, v_irq4_w))
	MCFG_PIT8253_CLK1(240000)
	MCFG_PIT8253_OUT1_HANDLER(WRITELINE(vertigo_state, v_irq3_w))
	MCFG_PIT8253_CLK2(240000)

	MCFG_DEVICE_ADD("74148", TTL74148, 0)
	MCFG_74148_OUTPUT_CB(vertigo_state, update_irq)

	/* motor controller */
	/*
	MCFG_CPU_ADD("motor", M6805, 1000000)
	MCFG_CPU_PROGRAM_MAP(vertigo_motor)
	*/
	MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_VECTOR_ADD("vector")
	MCFG_SCREEN_ADD("screen", VECTOR)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(400, 300)
	MCFG_SCREEN_VISIBLE_AREA(0, 510, 0, 400)
	MCFG_SCREEN_UPDATE_DEVICE("vector", vector_device, screen_update)
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definition
 *
 *************************************/

ROM_START( topgunnr )
	ROM_REGION( 0xf00000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "tgl-2.9p",  0x000000, 0x002000, CRC(1d10b31e) SHA1(c66f11d2bee81a51baccf96f8e8335fc86dc20e4) )
	ROM_LOAD16_BYTE( "tgl-2.10p", 0x000001, 0x002000, CRC(9c80b387) SHA1(aa7b770ddfaf65fd26959e7f9a3f15ba60979e50) )

	ROM_COPY( "maincpu", 0x00000, 0x800000,0x004000)
	ROM_LOAD16_BYTE( "tgl-2.9r",  0x804000, 0x002000, CRC(74454ac9) SHA1(4cf1e5373d5940ed81fe7d07324abb10667df097) )
	ROM_LOAD16_BYTE( "tgl-2.10r", 0x804001, 0x002000, CRC(f5c28223) SHA1(16bf122f289129b50545e463f685f517cb9baca7) )
	ROM_LOAD16_BYTE( "tgl-2.9t",  0x808000, 0x002000, CRC(d415d189) SHA1(3b726815292365a9206b83d1f2f5e314fcb24e73) )
	ROM_LOAD16_BYTE( "tgl-2.10t", 0x808001, 0x002000, CRC(7f6a735c) SHA1(15abe2f705ed95a0f84c0305300e3aea720be906) )
	ROM_LOAD16_BYTE( "tgl-2.9u",  0x80c000, 0x002000, CRC(723aea0c) SHA1(0f74fce22a832400906a886073f1252de327d85e) )
	ROM_LOAD16_BYTE( "tgl-2.10u", 0x80c001, 0x002000, CRC(a28994ad) SHA1(4bba76670b7bfeaa3709b205baa83d51226c5db5) )
	ROM_LOAD16_BYTE( "vgl-2.v9",  0x810000, 0x002000, CRC(bcfa709c) SHA1(575bba7471621f3f9cdf3c748500be5a5baf235d) )
	ROM_LOAD16_BYTE( "vgl-2.v10", 0x810001, 0x002000, CRC(59d061b4) SHA1(154671746f79142cd6757793c71fb30661fc04f0) )

	ROM_REGION16_BE( 0x10000, "user1", 0 )
	ROM_LOAD16_BYTE( "tgl-2.1e", 0x0000, 0x2000, CRC(25832d56) SHA1(6dfd85f5e1c1d30be540b306851016328bb1cc00) )
	ROM_LOAD16_BYTE( "tgl-2.2e", 0x0001, 0x2000, CRC(8746431f) SHA1(9e749e0e3aba51ba76e243e4c54b151dee9ff637) )
	ROM_LOAD16_BYTE( "tgl-2.1d", 0x4000, 0x2000, CRC(639cab24) SHA1(ae97efa07054130413bf4230b89c03fa3d0d5e41) )
	ROM_LOAD16_BYTE( "tgl-2.2d", 0x4001, 0x2000, CRC(10de7f77) SHA1(845e1dd7eb49116f0ba9332f27bf245f7625a598) )
	ROM_LOAD16_BYTE( "tgl-2.1b", 0x8000, 0x2000, CRC(9671b463) SHA1(8716c299e983f13ed0e82a17bd25cb9ff5cfd43f) )
	ROM_LOAD16_BYTE( "tgl-2.2b", 0x8001, 0x2000, CRC(258d507c) SHA1(16315039060d695c8278f544fbfa10ed1a0db3bc) )
	ROM_LOAD16_BYTE( "tgl-2.1a", 0xc000, 0x2000, CRC(0f7b2123) SHA1(17287ff5fb3be2a4d145daf10f9fa2c93a19fcc5) )
	ROM_LOAD16_BYTE( "tgl-2.2a", 0xc001, 0x2000, CRC(6edc8a05) SHA1(c257a845ecece072a9c1702e59edb2c65f9f4c02) )

	ROM_REGION64_BE( 0x1000, "proms", 0 )
	ROMX_LOAD( "vuc.10", 1, 0x200, CRC(8122e934) SHA1(a9bc0003f9597904fde49862c3d9f28522472b63), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(7))
	ROMX_LOAD( "vuc.09", 2, 0x200, CRC(5aa2240f) SHA1(c922961acfdefca67ba5555a1345d0a1c6cce526), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(7))
	ROMX_LOAD( "vuc.13", 2, 0x200, CRC(616aa606) SHA1(df985813ab35b98bd5b272b6e898c31b7bc16a5f), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(7))
	ROMX_LOAD( "vuc.07", 3, 0x200, CRC(b126c612) SHA1(1b9e22618b2cf68fac7d24ac87acc1f084af0f84), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(7))
	ROMX_LOAD( "vuc.08", 3, 0x200, CRC(5eb2f89f) SHA1(1c141da5abfd0a0899082ed5953b22f6ae3bb06d), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(7))
	ROMX_LOAD( "vuc.05", 4, 0x200, CRC(d54cab61) SHA1(05d0548ceb292e11a64c101ff0638bc8a406c29a), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(7))
	ROMX_LOAD( "vuc.06", 4, 0x200, CRC(c1b007a3) SHA1(c084c3767d5e6c0f995e33f3f1a642ad971301f4), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(7))
	ROMX_LOAD( "vuc.11", 5, 0x200, CRC(1417c4c6) SHA1(7809b288611db8095d51f4d8a4dc51d3b67ff1c4), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(7))
	ROMX_LOAD( "vuc.12", 5, 0x200, CRC(9e6e1f2e) SHA1(9b7ff0617f001c409680e5950dae055148590a55), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(7))
	ROMX_LOAD( "vuc.01", 6, 0x200, CRC(aae009c2) SHA1(7e73dc6106a772525d737ebdeeb9a3520d02ecd7), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(7))
	ROMX_LOAD( "vuc.02", 6, 0x200, CRC(3c340a9a) SHA1(b0bcf81a417ddab848b9b4d4c4e279c8ff24a874), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(7))
	ROMX_LOAD( "vuc.03", 7, 0x200, CRC(23c1f136) SHA1(0eb959aa8fb6028dd97bdaa28981cec16652bf2d), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(7))
	ROMX_LOAD( "vuc.04", 7, 0x200, CRC(a5389228) SHA1(922d49c949e31413bbbff118c04965b649864a67), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(7))

	ROM_REGION( 0x010000, "audiocpu", 0 )
	ROM_LOAD( "vga1_7.g7",  0x0e000, 0x2000, CRC(db109b19) SHA1(c3fbb28cb4679c021bc48f844097add39a2208a5) )

	ROM_REGION( 0x20000, "cvsd", 0 )
	ROM_LOAD( "vga1_7.l6",  0x00000, 0x2000, CRC(20cbf97a) SHA1(13e138b08ba3328db6a2fba95a369422455d1c5c) )
	ROM_LOAD( "vga1_7.m6",  0x02000, 0x2000, CRC(76197050) SHA1(d26701ba83a34384348fa34e3de78cc69dc5362e) )
	ROM_LOAD( "vga1_7.n6",  0x04000, 0x2000, CRC(b93d7cbb) SHA1(1a4d05e03765b66ff20b963c5a0b5f7c3d5a360c) )
	ROM_LOAD( "vga1_7.p6",  0x06000, 0x2000, CRC(b5bdb067) SHA1(924d76ff09dc173b582f84d1bb7ecd0a60cc1ab4) )
	ROM_LOAD( "vga1_7.rs6", 0x08000, 0x2000, CRC(772f13a8) SHA1(87a6247ba58c006d1a062a7ac338c34e85d5cd01) )
	ROM_LOAD( "vga1_7.st6", 0x0a000, 0x2000, CRC(a86f2178) SHA1(203fe71e2d42db4fb968c4e529eec7de0788aec1) )
	ROM_LOAD( "vga1_7.tu6", 0x0c000, 0x2000, CRC(c1ab1d39) SHA1(ada43570ecf4ae76030dab4a916c53536e41606d) )
	ROM_LOAD( "vga1_7.uv6", 0x0e000, 0x2000, CRC(95a05700) SHA1(e9f16408ac9a0ed28af74bfd8419a58e7b0f599a) )
	ROM_LOAD( "vga1_7.l7",  0x10000, 0x2000, CRC(183ba71d) SHA1(03b4dc21094d5911b6f964e060cbe4450ecb71e6) )
	ROM_LOAD( "vga1_7.m7",  0x12000, 0x2000, CRC(4866b4b7) SHA1(fa28d602b1e0a47528b710602bb32d5cc52c8db8) )

	ROM_REGION( 0x800, "cpu2", 0 )
	ROM_LOAD( "vga3_4.bd1",  0x080, 0x780, CRC(a50dde56) SHA1(ef13f4cf01c9d483f2dc829a2e23965a6053f37a) )
ROM_END



/*************************************
 *
 *  Game driver
 *
 *************************************/

GAME( 1986, topgunnr, 0, vertigo, vertigo, driver_device, 0, ROT0, "Exidy", "Top Gunner (Exidy)", MACHINE_SUPPORTS_SAVE )
