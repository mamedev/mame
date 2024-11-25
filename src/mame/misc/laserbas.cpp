// license:BSD-3-Clause
// copyright-holders: Tomasz Slanina, Morten Shearman Kirkegaard, Samuel Neves, Peter Wilhelmsen

/********************************************
 Laser Base / Future Flash driver

============================================
TODO:
- Video: weird palette changes, Laserbase colors, gfx rendering in general needs verification
- Sound: sound related i/o writes ( out_w handler )
- Interrupts - NMI/Int timing is wrong, it's based on measures of broken PCB
TS 20.01.2017

There's incomplete schematics available. It's missing a couple of important elements (analog sound, interrupts).
PCB contains big analog sound section, currently only partially emulated ( timers + counters + xors + dacs). There's many capacitors,
resistors and few ICs, including:

7B LM2902N - Op Amps
4B LM2902N
2B LM2902N
1B LM2902N
6B LM1496N - Balanced Modulator-Demodulator
3B CA3080E - Op Amp
6C NE555P - Timer
4A NE555P - Timer
6A SN94560AN - ? a tone generator (similar to SN76477 ?)
3A MB84013B - Dual D flip-flop
2A MN3008 - 2048-stage long delay low noise BBD  (reverb effect generator)
1C MB3712  - Power Amp.

DASM notes:

0x100: check if test mode bit is active.
0x3ae8: ?
0x3aec: tests 0xfc00 work ram ONLY, resets if fails
0x3afe: fill 0xfc00-0xffff to zero
0x20dc: writes ROM 0x3146 to prot RAM 0xf800-0xfbff
0x20e9: reads from 0xfa47, A = (n & 0x8) | 0x80 then HL = 0x0200 | A
0x2cef: unknown, reads from 0x02** to 0x2d00, fancy ROM checksum?
...
0x0577

NOTE: None of the current sets passes the ROM check

laserbas  1) 2) 3) 4) 5) 6) 7) 8) Z1)
measured: 16 62 9F AD D4 BE 6C 01 AB
expected: 17 5B CC 62 D4 23 6C 01 3E  <- From ROM 4
          1  1  1  1  0  1  0  0  1   <- 1 = BAD

laserbasa 1) 2) 3) 4) 5) 6) 7) 8) Z1)
measured: 0A 62 F6 AD D4 BE 6C 01 AB
expected: 17 5B CC 62 D4 23 6C 01 3E  <- From ROM 4
          1  1  1  1  0  1  0  0  1   <- 1 = BAD

futflash  1) 2) 3) 4) 5) 6) 7) 8) Z1)
measured: 43 5B CC 9A D4 23 6C 01 AB
expected: 43 FB CC 9A D4 23 6C 01 3E  <- From ROM 4
          0  1  0  0  0  0  0  0  1   <- 1 = BAD

********************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/pit8253.h"
#include "machine/timer.h"
#include "sound/dac.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class laserbas_state : public driver_device
{
public:
	laserbas_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_palette(*this, "palette"),
		m_dac(*this, "dac%u", 1U),
		m_vrambank(*this, "vram"),
		m_track(*this, { "TRACK_X", "TRACK_Y" })
	{ }

	void laserbas(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
	required_device_array<dac_byte_interface, 6> m_dac;
	required_memory_bank m_vrambank;
	required_ioport_array<2> m_track;

	// misc
	uint8_t  m_counter[6];
	uint8_t  m_cnt_out[6];
	int m_nmi;

	// input-related
	uint8_t  m_track_prv[2];
	int8_t   m_track_cnt[2];

	// video-related
	uint8_t  m_vram[0x10000];
	uint8_t  m_hset, m_vset;
	uint8_t  m_bset;
	uint8_t  m_scl;
	bool     m_flipscreen;
	uint64_t m_z1data;

	void videoctrl1_w(offs_t offset, uint8_t data);
	void videoctrl2_w(offs_t offset, uint8_t data);
	uint8_t z1_r(offs_t offset);
	uint8_t track_dir_r();
	uint8_t track_val_r();
	void out_w(uint8_t data);
	template<uint8_t Which> void pit_out_w(int state);
	TIMER_DEVICE_CALLBACK_MEMBER(laserbas_scanline);
	MC6845_UPDATE_ROW(crtc_update_row);

	void laserbas_io(address_map &map) ATTR_COLD;
	void laserbas_memory(address_map &map) ATTR_COLD;
};

TIMER_DEVICE_CALLBACK_MEMBER(  laserbas_state::laserbas_scanline )
{
	int scanline = param;

	if(scanline == 0 || scanline == 135)
	{
		m_maincpu->set_input_line(0, HOLD_LINE );
	}

	if(scanline == 240 && m_nmi)
	{
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
	}
}

MC6845_UPDATE_ROW( laserbas_state::crtc_update_row )
{
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();

	offs_t addr = ((ma & 0x3ff) << 5) | (ra << 7);
	addr += (m_vset << 7);

	for (int x = 0; x < x_count; x++)
	{
		// draw 8 pixels
		for (int i = 0; i < 8; i++)
		{
			// layer 1 (scrolling)
			offs_t offset_p1 = (addr + (x << 2) + (i >> 1)) & 0x7fff;
			uint8_t p1 = (m_vram[0x0000 + offset_p1] >> ((i & 1) * 4)) & 0x0f;

			// layer 2 (fixed)
			offs_t offset_p2= ((y * 0x80) | (ra << 7)) + (x << 2) + (i >> 1);
			uint8_t p2 = (m_vram[0x8000 + offset_p2] >> ((i & 1) * 4)) & 0x0f;

			// priority: p2 > p1 > background
			uint8_t p = p2 ? p2 : p1 ? (p1 + 16) : m_bset;

			if (m_flipscreen)
				bitmap.pix(0xdf - y, 0xff - (x * 8 + i)) = palette[p];
			else
				bitmap.pix(y, x * 8 + i) = palette[p];
		}
	}
}

void laserbas_state::videoctrl1_w(offs_t offset, uint8_t data)
{
	data ^= 0xff;

	// 7-------  flip screen
	// -6------  layer select
	// --543---  vset (vertical scroll, inc'ed on interrupts - 8 ints/frame?)
	// -----210  hset (presumably horizontal scroll)

	m_flipscreen = BIT(data, 7);
	m_vrambank->set_entry(BIT(~data, 6));
	m_vset = (data >> 3) & 0x07;
	m_hset = (data >> 0) & 0x07;
}

void laserbas_state::videoctrl2_w(offs_t offset, uint8_t data)
{
	data ^= 0xff;

	// 7654----  background pen
	// ----3---  scl (unknown)
	// -----21-  not used?
	// -------0  nmi enable (not on schematics, traced)

	m_bset = (data >> 4) & 0x0f;
	m_scl = BIT(data, 3);
	m_nmi = BIT(data, 0);
}

uint8_t laserbas_state::z1_r(offs_t offset)
{
	m_z1data = (m_z1data >> 10) | (uint64_t(offset & 0x03ff) << 30);

	auto const x = [this] (unsigned b) { return BIT(m_z1data, b); };
	auto const nx = [this] (unsigned b) { return BIT(~m_z1data, b); };
	auto const MUX2 = [] (bool s, uint16_t a, uint16_t b) { return s ? a : b; };

	uint8_t const bit7 = MUX2(x(36) & x(33), x(31) ^ x(35), (nx(33) & (nx(20) | nx(36))) | (nx(36) & x(24)));
	uint8_t const bit6 = MUX2(x(36), MUX2(x(33), x(29), nx(23) | x(30)), x(33) & x(15));
	uint8_t const bit5 = MUX2(x(36), nx(33) & x(27), x(32) | x(33));
	uint8_t const bit4 = MUX2(x(36), MUX2(x(33), nx(24), nx(35)), MUX2(nx(33), x(4), MUX2(nx(26), x(5) & x(23), MUX2(x(23), x(19), nx(13) | x(20)))));
	uint8_t const bit3 = MUX2(x(36), x(33) & x(11), MUX2(x(33), x(25) | nx(31), x(24)));
	uint8_t const bit2 = MUX2(x(33), MUX2(x(36), x(28), x(20)), MUX2(x(36), nx(11), nx(39)));
	uint8_t const bit1 = MUX2(x(36), MUX2(x(23), MUX2(x(26), nx(18), nx(10)) | x(33), MUX2(x(26), x(1), x(29)) & nx(33)), MUX2(x(33), x(7), x(17)));
	uint8_t const bit0 = MUX2(x(33), MUX2(x(36), x(22), nx(26)), MUX2(x(36), x(14), x(21)));

	return (bit7 << 7) | (bit6 << 6) | (bit5 << 5) | (bit4 << 4) | (bit3 << 3) | (bit2 << 2) | (bit1 << 1) | (bit0 << 0);
}

// trackball read twice per frame, direction first then value

uint8_t laserbas_state::track_dir_r()
{
	for (unsigned i = 0; m_track.size() > i; ++i)
	{
		uint8_t const track = uint8_t(m_track[i]->read());
		int diff = track - m_track_prv[i];
		m_track_prv[i] = track;

		if (diff > 0x20)
			diff -= 0x40;
		else if (diff < -0x20)
			diff += 0x40;

		m_track_cnt[i] += diff;
	}
	return ((m_track_cnt[0] < 0) ? 0x01 : 0x00) | ((m_track_cnt[1] > 0) ? 0x02 : 0x00);
}

uint8_t laserbas_state::track_val_r()
{
	int8_t const x = std::clamp<int8_t>(m_track_cnt[0], -15, 15);
	int8_t const y = std::clamp<int8_t>(m_track_cnt[1], -15, 15);
	m_track_cnt[0] -= x;
	m_track_cnt[1] -= y;

	return std::abs(x) | (std::abs(y) << 4);
}

void laserbas_state::out_w(uint8_t data)
{
	// sound related , maybe also lamps
}

void laserbas_state::machine_start()
{
	m_vrambank->configure_entries(0, 2, m_vram, 0x8000);

	std::fill(std::begin(m_counter), std::end(m_counter), 0);
	std::fill(std::begin(m_cnt_out), std::end(m_cnt_out), 0);
	m_nmi = 0;

	std::fill(std::begin(m_track_prv), std::end(m_track_prv), 0);
	std::fill(std::begin(m_track_cnt), std::end(m_track_cnt), 0);

	save_item(NAME(m_counter));
	save_item(NAME(m_cnt_out));
	save_item(NAME(m_nmi));
	save_item(NAME(m_track_prv));
	save_item(NAME(m_track_cnt));
	save_item(NAME(m_vram));
	save_item(NAME(m_hset));
	save_item(NAME(m_vset));
	save_item(NAME(m_bset));
	save_item(NAME(m_scl));
	save_item(NAME(m_flipscreen));
	save_item(NAME(m_z1data));
}

void laserbas_state::machine_reset()
{
	m_vrambank->set_entry(1);

	m_hset = 0;
	m_vset = 0;
	m_bset = 0;
	m_scl = 0;
	m_flipscreen = 0;
}

template<uint8_t Which>
void laserbas_state::pit_out_w(int state)
{
	state ^= 1; // 7404  (6G)
	if (!state && m_cnt_out[Which]) // 0->1 rising edge CLK
		m_counter[Which] = (m_counter[Which] + 1) & 0x0f; // 4 bit counters 74393

	int data = state | ((m_counter[Which] & 7) << 1); // combine output from 8253 with counter bits 0-3
	if (m_counter[Which] & 8) // counter bit 4 XORs the data (7486 x 6)
		data ^= 0x0f;
	m_dac[Which]->write(data); // 4 resistor packs:  47k, 100k, 220k, 470k

	m_cnt_out[Which] = state;
}

void laserbas_state::laserbas_memory(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0xbfff).bankrw(m_vrambank);
	map(0xc000, 0xf7ff).rom().nopw();
	map(0xf800, 0xfbff).r(FUNC(laserbas_state::z1_r)).nopw(); /* protection device */
	map(0xfc00, 0xffff).ram();
}

void laserbas_state::laserbas_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w("crtc", FUNC(mc6845_device::address_w));
	map(0x01, 0x01).w("crtc", FUNC(mc6845_device::register_w));
	map(0x10, 0x10).w(FUNC(laserbas_state::videoctrl1_w));
	map(0x11, 0x11).w(FUNC(laserbas_state::videoctrl2_w));
	map(0x20, 0x20).portr("DSW");
	map(0x21, 0x21).portr("INPUTS");
	map(0x22, 0x22).r(FUNC(laserbas_state::track_dir_r));
	map(0x23, 0x23).r(FUNC(laserbas_state::track_val_r));
	map(0x20, 0x23).w(FUNC(laserbas_state::out_w));
	map(0x40, 0x43).rw("pit0", FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x44, 0x47).rw("pit1", FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x80, 0x9f).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
}

static INPUT_PORTS_START( laserbas )
	PORT_START("DSW")   // $20
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Cabinet ) )      PORT_CONDITION("INPUTS", 0x20, EQUALS, 0x20)
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )      PORT_CONDITION("INPUTS", 0x20, EQUALS, 0x20)
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )     PORT_CONDITION("INPUTS", 0x20, EQUALS, 0x20)
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Difficulty ) )   PORT_CONDITION("INPUTS", 0x20, EQUALS, 0x20)
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )         PORT_CONDITION("INPUTS", 0x20, EQUALS, 0x20)
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ) )       PORT_CONDITION("INPUTS", 0x20, EQUALS, 0x20)
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Bonus_Life ) )   PORT_CONDITION("INPUTS", 0x20, EQUALS, 0x20)
	PORT_DIPSETTING(    0x04, "10k" )                   PORT_CONDITION("INPUTS", 0x20, EQUALS, 0x20)
	PORT_DIPSETTING(    0x00, "30k" )                   PORT_CONDITION("INPUTS", 0x20, EQUALS, 0x20)
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) )  PORT_CONDITION("INPUTS", 0x20, EQUALS, 0x20)
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )          PORT_CONDITION("INPUTS", 0x20, EQUALS, 0x20)
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )           PORT_CONDITION("INPUTS", 0x20, EQUALS, 0x20)
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_B ) )       PORT_CONDITION("INPUTS", 0x20, EQUALS, 0x20)
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) )        PORT_CONDITION("INPUTS", 0x20, EQUALS, 0x20)
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )        PORT_CONDITION("INPUTS", 0x20, EQUALS, 0x20)
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )        PORT_CONDITION("INPUTS", 0x20, EQUALS, 0x20)
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )    PORT_CONDITION("INPUTS", 0x20, EQUALS, 0x20)
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coin_A ) )       PORT_CONDITION("INPUTS", 0x20, EQUALS, 0x20)
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )        PORT_CONDITION("INPUTS", 0x20, EQUALS, 0x20)
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )        PORT_CONDITION("INPUTS", 0x20, EQUALS, 0x20)
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_3C ) )        PORT_CONDITION("INPUTS", 0x20, EQUALS, 0x20)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )        PORT_CONDITION("INPUTS", 0x20, EQUALS, 0x20)
	// To do: split into separate switches (easier to debug as it is now though)
	PORT_DIPNAME( 0xff, 0xfe, "Service Mode Test" )     PORT_CONDITION("INPUTS", 0x20, EQUALS, 0x00)
	PORT_DIPSETTING(    0xfe, "S RAM CHECK" )           PORT_CONDITION("INPUTS", 0x20, EQUALS, 0x00)
	PORT_DIPSETTING(    0xfd, "D RAM CHECK F" )         PORT_CONDITION("INPUTS", 0x20, EQUALS, 0x00)
	PORT_DIPSETTING(    0xfb, "D RAM CHECK B" )         PORT_CONDITION("INPUTS", 0x20, EQUALS, 0x00)
	PORT_DIPSETTING(    0xf7, "ROM CHECK" )             PORT_CONDITION("INPUTS", 0x20, EQUALS, 0x00)
	PORT_DIPSETTING(    0xef, "CRT INVERT CHECK" )      PORT_CONDITION("INPUTS", 0x20, EQUALS, 0x00)    // press start 2
	PORT_DIPSETTING(    0xdf, "SWITCH CHECK" )          PORT_CONDITION("INPUTS", 0x20, EQUALS, 0x00)
	PORT_DIPSETTING(    0xbf, "COLOR CHECK" )           PORT_CONDITION("INPUTS", 0x20, EQUALS, 0x00)
	PORT_DIPSETTING(    0x7f, "SOUND CHECK" )           PORT_CONDITION("INPUTS", 0x20, EQUALS, 0x00)

	PORT_START("INPUTS")    // $21
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE( 0x20, IP_ACTIVE_LOW )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )   // service coin

	PORT_START("TRACK_X")
	PORT_BIT( 0x03f, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(30) PORT_KEYDELTA(20)

	PORT_START("TRACK_Y")
	PORT_BIT( 0x03f, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(30) PORT_KEYDELTA(20)
INPUT_PORTS_END

#define CLOCK 16680000
#define PIT_CLOCK (CLOCK/16) // 12 divider ?

void laserbas_state::laserbas(machine_config &config)
{
	Z80(config, m_maincpu, CLOCK / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &laserbas_state::laserbas_memory);
	m_maincpu->set_addrmap(AS_IO, &laserbas_state::laserbas_io);
	TIMER(config, "scantimer").configure_scanline(FUNC(laserbas_state::laserbas_scanline), "screen", 0, 1);

	/* TODO: clocks aren't known */
	pit8253_device &pit0(PIT8253(config, "pit0", 0));
	pit0.set_clk<0>(PIT_CLOCK);
	pit0.set_clk<1>(PIT_CLOCK);
	pit0.set_clk<2>(PIT_CLOCK);
	pit0.out_handler<0>().set(FUNC(laserbas_state::pit_out_w<0>));
	pit0.out_handler<1>().set(FUNC(laserbas_state::pit_out_w<1>));
	pit0.out_handler<2>().set(FUNC(laserbas_state::pit_out_w<2>));

	pit8253_device &pit1(PIT8253(config, "pit1", 0));
	pit1.set_clk<0>(PIT_CLOCK);
	pit1.set_clk<1>(PIT_CLOCK);
	pit1.set_clk<2>(PIT_CLOCK);
	pit1.out_handler<0>().set(FUNC(laserbas_state::pit_out_w<3>));
	pit1.out_handler<1>().set(FUNC(laserbas_state::pit_out_w<4>));
	pit1.out_handler<2>().set(FUNC(laserbas_state::pit_out_w<5>));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(6000000, 360, 0, 256, 274, 0, 224);
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	mc6845_device &crtc(MC6845(config, "crtc", 3000000/4)); /* unknown clock, hand tuned to get ~60 fps */
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);
	crtc.set_update_row_callback(FUNC(laserbas_state::crtc_update_row));

	PALETTE(config, m_palette).set_format(palette_device::RGB_332, 32);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	for (auto &dac : m_dac)
		DAC_4BIT_R2R(config, dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.16);
}

/*
Amstar LaserBase 1981 (Hoei)

XBC-101-00-1 - CPU board
  Z80A (D780C-1)
  2 NB5K8253
  2 2114 Rams
  8 Dipswitches

XBC-102-01-1 - RAM board?
  HD46505R
  32 MB8118 Rams

------------------------------------

Filename  Label Type   CSum Description
--------- ----- ------ ---- -----------------------------
MB8532.1    1   2532   9316
MB8532.2    2   2532   5662
MB8532.3    3   2532   7E9F
MB8532.4    4   2532   7CAD
MB8532.5    5   2532   C7D4 (Marked F.F.)
MB8532.6    6   2532   16BE
MB8532.7    7   2532   CF6C (Marked F.F.)
MB8716.8    8   2716   9601 (Marked F.F.)
TI2716.Z1  Z1   TI2716 D925
--------- ----- ------ ---- -----------------------------

I believe the F.F. markings on these chips show that
these roms have been changed to Future Flash.

It is unknown what the Z1 chip is, but the label screened
on the board under the socket says 2716.  All the identifying
numbers have been scratched off and has Z1 stamped on it.
It appears that each one was then numbered by hand in red
marker and stamped with white ink with Z1.

The Z1 chip was read from 3 different boards, it is Valid.
The chips were numbered 69, 82 & 624, all three read the same.
Turns out it was a TI2716.  The TI chip has A10 on a different
pin than a standard 2716.

*/

ROM_START( laserbas )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mb8532.1",   0x0000, 0x1000, CRC(ef85e0c5) SHA1(c26da919c206a23eb6e53ffe39acd5a5dfd47c18) )
	ROM_LOAD( "mb8532.2",   0x1000, 0x1000, CRC(0ba2236c) SHA1(416e4be957c395b05537d2e513e0c4561d8ca7c5) )
	ROM_LOAD( "mb8532.3",   0x2000, 0x1000, CRC(83998e0b) SHA1(ac435fb8dd67aec0737d6c750c527841b2b91a5b) )
	ROM_LOAD( "mb8532.4",   0x3000, 0x1000, CRC(00f9d909) SHA1(90b800cc5fcea53454584f8ad93eebbd193bdd21) )
	ROM_LOAD( "lb2532.5",   0xc000, 0x1000, CRC(6459073e) SHA1(78b8a23534826dd2d3b3c6c5d5708c8a78a4b6bf) )
	ROM_LOAD( "lb2532.6",   0xd000, 0x1000, CRC(a2dc1e7e) SHA1(78643a3aa852c73dab12e09a6cfc53141c936d12) )
	ROM_LOAD( "mb8532.7",   0xe000, 0x1000, CRC(9d2148d7) SHA1(24954d82a09d9fcfdc61e91b7c824daa5dd701c3) )
	ROM_LOAD( "mb8516.8",   0xf000, 0x0800, CRC(623f558f) SHA1(be6c6565df658555f21c43a8c2459cf399794a84) )
ROM_END

ROM_START( laserbasa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2732.u1",       0x0000, 0x1000, CRC(f3ab00dc) SHA1(4730b13b55c93c71ed483463e180e71e506cfbd6) )
	ROM_LOAD( "2732.u2",       0x1000, 0x1000, CRC(0ba2236c) SHA1(416e4be957c395b05537d2e513e0c4561d8ca7c5) )
	ROM_LOAD( "mb8532.u3",     0x2000, 0x1000, CRC(c58a7745) SHA1(382e2235d89520860335c1c2760339e116c0466f) )
	ROM_LOAD( "mbm2732.u4",    0x3000, 0x1000, CRC(00f9d909) SHA1(90b800cc5fcea53454584f8ad93eebbd193bdd21) )
	ROM_LOAD( "2732.u5",       0xc000, 0x1000, CRC(6459073e) SHA1(78b8a23534826dd2d3b3c6c5d5708c8a78a4b6bf) )
	ROM_LOAD( "2732.u6",       0xd000, 0x1000, CRC(a2dc1e7e) SHA1(78643a3aa852c73dab12e09a6cfc53141c936d12) )
	ROM_LOAD( "2732.u7",       0xe000, 0x1000, CRC(9d2148d7) SHA1(24954d82a09d9fcfdc61e91b7c824daa5dd701c3) )
	ROM_LOAD( "mb8516.u8",     0xf000, 0x0800, CRC(623f558f) SHA1(be6c6565df658555f21c43a8c2459cf399794a84) )
ROM_END

/*
It was unclear what type of device FF.9 was. The silkscreen on the PCB said 2716,
but the device is a masked ROM with its identifying marks rubbed off.
I dumped it as a 2716 (FF.9), a 2532 like the others (FF.9A) and a 2732 (FF.9B).
*/

ROM_START( futflash )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ff.1",         0x0000, 0x1000, CRC(bcd6b998) SHA1(4a210c40ce6015e2b921558b7571b7f2a27e1815) )
	ROM_LOAD( "ff.2",         0x1000, 0x1000, CRC(1b1f6953) SHA1(8cd7b7e2236700ce63c60b4d2286099c8091bdbd) )
	ROM_LOAD( "ff.3",         0x2000, 0x1000, CRC(30008f04) SHA1(e03b2dbcb6d2615650cdd47ecf1d587906ce149b) )
	ROM_LOAD( "ff.4",         0x3000, 0x1000, CRC(e559aa12) SHA1(0fecfb60b0147e8060c640f684f69503478200ff) )
	ROM_LOAD( "ff.5",         0xc000, 0x1000, CRC(6459073e) SHA1(78b8a23534826dd2d3b3c6c5d5708c8a78a4b6bf) )
	ROM_LOAD( "ff.6",         0xd000, 0x1000, CRC(a8b17f49) SHA1(aea349bd19d001233bfb1805e586c950275010b4) )
	ROM_LOAD( "ff.7",         0xe000, 0x1000, CRC(9d2148d7) SHA1(24954d82a09d9fcfdc61e91b7c824daa5dd701c3) )
	ROM_LOAD( "ff.8",         0xf000, 0x0800, CRC(623f558f) SHA1(be6c6565df658555f21c43a8c2459cf399794a84) )
ROM_END

} // anonymous namespace

GAME( 1980, futflash,  0,        laserbas, laserbas, laserbas_state, empty_init, ROT270, "Hoei",                  "Future Flash",        MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1981, laserbas,  futflash, laserbas, laserbas, laserbas_state, empty_init, ROT270, "Hoei (Amstar license)", "Laser Base (set 1)",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1981, laserbasa, futflash, laserbas, laserbas, laserbas_state, empty_init, ROT270, "Hoei (Amstar license)", "Laser Base (set 2)",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
