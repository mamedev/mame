// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************************************

PINBALL
Zaccaria Generation 2

All games (except where noted below)
- To start, press 1
- Wait for score to flash before you can score (first ball can take a while, jiggle XZ to push
    things along)
- Press X to end ball
- Ball counter runs backwards, so 1 = last ball.
- When on the last ball, scoring points racks up a bonus counter. At the end, you get
   an extra timed part, with infinite balls until the time runs out. Then press X.

Games:
- Pinball Champ '82
- Soccer Kings
- Pinball Champ
- Time Machine
- Farfalla
- Devil Riders
- Magic Castle
- Robot
- Clown
- Pool Champion
- Mystic Star
- Black Belt
- Mexico 86 (not working)
- Zankor
- Spooky
- Star's Phoenix (Multiball game: To start, hit 1, then jiggle ABX. To end ball, press AB)
- New Star's Phoenix (same as Star's Phoenix)
**** Other Manufacturer ****
- Scramble (not working)
- Thunder Man

Status:
- Most games are playable

ToDo:
- Fix scram_tp, mexico
- Sound
- Mechanical sounds

****************************************************************************************************/


#include "emu.h"
#include "genpin.h"
#include "cpu/s2650/s2650.h"
#include "machine/timer.h"
#include "speaker.h"
#include "zac_2.lh"

namespace {

class zac_2_state : public genpin_class
{
public:
	zac_2_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_p_ram(*this, "ram")
		, m_io_keyboard(*this, "X%d", 0U)
		, m_digits(*this, "digit%d", 0U)
		, m_io_outputs(*this, "out%d", 0U)
	{ }

	void zac_2(machine_config &config);
	void init_1() { m_game = 1; }  // 7 digits

private:
	u8 ctrl_r();
	u8 data_r();
	void ctrl_w(u8 data);
	void data_w(u8 data);
	int serial_r();
	void serial_w(int state);
	TIMER_DEVICE_CALLBACK_MEMBER(zac_2_inttimer);
	TIMER_DEVICE_CALLBACK_MEMBER(zac_2_outtimer);
	void zac_2_data(address_map &map) ATTR_COLD;
	void zac_2_io(address_map &map) ATTR_COLD;
	void zac_2_map(address_map &map) ATTR_COLD;

	u8 m_row = 0U;
	u8 m_t_c = 0U;
	u8 m_out_offs = 0U;
	u8 m_game = 0U;
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
	required_device<s2650_device> m_maincpu;
	required_shared_ptr<u8> m_p_ram;
	required_ioport_array<8> m_io_keyboard;
	output_finder<78> m_digits;
	output_finder<128> m_io_outputs;   // 24 solenoids + 80 lamps
};


void zac_2_state::zac_2_map(address_map &map)
{
	map(0x0000, 0x07ff).rom();
	map(0x0800, 0x17ff).mirror(0x4000).rom();
	map(0x1800, 0x1bff).mirror(0x6400).ram().share("ram");
	map(0x2000, 0x27ff).rom();
	map(0x2800, 0x37ff).mirror(0x4000).rom();
	map(0x4000, 0x47ff).rom();
	map(0x6000, 0x67ff).rom();
}

void zac_2_state::zac_2_io(address_map &map)
{
	map.unmap_value_high();
}

void zac_2_state::zac_2_data(address_map &map)
{
	map(S2650_CTRL_PORT, S2650_CTRL_PORT).rw(FUNC(zac_2_state::ctrl_r), FUNC(zac_2_state::ctrl_w));
	map(S2650_DATA_PORT, S2650_DATA_PORT).rw(FUNC(zac_2_state::data_r), FUNC(zac_2_state::data_w));
}

static INPUT_PORTS_START( zac_2 )
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Switch A") PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x02, 0x02, "Switch B") PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x04, 0x04, "Switch C") PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x08, 0x08, "Switch D") PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))

	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Advance Test")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Return Test")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_NAME("Tilt 2")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Credit Service")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Factory Burn Test")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
	// from here is not correct
	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Outhole")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("INP17")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("INP18")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("INP19")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("INP20")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("INP21")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("INP22")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("INP23")

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_NAME("INP24")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME("INP25")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_NAME("INP26")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_K) PORT_NAME("INP27")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_L) PORT_NAME("INP28")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("INP29")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_NAME("INP30")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_O) PORT_NAME("INP31")

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_P) PORT_NAME("INP32")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("INP33")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("INP34")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("INP35")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("INP36")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_U) PORT_NAME("INP37")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_V) PORT_NAME("INP38")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("INP39")

	PORT_START("X5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Y) PORT_NAME("INP40")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("INP41")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("INP42")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_STOP) PORT_NAME("INP43")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("INP44")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_COLON) PORT_NAME("INP45")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("INP46")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_NAME("INP47")

	PORT_START("X6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("INP48")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("INP49")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("INP50")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("INP51")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("INP52")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("INP53")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_PGUP) PORT_NAME("INP54")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_PGDN) PORT_NAME("INP55")

	PORT_START("X7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_DEL) PORT_NAME("INP56")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_HOME) PORT_NAME("INP57")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_END) PORT_NAME("INP58")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_SPACE) PORT_NAME("INP59")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_UP) PORT_NAME("INP60")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_LEFT) PORT_NAME("INP61")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("INP62")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_DOWN) PORT_NAME("INP63")
INPUT_PORTS_END

u8 zac_2_state::ctrl_r()
{
	return m_io_keyboard[m_row]->read();
}

void zac_2_state::ctrl_w(u8 data)
{
	m_row = data & 7;
}

u8 zac_2_state::data_r()
{
	return ioport("DSW")->read();  // and sound card status
}

void zac_2_state::data_w(u8 data)
{
// writes to sound card
}

int zac_2_state::serial_r()
{
// from printer
	return 0;
}

void zac_2_state::serial_w(int state)
{
// to printer
}

void zac_2_state::machine_start()
{
	m_digits.resolve();
	m_io_outputs.resolve();

	save_item(NAME(m_row));
	save_item(NAME(m_t_c));
	save_item(NAME(m_out_offs));
}

void zac_2_state::machine_reset()
{
	for (u8 i = 0; i < m_io_outputs.size(); i++)
		m_io_outputs[i] = 0;

	m_t_c = 0;
}

TIMER_DEVICE_CALLBACK_MEMBER(zac_2_state::zac_2_inttimer)
{
	// a pulse is sent via a capacitor (similar to what one finds at a reset pin)
	if (m_t_c > 0x80)
		m_maincpu->pulse_input_line(INPUT_LINE_IRQ0, 2 * m_maincpu->minimum_quantum_time());
	else
		m_t_c++;
}

TIMER_DEVICE_CALLBACK_MEMBER(zac_2_state::zac_2_outtimer)
{
	static const u8 patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0, 0, 0, 0, 0, 0 };
	m_out_offs++;
	u8 data = m_p_ram[m_out_offs] & 15;

// displays, solenoids, lamps

	if (m_out_offs < 0x40)
		return;

	if (m_out_offs < 0xc0)
	{
		m_io_outputs[m_out_offs-0x40] = data;

		// These addresses yet to be ascertained
		if (m_out_offs == 0x4a) // outhole
		{
			//if (BIT(data, 0))
				//m_samples->start(0, 5);
		}
		else
		if (m_out_offs == 0x4b) // knocker
		{
			//if (BIT(data, 0))
				//m_samples->start(0, 6);
		}
	}
	else
	if (m_out_offs < 0xf0)
	{
		u8 display = BIT(m_out_offs, 3, 3);
		u8 digit = m_out_offs & 7;
		u16 segment = patterns[data];
		// fix g segment
		if (BIT(segment, 6))
			segment |= 0x80;
		// add commas
		if ((digit == 3) || (digit == 6))
		{
			if (display < 4)
				segment |= 0xc000;
			else
			if (m_digits[42] && m_digits[44])
				segment |= 0xc000;
		}
		// if 7-digit display, blank unwanted 8th digit
		if (m_game && (digit == 7))
			segment = 0;
		m_digits[display * 10 + digit] = segment;
	}
	else
		m_out_offs = 0x3f;
}

void zac_2_state::zac_2(machine_config &config)
{
	/* basic machine hardware */
	S2650(config, m_maincpu, 6000000/2); // according to sch the speed is half this, but then the last ball timer runs far too slowly
	m_maincpu->set_addrmap(AS_PROGRAM, &zac_2_state::zac_2_map);
	m_maincpu->set_addrmap(AS_IO, &zac_2_state::zac_2_io);
	m_maincpu->set_addrmap(AS_DATA, &zac_2_state::zac_2_data);
	m_maincpu->sense_handler().set(FUNC(zac_2_state::serial_r));
	m_maincpu->flag_handler().set(FUNC(zac_2_state::serial_w));
	m_maincpu->intack_handler().set_constant(0xbf);

	NVRAM(config, "ram", nvram_device::DEFAULT_ALL_0);

	TIMER(config, "zac_2_inttimer").configure_periodic(FUNC(zac_2_state::zac_2_inttimer), attotime::from_hz(6'000'000/(8*512)));
	TIMER(config, "zac_2_outtimer").configure_periodic(FUNC(zac_2_state::zac_2_outtimer), attotime::from_hz((6'000'000/4)/32));

	/* Video */
	config.set_default_layout(layout_zac_2);

	/* Sound */
	genpin_audio(config);
	//SPEAKER(config, "mono").front_center();
}

/*--------------------------------
/ Black Belt (03/86)
/-------------------------------*/
ROM_START(bbeltzac)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD ( "bbz-1.fil", 0x0000, 0x0800, CRC(2e7e1575) SHA1(1b9e6e4ff461962f4c7249bd2a748444cb658c30))
	ROM_CONTINUE(0x2000, 0x0800)
	ROM_CONTINUE(0x4000, 0x0800)
	ROM_CONTINUE(0x6000, 0x0800)
	ROM_LOAD ( "bbz-2.fil", 0x0800, 0x0800, CRC(dbec92ae) SHA1(7a1c6e5ac81d3cfcbb135a1c8b69e55296fffcc5))
	ROM_CONTINUE(0x2800, 0x0800)
	ROM_CONTINUE(0x1000, 0x0800)
	ROM_CONTINUE(0x3000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("bbz-e.snd", 0xa000, 0x2000, CRC(1fe045d2) SHA1(d17d7dbcafe9f8644cbe393a56ff6b45d9d40155))
	ROM_LOAD("bbz-f.snd", 0xc000, 0x4000, CRC(9f58f369) SHA1(32472d93284c0f1fc2875714b40428406dcf6325))
ROM_END

ROM_START(bbeltzaci)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD ( "bbz-1.fil", 0x0000, 0x0800, CRC(2e7e1575) SHA1(1b9e6e4ff461962f4c7249bd2a748444cb658c30))
	ROM_CONTINUE(0x2000, 0x0800)
	ROM_CONTINUE(0x4000, 0x0800)
	ROM_CONTINUE(0x6000, 0x0800)
	ROM_LOAD ( "bbz-2.fil", 0x0800, 0x0800, CRC(dbec92ae) SHA1(7a1c6e5ac81d3cfcbb135a1c8b69e55296fffcc5))
	ROM_CONTINUE(0x2800, 0x0800)
	ROM_CONTINUE(0x1000, 0x0800)
	ROM_CONTINUE(0x3000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("bbelt_it.1e", 0xa000, 0x2000, CRC(fab5b89f) SHA1(9a2c2ae0a2035762b11cbd84fe3cddbde4572f18))
	ROM_LOAD("bbz-f.snd", 0xc000, 0x4000, CRC(9f58f369) SHA1(32472d93284c0f1fc2875714b40428406dcf6325))
ROM_END

ROM_START(bbeltzacg)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD ( "bbz-1.fil", 0x0000, 0x0800, CRC(2e7e1575) SHA1(1b9e6e4ff461962f4c7249bd2a748444cb658c30))
	ROM_CONTINUE(0x2000, 0x0800)
	ROM_CONTINUE(0x4000, 0x0800)
	ROM_CONTINUE(0x6000, 0x0800)
	ROM_LOAD ( "bbz-2.fil", 0x0800, 0x0800, CRC(dbec92ae) SHA1(7a1c6e5ac81d3cfcbb135a1c8b69e55296fffcc5))
	ROM_CONTINUE(0x2800, 0x0800)
	ROM_CONTINUE(0x1000, 0x0800)
	ROM_CONTINUE(0x3000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("bbelt_de.1e", 0xa000, 0x2000, CRC(f343103d) SHA1(d0ee91c873a10049f9aae6e762637d0384ff052a))
	ROM_LOAD("bbz-f.snd", 0xc000, 0x4000, CRC(9f58f369) SHA1(32472d93284c0f1fc2875714b40428406dcf6325))
ROM_END

ROM_START(bbeltzacf)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD ( "bbz-1.fil", 0x0000, 0x0800, CRC(2e7e1575) SHA1(1b9e6e4ff461962f4c7249bd2a748444cb658c30))
	ROM_CONTINUE(0x2000, 0x0800)
	ROM_CONTINUE(0x4000, 0x0800)
	ROM_CONTINUE(0x6000, 0x0800)
	ROM_LOAD ( "bbz-2.fil", 0x0800, 0x0800, CRC(dbec92ae) SHA1(7a1c6e5ac81d3cfcbb135a1c8b69e55296fffcc5))
	ROM_CONTINUE(0x2800, 0x0800)
	ROM_CONTINUE(0x1000, 0x0800)
	ROM_CONTINUE(0x3000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("bbelt_fr.1e", 0xa000, 0x2000, CRC(81e89d96) SHA1(f38610ffc2b12601b2b2f6871645bd4186d9b229))
	ROM_LOAD("bbz-f.snd", 0xc000, 0x4000, CRC(9f58f369) SHA1(32472d93284c0f1fc2875714b40428406dcf6325))
ROM_END

/*--------------------------------
/ Clown (07/85)
/-------------------------------*/
ROM_START(clown)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD ( "clown_1.lgc", 0x0000, 0x0800, CRC(16f09833) SHA1(5c9c8b9403d8b69ae7252bf904edc617784b8165))
	ROM_CONTINUE(0x2000, 0x0800)
	ROM_LOAD ( "clown_2.lgc", 0x0800, 0x0800, CRC(697e6b5b) SHA1(d2c459cbffec94730eb2abe3c63b4913a18085a7))
	ROM_CONTINUE(0x2800, 0x0800)
	ROM_CONTINUE(0x1000, 0x0800)
	ROM_CONTINUE(0x3000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("clown_e.snd", 0xc000, 0x2000, CRC(04a34cc1) SHA1(56fcc07ccab3cac27928f5c5411868bde1769603))
	ROM_LOAD("clown_f.snd", 0xe000, 0x2000, CRC(e35a4f72) SHA1(0037c1072f58798ba61af85a1b4b374b85c883ae))
ROM_END

/*--------------------------------
/ Devil Riders (04/84)
/-------------------------------*/
ROM_START(dvlrider)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD ( "cpu.ic1", 0x0000, 0x0800, CRC(5874ab12) SHA1(e616193943797d91e5cf2abfcc052821d24336b4))
	ROM_CONTINUE(0x2000, 0x0800)
	ROM_CONTINUE(0x4000, 0x0800)
	ROM_CONTINUE(0x6000, 0x0800)
	ROM_LOAD ( "cpu.ic2", 0x0800, 0x0800, CRC(09829446) SHA1(dc82135eae544f8eb1a3227bc6de0bd9a464e778))
	ROM_CONTINUE(0x2800, 0x0800)
	ROM_CONTINUE(0x1000, 0x0800)
	ROM_CONTINUE(0x3000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("gb01snd1.1d", 0xe000, 0x2000, CRC(5d48462c) SHA1(755bc259e992a9b375bd1e338775da14c15932bd))
	ROM_LOAD("gb01snd2.1e", 0xc000, 0x2000, CRC(1127be59) SHA1(be074fe3efecd0c1e10599c8981bf7c5debb4d37))
	ROM_LOAD("gb01snd3.1g", 0xa000, 0x2000, CRC(1ae91ae8) SHA1(05bcc7e509beb5fc2510bca99c39af0bc02530a7))
ROM_END

ROM_START(dvlrideri)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD ( "cpu.ic1", 0x0000, 0x0800, CRC(5874ab12) SHA1(e616193943797d91e5cf2abfcc052821d24336b4))
	ROM_CONTINUE(0x2000, 0x0800)
	ROM_CONTINUE(0x4000, 0x0800)
	ROM_CONTINUE(0x6000, 0x0800)
	ROM_LOAD ( "cpu.ic2", 0x0800, 0x0800, CRC(09829446) SHA1(dc82135eae544f8eb1a3227bc6de0bd9a464e778))
	ROM_CONTINUE(0x2800, 0x0800)
	ROM_CONTINUE(0x1000, 0x0800)
	ROM_CONTINUE(0x3000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("dride_it.1d", 0xe000, 0x2000, CRC(cc33b947) SHA1(1b240ed6b38a78e21c5009342c4abab8bfd9ff7e))
	ROM_LOAD("dride_it.1e", 0xc000, 0x2000, CRC(b3764fd7) SHA1(27b5332af1aaedfc36d942f78146baa85617dbbe))
	ROM_LOAD("dride_it.1g", 0xa000, 0x2000, CRC(04b6ee80) SHA1(03157af1b4c7c8e882e7a482b3313584418d2d9a))
ROM_END

ROM_START(dvlriderg)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD ( "cpu.ic1", 0x0000, 0x0800, CRC(5874ab12) SHA1(e616193943797d91e5cf2abfcc052821d24336b4))
	ROM_CONTINUE(0x2000, 0x0800)
	ROM_CONTINUE(0x4000, 0x0800)
	ROM_CONTINUE(0x6000, 0x0800)
	ROM_LOAD ( "cpu.ic2", 0x0800, 0x0800, CRC(09829446) SHA1(dc82135eae544f8eb1a3227bc6de0bd9a464e778))
	ROM_CONTINUE(0x2800, 0x0800)
	ROM_CONTINUE(0x1000, 0x0800)
	ROM_CONTINUE(0x3000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("g_snd_1.bin", 0xe000, 0x2000, CRC(77d042dc) SHA1(78e056468887a315e29c913803e3c36f9c7f694e))
	ROM_LOAD("g_snd_2.bin", 0xc000, 0x2000, CRC(31e35fd4) SHA1(2eeefbd831159d975fe9cac99db99dfdca04b0dc))
	ROM_LOAD("g_snd_3.bin", 0xa000, 0x2000, CRC(2e64a401) SHA1(694808963d6e6a02ddeb9228073825ff16f91d49))
ROM_END

ROM_START(dvlriderf)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD ( "cpu.ic1", 0x0000, 0x0800, CRC(5874ab12) SHA1(e616193943797d91e5cf2abfcc052821d24336b4))
	ROM_CONTINUE(0x2000, 0x0800)
	ROM_CONTINUE(0x4000, 0x0800)
	ROM_CONTINUE(0x6000, 0x0800)
	ROM_LOAD ( "cpu.ic2", 0x0800, 0x0800, CRC(09829446) SHA1(dc82135eae544f8eb1a3227bc6de0bd9a464e778))
	ROM_CONTINUE(0x2800, 0x0800)
	ROM_CONTINUE(0x1000, 0x0800)
	ROM_CONTINUE(0x3000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("fr01snd1.1d", 0xe000, 0x2000, CRC(ba435706) SHA1(a05fb75aa135d02e9f97024524fe780d48d92be0))
	ROM_LOAD("fr01snd2.1e", 0xc000, 0x2000, NO_DUMP)
	ROM_LOAD("fr01snd3.1g", 0xa000, 0x2000, CRC(f707ad05) SHA1(0e3254ccf901181854f2e31c7339bd9e21d7d0bd))
ROM_END

/*--------------------------------
/ Farfalla (09/83)
/-------------------------------*/
ROM_START(farfalla)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD ( "cpurom1.bin", 0x0000, 0x0800, CRC(ac249150) SHA1(9eac1bf6119cd1fa6cc823faf02b9bf153519a77))
	ROM_CONTINUE(0x2000, 0x0800)
	ROM_CONTINUE(0x4000, 0x0800)
	ROM_CONTINUE(0x6000, 0x0800)
	ROM_LOAD ( "cpurom2.bin", 0x0800, 0x0800, CRC(6edc823f) SHA1(b10fcbc308ec06762a2eb35921a7e6a68fd5c9b1))
	ROM_CONTINUE(0x2800, 0x0800)
	ROM_CONTINUE(0x1000, 0x0800)
	ROM_CONTINUE(0x3000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("rom1.snd", 0xe000, 0x2000, CRC(aca09674) SHA1(8e1edc25c7fe2189215f73da8f1bec4b670bd8e6))
	ROM_LOAD("rom2.snd", 0xc000, 0x1000, CRC(76da384d) SHA1(0e4616bf2fb2c21270aecfc04ad9e68ce9390bfb))
	ROM_RELOAD(0xd000, 0x1000)
	ROM_LOAD("rom3.snd", 0xa000, 0x2000, CRC(d0584952) SHA1(80fe571a2e8a2a34fae03589df930b3eb3fa1f6b))
ROM_END

ROM_START(farfallai)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD ( "cpurom1.bin", 0x0000, 0x0800, CRC(ac249150) SHA1(9eac1bf6119cd1fa6cc823faf02b9bf153519a77))
	ROM_CONTINUE(0x2000, 0x0800)
	ROM_CONTINUE(0x4000, 0x0800)
	ROM_CONTINUE(0x6000, 0x0800)
	ROM_LOAD ( "cpurom2.bin", 0x0800, 0x0800, CRC(6edc823f) SHA1(b10fcbc308ec06762a2eb35921a7e6a68fd5c9b1))
	ROM_CONTINUE(0x2800, 0x0800)
	ROM_CONTINUE(0x1000, 0x0800)
	ROM_CONTINUE(0x3000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("farsnd1.bin", 0xe000, 0x2000, CRC(fd80040d) SHA1(122c99627d944b253e091b56d32336367df615c1))
	ROM_LOAD("rom2.snd", 0xc000, 0x1000, CRC(76da384d) SHA1(0e4616bf2fb2c21270aecfc04ad9e68ce9390bfb))
	ROM_RELOAD(0xd000, 0x1000)
	ROM_LOAD("farsnd3.bin", 0xa000, 0x2000, CRC(b58618c2) SHA1(89330ee928b5a5f99d50f1150c94732775907fd8))
ROM_END

ROM_START(farfallag)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD ( "cpurom1.bin", 0x0000, 0x0800, CRC(ac249150) SHA1(9eac1bf6119cd1fa6cc823faf02b9bf153519a77))
	ROM_CONTINUE(0x2000, 0x0800)
	ROM_CONTINUE(0x4000, 0x0800)
	ROM_CONTINUE(0x6000, 0x0800)
	ROM_LOAD ( "cpurom2.bin", 0x0800, 0x0800, CRC(6edc823f) SHA1(b10fcbc308ec06762a2eb35921a7e6a68fd5c9b1))
	ROM_CONTINUE(0x2800, 0x0800)
	ROM_CONTINUE(0x1000, 0x0800)
	ROM_CONTINUE(0x3000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("farf_de.1d", 0xe000, 0x2000, CRC(5f64df81) SHA1(d8bd6d1fb3eec704fe31ccc1feeb5a9529c70d07))
	ROM_LOAD("rom2.snd", 0xc000, 0x1000, CRC(76da384d) SHA1(0e4616bf2fb2c21270aecfc04ad9e68ce9390bfb))
	ROM_RELOAD(0xd000, 0x1000)
	ROM_LOAD("farf_de.1g", 0xa000, 0x2000, CRC(0500d468) SHA1(f7dfc6f52e4db1d0d42edb646d719badbcee8ef0))
ROM_END

ROM_START(farfallaf)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD ( "cpurom1.bin", 0x0000, 0x0800, CRC(ac249150) SHA1(9eac1bf6119cd1fa6cc823faf02b9bf153519a77))
	ROM_CONTINUE(0x2000, 0x0800)
	ROM_CONTINUE(0x4000, 0x0800)
	ROM_CONTINUE(0x6000, 0x0800)
	ROM_LOAD ( "cpurom2.bin", 0x0800, 0x0800, CRC(6edc823f) SHA1(b10fcbc308ec06762a2eb35921a7e6a68fd5c9b1))
	ROM_CONTINUE(0x2800, 0x0800)
	ROM_CONTINUE(0x1000, 0x0800)
	ROM_CONTINUE(0x3000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("farf_fr.1d", 0xe000, 0x2000, CRC(8c8f0d22) SHA1(442ea7bd239f6b7828ecec2188739a8cc1605be8))
	ROM_LOAD("rom2.snd", 0xc000, 0x1000, CRC(76da384d) SHA1(0e4616bf2fb2c21270aecfc04ad9e68ce9390bfb))
	ROM_RELOAD(0xd000, 0x1000)
	ROM_LOAD("farf_fr.1g", 0xa000, 0x2000, CRC(581d0513) SHA1(ea861a0299a556c6c3dd1189c53f9e6d99140189))
ROM_END

/*--------------------------------
/ Magic Castle (09/84)
/-------------------------------*/
ROM_START(mcastle)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD ( "cpu.ic1", 0x0000, 0x0800, CRC(50dd8209) SHA1(c1df8ea16d8a8ae1d6f524fa25c94c4664f314c1))
	ROM_CONTINUE(0x2000, 0x0800)
	ROM_LOAD ( "cpu.ic2", 0x0800, 0x0800, CRC(11372bec) SHA1(bd822c0ee455941630cdade83335c84224d351e4))
	ROM_CONTINUE(0x2800, 0x0800)
	ROM_CONTINUE(0x1000, 0x0800)
	ROM_CONTINUE(0x3000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("gb01snd1.1d", 0xe000, 0x2000, CRC(cd6a4a07) SHA1(47359747f79feca9d85d8f7657325874eda0f915))
	ROM_LOAD("gb01snd2.1e", 0xc000, 0x2000, CRC(d289952d) SHA1(94052cbee4cd499fb55d59b047828c21d71ab288))
	ROM_LOAD("gb01snd3.1g", 0xa000, 0x2000, CRC(8b4342eb) SHA1(a8534cb2ebaff4d5d4101eb710c068f3b91e9e0c))
ROM_END

ROM_START(mcastlei)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD ( "cpu.ic1", 0x0000, 0x0800, CRC(50dd8209) SHA1(c1df8ea16d8a8ae1d6f524fa25c94c4664f314c1))
	ROM_CONTINUE(0x2000, 0x0800)
	ROM_LOAD ( "cpu.ic2", 0x0800, 0x0800, CRC(11372bec) SHA1(bd822c0ee455941630cdade83335c84224d351e4))
	ROM_CONTINUE(0x2800, 0x0800)
	ROM_CONTINUE(0x1000, 0x0800)
	ROM_CONTINUE(0x3000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("mgic_it.1d", 0xe000, 0x2000, CRC(16911674) SHA1(8fc5b0ec48c76eac21bcab44cf2fe9635e55ef49))
	ROM_LOAD("mgic_it.1e", 0xc000, 0x2000, CRC(646f9673) SHA1(cf78029f63c8264db2d0012143981d36b5410499))
	ROM_LOAD("mgic_it.1g", 0xa000, 0x2000, CRC(ffef01b2) SHA1(0c8a549432f3aed4b17eb5e3b6917f557d3f6050))
ROM_END

ROM_START(mcastleg)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD ( "cpu.ic1", 0x0000, 0x0800, CRC(50dd8209) SHA1(c1df8ea16d8a8ae1d6f524fa25c94c4664f314c1))
	ROM_CONTINUE(0x2000, 0x0800)
	ROM_LOAD ( "cpu.ic2", 0x0800, 0x0800, CRC(11372bec) SHA1(bd822c0ee455941630cdade83335c84224d351e4))
	ROM_CONTINUE(0x2800, 0x0800)
	ROM_CONTINUE(0x1000, 0x0800)
	ROM_CONTINUE(0x3000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("magic1d.snd", 0xe000, 0x2000, CRC(1f1a1140) SHA1(fed351c78e4c46c05e910f1844351492faa9edcf))
	ROM_LOAD("magic1e.snd", 0xc000, 0x2000, CRC(a8787011) SHA1(16da0b40e24346f4e90d553c7c3e68daa4d4a656))
	ROM_LOAD("magic1g.snd", 0xa000, 0x2000, CRC(313fb216) SHA1(1065f057654dd41cdac0553e1c315edf141f1d19))
ROM_END

ROM_START(mcastlef)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD ( "cpu.ic1", 0x0000, 0x0800, CRC(50dd8209) SHA1(c1df8ea16d8a8ae1d6f524fa25c94c4664f314c1))
	ROM_CONTINUE(0x2000, 0x0800)
	ROM_LOAD ( "cpu.ic2", 0x0800, 0x0800, CRC(11372bec) SHA1(bd822c0ee455941630cdade83335c84224d351e4))
	ROM_CONTINUE(0x2800, 0x0800)
	ROM_CONTINUE(0x1000, 0x0800)
	ROM_CONTINUE(0x3000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("mgic_fr.1d", 0xe000, 0x2000, CRC(7d3faa3b) SHA1(4f9ab1a868f7b9900bbbde02c2e654e7f778ed9d))
	ROM_LOAD("mgic_fr.1e", 0xc000, 0x2000, CRC(0077241c) SHA1(113d9039ad14f3887533f5e655a7912ddd441e77))
	ROM_LOAD("mgic_fr.1g", 0xa000, 0x2000, CRC(12d8b4f6) SHA1(a1b428e36c9d14bfb5b258a1c10ab1d02b502b56))
ROM_END

/*--------------------------------
/ Mexico '86 (07/86)
/-------------------------------*/
ROM_START(mexico)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD ( "mex86_1.lgc", 0x0000, 0x0800, CRC(60d559b1) SHA1(1097f32dd0c89b6e3653a620e39696d8ab1289fc))
	ROM_CONTINUE(0x4000, 0x0800)
	ROM_CONTINUE(0x6000, 0x0800)
	ROM_CONTINUE(0x2000, 0x0800)
	ROM_LOAD ( "mex86_2.lgc", 0x0800, 0x0800, CRC(5c984c15) SHA1(c6228568cee6a365a3c552a57e5e1e0445108bad))
	ROM_CONTINUE(0x2800, 0x0800)
	ROM_CONTINUE(0x1000, 0x0800)
	ROM_CONTINUE(0x3000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("mex86_e.snd", 0xa000, 0x2000, CRC(a985e8db) SHA1(11f91179fa1d46c1c83cdd4fbcf8ebdfd2a41f3f))
	ROM_LOAD("mex86_f.snd", 0xc000, 0x4000, CRC(301c2b63) SHA1(df4a4cb48d28d53c3728066d3e3fa9eac17c78c5))
ROM_END

ROM_START(mexicoi)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD ( "mex86_1.lgc", 0x0000, 0x0800, CRC(60d559b1) SHA1(1097f32dd0c89b6e3653a620e39696d8ab1289fc))
	ROM_CONTINUE(0x4000, 0x0800)
	ROM_CONTINUE(0x6000, 0x0800)
	ROM_CONTINUE(0x2000, 0x0800)
	ROM_LOAD ( "mex86_2.lgc", 0x0800, 0x0800, CRC(5c984c15) SHA1(c6228568cee6a365a3c552a57e5e1e0445108bad))
	ROM_CONTINUE(0x2800, 0x0800)
	ROM_CONTINUE(0x1000, 0x0800)
	ROM_CONTINUE(0x3000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("mex86_it.snd", 0xa000, 0x2000, CRC(18ceec3a) SHA1(074429ea8610c9fc423ba0e23e557223137c768d))
	ROM_LOAD("mex86_f.snd",  0xc000, 0x4000, CRC(301c2b63) SHA1(df4a4cb48d28d53c3728066d3e3fa9eac17c78c5))
ROM_END

/*------------------------------------------------------------------------------------
/ New Star's Phoenix (08/87) same roms as Star's Phoenix, playboard slightly changed
/------------------------------------------------------------------------------------*/
ROM_START(nstrphnx)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD ( "strphnx1.cpu", 0x0000, 0x0800, CRC(2a31b7da) SHA1(05f2173783e686cc8774bed6eb59b41f7af88d11))
	ROM_CONTINUE(0x2000, 0x0800)
	ROM_CONTINUE(0x4000, 0x0800)
	ROM_CONTINUE(0x6000, 0x0800)
	ROM_LOAD ( "strphnx2.cpu", 0x0800, 0x0800, CRC(db830505) SHA1(55d6d6e12e2861fec81b46fb90c29aad5ad922aa))
	ROM_CONTINUE(0x2800, 0x0800)
	ROM_CONTINUE(0x1000, 0x0800)
	ROM_CONTINUE(0x3000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("snd_ic24.bin", 0x0000, 0x8000, CRC(158d6f83) SHA1(281e1b13be43025be1b33dcd366cec0b36f29e5c))
	ROM_LOAD("snd_ic25.bin", 0x8000, 0x8000, CRC(b1c9238e) SHA1(88c9df1fca94d32a0fa5d75312dabff257e867dd))

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("snd_ic05.bin", 0x0000, 0x8000, CRC(74cc4902) SHA1(e2f46bcf5446f98d098c49f8c2416292401265b9))
	ROM_LOAD("snd_ic06.bin", 0x8000, 0x8000, CRC(a0400411) SHA1(da9de6105639c4f6174f5bc92f44e02c339a2bc3))

	ROM_REGION(0x10000, "cpu4", 0)
	ROM_LOAD("snd_ic40.bin", 0x0000, 0x8000, CRC(974ceb9c) SHA1(3665af9170a2afbe26f68e8f3cedb0d177f476c4))
ROM_END

ROM_START(nstrphnxf)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD ( "strphnx1.cpu", 0x0000, 0x0800, CRC(2a31b7da) SHA1(05f2173783e686cc8774bed6eb59b41f7af88d11))
	ROM_CONTINUE(0x2000, 0x0800)
	ROM_CONTINUE(0x4000, 0x0800)
	ROM_CONTINUE(0x6000, 0x0800)
	ROM_LOAD ( "strphnx2.cpu", 0x0800, 0x0800, CRC(db830505) SHA1(55d6d6e12e2861fec81b46fb90c29aad5ad922aa))
	ROM_CONTINUE(0x2800, 0x0800)
	ROM_CONTINUE(0x1000, 0x0800)
	ROM_CONTINUE(0x3000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("snd_ic24.bin", 0x0000, 0x8000, CRC(158d6f83) SHA1(281e1b13be43025be1b33dcd366cec0b36f29e5c))
	ROM_LOAD("snd_ic25.bin", 0x8000, 0x8000, CRC(b1c9238e) SHA1(88c9df1fca94d32a0fa5d75312dabff257e867dd))

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("snd_ic05.bin", 0x0000, 0x8000, CRC(74cc4902) SHA1(e2f46bcf5446f98d098c49f8c2416292401265b9))
	ROM_LOAD("snd_ic06.bin", 0x8000, 0x8000, CRC(a0400411) SHA1(da9de6105639c4f6174f5bc92f44e02c339a2bc3))

	ROM_REGION(0x10000, "cpu4", 0)
	ROM_LOAD("sndf_ic40.bin", 0x0000, 0x8000, CRC(1b40de42) SHA1(cfebab38a493edab1dfd0e5f591ccb9658da80c6))
ROM_END

/*--------------------------------
/ Pinball Champ (04/83)
/-------------------------------*/
ROM_START(pinchamp)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD ( "pinchamp.ic1", 0x0000, 0x0800, CRC(1412ec33) SHA1(82c158ec0536f76cbe80e8c12e0047579439a5b7))
	ROM_CONTINUE(0x2000, 0x0800)
	ROM_LOAD ( "pinchamp.ic2", 0x0800, 0x0800, CRC(a24ba4c6) SHA1(4f02c4d6cd727fa96a68c72012b0b4a4484397c4))
	ROM_CONTINUE(0x2800, 0x0800)
	ROM_LOAD ( "pinchamp.ic3", 0x1000, 0x0800, CRC(df5f4f88) SHA1(249cf958b0998aa41fa26c617be9b6c52c2f5549))
	ROM_CONTINUE(0x3000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("pchmp_gb.1c", 0xf000, 0x1000, CRC(f739fcba) SHA1(7460f1da99c474601e8cec64683cbd61837a82e8))
	ROM_LOAD("pchmp_gb.1e", 0xe000, 0x1000, CRC(24d83e74) SHA1(f78e151c9885b965cd5209777580414522362ebf))
	ROM_LOAD("pchmp_gb.1f", 0xd000, 0x1000, CRC(d055e8c6) SHA1(0820d941880aa8925b400c792af7ce6b80dcbc48))
	ROM_LOAD("pchmp_gb.1g", 0xc000, 0x0800, CRC(39b68215) SHA1(4d57f1f1f71f7bdbef67ca4cc62cfde80d1ab04c))
ROM_END

ROM_START(pinchampf)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD ( "pinchamp.ic1", 0x0000, 0x0800, CRC(1412ec33) SHA1(82c158ec0536f76cbe80e8c12e0047579439a5b7))
	ROM_CONTINUE(0x2000, 0x0800)
	ROM_LOAD ( "pinchamp.ic2", 0x0800, 0x0800, CRC(a24ba4c6) SHA1(4f02c4d6cd727fa96a68c72012b0b4a4484397c4))
	ROM_CONTINUE(0x2800, 0x0800)
	ROM_LOAD ( "pinchamp.ic3", 0x1000, 0x0800, CRC(df5f4f88) SHA1(249cf958b0998aa41fa26c617be9b6c52c2f5549))
	ROM_CONTINUE(0x3000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("pchmp_fr.1c", 0xf000, 0x1000, CRC(fcaa6907) SHA1(9c6a3c88754651fd3972bad7c3a920cd4fa885ce))
	ROM_LOAD("pchmp_fr.1e", 0xe000, 0x1000, CRC(24d83e74) SHA1(f78e151c9885b965cd5209777580414522362ebf))
	ROM_LOAD("pchmp_fr.1f", 0xd000, 0x1000, CRC(1cd8398c) SHA1(cc72bdce47d8f7189fe315e44cb0af976fc25d65))
	ROM_LOAD("pchmp_fr.1g", 0xc000, 0x1000, CRC(e8f5cdcb) SHA1(b94dd696cf1688ede90c54e9b55a8d6ec546ecba))
ROM_END

ROM_START(pinchampg)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD ( "pinchamp.ic1", 0x0000, 0x0800, CRC(1412ec33) SHA1(82c158ec0536f76cbe80e8c12e0047579439a5b7))
	ROM_CONTINUE(0x2000, 0x0800)
	ROM_LOAD ( "pinchamp.ic2", 0x0800, 0x0800, CRC(a24ba4c6) SHA1(4f02c4d6cd727fa96a68c72012b0b4a4484397c4))
	ROM_CONTINUE(0x2800, 0x0800)
	ROM_LOAD ( "pinchamp.ic3", 0x1000, 0x0800, CRC(df5f4f88) SHA1(249cf958b0998aa41fa26c617be9b6c52c2f5549))
	ROM_CONTINUE(0x3000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("pchmp_de.1c", 0xf000, 0x1000, CRC(6e2defe5) SHA1(fcb62da1aed23d9fb9a222862b4b772aad9792a1))
	ROM_LOAD("pchmp_de.1e", 0xe000, 0x1000, CRC(703b3cae) SHA1(c7bd021e936fb0fd4bc16d48c3ef1df69d1fe01a))
	ROM_LOAD("pchmp_de.1f", 0xd000, 0x1000, CRC(f3f4b950) SHA1(ed5c02f701530d2d6255cc72d695e24d4df40fc3))
	ROM_LOAD("pchmp_de.1g", 0xc000, 0x0800, CRC(44adae13) SHA1(0d8d538704db62b41ad5781ec53c34e482342025))
ROM_END

ROM_START(pinchampi)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD ( "pinchamp.ic1", 0x0000, 0x0800, CRC(1412ec33) SHA1(82c158ec0536f76cbe80e8c12e0047579439a5b7))
	ROM_CONTINUE(0x2000, 0x0800)
	ROM_LOAD ( "pinchamp.ic2", 0x0800, 0x0800, CRC(a24ba4c6) SHA1(4f02c4d6cd727fa96a68c72012b0b4a4484397c4))
	ROM_CONTINUE(0x2800, 0x0800)
	ROM_LOAD ( "pinchamp.ic3", 0x1000, 0x0800, CRC(df5f4f88) SHA1(249cf958b0998aa41fa26c617be9b6c52c2f5549))
	ROM_CONTINUE(0x3000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("pchmp_it.1c", 0xf000, 0x1000, CRC(a0033b90) SHA1(bca8fe29fdfcbc22fd0e8bafbd7946db5c2c4041))
	ROM_LOAD("pchmp_gb.1e", 0xe000, 0x1000, CRC(24d83e74) SHA1(f78e151c9885b965cd5209777580414522362ebf))
	ROM_LOAD("pchmp_it.1f", 0xd000, 0x1000, CRC(5555f341) SHA1(8aa27d17711f4162c9d10f60afba7f823112bfe0))
	ROM_LOAD("pchmp_it.1g", 0xc000, 0x1000, CRC(2561579b) SHA1(a280cd81f58a17601adfa9ce17f225111c7d9f95))
ROM_END

ROM_START(pinchamp7)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD ( "pblchmp7.ic1", 0x0000, 0x0800, CRC(f050b7fa) SHA1(918bdfd77e785c546202c29b1e296ca5f683ca66))
	ROM_CONTINUE(0x2000, 0x0800)
	ROM_LOAD ( "pblchmp7.ic2", 0x0800, 0x0800, CRC(cbcb63c7) SHA1(c15329482f02614185adcd0475a02c667cadfc98))
	ROM_CONTINUE(0x2800, 0x0800)
	ROM_LOAD ( "pblchmp7.ic3", 0x1000, 0x0800, CRC(54abff9c) SHA1(925c7c1fb903bd6069aee1967c75eb8e61ecf591))
	ROM_CONTINUE(0x3000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("pchmp_gb.1c", 0xf000, 0x1000, CRC(f739fcba) SHA1(7460f1da99c474601e8cec64683cbd61837a82e8))
	ROM_LOAD("pchmp_gb.1e", 0xe000, 0x1000, CRC(24d83e74) SHA1(f78e151c9885b965cd5209777580414522362ebf))
	ROM_LOAD("pchmp_gb.1f", 0xd000, 0x1000, CRC(d055e8c6) SHA1(0820d941880aa8925b400c792af7ce6b80dcbc48))
	ROM_LOAD("pchmp_gb.1g", 0xc000, 0x0800, CRC(39b68215) SHA1(4d57f1f1f71f7bdbef67ca4cc62cfde80d1ab04c))
ROM_END

ROM_START(pinchamp7f)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD ( "pblchmp7.ic1", 0x0000, 0x0800, CRC(f050b7fa) SHA1(918bdfd77e785c546202c29b1e296ca5f683ca66))
	ROM_CONTINUE(0x2000, 0x0800)
	ROM_LOAD ( "pblchmp7.ic2", 0x0800, 0x0800, CRC(cbcb63c7) SHA1(c15329482f02614185adcd0475a02c667cadfc98))
	ROM_CONTINUE(0x2800, 0x0800)
	ROM_LOAD ( "pblchmp7.ic3", 0x1000, 0x0800, CRC(54abff9c) SHA1(925c7c1fb903bd6069aee1967c75eb8e61ecf591))
	ROM_CONTINUE(0x3000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("pchmp_fr.1c", 0xf000, 0x1000, CRC(fcaa6907) SHA1(9c6a3c88754651fd3972bad7c3a920cd4fa885ce))
	ROM_LOAD("pchmp_fr.1e", 0xe000, 0x1000, CRC(24d83e74) SHA1(f78e151c9885b965cd5209777580414522362ebf))
	ROM_LOAD("pchmp_fr.1f", 0xd000, 0x1000, CRC(1cd8398c) SHA1(cc72bdce47d8f7189fe315e44cb0af976fc25d65))
	ROM_LOAD("pchmp_fr.1g", 0xc000, 0x1000, CRC(e8f5cdcb) SHA1(b94dd696cf1688ede90c54e9b55a8d6ec546ecba))
ROM_END

ROM_START(pinchamp7g)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD ( "pblchmp7.ic1", 0x0000, 0x0800, CRC(f050b7fa) SHA1(918bdfd77e785c546202c29b1e296ca5f683ca66))
	ROM_CONTINUE(0x2000, 0x0800)
	ROM_LOAD ( "pblchmp7.ic2", 0x0800, 0x0800, CRC(cbcb63c7) SHA1(c15329482f02614185adcd0475a02c667cadfc98))
	ROM_CONTINUE(0x2800, 0x0800)
	ROM_LOAD ( "pblchmp7.ic3", 0x1000, 0x0800, CRC(54abff9c) SHA1(925c7c1fb903bd6069aee1967c75eb8e61ecf591))
	ROM_CONTINUE(0x3000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("pchmp_de.1c", 0xf000, 0x1000, CRC(6e2defe5) SHA1(fcb62da1aed23d9fb9a222862b4b772aad9792a1))
	ROM_LOAD("pchmp_de.1e", 0xe000, 0x1000, CRC(703b3cae) SHA1(c7bd021e936fb0fd4bc16d48c3ef1df69d1fe01a))
	ROM_LOAD("pchmp_de.1f", 0xd000, 0x1000, CRC(f3f4b950) SHA1(ed5c02f701530d2d6255cc72d695e24d4df40fc3))
	ROM_LOAD("pchmp_de.1g", 0xc000, 0x0800, CRC(44adae13) SHA1(0d8d538704db62b41ad5781ec53c34e482342025))
ROM_END

ROM_START(pinchamp7i)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD ( "pblchmp7.ic1", 0x0000, 0x0800, CRC(f050b7fa) SHA1(918bdfd77e785c546202c29b1e296ca5f683ca66))
	ROM_CONTINUE(0x2000, 0x0800)
	ROM_LOAD ( "pblchmp7.ic2", 0x0800, 0x0800, CRC(cbcb63c7) SHA1(c15329482f02614185adcd0475a02c667cadfc98))
	ROM_CONTINUE(0x2800, 0x0800)
	ROM_LOAD ( "pblchmp7.ic3", 0x1000, 0x0800, CRC(54abff9c) SHA1(925c7c1fb903bd6069aee1967c75eb8e61ecf591))
	ROM_CONTINUE(0x3000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("pchmp_it.1c", 0xf000, 0x1000, CRC(a0033b90) SHA1(bca8fe29fdfcbc22fd0e8bafbd7946db5c2c4041))
	ROM_LOAD("pchmp_gb.1e", 0xe000, 0x1000, CRC(24d83e74) SHA1(f78e151c9885b965cd5209777580414522362ebf))
	ROM_LOAD("pchmp_it.1f", 0xd000, 0x1000, CRC(5555f341) SHA1(8aa27d17711f4162c9d10f60afba7f823112bfe0))
	ROM_LOAD("pchmp_it.1g", 0xc000, 0x1000, CRC(2561579b) SHA1(a280cd81f58a17601adfa9ce17f225111c7d9f95))
ROM_END

// Pinball Champ '82 (04/82) - using the same roms as Pinball Champ

/*--------------------------------
/ Pool Champion (12/85)
/-------------------------------*/
ROM_START(poolcham)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD ( "poolcham.ic1", 0x0000, 0x0800, CRC(fca2a2b2) SHA1(9a0d9c495e38628c5e0bc10f6335100eb934f153))
	ROM_CONTINUE(0x2000, 0x0800)
	ROM_LOAD ( "poolcham.ic2", 0x0800, 0x0800, CRC(267a2a02) SHA1(049ada7bfcf0d8560ac03effd3fbb02ead51933c))
	ROM_CONTINUE(0x2800, 0x0800)
	ROM_CONTINUE(0x1000, 0x0800)
	ROM_CONTINUE(0x3000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("poolcham.1f", 0xc000, 0x4000, CRC(efe33926) SHA1(30444a2ee7f453f46c74fff8365d80fc4f0a277f))
ROM_END

ROM_START(poolchami)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD ( "poolchai.ic1", 0x0000, 0x0800, CRC(fca2a2b2) SHA1(9a0d9c495e38628c5e0bc10f6335100eb934f153))
	ROM_CONTINUE(0x2000, 0x0800)
	ROM_LOAD ( "poolchai.ic2", 0x0800, 0x0800, CRC(267a2a02) SHA1(049ada7bfcf0d8560ac03effd3fbb02ead51933c))
	ROM_CONTINUE(0x2800, 0x0800)
	ROM_CONTINUE(0x1000, 0x0800)
	ROM_CONTINUE(0x3000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("poolc_it.1e", 0xa000, 0x2000, CRC(28a3e5ee) SHA1(c090c81c78d3296e91ce12e1170ee2c71ba07177))
	ROM_LOAD("sound1.f", 0xc000, 0x4000, CRC(b4b4e31e) SHA1(bcd1c4c7f6f079655a9c37d0b978d997f95b93ad))
ROM_END

/*--------------------------------
/ Robot (01/85)
/-------------------------------*/
ROM_START(robot)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD ( "robot_1.lgc", 0x0000, 0x0800, CRC(5e754418) SHA1(81a25ef85147d8c043b7d243d9d0d3e8bf90f852))
	ROM_CONTINUE(0x2000, 0x0800)
	ROM_LOAD ( "robot_2.lgc", 0x0800, 0x0800, CRC(28ba9687) SHA1(8e99834328783361856fa9632b2c6e3a5a05d49b))
	ROM_CONTINUE(0x2800, 0x0800)
	ROM_CONTINUE(0x1000, 0x0800)
	ROM_CONTINUE(0x3000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("robot_d.snd", 0xe000, 0x2000, CRC(ab5e5524) SHA1(9aae2560bccf64daeab0514c8934c55f77fe240d))
	ROM_LOAD("robot_e.snd", 0xc000, 0x2000, CRC(2f314e33) SHA1(1f92aff3d99c2e86820720a3290285b9f36cb15b))
	ROM_LOAD("robot_g.snd", 0xa000, 0x2000, CRC(6fb1caf5) SHA1(0bc6a6edaa9589b7d171f96fa74855a022c2b050))
ROM_END

ROM_START(roboti)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD ( "robot_1.lgc", 0x0000, 0x0800, CRC(5e754418) SHA1(81a25ef85147d8c043b7d243d9d0d3e8bf90f852))
	ROM_CONTINUE(0x2000, 0x0800)
	ROM_LOAD ( "robot_2.lgc", 0x0800, 0x0800, CRC(28ba9687) SHA1(8e99834328783361856fa9632b2c6e3a5a05d49b))
	ROM_CONTINUE(0x2800, 0x0800)
	ROM_CONTINUE(0x1000, 0x0800)
	ROM_CONTINUE(0x3000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("robot_it.1d", 0xe000, 0x2000, CRC(a4a20ed7) SHA1(459519e10bad59ba27cd5d5d31c5f276726c9bd0))
	ROM_LOAD("robot_it.1e", 0xc000, 0x2000, CRC(2f314e33) SHA1(1f92aff3d99c2e86820720a3290285b9f36cb15b))
	ROM_LOAD("robot_it.1g", 0xa000, 0x2000, CRC(6bce79ac) SHA1(f93871b050edebccca7f0265c3f5144e10b6cc79))
ROM_END

ROM_START(robotg)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD ( "robot_1.lgc", 0x0000, 0x0800, CRC(5e754418) SHA1(81a25ef85147d8c043b7d243d9d0d3e8bf90f852))
	ROM_CONTINUE(0x2000, 0x0800)
	ROM_LOAD ( "robot_2.lgc", 0x0800, 0x0800, CRC(28ba9687) SHA1(8e99834328783361856fa9632b2c6e3a5a05d49b))
	ROM_CONTINUE(0x2800, 0x0800)
	ROM_CONTINUE(0x1000, 0x0800)
	ROM_CONTINUE(0x3000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("robot_dg.snd", 0xe000, 0x2000, CRC(88685b1e) SHA1(7d49a1d42f3e07948390a00a562aeba9dd4ddeeb))
	ROM_LOAD("robot_eg.snd", 0xc000, 0x2000, CRC(e326a851) SHA1(c2bb5e329803922fa1c1ca30be6e3ae3d292135a))
	ROM_LOAD("robot_gg.snd", 0xa000, 0x2000, CRC(7ed5da55) SHA1(d70f1f470cf9d300375600352f9625b4e34f5ed3))
ROM_END

ROM_START(robotf)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD ( "robot_1.lgc", 0x0000, 0x0800, CRC(5e754418) SHA1(81a25ef85147d8c043b7d243d9d0d3e8bf90f852))
	ROM_CONTINUE(0x2000, 0x0800)
	ROM_LOAD ( "robot_2.lgc", 0x0800, 0x0800, CRC(28ba9687) SHA1(8e99834328783361856fa9632b2c6e3a5a05d49b))
	ROM_CONTINUE(0x2800, 0x0800)
	ROM_CONTINUE(0x1000, 0x0800)
	ROM_CONTINUE(0x3000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("robot_fr.1d", 0xe000, 0x2000, CRC(94957954) SHA1(22f729a1ca48399aa222f5037071d0482b9d59aa))
	ROM_LOAD("robot_fr.1e", 0xc000, 0x2000, CRC(fdcfff02) SHA1(1ef02ad646dfea1b9727a0a99e93db724cd38cce))
	ROM_LOAD("robot_fr.1g", 0xa000, 0x2000, CRC(ccf6413f) SHA1(71242e999985ee78ec0fb282e4de2f45c1867051))
ROM_END

/*--------------------------------
/ Scramble (Tecnoplay 1987)
/-------------------------------*/
ROM_START(scram_tp)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD ( "scram_1.lgc", 0x0000, 0x0800, CRC(da565549) SHA1(d187801428824df2b506c999548a5c6d146bc59e))
	ROM_CONTINUE(0x4000, 0x0800)
	ROM_CONTINUE(0x6000, 0x0800)
	ROM_CONTINUE(0x2000, 0x0800)
	ROM_LOAD ( "scram_2.lgc", 0x0800, 0x0800, CRC(537e6c61) SHA1(84e0db4268d3c990c3834ebd20bf7c475a70082d))
	ROM_CONTINUE(0x2800, 0x0800)
	ROM_CONTINUE(0x1000, 0x0800)
	ROM_CONTINUE(0x3000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("scram_1.snd", 0xe000, 0x2000, CRC(ee5f868b) SHA1(23ef4112b94109ad4d4a6b9bb5215acec20e5e55))

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("scram_2.snd", 0xe000, 0x2000, CRC(a04bf7d0) SHA1(5be5d445b199e7dc9d42e7ee5e9b31c18dec3881))
	ROM_REGION(0x20000, "cpu4", 0)
	ROM_LOAD("scram_5.snd", 0x8000, 0x8000, CRC(3aa782ec) SHA1(7cbbd3a737239b2755c6a6651a284e83fcfa22f6))
	ROM_LOAD("scram_4.snd", 0x10000, 0x8000, CRC(943f279d) SHA1(52767708d706a01ea16e37c866eb5762297e1f86))
	ROM_LOAD("scram_3.snd", 0x18000, 0x8000, CRC(ed27cd78) SHA1(a062ee1a3ec8819acddac13a4b454f5fd95d1e29))
ROM_END

/*--------------------------------
/ Soccer Kings (09/82)
/-------------------------------*/
ROM_START(socrking)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD ( "soccer.ic1", 0x0000, 0x0800, CRC(3fbd7c32) SHA1(2f56f67d1ad987638284000cca1e20ff17fcd4f9))
	ROM_CONTINUE(0x2000, 0x0800)
	ROM_LOAD ( "soccer.ic2", 0x0800, 0x0800, CRC(0cc0df1f) SHA1(2fd05af0ec63835a8f69fdc50e2faceb829b4df2))
	ROM_CONTINUE(0x2800, 0x0800)
	ROM_LOAD ( "soccer.ic3", 0x1000, 0x0800, CRC(72caac2c) SHA1(7d63e0cf699365ee1787004d6155646e715b672e))
	ROM_CONTINUE(0x3000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("sound1.c", 0xf000, 0x1000, CRC(3aa95018) SHA1(5347c3aefb642fc5cabd9d5e61fe6515a2dcb2aa))
	ROM_LOAD("sound2.e", 0xe000, 0x1000, CRC(f9b57fd6) SHA1(50e42ed349680211eedf55ae639dbae899f3c6da))
	ROM_LOAD("sound3.f", 0xd000, 0x1000, CRC(551566e6) SHA1(350432dbc0d6f55404cae970524a0dfda15d8aa0))
	ROM_LOAD("sound4.g", 0xc000, 0x1000, CRC(720593fb) SHA1(93aa9ae1be299548e17b4fe97a7fb4ddab76de40))
ROM_END

ROM_START(socrkinga)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD ( "soccer.ic1", 0x0000, 0x0800, CRC(3fbd7c32) SHA1(2f56f67d1ad987638284000cca1e20ff17fcd4f9))
	ROM_CONTINUE(0x2000, 0x0800)
	ROM_LOAD ( "soccer.ic2", 0x0800, 0x0800, CRC(0cc0df1f) SHA1(2fd05af0ec63835a8f69fdc50e2faceb829b4df2))
	ROM_CONTINUE(0x2800, 0x0800)
	ROM_LOAD ( "soccer.ic3", 0x1000, 0x0800, CRC(5da6ea20) SHA1(c0e740e20a968ee58e0cd12a0089bb047f5e2cbb)) // sldh
	ROM_CONTINUE(0x3000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("sound1.c", 0xf000, 0x1000, CRC(3aa95018) SHA1(5347c3aefb642fc5cabd9d5e61fe6515a2dcb2aa))
	ROM_LOAD("sound2.e", 0xe000, 0x1000, CRC(f9b57fd6) SHA1(50e42ed349680211eedf55ae639dbae899f3c6da))
	ROM_LOAD("sound3.f", 0xd000, 0x1000, CRC(551566e6) SHA1(350432dbc0d6f55404cae970524a0dfda15d8aa0))
	ROM_LOAD("sound4.g", 0xc000, 0x1000, CRC(720593fb) SHA1(93aa9ae1be299548e17b4fe97a7fb4ddab76de40))
ROM_END

ROM_START(socrkingi)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD ( "soccer.ic1", 0x0000, 0x0800, CRC(3fbd7c32) SHA1(2f56f67d1ad987638284000cca1e20ff17fcd4f9))
	ROM_CONTINUE(0x2000, 0x0800)
	ROM_LOAD ( "soccer.ic2", 0x0800, 0x0800, CRC(0cc0df1f) SHA1(2fd05af0ec63835a8f69fdc50e2faceb829b4df2))
	ROM_CONTINUE(0x2800, 0x0800)
	ROM_LOAD ( "soccer.ic3", 0x1000, 0x0800, CRC(72caac2c) SHA1(7d63e0cf699365ee1787004d6155646e715b672e))
	ROM_CONTINUE(0x3000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("sking_it.1c", 0xf000, 0x1000, CRC(2965643f) SHA1(06de48e7afe1004ad27b805ab4b5111ef5db4380))
	ROM_LOAD("sking_it.1e", 0xe000, 0x1000, CRC(f70ae48f) SHA1(c7aec7b54ae298d833f79f041dd9b08ec3e0ccb4))
	ROM_LOAD("sking_it.1f", 0xd000, 0x1000, CRC(1b817503) SHA1(6efbb2c5cfeb5286d82155a4b506a2c347aebad8))
	ROM_LOAD("sking_it.1g", 0xc000, 0x1000, CRC(853a3cbc) SHA1(26d9273bc5cddd47daf88432bf8118e94334a6c1))
ROM_END

ROM_START(socrkingg)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD ( "soccer.ic1", 0x0000, 0x0800, CRC(3fbd7c32) SHA1(2f56f67d1ad987638284000cca1e20ff17fcd4f9))
	ROM_CONTINUE(0x2000, 0x0800)
	ROM_LOAD ( "soccer.ic2", 0x0800, 0x0800, CRC(0cc0df1f) SHA1(2fd05af0ec63835a8f69fdc50e2faceb829b4df2))
	ROM_CONTINUE(0x2800, 0x0800)
	ROM_LOAD ( "soccer.ic3", 0x1000, 0x0800, CRC(72caac2c) SHA1(7d63e0cf699365ee1787004d6155646e715b672e))
	ROM_CONTINUE(0x3000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("sk-de1.c", 0xf000, 0x1000, CRC(702e3e67) SHA1(ad4c02ef480d3923eebaedb12851018146740558))
	ROM_LOAD("sk-de2.e", 0xe000, 0x1000, CRC(b60eddb5) SHA1(7e335315d0b91fc67888cda644dabafdef1afa19))
	ROM_LOAD("sk-de3.f", 0xd000, 0x1000, CRC(2f72a94e) SHA1(912ef1e2878b61edff88e5cc1ec19d1b22d44f2d))
	ROM_LOAD("sk-de4.g", 0xc000, 0x1000, CRC(23adcc78) SHA1(c25185c08377286c04c43fa2156245a71fc68e2e))
	ROM_LOAD("sk-de5.h", 0x7000, 0x1000, CRC(c6f0302d) SHA1(c57d36f3bc3a7e3a056b930b8e11b4cee4af0558))
ROM_END

/*--------------------------------
/ Spooky (04/87)
/-------------------------------*/
ROM_START(spookyp)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD ( "spook_1.lgc", 0x0000, 0x0800, CRC(377b347d) SHA1(c7334cf2b10b749f5f75b8feaa8ec773a576b2f1))
	ROM_CONTINUE(0x2000, 0x0800)
	ROM_CONTINUE(0x4000, 0x0800)
	ROM_CONTINUE(0x6000, 0x0800)
	ROM_LOAD ( "spook_2.lgc", 0x0800, 0x0800, CRC(ae0598b0) SHA1(aab725d1e386a3792100eb55c5836e6ed68cafdd))
	ROM_CONTINUE(0x2800, 0x0800)
	ROM_CONTINUE(0x1000, 0x0800)
	ROM_CONTINUE(0x3000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("spook_e.snd", 0xa000, 0x2000, CRC(3d632c93) SHA1(3cc127956a6df1a4fd551826068810724b32ad0e))
	ROM_LOAD("spook_f.snd", 0xc000, 0x4000, CRC(cc04a448) SHA1(e837a7d7640aa1d2c2880616bd377b64dc8fac9d))

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("spook_4.snd", 0x0000, 0x8000, CRC(3ab517a4) SHA1(4a9dd9d571f958c270b437a1665e6d3dd3eef598))
	ROM_LOAD("spook_6.snd", 0x8000, 0x8000, CRC(d4320bc7) SHA1(30b959f5df44d097baffc2de70b12fc767f5663b))
ROM_END

ROM_START(spookyi)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD ( "spook_1.lgc", 0x0000, 0x0800, CRC(377b347d) SHA1(c7334cf2b10b749f5f75b8feaa8ec773a576b2f1))
	ROM_CONTINUE(0x2000, 0x0800)
	ROM_CONTINUE(0x4000, 0x0800)
	ROM_CONTINUE(0x6000, 0x0800)
	ROM_LOAD ( "spook_2.lgc", 0x0800, 0x0800, CRC(ae0598b0) SHA1(aab725d1e386a3792100eb55c5836e6ed68cafdd))
	ROM_CONTINUE(0x2800, 0x0800)
	ROM_CONTINUE(0x1000, 0x0800)
	ROM_CONTINUE(0x3000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("spook_it.1e", 0xa000, 0x2000, CRC(cdbe248e) SHA1(2337836e01622b3fc3f31272faaebf30a608a138))
	ROM_LOAD("spook_f.snd", 0xc000, 0x4000, CRC(cc04a448) SHA1(e837a7d7640aa1d2c2880616bd377b64dc8fac9d))

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("spook_4.snd", 0x0000, 0x8000, CRC(3ab517a4) SHA1(4a9dd9d571f958c270b437a1665e6d3dd3eef598))
	ROM_LOAD("spook_6.snd", 0x8000, 0x8000, CRC(d4320bc7) SHA1(30b959f5df44d097baffc2de70b12fc767f5663b))
ROM_END

/*--------------------------------
/ Star's Phoenix (07/87)
/-------------------------------*/
ROM_START(strsphnx)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD ( "strphnx1.cpu", 0x0000, 0x0800, CRC(2a31b7da) SHA1(05f2173783e686cc8774bed6eb59b41f7af88d11))
	ROM_CONTINUE(0x2000, 0x0800)
	ROM_CONTINUE(0x4000, 0x0800)
	ROM_CONTINUE(0x6000, 0x0800)
	ROM_LOAD ( "strphnx2.cpu", 0x0800, 0x0800, CRC(db830505) SHA1(55d6d6e12e2861fec81b46fb90c29aad5ad922aa))
	ROM_CONTINUE(0x2800, 0x0800)
	ROM_CONTINUE(0x1000, 0x0800)
	ROM_CONTINUE(0x3000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("snd_ic24.bin", 0x0000, 0x8000, CRC(158d6f83) SHA1(281e1b13be43025be1b33dcd366cec0b36f29e5c))
	ROM_LOAD("snd_ic25.bin", 0x8000, 0x8000, CRC(b1c9238e) SHA1(88c9df1fca94d32a0fa5d75312dabff257e867dd))

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("snd_ic05.bin", 0x0000, 0x8000, CRC(74cc4902) SHA1(e2f46bcf5446f98d098c49f8c2416292401265b9))
	ROM_LOAD("snd_ic06.bin", 0x8000, 0x8000, CRC(a0400411) SHA1(da9de6105639c4f6174f5bc92f44e02c339a2bc3))

	ROM_REGION(0x10000, "cpu4", 0)
	ROM_LOAD("snd_ic40.bin", 0x0000, 0x8000, CRC(974ceb9c) SHA1(3665af9170a2afbe26f68e8f3cedb0d177f476c4))
ROM_END

ROM_START(strsphnxf)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD ( "strphnx1.cpu", 0x0000, 0x0800, CRC(2a31b7da) SHA1(05f2173783e686cc8774bed6eb59b41f7af88d11))
	ROM_CONTINUE(0x2000, 0x0800)
	ROM_CONTINUE(0x4000, 0x0800)
	ROM_CONTINUE(0x6000, 0x0800)
	ROM_LOAD ( "strphnx2.cpu", 0x0800, 0x0800, CRC(db830505) SHA1(55d6d6e12e2861fec81b46fb90c29aad5ad922aa))
	ROM_CONTINUE(0x2800, 0x0800)
	ROM_CONTINUE(0x1000, 0x0800)
	ROM_CONTINUE(0x3000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("snd_ic24.bin", 0x0000, 0x8000, CRC(158d6f83) SHA1(281e1b13be43025be1b33dcd366cec0b36f29e5c))
	ROM_LOAD("snd_ic25.bin", 0x8000, 0x8000, CRC(b1c9238e) SHA1(88c9df1fca94d32a0fa5d75312dabff257e867dd))

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("snd_ic05.bin", 0x0000, 0x8000, CRC(74cc4902) SHA1(e2f46bcf5446f98d098c49f8c2416292401265b9))
	ROM_LOAD("snd_ic06.bin", 0x8000, 0x8000, CRC(a0400411) SHA1(da9de6105639c4f6174f5bc92f44e02c339a2bc3))

	ROM_REGION(0x10000, "cpu4", 0)
	ROM_LOAD("sndf_ic40.bin", 0x0000, 0x8000, CRC(1b40de42) SHA1(cfebab38a493edab1dfd0e5f591ccb9658da80c6))
ROM_END

/*--------------------------------
/ Thunder Man (1987)
/-------------------------------*/
ROM_START(thndrman)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("mpu_ic1.764",0x0000, 0x0800, CRC(d4861835) SHA1(b3518eb3126fc3f5b6f3e9a4c53df8eec21768c5))
	ROM_CONTINUE(0x2000, 0x0800)
	ROM_CONTINUE(0x0800, 0x0800)
	ROM_CONTINUE(0x2800, 0x0800)
	ROM_RELOAD  (0x4000, 0x0800)
	ROM_CONTINUE(0x6000, 0x0800)
	ROM_CONTINUE(0x4800, 0x0800)
	ROM_CONTINUE(0x6800, 0x0800)
	ROM_LOAD("mpu_ic3.764",0x1000, 0x0800, CRC(db85aec2) SHA1(ac5b321cc9d6f86a7f554af010858d2144cbdf2a))
	ROM_CONTINUE(0x3000, 0x0800)
	ROM_CONTINUE(0x1800, 0x0800)
	ROM_CONTINUE(0x3800, 0x0800)
	ROM_RELOAD  (0x5000, 0x0800)
	ROM_CONTINUE(0x7000, 0x0800)
	ROM_CONTINUE(0x5800, 0x0800)
	ROM_CONTINUE(0x7800, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("snd_1f.764", 0xc000, 0x2000, CRC(400e8e2a) SHA1(24af3a8e11aec89ae27a5cfcce9d4624bede18f7))
	ROM_LOAD("snd_1c.764", 0xe000, 0x2000, CRC(4f18409f) SHA1(21002a147e2542caacba0392cec62511343b90c2))
ROM_END

/*--------------------------------
/ Time Machine (04/83)
/-------------------------------*/
ROM_START(tmachzac)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD ( "timemach.ic1", 0x0000, 0x0800, CRC(d88f424b) SHA1(a0c51f894d604504253f66e49298a9d836e25308))
	ROM_CONTINUE(0x2000, 0x0800)
	ROM_CONTINUE(0x4000, 0x0800)
	ROM_CONTINUE(0x6000, 0x0800)
	ROM_LOAD ( "timemach.ic2", 0x0800, 0x0800, CRC(3c313487) SHA1(17c6c4a0c0c6dd90cf7fd9298b945305f734747d))
	ROM_CONTINUE(0x2800, 0x0800)
	ROM_CONTINUE(0x1000, 0x0800)
	ROM_CONTINUE(0x3000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("sound1.d", 0xe000, 0x2000, CRC(efc1d724) SHA1(f553767c053e4854fe7839f8c8f4a7f5aefe2692))
	ROM_LOAD("sound2.e", 0xc000, 0x1000, CRC(41881a1d) SHA1(42f8dd13c38e11c0dd3cf59c64751baaacb00ac1))
	ROM_RELOAD(0xd000, 0x1000)
	ROM_LOAD("sound3.g", 0xa000, 0x2000, CRC(b7b872da) SHA1(dfeb48a683c6d249101488f244b26509a4c4d81d))
ROM_END

ROM_START(tmachzacg)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD ( "timemach.ic1", 0x0000, 0x0800, CRC(d88f424b) SHA1(a0c51f894d604504253f66e49298a9d836e25308))
	ROM_CONTINUE(0x2000, 0x0800)
	ROM_CONTINUE(0x4000, 0x0800)
	ROM_CONTINUE(0x6000, 0x0800)
	ROM_LOAD ( "timemach.ic2", 0x0800, 0x0800, CRC(3c313487) SHA1(17c6c4a0c0c6dd90cf7fd9298b945305f734747d))
	ROM_CONTINUE(0x2800, 0x0800)
	ROM_CONTINUE(0x1000, 0x0800)
	ROM_CONTINUE(0x3000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("tmach_de.1d", 0xe000, 0x2000, CRC(8e8c27a4) SHA1(2e418e509bc241c193564e926583b09582944233))
	ROM_LOAD("sound2.e", 0xc000, 0x1000, CRC(41881a1d) SHA1(42f8dd13c38e11c0dd3cf59c64751baaacb00ac1))
	ROM_RELOAD(0xd000, 0x1000)
	ROM_LOAD("tmach_de.1g", 0xa000, 0x2000, CRC(06cba6e4) SHA1(c6ebd9170943da9f74944ada5c7ebd0929e627d0))
ROM_END

ROM_START(tmachzacf)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD ( "timemach.ic1", 0x0000, 0x0800, CRC(d88f424b) SHA1(a0c51f894d604504253f66e49298a9d836e25308))
	ROM_CONTINUE(0x2000, 0x0800)
	ROM_CONTINUE(0x4000, 0x0800)
	ROM_CONTINUE(0x6000, 0x0800)
	ROM_LOAD ( "timemach.ic2", 0x0800, 0x0800, CRC(3c313487) SHA1(17c6c4a0c0c6dd90cf7fd9298b945305f734747d))
	ROM_CONTINUE(0x2800, 0x0800)
	ROM_CONTINUE(0x1000, 0x0800)
	ROM_CONTINUE(0x3000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("tmach_fr.1d", 0xe000, 0x2000, CRC(831203a4) SHA1(fc60086c2b9b83a47f30b028e7512090658c5700))
	ROM_LOAD("sound2.e", 0xc000, 0x1000, CRC(41881a1d) SHA1(42f8dd13c38e11c0dd3cf59c64751baaacb00ac1))
	ROM_RELOAD(0xd000, 0x1000)
	ROM_LOAD("tmach_fr.1g", 0xa000, 0x2000, CRC(4fb43fa3) SHA1(35ef929976e16abef9e70e569a6c005fd7995a6b))
ROM_END

ROM_START(tmachzaci)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD ( "timemach.ic1", 0x0000, 0x0800, CRC(d88f424b) SHA1(a0c51f894d604504253f66e49298a9d836e25308))
	ROM_CONTINUE(0x2000, 0x0800)
	ROM_CONTINUE(0x4000, 0x0800)
	ROM_CONTINUE(0x6000, 0x0800)
	ROM_LOAD ( "timemach.ic2", 0x0800, 0x0800, CRC(3c313487) SHA1(17c6c4a0c0c6dd90cf7fd9298b945305f734747d))
	ROM_CONTINUE(0x2800, 0x0800)
	ROM_CONTINUE(0x1000, 0x0800)
	ROM_CONTINUE(0x3000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("tmach_it.1d", 0xe000, 0x2000, CRC(8d0272fe) SHA1(031d868a847eb157e182ac2cf69d2dd1bbf33aa5))
	ROM_LOAD("sound2.e", 0xc000, 0x1000, CRC(41881a1d) SHA1(42f8dd13c38e11c0dd3cf59c64751baaacb00ac1))
	ROM_RELOAD(0xd000, 0x1000)
	ROM_LOAD("tmach_it.1g", 0xa000, 0x2000, CRC(fc94b56c) SHA1(cacb1e2414416b0ddaacc37a3e9c2501d7b57ef9))
ROM_END


/*--------------------------------
/ Zankor (12/86)
/-------------------------------*/
ROM_START(zankor)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD ( "zan_ic1.764", 0x0000, 0x0800, CRC(e7ba5acf) SHA1(48b64921dd8a22c2483162db571512cad8cbb072))
	ROM_CONTINUE(0x2000, 0x0800)
	ROM_CONTINUE(0x4000, 0x0800)
	ROM_CONTINUE(0x6000, 0x0800)
	ROM_LOAD ( "zan_ic2.764", 0x0800, 0x0800, CRC(5804ff10) SHA1(fc3c4acb183c5c3e0a6504583c78f25a7a322cce))
	ROM_CONTINUE(0x2800, 0x0800)
	ROM_CONTINUE(0x1000, 0x0800)
	ROM_CONTINUE(0x3000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("1en.64", 0xa000, 0x2000, CRC(abc930cc) SHA1(6c658aae3f26db21df7b74a616cf37307dba63e3))
	ROM_LOAD("zan_1f.128", 0xc000, 0x4000, CRC(74fcadc9) SHA1(efd6fc99d7a3ed8e59fbbafbee161af6fb527028))

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("zan_ic4.128", 0x0000, 0x4000, CRC(f34a2aaa) SHA1(5e415874f68586aa30dba9fff0dc8990c636cecd))
	ROM_LOAD("zan_ic5.128", 0x4000, 0x4000, CRC(bf61aab0) SHA1(939266696d0562f255f0fa5068280fe6a4cf8267))
	ROM_LOAD("zan_ic6.128", 0x8000, 0x4000, CRC(13a5b8d4) SHA1(d8c976b3f5e9c7cded0922feefa1531c59432515))
ROM_END

ROM_START(zankor2i)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD ( "zan_ic1.764", 0x0000, 0x0800, CRC(e7ba5acf) SHA1(48b64921dd8a22c2483162db571512cad8cbb072))
	ROM_CONTINUE(0x2000, 0x0800)
	ROM_CONTINUE(0x4000, 0x0800)
	ROM_CONTINUE(0x6000, 0x0800)
	ROM_LOAD ( "zan_ic2.764", 0x0800, 0x0800, CRC(5804ff10) SHA1(fc3c4acb183c5c3e0a6504583c78f25a7a322cce))
	ROM_CONTINUE(0x2800, 0x0800)
	ROM_CONTINUE(0x1000, 0x0800)
	ROM_CONTINUE(0x3000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("zan_1e.764", 0xa000, 0x2000, CRC(3d2c53d2) SHA1(2de4a56f50752a97af0acfed816eae83b36cf7af))
	ROM_LOAD("zan_1f.128", 0xc000, 0x4000, CRC(74fcadc9) SHA1(efd6fc99d7a3ed8e59fbbafbee161af6fb527028))

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("zan_ic4.128", 0x0000, 0x4000, CRC(f34a2aaa) SHA1(5e415874f68586aa30dba9fff0dc8990c636cecd))
	ROM_LOAD("zan_ic5.128", 0x4000, 0x4000, CRC(bf61aab0) SHA1(939266696d0562f255f0fa5068280fe6a4cf8267))
	ROM_LOAD("zan_ic6.128", 0x8000, 0x4000, CRC(13a5b8d4) SHA1(d8c976b3f5e9c7cded0922feefa1531c59432515))
ROM_END

} // anonymous namespace

// 1B1170 sound (IMDB), 1B1370 (schematics)
GAME(1982,  socrking,   0,        zac_2,  zac_2, zac_2_state, empty_init, ROT0, "Zaccaria",    "Soccer Kings",                            MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1982,  socrkinga,  socrking, zac_2,  zac_2, zac_2_state, empty_init, ROT0, "Zaccaria",    "Soccer Kings (alternate set)",            MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1982,  socrkingi,  socrking, zac_2,  zac_2, zac_2_state, empty_init, ROT0, "Zaccaria",    "Soccer Kings (Italian speech)",           MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1982,  socrkingg,  socrking, zac_2,  zac_2, zac_2_state, empty_init, ROT0, "Zaccaria",    "Soccer Kings (German speech)",            MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1983,  pinchamp,   0,        zac_2,  zac_2, zac_2_state, empty_init, ROT0, "Zaccaria",    "Pinball Champ",                           MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1983,  pinchampf,  pinchamp, zac_2,  zac_2, zac_2_state, empty_init, ROT0, "Zaccaria",    "Pinball Champ (French speech)",           MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1983,  pinchampg,  pinchamp, zac_2,  zac_2, zac_2_state, empty_init, ROT0, "Zaccaria",    "Pinball Champ (German speech)",           MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1983,  pinchampi,  pinchamp, zac_2,  zac_2, zac_2_state, empty_init, ROT0, "Zaccaria",    "Pinball Champ (Italian speech)",          MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1983,  pinchamp7,  pinchamp, zac_2,  zac_2, zac_2_state, init_1,     ROT0, "Zaccaria",    "Pinball Champ (7 digits)",                MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1983,  pinchamp7f, pinchamp, zac_2,  zac_2, zac_2_state, init_1,     ROT0, "Zaccaria",    "Pinball Champ (7 digits French speech)",  MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1983,  pinchamp7g, pinchamp, zac_2,  zac_2, zac_2_state, init_1,     ROT0, "Zaccaria",    "Pinball Champ (7 digits German speech)",  MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1983,  pinchamp7i, pinchamp, zac_2,  zac_2, zac_2_state, init_1,     ROT0, "Zaccaria",    "Pinball Champ (7 digits Italian speech)", MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )

// 1B11136 or 1B13136
GAME(1983,  tmachzac,   0,        zac_2,  zac_2, zac_2_state, init_1,     ROT0, "Zaccaria",    "Time Machine (Zaccaria)",                 MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1983,  tmachzacg,  tmachzac, zac_2,  zac_2, zac_2_state, init_1,     ROT0, "Zaccaria",    "Time Machine (Zaccaria, German speech)",  MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1983,  tmachzacf,  tmachzac, zac_2,  zac_2, zac_2_state, init_1,     ROT0, "Zaccaria",    "Time Machine (Zaccaria, French speech)",  MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1983,  tmachzaci,  tmachzac, zac_2,  zac_2, zac_2_state, init_1,     ROT0, "Zaccaria",    "Time Machine (Zaccaria, Italian speech)", MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1983,  farfalla,   0,        zac_2,  zac_2, zac_2_state, init_1,     ROT0, "Zaccaria",    "Farfalla",                                MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1983,  farfallai,  farfalla, zac_2,  zac_2, zac_2_state, init_1,     ROT0, "Zaccaria",    "Farfalla (Italian speech)",               MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1983,  farfallag,  farfalla, zac_2,  zac_2, zac_2_state, init_1,     ROT0, "Zaccaria",    "Farfalla (German speech)",                MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1983,  farfallaf,  farfalla, zac_2,  zac_2, zac_2_state, init_1,     ROT0, "Zaccaria",    "Farfalla (French speech)",                MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1984,  dvlrider,   0,        zac_2,  zac_2, zac_2_state, init_1,     ROT0, "Zaccaria",    "Devil Riders",                            MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1984,  dvlrideri,  dvlrider, zac_2,  zac_2, zac_2_state, init_1,     ROT0, "Zaccaria",    "Devil Riders (Italian speech)",           MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1984,  dvlriderg,  dvlrider, zac_2,  zac_2, zac_2_state, init_1,     ROT0, "Zaccaria",    "Devil Riders (German speech)",            MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1984,  dvlriderf,  dvlrider, zac_2,  zac_2, zac_2_state, empty_init, ROT0, "Zaccaria",    "Devil Riders (French speech)",            MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1984,  mcastle,    0,        zac_2,  zac_2, zac_2_state, empty_init, ROT0, "Zaccaria",    "Magic Castle",                            MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1984,  mcastlei,   mcastle,  zac_2,  zac_2, zac_2_state, empty_init, ROT0, "Zaccaria",    "Magic Castle (Italian speech)",           MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1984,  mcastleg,   mcastle,  zac_2,  zac_2, zac_2_state, empty_init, ROT0, "Zaccaria",    "Magic Castle (German speech)",            MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1984,  mcastlef,   mcastle,  zac_2,  zac_2, zac_2_state, empty_init, ROT0, "Zaccaria",    "Magic Castle (French speech)",            MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1985,  robot,      0,        zac_2,  zac_2, zac_2_state, empty_init, ROT0, "Zaccaria",    "Robot (Zaccaria)",                        MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1985,  roboti,     robot,    zac_2,  zac_2, zac_2_state, empty_init, ROT0, "Zaccaria",    "Robot (Zaccaria, Italian speech)",        MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1985,  robotg,     robot,    zac_2,  zac_2, zac_2_state, empty_init, ROT0, "Zaccaria",    "Robot (Zaccaria, German speech)",         MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1985,  robotf,     robot,    zac_2,  zac_2, zac_2_state, empty_init, ROT0, "Zaccaria",    "Robot (Zaccaria, French speech)",         MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )

// 1B11178
GAME(1985,  clown,      0,        zac_2,  zac_2, zac_2_state, empty_init, ROT0, "Zaccaria",    "Clown",                                   MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1985,  poolcham,   0,        zac_2,  zac_2, zac_2_state, empty_init, ROT0, "Zaccaria",    "Pool Champion",                           MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1985,  poolchami,  poolcham, zac_2,  zac_2, zac_2_state, empty_init, ROT0, "Zaccaria",    "Pool Champion (Italian speech)",          MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1986,  bbeltzac,   0,        zac_2,  zac_2, zac_2_state, empty_init, ROT0, "Zaccaria",    "Black Belt (Zaccaria)",                   MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1986,  bbeltzaci,  bbeltzac, zac_2,  zac_2, zac_2_state, empty_init, ROT0, "Zaccaria",    "Black Belt (Zaccaria, Italian speech)",   MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1986,  bbeltzacg,  bbeltzac, zac_2,  zac_2, zac_2_state, empty_init, ROT0, "Zaccaria",    "Black Belt (Zaccaria, German speech)",    MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1986,  bbeltzacf,  bbeltzac, zac_2,  zac_2, zac_2_state, empty_init, ROT0, "Zaccaria",    "Black Belt (Zaccaria, French speech)",    MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1986,  mexico,     0,        zac_2,  zac_2, zac_2_state, empty_init, ROT0, "Zaccaria",    "Mexico 86 (German speech)",               MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1986,  mexicoi,    mexico,   zac_2,  zac_2, zac_2_state, empty_init, ROT0, "Zaccaria",    "Mexico 86 (Italian speech)",              MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1986,  zankor,     0,        zac_2,  zac_2, zac_2_state, empty_init, ROT0, "Zaccaria",    "Zankor (TMS5200 Italian speech)",         MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1986,  zankor2i,   zankor,   zac_2,  zac_2, zac_2_state, empty_init, ROT0, "Zaccaria",    "Zankor (TMS5220 Italian speech)",         MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1987,  spookyp,    0,        zac_2,  zac_2, zac_2_state, empty_init, ROT0, "Zaccaria",    "Spooky",                                  MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1987,  spookyi,    spookyp,  zac_2,  zac_2, zac_2_state, empty_init, ROT0, "Zaccaria",    "Spooky (Italian speech)",                 MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1987,  strsphnx,   0,        zac_2,  zac_2, zac_2_state, empty_init, ROT0, "Zaccaria",    "Star's Phoenix (Italian speech)",         MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1987,  strsphnxf,  strsphnx, zac_2,  zac_2, zac_2_state, empty_init, ROT0, "Zaccaria",    "Star's Phoenix (French speech)",          MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )

// 1B11183
GAME(1987,  nstrphnx,   0,        zac_2,  zac_2, zac_2_state, empty_init, ROT0, "Zaccaria",    "New Star's Phoenix (Italian speech)",     MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1987,  nstrphnxf,  nstrphnx, zac_2,  zac_2, zac_2_state, empty_init, ROT0, "Zaccaria",    "New Star's Phoenix (French speech)",      MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )

// unknown
GAME(1987,  thndrman,   0,        zac_2,  zac_2, zac_2_state, empty_init, ROT0, "Apple Time",  "Thunder Man",                             MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )

// Technoplay sound card
GAME(1987,  scram_tp,   0,        zac_2,  zac_2, zac_2_state, empty_init, ROT0, "Tecnoplay",   "Scramble (Pinball)",                      MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
