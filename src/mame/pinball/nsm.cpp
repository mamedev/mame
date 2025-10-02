// license:BSD-3-Clause
// copyright-holders:Robbbert
/********************************************************************************

PINBALL
NSM (Lowen) : Hot Fire Birds

Schematic and PinMAME used as references

Everything in this machine is controlled by a serial bus based on the
 processor's CRU pins (serial i/o).
To program the machine, you need to plug in a numeric keypad, engage test
 mode, make the adjustments, and remove the keypad. For full scale setting up,
 it seems you need a pilot's licence, and to be able to understand German.

Setting up the machine (the simple way):
- Turn Diag6 dip on, choose Reset
- You get the test menu. Press num-4, then num-1 then num-Enter. It will slowly
  set itself up.
- When the menu reappears, turn Diag6 dip off, and choose Reset.
- The machine now seems to be working, and you can insert coins, but then it reboots.

If the display says FLiPPEr you can go into the test menu and then exit.

Status:
- See above. Unable to start a game.
- gamesnsm keeps rebooting itself at start
- Pressing the coin input buttons restarts the machine.

ToDo:
- There's a protection circuit for E600 which isn't quite right yet.
- Inputs (they are there, but unable to test)
- Mechanical sounds
- Solenoids fail in the test menu

*********************************************************************************/

#include "emu.h"
#include "genpin.h"

#include "cpu/tms9900/tms9995.h"
#include "sound/ay8910.h"
#include "speaker.h"

#include "nsm.lh"


namespace {

class nsm_state : public genpin_class
{
public:
	nsm_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_nvram(*this, "nvram")
		, m_io_keyboard(*this, "X%d", 0U)
		, m_digits(*this, "digit%d", 0U)
		, m_io_outputs(*this, "out%d", 0U)
		{ }

	void nsm(machine_config &config);

private:

	u8 ff_r() { return 1; }
	void e600_w(offs_t, u8);
	u8 enwnv_r();
	u8 loadin_r();
	u8 loadout_r();
	u8 clockin_r();
	void clockout_w(u8);
	u8 np_clock_r(offs_t);
	void np_sel1_w(u8);
	void np_sel2_w(u8);
	void np_load_w(u8);
	u8 diag3_r();
	u8 diag4_r();
	u8 diag6_r();
	void ay1a_w(u8);
	void ay2a_w(u8);
	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	u8 m_cru_out[9]{};
	u8 m_cru_in[3]{};
	u8 m_cru_in_bit = 0U;
	u8 m_cru_overflow = 0U;
	u8 m_row = 0U;
	u8 m_np_cru = 0U;
	u8 m_np_sel = 0U;
	bool m_e600_locked = false;
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
	required_device<tms9995_device> m_maincpu;
	required_shared_ptr<u8> m_nvram;
	required_ioport_array<13> m_io_keyboard;
	output_finder<48> m_digits;
	output_finder<80> m_io_outputs;   // 16 solenoids + 64 lamps
};

void nsm_state::mem_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();  // ic602/3/4/5, 2764
	map(0xe000, 0xe7ff).ram().share("nvram"); // ic606, 6116
	map(0xe800, 0xefff).ram();  // ic607, 2016
	map(0xffec, 0xffed).w("ay1", FUNC(ay8910_device::address_data_w));
	map(0xffee, 0xffef).w("ay2", FUNC(ay8910_device::address_data_w));
	map(0xe600, 0xe601).w(FUNC(nsm_state::e600_w));
}

void nsm_state::io_map(address_map &map)
{
	// 000-71f selected by IC600 (74LS151)
	map(0x0000, 0x001f).mirror(0xf8e0).r(FUNC(nsm_state::ff_r)); // 5v supply (low = black screen)
	map(0x0100, 0x011f).mirror(0xf8e0).nopr(); // antenna (high = black screen)
	map(0x0200, 0x021f).mirror(0xf8e0).nopr(); // reset circuit, not used?
	map(0x0300, 0x031f).mirror(0xf8e0).r(FUNC(nsm_state::diag3_r)); // service plug (if low, the test fail msg stays up longer)
	map(0x0400, 0x041f).mirror(0xf8e0).r(FUNC(nsm_state::diag4_r)); // service plug, not used?
	map(0x0500, 0x051f).mirror(0xf8e0).nopr(); // test of internal battery (high = BA error)
	map(0x0600, 0x061f).mirror(0xf8e0).r(FUNC(nsm_state::diag6_r)); // low = test menu (sch says ST703 but it's ST702)
	map(0x1ee0, 0x1efb).noprw();
	map(0xfe40, 0xfe4f).r(FUNC(nsm_state::np_clock_r));
	map(0xff00, 0xff1f).r(FUNC(nsm_state::loadin_r)).w(FUNC(nsm_state::np_load_w));
	map(0xff20, 0xff3f).r(FUNC(nsm_state::loadout_r));
	map(0xff40, 0xff5f).r(FUNC(nsm_state::clockin_r));
	map(0xff60, 0xff7f).w(FUNC(nsm_state::clockout_w));
	map(0xff80, 0xff9f).w(FUNC(nsm_state::np_sel1_w));
	map(0xffa0, 0xffbf).w(FUNC(nsm_state::np_sel2_w));
	map(0xffe0, 0xffff).r(FUNC(nsm_state::enwnv_r));
}

static INPUT_PORTS_START( nsm )
	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("INP09")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("INP10")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("INP11")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("INP12")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_NAME("INP13")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_NAME("INP14")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_NAME("INP15")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_NAME("INP16")

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("INP17")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("INP18")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("INP19")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_NAME("INP20")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME("INP21")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_NAME("INP22")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_NAME("INP23")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_NAME("INP24")

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_NAME("INP25")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_K) PORT_NAME("INP26")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_L) PORT_NAME("INP27")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("INP28")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_NAME("INP29")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_NAME("INP30")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_NAME("INP31")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_NAME("INP32")

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_NAME("INP33")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_O) PORT_NAME("INP34")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_P) PORT_NAME("INP35")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("INP36")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_NAME("INP37")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_NAME("INP38")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_NAME("INP39")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_NAME("INP40")

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("INP41")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("INP42")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("INP43")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_U) PORT_NAME("INP44")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_NAME("INP45")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_NAME("INP46")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_NAME("INP47")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_NAME("INP48")

	PORT_START("X5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_V) PORT_NAME("INP49")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("INP50")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Y) PORT_NAME("INP51")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("INP52")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_NAME("INP53")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_NAME("INP54")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_NAME("INP55")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_NAME("INP56")

	PORT_START("X6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("INP57")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_STOP) PORT_NAME("INP58")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("INP59")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_COLON) PORT_NAME("INP60")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_NAME("INP61")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_NAME("INP62")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_NAME("INP63")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_NAME("INP64")

	PORT_START("X7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("INP01")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_NAME("INP02")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("INP03")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("INP04")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_NAME("INP05")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_NAME("INP06")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_NAME("INP07")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_NAME("INP08")

	PORT_START("X8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("Right Flipper")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED ) //  *OUT_OK
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED ) // I5
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED ) // I4
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED ) // I1

	PORT_START("X9")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START ) // INP73
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("Left Flipper")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Outhole")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED ) // 12v
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN3 )

	// Programming pad, keys are guesswork, no schematic or pictures exist
	PORT_START("X10")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH_PAD) PORT_NAME("PAD12") // nothing
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_ASTERISK) PORT_NAME("PAD13") // nothing
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("PAD04")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("PAD09")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("PAD08") // and scroll list
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("PAD07")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("PAD10")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("PAD00")

	PORT_START("X11")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("PAD14") // nothing
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_DEL_PAD) PORT_NAME("PAD11") // backspace
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("PAD15") // nothing
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("PAD03")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("PAD02")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("PAD01")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("PAD06")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("PAD05")

	PORT_START("X12")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_NAME("Diag3")
	PORT_DIPNAME( 0x02, 0x02, "Diag4")   // Appears to do nothing
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x02, DEF_STR(Off))
	PORT_DIPNAME( 0x04, 0x04, "Diag6")   // Test mode
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x04, DEF_STR(Off))
INPUT_PORTS_END

// **** SERVICE SWITCHES ****
u8 nsm_state::diag3_r()
{
	return BIT(m_io_keyboard[12]->read(), 0);
}

u8 nsm_state::diag4_r()
{
	return BIT(m_io_keyboard[12]->read(), 1);
}

u8 nsm_state::diag6_r()
{
	return BIT(m_io_keyboard[12]->read(), 2);
}

u8 nsm_state::enwnv_r()
{
	m_e600_locked = false;
	return m_cru_in_bit;
}

// Todo: find out how this really works
void nsm_state::e600_w(offs_t offset, u8 data)
{
	if (offset && BIT(data, 4))
		m_e600_locked = true;
	if (!m_e600_locked)
		m_nvram[0x600+offset] = data;
}

// **** CRU OUTPUTS ****
// send clocked data from 4094 chain to outputs
u8 nsm_state::loadout_r()
{
	// 0 = strobe for keyboard, lamps, display
	u8 i;
	for (i = 0; i < 8; i++)
		if (BIT(m_cru_out[0], i))
			m_row = i;

	for (i = 0; i < 8; i++)
	{
		// 1 = Lamps
		m_io_outputs[16+m_row*8+i] = BIT(m_cru_out[1], i) ? 0 : 1;
		// 2,3 = Solenoids
		m_io_outputs[i] = BIT(m_cru_out[2], i);
		m_io_outputs[8+i] = BIT(m_cru_out[3], i);
	}

	// 4 = player 1 display
	// 5 = player 2 display
	// 6 = player 3 display
	// 7 = player 4 display
	// 8 = status display
	for (i = 0; i < 5; i++)
		m_digits[i * 10 + m_row] = bitswap<10>(m_cru_out[i+4] ^ 0xff, 7, 7, 6, 6, 5, 4, 3, 2, 1, 0);
	return m_cru_in_bit;
}

// send a cru bit to 4094 chain
void nsm_state::clockout_w(u8 data)
{
	m_cru_overflow = BIT(m_cru_out[8], 7);
	m_cru_out[8] = (m_cru_out[8] << 1) | BIT(m_cru_out[7], 7);
	m_cru_out[7] = (m_cru_out[7] << 1) | BIT(m_cru_out[6], 7);
	m_cru_out[6] = (m_cru_out[6] << 1) | BIT(m_cru_out[5], 7);
	m_cru_out[5] = (m_cru_out[5] << 1) | BIT(m_cru_out[4], 7);
	m_cru_out[4] = (m_cru_out[4] << 1) | BIT(m_cru_out[3], 7);
	m_cru_out[3] = (m_cru_out[3] << 1) | BIT(m_cru_out[2], 7);
	m_cru_out[2] = (m_cru_out[2] << 1) | BIT(m_cru_out[1], 7);
	m_cru_out[1] = (m_cru_out[1] << 1) | BIT(m_cru_out[0], 7);
	m_cru_out[0] = (m_cru_out[0] << 1) | data;
	(void)clockin_r();
}

// **** CRU INPUTS ****
// load up 4021 chain with keyboard inputs
u8 nsm_state::loadin_r()
{
	// 0 = playboard inputs
	m_cru_in[0] = m_io_keyboard[m_row]->read();
	// 1 = hardware error detectors
	m_cru_in[1] = m_io_keyboard[8]->read();
	// 2 = door inputs
	m_cru_in[2] = m_io_keyboard[9]->read();
	return m_cru_in_bit;
}

// get a bit from 4021 chain
u8 nsm_state::clockin_r()
{
	m_cru_in_bit = BIT(m_cru_in[2], 7);
	m_cru_in[2] = (m_cru_in[2] << 1) | BIT(m_cru_in[1], 7);
	m_cru_in[1] = (m_cru_in[1] << 1) | BIT(m_cru_in[0], 7);
	m_cru_in[0] = (m_cru_in[0] << 1) | m_cru_overflow;
	return m_cru_in_bit;
}

// **** PROGRAMMER KEYPAD ****
// All guesswork, no schematic or pictures exist
u8 nsm_state::np_clock_r(offs_t offset)
{
	return BIT(m_np_cru, offset);
}

void nsm_state::np_sel1_w(u8 data)
{
	m_np_sel = 0;
}

void nsm_state::np_sel2_w(u8 data)
{
	m_np_sel = 1;
}

void nsm_state::np_load_w(u8 data)
{
	m_np_cru = m_io_keyboard[10U+m_np_sel]->read();
}

// Apply tonal changes to the sound
void nsm_state::ay1a_w(u8 data)
{
}

void nsm_state::ay2a_w(u8 data)
{
}

void nsm_state::machine_start()
{
	genpin_class::machine_start();

	m_digits.resolve();
	m_io_outputs.resolve();

	save_item(NAME(m_cru_out));
	save_item(NAME(m_cru_in));
	save_item(NAME(m_cru_in_bit));
	save_item(NAME(m_cru_overflow));
	save_item(NAME(m_row));
	save_item(NAME(m_np_cru));
	save_item(NAME(m_np_sel));
}

void nsm_state::machine_reset()
{
	genpin_class::machine_reset();
	for (u8 i = 0; i < m_io_outputs.size(); i++)
		m_io_outputs[i] = 0;

	// Disable auto wait state generation by raising the READY line on reset
	m_maincpu->ready_line(ASSERT_LINE);
	m_maincpu->reset_line(ASSERT_LINE);
}

void nsm_state::nsm(machine_config &config)
{
	// CPU TMS9995, standard variant; no line connection
	TMS9995(config, m_maincpu, 11052000);
	m_maincpu->set_addrmap(AS_PROGRAM, &nsm_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &nsm_state::io_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* Video */
	config.set_default_layout(layout_nsm);

	/* Sound */
	genpin_audio(config);
	SPEAKER(config, "speaker", 2).front();
	ay8912_device &ay1(AY8912(config, "ay1", 11052000/8));
	ay1.add_route(ALL_OUTPUTS, "speaker", 0.75, 0);
	ay1.port_a_write_callback().set(FUNC(nsm_state::ay1a_w));
	ay8912_device &ay2(AY8912(config, "ay2", 11052000/8));
	ay2.add_route(ALL_OUTPUTS, "speaker", 0.75, 1);
	ay2.port_a_write_callback().set(FUNC(nsm_state::ay2a_w));
}

/*-------------------------------------------------------------------
/ Cosmic Flash (1985)
/-------------------------------------------------------------------*/
ROM_START(cosflnsm)
	ROM_REGION(0x8000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("ic602.bin", 0x0000, 0x2000, CRC(1ce79cd7) SHA1(d5caf6d4323cc43a9c4379b51630190bf5799202))
	ROM_LOAD("ic603.bin", 0x2000, 0x2000, CRC(538de9f8) SHA1(c64942ffa600a2a7a37b986e1a346d351d0b65eb))
	ROM_LOAD("ic604.bin", 0x4000, 0x2000, CRC(4b52e5d7) SHA1(1547bb7a06ff0bdf55c635b2f4e57b7d93a191ee))
ROM_END

/*-------------------------------------------------------------------
/ Hot Fire Birds (1985)
/-------------------------------------------------------------------*/
ROM_START(firebird)
	ROM_REGION(0x8000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("nsmf02.ic602", 0x0000, 0x2000, CRC(236b5780) SHA1(19ef6e1fc900e5d94f615a4316f0383ed5ee939c))
	ROM_LOAD("nsmf03.ic603", 0x2000, 0x2000, CRC(d88c6ef5) SHA1(00edeefaab7e1141741aa132e6f7e56a911573be))
	ROM_LOAD("nsmf04.ic604", 0x4000, 0x2000, CRC(38a8add4) SHA1(74f781edc31aad07411feacad53c5f6cc73d09f4))
ROM_END

/*-------------------------------------------------------------------
/ Tag-Team Pinball (1986)
/-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
/ The Games (1985)
/-------------------------------------------------------------------*/
ROM_START(gamesnsm)
	ROM_REGION(0x8000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("151595.ic602", 0x0000, 0x2000, CRC(18f3e309) SHA1(f587d40ddf128f4e040e660c054e98cbebad99c7))
	ROM_LOAD("151596.ic603", 0x2000, 0x2000, CRC(fdf1b48b) SHA1(fd63ef5e49aa4b84b10972e118bd54219d680d36))
	ROM_LOAD("151597.ic604", 0x4000, 0x2000, CRC(5c8a3547) SHA1(843a56012227a61ff068bc1e14baf090d4a95fe1))
ROM_END

} // anonymous namespace


GAME(1985,  cosflnsm,  0,  nsm,  nsm, nsm_state, empty_init, ROT0, "NSM", "Cosmic Flash (NSM)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1985,  firebird,  0,  nsm,  nsm, nsm_state, empty_init, ROT0, "NSM", "Hot Fire Birds",     MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1985,  gamesnsm,  0,  nsm,  nsm, nsm_state, empty_init, ROT0, "NSM", "The Games (NSM)",    MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
