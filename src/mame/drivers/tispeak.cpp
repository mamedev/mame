// license:BSD-3-Clause
// copyright-holders:hap, Jonathan Gevaryahu
// thanks-to:Sean Riddle, David Viens, Kevin Horton
/***************************************************************************

  ** subclass of hh_tms1k_state (includes/hh_tms1k.h, drivers/hh_tms1k.cpp) **

  Texas Instruments 1st-gen. handheld speech devices.

  These devices, mostly edu-toys, are based around an MCU(TMS0270/TMS1100),
  TMS51xx speech, and VSM ROM(s). Newer devices, such as Speak & Music,
  are based around the TMP50C40 and belong in another driver, probably.

  note: except for tntell, MAME external artwork is not required. But it
  is objectively a large improvement.

----------------------------------------------------------------------------

Known devices on this hardware: (* denotes not dumped)


ROM (and/or source) code obtained from patents:
Some of these may have pre-release bugs.

    Speak & Spell: US4189779
    Speak & Math: US4946391
    Touch & Tell: US4403965, EP0048835A2 (patent calls it "Speak & Seek")
    Language Translator: US4631748


Speak & Spell:

This is the original Speak & Spell. TI had done educational toys before, like
Wiz-A-Tron or Little Professor. But the popularity of this product was much
above expectations. TI continued to manufacture many products for this line.

    Speak & Spell (US), 1978
    - MCU: TMC0271, label TMC0271NL DBS (die label 0271B T0270B)
    - TMS51xx: TMC0281
    - VSM(1/2): 16KB TMC0351NL
    - VSM(2/2): 16KB TMC0352NL
    - VFD: NEC FIP8A5AR no. 3A
    - notes: keyboard has buttons instead of cheap membrane

    Speak & Spell (US), 1979
    - MCU: TMC0271, label TMC0271H-N2L FDS (die label 0271H T0270D)
    - TMS51xx: TMC0281
    - VSM(1/2): 16KB TMC0351N2L
    - VSM(2/2): 16KB TMC0352N2L
    - notes: fixed a funny bug with gibberish-talk when Module button is pressed
      with no module inserted, MCU ROM contents differs from 1978 version

    Speak & Spell (US), 1980
    - MCU: same as 1979 version
    - TMS51xx: TMC0281D
    - VSM: 16KB CD2350(rev.A)
    - notes: only 1 VSM, meaning much smaller internal vocabulary

    Speak & Spell (Japan), 1980
    - MCU: TMC0271 (assume same as US 1979 version)
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
    - MCU: TMC0271 (assume same as US 1979 version)
    - TMS51xx: CD2801
    - VSM: 16KB CD62175
    - VFD: some seen with the one from Speak & Math(!)
    - notes: this one has a dedicated voice actor

    Speak & Spell (Spanish, prototype), 1981
    - MCU: CD2701N2L P (die label T0270D 2701)
    - TMS51xx: TMC0281
    - VSM(1/2): 16KB CD2319
    - VSM(2/2): 16KB CD2320
    - VFD: 8 digits with 14 segments, DP and accent mark

    Speak & Spell (France) "La Dictée Magique", 1980
    - MCU: CD2702, label CD2702AN2L (die label TMC0270F 2702A)
    - TMS51xx: CD2801
    - VSM: 16KB CD2352

    Speak & Spell (Germany) "Buddy", 1980
    - MCU & TMS51xx: same as French 1980 version
    - VSM(1/2): 16KB CD2345*
    - VSM(2/2): 16KB CD2346*

    Speak & Spell (Italy) "Grillo Parlante", 1982
    - MCU & TMS51xx: same as French 1980 version
    - VSM: 16KB CD62190
    - VFD: same as Speak & Math
    - notes: it appears that TI ran out of original snspell VFDs in the early 80s?

    Speak & Spell Compact (US), 1981
    - MCU: CD8011, label CD8011A-NL (die label 1100B)
    - TMS51xx: TMC0281D
    - VSM: 16KB CD2354, CD2354(rev.A)
    - notes: no display, MCU is TMS1100 instead of TMS0270, overall similar to Touch & Tell

    Speak & Spell Compact (UK) "Speak & Write", 1981
    - MCU: same as US 1981 version
    - TMS51xx: CD2801A
    - VSM: 16KB CD62174(rev.A)
    - notes: BTANB: gibberish when the module button is pressed (with module present),
      anecdotes from the developer, the same person working on the original UK version:
          "We included a pencil and writing pad - it was now about 'writing'.",
      and one about the welcome message:
          "I had to manually create a sentence of digital speech from thin air.
           I had to write down a 20 character code which would create each 10/s
           sound bite that made up the phrase "Welcome to Speak and Write".
           It took me 1 week. (...) Even Larry Brantingham was amazed."

Speak & Spell modules:
Note that they are interchangeable, eg. you can use a French module on a US Speak & Spell.

    English:
    - Vowel Power: VSM: 16KB CD2302
    - Number Stumpers 4-6: VSM: 16KB CD2305
    - Number Stumpers 7-8: VSM: 16KB CD2307(rev.A)
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
    - No.3: Les Animaux Familiers: VSM: 16KB? CD62047*
    - No.4: Les Magasins de la Rue: VSM: 16KB CD62048
    - No.5: Les Extra-Terrestres: VSM: 16KB? CD62178*

    Italian:
    - Super Modulo: VSM: 16KB? CD62313*


Speak & Math:

    Speak & Math (US), 1980 (renamed to "Speak & Maths" in UK, but is the same product)
    - MCU: CD2704, label CD2704B-N2L (die label TMC0270F 2704B) - 2nd revision?(mid-1982)
    - TMS51xx: CD2801
    - VSM(1/2): 16KB CD2392
    - VSM(2/2): 16KB CD2393
    - VFD: Futaba 9SY -02Z 7E
    - notes: As with the Speak & Spell, the voice actor was a radio announcer.
      However, the phrase "is greater than or less than" had to be added by one
      of the TI employees in a hurry, the day before a demo. Apparently QA
      never found out and it ended up in the final product.

    Speak & Math (US), 1986
    - MCU: CD2708, label CD2708N2L (die label TMC0270F 2708A)
    - TMS51xx: CD2801
    - VSM(1/2): 16KB CD2381
    - VSM(2/2): 4KB CD2614

    Speak & Math 'Compact' (France) "Les Maths Magiques", 1986?
    - MCU: CP3447-NL* (TMS1100?)
    - TMS51xx: CD2801
    - VSM: 16KB? CD62173*
    - notes: this is not the same as "Le Calcul Magique", that's from a
      series centered around a TMS50C40 instead of MCU+TMS51xx


Speak & Read:

    Speak & Read (US), 1980
    - MCU: CD2705, label CD2705B-N2L (die label TMC0270E 2705B) - 2nd revision?(late-1981)
    - TMS51xx: CD2801
    - VSM(1/2): 16KB CD2394(rev.A)
    - VSM(2/2): 16KB CD2395(rev.A)
    - VFD: same as Language Tutor, rightmost digit unused

Speak & Read modules:

    English:
    - Sea Sights: VSM: 16KB CD2396(rev.A)
    - Who's Who at the Zoo: VSM: 16KB CD2397
    - A Dog on a Log: VSM: 16KB CD3534(rev.A)
    - The Seal That Could Fly: VSM: 16KB CD3535
    - A Ghost in the House: VSM: 16KB CD3536
    - On the Track: VSM: 16KB CD3538
    - The Third Circle: VSM: 16KB CD3539
    - The Millionth Knight: VSM: 16KB CD3540


Touch & Tell:

    Touch & Tell (US), 1981
    - MCU: CD8012, label CD8012NL (die label 1100G CD8012)
    - TMS51xx: CD2802
    - VSM: 4KB CD2610
    - notes: MCU is TMS1100 instead of TMS0270. CD8010 is seen in some devices
      too, maybe an earlier version?

    Touch & Tell (UK), 1981
    - MCU & TMS51xx: same as US version
    - VSM: 16KB CD62170

    Touch & Tell (France) "Le Livre Magique", 1981
    - MCU & TMS51xx: same as US version
    - VSM: 16KB CD62171

    Touch & Tell (Germany) "Tipp & Sprich", 1981
    - MCU & TMS51xx: same as US version
    - VSM: ?KB CD62172*

    Touch & Tell (Italy) "Libro Parlante", 1982
    - MCU & TMS51xx: same as US version
    - VSM: ?KB CD62176*

    Vocaid (US), 1982
    - MCU & TMS51xx: same as Touch & Tell (US)
    - VSM: 16KB CD2357
    - notes: MCU is the same as in Touch & Tell, but instead of a toddler's toy,
      you get a serious medical aid device for the voice-impaired. The PCB is
      identical, it includes the edge connector for modules but no external slot.

Touch & Tell modules:

    English:
    - Alphabet Fun: VSM: 4KB CD2611
    - Animal Friends: VSM: 16KB CD2355
    - Number Fun: VSM: 4KB CD2612*, CD2612(rev.A)
    - All About Me: VSM: 4KB CD2613
    - World of Transportation: VSM: 16KB CD2361
    - Little Creatures: VSM: 16KB CD2362
    - E.T.: VSM: 16KB CD2363

Touch & Tell/Vocaid overlay reference:

    tntell CD2610:
    - $04: a - Colors
    - $01: b - Objects
    - $05: c - Shapes
    - $09: d - Home Scene
    tntelluk CD62170, tntellfr CD62171:
    - see tntell
    - see numfun(not A)
    - see animalfr
    - $08: ? - Clown Face
    - $0B: ? - Body Parts
    vocaid CD2357:
    - $1C: 1 - Leisure
    - $1E: 2 - Telephone
    - $1B: 3 - Bedside
    - $1D: 4 - Alphabet
    alphabet CD2611:
    - $0E: 1a - Alphabet A-M
    - $0D: 1b - Alphabet N-Z
    - $0C: 1c - Letter Jumble A-M
    - $0B: 1d - Letter Jumble N-Z
    animalfr CD2355:
    - $0A: 2a - Farm Animals
    - $0F: 2b - At The Farm
    - $0E: 2c - Animal Babies
    - $0D: 2d - In The Jungle
    numfun CD2612:
    - $02/$0A(rev.A): 3a - Numbers 1-10
    - $03/$0F(rev.A): 3b - Numbers 11-30
    - $07/$0D(rev.A): 3c - How Many?
    - $06/$0E(rev.A): 3d - Hidden Numbers
    aboutme CD2613:
    - $0E: 4a - Clown Face
    - $0B: 4b - Body Parts
    - $0D: 4c - Things to Wear
    - $0C: 4d - Just For Me
    wot CD2361:
    - $0A: 5a - On Land
    - $0B: 5b - In The Air
    - $0C: 5c - On The Water
    - $0D: 5d - In Space
    - $10: 5e - What Belongs Here?
    - $11: 5f - How It Used To Be
    - $12: 5g - Word Fun
    - $13: 5h - In the Surprise Garage
    lilcreat CD2362:
    - $14: 6a - In The Park
    - $15: 6b - In The Sea
    - $16: 6c - In The Woods
    - $17: 6d - Whose House?
    - $18: 6e - Hide & Seek
    - $1A: 6f - Who Is It?
    - $19: 6g - But It's Not
    - $1B: 6h - Word Fun
    et CD2363:
    - $0F: 7a - The Adventure On Earth I
    - $10: 7b - The Adventure On Earth II
    - $11: 7c - Fun And Friendship I
    - $12: 7d - Fun And Friendship II
    - $13: 7e - E.T. The Star I
    - $14: 7f - E.T. The Star II
    - $15: 7g - Do You Remember? I
    - $16: 7h - Do You Remember? II

    $00: none inserted, and $1F is for diagnostics


Magic Wand "Speaking Reader" or "Speak & Learn":

Limited release barcode reader text-to-speech device, meant for children.
The text is coded as phonemes. Books were sold separately.

    Magic Wand "Speaking Reader" (US), 1982
    - MCU: C14007* (TMS1400?)
    - TMS5220
    - VSM: 4KB? CD2228*

    Magic Wand "Speak & Learn" (US), 1983
    - notes: same hardware


Language Tutor/Translator:

A later device, called Language Teacher, was released without speech hardware.

    Language Tutor (US), 1978
    - MCU: TMC0275*
    - TMS51xx: CD2801
    - VFD: NEC FIP10xxx?
    - notes: external module is required (see below)

Language Tutor modules:

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


Other manufacturers:

Tiger Electronics K28 (model 7-232) Sold in Hong Kong, distributed in US as:
- Coleco: Talking Teacher
- Sears: Talkatron - Learning Computer

1981 K28 models 7-230 and 7-231 are on different hardware, showing a different
keyboard, VFD display, and use the SC-01 speech chip. --> driver k28.cpp

    K28 model 7-232 (HK), 1985
    - MCU: TMS1400 MP7324
    - TMS51xx: TMS5110A
    - VSM: 16KB CM62084
    - LCD: SMOS SMC1112 MCU to 8*14-seg display

K28 modules:

    - Spelling I: VSM: 16KB CM62086
    - Spelling II: VSM: 16KB CM62085?
    - Spelling III: VSM: 16KB CM62087
    - Expansion Module 1: VSM: 16KB CM62214? - assumed same VSM as CM62086
    - Expansion Module 2: VSM: 16KB CM62216 - assumed same VSM as the one in Spelling II
    - Expansion Module 3: VSM: 16KB CM62215 - same VSM as CM62087
    - Expansion Module 4: VSM: 16KB CM62217
    - Expansion Module 5: VSM: 16KB CM62218*
    - Expansion Module 6: VSM: 16KB CM62219

    note: these won't work on the 1981 version(s)

----------------------------------------------------------------------------

  TODO:
  - why doesn't lantutor work?
  - emulate k28 LCD
  - emulate other known devices


Dick Smith catalog numbers, taken from advertisements:

X-1300, Y-1300 : T.I. Speak & Spell
X-1301 : Super Stumper 1 (for Speak & Spell)
X-1302 : Super Stumper 2 (for Speak & Spell)
X-1305 : Vowell Power (for Speak & Spell)
Y-1310 : T.I. Speak & Math
Y-1313 : T.I. Speak & Read
Y-1320 : T.I. Dataman

***************************************************************************/

#include "emu.h"
#include "includes/hh_tms1k.h"

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "machine/timer.h"
#include "machine/tms6100.h"
#include "sound/tms5110.h"
#include "softlist_dev.h"
#include "speaker.h"
#include "render.h"

// internal artwork
#include "k28m2.lh"
#include "snmath.lh"
#include "snread.lh"
#include "snspell.lh"
#include "snspellsp.lh"
#include "tntell.lh" // keyboard overlay

namespace {

// The master clock is a single stage RC oscillator into TMS5100 RCOSC:
// In an early 1979 Speak & Spell, C is 68pf, R is a 50kohm trimpot which is set to around 33.6kohm
// (measured in-circuit). CPUCLK is this osc freq /2, ROMCLK is this osc freq /4.
// The typical osc freq curve for TMS5100 is unknown. Let's assume it is set to the default frequency,
// which is 640kHz for 8KHz according to the TMS5100 documentation.

#define MASTER_CLOCK 640000


class tispeak_state : public hh_tms1k_state
{
public:
	tispeak_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag),
		m_tms5100(*this, "tms5100"),
		m_tms6100(*this, "tms6100"),
		m_cart(*this, "cartslot"),
		m_ol_out(*this, "ol%u", 1U)
	{ }

	void init_snspell();
	void init_tntell();
	void init_lantutor();

	// machine configs
	void tms5110_route(machine_config &config);
	void sns_tmc0281(machine_config &config);
	void sns_tmc0281d(machine_config &config);
	void sns_cd2801(machine_config &config);
	void snspellit(machine_config &config);
	void snspellsp(machine_config &config);
	void snspellc(machine_config &config);
	void snspellcuk(machine_config &config);
	void snmath(machine_config &config);
	void snread(machine_config &config);
	void tntell(machine_config &config);
	void vocaid(machine_config &config);
	void lantutor(machine_config &config);
	void k28m2(machine_config &config);

private:
	virtual void power_off() override;
	void update_display();

	u8 snspell_read_k();
	void snmath_write_o(u16 data);
	void snspell_write_o(u16 data);
	void snspell_write_r(u16 data);
	void lantutor_write_r(u16 data);

	u8 snspellc_read_k();
	void snspellc_write_o(u16 data);
	void snspellc_write_r(u16 data);
	u8 tntell_read_k();

	void k28_update_display(u8 old, u8 data);
	u8 k28_read_k();
	void k28_write_o(u16 data);
	void k28_write_r(u16 data);

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);

	u8 tntell_get_hexchar(const char c);
	TIMER_DEVICE_CALLBACK_MEMBER(tntell_get_overlay);

	void init_cartridge();

	virtual void machine_start() override;

	// devices
	required_device<tms5110_device> m_tms5100;
	required_device<tms6100_device> m_tms6100;
	optional_device<generic_slot_device> m_cart;

	output_finder<5> m_ol_out;

	// cartridge
	u32 m_cart_max_size;
	u8 *m_cart_base;

	u8 m_overlay;
};

void tispeak_state::machine_start()
{
	hh_tms1k_state::machine_start();

	m_ol_out.resolve();
	init_cartridge();
}



/***************************************************************************

  Cartridge Handling

***************************************************************************/

void tispeak_state::init_cartridge()
{
	m_overlay = 0;

	if (m_cart != nullptr && m_cart->exists())
	{
		std::string region_tag;
		memory_region *src = memregion(region_tag.assign(m_cart->tag()).append(GENERIC_ROM_REGION_TAG).c_str());
		if (src)
			memcpy(m_cart_base, src->base(), src->bytes());
	}
}

DEVICE_IMAGE_LOAD_MEMBER(tispeak_state::cart_load)
{
	u32 size = m_cart->common_get_size("rom");

	if (size > m_cart_max_size)
	{
		image.seterror(image_error::INVALIDIMAGE, "Invalid file size");
		return image_init_result::FAIL;
	}

	m_cart->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return image_init_result::PASS;
}


void tispeak_state::init_snspell()
{
	m_cart_max_size = 0x4000;
	m_cart_base = memregion("tms6100")->base() + 0x8000;
}

void tispeak_state::init_tntell()
{
	m_cart_max_size = 0x4000;
	m_cart_base = memregion("tms6100")->base() + 0x4000;
}

void tispeak_state::init_lantutor()
{
	m_cart_max_size = 0x10000;
	m_cart_base = memregion("tms6100")->base();
}



/***************************************************************************

  I/O

***************************************************************************/

// common/snspell

void tispeak_state::power_off()
{
	hh_tms1k_state::power_off();
	m_tms5100->reset();
}

void tispeak_state::update_display()
{
	u16 gridmask = m_display->row_on(15) ? 0xffff : 0x8000;
	m_display->matrix(m_grid & gridmask, m_plate);
}

void tispeak_state::snspell_write_r(u16 data)
{
	// R13: power-off request, on falling edge
	if (~data & m_r & 0x2000)
		power_off();

	// R0-R7: input mux and select digit (+R8 if the device has 9 digits)
	// R15: filament on
	// other bits: MCU internal use
	m_r = m_inp_mux = data;
	m_grid = data & 0x81ff;
	update_display();
}

void tispeak_state::snspell_write_o(u16 data)
{
	// reorder opla to led14seg, plus DP as d14 and AP as d15:
	// note: lantutor and snread VFD has an accent triangle instead of DP, and no AP
	// E,D,C,G,B,A,I,M,L,K,N,J,[AP],H,F,[DP] (sidenote: TI KLMN = MAME MLNK)
	m_plate = bitswap<16>(data,12,15,10,7,8,9,11,6,13,3,14,0,1,2,4,5);
	update_display();
}

u8 tispeak_state::snspell_read_k()
{
	// K: multiplexed inputs (note: the Vss row is always on)
	return m_inputs[8]->read() | read_inputs(8);
}


// snmath specific

void tispeak_state::snmath_write_o(u16 data)
{
	// reorder opla to led14seg, plus DP as d14 and CT as d15:
	// [DP],D,C,H,F,B,I,M,L,K,N,J,[CT],E,G,A (sidenote: TI KLMN = MAME MLNK)
	m_plate = bitswap<16>(data,12,0,10,7,8,9,11,6,3,14,4,13,1,2,5,15);
	update_display();
}


// lantutor specific

void tispeak_state::lantutor_write_r(u16 data)
{
	// same as default, except R13 is used for an extra digit
	m_r = m_inp_mux = data;
	m_grid = data & 0xa1ff;
	update_display();
}


// snspellc specific

void tispeak_state::snspellc_write_r(u16 data)
{
	// R10: TMS5100 PDC pin
	m_tms5100->pdc_w(data >> 10 & 1);

	// R9: power-off request, on falling edge
	if (~data & m_r & 0x200)
		power_off();

	// R0-R8: input mux
	m_r = m_inp_mux = data;
}

void tispeak_state::snspellc_write_o(u16 data)
{
	// O3210: TMS5100 CTL8124
	m_tms5100->ctl_w(bitswap<4>(data,3,0,1,2));
	m_o = data;
}

u8 tispeak_state::snspellc_read_k()
{
	// K4: TMS5100 CTL1
	u8 k4 = m_tms5100->ctl_r() << 2 & 4;

	// K: multiplexed inputs (note: the Vss row is always on)
	return k4 | m_inputs[9]->read() | read_inputs(9);
}


// tntell specific

u8 tispeak_state::tntell_read_k()
{
	// K8: overlay code from R5,O4-O7
	u8 k8 = (((m_r >> 1 & 0x10) | (m_o >> 4 & 0xf)) & m_overlay) ? 8 : 0;

	// rest is same as snpellc
	return k8 | snspellc_read_k();
}

u8 tispeak_state::tntell_get_hexchar(const char c)
{
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'A' && c <= 'F') return c - 'A' + 10;
	if (c >= 'a' && c <= 'f') return c - 'a' + 10;

	return 0;
}

TIMER_DEVICE_CALLBACK_MEMBER(tispeak_state::tntell_get_overlay)
{
	// Each keyboard overlay insert has 5 holes, used by the game to determine
	// which one is active(if any). If it matches with the internal ROM or
	// external module, the game continues.

	// pick overlay code from machine config, see comment section above for reference
	m_overlay = m_inputs[10]->read();

	// try to get it from (external) layout
	if (m_overlay == 0x20)
	{
		// as output value, eg. with defstate (in decimal)
		m_overlay = output().get_value("overlay_code") & 0x1f;

		// and from current view name ($ + 2 hex digits)
		render_target *target = machine().render().first_target();
		const char *name = target->view_name(target->view());

		for (int i = 0; name && i < strlen(name); i++)
			if (name[i] == '$' && strlen(&name[i]) > 2)
				m_overlay = (tntell_get_hexchar(name[i + 1]) << 4 | tntell_get_hexchar(name[i + 2])) & 0x1f;
	}

	// overlay holes
	for (int i = 0; i < 5; i++)
		m_ol_out[i] = BIT(m_overlay, i);
}


// k28 specific

void tispeak_state::k28_update_display(u8 old, u8 data)
{
	// ?
}

void tispeak_state::k28_write_r(u16 data)
{
	// R1234: TMS5100 CTL8421
	u16 r = bitswap<5>(data,0,1,2,3,4) | (data & ~0x1f);
	m_tms5100->ctl_w(r & 0xf);

	// R0: TMS5100 PDC pin
	m_tms5100->pdc_w(data & 1);

	// R5: input mux high bit
	m_inp_mux = (m_inp_mux & 0xff) | (data << 3 & 0x100);

	// R6: power-off request, on falling edge
	if (~data & m_r & 0x40)
		power_off();

	// R7-R10: LCD data
	k28_update_display(m_r >> 7 & 0xf, data >> 7 & 0xf);
	m_r = r;
}

void tispeak_state::k28_write_o(u16 data)
{
	// O0-O7: input mux low
	m_inp_mux = (m_inp_mux & ~0xff) | data;
}

u8 tispeak_state::k28_read_k()
{
	// K: TMS5100 CTL, multiplexed inputs (also tied to R1234)
	return m_tms5100->ctl_r() | read_inputs(9) | (m_r & 0xf);
}



/***************************************************************************

  Inputs

***************************************************************************/

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
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Erase")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Enter")

	PORT_START("IN.6") // R6
	PORT_BIT( 0x1f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.7") // R7
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F2) PORT_NAME("Off") // -> auto_power_off
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_HOME) PORT_NAME("Go")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_NAME("Replay")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_NAME("Repeat")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_NAME("Clue")

	PORT_START("IN.8") // Vss!
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_NAME("Mystery Word")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_NAME("Secret Code")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_NAME("Letter")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_NAME("Say It")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_F1) PORT_NAME("Spell/On") PORT_CHANGED_MEMBER(DEVICE_SELF, tispeak_state, power_button, true)
INPUT_PORTS_END

static INPUT_PORTS_START( snspellfr ) // French button names
	PORT_INCLUDE( snspell )

	PORT_MODIFY("IN.5")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_NAME("Module 2")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Efface")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Essaie")

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
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_F1) PORT_NAME("Epelle/Marche") PORT_CHANGED_MEMBER(DEVICE_SELF, tispeak_state, power_button, true)
INPUT_PORTS_END

static INPUT_PORTS_START( snspellit ) // Italian button names
	PORT_INCLUDE( snspell )

	PORT_MODIFY("IN.5")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_NAME("Moduli")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Cancella")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Controllo")

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
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_F1) PORT_NAME("Scrivi") PORT_CHANGED_MEMBER(DEVICE_SELF, tispeak_state, power_button, true)
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
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_NAME(u8"Ñ")
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
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Entra")

	PORT_START("IN.6") // R6
	PORT_BIT( 0x1f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.7") // R7
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'') PORT_NAME("Acento")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_HOME) PORT_NAME("Listo")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_NAME("Programa")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Borra")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_NAME("Repite")

	PORT_START("IN.8") // Vss!
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_NAME("Otra Vez")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_NAME("Palabra Secreta")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_NAME("Dilo")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F2) PORT_NAME("Off") // -> auto_power_off
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_F1) PORT_NAME("Deletrea/On") PORT_CHANGED_MEMBER(DEVICE_SELF, tispeak_state, power_button, true)
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
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_CODE(KEYCODE_F1) PORT_NAME("Solve It/On") PORT_CHANGED_MEMBER(DEVICE_SELF, tispeak_state, power_button, true)

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
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_F1) PORT_NAME("Word Zap/On") PORT_CHANGED_MEMBER(DEVICE_SELF, tispeak_state, power_button, true)
INPUT_PORTS_END


static INPUT_PORTS_START( lantutor )
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


static INPUT_PORTS_START( snspellc )
	PORT_START("IN.0") // R0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_NAME("Letter Stumper")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('S')

	PORT_START("IN.1") // R1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('T')

	PORT_START("IN.2") // R2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_NAME("Review")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('U')

	PORT_START("IN.3") // R3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_NAME("Repeat")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('V')

	PORT_START("IN.4") // R4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Erase")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('N')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('W')

	PORT_START("IN.5") // R5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_NAME("Module")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('O')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('X')

	PORT_START("IN.6") // R6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_HOME) PORT_NAME("Go")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')

	PORT_START("IN.7") // R7
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Enter")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')

	PORT_START("IN.8") // R8
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'')

	PORT_START("IN.9") // Vss!
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_F1) PORT_NAME("Spell/On") PORT_CHANGED_MEMBER(DEVICE_SELF, tispeak_state, power_button, true)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) // speech chip data
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F2) PORT_NAME("Off") // -> auto_power_off
INPUT_PORTS_END

static INPUT_PORTS_START( snspellcuk )
	PORT_INCLUDE( snspellc )

	PORT_MODIFY("IN.9")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_F1) PORT_NAME("Write/On") PORT_CHANGED_MEMBER(DEVICE_SELF, tispeak_state, power_button, true) // just the label changed from Spell to Write
INPUT_PORTS_END


static INPUT_PORTS_START( tntell )
	PORT_START("IN.0") // R0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_NAME("Grid 1-1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_NAME("Grid 1-2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_NAME("Grid 1-4")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_NAME("Grid 1-3")

	PORT_START("IN.1") // R1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("Grid 2-1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("Grid 2-2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("Grid 2-4")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("Grid 2-3")

	PORT_START("IN.2") // R2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("Grid 3-1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("Grid 3-2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("Grid 3-4")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("Grid 3-3")

	PORT_START("IN.3") // R3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("Grid 4-1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Grid 4-2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_V) PORT_NAME("Grid 4-4")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("Grid 4-3")

	PORT_START("IN.4") // R4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_NAME("Grid 5-1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_NAME("Grid 5-2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_U) PORT_NAME("Grid 5-4")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_NAME("Grid 5-3")

	PORT_START("IN.5") // R5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_O) PORT_NAME("Grid 5-6")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("Grid 6-5")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME("Grid 5-5")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) // overlay code

	PORT_START("IN.6") // R6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("Grid 3-5")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("Grid 2-5")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("Grid 4-5")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_NAME("Grid 1-5")

	PORT_START("IN.7") // R7
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_NAME("Grid 3-6")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Y) PORT_NAME("Grid 2-6")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_NAME("Grid 4-6")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_NAME("Grid 1-6")

	PORT_START("IN.8") // R8
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_CODE(KEYCODE_F2) PORT_NAME("Grid 6-1 (Off)") // -> auto_power_off
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_K) PORT_NAME("Grid 6-2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("Grid 6-4")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_L) PORT_NAME("Grid 6-3")

	PORT_START("IN.9") // Vss!
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_STOP) PORT_CODE(KEYCODE_F1) PORT_NAME("Grid 6-6 (On)") PORT_CHANGED_MEMBER(DEVICE_SELF, tispeak_state, power_button, true)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) // speech chip data
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.10")
	PORT_CONFNAME( 0x3f, 0x20, "Overlay Code" )
	PORT_CONFSETTING(    0x20, "From Artwork View" )
	PORT_CONFSETTING(    0x00, "$00 (None)" )
	PORT_CONFSETTING(    0x01, "$01" )
	PORT_CONFSETTING(    0x02, "$02" )
	PORT_CONFSETTING(    0x03, "$03" )
	PORT_CONFSETTING(    0x04, "$04" )
	PORT_CONFSETTING(    0x05, "$05" )
	PORT_CONFSETTING(    0x06, "$06" )
	PORT_CONFSETTING(    0x07, "$07" )
	PORT_CONFSETTING(    0x08, "$08" )
	PORT_CONFSETTING(    0x09, "$09" )
	PORT_CONFSETTING(    0x0a, "$0A" )
	PORT_CONFSETTING(    0x0b, "$0B" )
	PORT_CONFSETTING(    0x0c, "$0C" )
	PORT_CONFSETTING(    0x0d, "$0D" )
	PORT_CONFSETTING(    0x0e, "$0E" )
	PORT_CONFSETTING(    0x0f, "$0F" )
	PORT_CONFSETTING(    0x10, "$10" )
	PORT_CONFSETTING(    0x11, "$11" )
	PORT_CONFSETTING(    0x12, "$12" )
	PORT_CONFSETTING(    0x13, "$13" )
	PORT_CONFSETTING(    0x14, "$14" )
	PORT_CONFSETTING(    0x15, "$15" )
	PORT_CONFSETTING(    0x16, "$16" )
	PORT_CONFSETTING(    0x17, "$17" )
	PORT_CONFSETTING(    0x18, "$18" )
	PORT_CONFSETTING(    0x19, "$19" )
	PORT_CONFSETTING(    0x1a, "$1A" )
	PORT_CONFSETTING(    0x1b, "$1B" )
	PORT_CONFSETTING(    0x1c, "$1C" )
	PORT_CONFSETTING(    0x1d, "$1D" )
	PORT_CONFSETTING(    0x1e, "$1E" )
	PORT_CONFSETTING(    0x1f, "$1F (Diagnostic)" )
INPUT_PORTS_END


static INPUT_PORTS_START( k28m2 )
	PORT_START("IN.0") // O0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F2) PORT_NAME("Off") // -> auto_power_off
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_CHAR('A') PORT_NAME("A/1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_CHAR('J') PORT_NAME("J/0")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('S')

	PORT_START("IN.1") // O1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F1) PORT_NAME("On") PORT_CHANGED_MEMBER(DEVICE_SELF, tispeak_state, power_button, true)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_CHAR('B') PORT_NAME("B/2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR('K') PORT_NAME("K/+")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('T')

	PORT_START("IN.2") // O2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("Repeat")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_CHAR('C') PORT_NAME("C/3")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR('L') PORT_NAME("L/-")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('U')

	PORT_START("IN.3") // O3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("Prompt")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_CHAR('D') PORT_NAME("D/4")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CODE(KEYCODE_ASTERISK) PORT_CHAR('M') PORT_NAME(u8"M/×")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('V')

	PORT_START("IN.4") // O4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_HOME) PORT_NAME("Menu")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_CHAR('E') PORT_NAME("E/5")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CODE(KEYCODE_SLASH_PAD) PORT_CHAR('N') PORT_NAME(u8"N/÷")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('W')

	PORT_START("IN.5") // O5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_END) PORT_NAME("Module")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_CHAR('F') PORT_NAME("F/6")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('O')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('X')

	PORT_START("IN.6") // O6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE) PORT_NAME("Select")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_CHAR('G') PORT_NAME("G/7")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')

	PORT_START("IN.7") // O7
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Clear")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_CHAR('H') PORT_NAME("H/8")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')

	PORT_START("IN.8") // R5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_CHAR('I') PORT_NAME("I/9")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Enter")
INPUT_PORTS_END



/***************************************************************************

  Machine Config

***************************************************************************/

void tispeak_state::tms5110_route(machine_config &config)
{
	// sound hardware
	m_tms5100->m0().set(m_tms6100, FUNC(tms6100_device::m0_w));
	m_tms5100->m1().set(m_tms6100, FUNC(tms6100_device::m1_w));
	m_tms5100->addr().set(m_tms6100, FUNC(tms6100_device::add_w));
	m_tms5100->data().set(m_tms6100, FUNC(tms6100_device::data_line_r));
	m_tms5100->romclk().set(m_tms6100, FUNC(tms6100_device::clk_w));
	m_tms5100->add_route(ALL_OUTPUTS, "mono", 0.5);
}

void tispeak_state::snmath(machine_config &config)
{
	// basic machine hardware
	TMS0270(config, m_maincpu, MASTER_CLOCK/2);
	m_maincpu->k().set(FUNC(tispeak_state::snspell_read_k));
	m_maincpu->o().set(FUNC(tispeak_state::snmath_write_o));
	m_maincpu->r().set(FUNC(tispeak_state::snspell_write_r));

	m_maincpu->read_ctl().set("tms5100", FUNC(tms5110_device::ctl_r));
	m_maincpu->write_ctl().set("tms5100", FUNC(tms5110_device::ctl_w));
	m_maincpu->write_pdc().set("tms5100", FUNC(tms5110_device::pdc_w));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(16, 16);
	m_display->set_segmask(0x21ff, 0x3fff);
	config.set_default_layout(layout_snmath);

	// sound hardware
	TMS6100(config, m_tms6100, MASTER_CLOCK/4);

	SPEAKER(config, "mono").front_center();
	CD2801(config, m_tms5100, MASTER_CLOCK);
	tms5110_route(config);
}


void tispeak_state::sns_cd2801(machine_config &config)
{
	snmath(config);

	// basic machine hardware
	m_maincpu->o().set(FUNC(tispeak_state::snspell_write_o));

	config.set_default_layout(layout_snspell);

	// cartridge
	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "snspell", "vsm");
	m_cart->set_device_load(FUNC(tispeak_state::cart_load));

	SOFTWARE_LIST(config, "cart_list").set_original("snspell");
}

void tispeak_state::snspellit(machine_config &config)
{
	sns_cd2801(config);

	// basic machine hardware
	config.set_default_layout(layout_snmath);
}

void tispeak_state::sns_tmc0281(machine_config &config)
{
	sns_cd2801(config);

	// sound hardware
	TMC0281(config.replace(), m_tms5100, MASTER_CLOCK);
	tms5110_route(config);
}

void tispeak_state::snspellsp(machine_config &config)
{
	sns_tmc0281(config);

	// basic machine hardware
	config.set_default_layout(layout_snspellsp);
}

void tispeak_state::sns_tmc0281d(machine_config &config)
{
	sns_cd2801(config);

	// sound hardware
	TMC0281D(config.replace(), m_tms5100, MASTER_CLOCK);
	tms5110_route(config);
}


void tispeak_state::snread(machine_config &config)
{
	snmath(config);

	// basic machine hardware
	m_maincpu->o().set(FUNC(tispeak_state::snspell_write_o));

	config.set_default_layout(layout_snread);

	// cartridge
	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "snread", "vsm");
	m_cart->set_device_load(FUNC(tispeak_state::cart_load));

	SOFTWARE_LIST(config, "cart_list").set_original("snread");
}


void tispeak_state::lantutor(machine_config &config)
{
	snmath(config);

	// basic machine hardware
	m_maincpu->o().set(FUNC(tispeak_state::snspell_write_o));
	m_maincpu->r().set(FUNC(tispeak_state::lantutor_write_r));

	config.set_default_layout(layout_snread);

	// cartridge
	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "lantutor", "vsm,bin");
	m_cart->set_must_be_loaded(true);
	m_cart->set_device_load(FUNC(tispeak_state::cart_load));

	SOFTWARE_LIST(config, "cart_list").set_original("lantutor");
}


void tispeak_state::snspellc(machine_config &config)
{
	// basic machine hardware
	TMS1100(config, m_maincpu, MASTER_CLOCK/2);
	m_maincpu->k().set(FUNC(tispeak_state::snspellc_read_k));
	m_maincpu->o().set(FUNC(tispeak_state::snspellc_write_o));
	m_maincpu->r().set(FUNC(tispeak_state::snspellc_write_r));

	// no visual feedback!

	// sound hardware
	TMS6100(config, m_tms6100, MASTER_CLOCK/4);

	SPEAKER(config, "mono").front_center();
	TMC0281D(config, m_tms5100, MASTER_CLOCK);
	tms5110_route(config);

	// cartridge
	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "snspell", "vsm");
	m_cart->set_device_load(FUNC(tispeak_state::cart_load));

	SOFTWARE_LIST(config, "cart_list").set_original("snspell");
}

void tispeak_state::snspellcuk(machine_config &config)
{
	snspellc(config);

	// sound hardware
	CD2801(config.replace(), m_tms5100, MASTER_CLOCK); // CD2801A!
	tms5110_route(config);
}


void tispeak_state::vocaid(machine_config &config)
{
	// basic machine hardware
	TMS1100(config, m_maincpu, MASTER_CLOCK/2);
	m_maincpu->k().set(FUNC(tispeak_state::tntell_read_k));
	m_maincpu->o().set(FUNC(tispeak_state::snspellc_write_o));
	m_maincpu->r().set(FUNC(tispeak_state::snspellc_write_r));

	TIMER(config, "ol_timer").configure_periodic(FUNC(tispeak_state::tntell_get_overlay), attotime::from_msec(50));
	config.set_default_layout(layout_tntell);

	// sound hardware
	TMS6100(config, m_tms6100, MASTER_CLOCK/4);

	SPEAKER(config, "mono").front_center();
	CD2802(config, m_tms5100, MASTER_CLOCK);
	tms5110_route(config);
}

void tispeak_state::tntell(machine_config &config)
{
	vocaid(config);

	// cartridge
	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "tntell", "vsm");
	m_cart->set_device_load(FUNC(tispeak_state::cart_load));

	SOFTWARE_LIST(config, "cart_list").set_original("tntell");
}


void tispeak_state::k28m2(machine_config &config)
{
	// basic machine hardware
	TMS1400(config, m_maincpu, MASTER_CLOCK/2);
	m_maincpu->k().set(FUNC(tispeak_state::k28_read_k));
	m_maincpu->o().set(FUNC(tispeak_state::k28_write_o));
	m_maincpu->r().set(FUNC(tispeak_state::k28_write_r));

	config.set_default_layout(layout_k28m2);

	// sound hardware
	TMS6100(config, m_tms6100, MASTER_CLOCK/4);

	SPEAKER(config, "mono").front_center();
	TMS5110A(config, m_tms5100, MASTER_CLOCK);
	tms5110_route(config);

	// cartridge
	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "k28m2", "vsm");
	m_cart->set_device_load(FUNC(tispeak_state::cart_load));

	SOFTWARE_LIST(config, "cart_list").set_original("k28m2");
}



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( snspell )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "tmc0271h-n2l_fds", 0x0000, 0x1000, CRC(f83b5d2d) SHA1(10155b0b7f7f1583c7def8a693553cd35944ea6f) )

	ROM_REGION( 1246, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0980_common1_instr.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 2127, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0270_common2_micro.pla", 0, 2127, CRC(86737ac1) SHA1(4aa0444f3ddf88738ea74aec404c684bf54eddba) )
	ROM_REGION( 1246, "maincpu:opla", 0 )
	ROM_LOAD( "tms0270_t0270d_output.pla", 0, 1246, CRC(2478c595) SHA1(9a8ac690902731e1e01533279a1c9223011e1537) )

	ROM_REGION( 0x10000, "tms6100", ROMREGION_ERASEFF ) // 8000-bfff = space reserved for cartridge
	ROM_LOAD( "tmc0351n2l.vsm", 0x0000, 0x4000, CRC(2d03b292) SHA1(a3e9a365307ae936c7472f720a7a8240741531d6) )
	ROM_LOAD( "tmc0352n2l.vsm", 0x4000, 0x4000, CRC(a6d56883) SHA1(eebf9c07f2f9001679dec06c2367d4a50596d04b) )
ROM_END

ROM_START( snspellp )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "us4189779_tmc0271", 0x0000, 0x1000, CRC(d3f5a37d) SHA1(f75ab617a6067d4d3a954a9f86126d2089554df8) ) // typed in from patent US4189779, verified by 2 sources

	ROM_REGION( 1246, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0980_common1_instr.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 2127, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0270_common2_micro.pla", 0, 2127, CRC(86737ac1) SHA1(4aa0444f3ddf88738ea74aec404c684bf54eddba) )
	ROM_REGION( 1246, "maincpu:opla", 0 )
	ROM_LOAD( "tms0270_t0270b_output.pla", 0, 1246, CRC(9ebe12ab) SHA1(acb4e07ba26f2daca5f1c234885ac0371c7ce87f) ) // using the one from 1st version

	ROM_REGION( 0x10000, "tms6100", ROMREGION_ERASEFF ) // 8000-bfff = space reserved for cartridge
	ROM_LOAD( "tmc0351nl.vsm", 0x0000, 0x4000, CRC(beea3373) SHA1(8b0f7586d2f12c3d4a885fdb528cf23feffa1a3b) ) // using the one from 1st version
	ROM_LOAD( "tmc0352nl.vsm", 0x4000, 0x4000, CRC(d51f0587) SHA1(ddaa484be1bba5fef46b481cafae517e4acaa8ed) ) // "
ROM_END

ROM_START( snspellua )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "tmc0271nl_dbs", 0x0000, 0x1000, CRC(c2a7b747) SHA1(05f3716cd3cc33b8dc897e75301efdd531932ec5) )

	ROM_REGION( 1246, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0980_common1_instr.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 2127, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0270_common2_micro.pla", 0, 2127, CRC(86737ac1) SHA1(4aa0444f3ddf88738ea74aec404c684bf54eddba) )
	ROM_REGION( 1246, "maincpu:opla", 0 )
	ROM_LOAD( "tms0270_t0270b_output.pla", 0, 1246, CRC(9ebe12ab) SHA1(acb4e07ba26f2daca5f1c234885ac0371c7ce87f) )

	ROM_REGION( 0x10000, "tms6100", ROMREGION_ERASEFF ) // 8000-bfff = space reserved for cartridge
	ROM_LOAD( "tmc0351nl.vsm", 0x0000, 0x4000, CRC(beea3373) SHA1(8b0f7586d2f12c3d4a885fdb528cf23feffa1a3b) )
	ROM_LOAD( "tmc0352nl.vsm", 0x4000, 0x4000, CRC(d51f0587) SHA1(ddaa484be1bba5fef46b481cafae517e4acaa8ed) )
ROM_END

ROM_START( snspellub )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "tmc0271h-n2l_fds", 0x0000, 0x1000, CRC(f83b5d2d) SHA1(10155b0b7f7f1583c7def8a693553cd35944ea6f) )

	ROM_REGION( 1246, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0980_common1_instr.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 2127, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0270_common2_micro.pla", 0, 2127, CRC(86737ac1) SHA1(4aa0444f3ddf88738ea74aec404c684bf54eddba) )
	ROM_REGION( 1246, "maincpu:opla", 0 )
	ROM_LOAD( "tms0270_t0270d_output.pla", 0, 1246, CRC(2478c595) SHA1(9a8ac690902731e1e01533279a1c9223011e1537) )

	ROM_REGION( 0x10000, "tms6100", ROMREGION_ERASEFF ) // uses only 1 rom, 8000-bfff = space reserved for cartridge
	ROM_LOAD( "cd2350a.vsm", 0x0000, 0x4000, CRC(2adda742) SHA1(3f868ed8284b723c815a30343057e03467c043b5) )
ROM_END

ROM_START( snspelluk )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "tmc0271nl_dbs", 0x0000, 0x1000, CRC(c2a7b747) SHA1(05f3716cd3cc33b8dc897e75301efdd531932ec5) )

	ROM_REGION( 1246, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0980_common1_instr.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 2127, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0270_common2_micro.pla", 0, 2127, CRC(86737ac1) SHA1(4aa0444f3ddf88738ea74aec404c684bf54eddba) )
	ROM_REGION( 1246, "maincpu:opla", 0 )
	ROM_LOAD( "tms0270_t0270b_output.pla", 0, 1246, CRC(9ebe12ab) SHA1(acb4e07ba26f2daca5f1c234885ac0371c7ce87f) )

	ROM_REGION( 0x10000, "tms6100", ROMREGION_ERASEFF ) // 8000-bfff = space reserved for cartridge
	ROM_LOAD( "cd2303.vsm", 0x0000, 0x4000, CRC(0fae755c) SHA1(b68c3120a63a61db474feb5d71a6e5dd67910d80) )
	ROM_LOAD( "cd2304.vsm", 0x4000, 0x4000, CRC(e2a270eb) SHA1(c13c95ad15f1923a4841f66504e0f22646e71d99) )
ROM_END

ROM_START( snspelluka )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "tmc0271h-n2l_fds", 0x0000, 0x1000, CRC(f83b5d2d) SHA1(10155b0b7f7f1583c7def8a693553cd35944ea6f) )

	ROM_REGION( 1246, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0980_common1_instr.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 2127, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0270_common2_micro.pla", 0, 2127, CRC(86737ac1) SHA1(4aa0444f3ddf88738ea74aec404c684bf54eddba) )
	ROM_REGION( 1246, "maincpu:opla", 0 )
	ROM_LOAD( "tms0270_t0270d_output.pla", 0, 1246, CRC(2478c595) SHA1(9a8ac690902731e1e01533279a1c9223011e1537) )

	ROM_REGION( 0x10000, "tms6100", ROMREGION_ERASEFF ) // uses only 1 rom, 8000-bfff = space reserved for cartridge
	ROM_LOAD( "cd62175.vsm", 0x0000, 0x4000, CRC(6e1063d4) SHA1(b5c66c51148c5921ecb8ffccd7a460ae639cdb68) )
ROM_END

ROM_START( snspelljp )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "tmc0271h-n2l_fds", 0x0000, 0x1000, CRC(f83b5d2d) SHA1(10155b0b7f7f1583c7def8a693553cd35944ea6f) )

	ROM_REGION( 1246, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0980_common1_instr.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 2127, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0270_common2_micro.pla", 0, 2127, CRC(86737ac1) SHA1(4aa0444f3ddf88738ea74aec404c684bf54eddba) )
	ROM_REGION( 1246, "maincpu:opla", 0 )
	ROM_LOAD( "tms0270_t0270d_output.pla", 0, 1246, CRC(2478c595) SHA1(9a8ac690902731e1e01533279a1c9223011e1537) )

	ROM_REGION( 0x10000, "tms6100", ROMREGION_ERASEFF ) // 8000-bfff = space reserved for cartridge
	ROM_LOAD( "cd2321.vsm", 0x0000, 0x4000, CRC(ac010cce) SHA1(c0200d857b62be696248ac2d684a390c66ab0c31) )
	ROM_LOAD( "cd2322.vsm", 0x4000, 0x4000, CRC(b6f4bba4) SHA1(65d686a9385b5ef3f080a5f47c6b2418bb9455b0) )
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
	ROM_LOAD( "cd2319.vsm", 0x0000, 0x4000, CRC(f293ac2f) SHA1(6f941743efcc2f05e514ce07167c094c554dca5d) )
	ROM_LOAD( "cd2320.vsm", 0x4000, 0x4000, CRC(16b68766) SHA1(a9ea335b4487cc333268bfd2e71428258968461d) )
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
	ROM_LOAD( "cd2352.vsm", 0x0000, 0x4000, CRC(181a239e) SHA1(e16043766c385e152b7005c1c010be4c5fccdd9b) )
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
	ROM_LOAD( "cd62190.vsm", 0x0000, 0x4000, CRC(63832002) SHA1(ea8124b2bf0f5908c5f1a56d60063f2468a10143) )
ROM_END


ROM_START( snspellc )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "cd8011a-nl", 0x0000, 0x0800, CRC(8a82a467) SHA1(fa4f8a232392603721bd8136c141a340fd5936a0) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_cd8011_micro.pla", 0, 867, CRC(bbb64ddc) SHA1(602cd5eef897c9115c12db7367c5654ab2297fa1) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_cd8011_output.pla", 0, 365, CRC(b400dd75) SHA1(5a4b5d4532a8932cf4b469ddb71ad6b3b9911672) )

	ROM_REGION( 0x10000, "tms6100", ROMREGION_ERASEFF ) // 8000-bfff = space reserved for cartridge
	ROM_LOAD( "cd2354.vsm", 0x0000, 0x4000, CRC(3af3232e) SHA1(f89d90dca209ee612634d664d5d4562f1d1786cf) )
ROM_END

ROM_START( snspellca )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "cd8011a-nl", 0x0000, 0x0800, CRC(8a82a467) SHA1(fa4f8a232392603721bd8136c141a340fd5936a0) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_cd8011_micro.pla", 0, 867, CRC(bbb64ddc) SHA1(602cd5eef897c9115c12db7367c5654ab2297fa1) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_cd8011_output.pla", 0, 365, CRC(b400dd75) SHA1(5a4b5d4532a8932cf4b469ddb71ad6b3b9911672) )

	ROM_REGION( 0x10000, "tms6100", ROMREGION_ERASEFF ) // 8000-bfff = space reserved for cartridge
	ROM_LOAD( "cd2354a.vsm", 0x0000, 0x4000, CRC(548a940c) SHA1(c37e620c4c70a05cbaaff9a166c6da2e2420196f) )
ROM_END

ROM_START( snspellcuk )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "cd8011a-nl", 0x0000, 0x0800, CRC(8a82a467) SHA1(fa4f8a232392603721bd8136c141a340fd5936a0) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_cd8011_micro.pla", 0, 867, CRC(bbb64ddc) SHA1(602cd5eef897c9115c12db7367c5654ab2297fa1) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_cd8011_output.pla", 0, 365, CRC(b400dd75) SHA1(5a4b5d4532a8932cf4b469ddb71ad6b3b9911672) )

	ROM_REGION( 0x10000, "tms6100", ROMREGION_ERASEFF ) // 8000-bfff = space reserved for cartridge
	ROM_LOAD( "cd62174a.vsm", 0x0000, 0x4000, CRC(b7bbaaf3) SHA1(9eb949fcf522982f9c3c4649f207703b746b90ef) )
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
	ROM_LOAD( "cd2392.vsm", 0x0000, 0x4000, CRC(4ed2e920) SHA1(8896f29e25126c1e4d9a47c9a325b35dddecc61f) )
	ROM_LOAD( "cd2393.vsm", 0x4000, 0x4000, CRC(571d5b5a) SHA1(83284755d9b77267d320b5b87fdc39f352433715) )
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
	ROM_LOAD( "cd2381.vsm", 0x0000, 0x4000, CRC(f048dc81) SHA1(e97667d1002de40ab3d702c63b82311480032e0f) )
	ROM_LOAD( "cd2614.vsm", 0x4000, 0x1000, CRC(11989074) SHA1(0e9cf906de9bcdf4acb425535dc442846fc48fa2) )
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
	ROM_LOAD( "cd2392.vsm", 0x0000, 0x4000, CRC(4ed2e920) SHA1(8896f29e25126c1e4d9a47c9a325b35dddecc61f) ) // using the one from 1st version
	ROM_LOAD( "cd2393.vsm", 0x4000, 0x4000, CRC(571d5b5a) SHA1(83284755d9b77267d320b5b87fdc39f352433715) ) // "
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
	ROM_LOAD( "cd2394a.vsm", 0x0000, 0x4000, CRC(cbb0e2b1) SHA1(5e322c683baf806523de171310258ae371671327) )
	ROM_LOAD( "cd2395a.vsm", 0x4000, 0x4000, CRC(3d519504) SHA1(76b19ba5a9a3486005e09c98e8a6abc8b88288dd) )
ROM_END


ROM_START( lantutor )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "us4631748_tmc0275", 0x0000, 0x1000, CRC(22818845) SHA1(1a84f15fb18ca66b1f2bf7491d76fbc56068984d) ) // extracted visually from patent US4631748, verified with source code

	ROM_REGION( 1246, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0980_common1_instr.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 2127, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0270_common1_micro.pla", 0, 2127, BAD_DUMP CRC(504b96bb) SHA1(67b691e7c0b97239410587e50e5182bf46475b43) ) // not verified
	ROM_REGION( 1246, "maincpu:opla", 0 )
	ROM_LOAD( "tms0270_t0270b_output.pla", 0, 1246, BAD_DUMP CRC(9ebe12ab) SHA1(acb4e07ba26f2daca5f1c234885ac0371c7ce87f) ) // taken from snspell, mostly looks correct

	ROM_REGION( 0x10000, "tms6100", ROMREGION_ERASEFF ) // cartridge area
ROM_END


ROM_START( tntell )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "cd8012nl", 0x0000, 0x0800, CRC(3d0fee24) SHA1(8b1b1df03d50ffe8adea59ece212dece5245fe86) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_cd8012_micro.pla", 0, 867, CRC(46d936c8) SHA1(b0aad486a90a5dec7fd2fb07caa503be771f91c8) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_cd8012_output.pla", 0, 365, CRC(5ada9306) SHA1(a4140118dd535af45a691832530d55cd86a23510) )

	ROM_REGION( 0x8000, "tms6100", ROMREGION_ERASEFF ) // 4000-7fff = space reserved for cartridge
	ROM_LOAD( "cd2610.vsm", 0x0000, 0x1000, CRC(6db34e5a) SHA1(10fa5db20fdcba68034058e7194f35c90b9844e6) )
ROM_END

ROM_START( tntelluk )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "cd8012nl", 0x0000, 0x0800, CRC(3d0fee24) SHA1(8b1b1df03d50ffe8adea59ece212dece5245fe86) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_cd8012_micro.pla", 0, 867, CRC(46d936c8) SHA1(b0aad486a90a5dec7fd2fb07caa503be771f91c8) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_cd8012_output.pla", 0, 365, CRC(5ada9306) SHA1(a4140118dd535af45a691832530d55cd86a23510) )

	ROM_REGION( 0x8000, "tms6100", ROMREGION_ERASEFF ) // 4000-7fff = space reserved for cartridge
	ROM_LOAD( "cd62170.vsm", 0x0000, 0x4000, CRC(6dc9d072) SHA1(9d2c9ff57c4f8fe69768666ffa41fcac649279ef) )
ROM_END

ROM_START( tntellfr )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "cd8012nl", 0x0000, 0x0800, CRC(3d0fee24) SHA1(8b1b1df03d50ffe8adea59ece212dece5245fe86) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_cd8012_micro.pla", 0, 867, CRC(46d936c8) SHA1(b0aad486a90a5dec7fd2fb07caa503be771f91c8) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_cd8012_output.pla", 0, 365, CRC(5ada9306) SHA1(a4140118dd535af45a691832530d55cd86a23510) )

	ROM_REGION( 0x8000, "tms6100", ROMREGION_ERASEFF ) // 4000-7fff = space reserved for cartridge
	ROM_LOAD( "cd62171.vsm", 0x0000, 0x4000, CRC(cc26f7d1) SHA1(2b03e37b3bf3cbeca36980acfc45246dac706b83) )
ROM_END

ROM_START( tntellp )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "us4403965_cd1100", 0x0000, 0x0800, BAD_DUMP CRC(863a1c9e) SHA1(f2f9eb0ae17eedd4ef2b887b34601e75b4f6c720) ) // typed in from patent US4403965/EP0048835A2, may have errors

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_cd8012_micro.pla", 0, 867, CRC(46d936c8) SHA1(b0aad486a90a5dec7fd2fb07caa503be771f91c8) ) // from cd8012, matches patent source code
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_us4403965_output.pla", 0, 365, CRC(66cfb3c3) SHA1(80a05e5d729518e1f35d8f26438f56e80ffbd003) )

	ROM_REGION( 0x8000, "tms6100", ROMREGION_ERASEFF ) // 4000-7fff = space reserved for cartridge
	ROM_LOAD( "cd2610.vsm", 0x0000, 0x1000, CRC(6db34e5a) SHA1(10fa5db20fdcba68034058e7194f35c90b9844e6) )
ROM_END


ROM_START( vocaid )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "cd8012nl", 0x0000, 0x0800, CRC(3d0fee24) SHA1(8b1b1df03d50ffe8adea59ece212dece5245fe86) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_cd8012_micro.pla", 0, 867, CRC(46d936c8) SHA1(b0aad486a90a5dec7fd2fb07caa503be771f91c8) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_cd8012_output.pla", 0, 365, CRC(5ada9306) SHA1(a4140118dd535af45a691832530d55cd86a23510) )

	ROM_REGION( 0x8000, "tms6100", ROMREGION_ERASEFF ) // same hw as tntell, but no external slot
	ROM_LOAD( "cd2357.vsm", 0x0000, 0x4000, CRC(19c251fa) SHA1(8f8163069f32413379e7e1681ce6a4d0819d4ebc) )
ROM_END


ROM_START( k28m2 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mp7324", 0x0000, 0x1000, CRC(08d15ab6) SHA1(5b0f6c53e6732a362c4bb25d966d4072fdd33db8) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common1_micro.pla", 0, 867, CRC(62445fc9) SHA1(d6297f2a4bc7a870b76cc498d19dbb0ce7d69fec) )
	ROM_REGION( 557, "maincpu:opla", 0 )
	ROM_LOAD( "tms1400_k28m2_output.pla", 0, 557, CRC(3a5c7005) SHA1(3fe5819c138a90e7fc12817415f2622ca81b40b2) )

	ROM_REGION( 0x10000, "tms6100", ROMREGION_ERASEFF ) // 8000-bfff? = space reserved for cartridge
	ROM_LOAD( "cm62084.vsm", 0x0000, 0x4000, CRC(cd1376f7) SHA1(96fa484c392c451599bc083b8376cad9c998df7d) )
ROM_END

} // anonymous namespace



//    YEAR  NAME        PARENT   CMP MACHINE       INPUT       CLASS          INIT           COMPANY              FULLNAME                            FLAGS
COMP( 1979, snspell,    0,        0, sns_tmc0281,  snspell,    tispeak_state, init_snspell,  "Texas Instruments", "Speak & Spell (US, 1979 version)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
COMP( 1978, snspellp,   snspell,  0, sns_tmc0281,  snspell,    tispeak_state, init_snspell,  "Texas Instruments", "Speak & Spell (US, patent)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
COMP( 1980, snspellub,  snspell,  0, sns_tmc0281d, snspell,    tispeak_state, init_snspell,  "Texas Instruments", "Speak & Spell (US, 1980 version)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
COMP( 1978, snspellua,  snspell,  0, sns_tmc0281,  snspell,    tispeak_state, init_snspell,  "Texas Instruments", "Speak & Spell (US, 1978 version)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
COMP( 1978, snspelluk,  snspell,  0, sns_tmc0281,  snspell,    tispeak_state, init_snspell,  "Texas Instruments", "Speak & Spell (UK, 1978 version)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
COMP( 1981, snspelluka, snspell,  0, sns_cd2801,   snspell,    tispeak_state, init_snspell,  "Texas Instruments", "Speak & Spell (UK, 1981 version)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
COMP( 1979, snspelljp,  snspell,  0, sns_tmc0281,  snspell,    tispeak_state, init_snspell,  "Texas Instruments", "Speak & Spell (Japan)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
COMP( 1981, snspellsp,  snspell,  0, snspellsp,    snspellsp,  tispeak_state, init_snspell,  "Texas Instruments", "Speak & Spell (Spanish, prototype)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
COMP( 1980, snspellfr,  snspell,  0, sns_cd2801,   snspellfr,  tispeak_state, init_snspell,  "Texas Instruments", u8"La Dictée Magique (France)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
COMP( 1982, snspellit,  snspell,  0, snspellit,    snspellit,  tispeak_state, init_snspell,  "Texas Instruments", "Grillo Parlante (Italy)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )

COMP( 1981, snspellc,   0,        0, snspellc,     snspellc,   tispeak_state, init_snspell,  "Texas Instruments", "Speak & Spell Compact (US, 1981 version)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
COMP( 1982, snspellca,  snspellc, 0, snspellc,     snspellc,   tispeak_state, init_snspell,  "Texas Instruments", "Speak & Spell Compact (US, 1982 version)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
COMP( 1982, snspellcuk, snspellc, 0, snspellcuk,   snspellcuk, tispeak_state, init_snspell,  "Texas Instruments", "Speak & Write (UK)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )

COMP( 1980, snmath,     0,        0, snmath,       snmath,     tispeak_state, empty_init,    "Texas Instruments", "Speak & Math (US, 1980 version)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
COMP( 1986, snmatha,    snmath,   0, snmath,       snmath,     tispeak_state, empty_init,    "Texas Instruments", "Speak & Math (US, 1986 version)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
COMP( 1980, snmathp,    snmath,   0, snmath,       snmath,     tispeak_state, empty_init,    "Texas Instruments", "Speak & Math (US, patent)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND | MACHINE_IS_INCOMPLETE )

COMP( 1980, snread,     0,        0, snread,       snread,     tispeak_state, init_snspell,  "Texas Instruments", "Speak & Read (US)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )

COMP( 1979, lantutor,   0,        0, lantutor,     lantutor,   tispeak_state, init_lantutor, "Texas Instruments", "Language Tutor (patent)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )

COMP( 1981, tntell,     0,        0, tntell,       tntell,     tispeak_state, init_tntell,   "Texas Instruments", "Touch & Tell (US)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND | MACHINE_CLICKABLE_ARTWORK | MACHINE_REQUIRES_ARTWORK )
COMP( 1980, tntellp,    tntell,   0, tntell,       tntell,     tispeak_state, init_tntell,   "Texas Instruments", "Touch & Tell (US, patent)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND | MACHINE_CLICKABLE_ARTWORK | MACHINE_REQUIRES_ARTWORK | MACHINE_NOT_WORKING )
COMP( 1981, tntelluk,   tntell,   0, tntell,       tntell,     tispeak_state, init_tntell,   "Texas Instruments", "Touch & Tell (UK)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND | MACHINE_CLICKABLE_ARTWORK | MACHINE_REQUIRES_ARTWORK )
COMP( 1981, tntellfr,   tntell,   0, tntell,       tntell,     tispeak_state, init_tntell,   "Texas Instruments", "Le Livre Magique (France)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND | MACHINE_CLICKABLE_ARTWORK | MACHINE_REQUIRES_ARTWORK )

COMP( 1982, vocaid,     0,        0, vocaid,       tntell,     tispeak_state, empty_init,    "Texas Instruments", "Vocaid", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND | MACHINE_REQUIRES_ARTWORK )

COMP( 1985, k28m2,      0,        0, k28m2,        k28m2,      tispeak_state, init_snspell,  "Tiger Electronics", "K28: Talking Learning Computer (model 7-232)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
