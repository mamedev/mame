// license:BSD-3-Clause
// copyright-holders: Tomasz Slanina, Pierpaolo Prazzoli

/****************************************************
   Pit&Run - Taito 1984

 driver by  Tomasz Slanina and  Pierpaolo Prazzoli

 hardware is very similar to suprridr.cpp, thepit.cpp, timelimt.cpp

TODO:

 - analog sound
   writes to $a8xx triggering analog sound :
    $a800 - drivers are getting into the cars
    $a801 - collisions
    $a802 - same as above
    $a803 - slide on water
    $a804 - accelerate
    $a807 - analog sound reset


-----------------------------------------------------
$8101 B - course
$8102 B - trial
$8492 B - fuel
$84f6 B - lap
$84c1 W - time
-----------------------------------------------------

N4200374A

K1000232A
            A11_17     2128  PR9
           (68705P5)         PR10
                             PR11
     SW1                     PR12
                        Z80
                                      clr.1
                        PR8           clr.2

               PR6                    clr.3
               PR7
                              2114
                              2114

K1000231A

    2114 2114
    PR13
                Z80

          8910 8910
   5MHz

K1000233A

  2125      2125        2128
  2125      2125        2128
  2125      2125        PR4
  2125      2125        PR5
  2125      2125

                             2114
     PR1                     2114
     PR2
     PR3
*/

#include "emu.h"

#include "cpu/m6805/m68705.h"
#include "cpu/z80/z80.h"
#include "machine/74157.h"
#include "machine/74259.h"
#include "machine/gen_latch.h"
#include "machine/watchdog.h"
#include "sound/ay8910.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class pitnrun_state : public driver_device
{
public:
	pitnrun_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_watchdog(*this, "watchdog"),
		m_inputmux(*this, "inputmux"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram%u", 1U),
		m_spriteram(*this, "spriteram"),
		m_spotrom(*this, "spot")
	{ }

	void pitnrun(machine_config &config);

	void tilt_w(int state); // TODO: privatize eventually

protected:
	virtual void machine_start() override;
	virtual void video_start() override;

	uint8_t inputs_r();

	required_device<cpu_device> m_maincpu;

	void base_map(address_map &map);

private:
	required_device<watchdog_timer_device> m_watchdog;
	required_device<ls157_x2_device> m_inputmux;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr_array<uint8_t, 2> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;

	required_region_ptr<uint8_t> m_spotrom;

	uint8_t m_nmi = 0;
	uint8_t m_h_heed = 0;
	uint8_t m_v_heed = 0;
	uint8_t m_ha = 0;
	uint16_t m_scroll = 0;
	uint8_t m_char_bank = 0;
	uint8_t m_color_select = 0;
	std::unique_ptr<bitmap_ind16> m_tmp_bitmap[4];
	tilemap_t *m_bg = nullptr;
	tilemap_t *m_fg = nullptr;

	void nmi_enable_w(int state);
	uint8_t inputs_watchdog_r();
	template <uint8_t Which> void videoram_w(offs_t offset, uint8_t data);
	void char_bank_select_w(int state);
	void scroll_w(offs_t offset, uint8_t data);
	void scroll_y_w(uint8_t data);
	void ha_w(uint8_t data);
	void h_heed_w(uint8_t data);
	void v_heed_w(uint8_t data);
	void color_select_w(int state);

	TILE_GET_INFO_MEMBER(get_tile_info_fg);
	TILE_GET_INFO_MEMBER(get_tile_info_bg);

	void vbl_w(int state);

	void main_map(address_map &map);

	void palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void spotlights();
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void sound_io_map(address_map &map);
	void sound_prg_map(address_map &map);
};

class pitnrun_mcu_state : public pitnrun_state
{
public:
	pitnrun_mcu_state(const machine_config &mconfig, device_type type, const char *tag) :
		pitnrun_state(mconfig, type, tag),
		m_mcu(*this, "mcu")
	{ }

	void pitnrun_mcu(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	required_device<m68705p5_device> m_mcu;
	uint8_t m_fromz80 = 0;
	uint8_t m_toz80 = 0;
	uint8_t m_zaccept = 0;
	uint8_t m_zready = 0;
	uint8_t m_porta_in = 0;
	uint8_t m_porta_out = 0;
	uint16_t m_address = 0;

	uint8_t mcu_data_r();
	void mcu_data_w(uint8_t data);
	uint8_t mcu_status_r();
	uint8_t m68705_porta_r();
	void m68705_porta_w(uint8_t data);
	uint8_t m68705_portb_r();
	void m68705_portb_w(uint8_t data);
	uint8_t m68705_portc_r();

	TIMER_CALLBACK_MEMBER(mcu_real_data_r);
	TIMER_CALLBACK_MEMBER(mcu_real_data_w);
	TIMER_CALLBACK_MEMBER(mcu_data_real_r);
	TIMER_CALLBACK_MEMBER(mcu_status_real_w);

	void mcu_map(address_map &map);
};


/***************************************************************************

  - BG layer 32x128 , 8x8 tiles 4bpp , 2 palettes  (2nd is black )
  - TXT layer 32x32 , 8x8 tiles 4bpp , 2 palettes (2nd is black)
  - Sprites 16x16 3bpp, 8 palettes (0-3 are black)

  'Special' effects :

  - spotlight - gfx(BG+Sprites) outside spotlight is using black pals
                spotlight masks are taken from ROM pr8
                simulated using bitmaps and custom clipping rect

  - lightning - BG color change (darkening ?) - simple analog circ.
                            simulated by additional palette

In debug build press 'w' for spotlight and 'e' for lightning

***************************************************************************/


TILE_GET_INFO_MEMBER(pitnrun_state::get_tile_info_fg)
{
	int const code = m_videoram[0][tile_index];
	tileinfo.set(0,
		code,
		0,
		0);
}

TILE_GET_INFO_MEMBER(pitnrun_state::get_tile_info_bg)
{
	int const code = m_videoram[1][tile_index];
	tileinfo.set(1,
		code + (m_char_bank << 8),
		m_color_select,
		0);
}

template <uint8_t Which>
void pitnrun_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[Which][offset] = data;
	Which ? m_bg->mark_all_dirty() : m_fg->mark_all_dirty();
}

void pitnrun_state::char_bank_select_w(int state)
{
	m_char_bank = state;
	m_bg->mark_all_dirty();
}


void pitnrun_state::scroll_w(offs_t offset, uint8_t data)
{
	m_scroll = (m_scroll & (0xff << ((offset) ? 0 : 8))) |( data << ((offset) ? 8 : 0));
	m_bg->set_scrollx(0, m_scroll);
}

void pitnrun_state::scroll_y_w(uint8_t data)
{
	m_bg->set_scrolly(0, data);
}

void pitnrun_state::ha_w(uint8_t data)
{
	m_ha = data;
}

void pitnrun_state::h_heed_w(uint8_t data)
{
	m_h_heed = data;
}

void pitnrun_state::v_heed_w(uint8_t data)
{
	m_v_heed = data;
}

void pitnrun_state::color_select_w(int state)
{
	m_color_select = state;
	machine().tilemap().mark_all_dirty();
}

void pitnrun_state::spotlights()
{
	for (int i = 0; i < 4; i++)
	{
		for (int y = 0; y < 128; y++)
		{
			for (int x = 0; x < 16; x++)
			{
				int datapix = m_spotrom[128 * 16 * i + x + y * 16];
				for (int b = 0; b < 8; b++)
				{
					m_tmp_bitmap[i]->pix(y, x * 8 + (7 - b)) = (datapix & 1);
					datapix >>= 1;
				}
			}
		}
	}
}


void pitnrun_state::palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();

	for (int i = 0; i < 32*3; i++)
	{
		int bit0, bit1, bit2;

		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		int const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		bit0 = BIT(color_prom[i], 3);
		bit1 = BIT(color_prom[i], 4);
		bit2 = BIT(color_prom[i], 5);
		int const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		bit0 = 0;
		bit1 = BIT(color_prom[i], 6);
		bit2 = BIT(color_prom[i], 7);
		int const b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}

	// fake bg palette for lightning effect
	for (int i = 2 * 16; i < 2 * 16 + 16; i++)
	{
		int bit0, bit1, bit2;

		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		int r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		bit0 = BIT(color_prom[i], 3);
		bit1 = BIT(color_prom[i], 4);
		bit2 = BIT(color_prom[i], 5);
		int g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		bit0 = 0;
		bit1 = BIT(color_prom[i], 6);
		bit2 = BIT(color_prom[i], 7);
		int b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		r /= 3;
		g /= 3;
		b /= 3;

		palette.set_pen_color(i + 16, (r > 0xff) ? 0xff : r, (g > 0xff) ? 0xff : g, (b > 0xff) ? 0xff : b);

	}
}

void pitnrun_state::video_start()
{
	m_fg = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(pitnrun_state::get_tile_info_fg)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bg = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(pitnrun_state::get_tile_info_bg)), TILEMAP_SCAN_ROWS, 8, 8, 32 * 4, 32);
	m_fg->set_transparent_pen(0 );
	m_tmp_bitmap[0] = std::make_unique<bitmap_ind16>(128, 128);
	m_tmp_bitmap[1] = std::make_unique<bitmap_ind16>(128, 128);
	m_tmp_bitmap[2] = std::make_unique<bitmap_ind16>(128, 128);
	m_tmp_bitmap[3] = std::make_unique<bitmap_ind16>(128, 128);
	spotlights();

	save_item(NAME(m_h_heed));
	save_item(NAME(m_v_heed));
	save_item(NAME(m_ha));
	save_item(NAME(m_scroll));
	save_item(NAME(m_char_bank));
	save_item(NAME(m_color_select));
}

void pitnrun_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int offs = 0; offs < 0x100; offs += 4)
	{
		int const pal = m_spriteram[offs + 2] & 0x3;

		int sy = 256 - m_spriteram[offs + 0] - 16;
		int sx = m_spriteram[offs + 3] + 1; // +1 needed to properly align Jump Kun
		int flipy = (m_spriteram[offs + 1] & 0x80) >> 7;
		int flipx = (m_spriteram[offs + 1] & 0x40) >> 6;

		if (flip_screen_x())
		{
			sx = 242 - sx;
			flipx = !flipx;
		}
		if (flip_screen_y())
		{
			sy = 240 - sy;
			flipy = !flipy;
		}

		m_gfxdecode->gfx(2)->transpen(bitmap, cliprect,
			(m_spriteram[offs + 1] & 0x3f) + ((m_spriteram[offs + 2] & 0x80) >> 1)+ ((m_spriteram[offs + 2] & 0x40) << 1),
			pal,
			flipx, flipy,
			sx, sy, 0);
	}
}

uint32_t pitnrun_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int dx = 0, dy = 0;
	rectangle myclip = cliprect;

#ifdef MAME_DEBUG
	if (machine().input().code_pressed_once(KEYCODE_Q))
	{
		uint8_t *ROM = memregion("maincpu")->base();
		ROM[0x84f6] = 0; // lap 0 - normal
	}

	if (machine().input().code_pressed_once(KEYCODE_W))
	{
		uint8_t *ROM = memregion("maincpu")->base();
		ROM[0x84f6] = 6; // lap 6 = spotlight
	}

	if (machine().input().code_pressed_once(KEYCODE_E))
	{
		uint8_t *ROM = memregion("maincpu")->base();
		ROM[0x84f6] = 2; // lap 3 (trial 2)= lightnings
		ROM[0x8102] = 1;
	}
#endif

	bitmap.fill(0, cliprect);

	if (!(m_ha & 4))
		m_bg->draw(screen, bitmap, cliprect, 0, 0);
	else
	{
		dx = 128 - m_h_heed + ((m_ha & 8) << 5) + 3;
		dy = 128 - m_v_heed + ((m_ha & 0x10) << 4);

		if (flip_screen_x())
			dx = 128 - dx + 16;

		if (flip_screen_y())
			dy = 128 - dy;

		myclip.set(dx, dx + 127, dy, dy + 127);
		myclip &= cliprect;

		m_bg->draw(screen, bitmap, myclip, 0, 0);
	}

	draw_sprites(bitmap, myclip);

	if (m_ha & 4)
		copybitmap_trans(bitmap, *m_tmp_bitmap[m_ha & 3], flip_screen_x(), flip_screen_y(), dx, dy, myclip, 1);
	m_fg->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


// MCU

/***************************************************************************

    Based on TaitsoSJ driver
    68705 has access to Z80 memory space

***************************************************************************/

TIMER_CALLBACK_MEMBER(pitnrun_mcu_state::mcu_real_data_r)
{
	m_zaccept = 1;
}

uint8_t pitnrun_mcu_state::mcu_data_r()
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(pitnrun_mcu_state::mcu_real_data_r), this));
	return m_toz80;
}

TIMER_CALLBACK_MEMBER(pitnrun_mcu_state::mcu_real_data_w)
{
	m_zready = 1;
	m_mcu->set_input_line(0, ASSERT_LINE);
	m_fromz80 = param;
}

void pitnrun_mcu_state::mcu_data_w(uint8_t data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(pitnrun_mcu_state::mcu_real_data_w), this), data);
	machine().scheduler().perfect_quantum(attotime::from_usec(5));
}

uint8_t pitnrun_mcu_state::mcu_status_r()
{
	/* MCU synchronization
	   bit 0 = the 68705 has read data from the Z80
	   bit 1 = the 68705 has written data for the Z80 */
	return ~((m_zready << 1) | (m_zaccept << 0));
}


uint8_t pitnrun_mcu_state::m68705_porta_r()
{
	return m_porta_in;
}

void pitnrun_mcu_state::m68705_porta_w(uint8_t data)
{
	m_porta_out = data;
}



/*
 *  Port B connections:
 *
 *  all bits are logical 1 when read (+5V pullup)
 *
 *  0   W  !68INTRQ
 *  1   W  !68LRD (enables latch which holds command from the Z80)
 *  2   W  !68LWR (loads the latch which holds data for the Z80, and sets a
 *                 status bit so the Z80 knows there's data waiting)
 *  3   W  to Z80 !BUSRQ (aka !WAIT) pin
 *  4   W  !68WRITE (triggers write to main Z80 memory area )
 *  5   W  !68READ (triggers read from main Z80 memory area )
 *  6   W  !LAL (loads the latch which holds the low 8 bits of the address of
 *               the main Z80 memory location to access)
 *  7   W  !UAL (loads the latch which holds the high 8 bits of the address of
 *               the main Z80 memory location to access)
 */

uint8_t pitnrun_mcu_state::m68705_portb_r()
{
	return 0xff;
}


TIMER_CALLBACK_MEMBER(pitnrun_mcu_state::mcu_data_real_r)
{
	m_zready = 0;
}

TIMER_CALLBACK_MEMBER(pitnrun_mcu_state::mcu_status_real_w)
{
	m_toz80 = param;
	m_zaccept = 0;
}

void pitnrun_mcu_state::m68705_portb_w(uint8_t data)
{
	address_space &cpu0space = m_maincpu->space(AS_PROGRAM);
	if (~data & 0x02)
	{
		// 68705 is going to read data from the Z80
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(pitnrun_mcu_state::mcu_data_real_r), this));
		m_mcu->set_input_line(0, CLEAR_LINE);
		m_porta_in = m_fromz80;
	}
	if (~data & 0x04)
	{
		// 68705 is writing data for the Z80
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(pitnrun_mcu_state::mcu_status_real_w), this), m_porta_out);
	}
	if (~data & 0x10)
	{
		cpu0space.write_byte(m_address, m_porta_out);
	}
	if (~data & 0x20)
	{
		m_porta_in = cpu0space.read_byte(m_address);
	}
	if (~data & 0x40)
	{
		m_address = (m_address & 0xff00) | m_porta_out;
	}
	if (~data & 0x80)
	{
		m_address = (m_address & 0x00ff) | (m_porta_out << 8);
	}
}

/*
 *  Port C connections:
 *
 *  0   R  ZREADY (1 when the Z80 has written a command in the latch)
 *  1   R  ZACCEPT (1 when the Z80 has read data from the latch)
 *  2   R  from Z80 !BUSAK pin
 *  3   R  68INTAK (goes 0 when the interrupt request done with 68INTRQ
 *                  passes through)
 */

uint8_t pitnrun_mcu_state::m68705_portc_r()
{
	return (m_zready << 0) | (m_zaccept << 1);
}


void pitnrun_state::machine_start()
{
	save_item(NAME(m_nmi));
}

void pitnrun_mcu_state::machine_start()
{
	pitnrun_state::machine_start();

	save_item(NAME(m_fromz80));
	save_item(NAME(m_toz80));
	save_item(NAME(m_zaccept));
	save_item(NAME(m_zready));
	save_item(NAME(m_porta_in));
	save_item(NAME(m_porta_out));
	save_item(NAME(m_address));
}

void pitnrun_mcu_state::machine_reset()
{
	m_zaccept = 1;
	m_zready = 0;
	m_mcu->set_input_line(0, CLEAR_LINE);
}

void pitnrun_state::tilt_w(int state)
{
	// HACK: this input actually asserts the master reset line on all devices
	if (state)
		reset();
}

void pitnrun_state::vbl_w(int state)
{
	if (state && m_nmi)
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

void pitnrun_state::nmi_enable_w(int state)
{
	m_nmi = state;
	if (!m_nmi)
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

uint8_t pitnrun_state::inputs_r()
{
	return ~m_inputmux->output_r();
}

uint8_t pitnrun_state::inputs_watchdog_r()
{
	if (!machine().side_effects_disabled())
		m_watchdog->reset_w();
	return ~m_inputmux->output_r();
}

void pitnrun_state::base_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0x8800, 0x8fff).ram().w(FUNC(pitnrun_state::videoram_w<0>)).share(m_videoram[0]);
	map(0x9000, 0x9fff).ram().w(FUNC(pitnrun_state::videoram_w<1>)).share(m_videoram[1]);
	map(0xa000, 0xa0ff).ram().share(m_spriteram);
	map(0xa800, 0xa800).portr("SYSTEM");
	map(0xa800, 0xa807).w("noiselatch", FUNC(ls259_device::write_d0)); // analog sound
	map(0xb000, 0xb000).portr("DSW");
	map(0xb000, 0xb007).w("mainlatch", FUNC(ls259_device::write_d0));
	map(0xb800, 0xb800).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0xc800, 0xc801).w(FUNC(pitnrun_state::scroll_w));
	map(0xc802, 0xc802).w(FUNC(pitnrun_state::scroll_y_w));
	map(0xc805, 0xc805).w(FUNC(pitnrun_state::h_heed_w));
	map(0xc806, 0xc806).w(FUNC(pitnrun_state::v_heed_w));
	map(0xc807, 0xc807).w(FUNC(pitnrun_state::ha_w));
}

void pitnrun_state::main_map(address_map &map)
{
	base_map(map);
	map(0xb800, 0xb800).r(FUNC(pitnrun_state::inputs_watchdog_r));
}

void pitnrun_mcu_state::mcu_map(address_map &map)
{
	base_map(map);
	map(0xb800, 0xb800).r(FUNC(pitnrun_mcu_state::inputs_r));
	map(0xc804, 0xc804).w(FUNC(pitnrun_mcu_state::mcu_data_w));
	map(0xd000, 0xd000).r(FUNC(pitnrun_mcu_state::mcu_data_r));
	map(0xd800, 0xd800).r(FUNC(pitnrun_mcu_state::mcu_status_r));
	map(0xf000, 0xf000).r("watchdog", FUNC(watchdog_timer_device::reset_r));
}

void pitnrun_state::sound_prg_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x3800, 0x3bff).ram();
}

void pitnrun_state::sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w("soundlatch", FUNC(generic_latch_8_device::clear_w));
	map(0x8c, 0x8d).w("ay2", FUNC(ay8910_device::address_data_w));
	map(0x8e, 0x8f).w("ay1", FUNC(ay8910_device::address_data_w));
	map(0x8f, 0x8f).r("ay1", FUNC(ay8910_device::data_r));
	map(0x90, 0x96).nopw();
	map(0x97, 0x97).nopw();
	map(0x98, 0x98).nopw();
}


static INPUT_PORTS_START( pitnrun )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1  )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("COCKTAIL")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DSW")
	PORT_DIPNAME( 0x07, 0x01, DEF_STR( Coinage ) )  PORT_DIPLOCATION("DSW:1,2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_7C ) )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x00, "DSW:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x00, "DSW:5" )
	PORT_DIPNAME( 0x20, 0x00, "Gasoline Count" )    PORT_DIPLOCATION("DSW:6")
	PORT_DIPSETTING(    0x00, "10 Up or 10 Down" )
	PORT_DIPSETTING(    0x20, "20 Up or 20 Down" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )  PORT_DIPLOCATION("DSW:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x00, "No Hit (Cheat)")     PORT_DIPLOCATION("DSW:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )       // also enables bootup test

	PORT_START("TILT")
	PORT_BIT( 1, IP_ACTIVE_HIGH, IPT_TILT ) PORT_WRITE_LINE_MEMBER(pitnrun_state, tilt_w)
INPUT_PORTS_END

static INPUT_PORTS_START( jumpkun )
	PORT_INCLUDE(pitnrun)

	PORT_MODIFY("INPUTS")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )

	PORT_MODIFY("COCKTAIL")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Lives ) ) PORT_DIPLOCATION("DSW:5")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )  PORT_DIPLOCATION("DSW:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x00, "Invincibility (Cheat)") PORT_DIPLOCATION("DSW:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END


static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{ 0,RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
		8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },
	32*8
};

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{RGN_FRAC(1,2),RGN_FRAC(1,2)+4,0,4},
	{ STEP4(0,1), STEP4(8,1) },
	{ STEP8(0,8*2) },
	8*8*2
};

static GFXDECODE_START( gfx_pitnrun )
	GFXDECODE_ENTRY( "fgtiles", 0, charlayout,   64, 2 )
	GFXDECODE_ENTRY( "bgtiles", 0, charlayout,   32, 2 )
	GFXDECODE_ENTRY( "sprites", 0, spritelayout,  0, 4 )
GFXDECODE_END

void pitnrun_state::pitnrun(machine_config &config)
{
	Z80(config, m_maincpu, 18.432_MHz_XTAL / 6); // verified on PCB
	m_maincpu->set_addrmap(AS_PROGRAM, &pitnrun_state::main_map);

	ls259_device &mainlatch(LS259(config, "mainlatch")); // 7B (mislabeled LS156 on schematic)
	mainlatch.q_out_cb<0>().set(FUNC(pitnrun_state::nmi_enable_w)); // NMION
	mainlatch.q_out_cb<1>().set(FUNC(pitnrun_state::color_select_w));
	mainlatch.q_out_cb<4>().set_nop(); // COLOR SEL 2 - not used ?
	mainlatch.q_out_cb<5>().set(FUNC(pitnrun_state::char_bank_select_w));
	mainlatch.q_out_cb<6>().set(FUNC(pitnrun_state::flip_screen_x_set)); // HFLIP
	mainlatch.q_out_cb<6>().append(m_inputmux, FUNC(ls157_x2_device::select_w));
	mainlatch.q_out_cb<7>().set(FUNC(pitnrun_state::flip_screen_y_set)); // VFLIP

	LS157_X2(config, m_inputmux); // 2F (0-3) & 2H (4-7)
	m_inputmux->a_in_callback().set_ioport("INPUTS");
	m_inputmux->b_in_callback().set_ioport("COCKTAIL");

	z80_device &audiocpu(Z80(config, "audiocpu", 5_MHz_XTAL / 2)); // verified on PCB
	audiocpu.set_addrmap(AS_PROGRAM, &pitnrun_state::sound_prg_map);
	audiocpu.set_addrmap(AS_IO, &pitnrun_state::sound_io_map);
	audiocpu.set_vblank_int("screen", FUNC(pitnrun_state::irq0_line_hold));

	WATCHDOG_TIMER(config, m_watchdog).set_vblank_count("screen", 16); // LS393 at 8N (+ misc. gates)

	config.set_maximum_quantum(attotime::from_hz(6000));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(18.432_MHz_XTAL / 3, 384, 0, 256, 264, 16, 240);
	screen.set_screen_update(FUNC(pitnrun_state::screen_update));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(FUNC(pitnrun_state::vbl_w));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_pitnrun);
	PALETTE(config, m_palette, FUNC(pitnrun_state::palette), 32 * 3);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	ay8910_device &ay1(AY8910(config, "ay1", XTAL(18'432'000) / 12));    // verified on PCB
	ay1.port_a_read_callback().set("soundlatch", FUNC(generic_latch_8_device::read));
	ay1.port_b_read_callback().set("soundlatch", FUNC(generic_latch_8_device::read));
	ay1.add_route(ALL_OUTPUTS, "mono", 0.50);

	ay8910_device &ay2(AY8910(config, "ay2", XTAL(18'432'000) / 12));    // verified on PCB
	ay2.port_a_read_callback().set("soundlatch", FUNC(generic_latch_8_device::read));
	ay2.port_b_read_callback().set("soundlatch", FUNC(generic_latch_8_device::read));
	ay2.add_route(ALL_OUTPUTS, "mono", 0.50);

	LS259(config, "noiselatch"); // 1J
}

void pitnrun_mcu_state::pitnrun_mcu(machine_config &config)
{
	pitnrun(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &pitnrun_mcu_state::mcu_map);

	M68705P5(config, m_mcu, 18.432_MHz_XTAL / 6); // verified on PCB
	m_mcu->porta_r().set(FUNC(pitnrun_mcu_state::m68705_porta_r));
	m_mcu->portb_r().set(FUNC(pitnrun_mcu_state::m68705_portb_r));
	m_mcu->portc_r().set(FUNC(pitnrun_mcu_state::m68705_portc_r));
	m_mcu->porta_w().set(FUNC(pitnrun_mcu_state::m68705_porta_w));
	m_mcu->portb_w().set(FUNC(pitnrun_mcu_state::m68705_portb_w));
}

ROM_START( pitnrun )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "pr_12-1.5d", 0x0000, 0x2000, CRC(2539aec3) SHA1(5ee87cf2379a6b6218f0c1f79374edafe5413616) )
	ROM_LOAD( "pr_11-1.5c", 0x2000, 0x2000, CRC(818a49f8) SHA1(0a4c77055529967595984277f11dc1cd1eec4dae) )
	ROM_LOAD( "pr_10-1.5b", 0x4000, 0x2000, CRC(69b3a864) SHA1(3d29e1f71f1a94650839696c3070d5739360bee0) )
	ROM_LOAD( "pr_9-1.5a",  0x6000, 0x2000, CRC(ba0c4093) SHA1(0273e4bd09b9eebff490fdac27e6ae9b54bb3cd9) )

	ROM_REGION( 0x2000, "audiocpu", 0 )
	ROM_LOAD( "pr-13", 0x0000, 0x1000, CRC(32a18d3b) SHA1(fcff1c13183b64ede0865dd04eee5182029bebdf) )

	ROM_REGION( 0x0800, "mcu", 0 ) // M68705P5 internal ROM
	ROM_LOAD( "a11_17.3a", 0x0000, 0x0800, CRC(e7d5d6e1) SHA1(c1131d6fcc36926e287be26090a3c89f22feaa35) )

	ROM_REGION( 0x06000, "sprites", 0 )
	ROM_LOAD( "pr-1.1k", 0x0000, 0x2000, CRC(c3b3131e) SHA1(ed0463e7eef452d7fbdcb031f9477825e9780943) )
	ROM_LOAD( "pr-2.1m", 0x2000, 0x2000, CRC(2fa1682a) SHA1(9daefb525fd69f0d9a45ff27e89865545e177a5a) )
	ROM_LOAD( "pr-3.1n", 0x4000, 0x2000, CRC(e678fe39) SHA1(134e36fd30bf3cf5884732f3455ca4d9dab6b665) )

	ROM_REGION( 0x4000, "bgtiles", 0 )
	ROM_LOAD( "pr-4.6d", 0x0000, 0x2000, CRC(fbae3504) SHA1(ce799dfd653462c0814e7530f3f8a686ab0ad7f4) )
	ROM_LOAD( "pr-5.6f", 0x2000, 0x2000, CRC(c9177180) SHA1(98c8f8f586b78b88dba254bd662642ee27f9b131) )

	ROM_REGION( 0x2000, "fgtiles", 0 )
	ROM_LOAD( "pr-6.3m", 0x0000, 0x1000, CRC(c53cb897) SHA1(81a73e6031b52fa45ec507ff4264b14474ef42a2) )
	ROM_LOAD( "pr-7.3p", 0x1000, 0x1000, CRC(7cdf9a55) SHA1(404dface7e09186e486945981e39063929599efc) )

	ROM_REGION( 0x2000, "spot", 0 )
	ROM_LOAD( "pr-8.4j", 0x0000, 0x2000, CRC(8e346d10) SHA1(1362ce4362c2d28c48fbd8a33da0cec5ef8e321f) )

	ROM_REGION( 0x0060, "proms", 0 )
	ROM_LOAD( "clr.1",  0x0000, 0x0020, CRC(643012f4) SHA1(4a0c9766b9da456e39ce379ad62d695bf82413b0) )
	ROM_LOAD( "clr.2",  0x0020, 0x0020, CRC(50705f02) SHA1(a3d348678fd66f37c7a0d29af88f40740918b8d3) )
	ROM_LOAD( "clr.3",  0x0040, 0x0020, CRC(25e70e5e) SHA1(fdb9c69e9568a725dd0e3ac25835270fb4f49280) )
ROM_END

ROM_START( pitnruna )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "pr_12.5d", 0x0000, 0x2000, CRC(587a7b85) SHA1(f200ff9b706e13760a23e0187c6bffe496af0087) )
	ROM_LOAD( "pr_11.5c", 0x2000, 0x2000, CRC(270cd6dd) SHA1(ad42562e18aa30319fc55c201e5507e8734a5b4d) )
	ROM_LOAD( "pr_10.5b", 0x4000, 0x2000, CRC(65d92d89) SHA1(4030ccdb4d84e69c256e95431ee5a18cffeae5c0) )
	ROM_LOAD( "pr_9.5a",  0x6000, 0x2000, CRC(3155286d) SHA1(45af8cb81d70f2e30b52bbc7abd9f8d15231735f) )

	ROM_REGION( 0x2000, "audiocpu", 0 )
	ROM_LOAD( "pr13", 0x0000, 0x1000, CRC(fc8fd05c) SHA1(f40cc9c6fff6bda8411f4d638a0f5c5915aa3746) )

	ROM_REGION( 0x0800, "mcu", 0 ) // M68705P5 internal ROM
	ROM_LOAD( "a11_17.3a", 0x0000, 0x0800, CRC(e7d5d6e1) SHA1(c1131d6fcc36926e287be26090a3c89f22feaa35) )

	ROM_REGION( 0x6000, "sprites", 0 )
	ROM_LOAD( "pr-1.1k", 0x0000, 0x2000, CRC(c3b3131e) SHA1(ed0463e7eef452d7fbdcb031f9477825e9780943) )
	ROM_LOAD( "pr-2.1m", 0x2000, 0x2000, CRC(2fa1682a) SHA1(9daefb525fd69f0d9a45ff27e89865545e177a5a) )
	ROM_LOAD( "pr-3.1n", 0x4000, 0x2000, CRC(e678fe39) SHA1(134e36fd30bf3cf5884732f3455ca4d9dab6b665) )

	ROM_REGION( 0x4000, "bgtiles", 0 )
	ROM_LOAD( "pr-4.6d", 0x0000, 0x2000, CRC(fbae3504) SHA1(ce799dfd653462c0814e7530f3f8a686ab0ad7f4) )
	ROM_LOAD( "pr-5.6f", 0x2000, 0x2000, CRC(c9177180) SHA1(98c8f8f586b78b88dba254bd662642ee27f9b131) )

	ROM_REGION( 0x2000, "fgtiles", 0 )
	ROM_LOAD( "pr-6.3m", 0x0000, 0x1000, CRC(c53cb897) SHA1(81a73e6031b52fa45ec507ff4264b14474ef42a2) )
	ROM_LOAD( "pr-7.3p", 0x1000, 0x1000, CRC(7cdf9a55) SHA1(404dface7e09186e486945981e39063929599efc) )

	ROM_REGION( 0x2000, "spot", 0 )
	ROM_LOAD( "pr-8.4j", 0x0000, 0x2000, CRC(8e346d10) SHA1(1362ce4362c2d28c48fbd8a33da0cec5ef8e321f) )

	ROM_REGION( 0x0060, "proms", 0 )
	ROM_LOAD( "clr.1",  0x0000, 0x0020, CRC(643012f4) SHA1(4a0c9766b9da456e39ce379ad62d695bf82413b0) )
	ROM_LOAD( "clr.2",  0x0020, 0x0020, CRC(50705f02) SHA1(a3d348678fd66f37c7a0d29af88f40740918b8d3) )
	ROM_LOAD( "clr.3",  0x0040, 0x0020, CRC(25e70e5e) SHA1(fdb9c69e9568a725dd0e3ac25835270fb4f49280) )
ROM_END

ROM_START( pitnrunb ) // all labels handwritten
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "test.a1", 0x0000, 0x2000, CRC(395b5514) SHA1(af5c8eb4b99a0bdcd3565e121a407febd560a2a5) )
	ROM_LOAD( "test.a2", 0x2000, 0x2000, CRC(09ffb063) SHA1(cf0aaf938366122a1c4f1b8f38f92c322ae9cb48) )
	ROM_LOAD( "test.a3", 0x4000, 0x2000, CRC(4f96e346) SHA1(e33c82fce30f769fb4e706f937c6c5344065cba6) )
	ROM_LOAD( "test.a4", 0x6000, 0x2000, CRC(3d04ef80) SHA1(17c966eb1256813302e1bce86fb6bd860138ef88) )

	ROM_REGION( 0x2000, "audiocpu", 0 )
	ROM_LOAD( "sound", 0x0000, 0x1000, CRC(fbd63042) SHA1(a473b42b76599a37434772ea1aa113397a842c1f) )

	ROM_REGION( 0x0800, "mcu", 0 ) // not dumped for this set, but seems to work fine. Marked 15-00011-001 DA68233
	ROM_LOAD( "a11_17.3a", 0x0000, 0x0800, BAD_DUMP CRC(e7d5d6e1) SHA1(c1131d6fcc36926e287be26090a3c89f22feaa35) )

	ROM_REGION( 0x6000, "sprites", 0 )
	ROM_LOAD( "obj3", 0x0000, 0x2000, CRC(c3b3131e) SHA1(ed0463e7eef452d7fbdcb031f9477825e9780943) ) // == pr-1.1k
	ROM_LOAD( "obj2", 0x2000, 0x2000, CRC(2fa1682a) SHA1(9daefb525fd69f0d9a45ff27e89865545e177a5a) ) // == pr-2.1m
	ROM_LOAD( "obj1", 0x4000, 0x2000, CRC(e678fe39) SHA1(134e36fd30bf3cf5884732f3455ca4d9dab6b665) ) // == pr-3.1n

	ROM_REGION( 0x4000, "bgtiles", 0 )
	ROM_LOAD( "chr1", 0x0000, 0x2000, CRC(fbae3504) SHA1(ce799dfd653462c0814e7530f3f8a686ab0ad7f4) ) // == pr-4.6d
	ROM_LOAD( "chr2", 0x2000, 0x2000, CRC(c9177180) SHA1(98c8f8f586b78b88dba254bd662642ee27f9b131) ) // == pr-5.6f

	ROM_REGION( 0x2000, "fgtiles", 0 )
	ROM_LOAD( "bsc1", 0x0000, 0x1000, CRC(c53cb897) SHA1(81a73e6031b52fa45ec507ff4264b14474ef42a2) ) // == pr-6.3m
	ROM_LOAD( "bsc2", 0x1000, 0x1000, CRC(7cdf9a55) SHA1(404dface7e09186e486945981e39063929599efc) ) // == pr-7.3p

	ROM_REGION( 0x2000, "spot", 0 )
	ROM_LOAD( "lightdata", 0x0000, 0x2000, CRC(8e346d10) SHA1(1362ce4362c2d28c48fbd8a33da0cec5ef8e321f) ) // == pr-8.4j

	ROM_REGION( 0x0060, "proms", 0 )
	ROM_LOAD( "bp1",  0x0000, 0x0020, CRC(643012f4) SHA1(4a0c9766b9da456e39ce379ad62d695bf82413b0) )
	ROM_LOAD( "bp2",  0x0020, 0x0020, CRC(50705f02) SHA1(a3d348678fd66f37c7a0d29af88f40740918b8d3) )
	ROM_LOAD( "bp3",  0x0040, 0x0020, CRC(25e70e5e) SHA1(fdb9c69e9568a725dd0e3ac25835270fb4f49280) )
ROM_END

ROM_START( jumpkun )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "pr1.5d.2764", 0x00000, 0x02000, CRC(b0eabe9f) SHA1(e662f3946efe72b0bbf6c6934201163f765bb7aa) )
	ROM_LOAD( "pr2.5c.2764", 0x02000, 0x02000, CRC(d9240413) SHA1(f4d0491e125f1fe435b200b38fa125889784af0a) )
	ROM_LOAD( "pr3.5b.2764", 0x04000, 0x02000, CRC(105e3fec) SHA1(06ea902e6647fc37a603146324e3d0a067e1f649) )
	ROM_LOAD( "pr4.5a.2764", 0x06000, 0x02000, CRC(3a17ca88) SHA1(00516798d546098831e75547664c8fdaa2bbf050) )

	ROM_REGION( 0x2000, "audiocpu", 0 )
	ROM_LOAD( "snd1.2732", 0x00000, 0x01000, CRC(1290f316) SHA1(13e393860c1f7d1f97343b9f936c60adb7641efc) )
	ROM_LOAD( "snd2.2732", 0x01000, 0x01000, CRC(ec5e4489) SHA1(fc94fe798a1925e8e3dd15161648e9a960969fc4) )

	ROM_REGION( 0x0800, "mcu", ROMREGION_ERASE00 )
	// not populated

	ROM_REGION( 0x6000, "sprites", 0 )
	ROM_LOAD( "obj1.1k.2764", 0x00000, 0x02000, CRC(8929abfd) SHA1(978994af5816c20a8cd520263d04d1cc1e4df576) )
	ROM_LOAD( "obj2.1m.2764", 0x02000, 0x02000, CRC(c7bf5819) SHA1(15d8e1dd1c0911785237e9063a75a42a2dc1bd50) )
	ROM_LOAD( "obj3.1n.2764", 0x04000, 0x02000, CRC(5eeec986) SHA1(e58a0b98b90a1dd3971ed305100337aa2e5ec450) )

	ROM_REGION( 0x4000, "bgtiles", 0 )
	ROM_LOAD( "chr1.6d.2764", 0x00000, 0x02000, CRC(3c93d4ee) SHA1(003121c49bccbb95efb137e6d92d26eea1957fbd)  )
	ROM_LOAD( "chr2.6f.2764", 0x02000, 0x02000, CRC(154fad33) SHA1(7eddc794bd547053f185bb79a8220907bab13d85)  )

	ROM_REGION( 0x2000, "fgtiles", 0 )
	ROM_LOAD( "bsc2.3m.2764", 0x00000, 0x01000, CRC(25445f17) SHA1(b1ada95d8f02623bb4a4562d2d278a882414e57e) )
	ROM_LOAD( "bsc1.3p.2764", 0x01000, 0x01000, CRC(39ca2c37) SHA1(b8c71f443a0faf54df03ac5aca46ddd34c42d3a0) )

	ROM_REGION( 0x2000, "spot", ROMREGION_ERASE00 )
	// not populated

	ROM_REGION( 0x0060, "proms", 0 )
	ROM_LOAD( "8h.82s123.bin", 0x0000, 0x0020, CRC(e54a6fe6) SHA1(c51da2cbf54b7abff7b0cdf0d6846c375b71edcd) )
	ROM_LOAD( "8l.82s123.bin", 0x0020, 0x0020, CRC(624830d5) SHA1(793b2f770ccef6c03bf3ecbf4debcc0531f62da1) )
	ROM_LOAD( "8j.82s123.bin", 0x0040, 0x0020, CRC(223a6990) SHA1(06e16de037c2c7ad5733390859fa7ec1ab1e2f69) )
ROM_END

} // anonymous namespace


GAME( 1984, pitnrun,  0,       pitnrun_mcu, pitnrun, pitnrun_mcu_state, empty_init, ROT90, "Taito Corporation", "Pit & Run - F-1 Race (rev 1)",          MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1984, pitnruna, pitnrun, pitnrun_mcu, pitnrun, pitnrun_mcu_state, empty_init, ROT90, "Taito Corporation", "Pit & Run - F-1 Race",                  MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1984, pitnrunb, pitnrun, pitnrun_mcu, pitnrun, pitnrun_mcu_state, empty_init, ROT90, "Taito Corporation", "Pit & Run - F-1 Race (location test?)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1984, jumpkun,  0,       pitnrun,     jumpkun, pitnrun_state,     empty_init, ROT90, "Kaneko",            "Jump Kun (prototype)",                  MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // no copyright message
