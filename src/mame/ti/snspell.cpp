// license:BSD-3-Clause
// copyright-holders:hap, Jonathan Gevaryahu
// thanks-to:Sean Riddle, David Viens, Kevin Horton
/*******************************************************************************

Texas Instruments 1st-gen. Speak & Spell family

These devices, mostly edu-toys, are based around an MCU(TMS0270), TMS51xx speech,
and VSM ROM(s). Newer devices, such as Speak & Music, are based around the
TMP50C40 and belong in another driver, probably.

MAME external artwork is not required. But it is objectively a large improvement.
For the 'Compact' series, see the snspellc driver.

TODO:
- why doesn't lantransp work? maybe incompatible with released modules?

================================================================================

Known devices on this hardware: (* denotes not dumped)

Speak & Spell:

This is the original Speak & Spell. TI had done educational toys before, like
Wiz-A-Tron or Little Professor. But the popularity of this product was much
above expectations. TI continued to manufacture many products for this line.

Speak & Spell (US), 1978
- MCU: TMC0271, label TMC0271NL DBS (die label: T0270B, 0271B)
- TMS51xx: TMC0281
- VSM(1/2): 16KB TMC0351NL
- VSM(2/2): 16KB TMC0352NL
- VFD: NEC FIP8A5AR no. 3A
- notes: keyboard has buttons instead of cheap membrane

Speak & Spell (US), 1980
- MCU: TMC0271, label TMC0271H-N2L FDS (die label: T0270D, 0271H)
- TMS51xx: TMC0281
- VSM(1/2): 16KB TMC0351N2L
- VSM(2/2): 16KB TMC0352N2L
- notes: fixed a funny bug with gibberish-talk when Module button is pressed
  with no module inserted, MCU ROM contents differs from 1978 version

Speak & Spell (US), 1981
- MCU: same as 1980 version
- TMS51xx: TMC0281D
- VSM: 16KB CD2350A
- notes: only 1 VSM, meaning much smaller internal vocabulary

Speak & Spell (Japan), 1980
- MCU: TMC0271 (assume same as US 1980 version)
- TMS51xx: TMC0281
- VSM(1/2): 16KB CD2321
- VSM(2/2): 16KB CD2322
- notes: no local name for the product, words are in English but very low difficulty

Speak & Spell (UK), 1978
- MCU: TMC0271 (assume same as US 1978 version)
- TMS51xx: TMC0281
- VSM(1/2): 16KB CD2303
- VSM(2/2): 16KB CD2304
- notes: voice data was manually altered to give it a UK accent,
  here's a small anecdote from developer:
  "(...) I cannot bear to listen the product even now. I remember the
   word 'butcher' took 3 days - I still don't know if it sounds right."

Speak & Spell (UK), 1981
- MCU: TMC0271 (assume same as US 1980 version)
- TMS51xx: CD2801
- VSM: 16KB CD62175
- VFD: some seen with the one from Speak & Math(!)
- notes: this one has a dedicated voice actor

Speak & Spell (Spanish, prototype), 1980
- MCU: CD2701N2L P (die label: T0270D, 2701)
- TMS51xx: TMC0281 (die label: T0280A 0281)
- VSM(1/2): 16KB CD2319
- VSM(2/2): 16KB CD2320
- VFD: 8 digits with 14 segments, DP and accent mark

Speak & Spell (France) "La Dictée Magique", 1981
- MCU: CD2702, label CD2702AN2L (die label: TMC0270F, 2702A)
- TMS51xx: CD2801
- VSM: 16KB CD2352

Speak & Spell (Germany) "Buddy", 1980 (stylized as "buddy")
- MCU & TMS51xx: same as French version
- VSM(1/2): 16KB CD2345*
- VSM(2/2): 16KB CD2346*
- VFD: has umlaut instead of apostrophe

Speak & Spell (Italy) "Grillo Parlante", 1983
- MCU & TMS51xx: same as French version
- VSM: 16KB CD62190
- VFD: same as Speak & Math

Speak & Spell modules:
Note that they are interchangeable, eg. you can use a French module on a US Speak & Spell.

English:
- Vowel Power: VSM: 16KB CD2302
- Number Stumpers 4-6: VSM: 16KB CD2305
- Number Stumpers 7-8: VSM: 16KB CD2307A
- Basic Builders: VSM: 16KB CD2308
- Mighty Verbs: VSM: 16KB CD2309(rev.B)
- Homonym Heroes: VSM: 16KB CD2310
- Vowel Ventures: VSM: 16KB CD2347(rev.C)
- Noun Endings: VSM: 16KB CD2348
- Magnificent Modifiers: VSM: 16KB CD2349
- E.T. Fantasy: VSM: 16KB CD2360

French:
- No.1: Les Mots de Base: VSM: 16KB CD2353 (1st release was called "Module No. 1 de Jacques Capelovici")
- No.2: Les Mots Difficiles (aka Les Mots de Base): VSM: 16KB CD62177A
- No.3: Les Animaux Familiers: VSM: 16KB CD62047*
- No.4: Les Magasins de la Rue: VSM: 16KB CD62048
- No.5: Les Extra-Terrestres: VSM: 16KB CD62178*

Italian:
- Super Modulo: VSM: 16KB? CD62313*

================================================================================

Speak & Math:

Speak & Math (US), 1980 (renamed to "Speak & Maths" in UK, but is the same product)
- MCU: CD2704, label CD2704B-N2L (die label: TMC0270F, 2704B) - 2nd revision?(mid-1982)
- TMS51xx: CD2801
- VSM(1/2): 16KB CD2392
- VSM(2/2): 16KB CD2393
- VFD: Futaba 9SY -02Z 7E
- notes: As with the Speak & Spell, the voice actor was a radio announcer.
  However, the phrase "is greater than or less than" had to be added by one
  of the TI employees in a hurry, the day before a demo. Apparently QA
  never found out and it ended up in the final product.

Speak & Math (US), 1984
- MCU: CD2708, label CD2708N2L (die label: TMC0270F, 2708A)
- TMS51xx: CD2801
- VSM(1/2): 16KB CD2381
- VSM(2/2): 4KB CD2614

================================================================================

Speak & Read:

Speak & Read (US), 1980
- MCU: CD2705, label CD2705B-N2L (die label: TMC0270E, 2705B) - 2nd revision?(late-1981)
- TMS51xx: CD2801
- VSM(1/2): 16KB CD2394A
- VSM(2/2): 16KB CD2395A
- VFD: same as Language Translator, rightmost digit unused

Speak & Read modules:

English:
- Sea Sights: VSM: 16KB CD2396A
- Who's Who at the Zoo: VSM: 16KB CD2397
- A Dog on a Log: VSM: 16KB CD3534A
- The Seal That Could Fly: VSM: 16KB CD3535
- A Ghost in the House: VSM: 16KB CD3536
- On the Track: VSM: 16KB CD3538
- The Third Circle: VSM: 16KB CD3539
- The Millionth Knight: VSM: 16KB CD3540

================================================================================

Language Translator/Tutor:

Initially sold as Language Translator, renamed to Language Tutor a year later.
It was rebranded from translator to a 'language aid'.

Language Translator (US), 1979
- MCU: TMC0275 (die label: T0270D, 0275B)
- TMS51xx: CD2801 (die label: T0280B 2801)
- VFD: Itron FG106A2
- notes: external module is required (see below)

Language Tutor (US), 1980
- notes: identical hardware, module stickers differ but have same VSMs

Language Translator modules:

- Ingles(1/4): VSM: 16KB CD2311
- Ingles(2/4): VSM: 16KB CD2312
- Ingles(3/4): VSM: 16KB CD2313
- Ingles(4/4): VSM: 16KB CD2314

- Spanish(1/4): VSM: 16KB CD2315
- Spanish(2/4): VSM: 16KB CD2316
- Spanish(3/4): VSM: 16KB CD2317
- Spanish(4/4): VSM: 16KB CD2318

- French(1/4): VSM: 16KB CD2327
- French(2/4): VSM: 16KB CD2328
- French(3/4): VSM: 16KB CD2329
- French(4/4): VSM: 16KB CD2330

- German(1/4): VSM: 16KB CD2331
- German(2/4): VSM: 16KB CD2332
- German(3/4): VSM: 16KB CD2333
- German(4/4): VSM: 16KB CD2334

- English(1/4): VSM: 16KB CD3526
- English(2/4): VSM: 16KB CD3527
- English(3/4): VSM: 16KB CD3528
- English(4/4): VSM: 16KB CD3529

Some other language modules were announced by TI, but not released.

================================================================================

Language Teacher:

A cost-reduced version of the translator. Similar PCB, the speaker was removed.
Modules require less ROMs due to missing speech data. Translator/Teacher modules
are not interchangeable.

Language Teacher (US), 1980
- MCU: TMC0270N2LP CD2706 (die label: T0270D, 2706)
- other: see Language Translator

Language Teacher modules (only 1 known released):

- German For Travel: VSM: 16KB CD3509

*******************************************************************************/

#include "emu.h"

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "cpu/tms1000/tms0270.h"
#include "machine/tms6100.h"
#include "sound/tms5110.h"
#include "video/pwm.h"

#include "softlist_dev.h"
#include "speaker.h"

// internal artwork
#include "snmath.lh"
#include "snread.lh"
#include "snspell.lh"
#include "snspellsp.lh"


namespace {

// The master clock is a single stage RC oscillator into TMS5100 RCOSC:
// In an early 1979 Speak & Spell, C is 68pf, R is a 50kohm trimpot which is set to around 33.6kohm
// (measured in-circuit). CPUCLK is this osc freq /2, ROMCLK is this osc freq /4.
// The typical osc freq curve for TMS5100 is unknown. Let's assume it is set to the default frequency,
// which is 640kHz for 8KHz according to the TMS5100 documentation.

static constexpr u32 MASTER_CLOCK = 640'000;


class snspell_state : public driver_device
{
public:
	snspell_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_display(*this, "display"),
		m_tms5100(*this, "tms5100"),
		m_tms6100(*this, "tms6100"),
		m_cart(*this, "cartslot"),
		m_softlist(*this, "cart_list"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	void init_snspell();
	void init_lantrans();

	// machine configs
	void tms5110_route(machine_config &config);
	void sns_tmc0281(machine_config &config);
	void sns_tmc0281d(machine_config &config);
	void sns_cd2801(machine_config &config);
	void snspellit(machine_config &config);
	void snspellsp(machine_config &config);
	void snmath(machine_config &config);
	void snread(machine_config &config);
	void lantrans(machine_config &config);
	void lanteach(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(power_on);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	// devices
	required_device<tms0270_cpu_device> m_maincpu;
	required_device<pwm_display_device> m_display;
	required_device<tms5110_device> m_tms5100;
	required_device<tms6100_device> m_tms6100;
	optional_device<generic_slot_device> m_cart;
	optional_device<software_list_device> m_softlist;
	required_ioport_array<9> m_inputs;

	u32 m_cart_max_size = 0;
	u8 *m_cart_base = nullptr;

	bool m_power_on = false;
	u32 m_r = 0;
	u16 m_grid = 0;
	u16 m_plate = 0;

	void power_off();
	void update_display();

	u8 read_k();
	void write_o(u16 data);
	void write_r(u32 data);

	void snmath_write_o(u16 data);
	void lantrans_write_r(u32 data);

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);
};

void snspell_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_power_on));
	save_item(NAME(m_r));
	save_item(NAME(m_grid));
	save_item(NAME(m_plate));
}



/*******************************************************************************
    Power
*******************************************************************************/

void snspell_state::machine_reset()
{
	m_power_on = true;
	m_maincpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
}

INPUT_CHANGED_MEMBER(snspell_state::power_on)
{
	if (newval && !m_power_on)
		machine_reset();
}

void snspell_state::power_off()
{
	m_power_on = false;
	m_maincpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_display->clear();
}



/*******************************************************************************
    Cartridge
*******************************************************************************/

DEVICE_IMAGE_LOAD_MEMBER(snspell_state::cart_load)
{
	u32 const size = m_cart->common_get_size("rom");

	if (size > m_cart_max_size)
	{
		return std::make_pair(
				image_error::INVALIDLENGTH,
				util::string_format("Invalid file size (must be no more than %u bytes)", m_cart_max_size));
	}

	m_cart->common_load_rom(m_cart_base, size, "rom");
	return std::make_pair(std::error_condition(), std::string());
}

void snspell_state::init_snspell()
{
	m_cart_max_size = 0x4000;
	m_cart_base = memregion("tms6100")->base() + 0x8000;
}

void snspell_state::init_lantrans()
{
	m_cart_max_size = 0x10000;
	m_cart_base = memregion("tms6100")->base();
}



/*******************************************************************************
    I/O
*******************************************************************************/

// common/snspell

void snspell_state::update_display()
{
	u16 gridmask = m_display->row_on(15) ? 0xffff : 0x8000;
	m_display->matrix(m_grid & gridmask, m_plate);
}

void snspell_state::write_r(u32 data)
{
	// R0-R7: input mux and select digit (+R8 if the device has 9 digits)
	// R15: filament on
	// other bits: MCU internal use
	m_grid = data & 0x81ff;
	update_display();

	// R13: power-off request, on falling edge
	if (~data & m_r & 0x2000)
		power_off();

	m_r = data;
}

void snspell_state::write_o(u16 data)
{
	// reorder opla to led14seg, plus DP as d14 and AP as d15:
	// note: lantrans and snread VFD has an accent triangle instead of DP, and no AP
	// E,D,C,G,B,A,I,M,L,K,N,J,[AP],H,F,[DP] (sidenote: TI KLMN = MAME MLNK)
	m_plate = bitswap<16>(data,12,15,10,7,8,9,11,6,13,3,14,0,1,2,4,5);
	update_display();
}

u8 snspell_state::read_k()
{
	u8 data = 0;

	// K: multiplexed inputs
	for (int i = 0; i < 8; i++)
		if (BIT(m_r, i))
			data |= m_inputs[i]->read();

	// Vss row is always on
	return data | m_inputs[8]->read();
}


// snmath specific

void snspell_state::snmath_write_o(u16 data)
{
	// reorder opla to led14seg, plus DP as d14 and CT as d15:
	// [DP],D,C,H,F,B,I,M,L,K,N,J,[CT],E,G,A (sidenote: TI KLMN = MAME MLNK)
	m_plate = bitswap<16>(data,12,0,10,7,8,9,11,6,3,14,4,13,1,2,5,15);
	update_display();
}


// lantrans specific

void snspell_state::lantrans_write_r(u32 data)
{
	// same as default, except R13 is used for an extra digit instead of power-off
	m_r = data;
	m_grid = data & 0xa1ff;
	update_display();
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( snspell )
	PORT_START("IN.0") // R0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')

	PORT_START("IN.1") // R1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('J')

	PORT_START("IN.2") // R2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('N')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('O')

	PORT_START("IN.3") // R3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('T')

	PORT_START("IN.4") // R4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('W')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')

	PORT_START("IN.5") // R5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_NAME("Module Select")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8) PORT_NAME("Erase")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13) PORT_NAME("Enter")

	PORT_START("IN.6") // R6
	PORT_BIT( 0x1f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.7") // R7
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F2) PORT_NAME("Off") // -> auto_power_off
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_HOME) PORT_NAME("Go")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_NAME("Replay")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_NAME("Repeat")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_NAME("Clue")

	PORT_START("IN.8") // Vss
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_NAME("Mystery Word")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_NAME("Secret Code")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_NAME("Letter")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_NAME("Say It")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_F1) PORT_NAME("Spell/On") PORT_CHANGED_MEMBER(DEVICE_SELF, snspell_state, power_on, 0)
INPUT_PORTS_END

static INPUT_PORTS_START( snspellfr ) // French button names
	PORT_INCLUDE( snspell )

	PORT_MODIFY("IN.5")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_NAME("Module 2")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8) PORT_NAME("Efface")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13) PORT_NAME("Essaie")

	PORT_MODIFY("IN.7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F2) PORT_NAME(u8"Arrét") // -> auto_power_off
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_HOME) PORT_NAME(u8"Départ")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_NAME("Rejoue")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_NAME("Repete")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_NAME("Aide")

	PORT_MODIFY("IN.8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_NAME("Mot Mystere")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_NAME("Code Secret")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_NAME("Lettre")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_NAME("Dis-le")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_F1) PORT_NAME("Epelle/Marche") PORT_CHANGED_MEMBER(DEVICE_SELF, snspell_state, power_on, 0)
INPUT_PORTS_END

static INPUT_PORTS_START( snspellit ) // Italian button names
	PORT_INCLUDE( snspell )

	PORT_MODIFY("IN.5")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_NAME("Moduli")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8) PORT_NAME("Cancella")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13) PORT_NAME("Controllo")

	PORT_MODIFY("IN.7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F2) PORT_NAME("Stop") // -> auto_power_off
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_HOME) PORT_NAME("Via")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_NAME("Ritorno")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_NAME("Replica")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_NAME("Aiuto")

	PORT_MODIFY("IN.8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_NAME("Indovina")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_NAME("Codice")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_NAME("Alfabeto")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_NAME("Ripeti")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_F1) PORT_NAME("Scrivi") PORT_CHANGED_MEMBER(DEVICE_SELF, snspell_state, power_on, 0)
INPUT_PORTS_END

static INPUT_PORTS_START( snspellsp ) // Spanish button names, different alphabet
	PORT_START("IN.0") // R0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("CH")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')

	PORT_START("IN.1") // R1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('I')

	PORT_START("IN.2") // R2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_NAME("LL")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('M')

	PORT_START("IN.3") // R3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('N')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(U'Ñ')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('O')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')

	PORT_START("IN.4") // R4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("RR")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('U')

	PORT_START("IN.5") // R5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13) PORT_NAME("Entra")

	PORT_START("IN.6") // R6
	PORT_BIT( 0x1f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.7") // R7
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'') PORT_NAME("Acento")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_HOME) PORT_NAME("Listo")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_NAME("Programa")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8) PORT_NAME("Borra")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_NAME("Repite")

	PORT_START("IN.8") // Vss
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_NAME("Otra Vez")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_NAME("Palabra Secreta")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_NAME("Dilo")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F2) PORT_NAME("Off") // -> auto_power_off
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_F1) PORT_NAME("Deletrea/On") PORT_CHANGED_MEMBER(DEVICE_SELF, snspell_state, power_on, 0)
INPUT_PORTS_END


static INPUT_PORTS_START( snmath )
	PORT_START("IN.0") // R0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_DEL_PAD) PORT_NAME(".")

	PORT_START("IN.1") // R1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // R2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // R3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Enter")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_CODE(KEYCODE_HOME) PORT_NAME("Go")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_F2) PORT_NAME("Off") // -> auto_power_off
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // R4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Clear")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("<")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_STOP) PORT_NAME(">")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("Repeat")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.5") // R5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("+")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("-")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ASTERISK) PORT_NAME(u8"×")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH_PAD) PORT_NAME(u8"÷")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("Mix It")

	PORT_START("IN.6") // R6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME("Number Stumper")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_U) PORT_NAME("Write It")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Y) PORT_NAME("Greater/Less")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("Word Problems")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_CODE(KEYCODE_F1) PORT_NAME("Solve It/On") PORT_CHANGED_MEMBER(DEVICE_SELF, snspell_state, power_on, 0)

	PORT_START("IN.7")
	PORT_BIT( 0x1f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.8")
	PORT_BIT( 0x1f, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( snread )
	PORT_INCLUDE( snspell )

	PORT_MODIFY("IN.7")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_NAME("Word Zapper")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_NAME("Word Maker")

	PORT_MODIFY("IN.8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_NAME("Read It")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_NAME("Picture Read")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_NAME("Letter Stumper")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_NAME("Hear It")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_F1) PORT_NAME("Word Zap/On") PORT_CHANGED_MEMBER(DEVICE_SELF, snspell_state, power_on, 0)
INPUT_PORTS_END


static INPUT_PORTS_START( lantrans )
	PORT_INCLUDE( snspell )

	PORT_MODIFY("IN.5")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("Diacritical")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE) PORT_NAME("Space")

	PORT_MODIFY("IN.6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_NAME("1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_NAME("2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_NAME("3")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_NAME("4")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_NAME("5")

	PORT_MODIFY("IN.7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_NAME("6")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_NAME("7")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_NAME("8")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_NAME("9")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_NAME("0")

	PORT_MODIFY("IN.8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_NAME("Translate")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("Learn")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("Phrase")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("Link")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("Repeat")
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void snspell_state::tms5110_route(machine_config &config)
{
	// sound hardware
	m_tms5100->m0().set(m_tms6100, FUNC(tms6100_device::m0_w));
	m_tms5100->m1().set(m_tms6100, FUNC(tms6100_device::m1_w));
	m_tms5100->addr().set(m_tms6100, FUNC(tms6100_device::add_w));
	m_tms5100->data().set(m_tms6100, FUNC(tms6100_device::data_line_r));
	m_tms5100->romclk().set(m_tms6100, FUNC(tms6100_device::clk_w));
	m_tms5100->add_route(ALL_OUTPUTS, "mono", 0.5);
}

void snspell_state::sns_tmc0281(machine_config &config)
{
	// basic machine hardware
	TMS0270(config, m_maincpu, MASTER_CLOCK/2);
	m_maincpu->read_k().set(FUNC(snspell_state::read_k));
	m_maincpu->write_o().set(FUNC(snspell_state::write_o));
	m_maincpu->write_r().set(FUNC(snspell_state::write_r));

	m_maincpu->read_ctl().set("tms5100", FUNC(tms5110_device::ctl_r));
	m_maincpu->write_ctl().set("tms5100", FUNC(tms5110_device::ctl_w));
	m_maincpu->write_pdc().set("tms5100", FUNC(tms5110_device::pdc_w));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(16, 16);
	m_display->set_segmask(0x21ff, 0x3fff);
	config.set_default_layout(layout_snspell);

	// sound hardware
	TMS6100(config, m_tms6100, MASTER_CLOCK/4);

	SPEAKER(config, "mono").front_center();
	TMC0281(config, m_tms5100, MASTER_CLOCK);
	tms5110_route(config);

	// cartridge
	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "snspell", "vsm,bin");
	m_cart->set_device_load(FUNC(snspell_state::cart_load));

	SOFTWARE_LIST(config, m_softlist).set_original("snspell");
}

void snspell_state::sns_tmc0281d(machine_config &config)
{
	sns_tmc0281(config);

	// sound hardware
	TMC0281D(config.replace(), m_tms5100, MASTER_CLOCK);
	tms5110_route(config);
}

void snspell_state::sns_cd2801(machine_config &config)
{
	sns_tmc0281(config);

	// sound hardware
	CD2801(config.replace(), m_tms5100, MASTER_CLOCK);
	tms5110_route(config);
}

void snspell_state::snspellit(machine_config &config)
{
	sns_cd2801(config);

	config.set_default_layout(layout_snmath);
}

void snspell_state::snspellsp(machine_config &config)
{
	sns_tmc0281(config);

	config.set_default_layout(layout_snspellsp);
}


void snspell_state::snmath(machine_config &config)
{
	sns_cd2801(config);

	// basic machine hardware
	m_maincpu->write_o().set(FUNC(snspell_state::snmath_write_o));

	config.set_default_layout(layout_snmath);

	// no cartridge
	config.device_remove("cartslot");
	config.device_remove("cart_list");
}


void snspell_state::snread(machine_config &config)
{
	sns_cd2801(config);

	config.set_default_layout(layout_snread);

	// cartridge
	m_cart->set_interface("snread");
	m_softlist->set_original("snread");
}


void snspell_state::lantrans(machine_config &config)
{
	sns_cd2801(config);

	// basic machine hardware
	m_maincpu->write_r().set(FUNC(snspell_state::lantrans_write_r));

	config.set_default_layout(layout_snread);

	// cartridge
	m_cart->set_interface("lantrans");
	m_cart->set_must_be_loaded(true);
	m_softlist->set_original("lantrans");
}

void snspell_state::lanteach(machine_config &config)
{
	lantrans(config);

	// cartridge
	m_cart->set_interface("lanteach");
	m_softlist->set_original("lanteach");

	// no sound
	m_tms5100->reset_routes();
	config.device_remove("mono");
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( snspell )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "tmc0271h-n2l_fds", 0x0000, 0x1000, CRC(f83b5d2d) SHA1(10155b0b7f7f1583c7def8a693553cd35944ea6f) )

	ROM_REGION( 1246, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0980_common1_instr.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 2127, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0270_common2_micro.pla", 0, 2127, CRC(86737ac1) SHA1(4aa0444f3ddf88738ea74aec404c684bf54eddba) )
	ROM_REGION( 1246, "maincpu:opla", 0 )
	ROM_LOAD( "tms0270_tmc271h_output.pla", 0, 1246, CRC(2478c595) SHA1(9a8ac690902731e1e01533279a1c9223011e1537) )

	ROM_REGION( 0x10000, "tms6100", ROMREGION_ERASEFF ) // 8000-bfff = space reserved for cartridge
	ROM_LOAD( "tmc0351n2l", 0x0000, 0x4000, CRC(2d03b292) SHA1(a3e9a365307ae936c7472f720a7a8240741531d6) )
	ROM_LOAD( "tmc0352n2l", 0x4000, 0x4000, CRC(a6d56883) SHA1(eebf9c07f2f9001679dec06c2367d4a50596d04b) )
ROM_END

ROM_START( snspellp )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "us4189779", 0x0000, 0x1000, CRC(d3f5a37d) SHA1(f75ab617a6067d4d3a954a9f86126d2089554df8) ) // typed in from patent US4189779, verified by 2 sources

	ROM_REGION( 1246, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0980_common1_instr.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 2127, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0270_common2_micro.pla", 0, 2127, CRC(86737ac1) SHA1(4aa0444f3ddf88738ea74aec404c684bf54eddba) )
	ROM_REGION( 1246, "maincpu:opla", 0 )
	ROM_LOAD( "tms0270_tmc271_output.pla", 0, 1246, CRC(9ebe12ab) SHA1(acb4e07ba26f2daca5f1c234885ac0371c7ce87f) ) // using the one from 1st version

	ROM_REGION( 0x10000, "tms6100", ROMREGION_ERASEFF ) // 8000-bfff = space reserved for cartridge
	ROM_LOAD( "tmc0351nl", 0x0000, 0x4000, CRC(beea3373) SHA1(8b0f7586d2f12c3d4a885fdb528cf23feffa1a3b) ) // using the one from 1st version
	ROM_LOAD( "tmc0352nl", 0x4000, 0x4000, CRC(d51f0587) SHA1(ddaa484be1bba5fef46b481cafae517e4acaa8ed) ) // "
ROM_END

ROM_START( snspellua )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "tmc0271nl_dbs", 0x0000, 0x1000, CRC(c2a7b747) SHA1(05f3716cd3cc33b8dc897e75301efdd531932ec5) )

	ROM_REGION( 1246, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0980_common1_instr.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 2127, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0270_common2_micro.pla", 0, 2127, CRC(86737ac1) SHA1(4aa0444f3ddf88738ea74aec404c684bf54eddba) )
	ROM_REGION( 1246, "maincpu:opla", 0 )
	ROM_LOAD( "tms0270_tmc271_output.pla", 0, 1246, CRC(9ebe12ab) SHA1(acb4e07ba26f2daca5f1c234885ac0371c7ce87f) )

	ROM_REGION( 0x10000, "tms6100", ROMREGION_ERASEFF ) // 8000-bfff = space reserved for cartridge
	ROM_LOAD( "tmc0351nl", 0x0000, 0x4000, CRC(beea3373) SHA1(8b0f7586d2f12c3d4a885fdb528cf23feffa1a3b) )
	ROM_LOAD( "tmc0352nl", 0x4000, 0x4000, CRC(d51f0587) SHA1(ddaa484be1bba5fef46b481cafae517e4acaa8ed) )
ROM_END

ROM_START( snspellub )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "tmc0271h-n2l_fds", 0x0000, 0x1000, CRC(f83b5d2d) SHA1(10155b0b7f7f1583c7def8a693553cd35944ea6f) )

	ROM_REGION( 1246, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0980_common1_instr.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 2127, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0270_common2_micro.pla", 0, 2127, CRC(86737ac1) SHA1(4aa0444f3ddf88738ea74aec404c684bf54eddba) )
	ROM_REGION( 1246, "maincpu:opla", 0 )
	ROM_LOAD( "tms0270_tmc271h_output.pla", 0, 1246, CRC(2478c595) SHA1(9a8ac690902731e1e01533279a1c9223011e1537) )

	ROM_REGION( 0x10000, "tms6100", ROMREGION_ERASEFF ) // uses only 1 rom, 8000-bfff = space reserved for cartridge
	ROM_LOAD( "cd2350a", 0x0000, 0x4000, CRC(2adda742) SHA1(3f868ed8284b723c815a30343057e03467c043b5) )
ROM_END

ROM_START( snspelluk )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "tmc0271nl_dbs", 0x0000, 0x1000, CRC(c2a7b747) SHA1(05f3716cd3cc33b8dc897e75301efdd531932ec5) )

	ROM_REGION( 1246, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0980_common1_instr.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 2127, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0270_common2_micro.pla", 0, 2127, CRC(86737ac1) SHA1(4aa0444f3ddf88738ea74aec404c684bf54eddba) )
	ROM_REGION( 1246, "maincpu:opla", 0 )
	ROM_LOAD( "tms0270_tmc271_output.pla", 0, 1246, CRC(9ebe12ab) SHA1(acb4e07ba26f2daca5f1c234885ac0371c7ce87f) )

	ROM_REGION( 0x10000, "tms6100", ROMREGION_ERASEFF ) // 8000-bfff = space reserved for cartridge
	ROM_LOAD( "cd2303", 0x0000, 0x4000, CRC(0fae755c) SHA1(b68c3120a63a61db474feb5d71a6e5dd67910d80) )
	ROM_LOAD( "cd2304", 0x4000, 0x4000, CRC(e2a270eb) SHA1(c13c95ad15f1923a4841f66504e0f22646e71d99) )
ROM_END

ROM_START( snspelluka )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "tmc0271h-n2l_fds", 0x0000, 0x1000, CRC(f83b5d2d) SHA1(10155b0b7f7f1583c7def8a693553cd35944ea6f) )

	ROM_REGION( 1246, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0980_common1_instr.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 2127, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0270_common2_micro.pla", 0, 2127, CRC(86737ac1) SHA1(4aa0444f3ddf88738ea74aec404c684bf54eddba) )
	ROM_REGION( 1246, "maincpu:opla", 0 )
	ROM_LOAD( "tms0270_tmc271h_output.pla", 0, 1246, CRC(2478c595) SHA1(9a8ac690902731e1e01533279a1c9223011e1537) )

	ROM_REGION( 0x10000, "tms6100", ROMREGION_ERASEFF ) // uses only 1 rom, 8000-bfff = space reserved for cartridge
	ROM_LOAD( "cd62175", 0x0000, 0x4000, CRC(6e1063d4) SHA1(b5c66c51148c5921ecb8ffccd7a460ae639cdb68) )
ROM_END

ROM_START( snspelljp )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "tmc0271h-n2l_fds", 0x0000, 0x1000, CRC(f83b5d2d) SHA1(10155b0b7f7f1583c7def8a693553cd35944ea6f) )

	ROM_REGION( 1246, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0980_common1_instr.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 2127, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0270_common2_micro.pla", 0, 2127, CRC(86737ac1) SHA1(4aa0444f3ddf88738ea74aec404c684bf54eddba) )
	ROM_REGION( 1246, "maincpu:opla", 0 )
	ROM_LOAD( "tms0270_tmc271h_output.pla", 0, 1246, CRC(2478c595) SHA1(9a8ac690902731e1e01533279a1c9223011e1537) )

	ROM_REGION( 0x10000, "tms6100", ROMREGION_ERASEFF ) // 8000-bfff = space reserved for cartridge
	ROM_LOAD( "cd2321", 0x0000, 0x4000, CRC(ac010cce) SHA1(c0200d857b62be696248ac2d684a390c66ab0c31) )
	ROM_LOAD( "cd2322", 0x4000, 0x4000, CRC(b6f4bba4) SHA1(65d686a9385b5ef3f080a5f47c6b2418bb9455b0) )
ROM_END

ROM_START( snspellsp )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "cd2701n2l_p", 0x0000, 0x1000, CRC(5230612a) SHA1(3ec75b207380af2efe84e865fe83ea29e79e8eea) )

	ROM_REGION( 1246, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0980_common1_instr.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 2127, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0270_common2_micro.pla", 0, 2127, CRC(86737ac1) SHA1(4aa0444f3ddf88738ea74aec404c684bf54eddba) )
	ROM_REGION( 1246, "maincpu:opla", 0 )
	ROM_LOAD( "tms0270_cd2701_output.pla", 0, 1246, CRC(f26980bd) SHA1(8d0c98fe5240541cb53c1e1d14c2a4560e7a7f32) )

	ROM_REGION( 0x10000, "tms6100", ROMREGION_ERASEFF ) // 8000-bfff = space reserved for cartridge
	ROM_LOAD( "cd2319", 0x0000, 0x4000, CRC(f293ac2f) SHA1(6f941743efcc2f05e514ce07167c094c554dca5d) )
	ROM_LOAD( "cd2320", 0x4000, 0x4000, CRC(16b68766) SHA1(a9ea335b4487cc333268bfd2e71428258968461d) )
ROM_END

ROM_START( snspellfr )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "cd2702an2l", 0x0000, 0x1000, CRC(895d6a4e) SHA1(a8bc118c83a84260033734191dcaa71a93dfa52b) )

	ROM_REGION( 1246, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0980_common1_instr.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 2127, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0270_common1_micro.pla", 0, 2127, CRC(504b96bb) SHA1(67b691e7c0b97239410587e50e5182bf46475b43) )
	ROM_REGION( 1246, "maincpu:opla", 0 )
	ROM_LOAD( "tms0270_cd2702_output.pla", 0, 1246, CRC(2478c595) SHA1(9a8ac690902731e1e01533279a1c9223011e1537) )

	ROM_REGION( 0x10000, "tms6100", ROMREGION_ERASEFF ) // uses only 1 rom, 8000-bfff = space reserved for cartridge
	ROM_LOAD( "cd2352", 0x0000, 0x4000, CRC(181a239e) SHA1(e16043766c385e152b7005c1c010be4c5fccdd9b) )
ROM_END

ROM_START( snspellit )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "cd2702an2l", 0x0000, 0x1000, CRC(895d6a4e) SHA1(a8bc118c83a84260033734191dcaa71a93dfa52b) )

	ROM_REGION( 1246, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0980_common1_instr.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 2127, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0270_common1_micro.pla", 0, 2127, CRC(504b96bb) SHA1(67b691e7c0b97239410587e50e5182bf46475b43) )
	ROM_REGION( 1246, "maincpu:opla", 0 )
	ROM_LOAD( "tms0270_cd2702_output.pla", 0, 1246, CRC(2478c595) SHA1(9a8ac690902731e1e01533279a1c9223011e1537) )

	ROM_REGION( 0x10000, "tms6100", ROMREGION_ERASEFF ) // uses only 1 rom, 8000-bfff = space reserved for cartridge
	ROM_LOAD( "cd62190", 0x0000, 0x4000, CRC(63832002) SHA1(ea8124b2bf0f5908c5f1a56d60063f2468a10143) )
ROM_END


ROM_START( snmath )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "cd2704b-n2l", 0x0000, 0x1000, CRC(7e06c7c5) SHA1(d60a35a8163ab593c31afc840a0d8a9b3a762f29) )

	ROM_REGION( 1246, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0980_common1_instr.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 2127, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0270_common1_micro.pla", 0, 2127, CRC(504b96bb) SHA1(67b691e7c0b97239410587e50e5182bf46475b43) )
	ROM_REGION( 1246, "maincpu:opla", 0 )
	ROM_LOAD( "tms0270_cd2704_output.pla", 0, 1246, CRC(5a2eb949) SHA1(8bb161d4884f229af65f8d155e59b9d8966fe3d1) )

	ROM_REGION( 0x8000, "tms6100", ROMREGION_ERASEFF )
	ROM_LOAD( "cd2392", 0x0000, 0x4000, CRC(4ed2e920) SHA1(8896f29e25126c1e4d9a47c9a325b35dddecc61f) )
	ROM_LOAD( "cd2393", 0x4000, 0x4000, CRC(571d5b5a) SHA1(83284755d9b77267d320b5b87fdc39f352433715) )
ROM_END

ROM_START( snmatha )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "cd2708-n2l", 0x0000, 0x1000, CRC(35937360) SHA1(69c362c75bb459056c09c7fab37c91040485474b) )

	ROM_REGION( 1246, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0980_common1_instr.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 2127, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0270_common1_micro.pla", 0, 2127, CRC(504b96bb) SHA1(67b691e7c0b97239410587e50e5182bf46475b43) )
	ROM_REGION( 1246, "maincpu:opla", 0 )
	ROM_LOAD( "tms0270_cd2708_output.pla", 0, 1246, CRC(1abad753) SHA1(53d20b519ed73ce248368047a056836afbe3cd46) )

	ROM_REGION( 0x8000, "tms6100", ROMREGION_ERASEFF )
	ROM_LOAD( "cd2381", 0x0000, 0x4000, CRC(f048dc81) SHA1(e97667d1002de40ab3d702c63b82311480032e0f) )
	ROM_LOAD( "cd2614", 0x4000, 0x1000, CRC(11989074) SHA1(0e9cf906de9bcdf4acb425535dc442846fc48fa2) )
	ROM_RELOAD(             0x5000, 0x1000 )
	ROM_RELOAD(             0x6000, 0x1000 )
	ROM_RELOAD(             0x7000, 0x1000 )
ROM_END

ROM_START( snmathp )
	ROM_REGION( 0x1000, "maincpu", 0 )
	// typed in from patent 4946391, verified with source code
	// BTANB note: Mix It does not work at all, this is an original bug in the patent listing. There are probably other minor bugs too.
	ROM_LOAD( "us4946391_t2074", 0x0000, 0x1000, CRC(011f0c2d) SHA1(d2e14d72e03ca864abd51da78ffb71a9da82f624) )

	ROM_REGION( 1246, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0980_common1_instr.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 2127, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0270_common1_micro.pla", 0, 2127, CRC(504b96bb) SHA1(67b691e7c0b97239410587e50e5182bf46475b43) )
	ROM_REGION( 1246, "maincpu:opla", 0 )
	ROM_LOAD( "tms0270_cd2704_output.pla", 0, 1246, CRC(5a2eb949) SHA1(8bb161d4884f229af65f8d155e59b9d8966fe3d1) ) // using the one from 1st version

	ROM_REGION( 0x8000, "tms6100", ROMREGION_ERASEFF )
	ROM_LOAD( "cd2392", 0x0000, 0x4000, CRC(4ed2e920) SHA1(8896f29e25126c1e4d9a47c9a325b35dddecc61f) ) // using the one from 1st version
	ROM_LOAD( "cd2393", 0x4000, 0x4000, CRC(571d5b5a) SHA1(83284755d9b77267d320b5b87fdc39f352433715) ) // "
ROM_END


ROM_START( snread )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "cd2705b-n2l", 0x0000, 0x1000, CRC(c235636e) SHA1(57b24dd8414bf76ec786a51d10cb8a5898b60e18) )

	ROM_REGION( 1246, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0980_common1_instr.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 2127, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0270_common1_micro.pla", 0, 2127, CRC(504b96bb) SHA1(67b691e7c0b97239410587e50e5182bf46475b43) )
	ROM_REGION( 1246, "maincpu:opla", 0 )
	ROM_LOAD( "tms0270_cd2705_output.pla", 0, 1246, CRC(bf859848) SHA1(66b297fbf534968fa6db7413b99ef0e81cc35ddc) )

	ROM_REGION( 0x10000, "tms6100", ROMREGION_ERASEFF ) // 8000-bfff = space reserved for cartridge
	ROM_LOAD( "cd2394a", 0x0000, 0x4000, CRC(cbb0e2b1) SHA1(5e322c683baf806523de171310258ae371671327) )
	ROM_LOAD( "cd2395a", 0x4000, 0x4000, CRC(3d519504) SHA1(76b19ba5a9a3486005e09c98e8a6abc8b88288dd) )
ROM_END


ROM_START( lantrans )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "tmc0275nl_dbs", 0x0000, 0x1000, CRC(43535d2a) SHA1(cf190fc0b1b2d9d11b167896adda116bc28348fd) )

	ROM_REGION( 1246, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0980_common1_instr.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 2127, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0270_common2_micro.pla", 0, 2127, CRC(86737ac1) SHA1(4aa0444f3ddf88738ea74aec404c684bf54eddba) )
	ROM_REGION( 1246, "maincpu:opla", 0 )
	ROM_LOAD( "tms0270_tmc0275_output.pla", 0, 1246, CRC(3c67e7b8) SHA1(06134367754a687e933ed629a105b6956fc30375) )

	ROM_REGION( 0x10000, "tms6100", ROMREGION_ERASEFF ) // cartridge area
ROM_END

ROM_START( lantransp )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "us4631748", 0x0000, 0x1000, CRC(22818845) SHA1(1a84f15fb18ca66b1f2bf7491d76fbc56068984d) ) // extracted visually from patent US4631748, verified with source code

	ROM_REGION( 1246, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0980_common1_instr.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 2127, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0270_common2_micro.pla", 0, 2127, CRC(86737ac1) SHA1(4aa0444f3ddf88738ea74aec404c684bf54eddba) )
	ROM_REGION( 1246, "maincpu:opla", 0 )
	ROM_LOAD( "tms0270_tmc0275_output.pla", 0, 1246, CRC(3c67e7b8) SHA1(06134367754a687e933ed629a105b6956fc30375) ) // using the one from release version

	ROM_REGION( 0x10000, "tms6100", ROMREGION_ERASEFF ) // cartridge area
ROM_END

ROM_START( lanteach )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "cd2706", 0x0000, 0x1000, CRC(4eca02d7) SHA1(1b13b7679b868c6c978d3047b01a372abfd7f01a) )

	ROM_REGION( 1246, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0980_common1_instr.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 2127, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0270_common2_micro.pla", 0, 2127, CRC(86737ac1) SHA1(4aa0444f3ddf88738ea74aec404c684bf54eddba) )
	ROM_REGION( 1246, "maincpu:opla", 0 )
	ROM_LOAD( "tms0270_cd2706_output.pla", 0, 1246, CRC(c290b20f) SHA1(377205266b2b8932d06abd98a6071ea75eec834e) )

	ROM_REGION( 0x10000, "tms6100", ROMREGION_ERASEFF ) // cartridge area
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME        PARENT    COMPAT  MACHINE       INPUT       CLASS          INIT           COMPANY, FULLNAME, FLAGS
SYST( 1980, snspell,    0,        0,      sns_tmc0281,  snspell,    snspell_state, init_snspell,  "Texas Instruments", "Speak & Spell (US, 1980 version)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
SYST( 1978, snspellua,  snspell,  0,      sns_tmc0281,  snspell,    snspell_state, init_snspell,  "Texas Instruments", "Speak & Spell (US, 1978 version)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
SYST( 1981, snspellub,  snspell,  0,      sns_tmc0281d, snspell,    snspell_state, init_snspell,  "Texas Instruments", "Speak & Spell (US, 1981 version)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND ) // less speech data
SYST( 1978, snspellp,   snspell,  0,      sns_tmc0281,  snspell,    snspell_state, init_snspell,  "Texas Instruments", "Speak & Spell (US, patent)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
SYST( 1978, snspelluk,  snspell,  0,      sns_tmc0281,  snspell,    snspell_state, init_snspell,  "Texas Instruments", "Speak & Spell (UK, 1978 version)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
SYST( 1981, snspelluka, snspell,  0,      sns_cd2801,   snspell,    snspell_state, init_snspell,  "Texas Instruments", "Speak & Spell (UK, 1981 version)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND ) // less speech data
SYST( 1980, snspelljp,  snspell,  0,      sns_tmc0281,  snspell,    snspell_state, init_snspell,  "Texas Instruments", "Speak & Spell (Japan)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND ) // speaks English
SYST( 1980, snspellsp,  snspell,  0,      snspellsp,    snspellsp,  snspell_state, init_snspell,  "Texas Instruments", "Speak & Spell (Spanish, prototype)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
SYST( 1981, snspellfr,  snspell,  0,      sns_cd2801,   snspellfr,  snspell_state, init_snspell,  "Texas Instruments", u8"La Dictée Magique (France)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
SYST( 1983, snspellit,  snspell,  0,      snspellit,    snspellit,  snspell_state, init_snspell,  "Texas Instruments / Clementoni", "Grillo Parlante (Italy)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )

SYST( 1980, snmath,     0,        0,      snmath,       snmath,     snspell_state, empty_init,    "Texas Instruments", "Speak & Math (US, 1980 version)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
SYST( 1984, snmatha,    snmath,   0,      snmath,       snmath,     snspell_state, empty_init,    "Texas Instruments", "Speak & Math (US, 1984 version)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND ) // less speech data
SYST( 1980, snmathp,    snmath,   0,      snmath,       snmath,     snspell_state, empty_init,    "Texas Instruments", "Speak & Math (US, patent)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND | MACHINE_IS_INCOMPLETE )

SYST( 1980, snread,     0,        0,      snread,       snread,     snspell_state, init_snspell,  "Texas Instruments", "Speak & Read (US)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )

SYST( 1979, lantrans,   0,        0,      lantrans,     lantrans,   snspell_state, init_lantrans, "Texas Instruments", "Language Translator", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
SYST( 1979, lantransp,  lantrans, 0,      lantrans,     lantrans,   snspell_state, init_lantrans, "Texas Instruments", "Language Translator (patent)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )

SYST( 1980, lanteach,   0,        0,      lanteach,     lantrans,   snspell_state, init_lantrans, "Texas Instruments", "Language Teacher", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
