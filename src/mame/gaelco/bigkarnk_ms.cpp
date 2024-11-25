// license:BSD-3-Clause
// copyright-holders:David Haywood

/*
    Big Karnak (Modular System) (checksum 1BD1EFB)

    this appears to be a different revision than the gaelco.cpp version, so could be original code, or
    based off one that is otherwise undumped.

    as with most of the 'Modular System' setups, the hardware is heavily modified from the original
    and consists of a multi-board stack in a cage, hence different driver.

    BOARDS

 Board 5-1
 ___________________________________________________________
 |                      ________________  ________________  |
 |                      |               | |               | |
 |                      | IC21-27C010   | | IC27-27C010   | |
 |                      |_______________| |_______________| |
 |           ::::::::   ________________  ________________  |
 |                      |               | |               | |
 | __                   | IC20-27C010   | | IC26-27C010   | |
 | | |                  |_______________| |_______________| |
 | | |     __________   ________________  ________________  |
 | | |     |_GAL16V8|   |               | |               | |
 | | |        5148      | IC19-27C010   | | IC25-27C010   | |
 | | |                  |_______________| |_______________| |
 | | |     __________   ________________  ________________  |
 | | |     |_GAL16V8|   |               | |               | |
 | | |        5248      | EMPTY         | | EMPTY         | |
 | | |                  |_______________| |_______________| |
 | | |      _________   ________________  ________________  |
 | | |      |74LS138N   |               | |               | |
 | | |                  | EMPTY         | | EMPTY         | |
 | | |                  |_______________| |_______________| |
 | | |________________  ________________                    |
 | |_||               | |               |                   |
 |    | IC6-27C010    | | IC15-27C101   |                   |
 |    |_______________| |_______________|                   |
 |    ________________  ________________                    |
 |    |               | |               |                   |
 |    | IC5-27C010    | | IC14-27C101   |                   |
 |    |_______________| |_______________|                   |
 |    ________________  ________________                    |
 |    |               | |               |     MODULAR       |
 |    | IC4-27C010    | | IC13-27C101   |    SYSTEM 2       |
 |    |_______________| |_______________|                   |
 |    ________________  ________________                    |
 |    |               | |               |                   |
 |    | EMPTY         | | EMPTY         |                   |
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

 Board 7/4
 __________________________________________________________
 | _________  _________ __________  _________  _________ __|_
 | 74LS163AN| 74LS163AN||_GAL20V8_| |74F174N_| |74S189BN||   |
 |                         7648                          |   |
 | _________  _________  _________  _________  _________ |   |
 | |74LS157N| MC74F283N  |74LS273P| 74LS290B1| |74S189BN||   |
 |                                                       |   |
 | _________  _________  _________  _________  _________ |   |
 | |74LS393N| MC74F283N 74HCTLS373N 74LS298B1| |74S189BN||   |
 |                                                       |   |
 | _________  __________  _____________  _________       |   |
 | |74LS157N| |_PAL16V8_| |84256A-10L  | |74F174N_|      |   |
 | _________  __75FL____  |____________| _________       |   |
 | |74LS283B1 |74LS245B1| _____________  |74F174N_|      |   |
 | _________  __________  |84256A-10L  | __________      |   |
 | |74LS08N_| |74LS245B1| |____________| |74LS245B1|     |   |
 | _________  __________    __________   _________       |   |
 | |74LS20N | |74LS374B1|   |74LS273N |  |74LS74AN|      |   |
 |                                                       |   |
 | _________  __________    __________   _________       |___|
 | |74LS04N_| |PALCE16V8|   |74LS374N_|  |74LS157N|        |
 |               7348                                      |
 | _________  __________    __________   _________         |
 | |74LS00N_| |_GAL10V8_|   |74LS273N_| 74HCTLS367AN     __|_
 |               7248                                    |   |
 | _________  __________    __________   _________       |   |
 | |74LS08N_| |74LS273N |   |74LS374N_|  PALCE16V8|      |   |
 |                                          7448         |   |
 | _________  __________    __________   _________       |   |
 | |74LS74AN| |74LS273N_|   74LS367AN_|  |74LS373N|      |   |
 |                                                       |   |
 | _________  __________    __________   _________       |   |
 | |74LS32N_| |74LS374N_|   74LS367AN_| GAL20V8-25LP     |   |
 |                                          7148         |   |
 | _________  _________  _________  _________  _________ |   |
 | |74LS139N  |_74F74N_| |74LS157N| |74LS20N_| 74HCT373N |   |
 | _________  _________  _________  _________  _________ |   |
 | |74LS32N_| |74LS157N|  74HCT597E 74HCT597E 74HCT597E  |   |
 | _________  _________  _________  _________  _________ |   |
 | |74LS74AN| |74LS86B1| 74HCT597N  74HCT597N 74HCT597N  |   |
 | _________  _________  _________  _________  _________ |   |
 | |74LS74AN| |74LS86B1| 74HCT597N  74HCT597N  74HCT597N |   |
 |            _________  _________  _________  _________ |   |
 |            |74LS377N| 74HCT597N  74HCT597N  74HCT597N |   |
 |            _________  _________  _________  _________ |___|
 |            |74LS273N| 74LS169BN  74LS169BN  74LS169BN   |
 |_________________________________________________________|

Board 8
 __________________________________________________________________________________
 |           :::::::: <- Jumpers                                                   |
 |          ________________  ________________  ________________  ________________ |
 | _______  |               | |               | |               | |               ||
 | 74LS175N | EMPTY         | | EMPTY         | | EMPTY         | | EMPTY         ||
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
 | 74LS175N | EMPTY         | | EMPTY         | | EMPTY         | | EMPTY         ||
 |          |_______________| |_______________| |_______________| |_______________||
 |          ________________  ________________  ________________  ________________ |
 | _______  |               | |               | |               | |               ||
 | 74LS175N | EMPTY         | | EMPTY         | | EMPTY         | | EMPTY         ||
 |          |_______________| |_______________| |_______________| |_______________||
 |          ________________  ________________  ________________  ________________ |
 | _______  |               | |               | |               | |               ||
 | 74LS175N | KA-814        | | KA-821        | | KA-829        | | KA-836        ||
 |          |_______________| |_______________| |_______________| |_______________||
 | _______  ________________  ________________  ________________  ________________ |
 | 74LS175N |               | |               | |               | |               ||
 |          | KA-815        | | KA-822        | | KA-830        | | KA-837        ||
 |          |_______________| |_______________| |_______________| |_______________||
 |  _____________________________      _______  _________________________________  |
 |__|                            |____74LS138N_|                                |__|
    |____________________________|             |________________________________|

 CPU Board 6/1
 _____________________________________________
 |             _______________              __|_
 |             |              |             |   |
 |             | 84256A-10L   |             |   |
 |             |______________|   _______   |   |
 |  _______   ________________    74LS138N  |   |
 | 74LS367AB1 |               |             |   |
 |            | EMPTY         |             |   |
 |            |_______________|             |   |
 |            ________________    _______   |   |
 |  _______   |               |   74LS245B1 |   |
 | 74LS367AB1 | IC20-27C010   |             |   |
 |            |_______________|             |   |
 |            ________________              |   |
 |  _______   |               |   _______   |   |
 | 74LS138N   | IC17-27C010   |   74LS374N  |   |
 |            |_______________|             |   |
 |             _______________              |   |
 |             |              |   _______   |___|
 |  _______    | 84256A-10L   |   74LS245B1   |
 |  GAL16V8    |______________|               |
 |    606     ________________                |
 |  _______   |               |   _______     |
 | 74LS174N   | EMPTY         |   74LS374N    |
 |            |_______________|               |
 |  _______   ________________                |
 |  PAL16V8   |               |               |
 |    648     | IC11-27C010   |   _______     |
 |            |_______________|   74LS138N    |
 |            ________________                |
 |            |               |               |
 |            | IC8-27C010    |               |
 |            |_______________|   _______     |
 | ___________________________    74LS32N     |
 | |                          |               |
 | |       TS68000CP12        |   _______     |
 | |__________________________|   74LS20N     |
 |                                            |
 |  _______   ______   _______    _______     |
 |  |74S74N    XTAL   74LS368AN   74LS132N    |
 |           24.000MHz                        |
 |____________________________________________|


Board 51/1
 _____________________________________________
 |                                            |
 | __________  ________ ________ ________     |
 | |TO SUB 51| 74LS299N 74LS169N AM2149-35DC  |
 | |_________| ________ ________ ________     |
 |             74LS169J 74LS169N AM2149-35DC  |
 |                                            |
 | __________  ________ ________ ________     |
 | |TO SUB 51| 74LS158N 74LS169B1 82S129N <- P0502
 | |_________| ________ ________ ________     |
 |             74LS299N 74LS169B1 74LS244N    |
 |                                            |
 | __________  ________ ________ ________     |
 | |TO SUB 51| 74LS299N 74LS169B1 74LS244N    |
 | |_________| ________ ________ ________     |
 |             |74LS20N AM2149-35DC 74LS298N  |
 |                                            |
 | __________  ________ ________ ________     |
 | |TO SUB 51| 74LS299N AM2149-35DC 74LS298N  |
 | |_________|                                |
 |   ________  ________ ________ ________  __ |
 |   74LS273N  74LS00B1 |74LS86N 74LS244N  | ||
 |                                         | ||
_|_  ________  ________ ________ ________  |T||
|  | 74LS08B1  74LS158N 74LS74B1 |74LS20B1 |O||
|  |                                       | ||
|  | ________  ________ ________ ________  |S||
IC48->PAL16R6A 74LS393N 74LS368A 74LS373B1 |U||
|  |                                       |B||
|  | ________  ________ ________ ________  | ||
|  | 74LS138PC 74LS283N |UM2148_||_EMPTY_| |5||
|  |                                       |1||
|  | ________  ________ ________ ________  | ||
|  | 74LS175B1 74LS283N |UM2148_| 74LS273P | ||
|  |                                       | ||
|  | ________  ________ ________ ________  |_||
|  | 74LS298N 74LS157B1 74LS157B1 74LS273P    |
|  |                                          |
|  | ________  ________ ________ ________     |
|  | 74LS158N 74LS169B1 74LS169B1 74LS245N    |
|__|                                          |
 |____________________________________________|

Sound Board 9/2
 ____________   _____________   _______________
 |           |_|_|_|_|_|_|_|_|_|             __|_
 |                PRE-JAMMA            XTAL  |   |
 | TDA2003                          28.00000 MHz |
 |           ________   ________   ________  |   |
 |           CNY 74-4   CNY 74-4   74LS368AN |   |
 |                                           |   |
 |           ________   ________   ________  |   |
 |           CNY 74-4   CNY 74-4  |74S112N|  |   |
 |                                           |   |
 |           ________   ________   ________  |   |
 |           74LS245N   74LS245B1  74LS298B1 |   |
 |                                           |   |
 | _______   ________   ________   ________  |   |
 | |LM324N   OKI M5205  |GAL16V8   74LS298B1 |   |
 |                        9148               |   |
 |           ________   ________   ________  |   |
 |           74LS157N   74LS267AN  74LS298B1 |   |
 | _______                                   |___|
 | |LM324N   ________   ________   ________    |
 |           74LS273N   74LS138N  |74LS32N|    |
 | D  74                                       |
 | I  LS  ______        ________   ________    |
 | P  24  Y3014B        74LS174N   74LS174N    |
 | S  5N                                       |
 |                      ________   ________    |
 | D  74                74LS245N  CXK5814P-35L |
 | I  LS  __________                           |
 | P  24  |  YM3812 |   ________   ________    |
 | S  5N  |_________|   74LS374N   74LS367AN   |
 | _______   _________                         |
 | 74LS138N UM6116K-3L  ________   ________    |
 |         ____________ 74LS74AN  |GAL20V8|    |
 | _______ |IC11-27C101             9248       |
 | GAL16V8 |__________| _______ _______ ______ |
 |   9348  ____________ 74LS32N 74LS393N 74LS163AN
 | _______ |IC6-27C512|                        |
 |74LS273N |__________| _______ _______ ______ |
 |         ____________74LS393N 74F74N 74LS163AN
 | _______ |Z0840006PSC                        |
 | 74LS08N |_Z80 CPU__| _______ _______   ____ |
 |                     74LS74AN 74LS368AN XTAL |
 |________________________________________16.000 MHz

*/


#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"
#include "sound/msm5205.h"
#include "sound/ymopl.h"
#include "machine/gen_latch.h"
#include "machine/bankdev.h"


namespace {

class bigkarnk_ms_state : public driver_device
{
public:
	bigkarnk_ms_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_soundlatch(*this, "soundlatch"),
		m_msm(*this, "msm"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen"),
		m_paletteram(*this, "palette"),
		m_gfxdecode(*this, "gfxdecode"),
		m_spriteram(*this, "spriteram"),
		m_videoram1(*this, "videoram1"),
		m_videoram2(*this, "videoram2"),
		m_videoram3(*this, "videoram3"),
		m_scrollregs(*this, "scrollregs"),
		m_soundrom(*this, "soundrom")
	{ }

	void bigkarnkm(machine_config &config);
	void init_bigkarnkm();

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<msm5205_device> m_msm;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;

	required_shared_ptr<uint16_t> m_paletteram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_videoram1;
	required_shared_ptr<uint16_t> m_videoram2;
	required_shared_ptr<uint16_t> m_videoram3;
	required_shared_ptr<uint16_t> m_scrollregs;
	required_device<address_map_bank_device> m_soundrom;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void bigkarnkm_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
	void soundrom_map(address_map &map) ATTR_COLD;

	uint16_t vram1_r(offs_t offset, uint16_t mem_mask);
	uint16_t vram2_r(offs_t offset, uint16_t mem_mask);
	uint16_t vram3_r(offs_t offset, uint16_t mem_mask);

	void vram1_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	void vram2_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	void vram3_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	TILE_GET_INFO_MEMBER(get_tile_info_tilemap1);
	TILE_GET_INFO_MEMBER(get_tile_info_tilemap2);
	TILE_GET_INFO_MEMBER(get_tile_info_tilemap3);

	tilemap_t *m_bg_tilemap1 = nullptr;
	tilemap_t *m_bg_tilemap2 = nullptr;
	tilemap_t *m_bg_tilemap3 = nullptr;

	void splash_msm5205_int(int state);
	void splash_adpcm_data_w(uint8_t data);
	void splash_adpcm_control_w(uint8_t data);
	int m_adpcm_data = 0;

	void descramble_16x16tiles(uint8_t* src, int len);
};

TILE_GET_INFO_MEMBER(bigkarnk_ms_state::get_tile_info_tilemap1)
{
	int tile = m_videoram1[tile_index*2];
	int attr = m_videoram1[(tile_index*2)+1] & 0x1f;
//  int fx = (m_videoram1[(tile_index*2)+1] & 0xc0)>>6;

	// we rearranged the tile order for the 16x16 deode, so have to swap back here
	tile = ((tile & 0x300) >> 8) | ((tile & 0xff) << 2) | (tile & 0xfc00);

	tileinfo.set(1,tile,attr,0);
}


TILE_GET_INFO_MEMBER(bigkarnk_ms_state::get_tile_info_tilemap2)
{
	int tile = m_videoram2[tile_index*2];

	tile &= 0x1fff;

	int attr = m_videoram2[(tile_index*2)+1] & 0xff;
	//int fx = (m_videoram2[(tile_index*2)+1] & 0xc0)>>6;

	int col = attr & 0x1f;


	tileinfo.set(0,tile,col,0);
}

TILE_GET_INFO_MEMBER(bigkarnk_ms_state::get_tile_info_tilemap3)
{
	int tile = m_videoram3[tile_index*2];

	tile &= 0x1fff;

	int attr = m_videoram3[(tile_index*2)+1] & 0xff;
	//int fx = (m_videoram3[(tile_index*2)+1] & 0xc0)>>6;

	int col = attr & 0x1f;

	tileinfo.set(0,tile,col,0);
}

uint16_t bigkarnk_ms_state::vram1_r(offs_t offset, uint16_t mem_mask)
{
	return m_videoram1[offset];
}

void bigkarnk_ms_state::vram1_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_videoram1[offset]);
	m_bg_tilemap1->mark_tile_dirty(offset/2);
}

uint16_t bigkarnk_ms_state::vram2_r(offs_t offset, uint16_t mem_mask)
{
	return m_videoram2[offset];
}

void bigkarnk_ms_state::vram2_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_videoram2[offset]);
	m_bg_tilemap2->mark_tile_dirty(offset/2);
}


uint16_t bigkarnk_ms_state::vram3_r(offs_t offset, uint16_t mem_mask)
{
	return m_videoram3[offset];
}

void bigkarnk_ms_state::vram3_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_videoram3[offset]);
	m_bg_tilemap3->mark_tile_dirty(offset/2);
}



void bigkarnk_ms_state::video_start()
{
	m_bg_tilemap1 = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(bigkarnk_ms_state::get_tile_info_tilemap1)), TILEMAP_SCAN_ROWS,  8,  8, 64, 32);
	m_bg_tilemap2 = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(bigkarnk_ms_state::get_tile_info_tilemap2)), TILEMAP_SCAN_ROWS,  16,  16, 32, 32);
	m_bg_tilemap3 = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(bigkarnk_ms_state::get_tile_info_tilemap3)), TILEMAP_SCAN_ROWS,  16,  16, 32, 32);

	m_bg_tilemap1->set_transparent_pen(15);
	m_bg_tilemap2->set_transparent_pen(0);

}



void bigkarnk_ms_state::bigkarnkm_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();

	map(0x080000, 0x081fff).rw(FUNC(bigkarnk_ms_state::vram1_r), FUNC(bigkarnk_ms_state::vram1_w)).share("videoram1");

	map(0x090000, 0x090fff).rw(FUNC(bigkarnk_ms_state::vram2_r), FUNC(bigkarnk_ms_state::vram2_w)).share("videoram2");
	map(0x091000, 0x091fff).ram();

	map(0x0a0000, 0x0a0fff).rw(FUNC(bigkarnk_ms_state::vram3_r), FUNC(bigkarnk_ms_state::vram3_w)).share("videoram3");

	map(0x0c0000, 0x0c000f).ram().share("scrollregs");

	map(0x100000, 0x100fff).ram().share("spriteram");

	map(0x102000, 0x103fff).ram();

	map(0x200000, 0x2007ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");

	map(0x400000, 0x400001).portr("IN0");
	map(0x400002, 0x400003).portr("IN1");
	map(0x400006, 0x400007).portr("IN2");
	map(0x400008, 0x400009).portr("IN3");

	map(0x40000c, 0x40000d).noprw();
	map(0x40000e, 0x40000e).w(m_soundlatch, FUNC(generic_latch_8_device::write));

	map(0xff0000, 0xffffff).ram();
}


void bigkarnk_ms_state::machine_start()
{
}


uint32_t bigkarnk_ms_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	m_bg_tilemap3->set_scrollx(0, 112-(m_scrollregs[6]-0x4));
	m_bg_tilemap3->set_scrolly(0, -m_scrollregs[7]);

	m_bg_tilemap2->set_scrollx(0, 112-(m_scrollregs[0]-0x2));
	m_bg_tilemap2->set_scrolly(0, -m_scrollregs[1]);

	m_bg_tilemap1->set_scrollx(0, 112-(m_scrollregs[2]));
	m_bg_tilemap1->set_scrolly(0, -m_scrollregs[3]);

	m_bg_tilemap3->draw(screen, bitmap, cliprect, 0, 0);
	m_bg_tilemap2->draw(screen, bitmap, cliprect, 0, 0);


	// TODO, convert to device, share between Modualar System games
	const int NUM_SPRITES = 0x200;
	const int X_EXTRA_OFFSET = 112;

	for (int i = NUM_SPRITES-2; i >= 0; i-=2)
	{
		gfx_element *gfx = m_gfxdecode->gfx(2);

		uint16_t attr0 = m_spriteram[i + 0];
		uint16_t attr1 = m_spriteram[i + 1];

		uint16_t attr2 = m_spriteram[i + NUM_SPRITES];
		//uint16_t attr3 = m_spriteram[i + NUM_SPRITES + 1]; // unused?

		int ypos = attr0 & 0x00ff;
		int xpos = (attr1 & 0xff00)>>8;
		xpos |= (attr2 & 0x8000) ? 0x100 : 0x000;

		ypos = (0xff - ypos);
		ypos |= (attr2 & 0x4000) ? 0x100 : 0x000; // maybe

		int tile = (attr0 & 0xff00) >> 8;
		tile |= (attr1 & 0x003f) << 8;

		int flipx = (attr1 & 0x0040);
		int flipy = (attr1 & 0x0080);

		gfx->transpen(bitmap,cliprect,tile,(attr2&0x0f00)>>8,flipx,flipy,xpos-16-X_EXTRA_OFFSET,ypos-16,15);
	}

	m_bg_tilemap1->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

static INPUT_PORTS_START( bigkarnkm )

	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_DIPNAME( 0x000f, 0x000f, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:8,7,6,5")
	PORT_DIPSETTING(      0x0007, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0009, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x000f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x000e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x000d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x000b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x000a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0000, "Free Play (if Coin B too)" )
	PORT_DIPNAME( 0x00f0, 0x00f0, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:4,3,2,1")
	PORT_DIPSETTING(      0x0070, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0090, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x00f0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x00e0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x00d0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x00b0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x00a0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0000, "Free Play (if Coin A too)" )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")
	PORT_DIPNAME( 0x0007, 0x0006, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:8,7,6")
	PORT_DIPSETTING(      0x0007, "0" )
	PORT_DIPSETTING(      0x0006, "1" )
	PORT_DIPSETTING(      0x0005, "2" )
	PORT_DIPSETTING(      0x0004, "3" )
	PORT_DIPSETTING(      0x0003, "4" )
	PORT_DIPSETTING(      0x0002, "5" )
	PORT_DIPSETTING(      0x0001, "6" )
	PORT_DIPSETTING(      0x0000, "7" )
	PORT_DIPNAME( 0x0018, 0x0008, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:5,4")
	PORT_DIPSETTING(      0x0018, "1" )
	PORT_DIPSETTING(      0x0010, "2" )
	PORT_DIPSETTING(      0x0008, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Impact" ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW2:1" )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )
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


static GFXDECODE_START( gfx_bigkarnk_ms )
	GFXDECODE_ENTRY( "bgtile", 0, tiles16x16x4_layout, 0, 32 )
	GFXDECODE_ENTRY( "bgtile", 0, tiles8x8x4_layout, 0, 32 )
	GFXDECODE_ENTRY( "sprites", 0, tiles16x16x4_layout, 0x200, 32 )
GFXDECODE_END

void bigkarnk_ms_state::splash_adpcm_data_w(uint8_t data)
{
	m_adpcm_data = data;
}

void bigkarnk_ms_state::splash_adpcm_control_w(uint8_t data)
{
	m_msm->reset_w(BIT(data, 7));

	int bank = data & 0x7f;

	if ((bank != 0x02) &&
		(bank != 0x04) && (bank != 0x05) && (bank != 0x06) && (bank != 0x07) &&
		(bank != 0x0c) && (bank != 0x0d) && (bank != 0x0e) && (bank != 0x0f))
	{
		logerror("splash_adpcm_control_w %02x\n", data);
	}

	m_soundrom->set_bank(bank & 0xf);
}

void bigkarnk_ms_state::splash_msm5205_int(int state)
{
	m_msm->data_w(m_adpcm_data >> 4);
	m_adpcm_data = (m_adpcm_data << 4) & 0xf0;
}
void bigkarnk_ms_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();

	map(0x8000, 0xbfff).m(m_soundrom, FUNC(address_map_bank_device::amap8));

	map(0xe000, 0xe000).w(FUNC(bigkarnk_ms_state::splash_adpcm_control_w));
	map(0xe400, 0xe400).w(FUNC(bigkarnk_ms_state::splash_adpcm_data_w));

	map(0xe800, 0xe801).rw("ymsnd", FUNC(ym3812_device::read), FUNC(ym3812_device::write));

	map(0xf000, 0xf7ff).ram();
	map(0xf800, 0xf800).r(m_soundlatch, FUNC(generic_latch_8_device::read));
}

void bigkarnk_ms_state::soundrom_map(address_map &map)
{
	map(0x00000, 0x3ffff).rom().region("soundcpu", 0x000000);
}


void bigkarnk_ms_state::bigkarnkm(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 12_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &bigkarnk_ms_state::bigkarnkm_map);
	m_maincpu->set_vblank_int("screen", FUNC(bigkarnk_ms_state::irq6_line_hold));

	Z80(config, m_soundcpu, 16_MHz_XTAL/4);
	m_soundcpu->set_addrmap(AS_PROGRAM, &bigkarnk_ms_state::sound_map);
	m_soundcpu->set_periodic_int(FUNC(bigkarnk_ms_state::nmi_line_pulse), attotime::from_hz(60*64));

	ADDRESS_MAP_BANK(config, m_soundrom).set_map(&bigkarnk_ms_state::soundrom_map).set_options(ENDIANNESS_LITTLE, 8, 18, 0x4000);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	m_screen->set_size(512, 256);
	m_screen->set_visarea(0, 320-1, 0, 256-16-1);
	m_screen->set_screen_update(FUNC(bigkarnk_ms_state::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBRG_444, 0x400);

	GFXDECODE(config, m_gfxdecode, "palette", gfx_bigkarnk_ms);

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_soundcpu, INPUT_LINE_IRQ0);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	YM3812(config, "ymsnd", XTAL(16'000'000)/4).add_route(ALL_OUTPUTS, "mono", 0.80);

	MSM5205(config, m_msm, XTAL(384'000));
	m_msm->vck_legacy_callback().set(FUNC(bigkarnk_ms_state::splash_msm5205_int));
	m_msm->set_prescaler_selector(msm5205_device::S48_4B);
	m_msm->add_route(ALL_OUTPUTS, "mono", 0.80);
}


// reorganize graphics into something we can decode with a single pass
void bigkarnk_ms_state::descramble_16x16tiles(uint8_t* src, int len)
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

void bigkarnk_ms_state::init_bigkarnkm()
{
	descramble_16x16tiles(memregion("bgtile")->base(), memregion("bgtile")->bytes());
}


void bigkarnk_ms_state::machine_reset()
{
	m_soundrom->set_bank(0);
}



ROM_START( bigkarnkm )
	ROM_REGION( 0x080000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "cpu_ka_6.ic8",   0x000001, 0x020000, CRC(ab71c1d3) SHA1(3174f1c68e4aa5b6053b118da1fed1f4001193b0) )
	ROM_LOAD16_BYTE( "cpu_ka_6.ic17",  0x000000, 0x020000, CRC(9f5c0dac) SHA1(a8089f58e34d7ba581303f7cf819297d21867a6a) )
	ROM_LOAD16_BYTE( "cpu_ka_6.ic11",  0x040001, 0x020000, CRC(30674ef3) SHA1(d1b29337068ed7323c104a48de593c9ac4668e66) )
	ROM_LOAD16_BYTE( "cpu_ka_6.ic20",  0x040000, 0x020000, CRC(332d6dea) SHA1(cd7e402642f57c12cb7405c49b75bfaa0d104421) )

	ROM_REGION( 0x040000, "soundcpu", 0 )    /* Z80 code (uses YM3812 + M5205) */
	ROM_LOAD( "snd_ka.ic6",    0x000000, 0x010000, CRC(48a66be8) SHA1(0ca8e4ef5b5e257d56afda6946c5f2a0712917a3) ) // 0,1,2,3
	ROM_LOAD( "snd_ka.ic11",   0x010000, 0x010000, CRC(8e53a6b8) SHA1(5082bbcb042216a6d58c654a52c98d75df700ac8) ) // 4,5,6,7
	ROM_CONTINUE(0x30000,0x10000) // c,d,e,f

	ROM_REGION( 0x180000, "sprites", ROMREGION_ERASEFF | ROMREGION_INVERT ) // sprites (same rom subboard type as galpanic_ms.cpp)
	ROM_LOAD32_BYTE( "5_ka.ic4",         0x080003, 0x020000, CRC(2bee07ea) SHA1(afd8769955314768db894e4e98f65422fc0dbb4f) )
	ROM_LOAD32_BYTE( "5_ka.ic13",        0x080002, 0x020000, CRC(d55e3024) SHA1(71c84a76b08f8983f65ac4b99430eeb30dc3f8ea) )
	ROM_LOAD32_BYTE( "5_ka.ic19",        0x080001, 0x020000, CRC(fc682c21) SHA1(c3fa9907fbe276bc4b74b79dda52713e702e441c) )
	ROM_LOAD32_BYTE( "5_ka.ic25",        0x080000, 0x020000, CRC(1157b739) SHA1(fdea10f808f258409e19e41dedfb3d4699e7daa2) )

	ROM_LOAD32_BYTE( "5_ka.ic5",         0x100003, 0x020000, CRC(507056f7) SHA1(e754c803f85862a37d2a48be6554ff5bc4128b4d) )
	ROM_LOAD32_BYTE( "5_ka.ic14",        0x100002, 0x020000, CRC(ef936e76) SHA1(ed93b04a45c38c0fa7333f182beba33fafe17f38) )
	ROM_LOAD32_BYTE( "5_ka.ic20",        0x100001, 0x020000, CRC(38854cd6) SHA1(b32efc1c621a9d8559c294f7431c219a05a37db6) )
	ROM_LOAD32_BYTE( "5_ka.ic26",        0x100000, 0x020000, CRC(3f63c4ed) SHA1(e0dd3ec27e7aa0b7db1587e83d20d1b9333ca405) )

	ROM_LOAD32_BYTE( "5_ka.ic6",         0x000003, 0x020000, CRC(2fdbc484) SHA1(6e8ac1a8bde8189b7ebf32c59185425c512ab911) )
	ROM_LOAD32_BYTE( "5_ka.ic15",        0x000002, 0x020000, CRC(802128e4) SHA1(20cfdf28aa7ada404ceca236c6eb554dcaa8e633) )
	ROM_LOAD32_BYTE( "5_ka.ic21",        0x000001, 0x020000, CRC(5ccc0f99) SHA1(ae2b2d4b2aa77a099ad2711032e6a05ab52789b9) )
	ROM_LOAD32_BYTE( "5_ka.ic27",        0x000000, 0x020000, CRC(55509d96) SHA1(ddd064695ca7e8c2377f13484e385bf7ea7df610) )

	ROM_REGION( 0x100000, "bgtile", 0 )
	ROM_LOAD32_BYTE( "8_ka_815.ic15",      0x000003, 0x020000, CRC(59d79b33) SHA1(70b9c60a72e517ac70f807c918f0ad4dd6c98f98) )
	ROM_LOAD32_BYTE( "8_ka_822.ic22",      0x000002, 0x020000, CRC(12fc89c0) SHA1(883144d0c453cd8f829b2209d9a8028b7f87d0d5) )
	ROM_LOAD32_BYTE( "8_ka_830.ic30",      0x000001, 0x020000, CRC(9904ae87) SHA1(5df3b35185c53a64c0647d297a19b9c013a3b3c2) )
	ROM_LOAD32_BYTE( "8_ka_837.ic37",      0x000000, 0x020000, CRC(f475eaa7) SHA1(8e5c7f0231d7f84bc377b756b99d055a4791e3bf) )
	ROM_LOAD32_BYTE( "8_ka_814.ic14",      0x080003, 0x020000, CRC(50e6cab6) SHA1(5af8b27f35a59611484ea35a2883b1e59d5c7517) )
	ROM_LOAD32_BYTE( "8_ka_821.ic21",      0x080002, 0x020000, CRC(90c1d93e) SHA1(581a1e2f30e8b467c8d8f5c8e528c78c0c3904f2) )
	ROM_LOAD32_BYTE( "8_ka_829.ic29",      0x080001, 0x020000, CRC(8c5df0ec) SHA1(15a5b847d6d035f27300435a03bd254dd9b3f99c) )
	ROM_LOAD32_BYTE( "8_ka_836.ic36",      0x080000, 0x020000, CRC(43de75db) SHA1(419e7702d17c52365addb8bfda582e916762ead5) )

	ROM_REGION( 0x100, "prom", ROMREGION_ERASEFF )
	ROM_LOAD( "51_p0502_n82s129n.ic10",      0x000, 0x100, CRC(15085e44) SHA1(646e7100fcb112594023cf02be036bd3d42cc13c) ) // common PROM found on all? Modular System sets?

	ROM_REGION( 0x100000, "pals", 0 )
	ROM_LOAD( "51_p0503_pal16r6acn.ic48",       0x000, 0x104, CRC(07eb86d2) SHA1(482eb325df5bc60353bac85412cf45429cd03c6d) ) // matches one of the Euro League Modular System PALs

	ROM_REGION( 0x100, "protgal", 0 ) // all read protected
	ROM_LOAD( "5_5148_gal16v8-25lnc.ic9", 0, 1, NO_DUMP )
	ROM_LOAD( "5_5248_gal16v8-25lnc.ic8", 0, 1, NO_DUMP )
	ROM_LOAD( "7_75flv_gal16v8-25hb1.ic59", 0, 1, NO_DUMP )
	ROM_LOAD( "7_7148_gal20v8-25lp.ic7", 0, 1, NO_DUMP )
	ROM_LOAD( "7_7248_gal20v8-25lp.ic54", 0, 1, NO_DUMP )
	ROM_LOAD( "7_7348_gal16v8-25hb1.ic55", 0, 1, NO_DUMP )
	ROM_LOAD( "7_7448_gal16v8-25hb1.ic9", 0, 1, NO_DUMP )
	ROM_LOAD( "7_7648_gal20v8-25lp.ic44", 0, 1, NO_DUMP )
	ROM_LOAD( "cpu_606_gal16v8-25hb1.ic13", 0, 1, NO_DUMP )
	ROM_LOAD( "cpu_648_gal16v8-25ln.ic7", 0, 1, NO_DUMP )
	ROM_LOAD( "snd_9148_gal16v8-25hb1.ic142", 0, 1, NO_DUMP )
	ROM_LOAD( "snd_9248_gal20v8-25lp.ic18", 0, 1, NO_DUMP )
	ROM_LOAD( "snd_9348_gal16v8-25hb1.ic10", 0, 1, NO_DUMP )
ROM_END

} // anonymous namespace


GAME( 1991, bigkarnkm,  bigkarnk,  bigkarnkm,  bigkarnkm,  bigkarnk_ms_state, init_bigkarnkm, ROT0, "Gaelco", "Big Karnak (Modular System)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
