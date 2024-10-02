// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    Mosaic (c) 1990 Space

    Notes:
    - the ROM OK / RAM OK message in service mode is fake: ROM and RAM are not tested.


+--------------------------------------+
|            1  2  3  4     5  6  7  8 |
|       PAL                            |
|   2018      6116            6116     |
|J  2018      6116   PAL PAL  6116     |
|A                                     |
|M                                 PAL |
|M                           12.288MHz |
|A  14.31818MHz       PAL  6264  Z    P|
|       DSW                      1    I|
| VR                             8    C|
|        YM2203C              9  0     |
+--------------------------------------+

  CPU: Z180 (surface scratched 64-pin DIP)
       PIC16C5x (surface scratched, exact model unknown)
Sound: YM2203C
  OSC: 14.31818MHz, 12.288MHz
  DSW: 8 position DSW
   VR: Volume adjust pot
  RAM: SiS 6116-10 (x4)
       MCM2018ANS45 (x2)
       HY6264P-15

Actual Measured Clocks     Derived
     Z180 - 6.14522MHz  (12.288000MHz/2)
  YM2203C - 3.57543MHz  (14.318181MHz/4)
 PIC16C5x - 3.07262MHz  (12.288000MHz/4)

NOTE: PIC16C5x protection chip at 5A (UC02 as silkscreened on PCB)

***************************************************************************/

#include "emu.h"

#include "cpu/pic16c5x/pic16c5x.h"
#include "cpu/z180/z180.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


// configurable logging
#define LOG_PICSIM     (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_PICSIM)

#include "logmacro.h"

#define LOGPICSIM(...)     LOGMASKED(LOG_PICSIM,     __VA_ARGS__)


namespace {

class mosaic_state : public driver_device
{
public:
	mosaic_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_fgvideoram(*this, "fgvideoram"),
		m_bgvideoram(*this, "bgvideoram")
	{ }

	void mosaic(machine_config &config);
	void gfire2(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// devices
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;

	// memory pointers
	required_shared_ptr<uint8_t> m_fgvideoram;
	required_shared_ptr<uint8_t> m_bgvideoram;

	// video-related
	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;

	// misc
	uint16_t m_prot_val = 0;

	void protection_w(uint8_t data);
	uint8_t protection_r();
	void gfire2_protection_w(uint8_t data);
	uint8_t gfire2_protection_r();
	void fgvideoram_w(offs_t offset, uint8_t data);
	void bgvideoram_w(offs_t offset, uint8_t data);

	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void gfire2_io_map(address_map &map) ATTR_COLD;
	void gfire2_map(address_map &map) ATTR_COLD;
	void mosaic_io_map(address_map &map) ATTR_COLD;
	void mosaic_map(address_map &map) ATTR_COLD;
};


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(mosaic_state::get_fg_tile_info)
{
	tile_index *= 2;
	tileinfo.set(0,
			m_fgvideoram[tile_index] + (m_fgvideoram[tile_index + 1] << 8),
			0,
			0);
}

TILE_GET_INFO_MEMBER(mosaic_state::get_bg_tile_info)
{
	tile_index *= 2;
	tileinfo.set(1,
			m_bgvideoram[tile_index] + (m_bgvideoram[tile_index + 1] << 8),
			0,
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void mosaic_state::video_start()
{
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(mosaic_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(mosaic_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_fg_tilemap->set_transparent_pen(0xff);
}


/***************************************************************************

  Memory handlers

***************************************************************************/

void mosaic_state::fgvideoram_w(offs_t offset, uint8_t data)
{
	m_fgvideoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset / 2);
}

void mosaic_state::bgvideoram_w(offs_t offset, uint8_t data)
{
	m_bgvideoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}



uint32_t mosaic_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


void mosaic_state::protection_w(uint8_t data) // TODO: hook up PIC dump and remove this simulation (PIC dump contains the exact values in this jumptable)
{
	if (!BIT(data, 7))
	{
		// simply increment given value
		m_prot_val = (data + 1) << 8;
	}
	else
	{
		static const int jumptable[] =
		{
			0x02be, 0x0314, 0x0475, 0x0662, 0x0694, 0x08f3, 0x0959, 0x096f,
			0x0992, 0x09a4, 0x0a50, 0x0d69, 0x0eee, 0x0f98, 0x1040, 0x1075,
			0x10d8, 0x18b4, 0x1a27, 0x1a4a, 0x1ac6, 0x1ad1, 0x1ae2, 0x1b68,
			0x1c95, 0x1fd5, 0x20fc, 0x212d, 0x213a, 0x21b6, 0x2268, 0x22f3,
			0x231a, 0x24bb, 0x286b, 0x295f, 0x2a7f, 0x2fc6, 0x3064, 0x309f,
			0x3118, 0x31e1, 0x32d0, 0x35f7, 0x3687, 0x38ea, 0x3b86, 0x3c9a,
			0x411f, 0x473f
		};

		m_prot_val = jumptable[data & 0x7f];
	}
}

uint8_t mosaic_state::protection_r()
{
	int const res = (m_prot_val >> 8) & 0xff;

	LOGPICSIM("%06x: protection_r %02x\n", m_maincpu->pc(), res);

	m_prot_val <<= 8;

	return res;
}

void mosaic_state::gfire2_protection_w(uint8_t data)
{
	LOGPICSIM("%06x: protection_w %02x\n", m_maincpu->pc(), data);

	switch(data)
	{
		case 0x01:
			// written repeatedly; no effect??
			break;
		case 0x02:
			m_prot_val = 0x0a10;
			break;
		case 0x04:
			m_prot_val = 0x0a15;
			break;
		case 0x06:
			m_prot_val = 0x80e3;
			break;
		case 0x08:
			m_prot_val = 0x0965;
			break;
		case 0x0a:
			m_prot_val = 0x04b4;
			break;
	}
}

uint8_t mosaic_state::gfire2_protection_r()
{
	int const res = m_prot_val & 0xff;

	m_prot_val >>= 8;

	return res;
}



void mosaic_state::mosaic_map(address_map &map)
{
	map(0x00000, 0x0ffff).rom();
	map(0x20000, 0x21fff).ram();
	map(0x22000, 0x22fff).ram().w(FUNC(mosaic_state::bgvideoram_w)).share(m_bgvideoram);
	map(0x23000, 0x23fff).ram().w(FUNC(mosaic_state::fgvideoram_w)).share(m_fgvideoram);
	map(0x24000, 0x241ff).ram().w("palette", FUNC(palette_device::write8)).share("palette");
}

void mosaic_state::gfire2_map(address_map &map)
{
	map(0x00000, 0x0ffff).rom();
	map(0x10000, 0x17fff).ram();
	map(0x22000, 0x22fff).ram().w(FUNC(mosaic_state::bgvideoram_w)).share(m_bgvideoram);
	map(0x23000, 0x23fff).ram().w(FUNC(mosaic_state::fgvideoram_w)).share(m_fgvideoram);
	map(0x24000, 0x241ff).ram().w("palette", FUNC(palette_device::write8)).share("palette");
}

void mosaic_state::mosaic_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x3f).nopw();    // Z180 internal registers
	map(0x30, 0x30).nopr(); // Z180 internal registers
	map(0x70, 0x71).rw("ymsnd", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0x72, 0x72).rw(FUNC(mosaic_state::protection_r), FUNC(mosaic_state::protection_w));
	map(0x74, 0x74).portr("P1");
	map(0x76, 0x76).portr("P2");
}

void mosaic_state::gfire2_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x3f).nopw();    // Z180 internal registers
	map(0x30, 0x30).nopr(); // Z180 internal registers
	map(0x70, 0x71).rw("ymsnd", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0x72, 0x72).rw(FUNC(mosaic_state::gfire2_protection_r), FUNC(mosaic_state::gfire2_protection_w));
	map(0x74, 0x74).portr("P1");
	map(0x76, 0x76).portr("P2");
}


static INPUT_PORTS_START( mosaic )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("DSW")
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x40, 0x00, "Bombs" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x40, "5" )
	PORT_DIPNAME( 0x20, 0x20, "Speed" )
	PORT_DIPSETTING(    0x20, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x02, 0x00, "Music" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x01, 0x00, "Sound" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( gfire2 )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Language ) )
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Korean ) )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x02, 0x02, "Bonus Time" )
	PORT_DIPSETTING(    0x00, "*2 +30" )
	PORT_DIPSETTING(    0x02, "*2 +50" )
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW )
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,4),
	8,
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{   RGN_FRAC(3,4)+0, RGN_FRAC(2,4)+0, RGN_FRAC(1,4)+0, RGN_FRAC(0,4)+0,
		RGN_FRAC(3,4)+8, RGN_FRAC(2,4)+8, RGN_FRAC(1,4)+8, RGN_FRAC(0,4)+8 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};

static GFXDECODE_START( gfx_mosaic )
	GFXDECODE_ENTRY( "fgtiles", 0, charlayout, 0, 1 )
	GFXDECODE_ENTRY( "bgtiles", 0, charlayout, 0, 1 )
GFXDECODE_END

void mosaic_state::machine_start()
{
	save_item(NAME(m_prot_val));
}

void mosaic_state::machine_reset()
{
	m_prot_val = 0;
}

void mosaic_state::mosaic(machine_config &config)
{
	// basic machine hardware
	HD64180RP(config, m_maincpu, XTAL(12'288'000));  // 6.144MHz - Verified
	m_maincpu->set_addrmap(AS_PROGRAM, &mosaic_state::mosaic_map);
	m_maincpu->set_addrmap(AS_IO, &mosaic_state::mosaic_io_map);
	m_maincpu->set_vblank_int("screen", FUNC(mosaic_state::irq0_line_hold));

	PIC16C55(config, "pic", XTAL(12'288'000) / 4);  // 3.072MHz - Verified
	//read_a().set(FUNC(mosaic_state::));
	//write_a().set(FUNC(mosaic_state::));
	//read_b().set(FUNC(mosaic_state::));
	//write_b().set(FUNC(mosaic_state::));
	//read_c().set(FUNC(mosaic_state::));
	//write_c().set(FUNC(mosaic_state::));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(8*8, 48*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(mosaic_state::screen_update));
	screen.set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_mosaic);
	PALETTE(config, "palette").set_format(palette_device::xRGB_555, 256);


	// sound hardware
	SPEAKER(config, "mono").front_center();

	ym2203_device &ymsnd(YM2203(config, "ymsnd", XTAL(14'318'181) / 4)); // 3.579545MHz - Verified
	ymsnd.port_a_read_callback().set_ioport("DSW");
	ymsnd.add_route(ALL_OUTPUTS, "mono", 0.50);
}

void mosaic_state::gfire2(machine_config &config)
{
	mosaic(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &mosaic_state::gfire2_map);
	m_maincpu->set_addrmap(AS_IO, &mosaic_state::gfire2_io_map);

	subdevice<pic16c55_device>("pic")->set_disable(); // no PIC decap yet
}



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( mosaic )
	ROM_REGION( 0x100000, "maincpu", 0 )    // 1024k for Z180 address space
	ROM_LOAD( "9.ua02", 0x00000, 0x10000, CRC(5794dd39) SHA1(28784371f4ca561e3c0fb74d1f0a204f58ccdd3a) ) // at PCB location 7F

	ROM_REGION( 0x400, "pic", 0 )
	ROM_LOAD( "pic16c55.uc02",    0x000, 0x400, CRC(62d1d85d) SHA1(167e1f39e85f0bbecc4374f3975aa0c41173f070) ) // decapped, presumed to be 16C55

	ROM_REGION( 0x40000, "fgtiles", 0 )
	ROM_LOAD( "1.u505", 0x00000, 0x10000, CRC(05f4cc70) SHA1(367cfa716b5d24663efcd98a4a80bf02ef28f2f8) ) // at PCB location 1L
	ROM_LOAD( "2.u506", 0x10000, 0x10000, CRC(78907875) SHA1(073b90e0303f7812e7e8f66bb798a7734cb36bb9) ) // at PCB location 1K
	ROM_LOAD( "3.u507", 0x20000, 0x10000, CRC(f81294cd) SHA1(9bce627bbe3940769776121fb4296f92ac4c7d1a) ) // at PCB location 1I
	ROM_LOAD( "4.u508", 0x30000, 0x10000, CRC(fff72536) SHA1(4fc5d0a79128dd49275bc4c4cc2dd7c587096fd8) ) // at PCB location 1G

	ROM_REGION( 0x40000, "bgtiles", 0 )
	ROM_LOAD( "5.u305", 0x00000, 0x10000, CRC(28513fbf) SHA1(e69051206cc3df470e7b2358c51cbbed294795f5) ) // at PCB location 1F
	ROM_LOAD( "6.u306", 0x10000, 0x10000, CRC(1b8854c4) SHA1(d49df2565d9ccda403fafb9e219d3603776e3d34) ) // at PCB location 1D
	ROM_LOAD( "7.u307", 0x20000, 0x10000, CRC(35674ac2) SHA1(6422a81034b6d34aefc8ca5d2926d3d3c3d7ff77) ) // at PCB location 1C
	ROM_LOAD( "8.u308", 0x30000, 0x10000, CRC(6299c376) SHA1(eb64b20268c06c97c4201c8004a759b6de42fab6) ) // at PCB location 1A
ROM_END

ROM_START( mosaica )
	ROM_REGION( 0x100000, "maincpu", 0 )    // 1024k for Z180 address space
	ROM_LOAD( "mosaic_9.a02", 0x00000, 0x10000, CRC(ecb4f8aa) SHA1(e45c074bac92d1d079cf1bcc0a6a081beb3dbb8e) ) // at PCB location 7F

	ROM_REGION( 0x400, "pic", 0 )
	ROM_LOAD( "pic16c55.uc02",    0x000, 0x400, CRC(62d1d85d) SHA1(167e1f39e85f0bbecc4374f3975aa0c41173f070) ) // decapped, presumed to be 16C55

	ROM_REGION( 0x40000, "fgtiles", 0 )
	ROM_LOAD( "1.u505", 0x00000, 0x10000, CRC(05f4cc70) SHA1(367cfa716b5d24663efcd98a4a80bf02ef28f2f8) ) // at PCB location 1L
	ROM_LOAD( "2.u506", 0x10000, 0x10000, CRC(78907875) SHA1(073b90e0303f7812e7e8f66bb798a7734cb36bb9) ) // at PCB location 1K
	ROM_LOAD( "3.u507", 0x20000, 0x10000, CRC(f81294cd) SHA1(9bce627bbe3940769776121fb4296f92ac4c7d1a) ) // at PCB location 1I
	ROM_LOAD( "4.u508", 0x30000, 0x10000, CRC(fff72536) SHA1(4fc5d0a79128dd49275bc4c4cc2dd7c587096fd8) ) // at PCB location 1G

	ROM_REGION( 0x40000, "bgtiles", 0 )
	ROM_LOAD( "5.u305", 0x00000, 0x10000, CRC(28513fbf) SHA1(e69051206cc3df470e7b2358c51cbbed294795f5) ) // at PCB location 1F
	ROM_LOAD( "6.u306", 0x10000, 0x10000, CRC(1b8854c4) SHA1(d49df2565d9ccda403fafb9e219d3603776e3d34) ) // at PCB location 1D
	ROM_LOAD( "7.u307", 0x20000, 0x10000, CRC(35674ac2) SHA1(6422a81034b6d34aefc8ca5d2926d3d3c3d7ff77) ) // at PCB location 1C
	ROM_LOAD( "8.u308", 0x30000, 0x10000, CRC(6299c376) SHA1(eb64b20268c06c97c4201c8004a759b6de42fab6) ) // at PCB location 1A
ROM_END

ROM_START( gfire2 )
	ROM_REGION( 0x100000, "maincpu", 0 )    // 1024k for Z180 address space
	ROM_LOAD( "goldf2_i.7e",         0x00000, 0x10000, CRC(a102f7d0) SHA1(cfde51d0e9e69e9653fdfd70d4e4f4649b662005) )

	ROM_REGION( 0x400, "pic", 0 )
	ROM_LOAD( "pic16c55.uc02",    0x000, 0x400, NO_DUMP ) // same sanded off chip as mosaic, verified on PCB pic

	ROM_REGION( 0x100000, "fgtiles", 0 )
	ROM_LOAD( "goldf2_a.1k",         0x00000, 0x40000, CRC(1f086472) SHA1(c776a734869b6bab317627bd15457a9fb18e1159) )
	ROM_LOAD( "goldf2_b.1j",         0x40000, 0x40000, CRC(edb0d40c) SHA1(624a71b42a2e6c7c55cf455395aa0ad9b3eaeb9e) )
	ROM_LOAD( "goldf2_c.1i",         0x80000, 0x40000, CRC(d0ebd486) SHA1(ff2bfc84bc622b437913e1861f7acb373c7844c8) )
	ROM_LOAD( "goldf2_d.1h",         0xc0000, 0x40000, CRC(2b56ae2c) SHA1(667f9093ed28ba1804583fb201c7e3b37f1a9927) )

	ROM_REGION( 0x80000, "bgtiles", 0 )
	ROM_LOAD( "goldf2_e.1e",         0x00000, 0x20000, CRC(61b8accd) SHA1(d6317b8b7ab33a2a78d388b87ddb8946e6c6df29) )
	ROM_LOAD( "goldf2_f.1d",         0x20000, 0x20000, CRC(49f77e53) SHA1(6e7c8f86cb368bf1a32f02f72e7b418684c847dc) )
	ROM_LOAD( "goldf2_g.1b",         0x40000, 0x20000, CRC(aa79f3bf) SHA1(c0b62f5de7e36ce1ef1de92ee6f63d8286815566) )
	ROM_LOAD( "goldf2_h.1a",         0x60000, 0x20000, CRC(a3519259) SHA1(9e1edb50ade4a4ddcd628a897f6fa712075a888b) )
ROM_END

} // anonymous namespace


GAME( 1990, mosaic,  0,      mosaic, mosaic, mosaic_state, empty_init, ROT0, "Space",                 "Mosaic",         MACHINE_SUPPORTS_SAVE )
GAME( 1990, mosaica, mosaic, mosaic, mosaic, mosaic_state, empty_init, ROT0, "Space (Fuuki license)", "Mosaic (Fuuki)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, gfire2,  0,      gfire2, gfire2, mosaic_state, empty_init, ROT0, "Topis Corp",            "Golden Fire II", MACHINE_SUPPORTS_SAVE )
