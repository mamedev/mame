// license:BSD-3-Clause
// copyright-holders: Tomasz Slanina, Morten Shearman Kirkegaard, Samuel Neves, Peter Wilhelmsen

/********************************************
 Laser Base / Future Flash driver

============================================
TODO:
- Video: weird palette changes, Laserbase colors, missing bg scrolling between stages (CRT address lines + m_hset ( or m_vset ?))
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
#include "sound/volt_reg.h"
#include "video/mc6845.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

class laserbas_state : public driver_device
{
public:
	laserbas_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_palette(*this, "palette"),
		m_dac(*this, "dac%u", 1U)
	{ }

	void laserbas(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	/* misc */
	int m_dac_data;
	int m_counter[6];
	int m_cnt_out[6];
	int m_nmi;
	/* video-related */
	int      m_vrambank;
	uint8_t  m_vram[0x10000];
	int m_hset, m_vset;
	int m_bset;
	int m_scl;
	bool     m_flipscreen;
	uint64_t m_z1data;

	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
	required_device_array<dac_byte_interface, 6> m_dac;

	DECLARE_READ8_MEMBER(vram_r);
	DECLARE_WRITE8_MEMBER(vram_w);
	DECLARE_WRITE8_MEMBER(videoctrl_w);
	DECLARE_READ8_MEMBER(z1_r);
	DECLARE_READ8_MEMBER(track_lo_r);
	DECLARE_READ8_MEMBER(track_hi_r);
	DECLARE_WRITE8_MEMBER(out_w);
	template<uint8_t Which> DECLARE_WRITE_LINE_MEMBER(pit_out_w);
	TIMER_DEVICE_CALLBACK_MEMBER(laserbas_scanline);
	MC6845_UPDATE_ROW(crtc_update_row);

	void laserbas_io(address_map &map);
	void laserbas_memory(address_map &map);
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
	int x = 0;
	int x_max = 0x100;
	int dx = 1;

	if (m_flipscreen)
	{
		y = 0xdf - y;
		x = 0xff;
		x_max = -1;
		dx = -1;
	}

	int pixaddr = y << 8;
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	uint32_t *b = &bitmap.pix32(y);

	while (x != x_max)
	{
		int offset = (pixaddr >> 1) & 0x7fff;
		int shift = (pixaddr & 1) * 4; // two 4 bit pixels in one byte
		int p1 = (m_vram[offset] >> shift) & 0xf;
		int p2 = (m_vram[offset + 0x8000] >> shift) & 0xf; // 0x10000 VRAM, two 4 bit layers 0x8000 bytes each
		int p;

		if (p2)
			p = p2;
		else if (p1)
			p = p1 + 16;
		else
			p = m_bset;

		b[x] = palette[p];

		pixaddr++;
		x += dx;
	}
}

READ8_MEMBER(laserbas_state::vram_r)
{
	return m_vram[offset+(m_vrambank?0x8000:0)];
}

WRITE8_MEMBER(laserbas_state::vram_w)
{
	m_vram[offset+(m_vrambank?0x8000:0)] = data;
}

WRITE8_MEMBER(laserbas_state::videoctrl_w)
{
	if (!(offset&1))
	{
		m_vrambank = data & 0x40; // layer select
		m_flipscreen = !(data & 0x80);
		m_vset = (data>>3)&7; // inc-ed on interrupts ( 8 ints / frame ?)
		m_hset = data&7;
	}
	else
	{
		data^=0xff;
		m_bset = data>>4; // bg pen
		m_scl = (data&8)>>3; // unknown
		m_nmi=data&1; // nmi enable (not on schematics, traced)
	}
}

READ8_MEMBER(laserbas_state::z1_r)
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

READ8_MEMBER(laserbas_state::track_lo_r)
{
	uint8_t dx = ioport("TRACK_X")->read();
	uint8_t dy = ioport("TRACK_Y")->read();
	if (dx & 0x10)
		dx ^= 0xf;
	if (dy & 0x10)
		dy ^= 0x0f;
	int data = (dx & 0x0f) | ((dy & 0x0f) << 4);
	return data;
}

READ8_MEMBER(laserbas_state::track_hi_r)
{
	int data =   ((ioport("TRACK_X")->read() & 0x10) >> 4) | ((ioport("TRACK_Y")->read() & 0x10) >> 3);
	return data;
}

WRITE8_MEMBER(laserbas_state::out_w)
{
	/* sound related , maybe also lamps */
}

void laserbas_state::machine_start()
{
	save_item(NAME(m_vram));
	save_item(NAME(m_flipscreen));
	save_item(NAME(m_vrambank));
	save_item(NAME(m_hset));
	save_item(NAME(m_vset));
	save_item(NAME(m_bset));
	save_item(NAME(m_scl));
	save_item(NAME(m_nmi));
	save_item(NAME(m_dac_data));
	save_item(NAME(m_counter));
	save_item(NAME(m_cnt_out));
	save_item(NAME(m_z1data));
}

void laserbas_state::machine_reset()
{
	m_vrambank = 0;
	m_flipscreen = false;
	m_nmi=0;
	m_bset = 0;
	m_hset = 0;
	m_vset = 0;
	m_scl = 0;
}

template<uint8_t Which>
WRITE_LINE_MEMBER(laserbas_state::pit_out_w)
{
	state^=1; // 7404  (6G)
	if((!state)& m_cnt_out[Which]){ // 0->1 rising edge CLK
		m_counter[Which] = (m_counter[Which]+1)&0x0f; // 4 bit counters 74393
	}
	int data =(state) | ((m_counter[Which]&7)<<1); // combine output from 8253 with counter bits 0-3
	data<<=4;
	if(m_counter[Which]&8) data^=0x0f; // counter bit 4 xors the data ( 7486 x 6)
	m_dac[Which]->write(data); // 4 resistor packs :  47k, 100k, 220k, 470k

	m_cnt_out[Which]=state;
}

void laserbas_state::laserbas_memory(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0xbfff).rw(FUNC(laserbas_state::vram_r), FUNC(laserbas_state::vram_w));
	map(0xc000, 0xf7ff).rom().nopw();
	map(0xf800, 0xfbff).r(FUNC(laserbas_state::z1_r)).nopw(); /* protection device */
	map(0xfc00, 0xffff).ram();
}

void laserbas_state::laserbas_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w("crtc", FUNC(mc6845_device::address_w));
	map(0x01, 0x01).w("crtc", FUNC(mc6845_device::register_w));
	map(0x10, 0x11).w(FUNC(laserbas_state::videoctrl_w));
	map(0x20, 0x20).portr("DSW");
	map(0x21, 0x21).portr("INPUTS");
	map(0x22, 0x22).r(FUNC(laserbas_state::track_hi_r));
	map(0x23, 0x23).r(FUNC(laserbas_state::track_lo_r));
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
	PORT_BIT( 0x01f, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(30) PORT_KEYDELTA(20) PORT_RESET

	PORT_START("TRACK_Y")
	PORT_BIT( 0x01f, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(30) PORT_KEYDELTA(20) PORT_RESET PORT_REVERSE
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
	screen.set_raw(4000000, 256, 0, 256, 256, 0, 256);   /* temporary, CRTC will configure screen */
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	mc6845_device &crtc(MC6845(config, "crtc", 3000000/4)); /* unknown clock, hand tuned to get ~60 fps */
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);
	crtc.set_update_row_callback(FUNC(laserbas_state::crtc_update_row));

	PALETTE(config, m_palette).set_format(palette_device::RGB_332, 32);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_4BIT_R2R(config, m_dac[0], 0).add_route(ALL_OUTPUTS, "speaker", 0.16);
	DAC_4BIT_R2R(config, m_dac[1], 0).add_route(ALL_OUTPUTS, "speaker", 0.16);
	DAC_4BIT_R2R(config, m_dac[2], 0).add_route(ALL_OUTPUTS, "speaker", 0.16);
	DAC_4BIT_R2R(config, m_dac[3], 0).add_route(ALL_OUTPUTS, "speaker", 0.16);
	DAC_4BIT_R2R(config, m_dac[4], 0).add_route(ALL_OUTPUTS, "speaker", 0.16);
	DAC_4BIT_R2R(config, m_dac[5], 0).add_route(ALL_OUTPUTS, "speaker", 0.16);
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref", 0));
	vref.add_route(0, "dac1", 1.0, DAC_VREF_POS_INPUT); vref.add_route(0, "dac1", -1.0, DAC_VREF_NEG_INPUT);
	vref.add_route(0, "dac2", 1.0, DAC_VREF_POS_INPUT); vref.add_route(0, "dac2", -1.0, DAC_VREF_NEG_INPUT);
	vref.add_route(0, "dac3", 1.0, DAC_VREF_POS_INPUT); vref.add_route(0, "dac3", -1.0, DAC_VREF_NEG_INPUT);
	vref.add_route(0, "dac4", 1.0, DAC_VREF_POS_INPUT); vref.add_route(0, "dac4", -1.0, DAC_VREF_NEG_INPUT);
	vref.add_route(0, "dac5", 1.0, DAC_VREF_POS_INPUT); vref.add_route(0, "dac5", -1.0, DAC_VREF_NEG_INPUT);
	vref.add_route(0, "dac6", 1.0, DAC_VREF_POS_INPUT); vref.add_route(0, "dac6", -1.0, DAC_VREF_NEG_INPUT);
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
It was unclear what type of device FF.9 was. The silkscreen on the PCB said
2716,
but the device is a masked ROM with its identifying marks rubbed off.
I dumped it
as a 2716 (FF.9), a 2532 like the others (FF.9A) and a 2732 (FF.9B).
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

GAME( 1980, futflash,  0,        laserbas, laserbas, laserbas_state, empty_init, ROT270, "Hoei",                  "Future Flash",        MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1981, laserbas,  futflash, laserbas, laserbas, laserbas_state, empty_init, ROT270, "Hoei (Amstar license)", "Laser Base (set 1)",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1981, laserbasa, futflash, laserbas, laserbas, laserbas_state, empty_init, ROT270, "Hoei (Amstar license)", "Laser Base (set 2)",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
