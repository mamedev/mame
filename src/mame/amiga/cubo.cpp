// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Mariusz Wojcieszek, Ernesto Corvi, Dirk Best, Stephane Humbert
/*

   Cubo CD32 (additional hardware and games by CD Express, Milan, Italy)

   The CuboCD32 is a stock retail CD32 unit with additional hardware to adapt it
   for JAMMA use.

   TODO:
   - remove m_input_hack functions, needed to make inputs working;
   - extra RTC device mapped on i2c bus (shared with the Akiko one?);
   - later gambling games requires extra hopper devices;

   Known Games:
   Dumped | Title                | Rev.  | Year | Notes
   ----------------------------------------------------
     YES  | Candy Puzzle         |  1.0  | 1995 |
          | Double Strixx        |       | 1995 |
     YES  | El Dorado            |  4.2  | 199x | Laser Gate 2
          | Greyhound Race       |       | 199x |
     YES  | Harem Challenge      |       | 1995 |
          | Laser Quiz           |       | 1995 |
          | Laser Quiz France    |  1.0  | 1995 |
     YES  | Laser Quiz Greece    |       | 1995 |
     YES  | Laser Quiz Italy     |       | 1995 |
     YES  | Laser Quiz 2 Greece  |       | 1995 |
     YES  | Laser Quiz 2 Italy   |  1.0  | 1995 |
          | Laser Strixx         |       | 1995 |
     YES  | Laser Strixx 2       |       | 1995 |
          | Lucky Five           |       | 1995 |
     YES  | Magic Number         |       | 1995 |
     YES  | Magic Premium        |    1.1| 1996 |
     YES  | Odeon Twister        |    1.4| 199x |
     YES  | Odeon Twister 2      | 202.19| 1999 |

Stephh's notes (based on the game M68EC020 code and some tests) :


1) "Candy Puzzle"

settings (A5=0x059ac0) :

  - 051ba0.w (-$7f20,A5) : difficulty player 1 (0)
  - 051ba2.w (-$7f1e,A5) : difficulty player 2 (0)
  - 051ba4.w (-$7f1c,A5) : challenge rounds (3)
  - 051ba6.w (-$7f1a,A5) : death in 2P mode (0) - 0 = NO / 1 = YES
  - 051ba8.w (-$7f18,A5) : play x credit (1)
  - 051bac.w (-$7f14,A5) : coin x 1 play (1)
  - 051bb0.w (-$7f10,A5) : maxpalleattack (2)
  -                      : reset high score - NO* / YES
  - 051baa.w (-$7f16,A5) : 1 coin 2 players (1) - 0 = NO / 1 = YES

useful addresses :

  - 051c02.w (-$7ebe,A5) : must be 0x0000 instead of 0x0001 to accept coins !
  - 051c04.b (-$7ebc,A5) : credits
  - 051c10.l (-$7eb0,A5) : basic address for inputs = 0006fd18 :
      * BA + 1 x 6 + 4 : start 2 (bit 5)
      * BA + 1 x 6 + 0 : player 2 L/R (-1/0/1)
      * BA + 1 x 6 + 2 : player 2 U/D (-1/0/1)
      * BA + 0 x 6 + 4 : start 1 (bit 5)
      * BA + 0 x 6 + 0 : player 1 L/R (-1/0/1)
      * BA + 0 x 6 + 2 : player 1 U/D (-1/0/1)

routines :

  - 07acae : inputs read
  - 092c8a : buttons + coin read
      * D0 = 0 : read potgo (player 2)
      * D0 = 1 : read potgo (player 1)
  - 092baa : joy read - test bits 1 and 9 : 1 if bit 1, -1 if bit 9, 0 if none
      * D0 = 0 : read joy_0_dat (player 2)
      * D0 = 1 : read joy_1_dat (player 1)
  - 092bd4 : joy read - test bits 0 and 8 : 1 if bit 0, -1 if bit 8, 0 if none ("eor.w read, read >> 1" before test)
      * D0 = 0 : read joy_0_dat (player 2)
      * D0 = 1 : read joy_1_dat (player 1)

  - 07ada4 : coin verification
  - 07d0fe : start buttons verification


2) "Harem Challenge"

settings (A5=0x00a688 & A2=0x0028e8) :

  -                      : photo level - soft* / erotic / porno
  - 002906.b (A2 + 0x1e) : difficulty (2) - 1 = LOW / 2 = NORMAL / 3 = HIGH
  - 0028fc.b (A2 + 0x14) : deadly selftouch (0) - 0 = NO / 1 = YES
  - 0028ff.b (A2 + 0x17) : energy supply - 100%* / 0% / 50%
  - 002900.b (A2 + 0x18) : area to cover 1P - 75% / 80%* / 85%
  - 002901.b (A2 + 0x19) : area to cover 2P - 70% / 75%* / 80%
  - 002902.w (A2 + 0x1a) : level time (00C8) - 0096 = 1:00 / 00C8 = 1:20 / 00FA = 1:40
  - 002904.b (A2 + 0x1c) : tournament mode (2) - 0 = NO CHALLENGE / 2 = BEST OF 3 / 3 = BEST OF 5
  -                      : reset hi-score - NO* / YES
  - 0028fd.b (A2 + 0x15) : coin x 1 play (1) <=> number of credits used for each play
  - 0028fe.b (A2 + 0x16) : play x credit (1) <=> credits awarded for each coin
  -                      : demo photo level - soft* / erot

useful addresses :

  - 002907.b (-$7f00,A5 -> $1f,A2) : must be 0x00 instead of 0x01 to accept coins !
  - 00278c.w (-$7efc,A5) : player 2 inputs
  - 00278e.w (-$7efa,A5) : player 1 inputs
  - 002790.b (-$7ef8,A5) : credits

  - 0028e8.b (A2       ) : level - range (0x01 - 0x09 and 0x00 when in "attract mode")
  - 0028ea.w (A2 + 0x02) : time (not used for "continue play") - range (0x0000 - 0x0014 with 0x0000 being max)

  - 026fe8.l (-$7fa0,A5 -> A2 + $18 x 0 + 0x00) : player 1 energy (in MSW after swap) - range (0x00000000 - 0x00270000)
  - 026fec.l (-$7fa0,A5 -> A2 + $18 x 0 + 0x04) : player 1 score
  - 026ff8.w (-$7fa0,A5 -> A2 + $18 x 0 + 0x10) : player 1 girls
  - 027000.l (-$7fa0,A5 -> A2 + $18 x 1 + 0x00) : player 2 energy (in MSW after swap) - range (0x00000000 - 0x00270000)
  - 027004.l (-$7fa0,A5 -> A2 + $18 x 1 + 0x04) : player 2 score
  - 027010.w (-$7fa0,A5 -> A2 + $18 x 1 + 0x10) : player 2 girls

routines :

  - 04a5de : inputs read
  - 061928 : buttons + coin read
      * D0 = 0 : read potgo (player 2)
      * D0 = 1 : read potgo (player 1)

  - 04a692 : coin verification
  - 050b8c : start buttons verification


3) "Laser Quiz"

settings (A5=0x00a580 & A2=0x001e08) :

  - 001a54.w (see below) : numero di vite (2)
  - 001e08.w (A2       ) : primo bonus dom. (5)
  - 001e0a.w (A2 + 0x02) : max dom. facili (7)
  - 001e0c.w (A2 + 0x04) : incremento dom. (2)
  - 001e0e.w (A2 + 0x06) : inizio dom. diff. (10)
  - 001e10.w (A2 + 0x08) : risposte a video (4)
  - 001e12.w (A2 + 0x0a) : checkmark risp. (1) - 0 = CONTEMPORANEO / 1 = IMMEDIATO
  - 001e16.w (A2 + 0x0e) : vel. chronometro (32) - 3C = LENTA / 32 = NORMALE / 28 = VELOCE
  - 001e14.w (A2 + 0x0c) : volume musica (14) - 14 = NORMALE / 0A = BASSO / 00 = NO MUSICA
  - 001e18.b (A2 + 0x10) : vita premio bonus (1) - 0 = PER IL PRIMO / 1 = PER EMTRAMBI
  -                      : reset hi-score - NO* / SI
  - 001e19.b (A2 + 0x11) : coin x 1 play (1) <=> number of credits used for each play
  - 001e1a.b (A2 + 0x12) : foto erotiche (1) - 1 = SI / 2 = NO

useful addresses :

  - 001e1b.b (-$7fe0,A5 -> $13,A2) : must be 0x00 instead of 0x01 to accept coins !
  - 0026a4.w (-$7f0c,A5) : player 2 inputs
  - 0026a6.w (-$7f0a,A5) : player 1 inputs
  - 002606.b (-$7faa,A5) : credits

  - 001a54.w (-$7fe4,A5 -> $04,A2) : lives at start
  - 001a59.b (-$7fe4,A5 -> $09,A2) : bonus level (so lives aren't decremented) ? 0x00 = NO / 0x01 YES
  - 001a5a.b (-$7fe4,A5 -> $0a,A2) : level - range 0x00-0x63

  - 00c9b0.b (-$7fdc,A5 -> A2 + $06 x 0 + 0x00) : player 1 active ? 0x00 = NO / 0x01 YES / 0x02 = "ATTENDI"
  - 00c9b1.b (-$7fdc,A5 -> A2 + $06 x 0 + 0x01) : player 1 lives - range 0x00-0x63
  - 00c9b2.l (-$7fdc,A5 -> A2 + $06 x 0 + 0x02) : player 1 score
  - 00c9b6.b (-$7fdc,A5 -> A2 + $06 x 1 + 0x00) : player 2 active ? 0x00 = NO / 0x01 YES / 0x02 = "ATTENDI"
  - 00c9b7.b (-$7fdc,A5 -> A2 + $06 x 1 + 0x01) : player 2 lives - range 0x00-0x63
  - 00c9b8.l (-$7fdc,A5 -> A2 + $06 x 1 + 0x02) : player 2 score

  - 02327c.l (-$50,A4) : player 1 inputs (ingame)
  - 023280.l (-$4c,A4) : player 2 inputs (ingame)

routines :

  - 0cedf8 : inputs read
  - 0d8150 : buttons + coin read
      * D0 = 0 : read potgo (player 2)
      * D0 = 1 : read potgo (player 1)

  - 0cee42 : coin verification
  - 0cefc6 : start buttons verification

  - 0cb40c : inputs (ingame) read


4) "Laser Quiz 2"

settings (A5=0x0531e8 & A2=0x0460f0) :

  - 0460f0.w (A2       ) : numero di vite (2)
  - 0460f0.w (A2 + 0x02) : primo bonus dom. (5)
  - 0460f0.w (A2 + 0x04) : max dom. facili (7)
  - 0460f0.w (A2 + 0x06) : incremento dom. (2)
  - 0460f0.w (A2 + 0x08) : inizio dom. diff. (10)
  - 0460f0.w (A2 + 0x0a) : risposte a video (4)
  - 0460f0.w (A2 + 0x0c) : checkmark risp. (1) - 0 = CONTEMPORANEO / 1 = IMMEDIATO
  - 0460f0.w (A2 + 0x10) : vel. chronometro (5) - 5 = LENTA / 4 = NORMALE / 3 = VELOCE
  - 0460f0.w (A2 + 0x0e) : volume musica (3F) - 3F = NORMALE / 20 = BASSO / 00 = NO MUSICA
  - 0460f0.b (A2 + 0x12) : vita premio bonus (1) - 0 = PER IL PRIMO / 1 = PER TUTTI
  -                      : reset hi-score - NO* / SI
  - 0460f0.b (A2 + 0x13) : coin x 1 play (1) <=> number of credits used for each play
  - 0460f0.b (A2 + 0x14) : play x credit (1) <=> credits awarded for each coin
  - 0460f0.b (A2 + 0x15) : foto erotiche (1) - 1 = SI / 2 = NO
  - 0460f0.b (A2 + 0x16) : numero giocatori (0) - 0 = 2 / 1 = 4

useful addresses :

  - 046107.b (-$7fdc,A5 -> $17,A2) : must be 0x00 instead of 0x01 to accept coins !
  - 04b250.w (-$7f98,A5) : player 2 inputs
  - 04b252.w (-$7f96,A5) : player 1 inputs
  - 04b254.w (-$7f94,A5) : player 3 inputs
  - 04b256.w (-$7f92,A5) : player 4 inputs
  - 04b248.b (-$7fa0,A5) : credits

  - 04570d.b (-$7fe0,A5 -> $05,A2) : bonus level (so lives aren't decremented) ? 0x00 = NO / 0x01 YES
  - 04570e.b (-$7fe4,A5 -> $06,A2) : level - range 0x00-0x63

  - 051fc0.b (-$7fd8,A5 -> A2 + $12 x 0 + 0x00) : player 1 active ? 0x00 = NO / 0x01 = YES / 0x02 = "ATTENDI"
  - 051fc1.b (-$7fd8,A5 -> A2 + $12 x 0 + 0x01) : player 1 lives - range 0x00-0x63
  - 051fc2.l (-$7fd8,A5 -> A2 + $12 x 0 + 0x02) : player 1 score
  - 051fd2.b (-$7fd8,A5 -> A2 + $12 x 1 + 0x00) : player 2 active ? 0x00 = NO / 0x01 = YES / 0x02 = "ATTENDI"
  - 051fd3.b (-$7fd8,A5 -> A2 + $12 x 1 + 0x01) : player 2 lives - range 0x00-0x63
  - 051fd4.l (-$7fd8,A5 -> A2 + $12 x 1 + 0x02) : player 2 score
  - 051fe4.b (-$7fd8,A5 -> A2 + $12 x 2 + 0x00) : player 3 active ? 0x00 = NO / 0x01 = YES / 0x02 = "ATTENDI"
  - 051fe5.b (-$7fd8,A5 -> A2 + $12 x 2 + 0x01) : player 3 lives - range 0x00-0x63
  - 051fe6.l (-$7fd8,A5 -> A2 + $12 x 2 + 0x02) : player 3 score
  - 051ff6.b (-$7fd8,A5 -> A2 + $12 x 3 + 0x00) : player 4 active ? 0x00 = NO / 0x01 = YES / 0x02 = "ATTENDI"
  - 051ff7.b (-$7fd8,A5 -> A2 + $12 x 3 + 0x01) : player 4 lives - range 0x00-0x63
  - 051ff8.l (-$7fd8,A5 -> A2 + $12 x 3 + 0x02) : player 4 score

routines :

  - 0746d6 : inputs read
  - 08251a : buttons + coin read
      * D0 = 0 : read potgo (player 2)
      * D0 = 1 : read potgo (player 1)

  - 07472e : coin verification


5) "Laser Strixx 2"

settings (A5=0x00a688 & A2=0x0027f8) :

  -                      : photo level - soft* / erotic / porno
  - 002910.l (A2 + 0x18) : difficulty (00010000) - 00008000 = LOW / 00010000 = NORMAL / 00018000 = HIGH
  - 002818.b (A2 + 0x20) : deadly selftouch (0) - 0 = NO / 1 = YES
  - 00281b.b (A2 + 0x23) : energy supply - 100%* / 0% / 50%
  - 002819.b (A2 + 0x21) : coin x 1 play (1) <=> number of credits used for each play
  - 00281a.b (A2 + 0x22) : play x credit (1) <=> credits awarded for each coin

useful addresses :

  - 00281c.b (-$7fa2,A5 -> $24,A2) : must be 0x00 instead of 0x01 to accept coins !
  - 0026f4.l (-$7f94,A5) : player 1 inputs (in MSW after swap)
  - 0026ee.b (-$7f9a,A5) : credits

  - 0027f8.l (A2 + 0x00) : player 1 energy (in MSW after swap) - range (0x00000000 - 0x00270000)
  - 0027fc.b (A2 + 0x04) : level - range (0x01 - 0x09 and 0x00 when in "attract mode")
  - 0027fe.w (A2 + 0x06) : time (not used for "continue play") - range (0x0000 - 0x0014 with 0x0000 being max)
  - 002800.l (A2 + 0x08) : player 1 score
  - 00280a.b (A2 + 0x12) : mouth status (0x00 = not out - 0x01 = out - 0x02 = captured = LIVE STRIP at the end of level)
  - 00280b.b (A2 + 0x13) : strip phase (0x01 - n with 0x01 being last phase)
                             Ann : n = 0x05
                             Eva : n = 0x04
                             Jo  : n = 0x06
                             Sue : n = 0x04
                             Bob : n = 0x05

routines :

  - 04d1ae : inputs read
  - 058340 : buttons + coin read
      * D0 = 0 : read potgo (player 2)
      * D0 = 1 : read potgo (player 1)

  - 04d1da : coin verification


6) "Magic Number"

settings (A5=0x053e78 & A2=0x03f540) : TO DO !

useful addresses :

  - 04bfa0.l (-$7ed8,A5) : must be 0x00000000 instead of 0x00010000 to accept coins !
  - 04bf18.w (-$7f60,A5) : player 1 inputs (in MSW after swap)
  - 04bf78.l (-$7e00,A5) : player 2 inputs (in MSW after swap)
  - 03f547.b (-$7f92,A5 -> $7,A2) : credits

routines :

  - 07a480 : inputs read
  - 085bb8 : buttons + coin read
      * D0 = 0 : read potgo (player 2)
      * D0 = 1 : read potgo (player 1)

  - 07a50e : coin verification


7) "Magic Premium"

settings (A5=0x04ce48 & A2=0x0419b0) : TO DO !

useful addresses :

  - 044f7e.b (-$7eca,A5) : must be 0x00 instead of 0x01 to accept coins !
  - 044f02.w (-$7f94,A5) : player 1 inputs
  - 044f6a.l (-$7ede,A5) : player 2 inputs (in MSW after swap)
  - 0419b1.b (-$7fc0,A5 -> $1,A2) : credits - range 0x00-0x09

routines :

  - 0707d6 : inputs read
  - 07b3b2 : buttons + coin read
      * D0 = 0 : read potgo (player 2)
      * D0 = 1 : read potgo (player 1)

  - 070802 : coin verification

*/

#include "emu.h"
#include "amiga.h"
#include "imagedev/cdromimg.h"
#include "machine/microtch.h"
#include "machine/nvram.h"
#include "speaker.h"


namespace {

/* set to 0 to use control panel with only buttons (as in quiz games) - joy is default in dispenser setup */
#define MGPREM11_USE_JOY    1
#define MGNUMBER_USE_JOY    1


class cubo_state : public amiga_state
{
public:
	cubo_state(const machine_config &mconfig, device_type type, const char *tag)
		: amiga_state(mconfig, type, tag)
		, m_player_ports(*this, {"P1", "P2"})
		, m_microtouch(*this, "microtouch")
		, m_akiko(*this, "akiko")
		, m_nvram(*this, "nvram")
	{ }

	void handle_joystick_cia(uint8_t pra, uint8_t dra);
	uint16_t handle_joystick_potgor(uint16_t potgor);

	ioport_value cubo_input();
	template <int P> int cd32_sel_mirror_input();

	void akiko_int_w(int state);
	void akiko_cia_0_port_a_write(uint8_t data);

	void init_cubo();
	void init_mgprem11();
	void init_odeontw2();
	void init_cndypuzl();
	void init_haremchl();
	void init_mgnumber();
	void init_lsrquiz2();
	void init_lasstixx();
	void init_lsrquiz();

	optional_ioport_array<2> m_player_ports;

	int m_oldstate[2]{};
	int m_cd32_shifter[2]{};
	uint16_t m_potgo_value = 0;

	void cubo(machine_config &config);
	void cubo_mem(address_map &map) ATTR_COLD;
	void overlay_2mb_map32(address_map &map) ATTR_COLD;
protected:
	virtual void machine_start() override;
	virtual void rs232_tx(int state) override;
	virtual void potgo_w(uint16_t data) override;

private:
	required_device<microtouch_device> m_microtouch;
	required_device<akiko_device> m_akiko;
	required_device<nvram_device> m_nvram;

	typedef void (cubo_state::*input_hack_func)();
	input_hack_func m_input_hack{};
	void chip_ram_w8_hack(offs_t byteoffs, uint8_t data);
	void cndypuzl_input_hack();
	void haremchl_input_hack();
	void lsrquiz_input_hack();
	void lsrquiz2_input_hack();
	void lasstixx_input_hack();
	void mgnumber_input_hack();
	void mgprem11_input_hack();

	std::unique_ptr<u8[]> m_nvram_data;
};

void cubo_state::machine_start()
{
	amiga_state::machine_start();

	m_nvram_data = make_unique_clear<u8[]>(0x800);
	subdevice<nvram_device>("nvram")->set_base(&m_nvram_data[0], 0x800);

	save_pointer(NAME(m_nvram_data), 0x800);
}

void cubo_state::akiko_int_w(int state)
{
	set_interrupt(INTENA_SETCLR | INTENA_PORTS);
}


/*************************************
 *
 *  CIA-A port A access:
 *
 *  PA7 = game port 1, pin 6 (fire)
 *  PA6 = game port 0, pin 6 (fire)
 *  PA5 = /RDY (disk ready)
 *  PA4 = /TK0 (disk track 00)
 *  PA3 = /WPRO (disk write protect)
 *  PA2 = /CHNG (disk change)
 *  PA1 = /LED (LED, 0=bright / audio filter control)
 *  PA0 = MUTE
 *
 *************************************/


void cubo_state::akiko_cia_0_port_a_write(uint8_t data)
{
	/* bit 0 = cd audio mute */
	m_akiko->set_mute(data & 1);

	/* bit 1 = Power Led on Amiga */
	m_power_led = BIT(~data, 1);

	handle_joystick_cia(data, m_cia_0->read(2));
}



void cubo_state::overlay_2mb_map32(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x1fffff).ram().share("chip_ram");
	map(0x200000, 0x27ffff).rom().region("kickstart", 0);
}

void cubo_state::cubo_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x1fffff).m(m_overlay, FUNC(address_map_bank_device::amap32));
	map(0x600000, 0x601fff).lrw8(
		NAME([this] (offs_t offset) { return m_nvram_data[offset]; }),
		NAME([this] (offs_t offset, u8 data) { m_nvram_data[offset] = data; })
	).umask32(0x000000ff);
	map(0x800000, 0x800003).portr("DIPSW1");
	map(0x800008, 0x80000b).portw("OUTPUT1");
	map(0x800010, 0x800013).portr("DIPSW2");
	map(0x800018, 0x80001b).portw("OUTPUT2");
	map(0xa80000, 0xb7ffff).noprw();
	map(0xb80000, 0xb8003f).rw(m_akiko, FUNC(akiko_device::read), FUNC(akiko_device::write));
	map(0xbf0000, 0xbfffff).rw(FUNC(cubo_state::cia_r), FUNC(cubo_state::gayle_cia_w));
	map(0xc00000, 0xdfffff).m(m_chipset, FUNC(address_map_bank_device::amap32));
	map(0xe00000, 0xe7ffff).rom().region("kickstart", 0x80000);
	map(0xe80000, 0xf7ffff).noprw();
	map(0xf80000, 0xffffff).rom().region("kickstart", 0);
}


/*************************************
 *
 *  Inputs
 *
 *************************************/

void cubo_state::rs232_tx(int state)
{
	m_microtouch->rx_w(state);
}

void cubo_state::potgo_w(uint16_t data)
{
	int i;

	if (m_input_hack != nullptr)
		(this->*m_input_hack)();

	m_potgo_value = m_potgo_value & 0x5500;
	m_potgo_value |= data & 0xaa00;

	for (i = 0; i < 8; i += 2)
	{
		uint16_t dir = 0x0200 << i;
		if (data & dir)
		{
			uint16_t d = 0x0100 << i;
			m_potgo_value &= ~d;
			m_potgo_value |= data & d;
		}
	}
	for (i = 0; i < 2; i++)
	{
		uint16_t p5dir = 0x0200 << (i * 4); /* output enable P5 */
		uint16_t p5dat = 0x0100 << (i * 4); /* data P5 */
		if ((m_potgo_value & p5dir) && (m_potgo_value & p5dat))
			m_cd32_shifter[i] = 8;
	}
}

void cubo_state::handle_joystick_cia(uint8_t pra, uint8_t dra)
{
	for (int i = 0; i < 2; i++)
	{
		uint8_t but = 0x40 << i;
		uint16_t p5dir = 0x0200 << (i * 4); /* output enable P5 */
		uint16_t p5dat = 0x0100 << (i * 4); /* data P5 */

		if (!(m_potgo_value & p5dir) || !(m_potgo_value & p5dat))
		{
			if ((dra & but) && (pra & but) != m_oldstate[i])
			{
				if (!(pra & but))
				{
					m_cd32_shifter[i]--;
					if (m_cd32_shifter[i] < 0)
						m_cd32_shifter[i] = 0;
				}
			}
		}
		m_oldstate[i] = pra & but;
	}
}

uint16_t cubo_state::handle_joystick_potgor(uint16_t potgor)
{
	for (int i = 0; i < 2; i++)
	{
		uint16_t p9dir = 0x0800 << (i * 4); /* output enable P9 */
		uint16_t p9dat = 0x0400 << (i * 4); /* data P9 */
		uint16_t p5dir = 0x0200 << (i * 4); /* output enable P5 */
		uint16_t p5dat = 0x0100 << (i * 4); /* data P5 */

		/* p5 is floating in input-mode */
		potgor &= ~p5dat;
		potgor |= m_potgo_value & p5dat;
		if (!(m_potgo_value & p9dir))
			potgor |= p9dat;
		/* P5 output and 1 -> shift register is kept reset (Blue button) */
		if ((m_potgo_value & p5dir) && (m_potgo_value & p5dat))
			m_cd32_shifter[i] = 8;
		/* shift at 1 == return one, >1 = return button states */
		if (m_cd32_shifter[i] == 0)
			potgor &= ~p9dat; /* shift at zero == return zero */
		if (m_cd32_shifter[i] >= 2 && ((m_player_ports[1 - i])->read() & (1 << (m_cd32_shifter[i] - 2))))
			potgor &= ~p9dat;
	}
	return potgor;
}

ioport_value cubo_state::cubo_input()
{
	return handle_joystick_potgor(m_potgo_value) >> 8;
}

template <int P>
int cubo_state::cd32_sel_mirror_input()
{
	uint8_t bits = m_player_ports[P]->read();
	return (bits & 0x20)>>5;
}



static INPUT_PORTS_START( cubo )
	PORT_START("CIA0PORTA")
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_CUSTOM )
	/* this is the regular port for reading a single button joystick on the Amiga, many CD32 games require this to mirror the pad start button! */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(cubo_state::cd32_sel_mirror_input<1>))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(cubo_state::cd32_sel_mirror_input<0>))

	PORT_START("CIA0PORTB")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("joy_0_dat")
	PORT_BIT( 0x0303, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(cubo_state::amiga_joystick_convert<1>))
	PORT_BIT( 0xfcfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("joy_1_dat")
	PORT_BIT( 0x0303, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(cubo_state::amiga_joystick_convert<0>))
	PORT_BIT( 0xfcfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("potgo")
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(cubo_state::cubo_input))
	PORT_BIT( 0x00ff, IP_ACTIVE_HIGH, IPT_UNUSED )


	// CD32 '11' button pad (4 dpad directions + 7 buttons), not read directly
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 ||>")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 <<") /* left trigger */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("P1 >>") /* right trigger */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("P1 Green")    /* BUTTON3 */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_NAME("P1 Yellow")   /* BUTTON4 */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER(1) PORT_NAME("P1 Red / SELECT")  /* BUTTON1 = START1 */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_PLAYER(1) PORT_NAME("P1 Blue / STOP") /* BUTTON2 */
	PORT_START("p1_joy")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)

	// CD32 '11' button pad (4 dpad directions + 7 buttons), not read directly
	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 ||>")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 <<") /* left trigger */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 >>") /* right trigger */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_NAME("P2 Green")   /* BUTTON3 */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(2) PORT_NAME("P2 Yellow")   /* BUTTON4 */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER(2) PORT_NAME("P2 Red / SELECT")  /* BUTTON1 = START2 */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_PLAYER(2) PORT_NAME("P2 Blue / STOP") /* BUTTON2 */
	PORT_START("p2_joy")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)


	PORT_START("DIPSW1")
	PORT_DIPNAME( 0x01, 0x01, "DSW1 1" )
	PORT_DIPSETTING(    0x01, "Reset" )
	PORT_DIPSETTING(    0x00, "Set" )
	PORT_DIPNAME( 0x02, 0x02, "DSW1 2" )
	PORT_DIPSETTING(    0x02, "Reset" )
	PORT_DIPSETTING(    0x00, "Set" )
	PORT_DIPNAME( 0x04, 0x04, "DSW1 3" )
	PORT_DIPSETTING(    0x04, "Reset" )
	PORT_DIPSETTING(    0x00, "Set" )
	PORT_DIPNAME( 0x08, 0x08, "DSW1 4" )
	PORT_DIPSETTING(    0x08, "Reset" )
	PORT_DIPSETTING(    0x00, "Set" )
	PORT_DIPNAME( 0x10, 0x10, "DSW1 5" )
	PORT_DIPSETTING(    0x10, "Reset" )
	PORT_DIPSETTING(    0x00, "Set" )
	PORT_DIPNAME( 0x20, 0x20, "DSW1 6" )
	PORT_DIPSETTING(    0x20, "Reset" )
	PORT_DIPSETTING(    0x00, "Set" )
	PORT_DIPNAME( 0x40, 0x40, "DSW1 7" )
	PORT_DIPSETTING(    0x40, "Reset" )
	PORT_DIPSETTING(    0x00, "Set" )
	PORT_DIPNAME( 0x80, 0x80, "DSW1 8" )
	PORT_DIPSETTING(    0x80, "Reset" )
	PORT_DIPSETTING(    0x00, "Set" )

	PORT_START("DIPSW2")
	PORT_DIPNAME( 0x01, 0x01, "DSW2 1" )
	PORT_DIPSETTING(    0x01, "Reset" )
	PORT_DIPSETTING(    0x00, "Set" )
	PORT_DIPNAME( 0x02, 0x02, "DSW2 2" )
	PORT_DIPSETTING(    0x02, "Reset" )
	PORT_DIPSETTING(    0x00, "Set" )
	PORT_DIPNAME( 0x04, 0x04, "DSW2 3" ) // PIC data
	PORT_DIPSETTING(    0x04, "Reset" )
	PORT_DIPSETTING(    0x00, "Set" )
	PORT_DIPNAME( 0x08, 0x08, "DSW2 4" )
	PORT_DIPSETTING(    0x08, "Reset" )
	PORT_DIPSETTING(    0x00, "Set" )
	PORT_DIPNAME( 0x10, 0x10, "DSW2 5" )
	PORT_DIPSETTING(    0x10, "Reset" )
	PORT_DIPSETTING(    0x00, "Set" )
	PORT_DIPNAME( 0x20, 0x20, "DSW2 6" ) // i2c SDA for RTC
	PORT_DIPSETTING(    0x20, "Reset" )
	PORT_DIPSETTING(    0x00, "Set" )
	PORT_DIPNAME( 0x40, 0x40, "DSW2 7" )
	PORT_DIPSETTING(    0x40, "Reset" )
	PORT_DIPSETTING(    0x00, "Set" )
	PORT_DIPNAME( 0x80, 0x80, "DSW2 8" )
	PORT_DIPSETTING(    0x80, "Reset" )
	PORT_DIPSETTING(    0x00, "Set" )

	PORT_START("OUTPUT1")
	// bit 1 pic clock
	// bit 2 hopper related
	// bit 3 pic data write
	// bit 6 pic enable
	// bit 7 hopper motor

	PORT_START("OUTPUT2")
	// bit 0 chip select (0) normal ticket (1) "Suzo" / "MKXX" motor, "erogatore megagettoni"
	// bit 4 ticket motor out
	// bit 5 i2c RTC data
	// bit 6 i2c RTC CS?
	// bit 7 i2c RTC clock
INPUT_PORTS_END

static INPUT_PORTS_START( cndypuzl )
	PORT_INCLUDE( cubo )

	PORT_MODIFY("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)    /* START1 and launch bubble */
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)    /* START2 and launch bubble */
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("DIPSW1")
	PORT_BIT( 0x003f, IP_ACTIVE_HIGH, IPT_UNKNOWN )                   /* bits 0 to 5 must be set to 0 to insert coin */
	PORT_SERVICE( 0x0040, IP_ACTIVE_HIGH )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_MODIFY("DIPSW2")
	PORT_BIT( 0x00ff, IP_ACTIVE_HIGH, IPT_UNKNOWN )                   /* not read at all */

INPUT_PORTS_END

static INPUT_PORTS_START( haremchl )
	PORT_INCLUDE( cubo )

	PORT_MODIFY("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)    /* fire */
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)    /* START1 and draw */
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)    /* retract */
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)    /* fire */
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)    /* START2 and draw */
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)    /* retract */
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("DIPSW1")
	PORT_BIT( 0x003f, IP_ACTIVE_HIGH, IPT_UNKNOWN )                   /* bits 0 to 5 must be set to 0 to insert coin */
	PORT_SERVICE( 0x0040, IP_ACTIVE_HIGH )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_MODIFY("DIPSW2")
	PORT_BIT( 0x00ff, IP_ACTIVE_HIGH, IPT_UNKNOWN )                   /* not read at all */

INPUT_PORTS_END

static INPUT_PORTS_START( lsrquiz )
	PORT_INCLUDE( cubo )

	PORT_MODIFY("p1_joy")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("p2_joy")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)    /* "C" */
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(1)    /* "D" */
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)    /* START1 and "A" */
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)    /* "B" */
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)    /* "C" */
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(2)    /* "D" */
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)    /* START2 and "A" */
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)    /* "B" */
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("DIPSW1")
	PORT_BIT( 0x003f, IP_ACTIVE_HIGH, IPT_UNKNOWN )                   /* bits 0 to 5 must be set to 0 to insert coin */
	PORT_SERVICE( 0x0040, IP_ACTIVE_HIGH )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_MODIFY("DIPSW2")
	PORT_BIT( 0x00ff, IP_ACTIVE_HIGH, IPT_UNKNOWN )                   /* not read at all */

INPUT_PORTS_END

static INPUT_PORTS_START( lsrquiz2 )
	PORT_INCLUDE( cubo )

	PORT_MODIFY("p1_joy")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("p2_joy")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)    /* "C" */
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(1)    /* "D" */
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)    /* START1 and "A" */
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)    /* "B" */
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)    /* "C" */
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(2)    /* "D" */
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)    /* START2 and "A" */
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)    /* "B" */
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("DIPSW1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_BUTTON3 ) PORT_PLAYER(3)    /* "C" */
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(3)    /* "B" */
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(3)    /* START3 and "A" */
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_BUTTON4 ) PORT_PLAYER(3)    /* "D" */
	PORT_DIPNAME( 0x0010, 0x0000, DEF_STR( Free_Play ) )                  /* always set credits to 10 */
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0000, "Speed" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0020, "Ultra Turbo" )                         /* the game is unplayable !!! */
	PORT_SERVICE( 0x0040, IP_ACTIVE_HIGH )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_MODIFY("DIPSW2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_BUTTON3 ) PORT_PLAYER(4)    /* "C" */
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(4)    /* "B" */
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(4)    /* START4 and "A" */
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_BUTTON4 ) PORT_PLAYER(4)    /* "D" */
	PORT_BIT( 0x00f0, IP_ACTIVE_HIGH, IPT_UNKNOWN )                   /* bits 4 to 7 aren't tested */

INPUT_PORTS_END

static INPUT_PORTS_START( lasstixx )
	PORT_INCLUDE( cubo )

	PORT_MODIFY("CIA0PORTA")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_MODIFY("p2_joy")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)    /* draw */
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)    /* retract */
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("DIPSW1")
	PORT_BIT( 0x003f, IP_ACTIVE_HIGH, IPT_UNKNOWN )                   /* bits 0 to 5 must be set to 0 to insert coin */
	PORT_SERVICE( 0x0040, IP_ACTIVE_HIGH )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_MODIFY("DIPSW2")
	PORT_BIT( 0x00ff, IP_ACTIVE_HIGH, IPT_UNKNOWN )                   /* not read at all */

INPUT_PORTS_END

static INPUT_PORTS_START( mgnumber )
	PORT_INCLUDE( cubo )

#if MGNUMBER_USE_JOY
	/* p1_joy, p2_joy, P1 and P2 inputs when control panel is set to "joystick" in the dispenser setup */
	PORT_MODIFY("p2_joy")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_NAME( "C (setup)" )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME( "Put Number" )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME( "Discard" )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME( "B (setup)" )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )
#else
	/* p1_joy, p2_joy, P1 and P2 inputs when control panel is set to "buttons" ("pulsanti") in the dispenser setup */
	/* p1_joy is still needed in the dispenser setup, so I don't remove it even if it isn't needed to play the game */
	PORT_MODIFY("p2_joy")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_NAME( "1 / C (setup)" )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME( "Discard" )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME( "B (setup)" )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_PLAYER(1) PORT_NAME( "4" )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON9 ) PORT_PLAYER(1) PORT_NAME( "5" )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER(1) PORT_NAME( "2" )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_PLAYER(1) PORT_NAME( "3" )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )
#endif

	PORT_MODIFY("DIPSW1")
	PORT_DIPNAME( 0x01, 0x00, "Tokens" )                              /* Dip Switch or Input ? */
	PORT_DIPSETTING(    0x00, "OK" )
	PORT_DIPSETTING(    0x01, "ERROR!" )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_COIN1 )                     /* keep pressed to enter dispenser setup at start */
	PORT_BIT( 0x007e, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("DIPSW2")
	PORT_DIPNAME( 0x01, 0x00, "Tickets" )                             /* Dip Switch or Input ? */
	PORT_DIPSETTING(    0x00, "OK" )
	PORT_DIPSETTING(    0x01, "ERROR!" )
	PORT_BIT( 0x00fe, IP_ACTIVE_HIGH, IPT_UNKNOWN )

INPUT_PORTS_END

static INPUT_PORTS_START( mgprem11 )
	PORT_INCLUDE( cubo )

#if MGPREM11_USE_JOY
	/* p1_joy, p2_joy, P1 and P2 inputs when control panel is set to "joystick" in the dispenser setup */
	PORT_MODIFY("p1_joy")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("p2_joy")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN  )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN  )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_BUTTON3  ) PORT_PLAYER(1) PORT_NAME( "End Game / Abort / Confirm" )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON5  ) PORT_PLAYER(1) PORT_NAME( "C (setup)" )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1  ) PORT_PLAYER(1) PORT_NAME( "Put Card / Initialise / Continue Game / Cancel" )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2  ) PORT_PLAYER(1) PORT_NAME( "Discard / P1 A (setup)" )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON4  ) PORT_PLAYER(1) PORT_NAME( "B (setup)" )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN  )

	PORT_MODIFY("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN  )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN  )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNKNOWN  )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNKNOWN  )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN  )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON7  ) PORT_PLAYER(1) PORT_NAME( "P2 A (setup)" )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON10 ) PORT_PLAYER(1) PORT_NAME( "P2 B (setup)" )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN  )
#else
	/* p1_joy, p2_joy, P1 and P2 inputs when control panel is set to "buttons" ("pulsanti") in the dispenser setup */
	/* p1_joy is still needed in the dispenser setup, so I don't remove it even if it isn't needed to play the game */
	PORT_MODIFY("p1_joy")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("p2_joy")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN  )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN  )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_BUTTON3  ) PORT_PLAYER(1) PORT_NAME( "End Game / Abort / Confirm" )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON5  ) PORT_PLAYER(1) PORT_NAME( "1 / C (setup)" )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1  ) PORT_PLAYER(1) PORT_NAME( "Initialise / Continue Game / Cancel" )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2  ) PORT_PLAYER(1) PORT_NAME( "Discard / P1 A (setup)" )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON4  ) PORT_PLAYER(1) PORT_NAME( "B (setup)" )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN  )

	PORT_MODIFY("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN  )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN  )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_BUTTON8  ) PORT_PLAYER(1) PORT_NAME( "4" )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON9  ) PORT_PLAYER(1) PORT_NAME( "5" )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON6  ) PORT_PLAYER(1) PORT_NAME( "2" )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON7  ) PORT_PLAYER(1) PORT_NAME( "3 / P2 A (setup)" )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON10 ) PORT_PLAYER(1) PORT_NAME( "P2 B (setup)" )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN  )
#endif

	PORT_MODIFY("DIPSW1")
	PORT_DIPNAME( 0x01, 0x00, "Tokens" )                              /* Dip Switch or Input ? */
	PORT_DIPSETTING(    0x00, "OK" )
	PORT_DIPSETTING(    0x01, "ERROR!" )
	PORT_DIPNAME( 0x50, 0x50, "Setup" )                               /* also affects payout values */
	PORT_DIPSETTING(    0x50, "Full Tick" )
//  PORT_DIPSETTING(    0x10, "Full Tick" )                           /* duplicated setting */
	PORT_DIPSETTING(    0x40, "104 & 105" )
	PORT_DIPSETTING(    0x00, "Full T+C" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )                    /* tested in dispenser setup when P1B3 is pressed */
	PORT_DIPSETTING(    0x20, "0x20" )
	PORT_DIPSETTING(    0x00, "0x00" )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_COIN1 )                     /* keep pressed to enter dispenser setup at start */
	PORT_BIT( 0x000e, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("DIPSW2")
	PORT_DIPNAME( 0x01, 0x00, "Tickets" )                             /* Dip Switch or Input ? */
	PORT_DIPSETTING(    0x00, "OK" )
	PORT_DIPSETTING(    0x01, "ERROR!" )
	PORT_BIT( 0x00fe, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( eldoralg )
	PORT_INCLUDE( cubo )

	// TODO: verify layout
	PORT_MODIFY("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("P1 G") PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("P1 F") PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P1 C") PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("P1 D") PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 A") PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("P1 E") PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 B") PORT_PLAYER(1)

	PORT_MODIFY("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("P2 G") PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("P2 F") PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P2 C") PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("P2 D") PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P2 A") PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("P2 E") PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P2 B") PORT_PLAYER(2)

	PORT_MODIFY("DIPSW1")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_COIN1 )                     /* Hold at boot for test mode */

	PORT_MODIFY("DIPSW2")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_COIN2 )
INPUT_PORTS_END

void cubo_state::cubo(machine_config &config)
{
	/* basic machine hardware */
	M68EC020(config, m_maincpu, amiga_state::CLK_28M_PAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &cubo_state::cubo_mem);
	m_maincpu->reset_cb().set(FUNC(amiga_state::m68k_reset));

	ADDRESS_MAP_BANK(config, m_overlay).set_map(&cubo_state::overlay_2mb_map32).set_options(ENDIANNESS_BIG, 32, 22, 0x200000);
	ADDRESS_MAP_BANK(config, m_chipset).set_map(&cubo_state::aga_map).set_options(ENDIANNESS_BIG, 32, 9, 0x200);

	AGNUS_COPPER(config, m_copper, amiga_state::CLK_28M_PAL / 2);
	m_copper->set_host_cpu_tag(m_maincpu);
	m_copper->mem_read_cb().set(FUNC(amiga_state::chip_ram_r));
	m_copper->set_ecs_mode(true);

	I2C_24C08(config, "i2cmem", 0); // AT24C08N

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1); // unknown type, odeontw accesses it

	AKIKO(config, m_akiko, 0);
	m_akiko->mem_r_callback().set(FUNC(amiga_state::chip_ram_r));
	m_akiko->mem_w_callback().set(FUNC(amiga_state::chip_ram_w));
	m_akiko->int_callback().set(FUNC(cubo_state::akiko_int_w));
	m_akiko->scl_callback().set("i2cmem", FUNC(i2cmem_device::write_scl));
	m_akiko->sda_r_callback().set("i2cmem", FUNC(i2cmem_device::read_sda));
	m_akiko->sda_w_callback().set("i2cmem", FUNC(i2cmem_device::write_sda));

	// video hardware
	pal_video(config);
	m_screen->set_screen_update(FUNC(amiga_state::screen_update));
	PALETTE(config, m_palette, FUNC(amiga_state::amiga_palette), 4096);

	MCFG_VIDEO_START_OVERRIDE(amiga_state, amiga_aga)

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	PAULA_8364(config, m_paula, amiga_state::CLK_C1_PAL);
	m_paula->add_route(0, "lspeaker", 0.25);
	m_paula->add_route(1, "rspeaker", 0.25);
	m_paula->add_route(2, "rspeaker", 0.25);
	m_paula->add_route(3, "lspeaker", 0.25);
	m_paula->mem_read_cb().set(FUNC(amiga_state::chip_ram_r));
	m_paula->int_cb().set(FUNC(amiga_state::paula_int_w));

	/* cia */
	// these are setup differently on other amiga drivers (needed for floppy to work) which is correct / why?
	MOS8520(config, m_cia_0, amiga_state::CLK_E_PAL);
	m_cia_0->irq_wr_callback().set(FUNC(amiga_state::cia_0_irq));
	m_cia_0->pa_rd_callback().set_ioport("CIA0PORTA");
	m_cia_0->pa_wr_callback().set(FUNC(cubo_state::akiko_cia_0_port_a_write));

	MOS8520(config, m_cia_1, amiga_state::CLK_E_PAL);
	m_cia_1->irq_wr_callback().set(FUNC(amiga_state::cia_1_irq));

	MICROTOUCH(config, m_microtouch, 9600).stx().set(FUNC(cubo_state::rs232_rx_w));

	/* fdc */
	PAULA_FDC(config, m_fdc, amiga_state::CLK_7M_PAL);
	m_fdc->index_callback().set("cia_1", FUNC(mos8520_device::flag_w));
	m_fdc->read_dma_callback().set(FUNC(amiga_state::chip_ram_r));
	m_fdc->write_dma_callback().set(FUNC(amiga_state::chip_ram_w));
	m_fdc->dskblk_callback().set(FUNC(amiga_state::fdc_dskblk_w));
	m_fdc->dsksyn_callback().set(FUNC(amiga_state::fdc_dsksyn_w));
}



#define ROM_LOAD16_WORD_BIOS(bios,name,offset,length,hash)     ROMX_LOAD(name, offset, length, hash, ROM_BIOS(bios))

#define CD32_BIOS \
	ROM_REGION32_BE(0x100000, "kickstart", 0 ) \
	ROM_SYSTEM_BIOS(0, "cd32", "Kickstart v3.1 rev 40.60 with CD32 Extended-ROM" ) \
	ROM_LOAD16_WORD_BIOS(0, "391640-03.u6a", 0x000000, 0x100000, CRC(d3837ae4) SHA1(06807db3181637455f4d46582d9972afec8956d9) )

ROM_START( cubo )
	CD32_BIOS
ROM_END

/***************************************************************************************************/

void cubo_state::init_cubo()
{
	m_agnus_id = ALICE_PAL_NEW;
	m_denise_id = LISA;
	m_input_hack = nullptr;
}



ROM_START( cndypuzl )
	CD32_BIOS

	ROM_REGION(0x1000, "pic", 0) // unknown type
	ROM_LOAD( "pic.bin", 0x000000, 0x1000, NO_DUMP )

	DISK_REGION( "akiko" )
	DISK_IMAGE_READONLY( "cndypuzl", 0, BAD_DUMP SHA1(5f41ed3521b3e05d233ac1245b78cb0b118b2b90) )
ROM_END

ROM_START( haremchl )
	CD32_BIOS

	ROM_REGION(0x1000, "pic", 0) // unknown type
	ROM_LOAD( "pic.bin", 0x000000, 0x1000, NO_DUMP )

	DISK_REGION( "akiko" )
	DISK_IMAGE_READONLY( "haremchl", 0, BAD_DUMP SHA1(abbab347c0d7c5eef0465d0eee770754a452e874) )
ROM_END

ROM_START( lsrquiz )
	CD32_BIOS

	ROM_REGION(0x1000, "pic", 0) // unknown type
	ROM_LOAD( "pic.bin", 0x000000, 0x1000, NO_DUMP )

	DISK_REGION( "akiko" )
	DISK_IMAGE_READONLY( "lsrquiz", 0, BAD_DUMP SHA1(41fb6cd0c9d36bd77e9c3db69d36801edc791e96) )
ROM_END

ROM_START( lsrquiz2i )
	CD32_BIOS

	ROM_REGION(0x1000, "pic", 0) // unknown type
	ROM_LOAD( "pic.bin", 0x000000, 0x1000, NO_DUMP )

	DISK_REGION( "akiko" )
	DISK_IMAGE_READONLY( "lsrquiz2", 0, BAD_DUMP SHA1(78e261df1c548fa492e6cf37a9469640bb8816bf) )
ROM_END

ROM_START( lsrquizg )
	CD32_BIOS

	ROM_REGION(0x1000, "pic", 0) // unknown type
	ROM_LOAD( "pic.bin", 0x000000, 0x1000, NO_DUMP )

	DISK_REGION( "akiko" )
	DISK_IMAGE_READONLY( "laserquizgreek2pro", 0, BAD_DUMP SHA1(8538915b4a0078f19197a5562e37ed3e6d0429a4) )
ROM_END

ROM_START( mgprem11 )
	CD32_BIOS

	ROM_REGION(0x1000, "pic", 0) // unknown type
	ROM_LOAD( "pic.bin", 0x000000, 0x1000, NO_DUMP )

	DISK_REGION( "akiko" )
	DISK_IMAGE_READONLY( "mgprem11", 0, BAD_DUMP SHA1(7808db33d5949f6c86d12b32bc388c12377e7038) )
ROM_END

ROM_START( lasstixx )
	CD32_BIOS

	ROM_REGION(0x1000, "pic", 0) // unknown type
	ROM_LOAD( "pic.bin", 0x000000, 0x1000, NO_DUMP )

	DISK_REGION( "akiko" )
	DISK_IMAGE_READONLY( "lasstixx", 0, BAD_DUMP SHA1(b8f6138e1f1840c193e786c56dab03c512f3e21f) )
ROM_END

ROM_START( mgnumber )
	CD32_BIOS

	ROM_REGION(0x1000, "pic", 0) // unknown type
	ROM_LOAD( "pic.bin", 0x000000, 0x1000, NO_DUMP )

	DISK_REGION( "akiko" )
	DISK_IMAGE_READONLY( "magicnumber", 0, BAD_DUMP SHA1(60e1fadc42694742d19cc0ac2b6e99e9e33faa3d) )
ROM_END

ROM_START( odeontw )
	CD32_BIOS

	ROM_REGION(0x1000, "pic", 0) // unknown type
	ROM_LOAD( "pic.bin", 0x000000, 0x1000, NO_DUMP )

	DISK_REGION( "akiko" )
	DISK_IMAGE_READONLY( "twister32_17_3", 0, BAD_DUMP SHA1(a40ec484708e22059f7186415283084ebf01323e) ) // has its audio cut a little, worth to mark as redump needed
ROM_END

ROM_START( odeontw2 )
	CD32_BIOS

	ROM_REGION(0x1000, "pic", 0) // unknown type
	ROM_LOAD( "pic.bin", 0x000000, 0x1000, NO_DUMP )

	DISK_REGION( "akiko" )
	DISK_IMAGE_READONLY( "odeontw2", 0, BAD_DUMP SHA1(f39e09f35b65a6ae9f1eba4a22f970626b7d3b71) )
ROM_END

ROM_START( eldoralg )
	CD32_BIOS

	ROM_REGION(0x1000, "pic", 0) // unknown type
	ROM_LOAD( "pic.bin", 0x000000, 0x1000, NO_DUMP )

	DISK_REGION( "akiko" )
	DISK_IMAGE_READONLY( "eldorado", 0, BAD_DUMP SHA1(bc1617c2e3438b729421c1d8b1bf88840b12f030) )
ROM_END


/*************************************
 *
 *  Hacks (to allow coins to be inserted)
 *
 *************************************/

void cubo_state::chip_ram_w8_hack(offs_t byteoffs, uint8_t data)
{
	uint16_t word = read_chip_ram(byteoffs);

	if (byteoffs & 1)
		word = (word & 0xff00) | data;
	else
		word = (word & 0x00ff) | (((uint16_t)data) << 8);

	write_chip_ram(byteoffs, word);
}

void cubo_state::cndypuzl_input_hack()
{
	if (m_maincpu->pc() < m_chip_ram.bytes())
	{
		uint32_t r_A5 = m_maincpu->state_int(M68K_A5);
		write_chip_ram(r_A5 - 0x7ebe, 0x0000);
	}
}

void cubo_state::init_cndypuzl()
{
	init_cubo();
	m_input_hack = &cubo_state::cndypuzl_input_hack;
}

void cubo_state::haremchl_input_hack()
{
	if (m_maincpu->pc() < m_chip_ram.bytes())
	{
		uint32_t r_A5 = m_maincpu->state_int(M68K_A5);
		uint32_t r_A2 = (read_chip_ram(r_A5 - 0x7f00 + 0) << 16) | (read_chip_ram(r_A5 - 0x7f00 + 2));
		chip_ram_w8_hack(r_A2 + 0x1f, 0x00);
	}
}

void cubo_state::init_haremchl()
{
	init_cubo();
	m_input_hack = &cubo_state::haremchl_input_hack;
}

void cubo_state::lsrquiz_input_hack()
{
	if (m_maincpu->pc() < m_chip_ram.bytes())
	{
		uint32_t r_A5 = m_maincpu->state_int(M68K_A5);
		uint32_t r_A2 = (read_chip_ram(r_A5 - 0x7fe0 + 0) << 16) | (read_chip_ram(r_A5 - 0x7fe0 + 2));
		chip_ram_w8_hack(r_A2 + 0x13, 0x00);
	}
}

void cubo_state::init_lsrquiz()
{
	init_cubo();
	m_input_hack = &cubo_state::lsrquiz_input_hack;
}

/* The hack isn't working if you exit the test mode with P1 button 2 ! */
void cubo_state::lsrquiz2_input_hack()
{
	if (m_maincpu->pc() < m_chip_ram.bytes())
	{
		uint32_t r_A5 = m_maincpu->state_int(M68K_A5);
		uint32_t r_A2 = (read_chip_ram(r_A5 - 0x7fdc + 0) << 16) | (read_chip_ram(r_A5 - 0x7fdc + 2));
		chip_ram_w8_hack(r_A2 + 0x17, 0x00);
	}
}

void cubo_state::init_lsrquiz2()
{
	init_cubo();
	m_input_hack = &cubo_state::lsrquiz2_input_hack;
}

void cubo_state::lasstixx_input_hack()
{
	if (m_maincpu->pc() < m_chip_ram.bytes())
	{
		uint32_t r_A5 = m_maincpu->state_int(M68K_A5);
		uint32_t r_A2 = (read_chip_ram(r_A5 - 0x7fa2 + 0) << 16) | (read_chip_ram(r_A5 - 0x7fa2 + 2));
		chip_ram_w8_hack(r_A2 + 0x24, 0x00);
	}
}

void cubo_state::init_lasstixx()
{
	init_cubo();
	m_input_hack = &cubo_state::lasstixx_input_hack;
}

void cubo_state::mgnumber_input_hack()
{
	if (m_maincpu->pc() < m_chip_ram.bytes())
	{
		uint32_t r_A5 = m_maincpu->state_int(M68K_A5);
		write_chip_ram(r_A5 - 0x7ed8, 0x0000);
	}
}

void cubo_state::init_mgnumber()
{
	init_cubo();
	m_input_hack = &cubo_state::mgnumber_input_hack;
}

void cubo_state::mgprem11_input_hack()
{
	if (m_maincpu->pc() < m_chip_ram.bytes())
	{
		uint32_t r_A5 = m_maincpu->state_int(M68K_A5);
		chip_ram_w8_hack(r_A5 - 0x7eca, 0x00);
	}
}

void cubo_state::init_mgprem11()
{
	init_cubo();
	m_input_hack = &cubo_state::mgprem11_input_hack;
}

} // anonymous namespace


GAME( 1993, cubo,      0,    cubo, cubo,     cubo_state, init_cubo,     ROT0, "Commodore",     "Cubo BIOS",                 MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_IS_BIOS_ROOT )
GAME( 1995, cndypuzl,  cubo, cubo, cndypuzl, cubo_state, init_cndypuzl, ROT0, "CD Express",    "Candy Puzzle (v1.0)",       MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1995, haremchl,  cubo, cubo, haremchl, cubo_state, init_haremchl, ROT0, "CD Express",    "Harem Challenge",           MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1995, lsrquiz,   cubo, cubo, lsrquiz,  cubo_state, init_lsrquiz,  ROT0, "CD Express",    "Laser Quiz Italy",          MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND ) // no player 2 inputs (ingame), wrong pitch for most gfxs, access violation during gameplay or on emu exit (microtouch?)
GAME( 1995, lsrquizg,  cubo, cubo, lsrquiz,  cubo_state, init_lsrquiz,  ROT0, "CD Express",    "Laser Quiz Greece",         MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND ) // doesn't accept coins, no player 2 inputs (ingame), wrong pitch for most gfxs, access violation during gameplay or on emu exit (microtouch?)
GAME( 1995, lsrquiz2i, cubo, cubo, lsrquiz2, cubo_state, init_lsrquiz2, ROT0, "CD Express",    "Laser Quiz 2 Italy (v1.0)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND ) // wrong pitch for some gfxs, access violation during gameplay (microtouch?)
GAME( 1995, lasstixx,  cubo, cubo, lasstixx, cubo_state, init_lasstixx, ROT0, "CD Express",    "Laser Strixx 2",            MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1995, mgnumber,  cubo, cubo, mgnumber, cubo_state, init_mgnumber, ROT0, "CD Express",    "Magic Number",              MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1996, mgprem11,  cubo, cubo, mgprem11, cubo_state, init_mgprem11, ROT0, "CD Express",    "Magic Premium (v1.1)",      MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1997, eldoralg,  cubo, cubo, eldoralg, cubo_state, init_cubo,     ROT0, "Shangai Games", "Eldorado (4.2)",            MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND ) // touchscreen is offset and throws errors in calibration menu, joystick buttons aren't recognized properly in places, uses SPRES=3 (SHRES) for roulette ball in attract
GAME( 1998, odeontw,   cubo, cubo, eldoralg, cubo_state, init_cubo,     ROT0, "CD Express",    "Odeon Twister (v1.4)",      MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND ) // claims invalid RAM config once intialized (i2c RTC?), hangs with NVRAM viewer in service mode
GAME( 1998, odeontw2,  cubo, cubo, eldoralg, cubo_state, init_cubo,     ROT0, "CD Express",    "Odeon Twister 2 (v202.19)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND ) // "Security checking failed" once initialized
// Laser Gate 2, alt title for Eldorado?
// Lucky Five
// Greyhound Race
// Double Strixx
