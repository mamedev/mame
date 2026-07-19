// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/*
    World Cup Volleyball 95

    How to get the version and region:
	Boot the game holding down player 2 button 1

    Emulation by Bryan McPhail, mish@tendril.co.uk
*/

#include "emu.h"
#include "deco156.h"
#include "deco16ic.h"
#include "decocrpt.h"
#include "decospr.h"

#include "cpu/arm/arm.h"
#include "machine/eepromser.h"
#include "sound/ymz280b.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class wcvol95_state : public driver_device
{
public:
	wcvol95_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_deco_tilegen(*this, "tilegen")
		, m_sprgen(*this, "spritegen")
		, m_palette(*this, "palette")
		, m_pf_rowscroll(*this, "pf%u_rowscroll", 1U, 0x800U, ENDIANNESS_LITTLE)
		, m_spriteram(*this, "spriteram", 0x1000U, ENDIANNESS_LITTLE)
		, m_io_eepromout(*this, "EEPROMOUT")
	{ }

	void init_wcvol95() ATTR_COLD;

	void wcvol95(machine_config &config) ATTR_COLD;

private:
	template <unsigned Layer> uint32_t pf_rowscroll_r(offs_t offset);
	template <unsigned Layer> void pf_rowscroll_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	uint32_t spriteram_r(offs_t offset);
	void spriteram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void vblank_interrupt(int state);

	DECO16IC_BANK_CB_MEMBER(bank_callback);
	DECOSPR_PRIORITY_CB_MEMBER(pri_callback);

	void descramble_sound() ATTR_COLD;

	void wcvol95_map(address_map &map) ATTR_COLD;

	/* devices */
	required_device<arm_cpu_device> m_maincpu;
	required_device<deco16ic_device> m_deco_tilegen;
	required_device<decospr_device> m_sprgen;
	required_device<palette_device> m_palette;

	/* memory */
	memory_share_array_creator<uint16_t, 2> m_pf_rowscroll;
	memory_share_creator<uint16_t> m_spriteram;

	required_ioport m_io_eepromout;
};


uint32_t wcvol95_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// sprites are flipped relative to tilemaps
	m_sprgen->set_flip_screen(true);

	screen.priority().fill(0, cliprect);
	bitmap.fill(0, cliprect);

	m_deco_tilegen->pf_update(m_pf_rowscroll[0], m_pf_rowscroll[1]);

	m_deco_tilegen->tilemap_2_draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
	m_sprgen->draw_sprites(bitmap, cliprect, m_spriteram, 0x800);
	m_deco_tilegen->tilemap_1_draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

/***************************************************************************/

template <unsigned Layer>
uint32_t wcvol95_state::pf_rowscroll_r(offs_t offset) { return m_pf_rowscroll[Layer][offset] | 0xffff0000; }
template <unsigned Layer>
void wcvol95_state::pf_rowscroll_w(offs_t offset, uint32_t data, uint32_t mem_mask) { mem_mask &= 0x0000ffff; COMBINE_DATA(&m_pf_rowscroll[Layer][offset]); }
uint32_t wcvol95_state::spriteram_r(offs_t offset) { return m_spriteram[offset] | 0xffff0000; }
void wcvol95_state::spriteram_w(offs_t offset, uint32_t data, uint32_t mem_mask) { mem_mask &= 0x0000ffff; COMBINE_DATA(&m_spriteram[offset]); }


void wcvol95_state::wcvol95_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x10001f).rw(m_deco_tilegen, FUNC(deco16ic_device::pf_control_dword_r), FUNC(deco16ic_device::pf_control_dword_w));
	map(0x110000, 0x111fff).rw(m_deco_tilegen, FUNC(deco16ic_device::pf1_data_dword_r), FUNC(deco16ic_device::pf1_data_dword_w));
	map(0x114000, 0x115fff).rw(m_deco_tilegen, FUNC(deco16ic_device::pf2_data_dword_r), FUNC(deco16ic_device::pf2_data_dword_w));
	map(0x120000, 0x120fff).rw(FUNC(wcvol95_state::pf_rowscroll_r<0>), FUNC(wcvol95_state::pf_rowscroll_w<0>));
	map(0x124000, 0x124fff).rw(FUNC(wcvol95_state::pf_rowscroll_r<1>), FUNC(wcvol95_state::pf_rowscroll_w<1>));
	map(0x130000, 0x137fff).ram();
	map(0x140000, 0x140003).portr("INPUTS");
	map(0x150000, 0x150003).portw("EEPROMOUT");
	map(0x160000, 0x161fff).rw(FUNC(wcvol95_state::spriteram_r), FUNC(wcvol95_state::spriteram_w));
	map(0x170000, 0x170003).noprw(); // Irq ack?
	map(0x180000, 0x180fff).readonly().w(m_palette, FUNC(palette_device::write16)).umask32(0x0000ffff).share("palette");
	map(0x1a0000, 0x1a0007).rw("ymz", FUNC(ymz280b_device::read), FUNC(ymz280b_device::write)).umask32(0x000000ff);
}


/***************************************************************************/

static INPUT_PORTS_START( wcvol95 )
	PORT_START("INPUTS")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x00080000, IP_ACTIVE_LOW )
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_UNUSED ) /* 'soundmask' */
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x01000000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_UNUSED ) //PORT_PLAYER(1)
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_UNUSED ) //PORT_PLAYER(1)
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_UNUSED ) //PORT_PLAYER(2)
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_UNUSED ) //PORT_PLAYER(2)
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_UNUSED ) //PORT_PLAYER(2)
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x00000001, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::di_write))
	PORT_BIT( 0x00000002, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::clk_write))
	PORT_BIT( 0x00000004, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::cs_write))
INPUT_PORTS_END


/**********************************************************************************/

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+8, RGN_FRAC(1,2), 8, 0 },
	{ STEP8(0,1) },
	{ STEP8(0,8*2) },
	16*8    /* every char takes 8 consecutive bytes */
};

static const gfx_layout tilelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+8, RGN_FRAC(1,2), 8, 0 },
	{ STEP8(8*2*16,1), STEP8(0,1) },
	{ STEP16(0,8*2) },
	64*8
};

static GFXDECODE_START( gfx_wcvol95 )
	GFXDECODE_ENTRY( "tiles", 0, charlayout,   0, 32 )    /* Characters 8x8 */
	GFXDECODE_ENTRY( "tiles", 0, tilelayout,   0, 32 )    /* Tiles 16x16 */
GFXDECODE_END

static GFXDECODE_START( gfx_wcvol95_spr )
	GFXDECODE_ENTRY( "sprites", 0, tilelayout, 512, 32 )    /* Sprites 16x16 */
GFXDECODE_END


/**********************************************************************************/

void wcvol95_state::vblank_interrupt(int state)
{
	m_maincpu->set_input_line(ARM_IRQ_LINE, state ? HOLD_LINE : CLEAR_LINE);
}

DECO16IC_BANK_CB_MEMBER(wcvol95_state::bank_callback)
{
	return ((bank >> 4) & 0x7) * 0x1000;
}

DECOSPR_PRIORITY_CB_MEMBER(wcvol95_state::pri_callback)
{
	switch (pri & 0xc000)
	{
		case 0x0000: return 0;
		case 0x4000: return 0xf0;
		case 0x8000: return 0xf0 | 0xcc;
		case 0xc000: return 0xf0 | 0xcc;
	}

	return 0;
}

void wcvol95_state::wcvol95(machine_config &config)
{
	/* basic machine hardware */
	ARM(config, m_maincpu, 28000000); /* Unconfirmed */
	m_maincpu->set_addrmap(AS_PROGRAM, &wcvol95_state::wcvol95_map);

	EEPROM_93C46_16BIT(config, "eeprom");

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(58);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(529));
	screen.set_size(40*8, 32*8);
	screen.set_visarea(0*8, 40*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(wcvol95_state::screen_update));
	screen.screen_vblank().set(FUNC(wcvol95_state::vblank_interrupt));

	GFXDECODE(config, "gfxdecode", m_palette, gfx_wcvol95);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 1024);

	DECO16IC(config, m_deco_tilegen);
	m_deco_tilegen->set_pf1_size(DECO_64x32);
	m_deco_tilegen->set_pf2_size(DECO_64x32);
	m_deco_tilegen->set_pf1_col_bank(0x00);
	m_deco_tilegen->set_pf2_col_bank(0x10);
	m_deco_tilegen->set_pf1_col_mask(0x0f);
	m_deco_tilegen->set_pf2_col_mask(0x0f);
	m_deco_tilegen->set_bank1_callback(FUNC(wcvol95_state::bank_callback));
	m_deco_tilegen->set_bank2_callback(FUNC(wcvol95_state::bank_callback));
	m_deco_tilegen->set_pf12_8x8_bank(0);
	m_deco_tilegen->set_pf12_16x16_bank(1);
	m_deco_tilegen->set_gfxdecode_tag("gfxdecode");

	DECO_SPRITE(config, m_sprgen, m_palette, gfx_wcvol95_spr);
	m_sprgen->set_pri_callback(FUNC(wcvol95_state::pri_callback));

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	ymz280b_device &ymz(YMZ280B(config, "ymz", 28000000 / 2));
	ymz.add_route(0, "speaker", 1.0, 0);
	ymz.add_route(1, "speaker", 1.0, 1);
}


/**********************************************************************************/

/*
World Cup Volley '95
Data East, 1995

PCB Layout

DE-0430-2
|----------------------------------------------|
|          MBX-03.13J                MBX-02.13A|
|       LC7881         28MHz   DE52            |
|             YMZ280B-F              MBX-01.12A|
|                         CY7C185              |
|                         CY7C185    MBX-00.9A |
|             WE-02                            |
|J                                             |
|A                                             |
|M                 6264                        |
|M                             DE141           |
|A     DE223       6264                        |
|                                              |
|                                       WE-00  |
|                              WE-01           |
|                                              |
|TEST_SW           PN01-0.4F   6264            |
|         93C46.3K             6264            |
|                  PN00-0.2F   6264     DE156  |
|                              6264            |
|----------------------------------------------|

Notes:
      - CPU DE156 is a custom encrypted ARM7-based chip. The clock input is 7.000MHz on pin 90
        The package is a Quad Flat Pack and has 100 pins.
      - YMZ280B-F clock: 14.000MHz (28 / 2)
        SANYO LC7881 clock: 2.3333MHz (28 / 12)
      - VSync: 58Hz
      - WE-00, WE-01 and WE-02 are PALs type GAL16V8

*/

ROM_START( wcvol95 )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* DE156 code (encrypted) */
	ROM_LOAD32_WORD( "pw00-0.2f",    0x000002, 0x080000, CRC(86765209) SHA1(f78d073610b630ba6aa2352da9b394ef8b8ef628) )
	ROM_LOAD32_WORD( "pw01-0.4f",    0x000000, 0x080000, CRC(3a0ee861) SHA1(568ab26e9985b0a63b10bb0f57d45e1f15593047) )

	ROM_REGION( 0x080000, "tiles", 0 )
	ROM_LOAD( "mbx-00.9a",    0x000000, 0x080000, CRC(a0b24204) SHA1(cec8089c6c635f23b3a4aeeef2c43f519568ad70) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "mbx-01.12a",    0x100000, 0x100000, CRC(73deb3f1) SHA1(c0cabecfd88695afe0f27c5bb115b4973907207d) )
	ROM_LOAD( "mbx-02.13a",    0x000000, 0x100000, CRC(3204d324) SHA1(44102f71bae44bf3a9bd2de7e5791d959a2c9bdd) )

	ROM_REGION( 0x200000, "ymz", 0 ) /* YMZ280B-F samples */
	ROM_LOAD( "mbx-03.13j",    0x00000, 0x200000,  CRC(061632bc) SHA1(7900ac56e59f4a4e5768ce72f4a4b7c5875f5ae8) )

//  ROM_REGION( 0x80, "user1", 0 ) /* eeprom */
//  ROM_LOAD( "93c46.3k",    0x00, 0x80, CRC(88f8e270) SHA1(cb82203ad38e0c12ea998562b7b785979726afe5) )

	ROM_REGION( 0x200, "gals", 0 )
	ROM_LOAD( "gal16v8b.10j.bin",    0x000, 0x117,  CRC(06bbcbd5) SHA1(f7adb4bca13bb799bc42411eb178edfdc11a76c7) )
	ROM_LOAD( "gal16v8b.5d.bin",     0x000, 0x117,  CRC(117784f0) SHA1(daf3720740621fc3af49333c96795718b693f4d2))
ROM_END


ROM_START( wcvol95j )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* DE156 code (encrypted) */
	ROM_LOAD32_WORD( "pn00-0.2f",    0x000002, 0x080000, CRC(c9ed2006) SHA1(cee93eafc42c4de7a1453c85e7d6bca8d62cdc7b) )
	ROM_LOAD32_WORD( "pn01-0.4f",    0x000000, 0x080000, CRC(1c3641c3) SHA1(60dddc3585e4dedb485f7505fee03495f615c0c0) )

	ROM_REGION( 0x080000, "tiles", 0 )
	ROM_LOAD( "mbx-00.9a",    0x000000, 0x080000, CRC(a0b24204) SHA1(cec8089c6c635f23b3a4aeeef2c43f519568ad70) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "mbx-01.12a",    0x100000, 0x100000, CRC(73deb3f1) SHA1(c0cabecfd88695afe0f27c5bb115b4973907207d) )
	ROM_LOAD( "mbx-02.13a",    0x000000, 0x100000, CRC(3204d324) SHA1(44102f71bae44bf3a9bd2de7e5791d959a2c9bdd) )

	ROM_REGION( 0x200000, "ymz", 0 ) /* YMZ280B-F samples */
	ROM_LOAD( "mbx-03.13j",    0x00000, 0x200000,  CRC(061632bc) SHA1(7900ac56e59f4a4e5768ce72f4a4b7c5875f5ae8) )

//  ROM_REGION( 0x80, "user1", 0 ) /* eeprom */
//  ROM_LOAD( "93c46.3k",    0x00, 0x80, CRC(88f8e270) SHA1(cb82203ad38e0c12ea998562b7b785979726afe5) )

	ROM_REGION( 0x200, "gals", 0 )
	ROM_LOAD( "gal16v8b.10j.bin",    0x000, 0x117,  CRC(06bbcbd5) SHA1(f7adb4bca13bb799bc42411eb178edfdc11a76c7) )
	ROM_LOAD( "gal16v8b.5d.bin",     0x000, 0x117,  CRC(117784f0) SHA1(daf3720740621fc3af49333c96795718b693f4d2))
ROM_END


ROM_START( wcvol95x )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* DE156 code (encrypted) */
	// no label markings were present
	ROM_LOAD32_WORD( "2f.bin",    0x000002, 0x080000, CRC(ac06633d) SHA1(5d37ca3050f35d5fc06f70e91b1522e325471585) )
	ROM_LOAD32_WORD( "4f.bin",    0x000000, 0x080000, CRC(e211f67a) SHA1(d008c2b809482f17ada608134357fa1205d767d4) )

	ROM_REGION( 0x080000, "tiles", 0 )
	ROM_LOAD( "mbx-00.9a",    0x000000, 0x080000, CRC(a0b24204) SHA1(cec8089c6c635f23b3a4aeeef2c43f519568ad70) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "mbx-01.12a",    0x100000, 0x100000, CRC(73deb3f1) SHA1(c0cabecfd88695afe0f27c5bb115b4973907207d) )
	ROM_LOAD( "mbx-02.13a",    0x000000, 0x100000, CRC(3204d324) SHA1(44102f71bae44bf3a9bd2de7e5791d959a2c9bdd) )

	ROM_REGION( 0x200000, "ymz", 0 ) /* YMZ280B-F samples */
	ROM_LOAD( "mbx-03.13j",    0x00000, 0x200000,  CRC(061632bc) SHA1(7900ac56e59f4a4e5768ce72f4a4b7c5875f5ae8) )

	ROM_REGION( 0x200, "gals", 0 )
	ROM_LOAD( "gal16v8b.10j.bin",    0x000, 0x117,  CRC(06bbcbd5) SHA1(f7adb4bca13bb799bc42411eb178edfdc11a76c7) )
	ROM_LOAD( "gal16v8b.5d.bin",     0x000, 0x117,  CRC(117784f0) SHA1(daf3720740621fc3af49333c96795718b693f4d2))
ROM_END


/**********************************************************************************/

void wcvol95_state::descramble_sound()
{
	uint8_t *rom = memregion("ymz")->base();
	int length = memregion("ymz")->bytes();
	std::vector<uint8_t> buf1(length);
	uint32_t x;

	for (x = 0; x < length; x++)
	{
		uint32_t addr;

		addr = bitswap<24> (x,23,22,21,0, 20,
							19,18,17,16,
							15,14,13,12,
							11,10,9, 8,
							7, 6, 5, 4,
							3, 2, 1 );

		buf1[addr] = rom[x];
	}

	memcpy(rom,&buf1[0],length);
}

void wcvol95_state::init_wcvol95()
{
	deco56_decrypt_gfx(machine(), "tiles"); /* 141 */
	deco156_decrypt(machine());
	descramble_sound();
}

} // anonymous namespace


/**********************************************************************************/

GAME( 1995, wcvol95,  0,       wcvol95, wcvol95, wcvol95_state, init_wcvol95, ROT0, "Data East Corporation", "World Cup Volley '95 (Asia Ver. 1.0)",                MACHINE_SUPPORTS_SAVE )
GAME( 1995, wcvol95j, wcvol95, wcvol95, wcvol95, wcvol95_state, init_wcvol95, ROT0, "Data East Corporation", "World Cup Volley '95 (Japan Ver. 1.0)",               MACHINE_SUPPORTS_SAVE )
GAME( 1995, wcvol95x, wcvol95, wcvol95, wcvol95, wcvol95_state, init_wcvol95, ROT0, "Data East Corporation", "World Cup Volley '95 Extra Version (Asia Ver. 2.0B)", MACHINE_SUPPORTS_SAVE )
