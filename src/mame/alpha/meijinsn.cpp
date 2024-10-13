// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina
/*
 Meijinsen (snk/alpha)
 ---------------------
 driver by Tomasz Slanina

It's something between typical alpha 68k hardware (alpha mcu, sound hw (same as in jongbou))
and old alpha shougi hardware (framebuffer).

There's probably no upright cabinet, only cocktail table (controls in 2p mode are inverted).

TODO:
- protection simulation isn't right, there is a problem on the selection screen,
  it's usually not possible to choose tsume shogi (3 difficulty levels)

Buttons:
 1st = 'decision'
 2nd = 'promotion'

 Service switch  = memory clear

--------------------------------------------------



                        p8      p7
   16mhz                p6      p5
                5816    p4      p3
                5816    p2      p1
                 ?
                          68000-8

        4416 4416 4416 4416
             clr             8910
     z80 p9 p10 2016



5816 = Sony CXK5816-10L (Ram for video)
2016 = Toshiba TMM2016AP-10 (SRAM for sound)
4416 = TI TMS4416-15NL (DRAM for MC68000)
clr  = TI TBP18S030N (32*8 Bipolar PROM)
Z80  = Sharp LH0080A Z80A-CPU-D
8910 = GI AY-3-8910A (Sound chip)
?    = Chip with Surface Scratched ....

"0" "1" MC68000 Program ROMs:
p1  p2
p3  p4
p5  p6
p7  p8

P9  = Z80 Program
P10 = AY-3-8910A Sounds

Text inside P9:

ALPHA DENSHI CO.,LTD  JUNE / 24 / 1986  FOR
* SHOUGI * GAME USED  SOUND BOARD CONTROL
SOFT  PSG & VOICE  BY M.C & S.H

*/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/timer.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "video/resnet.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class meijinsn_state : public driver_device
{
public:
	meijinsn_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_videoram(*this, "videoram"),
		m_shared_ram(*this, "shared_ram"),
		m_coins(*this, "COINS"),
		m_dsw(*this, "DSW")
	{ }

	void meijinsn(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	uint16_t alpha_mcu_r(offs_t offset);
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(interrupt);

	void main_map(address_map &map) ATTR_COLD;
	void sound_io_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_shared_ptr<uint16_t> m_videoram;
	required_shared_ptr<uint16_t> m_shared_ram;
	required_ioport m_coins;
	required_ioport m_dsw;

	// misc
	uint8_t m_deposits1 = 0;
	uint8_t m_deposits2 = 0;
	uint8_t m_credits = 0;
	uint8_t m_coinvalue = 0;
	uint8_t m_mcu_latch = 0;
};


uint16_t meijinsn_state::alpha_mcu_r(offs_t offset)
{
	static const uint8_t coinage1[2][2] = {{1,1}, {1,2}};
	static const uint8_t coinage2[2][2] = {{1,5}, {2,1}};

	int source = m_shared_ram[offset];

	switch (offset)
	{
		case 0: // Dipswitch 2
			m_shared_ram[0] = (source & 0xff00) | m_dsw->read();
			return 0;

		case 0x22: // Coin value
			m_shared_ram[0x22] = (source & 0xff00) | (m_credits & 0x00ff);
			return 0;

		case 0x29: // Query microcontroller for coin insert
			m_credits = 0;

			if ((m_coins->read() & 0x3) == 3)
				m_mcu_latch = 0;

			if ((m_coins->read() & 0x1) == 0 && !m_mcu_latch)
			{
				m_shared_ram[0x29] = (source & 0xff00) | 0x22;  // coinA
				m_shared_ram[0x22] = (source & 0xff00) | 0x00;
				m_mcu_latch = 1;

				m_coinvalue = (~m_dsw->read()>>3) & 1;

				m_deposits1++;
				if (m_deposits1 == coinage1[m_coinvalue][0])
				{
					m_credits = coinage1[m_coinvalue][1];
					m_deposits1 = 0;
				}
				else
					m_credits = 0;
			}
			else if ((m_coins->read() & 0x2) == 0 && !m_mcu_latch)
			{
				m_shared_ram[0x29] = (source & 0xff00) | 0x22;  // coinA
				m_shared_ram[0x22] = (source & 0xff00) | 0x00;
				m_mcu_latch = 1;

				m_coinvalue = (~m_dsw->read() >> 3) & 1;

				m_deposits2++;
				if (m_deposits2 == coinage2[m_coinvalue][0])
				{
					m_credits = coinage2[m_coinvalue][1];
					m_deposits2 = 0;
				}
				else
					m_credits = 0;
			}
			else
			{
				m_shared_ram[0x29] = (source & 0xff00) | 0x22;
			}
			return 0;
	}
	return 0;
}



void meijinsn_state::main_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x080e00, 0x080fff).r(FUNC(meijinsn_state::alpha_mcu_r)).nopw();
	map(0x100000, 0x107fff).ram().share(m_videoram);
	map(0x180000, 0x180dff).ram();
	map(0x180e00, 0x180fff).ram().share(m_shared_ram);
	map(0x181000, 0x181fff).ram();
	map(0x1c0000, 0x1c0001).portr("P2");
	map(0x1a0000, 0x1a0001).portr("P1");
	map(0x1a0001, 0x1a0001).w("soundlatch", FUNC(generic_latch_8_device::write));
}

void meijinsn_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
}

void meijinsn_state::sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).w("aysnd", FUNC(ay8910_device::address_data_w));
	map(0x01, 0x01).r("aysnd", FUNC(ay8910_device::data_r));
	map(0x02, 0x02).w("soundlatch", FUNC(generic_latch_8_device::clear_w));
	map(0x04, 0x04).w("dac", FUNC(dac_6bit_r2r_device::write));
	map(0x06, 0x06).nopw();
}

static INPUT_PORTS_START( meijinsn )
	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x7cc0, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_SERVICE )

	PORT_START("P2")
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0xc0ff, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x07, 0x00, "Game time (actual game)" )
	PORT_DIPSETTING(    0x07, "1:00" )
	PORT_DIPSETTING(    0x06, "2:00" )
	PORT_DIPSETTING(    0x05, "3:00" )
	PORT_DIPSETTING(    0x04, "4:00" )
	PORT_DIPSETTING(    0x03, "5:00" )
	PORT_DIPSETTING(    0x02, "10:00" )
	PORT_DIPSETTING(    0x01, "20:00" )
	PORT_DIPSETTING(    0x00, "0:30" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x08, "A 1C/1C B 1C/5C" )
	PORT_DIPSETTING(    0x00, "A 1C/2C B 2C/1C" )
	PORT_DIPNAME( 0x10, 0x00, "2 Player" )
	PORT_DIPSETTING(    0x00, "1C" )
	PORT_DIPSETTING(    0x10, "2C" )
	PORT_DIPNAME( 0x20, 0x00, "Game time (tsumeshougi)" )
	PORT_DIPSETTING(    0x20, "1:00" )
	PORT_DIPSETTING(    0x00, "2:00" )

	PORT_START("COINS")  // Coin input to microcontroller
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
INPUT_PORTS_END


void meijinsn_state::palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();
	static const int resistances_b[2]  = { 470, 220 };
	static const int resistances_rg[3] = { 1000, 470, 220 };

	double weights_r[3], weights_g[3], weights_b[2];
	compute_resistor_weights(0, 255, -1.0,
			3, resistances_rg, weights_r, 0, 1000+1000,
			3, resistances_rg, weights_g, 0, 1000+1000,
			2, resistances_b,  weights_b, 0, 1000+1000);

	for (int i = 0; i < palette.entries(); i++)
	{
		// red component
		int bit0 = BIT(color_prom[i], 0);
		int bit1 = BIT(color_prom[i], 1);
		int bit2 = BIT(color_prom[i], 2);
		int const r = combine_weights(weights_r, bit0, bit1, bit2);

		// green component
		bit0 = BIT(color_prom[i], 3);
		bit1 = BIT(color_prom[i], 4);
		bit2 = BIT(color_prom[i], 5);
		int const g = combine_weights(weights_g, bit0, bit1, bit2);

		// blue component
		bit0 = BIT(color_prom[i], 6);
		bit1 = BIT(color_prom[i], 7);
		int const b = combine_weights(weights_b, bit0, bit1);

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}


uint32_t meijinsn_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int offs = 0; offs < 0x4000; offs++)
	{
		int sx = offs >> 8;
		int sy = offs & 0xff;

		int data1 = m_videoram[offs] >> 8;
		int data2 = m_videoram[offs] & 0xff;

		for (int x = 0; x < 4; x++)
		{
			int color= BIT(data1, x) | (BIT(data1, x + 4) << 1);
			int data = BIT(data2, x) | (BIT(data2, x + 4) << 1);
			bitmap.pix(sy, (sx * 4 + (3 - x))) = color * 4 + data;
		}
	}
	return 0;
}


TIMER_DEVICE_CALLBACK_MEMBER(meijinsn_state::interrupt)
{
	int scanline = param;

	if(scanline == 240)
		m_maincpu->set_input_line(1, HOLD_LINE);

	if(scanline == 0)
		m_maincpu->set_input_line(2, HOLD_LINE);
}

void meijinsn_state::machine_start()
{
	save_item(NAME(m_deposits1));
	save_item(NAME(m_deposits2));
	save_item(NAME(m_credits));
	save_item(NAME(m_coinvalue));
	save_item(NAME(m_mcu_latch));
}

void meijinsn_state::machine_reset()
{
	m_deposits1 = 0;
	m_deposits2 = 0;
	m_credits   = 0;
	m_coinvalue = 0;
	m_mcu_latch = 0;
}


void meijinsn_state::meijinsn(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 16_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &meijinsn_state::main_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(meijinsn_state::interrupt), "screen", 0, 1);

	z80_device &audiocpu(Z80(config, "audiocpu", 16_MHz_XTAL / 4));
	audiocpu.set_addrmap(AS_PROGRAM, &meijinsn_state::sound_map);
	audiocpu.set_addrmap(AS_IO, &meijinsn_state::sound_io_map);
	audiocpu.set_periodic_int(FUNC(meijinsn_state::irq0_line_hold), attotime::from_hz(16_MHz_XTAL / 0x800));

	GENERIC_LATCH_8(config, "soundlatch");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(12, 243, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(meijinsn_state::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(meijinsn_state::palette), 32);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ay8910_device &aysnd(AY8910(config, "aysnd", 16_MHz_XTAL / 8));
	aysnd.port_a_read_callback().set("soundlatch", FUNC(generic_latch_8_device::read));
	aysnd.port_b_write_callback().set_nop(); // ?
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.25);

	DAC_6BIT_R2R(config, "dac").add_route(ALL_OUTPUTS, "mono", 0.5);
}


ROM_START( meijinsn )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "p1", 0x00000, 0x4000, CRC(8c9697a3) SHA1(19258e20a6aaadd6ba3469079fef85bc6dba548c) )
	ROM_CONTINUE(          0x20000, 0x4000 )
	ROM_LOAD16_BYTE( "p2", 0x00001, 0x4000, CRC(f7da3535) SHA1(fdbacd075d45abda782966b16b3ea1ad68d60f91) )
	ROM_CONTINUE(          0x20001, 0x4000 )
	ROM_LOAD16_BYTE( "p3", 0x08000, 0x4000, CRC(0af0b266) SHA1(d68ed31bc932bc5e9c554b2c0df06a751dc8eb96) )
	ROM_CONTINUE(          0x28000, 0x4000 )
	ROM_LOAD16_BYTE( "p4", 0x08001, 0x4000, CRC(aab159c5) SHA1(0c9cad8f9893f4080b498433068e9324c7f2e13c) )
	ROM_CONTINUE(          0x28001, 0x4000 )
	ROM_LOAD16_BYTE( "p5", 0x10000, 0x4000, CRC(0ed10a47) SHA1(9e89ec69f1f4e1ffa712f2e0c590d067c8c63026) )
	ROM_CONTINUE(          0x30000, 0x4000 )
	ROM_LOAD16_BYTE( "p6", 0x10001, 0x4000, CRC(60b58755) SHA1(1786fc1b4c6d1793fb8e9311356fa4119611cfae) )
	ROM_CONTINUE(          0x30001, 0x4000 )
	ROM_LOAD16_BYTE( "p7", 0x18000, 0x4000, CRC(604c76f1) SHA1(37fdf904f5e4d69dc8cb711cf3dece8f3075254a) )
	ROM_CONTINUE(          0x38000, 0x4000 )
	ROM_LOAD16_BYTE( "p8", 0x18001, 0x4000, CRC(e3eaef19) SHA1(b290922f252a790443109e5023c3c35b133275cc) )
	ROM_CONTINUE(          0x38001, 0x4000 )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "p9",  0x00000, 0x04000, CRC(aedfefdf) SHA1(f9d35737a0e942fe7d483f87c52efa92a1bbb3e5) )
	ROM_LOAD( "p10", 0x04000, 0x04000, CRC(93b4d764) SHA1(4fedd3fd1f3ef6c5f60ca86219f877df68d3027d) )

	ROM_REGION( 0x20, "proms", 0 ) // Colour PROM
	ROM_LOAD( "clr", 0x00, 0x20, CRC(7b95b5a7) SHA1(c15be28bcd6f5ffdde659f2d352ae409f04b2557) )
ROM_END

ROM_START( meijinsna ) // ROMs with location were in the archive, the others not, but they pass the ROM check so probably good and the dumper only included ROMs that differed from the set in MAME
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "p1.e12", 0x00000, 0x4000, CRC(fddea817) SHA1(497c5a197c53d0fea2eb2ef62a93f56cd930bd5a) )
	ROM_CONTINUE(              0x20000, 0x4000 )
	ROM_LOAD16_BYTE( "p2.e10", 0x00001, 0x4000, CRC(f05659cc) SHA1(0f8d0da387329886903163333dbf0f36beb4198c) )
	ROM_CONTINUE(              0x20001, 0x4000 )
	ROM_LOAD16_BYTE( "p3.d12", 0x08000, 0x4000, CRC(906e9d49) SHA1(f52757317d441d0cf35cd3726ea8d4fe0d079c9b) )
	ROM_CONTINUE(              0x28000, 0x4000 )
	ROM_LOAD16_BYTE( "p4.d10", 0x08001, 0x4000, CRC(efa31978) SHA1(dadf226b993ecbac3112b7b0ce5047f0d686866e) )
	ROM_CONTINUE(              0x28001, 0x4000 )
	ROM_LOAD16_BYTE( "p5",     0x10000, 0x4000, BAD_DUMP CRC(0ed10a47) SHA1(9e89ec69f1f4e1ffa712f2e0c590d067c8c63026) )
	ROM_CONTINUE(              0x30000, 0x4000 )
	ROM_LOAD16_BYTE( "p6",     0x10001, 0x4000, BAD_DUMP CRC(60b58755) SHA1(1786fc1b4c6d1793fb8e9311356fa4119611cfae) )
	ROM_CONTINUE(              0x30001, 0x4000 )
	ROM_LOAD16_BYTE( "p7",     0x18000, 0x4000, BAD_DUMP CRC(604c76f1) SHA1(37fdf904f5e4d69dc8cb711cf3dece8f3075254a) )
	ROM_CONTINUE(              0x38000, 0x4000 )
	ROM_LOAD16_BYTE( "p8",     0x18001, 0x4000, BAD_DUMP CRC(e3eaef19) SHA1(b290922f252a790443109e5023c3c35b133275cc) )
	ROM_CONTINUE(              0x38001, 0x4000 )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "p9",  0x00000, 0x04000, BAD_DUMP CRC(aedfefdf) SHA1(f9d35737a0e942fe7d483f87c52efa92a1bbb3e5) )
	ROM_LOAD( "p10", 0x04000, 0x04000, BAD_DUMP CRC(93b4d764) SHA1(4fedd3fd1f3ef6c5f60ca86219f877df68d3027d) )

	ROM_REGION( 0x20, "proms", 0 ) // Colour PROM
	ROM_LOAD( "clr", 0x00, 0x20, BAD_DUMP CRC(7b95b5a7) SHA1(c15be28bcd6f5ffdde659f2d352ae409f04b2557) )
ROM_END

} // Anonymous namespace


GAME( 1986, meijinsn,  0,        meijinsn, meijinsn, meijinsn_state, empty_init, ROT0, "SNK", "Meijinsen (set 1)", MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE )
GAME( 1986, meijinsna, meijinsn, meijinsn, meijinsn, meijinsn_state, empty_init, ROT0, "SNK", "Meijinsen (set 2)", MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE )
