// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

macs.cpp - Multi Amenity Cassette System

processor seems to be ST0016 (z80 based) from SETA

around 0x3700 of the bios (when interleaved) contains the ram test text

TODO:
(general)
-Hook-Up bios.
(yujan)
-Girls disappears when you win.
-Some gfx are offset.


----- Game Notes -----

Kisekae Mahjong  (c)1995 I'MAX
Kisekae Hanafuda (c)1995 I'MAX
Seimei-Kantei-Meimei-Ki Cult Name (c)1996 I'MAX

KISEKAE -- info

* DIP SWITCH *

                      | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
-------------------------------------------------------
 P2 Level |  Normal   |off|off|                       |
          |   Weak    |on |off|                       |
          |  Strong   |off|on |                       |
          |Very strong|on |on |                       |
-------------------------------------------------------
 P2 Points|  Normal   |       |off|off|               |
          |  Easy     |       |on |off|               |
          |  Hard     |       |off|on |               |
          | Very hard |       |on |on |               |
-------------------------------------------------------
 P1       |  1000pts  |               |off|           |
 points   |  2000pts  |               |on |           |
-------------------------------------------------------
  Auto    |   Yes     |                   |off|       |
  tumo    |   No      |                   |on |       |
-------------------------------------------------------
  Not     |           |                       |off|   |
  Used    |           |                       |on |   |
-------------------------------------------------------
  Tumo    |   Long    |                           |off|
  time    |   Short   |                           |on |
-------------------------------------------------------

* at slotA -> DIP SW3
     slotB -> DIP SW4


*/

#include "emu.h"
#include "st0016.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "speaker.h"


namespace {

class macs_state : public driver_device
{
public:
	macs_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_cart_bank(0)
		, m_ram2(*this, "ram2")
		, m_maincpu(*this,"maincpu")
		, m_cart1(*this, "slot_a")
		, m_cart2(*this, "slot_b")
		, m_rombank(*this, "rombank%u", 1)
		, m_rambank(*this, "rambank%u", 1)
	{ }

	void macs(machine_config &config);

	void init_macs();
	void init_kisekaeh();
	void init_kisekaem();
	void init_macs2();

protected:
	void machine_reset() override ATTR_COLD;
	void machine_start() override ATTR_COLD;

private:
	uint8_t m_mux_data = 0;
	uint8_t m_rev = 0;
	uint8_t m_cart_bank;
	std::unique_ptr<uint8_t[]> m_ram1;
	required_shared_ptr<uint8_t> m_ram2;
	void rambank_w(uint8_t data);
	uint8_t macs_input_r(offs_t offset);
	void macs_rom_bank_w(uint8_t data);
	void macs_output_w(offs_t offset, uint8_t data);
	uint8_t dma_offset();

	uint32_t screen_update_macs(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	optional_device<st0016_cpu_device> m_maincpu;
	optional_device<generic_slot_device> m_cart1;
	optional_device<generic_slot_device> m_cart2;

	required_memory_bank_array<2> m_rombank;
	required_memory_bank_array<2> m_rambank;

	void macs_io(address_map &map) ATTR_COLD;
	void macs_mem(address_map &map) ATTR_COLD;
};



void macs_state::macs_mem(address_map &map)
{
	map(0x0000, 0x7fff).bankr("rombank1");
	map(0x8000, 0xbfff).bankr("rombank2");
	//map(0xc000, 0xcfff).rw(FUNC(macs_state::st0016_sprite_ram_r), FUNC(macs_state::st0016_sprite_ram_w));
	//map(0xd000, 0xdfff).rw(FUNC(macs_state::st0016_sprite2_ram_r), FUNC(macs_state::st0016_sprite2_ram_w));
	map(0xe000, 0xe7ff).ram(); /* work ram ? */
	map(0xe800, 0xe87f).ram().share("ram2");
	//map(0xe900, 0xe9ff) // sound - internal
	//map(0xea00, 0xebff).rw(FUNC(macs_state::st0016_palette_ram_r), FUNC(macs_state::st0016_palette_ram_w));
	//map(0xec00, 0xec1f).rw(FUNC(macs_state::st0016_character_ram_r), FUNC(macs_state::st0016_character_ram_w));
	map(0xf000, 0xf7ff).bankrw("rambank1"); /* common /backup ram ?*/
	map(0xf800, 0xffff).bankrw("rambank2"); /* common /backup ram ?*/
}

void macs_state::rambank_w(uint8_t data)
{
	m_rambank[0]->set_entry(2 + (data & 1));
}

uint8_t macs_state::macs_input_r(offs_t offset)
{
	switch(offset)
	{
		case 0:
		{
			/*It's bit-wise*/
			switch(m_mux_data&0x0f)
			{
				case 0x00: return ioport("IN0")->read();
				case 0x01: return ioport("IN1")->read();
				case 0x02: return ioport("IN2")->read();
				case 0x04: return ioport("IN3")->read();
				case 0x08: return ioport("IN4")->read();
				default:
				logerror("Unmapped mahjong panel mux data %02x\n",m_mux_data);
				return 0xff;
			}
		}
		case 1: return ioport("SYS0")->read();
		case 2: return ioport("DSW0")->read();
		case 3: return ioport("DSW1")->read();
		case 4: return ioport("DSW2")->read();
		case 5: return ioport("DSW3")->read();
		case 6: return ioport("DSW4")->read();
		case 7: return ioport("SYS1")->read();
		default:    popmessage("Unmapped I/O read at PC = %06x offset = %02x",m_maincpu->pc(),offset+0xc0);
	}

	return 0xff;
}


void macs_state::macs_rom_bank_w(uint8_t data)
{
	m_rombank[1]->set_entry(m_cart_bank * 0x100 + data);
}

void macs_state::macs_output_w(offs_t offset, uint8_t data)
{
	switch(offset)
	{
		case 0:
		/*
		--x- ---- sets RAM bank?
		---- -x-- Cassette B slot
		---- --x- Cassette A slot
		*/

		if(m_rev == 1)
		{
			/* FIXME: dunno if this RAM bank is right, DASM tracking made on the POST
			    screens indicates that there's just one RAM bank, but then MACS2 games
			    locks up. */
			m_rambank[0]->set_entry(BIT(data, 5));

			m_cart_bank = (data & 0xc) >> 2;
			m_rombank[0]->set_entry(m_cart_bank * 0x100);
		}

		m_rambank[1]->set_entry(BIT(data, 5));
		break;
		case 2: m_mux_data = data; break;

	}
}

void macs_state::macs_io(address_map &map)
{
	map.global_mask(0xff);
	//map(0x00, 0xbf).rw(FUNC(macs_state::st0016_vregs_r), FUNC(macs_state::st0016_vregs_w)); /* video/crt regs ? */
	map(0xc0, 0xc7).rw(FUNC(macs_state::macs_input_r), FUNC(macs_state::macs_output_w));
	map(0xe0, 0xe0).nopw(); /* renju = $40, neratte = 0 */
	map(0xe1, 0xe1).w(FUNC(macs_state::macs_rom_bank_w));
	//map(0xe2, 0xe2).w(FUNC(macs_state::st0016_sprite_bank_w));
	//map(0xe3, 0xe4).w(FUNC(macs_state::st0016_character_bank_w));
	//map(0xe5, 0xe5).w(FUNC(macs_state::st0016_palette_bank_w));
	map(0xe6, 0xe6).w(FUNC(macs_state::rambank_w)); /* banking ? ram bank ? shared rambank ? */
	map(0xe7, 0xe7).nopw(); /* watchdog */
	//map(0xf0, 0xf0).rw(FUNC(macs_state::st0016_dma_r));
}

//static GFXDECODE_START( macs )
//  GFXDECODE_ENTRY( nullptr, 0, charlayout,      0, 16*4  )
//GFXDECODE_END

static INPUT_PORTS_START( macs_base )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x01, "DSW0 - BIT 1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DSW0 - BIT 2" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DSW0 - BIT 4" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DSW0 - BIT 8" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DSW0 - BIT 10" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DSW0 - BIT 20" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DSW0 - BIT 40" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DSW0 - BIT 80" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "DSW1 - BIT 1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DSW1 - BIT 2" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DSW1 - BIT 4" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DSW1 - BIT 8" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DSW1 - BIT 10" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DSW1 - BIT 20" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DSW1 - BIT 40" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DSW1 - BIT 80" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "DSW2 - BIT 1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DSW2 - BIT 2" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DSW2 - BIT 4" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DSW2 - BIT 8" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DSW2 - BIT 10" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DSW2 - BIT 20" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DSW2 - BIT 40" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DSW2 - BIT 80" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "DSW3 - BIT 1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DSW3 - BIT 2" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Game" )
	PORT_DIPSETTING(    0x08, "Bet Type" )
	PORT_DIPSETTING(    0x00, "Normal Type" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Level_Select ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Memory Reset" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Analyzer" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Test Mode" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	// external (printer  in cultname)
	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x01, "DSW4 - BIT 1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DSW4 - BIT 2" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DSW4 - BIT 4" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DSW4 - BIT 8" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DSW4 - BIT 10" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DSW4 - BIT 20" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DSW4 - BIT 40" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "DSW4 - BIT 80" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	//Note: These could likely to be switches that are on the game board and not Dip Switches
	PORT_START("SYS0")
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Note In") PORT_CODE(KEYCODE_4_PAD)

	PORT_START("SYS1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Clear Coin Counter") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET ) PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Analyzer Key") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( macs_m )
	PORT_INCLUDE( macs_base )

	// MAHJONG PANEL
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( kisekaem )
	PORT_INCLUDE( macs_m )

	PORT_MODIFY("SYS1")
	PORT_DIPNAME( 0x01, 0x01, "SYS1 - BIT 1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_DIPNAME( 0x04, 0x04, "SYS1 - BIT 4" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "SYS1 - BIT 8" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "SYS1 - BIT 10" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "SYS1 - BIT 20" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "SYS1 - BIT 40" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "SYS1 - BIT 80" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( macs_h )
	PORT_INCLUDE( macs_base )

	// HANAFUDA PANEL
	// Also other inputs from the Mahjong panel are detected in Service Mode
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_A )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_E )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_HANAFUDA_YES )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_B )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_F )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_HANAFUDA_NO )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_C )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_G )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_D )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_H )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


uint32_t macs_state::screen_update_macs(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return m_maincpu->update(screen,bitmap,cliprect);
}


uint8_t macs_state::dma_offset()
{
	return m_cart_bank;
}


void macs_state::macs(machine_config &config)
{
	/* basic machine hardware */
	ST0016_CPU(config, m_maincpu, 8000000); // 8 MHz ?
	m_maincpu->set_memory_map(&macs_state::macs_mem);
	m_maincpu->set_io_map(&macs_state::macs_io);
	m_maincpu->set_dma_offs_callback(FUNC(macs_state::dma_offset));
	m_maincpu->set_screen("screen");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(128*8, 128*8);
	screen.set_visarea(0*8, 128*8-1, 0*8, 128*8-1);
	screen.set_screen_update(FUNC(macs_state::screen_update_macs));
	screen.set_palette("maincpu:palette");
	screen.screen_vblank().set_inputline(m_maincpu, INPUT_LINE_IRQ0, HOLD_LINE); // FIXME: HOLD_LINE is bad juju

	generic_cartslot_device &slot_a(GENERIC_CARTSLOT(config, "slot_a", generic_plain_slot, "macs_cart"));
	slot_a.set_default_option("rom");
	slot_a.set_user_loadable(false);
	generic_cartslot_device &slot_b(GENERIC_CARTSLOT(config, "slot_b", generic_plain_slot, "macs_cart"));
	slot_b.set_default_option("rom");
	slot_b.set_user_loadable(false);

	// TODO: Mono?
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	m_maincpu->add_route(0, "lspeaker", 1.0);
	m_maincpu->add_route(1, "rspeaker", 1.0);
}


#define MACS_BIOS \
	ROM_REGION( 0x1000000, "bios", 0 ) \
	ROM_LOAD16_BYTE( "macsos_l.u43", 0x00000, 0x80000, CRC(0b5aed5e) SHA1(042e705017ee34656e2c6af45825bb2dd3447747) ) \
	ROM_LOAD16_BYTE( "macsos_h.u44", 0x00001, 0x80000, CRC(538b68e4) SHA1(a0534147791e94e726f49451d0e95671ae0a87d5) )

#define MACS2_BIOS \
	ROM_REGION( 0x1000000, "bios", 0 ) \
	ROM_LOAD16_BYTE( "macs2os_l.bin", 0x00000, 0x80000, NO_DUMP ) \
	ROM_LOAD16_BYTE( "macs2os_h.bin", 0x00001, 0x80000, NO_DUMP )

ROM_START( macsbios )
	MACS_BIOS

	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_COPY( "bios",   0x000000, 0x000000, 0x400000 )

	ROM_REGION( 0x400000, "slot_a:rom", ROMREGION_ERASEFF )
	ROM_REGION( 0x400000, "slot_b:rom", ROMREGION_ERASEFF )
ROM_END

ROM_START( mac2bios )
	MACS2_BIOS

	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_COPY( "bios",   0x000000, 0x000000, 0x400000 )

	ROM_REGION( 0x400000, "slot_a:rom", ROMREGION_ERASEFF )
	ROM_REGION( 0x400000, "slot_b:rom", ROMREGION_ERASEFF )
ROM_END

ROM_START( kisekaem )
	MACS_BIOS

	ROM_REGION( 0x400000, "slot_a:rom", 0 )
	ROM_LOAD16_BYTE( "am-mj.u8", 0x000000, 0x100000, CRC(3cf85151) SHA1(e05400065c384730f04ef565db5ba27eb3973d15) )
	ROM_LOAD16_BYTE( "am-mj.u7", 0x000001, 0x100000, CRC(4b645354) SHA1(1dbf9141c3724e5dff2cd8066117fb1b94671a80) )
	ROM_LOAD16_BYTE( "am-mj.u6", 0x200000, 0x100000, CRC(23b3aa24) SHA1(bfabdb16f9b1b60230bb636a944ab46fdfda49d7) )
	ROM_LOAD16_BYTE( "am-mj.u5", 0x200001, 0x100000, CRC(b4d53e29) SHA1(d7683fdd5531bf1aa0ef1e4e6f517b31e2d5829e) )
	ROM_REGION( 0x400000, "slot_b:rom", ROMREGION_ERASEFF )

	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_COPY( "bios",   0x000000, 0x000000, 0x400000 )
	ROM_COPY( "slot_a:rom", 0x000000, 0x400000, 0x400000 ) // Slot A
ROM_END

ROM_START( kisekaeh )
	MACS_BIOS

	ROM_REGION( 0x400000, "slot_a:rom", 0 )
	ROM_LOAD16_BYTE( "kh-u8.bin", 0x000000, 0x100000, CRC(601b9e6a) SHA1(54508a6db3928f78897df64ce400791e4789d0f6) )
	ROM_LOAD16_BYTE( "kh-u7.bin", 0x000001, 0x100000, CRC(8f6e4bb3) SHA1(361545189feeda0887f930727d25655309b84629) )
	ROM_LOAD16_BYTE( "kh-u6.bin", 0x200000, 0x100000, CRC(8e700204) SHA1(876e5530d749828de077293cb109a71b67cef140) )
	ROM_LOAD16_BYTE( "kh-u5.bin", 0x200001, 0x100000, CRC(709bf7c8) SHA1(0a93e0c4f9be22a3302a1c5d2a6ec4739b202ea8) )
	ROM_REGION( 0x400000, "slot_b:rom", ROMREGION_ERASEFF )

	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_COPY( "bios",   0x000000, 0x000000, 0x400000 )
	ROM_COPY( "slot_a:rom", 0x000000, 0x400000, 0x400000 ) // Slot A
ROM_END

ROM_START( cultname ) // uses printer - two different games ? (slot a - checks for printer, slot b - not)
	MACS_BIOS

	ROM_REGION( 0x400000, "slot_a:rom", 0 )
	ROM_LOAD16_BYTE( "cult-d0.u8", 0x000000, 0x100000, CRC(394bc1a6) SHA1(98df5406862234815b46c7b0ac0b19e4b597d1b6) )
	ROM_LOAD16_BYTE( "cult-d1.u7", 0x000001, 0x100000, CRC(f628133b) SHA1(f06e20212074e5d95cc7d419ac8ce98fb9be3b62) )
	ROM_LOAD16_BYTE( "cult-d2.u6", 0x200000, 0x100000, CRC(c5521bc6) SHA1(7554b56b0201b7d81754defa2244fb7ff7452bf6) )
	ROM_LOAD16_BYTE( "cult-d3.u5", 0x200001, 0x100000, CRC(4325b09b) SHA1(45699a0444a221f893724754c917d33041cabcb9) )

	ROM_REGION( 0x400000, "slot_b:rom", 0 )
	ROM_LOAD16_BYTE( "cult-g0.u8", 0x000000, 0x100000, CRC(f5ab977b) SHA1(e7ee758cc2864500b339e236b944f98df9a1c10e) )
	ROM_LOAD16_BYTE( "cult-g1.u7", 0x000001, 0x100000, CRC(32ae15a4) SHA1(061992efec1ed5527f200bf4c111344b156e759d) )
	ROM_LOAD16_BYTE( "cult-g2.u6", 0x200000, 0x100000, CRC(30ed056d) SHA1(71735339bb501b94402ef403b5a2a60effa39c36) )
	ROM_LOAD16_BYTE( "cult-g3.u5", 0x200001, 0x100000, CRC(fe58b418) SHA1(512f5c544cfafaa98bd2b3791ff1cf67adecec8d) )

	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_COPY( "bios",   0x000000, 0x000000, 0x400000 )
	ROM_COPY( "slot_a:rom", 0x000000, 0x400000, 0x400000 ) // Slot A
	ROM_COPY( "slot_b:rom", 0x000000, 0x800000, 0x400000 ) // Slot B
ROM_END

/* these are listed as MACS2 sub-boards, is it the same?  - it's not ;) */

ROM_START( yuka )
	MACS2_BIOS

	ROM_REGION( 0x400000, "slot_a:rom", 0 )
	ROM_LOAD16_BYTE( "yu-ka_2.u6", 0x000001, 0x100000, CRC(c3c5728b) SHA1(e53cdcae556f34bab45d9342fd78ec29b6543c46) )
	ROM_LOAD16_BYTE( "yu-ka_4.u5", 0x000000, 0x100000, CRC(7e391ee6) SHA1(3a0c122c9d0e2a91df6d8039fb958b6d00997747) )
	ROM_LOAD16_BYTE( "yu-ka_1.u8", 0x200001, 0x100000, CRC(bccd1b15) SHA1(02511f3be60c53b5f5d90f12f0648f6e184ca667) )
	ROM_LOAD16_BYTE( "yu-ka_3.u7", 0x200000, 0x100000, CRC(45b8263e) SHA1(59e1846c91dc39a086e8306260506673eb91de0b) )

	ROM_REGION( 0x400000, "slot_b:rom", ROMREGION_ERASE00 )

	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_COPY( "slot_a:rom",   0x000000, 0x000000, 0x400000 )
ROM_END

ROM_START( yujan )
	MACS2_BIOS

	ROM_REGION( 0x400000, "slot_a:rom", 0 )
	ROM_LOAD16_BYTE( "yu-jan_2.u6", 0x000001, 0x100000, CRC(2f4a8d4b) SHA1(4b328a253b1980a76f46a9a98a7f486813894a33) )
	ROM_LOAD16_BYTE( "yu-jan_4.u5", 0x000000, 0x100000, CRC(226df87b) SHA1(a887728f1ea2ef5f6b4dcd6b5b61586f5e8f267d) )
	ROM_LOAD16_BYTE( "yu-jan_1.u8", 0x200001, 0x100000, CRC(feeeee6a) SHA1(e9613f50d6d2e62fac6b529f81486250cfe83819) )
	ROM_LOAD16_BYTE( "yu-jan_3.u7", 0x200000, 0x100000, CRC(1c1d6997) SHA1(9b07ae6b9ef1c0b57fbaa5fd0bcf1d2d7f17351f) )

	ROM_REGION( 0x400000, "slot_b:rom", ROMREGION_ERASEFF )

	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_COPY( "slot_a:rom",   0x000000, 0x000000, 0x400000 )
ROM_END

#if 0
static const uint8_t ramdata[160]=
{
	0xAF, 0xED, 0x47, 0xD3, 0xC1, 0xD3, 0x0C, 0xD3, 0xAF, 0xED, 0x47, 0xD3, 0xC1, 0xD3, 0x0C, 0xD3,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00, 0xAF, 0x32, 0x1F, 0xFE, 0xD3,
	0xE0, 0x3A, 0x0A, 0xE8, 0xF6, 0x04, 0x32, 0x0A, 0xE8, 0xD3, 0xC0, 0x01, 0x08, 0x00, 0x21, 0x00,
	0x01, 0x11, 0x00, 0xFE, 0xED, 0xB0, 0x01, 0x10, 0x00, 0x21, 0x10, 0x01, 0x11, 0x00, 0xF0, 0xED,
	0xB0, 0x3A, 0x0A, 0xE8, 0xE6, 0xF3, 0xF6, 0x08, 0x32, 0x0A, 0xE8, 0xD3, 0xC0, 0x01, 0x08, 0x00,
	0x21, 0x00, 0x01, 0x11, 0x08, 0xFE, 0xED, 0xB0, 0x01, 0x10, 0x00, 0x21, 0x10, 0x01, 0x11, 0x20,
	0xF0, 0xED, 0xB0, 0x3A, 0x0A, 0xE8, 0xE6, 0xF3, 0x32, 0x0A, 0xE8, 0xD3, 0xC0, 0xC9, 0x00, 0xF3
};
#endif

void macs_state::machine_start()
{
	m_rombank[0]->configure_entries(0  , 256, memregion("maincpu")->base(), 0x4000);
	m_rombank[0]->configure_entries(256, 256, m_cart1->get_rom_base(), 0x4000);
	m_rombank[0]->configure_entries(512, 256, m_cart2->get_rom_base(), 0x4000);
	m_rombank[0]->set_entry(0);

	m_rombank[1]->configure_entries(0  , 256, memregion("maincpu")->base(), 0x4000);
	m_rombank[1]->configure_entries(256, 256, m_cart1->get_rom_base(), 0x4000);
	m_rombank[1]->configure_entries(512, 256, m_cart2->get_rom_base(), 0x4000);
	m_rombank[1]->set_entry(0);

	m_rambank[0]->configure_entries(0, 4, m_ram1.get(), 0x800);
	m_rambank[0]->set_entry(2);

	m_rambank[1]->configure_entries(0, 2, m_ram1.get() + 0x2000, 0x800);
	m_rambank[1]->set_entry(0);
}

void macs_state::machine_reset()
{
#if 0
	uint8_t *macs_ram1 = m_ram1.get();
	uint8_t *macs_ram2 = m_ram2;
/*
        BIOS ram init:

        72CA: 01 C7 00      ld   bc,$00C7
        72CD: 11 9F FE      ld   de,$FE9F
        72D0: 21 27 73      ld   hl,$7327
        72D3: ED B0         ldir
        72D5: 3E C3         ld   a,$C3
        72D7: 32 16 E8      ld   ($E816),a
        72DA: 32 19 E8      ld   ($E819),a
        72DD: 21 9F FE      ld   hl,$FE9F
        72E0: 22 17 E8      ld   ($E817),hl
        72E3: 21 E0 FE      ld   hl,$FEE0
        72E6: 22 1A E8      ld   ($E81A),hl
        ...
        //bank change ? = set 5th bit in port $c0
        ...
        72F8: 01 C7 00      ld   bc,$00C7
        72FB: 11 9F FE      ld   de,$FE9F
        72FE: 21 27 73      ld   hl,$7327
        7301: ED B0         ldir
        ...
        7305: 01 07 05      ld   bc,$0507
        7308: 11 00 F8      ld   de,$F800
        730B: 21 FA 73      ld   hl,$73FA
        730E: ED B0         ldir
        ...
*/
	memcpy(macs_ram1 + 0x0e9f, memregion("bios")->base()+0x7327, 0xc7);
	memcpy(macs_ram1 + 0x1e9f, memregion("bios")->base()+0x7327, 0xc7);

	memcpy(macs_ram1 + 0x0800, memregion("bios")->base()+0x73fa, 0x507);
	memcpy(macs_ram1 + 0x1800, memregion("bios")->base()+0x73fa, 0x507);

#define MAKEJMP(n,m)    macs_ram2[(n) - 0xe800 + 0]=0xc3;\
						macs_ram2[(n) - 0xe800 + 1]=(m)&0xff;\
						macs_ram2[(n) - 0xe800 + 2]=((m)>>8)&0xff;

	MAKEJMP(0xe810, 0xfe4b);
	MAKEJMP(0xe816, 0xfe9f);
	MAKEJMP(0xe81a, 0xfee0);

#undef MAKEJMP

	for(int i=0;i<160;i++)
	{
		macs_ram1[0xe00+i]=ramdata[i];
		macs_ram1[0x1e00+i]=ramdata[i];
	}
	macs_ram1[0x0f67]=0xff;
	macs_ram1[0x1f67]=0xff;

	macs_ram1[0x0ff6]=0x02;
	macs_ram1[0x1ff6]=0x02;

	macs_ram1[0x0ff7]=0x08;
	macs_ram1[0x1ff7]=0x08;

	macs_ram1[0x0ff8]=0x6c;
	macs_ram1[0x1ff8]=0x6c;

	macs_ram1[0x0ff9]=0x07;
	macs_ram1[0x1ff9]=0x07;
#endif
}


void macs_state::init_macs()
{
	m_ram1=std::make_unique<uint8_t[]>(0x20000);
	m_maincpu->set_game_flag((10 | 0x80));
	m_rev = 1;
}

void macs_state::init_macs2()
{
	m_ram1=std::make_unique<uint8_t[]>(0x20000);
	m_maincpu->set_game_flag((10 | 0x80));
	m_rev = 2;
}

void macs_state::init_kisekaeh()
{
	m_ram1=std::make_unique<uint8_t[]>(0x20000);
	m_maincpu->set_game_flag((11 | 0x180));
	m_rev = 1;
}

void macs_state::init_kisekaem()
{
	m_ram1=std::make_unique<uint8_t[]>(0x20000);
	m_maincpu->set_game_flag((10 | 0x180));
	m_rev = 1;
}

} // anonymous namespace


GAME( 1995, macsbios, 0,        macs, macs_m,   macs_state, init_macs,     ROT0, "I'Max",            "Multi Amenity Cassette System BIOS",   MACHINE_IS_BIOS_ROOT | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1995, mac2bios, 0,        macs, macs_m,   macs_state, init_macs2,    ROT0, "I'Max",            "Multi Amenity Cassette System 2 BIOS", MACHINE_IS_BIOS_ROOT | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )

GAME( 1995, kisekaem, macsbios, macs, kisekaem, macs_state, init_kisekaem, ROT0, "I'Max",            "Kisekae Mahjong",                      MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )
GAME( 1995, kisekaeh, macsbios, macs, macs_h,   macs_state, init_kisekaeh, ROT0, "I'Max",            "Kisekae Hanafuda",                     MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )
GAME( 1996, cultname, macsbios, macs, macs_m,   macs_state, init_macs,     ROT0, "I'Max",            "Seimei-Kantei-Meimei-Ki Cult Name",    MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )
GAME( 1999, yuka,     macsbios, macs, macs_h,   macs_state, init_macs2,    ROT0, "Yubis / T.System", "Yu-Ka",                                0 )
GAME( 1999, yujan,    macsbios, macs, macs_m,   macs_state, init_macs2,    ROT0, "Yubis / T.System", "Yu-Jan",                               0 )
