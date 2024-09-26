// license:BSD-3-Clause
// copyright-holders: Nicola Salmoria, Pierpaolo Prazzoli, Tomasz Slanina, Mariusz Wojcieszek

/*

  Greyhound Electronics Inc. hardware

  driver by Nicola Salmoria
  driver by Pierpaolo Prazzoli and graphic fixes by Tomasz Slanina
  additional work by Mariusz Wojcieszek


ROM board has a part # UVM10B  1984
Main board has a part # UV-1B Rev 5 1982-83-84

Processor: Z80
Support Chips:(2) 8255s
RAM: 6117on ROM board and (24) MCM4517s on main board

When games are first run a RAM error will occur because the NVRAM needs initializing.
When ERROR appears, press F2, then F3 to reset, then F2 again and the game will start

Trivia games "No Coins" mode: if DSW "No Coins" is on, coin inputs are
replaced by a 6th button to start games. This is a feature of the PCB for private use.

Selection/Poker payout button: if pressed, all coins/credits are gone and added to the
payout bookkeeping, shown in the service mode under the coin in total. Last Winner shows
the last payout. Payout hardware is unknown.

Video Trivia sets (as stated from Greyhound Electronics, Inc. manual):

Series 1: (128K)           Series 2: (128K)           Series 3: (128K)
----------------           ----------------           ----------------
Science                    T.V. Mash                  Beatlemania
General I                  General II                 General III
Sports I                   Sports II                  Sports III
Music                      Comics                     Country-Western
Movies-T.V.                Entertainment              T.V. Soaps


Series 4: (128K)           Series 5: (128K)           Series 6: (128K)
----------------           ----------------           ----------------
History-Geography          The States                 Science II
Star Trek                  James Bond                 General IV
Baseball                   Hockey                     Commercials-Ads
Hollywood                  Elvismania                 Honeymooners
Television I               The Wild West              Television II


Series 7: (128K)
----------------
T.V. Dallas
General V
Kids Korner
Good Guys
Biblical
 or alt: Sex Trivia


NOTE: Series 8 through 14 are "T3" sets as noted by the first 2 bytes of the questions ROMs.
      The question ROMs are 27256 (twice the size of previous question ROMs) and contain 3
      times the number of questions.  ROM labels are yellow.


Series 8: (256K)           Series 9: (256K)           Series 10: (256K)
----------------           ----------------           -----------------
Science                    Facts                      New Science
General                    Rock-N-Roll                New General
Sports                     Television                 New T.V. Mash
T.V./Entertainment         Artists-Athletes           New Entertainment
Adult Sex 1                U.S.A. Trivia              New Sports
 or alt: Potpourri          or alt: Adult Sex 2        or alt: Adult Sex 3


Series 11: (256K)          Series 12: (256K)          Series 14: (256K)
-----------------          -----------------          -----------------
Rich and Famous            New Science 2              Famous Couples
General Facts              Cops and Robbers           The Sixties
TV / Music                 Rock Music                 TV Comedies
Fast Women and Cars        Famous Quotes              Horrors
Aerospace                  Vices                      War and Peace
 or alt: Gay Times          or alt: Adult Sex 4        or alt: Adult Sex 5


NOTE: Series 15 and later are "T4" sets as noted by the first 2 bytes of the question
      ROMs.  These sets have a built in signature. Bytes 0x03 through 0x0A must match
      expected values or the question ROM will not be visible. This was done to prevent
      operator ROM swaps. These sets use the TRIV3D ROM board which has 2 PALs, so it's
      believed one of the PALs supplies the values for the signature. ROM labels are
      pink.


Series 15: (256K)          Series 16: (256K)          Series 17: (256K)
-----------------          -----------------          -----------------
Entertainment 2            Late TV Shows              Night Time TV
The Seventies              The Eighties               New Eighties
Facts 2                    People and Places          History and Geography
New Science 3              Potluck                    New Potpourri
N F L Football             N B A Basketball           N H L Hockey
 or alt: Adult Sex 6        or alt: Adult Sex 7        or alt: Adult Sex 8


Series 18: (256K)          Series 19: (256K)
-----------------          -----------------
Entertainment 3            Social Study
This is Music              Television 2
Super Trivia               Variety Pack
World Geography            Famous Firsts
More Sports                Adult Sex 10
 or alt: Adult Sex 9


NOTE: There is no known documented Series 13 questions set.

NOTE: Trivia Question ROM names are the internal names used. IE: read from the file with
      a Hex Editor. Any "_alt" extension is used to separate different ROMs with the same
      label or internal name. Any "_old" extension is used to distinguish between ROM labels
      where the later ROM label included an asterisk to denote a revised version ROM.

*/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "sound/dac.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class gei_state : public driver_device
{
public:
	gei_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ppi(*this, "ppi8255_%u", 0U)
		, m_dac(*this, "dac")
		, m_ticket(*this, "ticket")
		, m_screen(*this, "screen")
		, m_signature(*this, "signature")
		, m_rombank(*this, "rombank")
		, m_lamps(*this, "lamp%u", 0U)
	{ }

	void findout(machine_config &config);
	void suprpokr(machine_config &config);
	void gselect(machine_config &config);
	void amuse1(machine_config &config);
	void gepoker(machine_config &config);
	void jokpokera(machine_config &config);
	void quizvid(machine_config &config);
	void getrivia(machine_config &config);
	void amuse(machine_config &config);
	void sprtauth(machine_config &config);

	void init_setbank();
	void init_bank2k();
	void init_bank8k();
	void init_geimulti();

protected:
	virtual void machine_start() override { m_lamps.resolve(); }
	virtual void video_start() override ATTR_COLD;

private:
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void drawctrl_w(offs_t offset, uint8_t data);
	void bitmap_w(offs_t offset, uint8_t data);
	uint8_t catchall(offs_t offset);
	template <unsigned N> void banksel_w(uint8_t data) { m_rombank->set_entry(N); }
	void geimulti_bank_w(offs_t offset, uint8_t data);
	template <unsigned N> uint8_t banksel_r();
	uint8_t signature_r();
	void signature_w(uint8_t data);
	void lamps_w(uint8_t data);
	void sound_w(uint8_t data);
	void sound2_w(uint8_t data);
	void lamps2_w(uint8_t data);
	void nmi_w(uint8_t data);
	uint8_t portc_r();

	INTERRUPT_GEN_MEMBER(vblank_irq);

	void amuse1_map(address_map &map) ATTR_COLD;
	void amuse_map(address_map &map) ATTR_COLD;
	void findout_map(address_map &map) ATTR_COLD;
	void gepoker_map(address_map &map) ATTR_COLD;
	void getrivia_map(address_map &map) ATTR_COLD;
	void gselect_map(address_map &map) ATTR_COLD;
	void quizvid_map(address_map &map) ATTR_COLD;
	void sprtauth_map(address_map &map) ATTR_COLD;
	void suprpokr_map(address_map &map) ATTR_COLD;

	bitmap_ind16 m_bitmap;

	uint8_t m_drawctrl[3];
	uint8_t m_color[8];
	uint16_t m_prevoffset;
	int m_yadd;
	uint8_t m_signature_answer;
	uint8_t m_signature_pos;
	uint8_t m_nmi_mask;

	required_device<cpu_device> m_maincpu;
	required_device_array<i8255_device, 2> m_ppi;
	required_device<dac_bit_interface> m_dac;
	optional_device<ticket_dispenser_device> m_ticket;
	required_device<screen_device> m_screen;
	optional_region_ptr<uint8_t> m_signature;
	optional_memory_bank m_rombank;
	output_finder<13> m_lamps;
};


void gei_state::drawctrl_w(offs_t offset, uint8_t data)
{
	m_drawctrl[offset] = data;

	if (offset == 2)
	{
		for (int i = 0; i < 8; i++)
			if (BIT(m_drawctrl[1], i)) m_color[i] = m_drawctrl[0] & 7;
	}
}

void gei_state::bitmap_w(offs_t offset, uint8_t data)
{
	m_yadd = (offset == m_prevoffset) ? (m_yadd + 1) : 0;
	m_prevoffset = offset;

	int const sx = 8 * (offset % 64);
	int sy = offset / 64;
	sy = (sy + m_yadd) & 0xff;

	for (int i = 0; i < 8; i++)
		m_bitmap.pix(sy, sx + i) = m_color[8 - i - 1];
}

void gei_state::video_start()
{
	m_screen->register_screen_bitmap(m_bitmap);

	save_item(NAME(m_drawctrl));
	save_item(NAME(m_color));
	save_item(NAME(m_prevoffset));
	save_item(NAME(m_yadd));
	save_item(NAME(m_nmi_mask));
	save_item(NAME(m_bitmap));

	if (m_signature.found())
	{
		save_item(NAME(m_signature_answer));
		save_item(NAME(m_signature_pos));
	}
}

uint32_t gei_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}

void gei_state::lamps_w(uint8_t data)
{
	// 5 button lamps
	m_lamps[0] = BIT(data, 0);
	m_lamps[1] = BIT(data, 1);
	m_lamps[2] = BIT(data, 2);
	m_lamps[3] = BIT(data, 3);
	m_lamps[4] = BIT(data, 4);

	/* 3 button lamps for deal, cancel, stand in poker games;
	lamp order verified in poker and selection self tests */
	m_lamps[7] = BIT(data, 5);
	m_lamps[5] = BIT(data, 6);
	m_lamps[6] = BIT(data, 7);
}

void gei_state::sound_w(uint8_t data)
{
	// bit 3 - coin lockout, lamp10 in poker / lamp6 in trivia test modes
	machine().bookkeeping().coin_lockout_global_w(BIT(~data, 3));
	m_lamps[9] = BIT(data, 3);

	// bit 5 - ticket out in trivia games
	if (m_ticket.found())
		m_ticket->motor_w(BIT(data, 5));

	// bit 6 enables NMI
	m_nmi_mask = data & 0x40;

	// bit 7 goes directly to the sound amplifier
	m_dac->write(BIT(data, 7));
}

void gei_state::sound2_w(uint8_t data)
{
	// bit 3,6 - coin lockout, lamp 10 + 11 in selection test mode
	machine().bookkeeping().coin_lockout_w(0, BIT(~data, 3));
	machine().bookkeeping().coin_lockout_w(1, BIT(~data, 6));
	m_lamps[9] = BIT(data, 3);
	m_lamps[10] = BIT(data, 6);

	/* bit 4,5 - lamps 12, 13 in selection test mode;
	12 lights up if dsw maximum bet = 30 an bet > 15 or if dsw maximum bet = 10 an bet = 10 */
	m_lamps[11] = BIT(data, 4);
	m_lamps[12] = BIT(data, 5);

	// bit 7 goes directly to the sound amplifier
	m_dac->write(BIT(data, 7));
}

void gei_state::lamps2_w(uint8_t data)
{
	// bit 4 - play/raise button lamp, lamp 9 in poker test mode
	m_lamps[8] = BIT(data, 4);
}

void gei_state::nmi_w(uint8_t data)
{
	// bit 4 - play/raise button lamp, lamp 9 in selection test mode
	m_lamps[8] = BIT(data, 4);

	// bit 6 enables NMI
	m_nmi_mask = data & 0x40;
}

uint8_t gei_state::catchall(offs_t offset)
{
	int const pc = m_maincpu->pc();

	if (pc != 0x3c74 && pc != 0x0364 && pc != 0x036d)   // weed out spurious blit reads
		logerror("%04x: unmapped memory read from %04x\n", pc, offset);

	return 0xff;
}

uint8_t gei_state::portc_r()
{
	return 4;
}


void gei_state::geimulti_bank_w(offs_t offset, uint8_t data)
{
	int bank = -1;

	switch(offset + 0x5a00)
	{
		case 0x5a7e: bank = 0; break;
		case 0x5a7d: bank = 1; break;
		case 0x5a7b: bank = 2; break;
		case 0x5a77: bank = 3; break;
		case 0x5a6f: bank = 4; break;
		case 0x5a5f: bank = 5; break;
		case 0x5a3f: bank = 6; break;
		case 0x5c7d: bank = 7; break;
		case 0x5c7b: bank = 8; break;
		case 0x5c77: bank = 9; break;
		case 0x5c6f: bank = 10; break;
		case 0x5c5f: bank = 11; break;
		case 0x5c3f: bank = 12; break;
		case 0x5c7e: bank = 13; break;
		case 0x5aff: case 0x5cff: break;
		default: logerror( "Unknown banking write, offset = %04x, data = %02x\n", offset, data);
	}

	if (bank != -1)
		m_rombank->set_entry(bank);
}

template <unsigned N>
uint8_t gei_state::banksel_r()
{
	if (!machine().side_effects_disabled())
		m_rombank->set_entry(N);
	return 0x03;
}

// This signature is used to validate the ROMs in some sets. Simple protection check?

uint8_t gei_state::signature_r()
{
	return m_signature_answer;
}

void gei_state::signature_w(uint8_t data)
{
	if (data == 0) m_signature_pos = 0;

	else
	{
		m_signature_answer = m_signature[m_signature_pos++];

		m_signature_pos &= 7;   // safety; shouldn't happen
	}
}

void gei_state::getrivia_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x3fff).bankr(m_rombank);
	map(0x4000, 0x47ff).ram().share("nvram");
	map(0x4800, 0x4803).rw(m_ppi[0], FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x5000, 0x5003).rw(m_ppi[1], FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x600f, 0x600f).w(FUNC(gei_state::banksel_w<8>));
	map(0x6017, 0x6017).w(FUNC(gei_state::banksel_w<6>));
	map(0x601b, 0x601b).w(FUNC(gei_state::banksel_w<4>));
	map(0x601d, 0x601d).w(FUNC(gei_state::banksel_w<2>));
	map(0x601e, 0x601e).w(FUNC(gei_state::banksel_w<0>));
	map(0x608f, 0x608f).w(FUNC(gei_state::banksel_w<9>));
	map(0x6097, 0x6097).w(FUNC(gei_state::banksel_w<7>));
	map(0x609b, 0x609b).w(FUNC(gei_state::banksel_w<5>));
	map(0x609d, 0x609d).w(FUNC(gei_state::banksel_w<3>));
	map(0x609e, 0x609e).w(FUNC(gei_state::banksel_w<1>));
	map(0x8000, 0x8002).w(FUNC(gei_state::drawctrl_w));
	map(0x8000, 0x9fff).rom(); // space for diagnostic ROM?
	map(0xa000, 0xbfff).rom();
	map(0xc000, 0xffff).ram().w(FUNC(gei_state::bitmap_w));
}

void gei_state::gselect_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x3fff).bankr(m_rombank);
	map(0x4000, 0x40ff).ram().share("nvram");
	map(0x4400, 0x4400).w(FUNC(gei_state::banksel_w<0>));
	map(0x4401, 0x4401).w(FUNC(gei_state::banksel_w<1>));
	map(0x4402, 0x4402).w(FUNC(gei_state::banksel_w<2>));
	map(0x4403, 0x4403).w(FUNC(gei_state::banksel_w<3>));
	map(0x4800, 0x4803).rw(m_ppi[0], FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x5000, 0x5003).rw(m_ppi[1], FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x8000, 0x8002).w(FUNC(gei_state::drawctrl_w));
	map(0xc000, 0xffff).ram().w(FUNC(gei_state::bitmap_w));
}

// TODO: where are mapped the lower 0x2000 bytes of the banks?
void gei_state::amuse_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x3fff).bankr(m_rombank);
	map(0x4000, 0x47ff).ram().share("nvram");
	map(0x4800, 0x4803).rw(m_ppi[0], FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x5000, 0x5003).rw(m_ppi[1], FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x606f, 0x606f).w(FUNC(gei_state::banksel_w<8>));
	map(0x6077, 0x6077).w(FUNC(gei_state::banksel_w<6>));
	map(0x607b, 0x607b).w(FUNC(gei_state::banksel_w<4>));
	map(0x607d, 0x607d).w(FUNC(gei_state::banksel_w<2>));
	map(0x607e, 0x607e).w(FUNC(gei_state::banksel_w<0>));
	map(0x8000, 0x8002).w(FUNC(gei_state::drawctrl_w));
	map(0x8000, 0xbfff).rom();
	map(0xc000, 0xffff).ram().w(FUNC(gei_state::bitmap_w));
}

void gei_state::gepoker_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x3fff).bankr(m_rombank);
	map(0x4000, 0x47ff).ram().share("nvram");
	map(0x4800, 0x4803).rw(m_ppi[0], FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x5000, 0x5003).rw(m_ppi[1], FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x60ef, 0x60ef).w(FUNC(gei_state::banksel_w<4>));
	map(0x60f7, 0x60f7).w(FUNC(gei_state::banksel_w<3>));
	map(0x60fb, 0x60fb).w(FUNC(gei_state::banksel_w<2>));
	map(0x60fd, 0x60fd).w(FUNC(gei_state::banksel_w<1>));
	map(0x60fe, 0x60fe).w(FUNC(gei_state::banksel_w<0>));
	map(0x8000, 0x8002).w(FUNC(gei_state::drawctrl_w));
	map(0x8000, 0xbfff).rom(); // space for diagnostic ROM?
	map(0xc000, 0xffff).ram().w(FUNC(gei_state::bitmap_w));
	map(0xe000, 0xffff).rom();
}

void gei_state::amuse1_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x3fff).bankr(m_rombank);
	map(0x4000, 0x43ff).ram().share("nvram");
	map(0x4400, 0x4400).w(FUNC(gei_state::banksel_w<0>));
	map(0x4401, 0x4401).w(FUNC(gei_state::banksel_w<2>));
	map(0x4402, 0x4402).w(FUNC(gei_state::banksel_w<4>));
	map(0x4403, 0x4403).w(FUNC(gei_state::banksel_w<6>));
	map(0x4800, 0x4803).rw(m_ppi[0], FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x5000, 0x5003).rw(m_ppi[1], FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x5800, 0x5fff).rom();
	map(0x8000, 0x8002).w(FUNC(gei_state::drawctrl_w));
	map(0x8000, 0xbfff).rom(); // space for diagnostic ROM?
	map(0xc000, 0xffff).ram().w(FUNC(gei_state::bitmap_w));
	map(0xe000, 0xffff).rom();
}

void gei_state::findout_map(address_map &map)
{
	map(0x0000, 0xffff).r(FUNC(gei_state::catchall));
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x47ff).ram().share("nvram");
	map(0x4800, 0x4803).rw(m_ppi[0], FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x5000, 0x5003).rw(m_ppi[1], FUNC(i8255_device::read), FUNC(i8255_device::write));
	// banked ROMs are enabled by low 6 bits of the address
	map(0x601f, 0x601f).w(FUNC(gei_state::banksel_w<0>));
	map(0x602f, 0x602f).w(FUNC(gei_state::banksel_w<5>));
	map(0x6037, 0x6037).w(FUNC(gei_state::banksel_w<4>));
	map(0x603b, 0x603b).w(FUNC(gei_state::banksel_w<3>));
	map(0x603d, 0x603d).w(FUNC(gei_state::banksel_w<2>));
	map(0x603e, 0x603e).w(FUNC(gei_state::banksel_w<1>));
	map(0x6200, 0x6200).w(FUNC(gei_state::signature_w));
	map(0x6400, 0x6400).r(FUNC(gei_state::signature_r));
	map(0x7800, 0x7fff).rom(); // space for diagnostic ROM?
	map(0x8000, 0xffff).bankr(m_rombank);
	map(0x8000, 0x8002).w(FUNC(gei_state::drawctrl_w));
	map(0xc000, 0xffff).w(FUNC(gei_state::bitmap_w));
}

void gei_state::quizvid_map(address_map &map)
{
	map(0x0000, 0xffff).r(FUNC(gei_state::catchall));
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x47ff).ram().share("nvram");
	map(0x4800, 0x4803).rw(m_ppi[0], FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x5000, 0x5003).rw(m_ppi[1], FUNC(i8255_device::read), FUNC(i8255_device::write));
	// banked ROMs are enabled by low 6 bits of the address
	map(0x602f, 0x602f).r(FUNC(gei_state::banksel_r<5>));
	map(0x6037, 0x6037).r(FUNC(gei_state::banksel_r<4>));
	map(0x603b, 0x603b).r(FUNC(gei_state::banksel_r<3>));
	map(0x603d, 0x603d).r(FUNC(gei_state::banksel_r<2>));
	map(0x603e, 0x603e).r(FUNC(gei_state::banksel_r<1>));
	map(0x7800, 0x7fff).rom(); // space for diagnostic ROM?
	map(0x8000, 0xffff).bankr(m_rombank);
	map(0x8000, 0x8002).w(FUNC(gei_state::drawctrl_w));
	map(0xc000, 0xffff).w(FUNC(gei_state::bitmap_w));
}

void gei_state::suprpokr_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x47ff).ram().share("nvram");
	map(0x4800, 0x4803).rw(m_ppi[0], FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x5000, 0x5003).rw(m_ppi[1], FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x6200, 0x6200).w(FUNC(gei_state::signature_w));
	map(0x6400, 0x6400).r(FUNC(gei_state::signature_r));
	map(0x8000, 0x8002).w(FUNC(gei_state::drawctrl_w));
	map(0xc000, 0xffff).w(FUNC(gei_state::bitmap_w));
	map(0x8000, 0xffff).rom();
}

void gei_state::sprtauth_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x47ff).ram().share("nvram");
	map(0x4800, 0x4803).rw(m_ppi[0], FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x5000, 0x5003).rw(m_ppi[1], FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x5600, 0x5600).r(FUNC(gei_state::signature_r));
	map(0x5800, 0x5800).w(FUNC(gei_state::signature_w));
	map(0x5a00, 0x5cff).w(FUNC(gei_state::geimulti_bank_w));
	map(0x6000, 0x7fff).rom();
	map(0x8000, 0x8002).w(FUNC(gei_state::drawctrl_w));
	map(0x8000, 0xffff).bankr(m_rombank);
	map(0xc000, 0xffff).w(FUNC(gei_state::bitmap_w));
}

static INPUT_PORTS_START(reelfun_standard)
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_Z) PORT_NAME("1 Left A-Z")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(KEYCODE_X) PORT_NAME("2 Right A-Z")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(KEYCODE_C) PORT_NAME("3 Select Letter")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CODE(KEYCODE_V) PORT_NAME("4 Start")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_B) PORT_NAME("5 Solve Puzzle")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START(trivia_standard)
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("ticket", ticket_dispenser_device, line_r)
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( gselect )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, "Poker: Discard Cards" )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x06, 0x06, "Poker: Pay on" )     PORT_DIPLOCATION("SW1:2,3")
	PORT_DIPSETTING(    0x06, "any Pair" )
	PORT_DIPSETTING(    0x04, "Pair of Eights or better" )
	PORT_DIPSETTING(    0x02, "Pair of Jacks or better" )
	PORT_DIPSETTING(    0x00, "Pair of Aces only" )
	PORT_DIPNAME( 0x08, 0x00, "Maximum Bet" )       PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, "30" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x10, 0x10, "Poker: Credits needed for 2 Jokers" )    PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, "8" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0xe0, 0x80, "Payout Percentage" )     PORT_DIPLOCATION("SW1:6,7,8")
	PORT_DIPSETTING(    0xe0, "35" )
	PORT_DIPSETTING(    0xc0, "40" )
	PORT_DIPSETTING(    0xa0, "45" )
	PORT_DIPSETTING(    0x80, "50" )
	PORT_DIPSETTING(    0x60, "55" )
	PORT_DIPSETTING(    0x40, "60" )
	PORT_DIPSETTING(    0x20, "65" )
	PORT_DIPSETTING(    0x00, "70" )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_IMPULSE(2) PORT_NAME("Button 12 ?")
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_IMPULSE(2) PORT_NAME ("Payout")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_IMPULSE(2) PORT_NAME ("Play / Raise")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_IMPULSE(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_IMPULSE(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_IMPULSE(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_IMPULSE(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_IMPULSE(2) PORT_NAME ("Deal")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_IMPULSE(2) PORT_NAME ("Cancel")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_IMPULSE(2) PORT_NAME ("Stand")
//  Button 8, 6, 7 order verified in test mode switch test

	PORT_START("IN2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( gepoker )
	PORT_INCLUDE( gselect )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no coin 2
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no button 12
INPUT_PORTS_END

static INPUT_PORTS_START( getrivia )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x03, 0x01, "Questions" )     PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x02, "5 (duplicate)" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x04, 0x00, "Show Answer" )   PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x00, "Max Coins" )     PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPSETTING(    0x08, "30" )
	PORT_DIPNAME( 0x10, 0x00, "Timeout" )       PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, "Tickets" )       PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, "No Coins" )      PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2) PORT_CONDITION("DSWA", 0x40, EQUALS, 0x40)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_IMPULSE(2) PORT_CONDITION("DSWA", 0x40, EQUALS, 0x00) PORT_NAME ("Start in no coins mode")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2) PORT_CONDITION("DSWA", 0x40, EQUALS, 0x40)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_CONDITION("DSWA", 0x40, EQUALS, 0x00)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("ticket", ticket_dispenser_device, line_r)
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( sextriv1 )
	PORT_INCLUDE( getrivia )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no coin 2
INPUT_PORTS_END

static INPUT_PORTS_START(suprpokr)
	PORT_INCLUDE(gepoker)

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Screen" )        PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, "Horizontal" )
	PORT_DIPSETTING(    0x00, "Vertical" )
	PORT_DIPNAME( 0x20, 0x20, "Percentage" )    PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, "Dynamic" )
	PORT_DIPSETTING(    0x00, "Actual" )
	PORT_DIPNAME( 0x40, 0x40, "Hopper" )        PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, "Disabled" )
	PORT_DIPSETTING(    0x00, "Enabled" )
	PORT_DIPNAME( 0x80, 0x80, "If RAM Error" )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, "Play" )
	PORT_DIPSETTING(    0x00, "Freeze" )
INPUT_PORTS_END

static INPUT_PORTS_START( reelfun )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x01, "Coinage Multiplier" )    PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x07, "7" )
	PORT_DIPSETTING(    0x06, "6" )
	PORT_DIPSETTING(    0x05, "5" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, "Credits per Coin" )
	PORT_DIPSETTING(    0x00, "Coins per Credit" )
	PORT_DIPNAME( 0x10, 0x10, "Screen" )        PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, "Horizontal" )
	PORT_DIPSETTING(    0x00, "Vertical" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Cabinet ) )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_INCLUDE(reelfun_standard)
INPUT_PORTS_END

static INPUT_PORTS_START( findout )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x01, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x07, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x08, 0x00, "Game Repetition" )   PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x10, "Orientation" )   PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, "Horizontal" )
	PORT_DIPSETTING(    0x00, "Vertical" )
	PORT_DIPNAME( 0x20, 0x20, "Buy Letter" )    PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, "Starting Letter" )   PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "Bonus Letter" )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_INCLUDE(reelfun_standard)
INPUT_PORTS_END

static INPUT_PORTS_START(sexappl)
	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x01, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x07, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Orientation" )   PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, "Horizontal" )
	PORT_DIPSETTING(    0x00, "Vertical" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Cabinet ) )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:7") // Shows Message #1 and "hangs" ???
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_INCLUDE(trivia_standard)
INPUT_PORTS_END

static INPUT_PORTS_START(bigjoke)
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Orientation" )   PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, "Horizontal" )
	PORT_DIPSETTING(    0x00, "Vertical" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Cabinet ) )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_INCLUDE(trivia_standard)
INPUT_PORTS_END

static INPUT_PORTS_START( gt103 )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x01, "Coinage Multiplier" )    PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x07, "7" )
	PORT_DIPSETTING(    0x06, "6" )
	PORT_DIPSETTING(    0x05, "5" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, "Credits per Coin" )
	PORT_DIPSETTING(    0x00, "Coins per Credit" )
	PORT_DIPNAME( 0x10, 0x10, "Screen" )        PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, "Horizontal" )
	PORT_DIPSETTING(    0x00, "Vertical" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Cabinet ) )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_INCLUDE(trivia_standard)
INPUT_PORTS_END

static INPUT_PORTS_START( quiz )
	PORT_INCLUDE( getrivia )

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) // no tickets
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no coin 2
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no tickets
INPUT_PORTS_END

static INPUT_PORTS_START( gt507uk )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x00, "If Ram Error" )
	PORT_DIPSETTING(    0x01, "Freeze" )
	PORT_DIPSETTING(    0x00, "Play" )
	PORT_DIPNAME( 0x0a, 0x08, "Payout" )
	PORT_DIPSETTING(    0x0a, "Bank" )
	PORT_DIPSETTING(    0x08, "N/A" )
	PORT_DIPSETTING(    0x02, "Credit" )
	PORT_DIPSETTING(    0x00, "Direct" )
	PORT_DIPNAME( 0x04, 0x04, "Payout Hardware" )
	PORT_DIPSETTING(    0x04, "Hopper" )
	PORT_DIPSETTING(    0x00, "Solenoid" )
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

	PORT_INCLUDE(trivia_standard)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2) // coin 3, 2, 4 order verified in test mode
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_IMPULSE(2)
INPUT_PORTS_END

static INPUT_PORTS_START(geimulti)
	PORT_INCLUDE(gselect)

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x0f, 0x09, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_7C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START(sprtauth)
	PORT_INCLUDE(trivia_standard)

	PORT_START("DSWA")
	PORT_DIPNAME( 0x0f, 0x09, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_7C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


INTERRUPT_GEN_MEMBER(gei_state::vblank_irq)
{
	if (m_nmi_mask)
		device.execute().pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}


void gei_state::getrivia(machine_config &config)
{
	Z80(config, m_maincpu, 4'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &gei_state::getrivia_map);
	m_maincpu->set_vblank_int("screen", FUNC(gei_state::vblank_irq));

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_screen_update(FUNC(gei_state::screen_update));
	m_screen->set_size(512, 256);
	m_screen->set_visarea(48, 511-48, 16, 255-16);
	m_screen->set_palette("palette");

	PALETTE(config, "palette", palette_device::GBR_3BIT);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	I8255A(config, m_ppi[0]);
	m_ppi[0]->in_pa_callback().set_ioport("DSWA");
	m_ppi[0]->in_pb_callback().set_ioport("IN0");
	m_ppi[0]->out_pc_callback().set(FUNC(gei_state::sound_w));

	I8255A(config, m_ppi[1]);
	m_ppi[1]->in_pa_callback().set_ioport("IN1");
	m_ppi[1]->out_pb_callback().set(FUNC(gei_state::lamps_w));
	m_ppi[1]->out_pc_callback().set(FUNC(gei_state::lamps2_w));

	TICKET_DISPENSER(config, m_ticket, attotime::from_msec(100));

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.99);
}

void gei_state::findout(machine_config &config)
{
	getrivia(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &gei_state::findout_map);

	m_ppi[1]->in_pc_callback().set(FUNC(gei_state::portc_r));
	m_ppi[1]->out_pc_callback().set_nop();
}

void gei_state::quizvid(machine_config &config)
{
	findout(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &gei_state::quizvid_map);

	PALETTE(config.replace(), "palette", palette_device::GRB_3BIT);
}

void gei_state::gselect(machine_config &config)
{
	getrivia(config);

	// basic machine hardware

	config.device_remove("ticket");

	m_maincpu->set_addrmap(AS_PROGRAM, &gei_state::gselect_map);

	m_ppi[0]->out_pc_callback().set(FUNC(gei_state::sound2_w));

	m_ppi[1]->in_pc_callback().set_ioport("IN2");
	m_ppi[1]->out_pc_callback().set(FUNC(gei_state::nmi_w));
}

void gei_state::jokpokera(machine_config &config)
{
	getrivia(config);

	// basic machine hardware

	config.device_remove("ticket");

	m_maincpu->set_addrmap(AS_PROGRAM, &gei_state::gselect_map);
}

void gei_state::amuse(machine_config &config)
{
	getrivia(config);

	// basic machine hardware

	m_maincpu->set_addrmap(AS_PROGRAM, &gei_state::amuse_map);
}

void gei_state::gepoker(machine_config &config)
{
	getrivia(config);

	// basic machine hardware

	m_maincpu->set_addrmap(AS_PROGRAM, &gei_state::gepoker_map);
}

void gei_state::amuse1(machine_config &config)
{
	getrivia(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &gei_state::amuse1_map);
}

void gei_state::suprpokr(machine_config &config)
{
	getrivia(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &gei_state::suprpokr_map);
}

void gei_state::sprtauth(machine_config &config)
{
	getrivia(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &gei_state::sprtauth_map);
}


/***************************************************
ROM board is UVM-1A

Contains:
 3 2732  EPROMs (Program Code, 1 empty socket)
 2 X2212P (RAM chips, no battery backup)
 DM7408N

PCB labeled M075

****************************************************/

ROM_START( jokpoker )
	ROM_REGION( 0x24000, "maincpu", 0 )
	ROM_LOAD( "m075.1", 0x00000, 0x1000, CRC(ad42465b) SHA1(3f06847a9aecb0592f99419dba9be5f18005d57b) ) // ROM board UMV-1A
	ROM_LOAD( "m075.2", 0x01000, 0x1000, CRC(bd129fc2) SHA1(2e05ba34922c16d127be32941447013efea05bcd) )
	ROM_LOAD( "m075.3", 0x02000, 0x1000, CRC(45725bc9) SHA1(9e6dcbec955ef8190f2307ddb367b24b7f34338d) )
ROM_END


ROM_START( jokpokera ) // UMV-7C ROM board
	ROM_REGION( 0x24000, "maincpu", 0 )
	ROM_LOAD( "jpbiwr930-1.bin", 0x00000, 0x2000, CRC(d0f4fec5) SHA1(5fcc72522df66464759d5ba3d5209bc7a3a80002) ) // 16.03BI 5-10-85
	ROM_LOAD( "jpbiwr930-2.bin", 0x02000, 0x2000, CRC(824d1aee) SHA1(6eebde351c3b5bbed3796846d8d651b41ed6f84a) ) // Joker Poker ICB 9-30-86
ROM_END

ROM_START( jokpokerb ) // UVM 7S REV A ROM board
	ROM_REGION( 0x24000, "maincpu", 0 )
	ROM_LOAD( "jp_bi_10-19-88.e1", 0x00000, 0x2000, CRC(d59a78e2) SHA1(d8463675f30a52e0f93c5ea5c2ee663095d3d5ea) ) // 16.04BI 10-19-88
	ROM_LOAD( "jp_bi_10-19-88.e2", 0x02000, 0x2000, CRC(1a34dc80) SHA1(27dff743e661ae7421fef0b046e3ae205b842603) ) // Joker Poker ICB 9-30-86
ROM_END

ROM_START( jokpokerc ) // UMV-7C ROM board
	ROM_REGION( 0x24000, "maincpu", 0 )
	ROM_LOAD( "jp_bi_5-10-85.1", 0x00000, 0x2000, CRC(d0f4fec5) SHA1(5fcc72522df66464759d5ba3d5209bc7a3a80002) ) // 16.03BI 5-10-85
	ROM_LOAD( "jp_bi_5-10-85.2", 0x02000, 0x2000, CRC(9f8bee22) SHA1(8d894d2a07bd18d731b7a54a16bb9b9230c79306) ) // Poker No Raise ICB 9-30-86
ROM_END

/***************************************************
ROM board is UVM-1B

Contains:
 4 2732  EPROMs (Program Code)
 Battery (3.5V lithium battery) backed up NEC 8444XF301
 DM7408N

****************************************************/

ROM_START( superbwl )
	ROM_REGION( 0x24000, "maincpu", 0 )
	ROM_LOAD( "super_bowl.1", 0x00000, 0x1000, CRC(82edf064) SHA1(8a26377590282f51fb39d013452ba11252e7dd05) ) // ROM board UMV-1B
	ROM_LOAD( "super_bowl.2", 0x01000, 0x1000, CRC(2438dd1f) SHA1(26bbd1cb3d0d5b93f61b92ff95948ac9de060715) )
	ROM_LOAD( "super_bowl.3", 0x02000, 0x1000, CRC(9b111430) SHA1(9aaa755f3e4b369477c1a0525c994a19fe0f6107) )
	ROM_LOAD( "super_bowl.4", 0x03000, 0x1000, CRC(037cad42) SHA1(d4037a28bb49b31358b5d560e5e028d958ae2bc9) )
ROM_END

/***************************************************
ROM board is UVM 10 B

Contains:
 2 2764  EPROMs (Program Code)
 5 27128 EPROMs (Question ROMs)
 Battery (3V lithium battery) backed up HM6117P-4
 SN74LS374
 MMI PAL10L8


Sets will be listed by "series" - the program code version
 is not as important as maintaining the correct questions
 sets as per known series
Missing sets will be filled as dumped, as question ROMs
 are interchangeable, operators did their own swaps

Note: Question ROMs that contain "#1" (or 2 ect)
      are corrected ROMs (spelling and / or answers)

****************************************************/

ROM_START( gtsers1 ) // Series 1 (Complete)
	ROM_REGION( 0x24000, "maincpu", 0 )
	ROM_LOAD( "prog101c_right", 0x00000, 0x2000, CRC(767f0e46) SHA1(5de7b54876fcbfb2328174ffe6b656ffea886fcb) ) // ROM board UMV 10 B
	ROM_LOAD( "prog101c_left",  0x0a000, 0x2000, CRC(24c0a097) SHA1(b8de58baecb92775e0882cd6eca3b9e07cf7c5a5) )
	// Question ROMs
	ROM_LOAD( "science_@1",    0x10000, 0x4000, CRC(68259e09) SHA1(29e848b4744b767c51ff81a756fba7bf96daefec) )
	ROM_LOAD( "general_@1",    0x14000, 0x4000, CRC(25a0ef9d) SHA1(793abd779cc237e14933933747bbf27bbcbfcd32) )
	ROM_LOAD( "sports_@1",     0x18000, 0x4000, CRC(cb1744f5) SHA1(ea3f7bfcecf5c58c26aa0f34908ba5d54f7279ec) )
	ROM_LOAD( "music_@1",      0x1c000, 0x4000, CRC(1b546857) SHA1(31e04bb5016e8ef6dc48f9b3ddaeab5fe04f91c2) )
	ROM_LOAD( "movies-tv_@1",  0x20000, 0x4000, CRC(e9a55dad) SHA1(c87682e72bad3507b24eb6a52b4e430e0bfcdab6) )
ROM_END

ROM_START( gtsers2 ) // Series 2 (Complete - question ROMs dated 2/9)
	ROM_REGION( 0x24000, "maincpu", 0 )
	ROM_LOAD( "prog101c_right", 0x00000, 0x2000, CRC(767f0e46) SHA1(5de7b54876fcbfb2328174ffe6b656ffea886fcb) ) // ROM board UMV 10 B
	ROM_LOAD( "prog101c_left",  0x0a000, 0x2000, CRC(24c0a097) SHA1(b8de58baecb92775e0882cd6eca3b9e07cf7c5a5) )
	// Question ROMs
	ROM_LOAD( "tv_mash",          0x10000, 0x4000, CRC(a86990fc) SHA1(6a11b038d48bb97feb4857546349ed93ea1f9273) )
	ROM_LOAD( "general_2",        0x14000, 0x4000, CRC(5798f2b3) SHA1(0636017969d9b1eac5d33cfb18cb36f7cf4cba88) )
	ROM_LOAD( "sports_2_@2",      0x18000, 0x4000, CRC(fb632622) SHA1(c14d8178f5cfc5994e2ab4f829e353fa75b57304) )
	ROM_LOAD( "comics_@1",        0x1c000, 0x4000, CRC(8c5cd561) SHA1(1ca566acf72ce636b1b34ee6b7cafb9584340bcc) )
	ROM_LOAD( "entertainment_@1", 0x20000, 0x4000, CRC(cd3ce4c7) SHA1(4bd121fa5899a96b015605f84179ed82be0a25f3) ) // Correct spelling of "Acapella"
ROM_END

ROM_START( gtsers3 ) // Series 3 (Complete - question ROMs dated 3/9)
	ROM_REGION( 0x24000, "maincpu", 0 )
	ROM_LOAD( "prog102b_right",   0x00000, 0x2000, CRC(e8391f71) SHA1(a955eff87d622d4fcfd25f6d888c48ff82556879) )
	ROM_LOAD( "prog102b_left",    0x0a000, 0x2000, CRC(cc7b45a7) SHA1(c708f56feb36c1241358a42bb7dce25b799f1f0b) )
	// Question ROMs
	ROM_LOAD( "beatlemania_@1", 0x10000, 0x4000, CRC(c35ab539) SHA1(aa7c9b532aeb289b71c179e6ff1cc5b63dbe240c) )
	ROM_LOAD( "general_3",      0x14000, 0x4000, CRC(a60f17a4) SHA1(0d79be9e2e49b9817e94d410e25bb6dcda10aa9e) )
	ROM_LOAD( "sports_3_@3",    0x18000, 0x4000, CRC(b22cec38) SHA1(a416c3de9749fda3ab5ae5841304da0cef900cbf) )
	ROM_LOAD( "country-west",   0x1c000, 0x4000, CRC(3227c475) SHA1(d07ad4876122223fe7ab3f21781e0d847332ea5c) )
	ROM_LOAD( "tv_soaps",       0x20000, 0x4000, CRC(26914f3a) SHA1(aec380cea14d6acb71986f3d65c7620b16c174ae) )
ROM_END

ROM_START( gtsers4 ) // Series 4 (Incomplete - question ROMs dated 4/9)
	ROM_REGION( 0x24000, "maincpu", 0 )
	ROM_LOAD( "prog102c_right", 0x00000, 0x2000, CRC(76fdc3a3) SHA1(212e09644b9cab334aad22ec5860e8638c6ba3fa) )
	ROM_LOAD( "prog102c_left",  0x0a000, 0x2000, CRC(901fb2f9) SHA1(98e49c74d89c4911a1f4d5ccf3e6cf3226c6a178) )
	// Question ROMs
	ROM_LOAD( "history-geog",   0x10000, 0x4000, CRC(76d6b026) SHA1(613809b247cb27773631a1bb34af485c2b1bd486) )
	ROM_LOAD( "star_trek",      0x14000, 0x4000, CRC(19764e00) SHA1(d7ed577dba02776ac58e8f34b833ed07679c0af1) )
	ROM_LOAD( "television_@1",  0x18000, 0x4000, CRC(0f646389) SHA1(23fefe2e6cc26767d52604e7ab15bb4db99a6e94) )
	// Missing "baseball"
	// Missing "hollywood"
ROM_END

ROM_START( gtsers5 ) // Series 5 (Incomplete - question ROMs dated 5/9)
	ROM_REGION( 0x24000, "maincpu", 0 )
	ROM_LOAD( "prog102c_right", 0x00000, 0x2000, CRC(76fdc3a3) SHA1(212e09644b9cab334aad22ec5860e8638c6ba3fa) )
	ROM_LOAD( "prog102c_left",  0x0a000, 0x2000, CRC(901fb2f9) SHA1(98e49c74d89c4911a1f4d5ccf3e6cf3226c6a178) )
	// Question ROMs
	ROM_LOAD( "james_bond",    0x10000, 0x4000, CRC(fe9fadfd) SHA1(44b3fee1f14148f47b0b40600aabd5bff9b65e85) )
	ROM_LOAD( "hockey",        0x14000, 0x4000, CRC(4874a431) SHA1(f3c11dfbf71d101aa1a6cd3622b282a4ebe4664b) )
	// Missing "the_states"
	// Missing "wild_west"
	// Missing "elvismania"
ROM_END

ROM_START( gtsers7 ) // Series 7 (Complete - question ROMs dated 7/9)
	ROM_REGION( 0x24000, "maincpu", 0 )
	ROM_LOAD( "prog102c_right", 0x00000, 0x2000, CRC(76fdc3a3) SHA1(212e09644b9cab334aad22ec5860e8638c6ba3fa) )
	ROM_LOAD( "prog102c_left",  0x0a000, 0x2000, CRC(901fb2f9) SHA1(98e49c74d89c4911a1f4d5ccf3e6cf3226c6a178) )
	// Question ROMs
	ROM_LOAD( "t.v._dallas", 0x10000, 0x4000, CRC(7540453b) SHA1(9e215d7504ddf57d32d513b4cf87e7306915d1a4) )
	ROM_LOAD( "general_v",   0x14000, 0x4000, CRC(81bf07c7) SHA1(a53f050b4ef8ffc0499b50224d4bbed4af0ca09c) )
	ROM_LOAD( "kids_korner", 0x18000, 0x4000, CRC(66631b79) SHA1(ec534941add7113c9bb96d00f2e09834275e314b) )
	ROM_LOAD( "good_guys",   0x1c000, 0x4000, CRC(4d638326) SHA1(2d6d00ae7f02d1607f37eb1cefae31c42797b2cf) )
	ROM_LOAD( "biblical",    0x20000, 0x4000, CRC(3372692d) SHA1(b45a2d22d4d90eb947ed9d3469ef4328640d69a8) )
ROM_END

ROM_START( gtsers7a ) // Series 7 (Complete - question ROMs dated 7/9)
	ROM_REGION( 0x24000, "maincpu", 0 )
	ROM_LOAD( "prog102c_right", 0x00000, 0x2000, CRC(76fdc3a3) SHA1(212e09644b9cab334aad22ec5860e8638c6ba3fa) )
	ROM_LOAD( "prog102c_left",  0x0a000, 0x2000, CRC(901fb2f9) SHA1(98e49c74d89c4911a1f4d5ccf3e6cf3226c6a178) )
	// Question ROMs
	ROM_LOAD( "t.v._dallas", 0x10000, 0x4000, CRC(7540453b) SHA1(9e215d7504ddf57d32d513b4cf87e7306915d1a4) )
	ROM_LOAD( "general_v",   0x14000, 0x4000, CRC(81bf07c7) SHA1(a53f050b4ef8ffc0499b50224d4bbed4af0ca09c) )
	ROM_LOAD( "kids_korner", 0x18000, 0x4000, CRC(66631b79) SHA1(ec534941add7113c9bb96d00f2e09834275e314b) )
	ROM_LOAD( "good_guys",   0x1c000, 0x4000, CRC(4d638326) SHA1(2d6d00ae7f02d1607f37eb1cefae31c42797b2cf) )
	ROM_LOAD( "sex_triv",    0x20000, 0x4000, CRC(cd0ce4e2) SHA1(2046ee3da94f00bf4a8b3fc62b1190d58e83cc89) ) // Listed as an alternate question set
ROM_END

ROM_START( gtsersa ) // alt or older version questions
	ROM_REGION( 0x24000, "maincpu", 0 )
	ROM_LOAD( "prog102c_right", 0x00000, 0x2000, CRC(76fdc3a3) SHA1(212e09644b9cab334aad22ec5860e8638c6ba3fa) )
	ROM_LOAD( "prog102c_left",  0x0a000, 0x2000, CRC(901fb2f9) SHA1(98e49c74d89c4911a1f4d5ccf3e6cf3226c6a178) )
	// Question ROMs
	ROM_LOAD( "sports",               0x10000, 0x4000, CRC(9b4a17b6) SHA1(1b5358b5bc83c2817ecfa4e277fa351a679d5023) ) // Series 1 question
	ROM_LOAD( "entertainment_@1_old", 0x14000, 0x4000, CRC(2bffb3b4) SHA1(5947ebd708df35cefa86608392909c16b25d0710) ) // Dated 2/9 - Spells "Acapella" as "Cappella"
	ROM_LOAD( "sports_2",             0x18000, 0x4000, CRC(e8f8e168) SHA1(d2bc57dc0799dd8817b15857f17c4d7ee4d9f932) ) // Dated 2/9
	ROM_LOAD( "comics",               0x1c000, 0x4000, CRC(7efdfe8f) SHA1(ec255777c61677ca32c49b9da5e85e07c0647e5f) ) // Dated 2/9
	ROM_LOAD( "entertainment",        0x20000, 0x4000, CRC(b670b9e8) SHA1(0d2246fcc6c753694bc9bd1fc05ac439f24059ef) ) // Dated 2/9
ROM_END

ROM_START( gtsersb ) // alt or older version questions
	ROM_REGION( 0x24000, "maincpu", 0 )
	ROM_LOAD( "prog102c_right", 0x00000, 0x2000, CRC(76fdc3a3) SHA1(212e09644b9cab334aad22ec5860e8638c6ba3fa) )
	ROM_LOAD( "prog102c_left",  0x0a000, 0x2000, CRC(901fb2f9) SHA1(98e49c74d89c4911a1f4d5ccf3e6cf3226c6a178) )
	// Question ROMs
	ROM_LOAD( "beatlemania_old", 0x10000, 0x4000, CRC(cb241960) SHA1(e560b776b2cb5fd29d1663fffdf68f4427d674a9) ) // Dated 3/9
	ROM_LOAD( "sports_3_old",    0x14000, 0x4000, CRC(5986996c) SHA1(56432c15a3b0204ed527c18e24716f17bb52dc4e) ) // Dated 3/9
	ROM_LOAD( "television_old",  0x18000, 0x4000, CRC(413f34c8) SHA1(318f6b464449bf3f0c43c4210a667190c774eb67) ) // Dated 4/9
	ROM_LOAD( "facts_of_life",   0x1c000, 0x4000, CRC(1668c7bf) SHA1(6bf43de26f8a626560579ab75fd0890fe00f99dd) ) // Unknown series question set
ROM_END

ROM_START( sextriv1 )
	ROM_REGION( 0x24000, "maincpu", 0 )
	ROM_LOAD( "prog1_right",   0x00000, 0x2000, CRC(73abcd12) SHA1(3b985f25a11507878cef6d11420e215065fb0906) )
	ROM_LOAD( "prog1_left",    0x0a000, 0x2000, CRC(04ee6ecd) SHA1(28342fcdcf36b34fa93f1a985163ca5aab03defe) )
	// Question ROMs
	ROM_LOAD( "adult_sex",    0x10000, 0x4000, CRC(509a8183) SHA1(635c784860e423b22aaea94abc53c1d9477cb1df) )
	ROM_LOAD( "arousing_sex", 0x14000, 0x4000, CRC(1dbf4578) SHA1(51a548d5fe59739e62b5f0e9e6ebc7deb8656210) )
	ROM_LOAD( "intimate_sex", 0x18000, 0x4000, CRC(1f46b626) SHA1(04aa5306c69d130e0f84fa390a773e73c06e5e9b) )
	ROM_LOAD( "sizzling_sex", 0x1c000, 0x4000, CRC(c718833d) SHA1(02ea341e56554dd9302fe95f45dcf446a2978917) )
	ROM_LOAD( "novelty_sex",  0x20000, 0x4000, CRC(26603979) SHA1(78061741e5224b3162be51e637a2fbb9a48962a3) )
ROM_END

ROM_START( sextriv2 )
	ROM_REGION( 0x24000, "maincpu", 0 )
	ROM_LOAD( "prog1_right",     0x00000, 0x2000, CRC(73abcd12) SHA1(3b985f25a11507878cef6d11420e215065fb0906) )
	ROM_LOAD( "prog1_left",      0x0a000, 0x2000, CRC(04ee6ecd) SHA1(28342fcdcf36b34fa93f1a985163ca5aab03defe) )
	// Question ROMs
	ROM_LOAD( "sex_triv",        0x10000, 0x4000, CRC(cd0ce4e2) SHA1(2046ee3da94f00bf4a8b3fc62b1190d58e83cc89) )
	ROM_LOAD( "general_sex",     0x14000, 0x4000, CRC(36fed946) SHA1(25d445ab6cb4b6f41a1dd7104ee1141e597b2e9e) )
	ROM_LOAD( "educational_sex", 0x18000, 0x4000, CRC(281cbe03) SHA1(9c3900cd2535f942a5cbae7edd46ac0170e04c52) )
ROM_END

/***************************************************
ROM board is UVM-4B

Contains 5 2764 EPROMs, MMI PAL16R4CN

Battery (3V lithium battery) backed up HM6117P-4

ROMs labeled as:

4/1  at spot 1
BLJK at spot 2
POKR at spot 3
SPRD at spot 4
SLOT at spot 3

Alt set included BONE in place of SPRD & a newer SLOT

These board sets may also be known as the V116 (or V16)
sets as the alt set also included that name on the labels

To clear remaining credits:
  Press STAND, DEAL and 3rd DISCARD buttons at the same time.
  Hold all 3 buttons down for 5 seconds.  The screen will
   turn RED indicating a successful "CLEAR" occurred

****************************************************/

ROM_START( gs4002 )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "4-1.1",          0x00000, 0x2000, CRC(a456e456) SHA1(f36b96ac31ce0f128ecb94f94d1dbdd88ee03c77) ) // V16M 5-4-84
	// Banked ROMs
	ROM_LOAD( "bljk_3-16-84.2", 0x10000, 0x2000, CRC(c3785523) SHA1(090f324fc7adb0a36b189cf04086f0e050895ee4) )
	ROM_LOAD( "pokr_5-16-84.3", 0x12000, 0x2000, CRC(f0e99cc5) SHA1(02fdc95974e503b6627930918fcc3c029a7a4612) )
	ROM_LOAD( "sprd_1-24-84.4", 0x14000, 0x2000, CRC(5fe90ed4) SHA1(38db69567d9c38f78127e581fdf924aca4926378) )
	ROM_LOAD( "slot_1-24-84.5", 0x16000, 0x2000, CRC(cd7cfa4c) SHA1(aa3de086e5a1018b9e5a18403a6144a6b0ed1036) )
ROM_END

ROM_START( gs4002a )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "4-1.1",          0x00000, 0x2000, CRC(a456e456) SHA1(f36b96ac31ce0f128ecb94f94d1dbdd88ee03c77) ) // V16M 5-4-84
	// Banked ROMs
	ROM_LOAD( "bljk_3-16-84.2", 0x10000, 0x2000, CRC(c3785523) SHA1(090f324fc7adb0a36b189cf04086f0e050895ee4) )
	ROM_LOAD( "pokr_5-16-84.3", 0x12000, 0x2000, CRC(f0e99cc5) SHA1(02fdc95974e503b6627930918fcc3c029a7a4612) )
	ROM_LOAD( "bone_5-16-84.4", 0x14000, 0x2000, CRC(eccd2fb0) SHA1(2683e432ffcca4280c31f57b2596e4389bc59b7b) )
	ROM_LOAD( "slot_9-24-84.5", 0x16000, 0x2000, CRC(25d8c504) SHA1(2d52b66e8a1f06f486015440668bd924a123dad0) )
ROM_END


/*
Greyhound Poker board...

Standard Greyhound Electronics Inc UV-1B mainboard.

ROM board UVM 10 B or UMV 10 C

Battery backed up NEC D449C RAM
PAL16R4
74L374

ROMs in this order on the UMV 10 C board:

Label        Normally in the slot
--------------------------------
High
Control
ROM1         Joker Poker
ROM2         Black jack
ROM3         Rolling Bones
ROM4         Casino Slots
ROM5         Horse Race

For UMV 10 B: 1C, 2C, 1, 2, 3, 4, & 5

There looks to be several types and combos for these, some are "ICB" or "IAM"
It also looks like operators mixed & matched to upgrade (some times incorrectly)
their boards.  Sets will be filled in as found and dumped.

There are some versions, like, the ICB sets that use 2764 ROMs for all ROMs. While the IAM set uses
27128 ROMs for all ROMs.  These ROMs are the correct size, but it's currently not known if the ROM
board (UVM 10 B/C) "sees" them as 27128 or the standard size of 2764.

Dumped, but not known to be supported by any High/Control combo:
ROM_LOAD( "rollingbones_am_3-16-84",  0x16000, 0x4000, CRC(41879e9b) SHA1(5106d5772bf43b28817e27efd16c785359cd929e) ) // Might work with IAM control, once it gets figured out

The ICB set is known as the M105 set as some label sets included that name.

*/

ROM_START( gepoker ) // v50.02 with most ROMs for ICB dated 8-16-84
	ROM_REGION( 0x1b000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "control_icb_8-16",    0x00000, 0x2000, CRC(0103963d) SHA1(9bc646e721048b84111e0686eaca23bc24eee3e2) )
	ROM_LOAD( "high_icb_6-25-85-5",  0x0e000, 0x2000, CRC(dfb6592e) SHA1(d68de9f537d3c14279dc576424d195bb266e3897) )
	// Banked ROMs
	ROM_LOAD( "jokerpoker_icb_8-16-84",    0x10000, 0x2000, CRC(0834a1e6) SHA1(663e6f4e0586eb9b84d3098aef8c596585c27304) )
	ROM_LOAD( "blackjack_icb_8-16-84",     0x12000, 0x2000, CRC(cff27ffd) SHA1(fd85b54400b2f22ae92042b01a2c162e64d2d066) )
	ROM_LOAD( "rollingbones_icb_8-16-84",  0x14000, 0x2000, CRC(52d66cb6) SHA1(57db34906fcafd37f3a361df209dafe080aeac16) )
	ROM_LOAD( "casinoslots_icb_8-16-84",   0x16000, 0x2000, CRC(3db002a3) SHA1(7dff4efceee37b25328303cf0606bf4baa4df5f3) )
	ROM_LOAD( "horserace_icb_3-19-85",     0x18000, 0x2000, CRC(f1e6e61e) SHA1(944b1ab4af911e5ed136f1fca3c44219726eeebb) )
ROM_END

ROM_START( gepoker1 ) // v50.02 with ROMs for ICB dated 9-30-86
	ROM_REGION( 0x1b000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "control_icb_9-30",    0x00000, 0x2000, CRC(08b996f2) SHA1(5f5efb5015ec9571cc94734c18debfadaa28f585) )
	ROM_LOAD( "high_icb_6-25-85-5a", 0x0e000, 0x2000, CRC(6ddc1750) SHA1(ee19206b7f4a98e3e7647414127f4e09b3e9134f) )
	// Banked ROMs
	ROM_LOAD( "jokerpoker_icb_9-30-86",    0x10000, 0x2000, CRC(a1473367) SHA1(9b37ccafc02704e8f1d61150326494e86148d84e) )
	ROM_LOAD( "blackjack_icb_9-30-86",     0x12000, 0x2000, CRC(82804184) SHA1(2e2e6a80c99c8eb226dc54c1d32d0bf24de300a4) )
	ROM_LOAD( "casinoslots_icb_9-30-86",   0x14000, 0x2000, CRC(713c3963) SHA1(a9297c04fc44522ca6891516a2c744712132896a) )
	ROM_LOAD( "beatthespread_icb_9-30-86", 0x16000, 0x2000, CRC(93654d2a) SHA1(3aa5a54b91867c03182e93a7f1607545503a33f7) )
	ROM_LOAD( "instantbingo_t24_10-07-86", 0x18000, 0x2000, CRC(de87ed0a) SHA1(4a26d93368c1a39dd38aabe450c34203101f0ef7) ) // Found with this set, is it compatible or an operator swap?
ROM_END

ROM_START( gepoker2 ) // v50.02 with control dated 9-30-84
	ROM_REGION( 0x1b000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "control_icb_9-30",  0x00000, 0x2000, CRC(08b996f2) SHA1(5f5efb5015ec9571cc94734c18debfadaa28f585) )
	ROM_LOAD( "high_icb_6-25a",    0x0e000, 0x2000, CRC(6ddc1750) SHA1(ee19206b7f4a98e3e7647414127f4e09b3e9134f) )
	// Banked ROMs
	ROM_LOAD( "jokerpoker_cb_10-19-88",    0x10000, 0x2000, CRC(a590af75) SHA1(63bc64fbc9ac0c489b1f4894d77a4be13d7251e7) )
	ROM_LOAD( "horserace_icb_1-1-87",      0x12000, 0x2000, CRC(6d5092e3) SHA1(ef99d1b858aef3c438c61c2b17e371dc6aca6623) )
ROM_END

ROM_START( gepoker3 ) // v50.02 with control dated 9-30-84
	ROM_REGION( 0x1b000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "cont_9-30_m105_pts.2c",  0x00000, 0x2000, CRC(08b996f2) SHA1(5f5efb5015ec9571cc94734c18debfadaa28f585) )
	ROM_LOAD( "hrom_6-25_m105_pts.1c",  0x0e000, 0x2000, CRC(6ddc1750) SHA1(ee19206b7f4a98e3e7647414127f4e09b3e9134f) )
	// Banked ROMs
	ROM_LOAD( "pokr_chig_2-12_m105.1",  0x10000, 0x2000, CRC(a1cbf67b) SHA1(a6cd081bbb19b2dd1a84b7750eac8a5258a663eb) )//not original sticker, twice the size of a regular ROM, but still don't match
	ROM_CONTINUE( 0x10000, 0x2000) // Discarding 1nd half, 0xff filled
	ROM_LOAD( "bljk_9-30_m105_pts.2",   0x12000, 0x2000, CRC(82804184) SHA1(2e2e6a80c99c8eb226dc54c1d32d0bf24de300a4) )
	ROM_LOAD( "bone_8-16_m105_pts.3",   0x14000, 0x2000, CRC(52d66cb6) SHA1(57db34906fcafd37f3a361df209dafe080aeac16) )
	ROM_LOAD( "slot_9-30_m105_pts.4",   0x16000, 0x2000, CRC(713c3963) SHA1(a9297c04fc44522ca6891516a2c744712132896a) )
	ROM_LOAD( "bingo_8-16_m105.5",      0x18000, 0x2000, CRC(de87ed0a) SHA1(4a26d93368c1a39dd38aabe450c34203101f0ef7) ) //not original sticker selftest report 10-7-86 date!!
ROM_END

ROM_START( amuse ) // v50.08 with most ROMs for IAM dated 8-16-84
	ROM_REGION( 0x24000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "control_iam_8-16",  0x00000, 0x4000, CRC(345434b9) SHA1(ec880f6f5e90aa971937e0270701e323f6a83671) ) // all ROMs were 27128, twice the size of other sets
	ROM_LOAD( "high_iam_8-16",     0x08000, 0x4000, CRC(57000fb7) SHA1(144723eb528c4816b9aa4b0ba77d2723c6442546) ) // Is only the 1st half used by the board / program?
	// Banked ROMs
	ROM_LOAD( "jokerpoker_iam_8-16-84",    0x10000, 0x4000, CRC(33794a87) SHA1(2b46809623713582746d9f817db33077f15a3684) ) // This set is verified correct by 3 different sets checked
	ROM_LOAD( "blackjack_iam_8-16-84",     0x14000, 0x4000, CRC(6e10b5b8) SHA1(5dc294b4a562193a99b0d307323fcc084a053426) )
	ROM_LOAD( "rollingbones_iam_8-16-84",  0x18000, 0x4000, CRC(26949774) SHA1(20571b955521ec3929430249aa651cee8a97043d) )
	ROM_LOAD( "casinoslots_iam_8-16-84",   0x1c000, 0x4000, CRC(c5a1eec6) SHA1(43d31bfe4cbbb6b86f52f675f513050866443176) )
	ROM_LOAD( "horserace_iam_3-19-84",     0x20000, 0x4000, CRC(7b9e75cb) SHA1(0db8da6f5f59f57886766bec96102d43796567ef) )
ROM_END

ROM_START( amuse1 ) // V30.08  ROM board UMV8-B
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "m108_control_5-16-84",  0x00000, 0x4000, CRC(9dda922f) SHA1(ded42adda8376452e0ac4f771ebb42fd86811dc5) )
	ROM_LOAD( "m108_hcon_5-16-84",     0x05800, 0x1000, CRC(ade5c42d) SHA1(d3e32ecaeb21a4e5eedd243c42d5914b03f572bd) )
	// Banked ROMs
	ROM_LOAD( "jokerpoker_amba_5-16-84",   0x10000, 0x4000, CRC(530b22d7) SHA1(418d41820429d0f8e054d383c502f4d75505ddca) )
	ROM_LOAD( "rollingbones_am_5-16-84",   0x14000, 0x4000, CRC(60496f5a) SHA1(9f9fd4191eaaa12350dbcc9d414306454b2adfba) )
	ROM_LOAD( "beatthespread_am2p_3-16-84",0x18000, 0x4000, CRC(40997230) SHA1(49e92a9f371a9839c94aa923aa5883284dae9dc2) )
ROM_END

/*

ROM board is "GEI, inc UVM-8B" with a date code of "8339"

Contains 1 AM2732A EPROM, 5 HN4827128G EPROMs, 1 MMI PAL16R4CN, 1 7474N

Battery (3V lithium battery) backed up HM6117P-4

ROMs labeled as:

HCON M108 9/30 at spot C (AM2732A-DC)
CONT M108 9/26 at spot 1 (HN4827128G-25)
POKR M108 9/26 at spot 2 (HN4827128G-30)
BLJK M108 9/26 at spot 3 (HN4827128G-25)
BONE M108 9/26 at spot 4 (HN4827128G-30)
SLOT M108 9/26 at spot 5 (HN4827128G-30)

*/

ROM_START( amuse1a )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "cont m108 9_26.1", 0x00000, 0x4000, CRC(122d235b) SHA1(5647e7f50cc4c18d98e8f0517ca2f0dacf57cae7) )
	ROM_LOAD( "hcon m108 9_30.c", 0x05800, 0x1000, CRC(5beb3d8b) SHA1(40a9458444f6a8e763a0374ab74e745500f4bf8a) )
	// Banked ROMs
	ROM_LOAD( "pokr m108 9_26.2", 0x10000, 0x4000, CRC(eafa1e22) SHA1(714b82ce2034c88b79d45a691dd71c975f91078c) )
	ROM_LOAD( "bljk m108 9_26.3", 0x14000, 0x4000, CRC(c31a8b89) SHA1(487a3be9b5f3db3388de03ebc5f4a3f1572df19b) )
	ROM_LOAD( "bone m108 9_26.4", 0x18000, 0x4000, CRC(40307a55) SHA1(3b276aa3ee6e8b25d1840d131db8d5dca34fe856) )
	ROM_LOAD( "slot m108 9_26.5", 0x1c000, 0x4000, CRC(fbcc8942) SHA1(fc9ff6db84906edb1dfa2b0235d5cfe9d0a637ab) )
ROM_END


ROM_START( suprpokr ) // Super Poker Version 10.19S BOBC. ROM board UMV-7C
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "supr_pokr_10.19s_@1", 0x00000, 0x4000, CRC(50662b4d) SHA1(967161a755db43d2cfd5ce92e14c5284f1f1f8ad) )
	ROM_LOAD( "supr_pokr_10.19s_@2", 0x08000, 0x4000, CRC(22b45aeb) SHA1(006c3072cc44c6fde9b4d15163dc70707bbd5a9c) )
	ROM_RELOAD(                      0x0c000, 0x4000 )

	ROM_REGION( 0x0800, "nvram", 0 )
	ROM_LOAD( "suprpokr.nvram",  0x0000, 0x0800, CRC(0e716e23) SHA1(15e35bfbab9a6778834df1c85c25643010054cdb) )

	ROM_REGION( 0x0008, "signature", 0 )
	ROM_LOAD( "suprpokr.sig",   0x0000, 0x0008, CRC(8f622afe) SHA1(8c1c8cee444c88211760a0f5c3adcfd887da5bb7) )
ROM_END

ROM_START( suprpokra ) // Super Poker Version 10.15S BOBC. ROM board UMV-7C
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "supr_pokr_10.15s_@1", 0x00000, 0x4000, CRC(5cc7c1e0) SHA1(1cdca32c4df7227dab77574abe344b291741139e) )
	ROM_LOAD( "supr_pokr_10.15s_@2", 0x08000, 0x4000, CRC(e47d6e2a) SHA1(9cabc42275dad8be6cd5b167e381ddb5bf08276d) )
	ROM_RELOAD(                      0x0c000, 0x4000 )

	ROM_REGION( 0x0800, "nvram", 0 )
	ROM_LOAD( "suprpokr.nvram",  0x0000, 0x0800, CRC(0e716e23) SHA1(15e35bfbab9a6778834df1c85c25643010054cdb) )

	ROM_REGION( 0x0008, "signature", 0 )
	ROM_LOAD( "suprpokr.sig",   0x0000, 0x0008, CRC(8f622afe) SHA1(8c1c8cee444c88211760a0f5c3adcfd887da5bb7) )
ROM_END

ROM_START( suprpokrb ) // Super Poker Version 10.10 BOBC. ROM board UMV-7C
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "supr_pokr_10.10_@1", 0x00000, 0x4000, CRC(8324471f) SHA1(c38b7a735ef06feea3e8d4ba6dd963e24d38c792) )
	ROM_LOAD( "supr_pokr_10.10_@2", 0x08000, 0x4000, CRC(a82ca9c5) SHA1(3b0f4ad7d53370dc1f00dec696e993359147a496) )
	ROM_RELOAD(                     0x0c000, 0x4000 )

	ROM_REGION( 0x0800, "nvram", 0 )
	ROM_LOAD( "suprpokr.nvram",  0x0000, 0x0800, CRC(0e716e23) SHA1(15e35bfbab9a6778834df1c85c25643010054cdb) )

	ROM_REGION( 0x0008, "signature", 0 )
	ROM_LOAD( "suprpokr.sig",   0x0000, 0x0008, CRC(8f622afe) SHA1(8c1c8cee444c88211760a0f5c3adcfd887da5bb7) )
ROM_END


ROM_START( reelfun ) // v7.03, From a TRIV3D ROM board - T4 question ROMs
	ROM_REGION( 0x38000, "maincpu", 0 )
	ROM_LOAD( "reelfun_v7.03.cnt",    0x00000, 0x4000, CRC(ce42e0ea) SHA1(87f703b14aa819c54e54b42e639448521c01f76b) )
	ROM_LOAD( "reelfun_v7.01.prg",    0x08000, 0x2000, CRC(615d846a) SHA1(ffa1c47393f4f364aa34d14cf3ac2f56d9eaecb0) ) // banked
	ROM_LOAD( "reelfun-1-title",      0x10000, 0x8000, CRC(0e165fbc) SHA1(a3a5b7db72ab86efe973f649f5dfe5133830e3fc) ) // banked ROMs for solution data
	ROM_LOAD( "reelfun-2-place",      0x18000, 0x8000, CRC(a0066bfd) SHA1(b6f031ab50eb396be79e79e06f2101400683ec3e) )
	ROM_LOAD( "reelfun-3-phrase",     0x20000, 0x8000, CRC(199e36b0) SHA1(d9dfe39c9a4fca1169150f8941f8ebc499dfbaf5) )
	ROM_LOAD( "reelfun-4-person",     0x28000, 0x8000, CRC(49b0710b) SHA1(a38b3251bcb8683d43bdb903036970140a9735e6) )
	ROM_LOAD( "reelfun-5-song_title", 0x30000, 0x8000, CRC(cce01c45) SHA1(c484f5828928edf39335cedf21acab0b9e2a6881) )

	ROM_REGION( 0x0800, "nvram", 0 )
	ROM_LOAD( "reelfun.nvram",   0x0000, 0x0800, CRC(fbb791ce) SHA1(0db77cbc42b5362b3d2ecde46a4289619e8f59a6) ) // Defaults

	ROM_REGION( 0x0008, "signature", 0 ) // bytes 0x03 through 0x0a of each question ROM - to prevent ROM swaps
	ROM_LOAD( "reelfun.sig",   0x0000, 0x0008, CRC(c8e944a3) SHA1(d34de9e3163ba61fa4e4f2264caff40434fcc9b0) ) // Same signature for all T4 ROM sets?
ROM_END

ROM_START( reelfun1 ) // v7.01, From a TRIV3D ROM board - T4 question ROMs
	ROM_REGION( 0x38000, "maincpu", 0 )
	ROM_LOAD( "reelfun_v7.01.cnt",    0x00000, 0x4000, CRC(d9d1e92b) SHA1(337f66a37b3734d565b3ff3d912e0f690fd7c445) )
	ROM_LOAD( "reelfun_v7.01.prg",    0x08000, 0x2000, CRC(615d846a) SHA1(ffa1c47393f4f364aa34d14cf3ac2f56d9eaecb0) ) // banked
	ROM_LOAD( "reelfun-1-title",      0x10000, 0x8000, CRC(0e165fbc) SHA1(a3a5b7db72ab86efe973f649f5dfe5133830e3fc) ) // banked ROMs for solution data
	ROM_LOAD( "reelfun-2-place",      0x18000, 0x8000, CRC(a0066bfd) SHA1(b6f031ab50eb396be79e79e06f2101400683ec3e) )
	ROM_LOAD( "reelfun-3-phrase",     0x20000, 0x8000, CRC(199e36b0) SHA1(d9dfe39c9a4fca1169150f8941f8ebc499dfbaf5) )
	ROM_LOAD( "reelfun-4-person",     0x28000, 0x8000, CRC(49b0710b) SHA1(a38b3251bcb8683d43bdb903036970140a9735e6) )
	ROM_LOAD( "reelfun-5-song_title", 0x30000, 0x8000, CRC(cce01c45) SHA1(c484f5828928edf39335cedf21acab0b9e2a6881) )

	ROM_REGION( 0x0800, "nvram", 0 )
	ROM_LOAD( "reelfun.nvram",   0x0000, 0x0800, CRC(fbb791ce) SHA1(0db77cbc42b5362b3d2ecde46a4289619e8f59a6) ) // Defaults

	ROM_REGION( 0x0008, "signature", 0 ) // bytes 0x03 through 0x0a of each question ROM - to prevent ROM swaps
	ROM_LOAD( "reelfun.sig",   0x0000, 0x0008, CRC(c8e944a3) SHA1(d34de9e3163ba61fa4e4f2264caff40434fcc9b0) ) // Same signature for all T4 ROM sets?
ROM_END

ROM_START( reelfun0 ) // v7.00, From a TRIV3D ROM board - T4 question ROMs
	ROM_REGION( 0x38000, "maincpu", 0 )
	ROM_LOAD( "reelfun_v7.00.cnt",    0x00000, 0x4000, CRC(5d3e87e5) SHA1(bdc0b980011d1b92fd27b03dbe31301dffdcc7f6) )
	ROM_LOAD( "reelfun_v7.00.prg",    0x08000, 0x2000, CRC(09c23777) SHA1(d46ac6df7d64bdf917821e4a1b7a135bc3459b71) ) // banked
	ROM_LOAD( "reelfun-1-title",      0x10000, 0x8000, CRC(0e165fbc) SHA1(a3a5b7db72ab86efe973f649f5dfe5133830e3fc) ) // banked ROMs for solution data
	ROM_LOAD( "reelfun-2-place",      0x18000, 0x8000, CRC(a0066bfd) SHA1(b6f031ab50eb396be79e79e06f2101400683ec3e) )
	ROM_LOAD( "reelfun-3-phrase",     0x20000, 0x8000, CRC(199e36b0) SHA1(d9dfe39c9a4fca1169150f8941f8ebc499dfbaf5) )
	ROM_LOAD( "reelfun-4-person",     0x28000, 0x8000, CRC(49b0710b) SHA1(a38b3251bcb8683d43bdb903036970140a9735e6) )
	ROM_LOAD( "reelfun-5-song_title", 0x30000, 0x8000, CRC(cce01c45) SHA1(c484f5828928edf39335cedf21acab0b9e2a6881) )

	ROM_REGION( 0x0800, "nvram", 0 )
	ROM_LOAD( "reelfun.nvram",   0x0000, 0x0800, CRC(fbb791ce) SHA1(0db77cbc42b5362b3d2ecde46a4289619e8f59a6) ) // Defaults

	ROM_REGION( 0x0008, "signature", 0 ) // bytes 0x03 through 0x0a of each question ROM - to prevent ROM swaps
	ROM_LOAD( "reelfun.sig",   0x0000, 0x0008, CRC(c8e944a3) SHA1(d34de9e3163ba61fa4e4f2264caff40434fcc9b0) ) // Same signature for all T4 ROM sets?
ROM_END

ROM_START( findout ) // From a TRIV3D ROM board - T4 question ROMs
	ROM_REGION( 0x38000, "maincpu", 0 )
	ROM_LOAD( "12.bin",       0x00000, 0x4000, CRC(21132d4c) SHA1(e3562ee2f46b3f022a852a0e0b1c8fb8164f64a3) )
	ROM_LOAD( "11.bin",       0x08000, 0x2000, CRC(0014282c) SHA1(c6792f2ff712ba3759ff009950d78750df844d01) ) // banked
	ROM_LOAD( "13.bin",       0x10000, 0x8000, CRC(cea91a13) SHA1(ad3b395ab0362f3decf178824b1feb10b6335bb3) ) // banked ROMs for solution data
	ROM_LOAD( "14.bin",       0x18000, 0x8000, CRC(2a433a40) SHA1(4132d81256db940789a40aa1162bf1b3997cb23f) )
	ROM_LOAD( "15.bin",       0x20000, 0x8000, CRC(d817b31e) SHA1(11e6e1042ee548ce2080127611ce3516a0528ae0) )
	ROM_LOAD( "16.bin",       0x28000, 0x8000, CRC(143f9ac8) SHA1(4411e8ba853d7d5c032115ce23453362ab82e9bb) )
	ROM_LOAD( "17.bin",       0x30000, 0x8000, CRC(dd743bc7) SHA1(63f7e01ac5cda76a1d3390b6b83f4429b7d3b781) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s147.bin",   0x0000, 0x0200, CRC(f3b663bb) SHA1(5a683951c8d3a2baac4b49e379d6e10e35465c8a) ) // unknown

	ROM_REGION( 0x0800, "nvram", 0 )
	ROM_LOAD( "findout.nvram",   0x0000, 0x0800, CRC(b6950ffc) SHA1(ecc08904b208fb87c7a47d2f6081652405bf73b4) ) // Defaults

	ROM_REGION( 0x0008, "signature", 0 ) // bytes 0x03 through 0x0a of each question ROM - to prevent ROM swaps
	ROM_LOAD( "findout.sig",   0x0000, 0x0008, CRC(c8e944a3) SHA1(d34de9e3163ba61fa4e4f2264caff40434fcc9b0) ) // Same signature for all T4 ROM sets?
ROM_END

/*
findouta's 0.bin has 2 bytes changed compared to 12.bin in findout:

findout:  12.bin  0x1bd3:37 & 0x3fff:28
findouta: 0.bin   0x1bd3:39 & 0x3fff:26

Changes the copyright year from 1987 to 1989 and balances out the checksum

*/
ROM_START( findouta ) // PCB marked: 100686A QUIZ 1986 LATO COMP  (copy of a Greyhound PCB)
	ROM_REGION( 0x38000, "maincpu", 0 )
	ROM_LOAD( "0",          0x00000, 0x4000, CRC(4d1306a0) SHA1(cc403c280a2cefbe943e6e0e4d7ee42e339a274d) )
	ROM_LOAD( "rom",        0x08000, 0x2000, CRC(0014282c) SHA1(c6792f2ff712ba3759ff009950d78750df844d01) ) // banked
	ROM_LOAD( "sport.1",    0x10000, 0x2000, CRC(6139b2d7) SHA1(1aa5b25a7807806f6018cedf91f95f9172750867) ) // banked ROMs for solution data
	ROM_LOAD( "basket.2",   0x18000, 0x2000, CRC(7cc369e0) SHA1(b589707c8186a661f6565a5ae164d7169b9493d8) ) // smaller question ROMs - HH482764G
	ROM_LOAD( "cinema.3",   0x20000, 0x2000, CRC(65d78d41) SHA1(200ffb837dce352349842c37e0a3ebd8234ad254) )
	ROM_LOAD( "musica.4",   0x28000, 0x2000, CRC(a063f7bf) SHA1(15c82b188e730a89eead0d4c74c916a58733be66) )
	ROM_LOAD( "calcio_a.5", 0x30000, 0x2000, CRC(60839c64) SHA1(5269e76f890b285768417f8e58609162c913c271) )

	ROM_REGION( 0x0e00, "proms", 0 ) // unknown, all on main PCB
	ROM_LOAD( "am27s29.6f", 0x0000, 0x0200, CRC(19e3f161) SHA1(52da3c1e50c2329454de14cb9c46149e573e562b) )
	ROM_LOAD( "am27s13.4g", 0x0200, 0x0200, CRC(71df3345) SHA1(3d64c47b2ce093afad56d8963c151a7854451236) ) // all 6 from here on have identical contents
	ROM_LOAD( "am27s13.4l", 0x0400, 0x0200, CRC(71df3345) SHA1(3d64c47b2ce093afad56d8963c151a7854451236) )
	ROM_LOAD( "am27s13.4n", 0x0600, 0x0200, CRC(71df3345) SHA1(3d64c47b2ce093afad56d8963c151a7854451236) )
	ROM_LOAD( "am27s13.9g", 0x0800, 0x0200, CRC(71df3345) SHA1(3d64c47b2ce093afad56d8963c151a7854451236) )
	ROM_LOAD( "am27s13.9l", 0x0a00, 0x0200, CRC(71df3345) SHA1(3d64c47b2ce093afad56d8963c151a7854451236) )
	ROM_LOAD( "am27s13.9n", 0x0c00, 0x0200, CRC(71df3345) SHA1(3d64c47b2ce093afad56d8963c151a7854451236) )

	ROM_REGION( 0x0300, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "pal10l8cn",     0x0000, 0x002c, CRC(338cb46a) SHA1(2e0872da385fc13ee800f1cfcd17bca734420ea9) ) // on ROMs PCB
	ROM_LOAD( "pal10h8cn.2o",  0x0030, 0x002c, CRC(24d06342) SHA1(9b2392a99587958db6e037e02c0b713ef530dad5) ) // on main PCB
	ROM_LOAD( "pal10l8cn.10c", 0x0060, 0x002c, NO_DUMP ) // on main PCB, hardware error
	ROM_LOAD( "pal10l8cn.9d",  0x0090, 0x002c, NO_DUMP ) // on ROMs PCB, read protected
	ROM_LOAD( "pal10l8cn.10d", 0x00c0, 0x002c, NO_DUMP ) // on ROMs PCB, read protected
	ROM_LOAD( "pal16r8acn",    0x0100, 0x0104, NO_DUMP ) // on ROMs PCB, read protected

	ROM_REGION( 0x0800, "nvram", 0 )
	ROM_LOAD( "findouta.nvram", 0x0000, 0x0800, CRC(a0e19083) SHA1(76ebba4627667904e618d9fac71c415e852d7c24) ) // Defaults

	ROM_REGION( 0x0008, "signature", 0 ) // bytes 0x03 through 0x0a of each question ROM - to prevent ROM swaps
	ROM_LOAD( "findouta.sig",  0x0000, 0x0008, CRC(5c126c66) SHA1(12dbda545b2b2bf4ad9152e37e6316c6cd49a4e5) )
ROM_END

ROM_START( gt507uk ) // All question ROMs used here are "T3" ROMs
	ROM_REGION( 0x38000, "maincpu", 0 )
	ROM_LOAD( "triv_3_2.bin",    0x00000, 0x4000, CRC(2d72a081) SHA1(8aa32acf335d027466799b097e0de66bcf13247f) )
	ROM_LOAD( "rom_ad.bin",      0x08000, 0x2000, CRC(c81cc847) SHA1(057b7b75a2fe1abf88b23e7b2de230d9f96139f5) )
	ROM_LOAD( "aerospace",       0x10000, 0x8000, CRC(cb555d46) SHA1(559ae05160d7893ff96311a2177eba039a4cf186) ) // Also found in Series #11 set
	ROM_LOAD( "english_sport_4", 0x18000, 0x8000, CRC(6ae8a63d) SHA1(c6018141d8bbe0ed7619980bf7da89dd91d7fcc2) )
	ROM_LOAD( "general_facts",   0x20000, 0x8000, CRC(f921f108) SHA1(fd72282df5cee0e6ab55268b40785b3dc8e3d65b) ) // Also found in Series #11 set
	ROM_LOAD( "horrors",         0x28000, 0x8000, CRC(5f7b262a) SHA1(047480d6bf5c6d0603d538b84c996bd226f07f77) ) // Also found in Series #14 set
	ROM_LOAD( "pop_music",       0x30000, 0x8000, CRC(884fec7c) SHA1(b389216c17f516df4e15eee46246719dd4acb587) )
ROM_END

ROM_START( gtsers8 ) // TRIV-3 PCB, stickered 256 TRIV #8 4/85
	ROM_REGION( 0x38000, "maincpu", 0 )
	ROM_LOAD( "prog1_versionc",  0x00000, 0x4000, CRC(340246a4) SHA1(d655e1cf2b1e87a05e87ff6af4b794e6d54a2a52) )
	ROM_LOAD( "science",         0x10000, 0x8000, CRC(2f940ebd) SHA1(bead4988ac0a97d70f2a3c0b40a05968436de2ed) )
	ROM_LOAD( "general",         0x18000, 0x8000, CRC(1efa01c3) SHA1(801ef5ab55184e488b08ef99ebd641ea4f7edb24) )
	ROM_LOAD( "sports",          0x20000, 0x8000, CRC(6bd1ba9a) SHA1(7caac1bd438a9b1d11fb33e11814b5d76951211a) )
	ROM_LOAD( "entertainment+",  0x28000, 0x8000, CRC(07068c9f) SHA1(1aedc78d071281ec8b08488cd82655d41a77cf6b) ) // Labeled as ENTR 2*
	ROM_LOAD( "adult_sex",       0x30000, 0x8000, CRC(bc8ea9c3) SHA1(6aa4c5468508a50843d3f40b320fc06149fdd292) )
ROM_END

ROM_START( gtsers8a ) // TRIV-3 PCB, stickered 256 TRIV #8 4/85
	ROM_REGION( 0x38000, "maincpu", 0 )
	ROM_LOAD( "prog1_versionc",  0x00000, 0x4000, CRC(340246a4) SHA1(d655e1cf2b1e87a05e87ff6af4b794e6d54a2a52) )
	ROM_LOAD( "science",         0x10000, 0x8000, CRC(2f940ebd) SHA1(bead4988ac0a97d70f2a3c0b40a05968436de2ed) )
	ROM_LOAD( "general",         0x18000, 0x8000, CRC(1efa01c3) SHA1(801ef5ab55184e488b08ef99ebd641ea4f7edb24) )
	ROM_LOAD( "sports",          0x20000, 0x8000, CRC(6bd1ba9a) SHA1(7caac1bd438a9b1d11fb33e11814b5d76951211a) )
	ROM_LOAD( "entertainment+",  0x28000, 0x8000, CRC(07068c9f) SHA1(1aedc78d071281ec8b08488cd82655d41a77cf6b) ) // Labeled as ENTR 2*
	ROM_LOAD( "potpourri",       0x30000, 0x8000, CRC(f2968a28) SHA1(87c08c59dfee71e7bf071f09c3017c750a1c5694) ) // Listed as an alternate question set
ROM_END

ROM_START( gtsers9 ) // TRIV-3 PCB, stickered 256 TRIV #9 7/85
	ROM_REGION( 0x38000, "maincpu", 0 )
	ROM_LOAD( "prog1_versionc",  0x00000, 0x4000, CRC(340246a4) SHA1(d655e1cf2b1e87a05e87ff6af4b794e6d54a2a52) )
	ROM_LOAD( "facts",           0x10000, 0x8000, CRC(21bd6181) SHA1(609ae1097a4011e90d03d4c4f03140fbe84c084a) )
	ROM_LOAD( "rock-n-roll+",    0x18000, 0x8000, CRC(8eb83052) SHA1(93e3c1ae6c2048fb44ecafe1013b6a96da38fa84) ) // Labeled as ROCK-N-ROL*
	ROM_LOAD( "television+",     0x20000, 0x8000, CRC(731d4cc0) SHA1(184b6e48edda24f50e377a473a1a4709a218181b) ) // Labeled as T.V.*
	ROM_LOAD( "usa_trivia+",     0x28000, 0x8000, CRC(829543b4) SHA1(deb0a4132852643ad884cf194b0a2e6671aa2b4e) ) // Labeled as USA TRV*
	ROM_LOAD( "adult_sex_2+",    0x30000, 0x8000, CRC(0d683f21) SHA1(f47ce3c31c4c5ed02247fa280303e6ae760315df) ) // Listed as an alternate question set - Labeled as ADULT SEX 2*
	// Missing "Artists-Athletes"
ROM_END

ROM_START( gtsers10 ) // TRIV-3 PCB, stickered 256 TRIV #10 8/85
	ROM_REGION( 0x38000, "maincpu", 0 )
	ROM_LOAD( "prog1_versionc", 0x00000, 0x4000, CRC(340246a4) SHA1(d655e1cf2b1e87a05e87ff6af4b794e6d54a2a52) ) // Also found with program v5.03 (not dumped)
	ROM_LOAD( "new_general",    0x10000, 0x8000, CRC(ba1f5b92) SHA1(7e94be0ef6904331d3a6b266e5887e9a15c5e7f9) )
	ROM_LOAD( "new_tv_mash",    0x18000, 0x8000, CRC(f73240c6) SHA1(78020644074da719414133a86a91c1328e5d8929) )
	ROM_LOAD( "new_entrtnmnt",  0x20000, 0x8000, CRC(0f54340c) SHA1(1ca4c23b542339791a2d8f4a9a857f755feca8a1) )
	ROM_LOAD( "new_sports",     0x28000, 0x8000, CRC(19eff1a3) SHA1(8e024ae6cc572176c90d819a438ace7b2512dbf2) )
	ROM_LOAD( "adult_sex_3+",   0x30000, 0x8000, CRC(2c46e355) SHA1(387ab389abaaea8e870b00039dd884237f7dd9c6) ) // Listed as an alternate question set - Labeled as ADULT SEX 3*
	// Missing "new_science"
ROM_END

ROM_START( gtsers11 ) // TRIV-3 PCB, stickered 256 TRIV #11 8/85
	ROM_REGION( 0x38000, "maincpu", 0 )
	ROM_LOAD( "prog1_versionc", 0x00000, 0x4000, CRC(340246a4) SHA1(d655e1cf2b1e87a05e87ff6af4b794e6d54a2a52) )
	ROM_LOAD( "rich-famous",    0x10000, 0x8000, CRC(39e07e4a) SHA1(6e5a0bcefaa1169f313e8818cf50919108b3e121) )
	ROM_LOAD( "cars-women",     0x18000, 0x8000, CRC(4c5dd1df) SHA1(f3e2146eeab07ec71617c7614c6e8f6bc844e6e3) )
	ROM_LOAD( "aerospace",      0x20000, 0x8000, CRC(cb555d46) SHA1(559ae05160d7893ff96311a2177eba039a4cf186) )
	ROM_LOAD( "tv_music",       0x28000, 0x8000, CRC(5138e0fb) SHA1(102146d63752258c2fda95df49289c42b392c838) )
	ROM_LOAD( "general_facts",  0x30000, 0x8000, CRC(f921f108) SHA1(fd72282df5cee0e6ab55268b40785b3dc8e3d65b) )
ROM_END

ROM_START( gtsers11a ) // TRIV-3 PCB, stickered 256 TRIV #11 8/85
	ROM_REGION( 0x38000, "maincpu", 0 )
	ROM_LOAD( "prog1_versionc", 0x00000, 0x4000, CRC(340246a4) SHA1(d655e1cf2b1e87a05e87ff6af4b794e6d54a2a52) )
	ROM_LOAD( "rich-famous",    0x10000, 0x8000, CRC(39e07e4a) SHA1(6e5a0bcefaa1169f313e8818cf50919108b3e121) )
	ROM_LOAD( "cars-women",     0x18000, 0x8000, CRC(4c5dd1df) SHA1(f3e2146eeab07ec71617c7614c6e8f6bc844e6e3) )
	ROM_LOAD( "aerospace",      0x20000, 0x8000, CRC(cb555d46) SHA1(559ae05160d7893ff96311a2177eba039a4cf186) )
	ROM_LOAD( "tv_music",       0x28000, 0x8000, CRC(5138e0fb) SHA1(102146d63752258c2fda95df49289c42b392c838) )
	ROM_LOAD( "gay_times",      0x30000, 0x8000, CRC(c4f9a8cf) SHA1(9247ecc5708aba263e0365fc43a1a7d0c2b7c391) ) // Listed as an alternate question set
ROM_END

/*
The question ROM PCB below was labeled a month later than the other series 11 sets. Maybe a "re-release" to replace the possibly
  less popular Gay Times question ROM with a more popular Adult Sex 3 question ROM.
NOTE: Adult Sex 3, according to documentation is a series 10 question ROM, but two ROMs were clearly labeled as: ADLT SEX 3* #11
*/
ROM_START( gtsers11b ) // TRIV-3 PCB, stickered 256 TRIV #11 9/85
	ROM_REGION( 0x38000, "maincpu", 0 )
	ROM_LOAD( "prog1_versionc", 0x00000, 0x4000, CRC(340246a4) SHA1(d655e1cf2b1e87a05e87ff6af4b794e6d54a2a52) )
	ROM_LOAD( "rich-famous",    0x10000, 0x8000, CRC(39e07e4a) SHA1(6e5a0bcefaa1169f313e8818cf50919108b3e121) )
	ROM_LOAD( "cars-women",     0x18000, 0x8000, CRC(4c5dd1df) SHA1(f3e2146eeab07ec71617c7614c6e8f6bc844e6e3) )
	ROM_LOAD( "aerospace",      0x20000, 0x8000, CRC(cb555d46) SHA1(559ae05160d7893ff96311a2177eba039a4cf186) )
	ROM_LOAD( "tv_music",       0x28000, 0x8000, CRC(5138e0fb) SHA1(102146d63752258c2fda95df49289c42b392c838) )
	ROM_LOAD( "adult_sex_3+",   0x30000, 0x8000, CRC(2c46e355) SHA1(387ab389abaaea8e870b00039dd884237f7dd9c6) ) // Labeled as ADULT SEX 3*
ROM_END

ROM_START( gtsers12 ) // TRIV-3 PCB
	ROM_REGION( 0x38000, "maincpu", 0 )
	ROM_LOAD( "program_v5.03",   0x00000, 0x4000, CRC(888b7d9b) SHA1(e5ed4f22bff99c26cd6ef9a06cb386221e84bbf5) )
	ROM_LOAD( "new_science_2+",  0x10000, 0x8000, CRC(3bd80fb8) SHA1(9a196595bc5dc6ed5ee5853786839ed4847fa436) ) // Labeled as NEW SCNE 2*
	ROM_LOAD( "cops_+_robbers+", 0x18000, 0x8000, CRC(5176751a) SHA1(fbf0aeceeedb8a93c12920fecf6268893b393541) ) // Labeled as COPS ROBR*
	ROM_LOAD( "rock_music",      0x20000, 0x8000, CRC(7f11733a) SHA1(d4d0dee75518edf986cb1241ade45ccb4840f088) )
	ROM_LOAD( "famous_quotes",   0x28000, 0x8000, CRC(0a27d8ae) SHA1(427e6ae25e47da7f7f7c3e92a37e330d711da90c) )
	ROM_LOAD( "vices",           0x30000, 0x8000, CRC(e6069955) SHA1(68f7453f21a4ce1be912141bbe947fbd81d918a3) )
ROM_END

ROM_START( gtsers12a ) // TRIV-3 PCB
	ROM_REGION( 0x38000, "maincpu", 0 )
	ROM_LOAD( "program_v5.03",   0x00000, 0x4000, CRC(888b7d9b) SHA1(e5ed4f22bff99c26cd6ef9a06cb386221e84bbf5) )
	ROM_LOAD( "n.f.l._football", 0x10000, 0x8000, CRC(d676b7cd) SHA1(d652d2441adb500f7af526d110d0335ea453d75b) ) // undocumented series 12 question ROM
	ROM_LOAD( "new_science_2+",  0x18000, 0x8000, CRC(3bd80fb8) SHA1(9a196595bc5dc6ed5ee5853786839ed4847fa436) ) // Labeled as NEW SCNE 2*
	ROM_LOAD( "famous_quotes",   0x20000, 0x8000, CRC(0a27d8ae) SHA1(427e6ae25e47da7f7f7c3e92a37e330d711da90c) )
	ROM_LOAD( "vices",           0x28000, 0x8000, CRC(e6069955) SHA1(68f7453f21a4ce1be912141bbe947fbd81d918a3) )
	ROM_LOAD( "adult_sex_4+",    0x30000, 0x8000, CRC(9c32730e) SHA1(9d060e49a4c1dd8d978619b1c357c9e8238e5c96) ) // Listed as an alternate question set - Labeled as ADULT SEX 4*
ROM_END

// Trivia: There is no Series 13 question set.  ;-)

ROM_START( gtsers14 ) // TRIV-3 PCB, stickered 256 TRIV #14 11/85
	ROM_REGION( 0x38000, "maincpu", 0 )
	ROM_LOAD( "program5",        0x00000, 0x4000, CRC(99ddaaa7) SHA1(c929d7b67367f303dadf07d508bc72af19e9c2ef) ) // Unknown version, but earlier then the 5.06 for Series #15
	ROM_LOAD( "famous_couples",  0x10000, 0x8000, CRC(e0618218) SHA1(ff64fcd6dec83a2271b63c3ae64dc932a3954ec5) )
	ROM_LOAD( "the_sixties",     0x18000, 0x8000, CRC(8cfa854e) SHA1(81428c12f99841db1c61b471ac8d00f0c411883b) )
	ROM_LOAD( "tv_comedies",     0x20000, 0x8000, CRC(992ae38e) SHA1(312780d651a85a1c433f587ff2ede579456d3fd9) )
	ROM_LOAD( "horrors",         0x28000, 0x8000, CRC(5f7b262a) SHA1(047480d6bf5c6d0603d538b84c996bd226f07f77) )
	ROM_LOAD( "war_and_peace",   0x30000, 0x8000, CRC(bc709383) SHA1(2fba4c80773abea7bbd826c39378b821cddaa255) )
ROM_END

ROM_START( gtsers14a ) // TRIV-3 PCB, stickered 256 TRIV #14 11/85
	ROM_REGION( 0x38000, "maincpu", 0 )
	ROM_LOAD( "program5",        0x00000, 0x4000, CRC(99ddaaa7) SHA1(c929d7b67367f303dadf07d508bc72af19e9c2ef) ) // Unknown version, but earlier then the 5.06 for Series #15
	ROM_LOAD( "famous_couples",  0x10000, 0x8000, CRC(e0618218) SHA1(ff64fcd6dec83a2271b63c3ae64dc932a3954ec5) )
	ROM_LOAD( "the_sixties",     0x18000, 0x8000, CRC(8cfa854e) SHA1(81428c12f99841db1c61b471ac8d00f0c411883b) )
	ROM_LOAD( "tv_comedies",     0x20000, 0x8000, CRC(992ae38e) SHA1(312780d651a85a1c433f587ff2ede579456d3fd9) )
	ROM_LOAD( "horrors",         0x28000, 0x8000, CRC(5f7b262a) SHA1(047480d6bf5c6d0603d538b84c996bd226f07f77) )
	ROM_LOAD( "adult_sex_5",     0x30000, 0x8000, CRC(fdbc3729) SHA1(7cb7cec4439ddc39de2f7f62c25623cfb869f493) ) // Listed as an alternate question set
ROM_END

ROM_START( gtsers15 ) // v5.06, From a TRIV3D ROM board
	ROM_REGION( 0x38000, "maincpu", 0 )
	ROM_LOAD( "program_v5.06",   0x00000, 0x4000, CRC(e9d6226c) SHA1(42e62c5cafa3f051bf48c18c8c549ffcd4c766c5) )
	ROM_LOAD( "entertainment_2", 0x10000, 0x8000, CRC(c75c2331) SHA1(9c5947616a4cba2623c599def6cf3b2b1981b681) )
	ROM_LOAD( "facts_2",         0x18000, 0x8000, CRC(7836ef31) SHA1(6a84cfa39de392eed46a4b37752e00b6d094bbd6) )
	ROM_LOAD( "new_science_3",   0x20000, 0x8000, CRC(fcbc3bc3) SHA1(2dbdd39dce9dbf53c0954dec44a4f5109243dc60) )
	ROM_LOAD( "nfl_football",    0x28000, 0x8000, CRC(42eb2849) SHA1(c24e681a508ef8350f7e5d50aea2c31cf70ce5c9) )
	ROM_LOAD( "adult_sex_6",     0x30000, 0x8000, CRC(d66f35f7) SHA1(81b56756230b27b0903d0c5df30439726526afe2) ) // Listed as an alternate question set
	/* Missing "the_seventies" */

	ROM_REGION( 0x0400, "pld", 0 ) // probably one of the two GALs provides the "signature"
	ROM_LOAD( "gal16v8",   0x0000, 0x0117, NO_DUMP ) // read protected
	ROM_LOAD( "gal18v8",   0x0200, 0x0117, NO_DUMP ) // read protected

	ROM_REGION( 0x0008, "signature", 0 ) // bytes 0x03 through 0x0a of each question ROM - to prevent ROM swaps
	ROM_LOAD( "gtsers15.sig",   0x0000, 0x0008, CRC(c8e944a3) SHA1(d34de9e3163ba61fa4e4f2264caff40434fcc9b0) ) // Same signature for all T4 ROM sets?
ROM_END

ROM_START( gtsers18 ) // v5.06, From a TRIV3D ROM board
	ROM_REGION( 0x38000, "maincpu", 0 )
	ROM_LOAD( "program_v5.06",   0x00000, 0x4000, CRC(e9d6226c) SHA1(42e62c5cafa3f051bf48c18c8c549ffcd4c766c5) )
	ROM_LOAD( "entertainment_3", 0x10000, 0x8000, CRC(577161c4) SHA1(0bbe5a030aa5b34414346613016e6164cddc7032) )
	ROM_LOAD( "more_sports",     0x18000, 0x8000, CRC(1350777e) SHA1(97bda88a4340a3dc4b9edda0386a7f649e65939f) )
	ROM_LOAD( "super_trivia",    0x20000, 0x8000, CRC(178461ce) SHA1(dfed10c744c7b6d0ee0cc10f4a9851be9a0a72c4) )
	ROM_LOAD( "this_is_music",   0x28000, 0x8000, CRC(64eaf91a) SHA1(c7ca9012e00bd46758fd4942c5f882f9e6152090) )
	ROM_LOAD( "adult_sex_9",     0x30000, 0x8000, CRC(83c8839c) SHA1(9a4641204bfa23d98674a36ad6b957d1a559c87b) ) /* Listed as an alternate question set */
	/* Missing "world_geography" */

	ROM_REGION( 0x0400, "pld", 0 ) // probably one of the two GALs provides the "signature"
	ROM_LOAD( "gal16v8",   0x0000, 0x0117, NO_DUMP ) // read protected
	ROM_LOAD( "gal18v8",   0x0200, 0x0117, NO_DUMP ) // read protected

	ROM_REGION( 0x0008, "signature", 0 ) // bytes 0x03 through 0x0a of each question ROM - to prevent ROM swaps
	ROM_LOAD( "gtsers18.sig",   0x0000, 0x0008, CRC(c8e944a3) SHA1(d34de9e3163ba61fa4e4f2264caff40434fcc9b0) ) // Same signature for all T4 ROM sets?
ROM_END

ROM_START( gt103a1 ) // Need to verify which series or sets these belong to
	ROM_REGION( 0x38000, "maincpu", 0 )
	ROM_LOAD( "prog1_versiona",  0x00000, 0x4000, CRC(537d6566) SHA1(282a33e4a9fc54d34094393c00026bf31ccd6ab5) )
	ROM_LOAD( "history-geog",    0x10000, 0x8000, CRC(c9a70fc3) SHA1(4021e5d702844416e8c798ed0a57c9ecd20b1d4b) ) // T3 question ROM, NOT from series 17
	ROM_LOAD( "soccer",          0x18000, 0x8000, CRC(f821f860) SHA1(b0437ef5d31c507c6499c1fb732d2ba3b9beb151) ) // T3 question ROM - Likely a UK question ROM
ROM_END

ROM_START( gt103aa ) // Series 8 questions ROMs. Need to verify proper revision level for most of these.
	ROM_REGION( 0x38000, "maincpu", 0 )
	ROM_LOAD( "t_3a-8_1.bin",      0x00000, 0x4000, CRC(02aef306) SHA1(1ffc10c79a55d41ea36bcaab13cb3f02cb3f9712) ) // "Park" alternate version sets here
	ROM_LOAD( "entertainment_old", 0x10000, 0x8000, CRC(9a6628b9) SHA1(c0cb7e974329d4d5b91f107296d21a674e35a51b) )
	ROM_LOAD( "general_alt",       0x18000, 0x8000, CRC(df34f7f9) SHA1(329d123eea711d5135dc02dd7b89b220ce8ddd28) )
	ROM_LOAD( "science_alt",       0x20000, 0x8000, CRC(9eaebd18) SHA1(3a4d787cb006dbb23ce346577cb1bb5e543ba52c) )
	ROM_LOAD( "science_alt2",      0x28000, 0x8000, CRC(ac93d348) SHA1(55550ba6b5daffdf9653854075ad4f8398a5e621) )
	ROM_LOAD( "sports_alt",        0x30000, 0x8000, CRC(40207845) SHA1(2dddb9685dcefabfde07057a639aa9d08da2329e) )
ROM_END

ROM_START( gt103ab )
	ROM_REGION( 0x38000, "maincpu", 0 )
	ROM_LOAD( "t_3a-8_1.bin",       0x00000, 0x4000, CRC(02aef306) SHA1(1ffc10c79a55d41ea36bcaab13cb3f02cb3f9712) ) // "Park" alternate version sets here
	ROM_LOAD( "rock-n-roll_old",    0x10000, 0x8000, CRC(1be036b1) SHA1(0b262906044950319dd911b956ac2e0b433f6c7f) ) // From series 9 - dated 6/85
	ROM_LOAD( "usa_trivia_old",     0x18000, 0x8000, CRC(1ec59636) SHA1(65d63066a7ff500eda32468c8c9fef6cfd32f024) ) // From series 9 - dated 6/85
	ROM_LOAD( "new_science_2_old",  0x20000, 0x8000, CRC(95836bfb) SHA1(deb546bcd9109efd2b1f405354916e439cd0749b) ) // From series 12
	ROM_LOAD( "cops_+_robbers_old", 0x28000, 0x8000, CRC(8b367c33) SHA1(013468157bf469c9cf138809fdc45b3ba60a423b) ) // From series 12
ROM_END

ROM_START( gt103asx ) // Was there an all Adult Trivia version? These are just the Adult Sex questions from all the series combined here
	ROM_REGION( 0x38000, "maincpu", 0 )
	ROM_LOAD( "t_3a-8_1.bin", 0x00000, 0x4000, CRC(02aef306) SHA1(1ffc10c79a55d41ea36bcaab13cb3f02cb3f9712) )
	ROM_LOAD( "adult_sex_2+", 0x10000, 0x8000, CRC(0d683f21) SHA1(f47ce3c31c4c5ed02247fa280303e6ae760315df) ) // From series 9  - Labeled as ADULT SEX 2*
	ROM_LOAD( "adult_sex_3+", 0x18000, 0x8000, CRC(2c46e355) SHA1(387ab389abaaea8e870b00039dd884237f7dd9c6) ) // From series 10 - Labeled as ADULT SEX 3*
	ROM_LOAD( "adult_sex_4+", 0x20000, 0x8000, CRC(9c32730e) SHA1(9d060e49a4c1dd8d978619b1c357c9e8238e5c96) ) // From series 12 - Labeled as ADULT SEX 4*
	ROM_LOAD( "adult_sex_5",  0x28000, 0x8000, CRC(fdbc3729) SHA1(7cb7cec4439ddc39de2f7f62c25623cfb869f493) ) // From series 14
	ROM_LOAD( "adult_sex_6",  0x30000, 0x8000, CRC(d66f35f7) SHA1(81b56756230b27b0903d0c5df30439726526afe2) ) // From series 15
ROM_END

ROM_START( gt103asxa ) // Alternate / older versions of the questions
	ROM_REGION( 0x38000, "maincpu", 0 )
	ROM_LOAD( "t_3a-8_1.bin",    0x00000, 0x4000, CRC(02aef306) SHA1(1ffc10c79a55d41ea36bcaab13cb3f02cb3f9712) )
	ROM_LOAD( "adult_sex_2_old", 0x10000, 0x8000, CRC(8c0eacc8) SHA1(ddaa25548d161394b41c65a2db57a9fcf793062b) ) // From series 9 - dated 6/85
	ROM_LOAD( "adult_sex_3_old", 0x18000, 0x8000, CRC(63cbd1d6) SHA1(8dcd5546dc8688d6b8404d5cf63d8a59acc9bf4c) ) // From series 10
	ROM_LOAD( "adult_sex_4_old", 0x20000, 0x8000, CRC(36a75071) SHA1(f08d31f241e1dc9b94b940cd2872a692f6f8475b) ) // From series 12
ROM_END

ROM_START( quiz )
	ROM_REGION( 0x38000, "maincpu", 0 )
	ROM_LOAD( "1.bin",        0x00000, 0x4000, CRC(4e3204da) SHA1(291f1c9b8c4c07881621c3ecbba7af80f86b9520) )
	ROM_LOAD( "2.bin",        0x10000, 0x8000, CRC(b79f3ae1) SHA1(4b4aa50ec95138bc8ee4bc2a61bcbfa2515ac854) )
	ROM_LOAD( "3.bin",        0x18000, 0x8000, CRC(9c7e9608) SHA1(35ee9aa36d16bca64875640224c7fe9d327a95c3) )
	ROM_LOAD( "4.bin",        0x20000, 0x8000, CRC(30f6b4d0) SHA1(ab2624eb1a3fd9cd8d44433962d09496cd67d569) )
	ROM_LOAD( "5.bin",        0x28000, 0x8000, CRC(e9cdae21) SHA1(4de4a4edf9eccd8f9f7b935f47bee42c10ad606f) )
	ROM_LOAD( "6.bin",        0x30000, 0x8000, CRC(89e2b7e8) SHA1(e85c66f0cf37418f522c2d6384997d52f2f15117) )

	ROM_REGION( 0x0200, "proms", 0 ) // unknown
	ROM_LOAD( "prom_am27s29pc.bin", 0x0000, 0x0200, CRC(19e3f161) SHA1(52da3c1e50c2329454de14cb9c46149e573e562b) )
ROM_END

ROM_START( quizvid )
	ROM_REGION( 0x38000, "maincpu", 0 )
	ROM_LOAD( "quiz.rev2a.prg.0",    0x00000, 0x4000, CRC(28e5855a) SHA1(eec16ef7a46451054565c7001478382a0a84725a) )
	ROM_LOAD( "quiz.reva.sesso.1",   0x10000, 0x8000, CRC(72367eee) SHA1(09cd8e8c20e159ab78025e4201065c41dde90ef5) )
	ROM_LOAD( "quiz.rev2a.sport.2",  0x18000, 0x8000, CRC(1f050838) SHA1(2e44098476d05366b05f7071a615f52b588bcac5) )
	ROM_LOAD( "quiz.rev2a.musica.3", 0x20000, 0x8000, CRC(6ae39e6b) SHA1(b5ce2678c07d6c1d3d49f1703e2b9f84105e88fa) )
	ROM_LOAD( "quiz.reva.storia.4",  0x28000, 0x8000, CRC(1c648781) SHA1(e363ae1a3946446c0bc42f9857a9d66702cb3367) )
	ROM_LOAD( "quiz.reva.attuale.5", 0x30000, 0x8000, CRC(a3642478) SHA1(57851aabc6d2f5acff426c09574559e141da5d13) )

	ROM_REGION( 0x0104, "plds", 0 )
	ROM_LOAD( "pal10l8cn.pal1", 0x0000, 0x002c, CRC(7f4499de) SHA1(74838150d0b71171f00f65e03748b262c2bb6e4c) )
	ROM_LOAD( "pal10l8cn.pal4", 0x0000, 0x002c, CRC(f14a34ab) SHA1(78af7f5eafbf2d52ee7b01b497ad59448c986693) )
	ROM_LOAD( "pal16l8a-2.bin", 0x0000, 0x0104, NO_DUMP )
	ROM_LOAD( "pal16l8cn.pal3", 0x0000, 0x0104, NO_DUMP )
	ROM_LOAD( "pal16l8cn.pal5", 0x0000, 0x0104, NO_DUMP )
ROM_END

ROM_START( quiz211 )
	ROM_REGION( 0x38000, "maincpu", 0 )
	ROM_LOAD( "1a.bin",         0x000000, 0x4000, CRC(116de0ea) SHA1(9af97b100aa2c79a58de055abe726d6e2e00aab4) )
	ROM_CONTINUE(               0x000000, 0x4000 ) // halves identical
	ROM_LOAD( "hobby.bin",      0x10000, 0x8000, CRC(c86d0c2b) SHA1(987ef17c7b9cc119511a16cbd98ec44d24665af5) )
	ROM_LOAD( "musica.bin",     0x18000, 0x8000, CRC(6b08990f) SHA1(bbc633dc4e0c395269d3d3fbf1f7617ea7adabf1) )
	ROM_LOAD( "natura.bin",     0x20000, 0x8000, CRC(f17b0d59) SHA1(ebe3d5a0247f3065f0c5d4ee0b846a737700f379) )
	ROM_LOAD( "spettacolo.bin", 0x28000, 0x8000, CRC(38b8e37a) SHA1(e6df575f61ac61e825d98eaef99c128647806a75) )
	ROM_LOAD( "mondiali90.bin", 0x30000, 0x4000, CRC(35622870) SHA1(f2dab64106ca4ef07175a0ad9491470964d8a0d2) )

	ROM_REGION( 0x0e00, "proms", 0 ) // unknown
	ROM_LOAD( "prom_27s13-1.bin", 0x0000, 0x0200, NO_DUMP )
	ROM_LOAD( "prom_27s13-2.bin", 0x0200, 0x0200, NO_DUMP )
	ROM_LOAD( "prom_27s13-3.bin", 0x0400, 0x0200, NO_DUMP )
	ROM_LOAD( "prom_27s13-4.bin", 0x0600, 0x0200, NO_DUMP )
	ROM_LOAD( "prom_27s13-5.bin", 0x0800, 0x0200, NO_DUMP )
	ROM_LOAD( "prom_27s13-6.bin", 0x0a00, 0x0200, NO_DUMP )
	ROM_LOAD( "prom_6349-1n.bin", 0x0c00, 0x0200, CRC(19e3f161) SHA1(52da3c1e50c2329454de14cb9c46149e573e562b) )

	ROM_REGION( 0x0100, "plds", 0 )
	ROM_LOAD( "pal10l8cn.bin",   0x0000, 0x002c, CRC(86095226) SHA1(e7496efbd5ca240f0df2dfa5627402342c7f5384) )
ROM_END

ROM_START( bigjoke ) // TRIV3D PCB, stickered THE JOKE 11/87
	ROM_REGION( 0x38000, "maincpu", 0 )
	ROM_LOAD( "joke_cont_128",       0x00000, 0x4000, CRC(acb4355f) SHA1(0fc9a218e94fc49297bd11547fb193d1f91ce90c) )
	ROM_LOAD( "joke_2764",           0x08000, 0x2000, CRC(8a7ac263) SHA1(6f9a18b7f38e83cc2d1f1911e3b01ada7df4d2ba) ) // banked
	ROM_LOAD( "joke_drty_nsty_1012", 0x10000, 0x8000, CRC(e3fcd0e3) SHA1(fae1ddfb244e5c84e9ba6d4b90bd265365a4f662) ) // banked ROMs for solution data
	ROM_LOAD( "joke_fams_folk_1012", 0x18000, 0x8000, CRC(08eb35f2) SHA1(0dba706750a9276dac9976eb07f64ecf4a42e658) ) // J1 question ROMs
	ROM_LOAD( "joke_jmbl_joke_1012", 0x20000, 0x8000, CRC(26daf757) SHA1(76560bf437b4ee851f075b30e79df563367830cf) )
	ROM_LOAD( "joke_racl_ethn_1012", 0x28000, 0x8000, CRC(ca814fa9) SHA1(0c2ac15568bd64c282f257089305309ef7f91411) )
	ROM_LOAD( "joke_wrkg_wrld_1012", 0x30000, 0x8000, CRC(f4b0fa76) SHA1(70b4a29d928729da52948c12e55473ecf83b3daa) )

	ROM_REGION( 0x0800, "nvram", 0 )
	ROM_LOAD( "bigjoke.nvram",  0x0000, 0x0800, CRC(89e0e325) SHA1(c2398c64f938e8ed036e72a5ba0b703513f31a6d) )

	ROM_REGION( 0x0400, "pld", 0 ) // probably one of the two GALs provides the "signature"
	ROM_LOAD( "gal16v8",   0x0000, 0x0117, NO_DUMP ) // read protected
	ROM_LOAD( "gal18v8",   0x0200, 0x0117, NO_DUMP ) // read protected

	ROM_REGION( 0x0008, "signature", 0 ) // bytes 0x03 through 0x0a of each question ROM - to prevent ROM swaps
	ROM_LOAD( "bigjoke.sig",   0x0000, 0x0008, CRC(bfa5388b) SHA1(876bf954116fcc14d0bed017a9bec42038c5f89a) )
ROM_END

ROM_START( sexappl ) // TRIV3D PCB, stickered SEX APPL 6.02 5/92
	ROM_REGION( 0x38000, "maincpu", 0 )
	ROM_LOAD( "6.02_cont", 0x00000, 0x4000, CRC(63ad3593) SHA1(fd93a71b82ef04757d79485ca4c7e306b2983c76) )
	ROM_LOAD( "6.02_kb",   0x08000, 0x2000, CRC(025a5c7e) SHA1(bc935fb5ac081d089f3f9991d04cdf3708fa35c6) ) // banked
	ROM_LOAD( "hot_sex",   0x10000, 0x8000, CRC(f16b3363) SHA1(a05bb2ae6467cd28021321bb526ea2ae3da82361) ) // banked ROMs for solution data
	ROM_LOAD( "wild_sex",  0x18000, 0x8000, CRC(f257a023) SHA1(9c72c18f1acd7b36033a20dd1c8fafc801a3d174) ) // T6 question ROMs
	ROM_LOAD( "hard_sex",  0x20000, 0x8000, CRC(bdab9ac1) SHA1(e09a5276a5bd346e2b88dd8fa196f204267efe09) )
	ROM_LOAD( "kinky_sex", 0x28000, 0x8000, CRC(6b4c016d) SHA1(7d0b8af7c5ef384412535ab3e2ed1eb7c4ecd824) )
	ROM_LOAD( "adult_sex", 0x30000, 0x8000, CRC(05798340) SHA1(8db308bb725112327027a725b2c69299e6da1dad) )

	ROM_REGION( 0x0800, "nvram", 0 )
	ROM_LOAD( "sexappl.nvram",   0x0000, 0x0800, CRC(be65737c) SHA1(5b8a603a9ddecdad4aaef0b9e8ef373885b236c0) ) // Defaults but with card dispenser OFF!

	ROM_REGION( 0x0400, "pld", 0 ) // probably one of the two GALs provides the "signature"
	ROM_LOAD( "gal16v8",   0x0000, 0x0117, NO_DUMP ) // read protected
	ROM_LOAD( "gal18v8",   0x0200, 0x0117, NO_DUMP ) // read protected

	ROM_REGION( 0x0008, "signature", 0 ) // bytes 0x03 through 0x0a of each question ROM - to prevent ROM swaps
	ROM_LOAD( "sexappl.sig",   0x0000, 0x0008, CRC(c8e944a3) SHA1(d34de9e3163ba61fa4e4f2264caff40434fcc9b0) )
ROM_END

/*
GEI Multi Game System
(c) 1992

Much newer satellite board for the Greyhound UV-1B mainboard system

Contains:
Power LED
3 Pals
2 74LS374N

ST MK48Z02B-20 Zeropower RAM

Control, graphics & 13 banked ROMs (as labeled):

Cont: R.5 Cont 92
GRPH: R.5 K.B. 93

Bank  1: pokr 4/3 92
Bank  2: bljk 4/3 92
Bank  3: bone 4/3 92
Bank  4: slot 4/3 92
Bank  5: sprd 4/3 92
Bank  6: hrse 7/8 92
Bank  7: dogs 7/8 92
Bank  8: memo test
Bank  9: reel fun
Bank 10: fact #1
Bank 11: adlt sex #1
Bank 12: nfl #1
Bank 13: entr #1

All ROMs are 27C256 type EPROMs
*/

ROM_START( geimulti )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "cont", 0x00000, 0x8000, CRC(920020df) SHA1(01cb0a58bf863d63bcafe1e198ce5ea10bb8f59e) )

	ROM_REGION( 0x8000*(13 + 1), "bank", ROMREGION_ERASEFF )
	ROM_LOAD( "pokr",      0x00000, 0x8000, CRC(1e9651a0) SHA1(23f98633701a9ac01784b29d1283ec9236810baf) ) // C5 game ROM 4-21-92
	ROM_LOAD( "bljk",      0x08000, 0x8000, CRC(afde21e6) SHA1(4db37ee4ffc72e3513cbfe31d018caa062683adf) ) // C5 game ROM 4-21-92
	ROM_LOAD( "bone",      0x10000, 0x8000, CRC(6a84fc5d) SHA1(09d5d052a90744e5c6e0b06cabfdbbaa2458bbb5) ) // C5 game ROM 4-21-92
	ROM_LOAD( "slot",      0x18000, 0x8000, CRC(11b85a71) SHA1(5672f6f796b2743c252d159778bd8746cf1f5f86) ) // C5 game ROM 4-21-92
	ROM_LOAD( "sprd",      0x20000, 0x8000, CRC(2f067b3d) SHA1(cd7cae3d2def74369b8d015b9f36d5a174bea92c) ) // C5 game ROM 4-21-92
	ROM_LOAD( "hrse",      0x28000, 0x8000, CRC(ab1d014a) SHA1(aa7a372266decd7b65234de4d866fe7baf71c9bb) ) // C5 game ROM IAM 07-02-92
	ROM_LOAD( "dogs",      0x30000, 0x8000, CRC(83cd070a) SHA1(ed7ce3ffe030802e99dd27f2310524615fcfc8bf) ) // C5 game ROM IAM 07-02-92
	ROM_LOAD( "memotest",  0x38000, 0x8000, CRC(8942b98c) SHA1(3f182eb6d00618d9859cd8acdf13c829d4469075) ) // C5 game ROM 9-30-92
	ROM_LOAD( "reelfun",   0x40000, 0x8000, CRC(b3ede904) SHA1(e77c15e893583572650bb60432d68a1f7bf67d09) ) // C5 game ROM 9-11-92
	ROM_LOAD( "fact@1",    0x48000, 0x8000, CRC(949b5519) SHA1(d4ac35ccbbb50f4e0d1e21e8427e6ad535e6da53) ) // T5 question ROM
	ROM_LOAD( "adltsex@1", 0x50000, 0x8000, CRC(9e4d320b) SHA1(76a6280bce884acb3faa9ef3882de016612c34f6) ) // T5 question ROM
	ROM_LOAD( "nfl@1",     0x58000, 0x8000, CRC(74fbbf17) SHA1(9d559034d3ec0b293d4b720150197daea3d450ff) ) // T5 question ROM
	ROM_LOAD( "entr@1",    0x60000, 0x8000, CRC(caceaa7b) SHA1(c51f10f5acd3d3fedce43103b9f11d006139043c) ) // T5 question ROM
	ROM_LOAD( "grph",      0x68000, 0x8000, CRC(25e265db) SHA1(6e184309ee67dbe7930570b135ace09eeb1eb333) ) // game graphics

	ROM_REGION( 0x0800, "nvram", 0 )
	ROM_LOAD( "geimulti.nvram",   0x0000, 0x0800, CRC(232bbad1) SHA1(23a4011ad3b322cbda40859a1619dc1f5472fdfc) ) // Defaults

	// Only questions ROMs are checked for signatures, not the "game" ROMs
	ROM_REGION( 0x0008, "signature", 0 ) // bytes 0x03 through 0x0a of the listed question ROMs - to prevent ROM swaps
	ROM_LOAD( "geimulti.sig",   0x0000, 0x0008, CRC(c8e944a3) SHA1(d34de9e3163ba61fa4e4f2264caff40434fcc9b0) )
ROM_END

ROM_START( sprtauth )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "sprt-auth-rev3-cont.cont", 0x00000, 0x8000, CRC(1f1c890e) SHA1(570f1e8cdea7ed205aa3c964330bc5a4851c11e4) )

	ROM_REGION( 0x8000*(13 + 1), "bank", ROMREGION_ERASEFF )
	ROM_LOAD( "sprt-auth-prem-bsbl.bank1",    0x00000, 0x8000, CRC(ef296636) SHA1(e727fe58bc961f56503788b2b670f5b3b04743f2) ) // T8 question ROMs
	ROM_LOAD( "sprt-auth-prem-bskt.bank2",    0x08000, 0x8000, CRC(4649d523) SHA1(fdfa1263cc6b3c45e62af0e3baf79311f243ac5f) )
	ROM_LOAD( "sprt-auth-prem-foot.bank3",    0x10000, 0x8000, CRC(76076022) SHA1(5014133e7143c4f4ccf4c7d1f05effe286b5a30a) )
	ROM_LOAD( "sprt-auth-prem-hcky.bank4",    0x18000, 0x8000, CRC(135beec3) SHA1(66fce1c0c0abbbf4971ab0f764d27f1d0849ccdc) )
	ROM_LOAD( "sprt-auth-asrt-sprt.bank5",    0x20000, 0x8000, CRC(fe8fc879) SHA1(efe38e8d3a314062b8005636df683349cd54f857) )
	ROM_LOAD( "sprt-auth-wrld-seri.bank6",    0x28000, 0x8000, CRC(86ee6d56) SHA1(b879f2e2aae2ed741d8a8280d146dbb431a6ebb0) ) // Only new question category
	ROM_LOAD( "sprt-auth-ii-bsbl-ball.bank7", 0x30000, 0x8000, CRC(bad3e6bd) SHA1(e3df34a4d5db7cfbd014a841d224eeefc8386d40) )
	ROM_LOAD( "sprt-auth-ii-foot-ball.bank8", 0x38000, 0x8000, CRC(c305dec2) SHA1(924e608cd327bf6ffe831225affa270181599cea) )
	ROM_LOAD( "sprt-auth-supr-bowl.bank9",    0x40000, 0x8000, CRC(3a548fe5) SHA1(6ad35516651a8a878b512cb3eff697952e194dd0) )
	ROM_LOAD( "sprt-auth-auto-racg.bank10",   0x48000, 0x8000, CRC(7ac1bbd6) SHA1(8635791bf2707b4d028ee8b020199770984b9ef4) )
	ROM_LOAD( "sprt-auth-rev3-kb.grph",       0x68000, 0x8000, CRC(3c779fe8) SHA1(0a32f408677b3887dbfe505ed45a11fa695b7726) ) // game graphics

	ROM_REGION( 0x0800, "nvram", 0 )
	ROM_LOAD( "sprtauth.nvram",   0x0000, 0x0800, CRC(969869f9) SHA1(a5a7b679e99255650dc8ea12d2b36e97e6296aae) ) // Defaults but with card dispenser OFF!

	ROM_REGION( 0x0008, "signature", 0 ) // bytes 0x03 through 0x0a of each question ROM - to prevent ROM swaps
	ROM_LOAD( "sprtauth.sig",   0x0000, 0x0008, CRC(c8e944a3) SHA1(d34de9e3163ba61fa4e4f2264caff40434fcc9b0) )
ROM_END

ROM_START( sprtauth1 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "sprt-auth-rev1-cont.cont", 0x00000, 0x8000, CRC(19dd0aa6) SHA1(7805d8139ac20061fc782bcaeff2202ed451fa91) )

	ROM_REGION( 0x8000*(13 + 1), "bank", ROMREGION_ERASEFF )
	ROM_LOAD( "sprt-auth-prem-bsbl.bank1",    0x00000, 0x8000, CRC(ef296636) SHA1(e727fe58bc961f56503788b2b670f5b3b04743f2) ) // T8 question ROMs
	ROM_LOAD( "sprt-auth-prem-bskt.bank2",    0x08000, 0x8000, CRC(4649d523) SHA1(fdfa1263cc6b3c45e62af0e3baf79311f243ac5f) )
	ROM_LOAD( "sprt-auth-prem-foot.bank3",    0x10000, 0x8000, CRC(76076022) SHA1(5014133e7143c4f4ccf4c7d1f05effe286b5a30a) )
	ROM_LOAD( "sprt-auth-prem-hcky.bank4",    0x18000, 0x8000, CRC(135beec3) SHA1(66fce1c0c0abbbf4971ab0f764d27f1d0849ccdc) )
	ROM_LOAD( "sprt-auth-asrt-sprt.bank5",    0x20000, 0x8000, CRC(fe8fc879) SHA1(efe38e8d3a314062b8005636df683349cd54f857) )
	ROM_LOAD( "sprt-auth-auto-racg.bank6",    0x28000, 0x8000, CRC(7ac1bbd6) SHA1(8635791bf2707b4d028ee8b020199770984b9ef4) )
	ROM_LOAD( "sprt-auth-ii-bsbl-ball.bank7", 0x30000, 0x8000, CRC(bad3e6bd) SHA1(e3df34a4d5db7cfbd014a841d224eeefc8386d40) )
	ROM_LOAD( "sprt-auth-ii-foot-ball.bank8", 0x38000, 0x8000, CRC(c305dec2) SHA1(924e608cd327bf6ffe831225affa270181599cea) )
	ROM_LOAD( "sprt-auth-supr-bowl.bank9",    0x40000, 0x8000, CRC(3a548fe5) SHA1(6ad35516651a8a878b512cb3eff697952e194dd0) )
	ROM_LOAD( "sprt-auth-rev1-kb.grph",       0x68000, 0x8000, CRC(c4f734ac) SHA1(028217fe6d7be75f75e9f67b665d465c729d2995) ) // game graphics

	ROM_REGION( 0x0800, "nvram", 0 )
	ROM_LOAD( "sprtauth.nvram",   0x0000, 0x0800, CRC(969869f9) SHA1(a5a7b679e99255650dc8ea12d2b36e97e6296aae) ) // Defaults but with card dispenser OFF!

	ROM_REGION( 0x0008, "signature", 0 ) // bytes 0x03 through 0x0a of each question ROM - to prevent ROM swaps
	ROM_LOAD( "sprtauth.sig",   0x0000, 0x0008, CRC(c8e944a3) SHA1(d34de9e3163ba61fa4e4f2264caff40434fcc9b0) )
ROM_END

void gei_state::init_setbank()
{
	m_rombank->set_base(memregion("maincpu")->base() + 0x2000);
}

void gei_state::init_bank2k()
{
	m_rombank->configure_entries(0, 10, memregion("maincpu")->base() + 0x10000, 0x2000);
	m_rombank->set_entry(0);
}

void gei_state::init_bank8k()
{
	m_rombank->configure_entries(0, 6, memregion("maincpu")->base() + 0x8000, 0x8000);
	m_rombank->set_entry(0);
}

void gei_state::init_geimulti()
{
	m_rombank->configure_entries(0, 14, memregion("bank")->base(), 0x8000);
	m_rombank->set_entry(0);
}

} // anonymous namespace


GAME( 1982, jokpoker,  0,        gselect,   gselect,  gei_state, init_setbank,  ROT0, "Greyhound Electronics", "Joker Poker (Version 16.03B)",            MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1983, jokpokera, jokpoker, jokpokera, gselect,  gei_state, init_setbank,  ROT0, "Greyhound Electronics", "Joker Poker (Version 16.03BI 5-10-85, Joker Poker ICB 9-30-86)",     MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1983, jokpokerb, jokpoker, jokpokera, gselect,  gei_state, init_setbank,  ROT0, "Greyhound Electronics", "Joker Poker (Version 16.04BI 10-19-88, Joker Poker ICB 9-30-86)",    MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1983, jokpokerc, jokpoker, jokpokera, gselect,  gei_state, init_setbank,  ROT0, "Greyhound Electronics", "Joker Poker (Version 16.03BI 5-10-85, Poker No Raise ICB 9-30-86)",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1982, superbwl,  0,        gselect,   gselect,  gei_state, init_setbank,  ROT0, "Greyhound Electronics", "Super Bowl (Version 16.03B)",             MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )

GAME( 1982, gs4002,    0,        gselect,   gselect,  gei_state, init_bank2k,   ROT0, "Greyhound Electronics", "Selection (Version 40.02TMB, set 1)",     MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1982, gs4002a,   gs4002,   gselect,   gselect,  gei_state, init_bank2k,   ROT0, "Greyhound Electronics", "Selection (Version 40.02TMB, set 2)",     MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )

GAME( 1982, amuse,     0,        amuse,     gepoker,  gei_state, init_bank2k,   ROT0, "Greyhound Electronics", "Amuse (Version 50.08 IBA)",               MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1982, amuse1,    amuse,    amuse1,    gepoker,  gei_state, init_bank2k,   ROT0, "Greyhound Electronics", "Amuse (Version 30.08 IBA)",               MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1983, amuse1a,   amuse,    amuse1,    gepoker,  gei_state, init_bank2k,   ROT0, "Greyhound Electronics", "Amuse (Version 30.08A)",                  MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )

GAME( 1984, gepoker,   0,        gepoker,   gepoker,  gei_state, init_bank2k,   ROT0, "Greyhound Electronics", "Poker (Version 50.02 ICB, set 1)",        MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1984, gepoker1,  gepoker,  gepoker,   gepoker,  gei_state, init_bank2k,   ROT0, "Greyhound Electronics", "Poker (Version 50.02 ICB, set 2)",        MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1984, gepoker2,  gepoker,  gepoker,   gepoker,  gei_state, init_bank2k,   ROT0, "Greyhound Electronics", "Poker (Version 50.02 ICB, set 3)",        MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1984, gepoker3,  gepoker,  gepoker,   gepoker,  gei_state, init_bank2k,   ROT0, "Greyhound Electronics", "Poker (Version 50.02 ICB, set 4)",        MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )

GAME( 1984, gtsers1,   0,        getrivia,  getrivia, gei_state, init_bank2k,   ROT0, "Greyhound Electronics", "Trivia (Questions Series 1)",             MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1984, gtsers2,   gtsers1,  getrivia,  getrivia, gei_state, init_bank2k,   ROT0, "Greyhound Electronics", "Trivia (Questions Series 2)",             MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1984, gtsers3,   gtsers1,  getrivia,  getrivia, gei_state, init_bank2k,   ROT0, "Greyhound Electronics", "Trivia (Questions Series 3)",             MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1984, gtsers4,   gtsers1,  getrivia,  getrivia, gei_state, init_bank2k,   ROT0, "Greyhound Electronics", "Trivia (Questions Series 4)",             MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1984, gtsers5,   gtsers1,  getrivia,  getrivia, gei_state, init_bank2k,   ROT0, "Greyhound Electronics", "Trivia (Questions Series 5)",             MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1984, gtsers7,   gtsers1,  getrivia,  getrivia, gei_state, init_bank2k,   ROT0, "Greyhound Electronics", "Trivia (Questions Series 7)",             MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1984, gtsers7a,  gtsers1,  getrivia,  getrivia, gei_state, init_bank2k,   ROT0, "Greyhound Electronics", "Trivia (Questions Series 7, alt question ROM)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1984, gtsersa,   gtsers1,  getrivia,  getrivia, gei_state, init_bank2k,   ROT0, "Greyhound Electronics", "Trivia (Alt revision questions set 1)",   MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1984, gtsersb,   gtsers1,  getrivia,  getrivia, gei_state, init_bank2k,   ROT0, "Greyhound Electronics", "Trivia (Alt revision questions set 2)",   MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1984, gtsers8,   0,        findout,   getrivia, gei_state, init_bank8k,   ROT0, "Greyhound Electronics", "Trivia (Questions Series 8)",             MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1984, gtsers8a,  gtsers8,  findout,   getrivia, gei_state, init_bank8k,   ROT0, "Greyhound Electronics", "Trivia (Questions Series 8, alt question ROM)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1984, gtsers9,   gtsers8,  findout,   getrivia, gei_state, init_bank8k,   ROT0, "Greyhound Electronics", "Trivia (Questions Series 9)",             MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1984, gtsers10,  gtsers8,  findout,   getrivia, gei_state, init_bank8k,   ROT0, "Greyhound Electronics", "Trivia (Questions Series 10)",            MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1984, gtsers11,  gtsers8,  findout,   getrivia, gei_state, init_bank8k,   ROT0, "Greyhound Electronics", "Trivia (Questions Series 11)",            MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1984, gtsers11a, gtsers8,  findout,   getrivia, gei_state, init_bank8k,   ROT0, "Greyhound Electronics", "Trivia (Questions Series 11, alt question ROM, set 1)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1984, gtsers11b, gtsers8,  findout,   getrivia, gei_state, init_bank8k,   ROT0, "Greyhound Electronics", "Trivia (Questions Series 11, alt question ROM, set 2)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1984, gtsers12,  gtsers8,  findout,   gt103,    gei_state, init_bank8k,   ROT0, "Greyhound Electronics", "Trivia (Questions Series 12)",            MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1984, gtsers12a, gtsers8,  findout,   gt103,    gei_state, init_bank8k,   ROT0, "Greyhound Electronics", "Trivia (Questions Series 12, alt question ROM)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1986, gtsers14,  gtsers8,  findout,   gt103,    gei_state, init_bank8k,   ROT0, "Greyhound Electronics", "Trivia (Questions Series 14)",            MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1984, gtsers14a, gtsers8,  findout,   gt103,    gei_state, init_bank8k,   ROT0, "Greyhound Electronics", "Trivia (Questions Series 14, alt question ROM)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1986, gtsers15,  gtsers8,  findout,   gt103,    gei_state, init_bank8k,   ROT0, "Greyhound Electronics", "Trivia (Questions Series 15)",            MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1986, gtsers18,  gtsers8,  findout,   gt103,    gei_state, init_bank8k,   ROT0, "Greyhound Electronics", "Trivia (Questions Series 18)",            MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1984, gt103a1,   gtsers8,  findout,   getrivia, gei_state, init_bank8k,   ROT0, "Greyhound Electronics", "Trivia (Unsorted question ROMs)",         MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1984, gt103aa,   gtsers8,  findout,   getrivia, gei_state, init_bank8k,   ROT0, "Greyhound Electronics", "Trivia (Version 1.03a, alt questions 1)",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1984, gt103ab,   gtsers8,  findout,   getrivia, gei_state, init_bank8k,   ROT0, "Greyhound Electronics", "Trivia (Version 1.03a, alt questions 2)",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1984, gt103asx,  gtsers8,  findout,   getrivia, gei_state, init_bank8k,   ROT0, "Greyhound Electronics", "Trivia (Version 1.03a, sex questions)",    MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1984, gt103asxa, gtsers8,  findout,   getrivia, gei_state, init_bank8k,   ROT0, "Greyhound Electronics", "Trivia (Version 1.03a, sex questions, alt revision questions)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )

GAME( 1985, sextriv1,  0,        getrivia,  sextriv1, gei_state, init_bank2k,   ROT0, "Kinky Kit and Game Co.", "Sexual Trivia (Version 1.02SB, set 1)",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1985, sextriv2,  sextriv1, getrivia,  sextriv1, gei_state, init_bank2k,   ROT0, "Kinky Kit and Game Co.", "Sexual Trivia (Version 1.02SB, set 2)",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )

GAME( 1986, gt507uk,   0,        findout,   gt507uk,  gei_state, init_bank8k,   ROT0, "Grayhound Electronics",  "Trivia (UK Version 5.07)",               MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )

GAME( 1986, quiz,      0,        findout,   quiz,     gei_state, init_bank8k,   ROT0, "Elettronolo",            "Quiz (Revision 2)",                      MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )

GAME( 1986, quizvid,   0,        quizvid,   quiz,     gei_state, init_bank8k,   ROT0, "bootleg",                "Video Quiz",                             MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )

GAME( 1986, reelfun,   0,        findout,   reelfun,  gei_state, init_bank8k,   ROT0, "Grayhound Electronics",  "Reel Fun (Version 7.03)",                MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1986, reelfun1,  reelfun,  findout,   reelfun,  gei_state, init_bank8k,   ROT0, "Grayhound Electronics",  "Reel Fun (Version 7.01)",                MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1986, reelfun0,  reelfun,  findout,   reelfun,  gei_state, init_bank8k,   ROT0, "Grayhound Electronics",  "Reel Fun (Version 7.00)",                MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1987, findout,   0,        findout,   findout,  gei_state, init_bank8k,   ROT0, "Elettronolo",            "Find Out (Version 4.04, set 1)",         MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1989, findouta,  findout,  findout,   findout,  gei_state, init_bank8k,   ROT0, "Elettronolo",            "Find Out (Version 4.04, set 2)",         MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // same program ROMs with year hack, different questions

GAME( 1986, suprpokr,  0,        suprpokr,  suprpokr, gei_state, empty_init,    ROT0, "Grayhound Electronics",  "Super Poker (Version 10.19S)",           MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1986, suprpokra, suprpokr, suprpokr,  suprpokr, gei_state, empty_init,    ROT0, "Grayhound Electronics",  "Super Poker (Version 10.15S)",           MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1986, suprpokrb, suprpokr, suprpokr,  suprpokr, gei_state, empty_init,    ROT0, "Grayhound Electronics",  "Super Poker (Version 10.10)",            MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )

GAME( 1987, bigjoke,   0,        findout,   bigjoke,  gei_state, init_bank8k,   ROT0, "Grayhound Electronics",  "The Big Joke (Version 0.00)",            MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )

GAME( 1991, quiz211,   0,        findout,   quiz,     gei_state, init_bank8k,   ROT0, "Elettronolo",            "Quiz (Revision 2.11)",                   MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )

GAME( 1992, sexappl,   0,        findout,   sexappl,  gei_state, init_bank8k,   ROT0, "Grayhound Electronics",  "Sex Appeal (Version 6.02)",              MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )

GAME( 1992, geimulti,  0,        sprtauth,  geimulti, gei_state, init_geimulti, ROT0, "Grayhound Electronics",  "GEI Multi Game",                         MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1992, sprtauth,  0,        sprtauth,  sprtauth, gei_state, init_geimulti, ROT0, "Classic Games",          "Sports Authority Challenge (Rev 3)",     MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1992, sprtauth1, sprtauth, sprtauth,  sprtauth, gei_state, init_geimulti, ROT0, "Classic Games",          "Sports Authority (Rev 1)",               MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
