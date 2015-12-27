// license:BSD-3-Clause
// copyright-holders:hap, Jonathan Gevaryahu, Sean Riddle
/***************************************************************************

  ** subclass of hh_tms1k_state (includes/hh_tms1k.h, drivers/hh_tms1k.cpp) **

  Texas Instruments 1st-gen. handheld speech devices.

  These devices, mostly edu-toys, are based around an MCU(TMS0270/TMS1100),
  TMS51xx speech, and VSM ROM(s). Newer devices, such as Speak & Music,
  are based around the TMP50C40 and belong in another driver, probably.

  note: except for tntell, MAME external artwork is not required. But it
  is objectively a large improvement.

----------------------------------------------------------------------------

Known devices on this hardware: (* denotes not dumped, ** denotes pending dump)


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
    - MCU: TMC0271*
    - TMS51xx: TMC0281
    - VSM(1/2): 16KB TMC0351NL
    - VSM(2/2): 16KB TMC0352NL
    - VFD: NEC FIP8A5AR no. 3A
    - notes: keyboard has buttons instead of cheap membrane

    Speak & Spell (US), 1979
    - MCU: TMC0271* (different from 1978 version)
    - TMS51xx: TMC0281
    - VSM(1/2): 16KB TMC0351N2L
    - VSM(2/2): 16KB TMC0352N2L
    - notes: fixed a funny bug with gibberish-talk when Module button is pressed
      with no module inserted

    Speak & Spell (US), 1980
    - MCU: same as 1979 version
    - TMS51xx: TMC0281D
    - VSM: 16KB CD2350(rev.A)
    - notes: only 1 VSM, meaning much smaller internal vocabulary

    Speak & Spell (Japan), 1980
    - MCU: TMC0271* (assume same as US 1978 or 1979 version)
    - TMS51xx: TMC0281
    - VSM(1/2): 16KB CD2321
    - VSM(2/2): 16KB CD2322
    - notes: no local name for the product, words are in English but very low difficulty

    Speak & Spell (UK), 1978
    - MCU: TMC0271* (assume same as US 1978 version)
    - TMS51xx: TMC0281
    - VSM(1/2): 16KB CD2303
    - VSM(2/2): 16KB CD2304
    - notes: voice data was manually altered to give it a UK accent,
      here's a small anecdote from developer:
          "(...) I cannot bear to listen the product even now. I remember the
           word 'butcher' took 3 days - I still don't know if it sounds right."

    Speak & Spell (UK), 1981
    - MCU: TMC0271* (assume same as US 1979 version)
    - TMS51xx: CD2801
    - VSM: 16KB CD62175
    - VFD: same as Speak & Math(!)
    - notes: this one has a dedicated voice actor

    Speak & Spell (France) "La Dictee Magique", 1980
    - MCU: CD2702**
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
    - MCU: CD8011**
    - TMS51xx: TMC0281D
    - VSM: 16KB CD2354, CD2354(rev.A)
    - notes: no display, MCU is TMS1100 instead of TMS0270, overall similar to Touch & Tell

    Speak & Spell Compact (UK) "Speak & Write", 1981
    - MCU & TMS51xx: same as US 1981 version
    - VSM: 16KB CD62174(rev.A)
    - notes: anecdotes from the developer, the same person working on the original UK version:
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
    - No.2: Les Mots Difficilies: VSM: 16KB? CD62177*
    - No.3: Les Animaux Familiers: VSM: 16KB? CD62047*
    - No.4: Les Magasins De La Rue: VSM: 16KB CD62048
    - No.5: Les Extra-Terrestres: VSM: 16KB? CD62178*

    Italian:
    - Super Modulo: VSM: 16KB? CD62313*


Speak & Math:

    Speak & Math (US), 1980 (renamed to "Speak & Maths" in UK, but is the same product)
    - MCU: CD2704*
    - TMS51xx: CD2801
    - VSM(1/2): 16KB CD2392
    - VSM(2/2): 16KB CD2393
    - VFD: Futaba 9SY -02Z 7E
    - notes: As with the Speak & Spell, the voice actor was a radio announcer.
      However, the phrase "is greater than or less than" had to be added in a
      hurry by one of the TI employees in a hurry, the day before a demo.
      Apparently QA never found out and it ended up in the final product.

    Speak & Math (US), 1986
    - MCU: CD2708, labeled CD2708N2L (die labeled TMC0270F 2708A)
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
    - MCU: CD2705, labeled CD2705B-N2L (die labeled TMC0270E 2705B) - 2nd revision?
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
    - A Ghost in the House: VSM: 16KB CD3536*
    - On the Track: VSM: 16KB CD3538
    - The Third Circle: VSM: 16KB CD3539*
    - The Millionth Knight: VSM: 16KB CD3540


Touch & Tell:

    Touch & Tell (US), 1981
    - MCU: CD8012
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
    - 04: a - Colors
    - 01: b - Objects
    - 05: c - Shapes
    - 09: d - Home Scene
    tntelluk CD62170, tntellfr CD62171:
    - see tntell
    - see numfun(not A)
    - see animalfr
    - 08: ? - Clown Face
    - 0B: ? - Body Parts
    vocaid CD2357:
    - 1C: 1 - Leisure
    - 1E: 2 - Telephone
    - 1B: 3 - Bedside
    - 1D: 4 - Alphabet
    alphabet CD2611:
    - 0E: 1a - Alphabet A-M
    - 0D: 1b - Alphabet N-Z
    - 0C: 1c - Letter Jumble A-M
    - 0B: 1d - Letter Jumble N-Z
    animalfr CD2355:
    - 0A: 2a - Farm Animals
    - 0F: 2b - At The Farm
    - 0E: 2c - Animal Babies
    - 0D: 2d - In The Jungle
    numfun CD2612:
    - 02/0A(rev.A): 3a - Numbers 1-10
    - 03/0F(rev.A): 3b - Numbers 11-30
    - 07/0D(rev.A): 3c - How Many?
    - 06/0E(rev.A): 3d - Hidden Numbers
    aboutme CD2613:
    - 0E: 4a - Clown Face
    - 0B: 4b - Body Parts
    - 0D: 4c - Things to Wear
    - 0C: 4d - Just For Me
    wot CD2361:
    - 0A: 5a - On Land
    - 0B: 5b - In The Air
    - 0C: 5c - On The Water
    - 0D: 5d - In Space
    - 10: 5e - What Belongs Here?
    - 11: 5f - How It Used To Be
    - 12: 5g - Word Fun
    - 13: 5h - In the Surprise Garage
    lilcreat CD2362:
    - 14: 6a - In The Park
    - 15: 6b - In The Sea
    - 16: 6c - In The Woods
    - 17: 6d - Whose House?
    - 18: 6e - Hide & Seek
    - 1A: 6f - Who Is It?
    - 19: 6g - But It's Not
    - 1B: 6h - Word Fun
    et CD2363:
    - 0F: 7a - The Adventure On Earth I
    - 10: 7b - The Adventure On Earth II
    - 11: 7c - Fun And Friendship I
    - 12: 7d - Fun And Friendship II
    - 13: 7e - E.T. The Star I
    - 14: 7f - E.T. The Star II
    - 15: 7g - Do You Remember? I
    - 16: 7h - Do You Remember? II


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


----------------------------------------------------------------------------

  TODO:
  - why doesn't lantutor work?
  - emulate other known devices


***************************************************************************/

#include "includes/hh_tms1k.h"
#include "sound/tms5110.h"
#include "machine/tms6100.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "softlist.h"

// internal artwork
#include "lantutor.lh"
#include "snmath.lh"
#include "snspell.lh"
#include "tntell.lh" // keyboard overlay

// The master clock is a single stage RC oscillator into TMS5100 RCOSC:
// In an early 1979 Speak & Spell, C is 68pf, R is a 50kohm trimpot which is set to around 33.6kohm
// (measured in-circuit). CPUCLK is this osc freq /2, ROMCLK is this osc freq /4.
// The typical osc freq curve for TMS5100 is unknown. Let's assume it is set to the default frequency,
// which is 640kHz for 8KHz according to the TMS5100 documentation.

#define MASTER_CLOCK (640000)


class tispeak_state : public hh_tms1k_state
{
public:
	tispeak_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag),
		m_tms5100(*this, "tms5100"),
		m_tms6100(*this, "tms6100"),
		m_cart(*this, "cartslot")
	{ }

	// devices
	required_device<tms5110_device> m_tms5100;
	required_device<tms6100_device> m_tms6100;
	optional_device<generic_slot_device> m_cart;

	virtual DECLARE_INPUT_CHANGED_MEMBER(power_button) override;
	void power_off();
	void prepare_display();

	DECLARE_READ8_MEMBER(snspell_read_k);
	DECLARE_WRITE16_MEMBER(snmath_write_o);
	DECLARE_WRITE16_MEMBER(snspell_write_o);
	DECLARE_WRITE16_MEMBER(snspell_write_r);
	DECLARE_WRITE16_MEMBER(lantutor_write_r);

	DECLARE_READ8_MEMBER(tntell_read_k);
	DECLARE_WRITE16_MEMBER(tntell_write_o);
	DECLARE_WRITE16_MEMBER(tntell_write_r);

	// cartridge
	UINT32 m_cart_max_size;
	UINT8* m_cart_base;
	void init_cartridge();
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(tispeak_cartridge);
	DECLARE_DRIVER_INIT(snspell);
	DECLARE_DRIVER_INIT(tntell);
	DECLARE_DRIVER_INIT(lantutor);

	UINT8 m_overlay;
	TIMER_DEVICE_CALLBACK_MEMBER(tntell_get_overlay);

protected:
	virtual void machine_start() override;
};


void tispeak_state::machine_start()
{
	hh_tms1k_state::machine_start();
	memset(m_display_segmask, ~0, sizeof(m_display_segmask)); // !

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

DEVICE_IMAGE_LOAD_MEMBER(tispeak_state, tispeak_cartridge)
{
	UINT32 size = m_cart->common_get_size("rom");

	if (size > m_cart_max_size)
	{
		image.seterror(IMAGE_ERROR_UNSPECIFIED, "Invalid file size");
		return IMAGE_INIT_FAIL;
	}

	m_cart->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return IMAGE_INIT_PASS;
}


DRIVER_INIT_MEMBER(tispeak_state, snspell)
{
	m_cart_max_size = 0x4000;
	m_cart_base = memregion("tms6100")->base() + 0x8000;
}

DRIVER_INIT_MEMBER(tispeak_state, tntell)
{
	m_cart_max_size = 0x4000;
	m_cart_base = memregion("tms6100")->base() + 0x4000;
}

DRIVER_INIT_MEMBER(tispeak_state, lantutor)
{
	m_cart_max_size = 0x10000;
	m_cart_base = memregion("tms6100")->base();
}



/***************************************************************************

  I/O

***************************************************************************/

// common/snspell

void tispeak_state::prepare_display()
{
	UINT16 gridmask = (m_display_decay[15][16] != 0) ? 0xffff : 0x8000;
	display_matrix_seg(16+1, 16, m_plate | 0x10000, m_grid & gridmask, 0x3fff);
}

WRITE16_MEMBER(tispeak_state::snspell_write_r)
{
	// R13: power-off request, on falling edge
	if ((m_r >> 13 & 1) && !(data >> 13 & 1))
		power_off();

	// R0-R7: input mux and select digit (+R8 if the device has 9 digits)
	// R15: filament on
	// other bits: MCU internal use
	m_r = m_inp_mux = data;
	m_grid = data & 0x81ff;
	prepare_display();
}

WRITE16_MEMBER(tispeak_state::snspell_write_o)
{
	// reorder opla to led14seg, plus DP as d14 and AP as d15:
	// note: lantutor and snread VFD has an accent triangle instead of DP, and no AP
	// E,D,C,G,B,A,I,M,L,K,N,J,[AP],H,F,[DP] (sidenote: TI KLMN = MAME MLNK)
	m_plate = BITSWAP16(data,12,15,10,7,8,9,11,6,13,3,14,0,1,2,4,5);
	prepare_display();
}

READ8_MEMBER(tispeak_state::snspell_read_k)
{
	// K: multiplexed inputs (note: the Vss row is always on)
	return m_inp_matrix[8]->read() | read_inputs(8);
}


void tispeak_state::power_off()
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_tms5100->reset();

	m_power_on = false;
}


// snmath specific

WRITE16_MEMBER(tispeak_state::snmath_write_o)
{
	// reorder opla to led14seg, plus DP as d14 and CT as d15:
	// [DP],D,C,H,F,B,I,M,L,K,N,J,[CT],E,G,A (sidenote: TI KLMN = MAME MLNK)
	m_plate = BITSWAP16(data,12,0,10,7,8,9,11,6,3,14,4,13,1,2,5,15);
	prepare_display();
}


// lantutor specific

WRITE16_MEMBER(tispeak_state::lantutor_write_r)
{
	// same as default, except R13 is used for an extra digit
	m_r = m_inp_mux = data;
	m_grid = data & 0xa1ff;
	prepare_display();
}


// tntell specific

WRITE16_MEMBER(tispeak_state::tntell_write_r)
{
	// R10: CD2802 PDC pin
	m_tms5100->pdc_w(data >> 10);

	// R9: power-off request, on falling edge
	if ((m_r >> 9 & 1) && !(data >> 9 & 1))
		power_off();

	// R0-R8: input mux
	m_r = m_inp_mux = data;
}

WRITE16_MEMBER(tispeak_state::tntell_write_o)
{
	// O3210: CD2802 CTL8124
	m_o = BITSWAP8(data,7,6,5,4,3,0,1,2);
	m_tms5100->ctl_w(space, 0, m_o & 0xf);
}

READ8_MEMBER(tispeak_state::tntell_read_k)
{
	// multiplexed inputs (and K2 from on-button)
	UINT8 k = m_inp_matrix[9]->read() | read_inputs(9);

	// K4: CD2802 CTL1
	if (m_tms5100->ctl_r(space, 0) & 1)
		k |= 4;

	// K8: overlay code from R5,O4-O7
	if (((m_r >> 1 & 0x10) | (m_o >> 4 & 0xf)) & m_overlay)
		k |= 8;

	return k;
}

TIMER_DEVICE_CALLBACK_MEMBER(tispeak_state::tntell_get_overlay)
{
	// Each keyboard overlay insert has 5 holes, used by the game to determine
	// which one is active(if any). If it matches with the internal ROM or
	// external module, the game continues.
	// 00 for none, 1F for diagnostics, see comment section above for a list

	// try to get overlay code from artwork file(in decimal), otherwise pick the
	// one that was selected in machine configuration
	m_overlay = output_get_value("overlay_code") & 0x1f;
	if (m_overlay == 0)
		m_overlay = m_inp_matrix[10]->read();

	for (int i = 0; i < 5; i++)
		output_set_indexed_value("ol", i+1, m_overlay >> i & 1);
}



/***************************************************************************

  Inputs

***************************************************************************/

INPUT_CHANGED_MEMBER(tispeak_state::power_button)
{
	int on = (int)(FPTR)param;

	if (on && !m_power_on)
	{
		m_power_on = true;
		m_maincpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
	}
	else if (!on && m_power_on)
		power_off();
}

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
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_NAME("Module")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Erase")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Enter")

	PORT_START("IN.6") // R6
	PORT_BIT( 0x1f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.7") // R7
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PGDN) PORT_NAME("Off") // -> auto_power_off
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_HOME) PORT_NAME("Go")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_NAME("Replay")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_NAME("Repeat")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_NAME("Clue")

	PORT_START("IN.8") // Vss!
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_NAME("Mystery Word")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_NAME("Secret Code")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_NAME("Letter")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_NAME("Say It")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_PGUP) PORT_NAME("Spell/On") PORT_CHANGED_MEMBER(DEVICE_SELF, tispeak_state, power_button, (void *)true)
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
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_PGDN) PORT_NAME("Off") // -> auto_power_off
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
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ASTERISK) PORT_NAME(UTF8_MULTIPLY)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH_PAD) PORT_NAME(UTF8_DIVIDE) // /
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("Mix It")

	PORT_START("IN.6") // R6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME("Number Stumper")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_U) PORT_NAME("Write It")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Y) PORT_NAME("Greater/Less")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("Word Problems")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_CODE(KEYCODE_PGUP) PORT_NAME("Solve It/On") PORT_CHANGED_MEMBER(DEVICE_SELF, tispeak_state, power_button, (void *)true)

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
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_PGUP) PORT_NAME("Word Zap/On") PORT_CHANGED_MEMBER(DEVICE_SELF, tispeak_state, power_button, (void *)true)
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
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL ) // overlay code

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
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_CODE(KEYCODE_PGDN) PORT_NAME("Grid 6-1 (Off)") // -> auto_power_off
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_K) PORT_NAME("Grid 6-2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("Grid 6-4")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_L) PORT_NAME("Grid 6-3")

	PORT_START("IN.9") // Vss!
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_STOP) PORT_CODE(KEYCODE_PGUP) PORT_NAME("Grid 6-6 (On)") PORT_CHANGED_MEMBER(DEVICE_SELF, tispeak_state, power_button, (void *)true)

	PORT_START("IN.10")
	PORT_CONFNAME( 0x1f, 0x04, "Overlay Code" ) // only if not provided by external artwork
	PORT_CONFSETTING(    0x00, "00 (None)" )
	PORT_CONFSETTING(    0x01, "01" )
	PORT_CONFSETTING(    0x02, "02" )
	PORT_CONFSETTING(    0x03, "03" )
	PORT_CONFSETTING(    0x04, "04" )
	PORT_CONFSETTING(    0x05, "05" )
	PORT_CONFSETTING(    0x06, "06" )
	PORT_CONFSETTING(    0x07, "07" )
	PORT_CONFSETTING(    0x08, "08" )
	PORT_CONFSETTING(    0x09, "09" )
	PORT_CONFSETTING(    0x0a, "0A" )
	PORT_CONFSETTING(    0x0b, "0B" )
	PORT_CONFSETTING(    0x0c, "0C" )
	PORT_CONFSETTING(    0x0d, "0D" )
	PORT_CONFSETTING(    0x0e, "0E" )
	PORT_CONFSETTING(    0x0f, "0F" )
	PORT_CONFSETTING(    0x10, "10" )
	PORT_CONFSETTING(    0x11, "11" )
	PORT_CONFSETTING(    0x12, "12" )
	PORT_CONFSETTING(    0x13, "13" )
	PORT_CONFSETTING(    0x14, "14" )
	PORT_CONFSETTING(    0x15, "15" )
	PORT_CONFSETTING(    0x16, "16" )
	PORT_CONFSETTING(    0x17, "17" )
	PORT_CONFSETTING(    0x18, "18" )
	PORT_CONFSETTING(    0x19, "19" )
	PORT_CONFSETTING(    0x1a, "1A" )
	PORT_CONFSETTING(    0x1b, "1B" )
	PORT_CONFSETTING(    0x1c, "1C" )
	PORT_CONFSETTING(    0x1d, "1D" )
	PORT_CONFSETTING(    0x1e, "1E" )
	PORT_CONFSETTING(    0x1f, "1F (Diagnostic)" )
INPUT_PORTS_END



/***************************************************************************

  Machine Config

***************************************************************************/

static MACHINE_CONFIG_FRAGMENT( tms5110_route )

	/* sound hardware */
	MCFG_TMS5110_M0_CB(DEVWRITELINE("tms6100", tms6100_device, m0_w))
	MCFG_TMS5110_M1_CB(DEVWRITELINE("tms6100", tms6100_device, m1_w))
	MCFG_TMS5110_ADDR_CB(DEVWRITE8("tms6100", tms6100_device, addr_w))
	MCFG_TMS5110_DATA_CB(DEVREADLINE("tms6100", tms6100_device, data_line_r))
	MCFG_TMS5110_ROMCLK_CB(DEVWRITELINE("tms6100", tms6100_device, romclock_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( snmath, tispeak_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS0270, MASTER_CLOCK/2)
	MCFG_TMS1XXX_READ_K_CB(READ8(tispeak_state, snspell_read_k))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(tispeak_state, snmath_write_o))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(tispeak_state, snspell_write_r))

	MCFG_TMS0270_READ_CTL_CB(DEVREAD8("tms5100", tms5110_device, ctl_r))
	MCFG_TMS0270_WRITE_CTL_CB(DEVWRITE8("tms5100", tms5110_device, ctl_w))
	MCFG_TMS0270_WRITE_PDC_CB(DEVWRITELINE("tms5100", tms5110_device, pdc_w))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_tms1k_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_snmath)

	/* sound hardware */
	MCFG_DEVICE_ADD("tms6100", TMS6100, MASTER_CLOCK/4)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("tms5100", CD2801, MASTER_CLOCK)
	MCFG_FRAGMENT_ADD(tms5110_route)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( sns_cd2801, snmath )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(tispeak_state, snspell_write_o))

	MCFG_DEFAULT_LAYOUT(layout_snspell)

	/* cartridge */
	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_plain_slot, "snspell")
	MCFG_GENERIC_EXTENSIONS("vsm")
	MCFG_GENERIC_LOAD(tispeak_state, tispeak_cartridge)

	MCFG_SOFTWARE_LIST_ADD("cart_list", "snspell")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( sns_tmc0281, sns_cd2801 )

	/* sound hardware */
	MCFG_SOUND_REPLACE("tms5100", TMC0281, MASTER_CLOCK)
	MCFG_FRAGMENT_ADD(tms5110_route)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( sns_tmc0281d, sns_cd2801 )

	/* sound hardware */
	MCFG_SOUND_REPLACE("tms5100", TMC0281D, MASTER_CLOCK)
	MCFG_FRAGMENT_ADD(tms5110_route)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( sns_cd2801_m, sns_cd2801 )

	/* basic machine hardware */
	MCFG_DEFAULT_LAYOUT(layout_snmath)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( snread, snmath )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(tispeak_state, snspell_write_o))

	MCFG_DEFAULT_LAYOUT(layout_lantutor)

	/* cartridge */
	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_plain_slot, "snread")
	MCFG_GENERIC_EXTENSIONS("vsm")
	MCFG_GENERIC_LOAD(tispeak_state, tispeak_cartridge)

	MCFG_SOFTWARE_LIST_ADD("cart_list", "snread")
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( lantutor, snmath )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(tispeak_state, snspell_write_o))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(tispeak_state, lantutor_write_r))

	MCFG_DEFAULT_LAYOUT(layout_lantutor)

	/* cartridge */
	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_plain_slot, "lantutor")
	MCFG_GENERIC_MANDATORY
	MCFG_GENERIC_EXTENSIONS("vsm,bin")
	MCFG_GENERIC_LOAD(tispeak_state, tispeak_cartridge)

	MCFG_SOFTWARE_LIST_ADD("cart_list", "lantutor")
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( vocaid, tispeak_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS1100, MASTER_CLOCK/2)
	MCFG_TMS1XXX_READ_K_CB(READ8(tispeak_state, tntell_read_k))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(tispeak_state, tntell_write_o))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(tispeak_state, tntell_write_r))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_tms1k_state, display_decay_tick, attotime::from_msec(1))
	MCFG_TIMER_DRIVER_ADD_PERIODIC("ol_timer", tispeak_state, tntell_get_overlay, attotime::from_msec(50))
	MCFG_DEFAULT_LAYOUT(layout_tntell)

	/* sound hardware */
	MCFG_DEVICE_ADD("tms6100", TMS6100, MASTER_CLOCK/4)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("tms5100", CD2802, MASTER_CLOCK)
	MCFG_FRAGMENT_ADD(tms5110_route)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( tntell, vocaid )

	/* cartridge */
	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_plain_slot, "tntell")
	MCFG_GENERIC_EXTENSIONS("vsm")
	MCFG_GENERIC_LOAD(tispeak_state, tispeak_cartridge)

	MCFG_SOFTWARE_LIST_ADD("cart_list", "tntell")
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

#define rom_snspellp rom_snspell // until we have a correct dump

ROM_START( snspell )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "us4189779_tmc0271", 0x0000, 0x1000, CRC(d3f5a37d) SHA1(f75ab617a6067d4d3a954a9f86126d2089554df8) ) // typed in from patent US4189779, verified by 2 sources

	ROM_REGION( 1246, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0980_common1_instr.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 2127, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0270_common1_micro.pla", 0, 2127, BAD_DUMP CRC(504b96bb) SHA1(67b691e7c0b97239410587e50e5182bf46475b43) ) // not verified
	ROM_REGION( 1246, "maincpu:opla", 0 )
	ROM_LOAD( "tms0270_tmc0271_output.pla", 0, 1246, CRC(9ebe12ab) SHA1(acb4e07ba26f2daca5f1c234885ac0371c7ce87f) )

	ROM_REGION( 0xc000, "tms6100", ROMREGION_ERASEFF ) // 8000-bfff = space reserved for cartridge
	ROM_LOAD( "tmc0351nl.vsm", 0x0000, 0x4000, CRC(beea3373) SHA1(8b0f7586d2f12c3d4a885fdb528cf23feffa1a3b) )
	ROM_LOAD( "tmc0352nl.vsm", 0x4000, 0x4000, CRC(d51f0587) SHA1(ddaa484be1bba5fef46b481cafae517e4acaa8ed) )
ROM_END

ROM_START( snspella )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "us4189779_tmc0271", 0x0000, 0x1000, BAD_DUMP CRC(d3f5a37d) SHA1(f75ab617a6067d4d3a954a9f86126d2089554df8) ) // placeholder, use the one we have

	ROM_REGION( 1246, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0980_common1_instr.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 2127, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0270_common1_micro.pla", 0, 2127, BAD_DUMP CRC(504b96bb) SHA1(67b691e7c0b97239410587e50e5182bf46475b43) ) // not verified
	ROM_REGION( 1246, "maincpu:opla", 0 )
	ROM_LOAD( "tms0270_tmc0271_output.pla", 0, 1246, CRC(9ebe12ab) SHA1(acb4e07ba26f2daca5f1c234885ac0371c7ce87f) )

	ROM_REGION( 0xc000, "tms6100", ROMREGION_ERASEFF ) // 8000-bfff = space reserved for cartridge
	ROM_LOAD( "tmc0351n2l.vsm", 0x0000, 0x4000, CRC(2d03b292) SHA1(a3e9a365307ae936c7472f720a7a8240741531d6) )
	ROM_LOAD( "tmc0352n2l.vsm", 0x4000, 0x4000, CRC(a6d56883) SHA1(eebf9c07f2f9001679dec06c2367d4a50596d04b) )
ROM_END

ROM_START( snspellb )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "us4189779_tmc0271", 0x0000, 0x1000, BAD_DUMP CRC(d3f5a37d) SHA1(f75ab617a6067d4d3a954a9f86126d2089554df8) ) // placeholder, use the one we have

	ROM_REGION( 1246, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0980_common1_instr.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 2127, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0270_common1_micro.pla", 0, 2127, BAD_DUMP CRC(504b96bb) SHA1(67b691e7c0b97239410587e50e5182bf46475b43) ) // not verified
	ROM_REGION( 1246, "maincpu:opla", 0 )
	ROM_LOAD( "tms0270_tmc0271_output.pla", 0, 1246, CRC(9ebe12ab) SHA1(acb4e07ba26f2daca5f1c234885ac0371c7ce87f) )

	ROM_REGION( 0xc000, "tms6100", ROMREGION_ERASEFF ) // uses only 1 rom, 8000-bfff = space reserved for cartridge
	ROM_LOAD( "cd2350a.vsm", 0x0000, 0x4000, CRC(2adda742) SHA1(3f868ed8284b723c815a30343057e03467c043b5) )
ROM_END

ROM_START( snspelluk )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "us4189779_tmc0271", 0x0000, 0x1000, BAD_DUMP CRC(d3f5a37d) SHA1(f75ab617a6067d4d3a954a9f86126d2089554df8) ) // placeholder, use the one we have

	ROM_REGION( 1246, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0980_common1_instr.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 2127, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0270_common1_micro.pla", 0, 2127, BAD_DUMP CRC(504b96bb) SHA1(67b691e7c0b97239410587e50e5182bf46475b43) ) // not verified
	ROM_REGION( 1246, "maincpu:opla", 0 )
	ROM_LOAD( "tms0270_tmc0271_output.pla", 0, 1246, CRC(9ebe12ab) SHA1(acb4e07ba26f2daca5f1c234885ac0371c7ce87f) )

	ROM_REGION( 0xc000, "tms6100", ROMREGION_ERASEFF ) // 8000-bfff = space reserved for cartridge
	ROM_LOAD( "cd2303.vsm", 0x0000, 0x4000, CRC(0fae755c) SHA1(b68c3120a63a61db474feb5d71a6e5dd67910d80) )
	ROM_LOAD( "cd2304.vsm", 0x4000, 0x4000, CRC(e2a270eb) SHA1(c13c95ad15f1923a4841f66504e0f22646e71d99) )
ROM_END

ROM_START( snspelluka )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "us4189779_tmc0271", 0x0000, 0x1000, BAD_DUMP CRC(d3f5a37d) SHA1(f75ab617a6067d4d3a954a9f86126d2089554df8) ) // placeholder, use the one we have

	ROM_REGION( 1246, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0980_common1_instr.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 2127, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0270_common1_micro.pla", 0, 2127, BAD_DUMP CRC(504b96bb) SHA1(67b691e7c0b97239410587e50e5182bf46475b43) ) // not verified
	ROM_REGION( 1246, "maincpu:opla", 0 )
	ROM_LOAD( "tms0270_tmc0271_output.pla", 0, 1246, CRC(9ebe12ab) SHA1(acb4e07ba26f2daca5f1c234885ac0371c7ce87f) )

	ROM_REGION( 0xc000, "tms6100", ROMREGION_ERASEFF ) // uses only 1 rom, 8000-bfff = space reserved for cartridge
	ROM_LOAD( "cd62175.vsm", 0x0000, 0x4000, CRC(6e1063d4) SHA1(b5c66c51148c5921ecb8ffccd7a460ae639cdb68) )
ROM_END

ROM_START( snspelljp )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "us4189779_tmc0271", 0x0000, 0x1000, BAD_DUMP CRC(d3f5a37d) SHA1(f75ab617a6067d4d3a954a9f86126d2089554df8) ) // placeholder, use the one we have

	ROM_REGION( 1246, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0980_common1_instr.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 2127, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0270_common1_micro.pla", 0, 2127, BAD_DUMP CRC(504b96bb) SHA1(67b691e7c0b97239410587e50e5182bf46475b43) ) // not verified
	ROM_REGION( 1246, "maincpu:opla", 0 )
	ROM_LOAD( "tms0270_tmc0271_output.pla", 0, 1246, CRC(9ebe12ab) SHA1(acb4e07ba26f2daca5f1c234885ac0371c7ce87f) )

	ROM_REGION( 0xc000, "tms6100", ROMREGION_ERASEFF ) // 8000-bfff = space reserved for cartridge
	ROM_LOAD( "cd2321.vsm", 0x0000, 0x4000, CRC(ac010cce) SHA1(c0200d857b62be696248ac2d684a390c66ab0c31) )
	ROM_LOAD( "cd2322.vsm", 0x4000, 0x4000, CRC(b6f4bba4) SHA1(65d686a9385b5ef3f080a5f47c6b2418bb9455b0) )
ROM_END

ROM_START( snspellfr )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "us4189779_tmc0271", 0x0000, 0x1000, BAD_DUMP CRC(d3f5a37d) SHA1(f75ab617a6067d4d3a954a9f86126d2089554df8) ) // placeholder, use the one we have

	ROM_REGION( 1246, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0980_common1_instr.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 2127, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0270_common1_micro.pla", 0, 2127, BAD_DUMP CRC(504b96bb) SHA1(67b691e7c0b97239410587e50e5182bf46475b43) ) // not verified
	ROM_REGION( 1246, "maincpu:opla", 0 )
	ROM_LOAD( "tms0270_tmc0271_output.pla", 0, 1246, BAD_DUMP CRC(9ebe12ab) SHA1(acb4e07ba26f2daca5f1c234885ac0371c7ce87f) ) // placeholder, use the one we have

	ROM_REGION( 0xc000, "tms6100", ROMREGION_ERASEFF ) // uses only 1 rom, 8000-bfff = space reserved for cartridge
	ROM_LOAD( "cd2352.vsm", 0x0000, 0x4000, CRC(181a239e) SHA1(e16043766c385e152b7005c1c010be4c5fccdd9b) )
ROM_END

ROM_START( snspellit )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "us4189779_tmc0271", 0x0000, 0x1000, BAD_DUMP CRC(d3f5a37d) SHA1(f75ab617a6067d4d3a954a9f86126d2089554df8) ) // placeholder, use the one we have

	ROM_REGION( 1246, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0980_common1_instr.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 2127, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0270_common1_micro.pla", 0, 2127, BAD_DUMP CRC(504b96bb) SHA1(67b691e7c0b97239410587e50e5182bf46475b43) ) // not verified
	ROM_REGION( 1246, "maincpu:opla", 0 )
	ROM_LOAD( "tms0270_tmc0271_output.pla", 0, 1246, BAD_DUMP CRC(9ebe12ab) SHA1(acb4e07ba26f2daca5f1c234885ac0371c7ce87f) ) // placeholder, use the one we have

	ROM_REGION( 0xc000, "tms6100", ROMREGION_ERASEFF ) // uses only 1 rom, 8000-bfff = space reserved for cartridge
	ROM_LOAD( "cd62190.vsm", 0x0000, 0x4000, CRC(63832002) SHA1(ea8124b2bf0f5908c5f1a56d60063f2468a10143) )
ROM_END


ROM_START( snmath )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "cd2708n2l", 0x0000, 0x1000, CRC(35937360) SHA1(69c362c75bb459056c09c7fab37c91040485474b) )

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
	ROM_LOAD( "tms0270_common1_micro.pla", 0, 2127, BAD_DUMP CRC(504b96bb) SHA1(67b691e7c0b97239410587e50e5182bf46475b43) ) // not verified
	ROM_REGION( 1246, "maincpu:opla", 0 )
	ROM_LOAD( "tms0270_cd2708_output.pla", 0, 1246, BAD_DUMP CRC(1abad753) SHA1(53d20b519ed73ce248368047a056836afbe3cd46) ) // taken from cd2708, need to verify if it's same as cd2704

	ROM_REGION( 0x8000, "tms6100", ROMREGION_ERASEFF )
	ROM_LOAD( "cd2392.vsm", 0x0000, 0x4000, CRC(4ed2e920) SHA1(8896f29e25126c1e4d9a47c9a325b35dddecc61f) )
	ROM_LOAD( "cd2393.vsm", 0x4000, 0x4000, CRC(571d5b5a) SHA1(83284755d9b77267d320b5b87fdc39f352433715) )
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

	ROM_REGION( 0xc000, "tms6100", ROMREGION_ERASEFF ) // 8000-bfff = space reserved for cartridge
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
	ROM_LOAD( "tms0270_tmc0271_output.pla", 0, 1246, BAD_DUMP CRC(9ebe12ab) SHA1(acb4e07ba26f2daca5f1c234885ac0371c7ce87f) ) // taken from snspell, mostly looks correct

	ROM_REGION( 0x10000, "tms6100", ROMREGION_ERASEFF ) // cartridge area
ROM_END


ROM_START( tntell )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "cd8012", 0x0000, 0x0800, CRC(3d0fee24) SHA1(8b1b1df03d50ffe8adea59ece212dece5245fe86) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_cd8012_micro.pla", 0, 867, CRC(46d936c8) SHA1(b0aad486a90a5dec7fd2fb07caa503be771f91c8) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_cd8012_output.pla", 0, 365, CRC(5ada9306) SHA1(a4140118dd535af45a691832530d55cd86a23510) )

	ROM_REGION( 0x8000, "tms6100", ROMREGION_ERASEFF ) // 4000-7fff = space reserved for cartridge
	ROM_LOAD( "cd2610.vsm", 0x0000, 0x1000, CRC(6db34e5a) SHA1(10fa5db20fdcba68034058e7194f35c90b9844e6) )
ROM_END

ROM_START( tntelluk )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "cd8012", 0x0000, 0x0800, CRC(3d0fee24) SHA1(8b1b1df03d50ffe8adea59ece212dece5245fe86) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_cd8012_micro.pla", 0, 867, CRC(46d936c8) SHA1(b0aad486a90a5dec7fd2fb07caa503be771f91c8) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_cd8012_output.pla", 0, 365, CRC(5ada9306) SHA1(a4140118dd535af45a691832530d55cd86a23510) )

	ROM_REGION( 0x8000, "tms6100", ROMREGION_ERASEFF ) // 4000-7fff = space reserved for cartridge
	ROM_LOAD( "cd62170.vsm", 0x0000, 0x4000, CRC(6dc9d072) SHA1(9d2c9ff57c4f8fe69768666ffa41fcac649279ef) )
ROM_END

ROM_START( tntellfr )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "cd8012", 0x0000, 0x0800, CRC(3d0fee24) SHA1(8b1b1df03d50ffe8adea59ece212dece5245fe86) )

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
	ROM_LOAD( "cd8012", 0x0000, 0x0800, CRC(3d0fee24) SHA1(8b1b1df03d50ffe8adea59ece212dece5245fe86) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_cd8012_micro.pla", 0, 867, CRC(46d936c8) SHA1(b0aad486a90a5dec7fd2fb07caa503be771f91c8) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_cd8012_output.pla", 0, 365, CRC(5ada9306) SHA1(a4140118dd535af45a691832530d55cd86a23510) )

	ROM_REGION( 0x8000, "tms6100", ROMREGION_ERASEFF ) // same hw as tntell, but no external slot
	ROM_LOAD( "cd2357.vsm", 0x0000, 0x4000, CRC(19c251fa) SHA1(8f8163069f32413379e7e1681ce6a4d0819d4ebc) )
ROM_END



/*    YEAR  NAME        PARENT COMPAT MACHINE     INPUT     INIT                     COMPANY, FULLNAME, FLAGS */
COMP( 1978, snspell,    0,        0, sns_tmc0281, snspell,  tispeak_state, snspell,  "Texas Instruments", "Speak & Spell (US, 1978 version)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) // incomplete dump, uses patent MCU ROM
COMP( 1978, snspellp,   snspell,  0, sns_tmc0281, snspell,  tispeak_state, snspell,  "Texas Instruments", "Speak & Spell (US, patent)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
COMP( 1979, snspella,   snspell,  0, sns_tmc0281, snspell,  tispeak_state, snspell,  "Texas Instruments", "Speak & Spell (US, 1979 version)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) // "
COMP( 1980, snspellb,   snspell,  0, sns_tmc0281d,snspell,  tispeak_state, snspell,  "Texas Instruments", "Speak & Spell (US, 1980 version)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) // "
COMP( 1978, snspelluk,  snspell,  0, sns_tmc0281, snspell,  tispeak_state, snspell,  "Texas Instruments", "Speak & Spell (UK, 1978 version)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) // "
COMP( 1981, snspelluka, snspell,  0, sns_cd2801_m,snspell,  tispeak_state, snspell,  "Texas Instruments", "Speak & Spell (UK, 1981 version)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) // "
COMP( 1979, snspelljp,  snspell,  0, sns_tmc0281, snspell,  tispeak_state, snspell,  "Texas Instruments", "Speak & Spell (Japan)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) // "
COMP( 1980, snspellfr,  snspell,  0, sns_cd2801,  snspell,  tispeak_state, snspell,  "Texas Instruments", "La Dictee Magique (France)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) // doesn't work due to missing CD2702 MCU dump, German/Italian version has CD2702 too
COMP( 1982, snspellit,  snspell,  0, sns_cd2801_m,snspell,  tispeak_state, snspell,  "Texas Instruments", "Grillo Parlante (Italy)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) // "

COMP( 1986, snmath,     0,        0, snmath,      snmath,   driver_device, 0,        "Texas Instruments", "Speak & Math (US, 1986 version)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
COMP( 1980, snmathp,    snmath,   0, snmath,      snmath,   driver_device, 0,        "Texas Instruments", "Speak & Math (US, 1980 version/patent)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )

COMP( 1980, snread,     0,        0, snread,      snread,   tispeak_state, snspell,  "Texas Instruments", "Speak & Read (US)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )

COMP( 1979, lantutor,   0,        0, lantutor,    lantutor, tispeak_state, lantutor, "Texas Instruments", "Language Tutor (patent)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )

COMP( 1981, tntell,     0,        0, tntell,      tntell,   tispeak_state, tntell,   "Texas Instruments", "Touch & Tell (US, 1981 version)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND | MACHINE_REQUIRES_ARTWORK ) // assume there is an older version too, with CD8010 MCU
COMP( 1980, tntellp,    tntell,   0, tntell,      tntell,   tispeak_state, tntell,   "Texas Instruments", "Touch & Tell (patent)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND | MACHINE_REQUIRES_ARTWORK | MACHINE_NOT_WORKING )
COMP( 1981, tntelluk,   tntell,   0, tntell,      tntell,   tispeak_state, tntell,   "Texas Instruments", "Touch & Tell (UK)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND | MACHINE_REQUIRES_ARTWORK )
COMP( 1981, tntellfr,   tntell,   0, tntell,      tntell,   tispeak_state, tntell,   "Texas Instruments", "Le Livre Magique (France)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND | MACHINE_REQUIRES_ARTWORK )

COMP( 1982, vocaid,     0,        0, vocaid,      tntell,   driver_device, 0,        "Texas Instruments", "Vocaid", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND | MACHINE_REQUIRES_ARTWORK )
