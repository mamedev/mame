// license:BSD-3-Clause
// copyright-holders:
/*******************************************************************************

    Skeleton driver for Sleic Dardomania darts machines
    This game has a monitor to select the game type.
    PCB is marked "SLEIC", "DIANA'94" and "13/94".

    Hardware overview:
    -Main CPU: Z0840006PSC
    -Sound: AY-3-8910
    -Other: MK48Z02B-20
    -OSCs: 18.432 MHz
    -Dips: 1 x 6 dips banks

   ______________________________________________________SN74LS74N_________________________________
  |  ______            ___      ___   ___             ___   _V_   ___                           ___|
  | |Xtal |           |  |     |  |  |  | SN74LS14N->|  |  |  |  |  |<-74LS04N                  ___|
  | |18.432Mhz        |  |     |  |  |__|<-MX699     |  |  |  |  |  |                           ___|
  |          SN74LS04N->_|     |__|<-SN74LS92N       |__|  |__|  |__|                           ___|
  |                                              ______________                                 ___|
  |     __________   __________     __________  |DARDOMANIA    |  __________                    ___|
  |    SN74LS393N|  |_74LS153N|    |AM9114EPC|  |DMP05.V2.1    | SN74LS166N|                    ___|
  |     __________                              |______________|                 ____           ___|
  |    |SN74LS08N|                               ______________                 |6x  |          ___|
  |     __________   __________     __________  |DARDOMANIA    |  __________    |DIPS|          ___|
  |    SN74LS393N|  |74LS257BN|    |AM9114EPC|  |DMP04.V2.1    | SN74LS166N|    |    |          ___|
  |                                             |______________|                |____|          ___|
  |                                              ______________                                 ___|
  |     __________   __________     __________  |DARDOMANIA    |  __________                    ___|
  |    |SN74LS08N|  |74LS157BN|    |AM9114EPC|  |DMP03.V2.1    | SN74LS166N|                    ___|
  |                                 __________  |______________|                                ___|
  |                                SN74LS125AN                       ___                        ___|
  |     __________   __________     __________       __________     |  |<-SN74LS07N            |
  |    |SN74LS20N|  |SN74LS10N|    |SN74LS32N|       SN74LS241N     |  |        __________     |
  |                                 __________   ______________     |__|        SN74LS273N     |
  |                                |SN74LS139N  |DARDOMANIA    |     _______    __________     |
  |     __________   __________                 |DMP02.V2.1    |    |AY-3  |    SN74LS273N     |
  |     SN74LS74AN  |SN74LS32N|     __________  |______________|    |8910  |    __________     |
  |                  _______       |SN74LS241N   ______________     |      |    SN74LS244N     |
  |     __________  |Z80   |                    |DARDOMANIA    |    |GI    |    __________     |
  |     SN74LS74AN  |CPU   |        __________  |DMP01.V2.1    |    |      |    SN74LS244N     |
  |       ___       |Z8040006      |SN74LS00N|  |______________|    |      |    __________     |
  |      |  |       |PSC   |                     ______________     |      |    SN74LS244N     |
  |      |  |       |      |        __________  |MK48Z02B-20   |    |      |                   |
74LS245N->  |       |Zilog |       |SN74LS241N  |ZEROPOWER RAM |    |      |                   |
  |      |__|       |      |        __________  |______________|    |      |          DIANA'94 |
  |                 |      |       |_N82S123N|                      |______|                   |
  |     __________  |______|        __________  SLEIC             __________   __________      |
  |     SN74LS74AN                 |SN74LS08N|  13-94            |_74LS86N_|  |_ LM380N_|      |
  |____________________________________________________________________________________________|

*******************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"
#include "screen.h"
#include "tilemap.h"
#include "emupal.h"
#include "speaker.h"


namespace {

class drdmania_state : public driver_device
{
public:
	drdmania_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_gfxdecode(*this, "gfxdecode")
		, m_fgram(*this, "fgram")
	{
	}

	void drdmania(machine_config &config);

	void init_drdmania();

protected:
	virtual void video_start() override ATTR_COLD;

private:
	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_shared_ptr<uint8_t> m_fgram;

	tilemap_t *m_fg_tilemap = nullptr;

	uint8_t unk_port00_r();

	void fgram_w(offs_t offset, uint8_t data);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};

void drdmania_state::mem_map(address_map &map)
{
	map(0x0000, 0xbfff).rom().region("maincpu", 0x0000);

	map(0xc000, 0xc7ff).ram(); // stack is here, but also writes ASCII strings as if it were a tilemap, but tile ROMs don't decode as ASCII
	map(0xd000, 0xd3ff).ram().w(FUNC(drdmania_state::fgram_w)).share("fgram"); // vram? fills with value of blank tile
}

uint8_t drdmania_state::unk_port00_r()
{
	return machine().rand();
}


void drdmania_state::io_map(address_map &map)
{
	map(0x00, 0x00).r(FUNC(drdmania_state::unk_port00_r));

	map.global_mask(0xff);
}

static INPUT_PORTS_START(drdmania)
INPUT_PORTS_END

static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( gfxa )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout, 0, 1 )
GFXDECODE_END

void drdmania_state::fgram_w(offs_t offset, uint8_t data)
{
	m_fgram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}


TILE_GET_INFO_MEMBER(drdmania_state::get_fg_tile_info)
{
	int code =  m_fgram[tile_index];
	tileinfo.set(0,code,0,0);
}


void drdmania_state::video_start()
{
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(drdmania_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}

uint32_t drdmania_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}



void drdmania_state::drdmania(machine_config &config)
{
	Z80(config, m_maincpu, 18.432_MHz_XTAL / 4); // divider not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &drdmania_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &drdmania_state::io_map);
	m_maincpu->set_vblank_int("screen", FUNC(drdmania_state::irq0_line_hold));

	//NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // MK48Z02B-20

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(256, 256);
	m_screen->set_visarea(0, 256-1, 0, 256-1);
	m_screen->set_screen_update(FUNC(drdmania_state::screen_update));

	GFXDECODE(config, m_gfxdecode, "palette", gfxa);

	PALETTE(config, "palette").set_format(palette_device::xRGB_444, 0x100).set_endianness(ENDIANNESS_BIG);

	SPEAKER(config, "mono").front_center();

	AY8910(config, "ay8910", 18.432_MHz_XTAL / 10).add_route(ALL_OUTPUTS, "mono", 0.5); // divider not verified
}

ROM_START(drdmania)
	ROM_REGION(0x0c000, "maincpu", 0)
	ROM_LOAD( "dardomania_dmp01_v2.1.ic38", 0x00000, 0x8000, CRC(9f24336f) SHA1(9a82b851d5c67a50118a3669d3bc5793e94219e4) )
	// seems to have some bad bytes eg rst $08 instructions which should be calls but end up resetting it instead, see init
	ROM_LOAD( "dardomania_dmp02_v2.1.ic33", 0x08000, 0x4000, BAD_DUMP CRC(e5dbf948) SHA1(241be0f2969b962bba602548dab3e2bdbf8f0696) ) // 1ST AND 2ND HALF IDENTICAL
	ROM_IGNORE(0x4000)

	ROM_REGION(0xc000, "gfx1", 0)
	ROM_LOAD( "dardomania_dmp03_v2.1.ic21", 0x00000, 0x4000, CRC(b458975e) SHA1(862d62d147ac09b86aa8d2c54b2e03a6c5436f85) ) // 1ST AND 2ND HALF IDENTICAL
	ROM_IGNORE(0x4000)
	ROM_LOAD( "dardomania_dmp04_v2.1.ic16", 0x04000, 0x4000, CRC(8564d0ba) SHA1(38c81173f1cf788d1a524abfae9ef7b6697383e4) ) // 1ST AND 2ND HALF IDENTICAL
	ROM_IGNORE(0x4000)
	ROM_LOAD( "dardomania_dmp05_v2.1.ic10", 0x08000, 0x4000, CRC(e24f2a02) SHA1(16f3a9c80b3d60c66b070521a90c958b0fc690e7) ) // 1ST AND 2ND HALF IDENTICAL
	ROM_IGNORE(0x4000)

	ROM_REGION(0x20, "proms", 0)
	ROM_LOAD( "n82s123n.ic49", 0x00, 0x20, CRC(dcbd2352) SHA1(ce72e84129ed1b455aaf648e1dfaa4333e7e7628) )
ROM_END

void drdmania_state::init_drdmania()
{
	uint8_t *ROM = memregion("maincpu")->base();
	ROM[0x8de0] ^= 0x02; // these are 0xcf in ROM so bit 0x02 got flipped? 3 calls in a row are incorrect in this way, so call addresses are suspect too
	ROM[0x8de1] ^= 0x02; // call address

	ROM[0x8de3] ^= 0x02; // call opcode

	ROM[0x8de6] ^= 0x02; // call opcode
	ROM[0x8de7] ^= 0x02; // call address

	ROM[0x8e00] ^= 0x02; // wrong, different bit flipped?
}

} // anonymous namespace


GAME(1994, drdmania, 0, drdmania, drdmania, drdmania_state, init_drdmania, ROT0, "Sleic", "Dardomania (v2.1)", MACHINE_NOT_WORKING | MACHINE_MECHANICAL )
