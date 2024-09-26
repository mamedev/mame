// license:BSD-3-Clause
// copyright-holders:Robbbert
/****************************************************************************************************

PINBALL
Gottlieb System 3
Dot Matrix Display

Setting up must be done before you can play. Press NUM-0, press Left Flipper 3 times, press Right
 Flipper, Press 1 to accept Factory Settings, press 1, quit and restart.

Here are the key codes to enable play:

Game                                     NUM  Start game                           End ball
--------------------------------------------------------------------------------------------------
Super Mario Brothers                     733  (not working)
Cue Ball Wizard                          734  Hit 1, press YZ (black screen)       YZ
Street Fighter II                        735  (not working)
Tee'd Off                                736  Hit 1, press EM (bad gfx)            EM
Gladiators                               737  Hit 1, press LT                      L
Wipe Out                                 738  Hit 1, press MU                      M
Rescue 911                               740  Hit 1, press LT                      L
World Challenge Soccer                   741  Hit 1, press GO                      jiggle GO
Stargate                                 742  (not working)
Shaq Attack                              743  Hit 1, press LM                      M
Freddy A Nightmare on Elm Street         744  Hit 1, press KL                      L
Frank Thomas Big Hurt                    745  Hit 1, press EM                      jiggle EM
Waterworld                               746  Hit 1, press RS                      R
Mario Andretti                           747  Hit 1, press FN                      N
Barb Wire                                748  Hit 1, press GO                      G
Brooks & Dunn                            749  (unfinished prototype)
Super Mario Brothers Mushroom World     N105  (not working)
Strikes n Spares                        N111  (not working, press F3 to see the screen)
machinaZOIS Virtual Training Center           Hit 1, press LM                      M

Status:
- Some machines are playable

ToDo:
- Fix the non-workers
- Only one video plane is emulated, needs to be understood. Various gfx elements missing.
- There's an undumped GAL 16V8-25L on the DMD board (position U8)
- Strikes n Spares: sound, needs 2 DMD screens.
- Brooks & Dunn: roms are missing. Program was not finished, game will never work.

*****************************************************************************************************/

#include "emu.h"
#include "genpin.h"

#include "gottlieb_a.h"

#include "cpu/m6502/m65c02.h"
#include "machine/6522via.h"
#include "machine/input_merger.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class gts3a_state : public genpin_class
{
public:
	gts3a_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_palette(*this, "palette")
		, m_maincpu(*this, "maincpu")
		, m_dmdcpu(*this, "dmdcpu")
		, m_bank1(*this, "bank1")
		, m_crtc(*this, "crtc")
		, m_vram(*this, "vram")
		, m_u4(*this, "u4")
		, m_u5(*this, "u5")
		, m_p7_sound(*this, "p7sound")
		, m_io_keyboard(*this, "X%d", 0U)
		, m_io_outputs(*this, "out%d", 0U)
	{ }

	void p0(machine_config &config);
	void p7(machine_config &config);

	void init_gts3a();

	DECLARE_INPUT_CHANGED_MEMBER(test_inp);

protected:
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;

private:
	void segbank_w(u8 data);
	u8 u4a_r();
	u8 u4b_r();
	void lampret_w(u8);
	void solenoid_w(offs_t, u8);
	void u4b_w(u8 data);
	void u5a_w(u8 data);
	u8 dmd_r();
	void dmd_w(u8 data);
	void nmi_w(int state);
	void crtc_vs(int state);
	MC6845_UPDATE_ROW(crtc_update_row);
	void palette_init(palette_device &palette);
	required_device<palette_device> m_palette;
	void dmd_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	u8 m_row = 0U; // for lamps and switches
	u8 m_segment = 0U;
	u8 m_u4b = 0U;

	required_device<m65c02_device> m_maincpu;
	required_device<m65c02_device> m_dmdcpu;
	required_memory_bank    m_bank1;
	required_device<mc6845_device> m_crtc;
	required_shared_ptr<u8> m_vram;
	required_device<via6522_device> m_u4;
	required_device<via6522_device> m_u5;
	optional_device<gottlieb_sound_p7_device> m_p7_sound;
	required_ioport_array<12> m_io_keyboard;
	output_finder<128> m_io_outputs;   // 32 solenoids + 96 lamps
};


void gts3a_state::mem_map(address_map &map)
{
	map(0x0000, 0x1fff).ram().share("nvram");
	map(0x2000, 0x200f).mirror(0x1f80).m(m_u4, FUNC(via6522_device::map));
	map(0x2010, 0x201f).mirror(0x1f80).m(m_u5, FUNC(via6522_device::map));
	map(0x2020, 0x207f).nopr();
	map(0x2020, 0x2020).mirror(0x1f8c).w(FUNC(gts3a_state::segbank_w));
	map(0x2021, 0x2021).mirror(0x1f8c).lw8(NAME([this] (u8 data) { m_dmdcpu->reset(); } ));   // reset the dmd cpu if it stalls
	map(0x2030, 0x2033).mirror(0x1f8c).w(FUNC(gts3a_state::solenoid_w));
	map(0x2040, 0x2040).mirror(0x1f80).w(FUNC(gts3a_state::lampret_w));
	map(0x2041, 0x207f).mirror(0x1f80).nopw();   // AUX: purpose unknown
	map(0x4000, 0xffff).rom();
}

void gts3a_state::dmd_map(address_map &map)
{
	map(0x0000, 0x0fff).ram();
	map(0x1000, 0x1fff).ram().share("vram");
	map(0x2000, 0x2000).mirror(0x7fe).r(m_crtc, FUNC(mc6845_device::status_r));
	map(0x2001, 0x2001).mirror(0x7fe).r(m_crtc, FUNC(mc6845_device::register_r));
	map(0x2800, 0x2800).mirror(0x7fe).w(m_crtc, FUNC(mc6845_device::address_w));
	map(0x2801, 0x2801).mirror(0x7fe).w(m_crtc, FUNC(mc6845_device::register_w));
	map(0x3000, 0x37ff).r(FUNC(gts3a_state::dmd_r));
	map(0x3800, 0x3fff).w(FUNC(gts3a_state::dmd_w));
	map(0x4000, 0x7fff).bankr("bank1");
	map(0x8000, 0xffff).rom().region("dmdcpu", 0x78000);
}

static INPUT_PORTS_START( gts3a )
	PORT_START("TTS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0) PORT_NAME("Slam Tilt") PORT_CHANGED_MEMBER(DEVICE_SELF, gts3a_state, test_inp, 1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Test") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Tilt") PORT_CODE(KEYCODE_9)

	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_COIN1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_COIN3)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_COIN2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_COIN4)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_START)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_NAME("Tournament")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Front Door")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("INP10")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_NAME("INP11")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("INP12")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("INP13")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("INP14")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("INP15")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("INP16")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("INP17")

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("INP20")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_J) PORT_NAME("INP21")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_K) PORT_NAME("INP22")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("INP23")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("INP24")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("INP25")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("INP26")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_P) PORT_NAME("INP27")

	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("INP30")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("INP31")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("INP32")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("INP33")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("INP34")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("INP35")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("INP36")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_NAME("INP37")

	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("INP40")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_Z) PORT_NAME("INP41")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_COMMA) PORT_NAME("INP42")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_STOP) PORT_NAME("INP43")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_SLASH) PORT_NAME("INP44")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_COLON) PORT_NAME("INP45")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("INP46")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_NAME("INP47")

	PORT_START("X5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("INP50")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("INP51")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("INP52")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_MINUS) PORT_NAME("INP53")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("INP54")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("INP55")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_UP) PORT_NAME("INP56")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_LEFT) PORT_NAME("INP57")

	PORT_START("X6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("INP60")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_DOWN) PORT_NAME("INP61")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_NAME("INP62")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_HOME) PORT_NAME("INP63")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_END) PORT_NAME("INP64")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_PGUP) PORT_NAME("INP65")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_PGDN) PORT_NAME("INP66")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_SPACE) PORT_NAME("INP67")

	PORT_START("X7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_F1) PORT_NAME("INP70")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_F2) PORT_NAME("INP71")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL_PAD) PORT_NAME("INP72")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("INP73")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("INP74")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("INP75")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_ASTERISK) PORT_NAME("INP76")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_SLASH_PAD) PORT_NAME("INP77")

	PORT_START("X8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("INP80")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("INP81")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("INP82")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("INP83")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("INP84")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_NAME("INP85")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_NAME("INP86")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_LCONTROL) PORT_NAME("INP87")

	PORT_START("X9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_RCONTROL) PORT_NAME("INP90")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("INP91")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("INP92")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_LALT) PORT_NAME("INP93")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_RALT) PORT_NAME("INP94")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("INP95")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("INP96")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("INP97")

	PORT_START("X10")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("INPA0")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("INPA1")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("INPA2")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("INPA3")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("INPA4")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("INPA5")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("INPA6")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("INPA7")

	PORT_START("X11")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("INPB0")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("INPB1")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("INPB2")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("INPB3")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("INPB4")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("INPB5")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("INPB6")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("INPB7")
INPUT_PORTS_END

INPUT_CHANGED_MEMBER( gts3a_state::test_inp )
{
	m_u4->write_ca1(newval);
}

// This trampoline needed; WRITELINE("maincpu", m65c02_device, nmi_line) does not work
void gts3a_state::nmi_w(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, (state) ? CLEAR_LINE : HOLD_LINE);
}

void gts3a_state::lampret_w(u8 data)
{
	if (m_row < 12)
		for (u8 i = 0; i < 8; i++)
			m_io_outputs[32+m_row*8+i] = BIT(data, i);
}

void gts3a_state::solenoid_w(offs_t offset, u8 data)
{
	for (u8 i = 0; i < 8; i++)
		m_io_outputs[offset*8+i] = BIT(data, i);
	// Mechanical sounds
	if (offset == 3)
	{
		if (data & 0x18)
			m_samples->start(0, 9);  // outhole
		if (data & 0x20)
			m_samples->start(1, 6);  // knocker
	}
}

void gts3a_state::segbank_w(u8 data)
{
	m_segment = data;
	m_dmdcpu->set_input_line(M65C02_IRQ_LINE, ASSERT_LINE);
}

void gts3a_state::u4b_w(u8 data)
{
	bool clk_bit = BIT(data, 1);
	if ((!BIT(m_u4b, 1)) && clk_bit) // 0->1 is valid
	{
		if (BIT(data, 0))
			m_row = 0;
		else
			m_row++;
	}
	m_u4b = (m_u4b & 0xa0) | (data & 0x47);
}

u8 gts3a_state::u4a_r()
{
	if (m_row < 12)
		return m_io_keyboard[m_row]->read();
	else
		return 0xff;
}

u8 gts3a_state::u4b_r()
{
	return (m_u4b & 0xa0) | (ioport("TTS")->read() & 0x18);
}

void gts3a_state::u5a_w(u8 data)
{
	if (m_p7_sound)
		m_p7_sound->write(data);
}

void gts3a_state::init_gts3a()
{
	u8 *dmd = memregion("dmdcpu")->base();

	m_bank1->configure_entries(0, 32, &dmd[0x0000], 0x4000);
}

u8 gts3a_state::dmd_r()
{
	m_dmdcpu->set_input_line(M65C02_IRQ_LINE, CLEAR_LINE);
	return m_segment;
}

void gts3a_state::dmd_w(u8 data)
{
	m_bank1->set_entry(data & 0x1f);
	m_u4b = (m_u4b & 0x47) | (data & 0x20) | ((data & 0x40) << 1);
}

void gts3a_state::palette_init(palette_device &palette)
{
	palette.set_pen_color(0, rgb_t(0x00, 0x00, 0x00));
	palette.set_pen_color(1, rgb_t(0xf7, 0x00, 0x00));  // change to amber
}

MC6845_UPDATE_ROW( gts3a_state::crtc_update_row )
{
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();
	u16 offset = (BIT(ra, 0) << 4) | (BIT(ma, 7, 7) << 5);

	if ((offset < 0x400) || (offset > 0x5f0))
		return;

	if (y < 32)
	{
		u32 *p = &bitmap.pix(y);
		for (u8 x = 0; x < 16; x++)
		{
			u16 mem = offset+x;
			u8 gfx = m_vram[mem];

			*p++ = palette[BIT(gfx, 7)];
			*p++ = palette[BIT(gfx, 6)];
			*p++ = palette[BIT(gfx, 5)];
			*p++ = palette[BIT(gfx, 4)];
			*p++ = palette[BIT(gfx, 3)];
			*p++ = palette[BIT(gfx, 2)];
			*p++ = palette[BIT(gfx, 1)];
			*p++ = palette[BIT(gfx, 0)];
		}
	}
}

void gts3a_state::crtc_vs(int state)
{
	if (state)
		//m_dmdcpu->set_input_line(INPUT_LINE_NMI, HOLD_LINE);
		m_dmdcpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void gts3a_state::machine_start()
{
	m_io_outputs.resolve();

	save_item(NAME(m_segment));
	save_item(NAME(m_row));
	save_item(NAME(m_u4b));
}

void gts3a_state::machine_reset()
{
	for (u8 i = 0; i < m_io_outputs.size(); i++)
		m_io_outputs[i] = 0;

	m_bank1->set_entry(31);
}

void gts3a_state::p0(machine_config &config)
{
	M65C02(config, m_maincpu, XTAL(4'000'000) / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &gts3a_state::mem_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // 6116LP + DS1210

	M65C02(config, m_dmdcpu, XTAL(3'579'545) / 2);
	m_dmdcpu->set_addrmap(AS_PROGRAM, &gts3a_state::dmd_map);

	// Video
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_screen_update(m_crtc, FUNC(mc6845_device::screen_update));
	screen.set_size(128, 64);
	screen.set_visarea(0, 127, 0, 63);

	PALETTE(config, m_palette, FUNC(gts3a_state::palette_init), 2);

	MC6845(config, m_crtc, XTAL(3'579'545) / 2);
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(1);
	m_crtc->set_update_row_callback(FUNC(gts3a_state::crtc_update_row));
	//m_crtc->out_hsync_callback().set(FUNC(gts3a_state::crtc_hs));
	m_crtc->out_vsync_callback().set(FUNC(gts3a_state::crtc_vs));

	R65C22(config, m_u4, XTAL(4'000'000) / 2);
	m_u4->irq_handler().set("irq", FUNC(input_merger_device::in_w<0>));
	m_u4->readpa_handler().set(FUNC(gts3a_state::u4a_r));
	m_u4->readpb_handler().set(FUNC(gts3a_state::u4b_r));
	m_u4->writepb_handler().set(FUNC(gts3a_state::u4b_w));
	//m_u4->ca2_handler().set(FUNC(gts3a_state::u4ca2_w));
	m_u4->cb2_handler().set(FUNC(gts3a_state::nmi_w));

	R65C22(config, m_u5, XTAL(4'000'000) / 2);
	m_u5->irq_handler().set("irq", FUNC(input_merger_device::in_w<1>));
	m_u5->writepa_handler().set(FUNC(gts3a_state::u5a_w));
	//m_u5->readpb_handler().set(FUNC(gts3a_state::u5b_r));
	//m_u5->writepb_Handler().set(FUNC(gts3a_state::u5b_w));
	//m_u5->ca2_Handler().set(FUNC(gts3a_state::u5ca2_w));
	//m_u5->cb1_Handler().set(FUNC(gts3a_state::u5cb1_w));
	//m_u5->cb2_Handler().set(FUNC(gts3a_state::u5cb2_w));

	INPUT_MERGER_ANY_HIGH(config, "irq").output_handler().set_inputline("maincpu", m65c02_device::IRQ_LINE);

	// Sound
	genpin_audio(config);
	SPEAKER(config, "mono").front_center();
}

void gts3a_state::p7(machine_config &config)
{
	p0(config);
	GOTTLIEB_SOUND_PIN7(config, m_p7_sound).add_route(ALL_OUTPUTS, "mono", 1.00);
}


/*-------------------------------------------------------------------
/ Barb Wire (#748)
/-------------------------------------------------------------------*/
ROM_START(barbwire)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(2e130835) SHA1(f615eaf1c48851d837c57c17c038cc1d0806f6f7))

	ROM_REGION(0x80000, "dmdcpu", ROMREGION_ERASEFF)
	ROM_LOAD("dsprom.bin", 0x00000, 0x80000, CRC(2b9533cd) SHA1(2b154550006e37a9dd1acb0cb832535415a7266b))

	ROM_REGION(0x10000, "p7sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(ebde41b0) SHA1(38a132f815a5270dff58a5e34f5c73701d6e214d))

	ROM_REGION(0x100000, "p7sound:oki", ROMREGION_ERASEFF)
	ROM_LOAD("arom1.bin", 0x00000, 0x80000, CRC(7171bc86) SHA1(d9b1f54d34400490c219ca3ba566cc40cac517d7))
	ROM_LOAD("arom2.bin", 0x80000, 0x40000, CRC(ce83c6c3) SHA1(95a364844525548d28f78d54f9d058728cebf089))
	ROM_RELOAD(0xc0000, 0x40000)

	ROM_REGION(0x10000, "p7sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(7c602a35) SHA1(66dbd7679973683c8346836c28c02ff922d17375))
ROM_END

/*-------------------------------------------------------------------
/ Brooks & Dunn (#749T1)
/-------------------------------------------------------------------*/
ROM_START(brooks) // Rev. T1
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(26cebf07) SHA1(14741e2d216528f176dc35ade856baffab0f99a0))

	ROM_REGION(0x80000, "dmdcpu", ROMREGION_ERASEFF)
	ROM_LOAD("dsprom.bin", 0x00000, 0x80000, NO_DUMP)

	ROM_REGION(0x10000, "p7sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, NO_DUMP)

	ROM_REGION(0x100000, "p7sound:oki", ROMREGION_ERASEFF)
	ROM_LOAD("arom1.bin", 0x00000, 0x80000, NO_DUMP)
	ROM_LOAD("arom2.bin", 0x80000, 0x40000, NO_DUMP)
	ROM_RELOAD(0xc0000, 0x40000)

	ROM_REGION(0x10000, "p7sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, NO_DUMP)
ROM_END

/*-------------------------------------------------------------------
/ Casino Royale (#751) 08/1996
/-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
/ Cue Ball Wizard (#734)
/-------------------------------------------------------------------*/
ROM_START(cueball)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(3437fdd8) SHA1(2a0fc9bc8e3d0c430ce2cf8afad378fc93af609d))

	ROM_REGION(0x80000, "dmdcpu", ROMREGION_ERASEFF)
	ROM_LOAD("dsprom.bin", 0x00000, 0x40000, CRC(3cc7f470) SHA1(6adf8ac2ff93eb19c7b1dbbcf8fff6cd926dc563))
	ROM_RELOAD(0x40000, 0x40000)

	ROM_REGION(0x10000, "p7sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(9fd04109) SHA1(27864fe4e9c248dce6221c9e56861967d089b216))

	ROM_REGION(0x100000, "p7sound:oki", ROMREGION_ERASEFF)
	ROM_LOAD("arom1.bin", 0x00000, 0x40000, CRC(476bb11c) SHA1(ce546df59933cc230a6671dec493bbbe71146dee))
	ROM_RELOAD(0x40000, 0x40000)
	ROM_LOAD("arom2.bin", 0x80000, 0x40000, CRC(23708ad9) SHA1(156fcb19403f9845404af1a4ac4edfd3fcde601d))
	ROM_RELOAD(0xc0000, 0x40000)

	ROM_REGION(0x10000, "p7sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(c22f5cc5) SHA1(a5bfbc1824bc483eecc961851bd411cb0dbcdc4a))
ROM_END

ROM_START(cueballa)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gprom.r2", 0x0000, 0x10000, CRC(171c0a0e) SHA1(e53d32e7cddf47feacf3f5c00651c2216da39b7a))

	ROM_REGION(0x80000, "dmdcpu", ROMREGION_ERASEFF)
	ROM_LOAD("dsprom.r2", 0x00000, 0x40000, CRC(70345a0b) SHA1(38ccea4f367d6ac777119201156b2f35c4d2d379))
	ROM_RELOAD(0x40000, 0x40000)

	ROM_REGION(0x10000, "p7sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(9fd04109) SHA1(27864fe4e9c248dce6221c9e56861967d089b216))

	ROM_REGION(0x100000, "p7sound:oki", ROMREGION_ERASEFF)
	ROM_LOAD("arom1.bin", 0x00000, 0x40000, CRC(476bb11c) SHA1(ce546df59933cc230a6671dec493bbbe71146dee))
	ROM_RELOAD(0x40000, 0x40000)
	ROM_LOAD("arom2.bin", 0x80000, 0x40000, CRC(23708ad9) SHA1(156fcb19403f9845404af1a4ac4edfd3fcde601d))
	ROM_RELOAD(0xc0000, 0x40000)

	ROM_REGION(0x10000, "p7sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(c22f5cc5) SHA1(a5bfbc1824bc483eecc961851bd411cb0dbcdc4a))
ROM_END

ROM_START(cueballb)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gprom.r3", 0x0000, 0x10000, CRC(f2d6e9d8) SHA1(bac7d498876454092607116fd7d46034438c9bfa))

	ROM_REGION(0x80000, "dmdcpu", ROMREGION_ERASEFF)
	ROM_LOAD("dsprom.r2", 0x00000, 0x40000, CRC(70345a0b) SHA1(38ccea4f367d6ac777119201156b2f35c4d2d379))
	ROM_RELOAD( 0x40000, 0x40000)

	ROM_REGION(0x10000, "p7sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(9fd04109) SHA1(27864fe4e9c248dce6221c9e56861967d089b216))

	ROM_REGION(0x100000, "p7sound:oki", ROMREGION_ERASEFF)
	ROM_LOAD("arom1.bin", 0x00000, 0x40000, CRC(476bb11c) SHA1(ce546df59933cc230a6671dec493bbbe71146dee))
	ROM_RELOAD(0x40000, 0x40000)
	ROM_LOAD("arom2.bin", 0x80000, 0x40000, CRC(23708ad9) SHA1(156fcb19403f9845404af1a4ac4edfd3fcde601d))
	ROM_RELOAD(0xc0000, 0x40000)

	ROM_REGION(0x10000, "p7sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(c22f5cc5) SHA1(a5bfbc1824bc483eecc961851bd411cb0dbcdc4a))
ROM_END

ROM_START(cueballc)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(3437fdd8) SHA1(2a0fc9bc8e3d0c430ce2cf8afad378fc93af609d))

	ROM_REGION(0x80000, "dmdcpu", ROMREGION_ERASEFF)
	ROM_LOAD("dsprom0.bin", 0x00000, 0x40000, CRC(3756efeb) SHA1(2bf84a840ad134666c76d49b89c2de76c0420af8))
	ROM_RELOAD(0x40000, 0x40000)

	ROM_REGION(0x10000, "p7sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(9fd04109) SHA1(27864fe4e9c248dce6221c9e56861967d089b216))

	ROM_REGION(0x100000, "p7sound:oki", ROMREGION_ERASEFF)
	ROM_LOAD("arom1.bin", 0x00000, 0x40000, CRC(476bb11c) SHA1(ce546df59933cc230a6671dec493bbbe71146dee))
	ROM_RELOAD(0x40000, 0x40000)
	ROM_LOAD("arom2.bin", 0x80000, 0x40000, CRC(23708ad9) SHA1(156fcb19403f9845404af1a4ac4edfd3fcde601d))
	ROM_RELOAD(0xc0000, 0x40000)

	ROM_REGION(0x10000, "p7sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(c22f5cc5) SHA1(a5bfbc1824bc483eecc961851bd411cb0dbcdc4a))
ROM_END

/*-------------------------------------------------------------------
/ Frank Thomas' Big Hurt (#745)
/-------------------------------------------------------------------*/
ROM_START(bighurt)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(92ce9353) SHA1(479edb2e39fa610eb2854b028d3a039473e52eba))

	ROM_REGION(0x80000, "dmdcpu", ROMREGION_ERASEFF)
	ROM_LOAD("dsprom.bin", 0x00000, 0x80000, CRC(bbe96c5e) SHA1(4aaac8d88e739ccb22a7d87a820b14b6d40d3ff8))

	ROM_REGION(0x10000, "p7sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(d472210c) SHA1(4607e6f928cb9a5f41175210ba0427b6cd50fb83))

	ROM_REGION(0x100000, "p7sound:oki", ROMREGION_ERASEFF)
	ROM_LOAD("arom1.bin", 0x00000, 0x80000, CRC(b3def376) SHA1(94553052cfe80774affebd5b0f99512055552786))
	ROM_LOAD("arom2.bin", 0x80000, 0x40000, CRC(59789e66) SHA1(08b7f82f83c53f15cafefb009ab9833457c088cc))
	ROM_RELOAD(0xc0000, 0x40000)

	ROM_REGION(0x10000, "p7sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(c58941ed) SHA1(3b3545b1e8986b06238576a0cef69d3e3a59a325))
ROM_END

/*-------------------------------------------------------------------
/ Freddy: A Nightmare on Elm Street (#744)
/-------------------------------------------------------------------*/
ROM_START(freddy) // Rev. 4
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gprom4.bin", 0x0000, 0x10000, CRC(cd8b46ea) SHA1(3151a9f7b514314dc4989232e1eda444555242c0))

	ROM_REGION(0x80000, "dmdcpu", ROMREGION_ERASEFF)
	ROM_LOAD("dsprom.bin", 0x00000, 0x80000, CRC(d78d0fa3) SHA1(132c05e71cf5ad53184f044873fb3dd71f6da15f))

	ROM_REGION(0x10000, "p7sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(d472210c) SHA1(4607e6f928cb9a5f41175210ba0427b6cd50fb83))

	ROM_REGION(0x100000, "p7sound:oki", ROMREGION_ERASEFF)
	ROM_LOAD("arom1.bin", 0x00000, 0x80000, CRC(6bec0567) SHA1(510c0e5a5af7573761a69bad5ab36f0019767c48))
	ROM_LOAD("arom2.bin", 0x80000, 0x40000, CRC(f0e9284d) SHA1(6ffe8286e27b0eecab9620ca613e3d72bb7f77ce))
	ROM_RELOAD(0xc0000, 0x40000)

	ROM_REGION(0x10000, "p7sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(4a748665) SHA1(9f08b6d0731390c306194808226d2e99fbe9122d))
ROM_END

ROM_START(freddy3) // Rev. 3
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(f0a6f3e6) SHA1(ad9af12260b8adc639fa00de49366b1016df49ed))

	ROM_REGION(0x80000, "dmdcpu", ROMREGION_ERASEFF)
	ROM_LOAD("dsprom.bin", 0x00000, 0x80000, CRC(d78d0fa3) SHA1(132c05e71cf5ad53184f044873fb3dd71f6da15f))

	ROM_REGION(0x10000, "p7sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(d472210c) SHA1(4607e6f928cb9a5f41175210ba0427b6cd50fb83))

	ROM_REGION(0x100000, "p7sound:oki", ROMREGION_ERASEFF)
	ROM_LOAD("arom1.bin", 0x00000, 0x80000, CRC(6bec0567) SHA1(510c0e5a5af7573761a69bad5ab36f0019767c48))
	ROM_LOAD("arom2.bin", 0x80000, 0x40000, CRC(f0e9284d) SHA1(6ffe8286e27b0eecab9620ca613e3d72bb7f77ce))
	ROM_RELOAD(0xc0000, 0x40000)

	ROM_REGION(0x10000, "p7sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(4a748665) SHA1(9f08b6d0731390c306194808226d2e99fbe9122d))
ROM_END

/*-------------------------------------------------------------------
/ Gladiators (#737)
/-------------------------------------------------------------------*/
ROM_START(gladiatp)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(40386cf5) SHA1(3139e3707971a708ad98c735deec7e4ee7bb36cd))

	ROM_REGION(0x80000, "dmdcpu", ROMREGION_ERASEFF)
	ROM_LOAD("dsprom.bin", 0x00000, 0x80000, CRC(fdc8baed) SHA1(d8ad96665cd9d8b2a6ce94653753c692384685ff))

	ROM_REGION(0x10000, "p7sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(60779d60) SHA1(2fa09c65ddd6cf638382229062a48163e8972136))

	ROM_REGION(0x100000, "p7sound:oki", ROMREGION_ERASEFF)
	ROM_LOAD("arom1.bin", 0x00000, 0x40000, CRC(85cbdda7) SHA1(4eaea8866cb281034e30f425e864419fdb58081f))
	ROM_RELOAD(0x40000, 0x40000)
	ROM_LOAD("arom2.bin", 0x80000, 0x40000, CRC(da2c1073) SHA1(faf58099e78dffdce5c15f393ffa3707ec80dd51))
	ROM_RELOAD(0xc0000, 0x40000)

	ROM_REGION(0x10000, "p7sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(c5b72153) SHA1(c5d94f3fa815fc33952107c3a3ad698c3c443ce3))
ROM_END

/*-------------------------------------------------------------------
/ Mario Andretti (#747)
/-------------------------------------------------------------------*/
ROM_START(andretti) // Rev. T4
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gpromt4.bin", 0x0000, 0x10000, CRC(c6f6a23b) SHA1(01ea23a830be1e86f5ecd27d6d56c1c6d5ff3176))

	ROM_REGION(0x80000, "dmdcpu", ROMREGION_ERASEFF)
	ROM_LOAD("dsprom.bin", 0x00000, 0x80000, CRC(1f70baae) SHA1(cf07bb057093b2bd18e6ee45009245ea62094e53))

	ROM_REGION(0x10000, "p7sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(d472210c) SHA1(4607e6f928cb9a5f41175210ba0427b6cd50fb83))

	ROM_REGION(0x100000, "p7sound:oki", ROMREGION_ERASEFF)
	ROM_LOAD("arom1.bin", 0x00000, 0x80000, CRC(918c3270) SHA1(aa57d3bfba01e701b02ca7e4f0946144cfb7d4b1))
	ROM_LOAD("arom2.bin", 0x80000, 0x40000, CRC(3c61a2f7) SHA1(65cfb5d1261a1b0c219e1786b6635d7b0a188040))
	ROM_RELOAD(0xc0000, 0x40000)

	ROM_REGION(0x10000, "p7sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(4ffb15b0) SHA1(de4e9b2ccca865deb2595320015a149246795260))
ROM_END

ROM_START(andretti0) // No rev.
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(cffa788d) SHA1(84646880b09dce73a42a6d87666897f6bd74a8f9))

	ROM_REGION(0x80000, "dmdcpu", ROMREGION_ERASEFF)
	ROM_LOAD("dsprom.bin", 0x00000, 0x80000, CRC(1f70baae) SHA1(cf07bb057093b2bd18e6ee45009245ea62094e53))

	ROM_REGION(0x10000, "p7sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(d472210c) SHA1(4607e6f928cb9a5f41175210ba0427b6cd50fb83))

	ROM_REGION(0x100000, "p7sound:oki", ROMREGION_ERASEFF)
	ROM_LOAD("arom1.bin", 0x00000, 0x80000, CRC(918c3270) SHA1(aa57d3bfba01e701b02ca7e4f0946144cfb7d4b1))
	ROM_LOAD("arom2.bin", 0x80000, 0x40000, CRC(3c61a2f7) SHA1(65cfb5d1261a1b0c219e1786b6635d7b0a188040))
	ROM_RELOAD(0xc0000, 0x40000)

	ROM_REGION(0x10000, "p7sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(4ffb15b0) SHA1(de4e9b2ccca865deb2595320015a149246795260))
ROM_END


/*-------------------------------------------------------------------
/ Rescue 911 (#740)
/-------------------------------------------------------------------*/
ROM_START(rescu911)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(943a7597) SHA1(dcf4151727efa64e8740202b68fc8e76098ff8dd))

	ROM_REGION(0x80000, "dmdcpu", ROMREGION_ERASEFF)
	ROM_LOAD("dsprom.bin", 0x00000, 0x80000, CRC(9657ebd5) SHA1(b716daa71f8ec4332bf338f1f976425b6ec781ab))

	ROM_REGION(0x10000, "p7sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(034c6bc3) SHA1(c483690a6e4ce533b8939e27547175c301316172))

	ROM_REGION(0x100000, "p7sound:oki", ROMREGION_ERASEFF)
	ROM_LOAD("arom1.bin", 0x00000, 0x80000, CRC(f6daa16c) SHA1(be132072b27a94f61653de0a22eecc8b90db3077))
	ROM_LOAD("arom2.bin", 0x80000, 0x40000, CRC(59374104) SHA1(8ad7f5f0109771dd5cebe13e80f8e1a9420f4447))
	ROM_RELOAD(0xc0000, 0x40000)

	ROM_REGION(0x10000, "p7sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(14f86b56) SHA1(2364c284412eba719f88d50dcf47d5482365dbf3))
ROM_END

/*-------------------------------------------------------------------
/ Shaq Attaq (#743)
/-------------------------------------------------------------------*/
ROM_START(shaqattq) // Rev. 5
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(7a967fd1) SHA1(c06e2aad9452150d92cfd3ba37b8e4a932cf4324))

	ROM_REGION(0x80000, "dmdcpu", ROMREGION_ERASEFF)
	ROM_LOAD("dsprom.bin", 0x00000, 0x80000, CRC(d6cca842) SHA1(0498ab558d252e42dee9636e6736d159c7d06275))

	ROM_REGION(0x10000, "p7sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(16a03261) SHA1(25f5a3d32d2ec80766381106445fd624360fea78))

	ROM_REGION(0x100000, "p7sound:oki", ROMREGION_ERASEFF)
	ROM_LOAD("arom1.bin", 0x00000, 0x80000, CRC(019014ec) SHA1(808a8c3154fca6218fe991b46a2525926d8e51f9))
	ROM_LOAD("arom2.bin", 0x80000, 0x40000, CRC(cc5f157d) SHA1(81c3dadff1bbf37a1f091ea77d9061879be7d99c))
	ROM_RELOAD(0xc0000, 0x40000)

	ROM_REGION(0x10000, "p7sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(e81e2928) SHA1(4bfe57efa99bb762e4de6c7e88e79b8c5ff57626))
ROM_END

ROM_START(shaqattq2) // Rev. 2
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gprom2.bin", 0x0000, 0x10000, CRC(494b5cec) SHA1(91511eb9f8b0182ffeff5301fb5bcf4ee9056b3f))

	ROM_REGION(0x80000, "dmdcpu", ROMREGION_ERASEFF)
	ROM_LOAD("dsprom.bin", 0x00000, 0x80000, CRC(d6cca842) SHA1(0498ab558d252e42dee9636e6736d159c7d06275))

	ROM_REGION(0x10000, "p7sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(16a03261) SHA1(25f5a3d32d2ec80766381106445fd624360fea78))

	ROM_REGION(0x100000, "p7sound:oki", ROMREGION_ERASEFF)
	ROM_LOAD("arom1.bin", 0x00000, 0x80000, CRC(019014ec) SHA1(808a8c3154fca6218fe991b46a2525926d8e51f9))
	ROM_LOAD("arom2.bin", 0x80000, 0x40000, CRC(cc5f157d) SHA1(81c3dadff1bbf37a1f091ea77d9061879be7d99c))
	ROM_RELOAD(0xc0000, 0x40000)

	ROM_REGION(0x10000, "p7sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(e81e2928) SHA1(4bfe57efa99bb762e4de6c7e88e79b8c5ff57626))
ROM_END

/*-------------------------------------------------------------------
/ machinaZOIS Virtual Training Center (hack of Shaq Attack)
/-------------------------------------------------------------------*/
ROM_START(mac_zois)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gprommz.bin", 0x0000, 0x10000, CRC(7a967fd1) SHA1(c06e2aad9452150d92cfd3ba37b8e4a932cf4324))

	ROM_REGION(0x80000, "dmdcpu", ROMREGION_ERASEFF)
	ROM_LOAD("dsprom.bin", 0x00000, 0x80000, CRC(3b63f9c6) SHA1(b06ea3b8f7d3c4b22a8bbc687698654366c35f22) )

	ROM_REGION(0x10000, "p7sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(16a03261) SHA1(25f5a3d32d2ec80766381106445fd624360fea78))

	ROM_REGION(0x100000, "p7sound:oki", ROMREGION_ERASEFF)
	ROM_LOAD("arom1mz.bin", 0x00000, 0x80000, CRC(68ceeb43) SHA1(debe5a0683b1806c9813ba89a6438afb3eecb188) )
	ROM_LOAD("arom2mz.bin", 0x80000, 0x40000, CRC(7dabc8ca) SHA1(ca6dc59891222f8534b0a2de8cd29c52e5b33efc) )
	ROM_RELOAD(0xc0000, 0x40000)

	ROM_REGION(0x10000, "p7sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(e81e2928) SHA1(4bfe57efa99bb762e4de6c7e88e79b8c5ff57626))
ROM_END

/*-------------------------------------------------------------------
/ Stargate (#742)
/-------------------------------------------------------------------*/
ROM_START(stargatp) // Rev. 5
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("stgtcpu5.512", 0x0000, 0x10000, CRC(c0579d86) SHA1(ba7ea85ccf407ec72d19e15b34b96a7ca95bf893))

	ROM_REGION(0x80000, "dmdcpu", ROMREGION_ERASEFF)
	ROM_LOAD("dsprom3.bin", 0x00000, 0x80000, CRC(db483524) SHA1(ea14e8b04c32fc403ce2ff060caed5562104a862))

	ROM_REGION(0x10000, "p7sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(781b2b27) SHA1(06decd22b9064ee4859618a043055e0b3e3b9e04))

	ROM_REGION(0x100000, "p7sound:oki", ROMREGION_ERASEFF)
	ROM_LOAD("arom1.bin", 0x00000, 0x80000, CRC(a0f62605) SHA1(8c39452367150f66271371ab02be2f5a812cb954))
	ROM_RELOAD(0x80000, 0x80000)

	ROM_REGION(0x10000, "p7sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(53123fd4) SHA1(77fd183a10eea2e04a07edf9da14ef7aadb65f91))
ROM_END

ROM_START(stargatp4) // Rev. 4
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gprom4.bin", 0x0000, 0x10000, CRC(7b8f6920) SHA1(f354593e13c30e15c25580387ef2eb9b23622c89))

	ROM_REGION(0x80000, "dmdcpu", ROMREGION_ERASEFF)
	ROM_LOAD("dsprom3.bin", 0x00000, 0x80000, CRC(db483524) SHA1(ea14e8b04c32fc403ce2ff060caed5562104a862))

	ROM_REGION(0x10000, "p7sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(781b2b27) SHA1(06decd22b9064ee4859618a043055e0b3e3b9e04))

	ROM_REGION(0x100000, "p7sound:oki", ROMREGION_ERASEFF)
	ROM_LOAD("arom1.bin", 0x00000, 0x80000, CRC(a0f62605) SHA1(8c39452367150f66271371ab02be2f5a812cb954))
	ROM_RELOAD(0x80000, 0x80000)

	ROM_REGION(0x10000, "p7sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(53123fd4) SHA1(77fd183a10eea2e04a07edf9da14ef7aadb65f91))
ROM_END

ROM_START(stargatp3) // Rev. 3
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gprom3.bin", 0x0000, 0x10000, CRC(83f0a2e7) SHA1(5d247a3329a946449e4b333b18c13e351caa230b))

	ROM_REGION(0x80000, "dmdcpu", ROMREGION_ERASEFF)
	ROM_LOAD("dsprom3.bin", 0x00000, 0x80000, CRC(db483524) SHA1(ea14e8b04c32fc403ce2ff060caed5562104a862))

	ROM_REGION(0x10000, "p7sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(781b2b27) SHA1(06decd22b9064ee4859618a043055e0b3e3b9e04))

	ROM_REGION(0x100000, "p7sound:oki", ROMREGION_ERASEFF)
	ROM_LOAD("arom1.bin", 0x00000, 0x80000, CRC(a0f62605) SHA1(8c39452367150f66271371ab02be2f5a812cb954))
	ROM_RELOAD(0x80000, 0x80000)

	ROM_REGION(0x10000, "p7sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(53123fd4) SHA1(77fd183a10eea2e04a07edf9da14ef7aadb65f91))
ROM_END

ROM_START(stargatp2) // Rev. 2
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gprom2.bin", 0x0000, 0x10000, CRC(862920f8) SHA1(cde77e7937782f2f9fe4b7fe27b56206d6f26f63))

	ROM_REGION(0x80000, "dmdcpu", ROMREGION_ERASEFF)
	ROM_LOAD("dsprom2.bin", 0x00000, 0x80000, CRC(d0205e03) SHA1(d8dea47f0fa0e46e2bd107a1f57121372fdef0d8))

	ROM_REGION(0x10000, "p7sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(781b2b27) SHA1(06decd22b9064ee4859618a043055e0b3e3b9e04))

	ROM_REGION(0x100000, "p7sound:oki", ROMREGION_ERASEFF)
	ROM_LOAD("arom1.bin", 0x00000, 0x80000, CRC(a0f62605) SHA1(8c39452367150f66271371ab02be2f5a812cb954))
	ROM_RELOAD(0x80000, 0x80000)

	ROM_REGION(0x10000, "p7sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(53123fd4) SHA1(77fd183a10eea2e04a07edf9da14ef7aadb65f91))
ROM_END

ROM_START(stargatp1) // Rev. 1
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gprom1.bin", 0x0000, 0x10000, CRC(567ecd88) SHA1(2dc4bfbc971cc873af6ec32e5ddbbed001d2e1d2))

	ROM_REGION(0x80000, "dmdcpu", ROMREGION_ERASEFF)
	ROM_LOAD("dsprom1.bin", 0x00000, 0x80000, CRC(91c1b01a) SHA1(96eec2e9e52c8278c102f433a554327d420fe131))

	ROM_REGION(0x10000, "p7sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(781b2b27) SHA1(06decd22b9064ee4859618a043055e0b3e3b9e04))

	ROM_REGION(0x100000, "p7sound:oki", ROMREGION_ERASEFF)
	ROM_LOAD("arom1.bin", 0x00000, 0x80000, CRC(a0f62605) SHA1(8c39452367150f66271371ab02be2f5a812cb954))
	ROM_RELOAD(0x80000, 0x80000)

	ROM_REGION(0x10000, "p7sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(53123fd4) SHA1(77fd183a10eea2e04a07edf9da14ef7aadb65f91))
ROM_END

ROM_START(stargatp0) // No rev.
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(837e4354) SHA1(b7d1e270309b3d7965dafeec7b81d2dd41e5700c))

	ROM_REGION(0x80000, "dmdcpu", ROMREGION_ERASEFF)
	ROM_LOAD("dsprom.bin", 0x00000, 0x80000, CRC(17b89750) SHA1(927702f88013945cb9f2ea8389800b925182c347))

	ROM_REGION(0x10000, "p7sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(781b2b27) SHA1(06decd22b9064ee4859618a043055e0b3e3b9e04))

	ROM_REGION(0x100000, "p7sound:oki", ROMREGION_ERASEFF)
	ROM_LOAD("arom1.bin", 0x00000, 0x80000, CRC(a0f62605) SHA1(8c39452367150f66271371ab02be2f5a812cb954))
	ROM_RELOAD(0x80000, 0x80000)

	ROM_REGION(0x10000, "p7sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(53123fd4) SHA1(77fd183a10eea2e04a07edf9da14ef7aadb65f91))
ROM_END

/*-------------------------------------------------------------------
/ Street Fighter II (#735)
/-------------------------------------------------------------------*/
ROM_START(sfightii)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(299ad173) SHA1(95cca8c22cfabc55175a49b0439fc7858bdec1bd))

	ROM_REGION(0x80000, "dmdcpu", ROMREGION_ERASEFF)
	ROM_LOAD("dsprom.bin", 0x00000, 0x80000, CRC(e565e5e9) SHA1(c37abf28918feb38bbad6ebb610023d52ba96957))

	ROM_REGION(0x10000, "p7sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(f5c13e80) SHA1(4dd3d35c25e3cb92d6000e463ddce564e112c108))

	ROM_REGION(0x100000, "p7sound:oki", ROMREGION_ERASEFF)
	ROM_LOAD("arom1.bin", 0x00000, 0x40000, CRC(8518ff55) SHA1(b31678aa7c1b1240becf0ae0af05b30f7df4a491))
	ROM_RELOAD(0x40000, 0x40000)
	ROM_LOAD("arom2.bin", 0x80000, 0x40000, CRC(85a304d9) SHA1(71141dea44e4117cad66089c7a0806de1be1a96a))
	ROM_RELOAD(0xc0000, 0x40000)

	ROM_REGION(0x10000, "p7sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(9009f461) SHA1(589d94a9ae2269175be9f71b1946107bb85620ee))
ROM_END

ROM_START(sfightiia)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gprom1.bin", 0x0000, 0x10000, CRC(5b42c332) SHA1(958e9fe09e587038dc282fc2f276608ef3744b1d))

	ROM_REGION(0x80000, "dmdcpu", ROMREGION_ERASEFF)
	ROM_LOAD("dsprom2.bin", 0x00000, 0x80000, CRC(80eb7513) SHA1(d13d44545c7b177e27b596bac6eba173b34a017b))

	ROM_REGION(0x10000, "p7sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(f5c13e80) SHA1(4dd3d35c25e3cb92d6000e463ddce564e112c108))

	ROM_REGION(0x100000, "p7sound:oki", ROMREGION_ERASEFF)
	ROM_LOAD("arom1.bin", 0x00000, 0x40000, CRC(8518ff55) SHA1(b31678aa7c1b1240becf0ae0af05b30f7df4a491))
	ROM_RELOAD(0x40000, 0x40000)
	ROM_LOAD("arom2.bin", 0x80000, 0x40000, CRC(85a304d9) SHA1(71141dea44e4117cad66089c7a0806de1be1a96a))
	ROM_RELOAD(0xc0000, 0x40000)

	ROM_REGION(0x10000, "p7sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(9009f461) SHA1(589d94a9ae2269175be9f71b1946107bb85620ee))
ROM_END

ROM_START(sfightiib)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gprom2.bin", 0x0000, 0x10000, CRC(26d24c06) SHA1(c706bd6b2bd5b9ad6a6fb69178169977a54107b5))

	ROM_REGION(0x80000, "dmdcpu", ROMREGION_ERASEFF)
	ROM_LOAD("dsprom2.bin", 0x00000, 0x80000, CRC(80eb7513) SHA1(d13d44545c7b177e27b596bac6eba173b34a017b))

	ROM_REGION(0x10000, "p7sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(f5c13e80) SHA1(4dd3d35c25e3cb92d6000e463ddce564e112c108))

	ROM_REGION(0x100000, "p7sound:oki", ROMREGION_ERASEFF)
	ROM_LOAD("arom1.bin", 0x00000, 0x40000, CRC(8518ff55) SHA1(b31678aa7c1b1240becf0ae0af05b30f7df4a491))
	ROM_RELOAD(0x40000, 0x40000)
	ROM_LOAD("arom2.bin", 0x80000, 0x40000, CRC(85a304d9) SHA1(71141dea44e4117cad66089c7a0806de1be1a96a))
	ROM_RELOAD(0xc0000, 0x40000)

	ROM_REGION(0x10000, "p7sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(9009f461) SHA1(589d94a9ae2269175be9f71b1946107bb85620ee))
ROM_END

/*-------------------------------------------------------------------
/ Strikes n' Spares (#N111)
/-------------------------------------------------------------------*/
ROM_START(snspares) // Rev. 6
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(9e018496) SHA1(a4995f153ba2179198cfc56b7011707328e4ec89))

	ROM_REGION(0x80000, "dmdcpu", ROMREGION_ERASEFF)
	ROM_LOAD("dsprom.bin", 0x00000, 0x40000, CRC(5c901899) SHA1(d106561b2e382afdb16e938072c9c8f1d1ccdae6))
	ROM_RELOAD(0x40000, 0x40000)

	ROM_REGION(0x10000, "p7sound:speechcpu", ROMREGION_ERASEFF)

	ROM_REGION(0x80000, "dmdcpu2", ROMREGION_ERASEFF)
	ROM_LOAD("dsprom.bin", 0x00000, 0x40000, CRC(5c901899) SHA1(d106561b2e382afdb16e938072c9c8f1d1ccdae6))
	ROM_RELOAD(0x40000, 0x40000)

	ROM_REGION(0x100000, "p7sound:oki", ROMREGION_ERASEFF)
	ROM_LOAD("arom1.bin", 0x00000, 0x80000, CRC(e248574a) SHA1(d2bdc2b9a330bb81556d25d464f617e0934995eb))
ROM_END

ROM_START(snspares2) // Rev. 2
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gprom2.bin", 0x0000, 0x10000, CRC(79906dfc) SHA1(1efb68dd391f79e6f8ad5a588145d6ad7b36743c))

	ROM_REGION(0x80000, "dmdcpu", ROMREGION_ERASEFF)
	ROM_LOAD("dsprom.bin", 0x00000, 0x40000, CRC(5c901899) SHA1(d106561b2e382afdb16e938072c9c8f1d1ccdae6))
	ROM_RELOAD(0x40000, 0x40000)

	ROM_REGION(0x10000, "p7sound:speechcpu", ROMREGION_ERASEFF)

	ROM_REGION(0x80000, "dmdcpu2", ROMREGION_ERASEFF)
	ROM_LOAD("dsprom.bin", 0x00000, 0x40000, CRC(5c901899) SHA1(d106561b2e382afdb16e938072c9c8f1d1ccdae6))
	ROM_RELOAD(0x40000, 0x40000)

	ROM_REGION(0x100000, "p7sound:oki", ROMREGION_ERASEFF)
	ROM_LOAD("arom1.bin", 0x00000, 0x80000, CRC(e248574a) SHA1(d2bdc2b9a330bb81556d25d464f617e0934995eb))
ROM_END

ROM_START(snspares1) // Rev. 1
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gprom1.bin", 0x0000, 0x10000, CRC(590393f4) SHA1(f52400c620e510253abd1c0719050b9bb09be942))

	ROM_REGION(0x80000, "dmdcpu", ROMREGION_ERASEFF)
	ROM_LOAD("dsprom.bin", 0x00000, 0x40000, CRC(5c901899) SHA1(d106561b2e382afdb16e938072c9c8f1d1ccdae6))
	ROM_RELOAD(0x40000, 0x40000)

	ROM_REGION(0x10000, "p7sound:speechcpu", ROMREGION_ERASEFF)

	ROM_REGION(0x80000, "dmdcpu2", ROMREGION_ERASEFF)
	ROM_LOAD("dsprom.bin", 0x00000, 0x40000, CRC(5c901899) SHA1(d106561b2e382afdb16e938072c9c8f1d1ccdae6))
	ROM_RELOAD(0x40000, 0x40000)

	ROM_REGION(0x100000, "p7sound:oki", ROMREGION_ERASEFF)
	ROM_LOAD("arom1.bin", 0x00000, 0x80000, CRC(e248574a) SHA1(d2bdc2b9a330bb81556d25d464f617e0934995eb))
ROM_END

/*-------------------------------------------------------------------
/ Super Mario Brothers (#733) - Only one dsprom dump seems to work?
/-------------------------------------------------------------------*/
ROM_START(smbp)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(fa1f6e52) SHA1(d7ade0e129cb399494967e025d25614bf1650db7))

	ROM_REGION(0x80000, "dmdcpu", ROMREGION_ERASEFF)
	ROM_LOAD("dsprom2.bin", 0x00000, 0x40000, CRC(181e8234) SHA1(9b22681f61cae401269a88c3cfd783d683390877))
	ROM_RELOAD( 0x40000, 0x40000)

	ROM_REGION(0x10000, "p7sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(6f1d0a3e) SHA1(c7f665d79b9073f28f90debde16cafa9ab57a47c))

	ROM_REGION(0x100000, "p7sound:oki", ROMREGION_ERASEFF)
	ROM_LOAD("arom1.bin", 0x00000, 0x40000, CRC(e9cef116) SHA1(5f710bc24e1a168f296a22417aebecbde3bfaa5c))
	ROM_RELOAD(0x40000, 0x40000)
	ROM_LOAD("arom2.bin", 0x80000, 0x40000, CRC(0acdfd49) SHA1(0baabd32b546842bc5c76a61b509b558677b50f9))
	ROM_RELOAD(0xc0000, 0x40000)

	ROM_REGION(0x10000, "p7sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(e1379106) SHA1(10c46bad7cbae528716c5ba0709bb1fd3574a0a8))
ROM_END

ROM_START(smbpa)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gprom1.bin", 0x0000, 0x10000, CRC(1d8c4df8) SHA1(e301bf3b2a8ed6ef902fe15b890b4c06c4606aa9))

	ROM_REGION(0x80000, "dmdcpu", ROMREGION_ERASEFF)
	ROM_LOAD("dsprom2.bin", 0x00000, 0x40000, CRC(181e8234) SHA1(9b22681f61cae401269a88c3cfd783d683390877))
	ROM_RELOAD(0x40000, 0x40000)

	ROM_REGION(0x10000, "p7sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(6f1d0a3e) SHA1(c7f665d79b9073f28f90debde16cafa9ab57a47c))

	ROM_REGION(0x100000, "p7sound:oki", ROMREGION_ERASEFF)
	ROM_LOAD("arom1.bin", 0x00000, 0x40000, CRC(e9cef116) SHA1(5f710bc24e1a168f296a22417aebecbde3bfaa5c))
	ROM_RELOAD(0x40000, 0x40000)
	ROM_LOAD("arom2.bin", 0x80000, 0x40000, CRC(0acdfd49) SHA1(0baabd32b546842bc5c76a61b509b558677b50f9))
	ROM_RELOAD(0xc0000, 0x40000)

	ROM_REGION(0x10000, "p7sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(e1379106) SHA1(10c46bad7cbae528716c5ba0709bb1fd3574a0a8))
ROM_END

ROM_START(smbpb)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gprom2.bin", 0x0000, 0x10000, CRC(5b0f44c4) SHA1(ca9b0cd82c75612c85c956497c8f9c12992f6ad5))

	ROM_REGION(0x80000, "dmdcpu", ROMREGION_ERASEFF)
	ROM_LOAD("dsprom2.bin", 0x00000, 0x40000, CRC(181e8234) SHA1(9b22681f61cae401269a88c3cfd783d683390877))
	ROM_RELOAD(0x40000, 0x40000)

	ROM_REGION(0x10000, "p7sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(6f1d0a3e) SHA1(c7f665d79b9073f28f90debde16cafa9ab57a47c))

	ROM_REGION(0x100000, "p7sound:oki", ROMREGION_ERASEFF)
	ROM_LOAD("arom1.bin", 0x00000, 0x40000, CRC(e9cef116) SHA1(5f710bc24e1a168f296a22417aebecbde3bfaa5c))
	ROM_RELOAD(0x40000, 0x40000)
	ROM_LOAD("arom2.bin", 0x80000, 0x40000, CRC(0acdfd49) SHA1(0baabd32b546842bc5c76a61b509b558677b50f9))
	ROM_RELOAD(0xc0000, 0x40000)

	ROM_REGION(0x10000, "p7sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(e1379106) SHA1(10c46bad7cbae528716c5ba0709bb1fd3574a0a8))
ROM_END

ROM_START(smbpc)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gprom3.bin", 0x0000, 0x10000, CRC(5a40822c) SHA1(a87ec6307f848483c76141e47fd67e4549f9c9d3))

	ROM_REGION(0x80000, "dmdcpu", ROMREGION_ERASEFF)
	ROM_LOAD("dsprom2.bin", 0x00000, 0x40000, CRC(181e8234) SHA1(9b22681f61cae401269a88c3cfd783d683390877))
	ROM_RELOAD(0x40000, 0x40000)

	ROM_REGION(0x10000, "p7sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(6f1d0a3e) SHA1(c7f665d79b9073f28f90debde16cafa9ab57a47c))

	ROM_REGION(0x100000, "p7sound:oki", ROMREGION_ERASEFF)
	ROM_LOAD("arom1.bin", 0x00000, 0x40000, CRC(e9cef116) SHA1(5f710bc24e1a168f296a22417aebecbde3bfaa5c))
	ROM_RELOAD(0x40000, 0x40000)
	ROM_LOAD("arom2.bin", 0x80000, 0x40000, CRC(0acdfd49) SHA1(0baabd32b546842bc5c76a61b509b558677b50f9))
	ROM_RELOAD(0xc0000, 0x40000)

	ROM_REGION(0x10000, "p7sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(e1379106) SHA1(10c46bad7cbae528716c5ba0709bb1fd3574a0a8))
ROM_END

/*-------------------------------------------------------------------
/ Super Mario Brothers Mushroom World (N105)
/-------------------------------------------------------------------*/
ROM_START(smbmush)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(45f6d0cc) SHA1(a73c71ab64aee293ae46e65c34d70840296778d4))

	ROM_REGION(0x80000, "dmdcpu", ROMREGION_ERASEFF)
	ROM_LOAD("dsprom.bin", 0x00000, 0x40000, CRC(dda6c8be) SHA1(b64f73b81afe973674f9543a704b498e31d26c12))
	ROM_RELOAD(0x40000, 0x40000)

	ROM_REGION(0x10000, "p7sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(6f04a0ac) SHA1(53bbc182a3bd635ad18504692a4454994daef7ef))

	ROM_REGION(0x100000, "p7sound:oki", ROMREGION_ERASEFF)
	ROM_LOAD("arom1.bin", 0x00000, 0x40000, CRC(edce7951) SHA1(4a80d6367a5bebf9fee181456280619aa64b441f))
	ROM_RELOAD(0x40000, 0x40000)
	ROM_LOAD("arom2.bin", 0x80000, 0x40000, CRC(dd7ea212) SHA1(adaf0262e315c26b1f4d6365e9d465c7afb6984d))
	ROM_RELOAD(0xc0000, 0x40000)

	ROM_REGION(0x10000, "p7sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(09712c37) SHA1(e2ee902ea6eac3e6257880949bd07a90de08e7b9))
ROM_END

/*-------------------------------------------------------------------
/ Tee'd Off (#736)
/-------------------------------------------------------------------*/
ROM_START(teedoffp) // Rev. 3
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gprom3.bin", 0x0000, 0x10000, CRC(d7008579) SHA1(b7bc9f54340ffb2d684b5df80624e8c01e7fa18b))

	ROM_REGION(0x80000, "dmdcpu", ROMREGION_ERASEFF)
	ROM_LOAD("dsprom1.bin", 0x00000, 0x80000, CRC(24f10ad2) SHA1(15f44f69d39ca9782410a75070edf348f64dba62))

	ROM_REGION(0x10000, "p7sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(3868e77a) SHA1(2db91c527803a369ca659eaae6022667a126d2ef))

	ROM_REGION(0x100000, "p7sound:oki", ROMREGION_ERASEFF)
	ROM_LOAD("arom1.bin", 0x00000, 0x40000, CRC(9e442b71) SHA1(889023af42a2527a51343ccee7f66b089b6e6d01))
	ROM_RELOAD(0x40000, 0x40000)
	ROM_LOAD("arom2.bin", 0x80000, 0x40000, CRC(3dad9508) SHA1(70ed49fa82dbe7586bfca72c5020834f9173d563))
	ROM_RELOAD(0xc0000, 0x40000)

	ROM_REGION(0x10000, "p7sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(c51d98d8) SHA1(9387a39a03ca90bc8eaddc0c2df8874067a22dea))
ROM_END

ROM_START(teedoffp1) // Rev. 1
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gprom1.bin", 0x0000, 0x10000, CRC(95760ab1) SHA1(9342128e2de4e81c4b0cfc482bb0650434a04bee))

	ROM_REGION(0x80000, "dmdcpu", ROMREGION_ERASEFF)
	ROM_LOAD("dsprom1.bin", 0x00000, 0x80000, CRC(24f10ad2) SHA1(15f44f69d39ca9782410a75070edf348f64dba62))

	ROM_REGION(0x10000, "p7sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(3868e77a) SHA1(2db91c527803a369ca659eaae6022667a126d2ef))

	ROM_REGION(0x100000, "p7sound:oki", ROMREGION_ERASEFF)
	ROM_LOAD("arom1.bin", 0x00000, 0x40000, CRC(9e442b71) SHA1(889023af42a2527a51343ccee7f66b089b6e6d01))
	ROM_RELOAD(0x40000, 0x40000)
	ROM_LOAD("arom2.bin", 0x80000, 0x40000, CRC(3dad9508) SHA1(70ed49fa82dbe7586bfca72c5020834f9173d563))
	ROM_RELOAD(0xc0000, 0x40000)

	ROM_REGION(0x10000, "p7sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(c51d98d8) SHA1(9387a39a03ca90bc8eaddc0c2df8874067a22dea))
ROM_END

ROM_START(teedoffp0) // No rev.
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(0620365b) SHA1(18887c49a5d3806b725fa6289e50db82974c0f40))

	ROM_REGION(0x80000, "dmdcpu", ROMREGION_ERASEFF)
	ROM_LOAD("dsprom.bin", 0x00000, 0x80000, CRC(340b8a49) SHA1(3ac76faf920b00b77c77023c42595307840ed3a7))

	ROM_REGION(0x10000, "p7sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(3868e77a) SHA1(2db91c527803a369ca659eaae6022667a126d2ef))

	ROM_REGION(0x100000, "p7sound:oki", ROMREGION_ERASEFF)
	ROM_LOAD("arom1.bin", 0x00000, 0x40000, CRC(9e442b71) SHA1(889023af42a2527a51343ccee7f66b089b6e6d01))
	ROM_RELOAD(0x40000, 0x40000)
	ROM_LOAD("arom2.bin", 0x80000, 0x40000, CRC(3dad9508) SHA1(70ed49fa82dbe7586bfca72c5020834f9173d563))
	ROM_RELOAD(0xc0000, 0x40000)

	ROM_REGION(0x10000, "p7sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(c51d98d8) SHA1(9387a39a03ca90bc8eaddc0c2df8874067a22dea))
ROM_END

/*-------------------------------------------------------------------
/ Waterworld (#746)
/-------------------------------------------------------------------*/
ROM_START(waterwld) // Rev. 3
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(db1fd197) SHA1(caa22f7e3f52be85da496375115933722a414ee0))

	ROM_REGION(0x80000, "dmdcpu", ROMREGION_ERASEFF)
	ROM_LOAD("dsprom.bin", 0x00000, 0x80000, CRC(79164099) SHA1(fa048fb7aa91cadd6c0758c570a4c74337bd7cd5))

	ROM_REGION(0x10000, "p7sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(2a8c5d04) SHA1(1a6a698fc05a199923721e91e68aaaa8d3c6a3c2))

	ROM_REGION(0x100000, "p7sound:oki", ROMREGION_ERASEFF)
	ROM_LOAD("arom1.bin", 0x00000, 0x80000, CRC(3ee37668) SHA1(9ced05b4f060568bf686974bc2472ff7c05a87c6))
	ROM_LOAD("arom2.bin", 0x80000, 0x80000, CRC(a631bf12) SHA1(4784da1fabd2858b2c47af71784eb475cbbb4ab5))

	ROM_REGION(0x10000, "p7sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(6dddce0a) SHA1(6ad9b023ba8632dda0a4e04a4f66aac52ddd3b09))
ROM_END

ROM_START(waterwld2) // Rev. 2
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gprom2.bin", 0x0000, 0x10000, CRC(c3d64cd7) SHA1(63bfd26fdc7082c2bb60c978508820442ac90f14))

	ROM_REGION(0x80000, "dmdcpu", ROMREGION_ERASEFF)
	ROM_LOAD("dsprom.bin", 0x00000, 0x80000, CRC(79164099) SHA1(fa048fb7aa91cadd6c0758c570a4c74337bd7cd5))

	ROM_REGION(0x10000, "p7sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(2a8c5d04) SHA1(1a6a698fc05a199923721e91e68aaaa8d3c6a3c2))

	ROM_REGION(0x100000, "p7sound:oki", ROMREGION_ERASEFF)
	ROM_LOAD("arom1.bin", 0x00000, 0x80000, CRC(3ee37668) SHA1(9ced05b4f060568bf686974bc2472ff7c05a87c6))
	ROM_LOAD("arom2.bin", 0x80000, 0x80000, CRC(a631bf12) SHA1(4784da1fabd2858b2c47af71784eb475cbbb4ab5))

	ROM_REGION(0x10000, "p7sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(6dddce0a) SHA1(6ad9b023ba8632dda0a4e04a4f66aac52ddd3b09))
ROM_END

/*-------------------------------------------------------------------
/ Wipeout (#738)
/-------------------------------------------------------------------*/
ROM_START(wipeout) // Rev. 2 (GPROM checksum 1F9B)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(1161cdb7) SHA1(fdf4c0abb70a41149c69bd55c613849a662944d3))

	ROM_REGION(0x80000, "dmdcpu", ROMREGION_ERASEFF)
	ROM_LOAD("dsprom.bin", 0x00000, 0x80000, CRC(cbdec3ab) SHA1(2d70d436783830bf074a7a0590d5c48432136595))

	ROM_REGION(0x10000, "p7sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(98ae6da4) SHA1(3842c2c4e708a5deae6b5d9407694d337b62384f))

	ROM_REGION(0x100000, "p7sound:oki", ROMREGION_ERASEFF)
	ROM_LOAD("arom1.bin", 0x00000, 0x40000, CRC(cccdf23a) SHA1(1b1e31f04cd60d64f0b9b8ab2c6169dacd0bce69))
	ROM_RELOAD(0x40000, 0x40000)
	ROM_LOAD("arom2.bin", 0x80000, 0x40000, CRC(d4cc44a1) SHA1(c68264f00efa9f219fc257061ed39cd789e94126))
	ROM_RELOAD(0xc0000, 0x40000)

	ROM_REGION(0x10000, "p7sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(f08e6d7f) SHA1(284214ac80735ddd36933ecd60debc7aea18403c))
ROM_END

ROM_START(wipeout2a) // Rev. 2 (GPROM checksum 35FA)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("wipe_out_738_gprom.bin", 0x0000, 0x10000, CRC(968da1ac) SHA1(7400b7a36926d6bb39349a7654e3c33c5eda1052))

	ROM_REGION(0x80000, "dmdcpu", ROMREGION_ERASEFF)
	ROM_LOAD("wipe_out_738_dsprom.bin", 0x00000, 0x80000, CRC(cbdec3ab) SHA1(2d70d436783830bf074a7a0590d5c48432136595))

	ROM_REGION(0x10000, "p7sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("wipe_out_738_drom1.bin", 0x8000, 0x8000, CRC(98ae6da4) SHA1(3842c2c4e708a5deae6b5d9407694d337b62384f))

	ROM_REGION(0x100000, "p7sound:oki", ROMREGION_ERASEFF)
	ROM_LOAD("wipe_out_738_arom1.u4", 0x00000, 0x40000, CRC(cccdf23a) SHA1(1b1e31f04cd60d64f0b9b8ab2c6169dacd0bce69))
	ROM_RELOAD(0x40000, 0x40000)
	ROM_LOAD("wipe_out_738_arom2.u5", 0x80000, 0x40000, CRC(d4cc44a1) SHA1(c68264f00efa9f219fc257061ed39cd789e94126))
	ROM_RELOAD(0xc0000, 0x40000)

	ROM_REGION(0x10000, "p7sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("wipe_out_738_yrom1.bin", 0x8000, 0x8000, CRC(f08e6d7f) SHA1(284214ac80735ddd36933ecd60debc7aea18403c))
ROM_END

/*-------------------------------------------------------------------
/ World Challenge Soccer (#741)
/-------------------------------------------------------------------*/
ROM_START(wcsoccer)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(6382c32e) SHA1(e212f4a9a77d1cf089acb226a8079ac4cae8a96d))

	ROM_REGION(0x80000, "dmdcpu", ROMREGION_ERASEFF)
	ROM_LOAD("dsprom.bin", 0x00000, 0x80000, CRC(71ba5263) SHA1(e86c2cc89d31534fb2d9d24fab2fcdb0af7cc73d))

	ROM_REGION(0x10000, "p7sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(18d5edf3) SHA1(7d0d46506cf9d4b96b9b93139e3c65643e120c28))

	ROM_REGION(0x100000, "p7sound:oki", ROMREGION_ERASEFF)
	ROM_LOAD("arom1.bin", 0x00000, 0x40000, CRC(ece4eebf) SHA1(78f882668967194bd547ace5d22083faeb29ef5e))
	ROM_RELOAD(0x40000, 0x40000)
	ROM_LOAD("arom2.bin", 0x80000, 0x40000, CRC(4e466500) SHA1(78c4b41a174d82a7e0e7775713c76e679c8a7e89))
	ROM_RELOAD(0xc0000, 0x40000)

	ROM_REGION(0x10000, "p7sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(8b2795b0) SHA1(b838d4e410c815421099c65b0d3b22227dae17c6))
ROM_END

ROM_START(wcsoccer1a)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(6382c32e) SHA1(e212f4a9a77d1cf089acb226a8079ac4cae8a96d))

	ROM_REGION(0x80000, "dmdcpu", ROMREGION_ERASEFF)
	ROM_LOAD("dsprom2.bin", 0x00000, 0x80000, CRC(4c8ea71d) SHA1(ce751b84e2033e4de2f2c57490867ecafd423aaa))

	ROM_REGION(0x10000, "p7sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(18d5edf3) SHA1(7d0d46506cf9d4b96b9b93139e3c65643e120c28))

	ROM_REGION(0x100000, "p7sound:oki", ROMREGION_ERASEFF)
	ROM_LOAD("arom1.bin", 0x00000, 0x40000, CRC(ece4eebf) SHA1(78f882668967194bd547ace5d22083faeb29ef5e))
	ROM_RELOAD(0x40000, 0x40000)
	ROM_LOAD("arom2.bin", 0x80000, 0x40000, CRC(4e466500) SHA1(78c4b41a174d82a7e0e7775713c76e679c8a7e89))
	ROM_RELOAD(0xc0000, 0x40000)

	ROM_REGION(0x10000, "p7sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(8b2795b0) SHA1(b838d4e410c815421099c65b0d3b22227dae17c6))
ROM_END

} // anonymous namespace

GAME(1992,  smbp,       0,        p7, gts3a, gts3a_state, init_gts3a, ROT0, "Gottlieb", "Super Mario Brothers (pinball, set 1)",      MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1992,  smbpa,      smbp,     p7, gts3a, gts3a_state, init_gts3a, ROT0, "Gottlieb", "Super Mario Brothers (pinball, set 2)",      MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1992,  smbpb,      smbp,     p7, gts3a, gts3a_state, init_gts3a, ROT0, "Gottlieb", "Super Mario Brothers (pinball, set 3)",      MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1992,  smbpc,      smbp,     p7, gts3a, gts3a_state, init_gts3a, ROT0, "Gottlieb", "Super Mario Brothers (pinball, set 4)",      MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1992,  smbmush,    0,        p7, gts3a, gts3a_state, init_gts3a, ROT0, "Gottlieb", "Super Mario Brothers Mushroom World",        MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1992,  cueball,    0,        p7, gts3a, gts3a_state, init_gts3a, ROT0, "Gottlieb", "Cue Ball Wizard",                            MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1992,  cueballa,   cueball,  p7, gts3a, gts3a_state, init_gts3a, ROT0, "Gottlieb", "Cue Ball Wizard (rev. 2)",                   MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1992,  cueballb,   cueball,  p7, gts3a, gts3a_state, init_gts3a, ROT0, "Gottlieb", "Cue Ball Wizard (rev. 3)",                   MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1992,  cueballc,   cueball,  p7, gts3a, gts3a_state, init_gts3a, ROT0, "Gottlieb", "Cue Ball Wizard (older display rev.)",       MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1993,  sfightii,   0,        p7, gts3a, gts3a_state, init_gts3a, ROT0, "Gottlieb", "Street Fighter II (pinball, set 1)",         MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1993,  sfightiia,  sfightii, p7, gts3a, gts3a_state, init_gts3a, ROT0, "Gottlieb", "Street Fighter II (pinball, set 2)",         MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1993,  sfightiib,  sfightii, p7, gts3a, gts3a_state, init_gts3a, ROT0, "Gottlieb", "Street Fighter II (pinball, set 3)",         MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1993,  teedoffp,   0,        p7, gts3a, gts3a_state, init_gts3a, ROT0, "Gottlieb", "Tee'd Off (pinball, rev.3)",                 MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1993,  teedoffp1,  teedoffp, p7, gts3a, gts3a_state, init_gts3a, ROT0, "Gottlieb", "Tee'd Off (pinball, rev.1)",                 MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1993,  teedoffp0,  teedoffp, p7, gts3a, gts3a_state, init_gts3a, ROT0, "Gottlieb", "Tee'd Off (pinball)",                        MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1993,  gladiatp,   0,        p7, gts3a, gts3a_state, init_gts3a, ROT0, "Gottlieb", "Gladiators (pinball)",                       MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1993,  wipeout,    0,        p7, gts3a, gts3a_state, init_gts3a, ROT0, "Gottlieb", "Wipeout (rev.2, set 1)",                     MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1993,  wipeout2a,  wipeout,  p7, gts3a, gts3a_state, init_gts3a, ROT0, "Gottlieb", "Wipeout (rev.2, set 2)",                     MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1994,  rescu911,   0,        p7, gts3a, gts3a_state, init_gts3a, ROT0, "Gottlieb", "Rescue 911 (rev.1)",                         MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1994,  wcsoccer,   0,        p7, gts3a, gts3a_state, init_gts3a, ROT0, "Gottlieb", "World Challenge Soccer (rev.1, set 1)",      MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1994,  wcsoccer1a, wcsoccer, p7, gts3a, gts3a_state, init_gts3a, ROT0, "Gottlieb", "World Challenge Soccer (rev.1, set 2)",      MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1995,  stargatp,   0,        p7, gts3a, gts3a_state, init_gts3a, ROT0, "Gottlieb", "Stargate (pinball, rev.5)",                  MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1995,  stargatp4,  stargatp, p7, gts3a, gts3a_state, init_gts3a, ROT0, "Gottlieb", "Stargate (pinball, rev.4)",                  MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1995,  stargatp3,  stargatp, p7, gts3a, gts3a_state, init_gts3a, ROT0, "Gottlieb", "Stargate (pinball, rev.3)",                  MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1995,  stargatp2,  stargatp, p7, gts3a, gts3a_state, init_gts3a, ROT0, "Gottlieb", "Stargate (pinball, rev.2)",                  MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1995,  stargatp1,  stargatp, p7, gts3a, gts3a_state, init_gts3a, ROT0, "Gottlieb", "Stargate (pinball, rev.1)",                  MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1995,  stargatp0,  stargatp, p7, gts3a, gts3a_state, init_gts3a, ROT0, "Gottlieb", "Stargate (pinball)",                         MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1995,  shaqattq,   0,        p7, gts3a, gts3a_state, init_gts3a, ROT0, "Gottlieb", "Shaq Attaq (rev.5)",                         MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1995,  shaqattq2,  shaqattq, p7, gts3a, gts3a_state, init_gts3a, ROT0, "Gottlieb", "Shaq Attaq (rev.2)",                         MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(2003,  mac_zois,   shaqattq, p7, gts3a, gts3a_state, init_gts3a, ROT0, "Aksioma",  "machinaZOIS Virtual Training Center",        MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1994,  freddy,     0,        p7, gts3a, gts3a_state, init_gts3a, ROT0, "Gottlieb", "Freddy: A Nightmare on Elm Street (rev.4)",  MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1994,  freddy3,    freddy,   p7, gts3a, gts3a_state, init_gts3a, ROT0, "Gottlieb", "Freddy: A Nightmare on Elm Street (rev.3)",  MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1995,  bighurt,    0,        p7, gts3a, gts3a_state, init_gts3a, ROT0, "Gottlieb", "Frank Thomas' Big Hurt (rev.3)",             MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1995,  waterwld,   0,        p7, gts3a, gts3a_state, init_gts3a, ROT0, "Gottlieb", "Waterworld (rev.3)",                         MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1995,  waterwld2,  waterwld, p7, gts3a, gts3a_state, init_gts3a, ROT0, "Gottlieb", "Waterworld (rev.2)",                         MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1995,  snspares,   0,        p0, gts3a, gts3a_state, init_gts3a, ROT0, "Gottlieb", "Strikes n' Spares (rev.6)",                  MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1995,  snspares2,  snspares, p0, gts3a, gts3a_state, init_gts3a, ROT0, "Gottlieb", "Strikes n' Spares (rev.2)",                  MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1995,  snspares1,  snspares, p0, gts3a, gts3a_state, init_gts3a, ROT0, "Gottlieb", "Strikes n' Spares (rev.1)",                  MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1995,  andretti,   0,        p7, gts3a, gts3a_state, init_gts3a, ROT0, "Gottlieb", "Mario Andretti (rev.T4)",                    MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1995,  andretti0,  andretti, p7, gts3a, gts3a_state, init_gts3a, ROT0, "Gottlieb", "Mario Andretti",                             MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1996,  barbwire,   0,        p7, gts3a, gts3a_state, init_gts3a, ROT0, "Gottlieb", "Barb Wire",                                  MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1996,  brooks,     0,        p0, gts3a, gts3a_state, init_gts3a, ROT0, "Gottlieb", "Brooks & Dunn (rev.T1)",                     MACHINE_IS_SKELETON_MECHANICAL | MACHINE_IS_INCOMPLETE | MACHINE_SUPPORTS_SAVE )
