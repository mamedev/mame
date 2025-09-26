// license:BSD-3-Clause
// copyright-holders: Roberto Fresca, Grull Osgo, David Haywood

/***************************************************************************************************

  New Carrera, Version 2000.
  JT Amusement


  This is a stealth gambling machine disguised as a standard arcade game. Players initially interact
  with what appears to be a legitimate arcade game, until they enter a hidden password. Once activated,
  the game reveals its true nature: a fully functional video slots gambling game.

  The game's code resides in a PSD312-B (an MCU peripheral), while an AT90S8515 MCU handles:

   * Screen rendering
   * Symbol placement
   * Reel animation
   * Input processing
   * Management of hardcoded NVRAM values (game configuration & settings)


  Since we lack the original MCU firmware, we've simulated its functions by replicating register-level
  behavior based on how the game code interacts with the hardware.

  For a complete and accurate emulation, we need the MCU's contents, specifically the EPROM and FLASH data.


  Tech notes:

  This game uses a newer 'TYPE C-2000' board with a 'Rania Original 2000 Type 8515'
  riser board (the Z80 and MC6845 have been moved here along with a AT90S8515 MCU)

  - The MCU behavior has been simulated to make the game playable.
  - The MCU has full range memory access, so it can read ROM and Read/Write NVRAM.
  - The MCU manages reels display animation.
  - The MCU manages inputs.
  - The MCU has preset settings usually settled by DIP SW in this hardware.
  - The MCU manages some RTC, for protection purposes (not emulated, nor simulated).
  - Some game settings are available from amusement mode with BUTTON1+PAYOUT buttons via code: 4545.
  - General settings normally driven through DIP switches are hardcoded in NVRAM.
  - To enter Gambling Mode get access with BUTTON1+PAYOUT buttons and set code: 1965.


  Procedure to enter the codes:

  1) Press Button 1 + Payout.
  2) Keep pressed Button 1 while release the Payout key.
  3) Use joystick to change each digit on the 4-digits code.
  4) Release Button 1.


  How to play:

  Just coin in, bet with joystick up, and start to roll with joystick down.


  TODO:

  - Dump the AT90S8515 MCU (8 bit AVR RISC core) internal EPROM and FLASH.
  - M48T02 timekeeper (compatible with Dallas DS1642) support.


***************************************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class ncarrera_state : public driver_device
{
public:
	ncarrera_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_tileram(*this, "tileram"),
		m_maincpu(*this, "maincpu"),
		m_nvram(*this, "nvram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen"),
		m_in0(*this, "IN0"),
		m_in1(*this, "IN1")
	{ }

	virtual void machine_start() override ATTR_COLD;
	void ncarrera(machine_config &config);

private:
	uint8_t unknown_r();
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void prg_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void porta_w(uint8_t data);
	uint8_t nvram_r(offs_t offset);
	void nvram_w(offs_t offset, uint8_t data);
	uint16_t inc_idx(uint16_t &index, uint8_t i);
	void print_reels(uint8_t data);

	required_shared_ptr<uint8_t> m_tileram;
	required_device<cpu_device> m_maincpu;
	required_device<nvram_device> m_nvram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	required_ioport m_in0;
	required_ioport m_in1;
	std::unique_ptr<uint8_t[]> m_nvram8;

	uint16_t const reel_base = 0x2910;

	uint8_t m_vp = 0;
	uint8_t m_frame = 0;
	int m_sublin = 2;

	int m_delay = 0;
	int m_buffin0 = 0;
	int m_buffin1 = 0;
};


/*************************************************
*                 Machine Start                  *
*************************************************/

void ncarrera_state::machine_start()
{
	m_nvram8 = std::make_unique<uint8_t[]>(0x800);
	m_nvram->set_base(m_nvram8.get(), 0x800);
	save_item(NAME(m_frame));
}


/*************************************************
*                  Video Hardware                *
*************************************************/

uint32_t ncarrera_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int count = 0;

	for (int y = 0; y < 32; y++)
	{
		for (int x = 0; x < 64; x++)
		{
			int tile = m_tileram[count & 0x7ff] | m_tileram[(count & 0x7ff) + 0x800] << 8;

			m_gfxdecode->gfx(0)->opaque(bitmap, cliprect, tile, 0, 0, 0, x * 8, y * 8);
			count++;
		}
	}
	return 0;
}

void ncarrera_state::palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();
	for (int i = 0; i < 0x20; ++i)
	{
		int bit0, bit1;
		int const br_bit0 = BIT(color_prom[i], 7);
		int const br_bit1 = BIT(color_prom[i], 6);

		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 3);
		int const b = 0x0e * br_bit0 + 0x1f * br_bit1 + 0x43 * bit0 + 0x8f * bit1;
		bit0 = BIT(color_prom[i], 1);
		bit1 = BIT(color_prom[i], 4);
		int const g = 0x0e * br_bit0 + 0x1f * br_bit1 + 0x43 * bit0 + 0x8f * bit1;
		bit0 = BIT(color_prom[i], 2);
		bit1 = BIT(color_prom[i], 5);
		int const r = 0x0e * br_bit0 + 0x1f * br_bit1 + 0x43 * bit0 + 0x8f * bit1;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}


/*************************************************
*             Memory map information             *
*************************************************/

void ncarrera_state::prg_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xe000, 0xe7ff).rw(FUNC(ncarrera_state::nvram_r),FUNC(ncarrera_state::nvram_w));
	map(0xe800, 0xe800).w("crtc", FUNC(mc6845_device::address_w));
	map(0xe801, 0xe801).w("crtc", FUNC(mc6845_device::register_w));
	map(0xf000, 0xffff).ram().share(m_tileram);
}

void ncarrera_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).portr("IN0");
	map(0x01, 0x01).portr("IN1");
	map(0x08, 0x09).w("aysnd", FUNC(ay8910_device::address_data_w));
}


/*************************************************
*                 Read Write Handlers            *
*************************************************/

uint8_t ncarrera_state::unknown_r()
{
	return machine().rand();
}

void ncarrera_state::porta_w(uint8_t data)
{
}

uint16_t ncarrera_state::inc_idx(uint16_t& index, uint8_t i)
{
	uint16_t ret = index;
	uint16_t base = reel_base + (i - 1) * 0x1b;
	index = (index == base + 0x1a) ? base : index + 1;

	return ret;
}

void ncarrera_state::print_reels(uint8_t data)
{
	// constants and config
	static constexpr uint16_t TILE_OFFSET = 0x18;
	static constexpr uint16_t ROW_STRIDE = 64;
	static constexpr uint16_t TILE_SIZE = 8;
	static constexpr uint16_t ATTR_OFFSET = 0x800;
	static constexpr uint8_t  BRICK = 0x10;
	static constexpr uint8_t  FRUIT = 0x18;

	// local var
	uint8_t *rom = memregion("maincpu")->base();
	uint16_t destinations[3] = {0x2c7, 0x2d1, 0x2db};
	bool reels_active[3] = {false};
	uint8_t attr;

	// follow dip sw setting
	if(m_nvram8[0x00e] == 0)
		attr = FRUIT;
	else
		attr = BRICK;

	// reels status init
	reels_active[0] = (m_nvram8[0x16f] == 0x0e) || (m_nvram8[0x16f] == 0x13) || (m_nvram8[0x16f] == 0x14);
	reels_active[1] = (m_nvram8[0x16f] == 0x0e) || (m_nvram8[0x16f] == 0x13) || (m_nvram8[0x16f] == 0x14) || (m_nvram8[0x16f] == 0x10);
	reels_active[2] = (m_nvram8[0x16f] == 0x0e) || (m_nvram8[0x16f] == 0x13) || (m_nvram8[0x16f] == 0x14) || (m_nvram8[0x16f] == 0x11);

	// index config
	auto setup_indices = [&](uint8_t reel_id) -> uint16_t
	{
		int16_t step = (m_nvram8[0x156 + reel_id] + (data / 3) - 1) % 0x1b;
		return static_cast<uint16_t>((step < 0) ? 0x1a : step);
	};

	uint16_t indices[3] =
	{
		static_cast<uint16_t>(reel_base + 0x00 + setup_indices(0)),  // 0x2910 + offset
		static_cast<uint16_t>(reel_base + 0x1B + setup_indices(1)),  // 0x2910 + 27 = 0x292B
		static_cast<uint16_t>(reel_base + 0x36 + setup_indices(2))   // 0x2910 + 54 = 0x2946
	};

	// update tiles function
	auto update_tile = [&](uint16_t dest, uint16_t index, bool condition)
	{
		if(condition)
		{
			m_tileram[dest] = index;
			m_tileram[dest + ATTR_OFFSET] = attr;
		}
	};

	// generic process of symbols
	auto process_figure = [&](int rows, bool use_sublin_offset = false)
	{
		uint8_t tiles[3];

		// getting tiles for each reel
		for(int i = 0; i < 3; ++i)
		{
			tiles[i] = 9 - rom[inc_idx(indices[i], i + 1)];
		}

		// processing each reel
		for(int reel = 0; reel < 3; ++reel)
		{
			uint16_t base_index = tiles[reel] * TILE_OFFSET;

			if(use_sublin_offset)
			{
				base_index += m_sublin * TILE_SIZE;
			}

			for(int i = 0; i < rows; ++i)
			{
				for(int j = 0; j < TILE_SIZE; ++j)
				{
					const uint16_t offset = j + (i * ROW_STRIDE);
					update_tile(destinations[reel] + offset,
							   base_index + (i * TILE_SIZE) + j,
							   reels_active[reel]);
				}
			}

			destinations[reel] += rows * ROW_STRIDE;
		}
	};


	// principal process
	switch(m_sublin)
	{
		case 1: process_figure(2, true); break;
		case 2: process_figure(1, true); break;
		default: break;
	}

	// process symbols 2-4
	for(int i = 0; i < 3; ++i)
	{
		process_figure(3);
	}

	// process final symbol
	switch(m_sublin)
	{
		case 0: process_figure(2); break;
		case 2: process_figure(1); break;
		default: break;
	}

	// subline status update
	m_sublin = (m_sublin - 1 + 3) % 3;
}


/*************************************************
*               I/O NVRAM Handling               *
*************************************************/

uint8_t ncarrera_state::nvram_r(offs_t offset)
{
	uint8_t vp = m_screen->vpos();

	// mcu -> vblank timers
	if((vp==0) && (m_vp == 0xff))
	{
		m_frame++;

		// mcu -> vblank timers
		if(offset == 0)
			if(m_nvram8[0] > 0)
				m_nvram8[0] -= 1;

		if(offset == 3)
			if(m_nvram8[3] > 0)
				m_nvram8[3] -= 1;

		if(offset == 4)
			if(m_nvram8[4] > 0)
				m_nvram8[4] -= 1;

		if(offset == 7)
			if(m_nvram8[7] > 0)
				m_nvram8[7] -= 1;
	}

	if(offset == 2)
		if(m_nvram8[2] > 0)
		{
			m_nvram8[2] -= 1;
		}

	if( (m_frame & 0x07) == 1)
	{
		if (m_delay == 0)
		{
			m_buffin0 = ~m_nvram8[0x09];
			m_buffin1 = ~m_nvram8[0x0a];
			m_nvram8[0x0b] = m_buffin0;  // IN0
			m_nvram8[0x0c] = m_buffin1;  // IN1
		}

		if (m_delay > 0 && m_delay < 10)
		{
			m_buffin0 = 0x00;
			m_buffin1 = 0x00;
			m_nvram8[0x0b] = m_buffin0;  // IN0
			m_nvram8[0x0c] = m_buffin1;  // IN1
		}

		m_delay++;

		if (m_delay == 10)
			m_delay = 0;
	}

	m_nvram8[0x47] = BIT(m_nvram8[0x0c], 0);
	m_nvram8[0x48] = BIT(m_nvram8[0x0c], 1);

	m_vp = vp;
	return m_nvram8[offset];
}


void ncarrera_state::nvram_w(offs_t offset, uint8_t data)
{
	m_nvram8[offset] = data;

	if((offset == 0x16f) && (m_nvram8[0x005a] == 0))
	{
		switch(data)
		{
			case 0x0d:
			case 0x0e: print_reels(m_nvram8[0x15f]); break;
			case 0x10: print_reels(m_nvram8[0x15f]); break;
			case 0x11: print_reels(m_nvram8[0x15f]); break;
			case 0x12: break;
			case 0x13: { m_sublin = 2; print_reels(0);} break;
			case 0x14: { m_sublin = 2; print_reels(0);} break;
		}
	}
}


/*************************************************
*                  Input ports                   *
*************************************************/

static INPUT_PORTS_START( ncarrera )
	PORT_START("IN0")   // Port 0 (E00B)
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT  )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_OTHER ) PORT_NAME("0-6") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_SERVICE1 ) PORT_NAME("Master Reset")
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_GAMBLE_PAYOUT )  // + button 1 -> enable prompt code for gambling mode (code:1965)

	PORT_START("IN1")   // Port 1 (E00C)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_OTHER ) PORT_NAME("1-3") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_OTHER ) PORT_NAME("1-4") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_OTHER ) PORT_NAME("1-5") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_OTHER ) PORT_NAME("1-6") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_OTHER ) PORT_NAME("1-7") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_SERVICE2 )

//  DIP switches are harcoded in NVRAM.

INPUT_PORTS_END


/*************************************************
*                Graphics Layouts                *
*************************************************/

static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,5),
	5,
	{ RGN_FRAC(0,5), RGN_FRAC(1,5),RGN_FRAC(2,5),RGN_FRAC(3,5),RGN_FRAC(4,5) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


/*************************************************
*          Graphics Decode Information           *
*************************************************/

static GFXDECODE_START( gfx_carrera )
	GFXDECODE_ENTRY( "tiles", 0, tiles8x8_layout, 0, 1 )
GFXDECODE_END


/*************************************************
*                Machine Drivers                 *
*************************************************/

void ncarrera_state::ncarrera(machine_config &config)
{
	constexpr XTAL MASTER_CLOCK = 22.1184_MHz_XTAL;

	// basic machine hardware
	Z80(config, m_maincpu, MASTER_CLOCK / 6);
	m_maincpu->set_addrmap(AS_PROGRAM, &ncarrera_state::prg_map);
	m_maincpu->set_addrmap(AS_IO, &ncarrera_state::io_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(512, 256);
	m_screen->set_visarea_full();
	m_screen->set_screen_update(FUNC(ncarrera_state::screen_update));

	mc6845_device &crtc(MC6845(config, "crtc", MASTER_CLOCK / 16));
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);
	crtc.out_vsync_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_carrera);
	PALETTE(config, m_palette, FUNC(ncarrera_state::palette), 32);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ay8910_device &aysnd(AY8910(config, "aysnd", MASTER_CLOCK / 12));
	aysnd.port_a_write_callback().set(FUNC(ncarrera_state::porta_w));
	aysnd.port_b_read_callback().set(FUNC(ncarrera_state::unknown_r));
	aysnd.add_route(ALL_OUTPUTS, "mono", 1.00);
}


/*************************************************
*                    ROM Load                    *
************************************************/

/*
  New Carrera

  this game uses a newer 'TYPE C-2000' board with a 'Rania Original 2000 Type 8515'
  riser board (the Z80 and MC6845 have been moved here along with a AT90S8515 MCU)

*/
ROM_START( ncarrera )
	ROM_REGION( 0x10000, "maincpu", 0 ) // has 2001, Mpampis and Avraam strings
	ROM_LOAD( "27512.ic22", 0x00000, 0x10000, CRC(3ec2dbca) SHA1(896fbccaf844c1fa5861b176c09e4a3707b3524f) ) // 1xxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x2200, "mcu", 0 )
	ROM_LOAD( "internal_eeprom", 0x0000, 0x0200, NO_DUMP )
	ROM_LOAD( "internal_flash",  0x0200, 0x2000, NO_DUMP )

	ROM_REGION( 0x50000, "tiles", 0 ) // has both New Carrera and New Bomberman GFX
	ROM_LOAD( "27512.ic1", 0x00000, 0x10000, CRC(dbec54c7) SHA1(ca7e54c198ca8abeffba1b323a514678384c35f9) )
	ROM_LOAD( "27512.ic2", 0x10000, 0x10000, CRC(8e8c2b6d) SHA1(001121e0b91d8e0efdc3f5f99c43e1751b4be758) )
	ROM_LOAD( "27512.ic3", 0x20000, 0x10000, CRC(ac66cda8) SHA1(65fae21de9f9727c5d8198ff57b27d703a7518fc) )
	ROM_LOAD( "27512.ic4", 0x30000, 0x10000, CRC(c337a9b8) SHA1(0c4f86e1c7c94c492b09e3571e213308e9fa7c47) )
	ROM_LOAD( "27512.ic5", 0x40000, 0x10000, CRC(d8494f96) SHA1(11bc5d73f030361de8e6d6434ccbeac02c61a9eb) )

	ROM_REGION( 0x800, "nvram", 0 )
	ROM_LOAD( "nvram", 0x00, 0x800, CRC(de0bde20) SHA1(559c6129894173f75ec0e197fc5edfd93a8f6913) )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "am27s19.ic39", 0x00, 0x20, CRC(af16359f) SHA1(1ff5c9d7807e52be09c0ded56fb68a47e41b3fcf) )
ROM_END

} // anonymous namespace


/*************************************************
*                 Game Driver                    *
*************************************************/

//    YEAR  NAME      PARENT  MACHINE   INPUT     STATE           INIT        ROT    COMPANY          FULLNAME                      FLAGS
GAME( 2001, ncarrera, 0,      ncarrera, ncarrera, ncarrera_state, empty_init, ROT0, "bootleg (J.T.)", "New Carrera - Version 2000", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) // needs MCU dump
