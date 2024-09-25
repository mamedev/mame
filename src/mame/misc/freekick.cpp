// license:BSD-3-Clause
// copyright-holders: Tomasz Slanina,David Haywood

/***************************************************************************

Free Kick  - (c) 1987 Sega / Nihon System (made by Nihon, licensed to Sega)

Driver by Tomasz Slanina
based on initial work made by David Haywood

Z80 @ 3MHz (12.000/4)
IRQ frequency 120Hz, low for 4.02us, high for 8.1879ms
4*SN76489AN @ 3MHz (12.000/4)

12MHz (mclk), 6MHz pixel clock (12.000/2)
263 scanlines per frame - 224 visible + 39 blanking+sync;
16 lines bottom border, 7 lines vsync, 16 lines top border
768 mclks scanline - 512 mclks visible, 256 mclks blanking+sync;
96 mclks left border, 96 mclks right border, 64 mclks sync


Notes:
- Quite interestingly, Free Kick's sound ROM contains a Z80 program, but
  there isn't a sound CPU and that program isn't executed. Instead, the main
  CPU reads the sound program through an 8255 PPI and plays sounds directly.
- Gigas: according to a photo of a genuine board:
    Sega Game ID#: "834-6167 // GIGAS"
    NEC MC-8123 with Sega security number 317-5002
- Gigas MarkII: according to a photo of a genuine board:
    Sega Game ID#: "834-6167-01 // GIGAS 2"
    NEC MC-8123 with Sega security number 317-5002

TODO:
- Proper cocktail mode / flipscreen support for all games in driver

****************************************************************************

 ---

 $78d        - checksum calculations
 $d290,$d291 - level (color set , level number  - BCD)
 $d292       - lives

 To enter Test Mode - press Button1 durning RESET (code at $79d)

****************************************************************************

        Perfect Billiard/Gigas                        Omega
        ----------------------                        -----

           GND  A1   B1  GND                    GND  1A   1B  GND
           GND  A2   B2  GND                    GND  2A   2B  GND
           +5V  A3   B3  +5V                    +5V  3A   3B  GND
           +5V  A4   B4  +5V                    +5V  4A   4B  GND
          +12V  A5   B5  +12V                  +12V  5A   5B  GND
    SPEAKER(+)  A6   B6  SPEAKER(-)      SPEAKER(+)  6A   6B  SPEAKER(-)
  COIN METER 1  A7   B7  COIN METER 2        COIN A  7A   7B  COUNTER B
      COIN SW1  A8   B8  COIN SW2         COUNTER A  8A   8B  COIN B
      1P START  A9   B9  2P START          1P START  9A   9B  2P START
         1P UP  A10 B10  2P UP        COIN EMPTY SW  10A 10B
       1P DOWN  A11 B11  2P DOWN                     11A 11B
       1P LEFT  A12 B12  2P LEFT                     12A 12B
      1P RIGHT  A13 B13  2P RIGHT                    13A 13B
      1P SHOOT  A14 B14  2P SHOOT          1P SHOOT  14A 14B  2P SHOOT
                A15 B15                              15A 15B
           RED  A16 B16  BLUE                   RED  16A 16B  BLUE
         GREEN  A17 B17  SYNC                 GREEN  17A 17B  SYNC
           GND  A18 B18  GND                    GND  18A 18B  GND
                                               1P L  19A 19B  2P L
                                               1P R  20A 20B  2P R
                                                +5V  21A 21B  GND
                                              AC IN  22A 22B  AC OUT

                         Counter Run/Free Kick
                         ---------------------

                           GND  1B   1A  GND
                           GND  2B   2A  GND
                           +5V  3B   3A  +5V
                           +5V  4B   4A  +5V
                                5B   5A
                          +12V  6B   6A  +12V
          INPUT PREVENTION KEY  7B   7A  INPUT PREVENTION KEY
                     COUNTER B  8B   8A  COUNTER A
                                9B   9A
                    SPEAKER(-)  10B 10A  SPEAKER(+)
                                11B 11A
                         GREEN  12B 12A  RED
                          SYNC  13B 13A  BLUE
                                14B 14A  GND
                                15B 15A
               COIN B(SERVICE)  16B 16A  COIN A
                      2P START  17B 17A  1P START
                         2P UP  18B 18A  1P UP
                       2P DOWN  19B 19A  1P DOWN
                       2P LEFT  20B 20A  1P LEFT
                      2P RIGHT  21B 21A  1P RIGHT
                     2P PUSH 1  22B 22A  1P PUSH 1
                     2P PUSH 2  23B 23A  1P PUSH 2
                                24B 24A
             2PL (Sensor Dial)  25B 25A  1PL (Sensor Dial)
             2PR (Sensor Dial)  26B 26A  1PR (Sensor Dial)
                           GND  27B 27A  GND
                           GND  28B 28A  GND

****************************************************************************

*/

#include "emu.h"

#include "cpu/z80/mc8123.h"
#include "cpu/z80/z80.h"
#include "machine/74259.h"
#include "machine/i8255.h"
#include "sound/sn76496.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class base_state : public driver_device
{
public:
	base_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_outlatch(*this, "outlatch"),
		m_spinner(*this, "IN%u", 2U) { }

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	// memory pointers
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<ls259_device> m_outlatch;

	optional_ioport_array<2> m_spinner;

	// video-related
	tilemap_t *m_tilemap = nullptr;

	// misc
	uint8_t m_spinner_sel = 0;
	uint8_t m_nmi_en = 0;

	void base(machine_config &config);

	template <uint8_t Which> void coin_w(int state);
	uint8_t spinner_r();
	void nmi_enable_w(int state);
	void videoram_w(offs_t offset, uint8_t data);
	TILE_GET_INFO_MEMBER(get_tile_info);
	void vblank_irq(int state);
};

class freekick_state : public base_state
{
public:
	freekick_state(const machine_config &mconfig, device_type type, const char *tag)
		: base_state(mconfig, type, tag),
		m_sound_data(*this, "sound_data") { }

	void freekick(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_region_ptr<uint8_t> m_sound_data;

	uint16_t m_romaddr = 0;
	uint8_t m_ff_data = 0; // freekick only

	void spinner_select_w(int state);
	uint8_t ff_r();
	void ff_w(uint8_t data);
	void snd_rom_addr_l_w(uint8_t data);
	void snd_rom_addr_h_w(uint8_t data);
	uint8_t snd_rom_r();
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void io_map(address_map &map) ATTR_COLD;
	void program_map(address_map &map) ATTR_COLD;
};

class pbillrd_state : public base_state
{
public:
	pbillrd_state(const machine_config &mconfig, device_type type, const char *tag)
		: base_state(mconfig, type, tag),
		m_rombank(*this, "rombank"),
		m_opbank(*this, "opbank") { }

	void pbillrd(machine_config &config);
	void pbillrdbl(machine_config &config);
	void pbillrdm(machine_config &config);

	void init_pbillrd();
	void init_pbillrdbl();
	void init_pbillrds();

protected:
	optional_memory_bank m_rombank;
	optional_memory_bank m_opbank;

	std::unique_ptr<uint8_t[]> m_decrypted_opcodes;

	void decrypted_opcodes_map(address_map &map) ATTR_COLD;

private:
	void bankswitch_w(uint8_t data);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void program_map(address_map &map) ATTR_COLD;
	void pbillrdbl_program_map(address_map &map) ATTR_COLD;
};

class gigas_state : public pbillrd_state
{
public:
	gigas_state(const machine_config &mconfig, device_type type, const char *tag)
		: pbillrd_state(mconfig, type, tag)
		{ }

	void gigas(machine_config &config);
	void gigasm(machine_config &config);
	void omega(machine_config &config);

	void init_gigas();
	void init_gigasb();

protected:
	void spinner_select_w(uint8_t data);

private:
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void gigas_io_map(address_map &map) ATTR_COLD;
	void gigas_program_map(address_map &map) ATTR_COLD;
	void omega_io_map(address_map &map) ATTR_COLD;
	void omega_program_map(address_map &map) ATTR_COLD;
};

class oigas_state : public gigas_state
{
public:
	oigas_state(const machine_config &mconfig, device_type type, const char *tag)
		: gigas_state(mconfig, type, tag)
		{ }

	void oigas(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	uint16_t m_inval = 0;
	uint16_t m_outval = 0;
	uint8_t m_cnt = 0;

	void _5_w(uint8_t data);
	uint8_t _3_r();
	uint8_t _2_r();

	void io_map(address_map &map) ATTR_COLD;
};


TILE_GET_INFO_MEMBER(base_state::get_tile_info)
{
	int const tileno = m_videoram[tile_index] + ((m_videoram[tile_index + 0x400] & 0xe0) << 3);
	int const palno = m_videoram[tile_index + 0x400] & 0x1f;
	tileinfo.set(0, tileno, palno, 0);
}


void base_state::video_start()
{
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(base_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}


void base_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_tilemap->mark_tile_dirty(offset & 0x3ff);
}

void gigas_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int offs = 0; offs < m_spriteram.bytes(); offs += 4)
	{
		int xpos = m_spriteram[offs + 3];
		int ypos = m_spriteram[offs + 2];
		int const code = m_spriteram[offs + 0] | ((m_spriteram[offs + 1] & 0x20) << 3);

		int flipx = 0;
		int flipy = 0;
		int const color = m_spriteram[offs + 1] & 0x1f;

		if (flip_screen_x())
		{
			xpos = 240 - xpos;
			flipx = !flipx;
		}
		if (flip_screen_y())
		{
			ypos = 256 - ypos;
			flipy = !flipy;
		}

		m_gfxdecode->gfx(1)->transpen(bitmap, cliprect,
				code,
				color,
				flipx, flipy,
				xpos, 240 - ypos, 0);
	}
}


void pbillrd_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int offs = 0; offs < m_spriteram.bytes(); offs += 4)
	{
		int xpos = m_spriteram[offs + 3];
		int ypos = m_spriteram[offs + 2];
		int const code = m_spriteram[offs + 0];

		int flipx = 0;//m_spriteram[offs + 0] & 0x80; //?? unused ?
		int flipy = 0;//m_spriteram[offs + 0] & 0x40;
		int const color = m_spriteram[offs + 1] & 0x0f;

		if (flip_screen_x())
		{
			xpos = 240 - xpos;
			flipx = !flipx;
		}
		if (flip_screen_y())
		{
			ypos = 256 - ypos;
			flipy = !flipy;
		}

		m_gfxdecode->gfx(1)->transpen(bitmap, cliprect,
				code,
				color,
				flipx, flipy,
				xpos, 240 - ypos, 0);
	}
}



void freekick_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int offs = 0; offs < m_spriteram.bytes(); offs += 4)
	{
		int xpos = m_spriteram[offs + 3];
		int ypos = m_spriteram[offs + 0];
		int const code = m_spriteram[offs + 1] + ((m_spriteram[offs + 2] & 0x20) << 3);

		int flipx = m_spriteram[offs + 2] & 0x80;    //?? unused ?
		int flipy = m_spriteram[offs + 2] & 0x40;
		int const color = m_spriteram[offs + 2] & 0x1f;

		if (flip_screen_x())
		{
			xpos = 240 - xpos;
			flipx = !flipx;
		}
		if (flip_screen_y())
		{
			ypos = 256 - ypos;
			flipy = !flipy;
		}

		m_gfxdecode->gfx(1)->transpen(bitmap, cliprect,
				code,
				color,
				flipx, flipy,
				xpos, 248 - ypos, 0);
	}
}

uint32_t gigas_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}

uint32_t pbillrd_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}

uint32_t freekick_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}


/*************************************
 *
 *  Machines' structure
 *
 *************************************/

template <uint8_t Which>
void base_state::coin_w(int state)
{
	machine().bookkeeping().coin_counter_w(Which, state);
}

void freekick_state::spinner_select_w(int state)
{
	m_spinner_sel = state;
}

void gigas_state::spinner_select_w(uint8_t data)
{
	m_spinner_sel = data & 1;
}

uint8_t base_state::spinner_r()
{
	return m_spinner[m_spinner_sel]->read();
}

void pbillrd_state::bankswitch_w(uint8_t data)
{
	m_rombank->set_entry(data & 1);
	if (m_opbank)
		m_opbank->set_entry(data & 1);
}

void base_state::nmi_enable_w(int state)
{
	m_nmi_en = state;
	if (!m_nmi_en)
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

void base_state::vblank_irq(int state)
{
	if (state && m_nmi_en)
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

void oigas_state::_5_w(uint8_t data)
{
	if (data > 0xc0 && data < 0xe0)
		m_cnt = 1;

	switch (m_cnt)
	{
		case 1: m_inval = data << 8  ; break;
		case 2: m_inval |= data      ; break;
	}
}

uint8_t oigas_state::_3_r()
{
	switch (++m_cnt)
	{
		case 2: return ~(m_inval >> 8);
		case 3: return ~(m_inval & 0xff);
		case 4:
			switch (m_inval)
			{
				case 0xc500: m_outval = 0x17ef; break;
				case 0xc520:
				case 0xc540: m_outval = 0x19c1; break;
				case 0xc560: m_outval = 0x1afc; break;
				case 0xc580:
				case 0xc5a0:
				case 0xc5c0: m_outval = 0x1f28; break;
				case 0xc680: m_outval = 0x2e8a; break;
				case 0xc5e0:
				case 0xc600:
				case 0xc620:
				case 0xc640:
				case 0xc660: m_outval = 0x25cc; break;
				case 0xc6c0:
				case 0xc6e0: m_outval = 0x09d7; break;
				case 0xc6a0: m_outval = 0x3168; break;
				case 0xc720: m_outval = 0x2207; break;
				case 0xc700: m_outval = 0x0e34; break;
				case 0xc710: m_outval = 0x0fdd; break;
				case 0xc4f0: m_outval = 0x05b6; break;
				case 0xc4e0: m_outval = 0xae1e; break;
			}
			return m_outval >> 8;
		case 5: m_cnt = 0; return m_outval & 0xff;
	}
	return 0;
}

uint8_t oigas_state::_2_r()
{
	return 1;
}

uint8_t freekick_state::ff_r()
{
	return m_ff_data;
}

void freekick_state::ff_w(uint8_t data)
{
	m_ff_data = data;
}



/*************************************
 *
 *  Memory handlers
 *
 *************************************/

void gigas_state::omega_program_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xcfff).ram(); // RAM is 2x Sony CXK5813D-55
	map(0xd000, 0xd7ff).ram().w(FUNC(gigas_state::videoram_w)).share(m_videoram);
	map(0xd800, 0xd8ff).ram().share(m_spriteram);
	map(0xd900, 0xdfff).ram();
	map(0xe000, 0xe000).portr("IN0");
	map(0xe000, 0xe007).w(m_outlatch, FUNC(ls259_device::write_d0));
	map(0xe800, 0xe800).portr("IN1");
	map(0xf000, 0xf000).portr("DSW1").nopw(); //bankswitch ?
	map(0xf800, 0xf800).portr("DSW2");
	map(0xfc00, 0xfc00).w("sn1", FUNC(sn76489a_device::write));
	map(0xfc01, 0xfc01).w("sn2", FUNC(sn76489a_device::write));
	map(0xfc02, 0xfc02).w("sn3", FUNC(sn76489a_device::write));
	map(0xfc03, 0xfc03).w("sn4", FUNC(sn76489a_device::write));
}

void pbillrd_state::program_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr(m_rombank);
	map(0xc000, 0xcfff).ram();
	map(0xd000, 0xd7ff).ram().w(FUNC(pbillrd_state::videoram_w)).share(m_videoram);
	map(0xd800, 0xd8ff).ram().share(m_spriteram);
	map(0xd900, 0xdfff).ram();
	map(0xe000, 0xe000).portr("IN0");
	map(0xe000, 0xe007).w(m_outlatch, FUNC(ls259_device::write_d0));
	map(0xe800, 0xe800).portr("IN1");
	map(0xf000, 0xf000).portr("DSW1").w(FUNC(pbillrd_state::bankswitch_w));
	map(0xf800, 0xf800).portr("DSW2");
	map(0xfc00, 0xfc00).w("sn1", FUNC(sn76489a_device::write));
	map(0xfc01, 0xfc01).w("sn2", FUNC(sn76489a_device::write));
	map(0xfc02, 0xfc02).w("sn3", FUNC(sn76489a_device::write));
	map(0xfc03, 0xfc03).w("sn4", FUNC(sn76489a_device::write));
}

void pbillrd_state::pbillrdbl_program_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr(m_rombank);
	map(0xc000, 0xcfff).ram();
	map(0xd000, 0xd7ff).ram().w(FUNC(pbillrd_state::videoram_w)).share(m_videoram);
	map(0xd800, 0xd8ff).ram().share(m_spriteram);
	map(0xd900, 0xdfff).ram();
	map(0xe000, 0xe000).portr("IN0");
	map(0xe000, 0xe007).w(m_outlatch, FUNC(ls259_device::write_d0));
	map(0xe800, 0xe800).portr("IN1");
	map(0xf000, 0xf000).portr("DSW1").w(FUNC(pbillrd_state::bankswitch_w));
	map(0xf800, 0xf800).portr("DSW2");
	map(0xfc00, 0xfc00).w("sn1", FUNC(sn76489a_device::write));
	// map(0xfc01, 0xfc01).w("sn2", FUNC(sn76489a_device::write)); // not populated but still writes here
	map(0xfc02, 0xfc02).w("sn3", FUNC(sn76489a_device::write));
	// map(0xfc03, 0xfc03).w("sn4", FUNC(sn76489a_device::write)); // not populated but still writes here
}

void pbillrd_state::decrypted_opcodes_map(address_map &map)
{
	map(0x0000, 0x7fff).bankr("bank0d");
	map(0x8000, 0xbfff).bankr(m_opbank);
}

void freekick_state::program_map(address_map &map)
{
	map(0x0000, 0xcfff).rom();
	map(0xd000, 0xdfff).ram();
	map(0xe000, 0xe7ff).ram().w(FUNC(freekick_state::videoram_w)).share(m_videoram);
	map(0xe800, 0xe8ff).ram().share(m_spriteram);
	map(0xec00, 0xec03).rw("ppi8255_0", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xf000, 0xf003).rw("ppi8255_1", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xf800, 0xf800).portr("IN0");
	map(0xf801, 0xf801).portr("IN1");
	map(0xf802, 0xf802).nopr(); //MUST return bit 0 = 0, otherwise game resets
	map(0xf803, 0xf803).r(FUNC(freekick_state::spinner_r));
	map(0xf800, 0xf807).w(m_outlatch, FUNC(ls259_device::write_d0));
	map(0xfc00, 0xfc00).w("sn1", FUNC(sn76489a_device::write));
	map(0xfc01, 0xfc01).w("sn2", FUNC(sn76489a_device::write));
	map(0xfc02, 0xfc02).w("sn3", FUNC(sn76489a_device::write));
	map(0xfc03, 0xfc03).w("sn4", FUNC(sn76489a_device::write));
}

void gigas_state::gigas_program_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xcfff).ram();
	map(0xd000, 0xd7ff).ram().w(FUNC(gigas_state::videoram_w)).share(m_videoram);
	map(0xd800, 0xd8ff).ram().share(m_spriteram);
	map(0xd900, 0xdfff).ram();
	map(0xe000, 0xe000).portr("IN0");
	map(0xe000, 0xe007).w(m_outlatch, FUNC(ls259_device::write_d0));
	map(0xe800, 0xe800).portr("IN1");
	map(0xf000, 0xf000).portr("DSW1").nopw(); //bankswitch ?
	map(0xf800, 0xf800).portr("DSW2");
	map(0xfc00, 0xfc00).w("sn1", FUNC(sn76489a_device::write));
	map(0xfc01, 0xfc01).w("sn2", FUNC(sn76489a_device::write));
	map(0xfc02, 0xfc02).w("sn3", FUNC(sn76489a_device::write));
	map(0xfc03, 0xfc03).w("sn4", FUNC(sn76489a_device::write));
}

void gigas_state::omega_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).rw(FUNC(gigas_state::spinner_r), FUNC(gigas_state::spinner_select_w));
	map(0x01, 0x01).portr("DSW3");
}

void gigas_state::gigas_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).rw(FUNC(gigas_state::spinner_r), FUNC(gigas_state::spinner_select_w));
	map(0x01, 0x01).nopr(); //unused dip 3
}

void oigas_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).rw(FUNC(oigas_state::spinner_r), FUNC(oigas_state::spinner_select_w));
	map(0x01, 0x01).nopr(); //unused dip 3
	map(0x02, 0x02).r(FUNC(oigas_state::_2_r));
	map(0x03, 0x03).r(FUNC(oigas_state::_3_r));
	map(0x05, 0x05).w(FUNC(oigas_state::_5_w));
}

void freekick_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0xff, 0xff).rw(FUNC(freekick_state::ff_r), FUNC(freekick_state::ff_w));
}


/*************************************
 *
 *  Game-specific port definitions
 *
 *************************************/

static INPUT_PORTS_START( pbillrd )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "Balls" )         PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x06, 0x06, "Bonus Ball" )        PORT_DIPLOCATION("SW1:2,3")
	PORT_DIPSETTING(    0x06, "10000, 30000 & 50000 Points"  )
	PORT_DIPSETTING(    0x02, "20000 & 60000 Points" )
	PORT_DIPSETTING(    0x04, "30000 & 80000 Points" )
	PORT_DIPSETTING(    0x00, "Only 20000 Points" )
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW1:4" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, "Shot" )          PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x20, "3" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW2:1,2,3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x02, "3 Coins/5 Credits" )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW2:5,6,7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x20, "3 Coins/5 Credits" )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_5C ) )
INPUT_PORTS_END

static INPUT_PORTS_START( gigas )
	PORT_INCLUDE( pbillrd )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x3c, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x3c, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(30) PORT_KEYDELTA(15) PORT_REVERSE

	PORT_START("IN3")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(30) PORT_KEYDELTA(15) PORT_REVERSE PORT_COCKTAIL

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW1:2,3")
	PORT_DIPSETTING(    0x06, "20000 & 60000, Every 60000 Points" )
	PORT_DIPSETTING(    0x02, "20000 & 60000 Points" )
	PORT_DIPSETTING(    0x04, "30000 & 80000, Every 80000 Points" )
	PORT_DIPSETTING(    0x00, "Only 20000 Points" )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(    0x18, DEF_STR( Easy ) )    // level 1
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) ) // level 4
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Allow_Continue ) )       PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
INPUT_PORTS_END

static INPUT_PORTS_START( gigasm2 )
	PORT_INCLUDE( gigas )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW1:2,3")
	PORT_DIPSETTING(    0x06, "20000 & 60000, Every 60000 Points" )
	PORT_DIPSETTING(    0x02, "20000 & 60000 Points" )
	PORT_DIPSETTING(    0x04, "30000 & 90000, Every 90000 Points" )
	PORT_DIPSETTING(    0x00, "Only 20000 Points" )
INPUT_PORTS_END

static INPUT_PORTS_START( omega )
	PORT_INCLUDE( gigas ) // this covers in0 in1 in2 in3 which match gigas exactly

	PORT_MODIFY("DSW1") // dsw1 bits 1 and 2 do not match; bits 3:4 may or may not match, needs testing; all else matches
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW1:2,3")
	PORT_DIPSETTING(    0x06, "20000 & 60000, Every 60000 Points" )
	PORT_DIPSETTING(    0x02, "30000 & 80000, Every 80000 Points" )
	PORT_DIPSETTING(    0x04, "20000 & 60000 Points" )
	PORT_DIPSETTING(    0x00, "Only 20000 Points" )
	// TODO: verify that bits 3 and 4 match gigas and gigasmk2 for difficulty!

	PORT_MODIFY("DSW2") // dsw2 is completely unique for omega, although the 0xf0 COIN B bits match freekick port B
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW2:1,2,3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x02, "3 Coins/5 Credits" )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0c, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x04, "1 Coin/25 Credits" )
	PORT_DIPSETTING(    0x08, "1 Coin/50 Credits" )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW2:5,6,7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x20, "3 Coins/5 Credits" )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xc0, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x40, "1 Coin/25 Credits" )
	PORT_DIPSETTING(    0x80, "1 Coin/50 Credits" )

	PORT_START("DSW3") // omega has a third dipswitch array, similar to the later freekick hw below
	PORT_DIPNAME( 0x01, 0x01, "Hopper Status?" )        PORT_DIPLOCATION("SW3:1") // Prints "NORMAL" & "EMPTY" to title screen when set to ON ... medal/hopper status?
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Invulnerability" )     PORT_DIPLOCATION("SW3:2") // Ball always bounces up, player never dies
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW3:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW3:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW3:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW3:6")
	PORT_DIPNAME( 0xc0, 0xc0, "Prize Version")     PORT_DIPLOCATION("SW3:7,8") // Multiple settings for payout level?
	PORT_DIPSETTING(    0xc0, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, "On Setting 1" )
	PORT_DIPSETTING(    0x40, "On Setting 2" )
	PORT_DIPSETTING(    0x00, "On Setting 3" )
INPUT_PORTS_END

static INPUT_PORTS_START( freekick )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:2,3")
	PORT_DIPSETTING(    0x06, "2-3-4-5-60000 Points" )
	PORT_DIPSETTING(    0x02, "3-4-5-6-7-80000 Points" )
	PORT_DIPSETTING(    0x04, "20000 & 60000 Points" )
	PORT_DIPSETTING(    0x00, "ONLY 20000 Points" )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(    0x18, DEF_STR( Easy ) )    // level 1
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) ) // level 4
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW2:1,2,3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x02, "3 Coins/5 Credits" )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW2:5,6,7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x20, "3 Coins/5 Credits" )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xc0, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x40, "1 Coin/25 Credits" )
	PORT_DIPSETTING(    0x80, "1 Coin/50 Credits" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x00, "Manufacturer" )  PORT_DIPLOCATION("SW3:1")   // Set to "Sega" to show Japanese text on the "Continue" screen
	PORT_DIPSETTING(    0x00, "Nihon System" )
	PORT_DIPSETTING(    0x01, "Sega/Nihon System" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW3:2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW3:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW3:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW3:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW3:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW3:7" )
	PORT_DIPNAME( 0x80, 0x80, "Coin Slots" )    PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x80, "2" )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("IN2")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(30) PORT_KEYDELTA(15) PORT_REVERSE

	PORT_START("IN3")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(30) PORT_KEYDELTA(15) PORT_REVERSE PORT_COCKTAIL
INPUT_PORTS_END

static INPUT_PORTS_START( countrun )
	PORT_INCLUDE( freekick )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:2,3")
	PORT_DIPSETTING(    0x06, "20000, 60000 and every 60000 Points" )
	PORT_DIPSETTING(    0x02, "30000, 80000 and every 80000 Points" )
	PORT_DIPSETTING(    0x04, "20000 & 60000 Points" )
	PORT_DIPSETTING(    0x00, "ONLY 20000 Points" )

	PORT_MODIFY("DSW3")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW3:1" )
INPUT_PORTS_END



/*************************************
 *
 *  Sound definitions
 *
 *************************************/

void freekick_state::snd_rom_addr_l_w(uint8_t data)
{
	m_romaddr = (m_romaddr & 0xff00) | data;
}

void freekick_state::snd_rom_addr_h_w(uint8_t data)
{
	m_romaddr = (m_romaddr & 0x00ff) | (data << 8);
}

uint8_t freekick_state::snd_rom_r()
{
	return m_sound_data[m_romaddr & 0x7fff];
}

/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3),RGN_FRAC(2,3),RGN_FRAC(1,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
	128+0,128+1,128+2,128+3,128+4,128+5,128+6,128+7
	},
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		8*8, 9*8, 10*8, 11*8,12*8,13*8,14*8,15*8
	},
	16*16
};

static GFXDECODE_START( gfx_freekick )
	GFXDECODE_ENTRY( "tiles",   0, gfx_8x8x3_planar,   0x000, 32 )
	GFXDECODE_ENTRY( "sprites", 0, spritelayout,       0x100, 32 )
GFXDECODE_END

/*************************************
 *
 *  Machine driver(s)
 *
 *************************************/

void base_state::machine_start()
{
	save_item(NAME(m_spinner_sel));
	save_item(NAME(m_nmi_en));
}

void freekick_state::machine_start()
{
	base_state::machine_start();

	save_item(NAME(m_romaddr));
	save_item(NAME(m_ff_data));
}

void base_state::machine_reset()
{
	m_spinner_sel = 0;

	machine().bookkeeping().coin_counter_w(0, 0);
	machine().bookkeeping().coin_counter_w(1, 0);
}

void freekick_state::machine_reset()
{
	base_state::machine_reset();

	m_romaddr = 0;
	m_ff_data = 0;
}

void oigas_state::machine_start()
{
	save_item(NAME(m_inval));
	save_item(NAME(m_outval));
	save_item(NAME(m_cnt));

	base_state::machine_start();
}

void oigas_state::machine_reset()
{
	base_state::machine_reset();

	m_inval = 0;
	m_outval = 0;
	m_cnt = 0;
}

void gigas_state::omega(machine_config &config)
{
	MC8123(config, m_maincpu, XTAL(18'432'000) / 6); // unknown divisor
	m_maincpu->set_addrmap(AS_PROGRAM, &gigas_state::omega_program_map);
	m_maincpu->set_addrmap(AS_IO, &gigas_state::omega_io_map);
	m_maincpu->set_addrmap(AS_OPCODES, &gigas_state::decrypted_opcodes_map);
	m_maincpu->set_periodic_int(FUNC(gigas_state::irq0_line_hold), attotime::from_hz(120)); // measured on PCB

	LS259(config, m_outlatch); // 3M
	m_outlatch->q_out_cb<0>().set(FUNC(gigas_state::flip_screen_set)).invert();
	m_outlatch->q_out_cb<2>().set(FUNC(gigas_state::coin_w<0>));
	m_outlatch->q_out_cb<3>().set(FUNC(gigas_state::coin_w<1>));
	m_outlatch->q_out_cb<4>().set(FUNC(gigas_state::nmi_enable_w));
	m_outlatch->q_out_cb<5>().set_nop(); // ???

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(18'432'000) / 3, 768 / 2, 0, 512 / 2, 263, 0 + 16, 224 + 16); // unknown divisor
	screen.set_screen_update(FUNC(gigas_state::screen_update));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(FUNC(gigas_state::vblank_irq));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_freekick);
	PALETTE(config, m_palette, palette_device::RGB_444_PROMS, "proms", 0x200);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	SN76489A(config, "sn1", XTAL(18'432'000) / 6).add_route(ALL_OUTPUTS, "mono", 0.50); // unknown divisor

	SN76489A(config, "sn2", XTAL(18'432'000) / 6).add_route(ALL_OUTPUTS, "mono", 0.50); // unknown divisor

	SN76489A(config, "sn3", XTAL(18'432'000) / 6).add_route(ALL_OUTPUTS, "mono", 0.50); // unknown divisor

	SN76489A(config, "sn4", XTAL(18'432'000) / 6).add_route(ALL_OUTPUTS, "mono", 0.50); // unknown divisor
}

void base_state::base(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, XTAL(12'000'000) / 4);
	m_maincpu->set_periodic_int(FUNC(base_state::irq0_line_hold), attotime::from_hz(120)); // measured on PCB

	LS259(config, m_outlatch);
	m_outlatch->q_out_cb<2>().set(FUNC(base_state::coin_w<0>));
	m_outlatch->q_out_cb<3>().set(FUNC(base_state::coin_w<1>));
	m_outlatch->q_out_cb<4>().set(FUNC(base_state::nmi_enable_w));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(12'000'000) / 2, 768 / 2, 0, 512 / 2, 263, 0 + 16, 224 + 16);
	screen.set_palette(m_palette);
	screen.screen_vblank().set(FUNC(base_state::vblank_irq));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_freekick);
	PALETTE(config, m_palette, palette_device::RGB_444_PROMS, "proms", 0x200);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	SN76489A(config, "sn1", XTAL(12'000'000) / 4).add_route(ALL_OUTPUTS, "mono", 0.50);

	SN76489A(config, "sn2", XTAL(12'000'000) / 4).add_route(ALL_OUTPUTS, "mono", 0.50);

	SN76489A(config, "sn3", XTAL(12'000'000) / 4).add_route(ALL_OUTPUTS, "mono", 0.50);

	SN76489A(config, "sn4", XTAL(12'000'000) / 4).add_route(ALL_OUTPUTS, "mono", 0.50);
}

void pbillrd_state::pbillrd(machine_config &config)
{
	base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &pbillrd_state::program_map);

	// 10K
	m_outlatch->q_out_cb<0>().set(FUNC(pbillrd_state::flip_screen_x_set)).invert();
	m_outlatch->q_out_cb<1>().set(FUNC(pbillrd_state::flip_screen_y_set)).invert();
	// flip Y/X could be the other way round...

	// video hardware
	subdevice<screen_device>("screen")->set_screen_update(FUNC(pbillrd_state::screen_update));
}

void pbillrd_state::pbillrdbl(machine_config &config)
{
	pbillrd(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &pbillrd_state::pbillrdbl_program_map);
	m_maincpu->set_addrmap(AS_OPCODES, &pbillrd_state::decrypted_opcodes_map);
	m_maincpu->set_clock(XTAL(18'432'000) / 6); // unknown divisor

	// the PCB is marked to host 4 SN76489 but only two are populated (by SN76496N)
	// however the code still writes to all 4, so it's not clear which two are missing

	SN76496(config.replace(), "sn1", XTAL(18'432'000) / 6).add_route(ALL_OUTPUTS, "mono", 0.50);

	SN76496(config.replace(), "sn3", XTAL(18'432'000) / 6).add_route(ALL_OUTPUTS, "mono", 0.50);

	config.device_remove("sn2");

	config.device_remove("sn4");
}

void pbillrd_state::pbillrdm(machine_config &config)
{
	pbillrd(config);
	MC8123(config.replace(), m_maincpu, XTAL(12'000'000) / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &pbillrd_state::program_map);
	m_maincpu->set_addrmap(AS_OPCODES, &pbillrd_state::decrypted_opcodes_map);
	m_maincpu->set_periodic_int(FUNC(pbillrd_state::irq0_line_hold), attotime::from_hz(120)); // measured on PCB
}

void freekick_state::freekick(machine_config &config)
{
	base(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &freekick_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &freekick_state::io_map);

	// 5C
	m_outlatch->q_out_cb<0>().set(FUNC(freekick_state::flip_screen_set)).invert();
	m_outlatch->q_out_cb<6>().set(FUNC(freekick_state::spinner_select_w));

	i8255_device &ppi0(I8255A(config, "ppi8255_0"));
	ppi0.out_pa_callback().set(FUNC(freekick_state::snd_rom_addr_l_w));
	ppi0.out_pb_callback().set(FUNC(freekick_state::snd_rom_addr_h_w));
	ppi0.in_pc_callback().set(FUNC(freekick_state::snd_rom_r));

	i8255_device &ppi1(I8255A(config, "ppi8255_1"));
	ppi1.in_pa_callback().set_ioport("DSW1");
	ppi1.in_pb_callback().set_ioport("DSW2");
	ppi1.in_pc_callback().set_ioport("DSW3");

	// video hardware
	subdevice<screen_device>("screen")->set_screen_update(FUNC(freekick_state::screen_update));
}

void gigas_state::gigas(machine_config &config)
{
	base(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &gigas_state::gigas_program_map);
	m_maincpu->set_addrmap(AS_IO, &gigas_state::gigas_io_map);
	m_maincpu->set_addrmap(AS_OPCODES, &gigas_state::decrypted_opcodes_map);

	m_outlatch->q_out_cb<0>().set(FUNC(gigas_state::flip_screen_set)).invert();
	m_outlatch->q_out_cb<5>().set_nop(); // ???

	// video hardware
	subdevice<screen_device>("screen")->set_screen_update(FUNC(gigas_state::screen_update));
}

void gigas_state::gigasm(machine_config &config)
{
	base(config);

	// basic machine hardware
	MC8123(config.replace(), m_maincpu, XTAL(12'000'000) / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &gigas_state::gigas_program_map);
	m_maincpu->set_addrmap(AS_IO, &gigas_state::gigas_io_map);
	m_maincpu->set_addrmap(AS_OPCODES, &gigas_state::decrypted_opcodes_map);
	m_maincpu->set_periodic_int(FUNC(gigas_state::irq0_line_hold), attotime::from_hz(120)); // measured on PCB

	m_outlatch->q_out_cb<0>().set(FUNC(gigas_state::flip_screen_set)).invert();
	m_outlatch->q_out_cb<5>().set_nop(); // ???

	/* video hardware */
	subdevice<screen_device>("screen")->set_screen_update(FUNC(gigas_state::screen_update));
}

void oigas_state::oigas(machine_config &config)
{
	gigas(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_IO, &oigas_state::io_map);
}



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( pbillrd )
	ROM_REGION( 0x10000, "maincpu", 0 ) // Z80 code
	ROM_LOAD( "pb.18", 0x0000, 0x4000, CRC(9e6275ac) SHA1(482e845e7fb4190da483155bd908ad470373cd5c) )
	ROM_LOAD( "pb.7",  0x4000, 0x8000, CRC(dd438431) SHA1(07a950e38b3f627ecf95e5831e5480abb337a010) )
	ROM_LOAD( "pb.9",  0xc000, 0x4000, CRC(089ce80a) SHA1(779be9ba2277a26fbebf4acf9e2f5319a934b0f5) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "pb.4", 0x000000, 0x04000, CRC(2f4d4dd3) SHA1(ee4facabf591c235c270db4f4d3f612b8c474e57) )
	ROM_LOAD( "pb.5", 0x004000, 0x04000, CRC(9dfccbd3) SHA1(66ad8882f36630312b488d5d67ae554477574c31) )
	ROM_LOAD( "pb.6", 0x008000, 0x04000, CRC(b5c3f6f6) SHA1(586b47587619a766cf977b74978550aff41a58cc) )

	ROM_REGION( 0x6000, "sprites", 0 )
	ROM_LOAD( "10619.3r", 0x000000, 0x02000, CRC(3296b9d9) SHA1(51393306f74394de96c4097b6244e8eb36114dac) )
	ROM_LOAD( "10621.3m", 0x002000, 0x02000, CRC(3dca8e4b) SHA1(ca0416d8faba0bb5e6b8c0a8fc227b57caa75f71) )
	ROM_LOAD( "10620.3n", 0x004000, 0x02000, CRC(ee76b079) SHA1(99abe2c5b1889d20bc3f5720b168690e3979fb2f) )

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "82s129.3a", 0x0000, 0x0100, CRC(44802169) SHA1(f181d80185e0f87ee906d2b40e3a5deb6f563aa2) )
	ROM_LOAD( "82s129.4d", 0x0100, 0x0100, CRC(69ca07cc) SHA1(38ab08174633b53d70a38aacb40059a25cf12069) )
	ROM_LOAD( "82s129.4a", 0x0200, 0x0100, CRC(145f950a) SHA1(b007d0c1cc9545e0e241b39b79a48593d457f826) )
	ROM_LOAD( "82s129.3d", 0x0300, 0x0100, CRC(43d24e17) SHA1(de5c9391574781dcd8f244794010e8eddffa1c1e) )
	ROM_LOAD( "82s129.3b", 0x0400, 0x0100, CRC(7fdc872c) SHA1(98572560aa524490489d4202dba292a5af9f15e7) )
	ROM_LOAD( "82s129.3c", 0x0500, 0x0100, CRC(cc1657e5) SHA1(358f20dce376c2389009f9673ce38b297af863f6) )
ROM_END

ROM_START( pbillrds ) // Encrypted with a Sega MC-8123 (317-0030) CPU module, Sega ID# 836-6304 PERFECT BILLIARD
	ROM_REGION( 0x10000, "maincpu", 0 ) // Z80 code
	ROM_LOAD( "epr-10626.8n",  0x0000, 0x4000, CRC(51d725e6) SHA1(d7007c983530780e7fa3686cb7a6d7c382c802fa) ) // encrypted
	ROM_LOAD( "epr-10625.8r",  0x4000, 0x8000, CRC(8977c724) SHA1(f00835a04dc6fa7d8c1e382dace515f2aa7d6f44) ) // encrypted
	ROM_LOAD( "epr-10627.10n", 0xc000, 0x4000, CRC(2335e6dd) SHA1(82352b6f4abea88aad3a96ca63cccccb6e278f48) ) // encrypted

	ROM_REGION( 0x2000, "maincpu:key", 0 ) // MC8123 key
	ROM_LOAD( "317-0030.key", 0x0000, 0x2000, CRC(9223f06d) SHA1(51a22a4c80fe273526bde68918c13c6476cec383) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "epr-10622.3h", 0x000000, 0x04000, CRC(23b864ac) SHA1(5a13ad6f2278761967269eed8c07077293c921d6) ) // these 3 ROMs had a green strip down the middle of the label
	ROM_LOAD( "epr-10623.3h", 0x004000, 0x04000, CRC(3dbfb790) SHA1(81a2645b7b3addf8f5b83043c967647cea476002) ) // these 3 ROMs had a green strip down the middle of the label
	ROM_LOAD( "epr-10624.3g", 0x008000, 0x04000, CRC(b80032a9) SHA1(20096bdae1aad8913d8d7b1045912ea5ae7fce6f) ) // these 3 ROMs had a green strip down the middle of the label

	ROM_REGION( 0x6000, "sprites", 0 )
	ROM_LOAD( "epr-10619.3r", 0x000000, 0x02000, CRC(3296b9d9) SHA1(51393306f74394de96c4097b6244e8eb36114dac) )
	ROM_LOAD( "epr-10621.3m", 0x002000, 0x02000, CRC(3dca8e4b) SHA1(ca0416d8faba0bb5e6b8c0a8fc227b57caa75f71) )
	ROM_LOAD( "epr-10620.3n", 0x004000, 0x02000, CRC(ee76b079) SHA1(99abe2c5b1889d20bc3f5720b168690e3979fb2f) )

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "pr10628.3a", 0x0000, 0x0100, CRC(44802169) SHA1(f181d80185e0f87ee906d2b40e3a5deb6f563aa2) ) // 82s129 or compatible BPROM
	ROM_LOAD( "pr10633.4d", 0x0100, 0x0100, CRC(69ca07cc) SHA1(38ab08174633b53d70a38aacb40059a25cf12069) ) // 82s129 or compatible BPROM
	ROM_LOAD( "pr10632.4a", 0x0200, 0x0100, CRC(145f950a) SHA1(b007d0c1cc9545e0e241b39b79a48593d457f826) ) // 82s129 or compatible BPROM
	ROM_LOAD( "pr10631.3d", 0x0300, 0x0100, CRC(43d24e17) SHA1(de5c9391574781dcd8f244794010e8eddffa1c1e) ) // 82s129 or compatible BPROM
	ROM_LOAD( "pr10629.3b", 0x0400, 0x0100, CRC(7fdc872c) SHA1(98572560aa524490489d4202dba292a5af9f15e7) ) // 82s129 or compatible BPROM
	ROM_LOAD( "pr10630.3c", 0x0500, 0x0100, CRC(cc1657e5) SHA1(358f20dce376c2389009f9673ce38b297af863f6) ) // 82s129 or compatible BPROM
ROM_END

ROM_START( pbillrdsa ) // all ROMs were HN4827128G-25, except 17, HN27256G-25, CPU module marked 317-5008, but seems to act the same as above
	ROM_REGION( 0x10000, "maincpu", 0 ) // Z80 code
	ROM_LOAD( "20", 0x0000, 0x4000, CRC(da020258) SHA1(172276061c2e06bcf3477488734a72598412181b) ) // encrypted
	ROM_LOAD( "17", 0x4000, 0x8000, CRC(9bb3d467) SHA1(5d61c80c920363cbcb548f4a08434e2a05b3d5f3) ) // encrypted
	ROM_LOAD( "19", 0xc000, 0x4000, CRC(2335e6dd) SHA1(82352b6f4abea88aad3a96ca63cccccb6e278f48) ) // encrypted

	ROM_REGION( 0x2000, "maincpu:key", 0 ) // MC8123 key
	ROM_LOAD( "317-5008.key", 0x0000, 0x2000, CRC(9223f06d) SHA1(51a22a4c80fe273526bde68918c13c6476cec383) ) // same key as 317-0030

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "pb.4", 0x000000, 0x04000, CRC(2f4d4dd3) SHA1(ee4facabf591c235c270db4f4d3f612b8c474e57) )
	ROM_LOAD( "pb.5", 0x004000, 0x04000, CRC(9dfccbd3) SHA1(66ad8882f36630312b488d5d67ae554477574c31) )
	ROM_LOAD( "pb.6", 0x008000, 0x04000, CRC(b5c3f6f6) SHA1(586b47587619a766cf977b74978550aff41a58cc) )

	ROM_REGION( 0xc000, "sprites", 0 )
	ROM_LOAD( "1", 0x000000, 0x04000, CRC(c8ed651e) SHA1(9ddeb7906e0772f344af1d4f74755694cade1f97) )
	ROM_LOAD( "3", 0x004000, 0x04000, CRC(5282fc86) SHA1(dd8938489071ce61dc9bd4fed5a28403131e5706) )
	ROM_LOAD( "2", 0x008000, 0x04000, CRC(e9f73f5b) SHA1(470ad3b0d13269b098eec9d81956d70ac3aebc39) )

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "82s129.3a", 0x0000, 0x0100, CRC(44802169) SHA1(f181d80185e0f87ee906d2b40e3a5deb6f563aa2) )
	ROM_LOAD( "82s129.4d", 0x0100, 0x0100, CRC(69ca07cc) SHA1(38ab08174633b53d70a38aacb40059a25cf12069) )
	ROM_LOAD( "82s129.4a", 0x0200, 0x0100, CRC(145f950a) SHA1(b007d0c1cc9545e0e241b39b79a48593d457f826) )
	ROM_LOAD( "82s129.3d", 0x0300, 0x0100, CRC(43d24e17) SHA1(de5c9391574781dcd8f244794010e8eddffa1c1e) )
	ROM_LOAD( "82s129.3b", 0x0400, 0x0100, CRC(7fdc872c) SHA1(98572560aa524490489d4202dba292a5af9f15e7) )
	ROM_LOAD( "82s129.3c", 0x0500, 0x0100, CRC(cc1657e5) SHA1(358f20dce376c2389009f9673ce38b297af863f6) )
ROM_END

ROM_START( pbillrdbl ) // 87-16-12 on PCB
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "7-27128.8n",  0x10000, 0x4000, CRC(f9ef8bcd) SHA1(30f9c54ca8aee7d7d23c1c5e040d9d917f92f503) ) // marked on PCB as 27128, actually 27256
	ROM_CONTINUE(            0x00000, 0x4000)
	ROM_LOAD( "8-27256.8r",  0x14000, 0x8000, CRC(63e85ec6) SHA1(1ccc8de52e2618b9c1438941ddd44b78f0b66a6c) ) // marked on PCB as 27256, actually 27512
	ROM_CONTINUE(            0x04000, 0x8000)
	ROM_LOAD( "9-27256.10n", 0x1c000, 0x4000, CRC(7ae4866f) SHA1(771aa1f1cf1c1efb0d27e8b7d6a230101abf3303) )
	ROM_CONTINUE(            0x0c000, 0x4000)

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "4-2764-128.3j", 0x000000, 0x04000, CRC(23b864ac) SHA1(5a13ad6f2278761967269eed8c07077293c921d6) )
	ROM_LOAD( "5-2764-128.3h", 0x004000, 0x04000, CRC(3dbfb790) SHA1(81a2645b7b3addf8f5b83043c967647cea476002) )
	ROM_LOAD( "6-2764-128.3f", 0x008000, 0x04000, CRC(b80032a9) SHA1(20096bdae1aad8913d8d7b1045912ea5ae7fce6f) )

	ROM_REGION( 0x6000, "sprites", 0 )
	ROM_LOAD( "1-27125-64.3r", 0x000000, 0x02000, CRC(3296b9d9) SHA1(51393306f74394de96c4097b6244e8eb36114dac) )
	ROM_LOAD( "3-27128-64.3m", 0x002000, 0x02000, CRC(3dca8e4b) SHA1(ca0416d8faba0bb5e6b8c0a8fc227b57caa75f71) )
	ROM_LOAD( "2-27128-64.3n", 0x004000, 0x02000, CRC(ee76b079) SHA1(99abe2c5b1889d20bc3f5720b168690e3979fb2f) )

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "dm74s287n.3a", 0x0000, 0x0100, CRC(44802169) SHA1(f181d80185e0f87ee906d2b40e3a5deb6f563aa2) )
	ROM_LOAD( "dm74s287n.4d", 0x0100, 0x0100, CRC(69ca07cc) SHA1(38ab08174633b53d70a38aacb40059a25cf12069) )
	ROM_LOAD( "dm74s287n.4a", 0x0200, 0x0100, CRC(145f950a) SHA1(b007d0c1cc9545e0e241b39b79a48593d457f826) )
	ROM_LOAD( "dm74s287n.3d", 0x0300, 0x0100, CRC(43d24e17) SHA1(de5c9391574781dcd8f244794010e8eddffa1c1e) )
	ROM_LOAD( "dm74s287n.3b", 0x0400, 0x0100, CRC(7fdc872c) SHA1(98572560aa524490489d4202dba292a5af9f15e7) )
	ROM_LOAD( "dm74s287n.3c", 0x0500, 0x0100, CRC(cc1657e5) SHA1(358f20dce376c2389009f9673ce38b297af863f6) )
ROM_END

/*

The original Freekick boards have the main CPU code inside a custom CPU "block". This code is stored in battery
backed RAM. There is 64K of RAM, but only 52K is program code while the remaining RAM is actually used as RAM.

*/

ROM_START( freekick )
	ROM_REGION( 0x0d000, "maincpu", 0 ) // Z80 Code, internal program RAM is 52K in custom cpu module
	ROM_LOAD( "ns6201-a_1987.10_free_kick.cpu", 0x00000, 0x0d000, CRC(6d172850) SHA1(ac461bff9da263681085920ad6acd778241dedd3) )

	ROM_REGION( 0x08000, "sound_data", 0 )
	ROM_LOAD( "11.1e", 0x00000, 0x08000, CRC(a6030ba9) SHA1(f363100f54a7a80701a6395c7539b8daa60db054) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "12.1h", 0x000000, 0x04000, CRC(fb82e486) SHA1(bc672272dc32b2aa64e991992172c44bea1ca65c) )
	ROM_LOAD( "13.1j", 0x004000, 0x04000, CRC(3ad78ee2) SHA1(033285d4ab7d6f46abf4c1bd4671c874738f0ac1) )
	ROM_LOAD( "14.1l", 0x008000, 0x04000, CRC(0185695f) SHA1(126994c69de157fc7c452ccc7f1a767f5085da27) )

	ROM_REGION( 0xc000, "sprites", 0 )
	ROM_LOAD( "15.1m", 0x000000, 0x04000, CRC(0fa7c13c) SHA1(24b0ca73b0e35474e2392d8e729bcd44b80f9135) )
	ROM_LOAD( "16.1p", 0x004000, 0x04000, CRC(2b996e89) SHA1(c6900449d27e89c3b444fb028694fdcda8e79322) )
	ROM_LOAD( "17.1r", 0x008000, 0x04000, CRC(e7894def) SHA1(5c97b7cce43d1e51c709603a0d2394b8119764bd) )

	ROM_REGION( 0x0600, "proms", 0 ) // verified correct
	ROM_LOAD( "24s10n.8j", 0x0000, 0x0100, CRC(53a6bc21) SHA1(d4beedc226004c1aa9b6aae29bee9c8a9b0fff7c) ) // Or compatible type PROM like the 82S129
	ROM_LOAD( "24s10n.7j", 0x0100, 0x0100, CRC(38dd97d8) SHA1(468a0f87a704982dc1bce1ca21f9bb252ac241a0) )
	ROM_LOAD( "24s10n.8k", 0x0200, 0x0100, CRC(18e66087) SHA1(54857526179b738862d11ce87e9d0edcb7878488) )
	ROM_LOAD( "24s10n.7k", 0x0300, 0x0100, CRC(bc21797a) SHA1(4d6cf05e51b7ef9147eeff051c3728764021cfdb) )
	ROM_LOAD( "24s10n.8h", 0x0400, 0x0100, CRC(8aac5fd0) SHA1(07a179603c0167c1f998b2337d66be95db9911cc) )
	ROM_LOAD( "24s10n.7h", 0x0500, 0x0100, CRC(a507f941) SHA1(97619959ee4c366cb010525636ab5eefe5a3127a) )
ROM_END

ROM_START( freekicka ) // The bootlegs are derived from this set, "freekbl8.q7" is identical but includes more address space then needed
	ROM_REGION( 0x0d000, "maincpu", 0 ) // Z80 Code, internal program RAM is 52K in custom cpu module
	ROM_LOAD( "ns6201-a_1987.9_free_kick.cpu", 0x00000, 0x0d000, CRC(acc0a278) SHA1(27675870ece29ccd5135ca20fb2fa91125945ec5) )

	ROM_REGION( 0x08000, "sound_data", 0 )
	ROM_LOAD( "11.1e", 0x00000, 0x08000, CRC(a6030ba9) SHA1(f363100f54a7a80701a6395c7539b8daa60db054) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "12.1h", 0x000000, 0x04000, CRC(fb82e486) SHA1(bc672272dc32b2aa64e991992172c44bea1ca65c) )
	ROM_LOAD( "13.1j", 0x004000, 0x04000, CRC(3ad78ee2) SHA1(033285d4ab7d6f46abf4c1bd4671c874738f0ac1) )
	ROM_LOAD( "14.1l", 0x008000, 0x04000, CRC(0185695f) SHA1(126994c69de157fc7c452ccc7f1a767f5085da27) )

	ROM_REGION( 0xc000, "sprites", 0 )
	ROM_LOAD( "15.1m", 0x000000, 0x04000, CRC(0fa7c13c) SHA1(24b0ca73b0e35474e2392d8e729bcd44b80f9135) )
	ROM_LOAD( "16.1p", 0x004000, 0x04000, CRC(2b996e89) SHA1(c6900449d27e89c3b444fb028694fdcda8e79322) )
	ROM_LOAD( "17.1r", 0x008000, 0x04000, CRC(e7894def) SHA1(5c97b7cce43d1e51c709603a0d2394b8119764bd) )

	ROM_REGION( 0x0600, "proms", 0 ) // verified correct
	ROM_LOAD( "24s10n.8j", 0x0000, 0x0100, CRC(53a6bc21) SHA1(d4beedc226004c1aa9b6aae29bee9c8a9b0fff7c) ) // Or compatible type PROM like the 82S129
	ROM_LOAD( "24s10n.7j", 0x0100, 0x0100, CRC(38dd97d8) SHA1(468a0f87a704982dc1bce1ca21f9bb252ac241a0) )
	ROM_LOAD( "24s10n.8k", 0x0200, 0x0100, CRC(18e66087) SHA1(54857526179b738862d11ce87e9d0edcb7878488) )
	ROM_LOAD( "24s10n.7k", 0x0300, 0x0100, CRC(bc21797a) SHA1(4d6cf05e51b7ef9147eeff051c3728764021cfdb) )
	ROM_LOAD( "24s10n.8h", 0x0400, 0x0100, CRC(8aac5fd0) SHA1(07a179603c0167c1f998b2337d66be95db9911cc) )
	ROM_LOAD( "24s10n.7h", 0x0500, 0x0100, CRC(a507f941) SHA1(97619959ee4c366cb010525636ab5eefe5a3127a) )
ROM_END

ROM_START( freekickb1 )
	ROM_REGION( 0x10000, "maincpu", 0 ) // Z80 code
	ROM_LOAD( "freekbl8.q7", 0x00000, 0x10000, CRC(4208cfe5) SHA1(21628cbe8a217fbae30a6c24c9cc4c790fe45d65) )

	ROM_REGION( 0x08000, "sound_data", 0 )
	ROM_LOAD( "11.1e", 0x00000, 0x08000, CRC(a6030ba9) SHA1(f363100f54a7a80701a6395c7539b8daa60db054) ) // freekbl1.e2

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "12.1h", 0x000000, 0x04000, CRC(fb82e486) SHA1(bc672272dc32b2aa64e991992172c44bea1ca65c) ) // freekbl2.f2
	ROM_LOAD( "13.1j", 0x004000, 0x04000, CRC(3ad78ee2) SHA1(033285d4ab7d6f46abf4c1bd4671c874738f0ac1) ) // freekbl3.j2
	ROM_LOAD( "14.1l", 0x008000, 0x04000, CRC(0185695f) SHA1(126994c69de157fc7c452ccc7f1a767f5085da27) ) // freekbl4.k2

	ROM_REGION( 0xc000, "sprites", 0 )
	ROM_LOAD( "15.1m", 0x000000, 0x04000, CRC(0fa7c13c) SHA1(24b0ca73b0e35474e2392d8e729bcd44b80f9135) ) // freekbl5.m2
	ROM_LOAD( "16.1p", 0x004000, 0x04000, CRC(2b996e89) SHA1(c6900449d27e89c3b444fb028694fdcda8e79322) ) // freekbl6.n2
	ROM_LOAD( "17.1r", 0x008000, 0x04000, CRC(e7894def) SHA1(5c97b7cce43d1e51c709603a0d2394b8119764bd) ) // freekbl7.r2

	ROM_REGION( 0x0600, "proms", 0 ) // verified correct
	ROM_LOAD( "24s10n.8j", 0x0000, 0x0100, CRC(53a6bc21) SHA1(d4beedc226004c1aa9b6aae29bee9c8a9b0fff7c) ) // Or compatible type PROM like the 82S129
	ROM_LOAD( "24s10n.7j", 0x0100, 0x0100, CRC(38dd97d8) SHA1(468a0f87a704982dc1bce1ca21f9bb252ac241a0) )
	ROM_LOAD( "24s10n.8k", 0x0200, 0x0100, CRC(18e66087) SHA1(54857526179b738862d11ce87e9d0edcb7878488) )
	ROM_LOAD( "24s10n.7k", 0x0300, 0x0100, CRC(bc21797a) SHA1(4d6cf05e51b7ef9147eeff051c3728764021cfdb) )
	ROM_LOAD( "24s10n.8h", 0x0400, 0x0100, CRC(8aac5fd0) SHA1(07a179603c0167c1f998b2337d66be95db9911cc) )
	ROM_LOAD( "24s10n.7h", 0x0500, 0x0100, CRC(a507f941) SHA1(97619959ee4c366cb010525636ab5eefe5a3127a) )

	ROM_REGION( 0x0003, "pals", 0 ) // Replacements for custom chip on original?
	ROM_LOAD( "pal16l8.q10.bin", 0x0000, 0x0001, NO_DUMP) // PAL16L8ACN
	ROM_LOAD( "pal16l8.r1.bin",  0x0001, 0x0001, NO_DUMP) // PAL16L8ACN
	ROM_LOAD( "pal16l8.s1.bin",  0x0002, 0x0001, NO_DUMP) // PAL16L8ACN
ROM_END

ROM_START( freekickb2 )
	ROM_REGION( 0x10000, "maincpu", 0 ) // Z80 code
	ROM_LOAD( "15.bin", 0x00000, 0x10000, CRC(6569f2b0) SHA1(9306e316e6ae659bae3759a12a4e445b555a8893) )

	ROM_REGION( 0x08000, "sound_data", 0 )
	ROM_LOAD( "1.bin", 0x00000, 0x08000, CRC(a6030ba9) SHA1(f363100f54a7a80701a6395c7539b8daa60db054) )

	// the first half of the GFX ROMs isn't used on this bootleg (roms are double size) - the content is otherwise identical
	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "2.bin", 0x000000, 0x04000, CRC(96aeae91) SHA1(073ca6c9fbe14760ee10293791254da3bcb43940) )
	ROM_CONTINUE(0x0000,0x4000)
	ROM_LOAD( "3.bin", 0x004000, 0x04000, CRC(489f538c) SHA1(47e5b3db1bb9efe0e432be83220d9fc09695fdf1) )
	ROM_CONTINUE(0x4000,0x4000)
	ROM_LOAD( "4.bin", 0x008000, 0x04000, CRC(73cdb431) SHA1(97f62d8cb21dfd905713b40d54da270d9ff275b9) )
	ROM_CONTINUE(0x8000,0x4000)

	ROM_REGION( 0xc000, "sprites", 0 )
	ROM_LOAD( "5.bin", 0x000000, 0x04000, CRC(7def1c52) SHA1(dbe8c7b7c45ea681870b2039cb80728530a057c3) )
	ROM_CONTINUE(0x0000,0x4000)
	ROM_LOAD( "6.bin", 0x004000, 0x04000, CRC(59d1b3e7) SHA1(de82dcbaa0fa4f5b5f0cb0c925c12fac8828fa01) )
	ROM_CONTINUE(0x4000,0x4000)
	ROM_LOAD( "7.bin", 0x008000, 0x04000, CRC(95c19081) SHA1(d8abc6f8da9188ca7b322f17c16e0494d771df39) )
	ROM_CONTINUE(0x8000,0x4000)

	// no PROMs in the dump, but almost certainly identical
	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "24s10n.8j", 0x0000, 0x0100, CRC(53a6bc21) SHA1(d4beedc226004c1aa9b6aae29bee9c8a9b0fff7c) ) // Or compatible type PROM like the 82S129
	ROM_LOAD( "24s10n.7j", 0x0100, 0x0100, CRC(38dd97d8) SHA1(468a0f87a704982dc1bce1ca21f9bb252ac241a0) )
	ROM_LOAD( "24s10n.8k", 0x0200, 0x0100, CRC(18e66087) SHA1(54857526179b738862d11ce87e9d0edcb7878488) )
	ROM_LOAD( "24s10n.7k", 0x0300, 0x0100, CRC(bc21797a) SHA1(4d6cf05e51b7ef9147eeff051c3728764021cfdb) )
	ROM_LOAD( "24s10n.8h", 0x0400, 0x0100, CRC(8aac5fd0) SHA1(07a179603c0167c1f998b2337d66be95db9911cc) )
	ROM_LOAD( "24s10n.7h", 0x0500, 0x0100, CRC(a507f941) SHA1(97619959ee4c366cb010525636ab5eefe5a3127a) )
ROM_END

// Daughter card instead of CPU block containing: 2 TMS 27C256 eproms, 64K ram, Z80 and N82S123AN BPROM of unknown use
ROM_START( freekickb3 )
	ROM_REGION( 0x10000, "maincpu", 0 ) // Z80 code
	ROM_LOAD( "1", 0x00000, 0x08000, CRC(214e1868) SHA1(35cd24a1800cca40a7048c66412ae45e1f665958) )
	ROM_LOAD( "2", 0x08000, 0x08000, CRC(734cdfc7) SHA1(d37b3b5e89a1ca4e5441643b255cee4cac9c1b95) )

	ROM_REGION( 0x08000, "sound_data", 0 )
	ROM_LOAD( "11.1e", 0x00000, 0x08000, CRC(a6030ba9) SHA1(f363100f54a7a80701a6395c7539b8daa60db054) ) // freekbl1.e2

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "12.1h", 0x000000, 0x04000, CRC(fb82e486) SHA1(bc672272dc32b2aa64e991992172c44bea1ca65c) ) // freekbl2.f2
	ROM_LOAD( "13.1j", 0x004000, 0x04000, CRC(3ad78ee2) SHA1(033285d4ab7d6f46abf4c1bd4671c874738f0ac1) ) // freekbl3.j2
	ROM_LOAD( "14.1l", 0x008000, 0x04000, CRC(0185695f) SHA1(126994c69de157fc7c452ccc7f1a767f5085da27) ) // freekbl4.k2

	ROM_REGION( 0xc000, "sprites", 0 )
	ROM_LOAD( "15.1m", 0x000000, 0x04000, CRC(0fa7c13c) SHA1(24b0ca73b0e35474e2392d8e729bcd44b80f9135) ) // freekbl5.m2
	ROM_LOAD( "16.1p", 0x004000, 0x04000, CRC(2b996e89) SHA1(c6900449d27e89c3b444fb028694fdcda8e79322) ) // freekbl6.n2
	ROM_LOAD( "17.1r", 0x008000, 0x04000, CRC(e7894def) SHA1(5c97b7cce43d1e51c709603a0d2394b8119764bd) ) // freekbl7.r2

	ROM_REGION( 0x0700, "proms", 0 ) // verified correct
	ROM_LOAD( "24s10n.8j", 0x0000, 0x0100, CRC(53a6bc21) SHA1(d4beedc226004c1aa9b6aae29bee9c8a9b0fff7c) ) // Or compatible type PROM like the 82S129
	ROM_LOAD( "24s10n.7j", 0x0100, 0x0100, CRC(38dd97d8) SHA1(468a0f87a704982dc1bce1ca21f9bb252ac241a0) )
	ROM_LOAD( "24s10n.8k", 0x0200, 0x0100, CRC(18e66087) SHA1(54857526179b738862d11ce87e9d0edcb7878488) )
	ROM_LOAD( "24s10n.7k", 0x0300, 0x0100, CRC(bc21797a) SHA1(4d6cf05e51b7ef9147eeff051c3728764021cfdb) )
	ROM_LOAD( "24s10n.8h", 0x0400, 0x0100, CRC(8aac5fd0) SHA1(07a179603c0167c1f998b2337d66be95db9911cc) )
	ROM_LOAD( "24s10n.7h", 0x0500, 0x0100, CRC(a507f941) SHA1(97619959ee4c366cb010525636ab5eefe5a3127a) )
	ROM_LOAD( "n82s123an", 0x0600, 0x0020, CRC(5ed93a02) SHA1(995948c255a74c91ddb086dfff8ba88c71ef2a59) ) // Unknown use on the CPU daughter card
ROM_END

/*

The original Counter Run set doesn't work, it's missing the main CPU code which is inside a custom CPU "block"
just like the original Freekick boards. Hopefully an original Counter Run board can be found and dumped (using
the same method as used for Freekick) before they all die, batteries don't last for ever.

*/

ROM_START( countrun )
	ROM_REGION( 0x10000, "maincpu", 0 ) // Z80 code
	//  Custom CPU (pack) No. NS6201-A 1988.3 COUNTER RUN
	ROM_LOAD( "ns6201-a_1988.3_counter_run.cpu", 0x00000, 0x10000, NO_DUMP ) // Need to find a working PCB module to dump!!

	ROM_REGION( 0x08000, "sound_data", 0 )
	ROM_LOAD( "c-run.e1", 0x00000, 0x08000, CRC(2c3b6f8f) SHA1(ee7d71e6d8bb7138d5d029a10a95471d387b5f29) ) // rom 1

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "c-run.h1", 0x000000, 0x04000, CRC(3385b7b5) SHA1(3f8f96f2a5406369dd56a9fe9f509ebee4a0179a) ) // rom 2
	ROM_LOAD( "c-run.j1", 0x004000, 0x04000, CRC(58dc148d) SHA1(3b2e5c6ced885d945f6c02fbab7c6d40db78c66a) ) // rom 3
	ROM_LOAD( "c-run.l1", 0x008000, 0x04000, CRC(3201f1e9) SHA1(72bd35600bf6e38741730f39bfd2a19f359bfb93) ) // rom 4

	ROM_REGION( 0xc000, "sprites", 0 )
	ROM_LOAD( "c-run.m1", 0x000000, 0x04000, CRC(1efab3b4) SHA1(7ce39cecf2809d3a7cbca5c6dffee738ba6f7b11) ) // rom 5
	ROM_LOAD( "c-run.p1", 0x004000, 0x04000, CRC(d0bf8d42) SHA1(b8d1bd155dba065475c84db768f14a3562fe21e0) ) // rom 6
	ROM_LOAD( "c-run.r1", 0x008000, 0x04000, CRC(4bb4a3e3) SHA1(179696464fce548ec333eec233025840fdb1eac2) ) // rom 7

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "24s10n.8j", 0x0000, 0x0100, CRC(63c114ad) SHA1(db881c4ff92cb04a94988587503346a44eb89b69) ) // Or compatible type PROM like the 82S129
	ROM_LOAD( "24s10n.7j", 0x0100, 0x0100, CRC(d16f95cc) SHA1(041bb84576bd8492c1ad3e492d8cb3e04d316527) )
	ROM_LOAD( "24s10n.8k", 0x0200, 0x0100, CRC(217db2c1) SHA1(f2af1a74b0ce56290b1c119e1a9707287132194a) )
	ROM_LOAD( "24s10n.7k", 0x0300, 0x0100, CRC(8d983949) SHA1(d7331900d18a53ceb133f8a8848d3c108e03323a) )
	ROM_LOAD( "24s10n.8h", 0x0400, 0x0100, CRC(33e87550) SHA1(951ce0dc975b799c1056ce8eb005256cbb43a112) )
	ROM_LOAD( "24s10n.7h", 0x0500, 0x0100, CRC(c77d0077) SHA1(4cbbf625ad5e45d00ca6aebe9566538ff0a3348d) )
ROM_END

ROM_START( countrunb )
	ROM_REGION( 0x10000, "maincpu", 0 ) // Z80 code
	ROM_LOAD( "rom_cpu.bin", 0x00000, 0x10000, CRC(f65639ae) SHA1(faa81607858d49559098c887ac847722df955a76) )

	ROM_REGION( 0x08000, "sound_data", 0 )
	ROM_LOAD( "c-run.e1", 0x00000, 0x08000, CRC(2c3b6f8f) SHA1(ee7d71e6d8bb7138d5d029a10a95471d387b5f29) ) // rom 1

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "c-run.h1", 0x000000, 0x04000, CRC(3385b7b5) SHA1(3f8f96f2a5406369dd56a9fe9f509ebee4a0179a) ) // rom 2
	ROM_LOAD( "c-run.j1", 0x004000, 0x04000, CRC(58dc148d) SHA1(3b2e5c6ced885d945f6c02fbab7c6d40db78c66a) ) // rom 3
	ROM_LOAD( "c-run.l1", 0x008000, 0x04000, CRC(3201f1e9) SHA1(72bd35600bf6e38741730f39bfd2a19f359bfb93) ) // rom 4

	ROM_REGION( 0xc000, "sprites", 0 )
	ROM_LOAD( "c-run.m1", 0x000000, 0x04000, CRC(1efab3b4) SHA1(7ce39cecf2809d3a7cbca5c6dffee738ba6f7b11) ) // rom 5
	ROM_LOAD( "c-run.p1", 0x004000, 0x04000, CRC(d0bf8d42) SHA1(b8d1bd155dba065475c84db768f14a3562fe21e0) ) // rom 6
	ROM_LOAD( "c-run.r1", 0x008000, 0x04000, CRC(4bb4a3e3) SHA1(179696464fce548ec333eec233025840fdb1eac2) ) // rom 7

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "24s10n.8j", 0x0000, 0x0100, CRC(63c114ad) SHA1(db881c4ff92cb04a94988587503346a44eb89b69) ) // Or compatible type PROM like the 82S129
	ROM_LOAD( "24s10n.7j", 0x0100, 0x0100, CRC(d16f95cc) SHA1(041bb84576bd8492c1ad3e492d8cb3e04d316527) )
	ROM_LOAD( "24s10n.8k", 0x0200, 0x0100, CRC(217db2c1) SHA1(f2af1a74b0ce56290b1c119e1a9707287132194a) )
	ROM_LOAD( "24s10n.7k", 0x0300, 0x0100, CRC(8d983949) SHA1(d7331900d18a53ceb133f8a8848d3c108e03323a) )
	ROM_LOAD( "24s10n.8h", 0x0400, 0x0100, CRC(33e87550) SHA1(951ce0dc975b799c1056ce8eb005256cbb43a112) )
	ROM_LOAD( "24s10n.7h", 0x0500, 0x0100, CRC(c77d0077) SHA1(4cbbf625ad5e45d00ca6aebe9566538ff0a3348d) )
ROM_END

ROM_START( countrunb2 )
	ROM_REGION( 0x10000, "maincpu", 0 ) // Z80 code
	ROM_LOAD( "crunbl.8", 0x00000, 0x10000, CRC(318f95d9) SHA1(f2386b9d26d1bc98728aad9e257363b381043dc9) ) // encrypted? bad? its strange anyway

	ROM_REGION( 0x01000, "cpu1", 0 )
	ROM_LOAD( "68705.uc",  0x00000, 0x01000, NO_DUMP )

	ROM_REGION( 0x08000, "sound_data", 0 )
	ROM_LOAD( "c-run.e1", 0x00000, 0x08000, CRC(2c3b6f8f) SHA1(ee7d71e6d8bb7138d5d029a10a95471d387b5f29) ) // rom 1

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "c-run.h1", 0x000000, 0x04000, CRC(3385b7b5) SHA1(3f8f96f2a5406369dd56a9fe9f509ebee4a0179a) ) // rom 2
	ROM_LOAD( "c-run.j1", 0x004000, 0x04000, CRC(58dc148d) SHA1(3b2e5c6ced885d945f6c02fbab7c6d40db78c66a) ) // rom 3
	ROM_LOAD( "c-run.l1", 0x008000, 0x04000, CRC(3201f1e9) SHA1(72bd35600bf6e38741730f39bfd2a19f359bfb93) ) // rom 4

	ROM_REGION( 0xc000, "sprites", 0 )
	ROM_LOAD( "c-run.m1", 0x000000, 0x04000, CRC(1efab3b4) SHA1(7ce39cecf2809d3a7cbca5c6dffee738ba6f7b11) ) // rom 5
	ROM_LOAD( "c-run.p1", 0x004000, 0x04000, CRC(d0bf8d42) SHA1(b8d1bd155dba065475c84db768f14a3562fe21e0) ) // rom 6
	ROM_LOAD( "c-run.r1", 0x008000, 0x04000, CRC(4bb4a3e3) SHA1(179696464fce548ec333eec233025840fdb1eac2) ) // rom 7

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "24s10n.8j", 0x0000, 0x0100, CRC(63c114ad) SHA1(db881c4ff92cb04a94988587503346a44eb89b69) ) // Or compatible type PROM like the 82S129
	ROM_LOAD( "24s10n.7j", 0x0100, 0x0100, CRC(d16f95cc) SHA1(041bb84576bd8492c1ad3e492d8cb3e04d316527) )
	ROM_LOAD( "24s10n.8k", 0x0200, 0x0100, CRC(217db2c1) SHA1(f2af1a74b0ce56290b1c119e1a9707287132194a) )
	ROM_LOAD( "24s10n.7k", 0x0300, 0x0100, CRC(8d983949) SHA1(d7331900d18a53ceb133f8a8848d3c108e03323a) )
	ROM_LOAD( "24s10n.8h", 0x0400, 0x0100, CRC(33e87550) SHA1(951ce0dc975b799c1056ce8eb005256cbb43a112) )
	ROM_LOAD( "24s10n.7h", 0x0500, 0x0100, CRC(c77d0077) SHA1(4cbbf625ad5e45d00ca6aebe9566538ff0a3348d) )
ROM_END

ROM_START( countrunb3 ) // mostly similar to countrunb2, last routine at 0xb2a2 is slightly changed, GFX ROMs are double sized with the first half 0xff filled
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "electric_16.t2", 0x00000, 0x10000, CRC(14061503) SHA1(90780ff51a93284e5b92e7777a0ba0a18276071e) )

	ROM_REGION( 0x8000, "sound_data", 0 )
	ROM_LOAD( "electric_9.o2", 0x0000, 0x8000, CRC(2c3b6f8f) SHA1(ee7d71e6d8bb7138d5d029a10a95471d387b5f29) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "electric_10.m2", 0x0000, 0x4000, CRC(41cd6adb) SHA1(0ccbab05a3165902f10298188810734bb9560f7e) ) // 0xxxxxxxxxxxxxx = 0xFF
	ROM_CONTINUE(               0x0000, 0x4000 )
	ROM_LOAD( "electric_11.l2", 0x4000, 0x4000, CRC(2a94c9e3) SHA1(20869cbf656e87d9381efc9fd13914685317e672) ) // 0xxxxxxxxxxxxxx = 0xFF
	ROM_CONTINUE(               0x4000, 0x4000 )
	ROM_LOAD( "electric_12.g2", 0x8000, 0x4000, CRC(40492c87) SHA1(314f43465f6b11cd843ae12d7338a27160b06998) ) // 0xxxxxxxxxxxxxx = 0xFF
	ROM_CONTINUE(               0x8000, 0x4000 )

	ROM_REGION( 0xc000, "sprites", 0 )
	ROM_LOAD( "electric_13.f2", 0x0000, 0x4000, CRC(6cb26eda) SHA1(b5fd8379b01cd68865c0a6962ce075564847dc93) ) // 0xxxxxxxxxxxxxx = 0xFF
	ROM_CONTINUE(               0x0000, 0x4000 )
	ROM_LOAD( "electric_14.d2", 0x4000, 0x4000, CRC(a2f7502c) SHA1(6c548c44e4e03855e3258e1cbba73174c783b7ce) ) // 0xxxxxxxxxxxxxx = 0xFF
	ROM_CONTINUE(               0x4000, 0x4000 )
	ROM_LOAD( "electric_15.c2", 0x8000, 0x4000, CRC(39fc7e8d) SHA1(6ea88c8d9cf857802cc48c62e4c87cd913194c07) ) // 0xxxxxxxxxxxxxx = 0xFF
	ROM_CONTINUE(               0x8000, 0x4000 )

	ROM_REGION( 0x600, "proms", 0 )
	ROM_LOAD( "n82s129n.9l", 0x000, 0x100, CRC(63c114ad) SHA1(db881c4ff92cb04a94988587503346a44eb89b69) )
	ROM_LOAD( "n82s129n.8l", 0x100, 0x100, CRC(d16f95cc) SHA1(041bb84576bd8492c1ad3e492d8cb3e04d316527) )
	ROM_LOAD( "n82s129n.9h", 0x200, 0x100, CRC(217db2c1) SHA1(f2af1a74b0ce56290b1c119e1a9707287132194a) )
	ROM_LOAD( "n82s129n.8h", 0x300, 0x100, CRC(8d983949) SHA1(d7331900d18a53ceb133f8a8848d3c108e03323a) )
	ROM_LOAD( "n82s129n.9m", 0x400, 0x100, CRC(33e87550) SHA1(951ce0dc975b799c1056ce8eb005256cbb43a112) )
	ROM_LOAD( "n82s129n.8m", 0x500, 0x100, CRC(c77d0077) SHA1(4cbbf625ad5e45d00ca6aebe9566538ff0a3348d) )
ROM_END

ROM_START( gigas ) // From an actual Sega board "834-6167 // GIGAS" with MC-8123: 317-5002
/* The MC-8123 is located on a small daughterboard which plugs into the z80 socket;
 * the daughterboard also has an input for the spinner control the game uses
 * An empty socket marked 27256 is at location 10n */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "8.8n",   0x00000, 0x4000, CRC(34ea8262) SHA1(a71c865ffccfc79455402c53639c4fa77a746cf1) )
	ROM_LOAD( "7.8r",   0x04000, 0x8000, CRC(43653909) SHA1(30f6666ba5c0f016299f462c4c07c81ee4832808) ) // 27256

	ROM_REGION( 0x2000, "maincpu:key", 0 ) // MC8123 key
	ROM_LOAD( "317-5002.key", 0x0000, 0x2000, CRC(86a7e5f6) SHA1(3ff3a17c02eb5610182b6febfada4e8eca0c5eea) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "4.3k", 0x00000, 0x04000, CRC(8ed78981) SHA1(1f2c0584fcc6d04b042638c7b9a7e21fc560ca3d) )
	ROM_LOAD( "5.3h", 0x04000, 0x04000, CRC(0645ec2d) SHA1(ecf8b1ce98f845b5b32e7fc959cea7679a149d74) )
	ROM_LOAD( "6.3g", 0x08000, 0x04000, CRC(99e9cb27) SHA1(d141d6caa077e3cd182eb64cf803613ac17e7d09) )

	ROM_REGION( 0xc000, "sprites", 0 )
	ROM_LOAD( "1.3p", 0x00000, 0x04000, CRC(d78fae6e) SHA1(a7bf3b213f2a3a51b964959bd45003351670575a) )
	ROM_LOAD( "3.3l", 0x04000, 0x04000, CRC(37df4a4c) SHA1(ab996db636d89845474529ba2573307046fb96ee) )
	ROM_LOAD( "2.3n", 0x08000, 0x04000, CRC(3a46e354) SHA1(ebd6a5db4c9cdfc6fabe6b412a704aaf03c32d7c) )

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "tbp24s10n.3a", 0x0000, 0x0100, CRC(a784e71f) SHA1(1741ce98d719bad6cc5ea42337ef897f2435bbab) ) // Or compatible type PROM like the 82S129
	ROM_LOAD( "tbp24s10n.4d", 0x0100, 0x0100, CRC(376df30c) SHA1(cc95920cd1c133da1becc7d92f4b187b56a90ec7) )
	ROM_LOAD( "tbp24s10n.4a", 0x0200, 0x0100, CRC(4edff5bd) SHA1(305efc7ad7f86635489a655e214e216ac02b904d) )
	ROM_LOAD( "tbp24s10n.3d", 0x0300, 0x0100, CRC(fe201a4e) SHA1(15f8ecfcf6c63ffbf9777bec9b203c319ba1b96c) )
	ROM_LOAD( "tbp24s10n.3b", 0x0400, 0x0100, CRC(5796cc4a) SHA1(39576c4e48fd7ac52fc652a1ae0573db3d878878) )
	ROM_LOAD( "tbp24s10n.3c", 0x0500, 0x0100, CRC(28b5ee4c) SHA1(e21b9c38f433dca1e8894619b1d9f0389a81b48a) )
ROM_END

/*
Gigas (bootleg)

CPU: Z80 (on a small plug in board), 8748 MCU (on same plug-in board)
SND: 76489 x3
DIPSW: 8 position x3
RAM: 6116 x2 (near ROMs 1-6), 6264 x1 (near ROMs 7-8), 2018 x2 (located in center of board)
XTAL: 18.432MHz
PROMs: 82s129 x6 (not dumped yet, probably match existing archives....)

Note: MCU dump (in oigas?) has fixed bits, but read is good. If not correct, it's protected.
*/
ROM_START( gigasb )
	ROM_REGION( 2*0xc000, "maincpu", 0 )
	ROM_LOAD( "g-7.r8",   0x0c000, 0x4000, CRC(daf4e88d) SHA1(391dff914ce8e9b7975fc8827c066d7db16c4171) )
	ROM_CONTINUE(         0x00000, 0x4000 )
	ROM_LOAD( "g-8.t8",   0x10000, 0x8000, CRC(4ab4c1f1) SHA1(63d8f489c7a8271e99a66d97e6eb0eb252cb2b67) )
	ROM_CONTINUE(         0x04000, 0x8000 )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "g-4.3l",  0x00000, 0x04000, CRC(8ed78981) SHA1(1f2c0584fcc6d04b042638c7b9a7e21fc560ca3d) )
	ROM_LOAD( "g-5.3k",  0x04000, 0x04000, CRC(0645ec2d) SHA1(ecf8b1ce98f845b5b32e7fc959cea7679a149d74) )
	ROM_LOAD( "g-6.3fh", 0x08000, 0x04000, CRC(99e9cb27) SHA1(d141d6caa077e3cd182eb64cf803613ac17e7d09) )

	ROM_REGION( 0xc000, "sprites", 0 )
	ROM_LOAD( "g-1.3t", 0x00000, 0x04000, CRC(d78fae6e) SHA1(a7bf3b213f2a3a51b964959bd45003351670575a) )
	ROM_LOAD( "g-3.3p", 0x04000, 0x04000, CRC(37df4a4c) SHA1(ab996db636d89845474529ba2573307046fb96ee) )
	ROM_LOAD( "g-2.3r", 0x08000, 0x04000, CRC(3a46e354) SHA1(ebd6a5db4c9cdfc6fabe6b412a704aaf03c32d7c) )

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "1.pr", 0x0000, 0x0100, CRC(a784e71f) SHA1(1741ce98d719bad6cc5ea42337ef897f2435bbab) ) // 24S10N, 82S129 or compatible?
	ROM_LOAD( "6.pr", 0x0100, 0x0100, CRC(376df30c) SHA1(cc95920cd1c133da1becc7d92f4b187b56a90ec7) )
	ROM_LOAD( "5.pr", 0x0200, 0x0100, CRC(4edff5bd) SHA1(305efc7ad7f86635489a655e214e216ac02b904d) )
	ROM_LOAD( "4.pr", 0x0300, 0x0100, CRC(fe201a4e) SHA1(15f8ecfcf6c63ffbf9777bec9b203c319ba1b96c) )
	ROM_LOAD( "2.pr", 0x0400, 0x0100, CRC(5796cc4a) SHA1(39576c4e48fd7ac52fc652a1ae0573db3d878878) )
	ROM_LOAD( "3.pr", 0x0500, 0x0100, CRC(28b5ee4c) SHA1(e21b9c38f433dca1e8894619b1d9f0389a81b48a) )
ROM_END

ROM_START( oigas )
	ROM_REGION( 2*0xc000, "maincpu", 0 )
	ROM_LOAD( "rom.7",   0x0c000, 0x4000, CRC(e5bc04cc) SHA1(ffbd416313a9e49d2f9a7268d5ef48a8b641e480) )
	ROM_CONTINUE(        0x00000, 0x4000)
	ROM_LOAD( "rom.8",   0x04000, 0x8000, CRC(c199060d) SHA1(de8f1e0f941533abbbed25b595b1d51fadbb428d) )

	ROM_REGION( 0x0800, "cpu1", 0 )
	ROM_LOAD( "8748.bin", 0x0000, 0x0800, NO_DUMP ) // missing

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "g-4", 0x00000, 0x04000, CRC(8ed78981) SHA1(1f2c0584fcc6d04b042638c7b9a7e21fc560ca3d) )
	ROM_LOAD( "g-5", 0x04000, 0x04000, CRC(0645ec2d) SHA1(ecf8b1ce98f845b5b32e7fc959cea7679a149d74) )
	ROM_LOAD( "g-6", 0x08000, 0x04000, CRC(99e9cb27) SHA1(d141d6caa077e3cd182eb64cf803613ac17e7d09) )

	ROM_REGION( 0xc000, "sprites", 0 )
	ROM_LOAD( "g-1", 0x00000, 0x04000, CRC(d78fae6e) SHA1(a7bf3b213f2a3a51b964959bd45003351670575a) )
	ROM_LOAD( "g-3", 0x04000, 0x04000, CRC(37df4a4c) SHA1(ab996db636d89845474529ba2573307046fb96ee) )
	ROM_LOAD( "g-2", 0x08000, 0x04000, CRC(3a46e354) SHA1(ebd6a5db4c9cdfc6fabe6b412a704aaf03c32d7c) )

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "1.pr", 0x0000, 0x0100, CRC(a784e71f) SHA1(1741ce98d719bad6cc5ea42337ef897f2435bbab) ) // 24S10N, 82S129 or compatible?
	ROM_LOAD( "6.pr", 0x0100, 0x0100, CRC(376df30c) SHA1(cc95920cd1c133da1becc7d92f4b187b56a90ec7) )
	ROM_LOAD( "5.pr", 0x0200, 0x0100, CRC(4edff5bd) SHA1(305efc7ad7f86635489a655e214e216ac02b904d) )
	ROM_LOAD( "4.pr", 0x0300, 0x0100, CRC(fe201a4e) SHA1(15f8ecfcf6c63ffbf9777bec9b203c319ba1b96c) )
	ROM_LOAD( "2.pr", 0x0400, 0x0100, CRC(5796cc4a) SHA1(39576c4e48fd7ac52fc652a1ae0573db3d878878) )
	ROM_LOAD( "3.pr", 0x0500, 0x0100, CRC(28b5ee4c) SHA1(e21b9c38f433dca1e8894619b1d9f0389a81b48a) )
ROM_END

// Gigas Mark II is a romswap/upgrade to gigas, and uses the same MC-8123 (317-5002).
ROM_START( gigasm2 ) // From an actual Sega board "834-6167-01 // GIGAS 2" with MC-8123: 317-5002
/* The MC-8123 is located on a small daughterboard which plugs into the z80 socket;
 * the daughterboard also has an input for the spinner control the game uses
 * An empty socket marked 27256 is at location 10n */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "18.8n",   0x00000, 0x4000, CRC(32e83d80) SHA1(c1b5e995f32e775e9b73c56395a27f6a9f40d156) )
	ROM_LOAD( "17.8r",   0x04000, 0x8000, CRC(460dadd2) SHA1(9f0ee949f391bcc7c1b9911a5385eb1cbf9d0e4d) ) // 27256

	ROM_REGION( 0x2000, "maincpu:key", 0 ) // MC8123 key
	ROM_LOAD( "317-5002.key", 0x0000, 0x2000, CRC(86a7e5f6) SHA1(3ff3a17c02eb5610182b6febfada4e8eca0c5eea) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "14.3k", 0x00000, 0x04000, CRC(20b3405f) SHA1(32120d7b40e74648eb4ac4ab3ad3d2125033f6b1) )
	ROM_LOAD( "15.3h", 0x04000, 0x04000, CRC(d04ecfa8) SHA1(10bfb1d075da768f31a8c34cdfe1a1bf01e89f94) )
	ROM_LOAD( "16.3g", 0x08000, 0x04000, CRC(33776801) SHA1(29952818038a08c98b95ac801b8929cf1647049c) )

	ROM_REGION( 0xc000, "sprites", 0 )
	ROM_LOAD( "11.3p", 0x00000, 0x04000, CRC(f64cbd1e) SHA1(f8d9b110cdac6ef524e35bec9a5d406651cd7bab) )
	ROM_LOAD( "13.3l", 0x04000, 0x04000, CRC(c228df19) SHA1(584f269f7de2d531f2b038b4b7318f813c329f7f) )
	ROM_LOAD( "12.3n", 0x08000, 0x04000, CRC(a6ad9ce2) SHA1(db0338385208df9e9cf43efc11383412dec493e6) )

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "tbp24s10n.3a", 0x0000, 0x0100, CRC(a784e71f) SHA1(1741ce98d719bad6cc5ea42337ef897f2435bbab) ) // Or compatible type PROM like the 82S129
	ROM_LOAD( "tbp24s10n.4d", 0x0100, 0x0100, CRC(376df30c) SHA1(cc95920cd1c133da1becc7d92f4b187b56a90ec7) )
	ROM_LOAD( "tbp24s10n.4a", 0x0200, 0x0100, CRC(4edff5bd) SHA1(305efc7ad7f86635489a655e214e216ac02b904d) )
	ROM_LOAD( "tbp24s10n.3d", 0x0300, 0x0100, CRC(fe201a4e) SHA1(15f8ecfcf6c63ffbf9777bec9b203c319ba1b96c) )
	ROM_LOAD( "tbp24s10n.3b", 0x0400, 0x0100, CRC(5796cc4a) SHA1(39576c4e48fd7ac52fc652a1ae0573db3d878878) )
	ROM_LOAD( "tbp24s10n.3c", 0x0500, 0x0100, CRC(28b5ee4c) SHA1(e21b9c38f433dca1e8894619b1d9f0389a81b48a) )
ROM_END

// bootleg of Gigas MarkII without the MC-8123
ROM_START( gigasm2b )
	ROM_REGION( 2*0xc000, "maincpu", 0 )
	ROM_LOAD( "8.rom", 0x0c000, 0x4000, CRC(c00a4a6c) SHA1(0d1bb849c9bfe4e92ad70e4ef19da494c0bd7ba8) )
	ROM_CONTINUE(      0x00000, 0x4000 )
	ROM_LOAD( "7.rom", 0x10000, 0x4000, CRC(92bd9045) SHA1(e4d8a94deeb795bb284ca0bd211ed40ed498b172) )
	ROM_CONTINUE(      0x04000, 0x4000 )
	ROM_LOAD( "9.rom", 0x14000, 0x4000, CRC(a3ef809c) SHA1(6d4098658aa124e10e5edb8e8e3abe0aa26741a1) )
	ROM_CONTINUE(      0x08000, 0x4000 )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "4.rom", 0x00000, 0x04000, CRC(20b3405f) SHA1(32120d7b40e74648eb4ac4ab3ad3d2125033f6b1) ) // == 14.3k
	ROM_LOAD( "5.rom", 0x04000, 0x04000, CRC(d04ecfa8) SHA1(10bfb1d075da768f31a8c34cdfe1a1bf01e89f94) ) // == 15.3h
	ROM_LOAD( "6.rom", 0x08000, 0x04000, CRC(33776801) SHA1(29952818038a08c98b95ac801b8929cf1647049c) ) // == 16.3g

	ROM_REGION( 0xc000, "sprites", 0 )
	ROM_LOAD( "1.rom", 0x00000, 0x04000, CRC(f64cbd1e) SHA1(f8d9b110cdac6ef524e35bec9a5d406651cd7bab) ) // == 11.3p
	ROM_LOAD( "3.rom", 0x04000, 0x04000, CRC(c228df19) SHA1(584f269f7de2d531f2b038b4b7318f813c329f7f) ) // == 13.3l
	ROM_LOAD( "2.rom", 0x08000, 0x04000, CRC(a6ad9ce2) SHA1(db0338385208df9e9cf43efc11383412dec493e6) ) // == 12.3n

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "1.pr", 0x0000, 0x0100, CRC(a784e71f) SHA1(1741ce98d719bad6cc5ea42337ef897f2435bbab) ) // 24S10N, 82S129 or compatible?
	ROM_LOAD( "6.pr", 0x0100, 0x0100, CRC(376df30c) SHA1(cc95920cd1c133da1becc7d92f4b187b56a90ec7) )
	ROM_LOAD( "5.pr", 0x0200, 0x0100, CRC(4edff5bd) SHA1(305efc7ad7f86635489a655e214e216ac02b904d) )
	ROM_LOAD( "4.pr", 0x0300, 0x0100, CRC(fe201a4e) SHA1(15f8ecfcf6c63ffbf9777bec9b203c319ba1b96c) )
	ROM_LOAD( "2.pr", 0x0400, 0x0100, CRC(5796cc4a) SHA1(39576c4e48fd7ac52fc652a1ae0573db3d878878) )
	ROM_LOAD( "3.pr", 0x0500, 0x0100, CRC(28b5ee4c) SHA1(e21b9c38f433dca1e8894619b1d9f0389a81b48a) )
ROM_END

// Omega code/GFX looks to be based on gigas mk2, given the "MarkII" graphic in the GFX ROMs and gigas MarkII style continue numbers etc
// PCB is marked "K.K NS6102-A" and seems to be somewhere between gigas hardware and freekick hardware (3x dipswitch arrays)
// Supposedly an extremely limited release with ~10 PCBs produced.
// A second PCB found with the CPU (under a metal cap) to be a NEC MC-8123 317-5002 - same as Gigas & Gigas Mark II, however
//  neither Omega set will work with the 317-5002 key in MAME, so maybe the CPU was factory reprogrammed?
// A single byte difference at 0x1120 in 17.M10 (when decoded) looks like a legit bug fix as it changes a branch
//  which incorrectly jumps over a bit of initialization code

ROM_START( omega ) // ROM at M10 labeled "17" to indicate a later Bug fix version
	ROM_REGION(0xc000, "maincpu", 0) // encrypted
	ROM_LOAD( "17.m10", 0x0000, 0x4000, CRC(c7de0993) SHA1(35ecd464935faba1dc7d0dbf48e1b17153626bfd) ) // 27128
	ROM_LOAD( "8.n10",  0x4000, 0x8000, CRC(9bb61910) SHA1(f8a1210dbf93e901e246e6adf4cd905acc3ef376) ) // 27256

	ROM_REGION(0x2000, "maincpu:key", 0) // MC8123 key
	ROM_LOAD( "omega.key", 0x0000, 0x2000, CRC(0a63943f) SHA1(9e581ea0c5bf6c0ed5d402d3bab053766b8e44c2) )

	ROM_REGION(0xc000, "tiles", 0)
	ROM_LOAD( "4.f10",  0x00000, 0x04000, CRC(bf780a8e) SHA1(53bfabf74f1a7782c6c1803498a24da0bf8db995) ) // 27128
	ROM_LOAD( "5.h10",  0x04000, 0x04000, CRC(b491647f) SHA1(88017033a781ecc49a83241bc49e2077a480ac2b) ) // 27128
	ROM_LOAD( "6.j10",  0x08000, 0x04000, CRC(65beba5b) SHA1(e6d61dc52dcbb30570b48d7b1d7807dd0be41400) ) // 27128

	ROM_REGION(0xc000, "sprites", 0)
	ROM_LOAD( "3.d10",  0x00000, 0x04000, CRC(c678b202) SHA1(ee93385e11158ccaf51a22d813bd7020c04cfdad) ) // 27128
	ROM_LOAD( "1.a10",  0x04000, 0x04000, CRC(e0aeada9) SHA1(ed00f6dca4f9701ff89390922d39341b179597c7) ) // 27128
	ROM_LOAD( "2.c10",  0x08000, 0x04000, CRC(dbc0a47f) SHA1(b617c5a10c655e7befaeaecd9ce736e972285e6b) ) // 27128

	ROM_REGION(0x600, "proms", 0)
	ROM_LOAD( "tbp24s10n.3f", 0x0000, 0x100, CRC(75ec7472) SHA1(868811e838c570a0f576a0ece249cab2d4274d65) ) // Or compatible type PROM like the 82S129
	ROM_LOAD( "tbp24s10n.4f", 0x0100, 0x100, CRC(5113a114) SHA1(3a5ab68c93d1f2c05ceb0311e12a54fd124d8435) )
	ROM_LOAD( "tbp24s10n.3g", 0x0200, 0x100, CRC(b6b5d4a0) SHA1(2b7ba59a6c185326e11ce8ccd96b3c8cfd652fdf) )
	ROM_LOAD( "tbp24s10n.4g", 0x0300, 0x100, CRC(931bc299) SHA1(f116f1d6a4324b86b0aae0a5a040236b3a4fd12d) )
	ROM_LOAD( "tbp24s10n.3e", 0x0400, 0x100, CRC(899e089d) SHA1(5a485d3ef7d2102451ff76452cac106061cc5cd6) )
	ROM_LOAD( "tbp24s10n.4e", 0x0500, 0x100, CRC(28321dd8) SHA1(4ba0f6c381ef929a476d4d7aa71b1397c48a644e) )
ROM_END

ROM_START( omegaa ) // ROM at M10 labeled "7" to indicate the original version skipping some initialization code
	ROM_REGION(0xc000, "maincpu", 0) // encrypted
	ROM_LOAD( "7.m10",  0x0000, 0x4000, CRC(6e7d77e1) SHA1(7675cea41391595cd7a3e1893478185989f4c319) ) // 27128
	ROM_LOAD( "8.n10",  0x4000, 0x8000, CRC(9bb61910) SHA1(f8a1210dbf93e901e246e6adf4cd905acc3ef376) ) // 27256

	ROM_REGION(0x2000, "maincpu:key", 0) // MC8123 key
	ROM_LOAD( "omega.key", 0x0000, 0x2000, CRC(0a63943f) SHA1(9e581ea0c5bf6c0ed5d402d3bab053766b8e44c2) )

	ROM_REGION(0xc000, "tiles", 0)
	ROM_LOAD( "4.f10",  0x00000, 0x04000, CRC(bf780a8e) SHA1(53bfabf74f1a7782c6c1803498a24da0bf8db995) ) // 27128
	ROM_LOAD( "5.h10",  0x04000, 0x04000, CRC(b491647f) SHA1(88017033a781ecc49a83241bc49e2077a480ac2b) ) // 27128
	ROM_LOAD( "6.j10",  0x08000, 0x04000, CRC(65beba5b) SHA1(e6d61dc52dcbb30570b48d7b1d7807dd0be41400) ) // 27128

	ROM_REGION(0xc000, "sprites", 0)
	ROM_LOAD( "3.d10",  0x00000, 0x04000, CRC(c678b202) SHA1(ee93385e11158ccaf51a22d813bd7020c04cfdad) ) // 27128
	ROM_LOAD( "1.a10",  0x04000, 0x04000, CRC(e0aeada9) SHA1(ed00f6dca4f9701ff89390922d39341b179597c7) ) // 27128
	ROM_LOAD( "2.c10",  0x08000, 0x04000, CRC(dbc0a47f) SHA1(b617c5a10c655e7befaeaecd9ce736e972285e6b) ) // 27128

	ROM_REGION(0x600, "proms", 0)
	ROM_LOAD( "tbp24s10n.3f", 0x0000, 0x100, CRC(75ec7472) SHA1(868811e838c570a0f576a0ece249cab2d4274d65) ) // Or compatible type PROM like the 82S129
	ROM_LOAD( "tbp24s10n.4f", 0x0100, 0x100, CRC(5113a114) SHA1(3a5ab68c93d1f2c05ceb0311e12a54fd124d8435) )
	ROM_LOAD( "tbp24s10n.3g", 0x0200, 0x100, CRC(b6b5d4a0) SHA1(2b7ba59a6c185326e11ce8ccd96b3c8cfd652fdf) )
	ROM_LOAD( "tbp24s10n.4g", 0x0300, 0x100, CRC(931bc299) SHA1(f116f1d6a4324b86b0aae0a5a040236b3a4fd12d) )
	ROM_LOAD( "tbp24s10n.3e", 0x0400, 0x100, CRC(899e089d) SHA1(5a485d3ef7d2102451ff76452cac106061cc5cd6) )
	ROM_LOAD( "tbp24s10n.4e", 0x0500, 0x100, CRC(28321dd8) SHA1(4ba0f6c381ef929a476d4d7aa71b1397c48a644e) )
ROM_END


/*************************************
 *
 *  Game-specific driver inits
 *
 *************************************/

void gigas_state::init_gigasb()
{
	membank("bank0d")->set_base(memregion("maincpu")->base() + 0xc000);
	m_opbank->set_base(memregion("maincpu")->base() + 0x14000);
}

void pbillrd_state::init_pbillrd()
{
	m_rombank->configure_entries(0, 2, memregion("maincpu")->base() + 0x8000, 0x4000);
}

void pbillrd_state::init_pbillrds()
{
	init_pbillrd();

	m_decrypted_opcodes = std::make_unique<uint8_t[]>(0x10000);
	downcast<mc8123_device &>(*m_maincpu).decode(memregion("maincpu")->base(), m_decrypted_opcodes.get(), 0x10000);
	membank("bank0d")->set_base(m_decrypted_opcodes.get());
	m_opbank->configure_entries(0, 2, &m_decrypted_opcodes[0x8000], 0x4000);
}


void pbillrd_state::init_pbillrdbl()
{
	init_pbillrd();

	membank("bank0d")->set_base(memregion("maincpu")->base() + 0x10000);
	m_opbank->configure_entries(0, 2, memregion("maincpu")->base() + 0x18000, 0x4000);
}


void gigas_state::init_gigas()
{
	m_decrypted_opcodes = std::make_unique<uint8_t[]>(0xc000);
	downcast<mc8123_device &>(*m_maincpu).decode(memregion("maincpu")->base(), m_decrypted_opcodes.get(), 0xc000);
	membank("bank0d")->set_base(m_decrypted_opcodes.get());
	m_opbank->set_base(&m_decrypted_opcodes[0x8000]);
}

} // anonymous namespace


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/
//    YEAR  NAME        PARENT    MACHINE    INPUT     STATE           INIT            ROT     COMPANY                                  FULLNAME                                FLAGS
GAME( 1986, gigas,      0,        gigasm,    gigas,    gigas_state,    init_gigas,     ROT270, "Sega",                                  "Gigas (MC-8123, 317-5002)",            MACHINE_SUPPORTS_SAVE )
GAME( 1986, gigasb,     gigas,    gigas,     gigas,    gigas_state,    init_gigasb,    ROT270, "bootleg",                               "Gigas (bootleg)",                      MACHINE_SUPPORTS_SAVE )
GAME( 1986, oigas,      gigas ,   oigas,     gigas,    oigas_state,    init_gigasb,    ROT270, "bootleg",                               "Oigas (bootleg)",                      MACHINE_SUPPORTS_SAVE )
GAME( 1986, gigasm2,    0,        gigasm,    gigasm2,  gigas_state,    init_gigas,     ROT270, "Sega",                                  "Gigas Mark II (MC-8123, 317-5002)",    MACHINE_SUPPORTS_SAVE )
GAME( 1986, gigasm2b,   gigasm2,  gigas,     gigasm2,  gigas_state,    init_gigasb,    ROT270, "bootleg",                               "Gigas Mark II (bootleg)",              MACHINE_SUPPORTS_SAVE )
GAME( 1986, omega,      0,        omega,     omega,    gigas_state,    init_gigas,     ROT270, "Nihon System",                          "Omega",                                MACHINE_SUPPORTS_SAVE ) // Bug fix version
GAME( 1986, omegaa,     omega,    omega,     omega,    gigas_state,    init_gigas,     ROT270, "Nihon System",                          "Omega (earlier)",                      MACHINE_SUPPORTS_SAVE )
GAME( 1987, pbillrd,    0,        pbillrd,   pbillrd,  pbillrd_state,  init_pbillrd,   ROT0,   "Nihon System (United Artists license)", "Perfect Billiard",                     MACHINE_SUPPORTS_SAVE )
GAME( 1987, pbillrds,   pbillrd,  pbillrdm,  pbillrd,  pbillrd_state,  init_pbillrds,  ROT0,   "Nihon System (Sega license)",           "Perfect Billiard (MC-8123, 317-0030)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, pbillrdsa,  pbillrd,  pbillrdm,  pbillrd,  pbillrd_state,  init_pbillrds,  ROT0,   "Nihon System (Sega license)",           "Perfect Billiard (MC-8123, 317-5008)", MACHINE_SUPPORTS_SAVE ) // sticker on CPU module different (wrong?) functionality the same
GAME( 1987, pbillrdbl,  pbillrd,  pbillrdbl, pbillrd,  pbillrd_state,  init_pbillrdbl, ROT0,   "bootleg",                               "Perfect Billiard (bootleg)",           MACHINE_SUPPORTS_SAVE )
GAME( 1987, freekick,   0,        freekick,  freekick, freekick_state, empty_init,     ROT270, "Nihon System (Merit license)",          "Free Kick (NS6201-A 1987.10)",         MACHINE_SUPPORTS_SAVE )
GAME( 1987, freekicka,  freekick, freekick,  freekick, freekick_state, empty_init,     ROT270, "Nihon System",                          "Free Kick (NS6201-A 1987.9)",          MACHINE_SUPPORTS_SAVE )
GAME( 1987, freekickb1, freekick, freekick,  freekick, freekick_state, empty_init,     ROT270, "bootleg",                               "Free Kick (bootleg set 1)",            MACHINE_SUPPORTS_SAVE )
GAME( 1987, freekickb2, freekick, freekick,  freekick, freekick_state, empty_init,     ROT270, "bootleg",                               "Free Kick (bootleg set 2)",            MACHINE_SUPPORTS_SAVE )
GAME( 1987, freekickb3, freekick, freekick,  freekick, freekick_state, empty_init,     ROT270, "bootleg",                               "Free Kick (bootleg set 3)",            MACHINE_SUPPORTS_SAVE )
GAME( 1988, countrun,   0,        freekick,  countrun, freekick_state, empty_init,     ROT0,   "Nihon System (Sega license)",           "Counter Run (NS6201-A 1988.3)",        MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) // CPU module not dumped
GAME( 1988, countrunb,  countrun, freekick,  countrun, freekick_state, empty_init,     ROT0,   "bootleg",                               "Counter Run (bootleg set 1)",          MACHINE_SUPPORTS_SAVE )
GAME( 1988, countrunb2, countrun, freekick,  countrun, freekick_state, empty_init,     ROT0,   "bootleg",                               "Counter Run (bootleg set 2)",          MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 1988, countrunb3, countrun, freekick,  countrun, freekick_state, empty_init,     ROT0,   "bootleg",                               "Counter Run (bootleg set 3)",          MACHINE_SUPPORTS_SAVE )
