// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli, Bryan McPhail, Stephane Humbert, Angelo Salese
/***************************************************************************

    SNK/Alpha 68000 N based board games

    derived from alpha68k.cpp

    TODO:
    - Super Stingray MCU irq controls timer speed. The MCU has been
          hooked up but the clock is almost certainly wrong.
    - GFX region can eventually overflow in jongbou, and in general all three
      games can probably be run with the same gfx_layout structs

============================================================================

Kyros no Yakata
Alpha Denshi, 1986

PCB Layout
----------
Main Board

(no PCB number)
|----------------------------------------------------------|
|                                                          |
| 0.1T                                              24MHz  |
|                                                          |
| 15.3T                  5814                              |
|         PROMG.4R                             SW1  |   | 1|
| 16.5T   PROMR.5R PROMH.5P 5814                    |   | 8|
|         PROMB.6R PROML.6P                         |   | W|
| 17.7T                                        8511 |   | A|
|                             2016 2016             |   | Y|
| 18.9T   9.9S 8.9R           2016 2016             |   |  |
|                                                   |   |  |
| 19.11T                            68000 2016 2.10C 4.10A |
|              14.12R  12.12N             2016             |
| 20.13T           13.12P  11.12M              1.13C 3.13A |
|CN                                                        |
|----------------------------------------------------------|
Notes:
         68000 clock - 6.000MHz [24/4]
                2016 - 2kx8 SRAM
                5814 - 2kx8 SRAM
                  CN - 18-pin flat cable connector joining to sound PCB
                   | - ALPHA-INPUT84 custom ceramic module (x2)
                 SW1 - 8-position DIP Switch
               VSync - 60Hz
               HSync - 15.20kHz
                8511 - Alpha 8511 Microcontroller at location 7C. '8511' is silkscreened on the PCB.
                       Clock input 3.000MHz [24/8] (measured on pin 5).
                       The chip is pin-compatible with Motorola MC68705U3, Motorola MC6805U2
                       and Hitachi HD6805U1. The 4k MC68705U3 dump in MAME is from a bootleg PCB.

Note from Guru: The bootleg 4k MCU dump was written to a genuine
Motorola MC68705U3 microcontroller and tested on the original Alpha
Denshi Kyros no Yakata PCB and works. Since the bootleg PCB is
visually the same this suggests the bootleggers copied the PCB 1:1
including the HD6805U1 MCU data then adapted it for the 68705U3 with
minimal changes.
*******************************
romcmp -d *.bin
Comparing 2 files....
kyros_68705u3.bin [3/4]      kyros_mcu.bin [1/2]      99.902344%
kyros_68705u3.bin [4/4]      kyros_mcu.bin [2/2]      88.183594%
*******************************

Sound Board
-----------

SOUND BOARD NO.60MC01
|---------------------|
| 40174   324  UPC1181|
| 4013     VOL  VOL   |
| YM3014  324        6|
|                    W|
| YM2203  AY-3-8910  A|
|                    Y|
|         AY-3-8910   |
|                     |
| DIP28               |
|                     |
| 2.1F 2114           |
|      2114           |
| 1.1D          16MHz |
|                     |
|  Z80                |
|CN                   |
|---------------------|
Notes:
      Z80 clock - 4.000MHz [16/4]. The actual chip is a NEC D780C-1
AY-3-8910 clock - 2.000MHz [16/8]
   YM2203 clock - 2.000MHz [16/8]
         YM3014 - Yamaha YM3014 DAC. Clock input 1.33333MHz [16/12]
        UPC1181 - Power AMP IC
            324 - LM324 Quad Op Amp
           2114 - 1kx4 SRAM
           40xx - 4000-series logic chips
             CN - 18-pin flat cable connector joining to main PCB
          DIP28 - Empty socket

PCB Pinout
----------

       18-WAY Connector (Main)
           Parts   Solder
         -----------------
          +5V A1   B1 +5V
          +5V A2   B2 +5V
        1P UP A3   B3 2P DOWN
      1P DOWN A4   B4 2P RIGHT
     1P RIGHT A5   B5 2P LEFT
      1P LEFT A6   B6 2P BUTTON 1
  1P BUTTON 1 A7   B7 -
  1P BUTTON 2 A8   B8 -
           -  A9   B9 COIN 1
    1P START A10   B10 COIN 2
       2P UP A11   B11 2P BUTTON 2
       GREEN A12   B12 -
        BLUE A13   B13 2P START
         RED A14   B14 -
        SYNC A15   B15 -
         GND A16   B16 GND
         GND A17   B17 GND
         GND A18   B18 GND

6-WAY Connector (Sound)
    Parts   Solder
  -----------------
  +12V A1   B1 +12V
     - A2   B2 -
   +5V A3   B3 +5V
  SPK+ A4   B4 SPK+
  SPK- A5   B5 GND
   GND A6   B6 GND


***************************************************************************/


#include "emu.h"
#include "includes/alpha68k.h"

/*
 *
 * Video Section
 *
 */

void alpha68k_N_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int c, int d)
{
	for (int offs = 0; offs < 0x400; offs += 0x20)
	{
		int mx = m_spriteram[offs + c];
		int my = -(mx >> 8) & 0xff;
		mx &= 0xff;

		// TODO: not convinced by this
		if (m_is_super_stingray && mx > 0xf8)
			mx -= 0x100;

		if (m_flipscreen)
			my = 249 - my;

		for (int i = 0; i < 0x20; i++)
		{
			const u16 data = m_spriteram[offs + d + i];
			// TODO: not convinced by this either, must be transmask instead of transpen somehow
			if (data != m_tile_transchar)
			{
				u8 color, bank;
				u16 tile;

				bank = data >> 10 & 3;
				tile = data & 0x3ff;
				if (m_is_super_stingray == true)
					color = (data >> 7 & 0x18) | (data >> 13 & 7);
				else
				{
					color = m_color_proms[(data >> 1 & 0x1000) | (data & 0xffc) | (data >> 14 & 3)];
					bank += ((data >> m_tile_bankshift) & 4);
					tile += (data >> 3 & 0x400);
				}

				// can't be 0xff in super stingray
				if (color != 0xff)
				{
					int fy = ((data & 0x1000) >> 12) ^ m_flipscreen;
					int fx = m_flipscreen;

					m_gfxdecode->gfx(bank)->transpen(bitmap,cliprect, tile, color, fx, fy, mx, my, 0);
				}
			}

			if (m_flipscreen)
				my = (my - 8) & 0xff;
			else
				my = (my + 8) & 0xff;
		}
	}
}

u32 alpha68k_N_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_palette->set_pen_indirect(0x100, *m_videoram & 0xff);
	bitmap.fill(0x100, cliprect);

	draw_sprites(bitmap, cliprect, 2, 0x0800);
	draw_sprites(bitmap, cliprect, 3, 0x0c00);
	draw_sprites(bitmap, cliprect, 1, 0x0400);
	return 0;
}

/*
 *
 * MCU simulation
 *
 */

u16 alpha68k_N_state::kyros_alpha_trigger_r(offs_t offset)
{
	/* possible jump codes:
	     - Kyros          : 0x22
	     - Super Stingray : 0x21,0x22,0x23,0x24,0x34,0x37,0x3a,0x3d,0x40,0x43,0x46,0x49
	*/
	static const u8 coinage1[8][2] = {{1,1}, {1,5}, {1,3}, {2,3}, {1,2}, {1,6}, {1,4}, {3,2}};
	static const u8 coinage2[8][2] = {{1,1}, {5,1}, {3,1}, {7,1}, {2,1}, {6,1}, {4,1}, {8,1}};
	const u16 source = m_shared_ram[offset];

	switch (offset)
	{
	case 0x22: /* Coin value */
		m_shared_ram[0x22] = (source & 0xff00) | (m_credits & 0x00ff);
		return 0;
	case 0x29: /* Query microcontroller for coin insert */
		m_trigstate++;
		if ((m_in[2]->read() & 0x3) == 3)
			m_latch = 0;
		if ((m_in[2]->read() & 0x1) == 0 && !m_latch)
		{
			m_shared_ram[0x29] = (source & 0xff00) | (m_coin_id & 0xff);    // coinA
			m_shared_ram[0x22] = (source & 0xff00) | 0x0;
			m_latch = 1;

			m_coinvalue = (~m_in[1]->read() >> 1) & 7;
			m_deposits1++;
			if (m_deposits1 == coinage1[m_coinvalue][0])
			{
				m_credits = coinage1[m_coinvalue][1];
				m_deposits1 = 0;
			}
			else
				m_credits = 0;
		}
		else if ((m_in[2]->read() & 0x2) == 0 && !m_latch)
		{
			m_shared_ram[0x29] = (source & 0xff00) | (m_coin_id >> 8);  // coinB
			m_shared_ram[0x22] = (source & 0xff00) | 0x0;
			m_latch = 1;

			m_coinvalue = (~m_in[1]->read() >>1 ) & 7;
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
			if (m_microcontroller_id == 0x00ff || m_game_id == ALPHA68K_JONGBOU)     /* Super Stingry */
			{
				if (m_trigstate >= 12 || m_game_id == ALPHA68K_JONGBOU) /* arbitrary value ! */
				{
					m_trigstate = 0;
					m_microcontroller_data = 0x21;          // timer
				}
				else
					m_microcontroller_data = 0x00;
			}
			else
				m_microcontroller_data = 0x00;

			m_shared_ram[0x29] = (source & 0xff00) | m_microcontroller_data;
		}
		return 0;
	case 0xff:  /* Custom check, only used at bootup */
		m_shared_ram[0xff] = (source & 0xff00) | m_microcontroller_id;
		break;
	}

	logerror("%04x:  Alpha read trigger at %04x\n", m_maincpu->pc(), offset);

	return 0; /* Values returned don't matter */
}

u8 sstingray_state::alpha8511_command_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
		m_alpha8511_sync_timer->adjust(attotime::zero, offset << 8 | 0x100ff);
	return 0;
}

void sstingray_state::alpha8511_command_w(offs_t offset, u8 data)
{
	m_alpha8511_sync_timer->adjust(attotime::zero, offset << 8 | data);
}

TIMER_CALLBACK_MEMBER(sstingray_state::alpha8511_sync)
{
	m_microcontroller_data = param & 0xff;
	m_alpha8511_address = (param >> 8) & 0xff;
	m_alpha8511_read_mode = BIT(param, 16);
	if (BIT(m_alpha8511_control, 4))
	{
		m_alpha8511->set_input_line(MCS48_INPUT_IRQ, ASSERT_LINE);
		m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	}
}

u8 sstingray_state::alpha8511_bus_r()
{
	return m_microcontroller_data;
}

void sstingray_state::alpha8511_bus_w(u8 data)
{
	m_microcontroller_data = data;
}

u8 sstingray_state::alpha8511_address_r()
{
	return m_alpha8511_address;
}

u8 sstingray_state::alpha8511_rw_r()
{
	return m_alpha8511_read_mode ? 0xff : 0xf7;
}

void sstingray_state::alpha8511_control_w(u8 data)
{
	if (!BIT(data, 4))
	{
		m_alpha8511->set_input_line(MCS48_INPUT_IRQ, CLEAR_LINE);
		m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
	}
	if (BIT(data, 5))
		m_maincpu->set_input_line(M68K_IRQ_2, HOLD_LINE);
	if (!BIT(data, 6))
		m_shared_ram[m_alpha8511_address] = (m_shared_ram[m_alpha8511_address] & 0xff00) | m_microcontroller_data;
	m_flipscreen = !BIT(data, 7);

	m_alpha8511_control = data;
}

u16 jongbou_state::dial_inputs_r()
{
	u8 inp1 = m_in[3]->read();
	u8 inp2 = m_in[4]->read();
	inp1 = ((inp1 & 0x01) << 3) + ((inp1 & 0x02) << 1) + ((inp1 & 0x04) >> 1) + ((inp1 & 0x08) >> 3);
	inp2 = ((inp2 & 0x01) << 3) + ((inp2 & 0x02) << 1) + ((inp2 & 0x04) >> 1) + ((inp2 & 0x08) >> 3);
	return m_in[0]->read() | inp1 | inp2 << 4;
}

/*
 *
 * Memory Maps
 *
 */

void sstingray_state::main_map(address_map &map)
{
	map(0x000000, 0x01ffff).rom();                       // main program
	map(0x020000, 0x020fff).ram().share("shared_ram");  // work RAM
	map(0x040000, 0x041fff).ram().share("spriteram"); // sprite RAM
	map(0x060000, 0x060001).ram().share("videoram");  // MSB: watchdog, LSB: BGC
	map(0x080000, 0x0801ff).rw(FUNC(sstingray_state::alpha8511_command_r), FUNC(sstingray_state::alpha8511_command_w)).umask16(0x00ff);
	map(0x0c0000, 0x0c0001).portr("IN0");
	map(0x0e0000, 0x0e0000).lr8(NAME([this] () -> u8 { return m_in[1]->read(); })).w(m_soundlatch, FUNC(generic_latch_8_device::write));
}

void alpha68k_N_state::main_map(address_map &map)
{
	map(0x000000, 0x01ffff).rom();                       // main program
	map(0x020000, 0x020fff).ram().share("shared_ram");  // work RAM
	map(0x040000, 0x041fff).ram().share("spriteram"); // sprite RAM
	map(0x060000, 0x060001).ram().share("videoram");  // MSB: watchdog, LSB: BGC
	map(0x080000, 0x0801ff).rw(FUNC(kyros_state::kyros_alpha_trigger_r), FUNC(kyros_state::alpha_microcontroller_w));
	map(0x0c0000, 0x0c0001).portr("IN0");
	map(0x0e0000, 0x0e0000).lr8(NAME([this] () -> u8 { return m_in[1]->read(); })).w(m_soundlatch, FUNC(generic_latch_8_device::write));
}

void jongbou_state::main_map(address_map &map)
{
	alpha68k_N_state::main_map(map);
	map(0x0c0000, 0x0c0001).r(FUNC(jongbou_state::dial_inputs_r));
}

void alpha68k_N_state::sound_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xc7ff).ram();
	map(0xe000, 0xe000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0xe002, 0xe002).w(m_soundlatch, FUNC(generic_latch_8_device::clear_w));
	map(0xe004, 0xe004).w("dac", FUNC(dac_byte_interface::data_w));
	map(0xe006, 0xe00e).nopw(); // soundboard I/O's, ignored
/* reference only
    map(0xe006, 0xe006).nopw(); // NMI: diminishing saw-tooth
    map(0xe008, 0xe008).nopw(); // NMI: 00
    map(0xe00a, 0xe00a).nopw(); // RST38: 20
    map(0xe00c, 0xe00c).nopw(); // RST30: 00 on entry
    map(0xe00e, 0xe00e).nopw(); // RST30: 00,02,ff on exit(0x1d88)
*/
}

void sstingray_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0xc100, 0xc100).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0xc102, 0xc102).w(m_soundlatch, FUNC(generic_latch_8_device::clear_w));
	map(0xc104, 0xc104).w("dac", FUNC(dac_byte_interface::data_w));
	map(0xc106, 0xc10e).nopw(); // soundboard I/O's, ignored
}

void alpha68k_N_state::sound_iomap(address_map &map)
{
	map.global_mask(0xff);
	map(0x10, 0x11).w("ym", FUNC(ym2203_device::write));
	map(0x80, 0x80).w("aysnd1", FUNC(ay8910_device::data_w));
	map(0x81, 0x81).w("aysnd1", FUNC(ay8910_device::address_w));
	map(0x90, 0x90).w("aysnd2", FUNC(ay8910_device::data_w));
	map(0x91, 0x91).w("aysnd2", FUNC(ay8910_device::address_w));
}

void jongbou_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x83ff).ram();
}

void jongbou_state::sound_iomap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w("aysnd", FUNC(ay8910_device::address_w));
	map(0x01, 0x01).rw("aysnd", FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
	map(0x02, 0x02).w(m_soundlatch, FUNC(generic_latch_8_device::clear_w));
	map(0x06, 0x06).nopw();
}

/*
 *
 * GFX definitions
 *
 */

static const gfx_layout sstingry_layout1 =
{
	8,8,    /* 8*8 chars */
	1024,
	3,      /* 3 bits per pixel */
	{ 4, 0+(0x10000*4), 4+(0x8000*8) },
	{ 8*8+3, 8*8+2, 8*8+1, 8*8+0, 3, 2, 1, 0 },
	{ STEP8(0,8) },
	16*8    /* every char takes 16 consecutive bytes */
};

static const gfx_layout sstingry_layout2 =
{
	8,8,    /* 8*8 chars */
	1024,
	3,      /* 3 bits per pixel */
	{ 0, 0+(0x28000*8), 4+(0x28000*8) },
	{ 8*8+3, 8*8+2, 8*8+1, 8*8+0, 3, 2, 1, 0 },
	{ STEP8(0,8) },
	16*8    /* every char takes 16 consecutive bytes */
};

static const gfx_layout sstingry_layout3 =
{
	8,8,    /* 8*8 chars */
	1024,
	3,      /* 3 bits per pixel */
	{ 0, 0+(0x10000*8), 4+(0x10000*8) },
	{ 8*8+3, 8*8+2, 8*8+1, 8*8+0, 3, 2, 1, 0 },
	{ STEP8(0,8) },
	16*8    /* every char takes 16 consecutive bytes */
};

static const gfx_layout kyros_char_layout1 =
{
	8,8,    /* 8*8 chars */
	0x8000/16,
	3,  /* 3 bits per pixel */
	{ 4,0x8000*8,0x8000*8+4 },
	{ 8*8+3, 8*8+2, 8*8+1, 8*8+0, 3, 2, 1, 0 },
	{ STEP8(0,8) },
	16*8    /* every char takes 16 consecutive bytes */
};

static const gfx_layout kyros_char_layout2 =
{
	8,8,    /* 8*8 chars */
	0x8000/16,
	3,  /* 3 bits per pixel */
	{ 0,0x10000*8,0x10000*8+4 },
	{ 8*8+3, 8*8+2, 8*8+1, 8*8+0, 3, 2, 1, 0 },
	{ STEP8(0,8) },
	16*8    /* every char takes 16 consecutive bytes */
};

static const gfx_layout jongbou_layout1 =
{
	8,8,    /* 8*8 chars */
	1024,
	3,      /* 3 bits per pixel */
	{ 4+0x00000*8, 0+0x8000*8, 4+0x8000*8 },
	{ 8*8+3, 8*8+2, 8*8+1, 8*8+0, 3, 2, 1, 0 },
	{ STEP8(0,8) },
	16*8    /* every char takes 16 consecutive bytes */
};

static const gfx_layout jongbou_layout2 =
{
	8,8,    /* 8*8 chars */
	1024,
	3,      /* 3 bits per pixel */
	{ 0+0x00000*8, 0+0x10000*8, 4+0x10000*8 },
	{ 8*8+3, 8*8+2, 8*8+1, 8*8+0, 3, 2, 1, 0 },
	{ STEP8(0,8) },
	16*8    /* every char takes 16 consecutive bytes */
};

static GFXDECODE_START( gfx_sstingry )
	GFXDECODE_ENTRY( "gfx1", 0x00000, sstingry_layout1,  0, 32 )
	GFXDECODE_ENTRY( "gfx1", 0x00000, sstingry_layout2,  0, 32 )
	GFXDECODE_ENTRY( "gfx1", 0x10000, sstingry_layout1,  0, 32 )
	GFXDECODE_ENTRY( "gfx1", 0x10000, sstingry_layout3,  0, 32 )
GFXDECODE_END

static GFXDECODE_START( gfx_kyros )
	GFXDECODE_ENTRY( "gfx1", 0x00000, kyros_char_layout1,  0, 32 )
	GFXDECODE_ENTRY( "gfx1", 0x00000, kyros_char_layout2,  0, 32 )
	GFXDECODE_ENTRY( "gfx1", 0x18000, kyros_char_layout1,  0, 32 )
	GFXDECODE_ENTRY( "gfx1", 0x18000, kyros_char_layout2,  0, 32 )
	GFXDECODE_ENTRY( "gfx1", 0x30000, kyros_char_layout1,  0, 32 )
	GFXDECODE_ENTRY( "gfx1", 0x30000, kyros_char_layout2,  0, 32 )
	GFXDECODE_ENTRY( "gfx1", 0x48000, kyros_char_layout1,  0, 32 )
	GFXDECODE_ENTRY( "gfx1", 0x48000, kyros_char_layout2,  0, 32 )
GFXDECODE_END

static GFXDECODE_START( gfx_jongbou )
	GFXDECODE_ENTRY( "gfx1", 0x00000, jongbou_layout1,  0, 32 )
	GFXDECODE_ENTRY( "gfx1", 0x00000, jongbou_layout2,  0, 32 )
	GFXDECODE_ENTRY( "gfx1", 0x18000, jongbou_layout1,  0, 32 )
	GFXDECODE_ENTRY( "gfx1", 0x18000, jongbou_layout2,  0, 32 )
	GFXDECODE_ENTRY( "gfx1", 0x04000, jongbou_layout1,  0, 32 )
	GFXDECODE_ENTRY( "gfx1", 0x04000, jongbou_layout2,  0, 32 )
	GFXDECODE_ENTRY( "gfx1", 0x1c000, jongbou_layout1,  0, 32 )
	GFXDECODE_ENTRY( "gfx1", 0x1c000, jongbou_layout2,  0, 32 )
GFXDECODE_END

/*
 *
 * Input Defs
 *
 */

#define ALPHA68K_PLAYER_INPUT_SWAP_LR_LSB( player, button3, start, active ) \
	PORT_BIT( 0x0001, active, IPT_JOYSTICK_UP    ) PORT_PLAYER(player) \
	PORT_BIT( 0x0002, active, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(player) \
	PORT_BIT( 0x0004, active, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(player) \
	PORT_BIT( 0x0008, active, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(player) \
	PORT_BIT( 0x0010, active, IPT_BUTTON1        ) PORT_PLAYER(player) \
	PORT_BIT( 0x0020, active, IPT_BUTTON2        ) PORT_PLAYER(player) \
	PORT_BIT( 0x0040, active, button3            ) PORT_PLAYER(player) \
	PORT_BIT( 0x0080, active, start )

#define ALPHA68K_PLAYER_INPUT_SWAP_LR_MSB( player, button3, start, active ) \
	PORT_BIT( 0x0100, active, IPT_JOYSTICK_UP    ) PORT_PLAYER(player) \
	PORT_BIT( 0x0200, active, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(player) \
	PORT_BIT( 0x0400, active, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(player) \
	PORT_BIT( 0x0800, active, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(player) \
	PORT_BIT( 0x1000, active, IPT_BUTTON1        ) PORT_PLAYER(player) \
	PORT_BIT( 0x2000, active, IPT_BUTTON2        ) PORT_PLAYER(player) \
	PORT_BIT( 0x4000, active, button3            ) PORT_PLAYER(player) \
	PORT_BIT( 0x8000, active, start )


static INPUT_PORTS_START( sstingry )
	PORT_START("IN0")
	ALPHA68K_PLAYER_INPUT_SWAP_LR_LSB( 1, IPT_UNKNOWN, IPT_START1, IP_ACTIVE_HIGH )
	ALPHA68K_PLAYER_INPUT_SWAP_LR_MSB( 2, IPT_UNKNOWN, IPT_START2, IP_ACTIVE_HIGH )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:!1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x0e, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:!2,!3,!4")
	PORT_DIPSETTING(    0x00, "A 1C/1C B 1C/1C" )
	PORT_DIPSETTING(    0x02, "A 1C/2C B 2C/1C" )
	PORT_DIPSETTING(    0x04, "A 1C/3C B 3C/1C" )
	PORT_DIPSETTING(    0x06, "A 1C/4C B 4C/1C" )
	PORT_DIPSETTING(    0x08, "A 1C/5C B 5C/1C" )
	PORT_DIPSETTING(    0x0a, "A 1C/6C B 6C/1C" )
	PORT_DIPSETTING(    0x0c, "A 2C/3C B 7C/1C" )
	PORT_DIPSETTING(    0x0e, "A 3C/2C B 8C/1C" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:!5,!6")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x30, "6" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW1:!7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	ALPHA68K_MCU
INPUT_PORTS_END

static INPUT_PORTS_START( kyros )
	PORT_START("IN0")
	ALPHA68K_PLAYER_INPUT_SWAP_LR_LSB( 1, IPT_UNKNOWN, IPT_START1, IP_ACTIVE_HIGH )
	ALPHA68K_PLAYER_INPUT_SWAP_LR_MSB( 2, IPT_UNKNOWN, IPT_START2, IP_ACTIVE_HIGH )

	PORT_START("IN1")  /* dipswitches */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:!1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0e, 0x0e, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:2,3,4")
	PORT_DIPSETTING(    0x0e, "A 1C/1C B 1C/1C" )
	PORT_DIPSETTING(    0x06, "A 1C/2C B 2C/1C" )
	PORT_DIPSETTING(    0x0a, "A 1C/3C B 3C/1C" )
	PORT_DIPSETTING(    0x02, "A 1C/4C B 4C/1C" )
	PORT_DIPSETTING(    0x0c, "A 1C/5C B 5C/1C" )
	PORT_DIPSETTING(    0x04, "A 1C/6C B 6C/1C" )
	PORT_DIPSETTING(    0x08, "A 2C/3C B 7C/1C" )
	PORT_DIPSETTING(    0x00, "A 3C/2C B 8C/1C" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:!5,!6")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x30, "6" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:!7")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:!8")
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )

	ALPHA68K_MCU
INPUT_PORTS_END

static INPUT_PORTS_START( jongbou )
	PORT_START("IN0")
	PORT_BIT( 0x0fff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Lives ) )       PORT_DIPLOCATION("SW1:!1")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Coinage ) )     PORT_DIPLOCATION("SW1:!2")
	PORT_DIPSETTING(    0x00, "A 1C/1C B 1C/5C" )
	PORT_DIPSETTING(    0x02, "A 1C/2C B 1C/3C" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Bonus_Life ) )  PORT_DIPLOCATION("SW1:!3")
	PORT_DIPSETTING(    0x00, "30000 - 60000" )
	PORT_DIPSETTING(    0x04, "Every 30000" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:!4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Difficulty ) )  PORT_DIPLOCATION("SW1:!5,!6")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ) )     PORT_DIPLOCATION("SW1:!7")
	PORT_DIPSETTING(    0x40, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, "Show Girls" )           PORT_DIPLOCATION("SW1:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	ALPHA68K_MCU

	PORT_START("IN3")
	PORT_BIT( 0x0f, 0, IPT_DIAL ) PORT_SENSITIVITY(50) PORT_KEYDELTA(20)

	PORT_START("IN4")
	PORT_BIT( 0x0f, 0, IPT_DIAL ) PORT_MINMAX(0, 15) PORT_SENSITIVITY(50) PORT_KEYDELTA(20) PORT_PLAYER(2)
INPUT_PORTS_END


/*
 *
 * Machine Config
 *
 */

void alpha68k_N_state::base_config(machine_config &config)
{
	MCFG_MACHINE_START_OVERRIDE(alpha68k_N_state,common)
	MCFG_MACHINE_RESET_OVERRIDE(alpha68k_N_state,common)

	GENERIC_LATCH_8(config, m_soundlatch);

	SPEAKER(config, "speaker").front_center();
}

void alpha68k_N_state::video_config(machine_config &config, u8 tile_transchar, u8 tile_bankshift, bool is_super_stingray)
{
	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	set_screen_raw_params(config);
	m_screen->set_screen_update(FUNC(alpha68k_N_state::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette, FUNC(alpha68k_N_state::palette_init), 256 + 1, 256);

	m_tile_transchar = tile_transchar;
	m_tile_bankshift = tile_bankshift;
	m_is_super_stingray = is_super_stingray;
}

void sstingray_state::sstingry(machine_config &config)
{
	base_config(config);
	/* basic machine hardware */
	M68000(config, m_maincpu, 6000000); /* 24MHz/4? */
	m_maincpu->set_addrmap(AS_PROGRAM, &sstingray_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(sstingray_state::irq1_line_hold));
	//m_maincpu->set_periodic_int(FUNC(sstingray_state::irq2_line_hold), attotime::from_hz(60)); // MCU irq

	Z80(config, m_audiocpu, 3579545);
	m_audiocpu->set_addrmap(AS_PROGRAM, &sstingray_state::sound_map);
	m_audiocpu->set_addrmap(AS_IO, &sstingray_state::sound_iomap);
	m_audiocpu->set_vblank_int("screen", FUNC(sstingray_state::irq0_line_hold));
	m_audiocpu->set_periodic_int(FUNC(sstingray_state::nmi_line_pulse), attotime::from_hz(4000));

	I8748(config, m_alpha8511, 9263750);     /* 9.263750 MHz(?) oscillator, divided by 3*5 internally */
	m_alpha8511->bus_in_cb().set(FUNC(sstingray_state::alpha8511_bus_r));
	m_alpha8511->bus_out_cb().set(FUNC(sstingray_state::alpha8511_bus_w));
	m_alpha8511->p1_in_cb().set(FUNC(sstingray_state::alpha8511_address_r));
	m_alpha8511->p2_in_cb().set(FUNC(sstingray_state::alpha8511_rw_r));
	m_alpha8511->p2_out_cb().set(FUNC(sstingray_state::alpha8511_control_w));
	m_alpha8511->t0_in_cb().set_ioport("IN2").bit(0);
	m_alpha8511->t1_in_cb().set_ioport("IN2").bit(1);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_sstingry);
	video_config(config, 0x40, 0, true);

	// sound hardware
	ym2203_device &ym(YM2203(config, "ym", 2'000'000));            // Verified from video by PCB, 24MHz/12?
	ym.add_route(ALL_OUTPUTS, "speaker", 0.30);

	ay8910_device &aysnd1(AY8910(config, "aysnd1", 2'000'000));    // Verified from video by PCB, 24MHz/12?
	aysnd1.add_route(ALL_OUTPUTS, "speaker", 0.30);

	ay8910_device &aysnd2(AY8910(config, "aysnd2", 2'000'000));    // Verified from video by PCB, 24MHz/12?
	aysnd2.add_route(ALL_OUTPUTS, "speaker", 0.45);

	DAC_8BIT_R2R(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.50); // unknown DAC
}

void kyros_state::kyros(machine_config &config)
{
	base_config(config);
	/* basic machine hardware */
	M68000(config, m_maincpu, 24_MHz_XTAL / 4);   /* Verified on bootleg PCB */
	m_maincpu->set_addrmap(AS_PROGRAM, &kyros_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(kyros_state::irq1_line_hold));
	m_maincpu->set_periodic_int(FUNC(kyros_state::irq2_line_hold), attotime::from_hz(60)); // MCU irq

	Z80(config, m_audiocpu, 24_MHz_XTAL / 6); /* Verified on bootleg PCB */
	m_audiocpu->set_addrmap(AS_PROGRAM, &kyros_state::sound_map);
	m_audiocpu->set_addrmap(AS_IO, &kyros_state::sound_iomap);
	m_audiocpu->set_vblank_int("screen", FUNC(kyros_state::irq0_line_hold));
	m_audiocpu->set_periodic_int(FUNC(kyros_state::nmi_line_pulse), attotime::from_hz(4000));

	/* video hardware */
	video_config(config, 0x20, 13, false);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_kyros);

	// sound hardware
	ym2203_device &ym(YM2203(config, "ym", 24_MHz_XTAL / 12));            // Verified on bootleg PCB
	ym.add_route(ALL_OUTPUTS, "speaker", 0.30);

	ay8910_device &aysnd1(AY8910(config, "aysnd1", 24_MHz_XTAL / 12));    // Verified on bootleg PCB
	aysnd1.add_route(ALL_OUTPUTS, "speaker", 0.30);

	ay8910_device &aysnd2(AY8910(config, "aysnd2", 24_MHz_XTAL / 12));    // Verified on bootleg PCB
	aysnd2.add_route(ALL_OUTPUTS, "speaker", 0.6);

	DAC_8BIT_R2R(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.50); // unknown DAC
}

void jongbou_state::jongbou(machine_config &config)
{
	base_config(config);
	/* basic machine hardware */
	M68000(config, m_maincpu, 8000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &jongbou_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(jongbou_state::irq1_line_hold));
	m_maincpu->set_periodic_int(FUNC(jongbou_state::irq2_line_hold), attotime::from_hz(60*16)); // MCU irq

	Z80(config, m_audiocpu, 4000000);
	m_audiocpu->set_addrmap(AS_PROGRAM, &jongbou_state::sound_map);
	m_audiocpu->set_addrmap(AS_IO, &jongbou_state::sound_iomap);
	m_audiocpu->set_periodic_int(FUNC(jongbou_state::irq0_line_hold), attotime::from_hz(160*60));

	/* video hardware */
	video_config(config, 0x20, 11, false);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_jongbou);

	/* sound hardware */
	ay8910_device &aysnd(AY8910(config, "aysnd", 2000000));
	aysnd.port_a_read_callback().set(m_soundlatch, FUNC(generic_latch_8_device::read));
	aysnd.add_route(ALL_OUTPUTS, "speaker", 0.65);
}


ROM_START( sstingry )
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 68000 code */
	ROM_LOAD16_BYTE( "ss_05.rom",  0x0000,  0x4000, CRC(bfb28d53) SHA1(64a1b8627529ed13074bb949cb104077eb3eac1f) )
	ROM_LOAD16_BYTE( "ss_07.rom",  0x0001,  0x4000, CRC(eb1b65c5) SHA1(cffc4df82b7950358dd28f6a492e0aefaff73048) )
	ROM_LOAD16_BYTE( "ss_04.rom",  0x8000,  0x4000, CRC(2e477a79) SHA1(0af9238979c8a740ba49776cd65ffbc024339621) )
	ROM_LOAD16_BYTE( "ss_06.rom",  0x8001,  0x4000, CRC(597620cb) SHA1(5549df4843e029df17ce5de2159cc82bd985804b) )

	ROM_REGION( 0x10000, "audiocpu", 0 )      /* sound cpu */
	ROM_LOAD( "ss_01.rom",       0x0000,  0x4000, CRC(fef09a92) SHA1(77b6aded1eed1bd5e6ffb25b56b62b10b7b9a304) )
	ROM_LOAD( "ss_02.rom",       0x4000,  0x4000, CRC(ab4e8c01) SHA1(d96e7f97945fff48fb7b4661fdb575ac7ff77445) )

	ROM_REGION( 0x0400, "alpha8511", 0 )    /* 8748 MCU code */
	// Was this dumped from a bootleg? The original "ALPHA-8511" MCU appears to be some sort of Hitachi part.
	ROM_LOAD( "d8748.bin",       0x0000, 0x0400, CRC(7fcbfc30) SHA1(6d087a3d44e475b6c8260a5134952097f26459b7) )

	ROM_REGION( 0x60000, "gfx1", 0 )
	ROM_LOAD( "ss_12.rom",       0x00000, 0x4000, CRC(74caa9e9) SHA1(9f0874b2fcdf45acb941bd56b44bf2b9b08641e9) )
	ROM_LOAD( "ss_08.rom",       0x08000, 0x4000, CRC(32368925) SHA1(af26f73d33936410063de3164ec80f45bed487c7) )
	ROM_LOAD( "ss_13.rom",       0x10000, 0x4000, CRC(13da6203) SHA1(afa778c26da1adfdc8b2e2a1c7b2b46944b5d008) )
	ROM_LOAD( "ss_10.rom",       0x18000, 0x4000, CRC(2903234a) SHA1(552295ec60469227883eafb6756c86abc20455b5) )
	ROM_LOAD( "ss_11.rom",       0x20000, 0x4000, CRC(d134302e) SHA1(4c020ff41458c738596ab7a094d4a33a6dda64bf) )
	ROM_LOAD( "ss_09.rom",       0x28000, 0x4000, CRC(6f9d938a) SHA1(eb6934f8eaa7b22441ec4470280f228fe5f134a3) )

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "ic92",            0x0000, 0x0100, CRC(e7ce1179) SHA1(36835c46c1c3f820df39c59c16c362db07b32dc9) )
	ROM_LOAD( "ic93",            0x0100, 0x0100, CRC(9af8a375) SHA1(abb8b094a2df41acea688f87004207dc35233db5) )
	ROM_LOAD( "ic91",            0x0200, 0x0100, CRC(c3965079) SHA1(6b1f22afd2a849f0003ddcad344079e8043681f9) )

	ROM_REGION( 0x200, "clut_proms", 0 )
	ROM_LOAD( "ssprom2.bin",     0x0100, 0x0100, CRC(c2205b71) SHA1(a7db60ac7d559fe53a35264fab17f1d5e48d3f10) )
	ROM_LOAD( "ssprom1.bin",     0x0000, 0x0100, CRC(1003186c) SHA1(e50b60036d6b32a4d524c92d35c4d9901ee7ec0e) )
ROM_END

ROM_START( kyros )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "2.10c", 0x00000,  0x4000, CRC(4bd030b1) SHA1(e503dae8e12995ab0a551022a848a62315908e8b) )
	ROM_CONTINUE   (          0x10000,  0x4000 )
	ROM_LOAD16_BYTE( "1.13c", 0x00001,  0x4000, CRC(75cfbc5e) SHA1(2a70c56fd7192279157df8294743038a7ed7e68d) )
	ROM_CONTINUE   (          0x10001,  0x4000 )
	ROM_LOAD16_BYTE( "4.10b", 0x08000,  0x4000, CRC(be2626c2) SHA1(c3b01ec4b65172560a993b37421df6a61b780e43) )
	ROM_CONTINUE   (          0x18000,  0x4000 )
	ROM_LOAD16_BYTE( "3.13b", 0x08001,  0x4000, CRC(fb25e71a) SHA1(fab8fcbd2c5a8600d6e8577de4875e409cad723b) )
	ROM_CONTINUE   (          0x18001,  0x4000 )

	ROM_REGION( 0x10000, "audiocpu", 0 )   /* Sound CPU */
	ROM_LOAD( "2s.1f",      0x00000, 0x4000, CRC(800ceb27) SHA1(4daa1b8adcad7a90cfd5d20704a7c431673c4995) )
	ROM_LOAD( "1s.1d",      0x04000, 0x8000, CRC(87d3e719) SHA1(4b8b1b600c7c1de3a77030001e7e6f0ff118f294) )

	// not hooked up yet
	ROM_REGION( 0x1000, "mcu", 0 )
	ROM_LOAD( "kyros_68705u3.bin",    0x0000, 0x1000, CRC(c20880b7) SHA1(b041c36cbc4f348d74e0548df5cb14727f2d353b) ) // this one is from a bootleg PCB, program code *might* be compatible.
	ROM_LOAD( "kyros_mcu.bin",    0x0800, 0x0800,  BAD_DUMP CRC(3a902a19) SHA1(af1be8894c899b27b1106663ffaf2ab43fa1cdaa) ) // original MCU? (HD6805U1) zero-page ROM not dumped

	ROM_REGION( 0x60000, "gfx1", 0 )
	ROM_LOAD( "8.9pr",  0x00000, 0x8000, CRC(c5290944) SHA1(ec97482dc59220002780ae4d02be4cd172cf65ac) )
	ROM_LOAD( "11.11m", 0x08000, 0x8000, CRC(fbd44f1e) SHA1(d095544ea76674a7ad17c1b8c88614e65890281c) )
	ROM_LOAD( "12.11n", 0x10000, 0x8000, CRC(10fed501) SHA1(71c0b4b94f86046745105307938f6e2c5661e2a1) )
	ROM_LOAD( "9.9s",   0x18000, 0x8000, CRC(dd40ca33) SHA1(91a1d8b6b69fb0d27ed315cd2591f352360bc8e7) )
	ROM_LOAD( "13.11p", 0x20000, 0x8000, CRC(e6a02030) SHA1(0de58f8cc69dc76d4b0a45fba04972634a4021a6) )
	ROM_LOAD( "14.11r", 0x28000, 0x8000, CRC(722ad23a) SHA1(0e1be976c5a406e33236def5a0dce73911ebac28) )
	ROM_LOAD( "15.3t",  0x30000, 0x8000, CRC(045fdda4) SHA1(ac25368e446e6dcfb3ed244e7d6d699f917c202d) )
	ROM_LOAD( "17.7t",  0x38000, 0x8000, CRC(7618ec00) SHA1(7346ba41fd2b04e404225726ede2e42e62ca7901) )
	ROM_LOAD( "18.9t",  0x40000, 0x8000, CRC(0ee74171) SHA1(1fda8a1066eb7dafeaeffebfe718b408b34f1767) )
	ROM_LOAD( "16.5t",  0x48000, 0x8000, CRC(2cf14824) SHA1(07f6b232c4ca6c42b3f75443b0328653f5a3f71d) )
	ROM_LOAD( "19.11t", 0x50000, 0x8000, CRC(4f336306) SHA1(83e0c021d2732d3199c70ac433a31863075a5a72) )
	ROM_LOAD( "20.13t", 0x58000, 0x8000, CRC(a165d06b) SHA1(ef7f38a71c2f3fc836b28f0eac1c14a3877f0802) )

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "mb7114l.5r", 0x000, 0x100, CRC(3628bf36) SHA1(33b7b873a6e5da59f535d754f8c8257c0f4d0a31) )
	ROM_LOAD( "mb7114l.4r", 0x100, 0x100, CRC(850704e4) SHA1(8a9da9efc7bc6a037d4cd27152b853a7839ccd67) )
	ROM_LOAD( "mb7114l.6r", 0x200, 0x100, CRC(a54f60d7) SHA1(af039dee847913cb79f85d7abf4846322bba2e5b) )

	ROM_REGION( 0x200, "clut_proms", 0 )
	ROM_LOAD( "mb7114l.5p", 0x100, 0x100, CRC(1cc53765) SHA1(2b665f3e24ddb3ab591273f027ff7740f1c97e27) )
	ROM_LOAD( "mb7114l.6p", 0x000, 0x100, CRC(b0d6971f) SHA1(4aa2e9a89f9ea7487433e54ef4aa95a632477c4f) )

	ROM_REGION( 0x2000, "color_proms", 0 )
	ROM_LOAD( "0.1t",      0x0000,0x2000, CRC(5d0acb4c) SHA1(52fcdcb2bf6d6ada04aa447b5526c39848bf587f) )
ROM_END

ROM_START( kyrosj )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "2j.10c",0x00000,  0x4000, CRC(b324c11b) SHA1(9330ee0db8555a3623118c7bc5363b4f6fa87dbc) )
	ROM_CONTINUE   (          0x10000,  0x4000 )
	ROM_LOAD16_BYTE( "1j.13c",0x00001,  0x4000, CRC(8496241b) SHA1(474cdce735dcc2ff2111ae2f4cd11c0d27a4b4fc) )
	ROM_CONTINUE   (          0x10001,  0x4000 )
	ROM_LOAD16_BYTE( "4.10a", 0x08000,  0x4000, CRC(0187f59d) SHA1(3bc1b811cb29aa33c38bc8c76e066c8b37104167) )
	ROM_CONTINUE   (          0x18000,  0x4000 )
	ROM_LOAD16_BYTE( "3.13a", 0x08001,  0x4000, CRC(ab97941d) SHA1(014a55540e1777de5bee23e59773dbbd7efa8f91) )
	ROM_CONTINUE   (          0x18001,  0x4000 )

	ROM_REGION( 0x10000, "audiocpu", 0 )   /* Sound CPU */
	ROM_LOAD( "2s.1f",      0x00000, 0x4000, CRC(800ceb27) SHA1(4daa1b8adcad7a90cfd5d20704a7c431673c4995) )
	ROM_LOAD( "1s.1d",      0x04000, 0x8000, CRC(87d3e719) SHA1(4b8b1b600c7c1de3a77030001e7e6f0ff118f294) )

	ROM_REGION( 0x1000, "mcu", 0 ) // these comes from original set
	ROM_LOAD( "kyros_68705u3.bin",    0x0000, 0x1000, BAD_DUMP CRC(c20880b7) SHA1(b041c36cbc4f348d74e0548df5cb14727f2d353b) ) // this one is from a bootleg PCB, program code *might* be compatible.
	ROM_LOAD( "kyros_mcu.bin",    0x0800, 0x0800,  BAD_DUMP CRC(3a902a19) SHA1(af1be8894c899b27b1106663ffaf2ab43fa1cdaa) ) // original MCU? (HD6805U1)

	ROM_REGION( 0x60000, "gfx1", 0 )
	ROM_LOAD( "8.9r",   0x00000, 0x8000, CRC(d8203284) SHA1(7dede410239be6b674644fa76c91dd01837f841f) )
	ROM_LOAD( "11.12m", 0x08000, 0x8000, CRC(a2f9738c) SHA1(31be81274bf70674bf0c32fcddbacf0f58d8f897) )
	ROM_LOAD( "12.11n", 0x10000, 0x8000, CRC(10fed501) SHA1(71c0b4b94f86046745105307938f6e2c5661e2a1) )
	ROM_LOAD( "9j.9s",  0x18000, 0x8000, CRC(3e725349) SHA1(79c431d83a0f0d5e0d69086f54f6e60a42b69e14) )
	ROM_LOAD( "13.11p", 0x20000, 0x8000, CRC(e6a02030) SHA1(0de58f8cc69dc76d4b0a45fba04972634a4021a6) )
	ROM_LOAD( "14.12r", 0x28000, 0x8000, CRC(39d07db9) SHA1(05c0785eea29bc6329892f1e8f0bd37327163080) )
	ROM_LOAD( "15.3t",  0x30000, 0x8000, CRC(045fdda4) SHA1(ac25368e446e6dcfb3ed244e7d6d699f917c202d) )
	ROM_LOAD( "17.7t",  0x38000, 0x8000, CRC(7618ec00) SHA1(7346ba41fd2b04e404225726ede2e42e62ca7901) )
	ROM_LOAD( "18.9t",  0x40000, 0x8000, CRC(0ee74171) SHA1(1fda8a1066eb7dafeaeffebfe718b408b34f1767) )
	ROM_LOAD( "16j.5t", 0x48000, 0x8000, CRC(e1566679) SHA1(3653c3160798bea203e1c7713043729b356a7358) )
	ROM_LOAD( "19.11t", 0x50000, 0x8000, CRC(4f336306) SHA1(83e0c021d2732d3199c70ac433a31863075a5a72) )
	ROM_LOAD( "20j.13t",0x58000, 0x8000, CRC(0624b4c0) SHA1(a8ebdb1f9b7fd0b78102b54523e8680aaa8bcf42) )

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "mb7114l.5r", 0x000, 0x100, CRC(3628bf36) SHA1(33b7b873a6e5da59f535d754f8c8257c0f4d0a31) )
	ROM_LOAD( "mb7114l.4r", 0x100, 0x100, CRC(850704e4) SHA1(8a9da9efc7bc6a037d4cd27152b853a7839ccd67) )
	ROM_LOAD( "mb7114l.6r", 0x200, 0x100, CRC(a54f60d7) SHA1(af039dee847913cb79f85d7abf4846322bba2e5b) )

	ROM_REGION( 0x200, "clut_proms", 0 )
	ROM_LOAD( "mb7114l.5p", 0x100, 0x100, CRC(1cc53765) SHA1(2b665f3e24ddb3ab591273f027ff7740f1c97e27) )
	ROM_LOAD( "mb7114l.6p", 0x000, 0x100, CRC(b0d6971f) SHA1(4aa2e9a89f9ea7487433e54ef4aa95a632477c4f) )

	ROM_REGION( 0x2000, "color_proms", 0 )
	ROM_LOAD( "0j.1t",      0x0000,0x2000, CRC(a34ecb29) SHA1(60a0b0cfcd2d9830bc112774bac700ded40d4afb) )
ROM_END

ROM_START( jongbou )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "p2.a13", 0x00000, 0x10000, CRC(ee59e67a) SHA1(d73f15994879c645a8021dcd4f53948bcbd0748e) )
	ROM_LOAD16_BYTE( "p1.a15", 0x00001, 0x10000, CRC(1ab6803e) SHA1(a217138332d61b8f5996ead0280c970481db9abe) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "p7.i1", 0x00000, 0x8000, CRC(88d74794) SHA1(98dbbb4d88c1e96a0e251e39ef43b02bd68e0bba) )

	ROM_REGION( 0x10000, "mcu", 0 )
	ROM_LOAD( "alpha.mcu", 0x000, 0x1000, NO_DUMP )

	ROM_REGION( 0x30000, "gfx1", 0 )
	ROM_LOAD( "p6.l15", 0x00000, 0x08000, CRC(1facee65) SHA1(6c98338c616e53106960063d0d31483131b492b0) )
	ROM_CONTINUE(0x18000,0x8000)
	ROM_LOAD( "p5.k15", 0x20000, 0x10000, CRC(db0ad6bb) SHA1(c2ce0e78a4be9314f4f14ea87f521a79bab3697c) )
	ROM_LOAD( "p4.j15", 0x08000, 0x10000, CRC(56842cfa) SHA1(141ed992332540487cec951eab61c18be994b618) )

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "r.k2",  0x0000, 0x0100, CRC(0563235a) SHA1(c337a9a15c1a27012a963fc4e1345605aaa1401f) )
	ROM_LOAD( "g.k1",  0x0100, 0x0100, CRC(81fc51f2) SHA1(92df86898a1cc1fa2faf620466737f4e1cf83a58) )
	ROM_LOAD( "b.k3",  0x0200, 0x0100, CRC(6dfeba56) SHA1(abf569c400dc4366a0c7e483dbb672c089692c7e) )

	ROM_REGION( 0x200, "clut_proms", 0 )
	ROM_LOAD( "h.l9",  0x0100, 0x0100, CRC(e6e93b0b) SHA1(f64ff63699451910982a1a44c94ccd2c18fd389e) )
	ROM_LOAD( "l.l10", 0x0000, 0x0100, CRC(51676dac) SHA1(685d14f448501a63cc9fa063f65842caddad8f39) )

	ROM_REGION( 0x2000, "color_proms", 0 )
	ROM_LOAD( "p3.i15", 0x0000, 0x2000, CRC(8c09cd2a) SHA1(317764e0f5af29e78fd764bdf28579bf6be5630f) )
ROM_END

ROM_START( jongbou2 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "j.b1_ver.2.13a.27c512", 0x00000, 0x10000, CRC(22e18446) SHA1(e6f6f06a99c66dcf8b3e05b85c35597a26b57aa2) )
	ROM_LOAD16_BYTE( "j.b2_ver.2.16a.27c512", 0x00001, 0x10000, CRC(c30d0030) SHA1(56d3aacf7f53970c21abd6ac5592a5b29ac946c8) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "j.b7.1i.27256", 0x00000, 0x8000, CRC(88d74794) SHA1(98dbbb4d88c1e96a0e251e39ef43b02bd68e0bba) )

	ROM_REGION( 0x10000, "mcu", 0 )
	ROM_LOAD( "alpha.mcu", 0x000, 0x1000, NO_DUMP )

	ROM_REGION( 0x30000, "gfx1", 0 )
	ROM_LOAD( "j.b6.16l.27c512",  0x00000, 0x08000, CRC(71c53f95) SHA1(0e12d03f2bbcff14816f739026e40939c9f68bab) )
	ROM_CONTINUE(0x18000,0x8000)
	ROM_LOAD( "j.b5.16k.27c512",  0x20000, 0x10000, CRC(d68b6412) SHA1(6734f5eb31fbf7b5b3edf1d8c4369cc74aea74ab) )
	ROM_LOAD( "j.b24.16j.27c512", 0x08000, 0x10000, CRC(7dad0bda) SHA1(bea00314036a43806c07c39a0d3f7020990ded09) )

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "2.l2.82s129a",  0x0000, 0x0100, CRC(aa5c9b97) SHA1(2758d393aa210fab0af9f5fd817edbd5c71a88f2) )
	ROM_LOAD( "1.l1.82s129a",  0x0100, 0x0100, CRC(15e45cf2) SHA1(f0712b43e8423f811a1691b1f51683a7af58f868) )
	ROM_LOAD( "3.l3.82s129a",  0x0200, 0x0100, CRC(90de80ca) SHA1(d58c5fed42ac71b84bb51e6acfe1d65158532387) )

	ROM_REGION( 0x200, "clut_proms", 0 )
	ROM_LOAD( "4.l9.82s129a",  0x0100, 0x0100, CRC(a2c6204a) SHA1(3bb342eced4abc0d7404b00343b3770dd7a0003e) )
	ROM_LOAD( "5.l10.82s129a", 0x0000, 0x0100, CRC(f7cdebee) SHA1(20cb28e2fb9507ef1aaa9c05f22ca589756eb49d) )

	ROM_REGION( 0x2000, "color_proms", 0 )
	ROM_LOAD( "j.b3.16i.2764", 0x00000, 0x2000, CRC(d13a1c02) SHA1(7ad4b96f5ce05e0ca5b55c486b3870d756e6bded) )
ROM_END



void sstingray_state::init_sstingry()
{
	m_invert_controls = 0;
	m_microcontroller_id = 0x00ff;
	m_coin_id = 0x22 | (0x22 << 8);
	m_game_id = 0;

	m_alpha8511_sync_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(sstingray_state::alpha8511_sync), this));
	save_item(NAME(m_alpha8511_address));
	save_item(NAME(m_alpha8511_control));
	save_item(NAME(m_alpha8511_read_mode));
}

void kyros_state::init_kyros()
{
	m_invert_controls = 0;
	m_microcontroller_id = 0x0012;
	m_coin_id = 0x22 | (0x22 << 8);
	m_game_id = ALPHA68K_KYROS;
}

void jongbou_state::init_jongbou()
{
	m_invert_controls = 0;
	m_microcontroller_id = 0x00ff;
	m_coin_id = 0x23 | (0x24 << 8);
	m_game_id = ALPHA68K_JONGBOU;
}

void jongbou_state::init_jongbou2()
{
	init_jongbou();
	m_microcontroller_id = 0x0012;
}


GAME( 1986, sstingry,  0,        sstingry,       sstingry,  sstingray_state, init_sstingry,  ROT90, "Alpha Denshi Co.",                                  "Super Stingray (Japan)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_TIMING )

GAME( 1987, kyros,     0,        kyros,          kyros,     kyros_state, init_kyros,     ROT90, "Alpha Denshi Co. (World Games Inc. license)",       "Kyros", MACHINE_SUPPORTS_SAVE )
GAME( 1986, kyrosj,    kyros,    kyros,          kyros,     kyros_state, init_kyros,     ROT90, "Alpha Denshi Co.",                                  "Kyros no Yakata (Japan)", MACHINE_SUPPORTS_SAVE )

GAME( 1987, jongbou,   0,        jongbou,        jongbou,   jongbou_state,    init_jongbou,   ROT90, "SNK",                                               "Mahjong Block Jongbou (Japan)", MACHINE_SUPPORTS_SAVE )

GAME( 1987, jongbou2,  0,        jongbou,        jongbou,   jongbou_state,    init_jongbou2,  ROT90, "SNK",                                               "Mahjong Block Jongbou 2 (Japan)", MACHINE_SUPPORTS_SAVE )
