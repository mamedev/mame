// license:GPL-2.0+
// copyright-holders: Jarek Burczynski, Tomasz Slanina

/***************************************************************************
Big Event Golf (c) Taito 1986

driver by Jarek Burczynski
          Tomasz Slanina


****************************************************************************


Taito 1986  M4300056B

K1100156A Sound
J1100068A
                                YM2149     2016
                                MSM5232    A67-16
                                Z80        A67-17
                                           A67-18
                                8MHz
-----------------------------------------------------
K1100215A CPU
J1100066A

2064
A67-21
A67-20      A67-03
Z80         A67-04
            A67-05
            A67-06                          2016
            A67-07
            A67-08
            A67-09
            A67-10-2          2016
                              A67-11
            10MHz             Z80
                                   A67-19-1 (68705P5)
----------------------------------------------------
K1100215A VIDEO
J1100072A

                                 41464            2148
  93422                          41464            2148
  93422                          41464            2148
  93422                          41464
  93422                          41464
                                 41464
                                 41464
                                 41464


       A67-12-1                2016
       A67-13-1
       A67-14-1     2016
       A67-15-1                      18.432MHz

***************************************************************************/

#include "emu.h"

#include "taito68705.h"

#include "cpu/m6805/m6805.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/input_merger.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "sound/msm5232.h"

#include "emupal.h"
#include "speaker.h"
#include "screen.h"


namespace {

class bigevglf_state : public driver_device
{
public:
	bigevglf_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_paletteram(*this, "paletteram"),
		m_spriteram(*this, "spriteram%u", 1U),
		m_vidram(*this, "vidram", 0x100 * 0x100 * 4, ENDIANNESS_LITTLE),
		m_mainbank(*this, "mainbank"),
		m_maincpu(*this, "maincpu"),
		m_msm(*this, "msm"),
		m_soundlatch(*this, "soundlatch%u", 1),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_audiocpu(*this, "audiocpu"),
		m_bmcu(*this, "bmcu"),
		m_trackx(*this, { "P1X", "P2X" }),
		m_tracky(*this, { "P1Y", "P2Y" }),
		m_port04(*this, "PORT04") { }

	void bigevglf(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// memory pointers
	required_shared_ptr<uint8_t> m_paletteram;
	required_shared_ptr_array<uint8_t, 2> m_spriteram;
	memory_share_creator<uint8_t> m_vidram;
	required_memory_bank m_mainbank;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<msm5232_device> m_msm;
	required_device_array<generic_latch_8_device, 2> m_soundlatch;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<cpu_device> m_audiocpu;
	required_device<taito68705_mcu_device> m_bmcu;

	// I/O
	required_ioport_array<2> m_trackx;
	required_ioport_array<2> m_tracky;
	required_ioport m_port04;

	// video-related
	bitmap_ind16 m_tmp_bitmap[4]{};
	uint16_t m_vidram_bank = 0U;
	uint8_t m_plane_selected = 0U;
	uint8_t m_plane_visible = 0U;

	// MCU related
	uint8_t m_mcu_coin_bit5 = 0;

	// misc
	uint8_t m_13_ls74[2]{};
	uint8_t m_port_select = 0U;     // for muxed controls

	void banking_w(uint8_t data);
	uint8_t soundstate_r();
	void _13_a_clr_w(uint8_t data);
	void _13_b_clr_w(uint8_t data);
	void _13_a_set_w(uint8_t data);
	void _13_b_set_w(uint8_t data);
	uint8_t status_r();
	uint8_t trackball_x_r();
	uint8_t trackball_y_r();
	void port08_w(uint8_t data);
	uint8_t sub_cpu_mcu_coin_port_r();
	void palette_w(offs_t offset, uint8_t data);
	void gfxcontrol_w(uint8_t data);
	void vidram_addr_w(uint8_t data);
	void vidram_w(offs_t offset, uint8_t data);
	uint8_t vidram_r(offs_t offset);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(deferred_ls74_w);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_portmap(address_map &map) ATTR_COLD;
	void sub_portmap(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
	void sub_map(address_map &map) ATTR_COLD;
};


void bigevglf_state::palette_w(offs_t offset, uint8_t data)
{
	m_paletteram[offset] = data;
	int const color = m_paletteram[offset & 0x3ff] | (m_paletteram[0x400 + (offset & 0x3ff)] << 8);
	m_palette->set_pen_color(offset & 0x3ff, pal4bit(color >> 4), pal4bit(color >> 0), pal4bit(color >> 8));
}

void bigevglf_state::gfxcontrol_w(uint8_t data)
{
/* bits used: 0,1,2,3
 0 and 2 select plane,
 1 and 3 select visible plane,
*/
	m_plane_selected  = ((data & 4) >> 1) | (data & 1);
	m_plane_visible = ((data & 8) >> 2) | ((data & 2) >> 1);
}

void bigevglf_state::vidram_addr_w(uint8_t data)
{
	m_vidram_bank = (data & 0xff) * 0x100;
}

void bigevglf_state::vidram_w(offs_t offset, uint8_t data)
{
	uint32_t const o = m_vidram_bank + offset;
	m_vidram[o + 0x10000 * m_plane_selected] = data;
	uint32_t y = o >> 8;
	uint32_t x = (o & 255);
	m_tmp_bitmap[m_plane_selected].pix(y, x) = data;
}

uint8_t bigevglf_state::vidram_r(offs_t offset)
{
	return m_vidram[0x10000 * m_plane_selected + m_vidram_bank + offset];
}

void bigevglf_state::video_start()
{
	m_screen->register_screen_bitmap(m_tmp_bitmap[0]);
	m_screen->register_screen_bitmap(m_tmp_bitmap[1]);
	m_screen->register_screen_bitmap(m_tmp_bitmap[2]);
	m_screen->register_screen_bitmap(m_tmp_bitmap[3]);
	save_item(NAME(m_tmp_bitmap[0]));
	save_item(NAME(m_tmp_bitmap[1]));
	save_item(NAME(m_tmp_bitmap[2]));
	save_item(NAME(m_tmp_bitmap[3]));
}

void bigevglf_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int i = 0xc0 - 4; i >= 0; i -= 4)
	{
		int const code = m_spriteram[1][i + 1];
		int const sx = m_spriteram[1][i + 3];
		int const sy = 200 - m_spriteram[1][i];
		for (int j = 0; j < 16; j++)
			m_gfxdecode->gfx(0)->transpen(bitmap, cliprect,
				m_spriteram[0][(code << 4) + j] + ((m_spriteram[0][0x400 + (code << 4) + j] & 0xf) << 8),
				m_spriteram[1][i + 2] & 0xf,
				0,0,
				sx + ((j & 1) << 3), sy + ((j >> 1) << 3), 0);
	}
}

uint32_t bigevglf_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_tmp_bitmap[m_plane_visible], 0, 0, 0, 0, cliprect);
	draw_sprites(bitmap, cliprect);
	return 0;
}


void bigevglf_state::banking_w(uint8_t data)
{
/* d0-d3 connect to A11-A14 of the ROMs (via ls273 latch)
   d4-d7 select one of ROMs (via ls273(above) and then ls154)
*/
	m_mainbank->set_entry(data); // empty sockets for IC37-IC44 ROMS
}

uint8_t bigevglf_state::soundstate_r()
{
	uint8_t sound_state = m_soundlatch[0]->pending_r() ? 0 : 1;
	sound_state |= m_soundlatch[1]->pending_r() ? 2 : 0;
	return sound_state;
}

TIMER_CALLBACK_MEMBER(bigevglf_state::deferred_ls74_w)
{
	int const offs = (param >> 8) & 255;
	int const data = param & 255;
	m_13_ls74[offs] = data;
}

// do this on a timer to let the CPUs synchronize
void bigevglf_state::_13_a_clr_w(uint8_t data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(bigevglf_state::deferred_ls74_w),this), (0 << 8) | 0);
}

void bigevglf_state::_13_b_clr_w(uint8_t data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(bigevglf_state::deferred_ls74_w),this), (1 << 8) | 0);
}

void bigevglf_state::_13_a_set_w(uint8_t data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(bigevglf_state::deferred_ls74_w),this), (0 << 8) | 1);
}

void bigevglf_state::_13_b_set_w(uint8_t data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(bigevglf_state::deferred_ls74_w),this), (1 << 8) | 1);
}

uint8_t bigevglf_state::status_r()
{
/* d0 = Q of 74ls74 IC13(partA)
   d1 = Q of 74ls74 IC13(partB)
   d2 =
   d3 =
   d4 =
   d5 =
   d6 = d7 = 10MHz/2

*/
	// set a timer to force synchronization after the read
	machine().scheduler().synchronize();
	return (m_13_ls74[0] << 0) | (m_13_ls74[1] << 1);
}


uint8_t bigevglf_state::trackball_x_r()
{
	return m_trackx[m_port_select]->read();
}

uint8_t bigevglf_state::trackball_y_r()
{
	return m_tracky[m_port_select]->read();
}

void bigevglf_state::port08_w(uint8_t data)
{
	m_port_select = (data & 0x04) >> 2;
}


static INPUT_PORTS_START( bigevglf )
	PORT_START("PORT00")        // on sub CPU
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("PORT04")        // on sub CPU - bit 0 and bit 1 are coin inputs
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )

	PORT_START("DSW1")          // port 05 on sub CPU
	PORT_DIPNAME( 0x01,   0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02,   0x02, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x04, IP_ACTIVE_LOW, "SW1:3" )
	PORT_DIPNAME( 0x08,   0x08, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0xf0,   0xf0, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(      0x50, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0xa0, DEF_STR( 1C_2C ) )

	PORT_START("DSW2")          // port 06 on sub CPU
	PORT_DIPNAME( 0x03,   0x03, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x01, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c,   0x0c, "Holes" ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x04, "1" )
	PORT_DIPSETTING(      0x08, "2" )
	PORT_DIPSETTING(      0x0c, "3" )
	PORT_DIPSETTING(      0x00, "4" )
	PORT_DIPNAME( 0x10,   0x00, DEF_STR( Language ) ) PORT_DIPLOCATION("SW2:5") // Also changes the copyright on the title screen
	PORT_DIPSETTING(      0x00, DEF_STR( English ) )                            // (c) 1986 Taito America Corp.
	PORT_DIPSETTING(      0x10, DEF_STR( Japanese ) )                           // (c) Taito Corporation 1986
	PORT_DIPNAME( 0xe0,   0xa0, "Full game price (credits)" ) PORT_DIPLOCATION("SW2:6,7,8")
	PORT_DIPSETTING(      0xe0, "3" )
	PORT_DIPSETTING(      0xc0, "4" )
	PORT_DIPSETTING(      0xa0, "5" )
	PORT_DIPSETTING(      0x80, "6" )
	PORT_DIPSETTING(      0x60, "7" )
	PORT_DIPSETTING(      0x40, "8" )
	PORT_DIPSETTING(      0x20, "9" )
	PORT_DIPSETTING(      0x00, "10" )

	PORT_START("P1X")   // port 02 on sub CPU - muxed port 0
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(30) PORT_KEYDELTA(10)

	PORT_START("P1Y")   // port 03 on sub CPU - muxed port 0
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(30) PORT_KEYDELTA(10) PORT_REVERSE

	PORT_START("P2X")   // port 02 on sub CPU - muxed port 1
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(30) PORT_KEYDELTA(10) PORT_COCKTAIL

	PORT_START("P2Y")   // port 03 on sub CPU - muxed port 1
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(30) PORT_KEYDELTA(10) PORT_REVERSE PORT_COCKTAIL
INPUT_PORTS_END

static INPUT_PORTS_START( bigevglfj )
	PORT_INCLUDE(bigevglf)

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x10,   0x10, DEF_STR( Language ) ) PORT_DIPLOCATION("SW2:5") // Doesn't change the title screen copyright like the US set
	PORT_DIPSETTING(      0x00, DEF_STR( English ) )
	PORT_DIPSETTING(      0x10, DEF_STR( Japanese ) )
INPUT_PORTS_END


/*****************************************************************************/
// Main CPU

void bigevglf_state::main_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xcfff).ram();
	map(0xd000, 0xd7ff).bankr(m_mainbank);
	map(0xd800, 0xdbff).ram().share("main_sub"); // only half of the RAM is accessible, line a10 of IC73 (6116) is GNDed
	map(0xe000, 0xe7ff).w(FUNC(bigevglf_state::palette_w)).share(m_paletteram);
	map(0xe800, 0xefff).writeonly().share(m_spriteram[0]); // sprite 'templates'
	map(0xf000, 0xf0ff).rw(FUNC(bigevglf_state::vidram_r), FUNC(bigevglf_state::vidram_w)); // 41464 (64kB * 8 chips), addressed using ports 1 and 5
	map(0xf840, 0xf8ff).ram().share(m_spriteram[1]);  // x, y, offset in m_spriteram[0], palette
}

void bigevglf_state::main_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).nopw();    // video RAM enable ???
	map(0x01, 0x01).w(FUNC(bigevglf_state::gfxcontrol_w));  // plane select
	map(0x02, 0x02).w(FUNC(bigevglf_state::banking_w));
	map(0x03, 0x03).w(FUNC(bigevglf_state::_13_a_set_w));
	map(0x04, 0x04).w(FUNC(bigevglf_state::_13_b_clr_w));
	map(0x05, 0x05).w(FUNC(bigevglf_state::vidram_addr_w));   // video banking (256 banks) for f000-f0ff area
	map(0x06, 0x06).r(FUNC(bigevglf_state::status_r));
}


/*********************************************************************************/
// Sub CPU

void bigevglf_state::sub_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x47ff).ram();
	map(0x8000, 0x83ff).ram().share("main_sub");
}


uint8_t bigevglf_state::sub_cpu_mcu_coin_port_r()
{
	/*
	        bit 0 and bit 1 = coin inputs
	        bit 3 and bit 4 = MCU status
	        bit 5           = must toggle, vblank ?

	*/
	m_mcu_coin_bit5 ^= 0x20;
	return
		(m_port04->read() & 0x03) |
		((CLEAR_LINE == m_bmcu->host_semaphore_r()) ? 0x08 : 0x00) |
		((CLEAR_LINE != m_bmcu->mcu_semaphore_r()) ? 0x10 : 0x00) |
		m_mcu_coin_bit5;  // bit 0 and bit 1 - coin inputs
}

void bigevglf_state::sub_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).portr("PORT00");
	map(0x01, 0x01).nopr();
	map(0x02, 0x02).r(FUNC(bigevglf_state::trackball_x_r));
	map(0x03, 0x03).r(FUNC(bigevglf_state::trackball_y_r));
	map(0x04, 0x04).r(FUNC(bigevglf_state::sub_cpu_mcu_coin_port_r));
	map(0x05, 0x05).portr("DSW1");
	map(0x06, 0x06).portr("DSW2");
	map(0x07, 0x07).nopr();
	map(0x08, 0x08).w(FUNC(bigevglf_state::port08_w)); // muxed port select + other unknown stuff
	map(0x0b, 0x0b).r(m_bmcu, FUNC(taito68705_mcu_device::data_r));
	map(0x0c, 0x0c).w(m_bmcu, FUNC(taito68705_mcu_device::data_w));
	map(0x0e, 0x0e).nopw(); // 0-enable MCU, 1-keep reset line ASSERTED; D0 goes to the input of ls74 and the /Q of this ls74 goes to reset line on 68705
	map(0x10, 0x17).w(FUNC(bigevglf_state::_13_a_clr_w));
	map(0x18, 0x1f).w(FUNC(bigevglf_state::_13_b_set_w));
	map(0x20, 0x20).r(m_soundlatch[1], FUNC(generic_latch_8_device::read));
	map(0x20, 0x20).w(m_soundlatch[0], FUNC(generic_latch_8_device::write));
	map(0x21, 0x21).r(FUNC(bigevglf_state::soundstate_r));
}




/*********************************************************************************/
// Sound CPU

void bigevglf_state::sound_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xc7ff).ram();
	map(0xc800, 0xc801).w("aysnd", FUNC(ym2149_device::address_data_w));
	map(0xca00, 0xca0d).w(m_msm, FUNC(msm5232_device::write));
	map(0xcc00, 0xcc00).nopw();
	map(0xce00, 0xce00).nopw();
	map(0xd800, 0xd800).r(m_soundlatch[0], FUNC(generic_latch_8_device::read));
	map(0xd800, 0xd800).w(m_soundlatch[1], FUNC(generic_latch_8_device::write)); // write to D800 sets bit 1 in status
	map(0xda00, 0xda00).r(FUNC(bigevglf_state::soundstate_r));
	map(0xda00, 0xda00).w("soundnmi", FUNC(input_merger_device::in_set<1>)); // enable NMI
	map(0xdc00, 0xdc00).w("soundnmi", FUNC(input_merger_device::in_clear<1>)); // disable NMI
	map(0xde00, 0xde00).nopr().w("dac", FUNC(dac_byte_interface::data_w)); // signed 8-bit DAC & unknown read
	map(0xe000, 0xefff).nopr();     // space for diagnostics ROM
}




static const gfx_layout gfxlayout =
{
	8,8,
	RGN_FRAC(1,4),
	4,
	{ 0, RGN_FRAC(1,4),RGN_FRAC(2,4),RGN_FRAC(3,4)},
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8 },
	8*8
};

static GFXDECODE_START( gfx_bigevglf )
	GFXDECODE_ENTRY( "gfx", 0, gfxlayout, 0x20*16, 16 )
GFXDECODE_END


void bigevglf_state::machine_start()
{
	uint8_t *rom = memregion("maincpu")->base();
	m_mainbank->configure_entries(0, 0x100, &rom[0x10000], 0x800);

	save_item(NAME(m_vidram_bank));
	save_item(NAME(m_plane_selected));
	save_item(NAME(m_plane_visible));

	save_item(NAME(m_13_ls74));
	save_item(NAME(m_port_select));

	save_item(NAME(m_mcu_coin_bit5));
}

void bigevglf_state::machine_reset()
{
	m_vidram_bank = 0;
	m_plane_selected = 0;
	m_plane_visible = 0;

	m_13_ls74[0] = 0;
	m_13_ls74[1] = 0;
	m_port_select = 0;

	m_mcu_coin_bit5 = 0;
}

void bigevglf_state::bigevglf(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 10_MHz_XTAL / 2);     // 5 MHz ? divider not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &bigevglf_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &bigevglf_state::main_portmap);
	m_maincpu->set_vblank_int("screen", FUNC(bigevglf_state::irq0_line_hold));

	z80_device &subcpu(Z80(config, "sub", 10_MHz_XTAL / 2));     // 5 MHz ? divider not verified
	subcpu.set_addrmap(AS_PROGRAM, &bigevglf_state::sub_map);
	subcpu.set_addrmap(AS_IO, &bigevglf_state::sub_portmap);
	subcpu.set_vblank_int("screen", FUNC(bigevglf_state::irq0_line_hold));

	Z80(config, m_audiocpu, 8_MHz_XTAL / 2);     // 4 MHz ? divider not verified
	m_audiocpu->set_addrmap(AS_PROGRAM, &bigevglf_state::sound_map);
	m_audiocpu->set_periodic_int(FUNC(bigevglf_state::irq0_line_hold), attotime::from_hz(2 * 60));  /* IRQ generated by ???
	    2 irqs/frame give good music tempo but also SOUND ERROR in test mode,
	    4 irqs/frame give SOUND OK in test mode but music seems to be running too fast */
		// Clearly, then, there should be some sort of IRQ acknowledge mechanism, duh. -R

	GENERIC_LATCH_8(config, m_soundlatch[0]);
	m_soundlatch[0]->data_pending_callback().set("soundnmi", FUNC(input_merger_device::in_w<0>));

	GENERIC_LATCH_8(config, m_soundlatch[1]);

	INPUT_MERGER_ALL_HIGH(config, "soundnmi").output_handler().set_inputline("audiocpu", INPUT_LINE_NMI);

	TAITO68705_MCU(config, m_bmcu, 10_MHz_XTAL / 5);     // 2 MHz ? divider not verified

	config.set_maximum_quantum(attotime::from_hz(600));   // 10 CPU slices per frame - interleaving is forced on the fly

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(32*8, 32*8);
	m_screen->set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(bigevglf_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_bigevglf);
	PALETTE(config, m_palette).set_entries(0x800);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	YM2149(config, "aysnd", 8_MHz_XTAL / 4).add_route(ALL_OUTPUTS, "mono", 0.15); // divider not verified

	MSM5232(config, m_msm, 8_MHz_XTAL / 4); // divider not verified
	m_msm->set_capacitors(1e-6, 1e-6, 1e-6, 1e-6, 1e-6, 1e-6, 1e-6, 1e-6); // 1 uF capacitors ?
	m_msm->add_route(0, "mono", 1.0);   // pin 28  2'-1
	m_msm->add_route(1, "mono", 1.0);   // pin 29  4'-1
	m_msm->add_route(2, "mono", 1.0);   // pin 30  8'-1
	m_msm->add_route(3, "mono", 1.0);   // pin 31 16'-1
	m_msm->add_route(4, "mono", 1.0);   // pin 36  2'-2
	m_msm->add_route(5, "mono", 1.0);   // pin 35  4'-2
	m_msm->add_route(6, "mono", 1.0);   // pin 34  8'-2
	m_msm->add_route(7, "mono", 1.0);   // pin 33 16'-2
	// pin 1 SOLO  8'       not mapped
	// pin 2 SOLO 16'       not mapped
	// pin 22 Noise Output  not mapped

	DAC_8BIT_R2R(config, "dac", 0).add_route(ALL_OUTPUTS, "mono", 0.50); // unknown DAC
}

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( bigevglf )
	ROM_REGION( 0x90000, "maincpu", 0 )
	ROM_LOAD( "a67-21",   0x00000, 0x8000, CRC(2a62923d) SHA1(7b025180e203f268ae4d11baa18096e0a5704f77))
	ROM_LOAD( "a67-20",   0x08000, 0x4000, CRC(841561b1) SHA1(5d91449e135ef22508194a9543343c29e1c496cf))
	ROM_LOAD( "a67-03",   0x10000, 0x8000, CRC(695b3396) SHA1(2a3738e6bc492a4c68d2e15a2e474f7654aeb94d))
	ROM_LOAD( "a67-04",   0x18000, 0x8000, CRC(b8941902) SHA1(a03e432cbd8ea1df7223ea99ff1db220a57fc698))
	ROM_LOAD( "a67-05",   0x20000, 0x8000, CRC(681f5f4f) SHA1(2a5d8eeaf6ac697d5d4ee15164b6c4b1b81d7a29))
	ROM_LOAD( "a67-06",   0x28000, 0x8000, CRC(026f6fe5) SHA1(923b7d8363e587ef20b6518bee968d378166c76b))
	ROM_LOAD( "a67-07",   0x30000, 0x8000, CRC(27706bed) SHA1(da702c5a098eb106332996ec5d0e2c014782031e))
	ROM_LOAD( "a67-08",   0x38000, 0x8000, CRC(e922023a) SHA1(ea4c4b5e2f82ab20afb15f115e8cbc66d8471927))
	ROM_LOAD( "a67-09",   0x40000, 0x8000, CRC(a9d4263e) SHA1(8c3f2d541583e8e4b22e0beabcd04c5765508535))
	ROM_LOAD( "a67-10",   0x48000, 0x8000, CRC(7c35f4a3) SHA1(de60dc991c67fb7d48314bc8b2c1a27ad040bf1e))

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "a67-11",   0x00000, 0x4000, CRC(a2660d20) SHA1(3d8b670c071862d677d4e30655dcb6b8d830648a))

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "a67-16",   0x0000, 0x4000, CRC(5fb6d22e) SHA1(1701aa94b7f524187fd7213a94535bed3e8b6cc9))
	ROM_LOAD( "a67-17",   0x4000, 0x4000, CRC(9f57deae) SHA1(dbdb3d77c3de0113ef6671aec854e4e44ee162ef))
	ROM_LOAD( "a67-18",   0x8000, 0x4000, CRC(40d54fed) SHA1(bfa0922809bffafec15d3ef59ac8b8ad653860a1))

	ROM_REGION( 0x0800, "bmcu:mcu", 0 )
	ROM_LOAD( "a67_19-1", 0x0000, 0x0800, CRC(25691658) SHA1(aabf47abac43abe2ffed18ead1cb94e587149e6e))

	ROM_REGION( 0x20000, "gfx", 0 )
	ROM_LOAD( "a67-12",   0x00000, 0x8000, CRC(980cc3c5) SHA1(c35c20cfff0b5cfd5b95742333ae20a2c93371b5))
	ROM_LOAD( "a67-13",   0x08000, 0x8000, CRC(ad6e04af) SHA1(4680d789cf53c4808105ad4f3c70aedb6d8bcf36))
	ROM_LOAD( "a67-14",   0x10000, 0x8000, CRC(d6708cce) SHA1(5b48f9dff2a3e28242dc2004469dc2ac2b5d0321))
	ROM_LOAD( "a67-15",   0x18000, 0x8000, CRC(1d261428) SHA1(0f3e6d83a8a462436fa414de4e1e4306db869d3e))
ROM_END

ROM_START( bigevglfj )
	ROM_REGION( 0x90000, "maincpu", 0 )
	ROM_LOAD( "a67-02-2.10", 0x00000, 0x8000, CRC(b3edbb78) SHA1(7873b1a94cca830f1d1c143376cb49f6e48dbf0b))
	ROM_LOAD( "a67-01-2.9",  0x08000, 0x4000, CRC(7788b5d0) SHA1(f331138352c4d7b4566d342047785ed97e7b5990))
	ROM_LOAD( "a67-03",      0x10000, 0x8000, CRC(695b3396) SHA1(2a3738e6bc492a4c68d2e15a2e474f7654aeb94d))
	ROM_LOAD( "a67-04",      0x18000, 0x8000, CRC(b8941902) SHA1(a03e432cbd8ea1df7223ea99ff1db220a57fc698))
	ROM_LOAD( "a67-05",      0x20000, 0x8000, CRC(681f5f4f) SHA1(2a5d8eeaf6ac697d5d4ee15164b6c4b1b81d7a29))
	ROM_LOAD( "a67-06",      0x28000, 0x8000, CRC(026f6fe5) SHA1(923b7d8363e587ef20b6518bee968d378166c76b))
	ROM_LOAD( "a67-07",      0x30000, 0x8000, CRC(27706bed) SHA1(da702c5a098eb106332996ec5d0e2c014782031e))
	ROM_LOAD( "a67-08",      0x38000, 0x8000, CRC(e922023a) SHA1(ea4c4b5e2f82ab20afb15f115e8cbc66d8471927))
	ROM_LOAD( "a67-09",      0x40000, 0x8000, CRC(a9d4263e) SHA1(8c3f2d541583e8e4b22e0beabcd04c5765508535))
	ROM_LOAD( "a67-10",      0x48000, 0x8000, CRC(7c35f4a3) SHA1(de60dc991c67fb7d48314bc8b2c1a27ad040bf1e))

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "a67-11",   0x00000, 0x4000, CRC(a2660d20) SHA1(3d8b670c071862d677d4e30655dcb6b8d830648a))

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "a67-16",   0x0000, 0x4000, CRC(5fb6d22e) SHA1(1701aa94b7f524187fd7213a94535bed3e8b6cc9))
	ROM_LOAD( "a67-17",   0x4000, 0x4000, CRC(9f57deae) SHA1(dbdb3d77c3de0113ef6671aec854e4e44ee162ef))
	ROM_LOAD( "a67-18",   0x8000, 0x4000, CRC(40d54fed) SHA1(bfa0922809bffafec15d3ef59ac8b8ad653860a1))

	ROM_REGION( 0x0800, "bmcu:mcu", 0 )
	ROM_LOAD( "a67_19-1", 0x0000, 0x0800, CRC(25691658) SHA1(aabf47abac43abe2ffed18ead1cb94e587149e6e))

	ROM_REGION( 0x20000, "gfx", 0 )
	ROM_LOAD( "a67-12",   0x00000, 0x8000, CRC(980cc3c5) SHA1(c35c20cfff0b5cfd5b95742333ae20a2c93371b5))
	ROM_LOAD( "a67-13",   0x08000, 0x8000, CRC(ad6e04af) SHA1(4680d789cf53c4808105ad4f3c70aedb6d8bcf36))
	ROM_LOAD( "a67-14",   0x10000, 0x8000, CRC(d6708cce) SHA1(5b48f9dff2a3e28242dc2004469dc2ac2b5d0321))
	ROM_LOAD( "a67-15",   0x18000, 0x8000, CRC(1d261428) SHA1(0f3e6d83a8a462436fa414de4e1e4306db869d3e))
ROM_END

} // anonymous namespace


GAME( 1986, bigevglf,  0,        bigevglf, bigevglf,  bigevglf_state, empty_init, ROT270, "Taito America Corporation", "Big Event Golf (US)",    MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1986, bigevglfj, bigevglf, bigevglf, bigevglfj, bigevglf_state, empty_init, ROT270, "Taito Corporation",         "Big Event Golf (Japan)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
