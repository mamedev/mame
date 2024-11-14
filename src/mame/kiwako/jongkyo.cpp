// license:BSD-3-Clause
// copyright-holders:David Haywood, Nicola Salmoria
/**********************************************************

    Jongkyo
    (c)1985 Kiwako

    834-5558 JONGKYO
    C2-00173

    CPU: SEGA Custom 315-5084 (Z80)
    Sound: AY-3-8910
    OSC: 18.432MHz

    ROMs:
    EPR-6258 (2764)
    EPR-6259 (2764)
    EPR-6260 (2764)
    EPR-6261 (2764)
    EPR-6262 (2732)

    PR-6263.6J (82S123N)
    PR-6264.0H (82S123N)
    PR-6265.0M (82S129N)
    PR-6266.0B (82S129N)

**********************************************************/

#include "emu.h"

#include "machine/segacrpt_device.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class jongkyo_state : public driver_device
{
public:
	jongkyo_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_bank1(*this, "bank1"),
		m_bank1d(*this, "bank1d"),
		m_bank0d(*this, "bank0d"),
		m_mainregion(*this, "maincpu"),
		m_videoram(*this, "videoram"),
		m_credit_clear(*this, "CR_CLEAR"),
		m_coin_port(*this, "COINS"),
		m_pl1_inputs(*this, "PL1_%u", 1U),
		m_pl2_inputs(*this, "PL2_%u", 1U)
	{ }

	void jongkyo(machine_config &config);

	void init_jongkyo();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	// misc
	uint8_t m_rom_bank;
	uint8_t m_key_rows;
	uint8_t m_flip_screen;

	// memory pointers
	required_device<segacrpt_z80_device> m_maincpu;
	required_memory_bank m_bank1;
	required_memory_bank m_bank1d;
	required_memory_bank m_bank0d;
	required_region_ptr<uint8_t> m_mainregion;
	required_shared_ptr<uint8_t> m_videoram;
	required_ioport m_credit_clear;
	required_ioport m_coin_port;
	required_ioport_array<6> m_pl1_inputs;
	required_ioport_array<6> m_pl2_inputs;
	uint8_t m_videoram2[0x4000];
	std::unique_ptr<uint8_t[]> m_opcodes;
	void bank_select_w(offs_t offset, uint8_t data);
	void mux_w(uint8_t data);
	void coin_counter_w(uint8_t data);
	void videoram2_w(offs_t offset, uint8_t data);
	void unknown_w(offs_t offset, uint8_t data);
	uint8_t input_1p_r();
	uint8_t input_2p_r();
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void decrypted_opcodes_map(address_map &map) ATTR_COLD;
	void memmap(address_map &map) ATTR_COLD;
	void portmap(address_map &map) ATTR_COLD;
};


/*************************************
 *
 *  Video emulation
 *
 *************************************/


uint32_t jongkyo_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int y = 0; y < 256; ++y)
	{
		for (int x = 0; x < 256; x += 4)
		{
			uint8_t data3;

	//      data3 = m_videoram2[x/4 + y*64]; // wrong

	// good mahjong tiles
			data3 = 0x0f; // we're missing 2 bits.. there must be another piece of video ram somewhere or we can't use all the colours (6bpp).. banked somehow?
	// good girl tiles
	//  data3 = 0x00; // we're missing 2 bits.. there must be another piece of video ram somewhere or we can't use all the colours (6bpp).. banked somehow?


			uint8_t data1 = m_videoram[0x4000 + x / 4 + y * 64];
			uint8_t data2 = m_videoram[x / 4 + y * 64];

			for (int b = 0; b < 4; ++b)
			{
				int const res_x = m_flip_screen ? 255 - (x + b) : (x + b);
				int const res_y = m_flip_screen ? 255 - y : y;
				bitmap.pix(res_y, res_x) = ((data2 & 0x01)) + ((data2 & 0x10) >> 3) +
															((data1 & 0x01) << 2) + ((data1 & 0x10) >> 1) +
															((data3 & 0x01) << 4) + ((data3 & 0x10) << 1);
				data1 >>= 1;
				data2 >>= 1;
				data3 >>= 1;
			}
		}
	}

	return 0;
}


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

void jongkyo_state::bank_select_w(offs_t offset, uint8_t data)
{
	int mask = 1 << (offset >> 1);

	m_rom_bank &= ~mask;

	if (offset & 1)
		m_rom_bank |= mask;

	m_bank1->set_entry(m_rom_bank);
	m_bank1d->set_entry(m_rom_bank);
}

void jongkyo_state::mux_w(uint8_t data)
{
	m_key_rows = data;
	//  printf("%02x\n", m_key_rows);
}

void jongkyo_state::coin_counter_w(uint8_t data)
{
	// bit 0 = hopper out?

	// bit 1 = coin counter
	machine().bookkeeping().coin_counter_w(0, data & 2);

	// bit 2 always set?
	m_flip_screen = (data & 4) >> 2;
}

uint8_t jongkyo_state::input_1p_r()
{
	uint8_t keys = 0x3f;
	for (unsigned i = 0; m_pl1_inputs.size() > i; ++i)
	{
		if (!BIT(m_key_rows, i))
			keys &= m_pl1_inputs[i]->read();
	}

	return keys | m_credit_clear->read();
}

uint8_t jongkyo_state::input_2p_r()
{
	uint8_t keys = 0x3f;
	for (unsigned i = 0; m_pl2_inputs.size() > i; ++i)
	{
		if (!BIT(m_key_rows, i))
			keys &= m_pl2_inputs[i]->read();
	}

	return keys | m_coin_port->read();
}

void jongkyo_state::videoram2_w(offs_t offset, uint8_t data)
{
	m_videoram2[offset] = data;
}

// TODO: this actually looks some kind of memory space protection device, probably tied internally in the 315-5084 (unique for this game) within $4x
// $4d - $4e HL (source?)
// $46 trigger strobe?
// $40 reads (val & 0xf) != 0xa, writes 0x1 to RAM $7530 (buffer?), 0 otherwise
// $4c - $4f DE (VRAM bank destination?)
// $46 writes 0 or 1 depending on $7530 above
void jongkyo_state::unknown_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0: // different values
			break;
		case 1: // set to 0 at the boot and set to 1 continuously
			break;
		case 2: // only set to 0 at the boot
			break;
		case 3: // not used
			break;
		case 4: // set to 1 before the girl drawing (probably is the palette selector, not sure how to restore the old palette)
			break;
		case 5: // only set to 0 at the boot
			break;
		case 6: // different values
			break;
		case 7: // 07 and 08 are like a counter: every write in 08 is an incremented value (from 00 to ff)
			break;
		case 8: // when this value is 0xff the next value is 00 and port 07 is incremented (from 00 to ff)
			break;
		case 9: // different values
			break;
	}
}


/*************************************
 *
 *  Address maps
 *
 *************************************/

void jongkyo_state::memmap(address_map &map)
{
	map(0x0000, 0x3fff).rom().w(FUNC(jongkyo_state::videoram2_w)); // wrong, this doesn't seem to be video ram on write..
	map(0x4000, 0x6bff).rom(); // fixed rom
	map(0x6c00, 0x6fff).bankr(m_bank1);    // banked (8 banks)
	map(0x7000, 0x77ff).ram();
	map(0x8000, 0xffff).ram().share(m_videoram);
}

void jongkyo_state::decrypted_opcodes_map(address_map &map)
{
	map(0x0000, 0x6bff).bankr(m_bank0d);
	map(0x6c00, 0x6fff).bankr(m_bank1d);
}


void jongkyo_state::portmap(address_map &map)
{
	map.global_mask(0xff);
	// R 01 keyboard
	map(0x01, 0x01).r("aysnd", FUNC(ay8910_device::data_r));
	map(0x02, 0x03).w("aysnd", FUNC(ay8910_device::data_address_w));

	map(0x10, 0x10).portr("DSW").w(FUNC(jongkyo_state::coin_counter_w));
	map(0x11, 0x11).portr("IN0").w(FUNC(jongkyo_state::mux_w));
	// W 11 select keyboard row (fe fd fb f7)
	map(0x40, 0x40).nopr(); // unknown, if (A & 0xf) == 0x0a then a bit 0 write to 0x7520 doesn't occur
	map(0x40, 0x45).w(FUNC(jongkyo_state::bank_select_w));
	map(0x46, 0x4f).w(FUNC(jongkyo_state::unknown_w));
}

/*************************************
 *
 *  Input ports
 *
-------------------------------------------------------------
Jongkyo ?1985 Kiwako
DIPSW         |      |1    2    3    4   |5   |6   |7   |8
-------------------------------------------------------------
Payout rate   |50%   |on   on   on   on  |    |    |    |
              |53%   |off  on   on   on  |    |    |    |
              |56%   |on   off  on   on  |    |    |    |
              |59%   |off  off  on   on  |    |    |    |
              |62%   |on   on   off  on  |    |    |    |
              |65%   |off  on   off  on  |    |    |    |
              |68%   |on   off  off  on  |    |    |    |
              |71%   |off  off  off  on  |    |    |    |
              |75%   |on   on   on   off |    |    |    |
              |78%   |off  on   on   off |    |    |    |
              |81%   |on   off  on   off |    |    |    |
              |84%   |off  off  on   off |    |    |    |
              |87%   |on   on   off  off |    |    |    |
              |90%   |off  on   off  off |    |    |    |
              |93%   |on   off  off  off |    |    |    |
              |96%   |off  off  off  off |    |    |    |
-------------------------------------------------------------
Start chance  |Yes   |                   |on  |    |    |
              |No    |                   |off |    |    |
-------------------------------------------------------------
Bet up        |Yes   |                   |    |on  |    |
              |No    |                   |    |off |    |
-------------------------------------------------------------
Last chance   |5     |                   |    |    |on  |
              |1     |                   |    |    |off |
-------------------------------------------------------------
Bonus credit  |50    |                   |    |    |    |on
              |10    |                   |    |    |    |off
-------------------------------------------------------------

 *************************************/


static INPUT_PORTS_START( jongkyo )
	PORT_START("CR_CLEAR")
	PORT_DIPNAME( 0x40, 0x40, "Credit Clear-1" ) //button
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Credit Clear-2" ) //button
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("COINS")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 ) //player-1 side
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 ) //player-2 side

	PORT_START("PL1_1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_START("PL1_2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET ) PORT_CODE(KEYCODE_3) //rate button
	PORT_START("PL1_3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED ) //another D button
	PORT_START("PL1_4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNUSED ) //another opt 1 button
	PORT_START("PL1_5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE ) //PORT_NAME("1P Option 1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP ) //PORT_NAME("1P Option 2")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG ) //PORT_NAME("1P Option 3")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL ) //PORT_NAME("1P Option 4")
	PORT_START("PL1_6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1P Pass") //???
	PORT_BIT( 0x3c, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL2_1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_START("PL2_2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET ) PORT_CODE(KEYCODE_4) PORT_PLAYER(2) //rate button
	PORT_START("PL2_3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED ) //another D button
	PORT_START("PL2_4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) PORT_PLAYER(2)
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNUSED ) //another opt 1 button
	PORT_START("PL2_5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE ) PORT_PLAYER(2) //PORT_NAME("2P Option 1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP ) PORT_PLAYER(2) //PORT_NAME("2P Option 2")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG ) PORT_PLAYER(2) //PORT_NAME("2P Option 3")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL ) PORT_PLAYER(2) //PORT_NAME("2P Option 4")
	PORT_START("PL2_6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2P Pass") PORT_PLAYER(2) //???
	PORT_BIT( 0x3c, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x00, "Note" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_MEMORY_RESET )
	PORT_DIPNAME( 0x04, 0x00, "Analyzer" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_SERVICE( 0x08, IP_ACTIVE_HIGH )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0f, 0x0f, "Payout Rate" ) PORT_DIPLOCATION("SW:1,2,3,4")
	PORT_DIPSETTING(    0x00, "50%" )
	PORT_DIPSETTING(    0x01, "53%" )
	PORT_DIPSETTING(    0x02, "56%" )
	PORT_DIPSETTING(    0x03, "59%" )
	PORT_DIPSETTING(    0x04, "62%" )
	PORT_DIPSETTING(    0x05, "65%" )
	PORT_DIPSETTING(    0x06, "68%" )
	PORT_DIPSETTING(    0x07, "71%" )
	PORT_DIPSETTING(    0x08, "75%" )
	PORT_DIPSETTING(    0x09, "78%" )
	PORT_DIPSETTING(    0x0a, "81%" )
	PORT_DIPSETTING(    0x0b, "84$" )
	PORT_DIPSETTING(    0x0c, "87%" )
	PORT_DIPSETTING(    0x0d, "90%" )
	PORT_DIPSETTING(    0x0e, "93%" )
	PORT_DIPSETTING(    0x0f, "96%" )
	PORT_DIPNAME( 0x10, 0x10, "Start Chance" ) PORT_DIPLOCATION("SW:5")
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, "Bet Up" ) PORT_DIPLOCATION("SW:6")
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, "Last Chance" ) PORT_DIPLOCATION("SW:7")
	PORT_DIPSETTING(    0x40, "1" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x80, 0x80, "Bonus Credit" ) PORT_DIPLOCATION("SW:8")
	PORT_DIPSETTING(    0x80, "10" )
	PORT_DIPSETTING(    0x00, "50" )
INPUT_PORTS_END


/*************************************
 *
 *  Palette initialization and
 *    graphics definitions
 *
 *************************************/

void jongkyo_state::palette(palette_device &palette) const
{
	uint8_t const *const proms = memregion("proms")->base();
	for (int i = 0; i < 0x40; i++)
	{
		int const data = proms[i];

		int const r = pal3bit((data  >> 0) & 0x07);
		int const g = pal3bit((data  >> 3) & 0x07);
		int const b = pal2bit((data  >> 6) & 0x03);

		palette.set_pen_color(i, r, g, b);
	}
}

/*************************************
 *
 *  Machine driver
 *
 *************************************/

void jongkyo_state::machine_start()
{
	save_item(NAME(m_videoram2));
	save_item(NAME(m_rom_bank));
	save_item(NAME(m_key_rows));
	save_item(NAME(m_flip_screen));
}

void jongkyo_state::machine_reset()
{
	m_rom_bank = 0;
	m_key_rows = 0xff;
	m_flip_screen = 1;
}


void jongkyo_state::jongkyo(machine_config &config)
{
	// basic machine hardware
	sega_315_5084_device &maincpu(SEGA_315_5084(config, m_maincpu, 18.432_MHz_XTAL / 4));
	maincpu.set_addrmap(AS_PROGRAM, &jongkyo_state::memmap);
	maincpu.set_addrmap(AS_IO, &jongkyo_state::portmap);
	maincpu.set_addrmap(AS_OPCODES, &jongkyo_state::decrypted_opcodes_map);
	maincpu.set_vblank_int("screen", FUNC(jongkyo_state::irq0_line_hold));
	maincpu.set_size(0x6c00);
	maincpu.set_numbanks(8);
	maincpu.set_banksize(0x400);


	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(256, 256);
	screen.set_visarea(0, 256-1, 8, 256-8-1);
	screen.set_screen_update(FUNC(jongkyo_state::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(jongkyo_state::palette), 0x100);

	SPEAKER(config, "mono").front_center();
	ay8910_device &aysnd(AY8910(config, "aysnd", 18.432_MHz_XTAL / 8));
	aysnd.port_a_read_callback().set(FUNC(jongkyo_state::input_1p_r));
	aysnd.port_b_read_callback().set(FUNC(jongkyo_state::input_2p_r));
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.33);
}



/*************************************
 *
 *  ROM definition
 *
 *************************************/

ROM_START( jongkyo )
	ROM_REGION( 0x8c00, "maincpu", 0 )
	ROM_LOAD( "epr-6258", 0x00000, 0x02000, CRC(fb8b7bcc) SHA1(8ece7c2c82c237b4b51829d412b2109b96ccd0e7) )
	ROM_LOAD( "epr-6259", 0x02000, 0x02000, CRC(e46cde5d) SHA1(1cbe1677cfb3fa9f76ad90d5b1446ce9cefee6b7) )
	ROM_LOAD( "epr-6260", 0x04000, 0x02000, CRC(369a5365) SHA1(037a2971a59ab339595b333cbdfd4cbb104de2be) )
	ROM_LOAD( "epr-6262", 0x06000, 0x00c00, CRC(ecf50f34) SHA1(ecfa1a9360d8fbcbed457d46e53bae77f6d78c1d) )
	ROM_IGNORE(0x400)
	ROM_LOAD( "epr-6261", 0x06c00, 0x02000, CRC(9c475ae1) SHA1(b993c2636dafed9f80fa87e71921c3c85c039e45) )  // banked at 6c00-6fff

	ROM_REGION( 0x300, "proms", 0 )
	// colours
	ROM_LOAD( "pr-6263.6j", 0x00000, 0x00020, CRC(468134d9) SHA1(bb633929df17e448882ee80613fc1dfac3c35d7a) )
	ROM_LOAD( "pr-6264.0h", 0x00020, 0x00020, CRC(46014727) SHA1(eec451f292ee319fa6bfbbf223aaa12b231692c1) )

	// unknown purpose
	ROM_LOAD( "pr-6265.0m", 0x00100, 0x00100, CRC(f09d3c4c) SHA1(a9e752d75e7f3ebd05add4ccf2f9f15d8f9a8d15) )
	ROM_LOAD( "pr-6266.0b", 0x00200, 0x00100, CRC(86aeafd1) SHA1(c4e5c56ce5baf2be3962675ae333e28bd8108a00) )
ROM_END


/*************************************
 *
 *  Driver initialization
 *
 *************************************/

void jongkyo_state::init_jongkyo()
{
	// first of all, do a simple bitswap
	for (int i = 0x6000; i < 0x8c00; ++i)
	{
		m_mainregion[i] = bitswap<8>(m_mainregion[i], 7,6,5,3,4,2,1,0);
	}

	m_opcodes = std::make_unique<uint8_t[]>(0x6c00 + 0x400 * 8);

	m_maincpu->set_region_p(m_mainregion);
	m_maincpu->set_decrypted_p(m_opcodes.get());

	// then do the standard Sega decryption
	m_bank1->configure_entries(0, 8, m_mainregion + 0x6c00, 0x400);
	m_bank1d->configure_entries(0, 8, m_opcodes.get() + 0x6c00, 0x400);
	m_bank0d->set_base(m_opcodes.get());
}

} // Anonymous namespace

/*************************************
 *
 *  Game driver
 *
 *************************************/

GAME( 1985, jongkyo, 0, jongkyo, jongkyo, jongkyo_state, init_jongkyo, ROT0, "Kiwako", "Jongkyo", MACHINE_IMPERFECT_COLORS | MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE )
