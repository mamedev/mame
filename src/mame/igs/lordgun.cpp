// license:BSD-3-Clause
// copyright-holders:Luca Elia
/*************************************************************************************************************

                                                -= IGS Lord Of Gun =-

                                          driver by   Luca Elia (l.elia@tin.it)
                                 skeleton driver by   David Haywood
                                  code decrypted by   unknown


CPU     :   68000 + Z80
Custom  :   IGS005, IGS006, IGS007, IGS008
Sound   :   M6295 + YM3812 or YMF278B
NVRAM   :   93C46

-------------------------------------------------------------------------------------------------
Year + Game                PCB              FM Sound  Custom IGS Chips             Notes
-------------------------------------------------------------------------------------------------
1994  Lord Of Gun (World)  T0062-2+NO.0064  YM3812    IGS005 IGS006 IGS007 IGS008  Lightguns
1994  Lord Of Gun (US)     T0076            YM3812    IGS005 IGS006 IGS007 IGS008  Lightguns
1994  Alien Challenge      NO-0085          YMF278B   IGS005 IGS006 IGS007 IGS008  Not encrypted
-------------------------------------------------------------------------------------------------

To do:

- lordgun: in the 3rd leg of the ship stage, sometimes part of a far jetboat is drawn above a nearer sub (both sprites).
  But this is correct considering both priorities and sprite list positions. Original game bug?
- lordgun: wrong colors for tilemap 0 in the 2nd leg of the last stage (where some sprite priority bugs happen too)
- lordgun: in the jungle level, final enemy, tilemap 0 does not scroll. It may have wrong priority, or may need to be
  disabled, even though it is used by enemies to hide, so it's probably just odd but right after all.

Notes:

- aliencha: when booting into service mode, keep buttons 1, 2 and 3 pressed to show more options
- aliencha: original videos at http://www.youtube.com/watch?v=TRHb3WTGuvk, http://www.youtube.com/watch?v=_RaCpHwyS78.
  The latter shows English text and a 1995 copyright (instead of 1994) in a different font.
- aliencha: routine at A34 - english/chinese text, A38 - english/chinese names, A3C - Alien Challenge/Round House Rumble title

*************************************************************************************************************/

#include "emu.h"
#include "lordgun.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "sound/ymopl.h"
#include "speaker.h"


/***************************************************************************

    Memory Maps - Main

***************************************************************************/

void lordgun_state::lordgun_protection_w(offs_t offset, uint16_t data)
{
	switch (offset & 0x60)
	{
		case 0x00/2: // increment counter
		{
			m_protection_data++;
			m_protection_data &= 0x1f;

			return;
		}

		case 0xc0/2: // reset protection device
		{
			m_protection_data = 0;

			return;
		}
	}
}

uint16_t lordgun_state::lordgun_protection_r(offs_t offset)
{
	switch (offset & 0x60)
	{
		case 0x40/2: // bitswap and xor counter
		{
			uint8_t x = m_protection_data;

			m_protection_data  = ((( x >> 0) | ( x >> 1)) & 1) << 4;
			m_protection_data |=  ((~x >> 2) & 1) << 3;
			m_protection_data |= (((~x >> 4) | ( x >> 0)) & 1) << 2;
			m_protection_data |=  (( x >> 3) & 1) << 1;
			m_protection_data |= (((~x >> 0) | ( x >> 2)) & 1) << 0;

			return 0;
		}

		case 0x80/2: // return value if conditions are met
		{
			if ((m_protection_data & 0x11) == 0x01) return 0x10;
			if ((m_protection_data & 0x06) == 0x02) return 0x10;
			if ((m_protection_data & 0x09) == 0x08) return 0x10;

			return 0;
		}
	}

	return 0;
}

void lordgun_state::aliencha_protection_w(offs_t offset, uint16_t data)
{
	switch (offset & 0x60)
	{
		case 0xc0/2: // reset protection device
		{
			m_protection_data = 0;

			return;
		}
	}
}

uint16_t lordgun_state::aliencha_protection_r(offs_t offset)
{
	switch (offset & 0x60)
	{
		case 0x00/2: // de-increment counter
		{
			m_protection_data--;
			m_protection_data &= 0x1f;

			return 0;
		}

		case 0x40/2: // bitswap and xor counter
		{
			uint8_t x = m_protection_data;

			m_protection_data  = (((x >> 3) ^ (x >> 2)) & 1) << 4;
			m_protection_data |= (((x >> 2) ^ (x >> 1)) & 1) << 3;
			m_protection_data |= (((x >> 1) ^ (x >> 0)) & 1) << 2;
			m_protection_data |= (((x >> 4) ^ (x >> 0)) & 1) << 1;
			m_protection_data |= (((x >> 4) ^ (x >> 3)) & 1) << 0;

			return 0;
		}

		case 0x80/2: // return value if conditions are met
		{
			if ((m_protection_data & 0x11) == 0x00) return 0x20;
			if ((m_protection_data & 0x06) != 0x06) return 0x20;
			if ((m_protection_data & 0x18) == 0x00) return 0x20;

			return 0;
		}
	}

	return 0;
}

void lordgun_state::fake_w(uint8_t data)
{
//  popmessage("%02x",data);
}

void lordgun_state::fake2_w(uint8_t data)
{
//  popmessage("%02x",data);
}

void lordgun_state::lordgun_eeprom_w(uint8_t data)
{
	int i;

	if (data & 2)
	{
//      popmessage("EE: %02x", data);
		logerror("%s: Unknown EEPROM bit written %02X\n",machine().describe_context(),data);
	}

	machine().bookkeeping().coin_counter_w(0, data & 0x01);

	// Update light guns positions
	for (i = 0; i < 2; i++)
		if ( (data & (0x04 << i)) && !(m_old & (0x04 << i)) )
			lordgun_update_gun(i);

	// latch the bit
	m_eeprom->di_write(BIT(data, 6));

	// reset line asserted: reset.
	m_eeprom->cs_write((data & 0x10) ? ASSERT_LINE : CLEAR_LINE );

	// clock line asserted: write latch or select next bit to read
	m_eeprom->clk_write((data & 0x20) ? ASSERT_LINE : CLEAR_LINE );

	m_whitescreen = data & 0x80;

	m_old = data;
}

void lordgun_state::aliencha_eeprom_w(uint8_t data)
{
	if (~data & 7)
	{
//      popmessage("EE: %02x", data);
		logerror("%s: Unknown EEPROM bit written %02X\n",machine().describe_context(),data);
	}

	// bit 1? cleared during screen transitions
	m_whitescreen = !(data & 0x02);

	machine().bookkeeping().coin_counter_w(0, data & 0x08);
	machine().bookkeeping().coin_counter_w(1, data & 0x10);

	// latch the bit
	m_eeprom->di_write(BIT(data, 7));

	// reset line asserted: reset.
	m_eeprom->cs_write((data & 0x20) ? ASSERT_LINE : CLEAR_LINE );

	// clock line asserted: write latch or select next bit to read
	m_eeprom->clk_write((data & 0x40) ? ASSERT_LINE : CLEAR_LINE );
}


uint8_t lordgun_state::aliencha_dip_r()
{
	switch (m_aliencha_dip_sel & 0x70)
	{
		case 0x30:  return ioport("DIP1")->read();
		case 0x60:  return ioport("DIP2")->read();
		case 0x50:  return ioport("DIP3")->read();

		default:
			logerror("%s: dip_r with unknown dip_sel = %02X\n",machine().describe_context(),m_aliencha_dip_sel);
			return 0xff;
	}
}

void lordgun_state::aliencha_dip_w(uint8_t data)
{
	m_aliencha_dip_sel = data;
}

// Unknown, always equal to 7 in lordgun, aliencha.
void lordgun_state::priority_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_priority);
}


uint16_t lordgun_state::lordgun_gun_0_x_r()
{
	return m_gun[0].hw_x;
}

uint16_t lordgun_state::lordgun_gun_0_y_r()
{
	return m_gun[0].hw_y;
}

uint16_t lordgun_state::lordgun_gun_1_x_r()
{
	return m_gun[1].hw_x;
}

uint16_t lordgun_state::lordgun_gun_1_y_r()
{
	return m_gun[1].hw_y;
}

void lordgun_state::soundlatch_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)     m_soundlatch->write((data >> 0) & 0xff);
	if (ACCESSING_BITS_8_15)    m_soundlatch2->write((data >> 8) & 0xff);

	m_soundcpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

template<int Layer>
void lordgun_state::vram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_vram[Layer][offset]);
	m_tilemap[Layer]->mark_tile_dirty(offset/2);
}

void lordgun_state::common_map(address_map &map)
{
	map(0x200000, 0x20ffff).ram();
	map(0x210000, 0x21ffff).ram().share("priority_ram");                        // PRIORITY
	map(0x300000, 0x30ffff).ram().w(FUNC(lordgun_state::vram_w<0>)).share("vram.0");  // DISPLAY (BACKGROUND 1 for aliencha)
	map(0x310000, 0x313fff).ram().w(FUNC(lordgun_state::vram_w<1>)).share("vram.1");  // DISPLAY (BACKGROUND 2 for aliencha)
	map(0x314000, 0x314fff).ram().w(FUNC(lordgun_state::vram_w<2>)).share("vram.2");  // DISPLAY (BACKGROUND 3 for aliencha)
	map(0x315000, 0x317fff).ram();                                                     //
	map(0x318000, 0x319fff).ram().w(FUNC(lordgun_state::vram_w<3>)).share("vram.3");  // DISPLAY (TEXT for aliencha)
	map(0x31c000, 0x31c7ff).ram().share("scrollram");                           // LINE (LINE OFFSET for aliencha)
	map(0x400000, 0x4007ff).ram().share("spriteram");                       // ANIMATOR (ANIMATE for aliencha)
	map(0x500000, 0x500fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x502000, 0x502001).writeonly().share("scroll_x.0");
	map(0x502200, 0x502201).writeonly().share("scroll_x.1");
	map(0x502400, 0x502401).writeonly().share("scroll_x.2");
	map(0x502600, 0x502601).writeonly().share("scroll_x.3");
	map(0x502800, 0x502801).writeonly().share("scroll_y.0");
	map(0x502a00, 0x502a01).writeonly().share("scroll_y.1");
	map(0x502c00, 0x502c01).writeonly().share("scroll_y.2");
	map(0x502e00, 0x502e01).writeonly().share("scroll_y.3");
	map(0x503000, 0x503001).w(FUNC(lordgun_state::priority_w));
	map(0x504000, 0x504001).w(FUNC(lordgun_state::soundlatch_w));
	map(0x506000, 0x506007).rw("ppi8255_0", FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0x00ff);
	map(0x508000, 0x508007).rw("ppi8255_1", FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0x00ff);
}

void lordgun_state::lordgun_map(address_map &map)
{
	common_map(map);
	map(0x000000, 0x0fffff).rom();
	map(0x503800, 0x503801).r(FUNC(lordgun_state::lordgun_gun_0_x_r));
	map(0x503a00, 0x503a01).r(FUNC(lordgun_state::lordgun_gun_1_x_r));
	map(0x503c00, 0x503c01).r(FUNC(lordgun_state::lordgun_gun_0_y_r));
	map(0x503e00, 0x503e01).r(FUNC(lordgun_state::lordgun_gun_1_y_r));
	map(0x50a900, 0x50a9ff).rw(FUNC(lordgun_state::lordgun_protection_r), FUNC(lordgun_state::lordgun_protection_w));
}


void lordgun_state::aliencha_map(address_map &map)
{
	common_map(map);
	map(0x000000, 0x1fffff).rom();
	map(0x50b900, 0x50b9ff).rw(FUNC(lordgun_state::aliencha_protection_r), FUNC(lordgun_state::aliencha_protection_w));
}

void lordgun_state::ymf278_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom();
}

/***************************************************************************

    Memory Maps - Sound

***************************************************************************/

void lordgun_state::soundmem_map(address_map &map)
{
	map(0x0000, 0xefff).rom();
	map(0xf000, 0xffff).ram();
}

void lordgun_state::lordgun_okibank_w(uint8_t data)
{
	m_oki->set_rom_bank((data >> 1) & 1);
	if (data & ~3)  logerror("%s: unknown okibank bits %02x\n", machine().describe_context(), data);
//  popmessage("OKI %x", data);
}

void lordgun_state::lordgun_soundio_map(address_map &map)
{
	map(0x1000, 0x1001).w("ymsnd", FUNC(ym3812_device::write));
	map(0x2000, 0x2000).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x3000, 0x3000).r(m_soundlatch2, FUNC(generic_latch_8_device::read));
	map(0x4000, 0x4000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0x5000, 0x5000).nopr();
	map(0x6000, 0x6000).w(FUNC(lordgun_state::lordgun_okibank_w));
}


void lordgun_state::aliencha_soundio_map(address_map &map)
{
	map(0x3000, 0x3000).r(m_soundlatch2, FUNC(generic_latch_8_device::read));
	map(0x4000, 0x4000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0x5000, 0x5000).nopw();    // writes 03 then 07 at end of NMI
	map(0x7000, 0x7000).r("ymf", FUNC(ymf278b_device::read));
	map(0x7000, 0x7005).w("ymf", FUNC(ymf278b_device::write));
	map(0x7400, 0x7400).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x7800, 0x7800).rw("oki2", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
}


/***************************************************************************

    Graphics Layout

***************************************************************************/

static const gfx_layout lordgun_8x8x6_layout =
{
	8,8,
	RGN_FRAC(1,3),
	6,
	{   RGN_FRAC(2,3)+8, RGN_FRAC(2,3)+0,
		RGN_FRAC(1,3)+8, RGN_FRAC(1,3)+0,
		RGN_FRAC(0,3)+8, RGN_FRAC(0,3)+0 },
	{ STEP8(0,1) },
	{ STEP8(0,8*2) },
	8*8*2
};

static const gfx_layout lordgun_16x16x6_layout =
{
	16,16,
	RGN_FRAC(1,3),
	6,
	{   RGN_FRAC(2,3)+8, RGN_FRAC(2,3)+0,
		RGN_FRAC(1,3)+8, RGN_FRAC(1,3)+0,
		RGN_FRAC(0,3)+8, RGN_FRAC(0,3)+0 },
	{ STEP8(0,1),STEP8(8*16*2,1) },
	{ STEP16(0,8*2) },
	16*16*2
};

static const gfx_layout lordgun_32x32x6_layout =
{
	32,32,
	RGN_FRAC(1,3),
	6,
	{   RGN_FRAC(2,3)+8, RGN_FRAC(2,3)+0,
		RGN_FRAC(1,3)+8, RGN_FRAC(1,3)+0,
		RGN_FRAC(0,3)+8, RGN_FRAC(0,3)+0 },
	{ STEP8(0,1),STEP8(8*32*2,1),STEP8(8*32*2*2,1),STEP8(8*32*2*3,1) },
	{ STEP16(0,8*2),STEP16(16*8*2,8*2) },
	32*32*2
};

static GFXDECODE_START( gfx_lordgun )
	GFXDECODE_ENTRY( "tiles0",  0, lordgun_8x8x6_layout,    0x000, 0x800/0x40*8  )  // [0] Tilemap 0
	GFXDECODE_ENTRY( "tiles1",  0, lordgun_16x16x6_layout,  0x000, 0x800/0x40*8  )  // [1] Tilemap 1
	GFXDECODE_ENTRY( "tiles1",  0, lordgun_32x32x6_layout,  0x000, 0x800/0x40*8  )  // [2] Tilemap 2
	GFXDECODE_ENTRY( "tiles0",  0, lordgun_8x8x6_layout,    0x000, 0x800/0x40*8  )  // [3] Tilemap 3
	GFXDECODE_ENTRY( "sprites", 0, lordgun_16x16x6_layout,  0x000, 0x800/0x40*8  )  // [4] Sprites
GFXDECODE_END


/***************************************************************************

    Input Ports

***************************************************************************/

static INPUT_PORTS_START( lordgun )
	PORT_START("DIP")
	PORT_DIPNAME( 0x01, 0x01, "Stage Select" )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Guns" )      PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, "IGS" )     // x table offset  = 0x25
	PORT_DIPSETTING(    0x00, "Konami" )  // "" = 0x2c
	PORT_DIPNAME( 0x04, 0x04, "Ranking Music" ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, "Exciting" )
	PORT_DIPSETTING(    0x00, "Tender" )
	PORT_DIPNAME( 0x08, 0x08, "Coin Slots" )    PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_SERVICE_NO_TOGGLE( 0x40, IP_ACTIVE_LOW )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))

	PORT_START("START1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1   )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE2 ) // game cheat: skip stage

	PORT_START("START2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2  )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN  )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1   ) PORT_IMPULSE(5)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2   ) PORT_IMPULSE(5)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("LIGHT0_X")
	PORT_BIT( 0x1ff, 0x100, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX(0x3c,0x1d8) PORT_SENSITIVITY(35) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("LIGHT1_X")
	PORT_BIT( 0x1ff, 0x100, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX(0x3c,0x1d8) PORT_SENSITIVITY(35) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("LIGHT0_Y")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_MINMAX(0,224) PORT_SENSITIVITY(35) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("LIGHT1_Y")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_MINMAX(0,224) PORT_SENSITIVITY(35) PORT_KEYDELTA(10) PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( aliencha )
	PORT_START("DIP1")
	PORT_DIPNAME( 0x01, 0x01, "Credits To Start" )      PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x0e, 0x0e, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:2,3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x70, 0x70, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:5,6,7")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x80, 0x80, "Coin Slots" )        PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, "1" )
	PORT_DIPSETTING(    0x00, "2" )

	PORT_START("DIP2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_SERVICE_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW2:3" )
	PORT_DIPNAME( 0x08, 0x08, "Round Time" )        PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x00, "32 s" )  // 40 s (measured) AKA "Short"
	PORT_DIPSETTING(    0x08, "40 s" )  // 50 s (measured) AKA "Normal"
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Free_Play ) )    PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Allow Join" )        PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )

	PORT_START("DIP3")
	PORT_DIPNAME( 0x03, 0x03, "Buttons" )           PORT_DIPLOCATION("SW3:1,2")
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x03, "6" )
//  PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x04, 0x04, "Vs. Rounds" )        PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPUNUSED_DIPLOC( 0x0008, 0x0008, "SW3:4" ) /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0010, "SW3:5" ) /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0020, "SW3:6" ) /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0040, "SW3:7" ) /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0080, 0x0080, "SW3:8" ) /* Listed as "Unused" */

	PORT_START("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN  )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1         )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1        ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2        ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3        ) PORT_PLAYER(1)

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2         )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1        ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2        ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3        ) PORT_PLAYER(2)

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1          )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2          )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4        ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON5        ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON6        ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON4        ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON5        ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON6        ) PORT_PLAYER(2)
INPUT_PORTS_END


/***************************************************************************

    Machine Drivers

***************************************************************************/

void lordgun_state::machine_start()
{
	save_item(NAME(m_protection_data));
	save_item(NAME(m_priority));
	save_item(NAME(m_whitescreen));
}

void lordgun_state::lordgun(machine_config &config)
{
	M68000(config, m_maincpu, 20_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &lordgun_state::lordgun_map);
	m_maincpu->set_vblank_int("screen", FUNC(lordgun_state::irq4_line_hold));

	Z80(config, m_soundcpu, 20_MHz_XTAL / 4);
	m_soundcpu->set_addrmap(AS_PROGRAM, &lordgun_state::soundmem_map);
	m_soundcpu->set_addrmap(AS_IO, &lordgun_state::lordgun_soundio_map);

	i8255_device &ppi0(I8255A(config, "ppi8255_0"));
	ppi0.in_pa_callback().set_ioport("DIP");
	ppi0.out_pa_callback().set(FUNC(lordgun_state::fake_w));
	ppi0.out_pb_callback().set(FUNC(lordgun_state::lordgun_eeprom_w));
	ppi0.in_pc_callback().set_ioport("SERVICE");
	ppi0.out_pc_callback().set(FUNC(lordgun_state::fake2_w));

	i8255_device &ppi1(I8255A(config, "ppi8255_1"));
	ppi1.in_pa_callback().set_ioport("START1");
	ppi1.out_pa_callback().set(FUNC(lordgun_state::fake_w));
	ppi1.in_pb_callback().set_ioport("START2");
	ppi1.out_pb_callback().set(FUNC(lordgun_state::fake_w));
	ppi1.in_pc_callback().set_ioport("COIN");
	ppi1.out_pc_callback().set(FUNC(lordgun_state::fake_w));

	EEPROM_93C46_16BIT(config, m_eeprom);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(0x200, 0x100);
	m_screen->set_visarea(0,0x1c0-1, 0,0xe0-1);
	m_screen->set_screen_update(FUNC(lordgun_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_lordgun);

	// 0x800 real colors, repeated per priority level
	PALETTE(config, m_palette).set_format(palette_device::xBGR_444, 0x800 * 8);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	GENERIC_LATCH_8(config, m_soundlatch2);

	ym3812_device &ymsnd(YM3812(config, "ymsnd", 3.579545_MHz_XTAL));
	ymsnd.irq_handler().set_inputline("soundcpu", 0);
	ymsnd.add_route(ALL_OUTPUTS, "mono", 1.0);

	OKIM6295(config, m_oki, 20_MHz_XTAL / 20, okim6295_device::PIN7_HIGH);
	m_oki->add_route(ALL_OUTPUTS, "mono", 1.0);
}

void lordgun_state::aliencha(machine_config &config)
{
	M68000(config, m_maincpu, 20_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &lordgun_state::aliencha_map);
	m_maincpu->set_vblank_int("screen", FUNC(lordgun_state::irq4_line_hold));

	Z80(config, m_soundcpu, 20_MHz_XTAL / 4);
	m_soundcpu->set_addrmap(AS_PROGRAM, &lordgun_state::soundmem_map);
	m_soundcpu->set_addrmap(AS_IO, &lordgun_state::aliencha_soundio_map);

	i8255_device &ppi0(I8255A(config, "ppi8255_0"));
	ppi0.in_pa_callback().set(FUNC(lordgun_state::aliencha_dip_r));
	ppi0.out_pa_callback().set(FUNC(lordgun_state::fake2_w));
	ppi0.out_pb_callback().set(FUNC(lordgun_state::aliencha_eeprom_w));
	ppi0.in_pc_callback().set_ioport("SERVICE");
	ppi0.out_pc_callback().set(FUNC(lordgun_state::aliencha_dip_w));

	i8255_device &ppi1(I8255A(config, "ppi8255_1"));
	ppi1.in_pa_callback().set_ioport("P1");
	ppi1.out_pa_callback().set(FUNC(lordgun_state::fake_w));
	ppi1.in_pb_callback().set_ioport("P2");
	ppi1.out_pb_callback().set(FUNC(lordgun_state::fake_w));
	ppi1.in_pc_callback().set_ioport("COIN");
	ppi1.out_pc_callback().set(FUNC(lordgun_state::fake_w));

	EEPROM_93C46_16BIT(config, m_eeprom);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(0x200, 0x100);
	m_screen->set_visarea(0,0x1c0-1, 0,0xe0-1);
	m_screen->set_screen_update(FUNC(lordgun_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_lordgun);

	// 0x800 real colors, repeated per priority level
	PALETTE(config, m_palette).set_format(palette_device::xBGR_444, 0x800 * 8);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	GENERIC_LATCH_8(config, m_soundlatch2);

	ymf278b_device &ymf(YMF278B(config, "ymf", 33.8688_MHz_XTAL));
	ymf.set_addrmap(0, &lordgun_state::ymf278_map);
	ymf.irq_handler().set_inputline("soundcpu", 0);
	ymf.add_route(ALL_OUTPUTS, "mono", 0.5);

	OKIM6295(config, m_oki, 20_MHz_XTAL / 20, okim6295_device::PIN7_HIGH);
	m_oki->add_route(ALL_OUTPUTS, "mono", 0.5);

	OKIM6295(config, "oki2", 20_MHz_XTAL / 20, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.5);
}


/***************************************************************************

    ROMs Loading

***************************************************************************/

/***************************************************************************

Lord of Gun
IGS, 1994

PCB Layout
----------

IGSPCB NO. T0076
--------------------------------------------------------
| YM3014           62256      IGS008  IGS006   IGST003 |
| YM3812      6295 62256                       IGST002 |
|       3.57945MHz 62256                       IGST001 |
|                  62256                               |
|6116 LORDGUN.100                              IGSB003 |
|     Z80               62256                  IGSB002 |
|LORDGUN.90                                    IGSB001 |
|J    PAL              6116                            |
|A    PAL              6116                       6116 |
|M                          IGS005                6116 |
|M   68000P10 PAL                                 6116 |
|A                          PAL     PAL           6116 |
|                           PAL     6116               |
|                           PAL     6116        IGS007 |
|                           PAL     6116         20MHz |
|       DSW1(4)                     6116 PAL           |
|             62256    62256          IGSA001 IGSA004  |
|      8255          LORDGUN.10       IGSA002 IGSA005  |
|93C46 8255          LORDGUN.4        IGSA003 IGSA006  |
--------------------------------------------------------

HW Notes:
      68k clock: 10.000MHz (20.000MHz/2)
      Z80 clock: 5.000MHz (20.000MHz/4)
          VSync: 60Hz
          HSync: 15.15kHz
   YM3812 clock: 3.57945MHz
 OKI 6295 clock: 1.000MHz (20.000MHz/20)
  OKI 6295 pin7: HI

  All frequencies are checked with my frequency counter (i.e. they are not guessed)

  IGST* are 8M devices
  IGSA* and IGSB* are 16M devices
  LORDGUN.90 is 27C512
  LORDGUN.100 \
  LORDGUN.10  | 27C040
  LORDGUN.4   /

--------------------------------------------------

Lord of Gun - World version (c) 1994 IGS

PCB: IGS PCB NO.T0062-2 with a IGS PCB NO.0064 ROM board

  Main: MC68000P10 (10MHz rated)
   Sub: Zilog Z0840006PCS (Z80 6MHz rated)
 Sound: OKI M6295, Yamaha YM3812-F + Y3014B-F
   OSC: 20.000MHz, 3.579545MHz
EEPROM: NMC 9346N

1 Push Button - Test/Setup Mode

Custom chips:
IGS 005 (144 Pin PQFP)
IGS 006 (144 Pin PQFP)
IGS 007 (144 Pin PQFP)
Lattice pLSI 1024-60LJ instead of IGS 008

The PCB NO.0064 ROM board has sockets for 9 mask ROMs and 12 EPROMs:

IGS A001
IGS A002
IGS A003

IGS B001
IGS B002
IGS B003

IGS T001
IGS T002
IGS T003

EPROMs replace IGS A004 through IGS A006, EPROM data is interleaved ROMs with pairs
 matching the lower half of each mask ROM while the upper half has different graphics
 data but again interleaved.

NOTE: This set is for use in all Countries excluding USA, Canada, Mexico, Japan & Taiwan.

--------------------------------------------------

Lord of Gun - USA, Canada & Mexico (c) 1994 IGS

PCB: IGSPCB NO.T0076

  Main: MC68000P10 (10MHz rated)
   Sub: Zilog Z0840006PCS (Z80 6MHz rated)
 Sound: OKI M6295, Yamaha YM3812-F + Y3014B-F
   OSC: 20.000MHz, 3.579545MHz
EEPROM: NMC 9346N

1 Push Button - Test/Setup Mode

Custom chips:
IGS 005 (144 Pin PQFP)
IGS 006 (144 Pin PQFP)
IGS 007 (144 Pin PQFP)
IGS 008 (160 Pin PQFP)

lg_u122.m3 - Labelled as "LORD GUN U122-M3" MX 27C4000  (Note: U122 is for the IGS PCB NO.T0062-2)
lg_u144.m3 - Labelled as "LORD GUN U144-M3" MX 27C4000  (Note: U144 is for the IGS PCB NO.T0062-2)

lordgun.u90  - Labelled as "LORD GUN U90"  27C512
lordgunu.100 - Labelled as "LORD GUN U100" MX 27C4000

Surface mounted ROMs (42 pin DIP)

2 Unmarked ROM(?) chips

IGS A001
IGS A002
IGS A003
IGS A004
IGS A005
IGS A006

IGS B001
IGS B002
IGS B003

IGS T001
IGS T002
IGS T003

--------------------------------------------------

NOTE:  The following program ROM labels have been seen
       but not dumped to verify versions:

LORD/GUN U10-D3  (USA version, but not verfied to be currently dumped version)
LORD/GUN U4-D3

LORD/GUN U10-J3  (possibly a Japanese version?)
LORD/GUN U4-J3
LORD/GUN U100-1

LORD/GUN U10-J4  (possibly a Japanese version?)
LORD/GUN U4-J4
LORD/GUN U100-1

LORD/GUN U10-K5  (possibly a Korean version?)
LORD/GUN U4-K5
LORD/GUN U100-1

--------------------------------------------------

DIP Switch-1 (4 Position DIP)
--------------------------------------------------
    DipSwitch Title   | Function | 1 | 2 | 3 | 4 |
--------------------------------------------------
       Game Mode      |  Arcade  |off|           |
                      |  Street  |on |           |
--------------------------------------------------
       Selection      |   IGS    |   |off|       |
        of Guns       |  Konami  |   |on |       |
--------------------------------------------------
       Ranking        | Exciting |       |off|   |
      Background      |  Tender  |       |on |   |
--------------------------------------------------
      Coin Slots      | Separate |           |off|
                      |  Common  |           |on |
--------------------------------------------------
     Settings Upon Shipping      |off|off|off|off|
--------------------------------------------------

Game modes explained:
 In "Arcade Mode" players could play this game by entering each scene in a
  pre-defined order.
 In "Street Mode" this game now presents 10 selectable scenes for players,
  not 4 any more.  After all scenes are passed (except training courses),
  players can enter the last scene; the Headquarters

                       Lord of Gun JAMMA Pinout

                        Main Jamma Connector
          Solder Side            |             Parts Side
------------------------------------------------------------------
             GND             | A | 1 |             GND
             GND             | B | 2 |             GND
             +5              | C | 3 |             +5
             +5              | D | 4 |             +5
                             | E | 5 |
             +12             | F | 6 |             +12
------------ KEY ------------| H | 7 |------------ KEY -----------
                             | J | 8 |       Coin Counter
                             | K | 9 |
        Speaker (-)          | L | 10|        Speaker (+)
                             | M | 11|
        Video Green          | N | 12|        Video Red
        Video Sync           | P | 13|        Video Blue
                             | R | 14|        Video GND
                             | S | 15|        Test Switch
        Coin Switch 2        | T | 16|        Coin Switch 1
        Start Player 2       | U | 17|        Start Player 1
                             | V | 18|
                             | W | 19|
                             | X | 20|
                             | Y | 21|
                             | Z | 22|
                             | a | 23|
                             | b | 24|
                             | c | 25|
                             | d | 26|
             GND             | e | 27|             GND
             GND             | f | 28|             GND


NOTE: Speakers should be connected serially to Speaker (+) and Speaker (-).
      You must avoid connecting speakers parallelly or connecting speakers
      to Speaker (+) and GND, to keep the amplifier from being damaged or
      from malfunctioning.

 JP1: Player 1 Gun Connector Pinout

   1| +5 Volts - RED Wire    (Manual says "VCC")
   2| Trigger  - White Wire
   3| Ground   - Black Wire
   4| Gun OPTO - Blue Wire   (Manual says "HIT")

 JP2: Player 2 Gun Connector Pinout

   1| +5 Volts - RED Wire    (Manual says "VCC")
   2| Trigger  - White Wire
   3| Ground   - Black Wire
   4| Gun OPTO - Blue Wire   (Manual says "HIT")

***************************************************************************/

ROM_START( lordgun ) // World set, excluding USA, Canada, Mexico, Japan & Taiwan
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000
	ROM_LOAD16_BYTE( "lord_gun_u144-ch.u144", 0x00000, 0x80000, CRC(ea54ee18) SHA1(5ce3fe0d46c524562a627fe7418dab9753cf74b3) ) // hand written label  LORD/GUN   U144-CH
	ROM_LOAD16_BYTE( "lord_gun_u122-ch.u122", 0x00001, 0x80000, CRC(969a0348) SHA1(6ce064c7c42c51969ea1271c6f75fc602602cfa3) ) // hand written label  LORD/GUN   U122-CH

	ROM_REGION( 0x010000, "soundcpu", 0 ) // Z80
	ROM_LOAD( "lord_gun_160.u160", 0x00000, 0x10000, CRC(d59b5e28) SHA1(36696058684d69306f463ed543c8b0195bafa21e) ) // == lord_gun_u90.u90 - 1xxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x300000, "tiles0", 0 ) // Tilemaps 0 & 3
	ROM_LOAD( "igs_t001.u8",  0x000000, 0x100000, CRC(36dd96f3) SHA1(4e70eb807160e7ed1b19d7f38df3a38021f42d9b) ) // same data as lordgunu set, different U location
	ROM_LOAD( "igs_t002.u18", 0x100000, 0x100000, CRC(816a7665) SHA1(f2f2624ab262c957f84c657cfc432d14c61b19e8) )
	ROM_LOAD( "igs_t003.u19", 0x200000, 0x100000, CRC(cbfee543) SHA1(6fad8ef8d683f709f6ff2b16319447516c372fc8) )

	ROM_REGION( 0x600000, "tiles1", 0 ) // Tilemaps 1 & 2
	ROM_LOAD( "igs_b001.u1", 0x000000, 0x200000, CRC(3096de1c) SHA1(d010990d21cfda9cb8ab5b4bc0e329c23b7719f5) ) // same data as lordgunu set, different U location
	ROM_LOAD( "igs_b002.u2", 0x200000, 0x200000, CRC(2234531e) SHA1(58a82e31a1c0c1a4dd026576319f4e7ecffd140e) )
	ROM_LOAD( "igs_b003.u9", 0x400000, 0x200000, CRC(6cbf21ac) SHA1(ad25090a00f291aa48929ffa01347cc53e0051f8) )

	ROM_REGION( 0xc00000, "sprites", 0 ) // Sprites
	ROM_LOAD( "igs_a001.u22",             0x000000, 0x200000, CRC(400abe33) SHA1(20de1eb626424ea41bd55eb3cecd6b50be744ee0) ) // == igs_a001.u14, different U location
	ROM_LOAD16_BYTE( "lord_gun_u24.u24",  0x200000, 0x080000, CRC(454a5b11) SHA1(2e2e16e5f61c6e4648252ef172409efabbaa0b80) ) // == 1/2 IGS A004 even
	ROM_LOAD16_BYTE( "lord_gun_u23.u23",  0x200001, 0x080000, CRC(a0d7aada) SHA1(3c6d4053e0930dcfdd43c7eb85ca2ba2297c136f) ) // == 1/2 IGS A004 odd
	ROM_LOAD16_BYTE( "lord_gun_u7.u7",    0x300000, 0x080000, CRC(95ef3894) SHA1(0962e2545f7d0551a8272d87ea21401d2c5b8d80) )
	ROM_LOAD16_BYTE( "lord_gun_u14.u14",  0x300001, 0x080000, CRC(dc8a77a1) SHA1(0247acda7556c300e1baa9b726ec835523456761) )
	ROM_LOAD( "igs_a002.u21",             0x400000, 0x200000, CRC(a4810e38) SHA1(c31fe641feab2c93795fc35bf71d4f37af1056d4) ) // == igs_a002.u9, different U location
	ROM_LOAD16_BYTE( "lord_gun_u5.u5",    0x600000, 0x080000, CRC(63aa10c3) SHA1(743207237409319aec79dc2a486736247a582ca9) ) // == 1/2 IGS A005 even
	ROM_LOAD16_BYTE( "lord_gun_u13.u13",  0x600001, 0x080000, CRC(478e248c) SHA1(3b2c4cdf262fe766015e8c12a4fe356da8d88f84) ) // == 1/2 IGS A005 odd
	ROM_LOAD16_BYTE( "lord_gun_u4.u4",    0x700000, 0x080000, CRC(d203c24e) SHA1(d90ba0ea0f823a8013a5a2be50e1c36af96f9efd) )
	ROM_LOAD16_BYTE( "lord_gun_u11.u11",  0x700001, 0x080000, CRC(72277dcd) SHA1(89f6ad1f6813751fcccc15ee5ab6ce63b93a9b13) )
	ROM_LOAD( "igs_a003.u20",             0x800000, 0x200000, CRC(649e48d9) SHA1(ce346154024cf13f3e40000ceeb4c2003cd35894) ) // == igs_a003.u3, different U location
	ROM_LOAD16_BYTE( "lord_gun_u12.u12",  0xa00000, 0x080000, CRC(a2a55d65) SHA1(aad9ee5b9a93a5cd14a088c4bade9bab34f9e206) ) // == 1/2 IGS A006 even
	ROM_LOAD16_BYTE( "lord_gun_u6.u6",    0xa00001, 0x080000, CRC(fe649605) SHA1(80c22099ed630bdc825ebd910bcc48caad72e647) ) // == 1/2 IGS A006 odd
	ROM_LOAD16_BYTE( "lord_gun_u10.u10",  0xb00000, 0x080000, CRC(eea39e5e) SHA1(806a97a08d0108509d30732b507e1064215295c6) )
	ROM_LOAD16_BYTE( "lord_gun_u3.u3",    0xb00001, 0x080000, CRC(233782f8) SHA1(ef2049aadbcf5c409275ecfbbe75bdade5b087d4) )

	ROM_REGION( 0x080000, "oki", 0 ) // Samples
	ROM_LOAD( "lord_gun_u161-3.u161", 0x00000, 0x80000, CRC(b4e0fa07) SHA1(f5f33fe3f3a124f4737751fda3ea409fceeec0be) ) // == lord_gun_u100.u100

	ROM_REGION( 0x80, "eeprom", ROMREGION_LE|ROMREGION_16BIT ) // Default eeprom
	ROM_LOAD( "eeprom", 0x00, 0x80, CRC(0dad0e43) SHA1(c216d1f19228e103b78e5acb30a66dab3804ac70) )
ROM_END

ROM_START( lordgunu ) // USA, Canada & Mexico
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000
	ROM_LOAD16_BYTE( "lord_gun_u10.u10", 0x00000, 0x80000, CRC(acda77ef) SHA1(7cd8580419e2f62a3b5a1e4a6020a3ef978ff1e8) ) // == LORD GUN U144-M3?
	ROM_LOAD16_BYTE( "lord_gun_u4.u4",   0x00001, 0x80000, CRC(a1a61254) SHA1(b0c5aa656024cfb9be28a11061656159e7b72d00) ) // == LORD GUN U122-M3?

	ROM_REGION( 0x010000, "soundcpu", 0 ) // Z80
	ROM_LOAD( "lord_gun_u90.u90", 0x00000, 0x10000, CRC(d59b5e28) SHA1(36696058684d69306f463ed543c8b0195bafa21e) ) // labeled as LORD/GUN U90 - 1xxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x300000, "tiles0", 0 ) // Tilemaps 0 & 3
	ROM_LOAD( "igs_t001.u108", 0x000000, 0x100000, CRC(36dd96f3) SHA1(4e70eb807160e7ed1b19d7f38df3a38021f42d9b) )
	ROM_LOAD( "igs_t002.u114", 0x100000, 0x100000, CRC(816a7665) SHA1(f2f2624ab262c957f84c657cfc432d14c61b19e8) )
	ROM_LOAD( "igs_t003.u119", 0x200000, 0x100000, CRC(cbfee543) SHA1(6fad8ef8d683f709f6ff2b16319447516c372fc8) )

	ROM_REGION( 0x600000, "tiles1", 0 ) // Tilemaps 1 & 2
	ROM_LOAD( "igs_b001.u82", 0x000000, 0x200000, CRC(3096de1c) SHA1(d010990d21cfda9cb8ab5b4bc0e329c23b7719f5) )
	ROM_LOAD( "igs_b002.u91", 0x200000, 0x200000, CRC(2234531e) SHA1(58a82e31a1c0c1a4dd026576319f4e7ecffd140e) )
	ROM_LOAD( "igs_b003.u97", 0x400000, 0x200000, CRC(6cbf21ac) SHA1(ad25090a00f291aa48929ffa01347cc53e0051f8) )

	ROM_REGION( 0xc00000, "sprites", 0 ) // Sprites
	ROM_LOAD( "igs_a001.u14", 0x000000, 0x200000, CRC(400abe33) SHA1(20de1eb626424ea41bd55eb3cecd6b50be744ee0) )
	ROM_LOAD( "igs_a004.u13", 0x200000, 0x200000, CRC(52687264) SHA1(28444cf6b5662054e283992857e0827a2ca15b83) )
	ROM_LOAD( "igs_a002.u9",  0x400000, 0x200000, CRC(a4810e38) SHA1(c31fe641feab2c93795fc35bf71d4f37af1056d4) )
	ROM_LOAD( "igs_a005.u8",  0x600000, 0x200000, CRC(e32e79e3) SHA1(419f9b501e5a37d763ece9322271e61035b50217) )
	ROM_LOAD( "igs_a003.u3",  0x800000, 0x200000, CRC(649e48d9) SHA1(ce346154024cf13f3e40000ceeb4c2003cd35894) )
	ROM_LOAD( "igs_a006.u2",  0xa00000, 0x200000, CRC(39288eb6) SHA1(54d157f0e151f6665f4288b4d09bd65571005132) )

	ROM_REGION( 0x080000, "oki", 0 ) // Samples
	ROM_LOAD( "lord_gun_u100.u100", 0x00000, 0x80000, CRC(b4e0fa07) SHA1(f5f33fe3f3a124f4737751fda3ea409fceeec0be) ) // labeled as LORD/GUN U100

	ROM_REGION( 0x80, "eeprom", ROMREGION_LE|ROMREGION_16BIT ) // Default eeprom
	ROM_LOAD( "eeprom", 0x00, 0x80, CRC(0dad0e43) SHA1(c216d1f19228e103b78e5acb30a66dab3804ac70) )
ROM_END


/***************************************************************************

  Alien Challenge (World)
  (C) 1994 IGS
  01/16/95 21:51:28 in test mode

***************************************************************************/

ROM_START( aliencha )
	ROM_REGION( 0x200000, "maincpu", 0 ) // 68000
	ROM_LOAD( "igsc0102.u81", 0x00000, 0x200000, CRC(e3432be3) SHA1(d3597c885571d4a996afaaf29c78da123798371e) )

	ROM_REGION( 0x010000, "soundcpu", 0 ) // Z80
	ROM_LOAD( "hfh_s.u86", 0x00000, 0x10000, CRC(5728a9ed) SHA1(e5a9e4a1a2cc6c848b08608bc8727bc739270873) )

	ROM_REGION( 0x300000, "tiles0", 0 ) // Tilemaps 0 & 3
	ROM_LOAD( "igst0101.u9",  0x000000, 0x100000, CRC(2ce12d7b) SHA1(aa93a82e5f4015c46bb705efb2051b62cd5d7e04) )
	ROM_LOAD( "igst0102.u10", 0x100000, 0x100000, CRC(542a76a0) SHA1(6947b50a024d0053c1eaf9da8c90652bab875142) )
	ROM_LOAD( "igst0103.u11", 0x200000, 0x100000, CRC(adf5698a) SHA1(4b798f8acc5d7581c7e0989260863ae0ca654acd) )

	ROM_REGION( 0x600000, "tiles1", 0 ) // Tilemaps 1 & 2
	ROM_LOAD( "igsb0101.u8", 0x000000, 0x200000, CRC(5c995f7e) SHA1(4f08cf13e313c6802c924b914c73cab4b450da61) )
	ROM_LOAD( "igsb0102.u7", 0x200000, 0x200000, CRC(a2ae9baf) SHA1(338ee260c33448568f138ca00e1d4edda4da018f) )
	ROM_LOAD( "igsb0103.u6", 0x400000, 0x200000, CRC(11b927af) SHA1(2f15e5cea1b86cde3b679bdd0f3d79672d0ddd3e) )

	ROM_REGION( 0xc00000, "sprites", 0 ) // Sprites
	ROM_LOAD( "igsa0101.u3", 0x000000, 0x400000, CRC(374d07c4) SHA1(87e9bfe32cbfe9964ba7253847fbd14aa3c8ed20) )
	ROM_LOAD( "igsa0102.u2", 0x400000, 0x400000, CRC(dbeee7ac) SHA1(e0eb0d73d9230aa6f69f5ac25d44fa19affebe88) )
	ROM_LOAD( "igsa0103.u1", 0x800000, 0x400000, CRC(e5f19041) SHA1(c92a29bbbcb9a1f63364c665e3e0f9679add4389) )

	ROM_REGION( 0x40000, "oki", 0 ) // Samples
	ROM_LOAD( "hfh_g.u65", 0x00000, 0x40000, CRC(ec469b57) SHA1(ba1668078987ad51f47bcd3e61c51a0cf2545350) )

	ROM_REGION( 0x40000, "oki2", 0 ) // Samples
	ROM_LOAD( "hfh_g.u66", 0x00000, 0x40000, CRC(7cfcd98e) SHA1(3b03123160adfd3404a9e0c4c68420930e80ae48) )

	ROM_REGION( 0x200000, "ymf", 0 ) // Samples (Standard Yamaha YRW801 2MB samples ROM)
	ROM_LOAD( "yrw801-m", 0x000000, 0x200000, CRC(2a9d8d43) SHA1(32760893ce06dbe3930627755ba065cc3d8ec6ca) ) /* Not dumped from PCB, but is a standard samples rom */
ROM_END

/***************************************************************************

  Alien Challenge (China)
  (C) 1994 IGS
  12/13/94 13:55:47 in test mode

MADE IN TAIWAN IGS PCB NO-0085
+--------------------------------------------------------+
|              M6295 ALIEN.U66 +------++------+ IGST0103 |
|   YMF278B-F  M6295 ALIEN.U65 |  IGS ||  IGS | IGST0102 |
|     33.8688MHz 6116 6116     |  008 ||  006 | IGST0101 |
|  YRW801-M ALIEN.u68 6116     +------++------+ IGSB0101 |
|          Z840006PSC                           IGSB0102 |
|                                        PAL    IGSB0103 |
|                                   63C256   61C64 61C64 |
|J                                  63C256   61C64 61C64 |
|A 63C256                           63C256               |
|M 63C256              +------+     63C256 20.000MHz PAL |
|M  PAL  IGSC0101.U81  |  IGS |     63C256 PAL  +------+ |
|A                U80  |  005 |                 |  IGS | |
|      MC86000P10 U79  +------+                 |  007 | |
|                                               +------+ |
|                PAL        6116                         |
|          D8255 PAL       61C64                IGSA0101 |
|    93C46 D8255 PAL        6116                IGSA0102 |
| JP9       SW3 SW2 SW1     6116                IGSA0103 |
+--------------------------------------------------------+

HW Notes:
 MC68000P10 clock: 10.000MHz (20.000MHz/2)
 Z840006PSC clock: 5.000MHz (20.000MHz/4)
  YMF278B-F clock: 33.8688MHz
   OKI 6295 clock: 1.000MHz (20.000MHz/20)
    OKI 6295 pin7: High

Standard JAMMA edge connector
JP9 - 10 pin header

***************************************************************************/

ROM_START( alienchac )
	ROM_REGION( 0x300000, "maincpu", 0 ) // 68000
	ROM_LOAD16_BYTE( "hfh_p.u80", 0x000000, 0x080000, CRC(5175ebdc) SHA1(4a0bdda0f8291f895f888bfd45328b2b124b9051) )
	ROM_LOAD16_BYTE( "hfh_p.u79", 0x000001, 0x080000, CRC(42ad978c) SHA1(eccb96e7170902b37989c8f207e1a821f29b2475) )
	ROM_LOAD( "igsc0101.u81",     0x100000, 0x200000, CRC(704c48cf) SHA1(908017b206622680ab766251294806c8c8c9388b) ) // bytes 0x000000-0x0fffff == 0x100000-0x1fffff of IGSC0102

	ROM_REGION( 0x010000, "soundcpu", 0 ) // Z80
	ROM_LOAD( "alien_u-86.u86", 0x00000, 0x10000, CRC(5728a9ed) SHA1(e5a9e4a1a2cc6c848b08608bc8727bc739270873) ) // labeled as  ALIEN  U-86, == hfh_s.u86

	ROM_REGION( 0x300000, "tiles0", 0 ) // Tilemaps 0 & 3
	ROM_LOAD( "igst0101.u9",  0x000000, 0x100000, CRC(2ce12d7b) SHA1(aa93a82e5f4015c46bb705efb2051b62cd5d7e04) )
	ROM_LOAD( "igst0102.u10", 0x100000, 0x100000, CRC(542a76a0) SHA1(6947b50a024d0053c1eaf9da8c90652bab875142) )
	ROM_LOAD( "igst0103.u11", 0x200000, 0x100000, CRC(adf5698a) SHA1(4b798f8acc5d7581c7e0989260863ae0ca654acd) )

	ROM_REGION( 0x600000, "tiles1", 0 ) // Tilemaps 1 & 2
	ROM_LOAD( "igsb0101.u8",  0x000000, 0x200000, CRC(5c995f7e) SHA1(4f08cf13e313c6802c924b914c73cab4b450da61) )
	ROM_LOAD( "igsb0102.u7",  0x200000, 0x200000, CRC(a2ae9baf) SHA1(338ee260c33448568f138ca00e1d4edda4da018f) )
	ROM_LOAD( "igsb0103.u6",  0x400000, 0x200000, CRC(11b927af) SHA1(2f15e5cea1b86cde3b679bdd0f3d79672d0ddd3e) )

	ROM_REGION( 0xc00000, "sprites", 0 ) // Sprites
	ROM_LOAD( "igsa0101.u3",  0x000000, 0x400000, CRC(374d07c4) SHA1(87e9bfe32cbfe9964ba7253847fbd14aa3c8ed20) )
	ROM_LOAD( "igsa0102.u2",  0x400000, 0x400000, CRC(dbeee7ac) SHA1(e0eb0d73d9230aa6f69f5ac25d44fa19affebe88) )
	ROM_LOAD( "igsa0103.u1",  0x800000, 0x400000, CRC(e5f19041) SHA1(c92a29bbbcb9a1f63364c665e3e0f9679add4389) )

	ROM_REGION( 0x40000, "oki", 0 ) // Samples
	ROM_LOAD( "alien_u65.u65", 0x00000, 0x40000, CRC(ec469b57) SHA1(ba1668078987ad51f47bcd3e61c51a0cf2545350) ) // labeled as  ALIEN  U65, == hfh_g.u65

	ROM_REGION( 0x40000, "oki2", 0 ) // Samples
	ROM_LOAD( "alien_u66.u66", 0x00000, 0x40000, CRC(7cfcd98e) SHA1(3b03123160adfd3404a9e0c4c68420930e80ae48) ) // labeled as  ALIEN  U66, == hfh_g.u66

	ROM_REGION( 0x200000, "ymf", 0 ) // Samples (Standard Yamaha YRW801 2MB samples ROM)
	ROM_LOAD( "yrw801-m", 0x000000, 0x200000, CRC(2a9d8d43) SHA1(32760893ce06dbe3930627755ba065cc3d8ec6ca) ) // Not dumped from PCB, but is a standard samples rom
ROM_END


/***************************************************************************

    Code Decryption

***************************************************************************/

void lordgun_state::init_lordgun()
{
	uint16_t *rom = (uint16_t *)memregion("maincpu")->base();
	int rom_size = 0x100000;

	for(int i = 0; i < rom_size/2; i++)
	{
		uint16_t x = rom[i];

		if((i & 0x0120) == 0x0100 || (i & 0x0a00) == 0x0800)
			x ^= 0x0010;

		rom[i] = x;
	}

	save_item(NAME(m_old));

	for (int i = 0; i < 2; i++)
	{
		save_item(NAME(m_gun[i].scr_x), i);
		save_item(NAME(m_gun[i].scr_y), i);
		save_item(NAME(m_gun[i].hw_x), i);
		save_item(NAME(m_gun[i].hw_y), i);
	}
}

void lordgun_state::init_aliencha()
{
	save_item(NAME(m_aliencha_dip_sel));
}

/***************************************************************************

    Game Drivers

***************************************************************************/

GAME( 1994, lordgun,   0,        lordgun,  lordgun,  lordgun_state, init_lordgun,  ROT0, "IGS", "Lord of Gun (World)",     MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE ) // Excludes USA, Canada, Mexico, Japan & Taiwan
GAME( 1994, lordgunu,  lordgun,  lordgun,  lordgun,  lordgun_state, init_lordgun,  ROT0, "IGS", "Lord of Gun (USA)",       MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE ) // USA, Canada & Mexico
GAME( 1994, aliencha,  0,        aliencha, aliencha, lordgun_state, init_aliencha, ROT0, "IGS", "Alien Challenge (World)", MACHINE_SUPPORTS_SAVE )
GAME( 1994, alienchac, aliencha, aliencha, aliencha, lordgun_state, init_aliencha, ROT0, "IGS", "Alien Challenge (China)", MACHINE_SUPPORTS_SAVE )
