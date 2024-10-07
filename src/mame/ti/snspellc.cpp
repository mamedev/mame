// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle, David Viens, Kevin Horton
/*******************************************************************************

Texas Instruments Speak & Spell Compact, Touch & Tell

Speak & Spell Compact (US), 1981
- MCU: CD8011, label CD8011A-NL (die label: 1100B, CD8011A)
- TMS51xx: TMC0281D (die label: T0280F 0281D)
- VSM: 16KB CD2354, CD2354A
- notes: no display, MCU is TMS1100 instead of TMS0270, overall similar to Touch & Tell

Speak & Spell Compact (UK) "Speak & Write", 1981
- MCU: same as US 1981 version
- TMS51xx: CD2801A
- VSM: 16KB CD62174A
- notes: BTANB: gibberish when the module button is pressed (with module present)

Speak & Spell Compact is compatible with Speak & Spell modules.

Anecdotes from the developer, the same person working on the original Speak & Spell
UK version: "We included a pencil and writing pad - it was now about 'writing'."

And one about the welcome message: "I had to manually create a sentence of digital
speech from thin air. I had to write down a 20 character code which would create
each 10/s sound bite that made up the phrase "Welcome to Speak and Write". It took
me 1 week. (...) Even Larry Brantingham was amazed."

================================================================================

Speak & Math 'Compact' (France) "Les Maths Magiques", 1982
- MCU: CP3447-NL (die label: 1100F, MP3447)
- TMS51xx: CD2801A
- VSM: 16KB CD62173A
- notes: this is not the same as "Le Calcul Magique", that's from a
  series centered around a TMS50C40 instead of MCU+TMS51xx

There is no English version, but likewise there is no French version of the
regular Speak & Math. No known external modules were released.

================================================================================

Touch & Tell (US), 1981
- MCU: CD8012, label CD8012NL (die label: 1100G CD8012)
- TMS51xx: CD2802
- VSM: 4KB CD2610
- notes: MCU is TMS1100 instead of TMS0270. CD8010 is seen in some devices
  too, maybe an earlier version? For some reason, it doesn't use the standard
  TMS1100 microinstructions, the opcodes are scrambled.

Touch & Tell (UK), 1981
- MCU & TMS51xx: same as US version
- VSM: 16KB CD62170

Touch & Tell (France) "Le Livre Magique", 1981
- MCU & TMS51xx: same as US version
- VSM: 16KB CD62171

Touch & Tell (Germany) "Tipp & Sprich", 1981
- MCU & TMS51xx: same as US version
- VSM: 16KB? CD62172*

Touch & Tell (Italy) "Libro Parlante", 1982
- MCU & TMS51xx: same as US version
- VSM: 16KB? CD62176* (on a module)

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
- Number Fun: VSM: 4KB CD2612*, CD2612A
- All About Me: VSM: 4KB CD2613
- World of Transportation: VSM: 16KB CD2361
- Little Creatures: VSM: 16KB CD2362
- E.T.: VSM: 16KB CD2363

(* denotes not dumped)

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

*******************************************************************************/

#include "emu.h"

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "cpu/tms1000/tms1400.h"
#include "machine/timer.h"
#include "machine/tms6100.h"
#include "sound/tms5110.h"

#include "render.h"
#include "softlist_dev.h"
#include "speaker.h"

// internal artwork
#include "tntell.lh"


namespace {

// master clock for TMS5100, see snspell driver

static constexpr u32 MASTER_CLOCK = 640'000;


// Speak & Spell Compact / common

class snspellc_state : public driver_device
{
public:
	snspellc_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_tms5100(*this, "tms5100"),
		m_tms6100(*this, "tms6100"),
		m_cart(*this, "cartslot"),
		m_inputs(*this, "IN.%u", 0),
		m_power_on(*this, "power")
	{ }

	// machine configs
	void tms5110_route(machine_config &config);
	void snspellc(machine_config &config);
	void snwrite(machine_config &config);
	void mathsmag(machine_config &config);

	void init_snspellc();

	DECLARE_INPUT_CHANGED_MEMBER(power_on);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	// devices/pointers
	required_device<tms1100_cpu_device> m_maincpu;
	required_device<tms5110_device> m_tms5100;
	required_device<tms6100_device> m_tms6100;
	optional_device<generic_slot_device> m_cart;
	required_ioport_array<10> m_inputs;
	output_finder<> m_power_on;

	u8 *m_cart_base = nullptr;
	u16 m_o = 0;
	u32 m_r = 0;

	void power_off();
	virtual u8 read_k();
	void write_o(u16 data);
	void write_r(u32 data);

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);
};

void snspellc_state::machine_start()
{
	m_power_on.resolve();

	// register for savestates
	save_item(NAME(m_o));
	save_item(NAME(m_r));
}


// Touch & Tell

class tntell_state : public snspellc_state
{
public:
	tntell_state(const machine_config &mconfig, device_type type, const char *tag) :
		snspellc_state(mconfig, type, tag),
		m_overlay_inp(*this, "OVERLAY"),
		m_overlay_out(*this, "ol%u", 1U)
	{ }

	void tntell(machine_config &config);
	void vocaid(machine_config &config);

	void init_tntell();

protected:
	virtual void machine_start() override ATTR_COLD;

	virtual u8 read_k() override;

private:
	optional_ioport m_overlay_inp;
	output_finder<5> m_overlay_out;

	u8 m_overlay_code = 0;

	u8 get_hexchar(const char c);
	TIMER_DEVICE_CALLBACK_MEMBER(get_overlay);
};

void tntell_state::machine_start()
{
	snspellc_state::machine_start();

	m_overlay_out.resolve();
	save_item(NAME(m_overlay_code));
}



/*******************************************************************************
    Power
*******************************************************************************/

void snspellc_state::machine_reset()
{
	m_power_on = 1;
	m_maincpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
}

INPUT_CHANGED_MEMBER(snspellc_state::power_on)
{
	if (newval && !m_power_on)
		machine_reset();
}

void snspellc_state::power_off()
{
	m_power_on = 0;
	m_maincpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}



/*******************************************************************************
    Cartridge
*******************************************************************************/

DEVICE_IMAGE_LOAD_MEMBER(snspellc_state::cart_load)
{
	u32 const size = m_cart->common_get_size("rom");

	if (size > 0x4000)
		return std::make_pair(image_error::INVALIDLENGTH, "Invalid file size (must be no more than 16K)");

	m_cart->common_load_rom(m_cart_base, size, "rom");
	return std::make_pair(std::error_condition(), std::string());
}

void snspellc_state::init_snspellc()
{
	m_cart_base = memregion("tms6100")->base() + 0x8000;
}

void tntell_state::init_tntell()
{
	m_cart_base = memregion("tms6100")->base() + 0x4000;
}



/*******************************************************************************
    I/O
*******************************************************************************/

void snspellc_state::write_r(u32 data)
{
	// R10: TMS5100 PDC pin
	m_tms5100->pdc_w(data >> 10 & 1);

	// R9: power-off request, on falling edge
	if (~data & m_r & 0x200)
		power_off();

	// R0-R8: input mux
	m_r = data;
}

void snspellc_state::write_o(u16 data)
{
	// O3210: TMS5100 CTL8124
	m_tms5100->ctl_w(bitswap<4>(data,3,0,1,2));
	m_o = data;
}

u8 snspellc_state::read_k()
{
	// K4: TMS5100 CTL1
	u8 data = m_tms5100->ctl_r() << 2 & 4;

	// K: multiplexed inputs (note: the Vss row is always on)
	for (int i = 0; i < 9; i++)
		if (BIT(m_r, i))
			data |= m_inputs[i]->read();

	return data | m_inputs[9]->read();
}

u8 tntell_state::read_k()
{
	// K8: overlay code from R5,O4-O7
	u8 k8 = (((m_r >> 1 & 0x10) | (m_o >> 4 & 0xf)) & m_overlay_code) ? 8 : 0;

	// rest is same as snpellc
	return k8 | snspellc_state::read_k();
}



/*******************************************************************************
    Overlay (Touch & Tell)
*******************************************************************************/

u8 tntell_state::get_hexchar(const char c)
{
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'A' && c <= 'F') return c - 'A' + 10;
	if (c >= 'a' && c <= 'f') return c - 'a' + 10;

	return 0;
}

TIMER_DEVICE_CALLBACK_MEMBER(tntell_state::get_overlay)
{
	// Each keyboard overlay insert has 5 holes, used by the game to determine
	// which one is active(if any). If it matches with the internal ROM or
	// external module, the game continues.

	// pick overlay code from input config, see comment section above for reference
	m_overlay_code = m_overlay_inp->read();

	// try to get it from (external) layout
	if (m_overlay_code == 0x20)
	{
		// as output value, eg. with defstate (in decimal)
		m_overlay_code = output().get_value("overlay_code") & 0x1f;

		// and from current view name ($ + 2 hex digits)
		render_target *target = machine().render().first_target();
		const char *name = target->view_name(target->view());

		for (int i = 0; name && i < strlen(name); i++)
			if (name[i] == '$' && strlen(&name[i]) > 2)
				m_overlay_code = (get_hexchar(name[i + 1]) << 4 | get_hexchar(name[i + 2])) & 0x1f;
	}

	// overlay holes
	for (int i = 0; i < 5; i++)
		m_overlay_out[i] = BIT(m_overlay_code, i);
}

static INPUT_PORTS_START( overlay )
	PORT_START("OVERLAY")
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



/*******************************************************************************
    Input Ports
*******************************************************************************/

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
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8) PORT_NAME("Erase")
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
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13) PORT_NAME("Enter")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')

	PORT_START("IN.8") // R8
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'')

	PORT_START("IN.9") // Vss
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_F1) PORT_NAME("Spell/On") PORT_CHANGED_MEMBER(DEVICE_SELF, snspellc_state, power_on, 0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) // speech chip data
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F2) PORT_NAME("Off") // -> auto_power_off
INPUT_PORTS_END

static INPUT_PORTS_START( snwrite )
	PORT_INCLUDE( snspellc )

	PORT_MODIFY("IN.9")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_F1) PORT_NAME("Write/On") PORT_CHANGED_MEMBER(DEVICE_SELF, snspellc_state, power_on, 0) // just the label changed from Spell to Write
INPUT_PORTS_END


static INPUT_PORTS_START( mathsmag )
	PORT_START("IN.0") // R0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME(u8"Répète")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_STOP) PORT_NAME(">")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("<")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Efface")

	PORT_START("IN.1") // R1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0")

	PORT_START("IN.2") // R2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // R3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_DEL_PAD) PORT_NAME("*")

	PORT_START("IN.4") // R4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH_PAD) PORT_NAME(u8"÷")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ASTERISK) PORT_NAME(u8"×")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("-")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("+")

	PORT_START("IN.5") // R5
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.6") // R6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME(u8"Problèmes")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("Plus grand, plus petit que")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME(u8"Dictée")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.7") // R7
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_HOME) PORT_NAME(u8"Départ / Continue")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Essaie")

	PORT_START("IN.8") // R8
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.9") // Vss
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_F1) PORT_NAME("Marche / Calcule") PORT_CHANGED_MEMBER(DEVICE_SELF, snspellc_state, power_on, 0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_F2) PORT_NAME(u8"Arrét") // -> auto_power_off
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( tntell )
	PORT_INCLUDE( overlay )

	PORT_START("IN.0") // R0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_NAME("Grid 1-1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_NAME("Grid 1-2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_NAME("Grid 1-4")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_NAME("Grid 1-3")

	PORT_START("IN.1") // R1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_NAME("Grid 2-1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_NAME("Grid 2-2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_NAME("Grid 2-4")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_NAME("Grid 2-3")

	PORT_START("IN.2") // R2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("Grid 3-1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("Grid 3-2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("Grid 3-4")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("Grid 3-3")

	PORT_START("IN.3") // R3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_U) PORT_NAME("Grid 4-1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME("Grid 4-2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_P) PORT_NAME("Grid 4-4")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_O) PORT_NAME("Grid 4-3")

	PORT_START("IN.4") // R4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("Grid 5-1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("Grid 5-2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("Grid 5-4")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("Grid 5-3")

	PORT_START("IN.5") // R5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_NAME("Grid 5-6")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("Grid 6-5")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("Grid 5-5")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) // overlay code

	PORT_START("IN.6") // R6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("Grid 3-5")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("Grid 2-5")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("Grid 4-5")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_NAME("Grid 1-5")

	PORT_START("IN.7") // R7
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Y) PORT_NAME("Grid 3-6")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("Grid 2-6")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("Grid 4-6")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_NAME("Grid 1-6")

	PORT_START("IN.8") // R8
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_CODE(KEYCODE_F2) PORT_NAME("Grid 6-1 (Off)") // -> auto_power_off
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Grid 6-2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_V) PORT_NAME("Grid 6-4")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("Grid 6-3")

	PORT_START("IN.9") // Vss
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_CODE(KEYCODE_F1) PORT_NAME("Grid 6-6 (On)") PORT_CHANGED_MEMBER(DEVICE_SELF, tntell_state, power_on, 0)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) // speech chip data
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void snspellc_state::tms5110_route(machine_config &config)
{
	// sound hardware
	m_tms5100->m0().set(m_tms6100, FUNC(tms6100_device::m0_w));
	m_tms5100->m1().set(m_tms6100, FUNC(tms6100_device::m1_w));
	m_tms5100->addr().set(m_tms6100, FUNC(tms6100_device::add_w));
	m_tms5100->data().set(m_tms6100, FUNC(tms6100_device::data_line_r));
	m_tms5100->romclk().set(m_tms6100, FUNC(tms6100_device::clk_w));
	m_tms5100->add_route(ALL_OUTPUTS, "mono", 0.5);
}

void snspellc_state::snspellc(machine_config &config)
{
	// basic machine hardware
	TMS1100(config, m_maincpu, MASTER_CLOCK/2);
	m_maincpu->read_k().set(FUNC(snspellc_state::read_k));
	m_maincpu->write_o().set(FUNC(snspellc_state::write_o));
	m_maincpu->write_r().set(FUNC(snspellc_state::write_r));

	// sound hardware
	TMS6100(config, m_tms6100, MASTER_CLOCK/4);

	SPEAKER(config, "mono").front_center();
	TMC0281D(config, m_tms5100, MASTER_CLOCK);
	tms5110_route(config);

	// cartridge
	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "snspell", "vsm,bin");
	m_cart->set_device_load(FUNC(snspellc_state::cart_load));

	SOFTWARE_LIST(config, "cart_list").set_original("snspell");
}

void snspellc_state::snwrite(machine_config &config)
{
	snspellc(config);

	// sound hardware
	CD2801(config.replace(), m_tms5100, MASTER_CLOCK); // CD2801A!
	tms5110_route(config);
}

void snspellc_state::mathsmag(machine_config &config)
{
	snwrite(config);

	// cartridge
	m_cart->set_interface("mathsmag");
	config.device_remove("cart_list"); // N/A
}


void tntell_state::tntell(machine_config &config)
{
	snspellc(config);

	// basic machine hardware
	TIMER(config, "ol_timer").configure_periodic(FUNC(tntell_state::get_overlay), attotime::from_msec(50));
	config.set_default_layout(layout_tntell);

	// sound hardware
	CD2802(config.replace(), m_tms5100, MASTER_CLOCK);
	tms5110_route(config);

	// cartridge
	m_cart->set_interface("tntell");
	subdevice<software_list_device>("cart_list")->set_original("tntell");
}

void tntell_state::vocaid(machine_config &config)
{
	tntell(config);

	// no external module slot
	config.device_remove("cartslot");
	config.device_remove("cart_list");
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( snspellc )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "cd8011a-nl", 0x0000, 0x0800, CRC(8a82a467) SHA1(fa4f8a232392603721bd8136c141a340fd5936a0) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_cd8011_micro.pla", 0, 867, CRC(bbb64ddc) SHA1(602cd5eef897c9115c12db7367c5654ab2297fa1) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_cd8011_output.pla", 0, 365, CRC(b400dd75) SHA1(5a4b5d4532a8932cf4b469ddb71ad6b3b9911672) )

	ROM_REGION( 0x10000, "tms6100", ROMREGION_ERASEFF ) // 8000-bfff = space reserved for cartridge
	ROM_LOAD( "cd2354a", 0x0000, 0x4000, CRC(548a940c) SHA1(c37e620c4c70a05cbaaff9a166c6da2e2420196f) )
ROM_END

ROM_START( snspellca )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "cd8011a-nl", 0x0000, 0x0800, CRC(8a82a467) SHA1(fa4f8a232392603721bd8136c141a340fd5936a0) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_cd8011_micro.pla", 0, 867, CRC(bbb64ddc) SHA1(602cd5eef897c9115c12db7367c5654ab2297fa1) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_cd8011_output.pla", 0, 365, CRC(b400dd75) SHA1(5a4b5d4532a8932cf4b469ddb71ad6b3b9911672) )

	ROM_REGION( 0x10000, "tms6100", ROMREGION_ERASEFF ) // 8000-bfff = space reserved for cartridge
	ROM_LOAD( "cd2354", 0x0000, 0x4000, CRC(3af3232e) SHA1(f89d90dca209ee612634d664d5d4562f1d1786cf) )
ROM_END

ROM_START( snwrite )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "cd8011a-nl", 0x0000, 0x0800, CRC(8a82a467) SHA1(fa4f8a232392603721bd8136c141a340fd5936a0) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_cd8011_micro.pla", 0, 867, CRC(bbb64ddc) SHA1(602cd5eef897c9115c12db7367c5654ab2297fa1) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_cd8011_output.pla", 0, 365, CRC(b400dd75) SHA1(5a4b5d4532a8932cf4b469ddb71ad6b3b9911672) )

	ROM_REGION( 0x10000, "tms6100", ROMREGION_ERASEFF ) // 8000-bfff = space reserved for cartridge
	ROM_LOAD( "cd62174a", 0x0000, 0x4000, CRC(b7bbaaf3) SHA1(9eb949fcf522982f9c3c4649f207703b746b90ef) )
ROM_END


ROM_START( mathsmag )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "cp3447-nl", 0x0000, 0x0800, CRC(890751a1) SHA1(bd8bbd50f8ed31b2ae2d567c22cd92b21021bd20) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common1_micro.pla", 0, 867, CRC(62445fc9) SHA1(d6297f2a4bc7a870b76cc498d19dbb0ce7d69fec) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_mathsmag_output.pla", 0, 365, CRC(d21f19a2) SHA1(9781da173d473c255fa5cc5fcc8ae09c097c682d) )

	ROM_REGION( 0x10000, "tms6100", ROMREGION_ERASEFF ) // 8000-bfff = space reserved for cartridge?
	ROM_LOAD( "cd62173a", 0x0000, 0x4000, CRC(a7230863) SHA1(8a6d1742fb94555f3b3fe37554a7c46fe4213116) )
ROM_END


ROM_START( tntell )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "cd8012nl", 0x0000, 0x0800, CRC(3d0fee24) SHA1(8b1b1df03d50ffe8adea59ece212dece5245fe86) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_cd8012_micro.pla", 0, 867, CRC(46d936c8) SHA1(b0aad486a90a5dec7fd2fb07caa503be771f91c8) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_cd8012_output.pla", 0, 365, CRC(5ada9306) SHA1(a4140118dd535af45a691832530d55cd86a23510) )

	ROM_REGION( 0x8000, "tms6100", ROMREGION_ERASEFF ) // 4000-7fff = space reserved for cartridge
	ROM_LOAD( "cd2610", 0x0000, 0x1000, CRC(6db34e5a) SHA1(10fa5db20fdcba68034058e7194f35c90b9844e6) )
ROM_END

ROM_START( tntelluk )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "cd8012nl", 0x0000, 0x0800, CRC(3d0fee24) SHA1(8b1b1df03d50ffe8adea59ece212dece5245fe86) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_cd8012_micro.pla", 0, 867, CRC(46d936c8) SHA1(b0aad486a90a5dec7fd2fb07caa503be771f91c8) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_cd8012_output.pla", 0, 365, CRC(5ada9306) SHA1(a4140118dd535af45a691832530d55cd86a23510) )

	ROM_REGION( 0x8000, "tms6100", ROMREGION_ERASEFF ) // 4000-7fff = space reserved for cartridge
	ROM_LOAD( "cd62170", 0x0000, 0x4000, CRC(6dc9d072) SHA1(9d2c9ff57c4f8fe69768666ffa41fcac649279ef) )
ROM_END

ROM_START( tntellfr )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "cd8012nl", 0x0000, 0x0800, CRC(3d0fee24) SHA1(8b1b1df03d50ffe8adea59ece212dece5245fe86) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_cd8012_micro.pla", 0, 867, CRC(46d936c8) SHA1(b0aad486a90a5dec7fd2fb07caa503be771f91c8) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_cd8012_output.pla", 0, 365, CRC(5ada9306) SHA1(a4140118dd535af45a691832530d55cd86a23510) )

	ROM_REGION( 0x8000, "tms6100", ROMREGION_ERASEFF ) // 4000-7fff = space reserved for cartridge
	ROM_LOAD( "cd62171", 0x0000, 0x4000, CRC(cc26f7d1) SHA1(2b03e37b3bf3cbeca36980acfc45246dac706b83) )
ROM_END

ROM_START( tntellp )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "us4403965_cd1100", 0x0000, 0x0800, BAD_DUMP CRC(863a1c9e) SHA1(f2f9eb0ae17eedd4ef2b887b34601e75b4f6c720) ) // typed in from patent US4403965/EP0048835A2, may have errors

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_cd8012_micro.pla", 0, 867, CRC(46d936c8) SHA1(b0aad486a90a5dec7fd2fb07caa503be771f91c8) ) // from cd8012, matches patent source code
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_us4403965_output.pla", 0, 365, CRC(66cfb3c3) SHA1(80a05e5d729518e1f35d8f26438f56e80ffbd003) )

	ROM_REGION( 0x8000, "tms6100", ROMREGION_ERASEFF ) // 4000-7fff = space reserved for cartridge
	ROM_LOAD( "cd2610", 0x0000, 0x1000, CRC(6db34e5a) SHA1(10fa5db20fdcba68034058e7194f35c90b9844e6) )
ROM_END


ROM_START( vocaid )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "cd8012nl", 0x0000, 0x0800, CRC(3d0fee24) SHA1(8b1b1df03d50ffe8adea59ece212dece5245fe86) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_cd8012_micro.pla", 0, 867, CRC(46d936c8) SHA1(b0aad486a90a5dec7fd2fb07caa503be771f91c8) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_cd8012_output.pla", 0, 365, CRC(5ada9306) SHA1(a4140118dd535af45a691832530d55cd86a23510) )

	ROM_REGION( 0x8000, "tms6100", ROMREGION_ERASEFF ) // same hw as tntell, but no external slot
	ROM_LOAD( "cd2357", 0x0000, 0x4000, CRC(19c251fa) SHA1(8f8163069f32413379e7e1681ce6a4d0819d4ebc) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME       PARENT    COMPAT  MACHINE   INPUT     CLASS           INIT           COMPANY, FULLNAME, FLAGS
SYST( 1982, snspellc,  0,        0,      snspellc, snspellc, snspellc_state, init_snspellc, "Texas Instruments", "Speak & Spell Compact (US, 1982 version)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
SYST( 1981, snspellca, snspellc, 0,      snspellc, snspellc, snspellc_state, init_snspellc, "Texas Instruments", "Speak & Spell Compact (US, 1981 version)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
SYST( 1982, snwrite,   snspellc, 0,      snwrite,  snwrite,  snspellc_state, init_snspellc, "Texas Instruments", "Speak & Write (UK)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )

SYST( 1982, mathsmag,  0,        0,      mathsmag, mathsmag, snspellc_state, init_snspellc, "Texas Instruments", "Les Maths Magiques (France)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )

SYST( 1981, tntell,    0,        0,      tntell,   tntell,   tntell_state,   init_tntell,   "Texas Instruments", "Touch & Tell (US)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND | MACHINE_REQUIRES_ARTWORK )
SYST( 1980, tntellp,   tntell,   0,      tntell,   tntell,   tntell_state,   init_tntell,   "Texas Instruments", "Touch & Tell (US, patent)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND | MACHINE_REQUIRES_ARTWORK | MACHINE_NOT_WORKING )
SYST( 1981, tntelluk,  tntell,   0,      tntell,   tntell,   tntell_state,   init_tntell,   "Texas Instruments", "Touch & Tell (UK)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND | MACHINE_REQUIRES_ARTWORK )
SYST( 1981, tntellfr,  tntell,   0,      tntell,   tntell,   tntell_state,   init_tntell,   "Texas Instruments", "Le Livre Magique (France)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND | MACHINE_REQUIRES_ARTWORK )

SYST( 1982, vocaid,    0,        0,      vocaid,   tntell,   tntell_state,   empty_init,    "Texas Instruments", "Vocaid", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND | MACHINE_REQUIRES_ARTWORK )
