// license:BSD-3-Clause
// copyright-holders:David Haywood, Angelo Salese

/*
    Toki (Modular System)

    as with most of the 'Modular System' setups, the hardware is heavily modified from the original
    and consists of a multi-board stack in a cage, hence different driver.
    Update: it doesn't seem all that different than tokib, the differences are:
    * paletteram;
    * video registers;
    * sprite chip interface (basically shifting all the ports, why even?);
    * switching from YM3812 to a double YM2203 setup;

    TODO:
    - Subclass with tokib_state in toki.cpp driver;
    - Sound needs improvements:
        * meaning of port A/B of the two YMs;
        * MSM playback;
        * improve comms;
        * ROM bank;
        * One of the above for the scratchy sounds that happens from time to time;
        * Mixing;
    - Ranking screen has wrong colors, btanb?
    - Slight sprite lag when game scrolls vertically, another btanb?
    - "bajo licencia" -> "under license" ... from whoever developed Modular System or TAD itself?

    NOTES:
    PCB lacks raster effect on title screen (like toki bootlegs)

    BOARDS

 Board 5-1
 ___________________________________________________________
 |                      ________________  ________________  |
 |                      |               | |               | |
 |                      | TK-512        | | TK-516        | |
 |                      |_______________| |_______________| |
 |           ::::::::   ________________  ________________  |
 |                      |               | |               | |
 | __                   | TK-511        | | TK-515        | |
 | | |                  |_______________| |_______________| |
 | | |     __________   ________________  ________________  |
 | | |     |_GAL16V8|   |               | |               | |
 | | |        5140      | TK-510        | | TK-514        | |
 | | |                  |_______________| |_______________| |
 | | |     __________   ________________  ________________  |
 | | |     |_GAL16V8|   |               | |               | |
 | | |        5240      | TK-509        | | TK-513        | |
 | | |                  |_______________| |_______________| |
 | | |      _________   ________________  ________________  |
 | | |      |74LS138N   |               | |               | |
 | | |                  | EMPTY         | | EMPTY         | |
 | | |                  |_______________| |_______________| |
 | | |________________  ________________                    |
 | |_||               | |               |                   |
 |    | TK-504        | | TK-508        |                   |
 |    |_______________| |_______________|                   |
 |    ________________  ________________                    |
 |    |               | |               |                   |
 |    | TK-503        | | TK-507        |                   |
 |    |_______________| |_______________|                   |
 |    ________________  ________________                    |
 |    |               | |               |     MODULAR       |
 |    | TK-502        | | TK-506        |    SYSTEM 2       |
 |    |_______________| |_______________|                   |
 |    ________________  ________________                    |
 |    |               | |               |                   |
 |    | TK-501        | | TK-505        |                   |
 |    |_______________| |_______________|                   |
 |    ________________  ________________                    |
 |    |               | |               |                   |
 |    | EMPTY         | | EMPTY         |                   |
 |    |_______________| |_______________|                   |
 |    ________________  ________________                    |
 |    |               | |               |                   |
 |    | EMPTY         | | EMPTY         |                   |
 |    |_______________| |_______________|                   |
 |__________________________________________________________|

 Board 8
 __________________________________________________________________________________
 |           :::::::: <- Jumpers                                                   |
 |          ________________  ________________  ________________  ________________ |
 | _______  |               | |               | |               | |               ||
 | 74LS175N | TK-825        | | TK-826        | | TK-827        | | TK-828        ||
 |          |_______________| |_______________| |_______________| |_______________||
 | _______  ________________  ________________  ________________  ________________ |
 | 74LS175N |               | |               | |               | |               ||
 |          | EMPTY         | | EMPTY         | | EMPTY         | | EMPTY         ||
 | _______  |_______________| |_______________| |_______________| |_______________||
 | 74LS175N ________________  ________________  ________________  ________________ |
 |          |               | |               | |               | |               ||
 | _______  | EMPTY         | | EMPTY         | | EMPTY         | | EMPTY         ||
 | 74LS175N |_______________| |_______________| |_______________| |_______________||
 |          ________________  ________________  ________________  ________________ |
 | _______  |               | |               | |               | |               ||
 | 74LS175N | TK-813        | | TK-814        | | TK-815        | | TK-816        ||
 |          |_______________| |_______________| |_______________| |_______________||
 |          ________________  ________________  ________________  ________________ |
 | _______  |               | |               | |               | |               ||
 | 74LS175N | TK-809        | | TK-810        | | TK-811        | | TK-812        ||
 |          |_______________| |_______________| |_______________| |_______________||
 |          ________________  ________________  ________________  ________________ |
 | _______  |               | |               | |               | |               ||
 | 74LS175N | TK-805        | | TK-806        | | TK-807        | | TK-808        ||
 |          |_______________| |_______________| |_______________| |_______________||
 | _______  ________________  ________________  ________________  ________________ |
 | 74LS175N |               | |               | |               | |               ||
 |          | TK-801        | | TK-803        | | TK-805        | | TK-807        ||
 |          |_______________| |_______________| |_______________| |_______________||
 |  _____________________________      _______  _________________________________  |
 |__|                            |____74LS138N_|                                |__|
    |____________________________|             |________________________________|

 CPU Board 6/1                                       Board 21/1
 _____________________________________________       ______________________________________________
 |             _______________              __|_     |                                             |
 |             |              |             |   |    |                                             |
 |             |CXK58256PM-12 |             |   |    |   ________   _______  ________              |
 |             |______________|   _______   |   |    |   |_EMPTY_| |_EMPTY_| |_EMPTY_|             |
 |  _______   ________________    74LS138N  |   |    |                                             |
 | 74LS367AN  |               |             |   |    |                                             |
 |            | TK-601        |             |   |    |                                             |
 |            |_______________|             |   |    |   ________   _______  ________              |
 |            ________________    _______   |   |    |   |74S74N_| 74LS393B1 74LS393B1             |
 |  _______   |               |   74LS245B1 |   |    |                                             |
 | 74LS367AN  | TK-602        |             |   |    |                                             |
 |            |_______________|             |   |    |                                             |
 |            ________________              |   |    |                                             |
 |  _______   |               |   _______   |   |    |   74LS7273N |82S129_| |82S129_|             |
 | 74LS138N   | TK-603        |   74LS374N  |   |    |                202       201                |
 |            |_______________|             |   |    |                                             |
 |             _______________              |   |    |                                             |
 |             |              |   _______   |___|    |   ________   _______  ________              |
 |  _______    |CXK58256PM-12 |   74LS245B1   |      |   74LS74AN    XTAL    74LS367AB1            |
 |  GAL16V8    |______________|               |      |             20.000 MHz                      |
 |    604     ________________                |      |                                             |
 |  _______   |               |   _______     |      |   ________   _______  ________              |
 | 74LS174N   | TK-604        |   74LS374N    |    __|_  74LS732B1 74LS368AB1 NMC2148HJ-3          |
 |            |_______________|               |    |   |                                           |
 |  _______   ________________                |    |   |                                           |
 |  PAL16V8   |               |               |    |   |  ________   _______  ________             |
 |    640     | TK-605        |   _______     |    |   |  74LS157N  74S112N  AM2148-45DC           |
 |            |_______________|   74LS138N    |    |   |                                           |
 |            ________________                |    |   |                                           |
 |            |               |               |    |   |  ________   _______  ________             |
 |            | TK-606        |               |    |   |  74LS148N  74LS368AB1 AM2148-45DC         |
 |            |_______________|   _______     |    |   |                                           |
 | ___________________________    74LS32N     |    |   |                                           |
 | |                          |               |    |   |  ________   _______  ________   ________  |
 | |       TS68000CP12        |   _______     |    |   |  74LS298N  74LS298N  74LS298N   74LS174N  |
 | |__________________________|   74LS20P     |    |   |                                           |
 |                                            |    |   |                                           |
 |  _______   ______   _______    _______     |    |   |  ________   _______  ________   ________  |
 |  |74F74N    XTAL   74LS368AB1  74LS132B1   |    |   |  74LS245B1 74LS245B1 |74LS08N   74LS174N  |
 |           20.000MHz                        |    |___|                                           |
 |____________________________________________|      |_____________________________________________|

 Board 51/3                                          Sound Board 1/3
 _____________________________________________       ______________________________________________
 |                                            |      |                                             |
 | __________  ________ ________ ________     |      |  ________  ________  ________               |
 | |TO SUB 51| 74LS299N 74LS169N |D2149D_|    |      |  74LS107AN GD74LS32| |_EMPTY_|              |
 | |_________| ________ ________ ________     |      |                                             |
 |             74LS169N 74LS169N |D2149D_|    |      |  ________  __________________               |
 |                                            |      | 74LS368AB1 |                 |              |
 | __________  ________ ________ ________     |      |            | EMPTY           |              |
 | |TO SUB 51| 74LS158N 74LS169N |82S129AN <- 502    |  ________  |_________________|              |
 | |_________| ________ ________ ________     |      |  74LS74AN  __________________               |
 |             74LS299N 74LS169N 74LS244N     |      |            |  Z8400BB1       |              |
 |                                            |      |  ________  |  Z80 B CPU      |  ________    |
 | __________  ________ ________ ________     |      | 74HCT157P  |_________________|  |LM324M_|   |
 | |TO SUB 51| 74LS299N 74LS169N 74LS244N     |      |            ________________                 |
 | |_________| ________ ________ ________     |      |  ________  |               |             __ |
 |             74LS20B1 |D2149D| 74LS298P     |  P0110->N82S123N  | TK-101        |             |D||
 |                                            |      |  ________  |_______________|             |I||
 | __________  ________ ________ ________     |      | 74HCT157P  ________________              |P||  SOUND SUB
 | |TO SUB 51| 74LS299N |D2149D| 74LS298P     |      | __________ |               | _____ _____ |S|| _________________
 | |_________|                                |      | |         || SRM2064C-15   |Y3014B Y3014B|-|| | ______          |
 |   ________  ________ ________ ________  __ |      | | SOUND   ||_______________|             |D|| | LM358N     ___  |
 |   74LS273P  |74LS00N 74LS86B1 74LS244N  | ||      | |  SUB    |                              |I|| | .......... |  | |
 |                                         | ||      | |         |                              |P|| |            |OKI |
_|_  ________  ________ ________ ________  |T||    __|_|_________|__________________            |S|| | .......... |M5205
|  | |74LS08N  74LS158N 74LS74AN 74LS20B1  |O||    |   |          | YAMAHA          |           |_|| | ________   |__| |
|  |                                       | ||    |   |          | YM2203C         |              | | 74LS377N        |
|  | ________  ________ ________ ________  |S||    |   |          |_________________|              | |_________________|
IC46->PAL16V8H 74LS393N 74LS368AP 74LS377B1|U||    |   |                                           |
|  |                                       |B||    |   | _______  ________                         |
|  | ________  ________ ________ ________  | ||    |   | |EMPTY_| 74LS74AN                         |
|  | 74LS138N  74LS283N |D2149D| |_EMPTY_| |5||    |   |                                           |
|  |                                       |1||    |   |                                           |
|  | ________  ________ ________ ________  | ||    |   |          __________________    _____      |
|  | 74LS175B1 74LS283N |D2149D| 74LS273B1 | ||    |   | _______  | YAMAHA          |  TDA2003     |
|  |                                       | ||    |   | |EMPTY|  | YM2203C         |              |
|  | ________  ________ ________ ________  |_||    |   |          |_________________|              |
|  | 74LS298N  74LS157N 74LS157N 74LS273B1    |    |   |                                           |
|  |                                          |    |   |                                           |
|  | ________  ________ ________ ________     |    |   |                                           |
|  | 74LS158P  74LS169N 74LS169N 74LS245N     |    |   |                                           |
|__|                                          |    |___|        __________________________         |
 |____________________________________________|      |__________| |_|_|_|_|_|_|_|_|_|_|_| |________|
                                                                          PRE-JAMMA
 Board 7/4
 __________________________________________________________
 | _________  _________ __________  _________  _________ __|_
 | 74LS163AN| 74LS163AN||_GAL20V8_| |74S174N_| |74S189BN||   |
 |                         7636                          |   |
 | _________  _________  _________  _________  _________ |   |
 | |74LS157N| |74LS288B1 |74LS273P| 74LS290B1| |74S189VN||   |
 |                                                       |   |
 | _________  _________  _________  _________  _________ |   |
 | |74LS393N| 74LS283B1 74HCTLS373N 74LS298B1| |74S189BN||   |
 |                                                       |   |
 | _________  __________  _____________  _________       |   |
 | |74LS157N| |_PAL16V8_| |D43256C-12L | |74S174N_|      |   |
 | _________  __7540____  |____________| _________       |   |
 | |74LS283B1 |74LS245B1| _____________  |74S174N_|      |   |
 | _________  __________  |D43256C-12L | __________      |   |
 | |74LS08N_| |74LS245B1| |____________| |74LS245B1|     |   |
 | _________  __________    __________   _________       |   |
 | |74LS20B1| |74LS374B1|   |74LS273B1|  |74LS74AN|      |   |
 |                                                       |   |
 | _________  __________    __________   _________       |___|
 | |74LS04N_| |PALCE16V8|   |74LS374B1|  |74LS157N|        |
 |               7340                                      |
 | _________  __________    __________   _________         |
 | |74LS00B1| |_GAL10V8_|   |74LS273B1|  74LS367AN       __|_
 |               7240                                    |   |
 | _________  __________    __________   _________       |   |
 | |74LS08N_| |74LS273B1|   |74LS374B1|  PALCE16V8|      |   |
 |                                          7440         |   |
 | _________  __________    __________   _________       |   |
 | |74LS74AN| |74LS273B1|   74LS367AN_|  74HCTLS373N     |   |
 |                                                       |   |
 | _________  __________    __________   _________       |   |
 | |74LS32N_| |74LS374B1|   74LS367AN_| GAL20V8-25LP     |   |
 |                                          7140         |   |
 | _________  _________  _________  _________  _________ |   |
 | 74LS139B1  |_74F74N_| |74LS157N| |74LS20B1| 74HCT373N |   |
 | _________  _________  _________  _________  _________ |   |
 | |74LS32N_| |74LS157N|  74HCT597E 74HCT597E 74HCT597E  |   |
 | _________  _________  _________  _________  _________ |   |
 | |74LS74AN| |74LS86N_|  74HCT597E 74HCT597E 74HCT597E  |   |
 | _________  _________  _________  _________  _________ |   |
 | |74LS74AN| |74LS86N_|  74HCT597E 74HCT597E 74HCT597E  |   |
 |            _________  _________  _________  _________ |   |
 |            74LS377B1   74HCT597E 74HCT597E 74HCT597E  |   |
 |            _________  _________  _________  _________ |___|
 |            74LS273B1  74LS169BN  74LS169BN  74LS169BN   |
 |_________________________________________________________|

*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"
#include "video/bufsprite.h"
#include "sound/msm5205.h"
#include "sound/ymopn.h"


namespace {

class toki_ms_state : public driver_device
{
public:
	toki_ms_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_palette(*this, "palette")
		, m_screen(*this, "screen")
		, m_gfxdecode(*this, "gfxdecode")
		, m_bk1vram(*this, "bk1vram")
		, m_bk2vram(*this, "bk2vram")
		, m_vram(*this, "vram")
		, m_spriteram(*this, "spriteram")
		, m_scrollram(*this, "scrollram")
		, m_ym1(*this, "ym1")
		, m_ym2(*this, "ym2")
		, m_msm(*this, "msm5205")
		, m_soundlatch(*this, "soundlatch")
	{ }

	void tokims(machine_config &config);
	void init_tokims();

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;

	required_device<gfxdecode_device> m_gfxdecode;
	required_shared_ptr<u16> m_bk1vram;
	required_shared_ptr<u16> m_bk2vram;
	required_shared_ptr<u16> m_vram;
	required_device<buffered_spriteram16_device> m_spriteram;
	required_shared_ptr<u16> m_scrollram;
	required_device<ym2203_device> m_ym1;
	required_device<ym2203_device> m_ym2;
	required_device<msm5205_device> m_msm;
	required_device<generic_latch_8_device> m_soundlatch;

	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	TILE_GET_INFO_MEMBER(get_bk1_info);
	TILE_GET_INFO_MEMBER(get_bk2_info);
	TILE_GET_INFO_MEMBER(get_tile_info);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);
	void bk1vram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void bk2vram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void vram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	tilemap_t *m_bk1_tilemap = nullptr;
	tilemap_t *m_bk2_tilemap = nullptr;
	tilemap_t *m_tx_tilemap = nullptr;

	u8 palette_r(offs_t offset);
	void palette_w(offs_t offset, u8 data);
	std::unique_ptr<u8 []> m_paletteram;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void tokims_map(address_map &map) ATTR_COLD;
	void audio_map(address_map &map) ATTR_COLD;

	u8 sound_status_r();
	void sound_command_w(u8 data);
	void adpcm_w(u8 data);
	void adpcm_int(int state);
	u8 m_adpcm_data = 0;

	void descramble_16x16tiles(uint8_t* src, int len);
};

TILE_GET_INFO_MEMBER(toki_ms_state::get_tile_info)
{
	u16 code = (m_vram[tile_index] & 0xfff) | 0x1000;
	u8 color = (m_vram[tile_index] >> 12);

	tileinfo.set(0, code, color, 0);
}

TILE_GET_INFO_MEMBER(toki_ms_state::get_bk1_info)
{
	int tile = m_bk1vram[tile_index];
	int color = (tile >> 12) & 0xf;

	tile &= 0xfff;

	tileinfo.set(2,
			tile,
			color,
			0);
}

TILE_GET_INFO_MEMBER(toki_ms_state::get_bk2_info)
{
	int tile = m_bk2vram[tile_index];
	int color = (tile >> 12) & 0xf;

	tile &= 0xfff;

	tileinfo.set(3,
			tile,
			color,
			0);
}

/*
Copies from original location to an 8-bit RAM, split into two banks like paletteram.
First entry are global flags (unchecked)
Offsets are in 16-bit to make this less confusing
[0x000] tttt tttt tttt tttt ---- ---- ---- ---- tile low offset
        ---- ---- ---- ---- yyyy yyyy yyyy yyyy Y offset
[0x002] xxxx xxxx xxxx xxxx ---- ---- ---- ---- X offset
        ---- ---- ---- ---- ---- -f-- ---- ---- flip X
        ---- ---- ---- ---- ---- --t- ---- ---- tile banking
        ---- ---- ---- ---- ---- ---- tttt tttt tile upper offset
[0x200] x--- ---- ---- ---- ---- ---- ---- ---- x wraparound
        ---- ---- ---- cccc ---- ---- ---- ---- color offset
[0x202] ---- ---- ---- ---- ---- ---- ---- ---- <unused>
*/
void toki_ms_state::draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	int x,y,flipx,color;
	u16 tile;
	const int bank_size = m_spriteram->bytes() / 4;

	for (int offs = bank_size-2; offs >= 2; offs -= 2)
	{
		u16 *lobank = &m_spriteram->buffer()[offs];
		u16 *hibank = &m_spriteram->buffer()[offs+bank_size];

		y = 240 - (lobank[0] & 0xff);

		x = lobank[1] >> 8;
		if (!(hibank[0] & 0x8000))
			x -= 256;

		flipx = (lobank[1] & 0x40);

		tile = (lobank[0] >> 8);
		tile |= ((lobank[1] & 0x0f) << 8);
		if (lobank[1] & 0x20)
			tile |= 0x1000;

		color = (hibank[0] & 0x0f00) >> 8;

		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
				tile,
				color,
				flipx,0,
				x,y,15);
	}
}

void toki_ms_state::video_start()
{
	m_tx_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(toki_ms_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bk1_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(toki_ms_state::get_bk1_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_bk2_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(toki_ms_state::get_bk2_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);

	m_bk1_tilemap->set_transparent_pen(15);
	m_bk2_tilemap->set_transparent_pen(15);
	m_tx_tilemap->set_transparent_pen(15);

	m_paletteram = std::make_unique<u8 []>(0x800);
	std::fill_n(m_paletteram.get(), 0x800, 0);
	save_pointer(NAME(m_paletteram), 0x800);
}

uint32_t toki_ms_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen());
	m_bk1_tilemap->set_scrollx(0, (0x33-m_scrollram[0]) & 0x1ff );
	m_bk1_tilemap->set_scrolly(0, (511-m_scrollram[1]) & 0x1ff );
	m_bk2_tilemap->set_scrollx(0, (0x31-m_scrollram[2]) & 0x1ff );
	m_bk2_tilemap->set_scrolly(0, (511-m_scrollram[3]) & 0x1ff );

	if (m_scrollram[4] & 1)
	{
		m_bk2_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		m_bk1_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	}
	else
	{
		m_bk1_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		m_bk2_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	}
	draw_sprites(bitmap, cliprect);
	m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

void toki_ms_state::bk1vram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_bk1vram[offset]);
	m_bk1_tilemap->mark_tile_dirty(offset);
}

void toki_ms_state::bk2vram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_bk2vram[offset]);
	m_bk2_tilemap->mark_tile_dirty(offset);
}

void toki_ms_state::vram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_vram[offset]);
	m_tx_tilemap->mark_tile_dirty(offset);
}

u8 toki_ms_state::palette_r(offs_t offset)
{
	return m_paletteram[offset];
}

void toki_ms_state::palette_w(offs_t offset, u8 data)
{
	m_paletteram[offset] = data;
	// xBGR444 over an 8-bit bus
	int r,g,b;
	u16 pal_entry = offset & 0x3ff;
	b = (m_paletteram[pal_entry|0x400] & 0xf);
	g = (m_paletteram[pal_entry] & 0xf0) >> 4;
	r = m_paletteram[pal_entry] & 0xf;

	m_palette->set_pen_color(pal_entry, pal4bit(r),pal4bit(g),pal4bit(b));
}

u8 toki_ms_state::sound_status_r()
{
	// if non-zero skips new command, either soundlatch readback or audiocpu handshake
	return 0;
}

// TODO: remove this trampoline after confirming it just writes to the sound latch
// (definitely it isn't tied to irq 0)
void toki_ms_state::sound_command_w(u8 data)
{
	m_soundlatch->write(data & 0xff);
}

void toki_ms_state::tokims_map(address_map &map)
{
	map(0x000000, 0x05ffff).rom();
	map(0x060000, 0x06dfff).ram();
	map(0x06e800, 0x06efff).ram().w(FUNC(toki_ms_state::bk1vram_w)).share("bk1vram");
	map(0x06f000, 0x06f7ff).ram().w(FUNC(toki_ms_state::bk2vram_w)).share("bk2vram");
	map(0x06f800, 0x06ffff).ram().w(FUNC(toki_ms_state::vram_w)).share("vram");
	map(0x070000, 0x0707ff).rw(FUNC(toki_ms_state::palette_r), FUNC(toki_ms_state::palette_w)).umask16(0xffff);
	map(0x071000, 0x0713ff).ram().share("spriteram");
	map(0x072000, 0x072001).nopr(); // irq ack?
	map(0x073000, 0x07300f).ram().share("scrollram");
	map(0x0a0000, 0x0a005f).nopw(); // scrollram left-over?
	map(0x0c0000, 0x0c0001).portr("DSW");
	map(0x0c0002, 0x0c0003).portr("INPUTS");
	map(0x0c0004, 0x0c0005).portr("SYSTEM");
	map(0x0c000e, 0x0c000f).rw(FUNC(toki_ms_state::sound_status_r), FUNC(toki_ms_state::sound_command_w)).umask16(0x00ff);
}

void toki_ms_state::adpcm_w(u8 data)
{
//  membank("sound_bank")->set_entry(((data & 0x10) >> 4) ^ 1);
	m_msm->reset_w(BIT(data, 4));

	m_adpcm_data = data & 0xf;
	//m_msm->data_w(data & 0xf);
//  m_msm->vclk_w(BIT(data, 7));
	//m_msm->vclk_w(1);
	//m_msm->vclk_w(0);
}


void toki_ms_state::audio_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x8000).w(FUNC(toki_ms_state::adpcm_w));
	map(0x8000, 0xbfff).bankr("sound_bank");
	map(0xc000, 0xc7ff).ram();
	map(0xd000, 0xd7ff).ram();
	// area 0xdff0-5 is never ever readback, applying a RAM mirror causes sound to go significantly worse,
	// what they are even for?  (offset select bankswitch rather than data select?)
	map(0xdfff, 0xdfff).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0xe000, 0xe001).w(m_ym1, FUNC(ym2203_device::write));
	map(0xe002, 0xe003).w(m_ym2, FUNC(ym2203_device::write));
	map(0xe008, 0xe009).r(m_ym1, FUNC(ym2203_device::read));
	map(0xe00a, 0xe00b).r(m_ym2, FUNC(ym2203_device::read));
}

static INPUT_PORTS_START( tokims )
	// TODO: same as tokib, subclass into tokib_io_device
	PORT_START("DSW")
	PORT_DIPNAME( 0x001f, 0x001f, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:1,2,3,4,5")
	PORT_DIPSETTING(      0x0015, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(      0x0017, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0019, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x001b, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 8C_3C ) )
	PORT_DIPSETTING(      0x001d, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 5C_3C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x001f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0009, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0013, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0011, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x000f, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x000d, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x000b, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x001e, "A 1/1 B 1/2" )
	PORT_DIPSETTING(      0x0014, "A 2/1 B 1/3" )
	PORT_DIPSETTING(      0x000a, "A 3/1 B 1/5" )
	PORT_DIPSETTING(      0x0000, "A 5/1 B 1/6" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0020, 0x0000, "Joysticks" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0020, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) )  PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0200, "2" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0100, "5" )
	PORT_DIPSETTING(      0x0000, "Infinite (Cheat)")
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0800, "50000 150000" )
	PORT_DIPSETTING(      0x0000, "70000 140000 210000" )
	PORT_DIPSETTING(      0x0c00, "70000" )
	PORT_DIPSETTING(      0x0400, "100000 200000" )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( Medium ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( On ) )

	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW,  IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW,  IPT_BUTTON2 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x0400, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x0800, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x1000, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x2000, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


static const gfx_layout tiles16x16x4_layout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0,8,16,24 },
	{ 0,1,2,3,4,5,6,7, 512,513,514,515,516,517,518,519 },
	{ STEP16(0,32) },
	16 * 16 * 4
};


static const gfx_layout tiles8x8x4_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0,8,16,24 },
	{ 0,1,2,3,4,5,6,7 },
	{ STEP8(0,32) },
	16 * 16
};


static GFXDECODE_START( gfx_toki_ms )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8x4_layout, 0x100, 16 )

	GFXDECODE_ENTRY( "sprites", 0, tiles16x16x4_layout, 0x000, 16 )
	GFXDECODE_ENTRY( "gfx3", 0, tiles16x16x4_layout, 0x200, 16 )
	GFXDECODE_ENTRY( "gfx4", 0, tiles16x16x4_layout, 0x300, 16 )
GFXDECODE_END


void toki_ms_state::machine_start()
{
	membank("sound_bank")->configure_entries(0, 2, memregion("audiocpu")->base() + 0x8000, 0x4000);
}

void toki_ms_state::adpcm_int(int state)
{
	m_msm->data_w(m_adpcm_data);
	m_audiocpu->set_input_line(0, HOLD_LINE);
}

void toki_ms_state::tokims(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 20_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &toki_ms_state::tokims_map);
	m_maincpu->set_vblank_int("screen", FUNC(toki_ms_state::irq1_line_hold));

	Z80(config, m_audiocpu, XTAL(4'000'000));
	m_audiocpu->set_addrmap(AS_PROGRAM, &toki_ms_state::audio_map);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(20_MHz_XTAL / 3, 426, 0, 256, 272, 16, 240); // FIXME: generic, not measured
	m_screen->set_screen_update(FUNC(toki_ms_state::screen_update));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set("spriteram", FUNC(buffered_spriteram16_device::vblank_copy_rising));

	PALETTE(config, m_palette).set_entries(0x400);

	GFXDECODE(config, m_gfxdecode, "palette", gfx_toki_ms);

	BUFFERED_SPRITERAM16(config, m_spriteram);

	GENERIC_LATCH_8(config, m_soundlatch);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	YM2203(config, m_ym1, XTAL(4'000'000)/4); // unknown clock
	m_ym1->add_route(0, "mono", 0.15);
	m_ym1->add_route(1, "mono", 0.15);
	m_ym1->add_route(2, "mono", 0.15);
	m_ym1->add_route(3, "mono", 0.10);

	YM2203(config, m_ym2, XTAL(4'000'000)/4); // unknown clock
	m_ym2->add_route(0, "mono", 0.15);
	m_ym2->add_route(1, "mono", 0.15);
	m_ym2->add_route(2, "mono", 0.15);
	m_ym2->add_route(3, "mono", 0.10);

	MSM5205(config, m_msm, XTAL(384'000)); // unknown clock
	m_msm->vck_legacy_callback().set(FUNC(toki_ms_state::adpcm_int));
	m_msm->set_prescaler_selector(msm5205_device::S48_4B); // unverified
	m_msm->add_route(ALL_OUTPUTS, "mono", 0.25);
}


ROM_START( tokims )
	ROM_REGION( 0x080000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "6_tk_606.ic8",   0x000001, 0x010000, CRC(13ecfa14) SHA1(77b8600e41b6dc51e4ab927c626599e24f9a7853) )
	ROM_LOAD16_BYTE( "6_tk_603.ic17",  0x000000, 0x010000, CRC(714f76ff) SHA1(f2d13dcba84d5b7dd2f156d63421c511c518fc82) )
	ROM_LOAD16_BYTE( "6_tk_605.ic11",  0x020001, 0x010000, CRC(03644aa7) SHA1(4c874c0fe47213b9597690f4a4805e281fed20ad) )
	ROM_LOAD16_BYTE( "6_tk_602.ic20",  0x020000, 0x010000, CRC(71234409) SHA1(9d476ddde30b9f9ca80578a2d3ca5b94da6ee2f0) )
	ROM_LOAD16_BYTE( "6_tk_604.ic25",  0x040001, 0x010000, CRC(9879fde6) SHA1(79f8020bbc8e1545466bd12c81117f736c985afe) )
	ROM_LOAD16_BYTE( "6_tk_601.ic26",  0x040000, 0x010000, CRC(9810f8f0) SHA1(5a3fa4599058a3ff3a97d20764005e6d530187f8) )

	ROM_REGION( 0x010000, "audiocpu", 0 )    /* Z80 code */
	ROM_LOAD( "1_tk_101.c19", 0x000000, 0x10000, CRC(a447a394) SHA1(ccaa6aca5c2afc7c05035cb551b8368b18188dd6) )

	ROM_REGION( 0x040000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "8_tk_825.ic9",      0x000003, 0x10000, CRC(6d04def0) SHA1(36f23b0893dfae6cf4c6f4414ff54bb13cfdad41) )
	ROM_LOAD32_BYTE( "8_tk_826.ic16",     0x000002, 0x10000, CRC(d3a2a038) SHA1(a2a020397a427f5fd401aad09048c7d4a21cd728) )
	ROM_LOAD32_BYTE( "8_tk_827.ic24",     0x000001, 0x10000, CRC(d254ae6c) SHA1(cdbdd7d7c6cd4de8b8a0f54e1543caba5f3d11cb) )
	ROM_LOAD32_BYTE( "8_tk_828.ic31",     0x000000, 0x10000, CRC(a6fae34b) SHA1(d9a276d30bdcc25d9cd299c2502cf910273890f6) )

	ROM_REGION( 0x100000, "sprites", ROMREGION_ERASEFF | ROMREGION_INVERT ) // sprites (same rom subboard type as galpanic_ms.cpp)
	ROM_LOAD32_BYTE( "5_tk_501.ic3",         0x000003, 0x010000, CRC(c3cd26b6) SHA1(20d5a68eada4150642365dd61c699b7771de5372) )
	ROM_LOAD32_BYTE( "5_tk_505.ic12",        0x000002, 0x010000, CRC(ec096351) SHA1(10417266c2280b2d9c301423d8c41ed73d9654c9) )
	ROM_LOAD32_BYTE( "5_tk_509.ic18",        0x000001, 0x010000, CRC(a1a4ef7b) SHA1(92aad84f14f8257477920012bd1fe033ec96301b) )
	ROM_LOAD32_BYTE( "5_tk_513.ic24",        0x000000, 0x010000, CRC(8dfda6fa) SHA1(ee2600d6cdcb27500e61dd1beebed904fd2c3ac5) )
	ROM_LOAD32_BYTE( "5_tk_502.ic4",         0x040003, 0x010000, CRC(122d59eb) SHA1(5dc9c55667021630f49cfb70c0c70bdf3ac1e3a7) )
	ROM_LOAD32_BYTE( "5_tk_506.ic13",        0x040002, 0x010000, CRC(ed92289f) SHA1(fe612e704bf6aefdbd85f1d49a9bbc4d0fef0f95) )
	ROM_LOAD32_BYTE( "5_tk_510.ic19",        0x040001, 0x010000, CRC(56eb4876) SHA1(113d2b300d7670068e3587f63b4f0b0bd38d84a3) )
	ROM_LOAD32_BYTE( "5_tk_514.ic25",        0x040000, 0x010000, CRC(b0c7801c) SHA1(99e898bcb4a8c4dc00726908f9095df512539776) )
	ROM_LOAD32_BYTE( "5_tk_503.ic5",         0x080003, 0x010000, CRC(9201545b) SHA1(dee1736946ec781ee035714281298f2e2a48fec1) )
	ROM_LOAD32_BYTE( "5_tk_507.ic14",        0x080002, 0x010000, CRC(e61eebbd) SHA1(1f854ba98a1cde4473107b8282b88e6412094d19) )
	ROM_LOAD32_BYTE( "5_tk_511.ic20",        0x080001, 0x010000, CRC(06d9fd86) SHA1(22472905672c956941d41b3e5febb4cb57c91283) )
	ROM_LOAD32_BYTE( "5_tk_515.ic26",        0x080000, 0x010000, CRC(04b575a7) SHA1(c6c65745511e27b594818e3f7ba7313c0a6f599e) )
	ROM_LOAD32_BYTE( "5_tk_504.ic6",         0x0c0003, 0x010000, CRC(cec71122) SHA1(283d38f998b1ca4fa080bf9fac797f5ac91dd072) )
	ROM_LOAD32_BYTE( "5_tk_508.ic15",        0x0c0002, 0x010000, CRC(1873ae38) SHA1(a1633ab5c417e9851e285a6b322c06e7d2d0bccd) )
	ROM_LOAD32_BYTE( "5_tk_512.ic21",        0x0c0001, 0x010000, CRC(0228110f) SHA1(33a29f9f458ca9d0af3c8da8a5b67bab79cecdec) )
	ROM_LOAD32_BYTE( "5_tk_516.ic27",        0x0c0000, 0x010000, CRC(f4e29429) SHA1(706050b51e0afbddf6ec5c8f14d3649bb05c8550) )

	ROM_REGION( 0x080000, "gfx3", 0 ) // same ROMs as some of the other Toki bootlegs
	ROM_LOAD32_BYTE( "8_tk_809.ic13",     0x000003, 0x10000, CRC(feb13d35) SHA1(1b78ce1e48d16e58ad0721b30ab87765ded7d24e) )
	ROM_LOAD32_BYTE( "8_tk_810.ic20",     0x000002, 0x10000, CRC(617c32e6) SHA1(a80f93c83a06acf836e638e4ad2453692622015d) )
	ROM_LOAD32_BYTE( "8_tk_811.ic28",     0x000001, 0x10000, CRC(fbc3d456) SHA1(dd10455f2e6c415fb5e39fb239904c499b38ca3e) )
	ROM_LOAD32_BYTE( "8_tk_812.ic35",     0x000000, 0x10000, CRC(46a1b821) SHA1(74d9762aef3891463dc100d1bc2d4fdc3c1d163f) )
	ROM_LOAD32_BYTE( "8_tk_813.ic12",     0x040003, 0x10000, CRC(5b365637) SHA1(434775b0614d904beaf40d7e00c1eaf59b704cb1) )
	ROM_LOAD32_BYTE( "8_tk_814.ic19",     0x040002, 0x10000, CRC(2a11c0f0) SHA1(f9b1910c4932f5b95e5a9a8e8d5376c7210bcde7) )
	ROM_LOAD32_BYTE( "8_tk_815.ic27",     0x040001, 0x10000, CRC(4c2a72e1) SHA1(52a31f88e02e1689c2fffbbd86cbccd0bdab7dcc) )
	ROM_LOAD32_BYTE( "8_tk_816.ic34",     0x040000, 0x10000, CRC(82ce27f6) SHA1(db29396a336098664f48e3c04930b973a6ffe969) )

	ROM_REGION( 0x080000, "gfx4", 0 ) // same ROMs as some of the other Toki bootlegs
	ROM_LOAD32_BYTE( "8_tk_801.ic15",     0x000003, 0x10000, CRC(63026cad) SHA1(c8f3898985d99f2a61d4e17eba66b5989a23d0d7) )
	ROM_LOAD32_BYTE( "8_tk_802.ic22",     0x000002, 0x10000, CRC(48989aa0) SHA1(109c68c9f0966862194226cecc8b269d9307dd25) )
	ROM_LOAD32_BYTE( "8_tk_803.ic30",     0x000001, 0x10000, CRC(6cd22b18) SHA1(8281cfd46738448b6890c50c64fb72941e169bee) )
	ROM_LOAD32_BYTE( "8_tk_804.ic37",     0x000000, 0x10000, CRC(e15c1d0f) SHA1(d0d571dd1055d7307379850313216da86b0704e6) )
	ROM_LOAD32_BYTE( "8_tk_805.ic14",     0x040003, 0x10000, CRC(a7f2ce26) SHA1(6b12b3bd872112b42d91ce3c0d5bc95c0fc0f5b5) )
	ROM_LOAD32_BYTE( "8_tk_806.ic21",     0x040002, 0x10000, CRC(c2ad9342) SHA1(7c9b5c14c8061e1a57797b79677741b1b98e64fa) )
	ROM_LOAD32_BYTE( "8_tk_807.ic29",     0x040001, 0x10000, CRC(859e313a) SHA1(18ac471a72b3ed42ba74456789adbe323f723660) )
	ROM_LOAD32_BYTE( "8_tk_808.ic36",     0x040000, 0x10000, CRC(6f4b878a) SHA1(4560b1e705a0eb9fad7fdc11fadf952ff67eb264) )

	ROM_REGION( 0x100, "protpal", 0 ) // all read protected
	ROM_LOAD( "5_5140_palce16v8h-25pc.ic9", 0, 1, NO_DUMP )
	ROM_LOAD( "5_5240_palce16v8h-25pc.ic8", 0, 1, NO_DUMP )
	ROM_LOAD( "6_604_gal16v8-20hb1.ic13", 0, 1, NO_DUMP )
	ROM_LOAD( "6_640_palce16v8h-25pc.ic7", 0, 1, NO_DUMP )
	ROM_LOAD( "7_7140_gal20v8-20hb1.ic7", 0, 1, NO_DUMP )
	ROM_LOAD( "7_7240_gal20v8-20hb1.ic54", 0, 1, NO_DUMP )
	ROM_LOAD( "7_7340_palce16v8h-25pc.ic55", 0, 1, NO_DUMP )
	ROM_LOAD( "7_7440_palce16v8h-25pc.ic9", 0, 1, NO_DUMP )
	ROM_LOAD( "7_7540_palce16v8h-25pc.ic59", 0, 1, NO_DUMP )
	ROM_LOAD( "7_7640_gal20v8-20hb1.ic44", 0, 1, NO_DUMP )
	ROM_LOAD( "51_503_palce16v8h-25pc.ic46", 0, 1, NO_DUMP )

	ROM_REGION( 0x400, "proms", ROMREGION_ERASE00 )
	ROM_LOAD( "51_502_82s129an.ic10", 0x0000, 0x100, CRC(15085e44) SHA1(646e7100fcb112594023cf02be036bd3d42cc13c) ) // same as every other modular bootleg
	ROM_LOAD( "21_201_82s129.ic4",    0x0100, 0x100, CRC(2697da58) SHA1(e62516b886ff6e204b718e5f0c6ce2712e4b7fc5) )
	ROM_LOAD( "21_202_82s129.ic12",   0x0200, 0x100, CRC(e434128a) SHA1(ef0f6d8daef8b25211095577a182cdf120a272c1) )
	ROM_LOAD( "1_10110_82s123.ic20",  0x0300, 0x020, CRC(e26e680a) SHA1(9bbe30e98e952a6113c64e1171330153ddf22ce7) )
ROM_END

// reorganize graphics into something we can decode with a single pass
void toki_ms_state::descramble_16x16tiles(uint8_t* src, int len)
{
	std::vector<uint8_t> buffer(len);
	{
		for (int i = 0; i < len; i++)
		{
			int j = bitswap<20>(i, 19,18,17,16,15,12,11,10,9,8,7,6,5,14,13,4,3,2,1,0);
			buffer[j] = src[i];
		}

		std::copy(buffer.begin(), buffer.end(), &src[0]);
	}
}

void toki_ms_state::init_tokims()
{
	descramble_16x16tiles(memregion("gfx3")->base(), memregion("gfx3")->bytes());
	descramble_16x16tiles(memregion("gfx4")->base(), memregion("gfx4")->bytes());
	// gfx3 is 8x8 tiles
}

} // anonymous namespace


GAME( 1991, tokims,  toki,  tokims,  tokims,  toki_ms_state, init_tokims, ROT0, "bootleg", "Toki (Modular System)", MACHINE_IMPERFECT_SOUND )
