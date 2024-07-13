// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli
/*

  Merit trivia games

  driver by Pierpaolo Prazzoli

  Games known to be needed:
  - Some other missing categories romsets and new / old revision

  TODO:
  - add flipscreen according to schematics
  - pitboss: dip switches
  - general - add named output notifiers

Notes: it's important that "questions" is 0xa0000 bytes with empty space filled
       with 0xff, because the built-in ROMs test checks how many question ROMs
       the games has and the type of each one.
       The type is stored in one byte in an offset which changes for every game,
       using it as a form of protection.

       ROM type byte legend:
       0 -> 0x02000 bytes ROM
       1 -> 0x04000 bytes ROM
       2 -> 0x08000 bytes ROM
       3 -> 0x10000 bytes ROM

       ---------------------------------------------------------------------------

       Trivia Whiz ? Horizontal and Vertical can accept 10 question ROMs, with
       4 categories, but they only use 3 categories.

       ---------------------------------------------------------------------------

       The Couples: Year is uncertain, title screen says "1988"
       PCB marking (set 2) and (set 3) says "230188"
       service mode says "6221-51 U5-0 12/02/86"

       ---------------------------------------------------------------------------

       Casino Five: Requires a Keyboard to be attached to switch the game from the
       "High Score" mode to the "Points Replay" mode by entering a custom code.
       The Points Replay mode displays additional information on the service mode
       screen such as Max Bet, Percentage & Points Played / Won. Is the High Score
       or Points Replay mode controlled via a location in NVRAM or in RAM?
       Casino Five has optional Custom Ads which require the CRT-254 Video
       Billboard card & Keyboard to set up.

       ---------------------------------------------------------------------------

       The Pit Boss (2214-04): Has "Custom Ads" display always enabled? How do you
       disable it, is it controlled (stored) via NVRAM?

       ---------------------------------------------------------------------------

Merit Riviera Notes - There are several known versions:
  Riviera Hi-Score
  Riviera Super Star (not dumped)
  Riviera Montana Version (with journal printer, not dumped)
  Riviera Tennessee Draw (not dumped)
  Michigan Super Draw Poker (Is there a "Superstar" version?)
  Americana

  There are several law suites over the Riviera games. Riviera Distributors Inc. bought earlier versions
  of the various video poker games from Merit. RDI then licensed the games to Michigan Coin Op-Vending
  Inc. The legal battles over true ownership started in 2004 and carried on through at least 09/01/2011.

NOTE: Based on tests and deconstruction of the CRT-209 module, it uses the Z80's M1 signal when it fetches an
      opcode to overlay the module's built in 2816 EEPROM data on to the Z80's 0xB000 memory range. While this
      prevents a simple memory read of the 2816's memory region, it does limit the usable instructions to single
      byte opcodes. The CRT-209 module contains the following or similar code to read inputs, which would be
      encrypted by scrambling data and address lines to the 2816:
        7A A4 47 7B A5 4F 7A B4 57 7B B5 5F C9
      In case of future missing dumps of the CRT-209 module, the following data can be manually inserted into the
      crt209 memory region, adjusting the offsets to what the game expects:

    // called by subroutine which reads inputs
    ROM_FILL( 0x01, 0x01, 0x7a ) // ld   a,d
    ROM_FILL( 0x02, 0x01, 0xa4 ) // and  h
    ROM_FILL( 0x03, 0x01, 0x47 ) // ld   b,a
    ROM_FILL( 0x04, 0x01, 0x7b ) // ld   a,e
    ROM_FILL( 0x05, 0x01, 0xa5 ) // and  l
    ROM_FILL( 0x06, 0x01, 0x4f ) // ld   c,a
    ROM_FILL( 0x07, 0x01, 0x7a ) // ld   a,d
    ROM_FILL( 0x08, 0x01, 0xb4 ) // or   h
    ROM_FILL( 0x09, 0x01, 0x57 ) // ld   d,a
    ROM_FILL( 0x0a, 0x01, 0x7b ) // ld   a,e
    ROM_FILL( 0x0b, 0x01, 0xb5 ) // or   l
    ROM_FILL( 0x0c, 0x01, 0x5f ) // ld   e,a
    ROM_FILL( 0x0a, 0x01, 0xc9 ) // ret
*/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/mm58274c.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"
#include "video/mc6845.h"

#include "screen.h"
#include "speaker.h"


namespace {

#define MASTER_CLOCK            (XTAL(10'000'000))
#define CPU_CLOCK               (MASTER_CLOCK / 4)
#define PIXEL_CLOCK             (MASTER_CLOCK / 1)
#define CRTC_CLOCK              (MASTER_CLOCK / 8)

#define NUM_PENS                (16)
#define RAM_PALETTE_SIZE        (1024)


class merit_state : public driver_device
{
public:
	merit_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_ram_attr(*this, "raattr")
		, m_ram_video(*this, "ravideo")
		, m_maincpu(*this, "maincpu")
		, m_ram_palette(*this, "palette", RAM_PALETTE_SIZE, ENDIANNESS_LITTLE)
		, m_gfx(*this, "gfx%u", 1U)
		, m_screen(*this, "screen")
		, m_leds(*this, "led%u", 0U)
	{ }

	void bigappg(machine_config &config);
	void couple(machine_config &config);
	void misdraw(machine_config &config);
	void mosdraw(machine_config &config);
	void no_u40(machine_config &config);
	void pitboss(machine_config &config);
	void riviera(machine_config &config);

	void init_crt209();

	int rndbit_r();

protected:
	virtual void machine_start() override;

	required_shared_ptr<uint8_t> m_ram_attr;
	required_shared_ptr<uint8_t> m_ram_video;

	required_device<cpu_device> m_maincpu;

	uint8_t palette_r(offs_t offset);
	void palette_w(offs_t offset, uint8_t data);

	void bigappg_io_map(address_map &map);

private:
	memory_share_creator<uint8_t> m_ram_palette;
	required_region_ptr_array<uint8_t, 2> m_gfx;

	required_device<screen_device> m_screen;

	output_finder<10> m_leds;

	pen_t m_pens[NUM_PENS];
	uint8_t m_lscnblk = 0;
	uint16_t m_extra_video_bank_bit = 0;

	void hsync_changed(int state);
	void led1_w(uint8_t data);
	void led2_w(uint8_t data);
	void misc_w(uint8_t data);

	MC6845_BEGIN_UPDATE(crtc_begin_update);
	MC6845_UPDATE_ROW(crtc_update_row);
	MC6845_UPDATE_ROW(crtc_update_row_no_u40);
	void bigappg_map(address_map &map);
	void couple_map(address_map &map);
	void riviera_map(address_map &map);
	void misdraw_map(address_map &map);
	void pitboss_map(address_map &map);
	void pitboss_io_map(address_map &map);
};

class merit_banked_state : public merit_state
{
public:
	merit_banked_state(const machine_config &mconfig, device_type type, const char *tag)
		: merit_state(mconfig, type, tag)
		, m_rombank(*this, "rombank%u", 1U)
	{ }

	void casino5(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	required_memory_bank_array<2> m_rombank;

	void bank_w(uint8_t data);

	void casino5_map(address_map &map);
};

class merit_quiz_state : public merit_state
{
public:
	merit_quiz_state(const machine_config &mconfig, device_type type, const char *tag)
		: merit_state(mconfig, type, tag)
		, m_questions_bank(*this, "questions_bank")
	{ }

	void dtrvwz5(machine_config &config);
	void phrcraze(machine_config &config);
	void tictac(machine_config &config);
	void trvwhiz(machine_config &config);
	void trvwhziv(machine_config &config);

	void init_dtrvwz5();
	template <uint8_t Key> void init_key();

protected:
	virtual void machine_start() override;

private:
	memory_bank_creator m_questions_bank;

	int m_decryption_key = 0;
	uint32_t m_question_address = 0;

	uint8_t questions_r();
	void low_offset_w(offs_t offset, uint8_t data);
	void med_offset_w(offs_t offset, uint8_t data);
	void high_offset_w(offs_t offset, uint8_t data);

	void dtrvwz5_map(address_map &map);
	void phrcraze_io_map(address_map &map);
	void phrcraze_map(address_map &map);
	void tictac_map(address_map &map);
	void trvwhiz_map(address_map &map);
	void trvwhziv_map(address_map &map);
};


void merit_state::machine_start()
{
	m_leds.resolve();

	save_item(NAME(m_lscnblk));
	save_item(NAME(m_extra_video_bank_bit));
}

void merit_banked_state::machine_start()
{
	merit_state::machine_start();

	m_rombank[0]->configure_entries(0, 2, memregion("maincpu")->base() + 0x2000, 0x2000);
	m_rombank[1]->configure_entries(0, 2, memregion("maincpu")->base() + 0x6000, 0x2000);
	m_rombank[0]->set_entry(0);
	m_rombank[1]->set_entry(0);
}


void merit_quiz_state::machine_start()
{
	merit_state::machine_start();

	m_questions_bank->configure_entries(0, 10, memregion("questions")->base(), 0x10000);

	m_question_address = 0;

	save_item(NAME(m_question_address));
}


uint8_t merit_quiz_state::questions_r()
{
	switch (m_question_address >> 16)
	{
		case 0x30: m_questions_bank->set_entry(0);
			break;

		case 0x31: m_questions_bank->set_entry(1);
			break;

		case 0x32: m_questions_bank->set_entry(2);
			break;

		case 0x33: m_questions_bank->set_entry(3);
			break;

		case 0x34: m_questions_bank->set_entry(4);
			break;

		case 0x35: m_questions_bank->set_entry(5);
			break;

		case 0x36: m_questions_bank->set_entry(6);
			break;

		case 0x37: m_questions_bank->set_entry(7);
			break;

		case 0x28: m_questions_bank->set_entry(8);
			break;

		case 0x18: m_questions_bank->set_entry(9);
			break;

/* not used? only 10 ROMs are tested in service mode
 * b0 b1 b2 b3 b4 b5 b6 b7 a8 98
 *      case 0xb0: address = 0xa0000;
 *          break;
 */

		default: logerror("read unknown question ROM: %02X\n", m_question_address >> 16);
			return 0xff;
	}

	uint8_t *ret = (uint8_t*)m_questions_bank->base();
	return ret[m_question_address & 0xffff];
}

void merit_quiz_state::low_offset_w(offs_t offset, uint8_t data)
{
	offset = (offset & 0xf0) | ((offset - m_decryption_key) & 0x0f);
	offset = bitswap<8>(offset, 7, 6, 5, 4, 0, 1, 2, 3);
	m_question_address = (m_question_address & 0xffff00) | offset;
}

void merit_quiz_state::med_offset_w(offs_t offset, uint8_t data)
{
	offset = (offset & 0xf0) | ((offset - m_decryption_key) & 0x0f);
	offset = bitswap<8>(offset, 7, 6, 5, 4, 0, 1, 2, 3);
	m_question_address = (m_question_address & 0xff00ff) | (offset << 8);
}

void merit_quiz_state::high_offset_w(offs_t offset, uint8_t data)
{
	offset = bitswap<8>(offset, 7, 6, 5, 4, 0, 1, 2, 3);
	m_question_address = (m_question_address & 0x00ffff) | (offset << 16);
}

uint8_t merit_state::palette_r(offs_t offset)
{
	int const co = ((m_ram_attr[offset] & 0x7f) << 3) | (offset & 0x07);
	return m_ram_palette[co];
}

void merit_state::palette_w(offs_t offset, uint8_t data)
{
//  m_screen->update_now();
	m_screen->update_partial(m_screen->vpos());
	data &= 0x0f;

	int const co = ((m_ram_attr[offset] & 0x7f) << 3) | (offset & 0x07);
	m_ram_palette[co] = data;

}


MC6845_BEGIN_UPDATE(merit_state::crtc_begin_update)
{
	for (int i = 0; i < NUM_PENS; i++)
	{
		int const dim = BIT(i, 3) ? 255 : 127;
		int const bit0 = BIT(i, 0);
		int const bit1 = BIT(i, 1);
		int const bit2 = BIT(i, 2);
		m_pens[i] = rgb_t(dim * bit0, dim * bit1, dim * bit2);
	}
}


MC6845_UPDATE_ROW(merit_state::crtc_update_row)
{
	uint16_t x = 0;

	int const rlen = m_gfx[1].bytes();

	// ma = ma ^ 0x7ff;
	for (uint8_t cx = 0; cx < x_count; cx++)
	{
		int const attr = m_ram_attr[ma & 0x7ff];
		int const region = BIT(attr, 6);
		int addr = ((m_ram_video[ma & 0x7ff] | ((attr & 0x80) << 1) | m_extra_video_bank_bit) << 4) | (ra & 0x0f);
		int const colour = (attr & 0x7f) << 3;

		addr &= (rlen - 1);
		uint8_t const *const data = m_gfx[region];

		for (int i = 7; i >= 0; i--)
		{
			int col = colour;

			col |= (BIT(data[0x0000 | addr], i) << 2);
			if (region == 0)
			{
				col |= (BIT(data[rlen | addr], i) << 1);
				col |= (BIT(data[rlen << 1 | addr], i) << 0);
			}
			else
				col |= 0x03;

			col = m_ram_palette[col & 0x3ff];
			bitmap.pix(y, x) = m_pens[col ? col & (NUM_PENS - 1) : (m_lscnblk ? 8 : 0)];

			x++;
		}
		ma++;
	}
}

MC6845_UPDATE_ROW(merit_state::crtc_update_row_no_u40)
{
	uint16_t x = 0;

	for (uint8_t cx = 0; cx < x_count; cx++)
	{
		int const attr = m_ram_attr[ma & 0x7ff];
		int addr = ((m_ram_video[ma & 0x7ff] | ((attr & 0x80) << 1) | (attr & 0x40) << 3) << 4) | (ra & 0x0f);
		int const colour = (attr & 0x7f) << 3;

		addr &= 0x7fff;
		uint8_t const *const data = m_gfx[0];

		for (int i = 7; i >= 0; i--)
		{
			int col = colour;

			col |= (BIT(data[0x0000 | addr], i) << 2);
			col |= (BIT(data[0x8000 | addr], i) << 1);
			col |= (BIT(data[0x10000 | addr], i) << 0);

			col = m_ram_palette[col & 0x3ff];
			bitmap.pix(y, x) = m_pens[col ? col & (NUM_PENS - 1) : (m_lscnblk ? 8 : 0)];

			x++;
		}
		ma++;
	}
}

void merit_state::hsync_changed(int state)
{
	// update any video up to the current scanline
//  m_screen->update_now();
	m_screen->update_partial(m_screen->vpos());
}

void merit_state::led1_w(uint8_t data)
{
	// 5 button lamps player 1
	m_leds[0] = BIT(~data, 0);
	m_leds[1] = BIT(~data, 1);
	m_leds[2] = BIT(~data, 2);
	m_leds[3] = BIT(~data, 3);
	m_leds[4] = BIT(~data, 4);
}

void merit_state::led2_w(uint8_t data)
{
	// 5 button lamps player 2
	m_leds[5] = BIT(~data, 0);
	m_leds[6] = BIT(~data, 1);
	m_leds[7] = BIT(~data, 2);
	m_leds[8] = BIT(~data, 3);
	m_leds[9] = BIT(~data, 4);

	// coin counter
	machine().bookkeeping().coin_counter_w(0, BIT(~data, 7));
}

void merit_state::misc_w(uint8_t data)
{
	flip_screen_set(BIT(~data, 4));
	m_extra_video_bank_bit = bitswap<2>(data, 0, 1) << 9;
	m_lscnblk = BIT(data, 3);

	// other bits unknown
}

void merit_banked_state::bank_w(uint8_t data)
{
	if (data == 0)
	{
		m_rombank[0]->set_entry(1);
		m_rombank[1]->set_entry(1);
	}
	else if (data == 0xff)
	{
		m_rombank[0]->set_entry(0);
		m_rombank[1]->set_entry(0);
	}
	else
	{
		logerror("Unknown banking write %02x\n", data);
	}
}

int merit_state::rndbit_r()
{
	return machine().rand();
}

void merit_state::pitboss_map(address_map &map)
{
	map(0x0000, 0x5fff).rom();
	map(0x6000, 0x67ff).ram();
	map(0xa000, 0xa003).rw("ppi0", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xc000, 0xc003).rw("ppi1", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xe000, 0xe000).w("crtc", FUNC(mc6845_device::address_w));
	map(0xe001, 0xe001).w("crtc", FUNC(mc6845_device::register_w));
	map(0xe800, 0xefff).ram().share(m_ram_attr);
	map(0xf000, 0xf7ff).ram().share(m_ram_video);
	map(0xf800, 0xfbff).rw(FUNC(merit_state::palette_r), FUNC(merit_state::palette_w));
}

void merit_state::pitboss_io_map(address_map &map)
{
	map(0x8000, 0x8000).w("aysnd", FUNC(ay8912_device::address_w));
	map(0x8100, 0x8100).w("aysnd", FUNC(ay8912_device::data_w));
}

void merit_banked_state::casino5_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x3fff).bankr(m_rombank[0]);
	map(0x4000, 0x5fff).bankr(m_rombank[1]);
	map(0x6000, 0x6fff).ram().share("nvram");
	map(0x7000, 0x7000).w(FUNC(merit_banked_state::bank_w));
	map(0x7001, 0x7fff).ram();
	map(0xa000, 0xa003).rw("ppi0", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xc000, 0xc003).rw("ppi1", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xe000, 0xe000).w("crtc", FUNC(mc6845_device::address_w));
	map(0xe001, 0xe001).w("crtc", FUNC(mc6845_device::register_w));
	map(0xe800, 0xefff).ram().share(m_ram_attr);
	map(0xf000, 0xf7ff).ram().share(m_ram_video);
	map(0xf800, 0xfbff).rw(FUNC(merit_banked_state::palette_r), FUNC(merit_banked_state::palette_w));
}

void merit_state::bigappg_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xa000, 0xbfff).ram().share("nvram");
	map(0xc004, 0xc007).rw("ppi1", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xc008, 0xc00b).rw("ppi0", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xe000, 0xe000).w("crtc", FUNC(mc6845_device::address_w));
	map(0xe001, 0xe001).w("crtc", FUNC(mc6845_device::register_w));
	map(0xe800, 0xefff).ram().share(m_ram_attr);
	map(0xf000, 0xf7ff).ram().share(m_ram_video);
	map(0xf800, 0xfbff).rw(FUNC(merit_state::palette_r), FUNC(merit_state::palette_w));
}

void merit_state::bigappg_io_map(address_map &map)
{
	map(0xc00c, 0xc00c).mirror(0x1cf3).w("aysnd", FUNC(ay8912_device::address_w));
	map(0xc10c, 0xc10c).mirror(0x1cf3).w("aysnd", FUNC(ay8912_device::data_w));
}

void merit_state::riviera_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xa000, 0xbfff).ram().share("nvram");
	map(0xc004, 0xc007).rw("ppi0", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xc008, 0xc00b).rw("ppi1", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xe000, 0xe000).w("crtc", FUNC(mc6845_device::address_w));
	map(0xe001, 0xe001).w("crtc", FUNC(mc6845_device::register_w));
	map(0xe800, 0xefff).ram().share(m_ram_attr);
	map(0xf000, 0xf7ff).ram().share(m_ram_video);
	map(0xf800, 0xfbff).rw(FUNC(merit_state::palette_r), FUNC(merit_state::palette_w));
}

void merit_state::misdraw_map(address_map &map)
{
	riviera_map(map);

	map(0xb000, 0xb7ff).ram().share("crt209"); // 2816 EEPROM data in Z80 epoxy CPU module
}

void merit_state::couple_map(address_map &map)
{
	riviera_map(map);

	map(0x8000, 0x9fff).rom().region("maincpu", 0x14000);
	map(0xb000, 0xb7ff).rom().region("crt209", 0); // couple and clones have a standard 2716 ROM instead of a 2816
}

/* Address decoding is done by prom u13 on crt200a hardware. It decodes
 * the following addr lines: 2,3,9,13,14,15 ==> E20C
 * ==> mirror 1DF3 & ~effective_addr_lines
 * */

void merit_quiz_state::trvwhiz_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4c00, 0x4cff).rw(FUNC(merit_quiz_state::questions_r), FUNC(merit_quiz_state::high_offset_w));
	map(0x5400, 0x54ff).w(FUNC(merit_quiz_state::low_offset_w));
	map(0x5800, 0x58ff).w(FUNC(merit_quiz_state::med_offset_w));
	map(0x6000, 0x67ff).ram();
	map(0xa000, 0xa003).mirror(0x1df0).rw("ppi0", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xc000, 0xc003).mirror(0x1df0).rw("ppi1", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xe000, 0xe000).mirror(0x05f0).w("crtc", FUNC(mc6845_device::address_w));
	map(0xe001, 0xe001).mirror(0x05f0).w("crtc", FUNC(mc6845_device::register_w));
	map(0xe800, 0xefff).ram().share(m_ram_attr);
	map(0xf000, 0xf7ff).ram().share(m_ram_video);
	map(0xf800, 0xfbff).rw(FUNC(merit_quiz_state::palette_r), FUNC(merit_quiz_state::palette_w));
}

void merit_quiz_state::phrcraze_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xa000, 0xbfff).ram();
	map(0xc008, 0xc00b).mirror(0x1df0).rw("ppi1", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xc00c, 0xc00f).mirror(0x1df0).rw("ppi0", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xce00, 0xceff).rw(FUNC(merit_quiz_state::questions_r), FUNC(merit_quiz_state::high_offset_w));
	map(0xd600, 0xd6ff).w(FUNC(merit_quiz_state::low_offset_w));
	map(0xda00, 0xdaff).w(FUNC(merit_quiz_state::med_offset_w));
	map(0xe000, 0xe000).mirror(0x05f0).w("crtc", FUNC(mc6845_device::address_w));
	map(0xe001, 0xe001).mirror(0x05f0).w("crtc", FUNC(mc6845_device::register_w));
	map(0xe800, 0xefff).ram().share(m_ram_attr);
	map(0xf000, 0xf7ff).ram().share(m_ram_video);
	map(0xf800, 0xfbff).rw(FUNC(merit_quiz_state::palette_r), FUNC(merit_quiz_state::palette_w));
}

void merit_quiz_state::phrcraze_io_map(address_map &map)
{
	map(0xc004, 0xc004).mirror(0x1cf3).w("aysnd", FUNC(ay8912_device::address_w));
	map(0xc104, 0xc104).mirror(0x1cf3).w("aysnd", FUNC(ay8912_device::data_w));
}


void merit_quiz_state::tictac_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x9fff).ram().share("nvram");
	map(0xc004, 0xc007).mirror(0x1df0).rw("ppi0", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xc008, 0xc00b).mirror(0x1df0).rw("ppi1", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xce00, 0xceff).rw(FUNC(merit_quiz_state::questions_r), FUNC(merit_quiz_state::high_offset_w));
	map(0xd600, 0xd6ff).w(FUNC(merit_quiz_state::low_offset_w));
	map(0xda00, 0xdaff).w(FUNC(merit_quiz_state::med_offset_w));
	map(0xe000, 0xe000).mirror(0x05f0).w("crtc", FUNC(mc6845_device::address_w));
	map(0xe001, 0xe001).mirror(0x05f0).w("crtc", FUNC(mc6845_device::register_w));
	map(0xe800, 0xefff).ram().share(m_ram_attr);
	map(0xf000, 0xf7ff).ram().share(m_ram_video);
	map(0xf800, 0xfbff).rw(FUNC(merit_quiz_state::palette_r), FUNC(merit_quiz_state::palette_w));
}

void merit_quiz_state::dtrvwz5_map(address_map &map)
{
	tictac_map(map);

	map(0xb000, 0xb7ff).ram().share("crt209"); // 2816 EEPROM data in Z80 epoxy CPU module
}

void merit_quiz_state::trvwhziv_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xa000, 0xbfff).ram();
	map(0xc004, 0xc007).mirror(0x1df0).rw("ppi0", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xc008, 0xc00b).mirror(0x1df0).rw("ppi1", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xce00, 0xceff).rw(FUNC(merit_quiz_state::questions_r), FUNC(merit_quiz_state::high_offset_w));
	map(0xd600, 0xd6ff).w(FUNC(merit_quiz_state::low_offset_w));
	map(0xda00, 0xdaff).w(FUNC(merit_quiz_state::med_offset_w));
	map(0xe000, 0xe000).mirror(0x05f0).w("crtc", FUNC(mc6845_device::address_w));
	map(0xe001, 0xe001).mirror(0x05f0).w("crtc", FUNC(mc6845_device::register_w));
	map(0xe800, 0xefff).ram().share(m_ram_attr);
	map(0xf000, 0xf7ff).ram().share(m_ram_video);
	map(0xf800, 0xfbff).rw(FUNC(merit_quiz_state::palette_r), FUNC(merit_quiz_state::palette_w));
}


static INPUT_PORTS_START( meritpoker )

	PORT_START("IN0") // Pins #65 through #58 of J3 in descending order
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME( "Hold 1 / Take / Lo" )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME( "Hold 5 / Double Up / Hi" )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) PORT_NAME( "Cash Out / Hi-Score" )

	PORT_START("IN1") // Pins #57 through #51 of J3 in descending order
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )
	PORT_SERVICE_NO_TOGGLE( 0x08, IP_ACTIVE_LOW )       // AKA Diagnostics
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_STAND )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2") // Pins #46 through #41 of J3 in descending order (usually P2 controls - Not used!)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN ) // MUST be "LOW" or Riviera Hi-Score rev A will hang
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(merit_state, rndbit_r)

	PORT_START("DSW")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, IP_ACTIVE_LOW, "SW1:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, IP_ACTIVE_LOW, "SW1:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW1:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW1:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, IP_ACTIVE_LOW, "SW1:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW1:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW1:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW1:8" )
INPUT_PORTS_END



static INPUT_PORTS_START( bigappg )
	PORT_INCLUDE( meritpoker )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x00, "Auto Hold" )         PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Bonus Jackpot" )     PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Take Half Option" )      PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Raise Option" )      PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Unlimited Double Up" )   PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Double Up" )         PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x00, "Maximum Bet" )       PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPSETTING(    0xc0, "20" )
	PORT_DIPSETTING(    0x00, "50" ) PORT_CONDITION("DSW", 0x08, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, "99" ) PORT_CONDITION("DSW", 0x08, EQUALS, 0x08)
INPUT_PORTS_END

static INPUT_PORTS_START( chkndraw )
	PORT_INCLUDE( meritpoker )

	PORT_MODIFY("IN0") // Pins #65 through #58 of J3 in descending order
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_NAME( "Hold 3 / Take Half / Dbl Half" )

	PORT_MODIFY("IN1") // Pins #57 through #51 of J3 in descending order
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_COCKTAIL PORT_CODE(KEYCODE_W) PORT_NAME( "P2 Deal")
	PORT_DIPNAME( 0xc0, 0xc0, "Percentage (Rate of Return)" )    PORT_DIPLOCATION("Special:1,2") // Pins #52 & #51?? Listed as "Switch Common Ground"
	PORT_DIPSETTING(    0x80, "80%" )
	PORT_DIPSETTING(    0xc0, "85%" )
	PORT_DIPSETTING(    0x40, "90%" )
	PORT_DIPSETTING(    0x00, "85%" ) // Duplicate setting - Likely not used

	PORT_MODIFY("IN2") // Pins #46 through #41 of J3 in descending order
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL PORT_CODE(KEYCODE_A) PORT_NAME( "P2 Hold 1 / Take / Lo" )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL PORT_CODE(KEYCODE_S) PORT_NAME( "P2 Hold 2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL PORT_CODE(KEYCODE_D) PORT_NAME( "P2 Hold 3 / Take Half / Dbl Half" )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_COCKTAIL PORT_CODE(KEYCODE_F) PORT_NAME( "P2 Hold 4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_COCKTAIL PORT_CODE(KEYCODE_G) PORT_NAME( "P2 Hold 5 / Double Up / Hi" )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_COCKTAIL PORT_CODE(KEYCODE_L) PORT_NAME( "P2 Bet")


	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x00, "Raise Option" )         PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, "Raise Enabled" )
	PORT_DIPSETTING(    0x00, "No Raise" )
	PORT_DIPNAME( 0x02, 0x00, "Format" )     PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, "Format S" )
	PORT_DIPSETTING(    0x00, "Format M" )
	PORT_DIPNAME( 0x04, 0x00, "Second Joker In Deck" )  PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, "Play 4 Points" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Points Per Coin" )   PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x10, "1" )
	PORT_DIPNAME( 0x20, 0x00, "Players" )     PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, "One Player" )
	PORT_DIPSETTING(    0x00, "Two Plaerys" )
	PORT_DIPNAME( 0xc0, 0x00, "Maximum Bet" )   PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x40, "1 Point Per Hand" )
	PORT_DIPSETTING(    0xc0, "10" )
	PORT_DIPSETTING(    0x80, "20" )
	PORT_DIPSETTING(    0x00, "50" )
INPUT_PORTS_END

static INPUT_PORTS_START( riviera )
	PORT_INCLUDE( meritpoker )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x00, "Auto Hold" )     PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Bonus Jackpot" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:3") // Flyer suggests this might be "10-IN-A-ROW" bonus
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Raise Option" )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Points Per Coin" )   PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x10, "1" )
	PORT_DIPNAME( 0x20, 0x00, "Double Up" )     PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x00, "Maximum Bet" )   PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPSETTING(    0xc0, "20" )
	PORT_DIPSETTING(    0x80, "50" )
	PORT_DIPSETTING(    0x00, "50" ) PORT_CONDITION("DSW", 0x08, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, "99" ) PORT_CONDITION("DSW", 0x08, EQUALS, 0x08)
INPUT_PORTS_END

static INPUT_PORTS_START( msupstar )
	PORT_INCLUDE( meritpoker )

	PORT_MODIFY("DSW")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, IP_ACTIVE_HIGH, "SW1:1" ) // must be HIGH or game stalls with "GAME MALFUNCTION PLEASE CALL ATTENDANT" error!
	PORT_DIPNAME( 0x10, 0x10, "Points Per Coin" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, "1 Point" )
	PORT_DIPSETTING(    0x00, "5 Points" )
	PORT_DIPNAME( 0xc0, 0xc0, "Maximum Bet" ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPSETTING(    0xc0, "20" )
	PORT_DIPSETTING(    0x80, "50" )
	PORT_DIPSETTING(    0x00, "50" ) // duplicate setting
INPUT_PORTS_END

static INPUT_PORTS_START( rivierab )
	PORT_INCLUDE( riviera )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:1") // No Auto Hold feature for this set
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( mosdraw )
	PORT_INCLUDE( meritpoker )

	PORT_MODIFY("DSW") // DSW affects only points per coins, everything seems hard coded values
	PORT_DIPNAME( 0x10, 0x00, "Points Per Coin" )   PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x10, "1" )

	PORT_MODIFY("IN2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // to bypass printer check TODO: proper emulation
INPUT_PORTS_END

static INPUT_PORTS_START( iowapp )
	PORT_INCLUDE( meritpoker )

	PORT_MODIFY("IN0") // Pins #65 through #58 of J3 in descending order
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )

//  PORT_MODIFY("IN1") // Pins #57 through #51 of J3 in descending order
//  PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) // If HIGH triggers a "TOKEN LOW" error - Hopper related

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x00, "Auto Hold" )     PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Bonus Jackpot" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Cards Dealt" )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, "Slow" )
	PORT_DIPSETTING(    0x08, "Fast" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( dodge ) // Same as "meritpoker" but not verified correct. Will correct / verify when these clones work
	PORT_INCLUDE( meritpoker )
INPUT_PORTS_END

static INPUT_PORTS_START( pitboss ) // PCB pinout maps 12 lamp outputs - Where are they mapped?

	PORT_START("IN0") // Pins #65 through #58 of J3 in descending order
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_Z) PORT_NAME("P1/P2 Button 1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(KEYCODE_X) PORT_NAME("P1/P2 Button 2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(KEYCODE_C) PORT_NAME("P1/P2 Button 3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CODE(KEYCODE_V) PORT_NAME("P1/P2 Button 4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_B) PORT_NAME("P1/P2 Button 5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("P1/P2 Play")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_CODE(KEYCODE_Q) PORT_NAME("P1/P2 Cancel")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // pulling this LOW causes "unauthorized conversion" msg.

	PORT_START("IN1") // Pins #57 through #51 of J3 in descending order
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )
	PORT_SERVICE_NO_TOGGLE( 0x08, IP_ACTIVE_LOW )   // AKA Diagnostics - Seems to reset the game
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON7 )    PORT_COCKTAIL PORT_CODE(KEYCODE_E) PORT_NAME("P2 Cancel")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0xc0, 0xc0, "Hands Per Game" )    PORT_DIPLOCATION("Special:1,2") // Pins #52 & #51?? Listed as "Switch Common Ground"
	PORT_DIPSETTING(    0x80, "3" )
	PORT_DIPSETTING(    0xc0, "4" )
	PORT_DIPSETTING(    0x40, "5" )
	PORT_DIPSETTING(    0x00, "5" ) // Duplicate setting - Likely not used

	PORT_START("IN2") // Pins #46 through #41 of J3 in descending order
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL PORT_CODE(KEYCODE_A) PORT_NAME("P2 Button 1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL PORT_CODE(KEYCODE_S) PORT_NAME("P2 Button 2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL PORT_CODE(KEYCODE_D) PORT_NAME("P2 Button 3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_COCKTAIL PORT_CODE(KEYCODE_F) PORT_NAME("P2 Button 4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_COCKTAIL PORT_CODE(KEYCODE_G) PORT_NAME("P2 Button 5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_COCKTAIL PORT_CODE(KEYCODE_W) PORT_NAME("P2 Play")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(merit_state, rndbit_r)

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW:1")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Max Double Up" ) PORT_DIPLOCATION("SW:4")
	PORT_DIPSETTING(    0x08, "Once" )
	PORT_DIPSETTING(    0x00, "Twice" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ) )  PORT_DIPLOCATION("SW:7")
	PORT_DIPSETTING(    0x40, "Counter Top" )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, "Free Hands" )    PORT_DIPLOCATION("SW:8")
	PORT_DIPSETTING(    0x80, "100,000+ & 200,000+" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
INPUT_PORTS_END


static INPUT_PORTS_START( mroundup ) // TODO: Find were Player 2 "Play" is mapped, all "IPT_UNKNOWN" below checked and nothing seems to work
	PORT_START("IN0") // Pins #65 through #58 of J3 in descending order
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("P1 Play")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_CODE(KEYCODE_Q) PORT_NAME("Cancel")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_CODE(KEYCODE_R) PORT_NAME("Reset Points") // Counts down player point if pressed instead of "Play"

	PORT_START("IN1") // Pins #57 through #51 of J3 in descending order
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )
	PORT_SERVICE_NO_TOGGLE( 0x08, IP_ACTIVE_LOW )   // AKA Diagnostics - Seems to reset the game
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON7 )    PORT_COCKTAIL PORT_CODE(KEYCODE_E) PORT_NAME("P2 Cancel")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0xc0, 0xc0, "Percentage Out" )    PORT_DIPLOCATION("Special:1,2") // Pins #52 & #51?? Listed as "Switch Common Ground"
	PORT_DIPSETTING(    0x80, "80%" )
	PORT_DIPSETTING(    0x00, "85%" ) // Duplicate
	PORT_DIPSETTING(    0xc0, "85%" )
	PORT_DIPSETTING(    0x40, "90%" )


	PORT_START("IN2") // Pins #46 through #41 of J3 in descending order
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL PORT_CODE(KEYCODE_A) PORT_NAME("P2 Button 1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL PORT_CODE(KEYCODE_S) PORT_NAME("P2 Button 2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL PORT_CODE(KEYCODE_D) PORT_NAME("P2 Button 3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_COCKTAIL PORT_CODE(KEYCODE_F) PORT_NAME("P2 Button 4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_COCKTAIL PORT_CODE(KEYCODE_G) PORT_NAME("P2 Button 5")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // pulling this LOW causes "unauthorized conversion" msg.
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(merit_state, rndbit_r)

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Enable Draw Poker" )     PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Enable Foto Finish" )    PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Enable Black Jack" )     PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )     PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Points Per Coin" )       PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, "1 Point" )
	PORT_DIPSETTING(    0x00, "5 Points" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, "Counter Top" )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0xc0, 0x00, "Maximum Bet" )           PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x40, "1" )
	PORT_DIPSETTING(    0xc0, "10" )
	PORT_DIPSETTING(    0x80, "20" )
	PORT_DIPSETTING(    0x00, "50" )
INPUT_PORTS_END

static INPUT_PORTS_START( pitbossa )
	PORT_INCLUDE( pitboss )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x30, 0x30, "Coin Lockout" )      PORT_DIPLOCATION("SW:5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x30, "2 Coins" )
	PORT_DIPSETTING(    0x20, "10 Coins" )
INPUT_PORTS_END

static INPUT_PORTS_START( pitbossa1 )
	PORT_INCLUDE( pitboss )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x30, 0x30, "Coin Lockout" )      PORT_DIPLOCATION("SW:5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x30, "2 Coins" )
	PORT_DIPSETTING(    0x20, "10 Coins" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown) )       PORT_DIPLOCATION("SW:8") // No Free Hand Bonus for this set
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( pitbossb )
	PORT_INCLUDE( pitboss )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown) )       PORT_DIPLOCATION("SW:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( mdchoice ) // Does one of the dips (1 through 4) control number of Jokers? Currently there are 2
	PORT_INCLUDE( pitboss )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown) )       PORT_DIPLOCATION("SW:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown) )       PORT_DIPLOCATION("SW:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, "Coin Lockout" )      PORT_DIPLOCATION("SW:5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x30, "2 Coins" )
	PORT_DIPSETTING(    0x20, "10 Coins" )
INPUT_PORTS_END

static INPUT_PORTS_START( mpchoice ) // pitbossc games but dips like The Round Up
	PORT_INCLUDE( pitboss )

	PORT_MODIFY("IN1") // Pins #57 through #51 of J3 in descending order
	PORT_DIPNAME( 0xc0, 0xc0, "Percentage Out" )    PORT_DIPLOCATION("Special:1,2") // Pins #52 & #51?? Listed as "Switch Common Ground"
	PORT_DIPSETTING(    0x80, "80%" )
	PORT_DIPSETTING(    0x00, "85%" ) // Duplicate
	PORT_DIPSETTING(    0xc0, "85%" )
	PORT_DIPSETTING(    0x40, "90%" )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Enable Draw Poker" )     PORT_DIPLOCATION("SW:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Enable Acey Deucey" )    PORT_DIPLOCATION("SW:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Enable Black Jack" )     PORT_DIPLOCATION("SW:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "The Dice Game" )     PORT_DIPLOCATION("SW:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW:6")
	PORT_DIPSETTING(    0x20, "Counter Top" )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0xc0, 0x00, "Maximum Bet" )           PORT_DIPLOCATION("SW:7,8")
	PORT_DIPSETTING(    0x40, "1" )
	PORT_DIPSETTING(    0xc0, "10" )
	PORT_DIPSETTING(    0x80, "20" )
	PORT_DIPSETTING(    0x00, "50" )
INPUT_PORTS_END

static INPUT_PORTS_START( mpchoicea ) // pitbossc games but dips like The Round Up
	PORT_INCLUDE( pitboss )

	PORT_MODIFY("IN1") // Pins #57 through #51 of J3 in descending order
	PORT_DIPNAME( 0xc0, 0xc0, "Percentage Out" )    PORT_DIPLOCATION("Special:1,2") // Pins #52 & #51?? Listed as "Switch Common Ground"
	PORT_DIPSETTING(    0x80, "80%" )
	PORT_DIPSETTING(    0x00, "85%" ) // Duplicate
	PORT_DIPSETTING(    0xc0, "85%" )
	PORT_DIPSETTING(    0x40, "90%" )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Enable Draw Poker" )     PORT_DIPLOCATION("SW:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Enable Acey Deucey" )    PORT_DIPLOCATION("SW:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Enable Black Jack" )     PORT_DIPLOCATION("SW:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )     PORT_DIPLOCATION("SW:4") // 3 Games for this set, no dice game
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW:6")
	PORT_DIPSETTING(    0x20, "Counter Top" )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0xc0, 0x00, "Maximum Bet" )           PORT_DIPLOCATION("SW:7,8")
	PORT_DIPSETTING(    0x40, "1" )
	PORT_DIPSETTING(    0xc0, "10" )
	PORT_DIPSETTING(    0x80, "20" )
	PORT_DIPSETTING(    0x00, "50" )
INPUT_PORTS_END

static INPUT_PORTS_START( casino5 )

	PORT_START("IN0") // Pins #65 through #58 of J3 in descending order
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Points")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Play")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )   // pulling this LOW makes the horse racing game to not work

	PORT_START("IN1") // Pins #57 through #51 of J3 in descending order
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) // Runs basic Diagnostics on ROMs
	PORT_SERVICE_NO_TOGGLE( 0x08, IP_ACTIVE_LOW )    // AKA Diagnostics - Shows simple Statistics
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )    // 1 displays additional screens in attract mode - custom ads screen (requires optional Keyboard to set up)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0xc0, 0xc0, "Percentage Out" )    PORT_DIPLOCATION("Special:1,2") // Controls Percentage out, 75%, 80%, 85% & 90%  as per manual's "Tab Positions" (pins #52 & #51??)- Need to verify values
	PORT_DIPSETTING(    0x00, "75%" )
	PORT_DIPSETTING(    0x80, "80%" )
	PORT_DIPSETTING(    0xc0, "85%" )
	PORT_DIPSETTING(    0x40, "90%" )

	PORT_START("IN2") // Pins #46 through #41 of J3 in descending order (usually P2 controls - Not used!)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // pulling this LOW causes "Unathorized conversion"
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(merit_state, rndbit_r)

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Enable Draw Poker" )     PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Enable Black Jack" )     PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Enable Dice Game" )      PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Enable Foto Finish" )    PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Enable Acey Deucey" )    PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END


/* While on the "Books" (press "0") screen:
 * Some games have a hidden test mode, press Service Key (F2) to access it Example: Phrase Craze
 *  For other games:
 *   keep service test button pressed to clear the coin counter.
 *   keep it pressed for 10 seconds to clear all the memory.
 *
 * When NOT on the Books screen, Service Key acts like a reset.
 */
static INPUT_PORTS_START( merittrivia )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )   // Allows Test / Service menu from the "Books"

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )
	PORT_SERVICE_NO_TOGGLE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_COCKTAIL PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_COCKTAIL PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(merit_state, rndbit_r)

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Answers Shown" )     PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW1:3" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_1C ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, IP_ACTIVE_LOW, "SW1:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW1:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW1:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW1:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( phrcraze )
	PORT_INCLUDE( merittrivia )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:1") // no Demo Sounds DSW
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Lives ) )    PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x02, "6" )
	PORT_DIPNAME( 0x04, 0x04, "Topic \"8\"" )   PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Cabinet ) )  PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x40, "Upright 1 Player" )
	PORT_DIPSETTING(    0xc0, "Upright 1 Player" )
	PORT_DIPSETTING(    0x00, "Upright 2 Players" )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )
INPUT_PORTS_END

static INPUT_PORTS_START( phrcrazs )
	PORT_INCLUDE( merittrivia )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x04, 0x04, "XXX-Rated Sex Topic" )   PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x08, "Bonus Phraze" )      PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(    0x18, DEF_STR( None ) )
	PORT_DIPSETTING(    0x10, "800K" )
	PORT_DIPSETTING(    0x08, "1M" )
	PORT_DIPSETTING(    0x00, "1.5M" )
	PORT_DIPNAME( 0x20, 0x20, "Random Sex Category" )   PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x40, "Upright 1 Player" )
	PORT_DIPSETTING(    0xc0, "Upright 1 Player" )
	PORT_DIPSETTING(    0x00, "Upright 2 Players" )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )
INPUT_PORTS_END

static INPUT_PORTS_START( phrcraza )
	PORT_INCLUDE( phrcraze )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )    // Phraze Craze (6221-40, U5-0) will hang if pulled HIGH
INPUT_PORTS_END

static INPUT_PORTS_START( tictac )
	PORT_INCLUDE( merittrivia )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Lightning Round 1 Credit" )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:4") // no coinage DSW
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x40, "Upright 1 Player" )
	PORT_DIPSETTING(    0xc0, "Upright 1 Player" )
	PORT_DIPSETTING(    0x00, "Upright 2 Players" )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )
INPUT_PORTS_END

static INPUT_PORTS_START( trivia )
	PORT_INCLUDE( merittrivia )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x04, 0x04, "On 0 Points" )       PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, "Continue" )
	PORT_DIPSETTING(    0x00, "Game Over" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( trvwhziv )
	PORT_INCLUDE( trivia )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:4") // no coinage DSW
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( dtrvwz5 )
	PORT_INCLUDE( merittrivia )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW1:1") // no Demo Sounds DSW
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Hi Scores Retained" )    PORT_DIPLOCATION("SW1:4") // replaces coinage DSW
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, "Cocktail Type" )     PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, "Regular Cocktail" )
	PORT_DIPSETTING(    0x00, "Single Side Cocktail" )
INPUT_PORTS_END

static INPUT_PORTS_START( couple )
	PORT_START("IN0") // both players use the same controls. Start a 1 player game with button 1 or a 2 player game with button 2.
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x00, "Number of Attempts" )    PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, "99" )
	PORT_DIPSETTING(    0x00, "9" )
	PORT_DIPNAME( 0x02, 0x02, "Tries Per Coin" )        PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
//2 Coins for 2 Credits?I think this is an invalid setting,it doesn't even work correctly
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_2C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x20, 0x00, "Sound" )         PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Clear RAM" )         PORT_DIPLOCATION("SW1:8") // Service Mode shows this as "NOT USED"
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )
	PORT_SERVICE_NO_TOGGLE( 0x08, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x01, "3" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(merit_state, rndbit_r)
INPUT_PORTS_END

// Different DSWs
static INPUT_PORTS_START( couplep )
	PORT_INCLUDE( couple )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x40, 0x40, "Bonus Play" )    PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, "at 150.000" )
	PORT_DIPSETTING(    0x00, "at 200.000" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( matchem )
	PORT_INCLUDE( couple )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_5C ) ) // shows "5 PLAYS FOR 4 COINS"
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) ) // shows "4 PLAYS FOR 4 COINS"
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( matchemg )
	PORT_INCLUDE( couple )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:3") // it always gives 6 credits, but it only shows the possibility when off, maybe it locks out the second coin instead?
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_6C ) ) // 6 games for 5 DM
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) ) // 5 games for 5 DM
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


void merit_state::pitboss(machine_config &config)
{
	Z80(config, m_maincpu, CPU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &merit_state::pitboss_map);
	m_maincpu->set_addrmap(AS_IO, &merit_state::pitboss_io_map);

	i8255_device &ppi0(I8255A(config, "ppi0"));
	ppi0.in_pa_callback().set_ioport("IN0");
	ppi0.in_pb_callback().set_ioport("IN1");
	ppi0.in_pc_callback().set_ioport("IN2");

	i8255_device &ppi1(I8255A(config, "ppi1"));
	ppi1.in_pa_callback().set_ioport("DSW");
	ppi1.out_pb_callback().set(FUNC(merit_state::led1_w));
	ppi1.out_pc_callback().set(FUNC(merit_state::misc_w));

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(PIXEL_CLOCK, 512, 0, 512, 256, 0, 256);   // temporary, CRTC will configure screen
	m_screen->set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	mc6845_device &crtc(MC6845(config, "crtc", CRTC_CLOCK));
	crtc.set_screen(m_screen);
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);
	crtc.set_begin_update_callback(FUNC(merit_state::crtc_begin_update));
	crtc.set_update_row_callback(FUNC(merit_state::crtc_update_row));
	crtc.out_hsync_callback().set(FUNC(merit_state::hsync_changed));
	crtc.out_vsync_callback().set_inputline(m_maincpu, 0);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ay8912_device &aysnd(AY8912(config, "aysnd", CRTC_CLOCK));
	aysnd.port_a_write_callback().set(FUNC(merit_state::led2_w));
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.33);
}

void merit_banked_state::casino5(machine_config &config)
{
	pitboss(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &merit_banked_state::casino5_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
}

void merit_state::bigappg(machine_config &config)
{
	pitboss(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &merit_state::bigappg_map);
	m_maincpu->set_addrmap(AS_IO, &merit_state::bigappg_io_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
}

void merit_state::misdraw(machine_config &config)
{
	bigappg(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &merit_state::misdraw_map);

	NVRAM(config, "crt209", nvram_device::DEFAULT_ALL_0);
}

void merit_state::riviera(machine_config &config)
{
	bigappg(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &merit_state::riviera_map);
}

void merit_state::no_u40(machine_config &config)
{
	misdraw(config);

	subdevice<mc6845_device>("crtc")->set_update_row_callback(FUNC(merit_state::crtc_update_row_no_u40));
}

void merit_state::mosdraw(machine_config &config)
{
	riviera(config);

	// TODO: hook up RTC and printer
	MM58274C(config, "rtc", 0);  // actually an MM58174AN, but should be compatible according to other drivers
}

void merit_state::couple(machine_config &config)
{
	riviera(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &merit_state::couple_map);
}

void merit_quiz_state::tictac(machine_config &config)
{
	pitboss(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &merit_quiz_state::tictac_map);
	m_maincpu->set_addrmap(AS_IO, &merit_quiz_state::bigappg_io_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
}

void merit_quiz_state::trvwhiz(machine_config &config)
{
	pitboss(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &merit_quiz_state::trvwhiz_map);
}

void merit_quiz_state::dtrvwz5(machine_config &config)
{
	misdraw(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &merit_quiz_state::dtrvwz5_map);
}

void merit_quiz_state::phrcraze(machine_config &config)
{
	pitboss(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &merit_quiz_state::phrcraze_map);
	m_maincpu->set_addrmap(AS_IO, &merit_quiz_state::phrcraze_io_map);
}

void merit_quiz_state::trvwhziv(machine_config &config)
{
	pitboss(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &merit_quiz_state::trvwhziv_map);
}


ROM_START( pitboss ) // Program ROMs were all printed as 2214-05 with the "7" hand written over the 5, U5 also had an added hand written "A"
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2214-07_u5-0a.u5", 0x0000, 0x2000, CRC(0cd31f5e) SHA1(cd49b91e70a74b35990ca4602d4fa1e88a5438d9) ) // Internal designation: M4A4REV0
	ROM_LOAD( "2214-07_u6-0.u6",  0x2000, 0x2000, CRC(459aa388) SHA1(fdc734660a7e5c1efc1b09990db070d62288be3b) ) // Games included in this set are:
	ROM_LOAD( "2214-07_u7-0.u7",  0x4000, 0x2000, CRC(517cc893) SHA1(5d49ab4cddee53cc16d29404e5fb316c81943575) ) // Joker Poker, Blackjack, Foto Finish & The Dice Game

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "chr7_u39.u39",   0x0000, 0x2000, CRC(6662f607) SHA1(6b423f8de011d196700839af0be37effbf87383f) ) // Shows:
	ROM_LOAD( "chr7_u38.u38",   0x2000, 0x2000, CRC(a014b44f) SHA1(906d426b1de75f26030c19dcd599b6570909f510) ) // (c) 1983 Merit industries
	ROM_LOAD( "chr7_u37.u37",   0x4000, 0x2000, CRC(cb12e139) SHA1(06fe91281faae5d0c0ae4b3cd8ad103bd3995c38) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "chr7_u40c.u40",   0x0000, 0x2000, CRC(50fbbada) SHA1(6dd35fee5411606cab51ac3093dd1e171df86721) )
ROM_END

ROM_START( pitbossa ) // Program ROMs were all printed as 2214-04 with the "7" hand written over the 4
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2214-07_u5-0.u5",  0x0000, 0x2000, CRC(346e257a) SHA1(9c5baa3e764be35206f42c8154483d30b61a6e83) ) // Internal designation: M4A4REV0
	ROM_LOAD( "2214-07_u6-0.u6",  0x2000, 0x2000, CRC(459aa388) SHA1(fdc734660a7e5c1efc1b09990db070d62288be3b) ) // Games included in this set are:
	ROM_LOAD( "2214-07_u7-0.u7",  0x4000, 0x2000, CRC(517cc893) SHA1(5d49ab4cddee53cc16d29404e5fb316c81943575) ) // Joker Poker, Blackjack, Foto Finish & The Dice Game

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "chr7_u39.u39",   0x0000, 0x2000, CRC(6662f607) SHA1(6b423f8de011d196700839af0be37effbf87383f) ) // Shows:
	ROM_LOAD( "chr7_u38.u38",   0x2000, 0x2000, CRC(a014b44f) SHA1(906d426b1de75f26030c19dcd599b6570909f510) ) // (c) 1983 Merit industries
	ROM_LOAD( "chr7_u37.u37",   0x4000, 0x2000, CRC(cb12e139) SHA1(06fe91281faae5d0c0ae4b3cd8ad103bd3995c38) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "chr7_u40c.u40",   0x0000, 0x2000, CRC(50fbbada) SHA1(6dd35fee5411606cab51ac3093dd1e171df86721) )
ROM_END

ROM_START( pitboss04 ) // Program ROMs on a CTR-202 daughter card - Internal designation: PBVBREV0
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2214-04_u5-0.u5",   0x0000, 0x2000, CRC(10b782e7) SHA1(158819898ad81506c47b76ffe2a949ee7208740f) ) // Games included in this set are:
	ROM_LOAD( "2214-04_u6-0.u6",   0x2000, 0x2000, CRC(c3fd6510) SHA1(8c89fd2cbcb6f12fa6427883700971f7c39f6ccf) ) // Joker Poker, Blackjack, Foto Finish & The Dice Game
	ROM_RELOAD( 0x4000, 0x2000 )
	ROM_LOAD( "2214-04_u7-0.u7",   0x6000, 0x4000, CRC(c5cf7060) SHA1(4a3209ad24ae649348b0e0470fc446d37b667975) ) // 27128 EPROM

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "chr7_u39.u39",   0x0000, 0x2000, CRC(6662f607) SHA1(6b423f8de011d196700839af0be37effbf87383f) ) // Shows:
	ROM_LOAD( "chr7_u38.u38",   0x2000, 0x2000, CRC(a014b44f) SHA1(906d426b1de75f26030c19dcd599b6570909f510) ) // (c) 1983 Merit industries
	ROM_LOAD( "chr7_u37.u37",   0x4000, 0x2000, CRC(cb12e139) SHA1(06fe91281faae5d0c0ae4b3cd8ad103bd3995c38) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "chr7_u40a.u40",  0x0000, 0x2000, CRC(db62c5ec) SHA1(a9967eb51436f342902fa3ce9c43d4d1ec5e0f3c) )
ROM_END

ROM_START( pitboss03 ) // ROMs also found labeled simply as "PBHD" U5 through U7 (PBHD means Poker, Blackjack, Horse & Dice)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2214-03_u5-0c.u5", 0x0000, 0x2000, CRC(97f870bd) SHA1(b1b01abff0385e3b0585e49f78b93bcf56e434ef) ) // Internal designation: M4A4REV0
	ROM_LOAD( "2214-03_u6-0.u6",  0x2000, 0x2000, CRC(086e699b) SHA1(a1d1eafaac9262f924f175961aa52c6d8e779bf0) ) // Games included in this set are:
	ROM_LOAD( "2214-03_u7-0.u7",  0x4000, 0x2000, CRC(023e8cb8) SHA1(cdb180a94d801137466c13ddfaf65918cb608c5a) ) // Joker Poker, Blackjack, Foto Finish & The Dice Game

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "chr7_u39.u39",   0x0000, 0x2000, CRC(6662f607) SHA1(6b423f8de011d196700839af0be37effbf87383f) ) // Shows:
	ROM_LOAD( "chr7_u38.u38",   0x2000, 0x2000, CRC(a014b44f) SHA1(906d426b1de75f26030c19dcd599b6570909f510) ) // (c) 1983 Merit industries
	ROM_LOAD( "chr7_u37.u37",   0x4000, 0x2000, CRC(cb12e139) SHA1(06fe91281faae5d0c0ae4b3cd8ad103bd3995c38) ) // Cheltenham PA. 19012

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "chr7_u40.u40",   0x0000, 0x2000, CRC(52298162) SHA1(79aa6c4ab6bec6450d882615e64f61cfef934153) )
ROM_END

ROM_START( pitboss03a ) // Specific build for localized region with no Free Hand Bonus
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2214-03_u5-1c.u5", 0x0000, 0x2000, CRC(cf985f96) SHA1(d0e1c3887fe87b92c52410215b5eec600a793c50) ) // Internal designation: M4A4REV0
	ROM_LOAD( "2214-03_u6-0.u6",  0x2000, 0x2000, CRC(086e699b) SHA1(a1d1eafaac9262f924f175961aa52c6d8e779bf0) ) // Games included in this set are:
	ROM_LOAD( "2214-03_u7-0.u7",  0x4000, 0x2000, CRC(023e8cb8) SHA1(cdb180a94d801137466c13ddfaf65918cb608c5a) ) // Joker Poker, Blackjack, Foto Finish & The Dice Game

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "chr7_u39.u39",   0x0000, 0x2000, CRC(6662f607) SHA1(6b423f8de011d196700839af0be37effbf87383f) ) // Shows:
	ROM_LOAD( "chr7_u38.u38",   0x2000, 0x2000, CRC(a014b44f) SHA1(906d426b1de75f26030c19dcd599b6570909f510) ) // (c) 1983 Merit industries
	ROM_LOAD( "chr7_u37.u37",   0x4000, 0x2000, CRC(cb12e139) SHA1(06fe91281faae5d0c0ae4b3cd8ad103bd3995c38) ) // Cheltenham PA. 19012

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "chr7_u40.u40",   0x0000, 0x2000, CRC(52298162) SHA1(79aa6c4ab6bec6450d882615e64f61cfef934153) )
ROM_END

ROM_START( pitboss03b ) // ROMs had no labels, Set has Free Hand Bonus so it might an earlier revision of pitboss03
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u5.bin",           0x0000, 0x2000, CRC(f6b22d25) SHA1(418a1c3e671b5bb4823ea45a5b382b1cbde74f8a) ) // Internal designation: M4A4REV0
	ROM_LOAD( "2214-03_u6-0.u6",  0x2000, 0x2000, CRC(086e699b) SHA1(a1d1eafaac9262f924f175961aa52c6d8e779bf0) ) // Games included in this set are:
	ROM_LOAD( "2214-03_u7-0.u7",  0x4000, 0x2000, CRC(023e8cb8) SHA1(cdb180a94d801137466c13ddfaf65918cb608c5a) ) // Joker Poker, Blackjack, Foto Finish & The Dice Game

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "chr7_u39.u39",   0x0000, 0x2000, CRC(6662f607) SHA1(6b423f8de011d196700839af0be37effbf87383f) ) // Shows:
	ROM_LOAD( "chr7_u38.u38",   0x2000, 0x2000, CRC(a014b44f) SHA1(906d426b1de75f26030c19dcd599b6570909f510) ) // (c) 1983 Merit industries
	ROM_LOAD( "chr7_u37.u37",   0x4000, 0x2000, CRC(cb12e139) SHA1(06fe91281faae5d0c0ae4b3cd8ad103bd3995c38) ) // Cheltenham PA. 19012

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "chr7_u40.u40",   0x0000, 0x2000, CRC(52298162) SHA1(79aa6c4ab6bec6450d882615e64f61cfef934153) )
ROM_END

// Known to exist is Pit Boss version M4A2 (confirmed via manual) and likely a M4A3 as well (not confirmed, but M4A4 is dumped)

ROM_START( pitbossm4 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "m4a1_u5.u5",   0x0000, 0x2000, CRC(f5284472) SHA1(9170b90d06caa382be29feb2f6e80993bba1e07e) ) // Internal designation: M4A1REV0
	ROM_LOAD( "m4a1_u6.u6",   0x2000, 0x2000, CRC(dd8df5fe) SHA1(dab8c1077058263729b2589dd9bf9989ad53be1c) ) // Games included in this set are:
	ROM_LOAD( "m4a1_u7.u7",   0x4000, 0x2000, CRC(5fa5d436) SHA1(9f3fd81eae7f378268f3b4af8fd299ffb97d7fb6) ) // Draw Poker, Blackjack, Acey Deucey & The Dice Game

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "chr2_u39.u39",  0x0000, 0x2000, CRC(f9613e7b) SHA1(1e8cafe142a235d65b43c7e46a79ed4f6272b61c) ) // Shows:
	ROM_LOAD( "chr2_u38.u38",  0x2000, 0x2000, CRC(7af28902) SHA1(04f685389958d581aaf2c86940d1b8b8cec05d7a) ) // (c) 1983 Merit industries   Phila. PA.
	ROM_LOAD( "chr2_u37.u37",  0x4000, 0x2000, CRC(ea6f0c59) SHA1(f2c0ff99518c2cec3eb1b4042fa3754a702c0e34) ) // All Rights Reserved

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "chr2_u40.u40",  0x0000, 0x2000, CRC(40c94dce) SHA1(86611e3a1048b2a3fffcc0110811656a2d0fc4a5) )
ROM_END

ROM_START( pitbossps ) // ROMs also found labeled as U5-0C, U6-0 & U7-0
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "psb1_u5.u5", 0x0000, 0x2000, CRC(d8902656) SHA1(06da829201f6141a6b23afa0e277a3c7a122c26e) ) // Internal designation: PSB1REV0
	ROM_LOAD( "psb1_u6.u6", 0x2000, 0x2000, CRC(bf903b01) SHA1(1f5f69cfd3eb105bd9bad071016931a79defa16b) ) // Games included in this set are:
	ROM_LOAD( "psb1_u7.u7", 0x4000, 0x2000, CRC(306351b9) SHA1(32cd243aa65571ee7fc72971b6a16beeb4ed9d85) ) // Joker Poker, Blackjack, Super Slots & The Dice Game

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "chr7_u39.u39",   0x0000, 0x2000, CRC(6662f607) SHA1(6b423f8de011d196700839af0be37effbf87383f) ) // Shows:
	ROM_LOAD( "chr7_u38.u38",   0x2000, 0x2000, CRC(a014b44f) SHA1(906d426b1de75f26030c19dcd599b6570909f510) ) // (c) 1983 Merit industries
	ROM_LOAD( "chr7_u37.u37",   0x4000, 0x2000, CRC(cb12e139) SHA1(06fe91281faae5d0c0ae4b3cd8ad103bd3995c38) ) // Cheltenham PA. 19012

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "chr7_u40.u40",   0x0000, 0x2000, CRC(52298162) SHA1(79aa6c4ab6bec6450d882615e64f61cfef934153) )
ROM_END

ROM_START( housecard ) // Same exact games as pitbossps set above
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hcsd_u5b.u5", 0x0000, 0x2000, CRC(ecef424f) SHA1(9485be5d800b3ad79b3a6ddce86a174f9aae6bdf) ) // Internal designation: HSC1REV0
	ROM_LOAD( "hcsd_u6.u6",  0x2000, 0x2000, CRC(8fd6ae75) SHA1(6f2fc2903e0eebbe0f3c7bd2b6713046566fa488) ) // Games included in this set are:
	ROM_LOAD( "hcsd_u7.u7",  0x4000, 0x2000, CRC(6adecfa1) SHA1(d6007fbf06cfc4c710a7134de688af439dddcf60) ) // Joker Poker, Blackjack, Super Slots & The Dice Game

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "chr7_u39.u39",   0x0000, 0x2000, CRC(6662f607) SHA1(6b423f8de011d196700839af0be37effbf87383f) ) // Shows:
	ROM_LOAD( "chr7_u38.u38",   0x2000, 0x2000, CRC(a014b44f) SHA1(906d426b1de75f26030c19dcd599b6570909f510) ) // (c) 1983 Licensed By:
	ROM_LOAD( "chr7_u37.u37",   0x4000, 0x2000, CRC(cb12e139) SHA1(06fe91281faae5d0c0ae4b3cd8ad103bd3995c38) ) //         Merit industries

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "chr4_u40.u40",   0x0000, 0x2000, CRC(f4c34a26) SHA1(67183237be6952b3be9ef444d2018bc94e714a66) )
ROM_END

ROM_START( mdchoice )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "e4a1_u5.u5",   0x0000, 0x2000, CRC(bd77f8dc) SHA1(c9c85e3180be30e7a1d37abb6d4e7c777acfda81) ) // Internal designation: E4A1REV0
	ROM_LOAD( "e4a1_u6.u6",   0x2000, 0x2000, CRC(e5219c9a) SHA1(66511f5f9bcd64f3028bbf55a51a1b0db391567c) ) // Games included in this set are:
	ROM_LOAD( "e4a1_u7.u7",   0x4000, 0x2000, CRC(9451d8db) SHA1(c2431b25543218fd8bc5a4eb79e8a77690e26c5e) ) // Draw Poker, Blackjack, Acey Deucey & The Dice Game

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "chr2_u39.u39",  0x0000, 0x2000, CRC(f9613e7b) SHA1(1e8cafe142a235d65b43c7e46a79ed4f6272b61c) ) // Shows:
	ROM_LOAD( "chr2_u38.u38",  0x2000, 0x2000, CRC(7af28902) SHA1(04f685389958d581aaf2c86940d1b8b8cec05d7a) ) // MFG FOR:
	ROM_LOAD( "chr2_u37.u37",  0x4000, 0x2000, CRC(ea6f0c59) SHA1(f2c0ff99518c2cec3eb1b4042fa3754a702c0e34) ) // EEI-CVS International

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "chr2_u40.u40",  0x0000, 0x2000, CRC(40c94dce) SHA1(86611e3a1048b2a3fffcc0110811656a2d0fc4a5) )
ROM_END

ROM_START( mpchoice ) // Same games as pitbossc but different dips & can control the payout percentage (like The Round Up below)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "m4c1_u5.u5",   0x0000, 0x2000, CRC(ceb56251) SHA1(2ce7efccbd158ef59928e5501de9617761a93567) ) // Internal designation: M4C1V0
	ROM_LOAD( "m4c1_u6.u6",   0x2000, 0x2000, CRC(b8fe2ec4) SHA1(3846c2d7a1e5c87badc41277395076df13cb6343) ) // Games included in this set are:
	ROM_LOAD( "m4c1_u7.u7",   0x4000, 0x2000, CRC(f2b6aff8) SHA1(c5c1a4ba808d9830604bc2399e66b60c56bd6f05) ) // Draw Poker, Blackjack, Acey Deucey & The Dice Game

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "chr2_u39.u39",  0x0000, 0x2000, CRC(f9613e7b) SHA1(1e8cafe142a235d65b43c7e46a79ed4f6272b61c) ) // Shows:
	ROM_LOAD( "chr2_u38.u38",  0x2000, 0x2000, CRC(7af28902) SHA1(04f685389958d581aaf2c86940d1b8b8cec05d7a) ) // (c) 1983 Merit industries   Philadelphia PA.
	ROM_LOAD( "chr2_u37.u37",  0x4000, 0x2000, CRC(ea6f0c59) SHA1(f2c0ff99518c2cec3eb1b4042fa3754a702c0e34) ) // All Rights Reserved

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "chr2_u40.u40",  0x0000, 0x2000, CRC(40c94dce) SHA1(86611e3a1048b2a3fffcc0110811656a2d0fc4a5) )
ROM_END

ROM_START( mpchoicea ) // Like the M4C1 set above, but only 3 games here
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "m3c1_u5b.u5",  0x0000, 0x2000, CRC(685eb48a) SHA1(31f41527f7a29379bf783f48ea50c3b74523d304) ) // Internal designation: M3CG
	ROM_LOAD( "m3c1_u6b.u6",  0x2000, 0x2000, CRC(4cf91cca) SHA1(aaf685e66e153fa2c47b90c17af9f70751008e9a) ) // Games included in this set are:
	ROM_LOAD( "m3c1_u7.u7",   0x4000, 0x2000, CRC(5a2aca08) SHA1(0fb4600f61ff1aef2d79ce7a63ee3fd9e79f7f3f) ) // Draw Poker, Blackjack & Acey Deucey

	ROM_REGION( 0x6000, "gfx1", 0 ) // NOTE: U37 & U38 were operator swaps and are not likely correct for this set
	ROM_LOAD( "hcg_u39.u39",   0x0000, 0x2000, CRC(6f82560d) SHA1(206acc5a0fcf391e03a5963bd344e3e15b7c691d) ) // Shows:
	ROM_LOAD( "chr2_u38.u38",  0x2000, 0x2000, CRC(7af28902) SHA1(04f685389958d581aaf2c86940d1b8b8cec05d7a) ) // (c) 1982 Merit industries   Philadelphia PA.
	ROM_LOAD( "chr2_u37.u37",  0x4000, 0x2000, CRC(ea6f0c59) SHA1(f2c0ff99518c2cec3eb1b4042fa3754a702c0e34) ) // All Rights Reserved

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "hcg_u40.u40",  0x0000, 0x2000, CRC(6e20ba8f) SHA1(675cee5b8c38e3b9101c3c0788d2663ce397e40f) )
ROM_END

ROM_START( casino5 )
	ROM_REGION( 0x10000, "maincpu", 0 ) // Program ROMs on a CTR-202A daughter card
	ROM_LOAD( "3315-02_u5-2b.u5", 0x0000, 0x2000, CRC(31640f41) SHA1(22d22ea1b1ae1ff189629ffd4963fabcc300fca8) ) // Internal designation: PACASINO FIVE 331502 U5-0B
	ROM_LOAD( "3315-02_u6-2b.u6", 0x2000, 0x4000, CRC(03851536) SHA1(904bf76567f965e70c0baf545e2f77d07f1c286b) )
	ROM_LOAD( "3315-02_u7-2b.u7", 0x6000, 0x4000, CRC(76acb03f) SHA1(87bfd82015dd704a732f4a03dd470980af1dee72) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "chr7_u39.u39",   0x0000, 0x2000, CRC(6662f607) SHA1(6b423f8de011d196700839af0be37effbf87383f) )
	ROM_LOAD( "chr7_u38.u38",   0x2000, 0x2000, CRC(a014b44f) SHA1(906d426b1de75f26030c19dcd599b6570909f510) )
	ROM_LOAD( "chr7_u37.u37",   0x4000, 0x2000, CRC(cb12e139) SHA1(06fe91281faae5d0c0ae4b3cd8ad103bd3995c38) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "chr8_u40a.u40", 0x0000, 0x2000, CRC(b13a3fb1) SHA1(25760aa27c88b8be248a87df724bf8797d179e7a) )
ROM_END

ROM_START( casino5a ) // Standard version, the rom set with 3315-02 U5-1 is the "Minnesota" version and is undumped
	ROM_REGION( 0x10000, "maincpu", 0 ) // Program ROMs on a CTR-202A daughter card
	ROM_LOAD( "3315-02_u5-0.u5", 0x0000, 0x2000, CRC(abe240d8) SHA1(296eb3251dd51147d6984a8c08c3be22e5ed8e86) ) // Internal designation: PCFS1 331502-0
	ROM_LOAD( "3315-02_u6-0.u6", 0x2000, 0x4000, CRC(4d9f0c57) SHA1(d19b4b4f42d329ea35907d17c15a55b954b07295) )
	ROM_LOAD( "3315-02_u7-0.u7", 0x6000, 0x4000, CRC(d3bc510d) SHA1(6222badabf629dd6334591867596f811883aed52) ) // There is known to be a 3315-02 U7-0-A version (not dumped)

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "chr7_u39.u39",   0x0000, 0x2000, CRC(6662f607) SHA1(6b423f8de011d196700839af0be37effbf87383f) )
	ROM_LOAD( "chr7_u38.u38",   0x2000, 0x2000, CRC(a014b44f) SHA1(906d426b1de75f26030c19dcd599b6570909f510) )
	ROM_LOAD( "chr7_u37.u37",   0x4000, 0x2000, CRC(cb12e139) SHA1(06fe91281faae5d0c0ae4b3cd8ad103bd3995c38) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "chr8_u40a.u40", 0x0000, 0x2000, CRC(b13a3fb1) SHA1(25760aa27c88b8be248a87df724bf8797d179e7a) )
ROM_END

ROM_START( casino5b )
	ROM_REGION( 0x10000, "maincpu", 0 ) // Program ROMs on a CTR-202A daughter card
	ROM_LOAD( "3315-12_u5-0.u5", 0x0000, 0x2000, CRC(50116e80) SHA1(3563a685cba25bf6d2b47280f17488d6427b3bd8) ) // Internal designation: PCFSP 331502SP
	ROM_LOAD( "3315-12_u6-0.u6", 0x2000, 0x4000, CRC(7050ca92) SHA1(8ea351cee84812b31d7a6c3de77a7f43e8b077f8) )
	ROM_LOAD( "3315-12_u7-0.u7", 0x6000, 0x4000, CRC(ddd97b53) SHA1(57f86efa3d87e8eb226506f4a37481c5132a5a6a) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "chr7_u39.u39",   0x0000, 0x2000, CRC(6662f607) SHA1(6b423f8de011d196700839af0be37effbf87383f) )
	ROM_LOAD( "chr7_u38.u38",   0x2000, 0x2000, CRC(a014b44f) SHA1(906d426b1de75f26030c19dcd599b6570909f510) )
	ROM_LOAD( "chr7_u37.u37",   0x4000, 0x2000, CRC(cb12e139) SHA1(06fe91281faae5d0c0ae4b3cd8ad103bd3995c38) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "chr8_u40a.u40", 0x0000, 0x2000, CRC(b13a3fb1) SHA1(25760aa27c88b8be248a87df724bf8797d179e7a) )
ROM_END

ROM_START( mroundup )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "kru1cd_u5.u5",  0x0000, 0x2000, CRC(7bd90672) SHA1(5f763e807370df991cc2c4abaeb8184c42149a0e) ) // Internal designation: RUM1HRV0
	ROM_LOAD( "kru1cd_u6.u6",  0x2000, 0x2000, CRC(fccffb0d) SHA1(57cbfb7e2bf1050cb2da8b4cc7cdd7e18356bcd2) ) // Games included in this set are:
	ROM_LOAD( "kru1cd_u7.u7",  0x4000, 0x2000, CRC(72131230) SHA1(e7c08b537848a7c6e6e987c6d0644a031bb238d4) ) // Draw Poker, Blackjack, Foto Finish

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "chr7_u39.u39",   0x0000, 0x2000, CRC(6662f607) SHA1(6b423f8de011d196700839af0be37effbf87383f) )
	ROM_LOAD( "chr7_u38.u38",   0x2000, 0x2000, CRC(a014b44f) SHA1(906d426b1de75f26030c19dcd599b6570909f510) )
	ROM_LOAD( "chr7_u37.u37",   0x4000, 0x2000, CRC(cb12e139) SHA1(06fe91281faae5d0c0ae4b3cd8ad103bd3995c38) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "chr7_u40.u40",   0x0000, 0x2000, CRC(52298162) SHA1(79aa6c4ab6bec6450d882615e64f61cfef934153) )
ROM_END

ROM_START( chkndraw )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2131-04_u5-1.u5", 0x0000, 0x2000, CRC(abc38151) SHA1(a56f7e535e265cbff697863fdfa5e5c2ef8b690a) ) // 02131-10 U50 U60 U70 102984
	ROM_LOAD( "2131-04_u6-1.u6", 0x2000, 0x2000, CRC(a0ad37b6) SHA1(f6722a8920d894cbce43fa78f2812cd81e5e9185) ) // Need to verify label, different than U6 ROM from chkndrawa below
	ROM_LOAD( "2131-04_u7-0.u7", 0x4000, 0x2000, CRC(c8af231d) SHA1(97f36420c9f4dd75c673003b9fd8287517b948f0) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "u39", 0x0000, 0x2000, CRC(115d1fe5) SHA1(c01287bf2ee34f94f38092a14ef6a76ebc970c2c) )
	ROM_LOAD( "u38", 0x2000, 0x2000, CRC(841f5515) SHA1(47451418d9da845b5c4cf054e1dbbf68afa5bcee) )
	ROM_LOAD( "u37", 0x4000, 0x2000, CRC(f554134d) SHA1(417dc83d24b4d39232b680e4004bf050c9cbb159) )

	ROM_REGION( 0x2000, "gfx2", 0 )
//  ROM_LOAD( "u40",  0x0000, 0x2000, BAD_DUMP CRC(01722f98) SHA1(c75c9511c07379ea087be5d75cbc3e705628c824) )
//  ROM_LOAD( "u40a", 0x0000, 0x2000, BAD_DUMP CRC(03543d67) SHA1(5ae08dbc0f736c11070befb4cfad87ddaa24cef2) )
	ROM_LOAD( "u40b", 0x0000, 0x2000, BAD_DUMP CRC(c53a9e90) SHA1(2076d045c279405083fec8949425532e7e7e7844) )
ROM_END

ROM_START( chkndrawa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2131-04_u5-0.u5", 0x0000, 0x2000, CRC(8edcd0a0) SHA1(e876061997b1577e6e06ab036de0b2b037372815) ) // 02131-00D U50 U60 U70 102984
	ROM_LOAD( "2131-04_u6-0.u6", 0x2000, 0x2000, CRC(491466d7) SHA1(42a374612720ab6b642313fa0075e96dd306d207) )
	ROM_LOAD( "2131-04_u7-0.u7", 0x4000, 0x2000, CRC(c8af231d) SHA1(97f36420c9f4dd75c673003b9fd8287517b948f0) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "chr5_u39.u39", 0x0000, 0x2000, CRC(b5a5f7da) SHA1(51a3cda89d514c0230f9dd85626ffefb87bae3a6) )
	ROM_LOAD( "chr5_u38.u38", 0x2000, 0x2000, CRC(5c2cc495) SHA1(7b475d8bcbee5ecaadce48ae8e52a18c76b0b2ea) )
	ROM_LOAD( "chr5_u37.u37", 0x4000, 0x2000, CRC(4c584ff0) SHA1(9718124577a2132b8c6a117b86b3699041417204) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "chr5_u40.u40", 0x0000, 0x2000, CRC(45b84a7c) SHA1(e1d386dace8f73f07cf75fabb674668af6d8d833) )
ROM_END

ROM_START( riviera )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2131-08_u5-4a.u5", 0x0000, 0x8000, CRC(0bc8cf26) SHA1(da52010be2d44a240160bb1a13288b35e8feade2) ) // 08 U5-4A 111287 2131-84A, label shows (c) 1988

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "hisc_u39.u39", 0x00000, 0x2000, CRC(1814c2ea) SHA1(fecc5dc1c0a56cbc7b68ee6a52222de348d6cc79) )
	ROM_LOAD( "hisc_u38.u38", 0x02000, 0x2000, CRC(ef1d7a80) SHA1(539662bee187a300a6f1bcded954758c87171219) )
	ROM_LOAD( "hisc_u37.u37", 0x04000, 0x2000, CRC(f6e709f8) SHA1(02905be912d0aa794f82926462f854e8e67dc407) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "hisc_u40.u40", 0x00000, 0x2000, CRC(6d2a1ca8) SHA1(96ef3e0914c2b213ed9c9082fa3e27d75d52a8ec) )

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "dec003.u13", 0x000, 0x117, CRC(5b9a2fec) SHA1(c56c7bbe13028903cfc82440ee8b24df855134c2) ) // PAL16L8ANC - brute forced
ROM_END

ROM_START( rivieraa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2131-08_u5-4.u5", 0x0000, 0x8000, CRC(ce0b00f2) SHA1(c467c2c08d0bbadf80d67f41e17127e08ce3b3ff) ) // 08 U5-4 111786 2131-84, label shows (c) 1987

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "hisc_u39.u39", 0x00000, 0x2000, CRC(1814c2ea) SHA1(fecc5dc1c0a56cbc7b68ee6a52222de348d6cc79) )
	ROM_LOAD( "hisc_u38.u38", 0x02000, 0x2000, CRC(ef1d7a80) SHA1(539662bee187a300a6f1bcded954758c87171219) )
	ROM_LOAD( "hisc_u37.u37", 0x04000, 0x2000, CRC(f6e709f8) SHA1(02905be912d0aa794f82926462f854e8e67dc407) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "hisc_u40.u40", 0x00000, 0x2000, CRC(6d2a1ca8) SHA1(96ef3e0914c2b213ed9c9082fa3e27d75d52a8ec) )

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "dec003.u13", 0x000, 0x117, CRC(5b9a2fec) SHA1(c56c7bbe13028903cfc82440ee8b24df855134c2) ) // PAL16L8ANC - brute forced
ROM_END

ROM_START( rivierab )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2131-08_u5-2d.u5", 0x0000, 0x8000, CRC(64c6892b) SHA1(d245d4a9933e3b21279542da0cb6ee641569ef6c) ) // 08 U5-2D 022086 2131-82d, label shows (c) 1985

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "hisc_u39.u39", 0x00000, 0x2000, CRC(1814c2ea) SHA1(fecc5dc1c0a56cbc7b68ee6a52222de348d6cc79) )
	ROM_LOAD( "hisc_u38.u38", 0x02000, 0x2000, CRC(ef1d7a80) SHA1(539662bee187a300a6f1bcded954758c87171219) )
	ROM_LOAD( "hisc_u37.u37", 0x04000, 0x2000, CRC(f6e709f8) SHA1(02905be912d0aa794f82926462f854e8e67dc407) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "hisc_u40.u40", 0x00000, 0x2000, CRC(6d2a1ca8) SHA1(96ef3e0914c2b213ed9c9082fa3e27d75d52a8ec) )

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "dec003.u13", 0x000, 0x117, CRC(5b9a2fec) SHA1(c56c7bbe13028903cfc82440ee8b24df855134c2) ) // PAL16L8ANC - brute forced
ROM_END

// Sub board CRT-203 includes 2 P8255A, parallel printer connection & MM58174AN RTC that plugs in through the CRT-200's P8255 socket.
// There is a battery that connects to the PCB to keep the CRT-200's Mosel MS6264L-10PC RAM active and also runs to the CRT-203 for the RTC (guess)
ROM_START( mosdraw )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "4436-05_u5-0.u5", 0x0000, 0x8000, CRC(d0194059) SHA1(4e106c7e38fd92e005f5e1899b6fbca4ab62ce6d) ) // 4436-05 U5-0  041790

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "tana_u39.u39", 0x00000, 0x2000, CRC(e17c977d) SHA1(cb622fdb2ec001b9a77b17c11c4576c1b4efb248) )
	ROM_LOAD( "tana_u38.u38", 0x02000, 0x2000, CRC(b3e4e24a) SHA1(2711ad68937c71f8d2e5c8efe83928e03134917b) )
	ROM_LOAD( "tana_u37.u37", 0x04000, 0x2000, CRC(013c5eab) SHA1(363b8128e0ab3f00c26b0cd3cc8636b10b5fbd73) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "tana_u40.u40", 0x00000, 0x2000, CRC(a45cae66) SHA1(499759badc006fa09706d349e252284949d20a2d) )

	ROM_REGION( 0x2000, "nvram", 0 )
	ROM_LOAD( "nvram",  0x0000, 0x2000, CRC(61351962) SHA1(b2a18563c41b58385d6b0ccbc621fddd0d82f1b5) ) // preconfigured NVRAM to avoid error on boot
ROM_END

ROM_START( bigappg )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2131-13_u5-0.u5", 0x0000, 0x8000, CRC(47bad6fd) SHA1(87f6c603b52e184f82179869d7b58453cbd34814) ) // 2131-13 U5-0 111786

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "haip_u39.u39", 0x0000, 0x2000, CRC(506e20b4) SHA1(cd28d4f56f83ed3f7cfa44280975c0a6af474707) )
	ROM_LOAD( "haip_u38.u38", 0x2000, 0x2000, CRC(1924feeb) SHA1(c4656a04e2284a265554ea9774d0201c44864809) )
	ROM_LOAD( "haip_u37.u37", 0x4000, 0x2000, CRC(12a04867) SHA1(8e3a6050d854ccc906ae473a104a16d610e4f1da) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "haip_u40.u40", 0x0000, 0x2000, CRC(ac4983b8) SHA1(a552a15f813c331de67eaae2ed42cc037b26c5bd) )
ROM_END

ROM_START( misdraw )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2131-16_u5-2.u5", 0x0000, 0x8000, CRC(fc756320) SHA1(6b810c57ed1be844a04a6081d727e182509604b4) ) // 2131-16 U5-2 081889

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "u39.u39", 0x0000, 0x2000, CRC(0f09d19b) SHA1(1f98559d5bad7c84d92ecea5a6df9429914a47f0) )
	ROM_LOAD( "u38.u38", 0x2000, 0x2000, CRC(8210a48d) SHA1(9af3e8ac8dcf1e548c4ba3ca8096e48dbb3b4700) )
	ROM_LOAD( "u37.u37", 0x4000, 0x2000, CRC(34ca07d5) SHA1(3656b3eb78dd6ea06cf323a08fc3f949a01b76a3) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "haip_u40.u40", 0x0000, 0x2000, CRC(ac4983b8) SHA1(a552a15f813c331de67eaae2ed42cc037b26c5bd) )

	ROM_REGION( 0x0800, "crt209", 0 ) // contains Z80 program code, the game jumps here on startup
	ROM_LOAD( "crt-209_2131-16", 0x0000, 0x0800, BAD_DUMP CRC(34729437) SHA1(f097a1a97d8078d7d6a6af85be416b1d1d09c7f2) ) // pre-decrypted code, not sure of method used to dump/obtain data

	ROM_REGION( 0x117, "plds", 0 ) // PAL inside CRT-209 module
	ROM_LOAD( "crt-209_pal16l8.bin", 0x000, 0x117, CRC(e916c56f) SHA1(1517091ff1791d923e5bd62d18d1428b6a3a8c72) ) // SC3339 20-pin 16L8 type PAL
ROM_END

ROM_START( iowapp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2131-21_u5-1.u5", 0x0000, 0x8000, CRC(29ce9656) SHA1(24054176f63957883ad9c022644de28684b95623) ) // 2131-21 U5-1 981221 2131-21B - label shows copyright 1990

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "iowa_u39.u39", 0x0000, 0x2000, CRC(8d003bbe) SHA1(06711632f4dea6b794b112525b0e26db698b334b) ) // labels show copyright 1989
	ROM_LOAD( "iowa_u38.u38", 0x2000, 0x2000, CRC(2f2152a8) SHA1(c0a6dd92ef5eb60363ac3855d62fcea07006368e) )
	ROM_LOAD( "iowa_u37.u37", 0x4000, 0x2000, CRC(393c78fe) SHA1(d913d081a7205c19dff6a7c6d604716695de2e98) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "iowa_u40.u40", 0x0000, 0x2000, CRC(6d2a1ca8) SHA1(96ef3e0914c2b213ed9c9082fa3e27d75d52a8ec) )
ROM_END

ROM_START( dodgectyba )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2131-82_u5-0d.u5", 0x0000, 0x8000, CRC(ef71b268) SHA1(c85f2c8e7e9cd89b4720699814d8fcfbecf4dc1b) ) // 2131-82 U5-0D 884111 2131 820

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "dodg_u39.u39", 0x00000, 0x8000, CRC(3b3376a1) SHA1(6880cdc29686ff7328717c3833ff826c278b023e) ) // These 3 ROMs: 1st & 2nd half identical - Verified correct
	ROM_LOAD( "dodg_u38.u38", 0x08000, 0x8000, CRC(654d5b00) SHA1(9e16330b2dc8821fc20a39eb42176fda23085bfc) )
	ROM_LOAD( "dodg_u37.u37", 0x10000, 0x8000, CRC(bc9e63d4) SHA1(2320f5a0545f18e1e42a3a45fedce912c36fbe13) )

	ROM_REGION( 0x8000, "gfx2", ROMREGION_ERASEFF )
	// No U40 char ROM - Verified on 4 PCBs

	ROM_REGION( 0x0800, "crt209", 0 ) // contains Z80 program code to read inputs
	ROM_LOAD( "crt-209_2131-82", 0x0000, 0x0800, CRC(ec540d8a) SHA1(fbc64d4cc56f418bc090b47bb6798e3a90282f56) )
ROM_END

ROM_START( dodgectybb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2131-82_u5-50.u5", 0x0000, 0x8000, CRC(eb82515d) SHA1(d2c15bd633472f50b621ba90598559e345246d01) ) // 2131-82 U5-50 987130 2131 825

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "dodg_u39.u39", 0x00000, 0x8000, CRC(3b3376a1) SHA1(6880cdc29686ff7328717c3833ff826c278b023e) ) // These 3 ROMs: 1st & 2nd half identical - Verified correct
	ROM_LOAD( "dodg_u38.u38", 0x08000, 0x8000, CRC(654d5b00) SHA1(9e16330b2dc8821fc20a39eb42176fda23085bfc) )
	ROM_LOAD( "dodg_u37.u37", 0x10000, 0x8000, CRC(bc9e63d4) SHA1(2320f5a0545f18e1e42a3a45fedce912c36fbe13) )

	ROM_REGION( 0x8000, "gfx2", ROMREGION_ERASEFF )
	// No U40 char ROM - Verified on 4 PCBs

	ROM_REGION( 0x0800, "crt209", 0 ) // contains Z80 program code to read inputs
	ROM_LOAD( "crt-209_2131-82", 0x0000, 0x0800, CRC(ec540d8a) SHA1(fbc64d4cc56f418bc090b47bb6798e3a90282f56) )

	ROM_REGION( 0x117, "plds", 0 ) // PAL inside CRT-209 module
	ROM_LOAD( "crt-209_pal16l8.bin", 0x000, 0x117, CRC(e916c56f) SHA1(1517091ff1791d923e5bd62d18d1428b6a3a8c72) ) // SC3339 20-pin 16L8 type PAL
ROM_END

ROM_START( dodgectybc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2131-82_u5-0_gt.u5", 0x0000, 0x8000, CRC(3858cd50) SHA1(1b1e208076df964afd68d01aa8d5489d36a934a5) ) // 2131-82 U5-0 GT 982050 2131 820

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "dodg_u39.u39", 0x00000, 0x8000, CRC(3b3376a1) SHA1(6880cdc29686ff7328717c3833ff826c278b023e) ) // These 3 ROMs: 1st & 2nd half identical - Verified correct
	ROM_LOAD( "dodg_u38.u38", 0x08000, 0x8000, CRC(654d5b00) SHA1(9e16330b2dc8821fc20a39eb42176fda23085bfc) )
	ROM_LOAD( "dodg_u37.u37", 0x10000, 0x8000, CRC(bc9e63d4) SHA1(2320f5a0545f18e1e42a3a45fedce912c36fbe13) )

	ROM_REGION( 0x8000, "gfx2", ROMREGION_ERASEFF )
	// No U40 char ROM - Verified on 4 PCBs

	ROM_REGION( 0x0800, "crt209", 0 ) // contains Z80 program code to read inputs
	ROM_LOAD( "crt-209_2131-82", 0x0000, 0x0800, CRC(ec540d8a) SHA1(fbc64d4cc56f418bc090b47bb6798e3a90282f56) )

	ROM_REGION( 0x117, "plds", 0 ) // PAL inside CRT-209 module
	ROM_LOAD( "crt-209_pal16l8.bin", 0x000, 0x117, CRC(e916c56f) SHA1(1517091ff1791d923e5bd62d18d1428b6a3a8c72) ) // SC3339 20-pin 16L8 type PAL
ROM_END

ROM_START( msupstar )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "4435-81_u5-1.u5", 0x0000, 0x8000, CRC(38ed804a) SHA1(fc500db9d5e5eac7d9a88756f7d0176a887f1fd1) ) // 4435-81 U5-1 984140  4435811

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "u39", 0x00000, 0x8000, CRC(aba5aa05) SHA1(7929c5508e4eefc3905b40d7d51e5d80a1550f77) ) // These 3 ROMs: 1st & 2nd half identical - Verified correct
	ROM_LOAD( "u38", 0x08000, 0x8000, CRC(50032b4f) SHA1(e39b4068ee6863aa4ba22232928b450e7ab47e63) )
	ROM_LOAD( "u37", 0x10000, 0x8000, CRC(fe9c41fa) SHA1(4da945fd5c8e797ccb72ac931a01e322aabbe8ee) )

	ROM_REGION( 0x8000, "gfx2", ROMREGION_ERASEFF )
	// No U40 char ROM

	ROM_REGION( 0x0800, "crt209", 0 ) // contains Z80 program code to read inputs
	ROM_LOAD( "crt-209_4435-81.cpu", 0x0000, 0x0800, CRC(0c94ef71) SHA1(6e6eb0ffa7adf0ef7cdcc2d891c37814eb8d4a61) )

	ROM_REGION( 0x117, "plds", 0 ) // PAL inside CRT-209 module
	ROM_LOAD( "crt-209_pal16l8.bin", 0x000, 0x117, CRC(e916c56f) SHA1(1517091ff1791d923e5bd62d18d1428b6a3a8c72) ) // SC3339 20-pin 16L8 type PAL
ROM_END

ROM_START( trvwz )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6221-00_u5.u5", 0x0000, 0x2000, CRC(731fd5b1) SHA1(1074780321029446da0e6765b9e036b06b067a48) )
	ROM_LOAD( "6221-00_u6.u6", 0x2000, 0x2000, CRC(af6886c0) SHA1(48005b921d7ce33ffc0ba160be82053a26382a9d) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "triv_1_u39.u39", 0x0000, 0x2000, CRC(f8a5f5fb) SHA1(a511e1a2b5e887ef00dc919e9e664ccec2d36cfa) )
	ROM_LOAD( "triv_1_u38.u38", 0x2000, 0x2000, CRC(27621e52) SHA1(a7e88d329e2e774fef9bd8c5cefb4d8f1cfcba4c) )
	ROM_LOAD( "triv_1_u37.u37", 0x4000, 0x2000, CRC(f739b5dc) SHA1(fbf469b7f4cab50e06ec2def9344e3b9801a275e) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "triv_1_u40.u40", 0x0000, 0x2000, CRC(cea7319f) SHA1(663cd18a4699dfd5ad1d3357094eff247e9b4a47) )

	ROM_REGION( 0xa0000, "questions", ROMREGION_ERASEFF )
	ROM_LOAD( "ent-001_01a",  0x08000, 0x8000, CRC(ff45d92b) SHA1(10356bc6a04b2c53ecaf76cb0cba3ec70b4ba612) ) // This set verified as all found on the same question board
	ROM_LOAD( "ent-001_02a",  0x18000, 0x8000, CRC(902e26f7) SHA1(f13b816bfc507fb429fb3f44531de346a82c780d) )
	ROM_LOAD( "gen-001_01a",  0x28000, 0x8000, CRC(1d8d353f) SHA1(6bd0cc5c67da81a48737f32bc49cbf235648c4c6) )
	ROM_LOAD( "gen-001_02a",  0x3c000, 0x4000, CRC(2000e3c3) SHA1(21737fde3d1a1b22da4590476e4e52ee1bab026f) ) // 27128 EPROM, others are 27256
	ROM_LOAD( "spo-001_01a",  0x48000, 0x8000, CRC(ae111429) SHA1(ff551d7ac7ad367338e908805aeb78c59a747919) )
	ROM_LOAD( "spo-001_02a",  0x58000, 0x8000, CRC(ee9263b3) SHA1(1644ab01f17e3af1e193e509d64dcbb243d3eb80) )
	ROM_LOAD( "spo-001_03a",  0x68000, 0x8000, CRC(64181d34) SHA1(f84e28fc589b86ca6a596815871ed26602bcc095) )

	ROM_REGION( 0x0100, "prom", 0 ) // BPROM on Question ROM board used as KEY to decode questions
	ROM_LOAD( "sc-001", 0x00000, 0x0100, NO_DUMP ) // 74S287 (==N82S129N) BPROM
ROM_END

ROM_START( trvwza )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6221-00_u5.u5", 0x0000, 0x2000, CRC(731fd5b1) SHA1(1074780321029446da0e6765b9e036b06b067a48) )
	ROM_LOAD( "6221-00_u6.u6", 0x2000, 0x2000, CRC(af6886c0) SHA1(48005b921d7ce33ffc0ba160be82053a26382a9d) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "triv_1_u39.u39", 0x0000, 0x2000, CRC(f8a5f5fb) SHA1(a511e1a2b5e887ef00dc919e9e664ccec2d36cfa) )
	ROM_LOAD( "triv_1_u38.u38", 0x2000, 0x2000, CRC(27621e52) SHA1(a7e88d329e2e774fef9bd8c5cefb4d8f1cfcba4c) )
	ROM_LOAD( "triv_1_u37.u37", 0x4000, 0x2000, CRC(f739b5dc) SHA1(fbf469b7f4cab50e06ec2def9344e3b9801a275e) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "triv_1_u40.u40", 0x0000, 0x2000, CRC(cea7319f) SHA1(663cd18a4699dfd5ad1d3357094eff247e9b4a47) )

	ROM_REGION( 0xa0000, "questions", ROMREGION_ERASEFF )
	ROM_LOAD( "ent-001_01a",  0x08000, 0x8000, CRC(ff45d92b) SHA1(10356bc6a04b2c53ecaf76cb0cba3ec70b4ba612) )
	ROM_LOAD( "ent-001_02a",  0x18000, 0x8000, CRC(902e26f7) SHA1(f13b816bfc507fb429fb3f44531de346a82c780d) )
	ROM_LOAD( "gen-001_01a",  0x28000, 0x8000, CRC(1d8d353f) SHA1(6bd0cc5c67da81a48737f32bc49cbf235648c4c6) )
	ROM_LOAD( "gen-001_02a",  0x3c000, 0x4000, CRC(2000e3c3) SHA1(21737fde3d1a1b22da4590476e4e52ee1bab026f) ) // 27128 EPROM, others are 27256
	ROM_LOAD( "sex-001_01a",  0x48000, 0x8000, CRC(32519098) SHA1(d070e02bb10e04964893903599a69a8943f9ac8a) )
	ROM_LOAD( "sex-001_02a",  0x88000, 0x8000, CRC(0be4ef9a) SHA1(c80080f1c853e1043bf7e47bea322540a8ac9195) )

	ROM_REGION( 0x0100, "prom", 0 ) // BPROM on Question ROM board used as KEY to decode questions
	ROM_LOAD( "sc-001", 0x00000, 0x0100, NO_DUMP ) // 74S287 (==N82S129N) BPROM
ROM_END

// question board only - this contained a variety of ROMs from the 'trvwz' and 'trvwza' sets as well as 2 unique general knowledge ones
ROM_START( trvwzb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6221-00_u5.u5", 0x0000, 0x2000, CRC(731fd5b1) SHA1(1074780321029446da0e6765b9e036b06b067a48) )
	ROM_LOAD( "6221-00_u6.u6", 0x2000, 0x2000, CRC(af6886c0) SHA1(48005b921d7ce33ffc0ba160be82053a26382a9d) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "triv_1_u39.u39", 0x0000, 0x2000, CRC(f8a5f5fb) SHA1(a511e1a2b5e887ef00dc919e9e664ccec2d36cfa) )
	ROM_LOAD( "triv_1_u38.u38", 0x2000, 0x2000, CRC(27621e52) SHA1(a7e88d329e2e774fef9bd8c5cefb4d8f1cfcba4c) )
	ROM_LOAD( "triv_1_u37.u37", 0x4000, 0x2000, CRC(f739b5dc) SHA1(fbf469b7f4cab50e06ec2def9344e3b9801a275e) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "triv_1_u40.u40",   0x0000, 0x2000, CRC(cea7319f) SHA1(663cd18a4699dfd5ad1d3357094eff247e9b4a47) )

	ROM_REGION( 0xa0000, "questions", ROMREGION_ERASEFF )
	ROM_LOAD( "ent-001_01a",  0x08000, 0x8000, CRC(ff45d92b) SHA1(10356bc6a04b2c53ecaf76cb0cba3ec70b4ba612) )
	ROM_LOAD( "ent-001_02a",  0x18000, 0x8000, CRC(902e26f7) SHA1(f13b816bfc507fb429fb3f44531de346a82c780d) )
	ROM_LOAD( "merit2_6.1",   0x28000, 0x8000, CRC(8a4bcde3) SHA1(528ae9d3ff0b98201f89fd6b93a712cd7f0e9ab4) ) // alt General Trivia - Need correct ROM label
	ROM_LOAD( "merit2_6.2",   0x38000, 0x8000, CRC(ded7e124) SHA1(7e6e04ae79dceba70d83ccfde4f9d0ccc0737c78) ) // alt General Trivia - Need correct ROM label
	ROM_LOAD( "spo-001_01a",  0x48000, 0x8000, CRC(ae111429) SHA1(ff551d7ac7ad367338e908805aeb78c59a747919) )
	ROM_LOAD( "spo-001_02a",  0x58000, 0x8000, CRC(ee9263b3) SHA1(1644ab01f17e3af1e193e509d64dcbb243d3eb80) )
	ROM_LOAD( "spo-001_03a",  0x68000, 0x8000, CRC(64181d34) SHA1(f84e28fc589b86ca6a596815871ed26602bcc095) )

	ROM_REGION( 0x0100, "prom", 0 ) // BPROM on Question ROM board used as KEY to decode questions
	ROM_LOAD( "sc-001", 0x00000, 0x0100, NO_DUMP ) // 74S287 (==N82S129N) BPROM
ROM_END

ROM_START( trvwzv )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6221-02_u5.u5", 0x0000, 0x2000, CRC(fd3531ac) SHA1(d11573df65676e704b28cc2d99fb004b48a358a4) )
	ROM_LOAD( "6221-02_u6.u6", 0x2000, 0x2000, CRC(29e43d0e) SHA1(ad610748fe37436880648078f5d1a305cb147c5d) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "trvs_u39.u39", 0x0000, 0x2000, CRC(b9d9a80e) SHA1(55b6a0d09f8619df93ba936e083835c859a557df) )
	ROM_LOAD( "trvs_u38.u38", 0x2000, 0x2000, CRC(8348083e) SHA1(260a4c1ae043e7ceac65a8818c23940d32275879) )
	ROM_LOAD( "trvs_u37.u37", 0x4000, 0x2000, CRC(b4d3c9f4) SHA1(dda99549306519c147d275d8c6af672e80a96b67) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "trvs_u40.u40", 0x0000, 0x2000, CRC(1f0ff6e0) SHA1(5a31afde34aeb6f851389d093bb426e5cfdedbf2) )

	ROM_REGION( 0xa0000, "questions", ROMREGION_ERASEFF )
	ROM_LOAD( "ent-001_01",  0x08000, 0x8000, CRC(c68ce954) SHA1(bf70fe64f095d5cfcf5347d83651b78c6c8bf05f) )
	ROM_LOAD( "ent-001_02",  0x18000, 0x8000, CRC(aac4ff63) SHA1(d68c4408b4dad976e317a33f2a4eaee39d90dbed) )
	ROM_LOAD( "gen-001_01",  0x28000, 0x8000, CRC(5deb1900) SHA1(b7e9407c37481ef8953e8283d45949d951302e92) )
	ROM_LOAD( "gen-001_02",  0x3c000, 0x4000, CRC(d2b53b6a) SHA1(f75334e47885086e277682daf018818a02ce1026) ) // 27128 EPROM, others are 27256
	ROM_LOAD( "spo-001_01",  0x48000, 0x8000, CRC(7b56315d) SHA1(4c8c63b80176bfac9594958a7043627012baada3) )
	ROM_LOAD( "spo-001_02",  0x58000, 0x8000, CRC(148b63ee) SHA1(9f3b222d979f23b313f379cbc06cc00d88d08c56) )
	ROM_LOAD( "spo-001_03",  0x68000, 0x8000, CRC(a6af8e41) SHA1(64f672bfa5fb2c0575103614986e53e238c5984f) )

	ROM_REGION( 0x0100, "prom", 0 ) // BPROM on Question ROM board used as KEY to decode questions
	ROM_LOAD( "sc-001", 0x00000, 0x0100, NO_DUMP ) // 74S287 (==N82S129N) BPROM
ROM_END

ROM_START( trvwz2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6221-05_u5-0e.u5", 0x0000, 0x2000, CRC(97b8d320) SHA1(573945531113d8aae9418ba1e9a2063052227029) ) // internally shows 6221-05V
	ROM_LOAD( "6221-05_u6-0e.u6", 0x2000, 0x2000, CRC(2e86288d) SHA1(62c7024d8dfebed9bb05ea91302efe5d18cb7d2a) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "trvs_u39.u39", 0x0000, 0x2000, CRC(b9d9a80e) SHA1(55b6a0d09f8619df93ba936e083835c859a557df) )
	ROM_LOAD( "trvs_u38.u38", 0x2000, 0x2000, CRC(8348083e) SHA1(260a4c1ae043e7ceac65a8818c23940d32275879) )
	ROM_LOAD( "trvs_u37.u37", 0x4000, 0x2000, CRC(b4d3c9f4) SHA1(dda99549306519c147d275d8c6af672e80a96b67) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "trvs_u40a.u40", 0x0000, 0x2000, CRC(fbfae092) SHA1(b8569819952a5c805f11b6854d64b3ae9c857f97) )

	ROM_REGION( 0xa0000, "questions", ROMREGION_ERASEFF )
	ROM_LOAD( "ent-101.01a",  0x08000, 0x8000, CRC(3825ac47) SHA1(d0da047c4d30a26f496b3663cfda77c229279be8) ) // This set verified as all found on the same question board
	ROM_LOAD( "ent-101.02a",  0x18000, 0x8000, CRC(a0153407) SHA1(e669957a5d4775bfa2c16960a2a909a3505c078b) )
	ROM_LOAD( "ent-101.03a",  0x28000, 0x8000, CRC(755b16ab) SHA1(277ea4110479ecdb2c772299ea04f4918cf7f561) )
	ROM_LOAD( "gen-101.01a",  0x38000, 0x8000, CRC(74d14039) SHA1(54b85581d60fb535d37a051f375e687a933600ea) )
	ROM_LOAD( "gen-101.02a",  0x48000, 0x8000, CRC(b1b930d8) SHA1(57be3ee1c0adcb549088818dc7efda64508b5647) ) // These question ROMs have been found with the "trvwz3a"
	ROM_LOAD( "spo-101.01a",  0x58000, 0x8000, CRC(9dc4ba98) SHA1(4ce2bbbd7436a0ba8140879d5d8614bddbd5a8ec) )
	ROM_LOAD( "spo-101.02a",  0x68000, 0x8000, CRC(9c106ad9) SHA1(1d1a5c91152283e3937a2df17cd57b8fe04072b7) )
	ROM_LOAD( "spo-101.03a",  0x78000, 0x8000, CRC(3d69c3a3) SHA1(9f16d45660f3cb15e44e9fc0d940a7b2b12819e8) )
	ROM_LOAD( "sex-101.01a",  0x88000, 0x8000, CRC(301d65c2) SHA1(48d260077e9c9ed82f6dfa176b1103723dc9e19a) )
	ROM_LOAD( "sex-101.02a",  0x98000, 0x8000, CRC(2596091b) SHA1(7fbb9c2c3f74e12714513928c8cf3769bf29fc8b) )

	ROM_REGION( 0x0100, "prom", 0 ) // BPROM on Question ROM board used as KEY to decode questions
	ROM_LOAD( "sc-002", 0x00000, 0x0100, CRC(94a8da8a) SHA1(8bdaee436481418425c36de24477c96ec0787916) ) // N82S129N BPROM
ROM_END

ROM_START( trvwz2a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6221-05_u5-0e.u5", 0x0000, 0x2000, CRC(97b8d320) SHA1(573945531113d8aae9418ba1e9a2063052227029) ) // internally shows 6221-05V
	ROM_LOAD( "6221-05_u6-0e.u6", 0x2000, 0x2000, CRC(2e86288d) SHA1(62c7024d8dfebed9bb05ea91302efe5d18cb7d2a) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "trvs_u39.u39", 0x0000, 0x2000, CRC(b9d9a80e) SHA1(55b6a0d09f8619df93ba936e083835c859a557df) )
	ROM_LOAD( "trvs_u38.u38", 0x2000, 0x2000, CRC(8348083e) SHA1(260a4c1ae043e7ceac65a8818c23940d32275879) )
	ROM_LOAD( "trvs_u37.u37", 0x4000, 0x2000, CRC(b4d3c9f4) SHA1(dda99549306519c147d275d8c6af672e80a96b67) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "trvs_u40a.u40", 0x0000, 0x2000, CRC(fbfae092) SHA1(b8569819952a5c805f11b6854d64b3ae9c857f97) )

	ROM_REGION( 0xa0000, "questions", ROMREGION_ERASEFF )
	ROM_LOAD( "ent-101.01a",  0x08000, 0x8000, CRC(3825ac47) SHA1(d0da047c4d30a26f496b3663cfda77c229279be8) ) // This set verified as all found on the same question board
	ROM_LOAD( "ent-101.02a",  0x18000, 0x8000, CRC(a0153407) SHA1(e669957a5d4775bfa2c16960a2a909a3505c078b) )
	ROM_LOAD( "ent-101.03a",  0x28000, 0x8000, CRC(755b16ab) SHA1(277ea4110479ecdb2c772299ea04f4918cf7f561) )
	ROM_LOAD( "gen-101.01a",  0x38000, 0x8000, CRC(74d14039) SHA1(54b85581d60fb535d37a051f375e687a933600ea) )
	ROM_LOAD( "gen-101.02a",  0x48000, 0x8000, CRC(b1b930d8) SHA1(57be3ee1c0adcb549088818dc7efda64508b5647) ) // These question ROMs have been found with the "trvwz3a"
	ROM_LOAD( "spo-101.01a",  0x58000, 0x8000, CRC(9dc4ba98) SHA1(4ce2bbbd7436a0ba8140879d5d8614bddbd5a8ec) )
	ROM_LOAD( "spo-101.02a",  0x68000, 0x8000, CRC(9c106ad9) SHA1(1d1a5c91152283e3937a2df17cd57b8fe04072b7) )
	ROM_LOAD( "spo-101.03a",  0x78000, 0x8000, CRC(3d69c3a3) SHA1(9f16d45660f3cb15e44e9fc0d940a7b2b12819e8) )
	ROM_LOAD( "merit2_4.0",   0x88000, 0x8000, CRC(069b59a3) SHA1(e3d75edd3a9271df73bf51f409f066547025abbe) ) // alt Sex Trivia - Need correct ROM label
	ROM_LOAD( "merit2_4.1",   0x98000, 0x8000, CRC(938d319f) SHA1(5b5841692666c31f2c09cb318f7e106942fffea7) ) // alt Sex Trivia - Need correct ROM label

	ROM_REGION( 0x0100, "prom", 0 ) // BPROM on Question ROM board used as KEY to decode questions
	ROM_LOAD( "sc-002", 0x00000, 0x0100, CRC(94a8da8a) SHA1(8bdaee436481418425c36de24477c96ec0787916) ) // N82S129N BPROM
ROM_END

ROM_START( trvwz3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6221-04_u5-0d.u5", 0x0000, 0x2000, CRC(e0a07f06) SHA1(02cde0fc4a62d108ecd3e2f7704b9166c31707f2) ) // labeled as 6221-04 U5D, internally shows 6221-05
	ROM_LOAD( "6221-04_u6-0d.u6", 0x2000, 0x2000, CRC(223482d6) SHA1(4d9dbce7505b98ccd8e2b55f6f86a59b213d72a1) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "triv_1_u39.u39", 0x0000, 0x2000, CRC(f8a5f5fb) SHA1(a511e1a2b5e887ef00dc919e9e664ccec2d36cfa) )
	ROM_LOAD( "triv_1_u38.u38", 0x2000, 0x2000, CRC(27621e52) SHA1(a7e88d329e2e774fef9bd8c5cefb4d8f1cfcba4c) )
	ROM_LOAD( "triv_1_u37.u37", 0x4000, 0x2000, CRC(f739b5dc) SHA1(fbf469b7f4cab50e06ec2def9344e3b9801a275e) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "triv_1_u40b.u40", 0x0000, 0x2000, CRC(e829473f) SHA1(ba754d9377d955b409970494e1a14dbe1d359ee5) )

	ROM_REGION( 0xa0000, "questions", ROMREGION_ERASEFF )
	ROM_LOAD( "spo-002_01", 0x08000, 0x8000, CRC(974dca96) SHA1(eb4a745c84307a1bbb220659877f97c28cd515ac) ) // This set verified as all found on the same question board
	ROM_LOAD( "spo-002_02", 0x18000, 0x8000, CRC(e15ef8d0) SHA1(51c946311ffe507aa9031044bc34e5ae8d3473ab) )
	ROM_LOAD( "spo-002_03", 0x28000, 0x8000, CRC(503115a1) SHA1(5e6630191465b3d2a590fab08b4f47f7408ecc44) )
	ROM_LOAD( "ent-002_01", 0x38000, 0x8000, CRC(0e4fe73d) SHA1(9aee22a5837637ec5e360b72e71555942df1d26f) )
	ROM_LOAD( "ent-002_02", 0x48000, 0x8000, CRC(f56c0935) SHA1(8e16133ad90829bbc0e0f2e9ee9c26e9d0c5057e) )
	ROM_LOAD( "ent-002_03", 0x58000, 0x8000, CRC(057f6676) SHA1(a93a7a76fc8b8263568a50b00a57f3abe76c9aa3) )
	ROM_LOAD( "gen-002_01", 0x68000, 0x8000, CRC(1fa46b86) SHA1(16d54d0932fe342399faf303eafa3c0b7ba2e202) )
	ROM_LOAD( "gen-002_02", 0x78000, 0x8000, CRC(b395cd97) SHA1(a42c7c1687eaba64a725888cd6413568cc90b010) )
	ROM_LOAD( "sex-002_03", 0x88000, 0x8000, CRC(2f37dcb0) SHA1(e96eeabfa62c0a56c2f888cf1abdfdcb059572c6) ) // Shows in game as SEX TRIVIA III
	ROM_LOAD( "sex-002_04", 0x98000, 0x8000, CRC(20bf245e) SHA1(1286fd2eb51c6125a7560da3e2390ec51b64fb43) ) // Shows in game as SEX TRIVIA III

	ROM_REGION( 0x0100, "prom", 0 ) // BPROM on Question ROM board used as KEY to decode questions
	ROM_LOAD( "sc-002", 0x00000, 0x0100, CRC(94a8da8a) SHA1(8bdaee436481418425c36de24477c96ec0787916) ) // N82S129N BPROM

	ROM_REGION( 0x104, "pld", 0 )
	ROM_LOAD( "dec002.u13", 0x00000, 0x0104, CRC(651cd281) SHA1(aae1b9afcfa2837386afa322592dcfb914aea59d) ) // PAL16L8ANC - unprotected
ROM_END

ROM_START( trvwz3a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6221-04_u5-0c.u5", 0x0000, 0x2000, CRC(2b8ccf76) SHA1(ad51a04b04734fd70b7f9043cb6e99429780a62b) ) // labeled as 6221-04 U5D, internally shows 6221-05
	ROM_LOAD( "6221-04_u6-0b.u6", 0x2000, 0x2000, CRC(fd00ffec) SHA1(a3a31a689847d4abc3a4de093c1288f0e9e426f4) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "triv_1_u39.u39", 0x0000, 0x2000, CRC(f8a5f5fb) SHA1(a511e1a2b5e887ef00dc919e9e664ccec2d36cfa) )
	ROM_LOAD( "triv_1_u38.u38", 0x2000, 0x2000, CRC(27621e52) SHA1(a7e88d329e2e774fef9bd8c5cefb4d8f1cfcba4c) )
	ROM_LOAD( "triv_1_u37.u37", 0x4000, 0x2000, CRC(f739b5dc) SHA1(fbf469b7f4cab50e06ec2def9344e3b9801a275e) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "triv_1_u40b.u40", 0x0000, 0x2000, CRC(e829473f) SHA1(ba754d9377d955b409970494e1a14dbe1d359ee5) )

	ROM_REGION( 0xa0000, "questions", ROMREGION_ERASEFF )
	ROM_LOAD( "spo-002_01", 0x08000, 0x8000, CRC(974dca96) SHA1(eb4a745c84307a1bbb220659877f97c28cd515ac) ) // This set verified as all found on the same question board
	ROM_LOAD( "spo-002_02", 0x18000, 0x8000, CRC(e15ef8d0) SHA1(51c946311ffe507aa9031044bc34e5ae8d3473ab) )
	ROM_LOAD( "spo-002_03", 0x28000, 0x8000, CRC(503115a1) SHA1(5e6630191465b3d2a590fab08b4f47f7408ecc44) )
	ROM_LOAD( "ent-002_01", 0x38000, 0x8000, CRC(0e4fe73d) SHA1(9aee22a5837637ec5e360b72e71555942df1d26f) )
	ROM_LOAD( "ent-002_02", 0x48000, 0x8000, CRC(f56c0935) SHA1(8e16133ad90829bbc0e0f2e9ee9c26e9d0c5057e) )
	ROM_LOAD( "ent-002_03", 0x58000, 0x8000, CRC(057f6676) SHA1(a93a7a76fc8b8263568a50b00a57f3abe76c9aa3) )
	ROM_LOAD( "gen-002_01", 0x68000, 0x8000, CRC(1fa46b86) SHA1(16d54d0932fe342399faf303eafa3c0b7ba2e202) )
	ROM_LOAD( "gen-002_02", 0x78000, 0x8000, CRC(b395cd97) SHA1(a42c7c1687eaba64a725888cd6413568cc90b010) )
	ROM_LOAD( "sex-002_03", 0x88000, 0x8000, CRC(2f37dcb0) SHA1(e96eeabfa62c0a56c2f888cf1abdfdcb059572c6) ) // Shows in game as SEX TRIVIA III
	ROM_LOAD( "sex-002_04", 0x98000, 0x8000, CRC(20bf245e) SHA1(1286fd2eb51c6125a7560da3e2390ec51b64fb43) ) // Shows in game as SEX TRIVIA III

	ROM_REGION( 0x0100, "prom", 0 ) // BPROM on Question ROM board used as KEY to decode questions
	ROM_LOAD( "sc-002", 0x00000, 0x0100, CRC(94a8da8a) SHA1(8bdaee436481418425c36de24477c96ec0787916) ) // N82S129N BPROM

	ROM_REGION( 0x104, "pld", 0 )
	ROM_LOAD( "dec002.u13", 0x00000, 0x0104, CRC(651cd281) SHA1(aae1b9afcfa2837386afa322592dcfb914aea59d) ) // PAL16L8ANC - unprotected
ROM_END

ROM_START( trvwz3b )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u5", 0x0000, 0x2000, CRC(ad4ab519) SHA1(80e99f4e5542115e34074b41bbc69906e01a408f) ) // Unknown revision, should be labeled as 6221-04 U5-0x, internally shows 6221-05
	ROM_LOAD( "u6", 0x2000, 0x2000, CRC(21a44014) SHA1(331f8b4fa3f837de070b68b959c818122aedc68a) ) // Unknown revision, should be labeled as 6221-04 U6-0x

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "triv_1_u39.u39", 0x0000, 0x2000, CRC(f8a5f5fb) SHA1(a511e1a2b5e887ef00dc919e9e664ccec2d36cfa) )
	ROM_LOAD( "triv_1_u38.u38", 0x2000, 0x2000, CRC(27621e52) SHA1(a7e88d329e2e774fef9bd8c5cefb4d8f1cfcba4c) )
	ROM_LOAD( "triv_1_u37.u37", 0x4000, 0x2000, CRC(f739b5dc) SHA1(fbf469b7f4cab50e06ec2def9344e3b9801a275e) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "triv_1_u40b.u40", 0x0000, 0x2000, CRC(e829473f) SHA1(ba754d9377d955b409970494e1a14dbe1d359ee5) )

	ROM_REGION( 0xa0000, "questions", ROMREGION_ERASEFF )
	ROM_LOAD( "spo-002_01", 0x08000, 0x8000, CRC(974dca96) SHA1(eb4a745c84307a1bbb220659877f97c28cd515ac) )
	ROM_LOAD( "spo-002_02", 0x18000, 0x8000, CRC(e15ef8d0) SHA1(51c946311ffe507aa9031044bc34e5ae8d3473ab) )
	ROM_LOAD( "spo-002_03", 0x28000, 0x8000, CRC(503115a1) SHA1(5e6630191465b3d2a590fab08b4f47f7408ecc44) )
	ROM_LOAD( "ent-002_01", 0x38000, 0x8000, CRC(0e4fe73d) SHA1(9aee22a5837637ec5e360b72e71555942df1d26f) )
	ROM_LOAD( "ent-002_02", 0x48000, 0x8000, CRC(f56c0935) SHA1(8e16133ad90829bbc0e0f2e9ee9c26e9d0c5057e) )
	ROM_LOAD( "ent-002_03", 0x58000, 0x8000, CRC(057f6676) SHA1(a93a7a76fc8b8263568a50b00a57f3abe76c9aa3) )
	ROM_LOAD( "gen-002_01", 0x68000, 0x8000, CRC(1fa46b86) SHA1(16d54d0932fe342399faf303eafa3c0b7ba2e202) )
	ROM_LOAD( "gen-002_02", 0x78000, 0x8000, CRC(b395cd97) SHA1(a42c7c1687eaba64a725888cd6413568cc90b010) )
	ROM_LOAD( "sex-001_01", 0x88000, 0x8000, CRC(77a2a734) SHA1(7ba662d275b7914c9dcc9532116086e091e6cf88) )
	ROM_LOAD( "sex-001_02", 0x98000, 0x8000, CRC(b064876b) SHA1(588300fb6603f334de41a9685b1fcf8c642b5c16) )

	ROM_REGION( 0x0100, "prom", 0 ) // BPROM on Question ROM board used as KEY to decode questions
	ROM_LOAD( "sc-002", 0x00000, 0x0100, CRC(94a8da8a) SHA1(8bdaee436481418425c36de24477c96ec0787916) ) // N82S129N BPROM

	ROM_REGION( 0x104, "pld", 0 )
	ROM_LOAD( "dec002.u13", 0x00000, 0x0104, CRC(651cd281) SHA1(aae1b9afcfa2837386afa322592dcfb914aea59d) ) // PAL16L8ANC - unprotected
ROM_END

ROM_START( trvwz3v ) // Same program ROMs as trvwz2 sets
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6221-05_u5-0e.u5", 0x0000, 0x2000, CRC(97b8d320) SHA1(573945531113d8aae9418ba1e9a2063052227029) ) // internally shows 6221-05V
	ROM_LOAD( "6221-05_u6-0e.u6", 0x2000, 0x2000, CRC(2e86288d) SHA1(62c7024d8dfebed9bb05ea91302efe5d18cb7d2a) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "trvs_u39.u39", 0x0000, 0x2000, CRC(b9d9a80e) SHA1(55b6a0d09f8619df93ba936e083835c859a557df) )
	ROM_LOAD( "trvs_u38.u38", 0x2000, 0x2000, CRC(8348083e) SHA1(260a4c1ae043e7ceac65a8818c23940d32275879) )
	ROM_LOAD( "trvs_u37.u37", 0x4000, 0x2000, CRC(b4d3c9f4) SHA1(dda99549306519c147d275d8c6af672e80a96b67) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "trvs_u40a.u40", 0x0000, 0x2000, CRC(fbfae092) SHA1(b8569819952a5c805f11b6854d64b3ae9c857f97) )

	ROM_REGION( 0xa0000, "questions", ROMREGION_ERASEFF )
	ROM_LOAD( "spo-002_01", 0x08000, 0x8000, CRC(974dca96) SHA1(eb4a745c84307a1bbb220659877f97c28cd515ac) )
	ROM_LOAD( "spo-002_02", 0x18000, 0x8000, CRC(e15ef8d0) SHA1(51c946311ffe507aa9031044bc34e5ae8d3473ab) )
	ROM_LOAD( "spo-002_03", 0x28000, 0x8000, CRC(503115a1) SHA1(5e6630191465b3d2a590fab08b4f47f7408ecc44) )
	ROM_LOAD( "ent-002_01", 0x38000, 0x8000, CRC(0e4fe73d) SHA1(9aee22a5837637ec5e360b72e71555942df1d26f) )
	ROM_LOAD( "ent-002_02", 0x48000, 0x8000, CRC(f56c0935) SHA1(8e16133ad90829bbc0e0f2e9ee9c26e9d0c5057e) )
	ROM_LOAD( "ent-002_03", 0x58000, 0x8000, CRC(057f6676) SHA1(a93a7a76fc8b8263568a50b00a57f3abe76c9aa3) )
	ROM_LOAD( "gen-002_01", 0x68000, 0x8000, CRC(1fa46b86) SHA1(16d54d0932fe342399faf303eafa3c0b7ba2e202) )
	ROM_LOAD( "gen-002_02", 0x78000, 0x8000, CRC(b395cd97) SHA1(a42c7c1687eaba64a725888cd6413568cc90b010) )
	ROM_LOAD( "sex-002_01", 0x88000, 0x8000, CRC(15d16703) SHA1(9184f63669e9ec93e88276777e1b7f209543c3e3) )
	ROM_LOAD( "sex-002_02", 0x98000, 0x8000, CRC(647f3394) SHA1(636647ae620fd2f985b82e3516451e3bffd44040) )

	ROM_REGION( 0x0100, "prom", 0 ) // BPROM on Question ROM board used as KEY to decode questions
	ROM_LOAD( "sc-002", 0x00000, 0x0100, CRC(94a8da8a) SHA1(8bdaee436481418425c36de24477c96ec0787916) ) // N82S129N BPROM
ROM_END

ROM_START( trvwz4 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6221-10_u5-0a.u5", 0x0000, 0x8000, CRC(18425486) SHA1(53a223790f32c39abc098f58b42753844b628d54) ) // 6221-10 U5-0A 01/13/86

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "triv_1_u39.u39", 0x0000, 0x2000, CRC(f8a5f5fb) SHA1(a511e1a2b5e887ef00dc919e9e664ccec2d36cfa) )
	ROM_LOAD( "triv_1_u38.u38", 0x2000, 0x2000, CRC(27621e52) SHA1(a7e88d329e2e774fef9bd8c5cefb4d8f1cfcba4c) )
	ROM_LOAD( "triv_1_u37.u37", 0x4000, 0x2000, CRC(f739b5dc) SHA1(fbf469b7f4cab50e06ec2def9344e3b9801a275e) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "triv_1_u40e.u40", 0x0000, 0x2000, CRC(0430c239) SHA1(058b936789526b2a366ad87105703059ce2f3b48) ) // hand written E over D

	ROM_REGION( 0xa0000, "questions", ROMREGION_ERASEFF )
	ROM_LOAD( "tw4-05_spo-1", 0x08000, 0x8000, CRC(5fe0c6a3) SHA1(17bdb5262ce4edf5f022f075537f6161e1397b46) )
	ROM_LOAD( "tw4-05_spo-2", 0x18000, 0x8000, CRC(3f3390e0) SHA1(50bd7b79268438584bb0f497ab0055b4d4864590) )
	ROM_LOAD( "tw4-05_ent-1", 0x28000, 0x8000, CRC(1b317149) SHA1(94e882e9cc041ac8f292136c1ce2d21340ac5e7f) )
	ROM_LOAD( "tw4-05_ent-2", 0x38000, 0x8000, CRC(43d51697) SHA1(7af3f16f9519184ae63d8818bbc52a2ba897f275) )
	ROM_LOAD( "tw4-05_sbt-1", 0x48000, 0x8000, CRC(f1560804) SHA1(2ef0d587fbedfc342a12e913fa3c94eb8d67e2c5) )
	ROM_LOAD( "tw4-05_sbt-2", 0x58000, 0x8000, CRC(b0d6f6b2) SHA1(b08622d3775d1bb40c3b07ef932f3db4166ee284) )
	ROM_LOAD( "tw4-05_rnp-1", 0x68000, 0x8000, CRC(fee2d0b0) SHA1(9c9abec4ce693fc2d3976f3d499213c2ce67c197) )
	ROM_LOAD( "tw4-05_rnp-2", 0x78000, 0x8000, CRC(e54fc4bc) SHA1(4607974ed2bf83c475396fc1cbb1e09ad084ace8) )
	ROM_LOAD( "tw4-05_sex-1", 0x88000, 0x8000, CRC(976352b0) SHA1(5f89caca410704ba8a90da3167ba18e45fb21d43) )
	ROM_LOAD( "tw4-05_sex-2", 0x98000, 0x8000, CRC(5f148bc9) SHA1(2fd2cf819c2f395dcffad59857b3533fe3cce60b) )

	ROM_REGION( 0x0100, "prom", 0 ) // BPROM on Question ROM board used as KEY to decode questions
	ROM_LOAD( "sc-005", 0x00000, 0x0100, CRC(288ba0bd) SHA1(64868d80eca246b81da784441b3706c372c4e0f7) ) // 74S287 (==N82S129N) BPROM

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "dec003.u13", 0x000, 0x117, CRC(5b9a2fec) SHA1(c56c7bbe13028903cfc82440ee8b24df855134c2) ) // PAL16L8ANC - brute forced
ROM_END

ROM_START( trvwz4v )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6221-13_u5-0b.u5", 0x0000, 0x8000, CRC(bc23a1ab) SHA1(b9601f316e373c568c5b208de417617094046559) ) // 6221-13 U5-0B 03/17/86

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "trvs_u39.u39", 0x0000, 0x2000, CRC(b9d9a80e) SHA1(55b6a0d09f8619df93ba936e083835c859a557df) )
	ROM_LOAD( "trvs_u38.u38", 0x2000, 0x2000, CRC(8348083e) SHA1(260a4c1ae043e7ceac65a8818c23940d32275879) )
	ROM_LOAD( "trvs_u37.u37", 0x4000, 0x2000, CRC(b4d3c9f4) SHA1(dda99549306519c147d275d8c6af672e80a96b67) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "trvs_u40a.u40", 0x0000, 0x2000, CRC(fbfae092) SHA1(b8569819952a5c805f11b6854d64b3ae9c857f97) )

	ROM_REGION( 0xa0000, "questions", ROMREGION_ERASEFF )
	ROM_LOAD( "tw4-05_spo-1", 0x08000, 0x8000, CRC(5fe0c6a3) SHA1(17bdb5262ce4edf5f022f075537f6161e1397b46) )
	ROM_LOAD( "tw4-05_spo-2", 0x18000, 0x8000, CRC(3f3390e0) SHA1(50bd7b79268438584bb0f497ab0055b4d4864590) )
	ROM_LOAD( "tw4-05_ent-1", 0x28000, 0x8000, CRC(1b317149) SHA1(94e882e9cc041ac8f292136c1ce2d21340ac5e7f) )
	ROM_LOAD( "tw4-05_ent-2", 0x38000, 0x8000, CRC(43d51697) SHA1(7af3f16f9519184ae63d8818bbc52a2ba897f275) )
	ROM_LOAD( "tw4-05_sbt-1", 0x48000, 0x8000, CRC(f1560804) SHA1(2ef0d587fbedfc342a12e913fa3c94eb8d67e2c5) )
	ROM_LOAD( "tw4-05_sbt-2", 0x58000, 0x8000, CRC(b0d6f6b2) SHA1(b08622d3775d1bb40c3b07ef932f3db4166ee284) )
	ROM_LOAD( "tw4-05_rnp-1", 0x68000, 0x8000, CRC(fee2d0b0) SHA1(9c9abec4ce693fc2d3976f3d499213c2ce67c197) )
	ROM_LOAD( "tw4-05_rnp-2", 0x78000, 0x8000, CRC(e54fc4bc) SHA1(4607974ed2bf83c475396fc1cbb1e09ad084ace8) )
	ROM_LOAD( "tw4-05_sex-1", 0x88000, 0x8000, CRC(976352b0) SHA1(5f89caca410704ba8a90da3167ba18e45fb21d43) )
	ROM_LOAD( "tw4-05_sex-2", 0x98000, 0x8000, CRC(5f148bc9) SHA1(2fd2cf819c2f395dcffad59857b3533fe3cce60b) )

	ROM_REGION( 0x0100, "prom", 0 ) // BPROM on Question ROM board used as KEY to decode questions
	ROM_LOAD( "sc-005", 0x00000, 0x0100, CRC(288ba0bd) SHA1(64868d80eca246b81da784441b3706c372c4e0f7) ) // 74S287 (==N82S129N) BPROM

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "dec003.u13", 0x000, 0x117, CRC(5b9a2fec) SHA1(c56c7bbe13028903cfc82440ee8b24df855134c2) ) // PAL16L8ANC - brute forced
ROM_END

/* only the question board was dumped, but it contained a selection
  of ROMs from the above 'trvwz4' set, and one additional one which is Sex Trivia III
*/
ROM_START( trvwz4va )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6221-13_u5-0b.u5", 0x0000, 0x8000, CRC(bc23a1ab) SHA1(b9601f316e373c568c5b208de417617094046559) ) // 6221-13 U5-0B 03/17/86

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "trvs_u39.u39", 0x0000, 0x2000, CRC(b9d9a80e) SHA1(55b6a0d09f8619df93ba936e083835c859a557df) )
	ROM_LOAD( "trvs_u38.u38", 0x2000, 0x2000, CRC(8348083e) SHA1(260a4c1ae043e7ceac65a8818c23940d32275879) )
	ROM_LOAD( "trvs_u37.u37", 0x4000, 0x2000, CRC(b4d3c9f4) SHA1(dda99549306519c147d275d8c6af672e80a96b67) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "trvs_u40a.u40", 0x0000, 0x2000, CRC(fbfae092) SHA1(b8569819952a5c805f11b6854d64b3ae9c857f97) )

	ROM_REGION( 0xa0000, "questions", ROMREGION_ERASEFF )
	ROM_LOAD( "tw4-05_spo-1", 0x08000, 0x8000, CRC(5fe0c6a3) SHA1(17bdb5262ce4edf5f022f075537f6161e1397b46) )
	ROM_LOAD( "tw4-05_spo-2", 0x18000, 0x8000, CRC(3f3390e0) SHA1(50bd7b79268438584bb0f497ab0055b4d4864590) )
	ROM_LOAD( "tw4-05_ent-1", 0x28000, 0x8000, CRC(1b317149) SHA1(94e882e9cc041ac8f292136c1ce2d21340ac5e7f) )
	ROM_LOAD( "tw4-05_ent-2", 0x38000, 0x8000, CRC(43d51697) SHA1(7af3f16f9519184ae63d8818bbc52a2ba897f275) )
	ROM_LOAD( "tw4-05_sbt-1", 0x48000, 0x8000, CRC(f1560804) SHA1(2ef0d587fbedfc342a12e913fa3c94eb8d67e2c5) )
	ROM_LOAD( "tw4-05_sbt-2", 0x58000, 0x8000, CRC(b0d6f6b2) SHA1(b08622d3775d1bb40c3b07ef932f3db4166ee284) )
	ROM_LOAD( "tw4-05_rnp-1", 0x68000, 0x8000, CRC(fee2d0b0) SHA1(9c9abec4ce693fc2d3976f3d499213c2ce67c197) )
	ROM_LOAD( "tw4-05_rnp-2", 0x78000, 0x8000, CRC(e54fc4bc) SHA1(4607974ed2bf83c475396fc1cbb1e09ad084ace8) )
	ROM_LOAD( "merit2_5.0",   0x88000, 0x8000, CRC(e07d139f) SHA1(e364dcc628719c1bcdc119bdb2f3c98b5538c411) ) // sex trivia III - Need correct ROM label

	ROM_REGION( 0x0100, "prom", 0 ) // BPROM on Question ROM board used as KEY to decode questions
	ROM_LOAD( "sc-005", 0x00000, 0x0100, CRC(288ba0bd) SHA1(64868d80eca246b81da784441b3706c372c4e0f7) ) // 74S287 (==N82S129N) BPROM

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "dec003.u13", 0x000, 0x117, CRC(5b9a2fec) SHA1(c56c7bbe13028903cfc82440ee8b24df855134c2) ) // PAL16L8ANC - brute forced
ROM_END

ROM_START( dtrvwz5 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6221-70_u5-0a.u5", 0x0000, 0x8000, CRC(e5917a71) SHA1(2acebe337600cd490da1c6fb2d83e2e378e584f1) ) // 6221-70 U5-0A 04/15/87

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "trv3.u39", 0x0000, 0x2000, CRC(81a34357) SHA1(87ae9db78f043dbdcd1d50473fc09284eceaf884) )
	ROM_LOAD( "trv3.u38", 0x2000, 0x2000, CRC(c4082020) SHA1(744dc8745f3d54754184571b64664ee5c1497fb4) )
	ROM_LOAD( "trv3.u37", 0x4000, 0x2000, CRC(5e5e6fb3) SHA1(c182233367de6c9cda0e49a5958bb07460a5f300) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "trv3.u40", 0x0000, 0x2000, CRC(a2c934f2) SHA1(214cc1f47c11618457a7885712585c977107cab7) )

	ROM_REGION( 0xa0000, "questions", ROMREGION_ERASEFF )
	ROM_LOAD( "tw5-06_fif-1.1",  0x08000, 0x8000, CRC(300f245c) SHA1(9c380000ba5a6c826025e32f0e46932e234b46bc) )
	ROM_LOAD( "tw5-06_fif-2.2",  0x18000, 0x8000, CRC(99ee9cbe) SHA1(d6a4a604a070436b0acb1c774687f6c2266c8807) )
	ROM_LOAD( "tw5-06_six-1.3",  0x28000, 0x8000, CRC(87354939) SHA1(6e3de6df944da75e28d36dce3cca9b45a8936bf4) )
	ROM_LOAD( "tw5-06_six-2.4",  0x38000, 0x8000, CRC(ea8ed7ae) SHA1(2c084a88773e6f611a6cc6d847b9d74f5c8bfc77) )
	ROM_LOAD( "tw5-06_sev-1.5",  0x48000, 0x8000, CRC(fd5099aa) SHA1(81e978597aa348c77001f72763744491cfdad1d1) )
	ROM_LOAD( "tw5-06_sev-2.6",  0x58000, 0x8000, CRC(523520c8) SHA1(7dff9cda1ade5d3b4e573e77b7ec93ee8ae13c86) )
	ROM_LOAD( "tw5-06_eig-1.7",  0x68000, 0x8000, CRC(3a2a4562) SHA1(45565622d7057047b02050dcd34ff6f02663507d) )
	ROM_LOAD( "tw5-06_eig-2.8",  0x78000, 0x8000, CRC(cb7e9035) SHA1(d3344fb318f2241c07933c4b8e3525c219ea3aa6) )
	ROM_LOAD( "tw5-06_sx5-1.9",  0x88000, 0x8000, CRC(6ae2a208) SHA1(3cc935e616c247c6885319acc6a6ca92ee6fc3c0) )
	ROM_LOAD( "tw5-06_sx5-2.10", 0x98000, 0x8000, CRC(790184fc) SHA1(9c8b56852b31d3312f26a5901487f6b31d9e9b4f) )

	ROM_REGION( 0x0100, "prom", 0 ) // BPROM on Question ROM board used as KEY to decode questions
	ROM_LOAD( "sc-006", 0x00000, 0x0100, CRC(145f7f61) SHA1(f6967466791895710107987e9438177706d7b2a0) ) // 74S287 (==N82S129N) BPROM

	ROM_REGION( 0x0800, "crt209", 0 ) // contains Z80 program code to read inputs
	ROM_LOAD( "crt-209_6221-70.cpu", 0x0000, 0x0800, BAD_DUMP CRC(9f78d976) SHA1(098651945074c9a21ac72b1d73f0c895f67e9c4e) ) // didn't give consistent reads, byte at 0x70 hand-fixed

	ROM_REGION( 0x117, "plds", 0 ) // PAL inside CRT-209 module
	ROM_LOAD( "crt-209_pal16l8.bin", 0x000, 0x117, CRC(e916c56f) SHA1(1517091ff1791d923e5bd62d18d1428b6a3a8c72) ) // SC3339 20-pin 16L8 type PAL
ROM_END

ROM_START( dtrvwz5v )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6221-75_u5-0.u5", 0x0000, 0x8000, CRC(c8b27616) SHA1(8c6bd4aff45c1db04f3894686b053e9e39476a5e) ) // 6221-75 U5-0  03/23/87

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "trvs_u39.u39", 0x0000, 0x2000, CRC(b9d9a80e) SHA1(55b6a0d09f8619df93ba936e083835c859a557df) )
	ROM_LOAD( "trvs_u38.u38", 0x2000, 0x2000, CRC(8348083e) SHA1(260a4c1ae043e7ceac65a8818c23940d32275879) )
	ROM_LOAD( "trvs_u37.u37", 0x4000, 0x2000, CRC(b4d3c9f4) SHA1(dda99549306519c147d275d8c6af672e80a96b67) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "trvs_u40c.u40", 0x0000, 0x2000, CRC(0b8cdaed) SHA1(dbb868428b1117dc0a0d7c7a8b7224e1aa8b9a06) )

	ROM_REGION( 0xa0000, "questions", ROMREGION_ERASEFF )
	ROM_LOAD( "tw5-06_fif-1.1",  0x08000, 0x8000, CRC(300f245c) SHA1(9c380000ba5a6c826025e32f0e46932e234b46bc) )
	ROM_LOAD( "tw5-06_fif-2.2",  0x18000, 0x8000, CRC(99ee9cbe) SHA1(d6a4a604a070436b0acb1c774687f6c2266c8807) )
	ROM_LOAD( "tw5-06_six-1.3",  0x28000, 0x8000, CRC(87354939) SHA1(6e3de6df944da75e28d36dce3cca9b45a8936bf4) )
	ROM_LOAD( "tw5-06_six-2.4",  0x38000, 0x8000, CRC(ea8ed7ae) SHA1(2c084a88773e6f611a6cc6d847b9d74f5c8bfc77) )
	ROM_LOAD( "tw5-06_sev-1.5",  0x48000, 0x8000, CRC(fd5099aa) SHA1(81e978597aa348c77001f72763744491cfdad1d1) )
	ROM_LOAD( "tw5-06_sev-2.6",  0x58000, 0x8000, CRC(523520c8) SHA1(7dff9cda1ade5d3b4e573e77b7ec93ee8ae13c86) )
	ROM_LOAD( "tw5-06_eig-1.7",  0x68000, 0x8000, CRC(3a2a4562) SHA1(45565622d7057047b02050dcd34ff6f02663507d) )
	ROM_LOAD( "tw5-06_eig-2.8",  0x78000, 0x8000, CRC(cb7e9035) SHA1(d3344fb318f2241c07933c4b8e3525c219ea3aa6) )
	ROM_LOAD( "tw5-06_sx5-1.9",  0x88000, 0x8000, CRC(6ae2a208) SHA1(3cc935e616c247c6885319acc6a6ca92ee6fc3c0) )
	ROM_LOAD( "tw5-06_sx5-2.10", 0x98000, 0x8000, CRC(790184fc) SHA1(9c8b56852b31d3312f26a5901487f6b31d9e9b4f) )

	ROM_REGION( 0x0100, "prom", 0 ) // BPROM on Question ROM board used as KEY to decode questions
	ROM_LOAD( "sc-006", 0x00000, 0x0100, CRC(145f7f61) SHA1(f6967466791895710107987e9438177706d7b2a0) ) // 74S287 (==N82S129N) BPROM

	ROM_REGION( 0x0800, "crt209", 0 ) // contains Z80 program code to read inputs
	ROM_LOAD( "crt-209_6221-75.cpu", 0x0000, 0x0800, BAD_DUMP CRC(9f78d976) SHA1(098651945074c9a21ac72b1d73f0c895f67e9c4e) ) // didn't give consistent reads, byte at 0x70 hand-fixed

	ROM_REGION( 0x117, "plds", 0 ) // PAL inside CRT-209 module
	ROM_LOAD( "crt-209_pal16l8.bin", 0x000, 0x117, CRC(e916c56f) SHA1(1517091ff1791d923e5bd62d18d1428b6a3a8c72) ) // SC3339 20-pin 16L8 type PAL
ROM_END

/*

crt200 rev e-1
1985 merit industries

u39   u37                      z80b
u40   u38
           6845                 u5

      2016                      pb
      2016                      6264

                        ay3-8912
                        8255
                        8255


crt205a

pba pb9 pb8 pb7 pb6 pb5 pb4 pb3 pb2 pb1

*/

ROM_START( tictac ) // verfied by 2 separate PCB sets
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6221-23_u5-0c.u5", 0x00000, 0x8000, CRC(548e5857) SHA1(be1996bbb8e0ed8189dae1e6b9fd105db209de24) ) // 6221-23 U5-0C 07/07/86

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "ttt1_u39.u39", 0x00000, 0x2000, CRC(dd79e824) SHA1(d65ee1c758293ddf8a5f4913878a2867ba526e68) )
	ROM_LOAD( "ttt1_u38.u38", 0x02000, 0x2000, CRC(e1bf0fab) SHA1(291261ea817c42d6e8a19c17a2d3706fed7d78c4) )
	ROM_LOAD( "ttt1_u37.u37", 0x04000, 0x2000, CRC(94f9c7f8) SHA1(494389983fb62fe2d772c276e659b6b20c531933) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "ttt1_u40b.u40",    0x00000, 0x2000, CRC(ab0088eb) SHA1(23a05a4dc11a8497f4fc7e4a76085af15ff89cea) )

	ROM_REGION( 0xa0000, "questions", ROMREGION_ERASEFF ) // question ROMs had no labels - verified by 2 separate PCB sets
	ROM_LOAD( "01.1", 0x08000, 0x8000, CRC(ed465fad) SHA1(db190ee924071372a108deb33e17cbc7572a55ba) ) // Trivia categories are:
	ROM_LOAD( "02.2", 0x18000, 0x8000, CRC(ff8a9c6a) SHA1(de9ecbb48aa82ac53e4adce7dbcfc75821a69aa6) ) // Sex IV, Trivia Twist, T.V./Movies/Music, Sports Quiz & Around The World
	ROM_LOAD( "03.3", 0x28000, 0x8000, CRC(e416dd8a) SHA1(3bfeef915c8862ec74ad2cda820daaf6daee3ee3) ) // likely labeled as: sex-008, spo-008 ect...
	ROM_LOAD( "04.4", 0x38000, 0x8000, CRC(08503d1f) SHA1(ae33d0007235b14f3c06c32f8a598a9c81d78903) )
	ROM_LOAD( "05.5", 0x48000, 0x8000, CRC(cc80e9e5) SHA1(81fb7cba3f6aad5d5a590008e3dd2a4f0aafaf3f) )
	ROM_LOAD( "06.6", 0x58000, 0x8000, CRC(460e90fc) SHA1(be627ced5b24f040ca4c43475c3bc79013a5ef97) )
	ROM_LOAD( "07.7", 0x68000, 0x8000, CRC(ef47fc5d) SHA1(78e4326049bc7a2c43046bfa6baeef8f3b21395b) )
	ROM_LOAD( "08.8", 0x78000, 0x8000, CRC(3e1cf5fa) SHA1(abb3b2ff4914a9990c33808c6d554922f7ebd174) )
	ROM_LOAD( "09.9", 0x88000, 0x8000, CRC(64c6e9f0) SHA1(58656625f985330b8bf63eefc820ada3ce8ad91f) )
	ROM_LOAD( "0a.a", 0x98000, 0x8000, CRC(acf8e187) SHA1(dad6fc6f75a98a1d1b0cd5f789be4850140877f3) )

	ROM_REGION( 0x0100, "prom", 0 ) // BPROM on Question ROM board used as KEY to decode questions
	ROM_LOAD( "sc-008", 0x00000, 0x0100, CRC(63e61cf6) SHA1(3781a2708c40399eb9f942cd6211a854482424e5) ) // 74S287 (==N82S129N) BPROM
ROM_END

ROM_START( tictaca )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6221-23_u5-0c.u5", 0x00000, 0x8000, CRC(f0dd73f5) SHA1(f2988b84255ce5f7ea6d25150cdbae88b98e1be3) ) // 6221-23 U5-0C 02/11/86

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "ttt1_u39.u39", 0x00000, 0x2000, CRC(dd79e824) SHA1(d65ee1c758293ddf8a5f4913878a2867ba526e68) )
	ROM_LOAD( "ttt1_u38.u38", 0x02000, 0x2000, CRC(e1bf0fab) SHA1(291261ea817c42d6e8a19c17a2d3706fed7d78c4) )
	ROM_LOAD( "ttt1_u37.u37", 0x04000, 0x2000, CRC(94f9c7f8) SHA1(494389983fb62fe2d772c276e659b6b20c531933) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "ttt1_u40b.u40",    0x00000, 0x2000, CRC(ab0088eb) SHA1(23a05a4dc11a8497f4fc7e4a76085af15ff89cea) )

	ROM_REGION( 0xa0000, "questions", ROMREGION_ERASEFF )
	ROM_LOAD( "spo-004_01a.1", 0x08000, 0x8000, CRC(71b398a9) SHA1(5ea07c409afd52c7d08592b30ff0ff3b72c3f8c3) ) // Trivia categories are:
	ROM_LOAD( "spo-004_02a.2", 0x18000, 0x8000, CRC(eb34672f) SHA1(c472fc4445fc434029a2740dfc1d9ab9b1ef9f87) ) // Sports, Entertainment, General Interest & Sex Trivia III
	ROM_LOAD( "spo-004_03a.3", 0x28000, 0x8000, CRC(8eea30b9) SHA1(fe1d0332106631f56bc6c57a888da9e4e63fa52f) )
	ROM_LOAD( "ent-004_01.4",  0x38000, 0x8000, CRC(3f45064d) SHA1(de109ac0b19fd1cd7f0020cc174c2da21708108c) )
	ROM_LOAD( "ent-004_02a.5", 0x48000, 0x8000, CRC(f1c446cd) SHA1(9a6f18defbb64e202ae12e1a59502b8f2d6a58a6) )
	ROM_LOAD( "ent-004_03.6",  0x58000, 0x8000, CRC(206cfc0d) SHA1(78f6b684713459a617096aa3ffe6e9e62583938c) )
	ROM_LOAD( "gen-004_01a.7", 0x68000, 0x8000, CRC(d1584173) SHA1(7a2190203f478f446cc70c473c345e7cc332e049) )
	ROM_LOAD( "gen-004_02a.8", 0x78000, 0x8000, CRC(d00ab1fd) SHA1(c94269c8a478e88f71aeca94c6f20fc05a9c62bd) )
	ROM_LOAD( "sex-004_01a.9", 0x88000, 0x8000, CRC(9333dbca) SHA1(dd87e6f69d60580fdb6f979398edbeb1a51be355) )
	ROM_LOAD( "sex-004_02a.a", 0x98000, 0x8000, CRC(6eda81f4) SHA1(6d64344691e3e52035a7d30fb3e762f0bd397db7) )

	ROM_REGION( 0x0100, "prom", 0 ) // BPROM on Question ROM board used as KEY to decode questions
	ROM_LOAD( "sc-004", 0x00000, 0x0100, NO_DUMP ) // 74S287 (==N82S129N) BPROM
ROM_END

ROM_START( tictacv )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6221-22_u5-0.u5", 0x00000, 0x8000,  CRC(c3acd686) SHA1(0c652e88675e2098be2f26e8f1acefc9e69d630f) ) // 6221-22 U5-0 12/11/85

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "ttts_u-39.u39", 0x00000, 0x2000, CRC(20103ed6) SHA1(52741ba8e3b57a32446d3bf4d6f6d8368954fd54) )
	ROM_LOAD( "ttts_u-38.u38", 0x02000, 0x2000, CRC(32e791b9) SHA1(f206aef25b3a9b7042d804e628c356f7d8d3cdbe) )
	ROM_LOAD( "ttts_u-37.u37", 0x04000, 0x2000, CRC(adf19f83) SHA1(af2c0b9782f8e93a7c5e2a5ecc937694773d8ad0) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "ttts_u-40.u40", 0x00000, 0x2000, CRC(c7071c98) SHA1(88e1b26f198cfbbd86b492356f60fc1b81b38d97) )

	ROM_REGION( 0xa0000, "questions", ROMREGION_ERASEFF )
	ROM_LOAD( "spo-004_01a.1", 0x08000, 0x8000, CRC(71b398a9) SHA1(5ea07c409afd52c7d08592b30ff0ff3b72c3f8c3) ) // Trivia categories are:
	ROM_LOAD( "spo-004_02a.2", 0x18000, 0x8000, CRC(eb34672f) SHA1(c472fc4445fc434029a2740dfc1d9ab9b1ef9f87) ) // Sports, Entertainment, General Interest & Sex Trivia III
	ROM_LOAD( "spo-004_03a.3", 0x28000, 0x8000, CRC(8eea30b9) SHA1(fe1d0332106631f56bc6c57a888da9e4e63fa52f) )
	ROM_LOAD( "ent-004_01.4",  0x38000, 0x8000, CRC(3f45064d) SHA1(de109ac0b19fd1cd7f0020cc174c2da21708108c) )
	ROM_LOAD( "ent-004_02a.5", 0x48000, 0x8000, CRC(f1c446cd) SHA1(9a6f18defbb64e202ae12e1a59502b8f2d6a58a6) )
	ROM_LOAD( "ent-004_03.6",  0x58000, 0x8000, CRC(206cfc0d) SHA1(78f6b684713459a617096aa3ffe6e9e62583938c) )
	ROM_LOAD( "gen-004_01a.7", 0x68000, 0x8000, CRC(d1584173) SHA1(7a2190203f478f446cc70c473c345e7cc332e049) )
	ROM_LOAD( "gen-004_02a.8", 0x78000, 0x8000, CRC(d00ab1fd) SHA1(c94269c8a478e88f71aeca94c6f20fc05a9c62bd) )
	ROM_LOAD( "sex-004_01a.9", 0x88000, 0x8000, CRC(9333dbca) SHA1(dd87e6f69d60580fdb6f979398edbeb1a51be355) )
	ROM_LOAD( "sex-004_02a.a", 0x98000, 0x8000, CRC(6eda81f4) SHA1(6d64344691e3e52035a7d30fb3e762f0bd397db7) )

	ROM_REGION( 0x0100, "prom", 0 ) // BPROM on Question ROM board used as KEY to decode questions
	ROM_LOAD( "sc-004", 0x00000, 0x0100, NO_DUMP ) // 74S287 (==N82S129N) BPROM
ROM_END

ROM_START( phrcraze )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "5281-40_u5-3a.u5", 0x00000, 0x8000, CRC(d04c7657) SHA1(0b59fbf553eb5b68544ee2f94cf8106ab30ff1ed) ) // 6221-40 U5-3A 100086

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "phz1_u37.u37", 0x00000, 0x8000, CRC(237e221a) SHA1(7aa69375c2b9a9e73e0e4ed207bf595368b2deb2) ) // 1st & 2nd half identical, but correct and verified
	ROM_LOAD( "phz1_u38.u38", 0x08000, 0x8000, CRC(bfa78b67) SHA1(1b51c0e00240f798fe717624e706cb15700bc2f9) )
	ROM_LOAD( "phz1_u39.u39", 0x10000, 0x8000, CRC(9ce22cb3) SHA1(b653afb8f13decd993e434aaad69a6e09ab65f83) )

	ROM_REGION( 0x08000, "gfx2", 0 )
	ROM_LOAD( "phz1_u40.u40", 0x00000, 0x8000, CRC(17dcddd4) SHA1(51682bdbfb67cd0ccf20b97e8fa12d72f0fe82ed) ) // 1st & 2nd half identical, but correct and verified

	ROM_REGION( 0xa0000, "questions", ROMREGION_ERASEFF )
	ROM_LOAD( "phrz1-07_std-1a",  0x00000, 0x8000, CRC(367f1dfa) SHA1(01d69004c365acefb0e52ac12593a3874c16ab9d) )
	ROM_LOAD( "phrz1-07_std-2a",  0x10000, 0x8000, CRC(527b3025) SHA1(36dc129d2276909643e90ae3810c8341076fd88c) )
	ROM_LOAD( "phrz1-07_std-3a",  0x20000, 0x8000, CRC(c4c7dcee) SHA1(81d879df3da0fbe1cf2247d92b3853104a99689d) )
	ROM_LOAD( "phrz1-07_std-4a",  0x30000, 0x8000, CRC(effc811b) SHA1(2479539965ed541be417bbe48a5e66a58a6294aa) )
	ROM_LOAD( "phrz1-07_std-5a",  0x40000, 0x8000, CRC(7cb395a9) SHA1(48b3ac524e6ae23f885b9b767e77930a89a81f5f) )
	ROM_LOAD( "phrz1-07_std-6a",  0x50000, 0x8000, CRC(c5980f24) SHA1(a3b665c74aaa704ffa382f95adac70c7c46fb446) )
	ROM_LOAD( "phrz1-07_std-7a",  0x60000, 0x8000, CRC(05dd3900) SHA1(bb13a3c5f84771c450fa88560cc74c5a1be1b876) )
	ROM_LOAD( "phrz1-07_std-8a",  0x70000, 0x8000, CRC(423eecd6) SHA1(ca8d181ccba05acba8ebc57f20e0542eda00c917) )
	ROM_LOAD( "phrz1-07_sex1-1a", 0x80000, 0x8000, CRC(ed7604b8) SHA1(b1e841b50b8ef6ae95fafac1c34b6d0337a05d18) )
	ROM_LOAD( "phrz1-07_sex1-2a", 0x90000, 0x8000, CRC(7ef3bca7) SHA1(f25cd01f996882a500e1a800d924759cd1de255d) )

	ROM_REGION( 0x0100, "prom", 0 ) // BPROM on Question ROM board used as KEY to decode questions
	ROM_LOAD( "sc-007", 0x00000, 0x0100, CRC(8565ff9c) SHA1(688d2426dd10cd9a9edc4b22f5b3274e1067a5f8) ) // 74S287 (==N82S129N) BPROM

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "dec006.u13", 0x000, 0x117, CRC(050b675e) SHA1(7ae0bb579055b897a9b76db07a72b9930c7cd081) ) // PAL16L8ANC - brute forced
ROM_END

ROM_START( phrcrazea )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6221-40_u5-3.u5", 0x00000, 0x8000, CRC(bd8b5612) SHA1(614436da4ed45e0d974b565c5c765bcc1b9d94b5) ) // 6221-40 U5-3 070986

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "phz1_u37.u37", 0x00000, 0x8000, CRC(237e221a) SHA1(7aa69375c2b9a9e73e0e4ed207bf595368b2deb2) ) // 1st & 2nd half identical, but correct and verified
	ROM_LOAD( "phz1_u38.u38", 0x08000, 0x8000, CRC(bfa78b67) SHA1(1b51c0e00240f798fe717624e706cb15700bc2f9) )
	ROM_LOAD( "phz1_u39.u39", 0x10000, 0x8000, CRC(9ce22cb3) SHA1(b653afb8f13decd993e434aaad69a6e09ab65f83) )

	ROM_REGION( 0x08000, "gfx2", 0 )
	ROM_LOAD( "phz1_u40.u40", 0x00000, 0x8000, CRC(17dcddd4) SHA1(51682bdbfb67cd0ccf20b97e8fa12d72f0fe82ed) ) // 1st & 2nd half identical, but correct and verified

	ROM_REGION( 0xa0000, "questions", ROMREGION_ERASEFF )
	ROM_LOAD( "phrz1-07_std-1a",  0x00000, 0x8000, CRC(367f1dfa) SHA1(01d69004c365acefb0e52ac12593a3874c16ab9d) )
	ROM_LOAD( "phrz1-07_std-2a",  0x10000, 0x8000, CRC(527b3025) SHA1(36dc129d2276909643e90ae3810c8341076fd88c) )
	ROM_LOAD( "phrz1-07_std-3a",  0x20000, 0x8000, CRC(c4c7dcee) SHA1(81d879df3da0fbe1cf2247d92b3853104a99689d) )
	ROM_LOAD( "phrz1-07_std-4a",  0x30000, 0x8000, CRC(effc811b) SHA1(2479539965ed541be417bbe48a5e66a58a6294aa) )
	ROM_LOAD( "phrz1-07_std-5a",  0x40000, 0x8000, CRC(7cb395a9) SHA1(48b3ac524e6ae23f885b9b767e77930a89a81f5f) )
	ROM_LOAD( "phrz1-07_std-6a",  0x50000, 0x8000, CRC(c5980f24) SHA1(a3b665c74aaa704ffa382f95adac70c7c46fb446) )
	ROM_LOAD( "phrz1-07_std-7a",  0x60000, 0x8000, CRC(05dd3900) SHA1(bb13a3c5f84771c450fa88560cc74c5a1be1b876) )
	ROM_LOAD( "phrz1-07_std-8a",  0x70000, 0x8000, CRC(423eecd6) SHA1(ca8d181ccba05acba8ebc57f20e0542eda00c917) )
	ROM_LOAD( "phrz1-07_sex1-1a", 0x80000, 0x8000, CRC(ed7604b8) SHA1(b1e841b50b8ef6ae95fafac1c34b6d0337a05d18) )
	ROM_LOAD( "phrz1-07_sex1-2a", 0x90000, 0x8000, CRC(7ef3bca7) SHA1(f25cd01f996882a500e1a800d924759cd1de255d) )

	ROM_REGION( 0x0100, "prom", 0 ) // BPROM on Question ROM board used as KEY to decode questions
	ROM_LOAD( "sc-007", 0x00000, 0x0100, CRC(8565ff9c) SHA1(688d2426dd10cd9a9edc4b22f5b3274e1067a5f8) ) // 74S287 (==N82S129N) BPROM

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "dec006.u13", 0x000, 0x117, CRC(050b675e) SHA1(7ae0bb579055b897a9b76db07a72b9930c7cd081) ) // PAL16L8ANC - brute forced
ROM_END

ROM_START( phrcrazeb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6221-40_u5-0a.u5", 0x00000, 0x8000, CRC(ccd33a0c) SHA1(869b66af4369f3b4bc19336ca2b8104c7f652de7) ) // 6221-40 U5-0A 041686

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "phz1_u37.u37", 0x00000, 0x8000, CRC(237e221a) SHA1(7aa69375c2b9a9e73e0e4ed207bf595368b2deb2) ) // 1st & 2nd half identical, but correct and verified
	ROM_LOAD( "phz1_u38.u38", 0x08000, 0x8000, CRC(bfa78b67) SHA1(1b51c0e00240f798fe717624e706cb15700bc2f9) )
	ROM_LOAD( "phz1_u39.u39", 0x10000, 0x8000, CRC(9ce22cb3) SHA1(b653afb8f13decd993e434aaad69a6e09ab65f83) )

	ROM_REGION( 0x08000, "gfx2", 0 )
	ROM_LOAD( "phz1_u40.u40", 0x00000, 0x8000, CRC(17dcddd4) SHA1(51682bdbfb67cd0ccf20b97e8fa12d72f0fe82ed) ) // 1st & 2nd half identical, but correct and verified

	ROM_REGION( 0xa0000, "questions", ROMREGION_ERASEFF )
	ROM_LOAD( "phrz1-07_std-1", 0x00000, 0x8000, CRC(0a016c5e) SHA1(1a24ecd7fe59b08c75a1b4575c7fe467cc7f0cf8) )
	ROM_LOAD( "phrz1-07_std-2", 0x10000, 0x8000, CRC(e67dc49e) SHA1(5265af228531dc16db7f7ee78da6e51ef9a1d772) )
	ROM_LOAD( "phrz1-07_std-3", 0x20000, 0x8000, CRC(5c79a653) SHA1(85a904465b347564e937074e2b18159604c83e51) )
	ROM_LOAD( "phrz1-07_std-4", 0x30000, 0x8000, CRC(9837f757) SHA1(01106114b6997fe6432e519101f95c83a1f7cc1e) )
	ROM_LOAD( "phrz1-07_std-5", 0x40000, 0x8000, CRC(dc9d8682) SHA1(46973da4298d0ed149c651498527c91b8ba57e0a) )
	ROM_LOAD( "phrz1-07_std-6", 0x50000, 0x8000, CRC(48e24f17) SHA1(f50c85505f6ab2360f0885494001f174224f8575) )

	ROM_REGION( 0x0100, "prom", 0 ) // BPROM on Question ROM board used as KEY to decode questions
	ROM_LOAD( "sc-007", 0x00000, 0x0100, CRC(8565ff9c) SHA1(688d2426dd10cd9a9edc4b22f5b3274e1067a5f8) ) // 74S287 (==N82S129N) BPROM

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "dec006.u13", 0x000, 0x117, CRC(050b675e) SHA1(7ae0bb579055b897a9b76db07a72b9930c7cd081) ) // PAL16L8ANC - brute forced
ROM_END

ROM_START( phrcrazec )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6221-40_u5-0.u5", 0x00000, 0x8000, CRC(f9642d0a) SHA1(6e9b9929bc28f6c26c70a8b762a2755dc097dbc4) ) // 6221-40 U5-0 040386

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "phz1_u37.u37", 0x00000, 0x8000, CRC(237e221a) SHA1(7aa69375c2b9a9e73e0e4ed207bf595368b2deb2) ) // 1st & 2nd half identical, but correct and verified
	ROM_LOAD( "phz1_u38.u38", 0x08000, 0x8000, CRC(bfa78b67) SHA1(1b51c0e00240f798fe717624e706cb15700bc2f9) )
	ROM_LOAD( "phz1_u39.u39", 0x10000, 0x8000, CRC(9ce22cb3) SHA1(b653afb8f13decd993e434aaad69a6e09ab65f83) )

	ROM_REGION( 0x08000, "gfx2", 0 )
	ROM_LOAD( "phz1_u40.u40", 0x00000, 0x8000, CRC(17dcddd4) SHA1(51682bdbfb67cd0ccf20b97e8fa12d72f0fe82ed) ) // 1st & 2nd half identical, but correct and verified

	ROM_REGION( 0xa0000, "questions", ROMREGION_ERASEFF )
	ROM_LOAD( "phrz1-07_std-1", 0x00000, 0x8000, CRC(0a016c5e) SHA1(1a24ecd7fe59b08c75a1b4575c7fe467cc7f0cf8) )
	ROM_LOAD( "phrz1-07_std-2", 0x10000, 0x8000, CRC(e67dc49e) SHA1(5265af228531dc16db7f7ee78da6e51ef9a1d772) )
	ROM_LOAD( "phrz1-07_std-3", 0x20000, 0x8000, CRC(5c79a653) SHA1(85a904465b347564e937074e2b18159604c83e51) )
	ROM_LOAD( "phrz1-07_std-4", 0x30000, 0x8000, CRC(9837f757) SHA1(01106114b6997fe6432e519101f95c83a1f7cc1e) )
	ROM_LOAD( "phrz1-07_std-5", 0x40000, 0x8000, CRC(dc9d8682) SHA1(46973da4298d0ed149c651498527c91b8ba57e0a) )
	ROM_LOAD( "phrz1-07_std-6", 0x50000, 0x8000, CRC(48e24f17) SHA1(f50c85505f6ab2360f0885494001f174224f8575) )

	ROM_REGION( 0x0100, "prom", 0 ) // BPROM on Question ROM board used as KEY to decode questions
	ROM_LOAD( "sc-007", 0x00000, 0x0100, CRC(8565ff9c) SHA1(688d2426dd10cd9a9edc4b22f5b3274e1067a5f8) ) // 74S287 (==N82S129N) BPROM

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "dec006.u13", 0x000, 0x117, CRC(050b675e) SHA1(7ae0bb579055b897a9b76db07a72b9930c7cd081) ) // PAL16L8ANC - brute forced
ROM_END

ROM_START( phrcrazev )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6221-45_u5-2.u5", 0x00000, 0x8000, CRC(6122b5bb) SHA1(9952b14334287a992eefefbdc887b9a9215304ef) ) // 6221-45 U5-2 070886 - Vertical version

	ROM_REGION( 0xc000, "gfx1", 0 )
	ROM_LOAD( "u39.bin",      0x00000, 0x4000, BAD_DUMP CRC(adbd2cdc) SHA1(a1e9481bd6ee0f8915cea43eaad3ebdd54438eed) )
	ROM_LOAD( "u38.bin",      0x04000, 0x4000, BAD_DUMP CRC(3578f00d) SHA1(c6780a6ee1b5eb00258a89bceabbbe380d79d299) )
	ROM_LOAD( "u37.bin",      0x08000, 0x4000, BAD_DUMP CRC(962f18a3) SHA1(ec1c3e470c59905c0f56fce2703f6ff586849512) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "u40.bin",      0x00000, 0x4000, BAD_DUMP CRC(493172c8) SHA1(a76ff5d0d3dd56b0ee4352f03c9ce92f107d34ec) )

	ROM_REGION( 0xa0000, "questions", ROMREGION_ERASEFF )
	ROM_LOAD( "phrz1-07_std-1", 0x00000, 0x8000, CRC(0a016c5e) SHA1(1a24ecd7fe59b08c75a1b4575c7fe467cc7f0cf8) )
	ROM_LOAD( "phrz1-07_std-2", 0x10000, 0x8000, CRC(e67dc49e) SHA1(5265af228531dc16db7f7ee78da6e51ef9a1d772) )
	ROM_LOAD( "phrz1-07_std-3", 0x20000, 0x8000, CRC(5c79a653) SHA1(85a904465b347564e937074e2b18159604c83e51) )
	ROM_LOAD( "phrz1-07_std-4", 0x30000, 0x8000, CRC(9837f757) SHA1(01106114b6997fe6432e519101f95c83a1f7cc1e) )
	ROM_LOAD( "phrz1-07_std-5", 0x40000, 0x8000, CRC(dc9d8682) SHA1(46973da4298d0ed149c651498527c91b8ba57e0a) )
	ROM_LOAD( "phrz1-07_std-6", 0x50000, 0x8000, CRC(48e24f17) SHA1(f50c85505f6ab2360f0885494001f174224f8575) )
	// empty space as per instructions for other "sex" category ROMs
	// "Sex" questions revision A
	ROM_LOAD( "phrz1-07_sex-2a", 0x80000, 0x8000, CRC(7ef3bca7) SHA1(f25cd01f996882a500e1a800d924759cd1de255d) )
	ROM_LOAD( "phrz1-07_sex-1a", 0x90000, 0x8000, CRC(ed7604b8) SHA1(b1e841b50b8ef6ae95fafac1c34b6d0337a05d18) )

	ROM_REGION( 0x0100, "prom", 0 ) // BPROM on Question ROM board used as KEY to decode questions
	ROM_LOAD( "sc-007", 0x00000, 0x0100, CRC(8565ff9c) SHA1(688d2426dd10cd9a9edc4b22f5b3274e1067a5f8) ) // 74S287 (==N82S129N) BPROM

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "dec006.u13", 0x000, 0x117, CRC(050b675e) SHA1(7ae0bb579055b897a9b76db07a72b9930c7cd081) ) // PAL16L8ANC - brute forced
ROM_END

ROM_START( matchem )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "6221-51_u5-1.u5", 0x00000, 0x8000, CRC(e26a6562) SHA1(f10c5bbfb175c807f16beab8eb453cbf5fbaf2d2) ) // 6221-51 U5-1 U6-1 01/15/86
	ROM_LOAD( "6221-51_u6-1.u6", 0x14000, 0x2000, CRC(35c4229a) SHA1(091c12e4bd4a6cde8323e208a9b24bbc2d1bc8c2) ) // MBM27C64

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "sex1_u39.u39", 0x00000, 0x8000, CRC(f017399a) SHA1(baf4c1bea6a12b1d4c8838552503fbdb81378411) )
	ROM_LOAD( "sex1_u38.u38", 0x08000, 0x8000, CRC(66da76c1) SHA1(8cdcec008d0d51704544069246e9eabb5d5958ea) )
	ROM_LOAD( "sex1_u37.u37", 0x10000, 0x8000, CRC(fc22bcf4) SHA1(cf3f6872965cb264d56d3a0b5ab998541b9af4ef) )

	ROM_REGION( 0x08000, "gfx2", 0 )
	ROM_LOAD( "sex1_u40.u40", 0x00000, 0x8000, CRC(a6a9a73d) SHA1(f3cb1d434d730f6e00f48079eaf8b88f57779fa0) )

	ROM_REGION( 0x0800, "crt209", 0 ) // contains Z80 program code to read inputs
	ROM_LOAD( "crt-209_6221-51.cpu",  0x00000, 0x0800, CRC(6c36361e) SHA1(7a018eecf3d8b7cf8845dcfcf8067feb292933b2) )

	ROM_REGION( 0x320, "plds", 0 )
	ROM_LOAD( "dec003.u13",          0x000, 0x117, CRC(5b9a2fec) SHA1(c56c7bbe13028903cfc82440ee8b24df855134c2) ) // PAL16L8ANC - brute forced
	ROM_LOAD( "crt-209_pal16l8.bin", 0x200, 0x117, CRC(e916c56f) SHA1(1517091ff1791d923e5bd62d18d1428b6a3a8c72) ) // SC3339 20-pin 16L8 type PAL (inside CRT-209 module)
ROM_END

/*
Known to exist but not currently dumped as seen from a manual:

SEX MATCH'em UP (GERMAN) for crt200 board Program No. 6221-52 (U5-0 & U6-0)
requires a CRT-209 Advanced Processor Module, properly encoded, inserted at U1

*/

ROM_START( matchemg )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "6221-55_u5-1.u5", 0x00000, 0x8000, CRC(152ad9f6) SHA1(fdd90ea7e5bbcd7dc8f7d6f10ac9efc08515b112) ) // 6221-55 U5-1 U6-1 01/14/86
	ROM_LOAD( "6221-55_u6-1.u6", 0x14000, 0x2000, CRC(0678d986) SHA1(c881aee9e977384a188f0f7b9e563b699da5fc0a) ) // 27C64

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "gex_1_u39.u39", 0x00000, 0x8000, CRC(da94fbc6) SHA1(af008eceba2e4ef35d0815d5cb1a5a50f1a9817f) ) // labeled  GEX 1   U39  C1987 MII - U38 & U39 had a space between GEX and 1
	ROM_LOAD( "gex_1_u38.u38", 0x08000, 0x8000, CRC(211b75cc) SHA1(52497743457afbcf2969a967d5982d8934a29864) ) // labeled  GEX 1   U38  C1987 MII
	ROM_LOAD( "gex1_u37.u37",  0x10000, 0x8000, CRC(dfc73155) SHA1(a922953ba238c3ca2f2f0a046109186d1057d76d) ) // labeled  GEX1   U37  C1987 MII - U37 & U40 had no space between GEX and 1

	ROM_REGION( 0x08000, "gfx2", 0 )
	ROM_LOAD( "gex1_u40.u40", 0x00000, 0x8000, CRC(a6a9a73d) SHA1(f3cb1d434d730f6e00f48079eaf8b88f57779fa0) ) // labeled  GEX1   U40  C1987 MII

	ROM_REGION( 0x0800, "crt209", 0 ) // contains Z80 program code to read inputs
	ROM_LOAD( "crt-209_6221-55.cpu",  0x00000, 0x0800, CRC(2c22b3a8) SHA1(663e3b687d4f2adc34e421e23773f234ca35c629) )

	ROM_REGION( 0x320, "plds", 0 )
	ROM_LOAD( "dec003.u13",          0x000, 0x117, CRC(5b9a2fec) SHA1(c56c7bbe13028903cfc82440ee8b24df855134c2) ) // PAL16L8ANC - brute forced
	ROM_LOAD( "crt-209_pal16l8.bin", 0x200, 0x117, CRC(e916c56f) SHA1(1517091ff1791d923e5bd62d18d1428b6a3a8c72) ) // SC3339 20-pin 16L8 type PAL (inside CRT-209 module)
ROM_END

ROM_START( couple ) // PCB is marked: "230188", bootleg of Match'em Up (6221-51 U5-0)
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "1.1d",  0x00000, 0x8000, CRC(bc70337a) SHA1(ffc484bc3965f0780d3fa5d8801af27a7164a417) )
	ROM_LOAD( "2.1e",  0x10000, 0x8000, CRC(17372a93) SHA1(e0f0980003473555c2543d98d1494f82afa49f1a) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "3.9c",  0x00000, 0x8000, CRC(f017399a) SHA1(baf4c1bea6a12b1d4c8838552503fbdb81378411) ) // == sex1_u39.u39
	ROM_LOAD( "4.9d",  0x08000, 0x8000, CRC(66da76c1) SHA1(8cdcec008d0d51704544069246e9eabb5d5958ea) ) // == sex1_u38.u38
	ROM_LOAD( "5.10c", 0x10000, 0x8000, CRC(fc22bcf4) SHA1(cf3f6872965cb264d56d3a0b5ab998541b9af4ef) ) // == sex1_u37.u37

	ROM_REGION( 0x08000, "gfx2", 0 )
	ROM_LOAD( "6.10d", 0x00000, 0x8000, CRC(a6a9a73d) SHA1(f3cb1d434d730f6e00f48079eaf8b88f57779fa0) ) // == sex1_u40.u40

	ROM_REGION( 0x0800, "crt209", 0 ) // same use as the CRT-209 modules, just a standard 2716 EPROM
	ROM_LOAD( "7.7a",  0x00000, 0x0800, CRC(6c36361e) SHA1(7a018eecf3d8b7cf8845dcfcf8067feb292933b2) )
ROM_END

ROM_START( couplep ) // PCB is marked: "230188", bootleg of Match'em Up (6221-51 U5-0)
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "p_1.1d", 0x00000, 0x8000, CRC(4601ace6) SHA1(a824ceebf8b9ce77ef2c8e92636e4261f2ae0420) ) // doesn't jump to the backup RAM area
	ROM_LOAD( "2.1e",   0x10000, 0x8000, CRC(17372a93) SHA1(e0f0980003473555c2543d98d1494f82afa49f1a) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "3.9c",  0x00000, 0x8000, CRC(f017399a) SHA1(baf4c1bea6a12b1d4c8838552503fbdb81378411) ) // == sex1_u39.u39
	ROM_LOAD( "4.9d",  0x08000, 0x8000, CRC(66da76c1) SHA1(8cdcec008d0d51704544069246e9eabb5d5958ea) ) // == sex1_u38.u38
	ROM_LOAD( "5.10c", 0x10000, 0x8000, CRC(fc22bcf4) SHA1(cf3f6872965cb264d56d3a0b5ab998541b9af4ef) ) // == sex1_u37.u37

	ROM_REGION( 0x08000, "gfx2", 0 )
	ROM_LOAD( "6.10d", 0x00000, 0x8000, CRC(a6a9a73d) SHA1(f3cb1d434d730f6e00f48079eaf8b88f57779fa0) ) // == sex1_u40.u40

	ROM_REGION( 0x0800, "crt209", 0 ) // same use as the CRT-209 modules, just a standard 2716 EPROM
	ROM_LOAD( "7.7a",  0x00000, 0x0800, CRC(6c36361e) SHA1(7a018eecf3d8b7cf8845dcfcf8067feb292933b2) )
ROM_END

ROM_START( couplei ) // PCB is marked: "230188", bootleg of Match'em Up (6221-51 U5-0)
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "i_1.1d", 0x00000, 0x8000, CRC(760fa29e) SHA1(a37a1562028d9615adff3d2ef88e0156354c720a) ) // looks like an intermediate release between set1 and set2
	ROM_LOAD( "2.1e",   0x10000, 0x8000, CRC(17372a93) SHA1(e0f0980003473555c2543d98d1494f82afa49f1a) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "3.9c",  0x00000, 0x8000, CRC(f017399a) SHA1(baf4c1bea6a12b1d4c8838552503fbdb81378411) ) // == sex1_u39.u39
	ROM_LOAD( "4.9d",  0x08000, 0x8000, CRC(66da76c1) SHA1(8cdcec008d0d51704544069246e9eabb5d5958ea) ) // == sex1_u38.u38
	ROM_LOAD( "5.10c", 0x10000, 0x8000, CRC(fc22bcf4) SHA1(cf3f6872965cb264d56d3a0b5ab998541b9af4ef) ) // == sex1_u37.u37

	ROM_REGION( 0x08000, "gfx2", 0 )
	ROM_LOAD( "6.10d", 0x00000, 0x8000, CRC(a6a9a73d) SHA1(f3cb1d434d730f6e00f48079eaf8b88f57779fa0) ) // == sex1_u40.u40

	ROM_REGION( 0x0800, "crt209", 0 ) // same use as the CRT-209 modules, just a standard 2716 EPROM
	ROM_LOAD( "7.7a",  0x00000, 0x0800, CRC(6c36361e) SHA1(7a018eecf3d8b7cf8845dcfcf8067feb292933b2) )
ROM_END

template <uint8_t Key>
void merit_quiz_state::init_key()
{
	m_decryption_key = Key;
}

void merit_state::init_crt209()
{
	uint8_t *rom = memregion("crt209")->base();
	std::vector<uint8_t> buffer(0x800);

	memcpy(&buffer[0], rom, 0x800);

	for (int i = 0; i < 0x800; i++)
	{
		rom[i] = buffer[bitswap<24>(i, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 1, 0, 2, 3, 4, 5, 6, 7)];
		rom[i] = bitswap<8>(rom[i], 2, 6, 5, 3, 4, 7, 0, 1);
	}
}

void merit_quiz_state::init_dtrvwz5()
{
	init_key<6>();

	init_crt209();
}

} // anonymous namespace


// Gambling type games

GAME( 1983, pitboss,    0,        pitboss, pitbossa,  merit_state,        empty_init,   ROT0,  "Merit", "The Pit Boss (2214-07, U5-0A)",      MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS ) // "7" hand written over a 5
GAME( 1983, pitbossa,   pitboss,  pitboss, pitbossa,  merit_state,        empty_init,   ROT0,  "Merit", "The Pit Boss (2214-07, U5-0)",       MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS ) // "7" hand written over a 4
GAME( 1983, pitboss04,  pitboss,  casino5, pitboss,   merit_banked_state, empty_init,   ROT0,  "Merit", "The Pit Boss (2214-04)",             MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1983, pitboss03,  pitboss,  pitboss, pitbossa,  merit_state,        empty_init,   ROT0,  "Merit", "The Pit Boss (2214-03, U5-0C)",      MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS ) // Also M4A4
GAME( 1983, pitboss03a, pitboss,  pitboss, pitbossa1, merit_state,        empty_init,   ROT0,  "Merit", "The Pit Boss (2214-03, U5-1C)",      MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS ) // Also M4A4
GAME( 1983, pitboss03b, pitboss,  pitboss, pitbossa,  merit_state,        empty_init,   ROT0,  "Merit", "The Pit Boss (M4A4)",                MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS ) // No labels, so use internal designation
GAME( 1983, pitbossm4,  pitboss,  pitboss, pitbossb,  merit_state,        empty_init,   ROT0,  "Merit", "The Pit Boss (M4A1)",                MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1983, pitbossps,  pitboss,  pitboss, pitbossa,  merit_state,        empty_init,   ROT0,  "Merit", "The Pit Boss (PSB1)",                MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1983, housecard,  pitboss,  pitboss, pitbossa,  merit_state,        empty_init,   ROT0,  "Merit", "House of Cards (HSC1)",              MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1983, mdchoice,   pitboss,  pitboss, mdchoice,  merit_state,        empty_init,   ROT0,  "Merit", "Dealer's Choice (E4A1)",             MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS ) // Copyright year based on other Pit Boss sets
GAME( 1983, mpchoice,   pitboss,  pitboss, mpchoice,  merit_state,        empty_init,   ROT0,  "Merit", "Player's Choice (M4C1)",             MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1982, mpchoicea,  pitboss,  pitboss, mpchoicea, merit_state,        empty_init,   ROT0,  "Merit", "Player's Choice (M3C1)",             MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS )

GAME( 1989, casino5,    0,        casino5, casino5,   merit_banked_state, empty_init,   ROT0,  "Merit", "Casino Five (3315-02, U5-2B)",       MACHINE_SUPPORTS_SAVE )
GAME( 1984, casino5a,   casino5,  casino5, casino5,   merit_banked_state, empty_init,   ROT0,  "Merit", "Casino Five (3315-02, U5-0)",        MACHINE_SUPPORTS_SAVE )
GAME( 1984, casino5b,   casino5,  casino5, casino5,   merit_banked_state, empty_init,   ROT0,  "Merit", "Casino Five (3315-12, U5-0)",        MACHINE_SUPPORTS_SAVE )

GAME( 1984, mroundup,   0,        pitboss, mroundup,  merit_state,        empty_init,   ROT0,  "Merit", "The Round Up",                       MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )

GAME( 1984, chkndraw,   0,        pitboss, chkndraw,  merit_state,        empty_init,   ROT0,  "Merit", "Chicken Draw (2131-04, U5-1)",       MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1984, chkndrawa,  chkndraw, pitboss, chkndraw,  merit_state,        empty_init,   ROT0,  "Merit", "Chicken Draw (2131-04, U5-0)",       MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )

GAME( 1987, riviera,    0,        riviera, riviera,   merit_state,        empty_init,   ROT0,  "Merit", "Riviera Hi-Score (2131-08, U5-4A)",  MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1986, rivieraa,   riviera,  riviera, riviera,   merit_state,        empty_init,   ROT0,  "Merit", "Riviera Hi-Score (2131-08, U5-4)",   MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1986, rivierab,   riviera,  riviera, rivierab,  merit_state,        empty_init,   ROT0,  "Merit", "Riviera Hi-Score (2131-08, U5-2D)",  MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1990, mosdraw,    0,        mosdraw, mosdraw,   merit_state,        empty_init,   ROT0,  "Merit", "Montana Super Draw (4436-05, U5-0)", MACHINE_NODEVICE_PRINTER | MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS ) // needs printer and RTC hook up

GAME( 1986, bigappg,    0,        bigappg, bigappg,   merit_state,        empty_init,   ROT0,  "Big Apple Games / Merit", "The Big Apple (2131-13, U5-0)",         MACHINE_SUPPORTS_SAVE )
GAME( 1986, misdraw,    0,        misdraw, bigappg,   merit_state,        empty_init,   ROT0,  "Big Apple Games / Merit", "Michigan Super Draw (2131-16, U5-2)",   MACHINE_SUPPORTS_SAVE )
GAME( 1990, iowapp,     0,        riviera, iowapp,    merit_state,        empty_init,   ROT0,  "Merit",                   "Iowa Premium Player (2131-21, U5-1)",   MACHINE_SUPPORTS_SAVE ) // Copyright year based on ROM label

GAME( 1988, dodgectyba, dodgecty, no_u40,  dodge,     merit_state,        init_crt209,  ROT0,  "Merit", "Dodge City (2131-82, U5-0D)",        MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1989, dodgectybb, dodgecty, no_u40,  dodge,     merit_state,        init_crt209,  ROT0,  "Merit", "Dodge City (2131-82, U5-50)",        MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1989, dodgectybc, dodgecty, no_u40,  dodge,     merit_state,        init_crt209,  ROT0,  "Merit", "Dodge City (2131-82, U5-0 GT)",      MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )

GAME( 1989, msupstar,   0,        no_u40,  msupstar,  merit_state,        init_crt209,  ROT0,  "Merit", "Superstar (4435-81, U5-1)",          MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NODEVICE_PRINTER )

// Trivia and Word games

GAME( 1985, trvwz,      0,        trvwhiz,  trivia,   merit_quiz_state,   empty_init,   ROT0,  "Merit", "Trivia ? Whiz (6221-00)",                                  MACHINE_SUPPORTS_SAVE )
GAME( 1985, trvwza,     trvwz,    trvwhiz,  trivia,   merit_quiz_state,   empty_init,   ROT0,  "Merit", "Trivia ? Whiz (6221-00, with Sex trivia)",                 MACHINE_SUPPORTS_SAVE )
GAME( 1985, trvwzb,     trvwz,    trvwhiz,  trivia,   merit_quiz_state,   empty_init,   ROT0,  "Merit", "Trivia ? Whiz (6221-00, Alt Gen trivia)",                  MACHINE_SUPPORTS_SAVE )
GAME( 1985, trvwzv,     trvwz,    trvwhiz,  trivia,   merit_quiz_state,   empty_init,   ROT90, "Merit", "Trivia ? Whiz (6221-02, Vertical)",                        MACHINE_SUPPORTS_SAVE )

GAME( 1985, trvwz2,     0,        trvwhiz,  trivia,   merit_quiz_state,   init_key<2>,  ROT90, "Merit", "Trivia ? Whiz Edition 2 (6221-05)",                        MACHINE_SUPPORTS_SAVE )
GAME( 1985, trvwz2a,    trvwz2,   trvwhiz,  trivia,   merit_quiz_state,   init_key<2>,  ROT90, "Merit", "Trivia ? Whiz Edition 2 (6221-05, Alt Sex trivia)",        MACHINE_SUPPORTS_SAVE )

GAME( 1985, trvwz3,     0,        trvwhiz,  trivia,   merit_quiz_state,   empty_init,   ROT0,  "Merit", "Trivia ? Whiz Edition 3 (6221-05, U5-0D)",                 MACHINE_SUPPORTS_SAVE )
GAME( 1985, trvwz3a,    trvwz3,   trvwhiz,  trivia,   merit_quiz_state,   empty_init,   ROT0,  "Merit", "Trivia ? Whiz Edition 3 (6221-05, U5-0C)",                 MACHINE_SUPPORTS_SAVE )
GAME( 1985, trvwz3b,    trvwz3,   trvwhiz,  trivia,   merit_quiz_state,   empty_init,   ROT0,  "Merit", "Trivia ? Whiz Edition 3 (6221-05, with Sex trivia III)",   MACHINE_SUPPORTS_SAVE )
GAME( 1985, trvwz3v,    trvwz3,   trvwhiz,  trivia,   merit_quiz_state,   empty_init,   ROT90, "Merit", "Trivia ? Whiz Edition 3 (6221-04, U5-0E, Vertical)",       MACHINE_SUPPORTS_SAVE )

GAME( 1985, trvwz4,     0,        trvwhziv, trvwhziv, merit_quiz_state,   init_key<5>,  ROT0,  "Merit", "Trivia ? Whiz Edition 4 (6221-10, U5-0A)",                 MACHINE_SUPPORTS_SAVE )
GAME( 1985, trvwz4v,    trvwz4,   trvwhziv, trvwhziv, merit_quiz_state,   init_key<5>,  ROT90, "Merit", "Trivia ? Whiz Edition 4 (6221-13, U5-0B, Vertical)",       MACHINE_SUPPORTS_SAVE )
GAME( 1985, trvwz4va,   trvwz4,   trvwhziv, trvwhziv, merit_quiz_state,   init_key<5>,  ROT90, "Merit", "Trivia ? Whiz Edition 4 (6221-13, U5-0B, Vertical, Alt Sex trivia)", MACHINE_SUPPORTS_SAVE )

GAME( 1985, tictac,     0,        tictac,   tictac,   merit_quiz_state,   init_key<8>,  ROT0,  "Merit", "Tic Tac Trivia (6221-23, U5-0C, 07/07/86)",                MACHINE_SUPPORTS_SAVE ) // all new trivia categories
GAME( 1985, tictaca,    tictac,   tictac,   tictac,   merit_quiz_state,   init_key<4>,  ROT0,  "Merit", "Tic Tac Trivia (6221-23, U5-0C, 02/11/86)",                MACHINE_SUPPORTS_SAVE )
GAME( 1985, tictacv,    tictac,   tictac,   tictac,   merit_quiz_state,   init_key<4>,  ROT90, "Merit", "Tic Tac Trivia (6221-22, U5-0, Vertical)",                 MACHINE_SUPPORTS_SAVE )

GAME( 1986, phrcraze,   0,        phrcraze, phrcrazs, merit_quiz_state,   init_key<7>,  ROT0,  "Merit", "Phraze Craze (6221-40, U5-3A, Expanded Questions)",        MACHINE_SUPPORTS_SAVE )
GAME( 1986, phrcrazea,  phrcraze, phrcraze, phrcrazs, merit_quiz_state,   init_key<7>,  ROT0,  "Merit", "Phraze Craze (6221-40, U5-3, Expanded Questions)",         MACHINE_SUPPORTS_SAVE )
GAME( 1986, phrcrazeb,  phrcraze, phrcraze, phrcraze, merit_quiz_state,   init_key<7>,  ROT0,  "Merit", "Phraze Craze (6221-40, U5-0A)",                            MACHINE_SUPPORTS_SAVE )
GAME( 1986, phrcrazec,  phrcraze, phrcraze, phrcraza, merit_quiz_state,   init_key<7>,  ROT0,  "Merit", "Phraze Craze (6221-40, U5-0)",                             MACHINE_SUPPORTS_SAVE )
GAME( 1986, phrcrazev,  phrcraze, phrcraze, phrcrazs, merit_quiz_state,   init_key<7>,  ROT90, "Merit", "Phraze Craze (6221-45, U5-2, Vertical)",                   MACHINE_SUPPORTS_SAVE )

GAME( 1987, dtrvwz5,   0,         dtrvwz5,  dtrvwz5,  merit_quiz_state,   init_dtrvwz5, ROT0,  "Merit", "Deluxe Trivia ? Whiz Edition 5 (6221-70, U5-0A)",          MACHINE_SUPPORTS_SAVE )
GAME( 1987, dtrvwz5v,  dtrvwz5,   dtrvwz5,  dtrvwz5,  merit_quiz_state,   init_dtrvwz5, ROT90, "Merit", "Deluxe Trivia ? Whiz Edition 5 (6221-75, U5-0, Vertical)", MACHINE_SUPPORTS_SAVE )

GAME( 1986, matchem,   0,         couple,   matchem,  merit_state,        init_crt209,  ROT0,  "Merit",   "Match'em Up (6221-51, U5-1)",                            MACHINE_SUPPORTS_SAVE )
GAME( 1986, matchemg,  matchem,   couple,   matchemg, merit_state,        init_crt209,  ROT0,  "Merit",   "Match'em Up (6221-55, U5-1, German)",                    MACHINE_SUPPORTS_SAVE )
GAME( 1988, couple,    matchem,   couple,   couple,   merit_state,        init_crt209,  ROT0,  "bootleg", "The Couples (set 1)",                                    MACHINE_SUPPORTS_SAVE )
GAME( 1988, couplep,   matchem,   couple,   couplep,  merit_state,        init_crt209,  ROT0,  "bootleg", "The Couples (set 2)",                                    MACHINE_SUPPORTS_SAVE )
GAME( 1988, couplei,   matchem,   couple,   couple,   merit_state,        init_crt209,  ROT0,  "bootleg", "The Couples (set 3)",                                    MACHINE_SUPPORTS_SAVE )
