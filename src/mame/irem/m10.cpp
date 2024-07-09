// license:BSD-3-Clause
// copyright-holders: Lee Taylor, Couriersud

/***************************************************************************

    Irem M-10 / M-11 / M-15 hardware

****************************************************************************

  (c) 12/2/1998 Lee Taylor

Notes:
- Colors are close to screen shots for IPM Invader. The other games have not
  been verified.
- The bitmap strips in IPM Invader might be slightly misplaced

TODO:
- DIP switches
- andromed M-29S starfield


Head On
-------
Irem, 1979? / 1980?

PCB Layout
----------

    M-15L
   |---------------------------------------------------------------------------------|
   |                                                                                 |
   | DSW(8)  74175   74175   7400  74LS08   74121   M53214     |-------|      E1.9A  |
   |                                                           | 6502  |             |
   |          7432   74175   7404    7427    7442  74LS241     |-------|             |
   |                                                                          E2.9B  |
   |                                                                                 |
   |        74LS74    7432  74161   74161    7442  74LS241  74LS367  74LS367         |
 |-|                                                                          E3.9C  |
 |          M53214 74LS367   7442    7486    8216     2114  74LS157  74LS367         |
 |                                                                                   |
 |4         M53214 74LS367  74161    7486    8216     2114  74LS157     2111  E4.9D  |
 |4                                                                                  |
 |W         M53214 74LS367  74161    7486    8216     2114  74LS157     2111         |
 |A                                                                           E5.9F  |
 |Y         M53214 74LS367  74161    7486    8216    74166     2114  74LS157         |
 |                         11.73MHz                                                  |
 |            7400    7432  7404    74161    8216    74166     2114  74LS157  E6.9G  |
 |-|                                                                                 |
   |   VR3 VR2 VR1    7432  7404     7400  *74173     7400  74LS139  74LS157         |
   |                                       *74S04                                    |
   |                                                                                 |
   |---------------------------------------------------------------------------------|
Notes:
      All IC's are listed
      All ROMs type 2708 (1K x8)

      6502 clock: 733kHz
               *: These 2 IC's piggybacked. 74S04 on top
         VR1/2/3: 5K potentiometers
            2114: 1K x4 SRAM
            2111: 256bytes x4 SRAM
            8216: 256bytes x1 SRAM

Sound PCB
---------

M-15S
|---------------------------|
|                           |
|  NE555  NE555             |
|                           |
|  NE555  NE555             |
|               LM3900   VR1|
|                           |
|  c1815x9               VR2|
|                           |
|               LM3900   VR3|
|                           |
|                        VR4|
|                           |
|                        VR5|
|    TA7222                 |
|---------------------------|
Notes:
      PCB contains lots of resistors, capacitors, transistors etc.

      VR1/2/3/4/5: Potentiometers for volume of each sound
            C1815: Transistor (x9)
           TA7222: Power Amp

****************************************************************************

Notes (couriersud)

    From http://www.crazykong.com/tech/IremBoardList.txt

    ipminvad:       M-10L + M-10S (also exists on M-11 hw)
    andromed:       M-11L + M-11S + M-29S
    skychut:        M-11 (?)
    spacbeam:       not listed
    headon:         not listed
    greenber:       M-15T, M-24S

    M10-Board: Has SN76477

    ipminvad1
    ========

    This is from an incomplete dump without documentation.
    The filename contained m10 and with a hack to work
    around the missing ROM you get some action.

    The files are all different from ipminvad. Either this has
    been a prototype or possibly the famous "Capsule Invader".

***************************************************************************/

#include "emu.h"

#include "cpu/m6502/m6502.h"
#include "machine/74123.h"
#include "machine/rescap.h"
#include "sound/samples.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


// configurable logging
#define LOG_CTRL     (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_CTRL)

#include "logmacro.h"

#define LOGCTRL(...)     LOGMASKED(LOG_CTRL,     __VA_ARGS__)


namespace {

class m1x_state : public driver_device
{
public:
	m1x_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_chargen(*this, "chargen"),
		m_maincpu(*this, "maincpu"),
		m_samples(*this, "samples"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_cab(*this, "CAB")
	{ }

	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_chargen;

	required_device<cpu_device> m_maincpu;
	required_device<samples_device> m_samples;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	required_ioport m_cab;

	static constexpr XTAL IREMM10_MASTER_CLOCK = 12.5_MHz_XTAL;

	static constexpr XTAL IREMM10_CPU_CLOCK = IREMM10_MASTER_CLOCK / 16;
	static constexpr XTAL IREMM10_PIXEL_CLOCK = IREMM10_MASTER_CLOCK / 2;
	static constexpr int IREMM10_HTOTAL = 360; // (0x100-0xd3) * 8
	static constexpr int IREMM10_HBSTART = 248;
	static constexpr int IREMM10_HBEND = 8;
	static constexpr int IREMM10_VTOTAL = 281; // (0x200-0xe7)
	static constexpr int IREMM10_VBSTART = 240;
	static constexpr int IREMM10_VBEND = 16;

	static constexpr XTAL IREMM11_MASTER_CLOCK = 11.73_MHz_XTAL;

	static constexpr XTAL IREMM11_CPU_CLOCK = IREMM11_MASTER_CLOCK / 16;
	static constexpr XTAL IREMM11_PIXEL_CLOCK = IREMM11_MASTER_CLOCK / 2;
	static constexpr int IREMM11_HTOTAL = 372;
	static constexpr int IREMM11_HBSTART = 256;
	static constexpr int IREMM11_HBEND = 0;
	static constexpr int IREMM11_VTOTAL = 262;
	static constexpr int IREMM11_VBSTART = 240;
	static constexpr int IREMM11_VBEND = 16;

	// video-related
	tilemap_t *m_tx_tilemap;

	// video state
	uint8_t m_flip = 0;

	// misc
	uint8_t m_last = 0;

	void colorram_w(offs_t offset, uint8_t data);
	TILEMAP_MAPPER_MEMBER(tilemap_scan);
	TILE_GET_INFO_MEMBER(get_tile_info);
	void palette(palette_device &palette) const;
};

class m10_state : public m1x_state
{
public:
	m10_state(const machine_config &mconfig, device_type type, const char *tag) :
		m1x_state(mconfig, type, tag),
		m_ic8j1(*this, "ic8j1"),
		m_ic8j2(*this, "ic8j2")
	{ }

	void m10(machine_config &config);
	void m11(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(set_vr1) { m_ic8j2->set_resistor_value(RES_K(10 + newval / 5.0)); }

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	required_device<ttl74123_device> m_ic8j1;
	required_device<ttl74123_device> m_ic8j2;

	gfx_element *m_back_gfx = nullptr;
	uint8_t m_back_color[4]{};
	uint8_t m_back_xpos[4]{};
	uint8_t m_bottomline = 0;

	void m10_ctrl_w(uint8_t data);
	void m11_ctrl_w(uint8_t data);
	void m10_a500_w(uint8_t data);
	void m11_a100_w(uint8_t data);
	uint8_t clear_74123_r();
	void chargen_w(offs_t offset, uint8_t data);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	inline void plot_pixel(bitmap_ind16 &bm, int x, int y, int col);

	void m10_main(address_map &map);
	void m11_main(address_map &map);
};

class m15_state : public m1x_state
{
public:
	m15_state(const machine_config &mconfig, device_type type, const char *tag) :
		m1x_state(mconfig, type, tag)
	{ }

	void m15(machine_config &config);

protected:
	virtual void video_start() override;

private:
	void ctrl_w(uint8_t data);
	void a100_w(uint8_t data);
	void chargen_w(offs_t offset, uint8_t data);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(interrupt);

	void main(address_map &map);
};


static const uint32_t extyoffs[] =
{
	STEP256(0, 8)
};

static const gfx_layout backlayout =
{
	8,8*32, // 8*(8*32) characters
	4,      // 4 characters
	1,      // 1 bit per pixel
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	EXTENDED_YOFFS,
	32*8*8, // every char takes 8 consecutive bytes
	nullptr, extyoffs
};

static const gfx_layout charlayout =
{
	8,8,    // 8*8 characters
	256,    // 256 characters
	1,      // 1 bit per pixel
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 // every char takes 8 consecutive bytes
};

TILEMAP_MAPPER_MEMBER(m1x_state::tilemap_scan)
{
	return (31 - col) * 32 + row;
}


TILE_GET_INFO_MEMBER(m1x_state::get_tile_info)
{
	tileinfo.set(0, m_videoram[tile_index], m_colorram[tile_index] & 0x07, 0);
}


void m1x_state::colorram_w(offs_t offset, uint8_t data)
{
	if (m_colorram[offset] != data)
	{
		m_tx_tilemap->mark_tile_dirty(offset);
		m_colorram[offset] = data;
	}
}


void m10_state::chargen_w(offs_t offset, uint8_t data)
{
	if (m_chargen[offset] != data)
	{
		m_chargen[offset] = data;
		m_back_gfx->mark_dirty(offset >> (3 + 5));
	}
}


void m15_state::chargen_w(offs_t offset, uint8_t data)
{
	if (m_chargen[offset] != data)
	{
		m_chargen[offset] = data;
		m_gfxdecode->gfx(0)->mark_dirty(offset >> 3);
	}
}


inline void m10_state::plot_pixel(bitmap_ind16 &bm, int x, int y, int col)
{
	if (!m_flip)
		bm.pix(y, x) = col;
	else
		bm.pix((IREMM10_VBSTART - 1) - (y - IREMM10_VBEND),
				(IREMM10_HBSTART - 1) - (x - IREMM10_HBEND)) = col; // only when flip_screen(?)
}

void m10_state::video_start()
{
	m_tx_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(m10_state::get_tile_info)), tilemap_mapper_delegate(*this, FUNC(m10_state::tilemap_scan)), 8, 8, 32, 32);
	m_tx_tilemap->set_transparent_pen(0);

	m_gfxdecode->set_gfx(1, std::make_unique<gfx_element>(m_palette, backlayout, m_chargen, 0, 8, 0));
	m_back_gfx = m_gfxdecode->gfx(1);
}

void m15_state::video_start()
{
	m_gfxdecode->set_gfx(0,std::make_unique<gfx_element>(m_palette, charlayout, m_chargen, 0, 8, 0));

	m_tx_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(m15_state::get_tile_info)), tilemap_mapper_delegate(*this, FUNC(m15_state::tilemap_scan)), 8, 8, 32, 32);
}

/***************************************************************************

  Draw the game screen in the given bitmap_ind16.

***************************************************************************/

uint32_t m10_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	for (int i = 0; i < 4; i++)
		if (m_flip)
				m_back_gfx->opaque(bitmap,cliprect, i, m_back_color[i], 1, 1, 31 * 8 - m_back_xpos[i], 0);
		else
				m_back_gfx->opaque(bitmap,cliprect, i, m_back_color[i], 0, 0, m_back_xpos[i], 0);

	if (m_bottomline)
	{
		for (int y = IREMM10_VBEND; y < IREMM10_VBSTART; y++)
			plot_pixel(bitmap, 16, y, 1);
	}

	for (int offs = m_videoram.bytes() - 1; offs >= 0; offs--)
		m_tx_tilemap->mark_tile_dirty(offs);

	m_tx_tilemap->set_flip(m_flip ? TILEMAP_FLIPX | TILEMAP_FLIPY : 0);
	m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}


/***************************************************************************

  Draw the game screen in the given bitmap_ind16.

***************************************************************************/

uint32_t m15_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int offs = m_videoram.bytes() - 1; offs >= 0; offs--)
		m_tx_tilemap->mark_tile_dirty(offs);

	//m_tx_tilemap->mark_all_dirty();
	m_tx_tilemap->set_flip(m_flip ? TILEMAP_FLIPX | TILEMAP_FLIPY : 0);
	m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}


/*************************************
 *
 *  Initialization
 *
 *************************************/

void m1x_state::palette(palette_device &palette) const
{
	for (int i = 0; i < 0x10; i++)
	{
		rgb_t const color = BIT(i, 0) ? rgb_t(pal1bit(~i >> 3), pal1bit(~i >> 2), pal1bit(~i >> 1)) : rgb_t::black();
		palette.set_pen_color(i, color);
	}
}

void m1x_state::machine_start()
{
	save_item(NAME(m_flip));
	save_item(NAME(m_last));
}

void m10_state::machine_start()
{
	m1x_state::machine_start();
	save_item(NAME(m_bottomline));
}

void m1x_state::machine_reset()
{
	m_flip = 0;
	m_last = 0;
}

void m10_state::machine_reset()
{
	m1x_state::machine_reset();
	m_bottomline = 0;
}


/*************************************
 *
 *  I/O handling
 *
 *************************************/

/*
 * M10 Ctrl Port
 *
 * 76543210
 * ========
 * e-------     ACTIVE LOW  Demo mode
 * -?------     ????
 * --b-----     ACTIVE LOW  Bottom line
 * ---f----     ACTIVE LOW  Flip screen
 * ----u---     ACTIVE LOW  UFO sound enable (SN76477)
 * -----sss     Sound #sss start
 *              0x01: MISSILE
 *              0x02: EXPLOSION
 *              0x03: INVADER HIT
 *              0x04: BONUS BASE
 *              0x05: FLEET MOVE
 *              0x06: SAUCER HIT
 */

void m10_state::m10_ctrl_w(uint8_t data)
{
	if (data & 0x40)
		LOGCTRL("ctrl: %02x",data);

	// I have NO IDEA if this is correct or not
	m_bottomline = ~data & 0x20;

	if (m_cab->read() & 0x01)
		m_flip = ~data & 0x10;

	if (!(m_cab->read() & 0x02))
		machine().sound().system_mute(data & 0x80);

	// sound command in lower 4 bytes
	switch (data & 0x07)
	{
		case 0x00:
			// no sound mapped
			break;
		case 0x01:
			// MISSILE sound
			m_samples->start(0, 0);
			break;
		case 0x02:
			// EXPLOSION sound
			m_samples->start(1, 1);
			break;
		case 0x03:
			// INVADER HIT sound
			m_samples->start(2, 2);
			break;
		case 0x04:
			// BONUS BASE sound
			m_samples->start(3, 8);
			break;
		case 0x05:
			// FLEET MOVE sound
			m_samples->start(3, 3);
			break;
		case 0x06:
			// SAUCER HIT SOUND
			m_samples->start(2, 7);
			break;
		default:
			logerror("Unknown sound M10: %02x\n", data & 0x07);
			break;
	}
	// UFO SOUND
	if (data & 0x08)
		m_samples->stop(4);
	else
		m_samples->start(4, 9, true);

}

/*
 * M11 Ctrl Port
 *
 * 76543210
 * ========
 * e-------     ACTIVE LOW  Demo mode
 * -?------     ????
 * --b-----     ACTIVE LOW  Bottom line
 * ---f----     ACTIVE LOW  Flip screen
 * ----??--     ????
 * ------cc     Credits indicator ?
 *              0x03: 0 Credits
 *              0x02: 1 Credit
 *              0x00: 2 or more credits
 *              Will be updated only in attract mode
 */

void m10_state::m11_ctrl_w(uint8_t data)
{
	if (data & 0x4c)
		LOGCTRL("M11 ctrl: %02x",data);

	m_bottomline = ~data & 0x20;

	if (m_cab->read() & 0x01)
		m_flip = ~data & 0x10;

	if (!(m_cab->read() & 0x02))
		machine().sound().system_mute(data & 0x80);
}

/*
 * M15 Ctrl Port
 *
 * 76543210
 * ========
 * ????----     ????
 * ----e---     ACTIVE LOW  Demo mode
 * -----f--     ACTIVE LOW  Flip screen
 * ------cc     Credits indicator ?
 *              0x03: 0 Credits
 *              0x02: 1 Credit
 *              0x00: 2 or more credits
 *              Will be updated only in attract mode
 */

void m15_state::ctrl_w(uint8_t data)
{
	if (data & 0xf0)
		LOGCTRL("M15 ctrl: %02x",data);

	if (m_cab->read() & 0x01)
		m_flip = ~data & 0x04;
	if (!(m_cab->read() & 0x02))
		machine().sound().system_mute(data & 0x08);
}


/*
 * M10 A500
 *
 * 76543210
 * ========
 * ??????--     Always 111111
 * ------cc     Credits indicator ?
 *              0x03: 0 Credits
 *              0x02: 1 Credit
 *              0x00: 2 or more credits
 *              Will be updated only in attract mode
 */

void m10_state::m10_a500_w(uint8_t data)
{
	if (data & 0xfc)
		LOGCTRL("a500: %02x",data);
}

void m10_state::m11_a100_w(uint8_t data)
{
	int const raising_bits = data & ~m_last;
	//int const falling_bits = ~data & m_last;

	// should a falling bit stop a sample?
	// This port is written to about 20x per vblank

	if ((m_last & 0xe8) != (data & 0xe8))
		LOGCTRL("A100: %02x\n", data);

	m_last = data;

	// audio control!
	// MISSILE sound
	if (raising_bits & 0x01)
		m_samples->start(0, 0);

	// EXPLOSION sound
	if (raising_bits & 0x02)
		m_samples->start(1, 1);

	// Rapidly falling parachute
	if (raising_bits & 0x04)
		m_samples->start(3, 8);

	// Background sound ?
	if (data & 0x10)
		m_samples->start(4, 9, true);
	else
		m_samples->stop(4);

}

void m15_state::a100_w(uint8_t data)
{
	//int const raising_bits = data & ~m_last;
	int const falling_bits = ~data & m_last;

	// should a falling bit stop a sample?
	// Bit 4 is used
	// Bit 5 is used 0xef
	// Bit 7 is used

	// headoni
	// 0x01: Acceleration
	// 0x04: background (motor) ?
	// 0x08: explosion
	// 0x10: player changes lane
	// 0x20: computer car changes lane
	// 0x40: dot

	if ((m_last & 0x82) != (data & 0x82))
		LOGCTRL("A100: %02x\n", data);

	// DOT sound
	if (falling_bits & 0x40)
		m_samples->start(0, 0);
#if 0
	if (raising_bits & 0x40)
		m_samples->stop(0);
#endif

	// EXPLOSION sound
	if (falling_bits & 0x08)
		m_samples->start(1, 1);
#if 0
	if (raising_bits & 0x08)
		m_samples->stop(1);
#endif

	// player changes lane
	if (falling_bits & 0x10)
		m_samples->start(3, 3);
#if 0
	if (raising_bits & 0x10)
		m_samples->stop(3);
#endif

	// computer car changes lane
	if (falling_bits & 0x20)
		m_samples->start(4, 4);
#if 0
	if (raising_bits & 0x20)
		m_samples->stop(4);
#endif

	m_last = data;
}


/*************************************
 *
 *  Interrupt handling
 *
 *************************************/

uint8_t m10_state::clear_74123_r()
{
	if (!machine().side_effects_disabled())
	{
		m_ic8j1->clear_w(0);
		m_ic8j1->clear_w(1);
	}
	return 0;
}

INPUT_CHANGED_MEMBER(m1x_state::coin_inserted)
{
	// coin insertion causes an NMI
	m_maincpu->set_input_line(INPUT_LINE_NMI, newval ? ASSERT_LINE : CLEAR_LINE);
}

INTERRUPT_GEN_MEMBER(m15_state::interrupt)
{
	device.execute().pulse_input_line(0, m_screen->time_until_pos(IREMM11_VBSTART + 1, 80));
}


/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

void m10_state::m10_main(address_map &map)
{
	map(0x0000, 0x02ff).ram(); // scratch ram
	map(0x1000, 0x2fff).rom();
	map(0x4000, 0x43ff).ram().share(m_videoram);
	map(0x4800, 0x4bff).ram().w(FUNC(m10_state::colorram_w)).share(m_colorram); // foreground colour
	map(0x5000, 0x53ff).ram().w(FUNC(m10_state::chargen_w)).share(m_chargen); // background ?????
	map(0xa200, 0xa200).portr("DSW");
	map(0xa300, 0xa300).portr("INPUTS");
	map(0xa400, 0xa400).w(FUNC(m10_state::m10_ctrl_w));   // line at bottom of screen?, sound, flip screen
	map(0xa500, 0xa500).w(FUNC(m10_state::m10_a500_w));   // ???
	map(0xa700, 0xa700).r(FUNC(m10_state::clear_74123_r));
	map(0xfc00, 0xffff).rom(); // for the reset / interrupt vectors
}

void m10_state::m11_main(address_map &map)
{
	map(0x0000, 0x02ff).ram(); // scratch ram
	map(0x1000, 0x2fff).rom();
	map(0x4000, 0x43ff).ram().share(m_videoram);
	map(0x4800, 0x4bff).ram().w(FUNC(m10_state::colorram_w)).share(m_colorram); // foreground colour
	map(0x5000, 0x53ff).ram().w(FUNC(m10_state::chargen_w)).share(m_chargen); // background ?????
	map(0xa100, 0xa100).w(FUNC(m10_state::m11_a100_w)); // sound writes ????
	map(0xa200, 0xa200).portr("DSW");
	map(0xa300, 0xa300).portr("INPUTS");
	map(0xa400, 0xa400).w(FUNC(m10_state::m11_ctrl_w));   // line at bottom of screen?, sound, flip screen
	map(0xa700, 0xa700).r(FUNC(m10_state::clear_74123_r));
	map(0xfc00, 0xffff).rom(); // for the reset / interrupt vectors
}

void m15_state::main(address_map &map)
{
	map(0x0000, 0x02ff).ram(); // scratch ram
	map(0x1000, 0x33ff).rom();
	map(0x4000, 0x43ff).ram().share(m_videoram);
	map(0x4800, 0x4bff).ram().w(FUNC(m15_state::colorram_w)).share(m_colorram); // foreground colour
	map(0x5000, 0x57ff).ram().w(FUNC(m15_state::chargen_w)).share(m_chargen); // background ?????
	map(0xa000, 0xa000).portr("P2");
	map(0xa100, 0xa100).w(FUNC(m15_state::a100_w)); // sound writes ????
	map(0xa200, 0xa200).portr("DSW");
	map(0xa300, 0xa300).portr("P1");
	map(0xa400, 0xa400).w(FUNC(m15_state::ctrl_w));   // sound, flip screen
	map(0xfc00, 0xffff).rom(); // for the reset / interrupt vectors
}


/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( cabinet )
	PORT_START("CAB") // fake port for cabinet type
	PORT_CONFNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_CONFSETTING(    0x00, DEF_STR( Upright ) )
	PORT_CONFSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_CONFNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x02, DEF_STR( On ) )
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( ipminvad )
	PORT_INCLUDE( cabinet )

	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING (   0x00, "3" )
	PORT_DIPSETTING (   0x01, "4" )
	PORT_DIPSETTING (   0x02, "5" )
	PORT_DIPSETTING (   0x03, "6" )
	PORT_DIPNAME( 0x04, 0x00, "Capsules" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "1500" )
	PORT_DIPSETTING(    0x08, "1000" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPUNUSED( 0x20, 0x00 ) // Verified with debugger
	PORT_DIPUNUSED( 0x40, 0x00 )
	PORT_DIPUNUSED( 0x80, 0x00 )

	PORT_START("FAKE")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, m10_state, coin_inserted, 0)

	PORT_START("VR1") // VR1 20K variable resistor on main PCB
	PORT_ADJUSTER(30, "IRQ Frequency") PORT_CHANGED_MEMBER(DEVICE_SELF, m10_state, set_vr1, 0)
INPUT_PORTS_END

static INPUT_PORTS_START( skychut )
	PORT_INCLUDE( ipminvad )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING (   0x00, "3" )
	PORT_DIPSETTING (   0x01, "4" )
	PORT_DIPSETTING (   0x03, "4 (duplicate)" )
	PORT_DIPSETTING (   0x02, "5" )
	PORT_DIPUNKNOWN( 0x04, 0x00 )
	PORT_DIPUNKNOWN( 0x08, 0x00 )
	PORT_DIPUNKNOWN( 0x10, 0x00 )
	PORT_DIPUNKNOWN( 0x20, 0x00 )
	PORT_DIPUNKNOWN( 0x40, 0x00 )
	PORT_DIPUNKNOWN( 0x80, 0x00 )
INPUT_PORTS_END

static INPUT_PORTS_START( andromed )
	PORT_INCLUDE( ipminvad )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING (   0x00, "3" )
	PORT_DIPSETTING (   0x01, "4" )
	PORT_DIPSETTING (   0x02, "5" )
	PORT_DIPSETTING (   0x03, "6" )
	PORT_DIPUNKNOWN( 0x04, 0x00 )
	PORT_DIPUNKNOWN( 0x08, 0x00 )
	PORT_DIPNAME( 0x30, 0x10, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPUNKNOWN( 0x40, 0x00 )
	PORT_DIPUNKNOWN( 0x80, 0x00 )
INPUT_PORTS_END

static INPUT_PORTS_START( spacbeam )
	PORT_INCLUDE( cabinet )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )

	PORT_START("P2")
	PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL

	PORT_START("DSW")
	PORT_DIPNAME(0x03, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING (  0x00, "2" )
	PORT_DIPSETTING (  0x01, "3" )
	PORT_DIPSETTING (  0x02, "4" )
	PORT_DIPSETTING (  0x03, "5" )
	PORT_DIPNAME(0x08, 0x00, "Replay" )
	PORT_DIPSETTING (  0x00, "30000" )
	PORT_DIPSETTING (  0x08, DEF_STR( None ) )
	PORT_DIPNAME(0x30, 0x10, DEF_STR( Coinage ) )
	PORT_DIPSETTING (  0x10, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING (  0x20, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING (  0x00, DEF_STR( Free_Play ) )

	PORT_START("FAKE")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, m10_state, coin_inserted, 0)
INPUT_PORTS_END

static INPUT_PORTS_START( headoni )
	PORT_INCLUDE( cabinet )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME(0x03, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING (  0x00, "2" )
	PORT_DIPSETTING (  0x01, "3" )
	PORT_DIPSETTING (  0x02, "4" )
	PORT_DIPSETTING (  0x03, "5" )
	PORT_DIPNAME(0x08, 0x00, "Replay" )
	PORT_DIPSETTING (  0x00, "30000" )
	PORT_DIPSETTING (  0x08, DEF_STR( None ) )
	PORT_DIPNAME(0x30, 0x10, DEF_STR( Coinage ) )
	PORT_DIPSETTING (  0x10, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING (  0x20, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING (  0x00, DEF_STR( Free_Play ) )

//  PORT_START("VR1")
//  PORT_ADJUSTER( 50, "Car Rumble Volume" )

//  PORT_START("VR2")
//  PORT_ADJUSTER( 50, "Collision Volume" )

//  PORT_START("VR3")
//  PORT_ADJUSTER( 50, "Tire Screech Volume" )

//  PORT_START("VR4")
//  PORT_ADJUSTER( 50, "Score Counter Volume" )

//  PORT_START("VR5")
//  PORT_ADJUSTER( 50, "Master Volume" )

	PORT_START("FAKE")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, m10_state, coin_inserted, 0)
INPUT_PORTS_END

static INPUT_PORTS_START( greenber )
	PORT_INCLUDE( cabinet )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )

	PORT_START("P2")
	PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL

	PORT_START("DSW")
	PORT_DIPNAME(0x03, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING (  0x03, "2" )
	PORT_DIPSETTING (  0x02, "3 (duplicate)" )
	PORT_DIPSETTING (  0x01, "3" )
	PORT_DIPSETTING (  0x00, "4" )
	PORT_DIPNAME(0x08, 0x00, "Replay" )
	PORT_DIPSETTING (  0x00, "5000" )
	PORT_DIPSETTING (  0x08, "7000" )
	PORT_DIPNAME(0x30, 0x20, DEF_STR( Coinage ) )
	PORT_DIPSETTING (  0x20, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING (  0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING (  0x00, DEF_STR( Free_Play ) )

	PORT_START("FAKE")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, m10_state, coin_inserted, 0)
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/


static GFXDECODE_START( gfx_m10 )
	GFXDECODE_ENTRY( "tiles", 0x0000, charlayout, 0, 8 )
GFXDECODE_END

/*************************************
 *
 *  Sound definitions
 *
 *************************************/

static const char *const m10_sample_names[] =
{
	"*ipminvad",
	"1",        // shot/missile
	"2",        // base hit/explosion
	"3",        // invader hit
	"4",        // fleet move 1
	"5",        // fleet move 2
	"6",        // fleet move 3
	"7",        // fleet move 4
	"8",        // UFO/saucer hit
	"9",        // bonus base
	"0",        // UFO sound
	nullptr
};


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void m10_state::m10(machine_config &config)
{
	// basic machine hardware
	M6502(config, m_maincpu, IREMM10_CPU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &m10_state::m10_main);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(IREMM10_PIXEL_CLOCK, IREMM10_HTOTAL, IREMM10_HBEND, IREMM10_HBSTART, IREMM10_VTOTAL, IREMM10_VBEND, IREMM10_VBSTART);
	m_screen->set_screen_update(FUNC(m10_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_m10);
	PALETTE(config, m_palette, FUNC(m10_state::palette), 2 * 8);

	TTL74123(config, m_ic8j1, 0); // completely illegible
	m_ic8j1->set_connection_type(TTL74123_NOT_GROUNDED_DIODE);  // the hook up type
	m_ic8j1->set_resistor_value(RES_K(1));                      // resistor connected to RCext
	m_ic8j1->set_capacitor_value(CAP_U(1));                     // capacitor connected to Cext and RCext
	m_ic8j1->set_a_pin_value(1);                                // A pin - driven by #2 /Q
	m_ic8j1->set_b_pin_value(1);                                // B pin - pulled high
	m_ic8j1->set_clear_pin_value(1);                            // Clear pin - pulled high
	m_ic8j1->out_cb().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	TTL74123(config, m_ic8j2, 0);
	m_ic8j2->set_connection_type(TTL74123_NOT_GROUNDED_DIODE);  // the hook up type
	// 10k + 20k variable resistor
	m_ic8j2->set_resistor_value(RES_K(10 + 6));                 // resistor connected to RCext
	m_ic8j2->set_capacitor_value(CAP_U(2.2));                   // capacitor connected to Cext and RCext
	m_ic8j2->set_a_pin_value(1);                                // A pin - driven by #2 /Q
	m_ic8j2->set_b_pin_value(1);                                // B pin - pulled high
	m_ic8j2->set_clear_pin_value(1);                            // Clear pin - pulled high
	m_ic8j2->out_cb().set(m_ic8j2, FUNC(ttl74123_device::a_w));
	m_ic8j2->out_cb().append(m_ic8j1, FUNC(ttl74123_device::a_w));

	// sound hardware
	SPEAKER(config, "mono").front_center();

	SAMPLES(config, m_samples);
	m_samples->set_channels(6);
	m_samples->set_samples_names(m10_sample_names);
	m_samples->add_route(ALL_OUTPUTS, "mono", 1.0);

	m_back_color[0] = 3; m_back_color[1] = 3; m_back_color[2] = 5; m_back_color[3] = 5;
	m_back_xpos[0] = 4 * 8; m_back_xpos[1] = 26 * 8; m_back_xpos[2] = 7 * 8; m_back_xpos[3] = 6 * 8;
}

void m10_state::m11(machine_config &config)
{
	m10(config);

	m_maincpu->set_clock(IREMM11_CPU_CLOCK);
	m_screen->set_raw(IREMM11_PIXEL_CLOCK, IREMM11_HTOTAL, IREMM11_HBEND, IREMM11_HBSTART, IREMM11_VTOTAL, IREMM11_VBEND, IREMM11_VBSTART);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &m10_state::m11_main);

	m_back_color[0] = 3; m_back_color[1] = 0; m_back_color[2] = 3; m_back_color[3] = 3;
	m_back_xpos[0] = 4 * 8; m_back_xpos[1] = 26 * 8; m_back_xpos[2] = 2 * 8; m_back_xpos[3] = 3 * 8;
}

void m15_state::m15(machine_config &config)
{
	// basic machine hardware
	M6502(config, m_maincpu, IREMM11_CPU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &m15_state::main);
	m_maincpu->set_vblank_int("screen", FUNC(m15_state::interrupt));

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(IREMM11_PIXEL_CLOCK, IREMM11_HTOTAL, IREMM11_HBEND, IREMM11_HBSTART, IREMM11_VTOTAL, IREMM11_VBEND, IREMM11_VBSTART);
	m_screen->set_screen_update(FUNC(m15_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfxdecode_device::empty);
	PALETTE(config, m_palette, FUNC(m15_state::palette), 2 * 8);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	SAMPLES(config, m_samples);
	m_samples->set_channels(6);
	m_samples->set_samples_names(m10_sample_names);
	m_samples->add_route(ALL_OUTPUTS, "mono", 1.0);
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( ipminvad )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "b1r",  0x1000, 0x0400, CRC(f9a7eb9b) SHA1(93ac65d3ac725d3e4c2fb769816ee808ab609911) )
	ROM_LOAD( "b2r",  0x1400, 0x0400, CRC(af11c1aa) SHA1(6a74fcc7cb1627b1c427a77da89b69ccf3175800) )
	ROM_LOAD( "b3r",  0x1800, 0x0400, CRC(ed49e481) SHA1(8771a34f432e6d88acc5f7529f16c980a77485db) )
	ROM_LOAD( "b4r",  0x1c00, 0x0400, CRC(6d5db95b) SHA1(135500fc17524e8608c3bcfe26321144aa0afb91) )
	ROM_RELOAD(       0xfc00, 0x0400 ) // for the reset and interrupt vectors
	ROM_LOAD( "b5r",  0x2000, 0x0400, CRC(eabba7aa) SHA1(75e47eacd429f48f0a3a4539e5ecb4b1ea7281b1) )
	ROM_LOAD( "b6r",  0x2400, 0x0400, CRC(3d0e7fa6) SHA1(14903bfc9506cb8e37807fb397be79f5eab99e3b) )
	ROM_LOAD( "b7r",  0x2800, 0x0400, CRC(cf04864f) SHA1(6fe3ce208334321b63ada779fed69ec7cf4051ad) )

	ROM_REGION( 0x0800, "tiles", 0 )
	ROM_LOAD( "b9r",  0x0000, 0x0400, CRC(56942cab) SHA1(ba13a856477fc6cf7fd36996e47a3724f862f888) )
	ROM_LOAD( "b10r", 0x0400, 0x0400, CRC(be4b8585) SHA1(0154eae62585e154cf20edcf4599bda8bd333aa9) )
ROM_END

ROM_START( ipminvad1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "b1g",  0x1000, 0x0400, CRC(069102e2) SHA1(90affe384a688b0d42154633e80b708371117fc2) )
	ROM_LOAD( "b2f",  0x1400, 0x0400, CRC(a6aa5879) SHA1(959ab207110785c03e57ca69c0e62356dd974085) )
	ROM_LOAD( "b3f",  0x1800, 0x0400, CRC(0c09feb9) SHA1(0db43f480162f8e3fb8b61fcceb2884d19ff115b) )
	ROM_LOAD( "b4f",  0x1c00, 0x0400, CRC(a4d32207) SHA1(ea9a01d09d82b8c27701601f03989735558d975c) )
	ROM_RELOAD(       0xfc00, 0x0400 ) // for the reset and interrupt vectors
	ROM_LOAD( "b5f",  0x2000, 0x0400, CRC(192361c7) SHA1(b13e80429a9183ce78c4df52a32070416d4ec988) )
	ROM_LOAD( "b6f",  0x2400, 0x0400, NO_DUMP )
	ROM_FILL(         0x2400, 0x0400, 0x60)
	ROM_LOAD( "b7f",  0x2800, 0x0400, CRC(0f5115ab) SHA1(3bdd3fc1cfe6bfacb5820ee12c15f2909d2f58d1) )

	ROM_REGION( 0x0800, "tiles", 0 )
	ROM_LOAD( "b9",  0x0000, 0x0400, CRC(f6cfa53c) SHA1(ec1076982edee95efb24a1bb08e733bcccacb922) )
	ROM_LOAD( "b10", 0x0400, 0x0400, CRC(63672cd2) SHA1(3d9fa15509a363e1a32e58a2242b266b1162e9a6) )
ROM_END

ROM_START( andromed )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "am1",  0x1000, 0x0400, CRC(53df0152) SHA1(d27113740094d219b0e05a930d8daa4c22129183) )
	ROM_LOAD( "am2",  0x1400, 0x0400, CRC(dab64957) SHA1(77ced520f8e78bb08ddab4213646cf55d834e63e) )
	ROM_LOAD( "am3",  0x1800, 0x0400, CRC(f983f35c) SHA1(1bfee6cf7d18b56594831f2efa7dcc53b47d7e30) )
	ROM_LOAD( "am4",  0x1c00, 0x0400, CRC(09f20717) SHA1(c54c9b7d16b40a7ab49eac255906b43b03939d2b) )
	ROM_RELOAD(       0xfc00, 0x0400 ) // for the reset and interrupt vectors
	ROM_LOAD( "am5",  0x2000, 0x0400, CRC(518a3b88) SHA1(5e20c905c2190b381a105327e112fcc0a127bb2f) )
	ROM_LOAD( "am6",  0x2400, 0x0400, CRC(ce3d5fff) SHA1(c34178aca9ffb8b2dd468d9e3369a985f52daf9a) )
	ROM_LOAD( "am7",  0x2800, 0x0400, CRC(30d3366f) SHA1(aa73bba194fa6d1f3909f8df517a0bff07583ea9) )
	ROM_LOAD( "am8",  0x2c00, 0x0400, CRC(57294dff) SHA1(3ef8d561e33434dce6e7d45e4739ca3b333681a8) )

	ROM_REGION( 0x0800, "tiles", 0 )
	ROM_LOAD( "am9",  0x0000, 0x0400, CRC(a1c8f4db) SHA1(bedf5d7126c7e9b91ad595188c69aa2c043c71e8) )
	ROM_LOAD( "am10", 0x0400, 0x0400, CRC(be2de8f3) SHA1(7eb3d1eb88b4481b0dcb7d001207f516a5db32b3) )
ROM_END

ROM_START( skychut )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sc1d",  0x1000, 0x0400, CRC(30b5ded1) SHA1(3a8b4fa344522404661b062808a2ea1d5858fdd0) )
	ROM_LOAD( "sc2d",  0x1400, 0x0400, CRC(fd1f4b9e) SHA1(e5606979abe1fa4cc9eae0c4f61516769db35c39) )
	ROM_LOAD( "sc3d",  0x1800, 0x0400, CRC(67ed201e) SHA1(589b1efdc1bbccff296f6420e2b320cd54b4ac8e) )
	ROM_LOAD( "sc4d",  0x1c00, 0x0400, CRC(9b23a679) SHA1(a101f9b0fdde927a43e53e9b7d0dffb9dcca9e16) )
	ROM_RELOAD(        0xfc00, 0x0400 ) // for the reset and interrupt vectors
	ROM_LOAD( "sc5a",  0x2000, 0x0400, CRC(51d975e6) SHA1(7d345025ef28c8a81f599cde445eeb336c368fce) )
	ROM_LOAD( "sc6e",  0x2400, 0x0400, CRC(617f302f) SHA1(4277ef97279eb63fc68b6c40f8545b31abaab474) )
	ROM_LOAD( "sc7",   0x2800, 0x0400, CRC(dd4c8e1a) SHA1(b5a141d8ac256ba6522308e5f194bfaf5c75fa5b) )
	ROM_LOAD( "sc8d",  0x2c00, 0x0400, CRC(aca8b798) SHA1(d9048d060314d8f20ab1967fee846d35c22ac693) )

	ROM_REGION( 0x0800, "tiles", 0 )
	ROM_LOAD( "sc9d",  0x0000, 0x0400, CRC(2101029e) SHA1(34cddf076d3d860aa03043db14837f42449aefe7) )
	ROM_LOAD( "sc10d", 0x0400, 0x0400, CRC(2f81c70c) SHA1(504935c89a4158a067cbf1dcdb27f7421678915d) )
ROM_END

ROM_START( spacbeam )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "m1b", 0x1000, 0x0400, CRC(5a1c3e0b) SHA1(1c9c58359d74b14ce96934fcc6acefbdfaf1e1be) )
	ROM_LOAD( "m2b", 0x1400, 0x0400, CRC(a02bd9d7) SHA1(d25dfa66b422bdbb29b1922007c84f1947fe9be1) )
	ROM_LOAD( "m3b", 0x1800, 0x0400, CRC(78040843) SHA1(0b8a3ab09dff951aa527649f82b8877cf01126c1) )
	ROM_LOAD( "m4b", 0x1c00, 0x0400, CRC(74705a44) SHA1(8fa9d22a58f08086bf2d89e3d92eca097cdd2cbf) )
	ROM_RELOAD(      0xfc00, 0x0400 ) // for the reset and interrupt vectors
	ROM_LOAD( "m5b", 0x2000, 0x0400, CRC(afdf1242) SHA1(e26a8e91edb3d8ba96b3d225813760f42238b003) )
	ROM_LOAD( "m6b", 0x2400, 0x0400, CRC(12afb0c2) SHA1(bf6ed90cf4815f0fb41d435954d4c346a55098f5) )
ROM_END

ROM_START( headoni )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "e1.9a", 0x1000, 0x0400, CRC(05da5265) SHA1(17e0c9261978770325a0befdcdd8a1b07ed39df0) )
	ROM_LOAD( "e2.9b", 0x1400, 0x0400, CRC(dada26a8) SHA1(1368ade1c0c57d33d15594370cf1edf95fc44fd1) )
	ROM_LOAD( "e3.9c", 0x1800, 0x0400, CRC(61ff24f5) SHA1(0e68aedd01b765fb2af76f914b3d287ecf30f716) )
	ROM_LOAD( "e4.9d", 0x1c00, 0x0400, CRC(ce4c5a67) SHA1(8db493d43f311a29127405aad7693bc08b570b14) )
	ROM_RELOAD(        0xfc00, 0x0400 ) // for the reset and interrupt vectors
	ROM_LOAD( "e5.9f", 0x2000, 0x0400, CRC(b5232439) SHA1(39b8fb4bbd00a73b9a2b68bc3e88fb45d3f62d7c) )
	ROM_LOAD( "e6.9g", 0x2400, 0x0400, CRC(99acd1a6) SHA1(799382c1b079aad3034a1cc738dc06954978a0ac) )
ROM_END

ROM_START( greenber )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gb1", 0x1000, 0x0400, CRC(018ff672) SHA1(54d082a332831afc28b47704a5656da0a8a902fa) )
	ROM_LOAD( "gb2", 0x1400, 0x0400, CRC(ea8f2267) SHA1(ad5bb38a80fbc7c70c8fa6f41086a7ade81655bc) )
	ROM_LOAD( "gb3", 0x1800, 0x0400, CRC(8f337920) SHA1(ac3d76eb368645ba23f5823b39c04fae49d481e1) )
	ROM_LOAD( "gb4", 0x1c00, 0x0400, CRC(7eeac4eb) SHA1(c668ad45ebc4aca558371539031efc4ec3990e44) )
	ROM_RELOAD(      0xfc00, 0x0400 ) // for the reset and interrupt vectors
	ROM_LOAD( "gb5", 0x2000, 0x0400, CRC(b2f8e69a) SHA1(44295e58da890a8c4aba6fe90defe9c578c95592) )
	ROM_LOAD( "gb6", 0x2400, 0x0400, CRC(50ea8bd3) SHA1(a816c5fcc603b28c2ae59f217871a7e85fb794e1) )
	ROM_LOAD( "gb7", 0x2800, 0x0400, CRC(695124aa) SHA1(0715b1ebc5e08f91ebff7fb6c2a9dca457a7c13c) )
	ROM_LOAD( "gb8", 0x2c00, 0x0400, CRC(34700b31) SHA1(c148e2475eaaa0e9d1e2412eea359a7ba744e563) )
	ROM_LOAD( "gb9", 0x3000, 0x0400, CRC(c27b9ba3) SHA1(a2f4f0c4b61eb03bba13ae5d25dc01009a4f86ee) )
ROM_END

} // anonymous namespace


//    YEAR  NAME       PARENT    MACHINE INPUT     CLASS      INIT        ROT     COMPANY FULLNAME                         FLAGS
GAME( 1979, ipminvad,  0,        m10,    ipminvad, m10_state, empty_init, ROT270, "IPM",  "IPM Invader (set 1)",           MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_COLORS | MACHINE_SUPPORTS_SAVE )
GAME( 1979, ipminvad1, ipminvad, m10,    ipminvad, m10_state, empty_init, ROT270, "IPM",  "IPM Invader (set 2)",           MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_COLORS | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) // incomplete dump

GAME( 1980, andromed,  0,        m11,    andromed, m10_state, empty_init, ROT270, "Irem", "Andromeda SS (Japan?)",         MACHINE_NO_SOUND | MACHINE_IMPERFECT_COLORS | MACHINE_SUPPORTS_SAVE ) // export version known as simply "Andromeda"
GAME( 1980, skychut,   0,        m11,    skychut,  m10_state, empty_init, ROT270, "Irem", "Sky Chuter",                    MACHINE_NO_SOUND | MACHINE_IMPERFECT_COLORS | MACHINE_SUPPORTS_SAVE )

GAME( 1979, headoni,   0,        m15,    headoni,  m15_state, empty_init, ROT270, "Irem", "Head On (Irem, M-15 Hardware)", MACHINE_NO_SOUND | MACHINE_IMPERFECT_COLORS | MACHINE_SUPPORTS_SAVE )
GAME( 1979, spacbeam,  0,        m15,    spacbeam, m15_state, empty_init, ROT270, "Irem", "Space Beam",                    MACHINE_NO_SOUND | MACHINE_IMPERFECT_COLORS | MACHINE_SUPPORTS_SAVE ) // IPM or Irem?
GAME( 1980, greenber,  0,        m15,    greenber, m15_state, empty_init, ROT270, "Irem", "Green Beret (Irem)",            MACHINE_NO_SOUND | MACHINE_IMPERFECT_COLORS | MACHINE_SUPPORTS_SAVE )
