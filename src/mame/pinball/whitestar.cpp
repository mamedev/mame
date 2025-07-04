// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/********************************************************************************************************************
PINBALL
Sega/Stern Whitestar

Several PAL devices are not dumped.

Here are the key codes to enable play:
- "End ball" code, if it says BALL SAVED you need to play a while longer.
- May have to hold the key or hit it a few times

Game                                   NUM    Start game                 End ball
---------------------------------------------------------------------------------------------
**** Sega ****
Viper Night Drivin'                   5035    Hold FG, hit 1             DEFG
Mini-Viper (not emulated)
Godzilla                              5040    Hold FG, hit 1             DF
Twister                               5041    Hold FG, hit 1             DF
Goldeneye                             5042    Hold FG, hit 2             DF
Space Jam                             5043    Hold FG, hit 1             DG
Apollo 13                             5044    Hold FG, hit 2             DFG
Independence Day                      5045    Hold FG. hit 1             DF
X Files                               5046    Hold FG, hit 1 then Shift  DG
Cut The Cheese Deluxe (Redemption)      --    1
Titanic (Coin dropper)                  --    (unknown)
Wack-A-Doodle-Doo (Redemption)          --    1
Lost World Jurassic Park              5053    Hold DEFG, hit 1 H A       DEFG (it can be difficult to start or end a ball)
Star Wars Trilogy Special Edition     5056    Hold FG, hit 1             DG
Starship Troopers                     5059    Hold DE, hit 1             DG
Lost in Space                         5060    Hold DE, hit 1             FG
Golden Cue                            5064    Hold DFG, hit 1 (unable to navigate setup menu)
High Roller Casino                    5065    Hold FG, hit 1             D
Harley-Davidson                       5067    Hold DG, hit 1             DEFG
Harley-Davidson 2nd edition           5067    Hold DG, hit 1             DEFG
Irons and Woods (Redemption, not emulated)
South Park                            5071    Hold FG, hit 1             DEFG
**** Stern ****
Striker Xtreme                        5068    Hold CDEF, hit 1           CDEF
Sharkey's Shootout                    5072    1 (won't start)
NFL                                   5073    Hold CDEF, hit 1 then A    CDEF
Austin Powers                         5074    Hold CDEF, hit 1           CDEF
Monopoly (Coin dropper)               5075    Hold CDEF, hit 1           CDEF
Playboy                               5076    Hold CDEF, hit 1           CDEF
The Simpsons Pinball Party            5077    Hold BCDEF, hit 1          BCDEF
Rollercoaster Tycoon                  5078    Hold CDE, hit 1            CDE
Terminator 3: Rise of the Machines    5079    Hold CDE, hit 1            CDE
The Brain (conversion, not emulated)
Lord of the Rings                     5080    Hold CDEF, hit 1           CDEF
Ripley's Believe it or not!           5081    Hold CDEF, hit 1           CDEF
Elvis                                 5084    Hold CDEF, hit 1           CDEF
The Sopranos                          5085    Hold CDEF, hit 1           CDEF
NASCAR                                5086    Hold CDEF, hit 1           CDEF (unable to end ball)
Harley-Davidson 3rd edition           5087    Hold DG, hit 1             DEFG
Grand Prix                            5091    Hold CDEF, hit 1           CDEF (unable to end ball)
Dale Jr NASCAR Special Edition        5097    Hold CDEF, hit 1           CDEF (unable to end ball)


Status:
- Some machines are playable
- Volume is quite low. On some machines it can be boosted by hitting NUM2 to get the volume menu, then hold NUM1 to max.

ToDo:
- ATMEL ARM soundcard
- Mechanical sounds
- Outputs
- Some solenoids should interact with switches
- Memory protect switch
- Various unknown memory reads/writes.

*********************************************************************************************************************/
#include "emu.h"
#include "genpin.h"

#include "decodmd2.h"

#include "decobsmt.h"

//#include "video/mc6845.h"

#include "speaker.h"

namespace {

class whitestar_state : public genpin_class
{
public:
	whitestar_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_decobsmt(*this, "decobsmt")
		, m_decodmd(*this, "decodmd")
		, m_io_keyboard(*this, "X%d", 0U)
		, m_mainbank(*this, "mainbank")
	{ }

	void whitestar(machine_config &config);
	void whitestarm(machine_config &config);
	void goldcue(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void bank_w(uint8_t data);
	void dmddata_w(uint8_t data);

	uint8_t switch_r();
	void switch_w(uint8_t data);
	INTERRUPT_GEN_MEMBER(whitestar_firq_interrupt);

	void whitestar_base_map(address_map &map) ATTR_COLD;
	void whitestar_map(address_map &map) ATTR_COLD;
	void whitestarm_map(address_map &map) ATTR_COLD;
	void goldcue_map(address_map &map) ATTR_COLD;
	u8 m_row = 0U;
	required_device<cpu_device> m_maincpu;
	//required_device<cpu_device> m_dmdcpu;
	//required_device<mc6845_device> m_mc6845;
	optional_device<decobsmt_device> m_decobsmt;
	required_device<decodmd_type2_device> m_decodmd;
	required_ioport_array<8> m_io_keyboard;
	required_memory_bank m_mainbank;
};

static INPUT_PORTS_START( whitestar )

//-- Dedicated Switches --

// Note: active low
// D0 - DED #1 - Left Flipper
// D1 - DED #2 - Left Flipper EOS
// D2 - DED #3 - Right Flipper
// D3 - DED #4 - Right Flipper EOS
// D4 - DED #5 - Not Used (Upper Flipper on some games!)
// D5 - DED #6 - Volume (Red Button)
// D6 - DED #7 - Service Credit (Green Button)
// D7 - DED #8 - Begin Test (Black Button)
	PORT_START("DEDICATED")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("Left Flipper")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("Left Flipper EOS")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("Right Flipper")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("Right Flipper EOS")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Volume/Red")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Service Credit/Green")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Test/Black")
	PORT_START("DSW0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_F1) PORT_NAME("INP01")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("INP02") // Coin Slot 4
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START2 ) // used by Apollo 13 and Goldeneye
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("INP07") // Coin slot 5
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_F2) PORT_NAME("INP08")

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("INP09")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("INP10")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("INP11")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("INP12")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("INP13")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("INP14")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("INP15")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_NAME("INP16")

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME("INP17")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_NAME("INP18")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_K) PORT_NAME("INP19")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_L) PORT_NAME("INP20")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("INP21")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_NAME("INP22")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_O) PORT_NAME("INP23")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_P) PORT_NAME("INP24")

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("INP25")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("INP26")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("INP27")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("INP28")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_U) PORT_NAME("INP29")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_V) PORT_NAME("INP30")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("INP31")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("INP32")

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Y) PORT_NAME("INP33")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("INP34")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("INP35")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_STOP) PORT_NAME("INP36")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("INP37")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_COLON) PORT_NAME("INP38")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("INP39")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_NAME("INP40")

	PORT_START("X5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("INP41")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("INP42")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("INP43")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("INP44")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("INP45")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("INP46")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_LEFT) PORT_NAME("INP47")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_UP) PORT_NAME("INP48")

	PORT_START("X6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("INP49")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_DOWN) PORT_NAME("INP50")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_HOME) PORT_NAME("INP51")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_END) PORT_NAME("INP52")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_DEL) PORT_NAME("INP53")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_NAME("INP55") // usually slam tilt
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_NAME("INP56") // usually plumb-bob tilt

	PORT_START("X7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_RALT) PORT_NAME("INP57")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_DEL_PAD) PORT_NAME("INP58")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("INP59")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_PGUP) PORT_NAME("INP60")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_PGDN) PORT_NAME("INP61")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH_PAD) PORT_NAME("INP62")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ASTERISK) PORT_NAME("INP63")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("INP64")
INPUT_PORTS_END

void whitestar_state::whitestar_base_map(address_map &map)
{
	map(0x0000, 0x1fff).ram().share("nvram");
	map(0x2000, 0x200f).nopw();  // lots of unknown writes
	map(0x3000, 0x3000).portr("DEDICATED");
	map(0x3100, 0x3100).portr("DSW0");
	map(0x3200, 0x3200).w(FUNC(whitestar_state::bank_w));
	map(0x3300, 0x3300).w(FUNC(whitestar_state::switch_w));
	map(0x3400, 0x3400).r(FUNC(whitestar_state::switch_r));
	map(0x3600, 0x3600).w(FUNC(whitestar_state::dmddata_w));
	map(0x3601, 0x3601).rw(m_decodmd, FUNC(decodmd_type2_device::ctrl_r), FUNC(decodmd_type2_device::ctrl_w)); // TODO: should this be status_r?
	map(0x3700, 0x3700).r(m_decodmd, FUNC(decodmd_type2_device::busy_r));
	map(0x4000, 0x7fff).bankr(m_mainbank);
	map(0x8000, 0xffff).rom().region("maincpu", 0x18000);
}

void whitestar_state::whitestar_map(address_map &map)
{
	whitestar_base_map(map);
	map(0x3800, 0x3800).w(m_decobsmt, FUNC(decobsmt_device::bsmt_comms_w));
}

void whitestar_state::whitestarm_map(address_map &map)
{
	whitestar_base_map(map);
	// TODO: sound writes
}

void whitestar_state::goldcue_map(address_map &map)
{
	whitestar_base_map(map);
	map(0x8000, 0x81ff).ram();
}

uint8_t whitestar_state::switch_r()
{
	u8 data = 0U;
	for (u8 i = 0; i < 8; i++)
		if (BIT(m_row, i))
			data |= m_io_keyboard[i]->read();

	return ~data;
}

void whitestar_state::switch_w(uint8_t data)
{
	m_row = data;
}

void whitestar_state::bank_w(uint8_t data)
{
	m_mainbank->set_entry(data & 0x1f);
}

// Whitestar automatically pulses the DMD IRQ line?  DE hardware doesn't do that...
void whitestar_state::dmddata_w(uint8_t data)
{
	m_decodmd->data_w(data);
	m_decodmd->ctrl_w(1);
	m_decodmd->ctrl_w(0);
}

void whitestar_state::machine_start()
{
	m_mainbank->configure_entries(0, 32, memregion("maincpu")->base(), 0x4000);
	m_mainbank->set_entry(0);

	genpin_class::machine_start();
	//m_io_outputs.resolve();

	save_item(NAME(m_row));
}

void whitestar_state::machine_reset()
{
	genpin_class::machine_reset();
	//for (u8 i = 0; i < m_io_outputs.size(); i++)
		//m_io_outputs[i] = 0;
}

// the appropriate device is passed in, so we can share this routine
INTERRUPT_GEN_MEMBER(whitestar_state::whitestar_firq_interrupt)
{
	device.execute().set_input_line(M6809_FIRQ_LINE, HOLD_LINE);
}

void whitestar_state::whitestar(machine_config &config)
{
	// basic machine hardware
	MC6809E(config, m_maincpu, 2000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &whitestar_state::whitestar_map);
	m_maincpu->set_periodic_int(FUNC(whitestar_state::whitestar_firq_interrupt), attotime::from_hz(976));  // value taken from PinMAME

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// sound hardware
	genpin_audio(config);

	SPEAKER(config, "speaker", 2).front();

	DECOBSMT(config, m_decobsmt, 0);
	m_decobsmt->add_route(0, "speaker", 1.0, 0);
	m_decobsmt->add_route(1, "speaker", 1.0, 1);

	DECODMD2(config, m_decodmd, 0);
}

void whitestar_state::whitestarm(machine_config &config)
{
	whitestar(config);
	config.device_remove("decobsmt");
	m_maincpu->set_addrmap(AS_PROGRAM, &whitestar_state::whitestarm_map);

	// TODO: ATMEL ARM7 sound board
}

void whitestar_state::goldcue(machine_config &config)
{
	whitestar(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &whitestar_state::goldcue_map);
}

// 8Mbit ROMs are mapped oddly: the first 4Mbit of each of the ROMs goes in order u17, u21, u36, u37
// then the second 4Mbit of each of the ROMs in the same order starting at 0x200000
#define ROM_LOAD_SND_8M(name, start, checksums) \
	ROM_LOAD(name, start, 0x080000, checksums) \
	ROM_CONTINUE(0x200000+start, 0x080000)

/*-------------------------------------------------------------------
/ Apollo 13
/-------------------------------------------------------------------*/
ROM_START(apollo13)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("apolcpu.501", 0x00000, 0x20000, CRC(5afb8801) SHA1(65608148817f487c384dd36c221138962f1d9824))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("a13dspa.500", 0x00000, 0x80000, CRC(bf8e3249) SHA1(5e04681901ca794feb970f5388cb355427cf9a9a))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("apollo13.u7", 0x0000, 0x10000, CRC(e58a36b8) SHA1(ae60470a7b6c41cd40dbb7c0bea6f2f148f7b088))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD("apollo13.u17", 0x000000, 0x80000, CRC(4e863aca) SHA1(264f9176a1abf758b7a894d83883330ef91b7388))
	ROM_LOAD("apollo13.u21", 0x080000, 0x80000, CRC(28169e37) SHA1(df5209d24187b546a4296fc4629c58bf729349d2))
	ROM_LOAD("apollo13.u36", 0x100000, 0x80000, CRC(cede5e0f) SHA1(fa3b5820ed58e57b3c6185d91e9aea28aebc28d7))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(apollo13_d4)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("apolcpu.501", 0x00000, 0x20000, CRC(5afb8801) SHA1(65608148817f487c384dd36c221138962f1d9824))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("a13dspa.401", 0x00000, 0x80000, CRC(6516ee16) SHA1(17011df142707917af2e0ec77c0e5ae78df91c0d))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("apollo13.u7", 0x0000, 0x10000, CRC(e58a36b8) SHA1(ae60470a7b6c41cd40dbb7c0bea6f2f148f7b088))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD("apollo13.u17", 0x000000, 0x80000, CRC(4e863aca) SHA1(264f9176a1abf758b7a894d83883330ef91b7388))
	ROM_LOAD("apollo13.u21", 0x080000, 0x80000, CRC(28169e37) SHA1(df5209d24187b546a4296fc4629c58bf729349d2))
	ROM_LOAD("apollo13.u36", 0x100000, 0x80000, CRC(cede5e0f) SHA1(fa3b5820ed58e57b3c6185d91e9aea28aebc28d7))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(apollo13_20)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("a13cpu.203", 0x00000, 0x20000, CRC(4af048fc) SHA1(c82459247707a6cf07a10cc884f1391d0ca536a3))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("a13dps.201", 0x00000, 0x80000, CRC(ab97a71c) SHA1(1e01d3c2ac1b9153fb4f3f888fe01fcebbf853d7))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("apollo13.u7", 0x0000, 0x10000, CRC(e58a36b8) SHA1(ae60470a7b6c41cd40dbb7c0bea6f2f148f7b088))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD("apollo13.u17", 0x000000, 0x80000, CRC(4e863aca) SHA1(264f9176a1abf758b7a894d83883330ef91b7388))
	ROM_LOAD("apollo13.u21", 0x080000, 0x80000, CRC(28169e37) SHA1(df5209d24187b546a4296fc4629c58bf729349d2))
	ROM_LOAD("apollo13.u36", 0x100000, 0x80000, CRC(cede5e0f) SHA1(fa3b5820ed58e57b3c6185d91e9aea28aebc28d7))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(apollo13_10)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("a13cpu.100", 0x00000, 0x20000, CRC(5971e956) SHA1(89853912fc569480e66bec4cef369d8320c3a07d))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("a13dps.100", 0x00000, 0x80000, CRC(224f6149) SHA1(b2a1786adc358834615989fce8835e0f039abb24))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("apollo13.u7", 0x0000, 0x10000, CRC(e58a36b8) SHA1(ae60470a7b6c41cd40dbb7c0bea6f2f148f7b088))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD("apollo13.u17", 0x000000, 0x80000, CRC(4e863aca) SHA1(264f9176a1abf758b7a894d83883330ef91b7388))
	ROM_LOAD("apollo13.u21", 0x080000, 0x80000, CRC(28169e37) SHA1(df5209d24187b546a4296fc4629c58bf729349d2))
	ROM_LOAD("apollo13.u36", 0x100000, 0x80000, CRC(cede5e0f) SHA1(fa3b5820ed58e57b3c6185d91e9aea28aebc28d7))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

/*-------------------------------------------------------------------
/ Austin Powers (3.01)
/-------------------------------------------------------------------*/
ROM_START(aust301)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("apcpu.301", 0x00000, 0x20000, CRC(868d1f38) SHA1(df08b48437f88e66c4caa80602c28a2223f180b9))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("apdsp-a.300", 0x00000, 0x80000, CRC(ecf2c3bb) SHA1(952a7873067b8c70043a38a39a8f65089103336b)) // ver 3.00

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("apsndu7.100",0x0000,0x10000,CRC(d0e79d59) SHA1(7c3f1fa79ff193a976986339a551e3d03208550f))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("apsndu17.100", 0x000000, CRC(c1e33fee) SHA1(5a3581584cc1a841d884de4628f7b65d8670f96a))
	ROM_LOAD_SND_8M("apsndu21.100", 0x080000, CRC(07c3e077) SHA1(d48020f7da400c3682035d537289ce9a30732d74))
	ROM_LOAD_SND_8M("apsndu36.100", 0x100000, CRC(f70f2828) SHA1(9efbed4f68c22eb26e9100afaca5ebe85a97b605))
	ROM_LOAD_SND_8M("apsndu37.100", 0x180000, CRC(ddf0144b) SHA1(c2a56703a41ee31841993d63385491259d5a13f8))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

/*-------------------------------------------------------------------
/ Austin Powers (3.00)
/-------------------------------------------------------------------*/
ROM_START(aust300)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("apcpu.300", 0x00000, 0x20000, CRC(a06b2b03) SHA1(4c36212b43fdc497773425e586f64c3064e7000c))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("apdsp-a.300", 0x00000, 0x80000, CRC(ecf2c3bb) SHA1(952a7873067b8c70043a38a39a8f65089103336b))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("apsndu7.100",0x0000,0x10000,CRC(d0e79d59) SHA1(7c3f1fa79ff193a976986339a551e3d03208550f))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("apsndu17.100", 0x000000, CRC(c1e33fee) SHA1(5a3581584cc1a841d884de4628f7b65d8670f96a))
	ROM_LOAD_SND_8M("apsndu21.100", 0x080000, CRC(07c3e077) SHA1(d48020f7da400c3682035d537289ce9a30732d74))
	ROM_LOAD_SND_8M("apsndu36.100", 0x100000, CRC(f70f2828) SHA1(9efbed4f68c22eb26e9100afaca5ebe85a97b605))
	ROM_LOAD_SND_8M("apsndu37.100", 0x180000, CRC(ddf0144b) SHA1(c2a56703a41ee31841993d63385491259d5a13f8))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

/*-------------------------------------------------------------------
/ Austin Powers (2.01)
/-------------------------------------------------------------------*/
ROM_START(aust201)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("apcpu.201", 0x00000, 0x20000, CRC(a4ddcdca) SHA1(c1eb1ae3b9c9b10410d107165f3bddaa514c2113))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("apdsp-a.200", 0x00000, 0x80000, CRC(f3ca7fca) SHA1(b6b702ad7af75b3010a280adb99e4ee484a03242))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("apsndu7.100",0x0000,0x10000,CRC(d0e79d59) SHA1(7c3f1fa79ff193a976986339a551e3d03208550f))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("apsndu17.100", 0x000000, CRC(c1e33fee) SHA1(5a3581584cc1a841d884de4628f7b65d8670f96a))
	ROM_LOAD_SND_8M("apsndu21.100", 0x080000, CRC(07c3e077) SHA1(d48020f7da400c3682035d537289ce9a30732d74))
	ROM_LOAD_SND_8M("apsndu36.100", 0x100000, CRC(f70f2828) SHA1(9efbed4f68c22eb26e9100afaca5ebe85a97b605))
	ROM_LOAD_SND_8M("apsndu37.100", 0x180000, CRC(ddf0144b) SHA1(c2a56703a41ee31841993d63385491259d5a13f8))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

/*-------------------------------------------------------------------
/ Austin Powers (3.02)
/-------------------------------------------------------------------*/
ROM_START(austin)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("apcpu.302", 0x00000, 0x20000, CRC(2920b59b) SHA1(280cebbb39980fbcfd91fc1cf87a40ad926ffecb))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("apdsp-a.300", 0x00000, 0x80000, CRC(ecf2c3bb) SHA1(952a7873067b8c70043a38a39a8f65089103336b))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("apsndu7.100",0x0000,0x10000,CRC(d0e79d59) SHA1(7c3f1fa79ff193a976986339a551e3d03208550f))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("apsndu17.100", 0x000000, CRC(c1e33fee) SHA1(5a3581584cc1a841d884de4628f7b65d8670f96a))
	ROM_LOAD_SND_8M("apsndu21.100", 0x080000, CRC(07c3e077) SHA1(d48020f7da400c3682035d537289ce9a30732d74))
	ROM_LOAD_SND_8M("apsndu36.100", 0x100000, CRC(f70f2828) SHA1(9efbed4f68c22eb26e9100afaca5ebe85a97b605))
	ROM_LOAD_SND_8M("apsndu37.100", 0x180000, CRC(ddf0144b) SHA1(c2a56703a41ee31841993d63385491259d5a13f8))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(austnew)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("apcpu.302", 0x00000, 0x20000, CRC(2920b59b) SHA1(280cebbb39980fbcfd91fc1cf87a40ad926ffecb))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("apdsp-a.300", 0x00000, 0x80000, CRC(ecf2c3bb) SHA1(952a7873067b8c70043a38a39a8f65089103336b))

	ROM_REGION32_LE(0x600000, "cpu2", ROMREGION_ERASEMASK | 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("apsndu7.100",0x0000,0x10000,CRC(d0e79d59) SHA1(7c3f1fa79ff193a976986339a551e3d03208550f))

	ROM_REGION(0x1000000, "bsmt", 0 )
	ROM_LOAD_SND_8M("apsndu17.100", 0x000000, CRC(c1e33fee) SHA1(5a3581584cc1a841d884de4628f7b65d8670f96a))
	ROM_LOAD_SND_8M("apsndu21.100", 0x080000, CRC(07c3e077) SHA1(d48020f7da400c3682035d537289ce9a30732d74))
	ROM_LOAD_SND_8M("apsndu36.100", 0x100000, CRC(f70f2828) SHA1(9efbed4f68c22eb26e9100afaca5ebe85a97b605))
	ROM_LOAD_SND_8M("apsndu37.100", 0x180000, CRC(ddf0144b) SHA1(c2a56703a41ee31841993d63385491259d5a13f8))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(austinf)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("apcpu.302", 0x00000, 0x20000, CRC(2920b59b) SHA1(280cebbb39980fbcfd91fc1cf87a40ad926ffecb))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("apdsp-f.300", 0x00000, 0x80000, CRC(1aeaa83e) SHA1(8a749c0fbf7b03441780c2158e63d4a87c8d0702))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("apsndu7.100",0x0000,0x10000,CRC(d0e79d59) SHA1(7c3f1fa79ff193a976986339a551e3d03208550f))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("apsndu17.100", 0x000000, CRC(c1e33fee) SHA1(5a3581584cc1a841d884de4628f7b65d8670f96a))
	ROM_LOAD_SND_8M("apsndu21.100", 0x080000, CRC(07c3e077) SHA1(d48020f7da400c3682035d537289ce9a30732d74))
	ROM_LOAD_SND_8M("apsndu36.100", 0x100000, CRC(f70f2828) SHA1(9efbed4f68c22eb26e9100afaca5ebe85a97b605))
	ROM_LOAD_SND_8M("apsndu37.100", 0x180000, CRC(ddf0144b) SHA1(c2a56703a41ee31841993d63385491259d5a13f8))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(austing)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("apcpu.302", 0x00000, 0x20000, CRC(2920b59b) SHA1(280cebbb39980fbcfd91fc1cf87a40ad926ffecb))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("apdsp-g.300", 0x00000, 0x80000, CRC(28b91cc4) SHA1(037628c78955495f10a60cfc329232289417562e))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("apsndu7.100",0x0000,0x10000,CRC(d0e79d59) SHA1(7c3f1fa79ff193a976986339a551e3d03208550f))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("apsndu17.100", 0x000000, CRC(c1e33fee) SHA1(5a3581584cc1a841d884de4628f7b65d8670f96a))
	ROM_LOAD_SND_8M("apsndu21.100", 0x080000, CRC(07c3e077) SHA1(d48020f7da400c3682035d537289ce9a30732d74))
	ROM_LOAD_SND_8M("apsndu36.100", 0x100000, CRC(f70f2828) SHA1(9efbed4f68c22eb26e9100afaca5ebe85a97b605))
	ROM_LOAD_SND_8M("apsndu37.100", 0x180000, CRC(ddf0144b) SHA1(c2a56703a41ee31841993d63385491259d5a13f8))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(austini)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("apcpu.302", 0x00000, 0x20000, CRC(2920b59b) SHA1(280cebbb39980fbcfd91fc1cf87a40ad926ffecb))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("apdsp-i.300", 0x00000, 0x80000, CRC(8b1dd747) SHA1(b29d39a2fb464bd11f4bc5daeb35360126ddf45b))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("apsndu7.100",0x0000,0x10000,CRC(d0e79d59) SHA1(7c3f1fa79ff193a976986339a551e3d03208550f))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("apsndu17.100", 0x000000, CRC(c1e33fee) SHA1(5a3581584cc1a841d884de4628f7b65d8670f96a))
	ROM_LOAD_SND_8M("apsndu21.100", 0x080000, CRC(07c3e077) SHA1(d48020f7da400c3682035d537289ce9a30732d74))
	ROM_LOAD_SND_8M("apsndu36.100", 0x100000, CRC(f70f2828) SHA1(9efbed4f68c22eb26e9100afaca5ebe85a97b605))
	ROM_LOAD_SND_8M("apsndu37.100", 0x180000, CRC(ddf0144b) SHA1(c2a56703a41ee31841993d63385491259d5a13f8))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

/*-------------------------------------------------------------------
/ Godzilla
/-------------------------------------------------------------------*/
ROM_START(godzillp)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("gdzcpu.205", 0x00000, 0x20000, CRC(0156c21c) SHA1(37dcbe84c54e1b8f279f78e7a11544040e98c0b3))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("gzdspa.200", 0x00000, 0x80000, CRC(a254a01d) SHA1(e624a81437ab4d4b3c133baf47993facf6079f4b))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("gdzu7.100", 0x0000, 0x10000, CRC(a0afe8b7) SHA1(33e4a824b26b58e8f963fa8a525a64f4779b45db))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD("gdzu17.100", 0x000000, 0x80000, CRC(6bba69c8) SHA1(51341e188b4191eb1836349dfdd456163d464ad6))
	ROM_LOAD("gdzu21.100", 0x080000, 0x80000, CRC(db738958) SHA1(23082cf98bbcc6d356145414267da887a5ca9305))
	ROM_LOAD("gdzu36.100", 0x100000, 0x80000, CRC(e3f24234) SHA1(eb123200928221a647e10839ebb7f4628501c581))
	ROM_LOAD("gdzu37.100", 0x180000, 0x80000, CRC(2c1acb14) SHA1(4d710e09f5500da937932b4b01d862abb4a89e5a))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(godzillp_100)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("gdzcpu.100", 0x00000, 0x20000, CRC(55c46a98) SHA1(535c363fed2359add260149b6407dc95da32a1e3))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("gzdspa.100", 0x00000, 0x80000, CRC(9b97cd98) SHA1(6fd002a6986aa32832c0628899ba1bafe3642354))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("gdzu7.100", 0x0000, 0x10000, CRC(a0afe8b7) SHA1(33e4a824b26b58e8f963fa8a525a64f4779b45db))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD("gdzu17.100", 0x000000, 0x80000, CRC(6bba69c8) SHA1(51341e188b4191eb1836349dfdd456163d464ad6))
	ROM_LOAD("gdzu21.100", 0x080000, 0x80000, CRC(db738958) SHA1(23082cf98bbcc6d356145414267da887a5ca9305))
	ROM_LOAD("gdzu36.100", 0x100000, 0x80000, CRC(e3f24234) SHA1(eb123200928221a647e10839ebb7f4628501c581))
	ROM_LOAD("gdzu37.100", 0x180000, 0x80000, CRC(2c1acb14) SHA1(4d710e09f5500da937932b4b01d862abb4a89e5a))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(godzillp_090)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("gdzcpu.090", 0x00000, 0x20000, CRC(54e3b6d7) SHA1(c11cf71140c00c96c7feea569fa04f75061b9af7))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("gzdspa.090", 0x00000, 0x80000, CRC(56dde3a4) SHA1(332f09ade962e07a2979ad7bf743f632ea942440))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("gdzu7.090", 0x0000, 0x10000, CRC(076401a9) SHA1(17aa63c2b26e6fc4849a5101ff9704606de3de65))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD("gdzu17.090", 0x000000, 0x80000, CRC(b15be745) SHA1(395631df3fef80641c189e57cddfc0ec5dcdbcef))
	ROM_LOAD("gdzu21.090", 0x080000, 0x80000, CRC(019207d5) SHA1(a98d191d686d4a04f7fad90dd0e86e8b48ff3a3b))
	ROM_LOAD("gdzu36.090", 0x100000, 0x80000, CRC(3913ccb9) SHA1(ddce224661894438a12135306484f711d10ce8be))
	ROM_LOAD("gdzu37.090", 0x180000, 0x80000, CRC(1410ae6b) SHA1(28d025403fd60b1bb132cffcc14be21be48d808c))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

/*-------------------------------------------------------------------
/ Goldeneye
/-------------------------------------------------------------------*/
ROM_START(gldneye)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("bondcpu.404", 0x00000, 0x20000, CRC(5aa6ffcc) SHA1(0c8ffcfe008a650060c42d385e91addf44f5d88e))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("bondispa.400", 0x00000, 0x80000, CRC(9cc0c710) SHA1(3c6df97d881aed9d1d08cc2a5d0c4ec020295902))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("bondu7.bin", 0x0000, 0x10000, CRC(7581a349) SHA1(493236bdc52b601a08009f9b03d64b6047d52661))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD("bondu17.bin", 0x000000, 0x80000, CRC(d9c56b9d) SHA1(df8cde0b63d6a8437a1cb239094547262c3f8774))
	ROM_LOAD("bondu21.bin", 0x080000, 0x80000, CRC(5be0f205) SHA1(aaef8f6ee6c8d5ebf08f90368061288adf850a18))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(gldneye_402)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("bondcpu.402", 0x00000, 0x20000, CRC(6faad064) SHA1(c598e9073670f0aa27e61b728c85a8ff54691b20))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("bondispa.400", 0x00000, 0x80000, CRC(9cc0c710) SHA1(3c6df97d881aed9d1d08cc2a5d0c4ec020295902))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("bondu7.bin", 0x0000, 0x10000, CRC(7581a349) SHA1(493236bdc52b601a08009f9b03d64b6047d52661))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD("bondu17.bin", 0x000000, 0x80000, CRC(d9c56b9d) SHA1(df8cde0b63d6a8437a1cb239094547262c3f8774))
	ROM_LOAD("bondu21.bin", 0x080000, 0x80000, CRC(5be0f205) SHA1(aaef8f6ee6c8d5ebf08f90368061288adf850a18))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

/*-------------------------------------------------------------------
/ Golden Cue
/-------------------------------------------------------------------*/
ROM_START(goldcue)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("gc_cpu.210", 0x00000, 0x20000, CRC(8447eaee) SHA1(dcca699508809d66dcb2887968c7628067927798))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("gc_disp.u12", 0x00000, 0x80000, CRC(87f74b9d) SHA1(e8610ba2409dc6c5070fd413597e3629851b6106))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("gc_sound.u7", 0x0000, 0x10000, CRC(8b559e39) SHA1(59c33615b53864cd542c8bd3be2ba18e91c57dfd))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD("gc_sound.u17", 0x000000, 0x80000, CRC(28c39bae) SHA1(a11343a4043d8d5a8eaec383e1bb1f42016e33d2))
	ROM_LOAD("gc_sound.u21", 0x080000, 0x80000, CRC(d3f43a37) SHA1(e845370e75200570f828b8452453287b5f599276))
	ROM_LOAD("gc_sound.u36", 0x100000, 0x80000, CRC(81f27955) SHA1(eba4250898f6de96111232e49d965b78fc6ee2e2))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

/*-------------------------------------------------------------------
/ Harley Davidson
/-------------------------------------------------------------------*/
ROM_START(harl_a13)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("harcpu.103", 0x00000, 0x20000, CRC(2a812c75) SHA1(46e1f18e1c9992ca1823f7818b6d51c001f5a934))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("hddispa.104", 0x00000, 0x80000, CRC(fc7c2924) SHA1(172fceb4d3221608f48a4abe4c4c5f3043834957))

	ROM_REGION(0x010000, "decobsmt:soundcpu",0)
	ROM_LOAD("hdsnd.u7",0x0000,0x10000,CRC(b9accb75) SHA1(9575f1c372ec5603322255778fc003047acc8b01))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("hdvc1.u17", 0x000000, CRC(0265fe72) SHA1(7bd7b321bfa2a5092cdf273dfaf4ccdb043c06e8))
	ROM_LOAD_SND_8M("hdvc2.u21", 0x080000, CRC(89230898) SHA1(42d225e33ac1d679415e9dbf659591b7c4109740))
	ROM_LOAD_SND_8M("hdvc3.u36", 0x100000, CRC(41239811) SHA1(94fceff4dbefd3467ecb8b19e4b8baf24ddd68a3))
	ROM_LOAD("hdvc4.u37", 0x180000, 0x080000, CRC(a1bc39f6) SHA1(25af40cb3d8f774e1e37cbef9166e41753440460))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(harl_u13)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("harcpuk.103", 0x00000, 0x20000, CRC(e39130a7) SHA1(7854c885a82f42f35e266e3cb96a68969d49fbad))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("hddispa.104", 0x00000, 0x80000, CRC(fc7c2924) SHA1(172fceb4d3221608f48a4abe4c4c5f3043834957))

	ROM_REGION(0x010000, "decobsmt:soundcpu",0)
	ROM_LOAD("hdsnd.u7",0x0000,0x10000,CRC(b9accb75) SHA1(9575f1c372ec5603322255778fc003047acc8b01))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("hdvc1.u17", 0x000000, CRC(0265fe72) SHA1(7bd7b321bfa2a5092cdf273dfaf4ccdb043c06e8))
	ROM_LOAD_SND_8M("hdvc2.u21", 0x080000, CRC(89230898) SHA1(42d225e33ac1d679415e9dbf659591b7c4109740))
	ROM_LOAD_SND_8M("hdvc3.u36", 0x100000, CRC(41239811) SHA1(94fceff4dbefd3467ecb8b19e4b8baf24ddd68a3))
	ROM_LOAD("hdvc4.u37", 0x180000, 0x080000, CRC(a1bc39f6) SHA1(25af40cb3d8f774e1e37cbef9166e41753440460))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(harl_a10)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("harcpu.103", 0x00000, 0x20000, CRC(2a812c75) SHA1(46e1f18e1c9992ca1823f7818b6d51c001f5a934))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("hddispa.100", 0x00000, 0x80000, CRC(bdeac0fd) SHA1(5aa1392a13f3c632b660ea6cb3dee23327404d80))

	ROM_REGION(0x010000, "decobsmt:soundcpu",0)
	ROM_LOAD("hdsnd.u7",0x0000,0x10000,CRC(b9accb75) SHA1(9575f1c372ec5603322255778fc003047acc8b01))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("hdvc1.u17", 0x000000, CRC(0265fe72) SHA1(7bd7b321bfa2a5092cdf273dfaf4ccdb043c06e8))
	ROM_LOAD_SND_8M("hdvc2.u21", 0x080000, CRC(89230898) SHA1(42d225e33ac1d679415e9dbf659591b7c4109740))
	ROM_LOAD_SND_8M("hdvc3.u36", 0x100000, CRC(41239811) SHA1(94fceff4dbefd3467ecb8b19e4b8baf24ddd68a3))
	ROM_LOAD("hdvc4.u37", 0x180000, 0x080000, CRC(a1bc39f6) SHA1(25af40cb3d8f774e1e37cbef9166e41753440460))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(harl_f13)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("harcpu.103", 0x00000, 0x20000, CRC(2a812c75) SHA1(46e1f18e1c9992ca1823f7818b6d51c001f5a934))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("hddispf.104", 0x00000, 0x80000, CRC(5f80436e) SHA1(e89e561807670118c3d9e623d4aec2321c774576))

	ROM_REGION(0x010000, "decobsmt:soundcpu",0)
	ROM_LOAD("hdsnd.u7",0x0000,0x10000,CRC(b9accb75) SHA1(9575f1c372ec5603322255778fc003047acc8b01))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("hdvc1.u17", 0x000000, CRC(0265fe72) SHA1(7bd7b321bfa2a5092cdf273dfaf4ccdb043c06e8))
	ROM_LOAD_SND_8M("hdvc2.u21", 0x080000, CRC(89230898) SHA1(42d225e33ac1d679415e9dbf659591b7c4109740))
	ROM_LOAD_SND_8M("hdvc3.u36", 0x100000, CRC(41239811) SHA1(94fceff4dbefd3467ecb8b19e4b8baf24ddd68a3))
	ROM_LOAD("hdvc4.u37", 0x180000, 0x080000, CRC(a1bc39f6) SHA1(25af40cb3d8f774e1e37cbef9166e41753440460))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(harl_g13)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("harcpu.103", 0x00000, 0x20000, CRC(2a812c75) SHA1(46e1f18e1c9992ca1823f7818b6d51c001f5a934))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("hddispg.104", 0x00000, 0x80000, CRC(c7f197a0) SHA1(3b7f0699c08d387c67ff6cd185360e60fcd21b9e))

	ROM_REGION(0x010000, "decobsmt:soundcpu",0)
	ROM_LOAD("hdsnd.u7",0x0000,0x10000,CRC(b9accb75) SHA1(9575f1c372ec5603322255778fc003047acc8b01))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("hdvc1.u17", 0x000000, CRC(0265fe72) SHA1(7bd7b321bfa2a5092cdf273dfaf4ccdb043c06e8))
	ROM_LOAD_SND_8M("hdvc2.u21", 0x080000, CRC(89230898) SHA1(42d225e33ac1d679415e9dbf659591b7c4109740))
	ROM_LOAD_SND_8M("hdvc3.u36", 0x100000, CRC(41239811) SHA1(94fceff4dbefd3467ecb8b19e4b8baf24ddd68a3))
	ROM_LOAD("hdvc4.u37", 0x180000, 0x080000, CRC(a1bc39f6) SHA1(25af40cb3d8f774e1e37cbef9166e41753440460))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(harl_i13)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("harcpu.103", 0x00000, 0x20000, CRC(2a812c75) SHA1(46e1f18e1c9992ca1823f7818b6d51c001f5a934))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("hddispi.104", 0x00000, 0x80000, CRC(387a5aad) SHA1(a0eb99b240f6044db05668c4504e908aee205220))

	ROM_REGION(0x010000, "decobsmt:soundcpu",0)
	ROM_LOAD("hdsnd.u7",0x0000,0x10000,CRC(b9accb75) SHA1(9575f1c372ec5603322255778fc003047acc8b01))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("hdvc1.u17", 0x000000, CRC(0265fe72) SHA1(7bd7b321bfa2a5092cdf273dfaf4ccdb043c06e8))
	ROM_LOAD_SND_8M("hdvc2.u21", 0x080000, CRC(89230898) SHA1(42d225e33ac1d679415e9dbf659591b7c4109740))
	ROM_LOAD_SND_8M("hdvc3.u36", 0x100000, CRC(41239811) SHA1(94fceff4dbefd3467ecb8b19e4b8baf24ddd68a3))
	ROM_LOAD("hdvc4.u37", 0x180000, 0x080000, CRC(a1bc39f6) SHA1(25af40cb3d8f774e1e37cbef9166e41753440460))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(harl_l13)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("harcpu.103", 0x00000, 0x20000, CRC(2a812c75) SHA1(46e1f18e1c9992ca1823f7818b6d51c001f5a934))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("hddisps.104", 0x00000, 0x80000, CRC(2d26514a) SHA1(f15b22cad6329f29cd5cccfb91a2ba7ca2cd6d59))

	ROM_REGION(0x010000, "decobsmt:soundcpu",0)
	ROM_LOAD("hdsnd.u7",0x0000,0x10000,CRC(b9accb75) SHA1(9575f1c372ec5603322255778fc003047acc8b01))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("hdvc1.u17", 0x000000, CRC(0265fe72) SHA1(7bd7b321bfa2a5092cdf273dfaf4ccdb043c06e8))
	ROM_LOAD_SND_8M("hdvc2.u21", 0x080000, CRC(89230898) SHA1(42d225e33ac1d679415e9dbf659591b7c4109740))
	ROM_LOAD_SND_8M("hdvc3.u36", 0x100000, CRC(41239811) SHA1(94fceff4dbefd3467ecb8b19e4b8baf24ddd68a3))
	ROM_LOAD("hdvc4.u37", 0x180000, 0x080000, CRC(a1bc39f6) SHA1(25af40cb3d8f774e1e37cbef9166e41753440460))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(harl_a30)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("harcpu.300", 0x00000, 0x20000, CRC(c4ba9df5) SHA1(6bdcd555db946e396b630953db1ba50677177137))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("hddispa.300", 0x00000, 0x80000, CRC(61b274f8) SHA1(954e4b3527cefcb24376de9f6f7e5f9192ab3304))

	ROM_REGION(0x010000, "decobsmt:soundcpu",0)
	ROM_LOAD("hdsnd.u7",0x0000,0x10000,CRC(b9accb75) SHA1(9575f1c372ec5603322255778fc003047acc8b01))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("hdvc1.u17", 0x000000, CRC(0265fe72) SHA1(7bd7b321bfa2a5092cdf273dfaf4ccdb043c06e8))
	ROM_LOAD_SND_8M("hdvc2.u21", 0x080000, CRC(89230898) SHA1(42d225e33ac1d679415e9dbf659591b7c4109740))
	ROM_LOAD_SND_8M("hdvc3.u36", 0x100000, CRC(41239811) SHA1(94fceff4dbefd3467ecb8b19e4b8baf24ddd68a3))
	ROM_LOAD("hdvc4.u37", 0x180000, 0x080000, CRC(a1bc39f6) SHA1(25af40cb3d8f774e1e37cbef9166e41753440460))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(harl_f30)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("harcpu.300", 0x00000, 0x20000, CRC(c4ba9df5) SHA1(6bdcd555db946e396b630953db1ba50677177137))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("hddispf.300", 0x00000, 0x80000, CRC(106f7f1f) SHA1(92a8ab7d834439a2211208e0812cdb1199acb21d))

	ROM_REGION(0x010000, "decobsmt:soundcpu",0)
	ROM_LOAD("hdsnd.u7",0x0000,0x10000,CRC(b9accb75) SHA1(9575f1c372ec5603322255778fc003047acc8b01))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("hdvc1.u17", 0x000000, CRC(0265fe72) SHA1(7bd7b321bfa2a5092cdf273dfaf4ccdb043c06e8))
	ROM_LOAD_SND_8M("hdvc2.u21", 0x080000, CRC(89230898) SHA1(42d225e33ac1d679415e9dbf659591b7c4109740))
	ROM_LOAD_SND_8M("hdvc3.u36", 0x100000, CRC(41239811) SHA1(94fceff4dbefd3467ecb8b19e4b8baf24ddd68a3))
	ROM_LOAD("hdvc4.u37", 0x180000, 0x080000, CRC(a1bc39f6) SHA1(25af40cb3d8f774e1e37cbef9166e41753440460))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(harl_g30)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("harcpu.300", 0x00000, 0x20000, CRC(c4ba9df5) SHA1(6bdcd555db946e396b630953db1ba50677177137))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("hddispg.300", 0x00000, 0x80000, CRC(8f7da748) SHA1(fee1534b76769517d4e6dbed373583e573fb95b6))

	ROM_REGION(0x010000, "decobsmt:soundcpu",0)
	ROM_LOAD("hdsnd.u7",0x0000,0x10000,CRC(b9accb75) SHA1(9575f1c372ec5603322255778fc003047acc8b01))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("hdvc1.u17", 0x000000, CRC(0265fe72) SHA1(7bd7b321bfa2a5092cdf273dfaf4ccdb043c06e8))
	ROM_LOAD_SND_8M("hdvc2.u21", 0x080000, CRC(89230898) SHA1(42d225e33ac1d679415e9dbf659591b7c4109740))
	ROM_LOAD_SND_8M("hdvc3.u36", 0x100000, CRC(41239811) SHA1(94fceff4dbefd3467ecb8b19e4b8baf24ddd68a3))
	ROM_LOAD("hdvc4.u37", 0x180000, 0x080000, CRC(a1bc39f6) SHA1(25af40cb3d8f774e1e37cbef9166e41753440460))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(harl_i30)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("harcpu.300", 0x00000, 0x20000, CRC(c4ba9df5) SHA1(6bdcd555db946e396b630953db1ba50677177137))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("hddispi.300", 0x00000, 0x80000, CRC(686d3cf6) SHA1(fb27e2e4b39abb56deb1e66f012d151126971474))

	ROM_REGION(0x010000, "decobsmt:soundcpu",0)
	ROM_LOAD("hdsnd.u7",0x0000,0x10000,CRC(b9accb75) SHA1(9575f1c372ec5603322255778fc003047acc8b01))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("hdvc1.u17", 0x000000, CRC(0265fe72) SHA1(7bd7b321bfa2a5092cdf273dfaf4ccdb043c06e8))
	ROM_LOAD_SND_8M("hdvc2.u21", 0x080000, CRC(89230898) SHA1(42d225e33ac1d679415e9dbf659591b7c4109740))
	ROM_LOAD_SND_8M("hdvc3.u36", 0x100000, CRC(41239811) SHA1(94fceff4dbefd3467ecb8b19e4b8baf24ddd68a3))
	ROM_LOAD("hdvc4.u37", 0x180000, 0x080000, CRC(a1bc39f6) SHA1(25af40cb3d8f774e1e37cbef9166e41753440460))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(harl_l30)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("harcpu.300", 0x00000, 0x20000, CRC(c4ba9df5) SHA1(6bdcd555db946e396b630953db1ba50677177137))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("hddispl.300", 0x00000, 0x80000, CRC(4cc7251b) SHA1(7660fca37ac9fb442a059ddbafc2fa13f94dfae1))

	ROM_REGION(0x010000, "decobsmt:soundcpu",0)
	ROM_LOAD("hdsnd.u7",0x0000,0x10000,CRC(b9accb75) SHA1(9575f1c372ec5603322255778fc003047acc8b01))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("hdvc1.u17", 0x000000, CRC(0265fe72) SHA1(7bd7b321bfa2a5092cdf273dfaf4ccdb043c06e8))
	ROM_LOAD_SND_8M("hdvc2.u21", 0x080000, CRC(89230898) SHA1(42d225e33ac1d679415e9dbf659591b7c4109740))
	ROM_LOAD_SND_8M("hdvc3.u36", 0x100000, CRC(41239811) SHA1(94fceff4dbefd3467ecb8b19e4b8baf24ddd68a3))
	ROM_LOAD("hdvc4.u37", 0x180000, 0x080000, CRC(a1bc39f6) SHA1(25af40cb3d8f774e1e37cbef9166e41753440460))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(harl_a18)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("harcpu.108", 0x00000, 0x20000, CRC(a8e24328) SHA1(cc32d97521f706e3d8ddcf4117ec0c0e7424a378))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("hddispa.105", 0x00000, 0x80000, CRC(401a7b9f) SHA1(37e99a42738c1147c073585391772ecc55c9a759))

	ROM_REGION(0x010000, "decobsmt:soundcpu",0)
	ROM_LOAD("hdsnd.u7",0x0000,0x10000,CRC(b9accb75) SHA1(9575f1c372ec5603322255778fc003047acc8b01))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("hdvc1.u17", 0x000000, CRC(0265fe72) SHA1(7bd7b321bfa2a5092cdf273dfaf4ccdb043c06e8))
	ROM_LOAD_SND_8M("hdvc2.u21", 0x080000, CRC(89230898) SHA1(42d225e33ac1d679415e9dbf659591b7c4109740))
	ROM_LOAD_SND_8M("hdvc3.u36", 0x100000, CRC(41239811) SHA1(94fceff4dbefd3467ecb8b19e4b8baf24ddd68a3))
	ROM_LOAD("hdvc4.u37", 0x180000, 0x080000, CRC(a1bc39f6) SHA1(25af40cb3d8f774e1e37cbef9166e41753440460))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(harl_f18)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("harcpu.108", 0x00000, 0x20000, CRC(a8e24328) SHA1(cc32d97521f706e3d8ddcf4117ec0c0e7424a378))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("hddispf.105", 0x00000, 0x80000, CRC(31c77078) SHA1(8a0e2dbb698da77dffa1ab01df0f360fecf6c4c7))

	ROM_REGION(0x010000, "decobsmt:soundcpu",0)
	ROM_LOAD("hdsnd.u7",0x0000,0x10000,CRC(b9accb75) SHA1(9575f1c372ec5603322255778fc003047acc8b01))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("hdvc1.u17", 0x000000, CRC(0265fe72) SHA1(7bd7b321bfa2a5092cdf273dfaf4ccdb043c06e8))
	ROM_LOAD_SND_8M("hdvc2.u21", 0x080000, CRC(89230898) SHA1(42d225e33ac1d679415e9dbf659591b7c4109740))
	ROM_LOAD_SND_8M("hdvc3.u36", 0x100000, CRC(41239811) SHA1(94fceff4dbefd3467ecb8b19e4b8baf24ddd68a3))
	ROM_LOAD("hdvc4.u37", 0x180000, 0x080000, CRC(a1bc39f6) SHA1(25af40cb3d8f774e1e37cbef9166e41753440460))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(harl_g18)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("harcpu.108", 0x00000, 0x20000, CRC(a8e24328) SHA1(cc32d97521f706e3d8ddcf4117ec0c0e7424a378))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("hddispg.105", 0x00000, 0x80000, CRC(aed5a82f) SHA1(4c44b052a9b1fa702ff49c9b2fb7cf48173459d2))

	ROM_REGION(0x010000, "decobsmt:soundcpu",0)
	ROM_LOAD("hdsnd.u7",0x0000,0x10000,CRC(b9accb75) SHA1(9575f1c372ec5603322255778fc003047acc8b01))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("hdvc1.u17", 0x000000, CRC(0265fe72) SHA1(7bd7b321bfa2a5092cdf273dfaf4ccdb043c06e8))
	ROM_LOAD_SND_8M("hdvc2.u21", 0x080000, CRC(89230898) SHA1(42d225e33ac1d679415e9dbf659591b7c4109740))
	ROM_LOAD_SND_8M("hdvc3.u36", 0x100000, CRC(41239811) SHA1(94fceff4dbefd3467ecb8b19e4b8baf24ddd68a3))
	ROM_LOAD("hdvc4.u37", 0x180000, 0x080000, CRC(a1bc39f6) SHA1(25af40cb3d8f774e1e37cbef9166e41753440460))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(harl_i18)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("harcpu.108", 0x00000, 0x20000, CRC(a8e24328) SHA1(cc32d97521f706e3d8ddcf4117ec0c0e7424a378))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("hddispi.105", 0x00000, 0x80000, CRC(49c53391) SHA1(98f88eb8a49bbc59f78996d713c72ec495ba806f))

	ROM_REGION(0x010000, "decobsmt:soundcpu",0)
	ROM_LOAD("hdsnd.u7",0x0000,0x10000,CRC(b9accb75) SHA1(9575f1c372ec5603322255778fc003047acc8b01))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("hdvc1.u17", 0x000000, CRC(0265fe72) SHA1(7bd7b321bfa2a5092cdf273dfaf4ccdb043c06e8))
	ROM_LOAD_SND_8M("hdvc2.u21", 0x080000, CRC(89230898) SHA1(42d225e33ac1d679415e9dbf659591b7c4109740))
	ROM_LOAD_SND_8M("hdvc3.u36", 0x100000, CRC(41239811) SHA1(94fceff4dbefd3467ecb8b19e4b8baf24ddd68a3))
	ROM_LOAD("hdvc4.u37", 0x180000, 0x080000, CRC(a1bc39f6) SHA1(25af40cb3d8f774e1e37cbef9166e41753440460))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(harl_l18)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("harcpu.108", 0x00000, 0x20000, CRC(a8e24328) SHA1(cc32d97521f706e3d8ddcf4117ec0c0e7424a378))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("hddisps.105", 0x00000, 0x80000, CRC(6d6f2a7c) SHA1(1609c69a1584398c3504bb5a0c46f878e8dd547c))

	ROM_REGION(0x010000, "decobsmt:soundcpu",0)
	ROM_LOAD("hdsnd.u7",0x0000,0x10000,CRC(b9accb75) SHA1(9575f1c372ec5603322255778fc003047acc8b01))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("hdvc1.u17", 0x000000, CRC(0265fe72) SHA1(7bd7b321bfa2a5092cdf273dfaf4ccdb043c06e8))
	ROM_LOAD_SND_8M("hdvc2.u21", 0x080000, CRC(89230898) SHA1(42d225e33ac1d679415e9dbf659591b7c4109740))
	ROM_LOAD_SND_8M("hdvc3.u36", 0x100000, CRC(41239811) SHA1(94fceff4dbefd3467ecb8b19e4b8baf24ddd68a3))
	ROM_LOAD("hdvc4.u37", 0x180000, 0x080000, CRC(a1bc39f6) SHA1(25af40cb3d8f774e1e37cbef9166e41753440460))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(harl_a40)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("harcpu.400", 0x00000, 0x20000, CRC(752ed258) SHA1(aea0ab3c45649178a3b0e17a2eacc516600a2b63))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("hddispa.400", 0x00000, 0x80000, CRC(e2c98397) SHA1(212ac1a509f608c490dc4dfdc5cc04187ed2fe10))

	ROM_REGION(0x010000, "decobsmt:soundcpu",0)
	ROM_LOAD("hdsnd.u7",0x0000,0x10000,CRC(b9accb75) SHA1(9575f1c372ec5603322255778fc003047acc8b01))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("hdvc1.u17", 0x000000, CRC(0265fe72) SHA1(7bd7b321bfa2a5092cdf273dfaf4ccdb043c06e8))
	ROM_LOAD_SND_8M("hdvc2.u21", 0x080000, CRC(89230898) SHA1(42d225e33ac1d679415e9dbf659591b7c4109740))
	ROM_LOAD_SND_8M("hdvc3.u36", 0x100000, CRC(41239811) SHA1(94fceff4dbefd3467ecb8b19e4b8baf24ddd68a3))
	ROM_LOAD("hdvc4.u37", 0x180000, 0x080000, CRC(a1bc39f6) SHA1(25af40cb3d8f774e1e37cbef9166e41753440460))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(harl_f40)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("harcpu.400", 0x00000, 0x20000, CRC(752ed258) SHA1(aea0ab3c45649178a3b0e17a2eacc516600a2b63))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("hddispf.400", 0x00000, 0x80000, CRC(d061c238) SHA1(cb29e58970d43c2845c96e149e8fdd0c16e501c9))

	ROM_REGION(0x010000, "decobsmt:soundcpu",0)
	ROM_LOAD("hdsnd.u7",0x0000,0x10000,CRC(b9accb75) SHA1(9575f1c372ec5603322255778fc003047acc8b01))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("hdvc1.u17", 0x000000, CRC(0265fe72) SHA1(7bd7b321bfa2a5092cdf273dfaf4ccdb043c06e8))
	ROM_LOAD_SND_8M("hdvc2.u21", 0x080000, CRC(89230898) SHA1(42d225e33ac1d679415e9dbf659591b7c4109740))
	ROM_LOAD_SND_8M("hdvc3.u36", 0x100000, CRC(41239811) SHA1(94fceff4dbefd3467ecb8b19e4b8baf24ddd68a3))
	ROM_LOAD("hdvc4.u37", 0x180000, 0x080000, CRC(a1bc39f6) SHA1(25af40cb3d8f774e1e37cbef9166e41753440460))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(harl_g40)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("harcpu.400", 0x00000, 0x20000, CRC(752ed258) SHA1(aea0ab3c45649178a3b0e17a2eacc516600a2b63))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("hddispg.400", 0x00000, 0x80000, CRC(4bc89a23) SHA1(fb7dcc61194560845e150bc1c032c098ffd026e8))

	ROM_REGION(0x010000, "decobsmt:soundcpu",0)
	ROM_LOAD("hdsnd.u7",0x0000,0x10000,CRC(b9accb75) SHA1(9575f1c372ec5603322255778fc003047acc8b01))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("hdvc1.u17", 0x000000, CRC(0265fe72) SHA1(7bd7b321bfa2a5092cdf273dfaf4ccdb043c06e8))
	ROM_LOAD_SND_8M("hdvc2.u21", 0x080000, CRC(89230898) SHA1(42d225e33ac1d679415e9dbf659591b7c4109740))
	ROM_LOAD_SND_8M("hdvc3.u36", 0x100000, CRC(41239811) SHA1(94fceff4dbefd3467ecb8b19e4b8baf24ddd68a3))
	ROM_LOAD("hdvc4.u37", 0x180000, 0x080000, CRC(a1bc39f6) SHA1(25af40cb3d8f774e1e37cbef9166e41753440460))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(harl_i40)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("harcpu.400", 0x00000, 0x20000, CRC(752ed258) SHA1(aea0ab3c45649178a3b0e17a2eacc516600a2b63))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("hddispi.400", 0x00000, 0x80000, CRC(c4fc4990) SHA1(79d501c3123b604becbb87c12aca9848675811ec))

	ROM_REGION(0x010000, "decobsmt:soundcpu",0)
	ROM_LOAD("hdsnd.u7",0x0000,0x10000,CRC(b9accb75) SHA1(9575f1c372ec5603322255778fc003047acc8b01))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("hdvc1.u17", 0x000000, CRC(0265fe72) SHA1(7bd7b321bfa2a5092cdf273dfaf4ccdb043c06e8))
	ROM_LOAD_SND_8M("hdvc2.u21", 0x080000, CRC(89230898) SHA1(42d225e33ac1d679415e9dbf659591b7c4109740))
	ROM_LOAD_SND_8M("hdvc3.u36", 0x100000, CRC(41239811) SHA1(94fceff4dbefd3467ecb8b19e4b8baf24ddd68a3))
	ROM_LOAD("hdvc4.u37", 0x180000, 0x080000, CRC(a1bc39f6) SHA1(25af40cb3d8f774e1e37cbef9166e41753440460))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(harl_l40)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("harcpu.400", 0x00000, 0x20000, CRC(752ed258) SHA1(aea0ab3c45649178a3b0e17a2eacc516600a2b63))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("hddispl.400", 0x00000, 0x80000, CRC(96096e73) SHA1(b22f03ab3f08ff192a55e92ebe85bafa893c6234))

	ROM_REGION(0x010000, "decobsmt:soundcpu",0)
	ROM_LOAD("hdsnd.u7",0x0000,0x10000,CRC(b9accb75) SHA1(9575f1c372ec5603322255778fc003047acc8b01))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("hdvc1.u17", 0x000000, CRC(0265fe72) SHA1(7bd7b321bfa2a5092cdf273dfaf4ccdb043c06e8))
	ROM_LOAD_SND_8M("hdvc2.u21", 0x080000, CRC(89230898) SHA1(42d225e33ac1d679415e9dbf659591b7c4109740))
	ROM_LOAD_SND_8M("hdvc3.u36", 0x100000, CRC(41239811) SHA1(94fceff4dbefd3467ecb8b19e4b8baf24ddd68a3))
	ROM_LOAD("hdvc4.u37", 0x180000, 0x080000, CRC(a1bc39f6) SHA1(25af40cb3d8f774e1e37cbef9166e41753440460))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

/*-------------------------------------------------------------------
/ High Roller Casino
/-------------------------------------------------------------------*/
ROM_START(hirolcas)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("hrccpu.300", 0x00000, 0x20000, CRC(0d1117fa) SHA1(a19f9dfc2288fc16cb8e992ffd7f13e70ef042c7))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("hrcdispa.300", 0x00000, 0x80000, CRC(099ccaf0) SHA1(2e0c2706881208f08e8a1d30915424c8f9b1cf67))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("hrsndu7.100",0x0000,0x10000,CRC(c41f91a7) SHA1(2af3be10754ea633558bdbeded21c6f82d85cd1d))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("hrsndu17.100", 0x000000, CRC(5858dfd0) SHA1(0d88daf3b480f0e0a2653d9be37cafed79036a48))
	ROM_LOAD_SND_8M("hrsndu21.100", 0x080000, CRC(c653de96) SHA1(352567f4f4f973ed3d8c018c9bf37aeecdd101bf))
	ROM_LOAD_SND_8M("hrsndu36.100", 0x100000, CRC(5634eb4e) SHA1(8c76f49423fc0d7887aa5c57ad029b7371372739))
	ROM_LOAD_SND_8M("hrsndu37.100", 0x180000, CRC(d4d23c00) SHA1(c574dc4553bff693d9216229ce38a55f69e7368a))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(hirolcas_210)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("hrccpu.210", 0x00000, 0x20000, CRC(2e3c682a) SHA1(d9993ae7a0aad80e1eeff226a635873cb25437ce))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("hrcdispa.200", 0x00000, 0x80000, CRC(642bdce7) SHA1(7cd922a15c1443c6ed7636c9def4bc3ab0b47096))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("hrsndu7.100",0x0000,0x10000,CRC(c41f91a7) SHA1(2af3be10754ea633558bdbeded21c6f82d85cd1d))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("hrsndu17.100", 0x000000, CRC(5858dfd0) SHA1(0d88daf3b480f0e0a2653d9be37cafed79036a48))
	ROM_LOAD_SND_8M("hrsndu21.100", 0x080000, CRC(c653de96) SHA1(352567f4f4f973ed3d8c018c9bf37aeecdd101bf))
	ROM_LOAD_SND_8M("hrsndu36.100", 0x100000, CRC(5634eb4e) SHA1(8c76f49423fc0d7887aa5c57ad029b7371372739))
	ROM_LOAD_SND_8M("hrsndu37.100", 0x180000, CRC(d4d23c00) SHA1(c574dc4553bff693d9216229ce38a55f69e7368a))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(hironew)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("hrccpu.300", 0x00000, 0x20000, CRC(0d1117fa) SHA1(a19f9dfc2288fc16cb8e992ffd7f13e70ef042c7))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("hrcdispa.300", 0x00000, 0x80000, CRC(099ccaf0) SHA1(2e0c2706881208f08e8a1d30915424c8f9b1cf67))

	ROM_REGION32_LE(0x600000, "soundcpu", ROMREGION_ERASEMASK | 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x1000000, "bsmt", 0 )
	ROM_LOAD("hrsndu7.100",0x0000,0x10000,CRC(c41f91a7) SHA1(2af3be10754ea633558bdbeded21c6f82d85cd1d))

	ROM_REGION(0x400000, "sound2", 0)
	ROM_LOAD("hrsndu17.100", 0x000000, 0x100000, CRC(5858dfd0) SHA1(0d88daf3b480f0e0a2653d9be37cafed79036a48))
	ROM_LOAD("hrsndu21.100", 0x100000, 0x100000, CRC(c653de96) SHA1(352567f4f4f973ed3d8c018c9bf37aeecdd101bf))
	ROM_LOAD("hrsndu36.100", 0x200000, 0x100000, CRC(5634eb4e) SHA1(8c76f49423fc0d7887aa5c57ad029b7371372739))
	ROM_LOAD("hrsndu37.100", 0x300000, 0x100000, CRC(d4d23c00) SHA1(c574dc4553bff693d9216229ce38a55f69e7368a))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(hirolcat)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("hrccput3.300", 0x00000, 0x20000, CRC(b70e04a0) SHA1(0c8d6c1e488471617ba9e24704d0d44826c1daf3))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("hrcdspt3.300", 0x00000, 0x80000, CRC(e262f36c) SHA1(116b2b96adce953e00d1e6d7f2b4ed4cdc4a3f61))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("hrsndu7.100",0x0000,0x10000,CRC(c41f91a7) SHA1(2af3be10754ea633558bdbeded21c6f82d85cd1d))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("hrsndu17.100", 0x000000, CRC(5858dfd0) SHA1(0d88daf3b480f0e0a2653d9be37cafed79036a48))
	ROM_LOAD_SND_8M("hrsndu21.100", 0x080000, CRC(c653de96) SHA1(352567f4f4f973ed3d8c018c9bf37aeecdd101bf))
	ROM_LOAD_SND_8M("hrsndu36.100", 0x100000, CRC(5634eb4e) SHA1(8c76f49423fc0d7887aa5c57ad029b7371372739))
	ROM_LOAD_SND_8M("hrsndu37.100", 0x180000, CRC(d4d23c00) SHA1(c574dc4553bff693d9216229ce38a55f69e7368a))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(hirol_fr)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("hrccpu.300", 0x00000, 0x20000, CRC(0d1117fa) SHA1(a19f9dfc2288fc16cb8e992ffd7f13e70ef042c7))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("hrcdispf.300", 0x00000, 0x80000, CRC(1fb5046b) SHA1(8b121a9c75a7d9a312b8c03615838b748d149819))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("hrsndu7.100",0x0000,0x10000,CRC(c41f91a7) SHA1(2af3be10754ea633558bdbeded21c6f82d85cd1d))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("hrsndu17.100", 0x000000, CRC(5858dfd0) SHA1(0d88daf3b480f0e0a2653d9be37cafed79036a48))
	ROM_LOAD_SND_8M("hrsndu21.100", 0x080000, CRC(c653de96) SHA1(352567f4f4f973ed3d8c018c9bf37aeecdd101bf))
	ROM_LOAD_SND_8M("hrsndu36.100", 0x100000, CRC(5634eb4e) SHA1(8c76f49423fc0d7887aa5c57ad029b7371372739))
	ROM_LOAD_SND_8M("hrsndu37.100", 0x180000, CRC(d4d23c00) SHA1(c574dc4553bff693d9216229ce38a55f69e7368a))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(hirol_gr)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("hrccpu.300", 0x00000, 0x20000, CRC(0d1117fa) SHA1(a19f9dfc2288fc16cb8e992ffd7f13e70ef042c7))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("hrcdispg.300", 0x00000, 0x80000, CRC(a880903a) SHA1(4049f50ceaeb6c9e869150ec3d903775cdd865ff))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("hrsndu7.100",0x0000,0x10000,CRC(c41f91a7) SHA1(2af3be10754ea633558bdbeded21c6f82d85cd1d))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("hrsndu17.100", 0x000000, CRC(5858dfd0) SHA1(0d88daf3b480f0e0a2653d9be37cafed79036a48))
	ROM_LOAD_SND_8M("hrsndu21.100", 0x080000, CRC(c653de96) SHA1(352567f4f4f973ed3d8c018c9bf37aeecdd101bf))
	ROM_LOAD_SND_8M("hrsndu36.100", 0x100000, CRC(5634eb4e) SHA1(8c76f49423fc0d7887aa5c57ad029b7371372739))
	ROM_LOAD_SND_8M("hrsndu37.100", 0x180000, CRC(d4d23c00) SHA1(c574dc4553bff693d9216229ce38a55f69e7368a))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(hirol_gr_210)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("hrccpu.210", 0x00000, 0x20000, CRC(2e3c682a) SHA1(d9993ae7a0aad80e1eeff226a635873cb25437ce))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("hrcdispg.201", 0x00000, 0x80000, CRC(57b95712) SHA1(f7abe7511aa8b258615cd844dc76f3d2f9b47c31))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("hrsndu7.100",0x0000,0x10000,CRC(c41f91a7) SHA1(2af3be10754ea633558bdbeded21c6f82d85cd1d))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("hrsndu17.100", 0x000000, CRC(5858dfd0) SHA1(0d88daf3b480f0e0a2653d9be37cafed79036a48))
	ROM_LOAD_SND_8M("hrsndu21.100", 0x080000, CRC(c653de96) SHA1(352567f4f4f973ed3d8c018c9bf37aeecdd101bf))
	ROM_LOAD_SND_8M("hrsndu36.100", 0x100000, CRC(5634eb4e) SHA1(8c76f49423fc0d7887aa5c57ad029b7371372739))
	ROM_LOAD_SND_8M("hrsndu37.100", 0x180000, CRC(d4d23c00) SHA1(c574dc4553bff693d9216229ce38a55f69e7368a))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(hirol_it)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("hrccpu.300", 0x00000, 0x20000, CRC(0d1117fa) SHA1(a19f9dfc2288fc16cb8e992ffd7f13e70ef042c7))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("hrcdispi.300", 0x00000, 0x80000, CRC(2734f746) SHA1(aa924d998b6c3fbd80e9325093c9b3267dfaadef))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("hrsndu7.100",0x0000,0x10000,CRC(c41f91a7) SHA1(2af3be10754ea633558bdbeded21c6f82d85cd1d))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("hrsndu17.100", 0x000000, CRC(5858dfd0) SHA1(0d88daf3b480f0e0a2653d9be37cafed79036a48))
	ROM_LOAD_SND_8M("hrsndu21.100", 0x080000, CRC(c653de96) SHA1(352567f4f4f973ed3d8c018c9bf37aeecdd101bf))
	ROM_LOAD_SND_8M("hrsndu36.100", 0x100000, CRC(5634eb4e) SHA1(8c76f49423fc0d7887aa5c57ad029b7371372739))
	ROM_LOAD_SND_8M("hrsndu37.100", 0x180000, CRC(d4d23c00) SHA1(c574dc4553bff693d9216229ce38a55f69e7368a))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

/*-------------------------------------------------------------------
/ Independence Day
/-------------------------------------------------------------------*/
ROM_START(id4)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("id4cpu.202", 0x00000, 0x20000, CRC(108d88fd) SHA1(8317944201acfb97dadfdd364696c9e81a21d2c5))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("id4dspa.200", 0x00000, 0x80000, CRC(2d3fbcc4) SHA1(0bd69ebb68ae880ac9aae40916f13e1ff84ecfaa))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("id4sndu7.512", 0x0000, 0x10000, CRC(deeaed37) SHA1(06d79967a25af0b90a5f1d6360a5b5fdbb972d5a))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD("id4sdu17.400", 0x000000, 0x80000, CRC(89ffeca3) SHA1(b94c60e3a433f797d6c5ea793c3ecff0a3b6ba60))
	ROM_LOAD("id4sdu21.400", 0x080000, 0x80000, CRC(f384a9ab) SHA1(06bd607e7efd761017a7b605e0294a34e4c6255c))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(id4f)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("id4cpu.202", 0x00000, 0x20000, CRC(108d88fd) SHA1(8317944201acfb97dadfdd364696c9e81a21d2c5))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("id4dspf.200", 0x00000, 0x80000, CRC(4b52676b) SHA1(a881efb28d8bab424d8c12be2c16b8afc7472208))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("id4sndu7.512", 0x0000, 0x10000, CRC(deeaed37) SHA1(06d79967a25af0b90a5f1d6360a5b5fdbb972d5a))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD("id4sdu17.400", 0x000000, 0x80000, CRC(89ffeca3) SHA1(b94c60e3a433f797d6c5ea793c3ecff0a3b6ba60))
	ROM_LOAD("id4sdu21.400", 0x080000, 0x80000, CRC(f384a9ab) SHA1(06bd607e7efd761017a7b605e0294a34e4c6255c))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(id4_201)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("id4cpu.201", 0x00000, 0x20000, CRC(c0cd47a1) SHA1(63bb6da28b4f6fcc8525a8f1a6d262e35931efc9))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("id4dspa.200", 0x00000, 0x80000, CRC(2d3fbcc4) SHA1(0bd69ebb68ae880ac9aae40916f13e1ff84ecfaa))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("id4sndu7.512", 0x0000, 0x10000, CRC(deeaed37) SHA1(06d79967a25af0b90a5f1d6360a5b5fdbb972d5a))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD("id4sdu17.400", 0x000000, 0x80000, CRC(89ffeca3) SHA1(b94c60e3a433f797d6c5ea793c3ecff0a3b6ba60))
	ROM_LOAD("id4sdu21.400", 0x080000, 0x80000, CRC(f384a9ab) SHA1(06bd607e7efd761017a7b605e0294a34e4c6255c))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(id4f_201)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("id4cpu.201", 0x00000, 0x20000, CRC(c0cd47a1) SHA1(63bb6da28b4f6fcc8525a8f1a6d262e35931efc9))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("id4dspf.200", 0x00000, 0x80000, CRC(4b52676b) SHA1(a881efb28d8bab424d8c12be2c16b8afc7472208))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("id4sndu7.512", 0x0000, 0x10000, CRC(deeaed37) SHA1(06d79967a25af0b90a5f1d6360a5b5fdbb972d5a))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD("id4sdu17.400", 0x000000, 0x80000, CRC(89ffeca3) SHA1(b94c60e3a433f797d6c5ea793c3ecff0a3b6ba60))
	ROM_LOAD("id4sdu21.400", 0x080000, 0x80000, CRC(f384a9ab) SHA1(06bd607e7efd761017a7b605e0294a34e4c6255c))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

/*-------------------------------------------------------------------
/ Irons & Woods
/-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
/ Lost in Space
/-------------------------------------------------------------------*/
ROM_START(lostspc)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("liscpu.101", 0x00000, 0x20000, CRC(81b2ced8) SHA1(a1933e2686b2a4e48d0f327593df95a927b132cb))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("lisdspa.102", 0x00000, 0x80000, CRC(e8bf4a58) SHA1(572313fb79e5a0c0034938a09b04ef43fc235c84))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("lisu7.100", 0x0000, 0x10000, CRC(96e6b3c4) SHA1(5cfb43b8c182aed4b49ad1b8803812a18c6c8b6f))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD("lisu17.100", 0x000000, 0x80000, CRC(69076939) SHA1(f2cdf61a2b469d1a69eb3f08fc6e511d72336586))
	ROM_LOAD("lisu21.100", 0x080000, 0x80000, CRC(56eede09) SHA1(9ff53d7a188bd7293ad92089d143bd54623a50d4))
	ROM_LOAD("lisu36.100", 0x100000, 0x80000, CRC(56f2c53b) SHA1(5c2daf17116016fbead1320eb150cf655984662b))
	ROM_LOAD("lisu37.100", 0x180000, 0x80000, CRC(f9430c59) SHA1(f0f7169e63fc12d29fe39cd24dd67c5fb17779f7))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(lostspcf)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("liscpu.101", 0x00000, 0x20000, CRC(81b2ced8) SHA1(a1933e2686b2a4e48d0f327593df95a927b132cb))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("lis_f-102.bin", 0x00000, 0x80000, CRC(422ba6d5) SHA1(0cd09b14a953fda39f8c7e5521c4115d2ada9186))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("lisu7.100", 0x0000, 0x10000, CRC(96e6b3c4) SHA1(5cfb43b8c182aed4b49ad1b8803812a18c6c8b6f))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD("lisu17.100", 0x000000, 0x80000, CRC(69076939) SHA1(f2cdf61a2b469d1a69eb3f08fc6e511d72336586))
	ROM_LOAD("lisu21.100", 0x080000, 0x80000, CRC(56eede09) SHA1(9ff53d7a188bd7293ad92089d143bd54623a50d4))
	ROM_LOAD("lisu36.100", 0x100000, 0x80000, CRC(56f2c53b) SHA1(5c2daf17116016fbead1320eb150cf655984662b))
	ROM_LOAD("lisu37.100", 0x180000, 0x80000, CRC(f9430c59) SHA1(f0f7169e63fc12d29fe39cd24dd67c5fb17779f7))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(lostspcg)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("liscpu.101", 0x00000, 0x20000, CRC(81b2ced8) SHA1(a1933e2686b2a4e48d0f327593df95a927b132cb))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("lisdspg.102", 0x00000, 0x80000, CRC(66f3feb7) SHA1(a1f718193998f3210fb25c1353e4ae6703802311))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("lisu7.100", 0x0000, 0x10000, CRC(96e6b3c4) SHA1(5cfb43b8c182aed4b49ad1b8803812a18c6c8b6f))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD("lisu17.100", 0x000000, 0x80000, CRC(69076939) SHA1(f2cdf61a2b469d1a69eb3f08fc6e511d72336586))
	ROM_LOAD("lisu21.100", 0x080000, 0x80000, CRC(56eede09) SHA1(9ff53d7a188bd7293ad92089d143bd54623a50d4))
	ROM_LOAD("lisu36.100", 0x100000, 0x80000, CRC(56f2c53b) SHA1(5c2daf17116016fbead1320eb150cf655984662b))
	ROM_LOAD("lisu37.100", 0x180000, 0x80000, CRC(f9430c59) SHA1(f0f7169e63fc12d29fe39cd24dd67c5fb17779f7))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(lostspc_d1)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("liscpu.101", 0x00000, 0x20000, CRC(81b2ced8) SHA1(a1933e2686b2a4e48d0f327593df95a927b132cb))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("lisdspa.101", 0x00000, 0x80000, CRC(a8bfa71f) SHA1(45886ae8edcfd26a2225914aaf96eb960fc7e988))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("lisu7.100", 0x0000, 0x10000, CRC(96e6b3c4) SHA1(5cfb43b8c182aed4b49ad1b8803812a18c6c8b6f))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD("lisu17.100", 0x000000, 0x80000, CRC(69076939) SHA1(f2cdf61a2b469d1a69eb3f08fc6e511d72336586))
	ROM_LOAD("lisu21.100", 0x080000, 0x80000, CRC(56eede09) SHA1(9ff53d7a188bd7293ad92089d143bd54623a50d4))
	ROM_LOAD("lisu36.100", 0x100000, 0x80000, CRC(56f2c53b) SHA1(5c2daf17116016fbead1320eb150cf655984662b))
	ROM_LOAD("lisu37.100", 0x180000, 0x80000, CRC(f9430c59) SHA1(f0f7169e63fc12d29fe39cd24dd67c5fb17779f7))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

/*-------------------------------------------------------------------
/ Mini-Viper
/-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
/ Monopoly (3.20)
/-------------------------------------------------------------------*/
ROM_START(monopolp)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("moncpu.320", 0x00000, 0x20000, CRC(6c107c8b) SHA1(236d85b971c70a30e663787d643e6d589591d582))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("mondsp-a.301", 0x00000, 0x80000, CRC(c4e2e032) SHA1(691f7b6ed0616338683f7e3f316d64a70db58dd4))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("mnsndu7.100",0x0000,0x10000,CRC(400442e7) SHA1(d6c075dc439d5366b7ae71b5a523b86543b1ecd6))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("mnsndu17.100", 0x000000, CRC(f9bc55e8) SHA1(7dc41521305021961927ebde4dcf22611e3d622d))
	ROM_LOAD_SND_8M("mnsndu21.100", 0x080000, CRC(e0727e1f) SHA1(2093dba6e2f59cd1d1fc49c8d995b603ea0913ba))
	ROM_LOAD_SND_8M("mnsndu36.100", 0x100000, CRC(c845aa97) SHA1(2632aa8c5576b7afcb96693fa524c7d0350ac9a8))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

/*-------------------------------------------------------------------
/ Monopoly (3.03)
/-------------------------------------------------------------------*/
ROM_START(monop303)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("moncpu.303", 0x00000, 0x20000, CRC(4a66c9e4) SHA1(a368b0ced32f1017e781a59108670b979b50c9d7))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("mondsp-a.301", 0x00000, 0x80000, CRC(c4e2e032) SHA1(691f7b6ed0616338683f7e3f316d64a70db58dd4))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("mnsndu7.100",0x0000,0x10000,CRC(400442e7) SHA1(d6c075dc439d5366b7ae71b5a523b86543b1ecd6))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("mnsndu17.100", 0x000000, CRC(f9bc55e8) SHA1(7dc41521305021961927ebde4dcf22611e3d622d))
	ROM_LOAD_SND_8M("mnsndu21.100", 0x080000, CRC(e0727e1f) SHA1(2093dba6e2f59cd1d1fc49c8d995b603ea0913ba))
	ROM_LOAD_SND_8M("mnsndu36.100", 0x100000, CRC(c845aa97) SHA1(2632aa8c5576b7afcb96693fa524c7d0350ac9a8))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

/*-------------------------------------------------------------------
/ Monopoly (3.01)
/-------------------------------------------------------------------*/
ROM_START(monop301)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("moncpu.301", 0x00000, 0x20000, CRC(24978872) SHA1(48ef94fd720cdafc61f8de5efd5c6b6731237b18))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("mondsp-a.301", 0x00000, 0x80000, CRC(c4e2e032) SHA1(691f7b6ed0616338683f7e3f316d64a70db58dd4))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("mnsndu7.100",0x0000,0x10000,CRC(400442e7) SHA1(d6c075dc439d5366b7ae71b5a523b86543b1ecd6))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("mnsndu17.100", 0x000000, CRC(f9bc55e8) SHA1(7dc41521305021961927ebde4dcf22611e3d622d))
	ROM_LOAD_SND_8M("mnsndu21.100", 0x080000, CRC(e0727e1f) SHA1(2093dba6e2f59cd1d1fc49c8d995b603ea0913ba))
	ROM_LOAD_SND_8M("mnsndu36.100", 0x100000, CRC(c845aa97) SHA1(2632aa8c5576b7afcb96693fa524c7d0350ac9a8))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

/*-------------------------------------------------------------------
/ Monopoly (2.51)
/-------------------------------------------------------------------*/
ROM_START(monop251)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("moncpu.251", 0x00000, 0x20000, CRC(0645cfae) SHA1(d979234150b7fb62718debbeeeca1466a6c0344f))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("mondsp-a.206", 0x00000, 0x80000, CRC(6df6e158) SHA1(d3a9be2dc189b44b9e4b9f77f5011ed931df5634))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("mnsndu7.100",0x0000,0x10000,CRC(400442e7) SHA1(d6c075dc439d5366b7ae71b5a523b86543b1ecd6))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("mnsndu17.100", 0x000000, CRC(f9bc55e8) SHA1(7dc41521305021961927ebde4dcf22611e3d622d))
	ROM_LOAD_SND_8M("mnsndu21.100", 0x080000, CRC(e0727e1f) SHA1(2093dba6e2f59cd1d1fc49c8d995b603ea0913ba))
	ROM_LOAD_SND_8M("mnsndu36.100", 0x100000, CRC(c845aa97) SHA1(2632aa8c5576b7afcb96693fa524c7d0350ac9a8))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

/*-------------------------------------------------------------------
/ Monopoly (2.33)
/-------------------------------------------------------------------*/
ROM_START(monop233)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("moncpu.233", 0x00000, 0x20000, CRC(f20a5ca6) SHA1(12ae56bd149aa6635c19f4dd73580db550b26963))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("mondsp-a.203", 0x00000, 0x80000, CRC(6e4678fb) SHA1(c0f41f01e9e20e741f1b13d3bd6e824486ba9a0a))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("mnsndu7.100",0x0000,0x10000,CRC(400442e7) SHA1(d6c075dc439d5366b7ae71b5a523b86543b1ecd6))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("mnsndu17.100", 0x000000, CRC(f9bc55e8) SHA1(7dc41521305021961927ebde4dcf22611e3d622d))
	ROM_LOAD_SND_8M("mnsndu21.100", 0x080000, CRC(e0727e1f) SHA1(2093dba6e2f59cd1d1fc49c8d995b603ea0913ba))
	ROM_LOAD_SND_8M("mnsndu36.100", 0x100000, CRC(c845aa97) SHA1(2632aa8c5576b7afcb96693fa524c7d0350ac9a8))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

/*-------------------------------------------------------------------
/ Monopoly (3.20) (ARM7 Sound Board)
/-------------------------------------------------------------------*/
ROM_START(mononew)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("moncpu.320", 0x00000, 0x20000, CRC(6c107c8b) SHA1(236d85b971c70a30e663787d643e6d589591d582))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("mondsp-a.301", 0x00000, 0x80000, CRC(c4e2e032) SHA1(691f7b6ed0616338683f7e3f316d64a70db58dd4))

	ROM_REGION32_LE(0x600000, "soundcpu", ROMREGION_ERASEMASK | 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x100000)

	ROM_REGION(0x1000000, "bsmt", 0 )
	ROM_LOAD("mnsndu7.100",0x0000,0x10000,CRC(400442e7) SHA1(d6c075dc439d5366b7ae71b5a523b86543b1ecd6))

	ROM_REGION(0x400000, "sound2", 0)
	ROM_LOAD("mnsndu17.100", 0x000000, 0x100000, CRC(f9bc55e8) SHA1(7dc41521305021961927ebde4dcf22611e3d622d))
	ROM_LOAD("mnsndu21.100", 0x100000, 0x100000, CRC(e0727e1f) SHA1(2093dba6e2f59cd1d1fc49c8d995b603ea0913ba))
	ROM_LOAD("mnsndu36.100", 0x200000, 0x100000, CRC(c845aa97) SHA1(2632aa8c5576b7afcb96693fa524c7d0350ac9a8))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

/*-------------------------------------------------------------------
/ Monopoly (France)
/-------------------------------------------------------------------*/
ROM_START(monopolf)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("moncpu.320", 0x00000, 0x20000, CRC(6c107c8b) SHA1(236d85b971c70a30e663787d643e6d589591d582))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("mondsp-f.301", 0x00000, 0x80000, CRC(e78b1998) SHA1(bd022dc90b55374baed17360fad7bf0f89e2ee33))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("mnsndu7.100",0x0000,0x10000,CRC(400442e7) SHA1(d6c075dc439d5366b7ae71b5a523b86543b1ecd6))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("mnsndu17.100", 0x000000, CRC(f9bc55e8) SHA1(7dc41521305021961927ebde4dcf22611e3d622d))
	ROM_LOAD_SND_8M("mnsndu21.100", 0x080000, CRC(e0727e1f) SHA1(2093dba6e2f59cd1d1fc49c8d995b603ea0913ba))
	ROM_LOAD_SND_8M("mnsndu36.100", 0x100000, CRC(c845aa97) SHA1(2632aa8c5576b7afcb96693fa524c7d0350ac9a8))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

/*-------------------------------------------------------------------
/ Monopoly (France)
/-------------------------------------------------------------------*/
ROM_START(monopolg)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("moncpu.320", 0x00000, 0x20000, CRC(6c107c8b) SHA1(236d85b971c70a30e663787d643e6d589591d582))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("mondsp-g.301", 0x00000, 0x80000, CRC(aab48728) SHA1(b9ed8574ac463a5fc21dc5f41d090cf0ad3f8362))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("mnsndu7.100",0x0000,0x10000,CRC(400442e7) SHA1(d6c075dc439d5366b7ae71b5a523b86543b1ecd6))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("mnsndu17.100", 0x000000, CRC(f9bc55e8) SHA1(7dc41521305021961927ebde4dcf22611e3d622d))
	ROM_LOAD_SND_8M("mnsndu21.100", 0x080000, CRC(e0727e1f) SHA1(2093dba6e2f59cd1d1fc49c8d995b603ea0913ba))
	ROM_LOAD_SND_8M("mnsndu36.100", 0x100000, CRC(c845aa97) SHA1(2632aa8c5576b7afcb96693fa524c7d0350ac9a8))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

/*-------------------------------------------------------------------
/ Monopoly (Italy)
/-------------------------------------------------------------------*/
ROM_START(monopoli)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("moncpu.320", 0x00000, 0x20000, CRC(6c107c8b) SHA1(236d85b971c70a30e663787d643e6d589591d582))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("mondsp-i.301", 0x00000, 0x80000, CRC(32431b3c) SHA1(6266e17e705bd50d2358d9f7c0168de51aa13750))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("mnsndu7.100",0x0000,0x10000,CRC(400442e7) SHA1(d6c075dc439d5366b7ae71b5a523b86543b1ecd6))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("mnsndu17.100", 0x000000, CRC(f9bc55e8) SHA1(7dc41521305021961927ebde4dcf22611e3d622d))
	ROM_LOAD_SND_8M("mnsndu21.100", 0x080000, CRC(e0727e1f) SHA1(2093dba6e2f59cd1d1fc49c8d995b603ea0913ba))
	ROM_LOAD_SND_8M("mnsndu36.100", 0x100000, CRC(c845aa97) SHA1(2632aa8c5576b7afcb96693fa524c7d0350ac9a8))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

/*-------------------------------------------------------------------
/ Monopoly (Spain)
/-------------------------------------------------------------------*/
ROM_START(monopoll)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("moncpu.320", 0x00000, 0x20000, CRC(6c107c8b) SHA1(236d85b971c70a30e663787d643e6d589591d582))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("mondsp-s.301", 0x00000, 0x80000, CRC(9f70dad6) SHA1(bf4b1c579b4bdead51e6b34de81fe65c45b6596a))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("mnsndu7.100",0x0000,0x10000,CRC(400442e7) SHA1(d6c075dc439d5366b7ae71b5a523b86543b1ecd6))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("mnsndu17.100", 0x000000, CRC(f9bc55e8) SHA1(7dc41521305021961927ebde4dcf22611e3d622d))
	ROM_LOAD_SND_8M("mnsndu21.100", 0x080000, CRC(e0727e1f) SHA1(2093dba6e2f59cd1d1fc49c8d995b603ea0913ba))
	ROM_LOAD_SND_8M("mnsndu36.100", 0x100000, CRC(c845aa97) SHA1(2632aa8c5576b7afcb96693fa524c7d0350ac9a8))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

/*-------------------------------------------------------------------
/ NFL
/-------------------------------------------------------------------*/
ROM_START(nfl)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("nfl_v101.cpu", 0x00000, 0x20000, CRC(eeb81a51) SHA1(c87e5f50cc95b1d0206abc3c132f5f3314a9253c))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("nfl_v102.dmd", 0x00000, 0x80000, CRC(fd7bc50a) SHA1(5c92af91e7e12024026a06002e6c6bf68230fcc0))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("nfl_v100.u7",0x0000,0x10000,CRC(3fc766f8) SHA1(27341594e7d4a23146e6e6ec8ebdea125231cf91))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("nfl_v100.u17", 0x000000, CRC(f36b72ed) SHA1(f8fcbdb31295d363d1e7ad98dc318ab52bcfc52b))
	ROM_LOAD_SND_8M("nfl_v100.u21", 0x080000, CRC(f5a6c053) SHA1(30a9cda6c9d9c43f0f6690138cf74c39c79ba43e))
	ROM_LOAD_SND_8M("nfl_v100.u36", 0x100000, CRC(26dae8ac) SHA1(ec18f13578c5c291b777344b2830cde2ecf3581c))
	ROM_LOAD_SND_8M("nfl_v100.u37", 0x180000, CRC(375d5a99) SHA1(4b49c58968da645bd0ad60ed16744974b863164e))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END


/*-------------------------------------------------------------------
/ Playboy
/-------------------------------------------------------------------*/
ROM_START(playboys)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("pbcpu.500", 0x00000, 0x20000, CRC(e4d924ae) SHA1(2aab64eee57e9569c3cc1bde28cda69ff4cacc69))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("pbdispa.500", 0x00000, 0x80000, CRC(40450165) SHA1(80295466838cb08fe1499b19a98bf759fb7a306e))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("pbsndu7.102",0x0000,0x10000,CRC(12a68f34) SHA1(f2cd42918dec353883bc465f6302c2d94dcd6b87))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("pbsndu17.100", 0x000000, CRC(f5502fec) SHA1(c8edd56e0c4365dd6b4bef0f1c7cc83ea5fd73d6))
	ROM_LOAD_SND_8M("pbsndu21.100", 0x100000, CRC(7869d34f) SHA1(48a051045523c14ca06a7227b34ed9e3818828d0))
	ROM_LOAD_SND_8M("pbsndu36.100", 0x200000, CRC(d10f14a3) SHA1(972b480c23d484b627ecdce0322c08fe760a127f))
	ROM_LOAD_SND_8M("pbsndu37.100", 0x300000, CRC(6642524a) SHA1(9d0c0be5887cf4510c11243ee47b11c08cbae17c))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(playboys_401)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("pbcpu.401", 0x00000, 0x20000, CRC(cb2e2824) SHA1(01413ff1f4fbee9d62776babb60ccc88f97feedf))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("pbdispa.400", 0x00000, 0x80000, CRC(244e9740) SHA1(1a2de2c53104e1123cbcc9ccc92e0888b4cf4bec))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("pbsndu7.102",0x0000,0x10000,CRC(12a68f34) SHA1(f2cd42918dec353883bc465f6302c2d94dcd6b87))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("pbsndu17.100", 0x000000, CRC(f5502fec) SHA1(c8edd56e0c4365dd6b4bef0f1c7cc83ea5fd73d6))
	ROM_LOAD_SND_8M("pbsndu21.100", 0x100000, CRC(7869d34f) SHA1(48a051045523c14ca06a7227b34ed9e3818828d0))
	ROM_LOAD_SND_8M("pbsndu36.100", 0x200000, CRC(d10f14a3) SHA1(972b480c23d484b627ecdce0322c08fe760a127f))
	ROM_LOAD_SND_8M("pbsndu37.100", 0x300000, CRC(6642524a) SHA1(9d0c0be5887cf4510c11243ee47b11c08cbae17c))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(playboys_303)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("pbcpu.303", 0x00000, 0x20000, CRC(6a6f6aab) SHA1(cac5d68d699db4016244ffd56355e2834be3da1f))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("pbdispa.300", 0x00000, 0x80000, CRC(2dbb372a) SHA1(b694ae06f380ea9f9730ea6bbfab7f9f7ec7342b))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("pbsndu7.102",0x0000,0x10000,CRC(12a68f34) SHA1(f2cd42918dec353883bc465f6302c2d94dcd6b87))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("pbsndu17.100", 0x000000, CRC(f5502fec) SHA1(c8edd56e0c4365dd6b4bef0f1c7cc83ea5fd73d6))
	ROM_LOAD_SND_8M("pbsndu21.100", 0x100000, CRC(7869d34f) SHA1(48a051045523c14ca06a7227b34ed9e3818828d0))
	ROM_LOAD_SND_8M("pbsndu36.100", 0x200000, CRC(d10f14a3) SHA1(972b480c23d484b627ecdce0322c08fe760a127f))
	ROM_LOAD_SND_8M("pbsndu37.100", 0x300000, CRC(6642524a) SHA1(9d0c0be5887cf4510c11243ee47b11c08cbae17c))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(playboys_300)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("pbcpu.300", 0x00000, 0x20000, CRC(d7e5bada) SHA1(e4d5bc015751a559eb95acb6da04246b7418eaf5))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("pbdispa.300", 0x00000, 0x80000, CRC(2dbb372a) SHA1(b694ae06f380ea9f9730ea6bbfab7f9f7ec7342b))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("pbsndu7.102",0x0000,0x10000,CRC(12a68f34) SHA1(f2cd42918dec353883bc465f6302c2d94dcd6b87))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("pbsndu17.100", 0x000000, CRC(f5502fec) SHA1(c8edd56e0c4365dd6b4bef0f1c7cc83ea5fd73d6))
	ROM_LOAD_SND_8M("pbsndu21.100", 0x100000, CRC(7869d34f) SHA1(48a051045523c14ca06a7227b34ed9e3818828d0))
	ROM_LOAD_SND_8M("pbsndu36.100", 0x200000, CRC(d10f14a3) SHA1(972b480c23d484b627ecdce0322c08fe760a127f))
	ROM_LOAD_SND_8M("pbsndu37.100", 0x300000, CRC(6642524a) SHA1(9d0c0be5887cf4510c11243ee47b11c08cbae17c))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(playboys_203)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("pbcpu.203", 0x00000, 0x20000, CRC(50eb01b0) SHA1(1618874f35432bd9fb2592e1a56592e7624257c4))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("pbdisp-a.201", 0x00000, 0x80000, CRC(78ec6af8) SHA1(35b8de8ab345cf81eec4f7b7d4f654115fe69ddf))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("pbsndu7.102",0x0000,0x10000,CRC(12a68f34) SHA1(f2cd42918dec353883bc465f6302c2d94dcd6b87))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("pbsndu17.100", 0x000000, CRC(f5502fec) SHA1(c8edd56e0c4365dd6b4bef0f1c7cc83ea5fd73d6))
	ROM_LOAD_SND_8M("pbsndu21.100", 0x100000, CRC(7869d34f) SHA1(48a051045523c14ca06a7227b34ed9e3818828d0))
	ROM_LOAD_SND_8M("pbsndu36.100", 0x200000, CRC(d10f14a3) SHA1(972b480c23d484b627ecdce0322c08fe760a127f))
	ROM_LOAD_SND_8M("pbsndu37.100", 0x300000, CRC(6642524a) SHA1(9d0c0be5887cf4510c11243ee47b11c08cbae17c))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(playnew)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("pbcpu.500", 0x00000, 0x20000, CRC(e4d924ae) SHA1(2aab64eee57e9569c3cc1bde28cda69ff4cacc69))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("pbdispa.500", 0x00000, 0x80000, CRC(40450165) SHA1(80295466838cb08fe1499b19a98bf759fb7a306e))

	ROM_REGION32_LE(0x600000, "soundcpu", ROMREGION_ERASEMASK | 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x1000000, "bsmt", 0 )
	ROM_LOAD("pbsndu7.102",0x0000,0x10000,CRC(12a68f34) SHA1(f2cd42918dec353883bc465f6302c2d94dcd6b87))

	ROM_REGION(0x400000, "sound2", 0)
	ROM_LOAD("pbsndu17.100", 0x000000, 0x100000, CRC(f5502fec) SHA1(c8edd56e0c4365dd6b4bef0f1c7cc83ea5fd73d6))
	ROM_LOAD("pbsndu21.100", 0x100000, 0x100000, CRC(7869d34f) SHA1(48a051045523c14ca06a7227b34ed9e3818828d0))
	ROM_LOAD("pbsndu36.100", 0x200000, 0x100000, CRC(d10f14a3) SHA1(972b480c23d484b627ecdce0322c08fe760a127f))
	ROM_LOAD("pbsndu37.100", 0x300000, 0x100000, CRC(6642524a) SHA1(9d0c0be5887cf4510c11243ee47b11c08cbae17c))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(playboyf)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("pbcpu.500", 0x00000, 0x20000, CRC(e4d924ae) SHA1(2aab64eee57e9569c3cc1bde28cda69ff4cacc69))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("pbdispf.500", 0x00000, 0x80000, CRC(aedc6c32) SHA1(c930ae1b1308ae641553de34f8249b17f408be56))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("pbsndu7.102",0x0000,0x10000,CRC(12a68f34) SHA1(f2cd42918dec353883bc465f6302c2d94dcd6b87))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("pbsndu17.100", 0x000000, CRC(f5502fec) SHA1(c8edd56e0c4365dd6b4bef0f1c7cc83ea5fd73d6))
	ROM_LOAD_SND_8M("pbsndu21.100", 0x100000, CRC(7869d34f) SHA1(48a051045523c14ca06a7227b34ed9e3818828d0))
	ROM_LOAD_SND_8M("pbsndu36.100", 0x200000, CRC(d10f14a3) SHA1(972b480c23d484b627ecdce0322c08fe760a127f))
	ROM_LOAD_SND_8M("pbsndu37.100", 0x300000, CRC(6642524a) SHA1(9d0c0be5887cf4510c11243ee47b11c08cbae17c))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(playboyf_401)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("pbcpu.401", 0x00000, 0x20000, CRC(cb2e2824) SHA1(01413ff1f4fbee9d62776babb60ccc88f97feedf))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("pbdispf.400", 0x00000, 0x80000, CRC(8ccce5d9) SHA1(28370445d38b065001e96b455d283bc047ed0f21))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("pbsndu7.102",0x0000,0x10000,CRC(12a68f34) SHA1(f2cd42918dec353883bc465f6302c2d94dcd6b87))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("pbsndu17.100", 0x000000, CRC(f5502fec) SHA1(c8edd56e0c4365dd6b4bef0f1c7cc83ea5fd73d6))
	ROM_LOAD_SND_8M("pbsndu21.100", 0x100000, CRC(7869d34f) SHA1(48a051045523c14ca06a7227b34ed9e3818828d0))
	ROM_LOAD_SND_8M("pbsndu36.100", 0x200000, CRC(d10f14a3) SHA1(972b480c23d484b627ecdce0322c08fe760a127f))
	ROM_LOAD_SND_8M("pbsndu37.100", 0x300000, CRC(6642524a) SHA1(9d0c0be5887cf4510c11243ee47b11c08cbae17c))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(playboyf_303)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("pbcpu.303", 0x00000, 0x20000, CRC(6a6f6aab) SHA1(cac5d68d699db4016244ffd56355e2834be3da1f))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("pbdispf.300", 0x00000, 0x80000, CRC(69ab3bb2) SHA1(59d7ad5eca701d1216200cd489d2d07825a0856e))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("pbsndu7.102",0x0000,0x10000,CRC(12a68f34) SHA1(f2cd42918dec353883bc465f6302c2d94dcd6b87))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("pbsndu17.100", 0x000000, CRC(f5502fec) SHA1(c8edd56e0c4365dd6b4bef0f1c7cc83ea5fd73d6))
	ROM_LOAD_SND_8M("pbsndu21.100", 0x100000, CRC(7869d34f) SHA1(48a051045523c14ca06a7227b34ed9e3818828d0))
	ROM_LOAD_SND_8M("pbsndu36.100", 0x200000, CRC(d10f14a3) SHA1(972b480c23d484b627ecdce0322c08fe760a127f))
	ROM_LOAD_SND_8M("pbsndu37.100", 0x300000, CRC(6642524a) SHA1(9d0c0be5887cf4510c11243ee47b11c08cbae17c))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(playboyf_300)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("pbcpu.300", 0x00000, 0x20000, CRC(d7e5bada) SHA1(e4d5bc015751a559eb95acb6da04246b7418eaf5))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("pbdispf.300", 0x00000, 0x80000, CRC(69ab3bb2) SHA1(59d7ad5eca701d1216200cd489d2d07825a0856e))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("pbsndu7.102",0x0000,0x10000,CRC(12a68f34) SHA1(f2cd42918dec353883bc465f6302c2d94dcd6b87))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("pbsndu17.100", 0x000000, CRC(f5502fec) SHA1(c8edd56e0c4365dd6b4bef0f1c7cc83ea5fd73d6))
	ROM_LOAD_SND_8M("pbsndu21.100", 0x100000, CRC(7869d34f) SHA1(48a051045523c14ca06a7227b34ed9e3818828d0))
	ROM_LOAD_SND_8M("pbsndu36.100", 0x200000, CRC(d10f14a3) SHA1(972b480c23d484b627ecdce0322c08fe760a127f))
	ROM_LOAD_SND_8M("pbsndu37.100", 0x300000, CRC(6642524a) SHA1(9d0c0be5887cf4510c11243ee47b11c08cbae17c))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(playboyf_203)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("pbcpu.203", 0x00000, 0x20000, CRC(50eb01b0) SHA1(1618874f35432bd9fb2592e1a56592e7624257c4))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("pbdisp-f.201", 0x00000, 0x80000, CRC(eedea4f4) SHA1(31eb1d4de0a4aee73c424c0f21dd2042e6ad0dca))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("pbsndu7.102",0x0000,0x10000,CRC(12a68f34) SHA1(f2cd42918dec353883bc465f6302c2d94dcd6b87))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("pbsndu17.100", 0x000000, CRC(f5502fec) SHA1(c8edd56e0c4365dd6b4bef0f1c7cc83ea5fd73d6))
	ROM_LOAD_SND_8M("pbsndu21.100", 0x100000, CRC(7869d34f) SHA1(48a051045523c14ca06a7227b34ed9e3818828d0))
	ROM_LOAD_SND_8M("pbsndu36.100", 0x200000, CRC(d10f14a3) SHA1(972b480c23d484b627ecdce0322c08fe760a127f))
	ROM_LOAD_SND_8M("pbsndu37.100", 0x300000, CRC(6642524a) SHA1(9d0c0be5887cf4510c11243ee47b11c08cbae17c))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(playboyg)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("pbcpu.500", 0x00000, 0x20000, CRC(e4d924ae) SHA1(2aab64eee57e9569c3cc1bde28cda69ff4cacc69))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("pbdispg.500", 0x00000, 0x80000, CRC(681392fe) SHA1(23011d538282da144b8ff9cbb7c5655567017e5e))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("pbsndu7.102",0x0000,0x10000,CRC(12a68f34) SHA1(f2cd42918dec353883bc465f6302c2d94dcd6b87))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("pbsndu17.100", 0x000000, CRC(f5502fec) SHA1(c8edd56e0c4365dd6b4bef0f1c7cc83ea5fd73d6))
	ROM_LOAD_SND_8M("pbsndu21.100", 0x100000, CRC(7869d34f) SHA1(48a051045523c14ca06a7227b34ed9e3818828d0))
	ROM_LOAD_SND_8M("pbsndu36.100", 0x200000, CRC(d10f14a3) SHA1(972b480c23d484b627ecdce0322c08fe760a127f))
	ROM_LOAD_SND_8M("pbsndu37.100", 0x300000, CRC(6642524a) SHA1(9d0c0be5887cf4510c11243ee47b11c08cbae17c))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(playboyg_401)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("pbcpu.401", 0x00000, 0x20000, CRC(cb2e2824) SHA1(01413ff1f4fbee9d62776babb60ccc88f97feedf))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("pbdispg.400", 0x00000, 0x80000, CRC(c26a0c73) SHA1(1841ab278e5d3e146cb3b43dfedc208d302dbc17))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("pbsndu7.102",0x0000,0x10000,CRC(12a68f34) SHA1(f2cd42918dec353883bc465f6302c2d94dcd6b87))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("pbsndu17.100", 0x000000, CRC(f5502fec) SHA1(c8edd56e0c4365dd6b4bef0f1c7cc83ea5fd73d6))
	ROM_LOAD_SND_8M("pbsndu21.100", 0x100000, CRC(7869d34f) SHA1(48a051045523c14ca06a7227b34ed9e3818828d0))
	ROM_LOAD_SND_8M("pbsndu36.100", 0x200000, CRC(d10f14a3) SHA1(972b480c23d484b627ecdce0322c08fe760a127f))
	ROM_LOAD_SND_8M("pbsndu37.100", 0x300000, CRC(6642524a) SHA1(9d0c0be5887cf4510c11243ee47b11c08cbae17c))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(playboyg_303)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("pbcpu.303", 0x00000, 0x20000, CRC(6a6f6aab) SHA1(cac5d68d699db4016244ffd56355e2834be3da1f))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("pbdispg.300", 0x00000, 0x80000, CRC(ed7b7c62) SHA1(28b0ab490f8abd5f29e8cb0996da9e7200918157))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("pbsndu7.102",0x0000,0x10000,CRC(12a68f34) SHA1(f2cd42918dec353883bc465f6302c2d94dcd6b87))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("pbsndu17.100", 0x000000, CRC(f5502fec) SHA1(c8edd56e0c4365dd6b4bef0f1c7cc83ea5fd73d6))
	ROM_LOAD_SND_8M("pbsndu21.100", 0x100000, CRC(7869d34f) SHA1(48a051045523c14ca06a7227b34ed9e3818828d0))
	ROM_LOAD_SND_8M("pbsndu36.100", 0x200000, CRC(d10f14a3) SHA1(972b480c23d484b627ecdce0322c08fe760a127f))
	ROM_LOAD_SND_8M("pbsndu37.100", 0x300000, CRC(6642524a) SHA1(9d0c0be5887cf4510c11243ee47b11c08cbae17c))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(playboyg_300)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("pbcpu.300", 0x00000, 0x20000, CRC(d7e5bada) SHA1(e4d5bc015751a559eb95acb6da04246b7418eaf5))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("pbdispg.300", 0x00000, 0x80000, CRC(ed7b7c62) SHA1(28b0ab490f8abd5f29e8cb0996da9e7200918157))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("pbsndu7.102",0x0000,0x10000,CRC(12a68f34) SHA1(f2cd42918dec353883bc465f6302c2d94dcd6b87))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("pbsndu17.100", 0x000000, CRC(f5502fec) SHA1(c8edd56e0c4365dd6b4bef0f1c7cc83ea5fd73d6))
	ROM_LOAD_SND_8M("pbsndu21.100", 0x100000, CRC(7869d34f) SHA1(48a051045523c14ca06a7227b34ed9e3818828d0))
	ROM_LOAD_SND_8M("pbsndu36.100", 0x200000, CRC(d10f14a3) SHA1(972b480c23d484b627ecdce0322c08fe760a127f))
	ROM_LOAD_SND_8M("pbsndu37.100", 0x300000, CRC(6642524a) SHA1(9d0c0be5887cf4510c11243ee47b11c08cbae17c))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(playboyg_203)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("pbcpu.203", 0x00000, 0x20000, CRC(50eb01b0) SHA1(1618874f35432bd9fb2592e1a56592e7624257c4))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("pbdisp-g.201", 0x00000, 0x80000, CRC(ff525cc7) SHA1(475578cf8b2262a11f640883b70b706f705d90ff))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("pbsndu7.102",0x0000,0x10000,CRC(12a68f34) SHA1(f2cd42918dec353883bc465f6302c2d94dcd6b87))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("pbsndu17.100", 0x000000, CRC(f5502fec) SHA1(c8edd56e0c4365dd6b4bef0f1c7cc83ea5fd73d6))
	ROM_LOAD_SND_8M("pbsndu21.100", 0x100000, CRC(7869d34f) SHA1(48a051045523c14ca06a7227b34ed9e3818828d0))
	ROM_LOAD_SND_8M("pbsndu36.100", 0x200000, CRC(d10f14a3) SHA1(972b480c23d484b627ecdce0322c08fe760a127f))
	ROM_LOAD_SND_8M("pbsndu37.100", 0x300000, CRC(6642524a) SHA1(9d0c0be5887cf4510c11243ee47b11c08cbae17c))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(playboyi)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("pbcpu.500", 0x00000, 0x20000, CRC(e4d924ae) SHA1(2aab64eee57e9569c3cc1bde28cda69ff4cacc69))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("pbdispi.500", 0x00000, 0x80000, CRC(d90d7ec6) SHA1(7442160f403a8ccabfc9dc8dc53f8b44f5df26bc))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("pbsndu7.102",0x0000,0x10000,CRC(12a68f34) SHA1(f2cd42918dec353883bc465f6302c2d94dcd6b87))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("pbsndu17.100", 0x000000, CRC(f5502fec) SHA1(c8edd56e0c4365dd6b4bef0f1c7cc83ea5fd73d6))
	ROM_LOAD_SND_8M("pbsndu21.100", 0x100000, CRC(7869d34f) SHA1(48a051045523c14ca06a7227b34ed9e3818828d0))
	ROM_LOAD_SND_8M("pbsndu36.100", 0x200000, CRC(d10f14a3) SHA1(972b480c23d484b627ecdce0322c08fe760a127f))
	ROM_LOAD_SND_8M("pbsndu37.100", 0x300000, CRC(6642524a) SHA1(9d0c0be5887cf4510c11243ee47b11c08cbae17c))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(playboyi_401)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("pbcpu.401", 0x00000, 0x20000, CRC(cb2e2824) SHA1(01413ff1f4fbee9d62776babb60ccc88f97feedf))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)
	ROM_REGION(0x80000, "decodmd", 0)

	ROM_LOAD("pbdispi.400", 0x00000, 0x80000, CRC(90c5afed) SHA1(ea7ab06adb18854c9871b02f39fe616a27293415))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)

	ROM_LOAD("pbsndu7.102",0x0000,0x10000,CRC(12a68f34) SHA1(f2cd42918dec353883bc465f6302c2d94dcd6b87))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("pbsndu17.100", 0x000000, CRC(f5502fec) SHA1(c8edd56e0c4365dd6b4bef0f1c7cc83ea5fd73d6))
	ROM_LOAD_SND_8M("pbsndu21.100", 0x100000, CRC(7869d34f) SHA1(48a051045523c14ca06a7227b34ed9e3818828d0))
	ROM_LOAD_SND_8M("pbsndu36.100", 0x200000, CRC(d10f14a3) SHA1(972b480c23d484b627ecdce0322c08fe760a127f))
	ROM_LOAD_SND_8M("pbsndu37.100", 0x300000, CRC(6642524a) SHA1(9d0c0be5887cf4510c11243ee47b11c08cbae17c))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(playboyi_303)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("pbcpu.303", 0x00000, 0x20000, CRC(6a6f6aab) SHA1(cac5d68d699db4016244ffd56355e2834be3da1f))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("pbdispi.300", 0x00000, 0x80000, CRC(74c8cedf) SHA1(474ad1939ea0a58852003e549ed85478e239a67c))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("pbsndu7.102",0x0000,0x10000,CRC(12a68f34) SHA1(f2cd42918dec353883bc465f6302c2d94dcd6b87))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("pbsndu17.100", 0x000000, CRC(f5502fec) SHA1(c8edd56e0c4365dd6b4bef0f1c7cc83ea5fd73d6))
	ROM_LOAD_SND_8M("pbsndu21.100", 0x100000, CRC(7869d34f) SHA1(48a051045523c14ca06a7227b34ed9e3818828d0))
	ROM_LOAD_SND_8M("pbsndu36.100", 0x200000, CRC(d10f14a3) SHA1(972b480c23d484b627ecdce0322c08fe760a127f))
	ROM_LOAD_SND_8M("pbsndu37.100", 0x300000, CRC(6642524a) SHA1(9d0c0be5887cf4510c11243ee47b11c08cbae17c))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(playboyi_300)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("pbcpu.300", 0x00000, 0x20000, CRC(d7e5bada) SHA1(e4d5bc015751a559eb95acb6da04246b7418eaf5))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("pbdispi.300", 0x00000, 0x80000, CRC(74c8cedf) SHA1(474ad1939ea0a58852003e549ed85478e239a67c))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("pbsndu7.102",0x0000,0x10000,CRC(12a68f34) SHA1(f2cd42918dec353883bc465f6302c2d94dcd6b87))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("pbsndu17.100", 0x000000, CRC(f5502fec) SHA1(c8edd56e0c4365dd6b4bef0f1c7cc83ea5fd73d6))
	ROM_LOAD_SND_8M("pbsndu21.100", 0x100000, CRC(7869d34f) SHA1(48a051045523c14ca06a7227b34ed9e3818828d0))
	ROM_LOAD_SND_8M("pbsndu36.100", 0x200000, CRC(d10f14a3) SHA1(972b480c23d484b627ecdce0322c08fe760a127f))
	ROM_LOAD_SND_8M("pbsndu37.100", 0x300000, CRC(6642524a) SHA1(9d0c0be5887cf4510c11243ee47b11c08cbae17c))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(playboyi_203)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("pbcpu.203", 0x00000, 0x20000, CRC(50eb01b0) SHA1(1618874f35432bd9fb2592e1a56592e7624257c4))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("pbdisp-i.201", 0x00000, 0x80000, CRC(48f190dc) SHA1(bf5c096f755871b12783a63b55908751f9fa5b07))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("pbsndu7.102",0x0000,0x10000,CRC(12a68f34) SHA1(f2cd42918dec353883bc465f6302c2d94dcd6b87))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("pbsndu17.100", 0x000000, CRC(f5502fec) SHA1(c8edd56e0c4365dd6b4bef0f1c7cc83ea5fd73d6))
	ROM_LOAD_SND_8M("pbsndu21.100", 0x100000, CRC(7869d34f) SHA1(48a051045523c14ca06a7227b34ed9e3818828d0))
	ROM_LOAD_SND_8M("pbsndu36.100", 0x200000, CRC(d10f14a3) SHA1(972b480c23d484b627ecdce0322c08fe760a127f))
	ROM_LOAD_SND_8M("pbsndu37.100", 0x300000, CRC(6642524a) SHA1(9d0c0be5887cf4510c11243ee47b11c08cbae17c))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(playboyl)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("pbcpu.500", 0x00000, 0x20000, CRC(e4d924ae) SHA1(2aab64eee57e9569c3cc1bde28cda69ff4cacc69))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("pbdispl.500", 0x00000, 0x80000, CRC(b019f0f6) SHA1(184a9905cd3af9d272577e43666aed5e5a8a5281))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("pbsndu7.102",0x0000,0x10000,CRC(12a68f34) SHA1(f2cd42918dec353883bc465f6302c2d94dcd6b87))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("pbsndu17.100", 0x000000, CRC(f5502fec) SHA1(c8edd56e0c4365dd6b4bef0f1c7cc83ea5fd73d6))
	ROM_LOAD_SND_8M("pbsndu21.100", 0x100000, CRC(7869d34f) SHA1(48a051045523c14ca06a7227b34ed9e3818828d0))
	ROM_LOAD_SND_8M("pbsndu36.100", 0x200000, CRC(d10f14a3) SHA1(972b480c23d484b627ecdce0322c08fe760a127f))
	ROM_LOAD_SND_8M("pbsndu37.100", 0x300000, CRC(6642524a) SHA1(9d0c0be5887cf4510c11243ee47b11c08cbae17c))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(playboyl_401)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("pbcpu.401", 0x00000, 0x20000, CRC(cb2e2824) SHA1(01413ff1f4fbee9d62776babb60ccc88f97feedf))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("pbdisps.400", 0x00000, 0x80000, CRC(92b48fdd) SHA1(deebb0bf6ec13b0cead3970c19bd0e8321046786))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("pbsndu7.102",0x0000,0x10000,CRC(12a68f34) SHA1(f2cd42918dec353883bc465f6302c2d94dcd6b87))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("pbsndu17.100", 0x000000, CRC(f5502fec) SHA1(c8edd56e0c4365dd6b4bef0f1c7cc83ea5fd73d6))
	ROM_LOAD_SND_8M("pbsndu21.100", 0x100000, CRC(7869d34f) SHA1(48a051045523c14ca06a7227b34ed9e3818828d0))
	ROM_LOAD_SND_8M("pbsndu36.100", 0x200000, CRC(d10f14a3) SHA1(972b480c23d484b627ecdce0322c08fe760a127f))
	ROM_LOAD_SND_8M("pbsndu37.100", 0x300000, CRC(6642524a) SHA1(9d0c0be5887cf4510c11243ee47b11c08cbae17c))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(playboyl_303)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("pbcpu.303", 0x00000, 0x20000, CRC(6a6f6aab) SHA1(cac5d68d699db4016244ffd56355e2834be3da1f))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("pbdispl.300", 0x00000, 0x80000, CRC(e7697fc3) SHA1(7a9796e7b99af0d3d2079876a8054209a3067e64))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("pbsndu7.102",0x0000,0x10000,CRC(12a68f34) SHA1(f2cd42918dec353883bc465f6302c2d94dcd6b87))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("pbsndu17.100", 0x000000, CRC(f5502fec) SHA1(c8edd56e0c4365dd6b4bef0f1c7cc83ea5fd73d6))
	ROM_LOAD_SND_8M("pbsndu21.100", 0x100000, CRC(7869d34f) SHA1(48a051045523c14ca06a7227b34ed9e3818828d0))
	ROM_LOAD_SND_8M("pbsndu36.100", 0x200000, CRC(d10f14a3) SHA1(972b480c23d484b627ecdce0322c08fe760a127f))
	ROM_LOAD_SND_8M("pbsndu37.100", 0x300000, CRC(6642524a) SHA1(9d0c0be5887cf4510c11243ee47b11c08cbae17c))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(playboyl_300)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("pbcpu.300", 0x00000, 0x20000, CRC(d7e5bada) SHA1(e4d5bc015751a559eb95acb6da04246b7418eaf5))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("pbdispl.300", 0x00000, 0x80000, CRC(e7697fc3) SHA1(7a9796e7b99af0d3d2079876a8054209a3067e64))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("pbsndu7.102",0x0000,0x10000,CRC(12a68f34) SHA1(f2cd42918dec353883bc465f6302c2d94dcd6b87))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("pbsndu17.100", 0x000000, CRC(f5502fec) SHA1(c8edd56e0c4365dd6b4bef0f1c7cc83ea5fd73d6))
	ROM_LOAD_SND_8M("pbsndu21.100", 0x100000, CRC(7869d34f) SHA1(48a051045523c14ca06a7227b34ed9e3818828d0))
	ROM_LOAD_SND_8M("pbsndu36.100", 0x200000, CRC(d10f14a3) SHA1(972b480c23d484b627ecdce0322c08fe760a127f))
	ROM_LOAD_SND_8M("pbsndu37.100", 0x300000, CRC(6642524a) SHA1(9d0c0be5887cf4510c11243ee47b11c08cbae17c))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(playboyl_203)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("pbcpu.203", 0x00000, 0x20000, CRC(50eb01b0) SHA1(1618874f35432bd9fb2592e1a56592e7624257c4))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("pbdisp-l.201", 0x00000, 0x80000, CRC(eaa65c45) SHA1(4ec9f815e40ac2185812faad7b991723f0b5775b))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("pbsndu7.102",0x0000,0x10000,CRC(12a68f34) SHA1(f2cd42918dec353883bc465f6302c2d94dcd6b87))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("pbsndu17.100", 0x000000, CRC(f5502fec) SHA1(c8edd56e0c4365dd6b4bef0f1c7cc83ea5fd73d6))
	ROM_LOAD_SND_8M("pbsndu21.100", 0x100000, CRC(7869d34f) SHA1(48a051045523c14ca06a7227b34ed9e3818828d0))
	ROM_LOAD_SND_8M("pbsndu36.100", 0x200000, CRC(d10f14a3) SHA1(972b480c23d484b627ecdce0322c08fe760a127f))
	ROM_LOAD_SND_8M("pbsndu37.100", 0x300000, CRC(6642524a) SHA1(9d0c0be5887cf4510c11243ee47b11c08cbae17c))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(playboys_302)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("pbcpu.302", 0x00000, 0x20000, CRC(206285ed) SHA1(65ec90b20f7be6fac62170f69b744f9e4eb6254c))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("pbdispa.300", 0x00000, 0x80000, CRC(2dbb372a) SHA1(b694ae06f380ea9f9730ea6bbfab7f9f7ec7342b))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("pbsndu7.102",0x0000,0x10000,CRC(12a68f34) SHA1(f2cd42918dec353883bc465f6302c2d94dcd6b87))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("pbsndu17.100", 0x000000, CRC(f5502fec) SHA1(c8edd56e0c4365dd6b4bef0f1c7cc83ea5fd73d6))
	ROM_LOAD_SND_8M("pbsndu21.100", 0x100000, CRC(7869d34f) SHA1(48a051045523c14ca06a7227b34ed9e3818828d0))
	ROM_LOAD_SND_8M("pbsndu36.100", 0x200000, CRC(d10f14a3) SHA1(972b480c23d484b627ecdce0322c08fe760a127f))
	ROM_LOAD_SND_8M("pbsndu37.100", 0x300000, CRC(6642524a) SHA1(9d0c0be5887cf4510c11243ee47b11c08cbae17c))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(playboyl_302)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("pbcpu.302", 0x00000, 0x20000, CRC(206285ed) SHA1(65ec90b20f7be6fac62170f69b744f9e4eb6254c))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("pbdispl.300", 0x00000, 0x80000, CRC(e7697fc3) SHA1(7a9796e7b99af0d3d2079876a8054209a3067e64))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("pbsndu7.102",0x0000,0x10000,CRC(12a68f34) SHA1(f2cd42918dec353883bc465f6302c2d94dcd6b87))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("pbsndu17.100", 0x000000, CRC(f5502fec) SHA1(c8edd56e0c4365dd6b4bef0f1c7cc83ea5fd73d6))
	ROM_LOAD_SND_8M("pbsndu21.100", 0x100000, CRC(7869d34f) SHA1(48a051045523c14ca06a7227b34ed9e3818828d0))
	ROM_LOAD_SND_8M("pbsndu36.100", 0x200000, CRC(d10f14a3) SHA1(972b480c23d484b627ecdce0322c08fe760a127f))
	ROM_LOAD_SND_8M("pbsndu37.100", 0x300000, CRC(6642524a) SHA1(9d0c0be5887cf4510c11243ee47b11c08cbae17c))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(playboyg_302)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("pbcpu.302", 0x00000, 0x20000, CRC(206285ed) SHA1(65ec90b20f7be6fac62170f69b744f9e4eb6254c))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("pbdispg.300", 0x00000, 0x80000, CRC(ed7b7c62) SHA1(28b0ab490f8abd5f29e8cb0996da9e7200918157))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("pbsndu7.102",0x0000,0x10000,CRC(12a68f34) SHA1(f2cd42918dec353883bc465f6302c2d94dcd6b87))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("pbsndu17.100", 0x000000, CRC(f5502fec) SHA1(c8edd56e0c4365dd6b4bef0f1c7cc83ea5fd73d6))
	ROM_LOAD_SND_8M("pbsndu21.100", 0x100000, CRC(7869d34f) SHA1(48a051045523c14ca06a7227b34ed9e3818828d0))
	ROM_LOAD_SND_8M("pbsndu36.100", 0x200000, CRC(d10f14a3) SHA1(972b480c23d484b627ecdce0322c08fe760a127f))
	ROM_LOAD_SND_8M("pbsndu37.100", 0x300000, CRC(6642524a) SHA1(9d0c0be5887cf4510c11243ee47b11c08cbae17c))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(playboyf_302)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("pbcpu.302", 0x00000, 0x20000, CRC(206285ed) SHA1(65ec90b20f7be6fac62170f69b744f9e4eb6254c))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("pbdispf.300", 0x00000, 0x80000, CRC(69ab3bb2) SHA1(59d7ad5eca701d1216200cd489d2d07825a0856e))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("pbsndu7.102",0x0000,0x10000,CRC(12a68f34) SHA1(f2cd42918dec353883bc465f6302c2d94dcd6b87))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("pbsndu17.100", 0x000000, CRC(f5502fec) SHA1(c8edd56e0c4365dd6b4bef0f1c7cc83ea5fd73d6))
	ROM_LOAD_SND_8M("pbsndu21.100", 0x100000, CRC(7869d34f) SHA1(48a051045523c14ca06a7227b34ed9e3818828d0))
	ROM_LOAD_SND_8M("pbsndu36.100", 0x200000, CRC(d10f14a3) SHA1(972b480c23d484b627ecdce0322c08fe760a127f))
	ROM_LOAD_SND_8M("pbsndu37.100", 0x300000, CRC(6642524a) SHA1(9d0c0be5887cf4510c11243ee47b11c08cbae17c))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(playboyi_302)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("pbcpu.302", 0x00000, 0x20000, CRC(206285ed) SHA1(65ec90b20f7be6fac62170f69b744f9e4eb6254c))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("pbdispi.300", 0x00000, 0x80000, CRC(74c8cedf) SHA1(474ad1939ea0a58852003e549ed85478e239a67c))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("pbsndu7.102",0x0000,0x10000,CRC(12a68f34) SHA1(f2cd42918dec353883bc465f6302c2d94dcd6b87))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("pbsndu17.100", 0x000000, CRC(f5502fec) SHA1(c8edd56e0c4365dd6b4bef0f1c7cc83ea5fd73d6))
	ROM_LOAD_SND_8M("pbsndu21.100", 0x100000, CRC(7869d34f) SHA1(48a051045523c14ca06a7227b34ed9e3818828d0))
	ROM_LOAD_SND_8M("pbsndu36.100", 0x200000, CRC(d10f14a3) SHA1(972b480c23d484b627ecdce0322c08fe760a127f))
	ROM_LOAD_SND_8M("pbsndu37.100", 0x300000, CRC(6642524a) SHA1(9d0c0be5887cf4510c11243ee47b11c08cbae17c))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

/*-------------------------------------------------------------------
/ Roller Coaster Tycoon
/-------------------------------------------------------------------*/
ROM_START(rctycn)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("rctcpu.702", 0x00000, 0x20000, CRC(5736a816) SHA1(fcfd06eeca74df0bca2c0bc57aeaa00400e4ab55))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("rctdispa.701", 0x00000, 0x80000, CRC(0d527f13) SHA1(954116a79578b2a7679c401a2bb99b5bbfb603f2))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("rcsndu7.100",0x0000,0x10000,CRC(e6cde9b1) SHA1(cbaadafd18ad9c0338bf2cce94b2c2a89e734778))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("rcsndu17.100", 0x000000, CRC(18ba20ec) SHA1(c6e7a8a5fd6029b8e884a6eeb779a10d819e8c7c))
	ROM_LOAD_SND_8M("rcsndu21.100", 0x080000, CRC(64b19c11) SHA1(9ac714d372437cf1d8c4e01512c0647f13e40ddb))
	ROM_LOAD_SND_8M("rcsndu36.100", 0x100000, CRC(05c8bac9) SHA1(0771a393d5361c9a35d42a18b6c6a105b7752e03))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(rctycn_701)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("rctcpu.701",    0x0000, 0x20000, CRC(e1fe89f6) SHA1(9a76a5c267e055fcf0418394762bcea758da02d6))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("rctdispa.700",  0x0000, 0x80000, CRC(6a8925d7) SHA1(82a6c069f1e8f8e053fec708f56c8ffe56d70fc8))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("rcsndu7.100",0x0000,0x10000,CRC(e6cde9b1) SHA1(cbaadafd18ad9c0338bf2cce94b2c2a89e734778))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("rcsndu17.100", 0x000000, CRC(18ba20ec) SHA1(c6e7a8a5fd6029b8e884a6eeb779a10d819e8c7c))
	ROM_LOAD_SND_8M("rcsndu21.100", 0x080000, CRC(64b19c11) SHA1(9ac714d372437cf1d8c4e01512c0647f13e40ddb))
	ROM_LOAD_SND_8M("rcsndu36.100", 0x100000, CRC(05c8bac9) SHA1(0771a393d5361c9a35d42a18b6c6a105b7752e03))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(rctycn_600)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("rctcpu.600",    0x0000, 0x20000, CRC(2ada30e5) SHA1(fdcd608af155b16c15ec14c83927004c63e9c513))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("rctdispa.600",  0x0000, 0x80000, CRC(dbd294e1) SHA1(53c540594bfe5544b4a009665a1ca3edb1cb874c))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("rcsndu7.100",0x0000,0x10000,CRC(e6cde9b1) SHA1(cbaadafd18ad9c0338bf2cce94b2c2a89e734778))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("rcsndu17.100", 0x000000, CRC(18ba20ec) SHA1(c6e7a8a5fd6029b8e884a6eeb779a10d819e8c7c))
	ROM_LOAD_SND_8M("rcsndu21.100", 0x080000, CRC(64b19c11) SHA1(9ac714d372437cf1d8c4e01512c0647f13e40ddb))
	ROM_LOAD_SND_8M("rcsndu36.100", 0x100000, CRC(05c8bac9) SHA1(0771a393d5361c9a35d42a18b6c6a105b7752e03))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(rctycn_400)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("rctcpu.400", 0x00000, 0x20000, CRC(4691de23) SHA1(f47fc03f9b030cb7a1201d015bbec5a426176338))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("rctdispa.400", 0x00000, 0x80000, CRC(d128a8fa) SHA1(8fece3df33d21da020ba06d14d588384d90dd89f))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("rcsndu7.100",0x0000,0x10000,CRC(e6cde9b1) SHA1(cbaadafd18ad9c0338bf2cce94b2c2a89e734778))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("rcsndu17.100", 0x000000, CRC(18ba20ec) SHA1(c6e7a8a5fd6029b8e884a6eeb779a10d819e8c7c))
	ROM_LOAD_SND_8M("rcsndu21.100", 0x080000, CRC(64b19c11) SHA1(9ac714d372437cf1d8c4e01512c0647f13e40ddb))
	ROM_LOAD_SND_8M("rcsndu36.100", 0x100000, CRC(05c8bac9) SHA1(0771a393d5361c9a35d42a18b6c6a105b7752e03))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(rctnew)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("rctcpu.702", 0x00000, 0x20000, CRC(5736a816) SHA1(fcfd06eeca74df0bca2c0bc57aeaa00400e4ab55))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("rctdispa.701", 0x00000, 0x80000, CRC(0d527f13) SHA1(954116a79578b2a7679c401a2bb99b5bbfb603f2))

	ROM_REGION32_LE(0x600000, "soundcpu", ROMREGION_ERASEMASK | 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x100000)

	ROM_REGION(0x1000000, "bsmt", 0 )
	ROM_LOAD("rcsndu7.100",0x0000,0x10000,CRC(e6cde9b1) SHA1(cbaadafd18ad9c0338bf2cce94b2c2a89e734778))

	ROM_REGION(0x400000, "sound2", 0)
	ROM_LOAD_SND_8M("rcsndu17.100", 0x000000, CRC(18ba20ec) SHA1(c6e7a8a5fd6029b8e884a6eeb779a10d819e8c7c))
	ROM_LOAD_SND_8M("rcsndu21.100", 0x080000, CRC(64b19c11) SHA1(9ac714d372437cf1d8c4e01512c0647f13e40ddb))
	ROM_LOAD_SND_8M("rcsndu36.100", 0x100000, CRC(05c8bac9) SHA1(0771a393d5361c9a35d42a18b6c6a105b7752e03))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(rctycng)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("rctcpu.702", 0x00000, 0x20000, CRC(5736a816) SHA1(fcfd06eeca74df0bca2c0bc57aeaa00400e4ab55))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("rctdispg.701", 0x00000, 0x80000, CRC(6c70ab29) SHA1(fa3b713b79c0d724b918fa318795681308a4fce3))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("rcsndu7.100",0x0000,0x10000,CRC(e6cde9b1) SHA1(cbaadafd18ad9c0338bf2cce94b2c2a89e734778))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("rcsndu17.100", 0x000000, CRC(18ba20ec) SHA1(c6e7a8a5fd6029b8e884a6eeb779a10d819e8c7c))
	ROM_LOAD_SND_8M("rcsndu21.100", 0x080000, CRC(64b19c11) SHA1(9ac714d372437cf1d8c4e01512c0647f13e40ddb))
	ROM_LOAD_SND_8M("rcsndu36.100", 0x100000, CRC(05c8bac9) SHA1(0771a393d5361c9a35d42a18b6c6a105b7752e03))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(rctycng_701)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("rctcpu.701", 0x00000, 0x20000, CRC(e1fe89f6) SHA1(9a76a5c267e055fcf0418394762bcea758da02d6))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("rctdispg.700", 0x00000, 0x80000, CRC(0babf1ed) SHA1(db683aa392968d255d355d4a1b0c9d8d4fb9e799))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("rcsndu7.100",0x0000,0x10000,CRC(e6cde9b1) SHA1(cbaadafd18ad9c0338bf2cce94b2c2a89e734778))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("rcsndu17.100", 0x000000, CRC(18ba20ec) SHA1(c6e7a8a5fd6029b8e884a6eeb779a10d819e8c7c))
	ROM_LOAD_SND_8M("rcsndu21.100", 0x080000, CRC(64b19c11) SHA1(9ac714d372437cf1d8c4e01512c0647f13e40ddb))
	ROM_LOAD_SND_8M("rcsndu36.100", 0x100000, CRC(05c8bac9) SHA1(0771a393d5361c9a35d42a18b6c6a105b7752e03))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(rctycng_400)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("rctcpu.400", 0x00000, 0x20000, CRC(4691de23) SHA1(f47fc03f9b030cb7a1201d015bbec5a426176338))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("rctdispg.400", 0x00000, 0x80000, CRC(c88dc915) SHA1(894c3bb7f5d200448740f021b5b9421f6ee74c5f))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("rcsndu7.100",0x0000,0x10000,CRC(e6cde9b1) SHA1(cbaadafd18ad9c0338bf2cce94b2c2a89e734778))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("rcsndu17.100", 0x000000, CRC(18ba20ec) SHA1(c6e7a8a5fd6029b8e884a6eeb779a10d819e8c7c))
	ROM_LOAD_SND_8M("rcsndu21.100", 0x080000, CRC(64b19c11) SHA1(9ac714d372437cf1d8c4e01512c0647f13e40ddb))
	ROM_LOAD_SND_8M("rcsndu36.100", 0x100000, CRC(05c8bac9) SHA1(0771a393d5361c9a35d42a18b6c6a105b7752e03))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(rctycnf)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("rctcpu.702", 0x00000, 0x20000, CRC(5736a816) SHA1(fcfd06eeca74df0bca2c0bc57aeaa00400e4ab55))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("rctdispf.701", 0x00000, 0x80000, CRC(4a3bb40c) SHA1(053f494765ac7708401a7816af7186e71083fe8d))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("rcsndu7.100",0x0000,0x10000,CRC(e6cde9b1) SHA1(cbaadafd18ad9c0338bf2cce94b2c2a89e734778))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("rcsndu17.100", 0x000000, CRC(18ba20ec) SHA1(c6e7a8a5fd6029b8e884a6eeb779a10d819e8c7c))
	ROM_LOAD_SND_8M("rcsndu21.100", 0x080000, CRC(64b19c11) SHA1(9ac714d372437cf1d8c4e01512c0647f13e40ddb))
	ROM_LOAD_SND_8M("rcsndu36.100", 0x100000, CRC(05c8bac9) SHA1(0771a393d5361c9a35d42a18b6c6a105b7752e03))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(rctycnf_701)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("rctcpu.701", 0x00000, 0x20000, CRC(e1fe89f6) SHA1(9a76a5c267e055fcf0418394762bcea758da02d6))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("rctdispf.700", 0x00000, 0x80000, CRC(2de0eec8) SHA1(5b48eabddc1fb735866767ae008baf58205954db))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("rcsndu7.100",0x0000,0x10000,CRC(e6cde9b1) SHA1(cbaadafd18ad9c0338bf2cce94b2c2a89e734778))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("rcsndu17.100", 0x000000, CRC(18ba20ec) SHA1(c6e7a8a5fd6029b8e884a6eeb779a10d819e8c7c))
	ROM_LOAD_SND_8M("rcsndu21.100", 0x080000, CRC(64b19c11) SHA1(9ac714d372437cf1d8c4e01512c0647f13e40ddb))
	ROM_LOAD_SND_8M("rcsndu36.100", 0x100000, CRC(05c8bac9) SHA1(0771a393d5361c9a35d42a18b6c6a105b7752e03))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(rctycnf_600)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("rctcpu.600", 0x00000, 0x20000, CRC(2ada30e5) SHA1(fdcd608af155b16c15ec14c83927004c63e9c513))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("rctdispf.600", 0x00000, 0x80000, CRC(50aa2f48) SHA1(e7c0fb30ef15861e342bd6fada45885bd2403547))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("rcsndu7.100",0x0000,0x10000,CRC(e6cde9b1) SHA1(cbaadafd18ad9c0338bf2cce94b2c2a89e734778))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("rcsndu17.100", 0x000000, CRC(18ba20ec) SHA1(c6e7a8a5fd6029b8e884a6eeb779a10d819e8c7c))
	ROM_LOAD_SND_8M("rcsndu21.100", 0x080000, CRC(64b19c11) SHA1(9ac714d372437cf1d8c4e01512c0647f13e40ddb))
	ROM_LOAD_SND_8M("rcsndu36.100", 0x100000, CRC(05c8bac9) SHA1(0771a393d5361c9a35d42a18b6c6a105b7752e03))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(rctycnf_400)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("rctcpu.400", 0x00000, 0x20000, CRC(4691de23) SHA1(f47fc03f9b030cb7a1201d015bbec5a426176338))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("rctdispf.400", 0x00000, 0x80000, CRC(5b41893b) SHA1(66930d0f5d6542b41f22995df55447ba0e435576))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("rcsndu7.100",0x0000,0x10000,CRC(e6cde9b1) SHA1(cbaadafd18ad9c0338bf2cce94b2c2a89e734778))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("rcsndu17.100", 0x000000, CRC(18ba20ec) SHA1(c6e7a8a5fd6029b8e884a6eeb779a10d819e8c7c))
	ROM_LOAD_SND_8M("rcsndu21.100", 0x080000, CRC(64b19c11) SHA1(9ac714d372437cf1d8c4e01512c0647f13e40ddb))
	ROM_LOAD_SND_8M("rcsndu36.100", 0x100000, CRC(05c8bac9) SHA1(0771a393d5361c9a35d42a18b6c6a105b7752e03))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(rctycni)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("rctcpu.702", 0x00000, 0x20000, CRC(5736a816) SHA1(fcfd06eeca74df0bca2c0bc57aeaa00400e4ab55))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("rctdispi.701", 0x00000, 0x80000, CRC(6adc8236) SHA1(bb908c88e47987777c921f2948dfb802ee29c868))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("rcsndu7.100",0x0000,0x10000,CRC(e6cde9b1) SHA1(cbaadafd18ad9c0338bf2cce94b2c2a89e734778))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("rcsndu17.100", 0x000000, CRC(18ba20ec) SHA1(c6e7a8a5fd6029b8e884a6eeb779a10d819e8c7c))
	ROM_LOAD_SND_8M("rcsndu21.100", 0x080000, CRC(64b19c11) SHA1(9ac714d372437cf1d8c4e01512c0647f13e40ddb))
	ROM_LOAD_SND_8M("rcsndu36.100", 0x100000, CRC(05c8bac9) SHA1(0771a393d5361c9a35d42a18b6c6a105b7752e03))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(rctycni_701)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("rctcpu.701", 0x00000, 0x20000, CRC(e1fe89f6) SHA1(9a76a5c267e055fcf0418394762bcea758da02d6))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("rctdispi.700", 0x00000, 0x80000, CRC(0d07d8f2) SHA1(3ffd8ad7183ba20844c1e5d1933c8002ca4707aa))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("rcsndu7.100",0x0000,0x10000,CRC(e6cde9b1) SHA1(cbaadafd18ad9c0338bf2cce94b2c2a89e734778))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("rcsndu17.100", 0x000000, CRC(18ba20ec) SHA1(c6e7a8a5fd6029b8e884a6eeb779a10d819e8c7c))
	ROM_LOAD_SND_8M("rcsndu21.100", 0x080000, CRC(64b19c11) SHA1(9ac714d372437cf1d8c4e01512c0647f13e40ddb))
	ROM_LOAD_SND_8M("rcsndu36.100", 0x100000, CRC(05c8bac9) SHA1(0771a393d5361c9a35d42a18b6c6a105b7752e03))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(rctycni_600)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("rctcpu.600", 0x00000, 0x20000, CRC(2ada30e5) SHA1(fdcd608af155b16c15ec14c83927004c63e9c513))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("rctdispi.600", 0x00000, 0x80000, CRC(a55a86c1) SHA1(b18197063e2e4cc50b04f43de41d870eb672ce68))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("rcsndu7.100",0x0000,0x10000,CRC(e6cde9b1) SHA1(cbaadafd18ad9c0338bf2cce94b2c2a89e734778))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("rcsndu17.100", 0x000000, CRC(18ba20ec) SHA1(c6e7a8a5fd6029b8e884a6eeb779a10d819e8c7c))
	ROM_LOAD_SND_8M("rcsndu21.100", 0x080000, CRC(64b19c11) SHA1(9ac714d372437cf1d8c4e01512c0647f13e40ddb))
	ROM_LOAD_SND_8M("rcsndu36.100", 0x100000, CRC(05c8bac9) SHA1(0771a393d5361c9a35d42a18b6c6a105b7752e03))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(rctycni_400)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("rctcpu.400", 0x00000, 0x20000, CRC(4691de23) SHA1(f47fc03f9b030cb7a1201d015bbec5a426176338))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)
	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("rctdispi.400", 0x00000, 0x80000, CRC(7c1c0adb) SHA1(c9321a64ee4af8d475b441ae2385120bba66ccf9))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("rcsndu7.100",0x0000,0x10000,CRC(e6cde9b1) SHA1(cbaadafd18ad9c0338bf2cce94b2c2a89e734778))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("rcsndu17.100", 0x000000, CRC(18ba20ec) SHA1(c6e7a8a5fd6029b8e884a6eeb779a10d819e8c7c))

	ROM_LOAD_SND_8M("rcsndu21.100", 0x080000, CRC(64b19c11) SHA1(9ac714d372437cf1d8c4e01512c0647f13e40ddb))
	ROM_LOAD_SND_8M("rcsndu36.100", 0x100000, CRC(05c8bac9) SHA1(0771a393d5361c9a35d42a18b6c6a105b7752e03))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(rctycnl)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("rctcpu.702", 0x00000, 0x20000, CRC(5736a816) SHA1(fcfd06eeca74df0bca2c0bc57aeaa00400e4ab55))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("rctdisps.701", 0x00000, 0x80000, CRC(0efa8208) SHA1(6706ea3e31b478970fc65a8cf9db749db9c0fa39))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("rcsndu7.100",0x0000,0x10000,CRC(e6cde9b1) SHA1(cbaadafd18ad9c0338bf2cce94b2c2a89e734778))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("rcsndu17.100", 0x000000, CRC(18ba20ec) SHA1(c6e7a8a5fd6029b8e884a6eeb779a10d819e8c7c))
	ROM_LOAD_SND_8M("rcsndu21.100", 0x080000, CRC(64b19c11) SHA1(9ac714d372437cf1d8c4e01512c0647f13e40ddb))
	ROM_LOAD_SND_8M("rcsndu36.100", 0x100000, CRC(05c8bac9) SHA1(0771a393d5361c9a35d42a18b6c6a105b7752e03))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(rctycnl_701)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("rctcpu.701", 0x00000, 0x20000, CRC(e1fe89f6) SHA1(9a76a5c267e055fcf0418394762bcea758da02d6))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("rctdisps.700", 0x00000, 0x80000, CRC(6921d8cc) SHA1(1ada415af8e949829ceac75da507982ea2091f4d))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("rcsndu7.100",0x0000,0x10000,CRC(e6cde9b1) SHA1(cbaadafd18ad9c0338bf2cce94b2c2a89e734778))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("rcsndu17.100", 0x000000, CRC(18ba20ec) SHA1(c6e7a8a5fd6029b8e884a6eeb779a10d819e8c7c))
	ROM_LOAD_SND_8M("rcsndu21.100", 0x080000, CRC(64b19c11) SHA1(9ac714d372437cf1d8c4e01512c0647f13e40ddb))
	ROM_LOAD_SND_8M("rcsndu36.100", 0x100000, CRC(05c8bac9) SHA1(0771a393d5361c9a35d42a18b6c6a105b7752e03))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(rctycnl_600)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("rctcpu.600", 0x00000, 0x20000, CRC(2ada30e5) SHA1(fdcd608af155b16c15ec14c83927004c63e9c513))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("rctdisps.600", 0x00000, 0x80000, CRC(84a970a3) SHA1(8e00830ba446c8cef99d8e721b65597d9dd98379))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("rcsndu7.100",0x0000,0x10000,CRC(e6cde9b1) SHA1(cbaadafd18ad9c0338bf2cce94b2c2a89e734778))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("rcsndu17.100", 0x000000, CRC(18ba20ec) SHA1(c6e7a8a5fd6029b8e884a6eeb779a10d819e8c7c))
	ROM_LOAD_SND_8M("rcsndu21.100", 0x080000, CRC(64b19c11) SHA1(9ac714d372437cf1d8c4e01512c0647f13e40ddb))
	ROM_LOAD_SND_8M("rcsndu36.100", 0x100000, CRC(05c8bac9) SHA1(0771a393d5361c9a35d42a18b6c6a105b7752e03))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(rctycnl_400)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("rctcpu.400", 0x00000, 0x20000, CRC(4691de23) SHA1(f47fc03f9b030cb7a1201d015bbec5a426176338))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("rctdisps.400", 0x00000, 0x80000, CRC(c06e6669) SHA1(af389ba74e62d7065b3bc7d5f771a3461704cbe3))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("rcsndu7.100",0x0000,0x10000,CRC(e6cde9b1) SHA1(cbaadafd18ad9c0338bf2cce94b2c2a89e734778))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("rcsndu17.100", 0x000000, CRC(18ba20ec) SHA1(c6e7a8a5fd6029b8e884a6eeb779a10d819e8c7c))
	ROM_LOAD_SND_8M("rcsndu21.100", 0x080000, CRC(64b19c11) SHA1(9ac714d372437cf1d8c4e01512c0647f13e40ddb))
	ROM_LOAD_SND_8M("rcsndu36.100", 0x100000, CRC(05c8bac9) SHA1(0771a393d5361c9a35d42a18b6c6a105b7752e03))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

/*-------------------------------------------------------------------
/ Sharkey's Shootout
/-------------------------------------------------------------------*/
ROM_START(shrkysht)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("sscpu.211", 0x00000, 0x20000, CRC(66a0e5ce) SHA1(d33615081cd8cdf8a17a44b389c6d9746e093510))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("ssdispa.201", 0x00000, 0x80000, CRC(3360300b) SHA1(3169651a49bb7168fc04b2ae678b696ec6a21c85))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("sssndu7.101",0x0000,0x10000,CRC(fbc6267b) SHA1(e6e70662031e5209385f8b9c59296d7423cc03b4))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("sssndu17.100", 0x000000, CRC(dae78d8d) SHA1(a0e1722a19505e7d08266c490d27f772357722d3))
	ROM_LOAD_SND_8M("sssndu21.100", 0x080000, CRC(e1fa3f2a) SHA1(08731fd53ef81453a8f20602e76d435c6771bbb9))
	ROM_LOAD_SND_8M("sssndu36.100", 0x100000, CRC(d22fcfa3) SHA1(3fa407f72ecc64f9d00b92122c4e4d85022e4202))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(shrknew)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("sscpu.211", 0x00000, 0x20000, CRC(66a0e5ce) SHA1(d33615081cd8cdf8a17a44b389c6d9746e093510))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("ssdispa.201", 0x00000, 0x80000, CRC(3360300b) SHA1(3169651a49bb7168fc04b2ae678b696ec6a21c85))

	ROM_REGION32_LE(0x600000, "soundcpu", ROMREGION_ERASEMASK | 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x100000)

	ROM_REGION(0x1000000, "bsmt", 0 )
	ROM_LOAD("sssndu7.101",0x0000,0x10000,CRC(fbc6267b) SHA1(e6e70662031e5209385f8b9c59296d7423cc03b4))

	ROM_REGION(0x400000, "sound2", 0)
	ROM_LOAD("sssndu17.100", 0x000000, 0x100000, CRC(dae78d8d) SHA1(a0e1722a19505e7d08266c490d27f772357722d3))
	ROM_LOAD("sssndu21.100", 0x100000, 0x100000, CRC(e1fa3f2a) SHA1(08731fd53ef81453a8f20602e76d435c6771bbb9))
	ROM_LOAD("sssndu36.100", 0x200000, 0x100000, CRC(d22fcfa3) SHA1(3fa407f72ecc64f9d00b92122c4e4d85022e4202))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(shrkygr)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("sscpu.211", 0x00000, 0x20000, CRC(66a0e5ce) SHA1(d33615081cd8cdf8a17a44b389c6d9746e093510))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("ssdispg.201", 0x00000, 0x80000, CRC(1ad88b75) SHA1(f82ae59b59e545e2023879aad7e99862d9d339c5))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("sssndu7.101",0x0000,0x10000,CRC(fbc6267b) SHA1(e6e70662031e5209385f8b9c59296d7423cc03b4))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("sssndu17.100", 0x000000, CRC(dae78d8d) SHA1(a0e1722a19505e7d08266c490d27f772357722d3))
	ROM_LOAD_SND_8M("sssndu21.100", 0x080000, CRC(e1fa3f2a) SHA1(08731fd53ef81453a8f20602e76d435c6771bbb9))
	ROM_LOAD_SND_8M("sssndu36.100", 0x100000, CRC(d22fcfa3) SHA1(3fa407f72ecc64f9d00b92122c4e4d85022e4202))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(shrkyfr)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("sscpu.211", 0x00000, 0x20000, CRC(66a0e5ce) SHA1(d33615081cd8cdf8a17a44b389c6d9746e093510))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("ssdispf.201", 0x00000, 0x80000, CRC(89eaaebf) SHA1(0945a11a8f632cea3b9e9982cc4aed54f12ec55a))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("sssndu7.101",0x0000,0x10000,CRC(fbc6267b) SHA1(e6e70662031e5209385f8b9c59296d7423cc03b4))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("sssndu17.100", 0x000000, CRC(dae78d8d) SHA1(a0e1722a19505e7d08266c490d27f772357722d3))
	ROM_LOAD_SND_8M("sssndu21.100", 0x080000, CRC(e1fa3f2a) SHA1(08731fd53ef81453a8f20602e76d435c6771bbb9))
	ROM_LOAD_SND_8M("sssndu36.100", 0x100000, CRC(d22fcfa3) SHA1(3fa407f72ecc64f9d00b92122c4e4d85022e4202))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(shrkyit)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("sscpu.211", 0x00000, 0x20000, CRC(66a0e5ce) SHA1(d33615081cd8cdf8a17a44b389c6d9746e093510))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("ssdispi.201", 0x00000, 0x80000, CRC(fb70641d) SHA1(d152838eeacacc5dfe6fc3bc3f26b4d3e9e4c9cc))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("sssndu7.101",0x0000,0x10000,CRC(fbc6267b) SHA1(e6e70662031e5209385f8b9c59296d7423cc03b4))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("sssndu17.100", 0x000000, CRC(dae78d8d) SHA1(a0e1722a19505e7d08266c490d27f772357722d3))
	ROM_LOAD_SND_8M("sssndu21.100", 0x080000, CRC(e1fa3f2a) SHA1(08731fd53ef81453a8f20602e76d435c6771bbb9))
	ROM_LOAD_SND_8M("sssndu36.100", 0x100000, CRC(d22fcfa3) SHA1(3fa407f72ecc64f9d00b92122c4e4d85022e4202))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(shrky_207)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("sscpu.207", 0x00000, 0x20000, CRC(7355d65d) SHA1(d501cf1ff8b4be01970d47997f71f539df57b702))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("ssdispa.201", 0x00000, 0x80000, CRC(3360300b) SHA1(3169651a49bb7168fc04b2ae678b696ec6a21c85))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("sssndu7.101",0x0000,0x10000,CRC(fbc6267b) SHA1(e6e70662031e5209385f8b9c59296d7423cc03b4))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("sssndu17.100", 0x000000, CRC(dae78d8d) SHA1(a0e1722a19505e7d08266c490d27f772357722d3))
	ROM_LOAD_SND_8M("sssndu21.100", 0x080000, CRC(e1fa3f2a) SHA1(08731fd53ef81453a8f20602e76d435c6771bbb9))
	ROM_LOAD_SND_8M("sssndu36.100", 0x100000, CRC(d22fcfa3) SHA1(3fa407f72ecc64f9d00b92122c4e4d85022e4202))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(shrkygr_207)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("sscpu.207", 0x00000, 0x20000, CRC(7355d65d) SHA1(d501cf1ff8b4be01970d47997f71f539df57b702))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("ssdispg.201", 0x00000, 0x80000, CRC(1ad88b75) SHA1(f82ae59b59e545e2023879aad7e99862d9d339c5))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("sssndu7.101",0x0000,0x10000,CRC(fbc6267b) SHA1(e6e70662031e5209385f8b9c59296d7423cc03b4))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("sssndu17.100", 0x000000, CRC(dae78d8d) SHA1(a0e1722a19505e7d08266c490d27f772357722d3))
	ROM_LOAD_SND_8M("sssndu21.100", 0x080000, CRC(e1fa3f2a) SHA1(08731fd53ef81453a8f20602e76d435c6771bbb9))
	ROM_LOAD_SND_8M("sssndu36.100", 0x100000, CRC(d22fcfa3) SHA1(3fa407f72ecc64f9d00b92122c4e4d85022e4202))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(shrkyfr_207)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("sscpu.207", 0x00000, 0x20000, CRC(7355d65d) SHA1(d501cf1ff8b4be01970d47997f71f539df57b702))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)
	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("ssdispf.201", 0x00000, 0x80000, CRC(89eaaebf) SHA1(0945a11a8f632cea3b9e9982cc4aed54f12ec55a))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("sssndu7.101",0x0000,0x10000,CRC(fbc6267b) SHA1(e6e70662031e5209385f8b9c59296d7423cc03b4))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("sssndu17.100", 0x000000, CRC(dae78d8d) SHA1(a0e1722a19505e7d08266c490d27f772357722d3))
	ROM_LOAD_SND_8M("sssndu21.100", 0x080000, CRC(e1fa3f2a) SHA1(08731fd53ef81453a8f20602e76d435c6771bbb9))
	ROM_LOAD_SND_8M("sssndu36.100", 0x100000, CRC(d22fcfa3) SHA1(3fa407f72ecc64f9d00b92122c4e4d85022e4202))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(shrkyit_207)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("sscpu.207", 0x00000, 0x20000, CRC(7355d65d) SHA1(d501cf1ff8b4be01970d47997f71f539df57b702))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("ssdispi.201", 0x00000, 0x80000, CRC(fb70641d) SHA1(d152838eeacacc5dfe6fc3bc3f26b4d3e9e4c9cc))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("sssndu7.101",0x0000,0x10000,CRC(fbc6267b) SHA1(e6e70662031e5209385f8b9c59296d7423cc03b4))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("sssndu17.100", 0x000000, CRC(dae78d8d) SHA1(a0e1722a19505e7d08266c490d27f772357722d3))
	ROM_LOAD_SND_8M("sssndu21.100", 0x080000, CRC(e1fa3f2a) SHA1(08731fd53ef81453a8f20602e76d435c6771bbb9))
	ROM_LOAD_SND_8M("sssndu36.100", 0x100000, CRC(d22fcfa3) SHA1(3fa407f72ecc64f9d00b92122c4e4d85022e4202))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

/*-------------------------------------------------------------------
/ South Park
/-------------------------------------------------------------------*/
ROM_START(sprk_103)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("spkcpu.103", 0x00000, 0x20000, CRC(55ca8aa1) SHA1(ee3dda7d7e6ad32072cbf3acc8087a27b95cc68d))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("spdspa.101", 0x00000, 0x80000, CRC(48ca598d) SHA1(0827ac7bb5cf12b0e63860b73a808273d984509e))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("spku7.101",0x0000,0x10000,CRC(3d831d3e) SHA1(fd12e4639bf806c518a2733c32572b051517ff27))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("spku17.100", 0x000000, CRC(829262c9) SHA1(88adb13a6773f88658d6b8d6520a03ecd377e4e7))
	ROM_LOAD_SND_8M("spku21.100", 0x080000, CRC(3a55eef3) SHA1(2a643bd7a433a39d764294c1569182e6ff0af240))
	ROM_LOAD_SND_8M("spku36.100", 0x100000, CRC(bcf6d2cb) SHA1(8e8186f08ff1e39a7889469ec1fdfa9402a8695c))
	ROM_LOAD_SND_8M("spku37.100", 0x180000, CRC(7d8f6bcb) SHA1(579cfef19cf9b5c91151ae833bc6c21734589849))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(sprk_096)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("spkcpu.096", 0x00000, 0x20000, CRC(4d7e862a) SHA1(962c129504f678264ab847fac08bac91cbbccb33))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("spdspa.101", 0x00000, 0x80000, CRC(48ca598d) SHA1(0827ac7bb5cf12b0e63860b73a808273d984509e))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("spku7.090",0x0000,0x10000,CRC(19937fbd) SHA1(ebd7c8f1604accbeb7c00066ecf811193a2cb588))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("spku17.090", 0x000000, CRC(05a8670e) SHA1(7c0f1f0c9b94f0327c820f002bffc4ea05670ec8))
	ROM_LOAD_SND_8M("spku21.090", 0x080000, CRC(c8629ee7) SHA1(843a742cb5cfce21a83618d14ae08ee1930d36cc))
	ROM_LOAD_SND_8M("spku36.090", 0x100000, CRC(727d4624) SHA1(9019014e6057d279a37cc3ce269a1c68baeb9673))
	ROM_LOAD_SND_8M("spku37.090", 0x180000, CRC(0c01b0c7) SHA1(76b5af50514d110b49721e6916dd16b3e3a2f5fa))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(sprk_090)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("spkcpu.090", 0x00000, 0x20000, CRC(bc3f8531) SHA1(5408e008c4f545bb4f82308b118d15525f8a263a))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("spdspa.090", 0x00000, 0x80000, CRC(c333dd48) SHA1(fe2be9274c06b2f39fa2e14e0d44ce7213282f3b))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("spku7.090",0x0000,0x10000,CRC(19937fbd) SHA1(ebd7c8f1604accbeb7c00066ecf811193a2cb588))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("spku17.090", 0x000000, CRC(05a8670e) SHA1(7c0f1f0c9b94f0327c820f002bffc4ea05670ec8))
	ROM_LOAD_SND_8M("spku21.090", 0x080000, CRC(c8629ee7) SHA1(843a742cb5cfce21a83618d14ae08ee1930d36cc))
	ROM_LOAD_SND_8M("spku36.090", 0x100000, CRC(727d4624) SHA1(9019014e6057d279a37cc3ce269a1c68baeb9673))
	ROM_LOAD_SND_8M("spku37.090", 0x180000, CRC(0c01b0c7) SHA1(76b5af50514d110b49721e6916dd16b3e3a2f5fa))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

/*-------------------------------------------------------------------
/ Space Jam
/-------------------------------------------------------------------*/
ROM_START(spacejam)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("jamcpu.300", 0x00000, 0x20000, CRC(9dc8df2e) SHA1(b3b111afb5b1f1236be73e899b34a5d5a73813e9))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("jamdspa.300", 0x00000, 0x80000, CRC(198e5e34) SHA1(e2ba9ff1cea84c5d41f32afc50229cb5a18c7666))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("spcjam.u7", 0x0000, 0x10000, CRC(c693d853) SHA1(3e81e60967dff496c681962f3ff8c7c1fbb7746a))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD("spcjam.u17", 0x000000, 0x80000, CRC(ccefe457) SHA1(4186dee689fbfc08e5070ccfe8d4be95220cd87b))
	ROM_LOAD("spcjam.u21", 0x080000, 0x80000, CRC(14cb71cb) SHA1(46752c1792c26345abb4d5219917a1cda50c600b))
	ROM_LOAD("spcjam.u36", 0x100000, 0x80000, CRC(7f61143c) SHA1(40695d1d14695d3e4991ed39f4a354c16227975e))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(spacejam_200)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("jamcpu.200", 0x00000, 0x20000, CRC(d80c069b) SHA1(bf6e96100b158f058b5f07f537ad0fa0a0fbe31d))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("jamdspa.200", 0x00000, 0x80000, CRC(4a05ec31) SHA1(eb962f5f2160508e0f81b252e8644d8aa833d7fd))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("spcjam.u7", 0x0000, 0x10000, CRC(c693d853) SHA1(3e81e60967dff496c681962f3ff8c7c1fbb7746a))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD("spcjam.u17", 0x000000, 0x80000, CRC(ccefe457) SHA1(4186dee689fbfc08e5070ccfe8d4be95220cd87b))
	ROM_LOAD("spcjam.u21", 0x080000, 0x80000, CRC(14cb71cb) SHA1(46752c1792c26345abb4d5219917a1cda50c600b))
	ROM_LOAD("spcjam.u36", 0x100000, 0x80000, CRC(7f61143c) SHA1(40695d1d14695d3e4991ed39f4a354c16227975e))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(spacejmg)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("jamcpu.300", 0x00000, 0x20000, CRC(9dc8df2e) SHA1(b3b111afb5b1f1236be73e899b34a5d5a73813e9))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("jamdspg.300", 0x00000, 0x80000, CRC(41f6e188) SHA1(da2247022aadb0ead5a3b1d7b829c13ff1153ec8))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("spcjam.u7", 0x0000, 0x10000, CRC(c693d853) SHA1(3e81e60967dff496c681962f3ff8c7c1fbb7746a))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD("spcjam.u17", 0x000000, 0x80000, CRC(ccefe457) SHA1(4186dee689fbfc08e5070ccfe8d4be95220cd87b))
	ROM_LOAD("spcjamg.u21", 0x080000, 0x80000, CRC(9e7fe0a6) SHA1(187e5893f84d0c0fd70d15c3978fc3fc51e12a51))
	ROM_LOAD("spcjamg.u36", 0x100000, 0x80000, CRC(7d11e1eb) SHA1(96d4635b1edf8a22947a5cd529ce9025cf7d0c71))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(spacejmf)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("jamcpu.300", 0x00000, 0x20000, CRC(9dc8df2e) SHA1(b3b111afb5b1f1236be73e899b34a5d5a73813e9))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("jamdspf.300", 0x00000, 0x80000, CRC(1683909f) SHA1(e14810a5d8704ea052fddcb3b54043bf9d57b296))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("spcjam.u7", 0x0000, 0x10000, CRC(c693d853) SHA1(3e81e60967dff496c681962f3ff8c7c1fbb7746a))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD("spcjam.u17", 0x000000, 0x80000, CRC(ccefe457) SHA1(4186dee689fbfc08e5070ccfe8d4be95220cd87b))
	ROM_LOAD("spcjamf.u21", 0x080000, 0x80000, CRC(9e7fe0a6) SHA1(187e5893f84d0c0fd70d15c3978fc3fc51e12a51))
	ROM_LOAD("spcjamf.u36", 0x100000, 0x80000, CRC(7d11e1eb) SHA1(96d4635b1edf8a22947a5cd529ce9025cf7d0c71))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(spacejmi)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("jamcpu.300", 0x00000, 0x20000, CRC(9dc8df2e) SHA1(b3b111afb5b1f1236be73e899b34a5d5a73813e9))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("jamdspi.300", 0x00000, 0x80000, CRC(eb9b5971) SHA1(0bfac0511d0cd9d56eee59782c199ee0a78abe5e))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("spcjam.u7", 0x0000, 0x10000, CRC(c693d853) SHA1(3e81e60967dff496c681962f3ff8c7c1fbb7746a))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD("spcjam.u17", 0x000000, 0x80000, CRC(ccefe457) SHA1(4186dee689fbfc08e5070ccfe8d4be95220cd87b))
	ROM_LOAD("spcjami.u21", 0x080000, 0x80000, CRC(9e7fe0a6) SHA1(187e5893f84d0c0fd70d15c3978fc3fc51e12a51))
	ROM_LOAD("spcjami.u36", 0x100000, 0x80000, CRC(7d11e1eb) SHA1(96d4635b1edf8a22947a5cd529ce9025cf7d0c71))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

/*-------------------------------------------------------------------
/ Star Wars Trilogy
/-------------------------------------------------------------------*/
ROM_START(swtril43)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("swcpu.403", 0x00000, 0x20000, CRC(d022880e) SHA1(c1a2acf5740cef6a8aeee9b6a60942b51147963f))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("swsedspa.400", 0x00000, 0x80000, CRC(b9bcbf71) SHA1(036f53e638699de0447ecd02221f673a40f656be))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("sw0219.u7", 0x0000, 0x10000, CRC(cd7c84d9) SHA1(55b0208089933e4a30f0eb87b123dd178383ed43))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD("sw0211.u17", 0x000000, 0x80000, CRC(6863e7f6) SHA1(99f1e0170fbbb91a0eb7a796ab3bf757cb1b23ce))
	ROM_LOAD("sw0211.u21", 0x080000, 0x80000, CRC(6be68450) SHA1(d24652f74b109e47eb5d3d02e04f63c99e92c590))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(swtril41)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("swcpu.401", 0x00000, 0x20000, CRC(707dce87) SHA1(45fc3ffe646e5be72af9f7f00990ee5f85338f34))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("swsedspa.400", 0x00000, 0x80000, CRC(b9bcbf71) SHA1(036f53e638699de0447ecd02221f673a40f656be))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("sw0219.u7", 0x0000, 0x10000, CRC(cd7c84d9) SHA1(55b0208089933e4a30f0eb87b123dd178383ed43))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD("sw0211.u17", 0x000000, 0x80000, CRC(6863e7f6) SHA1(99f1e0170fbbb91a0eb7a796ab3bf757cb1b23ce))
	ROM_LOAD("sw0211.u21", 0x080000, 0x80000, CRC(6be68450) SHA1(d24652f74b109e47eb5d3d02e04f63c99e92c590))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(swtril30)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("swcpu.300", 0x00000, 0x20000, CRC(b90d2682) SHA1(a29a0d7c2e0adbad721556e6d5aefa47105a233f))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)
	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("swsedspa.300", 0x00000, 0x80000, CRC(7b9817d1) SHA1(fbca577df6867f5feaab677714cf5e56702c8ba9))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("sw0219.u7", 0x0000, 0x10000, CRC(cd7c84d9) SHA1(55b0208089933e4a30f0eb87b123dd178383ed43))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD("sw0211.u17", 0x000000, 0x80000, CRC(6863e7f6) SHA1(99f1e0170fbbb91a0eb7a796ab3bf757cb1b23ce))
	ROM_LOAD("sw0211.u21", 0x080000, 0x80000, CRC(6be68450) SHA1(d24652f74b109e47eb5d3d02e04f63c99e92c590))
ROM_END

/*-------------------------------------------------------------------
/ Starship Troopers
/-------------------------------------------------------------------*/
ROM_START(startrp)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("sstcpu.201", 0x00000, 0x20000, CRC(549699fe) SHA1(7d3a12c7a41716ee38b822f924ca94c6932ebc4e))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("sstdspa.200", 0x00000, 0x80000, CRC(76a0e09e) SHA1(a4103aeee752d824a3811124079e40acc7286271))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("u7_b130.512", 0x0000, 0x10000, CRC(f1559e4f) SHA1(82b56f097412052bc1638a3f1c1319009df707f4))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD("u17_152a.040", 0x000000, 0x80000, CRC(8caeccdb) SHA1(390f07e48a176a24fe99a202f3fa2b9767d84230))
	ROM_LOAD("u21_0291.040", 0x080000, 0x80000, CRC(0c5321f6) SHA1(4a51daa16d489ab61d462d44f887c8422f863c5c))
	ROM_LOAD("u36_95a7.040", 0x100000, 0x80000, CRC(c1e4ca6a) SHA1(487de78ebf1ee8cc721f2ef7b1bd42d2f7b27456))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(startrp2)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("sstcpu.200", 0x00000, 0x20000, CRC(1bd865a5) SHA1(770d87d5108b58e214e551cfdcd4b75a11d6b88b))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("sstdspa.200", 0x00000, 0x80000, CRC(76a0e09e) SHA1(a4103aeee752d824a3811124079e40acc7286271))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("u7_b130.512", 0x0000, 0x10000, CRC(f1559e4f) SHA1(82b56f097412052bc1638a3f1c1319009df707f4))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD("u17_152a.040", 0x000000, 0x80000, CRC(8caeccdb) SHA1(390f07e48a176a24fe99a202f3fa2b9767d84230))
	ROM_LOAD("u21_0291.040", 0x080000, 0x80000, CRC(0c5321f6) SHA1(4a51daa16d489ab61d462d44f887c8422f863c5c))
	ROM_LOAD("u36_95a7.040", 0x100000, 0x80000, CRC(c1e4ca6a) SHA1(487de78ebf1ee8cc721f2ef7b1bd42d2f7b27456))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

/*-------------------------------------------------------------------
/ Striker Extreme
/-------------------------------------------------------------------*/
ROM_START(strikext)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("sxcpua.102", 0x00000, 0x20000, CRC(5e5f0fb8) SHA1(1425d66064e59193ce7cacb496c12ae956670435))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("sxdispa.103", 0x00000, 0x80000, CRC(e4cf849f) SHA1(27b61f1659762b396ca3af375e28f9c56500f79e))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("sxsnda.u7",0x0000,0x10000,CRC(e7e1a0cb) SHA1(be8b3e4d4232519db8344ae9e75f77d159bb1911))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("sxsnda.u17", 0x000000, CRC(aeeed88f) SHA1(e150fd243feffcdc5d66487e840cefdfb50213da))
	ROM_LOAD_SND_8M("sxsnda.u21", 0x080000, CRC(62c9bfe3) SHA1(14a65a673a33b7e3d3005f76acf3098dc37958f8))
	ROM_LOAD_SND_8M("sxsnda.u36", 0x100000, CRC(a0bc0edb) SHA1(1025a28fe9a0e3681e8e99b513da29ec294da045))
	ROM_LOAD_SND_8M("sxsnda.u37", 0x180000, CRC(4c08c33c) SHA1(36bfad0c59fd228db76a6ff36698edd929c11336))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(strikext_100)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("sxcpua.100", 0x00000, 0x20000, CRC(ee4742dd) SHA1(1b334f857ccb34f09eba69f3a40b589f6a712811))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("sxdispa.103", 0x00000, 0x80000, CRC(e4cf849f) SHA1(27b61f1659762b396ca3af375e28f9c56500f79e))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("sxsnda.u7",0x0000,0x10000,CRC(e7e1a0cb) SHA1(be8b3e4d4232519db8344ae9e75f77d159bb1911))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("sxsnda.u17", 0x000000, CRC(aeeed88f) SHA1(e150fd243feffcdc5d66487e840cefdfb50213da))
	ROM_LOAD_SND_8M("sxsnda.u21", 0x080000, CRC(62c9bfe3) SHA1(14a65a673a33b7e3d3005f76acf3098dc37958f8))
	ROM_LOAD_SND_8M("sxsnda.u36", 0x100000, CRC(a0bc0edb) SHA1(1025a28fe9a0e3681e8e99b513da29ec294da045))
	ROM_LOAD_SND_8M("sxsnda.u37", 0x180000, CRC(4c08c33c) SHA1(36bfad0c59fd228db76a6ff36698edd929c11336))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(strknew)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("sxcpua.102", 0x00000, 0x20000, CRC(5e5f0fb8) SHA1(1425d66064e59193ce7cacb496c12ae956670435))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("sxdispa.103", 0x00000, 0x80000, CRC(e4cf849f) SHA1(27b61f1659762b396ca3af375e28f9c56500f79e))

	ROM_REGION32_LE(0x600000, "soundcpu", ROMREGION_ERASEMASK | 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x1000000, "bsmt", 0 )
	ROM_LOAD("sxsnda.u7",0x0000,0x10000,CRC(e7e1a0cb) SHA1(be8b3e4d4232519db8344ae9e75f77d159bb1911))

	ROM_REGION(0x400000, "sound2", 0)
	ROM_LOAD("sxsnda.u17", 0x000000, 0x100000, CRC(aeeed88f) SHA1(e150fd243feffcdc5d66487e840cefdfb50213da))
	ROM_LOAD("sxsnda.u21", 0x100000, 0x100000, CRC(62c9bfe3) SHA1(14a65a673a33b7e3d3005f76acf3098dc37958f8))
	ROM_LOAD("sxsnda.u36", 0x200000, 0x100000, CRC(a0bc0edb) SHA1(1025a28fe9a0e3681e8e99b513da29ec294da045))
	ROM_LOAD("sxsnda.u37", 0x300000, 0x100000, CRC(4c08c33c) SHA1(36bfad0c59fd228db76a6ff36698edd929c11336))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(strxt_uk)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("sxcpue.101", 0x00000, 0x20000, CRC(eac29785) SHA1(42e01c501b4a0a7eaae244040777be8ba69860d5))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("sxdispa.103", 0x00000, 0x80000, CRC(e4cf849f) SHA1(27b61f1659762b396ca3af375e28f9c56500f79e))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("sxsnda.u7",0x0000,0x10000,CRC(e7e1a0cb) SHA1(be8b3e4d4232519db8344ae9e75f77d159bb1911))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("sxsnda.u17", 0x000000, CRC(aeeed88f) SHA1(e150fd243feffcdc5d66487e840cefdfb50213da))
	ROM_LOAD_SND_8M("sxsnda.u21", 0x080000, CRC(62c9bfe3) SHA1(14a65a673a33b7e3d3005f76acf3098dc37958f8))
	ROM_LOAD_SND_8M("sxsnda.u36", 0x100000, CRC(a0bc0edb) SHA1(1025a28fe9a0e3681e8e99b513da29ec294da045))
	ROM_LOAD_SND_8M("sxsnda.u37", 0x180000, CRC(4c08c33c) SHA1(36bfad0c59fd228db76a6ff36698edd929c11336))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(strxt_gr)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("sxcpug.103", 0x00000, 0x20000, CRC(d73b9020) SHA1(491cd9db172db0a35870524ed4f1b15fb67f2e78))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("sxdispg.103", 0x00000, 0x80000, CRC(eb656489) SHA1(476315a5d22b6d8c63e9a592167a00f0c87e86c9))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("sxsndg.u7",0x0000,0x10000,CRC(b38ec07d) SHA1(239a3a51c049b007d4c16c3bd1e003a5dfd3cecc))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("sxsndg.u17", 0x000000, CRC(19ecf6ca) SHA1(f61f9e821bb0cf7978073a2d2cb939999265277b))
	ROM_LOAD_SND_8M("sxsndg.u21", 0x080000, CRC(ee410b1e) SHA1(a0f7ff46536060be8f7c2c0e575e85814cd183e1))
	ROM_LOAD_SND_8M("sxsndg.u36", 0x100000, CRC(f0e126c2) SHA1(a8c5eed27b33d20c2ff3bfd3d317c8b56bfa3625))
	ROM_LOAD_SND_8M("sxsndg.u37", 0x180000, CRC(82260f4b) SHA1(6c2eba67762bcdd01e7b0c1b8b03b91b778444d4))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(strxt_fr)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("sxcpuf.102", 0x00000, 0x20000, CRC(2804bc9f) SHA1(3c75d8cc4d2833baa163d99c709dcb9475ba0831))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("sxdispf.103", 0x00000, 0x80000, CRC(4b4b5c19) SHA1(d2612a2b8099b45cb67ac9b55c88b5b10519d49b))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("sxsndf.u7",0x0000,0x10000,CRC(e68b0607) SHA1(cd3a5ff51932914e977fe866f7ab569d0901967a))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("sxsndf.u17", 0x000000, CRC(443efde2) SHA1(823f395cc5b0c7f5665bd8c804707fb9bbad1066))
	ROM_LOAD_SND_8M("sxsndf.u21", 0x080000, CRC(e8ba1618) SHA1(920cecbdcfc948670ddf11b572af0bb536a1153d))
	ROM_LOAD_SND_8M("sxsndf.u36", 0x100000, CRC(89107426) SHA1(9e3c51f17aee0e803e54f9400c304b4da0b8cf7a))
	ROM_LOAD_SND_8M("sxsndf.u37", 0x180000, CRC(67c0f1de) SHA1(46867403d4b13d18c4ebcc5b042faf3aca165ffb))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(strxt_it)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("sxcpui.102", 0x00000, 0x20000, CRC(f955d0ef) SHA1(0f4ee87715bc085e2fb05e9ebdc89403f6bac444))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("sxdispi.103", 0x00000, 0x80000, CRC(40be3fe2) SHA1(a5e37ecf3b9772736ac88256c470f785dc113aa1))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("sxsndi.u7",0x0000,0x10000,CRC(81caf0a7) SHA1(5bb05c5bb49d12417b3ad49398623c3c222fd63b))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("sxsndi.u17", 0x000000, CRC(d0b21193) SHA1(2e5f92a67f0f18913e5d0af9936ab8694d095c66))
	ROM_LOAD_SND_8M("sxsndi.u21", 0x080000, CRC(5ab3f8f4) SHA1(44982725eb31b0b144e3ad6549734b5fc46cd8c5))
	ROM_LOAD_SND_8M("sxsndi.u36", 0x100000, CRC(4ee21ade) SHA1(1887f81b5f6753ce75ddcd0d7557c1644a925fcf))
	ROM_LOAD_SND_8M("sxsndi.u37", 0x180000, CRC(4427e364) SHA1(7046b65086aafc4c14793d7036bc5130fe1e7dbc))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(strxt_it_101)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("sxcpui.101", 0x00000, 0x20000, CRC(121e04de) SHA1(90c222106c3422a7cd10a493b2290315d8d7009f))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("sxdispi.103", 0x00000, 0x80000, CRC(40be3fe2) SHA1(a5e37ecf3b9772736ac88256c470f785dc113aa1))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("sxsndi.u7",0x0000,0x10000,CRC(81caf0a7) SHA1(5bb05c5bb49d12417b3ad49398623c3c222fd63b))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("sxsndi.u17", 0x000000, CRC(d0b21193) SHA1(2e5f92a67f0f18913e5d0af9936ab8694d095c66))
	ROM_LOAD_SND_8M("sxsndi.u21", 0x080000, CRC(5ab3f8f4) SHA1(44982725eb31b0b144e3ad6549734b5fc46cd8c5))
	ROM_LOAD_SND_8M("sxsndi.u36", 0x100000, CRC(4ee21ade) SHA1(1887f81b5f6753ce75ddcd0d7557c1644a925fcf))
	ROM_LOAD_SND_8M("sxsndi.u37", 0x180000, CRC(4427e364) SHA1(7046b65086aafc4c14793d7036bc5130fe1e7dbc))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(strxt_sp)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("sxcpul.102", 0x00000, 0x20000, CRC(6b1e417f) SHA1(b87e8659bc4481384913a28c1cb2d7c96532b8e3))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("sxdispl.103", 0x00000, 0x80000, CRC(3efd4a18) SHA1(64f6998f82947a5bd053ad8dd56682adb239b676))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("sxsndl.u7",0x0000,0x10000,CRC(a03131cf) SHA1(e50f665eb15cef799fdc0d1d88bc7d5e23390225))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("sxsndl.u17", 0x000000, CRC(e7ee91cb) SHA1(1bc9992601bd7d194e2f33241179d682a62bff4b))
	ROM_LOAD_SND_8M("sxsndl.u21", 0x080000, CRC(88cbf553) SHA1(d6afd262b47e31983c734c0054a7af2489da2f13))
	ROM_LOAD_SND_8M("sxsndl.u36", 0x100000, CRC(35474ad5) SHA1(818a0f86fa4aa0b0457c16a20f8358655c42ea0a))
	ROM_LOAD_SND_8M("sxsndl.u37", 0x180000, CRC(0e53f2a0) SHA1(7b89989ff87c25618d6f1c6479efd45b57f850fb))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

/*-------------------------------------------------------------------
/ Terminator 3: Rise of the Machines
/-------------------------------------------------------------------*/
ROM_START(term3)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("t3cpu.400", 0x00000, 0x20000, CRC(872f9351) SHA1(8fa8b503d8c3dbac66df1cb0ba400dbd58ee28ee))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("t3dispa.400", 0x00000, 0x80000, CRC(6b7cc4f8) SHA1(214e9b3e45b778841fc166acf4ff5fd634ae2670))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("t3100.u7",0x0000,0x10000,CRC(7f99e3af) SHA1(4916c074e2a4c947d1a658300f9f9629da1a8bb8))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("t3100.u17", 0x000000, CRC(f0c71b5d) SHA1(e9f726a232fbd4f34b8b07069f337dbb3daf394a))
	ROM_LOAD_SND_8M("t3100.u21", 0x080000, CRC(694331f7) SHA1(e9ae8c5db2e59c0f9df923c98f6e75896e150807))
	ROM_LOAD_SND_8M("t3100.u36", 0x100000, CRC(9eb512e9) SHA1(fa2fecf6cb0c1af3c6db244f9d94ba53d13e10fc))
	ROM_LOAD_SND_8M("t3100.u37", 0x180000, CRC(3efb0c19) SHA1(6894295eef05891d64c7274512ba27f2b63ca3ec))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(term3_205)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "t3cpu.205",     0x0000, 0x20000, CRC(219d8155) SHA1(dac3c390d238872703a0ec65cb3dddc89630359c))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "t3dispa.201",   0x0000, 0x80000, CRC(a314acd1) SHA1(4d5072e65f8041d24c1bab2985ef5b30e1895bf3))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("t3100.u7",0x0000,0x10000,CRC(7f99e3af) SHA1(4916c074e2a4c947d1a658300f9f9629da1a8bb8))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("t3100.u17", 0x000000, CRC(f0c71b5d) SHA1(e9f726a232fbd4f34b8b07069f337dbb3daf394a))
	ROM_LOAD_SND_8M("t3100.u21", 0x080000, CRC(694331f7) SHA1(e9ae8c5db2e59c0f9df923c98f6e75896e150807))
	ROM_LOAD_SND_8M("t3100.u36", 0x100000, CRC(9eb512e9) SHA1(fa2fecf6cb0c1af3c6db244f9d94ba53d13e10fc))
	ROM_LOAD_SND_8M("t3100.u37", 0x180000, CRC(3efb0c19) SHA1(6894295eef05891d64c7274512ba27f2b63ca3ec))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(t3new)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("t3cpu.400", 0x00000, 0x20000, CRC(872f9351) SHA1(8fa8b503d8c3dbac66df1cb0ba400dbd58ee28ee))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("t3dispa.400", 0x00000, 0x80000, CRC(6b7cc4f8) SHA1(214e9b3e45b778841fc166acf4ff5fd634ae2670))

	ROM_REGION32_LE(0x600000, "soundcpu", ROMREGION_ERASEMASK | 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x1000000, "bsmt", 0 )
	ROM_LOAD("t3100.u7",0x0000,0x10000,CRC(7f99e3af) SHA1(4916c074e2a4c947d1a658300f9f9629da1a8bb8))

	ROM_REGION(0x400000, "sound2", 0)
	ROM_LOAD("t3100.u17", 0x000000, 0x100000, CRC(f0c71b5d) SHA1(e9f726a232fbd4f34b8b07069f337dbb3daf394a))
	ROM_LOAD("t3100.u21", 0x100000, 0x100000, CRC(694331f7) SHA1(e9ae8c5db2e59c0f9df923c98f6e75896e150807))
	ROM_LOAD("t3100.u36", 0x200000, 0x100000, CRC(9eb512e9) SHA1(fa2fecf6cb0c1af3c6db244f9d94ba53d13e10fc))
	ROM_LOAD("t3100.u37", 0x300000, 0x100000, CRC(3efb0c19) SHA1(6894295eef05891d64c7274512ba27f2b63ca3ec))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others

ROM_END

ROM_START(term3g)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("t3cpu.400", 0x00000, 0x20000, CRC(872f9351) SHA1(8fa8b503d8c3dbac66df1cb0ba400dbd58ee28ee))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("t3dispg.400", 0x00000, 0x80000, CRC(20da21b2) SHA1(9115aef55d9ac36a49ae5c5fd423f05c669b0335))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("t3100.u7",0x0000,0x10000,CRC(7f99e3af) SHA1(4916c074e2a4c947d1a658300f9f9629da1a8bb8))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("t3100.u17", 0x000000, CRC(f0c71b5d) SHA1(e9f726a232fbd4f34b8b07069f337dbb3daf394a))
	ROM_LOAD_SND_8M("t3100.u21", 0x080000, CRC(694331f7) SHA1(e9ae8c5db2e59c0f9df923c98f6e75896e150807))
	ROM_LOAD_SND_8M("t3100.u36", 0x100000, CRC(9eb512e9) SHA1(fa2fecf6cb0c1af3c6db244f9d94ba53d13e10fc))
	ROM_LOAD_SND_8M("t3100.u37", 0x180000, CRC(3efb0c19) SHA1(6894295eef05891d64c7274512ba27f2b63ca3ec))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(term3l)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("t3cpu.400", 0x00000, 0x20000, CRC(872f9351) SHA1(8fa8b503d8c3dbac66df1cb0ba400dbd58ee28ee))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("t3displ.400", 0x00000, 0x80000, CRC(2e21caba) SHA1(d29afa05d68484c762799c799bd1ccd1aad252b7))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("t3100.u7",0x0000,0x10000,CRC(7f99e3af) SHA1(4916c074e2a4c947d1a658300f9f9629da1a8bb8))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("t3100.u17", 0x000000, CRC(f0c71b5d) SHA1(e9f726a232fbd4f34b8b07069f337dbb3daf394a))
	ROM_LOAD_SND_8M("t3100.u21", 0x080000, CRC(694331f7) SHA1(e9ae8c5db2e59c0f9df923c98f6e75896e150807))
	ROM_LOAD_SND_8M("t3100.u36", 0x100000, CRC(9eb512e9) SHA1(fa2fecf6cb0c1af3c6db244f9d94ba53d13e10fc))
	ROM_LOAD_SND_8M("t3100.u37", 0x180000, CRC(3efb0c19) SHA1(6894295eef05891d64c7274512ba27f2b63ca3ec))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Display PLD
ROM_END

ROM_START(term3l_205)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "t3cpu.205",     0x0000, 0x20000, CRC(219d8155) SHA1(dac3c390d238872703a0ec65cb3dddc89630359c))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "t3displ.201",   0x0000, 0x80000, CRC(180b55a2) SHA1(1d8161fc806804e0712ee8a07a2cac0561949f0c))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("t3100.u7",0x0000,0x10000,CRC(7f99e3af) SHA1(4916c074e2a4c947d1a658300f9f9629da1a8bb8))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("t3100.u17", 0x000000, CRC(f0c71b5d) SHA1(e9f726a232fbd4f34b8b07069f337dbb3daf394a))
	ROM_LOAD_SND_8M("t3100.u21", 0x080000, CRC(694331f7) SHA1(e9ae8c5db2e59c0f9df923c98f6e75896e150807))
	ROM_LOAD_SND_8M("t3100.u36", 0x100000, CRC(9eb512e9) SHA1(fa2fecf6cb0c1af3c6db244f9d94ba53d13e10fc))
	ROM_LOAD_SND_8M("t3100.u37", 0x180000, CRC(3efb0c19) SHA1(6894295eef05891d64c7274512ba27f2b63ca3ec))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Display PLD
ROM_END

ROM_START(term3f)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("t3cpu.400", 0x00000, 0x20000, CRC(872f9351) SHA1(8fa8b503d8c3dbac66df1cb0ba400dbd58ee28ee))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("t3dispf.400", 0x00000, 0x80000, CRC(0645fe6d) SHA1(1a7dfa160ba6cc1335a59b018289982f2a46a7bb))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("t3100.u7",0x0000,0x10000,CRC(7f99e3af) SHA1(4916c074e2a4c947d1a658300f9f9629da1a8bb8))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("t3100.u17", 0x000000, CRC(f0c71b5d) SHA1(e9f726a232fbd4f34b8b07069f337dbb3daf394a))
	ROM_LOAD_SND_8M("t3100.u21", 0x080000, CRC(694331f7) SHA1(e9ae8c5db2e59c0f9df923c98f6e75896e150807))
	ROM_LOAD_SND_8M("t3100.u36", 0x100000, CRC(9eb512e9) SHA1(fa2fecf6cb0c1af3c6db244f9d94ba53d13e10fc))
	ROM_LOAD_SND_8M("t3100.u37", 0x180000, CRC(3efb0c19) SHA1(6894295eef05891d64c7274512ba27f2b63ca3ec))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(term3f_205)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "t3cpu.205",     0x0000, 0x20000, CRC(219d8155) SHA1(dac3c390d238872703a0ec65cb3dddc89630359c))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "t3dispf.201",   0x0000, 0x80000, CRC(ced87154) SHA1(893c071bb2427429ca45f4d2088b015c5f638207))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("t3100.u7",0x0000,0x10000,CRC(7f99e3af) SHA1(4916c074e2a4c947d1a658300f9f9629da1a8bb8))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("t3100.u17", 0x000000, CRC(f0c71b5d) SHA1(e9f726a232fbd4f34b8b07069f337dbb3daf394a))
	ROM_LOAD_SND_8M("t3100.u21", 0x080000, CRC(694331f7) SHA1(e9ae8c5db2e59c0f9df923c98f6e75896e150807))
	ROM_LOAD_SND_8M("t3100.u36", 0x100000, CRC(9eb512e9) SHA1(fa2fecf6cb0c1af3c6db244f9d94ba53d13e10fc))
	ROM_LOAD_SND_8M("t3100.u37", 0x180000, CRC(3efb0c19) SHA1(6894295eef05891d64c7274512ba27f2b63ca3ec))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(term3i)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("t3cpu.400", 0x00000, 0x20000, CRC(872f9351) SHA1(8fa8b503d8c3dbac66df1cb0ba400dbd58ee28ee))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("t3dispi.400", 0x00000, 0x80000, CRC(e8ea9ab8) SHA1(7b25bb7d3095e6bd2d94342d0e078590cb75074b))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("t3100.u7",0x0000,0x10000,CRC(7f99e3af) SHA1(4916c074e2a4c947d1a658300f9f9629da1a8bb8))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("t3100.u17", 0x000000, CRC(f0c71b5d) SHA1(e9f726a232fbd4f34b8b07069f337dbb3daf394a))
	ROM_LOAD_SND_8M("t3100.u21", 0x080000, CRC(694331f7) SHA1(e9ae8c5db2e59c0f9df923c98f6e75896e150807))
	ROM_LOAD_SND_8M("t3100.u36", 0x100000, CRC(9eb512e9) SHA1(fa2fecf6cb0c1af3c6db244f9d94ba53d13e10fc))
	ROM_LOAD_SND_8M("t3100.u37", 0x180000, CRC(3efb0c19) SHA1(6894295eef05891d64c7274512ba27f2b63ca3ec))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(term3i_205)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "t3cpu.205",     0x0000, 0x20000, CRC(219d8155) SHA1(dac3c390d238872703a0ec65cb3dddc89630359c))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "t3dispi.201",   0x0000, 0x80000, CRC(c1f3604f) SHA1(8a391e6471ced52662aa69261ac29a279c7b8a7d))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("t3100.u7",0x0000,0x10000,CRC(7f99e3af) SHA1(4916c074e2a4c947d1a658300f9f9629da1a8bb8))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("t3100.u17", 0x000000, CRC(f0c71b5d) SHA1(e9f726a232fbd4f34b8b07069f337dbb3daf394a))
	ROM_LOAD_SND_8M("t3100.u21", 0x080000, CRC(694331f7) SHA1(e9ae8c5db2e59c0f9df923c98f6e75896e150807))
	ROM_LOAD_SND_8M("t3100.u36", 0x100000, CRC(9eb512e9) SHA1(fa2fecf6cb0c1af3c6db244f9d94ba53d13e10fc))
	ROM_LOAD_SND_8M("t3100.u37", 0x180000, CRC(3efb0c19) SHA1(6894295eef05891d64c7274512ba27f2b63ca3ec))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(term3_301)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("t3cpu.301", 0x00000, 0x20000, CRC(172a0b83) SHA1(68f6a228182040a0ea6b310cb25d3d5bdd2574bf))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("t3dispa.300", 0x00000, 0x80000, CRC(79b68a2f) SHA1(cd466c15ffe09666c115f843775e457138bf23bc))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("t3100.u7",0x0000,0x10000,CRC(7f99e3af) SHA1(4916c074e2a4c947d1a658300f9f9629da1a8bb8))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("t3100.u17", 0x000000, CRC(f0c71b5d) SHA1(e9f726a232fbd4f34b8b07069f337dbb3daf394a))
	ROM_LOAD_SND_8M("t3100.u21", 0x080000, CRC(694331f7) SHA1(e9ae8c5db2e59c0f9df923c98f6e75896e150807))
	ROM_LOAD_SND_8M("t3100.u36", 0x100000, CRC(9eb512e9) SHA1(fa2fecf6cb0c1af3c6db244f9d94ba53d13e10fc))
	ROM_LOAD_SND_8M("t3100.u37", 0x180000, CRC(3efb0c19) SHA1(6894295eef05891d64c7274512ba27f2b63ca3ec))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(term3l_301)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("t3cpu.301", 0x00000, 0x20000, CRC(172a0b83) SHA1(68f6a228182040a0ea6b310cb25d3d5bdd2574bf))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("t3displ.300", 0x00000, 0x80000, CRC(2df35b3f) SHA1(5716b46c16cc7c4478f3118c4e6c3959b10624f8))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("t3100.u7",0x0000,0x10000,CRC(7f99e3af) SHA1(4916c074e2a4c947d1a658300f9f9629da1a8bb8))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("t3100.u17", 0x000000, CRC(f0c71b5d) SHA1(e9f726a232fbd4f34b8b07069f337dbb3daf394a))
	ROM_LOAD_SND_8M("t3100.u21", 0x080000, CRC(694331f7) SHA1(e9ae8c5db2e59c0f9df923c98f6e75896e150807))
	ROM_LOAD_SND_8M("t3100.u36", 0x100000, CRC(9eb512e9) SHA1(fa2fecf6cb0c1af3c6db244f9d94ba53d13e10fc))
	ROM_LOAD_SND_8M("t3100.u37", 0x180000, CRC(3efb0c19) SHA1(6894295eef05891d64c7274512ba27f2b63ca3ec))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(term3f_301)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("t3cpu.301", 0x00000, 0x20000, CRC(172a0b83) SHA1(68f6a228182040a0ea6b310cb25d3d5bdd2574bf))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("t3dispf.300", 0x00000, 0x80000, CRC(d5c68903) SHA1(00ca09f087e5b2a742d0bf6f2ff5706a2b83a295))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("t3100.u7",0x0000,0x10000,CRC(7f99e3af) SHA1(4916c074e2a4c947d1a658300f9f9629da1a8bb8))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("t3100.u17", 0x000000, CRC(f0c71b5d) SHA1(e9f726a232fbd4f34b8b07069f337dbb3daf394a))
	ROM_LOAD_SND_8M("t3100.u21", 0x080000, CRC(694331f7) SHA1(e9ae8c5db2e59c0f9df923c98f6e75896e150807))
	ROM_LOAD_SND_8M("t3100.u36", 0x100000, CRC(9eb512e9) SHA1(fa2fecf6cb0c1af3c6db244f9d94ba53d13e10fc))
	ROM_LOAD_SND_8M("t3100.u37", 0x180000, CRC(3efb0c19) SHA1(6894295eef05891d64c7274512ba27f2b63ca3ec))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(term3g_301)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("t3cpu.301", 0x00000, 0x20000, CRC(172a0b83) SHA1(68f6a228182040a0ea6b310cb25d3d5bdd2574bf))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("t3dispg.300", 0x00000, 0x80000, CRC(9115ea52) SHA1(52bd2cbe609363d9904b82704072fc3c398a7c18))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("t3100.u7",0x0000,0x10000,CRC(7f99e3af) SHA1(4916c074e2a4c947d1a658300f9f9629da1a8bb8))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("t3100.u17", 0x000000, CRC(f0c71b5d) SHA1(e9f726a232fbd4f34b8b07069f337dbb3daf394a))
	ROM_LOAD_SND_8M("t3100.u21", 0x080000, CRC(694331f7) SHA1(e9ae8c5db2e59c0f9df923c98f6e75896e150807))
	ROM_LOAD_SND_8M("t3100.u36", 0x100000, CRC(9eb512e9) SHA1(fa2fecf6cb0c1af3c6db244f9d94ba53d13e10fc))
	ROM_LOAD_SND_8M("t3100.u37", 0x180000, CRC(3efb0c19) SHA1(6894295eef05891d64c7274512ba27f2b63ca3ec))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(term3i_301)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("t3cpu.301", 0x00000, 0x20000, CRC(172a0b83) SHA1(68f6a228182040a0ea6b310cb25d3d5bdd2574bf))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("t3dispi.300", 0x00000, 0x80000, CRC(30573629) SHA1(85ae7183b42a62f62aa3ba6441717fc7a49dd03a))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("t3100.u7",0x0000,0x10000,CRC(7f99e3af) SHA1(4916c074e2a4c947d1a658300f9f9629da1a8bb8))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("t3100.u17", 0x000000, CRC(f0c71b5d) SHA1(e9f726a232fbd4f34b8b07069f337dbb3daf394a))
	ROM_LOAD_SND_8M("t3100.u21", 0x080000, CRC(694331f7) SHA1(e9ae8c5db2e59c0f9df923c98f6e75896e150807))
	ROM_LOAD_SND_8M("t3100.u36", 0x100000, CRC(9eb512e9) SHA1(fa2fecf6cb0c1af3c6db244f9d94ba53d13e10fc))
	ROM_LOAD_SND_8M("t3100.u37", 0x180000, CRC(3efb0c19) SHA1(6894295eef05891d64c7274512ba27f2b63ca3ec))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

/*-------------------------------------------------------------------
/ The Brain
/-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
/ The Lost World: Jurassic Park
/-------------------------------------------------------------------*/
ROM_START(jplstw22)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("jp2cpu.202", 0x00000, 0x20000, CRC(d317e601) SHA1(fb4fee5fd08e0f1b5afb9817654bdb3d54a5220d))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("jp2dspa.201", 0x00000, 0x80000, CRC(8fc41ace) SHA1(9d11f7623eec41972d2be4313c7715e30116d889))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("jp2_u7.bin", 0x0000, 0x10000, CRC(73b74c96) SHA1(ffa47cbf1491ed4fbadc984189abbfffc70c9888))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD("jp2_u17.bin", 0x000000, 0x80000, CRC(8d6c0dbd) SHA1(e1179b2c94927a07efa7d16cf841d5ff7334ff36))
	ROM_LOAD("jp2_u21.bin", 0x080000, 0x80000, CRC(c670a997) SHA1(1576e11ec3669f61ff16188de31b9ef3a067c473))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(jplstw20)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("jp2cpu.200", 0x00000, 0x20000, CRC(42f5526e) SHA1(66254492c981117c06567305237cadfc0ce391b0))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("jp2dspa.201", 0x00000, 0x80000, CRC(8fc41ace) SHA1(9d11f7623eec41972d2be4313c7715e30116d889))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("jp2_u7.bin", 0x0000, 0x10000, CRC(73b74c96) SHA1(ffa47cbf1491ed4fbadc984189abbfffc70c9888))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD("jp2_u17.bin", 0x000000, 0x80000, CRC(8d6c0dbd) SHA1(e1179b2c94927a07efa7d16cf841d5ff7334ff36))
	ROM_LOAD("jp2_u21.bin", 0x080000, 0x80000, CRC(c670a997) SHA1(1576e11ec3669f61ff16188de31b9ef3a067c473))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

/*-------------------------------------------------------------------
/ The Simpsons Pinball Party
/-------------------------------------------------------------------*/
ROM_START(simpprty)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("spp-cpu.500", 0x00000, 0x20000, CRC(215ce09c) SHA1(f3baaaa1b9f12a98109da55746031eb9f5f8790c))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("sppdispa.500", 0x00000, 0x80000, CRC(c6db83ec) SHA1(6079981e19b4651a074b0005eca85faf0eebcec0))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("spp101.u7",0x0000,0x10000,CRC(32efcdf6) SHA1(1d437e8649408be91e0dd10598cc67336203077f))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("spp100.u17", 0x000000, CRC(65e9344e) SHA1(fe4797ccb71b31aa39d6a5d373a01fc22f9d055c))
	ROM_LOAD_SND_8M("spp100.u21", 0x080000, CRC(17fee0f9) SHA1(5b5ceb667f3bc9bde4ea08a1ef837e3b56c01977))
	ROM_LOAD_SND_8M("spp100.u36", 0x100000, CRC(ffb957b0) SHA1(d6876ec63525099a7073c196867c17111272c69a))
	ROM_LOAD_SND_8M("spp100.u37", 0x180000, CRC(0738e1fc) SHA1(268462c06e5c1f286e5faaee1c0815448cc2eafa))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(simpprty_400)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "spp-cpu.400",  0x00000, 0x20000, CRC(530b9782) SHA1(573b20cac205b7989cdefceb2c31cb7d88c2951a))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "sppdspa.400",   0x00000, 0x80000, CRC(cd5eaab7) SHA1(a06bef6fc0e7f3c0616439cb0e0431a3d52cdfa1))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("spp101.u7",0x0000,0x10000,CRC(32efcdf6) SHA1(1d437e8649408be91e0dd10598cc67336203077f))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("spp100.u17", 0x000000, CRC(65e9344e) SHA1(fe4797ccb71b31aa39d6a5d373a01fc22f9d055c))
	ROM_LOAD_SND_8M("spp100.u21", 0x080000, CRC(17fee0f9) SHA1(5b5ceb667f3bc9bde4ea08a1ef837e3b56c01977))
	ROM_LOAD_SND_8M("spp100.u36", 0x100000, CRC(ffb957b0) SHA1(d6876ec63525099a7073c196867c17111272c69a))
	ROM_LOAD_SND_8M("spp100.u37", 0x180000, CRC(0738e1fc) SHA1(268462c06e5c1f286e5faaee1c0815448cc2eafa))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(simpprty_204)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("spp-cpu.204",  0x00000, 0x20000, CRC(5bc155f7) SHA1(78d793cecbc6561a891ff8007f33c63ec5515e9f))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("sppdispa.201", 0x00000, 0x80000, CRC(f55505a4) SHA1(5616959caafc836d13db9c1a1e93cb4954f0c321))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("spp101.u7",0x0000,0x10000,CRC(32efcdf6) SHA1(1d437e8649408be91e0dd10598cc67336203077f))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("spp100.u17", 0x000000, CRC(65e9344e) SHA1(fe4797ccb71b31aa39d6a5d373a01fc22f9d055c))
	ROM_LOAD_SND_8M("spp100.u21", 0x080000, CRC(17fee0f9) SHA1(5b5ceb667f3bc9bde4ea08a1ef837e3b56c01977))
	ROM_LOAD_SND_8M("spp100.u36", 0x100000, CRC(ffb957b0) SHA1(d6876ec63525099a7073c196867c17111272c69a))
	ROM_LOAD_SND_8M("spp100.u37", 0x180000, CRC(0738e1fc) SHA1(268462c06e5c1f286e5faaee1c0815448cc2eafa))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(simpnew)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("spp-cpu.500", 0x00000, 0x20000, CRC(215ce09c) SHA1(f3baaaa1b9f12a98109da55746031eb9f5f8790c))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("sppdispa.500", 0x00000, 0x80000, CRC(c6db83ec) SHA1(6079981e19b4651a074b0005eca85faf0eebcec0))

	ROM_REGION32_LE(0x600000, "soundcpu", ROMREGION_ERASEMASK | 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x1000000, "bsmt", 0 )
	ROM_LOAD("spp101.u7",0x0000,0x10000,CRC(32efcdf6) SHA1(1d437e8649408be91e0dd10598cc67336203077f))

	ROM_REGION(0x400000, "sound2", 0)
	ROM_LOAD("spp100.u17", 0x000000, 0x100000, CRC(65e9344e) SHA1(fe4797ccb71b31aa39d6a5d373a01fc22f9d055c))
	ROM_LOAD("spp100.u21", 0x100000, 0x100000, CRC(17fee0f9) SHA1(5b5ceb667f3bc9bde4ea08a1ef837e3b56c01977))
	ROM_LOAD("spp100.u36", 0x200000, 0x100000, CRC(ffb957b0) SHA1(d6876ec63525099a7073c196867c17111272c69a))
	ROM_LOAD("spp100.u37", 0x300000, 0x100000, CRC(0738e1fc) SHA1(268462c06e5c1f286e5faaee1c0815448cc2eafa))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(simpprtg)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("spp-cpu.500", 0x00000, 0x20000, CRC(215ce09c) SHA1(f3baaaa1b9f12a98109da55746031eb9f5f8790c))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("sppdispg.500", 0x00000, 0x80000, CRC(6503bffc) SHA1(717aa8b7a0329c886ddb4b167c022b3a2ee3ab2d))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("spp101.u7",0x0000,0x10000,CRC(32efcdf6) SHA1(1d437e8649408be91e0dd10598cc67336203077f))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("spp100.u17", 0x000000, CRC(65e9344e) SHA1(fe4797ccb71b31aa39d6a5d373a01fc22f9d055c))
	ROM_LOAD_SND_8M("spp100.u21", 0x080000, CRC(17fee0f9) SHA1(5b5ceb667f3bc9bde4ea08a1ef837e3b56c01977))
	ROM_LOAD_SND_8M("spp100.u36", 0x100000, CRC(ffb957b0) SHA1(d6876ec63525099a7073c196867c17111272c69a))
	ROM_LOAD_SND_8M("spp100.u37", 0x180000, CRC(0738e1fc) SHA1(268462c06e5c1f286e5faaee1c0815448cc2eafa))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(simpprtg_400)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "spp-cpu.400",  0x00000, 0x20000, CRC(530b9782) SHA1(573b20cac205b7989cdefceb2c31cb7d88c2951a))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "sppdspg.400",   0x00000, 0x80000, CRC(3b408fe2) SHA1(ce8d7f0d58b5f8fb4df0b9811449e4dc0e1e6580))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("spp101.u7",0x0000,0x10000,CRC(32efcdf6) SHA1(1d437e8649408be91e0dd10598cc67336203077f))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("spp100.u17", 0x000000, CRC(65e9344e) SHA1(fe4797ccb71b31aa39d6a5d373a01fc22f9d055c))
	ROM_LOAD_SND_8M("spp100.u21", 0x080000, CRC(17fee0f9) SHA1(5b5ceb667f3bc9bde4ea08a1ef837e3b56c01977))
	ROM_LOAD_SND_8M("spp100.u36", 0x100000, CRC(ffb957b0) SHA1(d6876ec63525099a7073c196867c17111272c69a))
	ROM_LOAD_SND_8M("spp100.u37", 0x180000, CRC(0738e1fc) SHA1(268462c06e5c1f286e5faaee1c0815448cc2eafa))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(simpprtl)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("spp-cpu.500", 0x00000, 0x20000, CRC(215ce09c) SHA1(f3baaaa1b9f12a98109da55746031eb9f5f8790c))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("sppdispl.500", 0x00000, 0x80000, CRC(0821f182) SHA1(7998ab29dae59d077b1dedd28a30a3477251d107))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("spp101.u7",0x0000,0x10000,CRC(32efcdf6) SHA1(1d437e8649408be91e0dd10598cc67336203077f))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("spp100.u17", 0x000000, CRC(65e9344e) SHA1(fe4797ccb71b31aa39d6a5d373a01fc22f9d055c))
	ROM_LOAD_SND_8M("spp100.u21", 0x080000, CRC(17fee0f9) SHA1(5b5ceb667f3bc9bde4ea08a1ef837e3b56c01977))
	ROM_LOAD_SND_8M("spp100.u36", 0x100000, CRC(ffb957b0) SHA1(d6876ec63525099a7073c196867c17111272c69a))
	ROM_LOAD_SND_8M("spp100.u37", 0x180000, CRC(0738e1fc) SHA1(268462c06e5c1f286e5faaee1c0815448cc2eafa))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(simpprtl_400)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "spp-cpu.400",  0x00000, 0x20000, CRC(530b9782) SHA1(573b20cac205b7989cdefceb2c31cb7d88c2951a))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "sppdspl.400",   0x00000, 0x80000, CRC(a0bf567e) SHA1(ce6eb65da6bff15aeb787fd2cdac7cf6b4300108))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("spp101.u7",0x0000,0x10000,CRC(32efcdf6) SHA1(1d437e8649408be91e0dd10598cc67336203077f))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("spp100.u17", 0x000000, CRC(65e9344e) SHA1(fe4797ccb71b31aa39d6a5d373a01fc22f9d055c))
	ROM_LOAD_SND_8M("spp100.u21", 0x080000, CRC(17fee0f9) SHA1(5b5ceb667f3bc9bde4ea08a1ef837e3b56c01977))
	ROM_LOAD_SND_8M("spp100.u36", 0x100000, CRC(ffb957b0) SHA1(d6876ec63525099a7073c196867c17111272c69a))
	ROM_LOAD_SND_8M("spp100.u37", 0x180000, CRC(0738e1fc) SHA1(268462c06e5c1f286e5faaee1c0815448cc2eafa))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(simpprtl_204)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("spp-cpu.204",  0x00000, 0x20000, CRC(5bc155f7) SHA1(78d793cecbc6561a891ff8007f33c63ec5515e9f))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("sppdispl.201", 0x00000, 0x80000, CRC(78a67e23) SHA1(c4ef2b0301104280410aefdacc847e74a8c6a49f))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("spp101.u7",0x0000,0x10000,CRC(32efcdf6) SHA1(1d437e8649408be91e0dd10598cc67336203077f))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("spp100.u17", 0x000000, CRC(65e9344e) SHA1(fe4797ccb71b31aa39d6a5d373a01fc22f9d055c))
	ROM_LOAD_SND_8M("spp100.u21", 0x080000, CRC(17fee0f9) SHA1(5b5ceb667f3bc9bde4ea08a1ef837e3b56c01977))
	ROM_LOAD_SND_8M("spp100.u36", 0x100000, CRC(ffb957b0) SHA1(d6876ec63525099a7073c196867c17111272c69a))
	ROM_LOAD_SND_8M("spp100.u37", 0x180000, CRC(0738e1fc) SHA1(268462c06e5c1f286e5faaee1c0815448cc2eafa))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(simpprtf)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("spp-cpu.500", 0x00000, 0x20000, CRC(215ce09c) SHA1(f3baaaa1b9f12a98109da55746031eb9f5f8790c))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("sppdispf.500", 0x00000, 0x80000, CRC(8d3383ed) SHA1(a56b1043fe1b0280d11386981fe9c181c9b6f1b7))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("spp101.u7",0x0000,0x10000,CRC(32efcdf6) SHA1(1d437e8649408be91e0dd10598cc67336203077f))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("spp100.u17", 0x000000, CRC(65e9344e) SHA1(fe4797ccb71b31aa39d6a5d373a01fc22f9d055c))
	ROM_LOAD_SND_8M("spp100.u21", 0x080000, CRC(17fee0f9) SHA1(5b5ceb667f3bc9bde4ea08a1ef837e3b56c01977))
	ROM_LOAD_SND_8M("spp100.u36", 0x100000, CRC(ffb957b0) SHA1(d6876ec63525099a7073c196867c17111272c69a))
	ROM_LOAD_SND_8M("spp100.u37", 0x180000, CRC(0738e1fc) SHA1(268462c06e5c1f286e5faaee1c0815448cc2eafa))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(simpprtf_400)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "spp-cpu.400",  0x00000, 0x20000, CRC(530b9782) SHA1(573b20cac205b7989cdefceb2c31cb7d88c2951a))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "sppdspf.400",   0x00000, 0x80000, CRC(6cc306e2) SHA1(bfe6ef0cd5d0cb5e3b29d85ade1700005e22d81b))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("spp101.u7",0x0000,0x10000,CRC(32efcdf6) SHA1(1d437e8649408be91e0dd10598cc67336203077f))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("spp100.u17", 0x000000, CRC(65e9344e) SHA1(fe4797ccb71b31aa39d6a5d373a01fc22f9d055c))
	ROM_LOAD_SND_8M("spp100.u21", 0x080000, CRC(17fee0f9) SHA1(5b5ceb667f3bc9bde4ea08a1ef837e3b56c01977))
	ROM_LOAD_SND_8M("spp100.u36", 0x100000, CRC(ffb957b0) SHA1(d6876ec63525099a7073c196867c17111272c69a))
	ROM_LOAD_SND_8M("spp100.u37", 0x180000, CRC(0738e1fc) SHA1(268462c06e5c1f286e5faaee1c0815448cc2eafa))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(simpprtf_204)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("spp-cpu.204",  0x00000, 0x20000, CRC(5bc155f7) SHA1(78d793cecbc6561a891ff8007f33c63ec5515e9f))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("sppdispf.201", 0x00000, 0x80000, CRC(d1c0c484) SHA1(615e3a8ba62b3f6d0ba53fbaf4b7d9e7fcdc9d82))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("spp101.u7",0x0000,0x10000,CRC(32efcdf6) SHA1(1d437e8649408be91e0dd10598cc67336203077f))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("spp100.u17", 0x000000, CRC(65e9344e) SHA1(fe4797ccb71b31aa39d6a5d373a01fc22f9d055c))
	ROM_LOAD_SND_8M("spp100.u21", 0x080000, CRC(17fee0f9) SHA1(5b5ceb667f3bc9bde4ea08a1ef837e3b56c01977))
	ROM_LOAD_SND_8M("spp100.u36", 0x100000, CRC(ffb957b0) SHA1(d6876ec63525099a7073c196867c17111272c69a))
	ROM_LOAD_SND_8M("spp100.u37", 0x180000, CRC(0738e1fc) SHA1(268462c06e5c1f286e5faaee1c0815448cc2eafa))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(simpprti)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("spp-cpu.500", 0x00000, 0x20000, CRC(215ce09c) SHA1(f3baaaa1b9f12a98109da55746031eb9f5f8790c))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("sppdispi.500", 0x00000, 0x80000, CRC(eefe84db) SHA1(97c60f9182bdfe346ca4981b844a71f57414d470))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("spp101.u7",0x0000,0x10000,CRC(32efcdf6) SHA1(1d437e8649408be91e0dd10598cc67336203077f))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("spp100.u17", 0x000000, CRC(65e9344e) SHA1(fe4797ccb71b31aa39d6a5d373a01fc22f9d055c))
	ROM_LOAD_SND_8M("spp100.u21", 0x080000, CRC(17fee0f9) SHA1(5b5ceb667f3bc9bde4ea08a1ef837e3b56c01977))
	ROM_LOAD_SND_8M("spp100.u36", 0x100000, CRC(ffb957b0) SHA1(d6876ec63525099a7073c196867c17111272c69a))
	ROM_LOAD_SND_8M("spp100.u37", 0x180000, CRC(0738e1fc) SHA1(268462c06e5c1f286e5faaee1c0815448cc2eafa))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(simpprti_400)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "spp-cpu.400",  0x00000, 0x20000, CRC(530b9782) SHA1(573b20cac205b7989cdefceb2c31cb7d88c2951a))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "sppdspi.400",   0x00000, 0x80000, CRC(ebe45dee) SHA1(4cdf0f01b1df1fa35df67f19c67b82a39d887be8))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("spp101.u7",0x0000,0x10000,CRC(32efcdf6) SHA1(1d437e8649408be91e0dd10598cc67336203077f))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("spp100.u17", 0x000000, CRC(65e9344e) SHA1(fe4797ccb71b31aa39d6a5d373a01fc22f9d055c))
	ROM_LOAD_SND_8M("spp100.u21", 0x080000, CRC(17fee0f9) SHA1(5b5ceb667f3bc9bde4ea08a1ef837e3b56c01977))
	ROM_LOAD_SND_8M("spp100.u36", 0x100000, CRC(ffb957b0) SHA1(d6876ec63525099a7073c196867c17111272c69a))
	ROM_LOAD_SND_8M("spp100.u37", 0x180000, CRC(0738e1fc) SHA1(268462c06e5c1f286e5faaee1c0815448cc2eafa))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(simpprti_204)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("spp-cpu.204",  0x00000, 0x20000, CRC(5bc155f7) SHA1(78d793cecbc6561a891ff8007f33c63ec5515e9f))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("sppdispi.201", 0x00000, 0x80000, CRC(b4594819) SHA1(4ab83f3b6466eebdec802e57d6542ad4a3cf3fb0))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("spp101.u7",0x0000,0x10000,CRC(32efcdf6) SHA1(1d437e8649408be91e0dd10598cc67336203077f))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("spp100.u17", 0x000000, CRC(65e9344e) SHA1(fe4797ccb71b31aa39d6a5d373a01fc22f9d055c))
	ROM_LOAD_SND_8M("spp100.u21", 0x080000, CRC(17fee0f9) SHA1(5b5ceb667f3bc9bde4ea08a1ef837e3b56c01977))
	ROM_LOAD_SND_8M("spp100.u36", 0x100000, CRC(ffb957b0) SHA1(d6876ec63525099a7073c196867c17111272c69a))
	ROM_LOAD_SND_8M("spp100.u37", 0x180000, CRC(0738e1fc) SHA1(268462c06e5c1f286e5faaee1c0815448cc2eafa))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(simpprty_300)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("spp-cpu.300",  0x00000, 0x20000, CRC(d9e02665) SHA1(12875c845c12b6676aa0af7c717fdf074156d938))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("sppdspa.300", 0x00000, 0x80000, CRC(57c4f297) SHA1(91ae894293b1252213a7137400f89c7ac2c6e877))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("spp101.u7",0x0000,0x10000,CRC(32efcdf6) SHA1(1d437e8649408be91e0dd10598cc67336203077f))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("spp100.u17", 0x000000, CRC(65e9344e) SHA1(fe4797ccb71b31aa39d6a5d373a01fc22f9d055c))
	ROM_LOAD_SND_8M("spp100.u21", 0x080000, CRC(17fee0f9) SHA1(5b5ceb667f3bc9bde4ea08a1ef837e3b56c01977))
	ROM_LOAD_SND_8M("spp100.u36", 0x100000, CRC(ffb957b0) SHA1(d6876ec63525099a7073c196867c17111272c69a))
	ROM_LOAD_SND_8M("spp100.u37", 0x180000, CRC(0738e1fc) SHA1(268462c06e5c1f286e5faaee1c0815448cc2eafa))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(simpprtl_300)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("spp-cpu.300",  0x00000, 0x20000, CRC(d9e02665) SHA1(12875c845c12b6676aa0af7c717fdf074156d938))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("sppdspl.300", 0x00000, 0x80000, CRC(d91ec782) SHA1(a01ebecb03200738b47177b02a689148d822ff0e))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("spp101.u7",0x0000,0x10000,CRC(32efcdf6) SHA1(1d437e8649408be91e0dd10598cc67336203077f))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("spp100.u17", 0x000000, CRC(65e9344e) SHA1(fe4797ccb71b31aa39d6a5d373a01fc22f9d055c))
	ROM_LOAD_SND_8M("spp100.u21", 0x080000, CRC(17fee0f9) SHA1(5b5ceb667f3bc9bde4ea08a1ef837e3b56c01977))
	ROM_LOAD_SND_8M("spp100.u36", 0x100000, CRC(ffb957b0) SHA1(d6876ec63525099a7073c196867c17111272c69a))
	ROM_LOAD_SND_8M("spp100.u37", 0x180000, CRC(0738e1fc) SHA1(268462c06e5c1f286e5faaee1c0815448cc2eafa))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(simpprtf_300)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("spp-cpu.300",  0x00000, 0x20000, CRC(d9e02665) SHA1(12875c845c12b6676aa0af7c717fdf074156d938))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("sppdspf.300", 0x00000, 0x80000, CRC(cb848e0d) SHA1(ab9f32d3b693ebcef92fe21e04d760756c8f59c2))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("spp101.u7",0x0000,0x10000,CRC(32efcdf6) SHA1(1d437e8649408be91e0dd10598cc67336203077f))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("spp100.u17", 0x000000, CRC(65e9344e) SHA1(fe4797ccb71b31aa39d6a5d373a01fc22f9d055c))
	ROM_LOAD_SND_8M("spp100.u21", 0x080000, CRC(17fee0f9) SHA1(5b5ceb667f3bc9bde4ea08a1ef837e3b56c01977))
	ROM_LOAD_SND_8M("spp100.u36", 0x100000, CRC(ffb957b0) SHA1(d6876ec63525099a7073c196867c17111272c69a))
	ROM_LOAD_SND_8M("spp100.u37", 0x180000, CRC(0738e1fc) SHA1(268462c06e5c1f286e5faaee1c0815448cc2eafa))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(simpprti_300)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("spp-cpu.300",  0x00000, 0x20000, CRC(d9e02665) SHA1(12875c845c12b6676aa0af7c717fdf074156d938))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("sppdspi.300", 0x00000, 0x80000, CRC(31acf30a) SHA1(aad2b363bed93d22613b0530fcd2d7f850f8e616))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("spp101.u7",0x0000,0x10000,CRC(32efcdf6) SHA1(1d437e8649408be91e0dd10598cc67336203077f))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD_SND_8M("spp100.u17", 0x000000, CRC(65e9344e) SHA1(fe4797ccb71b31aa39d6a5d373a01fc22f9d055c))
	ROM_LOAD_SND_8M("spp100.u21", 0x080000, CRC(17fee0f9) SHA1(5b5ceb667f3bc9bde4ea08a1ef837e3b56c01977))
	ROM_LOAD_SND_8M("spp100.u36", 0x100000, CRC(ffb957b0) SHA1(d6876ec63525099a7073c196867c17111272c69a))
	ROM_LOAD_SND_8M("spp100.u37", 0x180000, CRC(0738e1fc) SHA1(268462c06e5c1f286e5faaee1c0815448cc2eafa))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

/*-------------------------------------------------------------------
/ X-Files
/-------------------------------------------------------------------*/
ROM_START(xfilesp)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("xfcpu.303", 0x00000, 0x20000, CRC(c7ab5efe) SHA1(dcb4b89adfb5ba39e59c1118a00b29941d3ea4e9))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("xfildspa.300", 0x00000, 0x80000, CRC(03c96af8) SHA1(06a26116f863bb9b2d127e18c5ba500537923d62))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("xfsndu7.512", 0x0000, 0x10000, CRC(01d65239) SHA1(9e680de940a15ef85a5615b789c58cd5973ff11b))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("xfsndu17.c40", 0x000000, 0x80000, CRC(40bfc835) SHA1(2d6ac82acbbf9645bcb84fab7f285f2373e516a8))
	ROM_LOAD("xfsndu21.c40", 0x080000, 0x80000, CRC(b56a5ca6) SHA1(5fa23a8bb57e45aca159882226e603d9a6be078b))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(xfilespf)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("xfcpu.303", 0x00000, 0x20000, CRC(c7ab5efe) SHA1(dcb4b89adfb5ba39e59c1118a00b29941d3ea4e9))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("xfildspf.300", 0x00000, 0x80000, CRC(fe9b1292) SHA1(ead40d2cdff060829008f468e08512c4f5f9e055))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("xfsndu7.512", 0x0000, 0x10000, CRC(01d65239) SHA1(9e680de940a15ef85a5615b789c58cd5973ff11b))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("xfsndu17.c40", 0x000000, 0x80000, CRC(40bfc835) SHA1(2d6ac82acbbf9645bcb84fab7f285f2373e516a8))
	ROM_LOAD("xfsndu21.c40", 0x080000, 0x80000, CRC(b56a5ca6) SHA1(5fa23a8bb57e45aca159882226e603d9a6be078b))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(xfiles2)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("xfcpu.204", 0x00000, 0x20000, CRC(a4913128) SHA1(1fe348725e13fd5dc56b6b2dbd173d0b49953483))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("xfildspa.201", 0x00000, 0x80000, CRC(bb015f24) SHA1(ca539d978ef0b8244227ea0c60087da5e7f0ee9e))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("xfsndu7.512", 0x0000, 0x10000, CRC(01d65239) SHA1(9e680de940a15ef85a5615b789c58cd5973ff11b))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("xfsndu17.c40", 0x000000, 0x80000, CRC(40bfc835) SHA1(2d6ac82acbbf9645bcb84fab7f285f2373e516a8))
	ROM_LOAD("xfsndu21.c40", 0x080000, 0x80000, CRC(b56a5ca6) SHA1(5fa23a8bb57e45aca159882226e603d9a6be078b))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(xfiles20)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("xfcpu.200", 0x00000, 0x20000, CRC(fd9e8ae8) SHA1(7f904eaae437bf938f01e9df875b9415167fc4c5))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("xfildspa.200", 0x00000, 0x80000, CRC(3fb161c3) SHA1(ea00c5c5a1e4908fcc34b0558b89325db091595d))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("xfsndu7.512", 0x0000, 0x10000, CRC(01d65239) SHA1(9e680de940a15ef85a5615b789c58cd5973ff11b))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("xfsndu17.c40", 0x000000, 0x80000, CRC(40bfc835) SHA1(2d6ac82acbbf9645bcb84fab7f285f2373e516a8))
	ROM_LOAD("xfsndu21.c40", 0x080000, 0x80000, CRC(b56a5ca6) SHA1(5fa23a8bb57e45aca159882226e603d9a6be078b))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

/*-------------------------------------------------------------------
/ Twister
/-------------------------------------------------------------------*/
ROM_START(twst_405)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("twstcpu.405", 0x00000, 0x20000, CRC(8c3ea1a8) SHA1(d495b7dc79186d442a89b6382a6dc1c83e64ef95))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("twstdspa.400", 0x00000, 0x80000, CRC(a6a3d41d) SHA1(ad42b3390ceeeea43c1cd47f300bcd4b4a4d2558))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("twstsnd.u7", 0x0000, 0x10000, CRC(5ccf0798) SHA1(ac591c508de8e9687c20b01c298084c99a251016))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("twstsnd.u17", 0x000000, 0x80000, CRC(0e35d640) SHA1(ce38a03fcc321cd9af07d24bf7aa35f254b629fc))
	ROM_LOAD("twstsnd.u21", 0x080000, 0x80000, CRC(c3eae590) SHA1(bda3e0a725339069c49c4282676a07b4e0e8d2eb))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(twst_404)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("twstcpu.404", 0x00000, 0x20000, CRC(43ac7f8b) SHA1(fc087b741c00baa9093dfec009440a7d64f4ca65))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("twstdspa.400", 0x00000, 0x80000, CRC(a6a3d41d) SHA1(ad42b3390ceeeea43c1cd47f300bcd4b4a4d2558))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("twstsnd.u7", 0x0000, 0x10000, CRC(5ccf0798) SHA1(ac591c508de8e9687c20b01c298084c99a251016))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("twstsnd.u17", 0x000000, 0x80000, CRC(0e35d640) SHA1(ce38a03fcc321cd9af07d24bf7aa35f254b629fc))
	ROM_LOAD("twstsnd.u21", 0x080000, 0x80000, CRC(c3eae590) SHA1(bda3e0a725339069c49c4282676a07b4e0e8d2eb))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(twst_300)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("twstcpu.300", 0x00000, 0x20000, CRC(5cc057d4) SHA1(9ff4b6951a974e3013edc30ba2310c3bffb224d2))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("twstdspa.301", 0x00000, 0x80000, CRC(78bc45cb) SHA1(d1915fab46f178c9842e44701c91a0db2495e4fd))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("twstsnd.u7", 0x0000, 0x10000, CRC(5ccf0798) SHA1(ac591c508de8e9687c20b01c298084c99a251016))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("twstsnd.u17", 0x000000, 0x80000, CRC(0e35d640) SHA1(ce38a03fcc321cd9af07d24bf7aa35f254b629fc))
	ROM_LOAD("twstsnd.u21", 0x080000, 0x80000, CRC(c3eae590) SHA1(bda3e0a725339069c49c4282676a07b4e0e8d2eb))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

/*-------------------------------------------------------------------
/ Viper Night Drivin'
/-------------------------------------------------------------------*/
ROM_START(viprsega)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("vipcpu.201", 0x00000, 0x20000, CRC(476557aa) SHA1(9018bb6850a3f4b37cc05d9c3ce6e2d9a1931bfd))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("vipdspa.201", 0x00000, 0x80000, CRC(24b1dc21) SHA1(73d92083c4795e143e7c34f52032292a142534f4))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("vpru7.dat", 0x0000, 0x10000, CRC(f21617d7) SHA1(78d1ade400b83c62bb6288bccf386ef34050dd04))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("vpru17.dat", 0x000000, 0x80000, CRC(47b1317c) SHA1(32259965b5a12f63267af96eef8396bf71895a65))
	ROM_LOAD("vpru21.dat", 0x080000, 0x80000, CRC(0e0e2dd6) SHA1(b409c837a52eb399c9a4896ca0c502360c93dcc9))
	ROM_LOAD("vpru36.dat", 0x100000, 0x80000, CRC(7b482876) SHA1(c8960c2d45a77a35d22408c7bb8ba322e7af36f0))
	ROM_LOAD("vpru37.dat", 0x180000, 0x80000, CRC(0bf23e0e) SHA1(b5724ed6cfe791320a8cf208cc20a2d3f0db85c8))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(viprsega_102)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("vipcpu.102", 0x00000, 0x20000, CRC(6046974b) SHA1(56e3de5ccb5a04d6ee5555ee6755835e75e7454f))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("vipdspa.100", 0x00000, 0x80000, CRC(25acf3db) SHA1(3476f2b95cfff9dfb4fe9cf7c5cccae85f23343a))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("vpru7.dat", 0x0000, 0x10000, CRC(f21617d7) SHA1(78d1ade400b83c62bb6288bccf386ef34050dd04))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("vpru17.dat", 0x000000, 0x80000, CRC(47b1317c) SHA1(32259965b5a12f63267af96eef8396bf71895a65))
	ROM_LOAD("vpru21.dat", 0x080000, 0x80000, CRC(0e0e2dd6) SHA1(b409c837a52eb399c9a4896ca0c502360c93dcc9))
	ROM_LOAD("vpru36.dat", 0x100000, 0x80000, CRC(7b482876) SHA1(c8960c2d45a77a35d22408c7bb8ba322e7af36f0))
	ROM_LOAD("vpru37.dat", 0x180000, 0x80000, CRC(0bf23e0e) SHA1(b5724ed6cfe791320a8cf208cc20a2d3f0db85c8))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

/*-------------------------------------------------------------
/ Cut The Cheese Deluxe (Redemption)
/------------------------------------------------------------*/
ROM_START(ctchzdlx)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("ctcdxcpu.100", 0x00000, 0x20000, CRC(faad6656) SHA1(4d868bc31f35e848424e3bb66cb87efe0cf24eca))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("ctcdxdsp.100", 0x00000, 0x80000, CRC(de61b12e) SHA1(2ef8f02ca995e67d1feebd33306f92e885077101))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("ctcu7d.bin", 0x0000, 0x10000, CRC(92bfe454) SHA1(8182f7ac84addf8bdb7976a85c801edf3424d16b))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("ctcdxu17.bin", 0x000000, 0x80000, CRC(7ee35d17) SHA1(f2c9b70285926fc782a2e1289532395cd8dbf999))
	ROM_LOAD("ctcdxu21.bin", 0x080000, 0x80000, CRC(84dd40ac) SHA1(c9327b95f1730a3aa741540c28078f214af214b8))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

/*-------------------------------------------------------------------
/ Wack-A-Doodle-Doo (Redemption)
/-------------------------------------------------------------------*/
ROM_START(wackadoo)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("wackcpu.100", 0x00000, 0x20000, CRC(995e0219) SHA1(57b2352a5a96e71ff48f838fde87a158afbf3701))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("wackdisp.bin", 0x00000, 0x80000, CRC(8b46f074) SHA1(38a98fff56186d1a152e2c2e321496fd24e9ad56))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("wacku7.bin", 0x0000, 0x10000, CRC(99df3022) SHA1(5a38cf0816b95ea399d659d724b3259fadb42a62))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("wacku17.bin", 0x000000, 0x80000, CRC(1876e659) SHA1(1ff3a6d7a7b2582d0482eae024efa078d0a61dbe))
	ROM_LOAD("wacku21.bin", 0x080000, 0x80000, CRC(a3207296) SHA1(19af8ebab298a353fe42fc297b61182d8f772347))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

/*-------------------------------------------------------------------
/ Titanic (Coin dropper)
/-------------------------------------------------------------------*/
ROM_START(titanic)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("titacpu.101", 0x00000, 0x20000, CRC(4217becf) SHA1(8b7aacbe75717f13623f6ceaa4ba2de61b1b732a))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("titadspa.101", 0x00000, 0x80000, CRC(5b4300b6) SHA1(c2b2de20f6c74b71d2a9ac43c17694eadd795586))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("titau7.101", 0x0000, 0x10000, CRC(544fe1ac) SHA1(5c62eef6a42660b13e626d1a6bb8cd09b77b0cc1))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("titau17.100", 0x000000, 0x80000, CRC(ab919e99) SHA1(8735b632a62d9cd3db26b3c832785c48552ba231))
	ROM_LOAD("titau21.100", 0x080000, 0x80000, CRC(76ca05f8) SHA1(3e1c56fe37393c345111665fd8ab730d53cb6970))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

/*-------------------------------------------------------------------
/ Monopoly (Coin dropper)
/-------------------------------------------------------------------*/
ROM_START(monopred)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("monopcpu.401", 0x00000, 0x20000, CRC(2d0ff130) SHA1(c0b7baa6973db8743186a950b0a1ad3383db5c98))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("monopdsp.400", 0x00000, 0x80000, CRC(0105572b) SHA1(fd9e1dfa4f396b953f82ccde11b54ee638382bee))

	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("monopred.u7", 0x0000, 0x10000, CRC(1ca0cf63) SHA1(c4ce78718e3e3f1a8451b134f2869dd6410fee30))

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD_SND_8M("monopred.u17", 0x000000, CRC(467dca62) SHA1(c727748b6b0b39ead19ce98bddd89fd05fb62d00))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

/*-------------------------------------------------------------------
/ Elvis
/-------------------------------------------------------------------*/
ROM_START(elvisp)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("elvscpua.500", 0x00000, 0x20000, CRC(aae9d65d) SHA1(d6e789f7257448e97697c406561c14a0abc45187))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("elvsdspa.500", 0x00000, 0x80000, CRC(76a672cb) SHA1(8a9d4ac9538f0f91f2e95800147478cbaeb152a5))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios1a.u8", 0x400000, 0x20000, CRC(e2e3fc18) SHA1(5ce564828c1d814b680788f20ba8b134249ccfb9))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("elvis.u7",0x0000,0x10000,CRC(1df6c1b5) SHA1(7a9ebfc555e54ce92ad140ac6fcb82d9848ad8a6))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("elvis.u17", 0x000000, 0x100000, CRC(ff032897) SHA1(bf347c26a450bc07cdc94fc582dedf3a0cdc2a1b))
	ROM_LOAD("elvis.u21", 0x100000, 0x100000, CRC(c3c19a40) SHA1(97f7f36eed62ca389c770bf5d746721724e17250))
	ROM_LOAD("elvis.u36", 0x200000, 0x100000, CRC(e98f0dc9) SHA1(6dbab09435e993fef97d6a80a73675723bea7c1d))
	ROM_LOAD("elvis.u37", 0x300000, 0x100000, CRC(88ba0966) SHA1(43ea198c9fcdc1c396d4180308042c6c08311829))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(elvisp4)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "elvscpua.400",  0x0000, 0x20000, CRC(385be9ed) SHA1(c25b54a9b52bece17de9c394c019a4d2649bcf47))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "elvsdspa.401",  0x0000, 0x80000, CRC(fb08d5c3) SHA1(03f9886381d32bfd012a2663d2b7331a76c8c1c0))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios1a.u8", 0x400000, 0x20000, CRC(e2e3fc18) SHA1(5ce564828c1d814b680788f20ba8b134249ccfb9))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("elvis.u7",0x0000,0x10000,CRC(1df6c1b5) SHA1(7a9ebfc555e54ce92ad140ac6fcb82d9848ad8a6))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("elvis.u17", 0x000000, 0x100000, CRC(ff032897) SHA1(bf347c26a450bc07cdc94fc582dedf3a0cdc2a1b))
	ROM_LOAD("elvis.u21", 0x100000, 0x100000, CRC(c3c19a40) SHA1(97f7f36eed62ca389c770bf5d746721724e17250))
	ROM_LOAD("elvis.u36", 0x200000, 0x100000, CRC(e98f0dc9) SHA1(6dbab09435e993fef97d6a80a73675723bea7c1d))
	ROM_LOAD("elvis.u37", 0x300000, 0x100000, CRC(88ba0966) SHA1(43ea198c9fcdc1c396d4180308042c6c08311829))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(elvisp303)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "elvscpua.303",  0x0000, 0x20000, CRC(a0dd77d8) SHA1(2882eed805c2eb3cabadcfe51997a534ddac9050))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "elvsdspa.302",  0x0000, 0x80000, CRC(892da6d2) SHA1(66a2f9faab9c7b925a90455ce7e1d31e19fce99e))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios1a.u8", 0x400000, 0x20000, CRC(e2e3fc18) SHA1(5ce564828c1d814b680788f20ba8b134249ccfb9))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("elvis.u7",0x0000,0x10000,CRC(1df6c1b5) SHA1(7a9ebfc555e54ce92ad140ac6fcb82d9848ad8a6))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("elvis.u17", 0x000000, 0x100000, CRC(ff032897) SHA1(bf347c26a450bc07cdc94fc582dedf3a0cdc2a1b))
	ROM_LOAD("elvis.u21", 0x100000, 0x100000, CRC(c3c19a40) SHA1(97f7f36eed62ca389c770bf5d746721724e17250))
	ROM_LOAD("elvis.u36", 0x200000, 0x100000, CRC(e98f0dc9) SHA1(6dbab09435e993fef97d6a80a73675723bea7c1d))
	ROM_LOAD("elvis.u37", 0x300000, 0x100000, CRC(88ba0966) SHA1(43ea198c9fcdc1c396d4180308042c6c08311829))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(elvisp302)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "elvscpua.302",  0x0000, 0x20000, CRC(52fd7068) SHA1(548568aeb30a17541b07489dfecde9d4d63bf82b))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "elvsdspa.302",  0x0000, 0x80000, CRC(892da6d2) SHA1(66a2f9faab9c7b925a90455ce7e1d31e19fce99e))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios1a.u8", 0x400000, 0x20000, CRC(e2e3fc18) SHA1(5ce564828c1d814b680788f20ba8b134249ccfb9))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("elvis.u7",0x0000,0x10000,CRC(1df6c1b5) SHA1(7a9ebfc555e54ce92ad140ac6fcb82d9848ad8a6))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("elvis.u17", 0x000000, 0x100000, CRC(ff032897) SHA1(bf347c26a450bc07cdc94fc582dedf3a0cdc2a1b))
	ROM_LOAD("elvis.u21", 0x100000, 0x100000, CRC(c3c19a40) SHA1(97f7f36eed62ca389c770bf5d746721724e17250))
	ROM_LOAD("elvis.u36", 0x200000, 0x100000, CRC(e98f0dc9) SHA1(6dbab09435e993fef97d6a80a73675723bea7c1d))
	ROM_LOAD("elvis.u37", 0x300000, 0x100000, CRC(88ba0966) SHA1(43ea198c9fcdc1c396d4180308042c6c08311829))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(elvisp100)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "elvis_cpu_1.00.bin",  0x0000, 0x20000, CRC(5ca9aadc) SHA1(563248af88a851e9eac115c34a80e107b1dba20e))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "elvis_display_1.00.bin",  0x0000, 0x80000, CRC(8feffc70) SHA1(71849701d49064c65adfdf76908f9fa79a8b25b8))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios1a.u8", 0x400000, 0x20000, CRC(e2e3fc18) SHA1(5ce564828c1d814b680788f20ba8b134249ccfb9))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("elvis.u7",0x0000,0x10000,CRC(1df6c1b5) SHA1(7a9ebfc555e54ce92ad140ac6fcb82d9848ad8a6))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("elvis.u17", 0x000000, 0x100000, CRC(ff032897) SHA1(bf347c26a450bc07cdc94fc582dedf3a0cdc2a1b))
	ROM_LOAD("elvis.u21", 0x100000, 0x100000, CRC(c3c19a40) SHA1(97f7f36eed62ca389c770bf5d746721724e17250))
	ROM_LOAD("elvis.u36", 0x200000, 0x100000, CRC(e98f0dc9) SHA1(6dbab09435e993fef97d6a80a73675723bea7c1d))
	ROM_LOAD("elvis.u37", 0x300000, 0x100000, CRC(88ba0966) SHA1(43ea198c9fcdc1c396d4180308042c6c08311829))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(elvisl)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("elvscpul.500", 0x00000, 0x20000, CRC(d6813d67) SHA1(863a1b5bc62eca218f64d9bae24b205e1a8e2b6c))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("elvsdspl.500", 0x00000, 0x80000, CRC(68946b3b) SHA1(5764a5f6779097acfcf82eb176f6d966f6bb6988))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios1a.u8", 0x400000, 0x20000, CRC(e2e3fc18) SHA1(5ce564828c1d814b680788f20ba8b134249ccfb9))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("elvisl.u7",0x0000,0x10000,CRC(f0d70ee6) SHA1(9fa2c9d7b3690ec0c17645be066496d6833da5d1))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("elvisl.u17", 0x000000, 0x100000, CRC(2f86bcda) SHA1(73972fd30e84a2f97478f076cc8771c501440be5))
	ROM_LOAD("elvisl.u21", 0x100000, 0x100000, CRC(400c7174) SHA1(a4fa0d51b7c11e70f6b93068a6bf859cdf3359c3))
	ROM_LOAD("elvisl.u36", 0x200000, 0x100000, CRC(01ebbdbe) SHA1(286fa471b20b6ffcb0114d66239ab6aebe9bca9d))
	ROM_LOAD("elvisl.u37", 0x300000, 0x100000, CRC(bed26746) SHA1(385cb77ec7599b12a4b021c53b42b8e9b9fb08a8))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(elvisl4)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "elvscpul.400",  0x0000, 0x20000, CRC(536d4e3c) SHA1(3470ffdc32dbce9142dab82c25915d617e1429e6))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "elvsdspl.401",  0x0000, 0x80000, CRC(57fc6ed2) SHA1(3349a56ce38ab4ffdf1469bac8dbc9d08c077f6e))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios1a.u8", 0x400000, 0x20000, CRC(e2e3fc18) SHA1(5ce564828c1d814b680788f20ba8b134249ccfb9))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("elvisl.u7",0x0000,0x10000,CRC(f0d70ee6) SHA1(9fa2c9d7b3690ec0c17645be066496d6833da5d1))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("elvisl.u17", 0x000000, 0x100000, CRC(2f86bcda) SHA1(73972fd30e84a2f97478f076cc8771c501440be5))
	ROM_LOAD("elvisl.u21", 0x100000, 0x100000, CRC(400c7174) SHA1(a4fa0d51b7c11e70f6b93068a6bf859cdf3359c3))
	ROM_LOAD("elvisl.u36", 0x200000, 0x100000, CRC(01ebbdbe) SHA1(286fa471b20b6ffcb0114d66239ab6aebe9bca9d))
	ROM_LOAD("elvisl.u37", 0x300000, 0x100000, CRC(bed26746) SHA1(385cb77ec7599b12a4b021c53b42b8e9b9fb08a8))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(elvisl303)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "elvscpul.303",  0x0000, 0x20000, CRC(691b9882) SHA1(fd8ceef9dbae6c788964d417ad1c61a4bb8e0d9b))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "elvsdspl.302",  0x0000, 0x80000, CRC(f75ea4cb) SHA1(aa351bb0912fd9dc93e9c95f96af2d31aaf03777))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios1a.u8", 0x400000, 0x20000, CRC(e2e3fc18) SHA1(5ce564828c1d814b680788f20ba8b134249ccfb9))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("elvisl.u7",0x0000,0x10000,CRC(f0d70ee6) SHA1(9fa2c9d7b3690ec0c17645be066496d6833da5d1))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("elvisl.u17", 0x000000, 0x100000, CRC(2f86bcda) SHA1(73972fd30e84a2f97478f076cc8771c501440be5))
	ROM_LOAD("elvisl.u21", 0x100000, 0x100000, CRC(400c7174) SHA1(a4fa0d51b7c11e70f6b93068a6bf859cdf3359c3))
	ROM_LOAD("elvisl.u36", 0x200000, 0x100000, CRC(01ebbdbe) SHA1(286fa471b20b6ffcb0114d66239ab6aebe9bca9d))
	ROM_LOAD("elvisl.u37", 0x300000, 0x100000, CRC(bed26746) SHA1(385cb77ec7599b12a4b021c53b42b8e9b9fb08a8))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(elvisl302)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "elvscpul.302",  0x0000, 0x20000, CRC(0c6f1897) SHA1(ac71f833d4227c2d2d665a169eb0d12c73aeab04))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "elvsdspl.302",  0x0000, 0x80000, CRC(f75ea4cb) SHA1(aa351bb0912fd9dc93e9c95f96af2d31aaf03777))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios1a.u8", 0x400000, 0x20000, CRC(e2e3fc18) SHA1(5ce564828c1d814b680788f20ba8b134249ccfb9))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("elvisl.u7",0x0000,0x10000,CRC(f0d70ee6) SHA1(9fa2c9d7b3690ec0c17645be066496d6833da5d1))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("elvisl.u17", 0x000000, 0x100000, CRC(2f86bcda) SHA1(73972fd30e84a2f97478f076cc8771c501440be5))
	ROM_LOAD("elvisl.u21", 0x100000, 0x100000, CRC(400c7174) SHA1(a4fa0d51b7c11e70f6b93068a6bf859cdf3359c3))
	ROM_LOAD("elvisl.u36", 0x200000, 0x100000, CRC(01ebbdbe) SHA1(286fa471b20b6ffcb0114d66239ab6aebe9bca9d))
	ROM_LOAD("elvisl.u37", 0x300000, 0x100000, CRC(bed26746) SHA1(385cb77ec7599b12a4b021c53b42b8e9b9fb08a8))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(elvisg)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("elvscpug.500", 0x00000, 0x20000, CRC(1582dd3a) SHA1(9a5d044dbad03e3ec2358ef16d983195761cb17b))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("elvsdspg.500", 0x00000, 0x80000, CRC(4b6e7d37) SHA1(259a5d0d11392f05504d4477cf03f2a270db670c))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios1a.u8", 0x400000, 0x20000, CRC(e2e3fc18) SHA1(5ce564828c1d814b680788f20ba8b134249ccfb9))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("elvisg.u7",0x0000,0x10000,CRC(1085bd7c) SHA1(2c34ee7d7c44906b0894c0c01b0fad74cb0d2a32))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("elvisg.u17", 0x000000, 0x100000, CRC(8b888d75) SHA1(b8c654d0fb558c205c338be2b458cbf931b23bac))
	ROM_LOAD("elvisg.u21", 0x100000, 0x100000, CRC(79955b60) SHA1(36ad9e487408c9fd26641d484490b1b3bc8e1194))
	ROM_LOAD("elvisg.u36", 0x200000, 0x100000, CRC(25ba1ad4) SHA1(1e1a846af4ff43bb47f081b0cc179cd732c0bbea))
	ROM_LOAD("elvisg.u37", 0x300000, 0x100000, CRC(f6d7a2a0) SHA1(54c160a298c7ead1fe0404bce51bc16211da82cf))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(elvisg4)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "elvscpug.400",  0x0000, 0x20000, CRC(c5992b96) SHA1(0e8c285b76374daad17017994cd9d8e68a98ba42))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "elvsdspg.401",  0x0000, 0x80000, CRC(a694f642) SHA1(a44febcc08445f53aa96b0c791a636d541cebb97))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios1a.u8", 0x400000, 0x20000, CRC(e2e3fc18) SHA1(5ce564828c1d814b680788f20ba8b134249ccfb9))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("elvisg.u7",0x0000,0x10000,CRC(1085bd7c) SHA1(2c34ee7d7c44906b0894c0c01b0fad74cb0d2a32))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("elvisg.u17", 0x000000, 0x100000, CRC(8b888d75) SHA1(b8c654d0fb558c205c338be2b458cbf931b23bac))
	ROM_LOAD("elvisg.u21", 0x100000, 0x100000, CRC(79955b60) SHA1(36ad9e487408c9fd26641d484490b1b3bc8e1194))
	ROM_LOAD("elvisg.u36", 0x200000, 0x100000, CRC(25ba1ad4) SHA1(1e1a846af4ff43bb47f081b0cc179cd732c0bbea))
	ROM_LOAD("elvisg.u37", 0x300000, 0x100000, CRC(f6d7a2a0) SHA1(54c160a298c7ead1fe0404bce51bc16211da82cf))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(elvisg303)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "elvscpug.303",  0x0000, 0x20000, CRC(66b50538) SHA1(2612c0618c1d438632ff56b3b779214cf6534ff8))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "elvsdspg.302",  0x0000, 0x80000, CRC(6340bb11) SHA1(d510f1a913cd3fb9593ef88c5652e03a5d3c3ebb))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios1a.u8", 0x400000, 0x20000, CRC(e2e3fc18) SHA1(5ce564828c1d814b680788f20ba8b134249ccfb9))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("elvisg.u7",0x0000,0x10000,CRC(1085bd7c) SHA1(2c34ee7d7c44906b0894c0c01b0fad74cb0d2a32))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("elvisg.u17", 0x000000, 0x100000, CRC(8b888d75) SHA1(b8c654d0fb558c205c338be2b458cbf931b23bac))
	ROM_LOAD("elvisg.u21", 0x100000, 0x100000, CRC(79955b60) SHA1(36ad9e487408c9fd26641d484490b1b3bc8e1194))
	ROM_LOAD("elvisg.u36", 0x200000, 0x100000, CRC(25ba1ad4) SHA1(1e1a846af4ff43bb47f081b0cc179cd732c0bbea))
	ROM_LOAD("elvisg.u37", 0x300000, 0x100000, CRC(f6d7a2a0) SHA1(54c160a298c7ead1fe0404bce51bc16211da82cf))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(elvisg302)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "elvscpug.302",  0x0000, 0x20000, CRC(9d907782) SHA1(3bad8d0429029b0055d09e50af16bca4da724e0f))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "elvsdspg.302",  0x0000, 0x80000, CRC(6340bb11) SHA1(d510f1a913cd3fb9593ef88c5652e03a5d3c3ebb))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios1a.u8", 0x400000, 0x20000, CRC(e2e3fc18) SHA1(5ce564828c1d814b680788f20ba8b134249ccfb9))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("elvisg.u7",0x0000,0x10000,CRC(1085bd7c) SHA1(2c34ee7d7c44906b0894c0c01b0fad74cb0d2a32))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("elvisg.u17", 0x000000, 0x100000, CRC(8b888d75) SHA1(b8c654d0fb558c205c338be2b458cbf931b23bac))
	ROM_LOAD("elvisg.u21", 0x100000, 0x100000, CRC(79955b60) SHA1(36ad9e487408c9fd26641d484490b1b3bc8e1194))
	ROM_LOAD("elvisg.u36", 0x200000, 0x100000, CRC(25ba1ad4) SHA1(1e1a846af4ff43bb47f081b0cc179cd732c0bbea))
	ROM_LOAD("elvisg.u37", 0x300000, 0x100000, CRC(f6d7a2a0) SHA1(54c160a298c7ead1fe0404bce51bc16211da82cf))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(elvisf)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("elvscpuf.500", 0x00000, 0x20000, CRC(e977fdb0) SHA1(aa24b4e6c461188b330bbc01204af8bfb03a9abf))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("elvsdspf.500", 0x00000, 0x80000, CRC(e4ce2da7) SHA1(030d9200844fc47d5ea6c4afeab0851de5b42b23))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios1a.u8", 0x400000, 0x20000, CRC(e2e3fc18) SHA1(5ce564828c1d814b680788f20ba8b134249ccfb9))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("elvisf.u7",0x0000,0x10000,CRC(84a057cd) SHA1(70e626f13a164df184dc5b0c79e8d320eeafb13b))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("elvisf.u17", 0x000000, 0x100000, CRC(9b13e40d) SHA1(7e7eac1be5cbc7bde4296d168a1cc0716bcb293a))
	ROM_LOAD("elvisf.u21", 0x100000, 0x100000, CRC(5b668b6d) SHA1(9b104af5df5cc21c2504760b119f3e6584a1871b))
	ROM_LOAD("elvisf.u36", 0x200000, 0x100000, CRC(03ee0c04) SHA1(45a994589e3d9e6fe971db8722848b5f7432b675))
	ROM_LOAD("elvisf.u37", 0x300000, 0x100000, CRC(aa265440) SHA1(36b13ef0be4203936d9816e521098e72d6b4e4c1))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(elvisf4)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "elvscpuf.400",  0x0000, 0x20000, CRC(9ae4f3c4) SHA1(5ec1590ca54f36565953467226a85491a96d2d3c))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "elvsdspf.401",  0x0000, 0x80000, CRC(1ddd1823) SHA1(86c2bacb596265a28a4b6dce3cd47ed5a2cf74d7))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios1a.u8", 0x400000, 0x20000, CRC(e2e3fc18) SHA1(5ce564828c1d814b680788f20ba8b134249ccfb9))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("elvisf.u7",0x0000,0x10000,CRC(84a057cd) SHA1(70e626f13a164df184dc5b0c79e8d320eeafb13b))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("elvisf.u17", 0x000000, 0x100000, CRC(9b13e40d) SHA1(7e7eac1be5cbc7bde4296d168a1cc0716bcb293a))
	ROM_LOAD("elvisf.u21", 0x100000, 0x100000, CRC(5b668b6d) SHA1(9b104af5df5cc21c2504760b119f3e6584a1871b))
	ROM_LOAD("elvisf.u36", 0x200000, 0x100000, CRC(03ee0c04) SHA1(45a994589e3d9e6fe971db8722848b5f7432b675))
	ROM_LOAD("elvisf.u37", 0x300000, 0x100000, CRC(aa265440) SHA1(36b13ef0be4203936d9816e521098e72d6b4e4c1))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(elvisf303)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "elvscpuf.303",  0x0000, 0x20000, CRC(bc5cc2b9) SHA1(f434164384153a3cca358af55ed82c7757e74fd9))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "elvsdspf.302",  0x0000, 0x80000, CRC(410b6ae5) SHA1(ea29e1c81695df25ad61deedd84e6c3159976797))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios1a.u8", 0x400000, 0x20000, CRC(e2e3fc18) SHA1(5ce564828c1d814b680788f20ba8b134249ccfb9))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("elvisf.u7",0x0000,0x10000,CRC(84a057cd) SHA1(70e626f13a164df184dc5b0c79e8d320eeafb13b))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("elvisf.u17", 0x000000, 0x100000, CRC(9b13e40d) SHA1(7e7eac1be5cbc7bde4296d168a1cc0716bcb293a))
	ROM_LOAD("elvisf.u21", 0x100000, 0x100000, CRC(5b668b6d) SHA1(9b104af5df5cc21c2504760b119f3e6584a1871b))
	ROM_LOAD("elvisf.u36", 0x200000, 0x100000, CRC(03ee0c04) SHA1(45a994589e3d9e6fe971db8722848b5f7432b675))
	ROM_LOAD("elvisf.u37", 0x300000, 0x100000, CRC(aa265440) SHA1(36b13ef0be4203936d9816e521098e72d6b4e4c1))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(elvisf302)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "elvscpuf.302",  0x0000, 0x20000, CRC(893a5ac6) SHA1(4be45cf036cdd643f10ecbb4d0b1778d25cf778a))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "elvsdspf.302",  0x0000, 0x80000, CRC(410b6ae5) SHA1(ea29e1c81695df25ad61deedd84e6c3159976797))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios1a.u8", 0x400000, 0x20000, CRC(e2e3fc18) SHA1(5ce564828c1d814b680788f20ba8b134249ccfb9))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("elvisf.u7",0x0000,0x10000,CRC(84a057cd) SHA1(70e626f13a164df184dc5b0c79e8d320eeafb13b))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("elvisf.u17", 0x000000, 0x100000, CRC(9b13e40d) SHA1(7e7eac1be5cbc7bde4296d168a1cc0716bcb293a))
	ROM_LOAD("elvisf.u21", 0x100000, 0x100000, CRC(5b668b6d) SHA1(9b104af5df5cc21c2504760b119f3e6584a1871b))
	ROM_LOAD("elvisf.u36", 0x200000, 0x100000, CRC(03ee0c04) SHA1(45a994589e3d9e6fe971db8722848b5f7432b675))
	ROM_LOAD("elvisf.u37", 0x300000, 0x100000, CRC(aa265440) SHA1(36b13ef0be4203936d9816e521098e72d6b4e4c1))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(elvisi)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("elvscpui.500", 0x00000, 0x20000, CRC(11bafdbb) SHA1(51036cdf4beda20f2680fff7cd8cae25219406bd))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("elvsdspi.500", 0x00000, 0x80000, CRC(3ecbadb2) SHA1(589cf6de348359944585b718b2289dd70676807a))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios1a.u8", 0x400000, 0x20000, CRC(e2e3fc18) SHA1(5ce564828c1d814b680788f20ba8b134249ccfb9))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("elvisi.u7",0x0000,0x10000,CRC(8c270da4) SHA1(6a21332fdd1f2714aa78a1730e0f90159022ad1c))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("elvisi.u17", 0x000000, 0x100000, CRC(bd2e6580) SHA1(dc8c974860498d5766dbb0881cc9d6866c9a98a1))
	ROM_LOAD("elvisi.u21", 0x100000, 0x100000, CRC(1932a22b) SHA1(864d6bc2c60e763431fd19511dc1a946cf131d63))
	ROM_LOAD("elvisi.u36", 0x200000, 0x100000, CRC(df6772d7) SHA1(96e98ff4e93fc0c6fb2d9924da99b97f0c436c44))
	ROM_LOAD("elvisi.u37", 0x300000, 0x100000, CRC(990fd624) SHA1(d5e104485dc8dd7386d8f3e7d99dc6cf7bf91568))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(elvisi4)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "elvscpui.400",  0x0000, 0x20000, CRC(dd2876fc) SHA1(3b860ad176e3fda58be1f165fc2f9965f36259ef))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "elvsdspi.401",  0x0000, 0x80000, CRC(51f92cec) SHA1(a678dcb29f867b07641f39f5e0a9bf0e5daf9dc3))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios1a.u8", 0x400000, 0x20000, CRC(e2e3fc18) SHA1(5ce564828c1d814b680788f20ba8b134249ccfb9))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("elvisi.u7",0x0000,0x10000,CRC(8c270da4) SHA1(6a21332fdd1f2714aa78a1730e0f90159022ad1c))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("elvisi.u17", 0x000000, 0x100000, CRC(bd2e6580) SHA1(dc8c974860498d5766dbb0881cc9d6866c9a98a1))
	ROM_LOAD("elvisi.u21", 0x100000, 0x100000, CRC(1932a22b) SHA1(864d6bc2c60e763431fd19511dc1a946cf131d63))
	ROM_LOAD("elvisi.u36", 0x200000, 0x100000, CRC(df6772d7) SHA1(96e98ff4e93fc0c6fb2d9924da99b97f0c436c44))
	ROM_LOAD("elvisi.u37", 0x300000, 0x100000, CRC(990fd624) SHA1(d5e104485dc8dd7386d8f3e7d99dc6cf7bf91568))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(elvisi303)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "elvscpui.303",  0x0000, 0x20000, CRC(11f47b7a) SHA1(4fbe64ed49719408b77ebf6871bb2211e03de394))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "elvsdspi.302",  0x0000, 0x80000, CRC(217c7d17) SHA1(bfd67e876ea85847212c936f9f8477aba8a7b573))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios1a.u8", 0x400000, 0x20000, CRC(e2e3fc18) SHA1(5ce564828c1d814b680788f20ba8b134249ccfb9))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("elvisi.u7",0x0000,0x10000,CRC(8c270da4) SHA1(6a21332fdd1f2714aa78a1730e0f90159022ad1c))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("elvisi.u17", 0x000000, 0x100000, CRC(bd2e6580) SHA1(dc8c974860498d5766dbb0881cc9d6866c9a98a1))
	ROM_LOAD("elvisi.u21", 0x100000, 0x100000, CRC(1932a22b) SHA1(864d6bc2c60e763431fd19511dc1a946cf131d63))
	ROM_LOAD("elvisi.u36", 0x200000, 0x100000, CRC(df6772d7) SHA1(96e98ff4e93fc0c6fb2d9924da99b97f0c436c44))
	ROM_LOAD("elvisi.u37", 0x300000, 0x100000, CRC(990fd624) SHA1(d5e104485dc8dd7386d8f3e7d99dc6cf7bf91568))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(elvisi302)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "elvscpui.302",  0x0000, 0x20000, CRC(a2c15460) SHA1(dcd4d4b61745ab2ccba662f9d72f56774a830ea0))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "elvsdspi.302",  0x0000, 0x80000, CRC(217c7d17) SHA1(bfd67e876ea85847212c936f9f8477aba8a7b573))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios1a.u8", 0x400000, 0x20000, CRC(e2e3fc18) SHA1(5ce564828c1d814b680788f20ba8b134249ccfb9))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("elvisi.u7",0x0000,0x10000,CRC(8c270da4) SHA1(6a21332fdd1f2714aa78a1730e0f90159022ad1c))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("elvisi.u17", 0x000000, 0x100000, CRC(bd2e6580) SHA1(dc8c974860498d5766dbb0881cc9d6866c9a98a1))
	ROM_LOAD("elvisi.u21", 0x100000, 0x100000, CRC(1932a22b) SHA1(864d6bc2c60e763431fd19511dc1a946cf131d63))
	ROM_LOAD("elvisi.u36", 0x200000, 0x100000, CRC(df6772d7) SHA1(96e98ff4e93fc0c6fb2d9924da99b97f0c436c44))
	ROM_LOAD("elvisi.u37", 0x300000, 0x100000, CRC(990fd624) SHA1(d5e104485dc8dd7386d8f3e7d99dc6cf7bf91568))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

/*-------------------------------------------------------------------
/ Grand Prix
/-------------------------------------------------------------------*/
ROM_START(gprix)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("gpcpua.450", 0x00000, 0x20000, CRC(3e5ae527) SHA1(27d50a0460b1733c6c857968b85da492fa2ad544))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("gpdspa.400", 0x00000, 0x80000, CRC(ce431306) SHA1(2573049b52b928052f196371dbc3a5236ce8cfc3))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("biosv8.u8", 0x400000, 0x20000, CRC(c81b5532) SHA1(7bd630179910423a01394f82ad7ff2008c02f7ff))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("gpsnda.u7",0x0000,0x10000,CRC(f784634f) SHA1(40847986003b01c9de5d9af4c66a0f1f9fb0cac8))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("gpsnda.u17", 0x000000, 0x100000, CRC(43dca7e2) SHA1(30726897950b168ffa5e0e8a4ff12856fd50f132))
	ROM_LOAD("gpsnda.u21", 0x100000, 0x100000, CRC(77717142) SHA1(055f975c3d1cf6560908f5d8553f7e64580a2bba))
	ROM_LOAD("gpsnda.u36", 0x200000, 0x100000, CRC(6e414e19) SHA1(5b7c9da9c340ec3b55163f5674d72ab30ffbb866))
	ROM_LOAD("gpsnda.u37", 0x300000, 0x100000, CRC(caf4c3f3) SHA1(ebdbaccf951ef6525f0fafa7e23d8140ef6b84e5))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(gprix_400)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "gpcpua.400",    0x0000, 0x20000, CRC(c6042517) SHA1(19729a86a3afafb516f000489c38d00379e4f85c))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("gpdspa.400", 0x00000, 0x80000, CRC(ce431306) SHA1(2573049b52b928052f196371dbc3a5236ce8cfc3))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("biosv8.u8", 0x400000, 0x20000, CRC(c81b5532) SHA1(7bd630179910423a01394f82ad7ff2008c02f7ff))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("gpsnda.u7",0x0000,0x10000,CRC(f784634f) SHA1(40847986003b01c9de5d9af4c66a0f1f9fb0cac8))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("gpsnda.u17", 0x000000, 0x100000, CRC(43dca7e2) SHA1(30726897950b168ffa5e0e8a4ff12856fd50f132))
	ROM_LOAD("gpsnda.u21", 0x100000, 0x100000, CRC(77717142) SHA1(055f975c3d1cf6560908f5d8553f7e64580a2bba))
	ROM_LOAD("gpsnda.u36", 0x200000, 0x100000, CRC(6e414e19) SHA1(5b7c9da9c340ec3b55163f5674d72ab30ffbb866))
	ROM_LOAD("gpsnda.u37", 0x300000, 0x100000, CRC(caf4c3f3) SHA1(ebdbaccf951ef6525f0fafa7e23d8140ef6b84e5))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(gprix_352)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "gpcpua.352", 0x0000, 0x20000, CRC(da46b437) SHA1(07cf288d47e1447c015e5bb1fd85df654bde71ef))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "gpdspa.303",    0x0000, 0x80000, CRC(814f6a50) SHA1(727eac96c4beaafc3ddd9ccd9ef098bd557cbc74))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("biosv8.u8", 0x400000, 0x20000, CRC(c81b5532) SHA1(7bd630179910423a01394f82ad7ff2008c02f7ff))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("gpsnda.u7",0x0000,0x10000,CRC(f784634f) SHA1(40847986003b01c9de5d9af4c66a0f1f9fb0cac8))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("gpsnda.u17", 0x000000, 0x100000, CRC(43dca7e2) SHA1(30726897950b168ffa5e0e8a4ff12856fd50f132))
	ROM_LOAD("gpsnda.u21", 0x100000, 0x100000, CRC(77717142) SHA1(055f975c3d1cf6560908f5d8553f7e64580a2bba))
	ROM_LOAD("gpsnda.u36", 0x200000, 0x100000, CRC(6e414e19) SHA1(5b7c9da9c340ec3b55163f5674d72ab30ffbb866))
	ROM_LOAD("gpsnda.u37", 0x300000, 0x100000, CRC(caf4c3f3) SHA1(ebdbaccf951ef6525f0fafa7e23d8140ef6b84e5))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(gprix_350)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "gpcpua.350", 0x0000, 0x20000, CRC(bd47be96) SHA1(31b7adc5cec10d18dd551fdba94fbdb8c6eac01b))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "gpdspa.303",    0x0000, 0x80000, CRC(814f6a50) SHA1(727eac96c4beaafc3ddd9ccd9ef098bd557cbc74))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("biosv8.u8", 0x400000, 0x20000, CRC(c81b5532) SHA1(7bd630179910423a01394f82ad7ff2008c02f7ff))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("gpsnda.u7",0x0000,0x10000,CRC(f784634f) SHA1(40847986003b01c9de5d9af4c66a0f1f9fb0cac8))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("gpsnda.u17", 0x000000, 0x100000, CRC(43dca7e2) SHA1(30726897950b168ffa5e0e8a4ff12856fd50f132))
	ROM_LOAD("gpsnda.u21", 0x100000, 0x100000, CRC(77717142) SHA1(055f975c3d1cf6560908f5d8553f7e64580a2bba))
	ROM_LOAD("gpsnda.u36", 0x200000, 0x100000, CRC(6e414e19) SHA1(5b7c9da9c340ec3b55163f5674d72ab30ffbb866))
	ROM_LOAD("gpsnda.u37", 0x300000, 0x100000, CRC(caf4c3f3) SHA1(ebdbaccf951ef6525f0fafa7e23d8140ef6b84e5))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(gprix_340)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "gpcpua.340",    0x0000, 0x20000, CRC(798f2ab3) SHA1(046cbbd0115511b2cbd7f132b0755d03edce1e7b))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "gpdspa.303",    0x0000, 0x80000, CRC(814f6a50) SHA1(727eac96c4beaafc3ddd9ccd9ef098bd557cbc74))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("biosv8.u8", 0x400000, 0x20000, CRC(c81b5532) SHA1(7bd630179910423a01394f82ad7ff2008c02f7ff))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("gpsnda.u7",0x0000,0x10000,CRC(f784634f) SHA1(40847986003b01c9de5d9af4c66a0f1f9fb0cac8))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("gpsnda.u17", 0x000000, 0x100000, CRC(43dca7e2) SHA1(30726897950b168ffa5e0e8a4ff12856fd50f132))
	ROM_LOAD("gpsnda.u21", 0x100000, 0x100000, CRC(77717142) SHA1(055f975c3d1cf6560908f5d8553f7e64580a2bba))
	ROM_LOAD("gpsnda.u36", 0x200000, 0x100000, CRC(6e414e19) SHA1(5b7c9da9c340ec3b55163f5674d72ab30ffbb866))
	ROM_LOAD("gpsnda.u37", 0x300000, 0x100000, CRC(caf4c3f3) SHA1(ebdbaccf951ef6525f0fafa7e23d8140ef6b84e5))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(gprix_301)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "gpcpua.301",    0x0000, 0x20000, CRC(07cdb3eb) SHA1(2246d253dca93ce8c5f6775352611a3145ab8776))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "gpdspa.301",    0x0000, 0x80000, CRC(b11d752d) SHA1(c0e6f5544a3061027bf9addef4363c744aaaf736))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("biosv8.u8", 0x400000, 0x20000, CRC(c81b5532) SHA1(7bd630179910423a01394f82ad7ff2008c02f7ff))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("gpsnda.u7",0x0000,0x10000,CRC(f784634f) SHA1(40847986003b01c9de5d9af4c66a0f1f9fb0cac8))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("gpsnda.u17", 0x000000, 0x100000, CRC(43dca7e2) SHA1(30726897950b168ffa5e0e8a4ff12856fd50f132))
	ROM_LOAD("gpsnda.u21", 0x100000, 0x100000, CRC(77717142) SHA1(055f975c3d1cf6560908f5d8553f7e64580a2bba))
	ROM_LOAD("gpsnda.u36", 0x200000, 0x100000, CRC(6e414e19) SHA1(5b7c9da9c340ec3b55163f5674d72ab30ffbb866))
	ROM_LOAD("gpsnda.u37", 0x300000, 0x100000, CRC(caf4c3f3) SHA1(ebdbaccf951ef6525f0fafa7e23d8140ef6b84e5))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(gprixg)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("gpcpug.450", 0x00000, 0x20000, CRC(d803128b) SHA1(f914f75f4ec38dcbd2e40818fe8cb0ad446c59bf))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("gpdspg.400", 0x00000, 0x80000, CRC(b3f64332) SHA1(84e1b094c74b2dfae8e3cd3ce3f1cd20dc400fd7))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("biosv8.u8", 0x400000, 0x20000, CRC(c81b5532) SHA1(7bd630179910423a01394f82ad7ff2008c02f7ff))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("gpsndg.u7",0x0000,0x10000,CRC(95129e03) SHA1(5fddd9d8213f9f1f68fe9e96c9e78dc6771fab21))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("gpsndg.u17", 0x000000, 0x100000, CRC(27ef97ea) SHA1(8ba941d5d4f929b8ec3222f1c91452395e2f690f))
	ROM_LOAD("gpsndg.u21", 0x100000, 0x100000, CRC(71391d71) SHA1(690b280710c79d94fc271541066ae90e462bbce2))
	ROM_LOAD("gpsndg.u36", 0x200000, 0x100000, CRC(415baa1b) SHA1(cca21e0e5ef0cbe34c9514d72a06fc129990787a))
	ROM_LOAD("gpsndg.u37", 0x300000, 0x100000, CRC(e4a6ae7f) SHA1(4a4cd973f90c13ced07459c8f457314c8280dd6a))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(gprixg_400)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "gpcpug.400",    0x0000, 0x20000, CRC(98d69588) SHA1(9d248b00a1aa966e69816cd7aaf869737e7a1ca7))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("gpdspg.400", 0x00000, 0x80000, CRC(b3f64332) SHA1(84e1b094c74b2dfae8e3cd3ce3f1cd20dc400fd7))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("biosv8.u8", 0x400000, 0x20000, CRC(c81b5532) SHA1(7bd630179910423a01394f82ad7ff2008c02f7ff))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("gpsndg.u7",0x0000,0x10000,CRC(95129e03) SHA1(5fddd9d8213f9f1f68fe9e96c9e78dc6771fab21))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("gpsndg.u17", 0x000000, 0x100000, CRC(27ef97ea) SHA1(8ba941d5d4f929b8ec3222f1c91452395e2f690f))
	ROM_LOAD("gpsndg.u21", 0x100000, 0x100000, CRC(71391d71) SHA1(690b280710c79d94fc271541066ae90e462bbce2))
	ROM_LOAD("gpsndg.u36", 0x200000, 0x100000, CRC(415baa1b) SHA1(cca21e0e5ef0cbe34c9514d72a06fc129990787a))
	ROM_LOAD("gpsndg.u37", 0x300000, 0x100000, CRC(e4a6ae7f) SHA1(4a4cd973f90c13ced07459c8f457314c8280dd6a))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(gprixg_352)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "gpcpug.352", 0x0000, 0x20000, CRC(b6c15f62) SHA1(b8757e7ef7064d200c7965902b624c8ef947f23d))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "gpdspg.303",    0x0000, 0x80000, CRC(0be9eb1d) SHA1(78c402efcc818e4960ef5ca17e7fa43a028b5c9b))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("biosv8.u8", 0x400000, 0x20000, CRC(c81b5532) SHA1(7bd630179910423a01394f82ad7ff2008c02f7ff))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("gpsndg.u7",0x0000,0x10000,CRC(95129e03) SHA1(5fddd9d8213f9f1f68fe9e96c9e78dc6771fab21))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("gpsndg.u17", 0x000000, 0x100000, CRC(27ef97ea) SHA1(8ba941d5d4f929b8ec3222f1c91452395e2f690f))
	ROM_LOAD("gpsndg.u21", 0x100000, 0x100000, CRC(71391d71) SHA1(690b280710c79d94fc271541066ae90e462bbce2))
	ROM_LOAD("gpsndg.u36", 0x200000, 0x100000, CRC(415baa1b) SHA1(cca21e0e5ef0cbe34c9514d72a06fc129990787a))
	ROM_LOAD("gpsndg.u37", 0x300000, 0x100000, CRC(e4a6ae7f) SHA1(4a4cd973f90c13ced07459c8f457314c8280dd6a))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(gprixg_350)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "gpcpug.350", 0x0000, 0x20000, CRC(8e77c953) SHA1(07d23ce2f0b0a2dc6284b71d1a4a8d1bb5dab6d0))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "gpdspg.303",    0x0000, 0x80000, CRC(0be9eb1d) SHA1(78c402efcc818e4960ef5ca17e7fa43a028b5c9b))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("biosv8.u8", 0x400000, 0x20000, CRC(c81b5532) SHA1(7bd630179910423a01394f82ad7ff2008c02f7ff))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("gpsndg.u7",0x0000,0x10000,CRC(95129e03) SHA1(5fddd9d8213f9f1f68fe9e96c9e78dc6771fab21))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("gpsndg.u17", 0x000000, 0x100000, CRC(27ef97ea) SHA1(8ba941d5d4f929b8ec3222f1c91452395e2f690f))
	ROM_LOAD("gpsndg.u21", 0x100000, 0x100000, CRC(71391d71) SHA1(690b280710c79d94fc271541066ae90e462bbce2))
	ROM_LOAD("gpsndg.u36", 0x200000, 0x100000, CRC(415baa1b) SHA1(cca21e0e5ef0cbe34c9514d72a06fc129990787a))
	ROM_LOAD("gpsndg.u37", 0x300000, 0x100000, CRC(e4a6ae7f) SHA1(4a4cd973f90c13ced07459c8f457314c8280dd6a))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(gprixg_340)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "gpcpug.340",    0x0000, 0x20000, CRC(34afe4e4) SHA1(0655be39f309f32dffca22d7ab780263da5e8cb2))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "gpdspg.303",    0x0000, 0x80000, CRC(0be9eb1d) SHA1(78c402efcc818e4960ef5ca17e7fa43a028b5c9b))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("biosv8.u8", 0x400000, 0x20000, CRC(c81b5532) SHA1(7bd630179910423a01394f82ad7ff2008c02f7ff))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("gpsndg.u7",0x0000,0x10000,CRC(95129e03) SHA1(5fddd9d8213f9f1f68fe9e96c9e78dc6771fab21))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("gpsndg.u17", 0x000000, 0x100000, CRC(27ef97ea) SHA1(8ba941d5d4f929b8ec3222f1c91452395e2f690f))
	ROM_LOAD("gpsndg.u21", 0x100000, 0x100000, CRC(71391d71) SHA1(690b280710c79d94fc271541066ae90e462bbce2))
	ROM_LOAD("gpsndg.u36", 0x200000, 0x100000, CRC(415baa1b) SHA1(cca21e0e5ef0cbe34c9514d72a06fc129990787a))
	ROM_LOAD("gpsndg.u37", 0x300000, 0x100000, CRC(e4a6ae7f) SHA1(4a4cd973f90c13ced07459c8f457314c8280dd6a))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(gprixg_301)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "gpcpug.301",    0x0000, 0x20000, CRC(23ac882e) SHA1(5194c3ea18a08f844f4ee293c9de44b62a956ee6))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "gpdspg.301",    0x0000, 0x80000, CRC(0d214a2a) SHA1(c7f9bbd56d7038931c8658bc586d29ad2b9ecac2))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("biosv8.u8", 0x400000, 0x20000, CRC(c81b5532) SHA1(7bd630179910423a01394f82ad7ff2008c02f7ff))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("gpsndg.u7",0x0000,0x10000,CRC(95129e03) SHA1(5fddd9d8213f9f1f68fe9e96c9e78dc6771fab21))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("gpsndg.u17", 0x000000, 0x100000, CRC(27ef97ea) SHA1(8ba941d5d4f929b8ec3222f1c91452395e2f690f))
	ROM_LOAD("gpsndg.u21", 0x100000, 0x100000, CRC(71391d71) SHA1(690b280710c79d94fc271541066ae90e462bbce2))
	ROM_LOAD("gpsndg.u36", 0x200000, 0x100000, CRC(415baa1b) SHA1(cca21e0e5ef0cbe34c9514d72a06fc129990787a))
	ROM_LOAD("gpsndg.u37", 0x300000, 0x100000, CRC(e4a6ae7f) SHA1(4a4cd973f90c13ced07459c8f457314c8280dd6a))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(gprixl)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("gpcpul.450", 0x00000, 0x20000, CRC(816bf4a4) SHA1(d5cca282e58d493be36400a7cd7dc4321d98f2f8))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("gpdspl.400", 0x00000, 0x80000, CRC(74d9aa40) SHA1(802c6fbe4248a516f18e4b69997254b3dcf27706))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("biosv8.u8", 0x400000, 0x20000, CRC(c81b5532) SHA1(7bd630179910423a01394f82ad7ff2008c02f7ff))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("gpsndl.u7",0x0000,0x10000,CRC(0640fe8f) SHA1(aa45bf89c4cae5b4c2143656cfe19fe8f1ec30a3))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("gpsndl.u17", 0x000000, 0x100000, CRC(2581ef04) SHA1(2d85040e355ed410c7d8348ef64fc2c8e76ec0f0))
	ROM_LOAD("gpsndl.u21", 0x100000, 0x100000, CRC(f4c97c9e) SHA1(ae04f416a7582efee20469ec686d02727558d850))
	ROM_LOAD("gpsndl.u36", 0x200000, 0x100000, CRC(863de01a) SHA1(3f1fd157c2abacdab072146499b64b9e0853fb3e))
	ROM_LOAD("gpsndl.u37", 0x300000, 0x100000, CRC(db16b68a) SHA1(815fdcd4ae01c6264133389ce3194da572e1c232))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(gprixl_400)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "gpcpul.400",    0x0000, 0x20000, CRC(90f506f0) SHA1(4642fa8bf15955a32b6ae8c6b859d94dcd40c542))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("gpdspl.400", 0x00000, 0x80000, CRC(74d9aa40) SHA1(802c6fbe4248a516f18e4b69997254b3dcf27706))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("biosv8.u8", 0x400000, 0x20000, CRC(c81b5532) SHA1(7bd630179910423a01394f82ad7ff2008c02f7ff))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("gpsndl.u7",0x0000,0x10000,CRC(0640fe8f) SHA1(aa45bf89c4cae5b4c2143656cfe19fe8f1ec30a3))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("gpsndl.u17", 0x000000, 0x100000, CRC(2581ef04) SHA1(2d85040e355ed410c7d8348ef64fc2c8e76ec0f0))
	ROM_LOAD("gpsndl.u21", 0x100000, 0x100000, CRC(f4c97c9e) SHA1(ae04f416a7582efee20469ec686d02727558d850))
	ROM_LOAD("gpsndl.u36", 0x200000, 0x100000, CRC(863de01a) SHA1(3f1fd157c2abacdab072146499b64b9e0853fb3e))
	ROM_LOAD("gpsndl.u37", 0x300000, 0x100000, CRC(db16b68a) SHA1(815fdcd4ae01c6264133389ce3194da572e1c232))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(gprixl_352)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "gpcpul.352", 0x0000, 0x20000, CRC(cd296661) SHA1(e706525b07e1e1278ab65a896616e63be52e8e73))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "gpdspl.303",    0x0000, 0x80000, CRC(82f30b13) SHA1(2f15228dbd6f3957f657772725f3280adf778d72))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("biosv8.u8", 0x400000, 0x20000, CRC(c81b5532) SHA1(7bd630179910423a01394f82ad7ff2008c02f7ff))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("gpsndl.u7",0x0000,0x10000,CRC(0640fe8f) SHA1(aa45bf89c4cae5b4c2143656cfe19fe8f1ec30a3))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("gpsndl.u17", 0x000000, 0x100000, CRC(2581ef04) SHA1(2d85040e355ed410c7d8348ef64fc2c8e76ec0f0))
	ROM_LOAD("gpsndl.u21", 0x100000, 0x100000, CRC(f4c97c9e) SHA1(ae04f416a7582efee20469ec686d02727558d850))
	ROM_LOAD("gpsndl.u36", 0x200000, 0x100000, CRC(863de01a) SHA1(3f1fd157c2abacdab072146499b64b9e0853fb3e))
	ROM_LOAD("gpsndl.u37", 0x300000, 0x100000, CRC(db16b68a) SHA1(815fdcd4ae01c6264133389ce3194da572e1c232))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(gprixl_350)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "gpcpul.350", 0x0000, 0x20000, CRC(714f0641) SHA1(c09aa248fe04fc3c569c6786c0db8d396cbd2403))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "gpdspl.303",    0x0000, 0x80000, CRC(82f30b13) SHA1(2f15228dbd6f3957f657772725f3280adf778d72))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("biosv8.u8", 0x400000, 0x20000, CRC(c81b5532) SHA1(7bd630179910423a01394f82ad7ff2008c02f7ff))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("gpsndl.u7",0x0000,0x10000,CRC(0640fe8f) SHA1(aa45bf89c4cae5b4c2143656cfe19fe8f1ec30a3))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("gpsndl.u17", 0x000000, 0x100000, CRC(2581ef04) SHA1(2d85040e355ed410c7d8348ef64fc2c8e76ec0f0))
	ROM_LOAD("gpsndl.u21", 0x100000, 0x100000, CRC(f4c97c9e) SHA1(ae04f416a7582efee20469ec686d02727558d850))
	ROM_LOAD("gpsndl.u36", 0x200000, 0x100000, CRC(863de01a) SHA1(3f1fd157c2abacdab072146499b64b9e0853fb3e))
	ROM_LOAD("gpsndl.u37", 0x300000, 0x100000, CRC(db16b68a) SHA1(815fdcd4ae01c6264133389ce3194da572e1c232))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(gprixl_340)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "gpcpul.340",    0x0000, 0x20000, CRC(2cd2f25f) SHA1(ee21a680cf56b6b415b2b9f5d89125062b24f8ae))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "gpdspl.303",    0x0000, 0x80000, CRC(82f30b13) SHA1(2f15228dbd6f3957f657772725f3280adf778d72))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("biosv8.u8", 0x400000, 0x20000, CRC(c81b5532) SHA1(7bd630179910423a01394f82ad7ff2008c02f7ff))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("gpsndl.u7",0x0000,0x10000,CRC(0640fe8f) SHA1(aa45bf89c4cae5b4c2143656cfe19fe8f1ec30a3))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("gpsndl.u17", 0x000000, 0x100000, CRC(2581ef04) SHA1(2d85040e355ed410c7d8348ef64fc2c8e76ec0f0))
	ROM_LOAD("gpsndl.u21", 0x100000, 0x100000, CRC(f4c97c9e) SHA1(ae04f416a7582efee20469ec686d02727558d850))
	ROM_LOAD("gpsndl.u36", 0x200000, 0x100000, CRC(863de01a) SHA1(3f1fd157c2abacdab072146499b64b9e0853fb3e))
	ROM_LOAD("gpsndl.u37", 0x300000, 0x100000, CRC(db16b68a) SHA1(815fdcd4ae01c6264133389ce3194da572e1c232))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(gprixl_301)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "gpcpul.301",    0x0000, 0x20000, CRC(ad739825) SHA1(c2fbc25985b83bacd4285b6608366de485f16982))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "gpdspl.301",    0x0000, 0x80000, CRC(2f483f0a) SHA1(42550741bee6af022bccd130626913edff6180a0))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("biosv8.u8", 0x400000, 0x20000, CRC(c81b5532) SHA1(7bd630179910423a01394f82ad7ff2008c02f7ff))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("gpsndl.u7",0x0000,0x10000,CRC(0640fe8f) SHA1(aa45bf89c4cae5b4c2143656cfe19fe8f1ec30a3))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("gpsndl.u17", 0x000000, 0x100000, CRC(2581ef04) SHA1(2d85040e355ed410c7d8348ef64fc2c8e76ec0f0))
	ROM_LOAD("gpsndl.u21", 0x100000, 0x100000, CRC(f4c97c9e) SHA1(ae04f416a7582efee20469ec686d02727558d850))
	ROM_LOAD("gpsndl.u36", 0x200000, 0x100000, CRC(863de01a) SHA1(3f1fd157c2abacdab072146499b64b9e0853fb3e))
	ROM_LOAD("gpsndl.u37", 0x300000, 0x100000, CRC(db16b68a) SHA1(815fdcd4ae01c6264133389ce3194da572e1c232))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(gprixf)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("gpcpuf.450", 0x00000, 0x20000, CRC(b14f7d20) SHA1(b91097490ee568e00be58f5dac184c8d47196adc))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("gpdspf.400", 0x00000, 0x80000, CRC(f9b1ef9a) SHA1(a7e3c0fc1526cf3632e6b1f22caf7f73749e77a6))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("biosv8.u8", 0x400000, 0x20000, CRC(c81b5532) SHA1(7bd630179910423a01394f82ad7ff2008c02f7ff))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("gpsndf.u7",0x0000,0x10000,CRC(9b34e55a) SHA1(670fe4e4b62c46266667f37c0341bb4266e55067))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("gpsndf.u17", 0x000000, 0x100000, CRC(18beb699) SHA1(a3bbd7c9fc1165da5e502e09f68321bd56992e76))
	ROM_LOAD("gpsndf.u21", 0x100000, 0x100000, CRC(b64702dd) SHA1(8762fb00d5549649444f7f85c3f6d72f27c6ba41))
	ROM_LOAD("gpsndf.u36", 0x200000, 0x100000, CRC(4e41f0bb) SHA1(4a25b472a9435c77712559d7ded1649dffbc885c))
	ROM_LOAD("gpsndf.u37", 0x300000, 0x100000, CRC(e6d96767) SHA1(a471d51796edad71eb21aadc4a26bb1529a0b9cc))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(gprixf_400)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "gpcpuf.400",    0x0000, 0x20000, CRC(0e7f8776) SHA1(0731c3a5350445f70dd8bdac68b2554942f12c8d))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("gpdspf.400", 0x00000, 0x80000, CRC(f9b1ef9a) SHA1(a7e3c0fc1526cf3632e6b1f22caf7f73749e77a6))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("biosv8.u8", 0x400000, 0x20000, CRC(c81b5532) SHA1(7bd630179910423a01394f82ad7ff2008c02f7ff))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("gpsndf.u7",0x0000,0x10000,CRC(9b34e55a) SHA1(670fe4e4b62c46266667f37c0341bb4266e55067))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("gpsndf.u17", 0x000000, 0x100000, CRC(18beb699) SHA1(a3bbd7c9fc1165da5e502e09f68321bd56992e76))
	ROM_LOAD("gpsndf.u21", 0x100000, 0x100000, CRC(b64702dd) SHA1(8762fb00d5549649444f7f85c3f6d72f27c6ba41))
	ROM_LOAD("gpsndf.u36", 0x200000, 0x100000, CRC(4e41f0bb) SHA1(4a25b472a9435c77712559d7ded1649dffbc885c))
	ROM_LOAD("gpsndf.u37", 0x300000, 0x100000, CRC(e6d96767) SHA1(a471d51796edad71eb21aadc4a26bb1529a0b9cc))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(gprixf_352)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "gpcpuf.352", 0x0000, 0x20000, CRC(78140cd3) SHA1(9c7257dede5c3bf78d9a9bbdf89bd01f12180c4f))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "gpdspf.303",    0x0000, 0x80000, CRC(f48f3a4b) SHA1(74c7d1670d6f1ed68d5aed5a755f27ffdb566cbd))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("biosv8.u8", 0x400000, 0x20000, CRC(c81b5532) SHA1(7bd630179910423a01394f82ad7ff2008c02f7ff))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("gpsndf.u7",0x0000,0x10000,CRC(9b34e55a) SHA1(670fe4e4b62c46266667f37c0341bb4266e55067))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("gpsndf.u17", 0x000000, 0x100000, CRC(18beb699) SHA1(a3bbd7c9fc1165da5e502e09f68321bd56992e76))
	ROM_LOAD("gpsndf.u21", 0x100000, 0x100000, CRC(b64702dd) SHA1(8762fb00d5549649444f7f85c3f6d72f27c6ba41))
	ROM_LOAD("gpsndf.u36", 0x200000, 0x100000, CRC(4e41f0bb) SHA1(4a25b472a9435c77712559d7ded1649dffbc885c))
	ROM_LOAD("gpsndf.u37", 0x300000, 0x100000, CRC(e6d96767) SHA1(a471d51796edad71eb21aadc4a26bb1529a0b9cc))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(gprixf_350)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "gpcpuf.350", 0x0000, 0x20000, CRC(ad4224e9) SHA1(781a76ef14e7abb2b57ae49cd8712ddace8a4fca))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "gpdspf.303",    0x0000, 0x80000, CRC(f48f3a4b) SHA1(74c7d1670d6f1ed68d5aed5a755f27ffdb566cbd))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("biosv8.u8", 0x400000, 0x20000, CRC(c81b5532) SHA1(7bd630179910423a01394f82ad7ff2008c02f7ff))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("gpsndf.u7",0x0000,0x10000,CRC(9b34e55a) SHA1(670fe4e4b62c46266667f37c0341bb4266e55067))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("gpsndf.u17", 0x000000, 0x100000, CRC(18beb699) SHA1(a3bbd7c9fc1165da5e502e09f68321bd56992e76))
	ROM_LOAD("gpsndf.u21", 0x100000, 0x100000, CRC(b64702dd) SHA1(8762fb00d5549649444f7f85c3f6d72f27c6ba41))
	ROM_LOAD("gpsndf.u36", 0x200000, 0x100000, CRC(4e41f0bb) SHA1(4a25b472a9435c77712559d7ded1649dffbc885c))
	ROM_LOAD("gpsndf.u37", 0x300000, 0x100000, CRC(e6d96767) SHA1(a471d51796edad71eb21aadc4a26bb1529a0b9cc))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(gprixf_340)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "gpcpuf.340",    0x0000, 0x20000, CRC(cefc30e4) SHA1(2ffdfd09ed8ba00a36a6bf12b79200c562b7dc0d))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "gpdspf.303",    0x0000, 0x80000, CRC(f48f3a4b) SHA1(74c7d1670d6f1ed68d5aed5a755f27ffdb566cbd))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("biosv8.u8", 0x400000, 0x20000, CRC(c81b5532) SHA1(7bd630179910423a01394f82ad7ff2008c02f7ff))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("gpsndf.u7",0x0000,0x10000,CRC(9b34e55a) SHA1(670fe4e4b62c46266667f37c0341bb4266e55067))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("gpsndf.u17", 0x000000, 0x100000, CRC(18beb699) SHA1(a3bbd7c9fc1165da5e502e09f68321bd56992e76))
	ROM_LOAD("gpsndf.u21", 0x100000, 0x100000, CRC(b64702dd) SHA1(8762fb00d5549649444f7f85c3f6d72f27c6ba41))
	ROM_LOAD("gpsndf.u36", 0x200000, 0x100000, CRC(4e41f0bb) SHA1(4a25b472a9435c77712559d7ded1649dffbc885c))
	ROM_LOAD("gpsndf.u37", 0x300000, 0x100000, CRC(e6d96767) SHA1(a471d51796edad71eb21aadc4a26bb1529a0b9cc))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(gprixf_301)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "gpcpuf.301",    0x0000, 0x20000, CRC(6f9d34ee) SHA1(a8b60300cc1e07d3beb8d607e24285dbbd871e83))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "gpdspf.301",    0x0000, 0x80000, CRC(b19729cd) SHA1(491fdf356f5a24b9895d2feccfe29d0bf45f4e27))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("biosv8.u8", 0x400000, 0x20000, CRC(c81b5532) SHA1(7bd630179910423a01394f82ad7ff2008c02f7ff))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("gpsndf.u7",0x0000,0x10000,CRC(9b34e55a) SHA1(670fe4e4b62c46266667f37c0341bb4266e55067))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("gpsndf.u17", 0x000000, 0x100000, CRC(18beb699) SHA1(a3bbd7c9fc1165da5e502e09f68321bd56992e76))
	ROM_LOAD("gpsndf.u21", 0x100000, 0x100000, CRC(b64702dd) SHA1(8762fb00d5549649444f7f85c3f6d72f27c6ba41))
	ROM_LOAD("gpsndf.u36", 0x200000, 0x100000, CRC(4e41f0bb) SHA1(4a25b472a9435c77712559d7ded1649dffbc885c))
	ROM_LOAD("gpsndf.u37", 0x300000, 0x100000, CRC(e6d96767) SHA1(a471d51796edad71eb21aadc4a26bb1529a0b9cc))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(gprixi)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("gpcpui.450", 0x00000, 0x20000, CRC(f18d8375) SHA1(b7e4e311623babc7b3c5d744122c88d45d77a33b))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("gpdspi.400", 0x00000, 0x80000, CRC(88675cdf) SHA1(b305a683350d38b43f2e3c9277af14d5503b3219))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("biosv8.u8", 0x400000, 0x20000, CRC(c81b5532) SHA1(7bd630179910423a01394f82ad7ff2008c02f7ff))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("gpsndi.u7",0x0000,0x10000,CRC(37d66e66) SHA1(219fd734d3a19407d9d47de198429c770d7d8856))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("gpsndi.u17", 0x000000, 0x100000, CRC(868b225d) SHA1(bc169cf5882002a1b58973a22a78d8dd4467bc51))
	ROM_LOAD("gpsndi.u21", 0x100000, 0x100000, CRC(b6692c39) SHA1(ac36ffb37ad945a857d5098547479c8cd62b6356))
	ROM_LOAD("gpsndi.u36", 0x200000, 0x100000, CRC(f8558b24) SHA1(ceb3880b026fb7fcc69eb8d94e33e30c56c24de8))
	ROM_LOAD("gpsndi.u37", 0x300000, 0x100000, CRC(a76c6682) SHA1(6d319a8f07c10fe392fc0b8e177cc6abbce0b536))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(gprixi_400)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "gpcpui.400",    0x0000, 0x20000, CRC(49e161b7) SHA1(855e35ddedf35055de384bade4b237810bc5ffec))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("gpdspi.400", 0x00000, 0x80000, CRC(88675cdf) SHA1(b305a683350d38b43f2e3c9277af14d5503b3219))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("biosv8.u8", 0x400000, 0x20000, CRC(c81b5532) SHA1(7bd630179910423a01394f82ad7ff2008c02f7ff))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("gpsndi.u7",0x0000,0x10000,CRC(37d66e66) SHA1(219fd734d3a19407d9d47de198429c770d7d8856))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("gpsndi.u17", 0x000000, 0x100000, CRC(868b225d) SHA1(bc169cf5882002a1b58973a22a78d8dd4467bc51))
	ROM_LOAD("gpsndi.u21", 0x100000, 0x100000, CRC(b6692c39) SHA1(ac36ffb37ad945a857d5098547479c8cd62b6356))
	ROM_LOAD("gpsndi.u36", 0x200000, 0x100000, CRC(f8558b24) SHA1(ceb3880b026fb7fcc69eb8d94e33e30c56c24de8))
	ROM_LOAD("gpsndi.u37", 0x300000, 0x100000, CRC(a76c6682) SHA1(6d319a8f07c10fe392fc0b8e177cc6abbce0b536))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(gprixi_352)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "gpcpui.352", 0x0000, 0x20000, CRC(941bd2a9) SHA1(11402004c5b57de5ec28ea2f4128b1852c205dac))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "gpdspi.303",    0x0000, 0x80000, CRC(36418722) SHA1(66f04e3069c51004cb82961a7d82ac0a5f6a84dd))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("biosv8.u8", 0x400000, 0x20000, CRC(c81b5532) SHA1(7bd630179910423a01394f82ad7ff2008c02f7ff))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("gpsndi.u7",0x0000,0x10000,CRC(37d66e66) SHA1(219fd734d3a19407d9d47de198429c770d7d8856))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("gpsndi.u17", 0x000000, 0x100000, CRC(868b225d) SHA1(bc169cf5882002a1b58973a22a78d8dd4467bc51))
	ROM_LOAD("gpsndi.u21", 0x100000, 0x100000, CRC(b6692c39) SHA1(ac36ffb37ad945a857d5098547479c8cd62b6356))
	ROM_LOAD("gpsndi.u36", 0x200000, 0x100000, CRC(f8558b24) SHA1(ceb3880b026fb7fcc69eb8d94e33e30c56c24de8))
	ROM_LOAD("gpsndi.u37", 0x300000, 0x100000, CRC(a76c6682) SHA1(6d319a8f07c10fe392fc0b8e177cc6abbce0b536))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(gprixi_350)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "gpcpui.350", 0x0000, 0x20000, CRC(64bc5f6b) SHA1(fb079323d3548f1915de93d724d3fb76b2e02f27))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "gpdspi.303",    0x0000, 0x80000, CRC(36418722) SHA1(66f04e3069c51004cb82961a7d82ac0a5f6a84dd))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("biosv8.u8", 0x400000, 0x20000, CRC(c81b5532) SHA1(7bd630179910423a01394f82ad7ff2008c02f7ff))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("gpsndi.u7",0x0000,0x10000,CRC(37d66e66) SHA1(219fd734d3a19407d9d47de198429c770d7d8856))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("gpsndi.u17", 0x000000, 0x100000, CRC(868b225d) SHA1(bc169cf5882002a1b58973a22a78d8dd4467bc51))
	ROM_LOAD("gpsndi.u21", 0x100000, 0x100000, CRC(b6692c39) SHA1(ac36ffb37ad945a857d5098547479c8cd62b6356))
	ROM_LOAD("gpsndi.u36", 0x200000, 0x100000, CRC(f8558b24) SHA1(ceb3880b026fb7fcc69eb8d94e33e30c56c24de8))
	ROM_LOAD("gpsndi.u37", 0x300000, 0x100000, CRC(a76c6682) SHA1(6d319a8f07c10fe392fc0b8e177cc6abbce0b536))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(gprixi_340)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "gpcpui.340",    0x0000, 0x20000, CRC(35f4a870) SHA1(24293fafcec6180ab62ef3298e4b53910e05a937))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "gpdspi.303",    0x0000, 0x80000, CRC(36418722) SHA1(66f04e3069c51004cb82961a7d82ac0a5f6a84dd))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("biosv8.u8", 0x400000, 0x20000, CRC(c81b5532) SHA1(7bd630179910423a01394f82ad7ff2008c02f7ff))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("gpsndi.u7",0x0000,0x10000,CRC(37d66e66) SHA1(219fd734d3a19407d9d47de198429c770d7d8856))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("gpsndi.u17", 0x000000, 0x100000, CRC(868b225d) SHA1(bc169cf5882002a1b58973a22a78d8dd4467bc51))
	ROM_LOAD("gpsndi.u21", 0x100000, 0x100000, CRC(b6692c39) SHA1(ac36ffb37ad945a857d5098547479c8cd62b6356))
	ROM_LOAD("gpsndi.u36", 0x200000, 0x100000, CRC(f8558b24) SHA1(ceb3880b026fb7fcc69eb8d94e33e30c56c24de8))
	ROM_LOAD("gpsndi.u37", 0x300000, 0x100000, CRC(a76c6682) SHA1(6d319a8f07c10fe392fc0b8e177cc6abbce0b536))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(gprixi_301)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "gpcpui.301",    0x0000, 0x20000, CRC(d9be9fd7) SHA1(3dfc997d8d17d153ee42df0adb7993293bfff7e8))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "gpdspi.301",    0x0000, 0x80000, CRC(1fc478da) SHA1(5307e9b302a7e49eb3460e8ba1e4c22525a1dcfe))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("biosv8.u8", 0x400000, 0x20000, CRC(c81b5532) SHA1(7bd630179910423a01394f82ad7ff2008c02f7ff))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("gpsndi.u7",0x0000,0x10000,CRC(37d66e66) SHA1(219fd734d3a19407d9d47de198429c770d7d8856))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("gpsndi.u17", 0x000000, 0x100000, CRC(868b225d) SHA1(bc169cf5882002a1b58973a22a78d8dd4467bc51))
	ROM_LOAD("gpsndi.u21", 0x100000, 0x100000, CRC(b6692c39) SHA1(ac36ffb37ad945a857d5098547479c8cd62b6356))
	ROM_LOAD("gpsndi.u36", 0x200000, 0x100000, CRC(f8558b24) SHA1(ceb3880b026fb7fcc69eb8d94e33e30c56c24de8))
	ROM_LOAD("gpsndi.u37", 0x300000, 0x100000, CRC(a76c6682) SHA1(6d319a8f07c10fe392fc0b8e177cc6abbce0b536))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

/*-------------------------------------------------------------------
/ Nascar
/-------------------------------------------------------------------*/
ROM_START(nascar)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("nascpua.450", 0x00000, 0x20000, CRC(da902e01) SHA1(afc6ace2b31c8682fb4d05e1b472c2ec30e7559b))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("nasdspa.400", 0x00000, 0x80000, CRC(364878bf) SHA1(a1fb477a37459a3583d3767386f87aa620e31e34))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("biosv8.u8", 0x400000, 0x20000, CRC(c81b5532) SHA1(7bd630179910423a01394f82ad7ff2008c02f7ff))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("nassnd.u7",0x0000,0x10000,CRC(3a3c8203) SHA1(c64c424c01ec91e2578fd6ddc5d3596b8a485c22))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("nassnd.u17", 0x000000, 0x100000, CRC(4dcf65fa) SHA1(bc745e16f1f4b92b97fd0536bea789909b9c0c67))
	ROM_LOAD("nassnd.u21", 0x100000, 0x100000, CRC(82ac1e4f) SHA1(8a6518885d89651df31afc8119d87a46fd802e16))
	ROM_LOAD("nassnd.u36", 0x200000, 0x100000, CRC(2385ada2) SHA1(d3b59beffe6817cc3ea1140698095886ec2f2324))
	ROM_LOAD("nassnd.u37", 0x300000, 0x100000, CRC(458ba148) SHA1(594fd9b48aa48ab7b3df921e689b1acba2b09d79))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(nascar_400)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "nascpua.400",   0x0000, 0x20000, CRC(24a72071) SHA1(5bfe473e85e12b30963b15dfc8732f2ef9c299c3))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("nasdspa.400", 0x00000, 0x80000, CRC(364878bf) SHA1(a1fb477a37459a3583d3767386f87aa620e31e34))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("biosv8.u8", 0x400000, 0x20000, CRC(c81b5532) SHA1(7bd630179910423a01394f82ad7ff2008c02f7ff))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("nassnd.u7",0x0000,0x10000,CRC(3a3c8203) SHA1(c64c424c01ec91e2578fd6ddc5d3596b8a485c22))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("nassnd.u17", 0x000000, 0x100000, CRC(4dcf65fa) SHA1(bc745e16f1f4b92b97fd0536bea789909b9c0c67))
	ROM_LOAD("nassnd.u21", 0x100000, 0x100000, CRC(82ac1e4f) SHA1(8a6518885d89651df31afc8119d87a46fd802e16))
	ROM_LOAD("nassnd.u36", 0x200000, 0x100000, CRC(2385ada2) SHA1(d3b59beffe6817cc3ea1140698095886ec2f2324))
	ROM_LOAD("nassnd.u37", 0x300000, 0x100000, CRC(458ba148) SHA1(594fd9b48aa48ab7b3df921e689b1acba2b09d79))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(nascar_352)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "nascpua.352", 0x0000, 0x20000, CRC(65b8132e) SHA1(b42dca3e68d3eff158bae830f6c8cca00e0ed3e2))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "nasdspa.303",   0x0000, 0x80000, CRC(86e20410) SHA1(c499682713facc6b2923fdd0eff47b98f6a36d14))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("biosv8.u8", 0x400000, 0x20000, CRC(c81b5532) SHA1(7bd630179910423a01394f82ad7ff2008c02f7ff))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("nassnd.u7",0x0000,0x10000,CRC(3a3c8203) SHA1(c64c424c01ec91e2578fd6ddc5d3596b8a485c22))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("nassnd.u17", 0x000000, 0x100000, CRC(4dcf65fa) SHA1(bc745e16f1f4b92b97fd0536bea789909b9c0c67))
	ROM_LOAD("nassnd.u21", 0x100000, 0x100000, CRC(82ac1e4f) SHA1(8a6518885d89651df31afc8119d87a46fd802e16))
	ROM_LOAD("nassnd.u36", 0x200000, 0x100000, CRC(2385ada2) SHA1(d3b59beffe6817cc3ea1140698095886ec2f2324))
	ROM_LOAD("nassnd.u37", 0x300000, 0x100000, CRC(458ba148) SHA1(594fd9b48aa48ab7b3df921e689b1acba2b09d79))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(nascar_350)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "nascpua.350",   0x0000, 0x20000, CRC(e5b4ed49) SHA1(0d38c0e08862a0a5a200225634c5bf0d0afe5afe))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "nasdspa.303",   0x0000, 0x80000, CRC(86e20410) SHA1(c499682713facc6b2923fdd0eff47b98f6a36d14))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("biosv8.u8", 0x400000, 0x20000, CRC(c81b5532) SHA1(7bd630179910423a01394f82ad7ff2008c02f7ff))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("nassnd.u7",0x0000,0x10000,CRC(3a3c8203) SHA1(c64c424c01ec91e2578fd6ddc5d3596b8a485c22))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("nassnd.u17", 0x000000, 0x100000, CRC(4dcf65fa) SHA1(bc745e16f1f4b92b97fd0536bea789909b9c0c67))
	ROM_LOAD("nassnd.u21", 0x100000, 0x100000, CRC(82ac1e4f) SHA1(8a6518885d89651df31afc8119d87a46fd802e16))
	ROM_LOAD("nassnd.u36", 0x200000, 0x100000, CRC(2385ada2) SHA1(d3b59beffe6817cc3ea1140698095886ec2f2324))
	ROM_LOAD("nassnd.u37", 0x300000, 0x100000, CRC(458ba148) SHA1(594fd9b48aa48ab7b3df921e689b1acba2b09d79))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(nascar_340)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "nascpua.340",   0x0000, 0x20000, CRC(120dc65a) SHA1(151c1604dacb1c1cf74449291d81629a05fb1b09))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "nasdspa.303",   0x0000, 0x80000, CRC(86e20410) SHA1(c499682713facc6b2923fdd0eff47b98f6a36d14))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("biosv8.u8", 0x400000, 0x20000, CRC(c81b5532) SHA1(7bd630179910423a01394f82ad7ff2008c02f7ff))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("nassnd.u7",0x0000,0x10000,CRC(3a3c8203) SHA1(c64c424c01ec91e2578fd6ddc5d3596b8a485c22))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("nassnd.u17", 0x000000, 0x100000, CRC(4dcf65fa) SHA1(bc745e16f1f4b92b97fd0536bea789909b9c0c67))
	ROM_LOAD("nassnd.u21", 0x100000, 0x100000, CRC(82ac1e4f) SHA1(8a6518885d89651df31afc8119d87a46fd802e16))
	ROM_LOAD("nassnd.u36", 0x200000, 0x100000, CRC(2385ada2) SHA1(d3b59beffe6817cc3ea1140698095886ec2f2324))
	ROM_LOAD("nassnd.u37", 0x300000, 0x100000, CRC(458ba148) SHA1(594fd9b48aa48ab7b3df921e689b1acba2b09d79))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(nascar_301)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "nascpua.301",   0x0000, 0x20000, CRC(8ede60c2) SHA1(aa49da40f2ed858c5fa260ce5e7dd096b4217544))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "nasdspa.301",   0x0000, 0x80000, CRC(4de3c8d5) SHA1(c2c08ddd0ecc511cf34ba6a6cae9968e903b88ad))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("biosv8.u8", 0x400000, 0x20000, CRC(c81b5532) SHA1(7bd630179910423a01394f82ad7ff2008c02f7ff))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("nassnd.u7",0x0000,0x10000,CRC(3a3c8203) SHA1(c64c424c01ec91e2578fd6ddc5d3596b8a485c22))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("nassnd.u17", 0x000000, 0x100000, CRC(4dcf65fa) SHA1(bc745e16f1f4b92b97fd0536bea789909b9c0c67))
	ROM_LOAD("nassnd.u21", 0x100000, 0x100000, CRC(82ac1e4f) SHA1(8a6518885d89651df31afc8119d87a46fd802e16))
	ROM_LOAD("nassnd.u36", 0x200000, 0x100000, CRC(2385ada2) SHA1(d3b59beffe6817cc3ea1140698095886ec2f2324))
	ROM_LOAD("nassnd.u37", 0x300000, 0x100000, CRC(458ba148) SHA1(594fd9b48aa48ab7b3df921e689b1acba2b09d79))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(dalejr)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "dalecpu.500",   0x0000, 0x20000, CRC(b723b7db) SHA1(ee5f96599f8ccb0fda0695e5e8af438c3f559df3))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "daledisp.500",   0x0000, 0x80000, CRC(5dad91cd) SHA1(ef9ce5573f580abc720a184625c96672b5337191))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("biosv8.u8", 0x400000, 0x20000, CRC(c81b5532) SHA1(7bd630179910423a01394f82ad7ff2008c02f7ff))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("nassnd.u7",0x0000,0x10000,CRC(3a3c8203) SHA1(c64c424c01ec91e2578fd6ddc5d3596b8a485c22))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("nassnd.u17", 0x000000, 0x100000, CRC(4dcf65fa) SHA1(bc745e16f1f4b92b97fd0536bea789909b9c0c67))
	ROM_LOAD("nassnd.u21", 0x100000, 0x100000, CRC(82ac1e4f) SHA1(8a6518885d89651df31afc8119d87a46fd802e16))
	ROM_LOAD("nassnd.u36", 0x200000, 0x100000, CRC(2385ada2) SHA1(d3b59beffe6817cc3ea1140698095886ec2f2324))
	ROM_LOAD("nassnd.u37", 0x300000, 0x100000, CRC(458ba148) SHA1(594fd9b48aa48ab7b3df921e689b1acba2b09d79))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(nascarl)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("nascpul.450", 0x00000, 0x20000, CRC(3eebae3f) SHA1(654f0e44ce009450e66250423fcf0ff4727e5ee1))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("nasdspl.400", 0x00000, 0x80000, CRC(a4de490f) SHA1(bc1aa9fc0182045f5d10044b3e4fa083572be4ac))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("biosv8.u8", 0x400000, 0x20000, CRC(c81b5532) SHA1(7bd630179910423a01394f82ad7ff2008c02f7ff))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("nascsp.u7",0x0000,0x10000,CRC(03a34394) SHA1(d1e3a1a8e14525c40e9f8a5441a106df662608f1))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("nassndl.u17", 0x000000, 0x100000, CRC(058c67ad) SHA1(70e22d8a1842108309f6c03dcc6ac23a822da3c3))
	ROM_LOAD("nassndl.u21", 0x100000, 0x100000, CRC(e34d3b6f) SHA1(63ef27ed5965d719215d0a469886d3852b6bffb6))
	ROM_LOAD("nassndl.u36", 0x200000, 0x100000, CRC(9e2658b1) SHA1(0d93a381a65f11022a1a6da5e5b0e4a0e779f336))
	ROM_LOAD("nassndl.u37", 0x300000, 0x100000, CRC(63f084ab) SHA1(519807bf6e868df6f756ad30af2f6636804f167c))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(nascarl_400)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "nascpul.400",   0x0000, 0x20000, CRC(23ca7b4a) SHA1(9ea7afb283157a8e65106dc027cfd45eecc3f86a))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("nasdspl.400", 0x00000, 0x80000, CRC(a4de490f) SHA1(bc1aa9fc0182045f5d10044b3e4fa083572be4ac))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("biosv8.u8", 0x400000, 0x20000, CRC(c81b5532) SHA1(7bd630179910423a01394f82ad7ff2008c02f7ff))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("nascsp.u7",0x0000,0x10000,CRC(03a34394) SHA1(d1e3a1a8e14525c40e9f8a5441a106df662608f1))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("nassndl.u17", 0x000000, 0x100000, CRC(058c67ad) SHA1(70e22d8a1842108309f6c03dcc6ac23a822da3c3))
	ROM_LOAD("nassndl.u21", 0x100000, 0x100000, CRC(e34d3b6f) SHA1(63ef27ed5965d719215d0a469886d3852b6bffb6))
	ROM_LOAD("nassndl.u36", 0x200000, 0x100000, CRC(9e2658b1) SHA1(0d93a381a65f11022a1a6da5e5b0e4a0e779f336))
	ROM_LOAD("nassndl.u37", 0x300000, 0x100000, CRC(63f084ab) SHA1(519807bf6e868df6f756ad30af2f6636804f167c))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(nascarl_352)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "nascpul.352", 0x0000, 0x20000, CRC(c78549d8) SHA1(9796c0d413fd2ea7f616ad238b67311c8c29286d))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "nasdspl.303",   0x0000, 0x80000, CRC(868277f0) SHA1(9c058054e6dc3b838bfc3a91d37438afcd59aa4b))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("biosv8.u8", 0x400000, 0x20000, CRC(c81b5532) SHA1(7bd630179910423a01394f82ad7ff2008c02f7ff))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("nascsp.u7",0x0000,0x10000,CRC(03a34394) SHA1(d1e3a1a8e14525c40e9f8a5441a106df662608f1))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("nassndl.u17", 0x000000, 0x100000, CRC(058c67ad) SHA1(70e22d8a1842108309f6c03dcc6ac23a822da3c3))
	ROM_LOAD("nassndl.u21", 0x100000, 0x100000, CRC(e34d3b6f) SHA1(63ef27ed5965d719215d0a469886d3852b6bffb6))
	ROM_LOAD("nassndl.u36", 0x200000, 0x100000, CRC(9e2658b1) SHA1(0d93a381a65f11022a1a6da5e5b0e4a0e779f336))
	ROM_LOAD("nassndl.u37", 0x300000, 0x100000, CRC(63f084ab) SHA1(519807bf6e868df6f756ad30af2f6636804f167c))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(nascarl_350)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "nascpul.350", 0x0000, 0x20000, CRC(ab749309) SHA1(95d35126bda75c68037010f001c28a860b6a6e0c))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "nasdspl.303",   0x0000, 0x80000, CRC(868277f0) SHA1(9c058054e6dc3b838bfc3a91d37438afcd59aa4b))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("biosv8.u8", 0x400000, 0x20000, CRC(c81b5532) SHA1(7bd630179910423a01394f82ad7ff2008c02f7ff))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("nascsp.u7",0x0000,0x10000,CRC(03a34394) SHA1(d1e3a1a8e14525c40e9f8a5441a106df662608f1))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("nassndl.u17", 0x000000, 0x100000, CRC(058c67ad) SHA1(70e22d8a1842108309f6c03dcc6ac23a822da3c3))
	ROM_LOAD("nassndl.u21", 0x100000, 0x100000, CRC(e34d3b6f) SHA1(63ef27ed5965d719215d0a469886d3852b6bffb6))
	ROM_LOAD("nassndl.u36", 0x200000, 0x100000, CRC(9e2658b1) SHA1(0d93a381a65f11022a1a6da5e5b0e4a0e779f336))
	ROM_LOAD("nassndl.u37", 0x300000, 0x100000, CRC(63f084ab) SHA1(519807bf6e868df6f756ad30af2f6636804f167c))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(nascarl_340)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "nascpul.340",   0x0000, 0x20000, CRC(d5827082) SHA1(660216472a1faa445701eb3735771568cdba7b24))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "nasdspl.303",   0x0000, 0x80000, CRC(868277f0) SHA1(9c058054e6dc3b838bfc3a91d37438afcd59aa4b))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("biosv8.u8", 0x400000, 0x20000, CRC(c81b5532) SHA1(7bd630179910423a01394f82ad7ff2008c02f7ff))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("nascsp.u7",0x0000,0x10000,CRC(03a34394) SHA1(d1e3a1a8e14525c40e9f8a5441a106df662608f1))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("nassndl.u17", 0x000000, 0x100000, CRC(058c67ad) SHA1(70e22d8a1842108309f6c03dcc6ac23a822da3c3))
	ROM_LOAD("nassndl.u21", 0x100000, 0x100000, CRC(e34d3b6f) SHA1(63ef27ed5965d719215d0a469886d3852b6bffb6))
	ROM_LOAD("nassndl.u36", 0x200000, 0x100000, CRC(9e2658b1) SHA1(0d93a381a65f11022a1a6da5e5b0e4a0e779f336))
	ROM_LOAD("nassndl.u37", 0x300000, 0x100000, CRC(63f084ab) SHA1(519807bf6e868df6f756ad30af2f6636804f167c))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(nascarl_301)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "nascpul.301",   0x0000, 0x20000, CRC(6c8fc295) SHA1(2852afb38807a96907bc7357c08235de643dfb29))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "nasdspl.301",   0x0000, 0x80000, CRC(a6d4a9e7) SHA1(c87286bd173a50e17994474f98574c30cd6d8d39))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("biosv8.u8", 0x400000, 0x20000, CRC(c81b5532) SHA1(7bd630179910423a01394f82ad7ff2008c02f7ff))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("nascsp.u7",0x0000,0x10000,CRC(03a34394) SHA1(d1e3a1a8e14525c40e9f8a5441a106df662608f1))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("nassndl.u17", 0x000000, 0x100000, CRC(058c67ad) SHA1(70e22d8a1842108309f6c03dcc6ac23a822da3c3))
	ROM_LOAD("nassndl.u21", 0x100000, 0x100000, CRC(e34d3b6f) SHA1(63ef27ed5965d719215d0a469886d3852b6bffb6))
	ROM_LOAD("nassndl.u36", 0x200000, 0x100000, CRC(9e2658b1) SHA1(0d93a381a65f11022a1a6da5e5b0e4a0e779f336))
	ROM_LOAD("nassndl.u37", 0x300000, 0x100000, CRC(63f084ab) SHA1(519807bf6e868df6f756ad30af2f6636804f167c))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

/*-------------------------------------------------------------------
/ Ripley's Believe It or Not!
/-------------------------------------------------------------------*/
ROM_START(ripleys)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("ripcpu.320", 0x00000, 0x20000, CRC(aa997826) SHA1(2f9701370e64dd55a9bafe0c65e7eb4b9c5dbdd2))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("ripdispa.300", 0x00000, 0x80000, CRC(016907c9) SHA1(d37f1ca5ebe089fca879339cdaffc3fabf09c15c))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("ripsnd.u7",0x0000,0x10000,CRC(4573a759) SHA1(189c1a2eaf9d92c40a1bc145f59ac428c74a7318))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("ripsnd.u17", 0x000000, 0x100000, CRC(d518f2da) SHA1(e7d75c6b7b45571ae6d39ed7405b1457e475b52a))
	ROM_LOAD("ripsnd.u21", 0x100000, 0x100000, CRC(3d8680d7) SHA1(1368965106094d78be6540eb87a478f853ba774f))
	ROM_LOAD("ripsnd.u36", 0x200000, 0x100000, CRC(b697b5cb) SHA1(b5cb426201287a6d1c40db8c81a58e2c656d1d81))
	ROM_LOAD("ripsnd.u37", 0x300000, 0x100000, CRC(01b9f20e) SHA1(cffb6a0136d7d17ab4450b3bfd97632d8b669d39))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(rip310)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "ripcpu.310",    0x0000, 0x20000, CRC(669f87cc) SHA1(0e07bbcf337bf7c289a9093d3db805da617cbfef))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("ripdispa.300", 0x00000, 0x80000, CRC(016907c9) SHA1(d37f1ca5ebe089fca879339cdaffc3fabf09c15c))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("ripsnd.u7",0x0000,0x10000,CRC(4573a759) SHA1(189c1a2eaf9d92c40a1bc145f59ac428c74a7318))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("ripsnd.u17", 0x000000, 0x100000, CRC(d518f2da) SHA1(e7d75c6b7b45571ae6d39ed7405b1457e475b52a))
	ROM_LOAD("ripsnd.u21", 0x100000, 0x100000, CRC(3d8680d7) SHA1(1368965106094d78be6540eb87a478f853ba774f))
	ROM_LOAD("ripsnd.u36", 0x200000, 0x100000, CRC(b697b5cb) SHA1(b5cb426201287a6d1c40db8c81a58e2c656d1d81))
	ROM_LOAD("ripsnd.u37", 0x300000, 0x100000, CRC(01b9f20e) SHA1(cffb6a0136d7d17ab4450b3bfd97632d8b669d39))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(rip302)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "ripcpu.302",    0x0000, 0x20000, CRC(ee79d9eb) SHA1(79b45ceac00ebd414a9fb1d97c05252d9f953872))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)
	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("ripdispa.300", 0x00000, 0x80000, CRC(016907c9) SHA1(d37f1ca5ebe089fca879339cdaffc3fabf09c15c))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("ripsnd.u7",0x0000,0x10000,CRC(4573a759) SHA1(189c1a2eaf9d92c40a1bc145f59ac428c74a7318))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("ripsnd.u17", 0x000000, 0x100000, CRC(d518f2da) SHA1(e7d75c6b7b45571ae6d39ed7405b1457e475b52a))
	ROM_LOAD("ripsnd.u21", 0x100000, 0x100000, CRC(3d8680d7) SHA1(1368965106094d78be6540eb87a478f853ba774f))
	ROM_LOAD("ripsnd.u36", 0x200000, 0x100000, CRC(b697b5cb) SHA1(b5cb426201287a6d1c40db8c81a58e2c656d1d81))
	ROM_LOAD("ripsnd.u37", 0x300000, 0x100000, CRC(01b9f20e) SHA1(cffb6a0136d7d17ab4450b3bfd97632d8b669d39))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(rip301)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "ripcpu.301", 0x0000, 0x20000, CRC(a867d1b4) SHA1(dca4ba5c981397d26cac016d8438704f7adea0f3))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("ripdispa.300", 0x00000, 0x80000, CRC(016907c9) SHA1(d37f1ca5ebe089fca879339cdaffc3fabf09c15c))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("ripsnd.u7",0x0000,0x10000,CRC(4573a759) SHA1(189c1a2eaf9d92c40a1bc145f59ac428c74a7318))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("ripsnd.u17", 0x000000, 0x100000, CRC(d518f2da) SHA1(e7d75c6b7b45571ae6d39ed7405b1457e475b52a))
	ROM_LOAD("ripsnd.u21", 0x100000, 0x100000, CRC(3d8680d7) SHA1(1368965106094d78be6540eb87a478f853ba774f))
	ROM_LOAD("ripsnd.u36", 0x200000, 0x100000, CRC(b697b5cb) SHA1(b5cb426201287a6d1c40db8c81a58e2c656d1d81))
	ROM_LOAD("ripsnd.u37", 0x300000, 0x100000, CRC(01b9f20e) SHA1(cffb6a0136d7d17ab4450b3bfd97632d8b669d39))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(rip300)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "ripcpu.300",    0x0000, 0x20000, CRC(8c4bf2a9) SHA1(260dd5a99a36de541b5f852047ae4166afc621cc))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("ripdispa.300", 0x00000, 0x80000, CRC(016907c9) SHA1(d37f1ca5ebe089fca879339cdaffc3fabf09c15c))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("ripsnd.u7",0x0000,0x10000,CRC(4573a759) SHA1(189c1a2eaf9d92c40a1bc145f59ac428c74a7318))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("ripsnd.u17", 0x000000, 0x100000, CRC(d518f2da) SHA1(e7d75c6b7b45571ae6d39ed7405b1457e475b52a))
	ROM_LOAD("ripsnd.u21", 0x100000, 0x100000, CRC(3d8680d7) SHA1(1368965106094d78be6540eb87a478f853ba774f))
	ROM_LOAD("ripsnd.u36", 0x200000, 0x100000, CRC(b697b5cb) SHA1(b5cb426201287a6d1c40db8c81a58e2c656d1d81))
	ROM_LOAD("ripsnd.u37", 0x300000, 0x100000, CRC(01b9f20e) SHA1(cffb6a0136d7d17ab4450b3bfd97632d8b669d39))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(ripleysf)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("ripcpu.320", 0x00000, 0x20000, CRC(aa997826) SHA1(2f9701370e64dd55a9bafe0c65e7eb4b9c5dbdd2))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("ripdispf.301", 0x00000, 0x80000, CRC(e5ae9d99) SHA1(74929b324b457d08a925c641430e6a7036c7039d))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("ripsndf.u7",0x0000,0x10000,CRC(5808e3fc) SHA1(0c83399e8dc846607c469b7dd95878f3c2b9cb82))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("ripsndf.u17", 0x000000, 0x100000, CRC(a6793b85) SHA1(96058777346be6e9ea7b1340d9aaf945ac3c853a))
	ROM_LOAD("ripsndf.u21", 0x100000, 0x100000, CRC(60c02170) SHA1(900d9de3ccb541019e5f1528e01c57ad96dac262))
	ROM_LOAD("ripsndf.u36", 0x200000, 0x100000, CRC(0a57f2fd) SHA1(9dd057888294ee8abeb582e8f6650fd6e32cc9ff))
	ROM_LOAD("ripsndf.u37", 0x300000, 0x100000, CRC(5c858958) SHA1(f4a9833b8aee033ed381e3bdf9f801b935d6186a))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(ripf310)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "ripcpu.310",    0x0000, 0x20000, CRC(669f87cc) SHA1(0e07bbcf337bf7c289a9093d3db805da617cbfef))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("ripdispf.301", 0x00000, 0x80000, CRC(e5ae9d99) SHA1(74929b324b457d08a925c641430e6a7036c7039d))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("ripsndf.u7",0x0000,0x10000,CRC(5808e3fc) SHA1(0c83399e8dc846607c469b7dd95878f3c2b9cb82))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("ripsndf.u17", 0x000000, 0x100000, CRC(a6793b85) SHA1(96058777346be6e9ea7b1340d9aaf945ac3c853a))
	ROM_LOAD("ripsndf.u21", 0x100000, 0x100000, CRC(60c02170) SHA1(900d9de3ccb541019e5f1528e01c57ad96dac262))
	ROM_LOAD("ripsndf.u36", 0x200000, 0x100000, CRC(0a57f2fd) SHA1(9dd057888294ee8abeb582e8f6650fd6e32cc9ff))
	ROM_LOAD("ripsndf.u37", 0x300000, 0x100000, CRC(5c858958) SHA1(f4a9833b8aee033ed381e3bdf9f801b935d6186a))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(ripf302)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "ripcpu.302",    0x0000, 0x20000, CRC(ee79d9eb) SHA1(79b45ceac00ebd414a9fb1d97c05252d9f953872))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("ripdispf.301", 0x00000, 0x80000, CRC(e5ae9d99) SHA1(74929b324b457d08a925c641430e6a7036c7039d))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("ripsndf.u7",0x0000,0x10000,CRC(5808e3fc) SHA1(0c83399e8dc846607c469b7dd95878f3c2b9cb82))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("ripsndf.u17", 0x000000, 0x100000, CRC(a6793b85) SHA1(96058777346be6e9ea7b1340d9aaf945ac3c853a))
	ROM_LOAD("ripsndf.u21", 0x100000, 0x100000, CRC(60c02170) SHA1(900d9de3ccb541019e5f1528e01c57ad96dac262))
	ROM_LOAD("ripsndf.u36", 0x200000, 0x100000, CRC(0a57f2fd) SHA1(9dd057888294ee8abeb582e8f6650fd6e32cc9ff))
	ROM_LOAD("ripsndf.u37", 0x300000, 0x100000, CRC(5c858958) SHA1(f4a9833b8aee033ed381e3bdf9f801b935d6186a))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(ripf301)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "ripcpu.301", 0x0000, 0x20000, CRC(a867d1b4) SHA1(dca4ba5c981397d26cac016d8438704f7adea0f3))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)
	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("ripdispf.301", 0x00000, 0x80000, CRC(e5ae9d99) SHA1(74929b324b457d08a925c641430e6a7036c7039d))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("ripsndf.u7",0x0000,0x10000,CRC(5808e3fc) SHA1(0c83399e8dc846607c469b7dd95878f3c2b9cb82))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("ripsndf.u17", 0x000000, 0x100000, CRC(a6793b85) SHA1(96058777346be6e9ea7b1340d9aaf945ac3c853a))
	ROM_LOAD("ripsndf.u21", 0x100000, 0x100000, CRC(60c02170) SHA1(900d9de3ccb541019e5f1528e01c57ad96dac262))
	ROM_LOAD("ripsndf.u36", 0x200000, 0x100000, CRC(0a57f2fd) SHA1(9dd057888294ee8abeb582e8f6650fd6e32cc9ff))
	ROM_LOAD("ripsndf.u37", 0x300000, 0x100000, CRC(5c858958) SHA1(f4a9833b8aee033ed381e3bdf9f801b935d6186a))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(ripf300)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "ripcpu.300",    0x0000, 0x20000, CRC(8c4bf2a9) SHA1(260dd5a99a36de541b5f852047ae4166afc621cc))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "ripdispf.300",  0x0000, 0x80000, CRC(b9901941) SHA1(653997ff5d63e7ee0270db08cad952ac8293a8cd))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("ripsndf.u7",0x0000,0x10000,CRC(5808e3fc) SHA1(0c83399e8dc846607c469b7dd95878f3c2b9cb82))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("ripsndf.u17", 0x000000, 0x100000, CRC(a6793b85) SHA1(96058777346be6e9ea7b1340d9aaf945ac3c853a))
	ROM_LOAD("ripsndf.u21", 0x100000, 0x100000, CRC(60c02170) SHA1(900d9de3ccb541019e5f1528e01c57ad96dac262))
	ROM_LOAD("ripsndf.u36", 0x200000, 0x100000, CRC(0a57f2fd) SHA1(9dd057888294ee8abeb582e8f6650fd6e32cc9ff))
	ROM_LOAD("ripsndf.u37", 0x300000, 0x100000, CRC(5c858958) SHA1(f4a9833b8aee033ed381e3bdf9f801b935d6186a))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(ripleysg)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("ripcpu.320", 0x00000, 0x20000, CRC(aa997826) SHA1(2f9701370e64dd55a9bafe0c65e7eb4b9c5dbdd2))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("ripdispg.300", 0x00000, 0x80000, CRC(1a75883b) SHA1(0ef2f4af72e435e5be9d3d8a6b69c66ae18271a1))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("ripsndg.u7",0x0000,0x10000,CRC(400b8a45) SHA1(62101995e632264df3c014b746cc4b2ae72676d4))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("ripsndg.u17", 0x000000, 0x100000, CRC(c387dcf0) SHA1(d4ef65d3f33ab82b63bf2782f335858ab4ad210a))
	ROM_LOAD("ripsndg.u21", 0x100000, 0x100000, CRC(6388ae8d) SHA1(a39c7977194daabf3f5b10d0269dcd4118a939bc))
	ROM_LOAD("ripsndg.u36", 0x200000, 0x100000, CRC(3143f9d3) SHA1(bd4ce64b245b5fcb9b9694bd8f71a9cd98303cae))
	ROM_LOAD("ripsndg.u37", 0x300000, 0x100000, CRC(2167617b) SHA1(62b55a39e2677eec9d56b10e8cc3e5d7c0d3bea5))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(ripg310)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "ripcpu.310",    0x0000, 0x20000, CRC(669f87cc) SHA1(0e07bbcf337bf7c289a9093d3db805da617cbfef))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("ripdispg.300", 0x00000, 0x80000, CRC(1a75883b) SHA1(0ef2f4af72e435e5be9d3d8a6b69c66ae18271a1))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("ripsndg.u7",0x0000,0x10000,CRC(400b8a45) SHA1(62101995e632264df3c014b746cc4b2ae72676d4))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("ripsndg.u17", 0x000000, 0x100000, CRC(c387dcf0) SHA1(d4ef65d3f33ab82b63bf2782f335858ab4ad210a))
	ROM_LOAD("ripsndg.u21", 0x100000, 0x100000, CRC(6388ae8d) SHA1(a39c7977194daabf3f5b10d0269dcd4118a939bc))
	ROM_LOAD("ripsndg.u36", 0x200000, 0x100000, CRC(3143f9d3) SHA1(bd4ce64b245b5fcb9b9694bd8f71a9cd98303cae))
	ROM_LOAD("ripsndg.u37", 0x300000, 0x100000, CRC(2167617b) SHA1(62b55a39e2677eec9d56b10e8cc3e5d7c0d3bea5))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(ripg302)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "ripcpu.302",    0x0000, 0x20000, CRC(ee79d9eb) SHA1(79b45ceac00ebd414a9fb1d97c05252d9f953872))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("ripdispg.300", 0x00000, 0x80000, CRC(1a75883b) SHA1(0ef2f4af72e435e5be9d3d8a6b69c66ae18271a1))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("ripsndg.u7",0x0000,0x10000,CRC(400b8a45) SHA1(62101995e632264df3c014b746cc4b2ae72676d4))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("ripsndg.u17", 0x000000, 0x100000, CRC(c387dcf0) SHA1(d4ef65d3f33ab82b63bf2782f335858ab4ad210a))
	ROM_LOAD("ripsndg.u21", 0x100000, 0x100000, CRC(6388ae8d) SHA1(a39c7977194daabf3f5b10d0269dcd4118a939bc))
	ROM_LOAD("ripsndg.u36", 0x200000, 0x100000, CRC(3143f9d3) SHA1(bd4ce64b245b5fcb9b9694bd8f71a9cd98303cae))
	ROM_LOAD("ripsndg.u37", 0x300000, 0x100000, CRC(2167617b) SHA1(62b55a39e2677eec9d56b10e8cc3e5d7c0d3bea5))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(ripg301)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "ripcpu.301", 0x0000, 0x20000, CRC(a867d1b4) SHA1(dca4ba5c981397d26cac016d8438704f7adea0f3))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("ripdispg.300", 0x00000, 0x80000, CRC(1a75883b) SHA1(0ef2f4af72e435e5be9d3d8a6b69c66ae18271a1))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("ripsndg.u7",0x0000,0x10000,CRC(400b8a45) SHA1(62101995e632264df3c014b746cc4b2ae72676d4))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("ripsndg.u17", 0x000000, 0x100000, CRC(c387dcf0) SHA1(d4ef65d3f33ab82b63bf2782f335858ab4ad210a))
	ROM_LOAD("ripsndg.u21", 0x100000, 0x100000, CRC(6388ae8d) SHA1(a39c7977194daabf3f5b10d0269dcd4118a939bc))
	ROM_LOAD("ripsndg.u36", 0x200000, 0x100000, CRC(3143f9d3) SHA1(bd4ce64b245b5fcb9b9694bd8f71a9cd98303cae))
	ROM_LOAD("ripsndg.u37", 0x300000, 0x100000, CRC(2167617b) SHA1(62b55a39e2677eec9d56b10e8cc3e5d7c0d3bea5))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(ripg300)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "ripcpu.300",    0x0000, 0x20000, CRC(8c4bf2a9) SHA1(260dd5a99a36de541b5f852047ae4166afc621cc))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("ripdispg.300", 0x00000, 0x80000, CRC(1a75883b) SHA1(0ef2f4af72e435e5be9d3d8a6b69c66ae18271a1))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("ripsndg.u7",0x0000,0x10000,CRC(400b8a45) SHA1(62101995e632264df3c014b746cc4b2ae72676d4))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("ripsndg.u17", 0x000000, 0x100000, CRC(c387dcf0) SHA1(d4ef65d3f33ab82b63bf2782f335858ab4ad210a))
	ROM_LOAD("ripsndg.u21", 0x100000, 0x100000, CRC(6388ae8d) SHA1(a39c7977194daabf3f5b10d0269dcd4118a939bc))
	ROM_LOAD("ripsndg.u36", 0x200000, 0x100000, CRC(3143f9d3) SHA1(bd4ce64b245b5fcb9b9694bd8f71a9cd98303cae))
	ROM_LOAD("ripsndg.u37", 0x300000, 0x100000, CRC(2167617b) SHA1(62b55a39e2677eec9d56b10e8cc3e5d7c0d3bea5))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(ripleysi)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("ripcpu.320", 0x00000, 0x20000, CRC(aa997826) SHA1(2f9701370e64dd55a9bafe0c65e7eb4b9c5dbdd2))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("ripdispi.300", 0x00000, 0x80000, CRC(c3541c04) SHA1(26256e8dee77bcfa96326d2e3f67b6fd3696c0c7))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("ripsndi.u7",0x0000,0x10000,CRC(86b1b2b2) SHA1(9e2cf7368b31531998d546a1be2af274a9cbbd2f))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("ripsndi.u17", 0x000000, 0x100000, CRC(a2911df4) SHA1(acb7956a6a30142c8da905b04778a074cb335807))
	ROM_LOAD("ripsndi.u21", 0x100000, 0x100000, CRC(1467eaff) SHA1(c6c4ea2abdad4334efbe3a084693e9e4d0dd0fd2))
	ROM_LOAD("ripsndi.u36", 0x200000, 0x100000, CRC(6a124fa6) SHA1(752c3d227b9a98dd859e4778ddd527edaa3cf512))
	ROM_LOAD("ripsndi.u37", 0x300000, 0x100000, CRC(7933c102) SHA1(f736ee86d7c67dab82c634d125d73a2453249706))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(ripi310)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "ripcpu.310",    0x0000, 0x20000, CRC(669f87cc) SHA1(0e07bbcf337bf7c289a9093d3db805da617cbfef))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("ripdispi.300", 0x00000, 0x80000, CRC(c3541c04) SHA1(26256e8dee77bcfa96326d2e3f67b6fd3696c0c7))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("ripsndi.u7",0x0000,0x10000,CRC(86b1b2b2) SHA1(9e2cf7368b31531998d546a1be2af274a9cbbd2f))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("ripsndi.u17", 0x000000, 0x100000, CRC(a2911df4) SHA1(acb7956a6a30142c8da905b04778a074cb335807))
	ROM_LOAD("ripsndi.u21", 0x100000, 0x100000, CRC(1467eaff) SHA1(c6c4ea2abdad4334efbe3a084693e9e4d0dd0fd2))
	ROM_LOAD("ripsndi.u36", 0x200000, 0x100000, CRC(6a124fa6) SHA1(752c3d227b9a98dd859e4778ddd527edaa3cf512))
	ROM_LOAD("ripsndi.u37", 0x300000, 0x100000, CRC(7933c102) SHA1(f736ee86d7c67dab82c634d125d73a2453249706))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(ripi302)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "ripcpu.302",    0x0000, 0x20000, CRC(ee79d9eb) SHA1(79b45ceac00ebd414a9fb1d97c05252d9f953872))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("ripdispi.300", 0x00000, 0x80000, CRC(c3541c04) SHA1(26256e8dee77bcfa96326d2e3f67b6fd3696c0c7))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("ripsndi.u7",0x0000,0x10000,CRC(86b1b2b2) SHA1(9e2cf7368b31531998d546a1be2af274a9cbbd2f))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("ripsndi.u17", 0x000000, 0x100000, CRC(a2911df4) SHA1(acb7956a6a30142c8da905b04778a074cb335807))
	ROM_LOAD("ripsndi.u21", 0x100000, 0x100000, CRC(1467eaff) SHA1(c6c4ea2abdad4334efbe3a084693e9e4d0dd0fd2))
	ROM_LOAD("ripsndi.u36", 0x200000, 0x100000, CRC(6a124fa6) SHA1(752c3d227b9a98dd859e4778ddd527edaa3cf512))
	ROM_LOAD("ripsndi.u37", 0x300000, 0x100000, CRC(7933c102) SHA1(f736ee86d7c67dab82c634d125d73a2453249706))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(ripi301)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "ripcpu.301", 0x0000, 0x20000, CRC(a867d1b4) SHA1(dca4ba5c981397d26cac016d8438704f7adea0f3))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("ripdispi.300", 0x00000, 0x80000, CRC(c3541c04) SHA1(26256e8dee77bcfa96326d2e3f67b6fd3696c0c7))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("ripsndi.u7",0x0000,0x10000,CRC(86b1b2b2) SHA1(9e2cf7368b31531998d546a1be2af274a9cbbd2f))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("ripsndi.u17", 0x000000, 0x100000, CRC(a2911df4) SHA1(acb7956a6a30142c8da905b04778a074cb335807))
	ROM_LOAD("ripsndi.u21", 0x100000, 0x100000, CRC(1467eaff) SHA1(c6c4ea2abdad4334efbe3a084693e9e4d0dd0fd2))
	ROM_LOAD("ripsndi.u36", 0x200000, 0x100000, CRC(6a124fa6) SHA1(752c3d227b9a98dd859e4778ddd527edaa3cf512))
	ROM_LOAD("ripsndi.u37", 0x300000, 0x100000, CRC(7933c102) SHA1(f736ee86d7c67dab82c634d125d73a2453249706))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(ripi300)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "ripcpu.300",    0x0000, 0x20000, CRC(8c4bf2a9) SHA1(260dd5a99a36de541b5f852047ae4166afc621cc))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("ripdispi.300", 0x00000, 0x80000, CRC(c3541c04) SHA1(26256e8dee77bcfa96326d2e3f67b6fd3696c0c7))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("ripsndi.u7",0x0000,0x10000,CRC(86b1b2b2) SHA1(9e2cf7368b31531998d546a1be2af274a9cbbd2f))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("ripsndi.u17", 0x000000, 0x100000, CRC(a2911df4) SHA1(acb7956a6a30142c8da905b04778a074cb335807))
	ROM_LOAD("ripsndi.u21", 0x100000, 0x100000, CRC(1467eaff) SHA1(c6c4ea2abdad4334efbe3a084693e9e4d0dd0fd2))
	ROM_LOAD("ripsndi.u36", 0x200000, 0x100000, CRC(6a124fa6) SHA1(752c3d227b9a98dd859e4778ddd527edaa3cf512))
	ROM_LOAD("ripsndi.u37", 0x300000, 0x100000, CRC(7933c102) SHA1(f736ee86d7c67dab82c634d125d73a2453249706))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(ripleysl)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("ripcpu.320", 0x00000, 0x20000, CRC(aa997826) SHA1(2f9701370e64dd55a9bafe0c65e7eb4b9c5dbdd2))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("ripdispl.301", 0x00000, 0x80000, CRC(47c87ad4) SHA1(eb372b9f17b28d0781c49a28cb850916ccec323d))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("ripsndl.u7",0x0000,0x10000,CRC(25fb729a) SHA1(46b9ca8fd5fb5a692adbdb7495af34a1db89dc37))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("ripsndl.u17", 0x000000, 0x100000, CRC(a98f4514) SHA1(e87ee8f5a87a8ae9ec996473bf9bc745105ea334))
	ROM_LOAD("ripsndl.u21", 0x100000, 0x100000, CRC(141f2b77) SHA1(15bab623beda8ae7ed9908f492ff2baab0a7954e))
	ROM_LOAD("ripsndl.u36", 0x200000, 0x100000, CRC(c5461b63) SHA1(fc574d44ad88ce1db590ea371225092c03fc6f80))
	ROM_LOAD("ripsndl.u37", 0x300000, 0x100000, CRC(2a58f491) SHA1(1c33f419420b3165ef18598560007ef612b24814))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(ripl310)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "ripcpu.310",    0x0000, 0x20000, CRC(669f87cc) SHA1(0e07bbcf337bf7c289a9093d3db805da617cbfef))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("ripdispl.301", 0x00000, 0x80000, CRC(47c87ad4) SHA1(eb372b9f17b28d0781c49a28cb850916ccec323d))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("ripsndl.u7",0x0000,0x10000,CRC(25fb729a) SHA1(46b9ca8fd5fb5a692adbdb7495af34a1db89dc37))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("ripsndl.u17", 0x000000, 0x100000, CRC(a98f4514) SHA1(e87ee8f5a87a8ae9ec996473bf9bc745105ea334))
	ROM_LOAD("ripsndl.u21", 0x100000, 0x100000, CRC(141f2b77) SHA1(15bab623beda8ae7ed9908f492ff2baab0a7954e))
	ROM_LOAD("ripsndl.u36", 0x200000, 0x100000, CRC(c5461b63) SHA1(fc574d44ad88ce1db590ea371225092c03fc6f80))
	ROM_LOAD("ripsndl.u37", 0x300000, 0x100000, CRC(2a58f491) SHA1(1c33f419420b3165ef18598560007ef612b24814))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(ripl302)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "ripcpu.302",    0x0000, 0x20000, CRC(ee79d9eb) SHA1(79b45ceac00ebd414a9fb1d97c05252d9f953872))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("ripdispl.301", 0x00000, 0x80000, CRC(47c87ad4) SHA1(eb372b9f17b28d0781c49a28cb850916ccec323d))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("ripsndl.u7",0x0000,0x10000,CRC(25fb729a) SHA1(46b9ca8fd5fb5a692adbdb7495af34a1db89dc37))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("ripsndl.u17", 0x000000, 0x100000, CRC(a98f4514) SHA1(e87ee8f5a87a8ae9ec996473bf9bc745105ea334))
	ROM_LOAD("ripsndl.u21", 0x100000, 0x100000, CRC(141f2b77) SHA1(15bab623beda8ae7ed9908f492ff2baab0a7954e))
	ROM_LOAD("ripsndl.u36", 0x200000, 0x100000, CRC(c5461b63) SHA1(fc574d44ad88ce1db590ea371225092c03fc6f80))
	ROM_LOAD("ripsndl.u37", 0x300000, 0x100000, CRC(2a58f491) SHA1(1c33f419420b3165ef18598560007ef612b24814))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(ripl301)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "ripcpu.301", 0x0000, 0x20000, CRC(a867d1b4) SHA1(dca4ba5c981397d26cac016d8438704f7adea0f3))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("ripdispl.301", 0x00000, 0x80000, CRC(47c87ad4) SHA1(eb372b9f17b28d0781c49a28cb850916ccec323d))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("ripsndl.u7",0x0000,0x10000,CRC(25fb729a) SHA1(46b9ca8fd5fb5a692adbdb7495af34a1db89dc37))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("ripsndl.u17", 0x000000, 0x100000, CRC(a98f4514) SHA1(e87ee8f5a87a8ae9ec996473bf9bc745105ea334))
	ROM_LOAD("ripsndl.u21", 0x100000, 0x100000, CRC(141f2b77) SHA1(15bab623beda8ae7ed9908f492ff2baab0a7954e))
	ROM_LOAD("ripsndl.u36", 0x200000, 0x100000, CRC(c5461b63) SHA1(fc574d44ad88ce1db590ea371225092c03fc6f80))
	ROM_LOAD("ripsndl.u37", 0x300000, 0x100000, CRC(2a58f491) SHA1(1c33f419420b3165ef18598560007ef612b24814))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(ripl300)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "ripcpu.300",    0x0000, 0x20000, CRC(8c4bf2a9) SHA1(260dd5a99a36de541b5f852047ae4166afc621cc))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "ripdispl.300",  0x0000, 0x80000, CRC(d2f496bb) SHA1(48622e25171030b83d8d1736735e97a13c5f47c6))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("ripsndl.u7",0x0000,0x10000,CRC(25fb729a) SHA1(46b9ca8fd5fb5a692adbdb7495af34a1db89dc37))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("ripsndl.u17", 0x000000, 0x100000, CRC(a98f4514) SHA1(e87ee8f5a87a8ae9ec996473bf9bc745105ea334))
	ROM_LOAD("ripsndl.u21", 0x100000, 0x100000, CRC(141f2b77) SHA1(15bab623beda8ae7ed9908f492ff2baab0a7954e))
	ROM_LOAD("ripsndl.u36", 0x200000, 0x100000, CRC(c5461b63) SHA1(fc574d44ad88ce1db590ea371225092c03fc6f80))
	ROM_LOAD("ripsndl.u37", 0x300000, 0x100000, CRC(2a58f491) SHA1(1c33f419420b3165ef18598560007ef612b24814))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END


/*-------------------------------------------------------------------
/ The Lord Of The Rings
/-------------------------------------------------------------------*/
ROM_START(lotr)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("lotrcpua.a00", 0x00000, 0x20000, CRC(00e43b70) SHA1(7100da644a1b166051915870d01cfa6baaf87293))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("lotrdspa.a00", 0x00000, 0x80000, CRC(99634603) SHA1(c40d1480e5df10a491bcd471c6a3a118a9120bcb))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("lotr-u7.101",0x0000,0x10000,CRC(ba018c5c) SHA1(67e4b9729f086de5e8d56a6ac29fce1c7082e470))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("lotr-u17.100", 0x000000, 0x100000, CRC(d503969d) SHA1(65a4313ca1b93391260c65fef6f878d264f9c8ab))
	ROM_LOAD("lotr-u21.100", 0x100000, 0x100000, CRC(8b3f41af) SHA1(9b2d04144edeb499b4ae68a97c65ccb8ef4d26c0))
	ROM_LOAD("lotr-u36.100", 0x200000, 0x100000, CRC(9575981e) SHA1(38083fd923c4a168a94d998ec3c4db42c1e2a2da))
	ROM_LOAD("lotr-u37.100", 0x300000, 0x100000, CRC(8e637a6f) SHA1(8087744ce36fc143381d49a312c98cf38b2f9854))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(lotr9)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("lotrcpu.900",   0x0000, 0x20000, CRC(eff1ab83) SHA1(cd9cfa3fa150224e44078602db7d8bbfe223b926))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "lotrdspa.900",  0x0000, 0x80000, CRC(2b1debd3) SHA1(eab1ffa7b5111bf224c47688bb6c0f40ee6e12fb))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("lotr-u7.101",0x0000,0x10000,CRC(ba018c5c) SHA1(67e4b9729f086de5e8d56a6ac29fce1c7082e470))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("lotr-u17.100", 0x000000, 0x100000, CRC(d503969d) SHA1(65a4313ca1b93391260c65fef6f878d264f9c8ab))
	ROM_LOAD("lotr-u21.100", 0x100000, 0x100000, CRC(8b3f41af) SHA1(9b2d04144edeb499b4ae68a97c65ccb8ef4d26c0))
	ROM_LOAD("lotr-u36.100", 0x200000, 0x100000, CRC(9575981e) SHA1(38083fd923c4a168a94d998ec3c4db42c1e2a2da))
	ROM_LOAD("lotr-u37.100", 0x300000, 0x100000, CRC(8e637a6f) SHA1(8087744ce36fc143381d49a312c98cf38b2f9854))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(lotr8)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("lotrcpu.800",   0x0000, 0x20000, CRC(655e5b3c) SHA1(bd6fd25e17cee40d6bb842367b1ce922bbd46003))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "lotrdspa.800",  0x0000, 0x80000, CRC(2aa1f00d) SHA1(e9df5b61b467c307aacdb5a6980a78af26492e6a))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("lotr-u7.101",0x0000,0x10000,CRC(ba018c5c) SHA1(67e4b9729f086de5e8d56a6ac29fce1c7082e470))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("lotr-u17.100", 0x000000, 0x100000, CRC(d503969d) SHA1(65a4313ca1b93391260c65fef6f878d264f9c8ab))
	ROM_LOAD("lotr-u21.100", 0x100000, 0x100000, CRC(8b3f41af) SHA1(9b2d04144edeb499b4ae68a97c65ccb8ef4d26c0))
	ROM_LOAD("lotr-u36.100", 0x200000, 0x100000, CRC(9575981e) SHA1(38083fd923c4a168a94d998ec3c4db42c1e2a2da))
	ROM_LOAD("lotr-u37.100", 0x300000, 0x100000, CRC(8e637a6f) SHA1(8087744ce36fc143381d49a312c98cf38b2f9854))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(lotr7)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("lotrcpu.700",   0x0000, 0x20000, CRC(208a4653) SHA1(570f3070c6b0f128d451f3dea01e41a9944081f2))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "lotrdspa.700",  0x0000, 0x80000, CRC(233ef0ad) SHA1(1564ae806639dac49add0c464f4499f46b5589ab)) // ver 7.00

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("lotr-u7.101",0x0000,0x10000,CRC(ba018c5c) SHA1(67e4b9729f086de5e8d56a6ac29fce1c7082e470))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("lotr-u17.100", 0x000000, 0x100000, CRC(d503969d) SHA1(65a4313ca1b93391260c65fef6f878d264f9c8ab))
	ROM_LOAD("lotr-u21.100", 0x100000, 0x100000, CRC(8b3f41af) SHA1(9b2d04144edeb499b4ae68a97c65ccb8ef4d26c0))
	ROM_LOAD("lotr-u36.100", 0x200000, 0x100000, CRC(9575981e) SHA1(38083fd923c4a168a94d998ec3c4db42c1e2a2da))
	ROM_LOAD("lotr-u37.100", 0x300000, 0x100000, CRC(8e637a6f) SHA1(8087744ce36fc143381d49a312c98cf38b2f9854))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(lotr6)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "lotrcpu.600", 0x0000, 0x20000, CRC(02786892) SHA1(6810d5a5eb80f520e611a46921dbd2906fbebf2f))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "lotrdspa.600", 0x0000, 0x80000, CRC(d2098cec) SHA1(06c5c0b29e1442f503b4b374537b9d233721b4b6))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("lotr-u7.101",0x0000,0x10000,CRC(ba018c5c) SHA1(67e4b9729f086de5e8d56a6ac29fce1c7082e470))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("lotr-u17.100", 0x000000, 0x100000, CRC(d503969d) SHA1(65a4313ca1b93391260c65fef6f878d264f9c8ab))
	ROM_LOAD("lotr-u21.100", 0x100000, 0x100000, CRC(8b3f41af) SHA1(9b2d04144edeb499b4ae68a97c65ccb8ef4d26c0))
	ROM_LOAD("lotr-u36.100", 0x200000, 0x100000, CRC(9575981e) SHA1(38083fd923c4a168a94d998ec3c4db42c1e2a2da))
	ROM_LOAD("lotr-u37.100", 0x300000, 0x100000, CRC(8e637a6f) SHA1(8087744ce36fc143381d49a312c98cf38b2f9854))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(lotr501)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "lotrcpu.501", 0x0000, 0x20000, CRC(df9fd692) SHA1(2d06c1a763330b1b9429961f3e13574e0eefe7a7))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "lotrdspa.501", 0x0000, 0x80000, CRC(2d555b9f) SHA1(d2d23182dea810624cab010890971d8997f8570c))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("lotr-u7.101",0x0000,0x10000,CRC(ba018c5c) SHA1(67e4b9729f086de5e8d56a6ac29fce1c7082e470))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("lotr-u17.100", 0x000000, 0x100000, CRC(d503969d) SHA1(65a4313ca1b93391260c65fef6f878d264f9c8ab))
	ROM_LOAD("lotr-u21.100", 0x100000, 0x100000, CRC(8b3f41af) SHA1(9b2d04144edeb499b4ae68a97c65ccb8ef4d26c0))
	ROM_LOAD("lotr-u36.100", 0x200000, 0x100000, CRC(9575981e) SHA1(38083fd923c4a168a94d998ec3c4db42c1e2a2da))
	ROM_LOAD("lotr-u37.100", 0x300000, 0x100000, CRC(8e637a6f) SHA1(8087744ce36fc143381d49a312c98cf38b2f9854))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(lotr5)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "lotrcpu.500", 0x0000, 0x20000, CRC(4b45a543) SHA1(e0be3a4244025abc2c109f58b0d637262711b9db))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "lotrdspa.500", 0x0000, 0x80000, CRC(19bda8d2) SHA1(a3d9e60f964d100594f82ed361f86e74c8d69748))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("lotr-u7.101",0x0000,0x10000,CRC(ba018c5c) SHA1(67e4b9729f086de5e8d56a6ac29fce1c7082e470))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("lotr-u17.100", 0x000000, 0x100000, CRC(d503969d) SHA1(65a4313ca1b93391260c65fef6f878d264f9c8ab))
	ROM_LOAD("lotr-u21.100", 0x100000, 0x100000, CRC(8b3f41af) SHA1(9b2d04144edeb499b4ae68a97c65ccb8ef4d26c0))
	ROM_LOAD("lotr-u36.100", 0x200000, 0x100000, CRC(9575981e) SHA1(38083fd923c4a168a94d998ec3c4db42c1e2a2da))
	ROM_LOAD("lotr-u37.100", 0x300000, 0x100000, CRC(8e637a6f) SHA1(8087744ce36fc143381d49a312c98cf38b2f9854))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(lotr410)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "lotrcpu.410", 0x0000, 0x20000, CRC(458af09a) SHA1(2680f16d7f33ffc70b64bfb7d35cccf6989c70e2))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "lotrdspa.404", 0x0000, 0x80000, CRC(1aefcbe7) SHA1(b17fc82425dd5a6ea5a17205d4000294324bb5cc))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("lotr-u7.101",0x0000,0x10000,CRC(ba018c5c) SHA1(67e4b9729f086de5e8d56a6ac29fce1c7082e470))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("lotr-u17.100", 0x000000, 0x100000, CRC(d503969d) SHA1(65a4313ca1b93391260c65fef6f878d264f9c8ab))
	ROM_LOAD("lotr-u21.100", 0x100000, 0x100000, CRC(8b3f41af) SHA1(9b2d04144edeb499b4ae68a97c65ccb8ef4d26c0))
	ROM_LOAD("lotr-u36.100", 0x200000, 0x100000, CRC(9575981e) SHA1(38083fd923c4a168a94d998ec3c4db42c1e2a2da))
	ROM_LOAD("lotr-u37.100", 0x300000, 0x100000, CRC(8e637a6f) SHA1(8087744ce36fc143381d49a312c98cf38b2f9854))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(lotr401)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("lotrcpu.401",   0x0000, 0x20000, CRC(b69cdecc) SHA1(40f7c2d25a1028255be8fe25e3aa6d11976edd25))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "lotrdspa.403",  0x0000, 0x80000, CRC(2630cef1) SHA1(1dfd929e7eb57983f2fd9184d471f2e919359de0))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("lotr-u7.101",0x0000,0x10000,CRC(ba018c5c) SHA1(67e4b9729f086de5e8d56a6ac29fce1c7082e470))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("lotr-u17.100", 0x000000, 0x100000, CRC(d503969d) SHA1(65a4313ca1b93391260c65fef6f878d264f9c8ab))
	ROM_LOAD("lotr-u21.100", 0x100000, 0x100000, CRC(8b3f41af) SHA1(9b2d04144edeb499b4ae68a97c65ccb8ef4d26c0))
	ROM_LOAD("lotr-u36.100", 0x200000, 0x100000, CRC(9575981e) SHA1(38083fd923c4a168a94d998ec3c4db42c1e2a2da))
	ROM_LOAD("lotr-u37.100", 0x300000, 0x100000, CRC(8e637a6f) SHA1(8087744ce36fc143381d49a312c98cf38b2f9854))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(lotr3)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("lotrcpu.300",   0x0000, 0x20000, CRC(fdc5351d) SHA1(92d58bcdd21026d278b5534d1a6ab299f6fffc60))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "lotrdspa.300",  0x0000, 0x80000, CRC(522e3e33) SHA1(11987872604e2a3e2c9567f8f9313c36e6c08cc7))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("lotr-u7.101",0x0000,0x10000,CRC(ba018c5c) SHA1(67e4b9729f086de5e8d56a6ac29fce1c7082e470))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("lotr-u17.100", 0x000000, 0x100000, CRC(d503969d) SHA1(65a4313ca1b93391260c65fef6f878d264f9c8ab))
	ROM_LOAD("lotr-u21.100", 0x100000, 0x100000, CRC(8b3f41af) SHA1(9b2d04144edeb499b4ae68a97c65ccb8ef4d26c0))
	ROM_LOAD("lotr-u36.100", 0x200000, 0x100000, CRC(9575981e) SHA1(38083fd923c4a168a94d998ec3c4db42c1e2a2da))
	ROM_LOAD("lotr-u37.100", 0x300000, 0x100000, CRC(8e637a6f) SHA1(8087744ce36fc143381d49a312c98cf38b2f9854))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(lotr_sp)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("lotrcpul.a00", 0x00000, 0x20000, CRC(c62aba47) SHA1(2fef599313e5cd9bded3ab00b933631586e2a1e7))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)
	ROM_REGION(0x80000, "decodmd", 0)

	ROM_LOAD("lotrdspl.a00", 0x00000, 0x80000, CRC(2494a5ee) SHA1(5b95711858d88eeb445503cac8b9b754cf8e9960))
	ROM_REGION32_LE(0x600000, "cpu2", 0)

	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("lotrlu7.100",0x0000,0x10000,CRC(980d970a) SHA1(cf70deddcc146ef9eaa64baec74ae800bebf8715))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("lotrlu17.100", 0x000000, 0x100000, CRC(c16d3e20) SHA1(43d9f186db361abb3aa119a7252f1bb13bbbbe39))
	ROM_LOAD("lotrlu21.100", 0x100000, 0x100000, CRC(f956a1be) SHA1(2980e85463704a154ed81d3241f866442d1ea2e6))
	ROM_LOAD("lotrlu36.100", 0x200000, 0x100000, CRC(502c3d93) SHA1(4ee42d70bccb9253fe1f8f441de6b5018257f107))
	ROM_LOAD("lotrlu37.100", 0x300000, 0x100000, CRC(61f21c6d) SHA1(3e9781b4981bd18cdb8c59c55b9942de6ae286db))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(lotr_sp9)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "lotrcpul.900",  0x0000, 0x20000, CRC(155b5d5b) SHA1(c032e3828ed256240a5155ec4c7820d615a2cbe1))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "lotrdspl.900",  0x0000, 0x80000, CRC(00f98242) SHA1(9a0e7e572e209b20691392a694a524192daa0d2a))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("lotrlu7.100",0x0000,0x10000,CRC(980d970a) SHA1(cf70deddcc146ef9eaa64baec74ae800bebf8715))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("lotrlu17.100", 0x000000, 0x100000, CRC(c16d3e20) SHA1(43d9f186db361abb3aa119a7252f1bb13bbbbe39))
	ROM_LOAD("lotrlu21.100", 0x100000, 0x100000, CRC(f956a1be) SHA1(2980e85463704a154ed81d3241f866442d1ea2e6))
	ROM_LOAD("lotrlu36.100", 0x200000, 0x100000, CRC(502c3d93) SHA1(4ee42d70bccb9253fe1f8f441de6b5018257f107))
	ROM_LOAD("lotrlu37.100", 0x300000, 0x100000, CRC(61f21c6d) SHA1(3e9781b4981bd18cdb8c59c55b9942de6ae286db))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(lotr_sp8)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "lotrcpul.800",  0x0000, 0x20000, CRC(38e83068) SHA1(603b1236bf195c72d32e5a1088a2806e77176436))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "lotrdspl.800",  0x0000, 0x80000, CRC(137c1255) SHA1(43d9ffec18ab2aa80f30b195ca5270d4574d7b8d))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("lotrlu7.100",0x0000,0x10000,CRC(980d970a) SHA1(cf70deddcc146ef9eaa64baec74ae800bebf8715))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("lotrlu17.100", 0x000000, 0x100000, CRC(c16d3e20) SHA1(43d9f186db361abb3aa119a7252f1bb13bbbbe39))
	ROM_LOAD("lotrlu21.100", 0x100000, 0x100000, CRC(f956a1be) SHA1(2980e85463704a154ed81d3241f866442d1ea2e6))
	ROM_LOAD("lotrlu36.100", 0x200000, 0x100000, CRC(502c3d93) SHA1(4ee42d70bccb9253fe1f8f441de6b5018257f107))
	ROM_LOAD("lotrlu37.100", 0x300000, 0x100000, CRC(61f21c6d) SHA1(3e9781b4981bd18cdb8c59c55b9942de6ae286db))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(lotr_sp7)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "lotrcpul.700",  0x0000, 0x20000, CRC(58d8611b) SHA1(ff1e1668993c7c31f04efc22f04ae53112219a1d))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "lotrdspl.700",  0x0000, 0x80000, CRC(3be0283d) SHA1(e019c69cd452b67d6427ddda12b5c3f341afb414))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("lotrlu7.100",0x0000,0x10000,CRC(980d970a) SHA1(cf70deddcc146ef9eaa64baec74ae800bebf8715))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("lotrlu17.100", 0x000000, 0x100000, CRC(c16d3e20) SHA1(43d9f186db361abb3aa119a7252f1bb13bbbbe39))
	ROM_LOAD("lotrlu21.100", 0x100000, 0x100000, CRC(f956a1be) SHA1(2980e85463704a154ed81d3241f866442d1ea2e6))
	ROM_LOAD("lotrlu36.100", 0x200000, 0x100000, CRC(502c3d93) SHA1(4ee42d70bccb9253fe1f8f441de6b5018257f107))
	ROM_LOAD("lotrlu37.100", 0x300000, 0x100000, CRC(61f21c6d) SHA1(3e9781b4981bd18cdb8c59c55b9942de6ae286db))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(lotr_sp6)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "lotrcpul.600", 0x0000, 0x20000, CRC(af06c560) SHA1(0dec564e8e50ca8e05c462517db38ae48e512e79))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "lotrdspl.600", 0x0000, 0x80000, CRC(d664d989) SHA1(575f9dcc7cb8aac4bfdb25575b7b9c00cf6459b9))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("lotrlu7.100",0x0000,0x10000,CRC(980d970a) SHA1(cf70deddcc146ef9eaa64baec74ae800bebf8715))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("lotrlu17.100", 0x000000, 0x100000, CRC(c16d3e20) SHA1(43d9f186db361abb3aa119a7252f1bb13bbbbe39))
	ROM_LOAD("lotrlu21.100", 0x100000, 0x100000, CRC(f956a1be) SHA1(2980e85463704a154ed81d3241f866442d1ea2e6))
	ROM_LOAD("lotrlu36.100", 0x200000, 0x100000, CRC(502c3d93) SHA1(4ee42d70bccb9253fe1f8f441de6b5018257f107))
	ROM_LOAD("lotrlu37.100", 0x300000, 0x100000, CRC(61f21c6d) SHA1(3e9781b4981bd18cdb8c59c55b9942de6ae286db))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(lotr_sp501)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "lotrcpul.501", 0x0000, 0x20000, CRC(4c0c7360) SHA1(f057931eb719a7a6691187fa7ca86cd6e4541d90))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "lotrdspl.501", 0x0000, 0x80000, CRC(7e96c0f8) SHA1(2ef63b1b30fb2680b97a9080f7b9d76b4d2a76d4))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("lotrlu7.100",0x0000,0x10000,CRC(980d970a) SHA1(cf70deddcc146ef9eaa64baec74ae800bebf8715))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("lotrlu17.100", 0x000000, 0x100000, CRC(c16d3e20) SHA1(43d9f186db361abb3aa119a7252f1bb13bbbbe39))
	ROM_LOAD("lotrlu21.100", 0x100000, 0x100000, CRC(f956a1be) SHA1(2980e85463704a154ed81d3241f866442d1ea2e6))
	ROM_LOAD("lotrlu36.100", 0x200000, 0x100000, CRC(502c3d93) SHA1(4ee42d70bccb9253fe1f8f441de6b5018257f107))
	ROM_LOAD("lotrlu37.100", 0x300000, 0x100000, CRC(61f21c6d) SHA1(3e9781b4981bd18cdb8c59c55b9942de6ae286db))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(lotr_sp5)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "lotrcpul.500", 0x0000, 0x20000, CRC(298d10bd) SHA1(133990aed459d5fcd191a08462a231bbd3449387))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "lotrdspl.500", 0x0000, 0x80000, CRC(ab538b24) SHA1(039aa8f4286694971cd9a78805bb9f3acabcd692))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("lotrlu7.100",0x0000,0x10000,CRC(980d970a) SHA1(cf70deddcc146ef9eaa64baec74ae800bebf8715))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("lotrlu17.100", 0x000000, 0x100000, CRC(c16d3e20) SHA1(43d9f186db361abb3aa119a7252f1bb13bbbbe39))
	ROM_LOAD("lotrlu21.100", 0x100000, 0x100000, CRC(f956a1be) SHA1(2980e85463704a154ed81d3241f866442d1ea2e6))
	ROM_LOAD("lotrlu36.100", 0x200000, 0x100000, CRC(502c3d93) SHA1(4ee42d70bccb9253fe1f8f441de6b5018257f107))
	ROM_LOAD("lotrlu37.100", 0x300000, 0x100000, CRC(61f21c6d) SHA1(3e9781b4981bd18cdb8c59c55b9942de6ae286db))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(lotr_sp401)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "lotrcpul.401",  0x0000, 0x20000, CRC(a9571728) SHA1(f21dd77003f42fafd9293fab3a077c5abf6d572a))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "lotrdspl.403",  0x0000, 0x80000, CRC(6d4075c9) SHA1(7944ba597cb476c33060cead4feaf6dcad4f4b16))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("lotrlu7.100",0x0000,0x10000,CRC(980d970a) SHA1(cf70deddcc146ef9eaa64baec74ae800bebf8715))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("lotrlu17.100", 0x000000, 0x100000, CRC(c16d3e20) SHA1(43d9f186db361abb3aa119a7252f1bb13bbbbe39))
	ROM_LOAD("lotrlu21.100", 0x100000, 0x100000, CRC(f956a1be) SHA1(2980e85463704a154ed81d3241f866442d1ea2e6))
	ROM_LOAD("lotrlu36.100", 0x200000, 0x100000, CRC(502c3d93) SHA1(4ee42d70bccb9253fe1f8f441de6b5018257f107))
	ROM_LOAD("lotrlu37.100", 0x300000, 0x100000, CRC(61f21c6d) SHA1(3e9781b4981bd18cdb8c59c55b9942de6ae286db))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END


ROM_START(lotr_gr)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("lotrcpua.a00", 0x00000, 0x20000, CRC(00e43b70) SHA1(7100da644a1b166051915870d01cfa6baaf87293))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("lotrdspg.a00", 0x00000, 0x80000, CRC(6743a910) SHA1(977773515f00af3937aa59426917e8111ec855ab))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("lotr-u7.101",0x0000,0x10000,CRC(ba018c5c) SHA1(67e4b9729f086de5e8d56a6ac29fce1c7082e470))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("lotr-u17.100", 0x000000, 0x100000, CRC(d503969d) SHA1(65a4313ca1b93391260c65fef6f878d264f9c8ab))
	ROM_LOAD("lotr-u21.100", 0x100000, 0x100000, CRC(8b3f41af) SHA1(9b2d04144edeb499b4ae68a97c65ccb8ef4d26c0))
	ROM_LOAD("lotr-u36.100", 0x200000, 0x100000, CRC(9575981e) SHA1(38083fd923c4a168a94d998ec3c4db42c1e2a2da))
	ROM_LOAD("lotr-u37.100", 0x300000, 0x100000, CRC(8e637a6f) SHA1(8087744ce36fc143381d49a312c98cf38b2f9854))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(lotr_gr9)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("lotrcpu.900",   0x0000, 0x20000, CRC(eff1ab83) SHA1(cd9cfa3fa150224e44078602db7d8bbfe223b926))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "lotrdspg.900",  0x0000, 0x80000, CRC(f5fdd2c2) SHA1(0c5f1b1efe3d38063e2327e2ccfe40936f3988b8))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("lotr-u7.101",0x0000,0x10000,CRC(ba018c5c) SHA1(67e4b9729f086de5e8d56a6ac29fce1c7082e470))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("lotr-u17.100", 0x000000, 0x100000, CRC(d503969d) SHA1(65a4313ca1b93391260c65fef6f878d264f9c8ab))
	ROM_LOAD("lotr-u21.100", 0x100000, 0x100000, CRC(8b3f41af) SHA1(9b2d04144edeb499b4ae68a97c65ccb8ef4d26c0))
	ROM_LOAD("lotr-u36.100", 0x200000, 0x100000, CRC(9575981e) SHA1(38083fd923c4a168a94d998ec3c4db42c1e2a2da))
	ROM_LOAD("lotr-u37.100", 0x300000, 0x100000, CRC(8e637a6f) SHA1(8087744ce36fc143381d49a312c98cf38b2f9854))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(lotr_gr8)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("lotrcpu.800",   0x0000, 0x20000, CRC(655e5b3c) SHA1(bd6fd25e17cee40d6bb842367b1ce922bbd46003))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "lotrdspg.800",  0x0000, 0x80000, CRC(55765c23) SHA1(690a72e8cb1099a6873eb3214e72bb0fea54fa22))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("lotr-u7.101",0x0000,0x10000,CRC(ba018c5c) SHA1(67e4b9729f086de5e8d56a6ac29fce1c7082e470))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("lotr-u17.100", 0x000000, 0x100000, CRC(d503969d) SHA1(65a4313ca1b93391260c65fef6f878d264f9c8ab))
	ROM_LOAD("lotr-u21.100", 0x100000, 0x100000, CRC(8b3f41af) SHA1(9b2d04144edeb499b4ae68a97c65ccb8ef4d26c0))
	ROM_LOAD("lotr-u36.100", 0x200000, 0x100000, CRC(9575981e) SHA1(38083fd923c4a168a94d998ec3c4db42c1e2a2da))
	ROM_LOAD("lotr-u37.100", 0x300000, 0x100000, CRC(8e637a6f) SHA1(8087744ce36fc143381d49a312c98cf38b2f9854))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(lotr_gr7)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("lotrcpu.700",   0x0000, 0x20000, CRC(208a4653) SHA1(570f3070c6b0f128d451f3dea01e41a9944081f2))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "lotrdspg.700",  0x0000, 0x80000, CRC(137f223c) SHA1(bb06a6f587bf86555aea85bc1c0402e2137e1c76))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("lotr-u7.101",0x0000,0x10000,CRC(ba018c5c) SHA1(67e4b9729f086de5e8d56a6ac29fce1c7082e470))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("lotr-u17.100", 0x000000, 0x100000, CRC(d503969d) SHA1(65a4313ca1b93391260c65fef6f878d264f9c8ab))
	ROM_LOAD("lotr-u21.100", 0x100000, 0x100000, CRC(8b3f41af) SHA1(9b2d04144edeb499b4ae68a97c65ccb8ef4d26c0))
	ROM_LOAD("lotr-u36.100", 0x200000, 0x100000, CRC(9575981e) SHA1(38083fd923c4a168a94d998ec3c4db42c1e2a2da))
	ROM_LOAD("lotr-u37.100", 0x300000, 0x100000, CRC(8e637a6f) SHA1(8087744ce36fc143381d49a312c98cf38b2f9854))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(lotr_gr6)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "lotrcpu.600", 0x0000, 0x20000, CRC(02786892) SHA1(6810d5a5eb80f520e611a46921dbd2906fbebf2f))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "lotrdspg.600", 0x0000, 0x80000, CRC(b0de0827) SHA1(1a0aa25a3b881148aafa5e2fabb7a3c501343524))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("lotr-u7.101",0x0000,0x10000,CRC(ba018c5c) SHA1(67e4b9729f086de5e8d56a6ac29fce1c7082e470))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("lotr-u17.100", 0x000000, 0x100000, CRC(d503969d) SHA1(65a4313ca1b93391260c65fef6f878d264f9c8ab))
	ROM_LOAD("lotr-u21.100", 0x100000, 0x100000, CRC(8b3f41af) SHA1(9b2d04144edeb499b4ae68a97c65ccb8ef4d26c0))
	ROM_LOAD("lotr-u36.100", 0x200000, 0x100000, CRC(9575981e) SHA1(38083fd923c4a168a94d998ec3c4db42c1e2a2da))
	ROM_LOAD("lotr-u37.100", 0x300000, 0x100000, CRC(8e637a6f) SHA1(8087744ce36fc143381d49a312c98cf38b2f9854))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(lotr_gr501)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "lotrcpu.501", 0x0000, 0x20000, CRC(df9fd692) SHA1(2d06c1a763330b1b9429961f3e13574e0eefe7a7))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "lotrdspg.501", 0x0000, 0x80000, CRC(16984eaa) SHA1(999254d12402b0866e4a6f5bb2c03dc5c1c59c5f))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("lotr-u7.101",0x0000,0x10000,CRC(ba018c5c) SHA1(67e4b9729f086de5e8d56a6ac29fce1c7082e470))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("lotr-u17.100", 0x000000, 0x100000, CRC(d503969d) SHA1(65a4313ca1b93391260c65fef6f878d264f9c8ab))
	ROM_LOAD("lotr-u21.100", 0x100000, 0x100000, CRC(8b3f41af) SHA1(9b2d04144edeb499b4ae68a97c65ccb8ef4d26c0))
	ROM_LOAD("lotr-u36.100", 0x200000, 0x100000, CRC(9575981e) SHA1(38083fd923c4a168a94d998ec3c4db42c1e2a2da))
	ROM_LOAD("lotr-u37.100", 0x300000, 0x100000, CRC(8e637a6f) SHA1(8087744ce36fc143381d49a312c98cf38b2f9854))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(lotr_gr5)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "lotrcpu.500", 0x0000, 0x20000, CRC(4b45a543) SHA1(e0be3a4244025abc2c109f58b0d637262711b9db))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "lotrdspg.500", 0x0000, 0x80000, CRC(39177315) SHA1(13bcf2833ff89fe056517d3ea7b58fb31963cbfc))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("lotr-u7.101",0x0000,0x10000,CRC(ba018c5c) SHA1(67e4b9729f086de5e8d56a6ac29fce1c7082e470))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("lotr-u17.100", 0x000000, 0x100000, CRC(d503969d) SHA1(65a4313ca1b93391260c65fef6f878d264f9c8ab))
	ROM_LOAD("lotr-u21.100", 0x100000, 0x100000, CRC(8b3f41af) SHA1(9b2d04144edeb499b4ae68a97c65ccb8ef4d26c0))
	ROM_LOAD("lotr-u36.100", 0x200000, 0x100000, CRC(9575981e) SHA1(38083fd923c4a168a94d998ec3c4db42c1e2a2da))
	ROM_LOAD("lotr-u37.100", 0x300000, 0x100000, CRC(8e637a6f) SHA1(8087744ce36fc143381d49a312c98cf38b2f9854))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(lotr_gr410)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "lotrcpu.410", 0x0000, 0x20000, CRC(458af09a) SHA1(2680f16d7f33ffc70b64bfb7d35cccf6989c70e2))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "lotrdspg.404", 0x0000, 0x80000, CRC(b78975e5) SHA1(33d9f4d29a83ce0f68e654c15973dfdeee4d224d))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("lotr-u7.101",0x0000,0x10000,CRC(ba018c5c) SHA1(67e4b9729f086de5e8d56a6ac29fce1c7082e470))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("lotr-u17.100", 0x000000, 0x100000, CRC(d503969d) SHA1(65a4313ca1b93391260c65fef6f878d264f9c8ab))
	ROM_LOAD("lotr-u21.100", 0x100000, 0x100000, CRC(8b3f41af) SHA1(9b2d04144edeb499b4ae68a97c65ccb8ef4d26c0))
	ROM_LOAD("lotr-u36.100", 0x200000, 0x100000, CRC(9575981e) SHA1(38083fd923c4a168a94d998ec3c4db42c1e2a2da))
	ROM_LOAD("lotr-u37.100", 0x300000, 0x100000, CRC(8e637a6f) SHA1(8087744ce36fc143381d49a312c98cf38b2f9854))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(lotr_gr401)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("lotrcpu.401",   0x0000, 0x20000, CRC(b69cdecc) SHA1(40f7c2d25a1028255be8fe25e3aa6d11976edd25))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "lotrdspg.403",  0x0000, 0x80000, CRC(74e925cb) SHA1(2edc8666d53f212a053b7a356d2bf6e3180d7bfb))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("lotr-u7.101",0x0000,0x10000,CRC(ba018c5c) SHA1(67e4b9729f086de5e8d56a6ac29fce1c7082e470))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("lotr-u17.100", 0x000000, 0x100000, CRC(d503969d) SHA1(65a4313ca1b93391260c65fef6f878d264f9c8ab))
	ROM_LOAD("lotr-u21.100", 0x100000, 0x100000, CRC(8b3f41af) SHA1(9b2d04144edeb499b4ae68a97c65ccb8ef4d26c0))
	ROM_LOAD("lotr-u36.100", 0x200000, 0x100000, CRC(9575981e) SHA1(38083fd923c4a168a94d998ec3c4db42c1e2a2da))
	ROM_LOAD("lotr-u37.100", 0x300000, 0x100000, CRC(8e637a6f) SHA1(8087744ce36fc143381d49a312c98cf38b2f9854))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(lotr_fr)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("lotrcpua.a00", 0x00000, 0x20000, CRC(00e43b70) SHA1(7100da644a1b166051915870d01cfa6baaf87293))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("lotrdspf.a00", 0x00000, 0x80000, CRC(15c26c2d) SHA1(c8e4b442d717aa5881f3d92f044c44d29a14126c))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("lotr-u7.101",0x0000,0x10000,CRC(ba018c5c) SHA1(67e4b9729f086de5e8d56a6ac29fce1c7082e470))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("lotr-u17.100", 0x000000, 0x100000, CRC(d503969d) SHA1(65a4313ca1b93391260c65fef6f878d264f9c8ab))
	ROM_LOAD("lotr-u21.100", 0x100000, 0x100000, CRC(8b3f41af) SHA1(9b2d04144edeb499b4ae68a97c65ccb8ef4d26c0))
	ROM_LOAD("lotr-u36.100", 0x200000, 0x100000, CRC(9575981e) SHA1(38083fd923c4a168a94d998ec3c4db42c1e2a2da))
	ROM_LOAD("lotr-u37.100", 0x300000, 0x100000, CRC(8e637a6f) SHA1(8087744ce36fc143381d49a312c98cf38b2f9854))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(lotr_fr9)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("lotrcpu.900",   0x0000, 0x20000, CRC(eff1ab83) SHA1(cd9cfa3fa150224e44078602db7d8bbfe223b926))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "lotrdspf.900",  0x0000, 0x80000, CRC(f2d8296e) SHA1(3eb6e1e6ba299b720816bf165b1e20e02f6c0c1e))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("lotr-u7.101",0x0000,0x10000,CRC(ba018c5c) SHA1(67e4b9729f086de5e8d56a6ac29fce1c7082e470))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("lotr-u17.100", 0x000000, 0x100000, CRC(d503969d) SHA1(65a4313ca1b93391260c65fef6f878d264f9c8ab))
	ROM_LOAD("lotr-u21.100", 0x100000, 0x100000, CRC(8b3f41af) SHA1(9b2d04144edeb499b4ae68a97c65ccb8ef4d26c0))
	ROM_LOAD("lotr-u36.100", 0x200000, 0x100000, CRC(9575981e) SHA1(38083fd923c4a168a94d998ec3c4db42c1e2a2da))
	ROM_LOAD("lotr-u37.100", 0x300000, 0x100000, CRC(8e637a6f) SHA1(8087744ce36fc143381d49a312c98cf38b2f9854))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(lotr_fr8)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("lotrcpu.800",   0x0000, 0x20000, CRC(655e5b3c) SHA1(bd6fd25e17cee40d6bb842367b1ce922bbd46003))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "lotrdspf.800",  0x0000, 0x80000, CRC(e1ccc04b) SHA1(1d5c7ea06f0cb2e1965c968ed01330867aae8e2b))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("lotr-u7.101",0x0000,0x10000,CRC(ba018c5c) SHA1(67e4b9729f086de5e8d56a6ac29fce1c7082e470))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("lotr-u17.100", 0x000000, 0x100000, CRC(d503969d) SHA1(65a4313ca1b93391260c65fef6f878d264f9c8ab))
	ROM_LOAD("lotr-u21.100", 0x100000, 0x100000, CRC(8b3f41af) SHA1(9b2d04144edeb499b4ae68a97c65ccb8ef4d26c0))
	ROM_LOAD("lotr-u36.100", 0x200000, 0x100000, CRC(9575981e) SHA1(38083fd923c4a168a94d998ec3c4db42c1e2a2da))
	ROM_LOAD("lotr-u37.100", 0x300000, 0x100000, CRC(8e637a6f) SHA1(8087744ce36fc143381d49a312c98cf38b2f9854))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(lotr_fr7)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("lotrcpu.700",   0x0000, 0x20000, CRC(208a4653) SHA1(570f3070c6b0f128d451f3dea01e41a9944081f2))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "lotrdspf.700",  0x0000, 0x80000, CRC(c98aeb30) SHA1(16b0ae41db8b4083121cc5ebf2706320d554dd08))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("lotr-u7.101",0x0000,0x10000,CRC(ba018c5c) SHA1(67e4b9729f086de5e8d56a6ac29fce1c7082e470))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("lotr-u17.100", 0x000000, 0x100000, CRC(d503969d) SHA1(65a4313ca1b93391260c65fef6f878d264f9c8ab))
	ROM_LOAD("lotr-u21.100", 0x100000, 0x100000, CRC(8b3f41af) SHA1(9b2d04144edeb499b4ae68a97c65ccb8ef4d26c0))
	ROM_LOAD("lotr-u36.100", 0x200000, 0x100000, CRC(9575981e) SHA1(38083fd923c4a168a94d998ec3c4db42c1e2a2da))
	ROM_LOAD("lotr-u37.100", 0x300000, 0x100000, CRC(8e637a6f) SHA1(8087744ce36fc143381d49a312c98cf38b2f9854))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(lotr_fr6)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "lotrcpu.600", 0x0000, 0x20000, CRC(02786892) SHA1(6810d5a5eb80f520e611a46921dbd2906fbebf2f))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "lotrdspf.600", 0x0000, 0x80000, CRC(5cf6c0b6) SHA1(d7fde5dda4c48da15b682ed9f52d20d8ea2accc9))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("lotr-u7.101",0x0000,0x10000,CRC(ba018c5c) SHA1(67e4b9729f086de5e8d56a6ac29fce1c7082e470))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("lotr-u17.100", 0x000000, 0x100000, CRC(d503969d) SHA1(65a4313ca1b93391260c65fef6f878d264f9c8ab))
	ROM_LOAD("lotr-u21.100", 0x100000, 0x100000, CRC(8b3f41af) SHA1(9b2d04144edeb499b4ae68a97c65ccb8ef4d26c0))
	ROM_LOAD("lotr-u36.100", 0x200000, 0x100000, CRC(9575981e) SHA1(38083fd923c4a168a94d998ec3c4db42c1e2a2da))
	ROM_LOAD("lotr-u37.100", 0x300000, 0x100000, CRC(8e637a6f) SHA1(8087744ce36fc143381d49a312c98cf38b2f9854))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(lotr_fr501)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "lotrcpu.501", 0x0000, 0x20000, CRC(df9fd692) SHA1(2d06c1a763330b1b9429961f3e13574e0eefe7a7))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "lotrdspf.501", 0x0000, 0x80000, CRC(a07596ac) SHA1(99db750971eafc3dba5f5d3e15728de306984ba4))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("lotr-u7.101",0x0000,0x10000,CRC(ba018c5c) SHA1(67e4b9729f086de5e8d56a6ac29fce1c7082e470))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("lotr-u17.100", 0x000000, 0x100000, CRC(d503969d) SHA1(65a4313ca1b93391260c65fef6f878d264f9c8ab))
	ROM_LOAD("lotr-u21.100", 0x100000, 0x100000, CRC(8b3f41af) SHA1(9b2d04144edeb499b4ae68a97c65ccb8ef4d26c0))
	ROM_LOAD("lotr-u36.100", 0x200000, 0x100000, CRC(9575981e) SHA1(38083fd923c4a168a94d998ec3c4db42c1e2a2da))
	ROM_LOAD("lotr-u37.100", 0x300000, 0x100000, CRC(8e637a6f) SHA1(8087744ce36fc143381d49a312c98cf38b2f9854))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(lotr_fr5)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "lotrcpu.500", 0x0000, 0x20000, CRC(4b45a543) SHA1(e0be3a4244025abc2c109f58b0d637262711b9db))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "lotrdspf.500", 0x0000, 0x80000, CRC(ee5768a1) SHA1(05e696bfc4a7630b483f2f9acd39e53fefe937ef))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("lotr-u7.101",0x0000,0x10000,CRC(ba018c5c) SHA1(67e4b9729f086de5e8d56a6ac29fce1c7082e470))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("lotr-u17.100", 0x000000, 0x100000, CRC(d503969d) SHA1(65a4313ca1b93391260c65fef6f878d264f9c8ab))
	ROM_LOAD("lotr-u21.100", 0x100000, 0x100000, CRC(8b3f41af) SHA1(9b2d04144edeb499b4ae68a97c65ccb8ef4d26c0))
	ROM_LOAD("lotr-u36.100", 0x200000, 0x100000, CRC(9575981e) SHA1(38083fd923c4a168a94d998ec3c4db42c1e2a2da))
	ROM_LOAD("lotr-u37.100", 0x300000, 0x100000, CRC(8e637a6f) SHA1(8087744ce36fc143381d49a312c98cf38b2f9854))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(lotr_fr410)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "lotrcpu.410", 0x0000, 0x20000, CRC(458af09a) SHA1(2680f16d7f33ffc70b64bfb7d35cccf6989c70e2))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "lotrdspf.404", 0x0000, 0x80000, CRC(ebf4bb43) SHA1(5e392c3363db3d56b2ec66fcc43a59b5e8cdf944))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("lotr-u7.101",0x0000,0x10000,CRC(ba018c5c) SHA1(67e4b9729f086de5e8d56a6ac29fce1c7082e470))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("lotr-u17.100", 0x000000, 0x100000, CRC(d503969d) SHA1(65a4313ca1b93391260c65fef6f878d264f9c8ab))
	ROM_LOAD("lotr-u21.100", 0x100000, 0x100000, CRC(8b3f41af) SHA1(9b2d04144edeb499b4ae68a97c65ccb8ef4d26c0))
	ROM_LOAD("lotr-u36.100", 0x200000, 0x100000, CRC(9575981e) SHA1(38083fd923c4a168a94d998ec3c4db42c1e2a2da))
	ROM_LOAD("lotr-u37.100", 0x300000, 0x100000, CRC(8e637a6f) SHA1(8087744ce36fc143381d49a312c98cf38b2f9854))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(lotr_fr401)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("lotrcpu.401",   0x0000, 0x20000, CRC(b69cdecc) SHA1(40f7c2d25a1028255be8fe25e3aa6d11976edd25))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "lotrdspf.403",  0x0000, 0x80000, CRC(d02a77cf) SHA1(8cf4312a04ad486714de5c0041cacb1eb475478f))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("lotr-u7.101",0x0000,0x10000,CRC(ba018c5c) SHA1(67e4b9729f086de5e8d56a6ac29fce1c7082e470))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("lotr-u17.100", 0x000000, 0x100000, CRC(d503969d) SHA1(65a4313ca1b93391260c65fef6f878d264f9c8ab))
	ROM_LOAD("lotr-u21.100", 0x100000, 0x100000, CRC(8b3f41af) SHA1(9b2d04144edeb499b4ae68a97c65ccb8ef4d26c0))
	ROM_LOAD("lotr-u36.100", 0x200000, 0x100000, CRC(9575981e) SHA1(38083fd923c4a168a94d998ec3c4db42c1e2a2da))
	ROM_LOAD("lotr-u37.100", 0x300000, 0x100000, CRC(8e637a6f) SHA1(8087744ce36fc143381d49a312c98cf38b2f9854))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(lotr_it)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("lotrcpua.a00", 0x00000, 0x20000, CRC(00e43b70) SHA1(7100da644a1b166051915870d01cfa6baaf87293))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("lotrdspi.a00", 0x00000, 0x80000, CRC(6c88f395) SHA1(365d5c6908f5861816b73f287194c85d2300635d))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("lotr-u7.101",0x0000,0x10000,CRC(ba018c5c) SHA1(67e4b9729f086de5e8d56a6ac29fce1c7082e470))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("lotr-u17.100", 0x000000, 0x100000, CRC(d503969d) SHA1(65a4313ca1b93391260c65fef6f878d264f9c8ab))
	ROM_LOAD("lotr-u21.100", 0x100000, 0x100000, CRC(8b3f41af) SHA1(9b2d04144edeb499b4ae68a97c65ccb8ef4d26c0))
	ROM_LOAD("lotr-u36.100", 0x200000, 0x100000, CRC(9575981e) SHA1(38083fd923c4a168a94d998ec3c4db42c1e2a2da))
	ROM_LOAD("lotr-u37.100", 0x300000, 0x100000, CRC(8e637a6f) SHA1(8087744ce36fc143381d49a312c98cf38b2f9854))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(lotr_it9)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("lotrcpu.900",   0x0000, 0x20000, CRC(eff1ab83) SHA1(cd9cfa3fa150224e44078602db7d8bbfe223b926))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "lotrdspi.900",  0x0000, 0x80000, CRC(a09407d7) SHA1(2cdb70ee0bae7f67f4bf12b0dd3e6cf574087e3d))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("lotr-u7.101",0x0000,0x10000,CRC(ba018c5c) SHA1(67e4b9729f086de5e8d56a6ac29fce1c7082e470))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("lotr-u17.100", 0x000000, 0x100000, CRC(d503969d) SHA1(65a4313ca1b93391260c65fef6f878d264f9c8ab))
	ROM_LOAD("lotr-u21.100", 0x100000, 0x100000, CRC(8b3f41af) SHA1(9b2d04144edeb499b4ae68a97c65ccb8ef4d26c0))
	ROM_LOAD("lotr-u36.100", 0x200000, 0x100000, CRC(9575981e) SHA1(38083fd923c4a168a94d998ec3c4db42c1e2a2da))
	ROM_LOAD("lotr-u37.100", 0x300000, 0x100000, CRC(8e637a6f) SHA1(8087744ce36fc143381d49a312c98cf38b2f9854))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(lotr_it8)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("lotrcpu.800",   0x0000, 0x20000, CRC(655e5b3c) SHA1(bd6fd25e17cee40d6bb842367b1ce922bbd46003))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "lotrdspi.800",  0x0000, 0x80000, CRC(b80730d7) SHA1(552c0bfac1c7a6b246829378a30d58769e695f7e))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("lotr-u7.101",0x0000,0x10000,CRC(ba018c5c) SHA1(67e4b9729f086de5e8d56a6ac29fce1c7082e470))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("lotr-u17.100", 0x000000, 0x100000, CRC(d503969d) SHA1(65a4313ca1b93391260c65fef6f878d264f9c8ab))
	ROM_LOAD("lotr-u21.100", 0x100000, 0x100000, CRC(8b3f41af) SHA1(9b2d04144edeb499b4ae68a97c65ccb8ef4d26c0))
	ROM_LOAD("lotr-u36.100", 0x200000, 0x100000, CRC(9575981e) SHA1(38083fd923c4a168a94d998ec3c4db42c1e2a2da))
	ROM_LOAD("lotr-u37.100", 0x300000, 0x100000, CRC(8e637a6f) SHA1(8087744ce36fc143381d49a312c98cf38b2f9854))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(lotr_it7)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("lotrcpu.700",   0x0000, 0x20000, CRC(208a4653) SHA1(570f3070c6b0f128d451f3dea01e41a9944081f2))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "lotrdspi.700",  0x0000, 0x80000, CRC(6a0d2a6d) SHA1(2b3ca8b26d79919b7102c60515972ab142d1cbf1))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("lotr-u7.101",0x0000,0x10000,CRC(ba018c5c) SHA1(67e4b9729f086de5e8d56a6ac29fce1c7082e470))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("lotr-u17.100", 0x000000, 0x100000, CRC(d503969d) SHA1(65a4313ca1b93391260c65fef6f878d264f9c8ab))
	ROM_LOAD("lotr-u21.100", 0x100000, 0x100000, CRC(8b3f41af) SHA1(9b2d04144edeb499b4ae68a97c65ccb8ef4d26c0))
	ROM_LOAD("lotr-u36.100", 0x200000, 0x100000, CRC(9575981e) SHA1(38083fd923c4a168a94d998ec3c4db42c1e2a2da))
	ROM_LOAD("lotr-u37.100", 0x300000, 0x100000, CRC(8e637a6f) SHA1(8087744ce36fc143381d49a312c98cf38b2f9854))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(lotr_it6)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "lotrcpu.600", 0x0000, 0x20000, CRC(02786892) SHA1(6810d5a5eb80f520e611a46921dbd2906fbebf2f))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "lotrdspi.600", 0x0000, 0x80000, CRC(9d0b9b3d) SHA1(21363ddbb2c2510fcc9386020f2fd3f49e9c49c3))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("lotr-u7.101",0x0000,0x10000,CRC(ba018c5c) SHA1(67e4b9729f086de5e8d56a6ac29fce1c7082e470))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("lotr-u17.100", 0x000000, 0x100000, CRC(d503969d) SHA1(65a4313ca1b93391260c65fef6f878d264f9c8ab))
	ROM_LOAD("lotr-u21.100", 0x100000, 0x100000, CRC(8b3f41af) SHA1(9b2d04144edeb499b4ae68a97c65ccb8ef4d26c0))
	ROM_LOAD("lotr-u36.100", 0x200000, 0x100000, CRC(9575981e) SHA1(38083fd923c4a168a94d998ec3c4db42c1e2a2da))
	ROM_LOAD("lotr-u37.100", 0x300000, 0x100000, CRC(8e637a6f) SHA1(8087744ce36fc143381d49a312c98cf38b2f9854))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(lotr_it501)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "lotrcpu.501", 0x0000, 0x20000, CRC(df9fd692) SHA1(2d06c1a763330b1b9429961f3e13574e0eefe7a7))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "lotrdspi.501", 0x0000, 0x80000, CRC(440bbba1) SHA1(e85eca9a1b04ba8bc2784414f7003674bcafba9d))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("lotr-u7.101",0x0000,0x10000,CRC(ba018c5c) SHA1(67e4b9729f086de5e8d56a6ac29fce1c7082e470))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("lotr-u17.100", 0x000000, 0x100000, CRC(d503969d) SHA1(65a4313ca1b93391260c65fef6f878d264f9c8ab))
	ROM_LOAD("lotr-u21.100", 0x100000, 0x100000, CRC(8b3f41af) SHA1(9b2d04144edeb499b4ae68a97c65ccb8ef4d26c0))
	ROM_LOAD("lotr-u36.100", 0x200000, 0x100000, CRC(9575981e) SHA1(38083fd923c4a168a94d998ec3c4db42c1e2a2da))
	ROM_LOAD("lotr-u37.100", 0x300000, 0x100000, CRC(8e637a6f) SHA1(8087744ce36fc143381d49a312c98cf38b2f9854))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(lotr_it5)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "lotrcpu.500", 0x0000, 0x20000, CRC(4b45a543) SHA1(e0be3a4244025abc2c109f58b0d637262711b9db))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "lotrdspi.500", 0x0000, 0x80000, CRC(ea8d4ac6) SHA1(f46e8c3f344babc67e72f7077880c21df0c42030))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("lotr-u7.101",0x0000,0x10000,CRC(ba018c5c) SHA1(67e4b9729f086de5e8d56a6ac29fce1c7082e470))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("lotr-u17.100", 0x000000, 0x100000, CRC(d503969d) SHA1(65a4313ca1b93391260c65fef6f878d264f9c8ab))
	ROM_LOAD("lotr-u21.100", 0x100000, 0x100000, CRC(8b3f41af) SHA1(9b2d04144edeb499b4ae68a97c65ccb8ef4d26c0))
	ROM_LOAD("lotr-u36.100", 0x200000, 0x100000, CRC(9575981e) SHA1(38083fd923c4a168a94d998ec3c4db42c1e2a2da))
	ROM_LOAD("lotr-u37.100", 0x300000, 0x100000, CRC(8e637a6f) SHA1(8087744ce36fc143381d49a312c98cf38b2f9854))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(lotr_it410)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "lotrcpu.410", 0x0000, 0x20000, CRC(458af09a) SHA1(2680f16d7f33ffc70b64bfb7d35cccf6989c70e2))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "lotrdspi.404", 0x0000, 0x80000, CRC(05db2615) SHA1(0146abd3681d351ef6c1160b85be8bed2886fb27))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("lotr-u7.101",0x0000,0x10000,CRC(ba018c5c) SHA1(67e4b9729f086de5e8d56a6ac29fce1c7082e470))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("lotr-u17.100", 0x000000, 0x100000, CRC(d503969d) SHA1(65a4313ca1b93391260c65fef6f878d264f9c8ab))
	ROM_LOAD("lotr-u21.100", 0x100000, 0x100000, CRC(8b3f41af) SHA1(9b2d04144edeb499b4ae68a97c65ccb8ef4d26c0))
	ROM_LOAD("lotr-u36.100", 0x200000, 0x100000, CRC(9575981e) SHA1(38083fd923c4a168a94d998ec3c4db42c1e2a2da))
	ROM_LOAD("lotr-u37.100", 0x300000, 0x100000, CRC(8e637a6f) SHA1(8087744ce36fc143381d49a312c98cf38b2f9854))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(lotr_it401)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("lotrcpu.401",   0x0000, 0x20000, CRC(b69cdecc) SHA1(40f7c2d25a1028255be8fe25e3aa6d11976edd25))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "lotrdspi.403",  0x0000, 0x80000, CRC(5922ce10) SHA1(c57f2de4e3344f16056405d71510c0c0b60ef86d)) // ver 4.03

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("lotr-u7.101",0x0000,0x10000,CRC(ba018c5c) SHA1(67e4b9729f086de5e8d56a6ac29fce1c7082e470))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("lotr-u17.100", 0x000000, 0x100000, CRC(d503969d) SHA1(65a4313ca1b93391260c65fef6f878d264f9c8ab))
	ROM_LOAD("lotr-u21.100", 0x100000, 0x100000, CRC(8b3f41af) SHA1(9b2d04144edeb499b4ae68a97c65ccb8ef4d26c0))
	ROM_LOAD("lotr-u36.100", 0x200000, 0x100000, CRC(9575981e) SHA1(38083fd923c4a168a94d998ec3c4db42c1e2a2da))
	ROM_LOAD("lotr-u37.100", 0x300000, 0x100000, CRC(8e637a6f) SHA1(8087744ce36fc143381d49a312c98cf38b2f9854))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(lotr_le)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("lotrcpua.a02", 0x00000, 0x20000, CRC(f732aeb1) SHA1(8c28e91d37525b3e356bddf2cf1be42ea44f3629))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("lotrdspa.a00", 0x00000, 0x80000, CRC(99634603) SHA1(c40d1480e5df10a491bcd471c6a3a118a9120bcb))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("bios.u8", 0x400000, 0x200000, CRC(c049bf99) SHA1(89e76760e8010fe642ed4312732de81c8a25f548))
	ROM_RELOAD(0x0,0x200000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("lotr-u7.101",0x0000,0x10000,CRC(ba018c5c) SHA1(67e4b9729f086de5e8d56a6ac29fce1c7082e470))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("lotr-u17.100", 0x000000, 0x100000, CRC(d503969d) SHA1(65a4313ca1b93391260c65fef6f878d264f9c8ab))
	ROM_LOAD("lotr-u21.100", 0x100000, 0x100000, CRC(8b3f41af) SHA1(9b2d04144edeb499b4ae68a97c65ccb8ef4d26c0))
	ROM_LOAD("lotr-u36.100", 0x200000, 0x100000, CRC(9575981e) SHA1(38083fd923c4a168a94d998ec3c4db42c1e2a2da))
	ROM_LOAD("lotr-u37.100", 0x300000, 0x100000, CRC(8e637a6f) SHA1(8087744ce36fc143381d49a312c98cf38b2f9854))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

/*-------------------------------------------------------------------
/ The Sopranos
/-------------------------------------------------------------------*/
ROM_START(sopranos)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("sopcpua.500", 0x00000, 0x20000, CRC(e3430f28) SHA1(3b942922087cca41701ef70fbc84baaffe8da90d))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("sopdspa.500", 0x00000, 0x80000, CRC(170bd8d1) SHA1(df8d240425ac2c1aa4bea560ecdd3d64120faeb7))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("biosv8.u8", 0x400000, 0x20000, CRC(c81b5532) SHA1(7bd630179910423a01394f82ad7ff2008c02f7ff))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("sopsnda.u7",0x0000,0x10000,CRC(47817240) SHA1(da2ff205fb5fe732514e7aa312ff552ecd4b31b7))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("sopsnda.u17", 0x000000, 0x100000, CRC(21e0cfd2) SHA1(d2ff1242f1f4a206e0b2884c079ef2be5df143ac))
	ROM_LOAD("sopsnda.u21", 0x100000, 0x100000, CRC(10669399) SHA1(6ad31c91ba3c772f7a79e4408a4422332243c7d1))
	ROM_LOAD("sopsnda.u36", 0x200000, 0x100000, CRC(f3535025) SHA1(3fc22af5db0a8b744c0513e24a6331c9cf82e3ed))
	ROM_LOAD("sopsnda.u37", 0x300000, 0x100000, CRC(4b67fe8a) SHA1(b980b9705b4a41a0524b3b0095d6398bdbed609f))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(sopranos_400)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("sopcpua.400", 0x00000, 0x20000, CRC(68efcf24) SHA1(9ef30808260f96fb19067ee473add0c43dd6180e))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("sopdspa.400", 0x00000, 0x80000, CRC(60d6b9d3) SHA1(925d2c84e486e4a71bd05b542429a0e22a99072f))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("biosv8.u8", 0x400000, 0x20000, CRC(c81b5532) SHA1(7bd630179910423a01394f82ad7ff2008c02f7ff))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("sopsnd3.u7", 0x0000,0x10000,CRC(b22ba5aa) SHA1(8f69932e3b115ae7a6bcb9a15a8b5bf6579e94e0)) // sldh

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("sopsnda.u17", 0x000000, 0x100000, CRC(21e0cfd2) SHA1(d2ff1242f1f4a206e0b2884c079ef2be5df143ac))
	ROM_LOAD("sopsnda.u21", 0x100000, 0x100000, CRC(257ab09d) SHA1(1d18e279139b1658ce02160d9a37b4bf043393f0)) // sldh
	ROM_LOAD("sopsnda.u36", 0x200000, 0x100000, CRC(db33b45c) SHA1(d3285008a3c770371389be470c1ec5ca49c1e568)) // sldh
	ROM_LOAD("sopsnda.u37", 0x300000, 0x100000, CRC(06a2a6e1) SHA1(fdbe622223724ac2b4c5183c43d3e635654864bf)) // sldh

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(sopranos_300)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("sopcpua.300", 0x00000, 0x20000, CRC(cca0e551) SHA1(ef109b871605879b55abb966d6736d7deeca404f))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("sopdspa.300", 0x00000, 0x80000, CRC(aa6306ac) SHA1(80737bd2b93bfc64490d07d2cd482350ed3303b3))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("biosv8.u8", 0x400000, 0x20000, CRC(c81b5532) SHA1(7bd630179910423a01394f82ad7ff2008c02f7ff))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("sopsnd3.u7", 0x0000,0x10000,CRC(b22ba5aa) SHA1(8f69932e3b115ae7a6bcb9a15a8b5bf6579e94e0)) // sldh

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("sopsnda.u17", 0x000000, 0x100000, CRC(21e0cfd2) SHA1(d2ff1242f1f4a206e0b2884c079ef2be5df143ac))
	ROM_LOAD("sopsnda.u21", 0x100000, 0x100000, CRC(257ab09d) SHA1(1d18e279139b1658ce02160d9a37b4bf043393f0)) // sldh
	ROM_LOAD("sopsnda.u36", 0x200000, 0x100000, CRC(db33b45c) SHA1(d3285008a3c770371389be470c1ec5ca49c1e568)) // sldh
	ROM_LOAD("sopsnda.u37", 0x300000, 0x100000, CRC(06a2a6e1) SHA1(fdbe622223724ac2b4c5183c43d3e635654864bf)) // sldh

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(sopranos_204)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "sopcpua.204",   0x0000, 0x20000, CRC(95251d1e) SHA1(c22754647afd07f42bb6b2d0944f696922e68feb))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "sopdspa.200",   0x0000, 0x80000, CRC(e5de9a5d) SHA1(6e18d4bdf2d35c9c0743fa6f91f540686d0a706b))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("biosv8.u8", 0x400000, 0x20000, CRC(c81b5532) SHA1(7bd630179910423a01394f82ad7ff2008c02f7ff))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("sopsnd3.u7", 0x0000,0x10000,CRC(4f6748b5) SHA1(63e953a1455dee2a44484fef951fa34cb2e55d7b)) // sldh

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("sopsnd1.u17", 0x000000, 0x100000, CRC(1ecc5ecc) SHA1(42897387b90df8da8ae556ccc46e281ca461c063))
	ROM_LOAD("sopsnd1.u21", 0x100000, 0x100000, CRC(257ab09d) SHA1(1d18e279139b1658ce02160d9a37b4bf043393f0))
	ROM_LOAD("sopsnd1.u36", 0x200000, 0x100000, CRC(db33b45c) SHA1(d3285008a3c770371389be470c1ec5ca49c1e568))
	ROM_LOAD("sopsnd1.u37", 0x300000, 0x100000, CRC(06a2a6e1) SHA1(fdbe622223724ac2b4c5183c43d3e635654864bf))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(sopranog)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("sopcpug.500", 0x00000, 0x20000, CRC(e6317603) SHA1(2e7f49cd77f65d0af08ab503fc82ec56c8890926))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("sopdspg.500", 0x00000, 0x80000, CRC(d8f365e9) SHA1(395209169e023913bf0bf3c3837e9a1b6b636e75))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("biosv8.u8", 0x400000, 0x20000, CRC(c81b5532) SHA1(7bd630179910423a01394f82ad7ff2008c02f7ff))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("sopsndg.u7",0x0000,0x10000,CRC(bb615e03) SHA1(ce5ef766376c060fc071d54aa878d69b3aa30294))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("sopsndg.u17", 0x000000, 0x100000, CRC(cfa7fca1) SHA1(2efbc8c3e8ad6dc39973908e37ecdc7b02be720a))
	ROM_LOAD("sopsndg.u21", 0x100000, 0x100000, CRC(7669769b) SHA1(adf0c0dbfbaa981fa90db4e54702a4dbaf474e82))
	ROM_LOAD("sopsndg.u36", 0x200000, 0x100000, CRC(08d715b5) SHA1(ddccd311ba2b608ab0845afb3ef63b8d3425d530))
	ROM_LOAD("sopsndg.u37", 0x300000, 0x100000, CRC(2405df73) SHA1(b8074610d9d87d3f1c0244ef0f450c766aac8a20))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(sopranog_400)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "sopcpug.400", 0x0000, 0x20000, CRC(44bd5b83) SHA1(edd11ee10a3bba9055363919f317421dee84cd85))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "sopdspg.400", 0x0000, 0x80000, CRC(2672ef2c) SHA1(8e042b6a98edd8d7b7682d01914d8d021f526b35))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("biosv8.u8", 0x400000, 0x20000, CRC(c81b5532) SHA1(7bd630179910423a01394f82ad7ff2008c02f7ff))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("sopsndg.u7",0x0000,0x10000,CRC(bb615e03) SHA1(ce5ef766376c060fc071d54aa878d69b3aa30294))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("sopsndg.u17", 0x000000, 0x100000, CRC(cfa7fca1) SHA1(2efbc8c3e8ad6dc39973908e37ecdc7b02be720a))
	ROM_LOAD("sopsndg.u21", 0x100000, 0x100000, CRC(7669769b) SHA1(adf0c0dbfbaa981fa90db4e54702a4dbaf474e82))
	ROM_LOAD("sopsndg.u36", 0x200000, 0x100000, CRC(08d715b5) SHA1(ddccd311ba2b608ab0845afb3ef63b8d3425d530))
	ROM_LOAD("sopsndg.u37", 0x300000, 0x100000, CRC(2405df73) SHA1(b8074610d9d87d3f1c0244ef0f450c766aac8a20))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(sopranog_300)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "sopcpug.300",   0x0000, 0x20000, CRC(acfa9c83) SHA1(8a31b1008097ff86930b2c946c2777511efc8f9a))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "sopdspg.300",   0x0000, 0x80000, CRC(9fa4f9d6) SHA1(86af57435d3d33f8686a56ac59e411f2cb69f565))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("biosv8.u8", 0x400000, 0x20000, CRC(c81b5532) SHA1(7bd630179910423a01394f82ad7ff2008c02f7ff))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("sopsndg.u7",0x0000,0x10000,CRC(bb615e03) SHA1(ce5ef766376c060fc071d54aa878d69b3aa30294))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("sopsndg.u17", 0x000000, 0x100000, CRC(cfa7fca1) SHA1(2efbc8c3e8ad6dc39973908e37ecdc7b02be720a))
	ROM_LOAD("sopsndg.u21", 0x100000, 0x100000, CRC(7669769b) SHA1(adf0c0dbfbaa981fa90db4e54702a4dbaf474e82))
	ROM_LOAD("sopsndg.u36", 0x200000, 0x100000, CRC(08d715b5) SHA1(ddccd311ba2b608ab0845afb3ef63b8d3425d530))
	ROM_LOAD("sopsndg.u37", 0x300000, 0x100000, CRC(2405df73) SHA1(b8074610d9d87d3f1c0244ef0f450c766aac8a20))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(sopranog_107)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "sopcpug.107",   0x0000, 0x20000, CRC(e9c83725) SHA1(958155919cbb347b72784c7da112b188e06c908f))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "sopdspg.100",   0x0000, 0x80000, CRC(38625560) SHA1(c41a6808fe05cafe44ea5026689b2ea6eb195e41))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("biosv8.u8", 0x400000, 0x20000, CRC(c81b5532) SHA1(7bd630179910423a01394f82ad7ff2008c02f7ff))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("sopsndg.u7",0x0000,0x10000,CRC(bb615e03) SHA1(ce5ef766376c060fc071d54aa878d69b3aa30294))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("sopsndg.u17", 0x000000, 0x100000, CRC(cfa7fca1) SHA1(2efbc8c3e8ad6dc39973908e37ecdc7b02be720a))
	ROM_LOAD("sopsndg1.u21", 0x100000, 0x100000, CRC(caae114a) SHA1(84703649d7ba05d011348f84e4cac31a023146c0))
	ROM_LOAD("sopsndg.u36", 0x200000, 0x100000, CRC(08d715b5) SHA1(ddccd311ba2b608ab0845afb3ef63b8d3425d530))
	ROM_LOAD("sopsndg.u37", 0x300000, 0x100000, CRC(2405df73) SHA1(b8074610d9d87d3f1c0244ef0f450c766aac8a20))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END


ROM_START(sopranof)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("sopcpuf.500", 0x00000, 0x20000, CRC(d047d4bd) SHA1(588ba8f6c7af32f6aa94f29e59ad2bcd2bc64968))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("sopdspf.500", 0x00000, 0x80000, CRC(e4252fb5) SHA1(be54181af8f3650023f20cf1bf3b8b0310adb1bb))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("biosv8.u8", 0x400000, 0x20000, CRC(c81b5532) SHA1(7bd630179910423a01394f82ad7ff2008c02f7ff))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("sopsndf.u7",0x0000,0x10000,CRC(57426738) SHA1(393e1d654ef09172580ad9a2722f696b6e44ec0f))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("sopsndf.u17", 0x000000, 0x100000, CRC(0e34f843) SHA1(2a026bda4c44803239a93e6603a4dfb25e3103b5))
	ROM_LOAD("sopsndf.u21", 0x100000, 0x100000, CRC(28726d20) SHA1(63c6bea953cc34b6a3c2c9688ab86641f94cd227))
	ROM_LOAD("sopsndf.u36", 0x200000, 0x100000, CRC(99549d4a) SHA1(15e3d35b9cefbc8825a7dee5309adc2526de3dec))
	ROM_LOAD("sopsndf.u37", 0x300000, 0x100000, CRC(2b4a9130) SHA1(eed9c84c932bb86954226b0d51461c5094ebe02e))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(sopranof_400)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "sopcpuf.400", 0x0000, 0x20000, CRC(d6f770cd) SHA1(35f35bb91c1444ba42e29542213745b7e90c9b27))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "sopdspf.400", 0x0000, 0x80000, CRC(df451810) SHA1(3c396cac89c57dbacde9b82681dd5600616d6d93))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("biosv8.u8", 0x400000, 0x20000, CRC(c81b5532) SHA1(7bd630179910423a01394f82ad7ff2008c02f7ff))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("sopsndf.u7",0x0000,0x10000,CRC(57426738) SHA1(393e1d654ef09172580ad9a2722f696b6e44ec0f))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("sopsndf.u17", 0x000000, 0x100000, CRC(0e34f843) SHA1(2a026bda4c44803239a93e6603a4dfb25e3103b5))
	ROM_LOAD("sopsndf.u21", 0x100000, 0x100000, CRC(28726d20) SHA1(63c6bea953cc34b6a3c2c9688ab86641f94cd227))
	ROM_LOAD("sopsndf.u36", 0x200000, 0x100000, CRC(99549d4a) SHA1(15e3d35b9cefbc8825a7dee5309adc2526de3dec))
	ROM_LOAD("sopsndf.u37", 0x300000, 0x100000, CRC(2b4a9130) SHA1(eed9c84c932bb86954226b0d51461c5094ebe02e))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(sopranof_300)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "sopcpuf.300",   0x0000, 0x20000, CRC(0bd0f8c4) SHA1(df32dcf1db996203fc8b50c78e973b784ae58dbd))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "sopdspf.300",   0x0000, 0x80000, CRC(693bd940) SHA1(dd277da4e8239ae5ede3ded37efc8377ba85919a))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("biosv8.u8", 0x400000, 0x20000, CRC(c81b5532) SHA1(7bd630179910423a01394f82ad7ff2008c02f7ff))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("sopsndf.u7",0x0000,0x10000,CRC(57426738) SHA1(393e1d654ef09172580ad9a2722f696b6e44ec0f))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("sopsndf.u17", 0x000000, 0x100000, CRC(0e34f843) SHA1(2a026bda4c44803239a93e6603a4dfb25e3103b5))
	ROM_LOAD("sopsndf.u21", 0x100000, 0x100000, CRC(28726d20) SHA1(63c6bea953cc34b6a3c2c9688ab86641f94cd227))
	ROM_LOAD("sopsndf.u36", 0x200000, 0x100000, CRC(99549d4a) SHA1(15e3d35b9cefbc8825a7dee5309adc2526de3dec))
	ROM_LOAD("sopsndf.u37", 0x300000, 0x100000, CRC(2b4a9130) SHA1(eed9c84c932bb86954226b0d51461c5094ebe02e))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(sopranof_107)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "sopcpuf.107",   0x0000, 0x20000, CRC(1cc86040) SHA1(0b4322eca8a5be7ea92356adf65b6c6c6f4205ca))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("sopdspf.100", 0x00000, 0x80000, CRC(18c36c19) SHA1(2b2e5cb00b92d7c8875de2d2d19b82305d9fb27f))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("biosv8.u8", 0x400000, 0x20000, CRC(c81b5532) SHA1(7bd630179910423a01394f82ad7ff2008c02f7ff))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("sopsndf.u7",0x0000,0x10000,CRC(57426738) SHA1(393e1d654ef09172580ad9a2722f696b6e44ec0f))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("sopsndf1.u17",0x000000, 0x100000, CRC(9e0dd4a8) SHA1(82b772eb7081f22f1203ed113ec7b05f2e26258c))
	ROM_LOAD("sopsndf.u21", 0x100000, 0x100000, CRC(28726d20) SHA1(63c6bea953cc34b6a3c2c9688ab86641f94cd227))
	ROM_LOAD("sopsndf.u36", 0x200000, 0x100000, CRC(99549d4a) SHA1(15e3d35b9cefbc8825a7dee5309adc2526de3dec))
	ROM_LOAD("sopsndf.u37", 0x300000, 0x100000, CRC(2b4a9130) SHA1(eed9c84c932bb86954226b0d51461c5094ebe02e))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(sopranol)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("sopcpul.500", 0x00000, 0x20000, CRC(ba978d00) SHA1(afe561cc439d9e51dee0697f423fce103448504c))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("sopdspl.500", 0x00000, 0x80000, CRC(a4100c9e) SHA1(08ea2424ff315f6174d56301c7a8164a32629367))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("biosv8.u8", 0x400000, 0x20000, CRC(c81b5532) SHA1(7bd630179910423a01394f82ad7ff2008c02f7ff))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("sopsndl.u7",0x0000,0x10000,CRC(137110f2) SHA1(9bd911fc74b91e811ada4c66bec214d22506a646))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("sopsndl.u17", 0x000000, 0x100000, CRC(3d5189e6) SHA1(7d846d0b18678ff7aa44029643571e237bc48d58))
	ROM_LOAD("sopsndl.u21", 0x100000, 0x100000, CRC(0ac4f407) SHA1(9473ab42c0f758901644256d7cd1cb47c8396433))
	ROM_LOAD("sopsndl.u36", 0x200000, 0x100000, CRC(147c4216) SHA1(ded2917188bea51cb03db72fe53fcd76a3e66ab9))
	ROM_LOAD("sopsndl.u37", 0x300000, 0x100000, CRC(cfe814fb) SHA1(51b6b10dda4640f8569e610b41c77e3657eabff2))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(sopranol_400)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "sopcpul.400", 0x0000, 0x20000, CRC(7fe54359) SHA1(cb00318484ad1e30ab86c3a239fd2ea322aa945e))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "sopdspl.400", 0x0000, 0x80000, CRC(4fbef543) SHA1(3ffb48031451d3b318f88bfab4d92d2903993492))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("biosv8.u8", 0x400000, 0x20000, CRC(c81b5532) SHA1(7bd630179910423a01394f82ad7ff2008c02f7ff))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("sopsndl.u7",0x0000,0x10000,CRC(137110f2) SHA1(9bd911fc74b91e811ada4c66bec214d22506a646))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("sopsndl.u17", 0x000000, 0x100000, CRC(3d5189e6) SHA1(7d846d0b18678ff7aa44029643571e237bc48d58))
	ROM_LOAD("sopsndl.u21", 0x100000, 0x100000, CRC(0ac4f407) SHA1(9473ab42c0f758901644256d7cd1cb47c8396433))
	ROM_LOAD("sopsndl.u36", 0x200000, 0x100000, CRC(147c4216) SHA1(ded2917188bea51cb03db72fe53fcd76a3e66ab9))
	ROM_LOAD("sopsndl.u37", 0x300000, 0x100000, CRC(cfe814fb) SHA1(51b6b10dda4640f8569e610b41c77e3657eabff2))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(sopranol_300)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "sopcpul.300",   0x0000, 0x20000, CRC(2efd7a45) SHA1(d179605c385c3e3b269bb81295f79a52e0f3f627))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "sopdspl.300",   0x0000, 0x80000, CRC(d6f7a723) SHA1(462c8c82ffb6e386adfc411d3e70c4b25553dc7a))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("biosv8.u8", 0x400000, 0x20000, CRC(c81b5532) SHA1(7bd630179910423a01394f82ad7ff2008c02f7ff))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("sopsndl.u7",0x0000,0x10000,CRC(137110f2) SHA1(9bd911fc74b91e811ada4c66bec214d22506a646))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("sopsndl.u17", 0x000000, 0x100000, CRC(3d5189e6) SHA1(7d846d0b18678ff7aa44029643571e237bc48d58))
	ROM_LOAD("sopsndl.u21", 0x100000, 0x100000, CRC(0ac4f407) SHA1(9473ab42c0f758901644256d7cd1cb47c8396433))
	ROM_LOAD("sopsndl.u36", 0x200000, 0x100000, CRC(147c4216) SHA1(ded2917188bea51cb03db72fe53fcd76a3e66ab9))
	ROM_LOAD("sopsndl.u37", 0x300000, 0x100000, CRC(cfe814fb) SHA1(51b6b10dda4640f8569e610b41c77e3657eabff2))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(sopranol_107)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "sopcpul.107",   0x0000, 0x20000, CRC(a08311fe) SHA1(93e3ecc3e2c69f30d0fbb72c7426f3c2ba4b27b4))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "sopdspl.100",   0x0000, 0x80000, CRC(1f52723e) SHA1(c972252a139c871e4bbbf20382ceb738b84f9a18))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("biosv8.u8", 0x400000, 0x20000, CRC(c81b5532) SHA1(7bd630179910423a01394f82ad7ff2008c02f7ff))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("sopsndl.u7",0x0000,0x10000,CRC(137110f2) SHA1(9bd911fc74b91e811ada4c66bec214d22506a646))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("sopsndl.u17", 0x000000, 0x100000, CRC(3d5189e6) SHA1(7d846d0b18678ff7aa44029643571e237bc48d58))
	ROM_LOAD("sopsndl1.u21", 0x100000, 0x100000, CRC(66cdb90d) SHA1(d96e1b92e54a94b5e0ed9d62cff9220b9e215e85))
	ROM_LOAD("sopsndl.u36", 0x200000, 0x100000, CRC(147c4216) SHA1(ded2917188bea51cb03db72fe53fcd76a3e66ab9))
	ROM_LOAD("sopsndl.u37", 0x300000, 0x100000, CRC(cfe814fb) SHA1(51b6b10dda4640f8569e610b41c77e3657eabff2))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END


ROM_START(sopranoi)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("sopcpui.500", 0x00000, 0x20000, CRC(c6b29efb) SHA1(1d7045c06bd8c6fc2304fba150c201e920ae92b4))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD("sopdspi.500", 0x00000, 0x80000, CRC(5a3f479b) SHA1(43f36e27549259df172ed4340ae891eca634a594))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("biosv8.u8", 0x400000, 0x20000, CRC(c81b5532) SHA1(7bd630179910423a01394f82ad7ff2008c02f7ff))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("sopsndi.u7",0x0000,0x10000,CRC(afb9c474) SHA1(fd184e8cd6afff61fd2874b08f0e841934916ccb))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("sopsndi.u17", 0x000000, 0x100000, CRC(ec8e4e36) SHA1(312f1d86bf6703b8ff6b807a3a2abea9fe0c20b8))
	ROM_LOAD("sopsndi.u21", 0x100000, 0x100000, CRC(37727b76) SHA1(8801091870a30222d5a99535bbe15ac97334e368))
	ROM_LOAD("sopsndi.u36", 0x200000, 0x100000, CRC(71568348) SHA1(516d5ea35f8323e247c25000cb223f3539796ea1))
	ROM_LOAD("sopsndi.u37", 0x300000, 0x100000, CRC(b34c0a5f) SHA1(b84979d6eef7d23e6dd5410993d83fba2121bc6a))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(sopranoi_400)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "sopcpui.400", 0x0000, 0x20000, CRC(9bd48a86) SHA1(16aa55892a295a1486ede60df104520362005155))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "sopdspi.400", 0x0000, 0x80000, CRC(7e58e364) SHA1(1fcf282ae68e3e725e16e43b85f57d1a18b43508))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("biosv8.u8", 0x400000, 0x20000, CRC(c81b5532) SHA1(7bd630179910423a01394f82ad7ff2008c02f7ff))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("sopsndi.u7",0x0000,0x10000,CRC(afb9c474) SHA1(fd184e8cd6afff61fd2874b08f0e841934916ccb))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("sopsndi.u17", 0x000000, 0x100000, CRC(ec8e4e36) SHA1(312f1d86bf6703b8ff6b807a3a2abea9fe0c20b8))
	ROM_LOAD("sopsndi.u21", 0x100000, 0x100000, CRC(37727b76) SHA1(8801091870a30222d5a99535bbe15ac97334e368))
	ROM_LOAD("sopsndi.u36", 0x200000, 0x100000, CRC(71568348) SHA1(516d5ea35f8323e247c25000cb223f3539796ea1))
	ROM_LOAD("sopsndi.u37", 0x300000, 0x100000, CRC(b34c0a5f) SHA1(b84979d6eef7d23e6dd5410993d83fba2121bc6a))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(sopranoi_300)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "sopcpui.300",   0x0000, 0x20000, CRC(1a2846e6) SHA1(10dd57d5f65f397d67105f7e1d0e8d920753893c))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "sopdspi.300",   0x0000, 0x80000, CRC(d7903ed2) SHA1(ae54952cd3e6f7fb0075e71d484701def764f0d6))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("biosv8.u8", 0x400000, 0x20000, CRC(c81b5532) SHA1(7bd630179910423a01394f82ad7ff2008c02f7ff))
	ROM_RELOAD(0x0,0x20000)

	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("sopsndi.u7",0x0000,0x10000,CRC(afb9c474) SHA1(fd184e8cd6afff61fd2874b08f0e841934916ccb))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("sopsndi.u17", 0x000000, 0x100000, CRC(ec8e4e36) SHA1(312f1d86bf6703b8ff6b807a3a2abea9fe0c20b8))
	ROM_LOAD("sopsndi.u21", 0x100000, 0x100000, CRC(37727b76) SHA1(8801091870a30222d5a99535bbe15ac97334e368))
	ROM_LOAD("sopsndi.u36", 0x200000, 0x100000, CRC(71568348) SHA1(516d5ea35f8323e247c25000cb223f3539796ea1))
	ROM_LOAD("sopsndi.u37", 0x300000, 0x100000, CRC(b34c0a5f) SHA1(b84979d6eef7d23e6dd5410993d83fba2121bc6a))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

ROM_START(sopranoi_107)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "sopcpui.107",   0x0000, 0x20000, CRC(d5cd6b07) SHA1(830de4af7f54c85feeae6fb7f630f84e48fdb98b))
	ROM_RELOAD(0x20000, 0x20000)
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x60000, 0x20000)

	ROM_REGION(0x80000, "decodmd", 0)
	ROM_LOAD( "sopdspi.100",   0x0000, 0x80000, CRC(2a6320c1) SHA1(8cd25c53abb353cbbb88af3e7384c7275d836dbb))

	ROM_REGION32_LE(0x600000, "cpu2", 0)
	ROM_LOAD("biosv8.u8", 0x400000, 0x20000, CRC(c81b5532) SHA1(7bd630179910423a01394f82ad7ff2008c02f7ff))

	ROM_RELOAD(0x0,0x20000)
	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("sopsndi.u7",0x0000,0x10000,CRC(afb9c474) SHA1(fd184e8cd6afff61fd2874b08f0e841934916ccb))

	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("sopsndi1.u17", 0x000000, 0x100000, CRC(7cb762dd) SHA1(84ec54d6495ccb02052c8d5b6b66c018a702bb4e))
	ROM_LOAD("sopsndi.u21", 0x100000, 0x100000, CRC(37727b76) SHA1(8801091870a30222d5a99535bbe15ac97334e368))
	ROM_LOAD("sopsndi.u36", 0x200000, 0x100000, CRC(71568348) SHA1(516d5ea35f8323e247c25000cb223f3539796ea1))
	ROM_LOAD("sopsndi.u37", 0x300000, 0x100000, CRC(b34c0a5f) SHA1(b84979d6eef7d23e6dd5410993d83fba2121bc6a))

	ROM_REGION(0x117, "plds", 0 )
	ROM_LOAD("16v8d.u213", 0x000, 0x117, BAD_DUMP CRC(942d99fd) SHA1(db33b16e582c6762ac6843a3613edb733cbeb499)) // Dumped from a Terminator 3 Spain machine, may be different on others
ROM_END

} // Anonymous namespace

/* Display ROM versions country/language codes:
     A = America (USA and UK)
     F = France
     G = Germany
     I = Italy
     J = Japan? (seen on "Dale Jr.")
     L = Latin (Spain)
*/

GAME(1995, apollo13,     0,        whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Sega",  "Apollo 13 (CPU 5.01, display A5.00)",                                            MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995, apollo13_d4,  apollo13, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Sega",  "Apollo 13 (CPU 5.01, display A4.01)",                                            MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995, apollo13_20,  apollo13, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Sega",  "Apollo 13 (CPU 2.03, display A2.01)",                                            MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995, apollo13_10,  apollo13, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Sega",  "Apollo 13 (CPU 1.00, display A1.00)",                                            MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2001, austin,       0,        whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Austin Powers (CPU 3.02, display A3.00)",                                        MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2001, austnew,      austin,   whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Austin Powers (CPU 3.02, display A3.00, ARM7 sound board)",                      MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2001, aust301,      austin,   whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Austin Powers (CPU 3.01, display A3.00)",                                        MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2001, aust300,      austin,   whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Austin Powers (CPU 3.00, display A3.00)",                                        MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2001, aust201,      austin,   whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Austin Powers (CPU 2.01, display A2.00)",                                        MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2001, austinf,      austin,   whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Austin Powers (CPU 3.02, display F3.00, France)",                                MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2001, austing,      austin,   whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Austin Powers (CPU 3.02, display G3.00, Germany)",                               MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2001, austini,      austin,   whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Austin Powers (CPU 3.02, display I3.00, Italy)",                                 MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2004, elvisp,       0,        whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Elvis (CPU 5.00, display A5.00)",                                                MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2004, elvisp4,      elvisp,   whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Elvis (CPU 4.00, display A4.01)",                                                MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2004, elvisp303,    elvisp,   whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Elvis (CPU 3.03, display A3.02)",                                                MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2004, elvisp302,    elvisp,   whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Elvis (CPU 3.02, display A3.02)",                                                MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2004, elvisp100,    elvisp,   whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Elvis (CPU 1.00, display A1.00)",                                                MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2004, elvisf,       elvisp,   whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Elvis (CPU 5.00, display F5.00, France)",                                        MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2004, elvisf4,      elvisp,   whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Elvis (CPU 4.00, display F4.01, France)",                                        MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2004, elvisf303,    elvisp,   whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Elvis (CPU 3.03, display F3.02, France)",                                        MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2004, elvisf302,    elvisp,   whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Elvis (CPU 3.02, display F3.02, France)",                                        MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2004, elvisg,       elvisp,   whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Elvis (CPU 5.00, display G5.00, Germany)",                                       MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2004, elvisg4,      elvisp,   whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Elvis (CPU 4.00, display G4.01, Germany)",                                       MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2004, elvisg303,    elvisp,   whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Elvis (CPU 3.03, display G3.02, Germany)",                                       MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2004, elvisg302,    elvisp,   whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Elvis (CPU 3.02, display G3.02, Germany)",                                       MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2004, elvisi,       elvisp,   whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Elvis (CPU 5.00, display I5.00, Italy)",                                         MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2004, elvisi4,      elvisp,   whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Elvis (CPU 4.00, display I4.01, Italy)",                                         MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2004, elvisi303,    elvisp,   whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Elvis (CPU 3.03, display I3.02, Italy)",                                         MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2004, elvisi302,    elvisp,   whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Elvis (CPU 3.02, display I3.02, Italy)",                                         MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2004, elvisl,       elvisp,   whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Elvis (CPU 5.00, display L5.00, Spain)",                                         MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2004, elvisl4,      elvisp,   whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Elvis (CPU 4.00, display L4.01, Spain)",                                         MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2004, elvisl303,    elvisp,   whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Elvis (CPU 3.03, display L3.02, Spain)",                                         MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2004, elvisl302,    elvisp,   whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Elvis (CPU 3.02, display L3.02, Spain)",                                         MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1998, godzillp,     0,        whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Sega",  "Godzilla (pinball, CPU 2.05, display A2.00)",                                    MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1998, godzillp_100, godzillp, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Sega",  "Godzilla (pinball, CPU 1.00, display A1.00)",                                    MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1998, godzillp_090, godzillp, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Sega",  "Godzilla (pinball, CPU 0.90, display A0.991)",                                   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1996, gldneye,      0,        whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Sega",  "Goldeneye (CPU 4.04, display A4.00)",                                            MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1996, gldneye_402,  gldneye,  whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Sega",  "Goldeneye (CPU 4.02, display A4.00)",                                            MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1998, goldcue,      0,        goldcue,    whitestar, whitestar_state, empty_init, ROT0, "Sega",  "Golden Cue (CPU 0.04, display 0.10",                                             MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // No country code on display ROM, USA CPU is 0.04, but 0.01 for every other country/languaje
GAME(2005, gprix,        0,        whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Grand Prix (CPU 4.50, display A4.00)",                                           MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2005, gprix_400,    gprix,    whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Grand Prix (CPU 4.00, display A4.00)",                                           MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2005, gprix_352,    gprix,    whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Grand Prix (CPU 3.52, display A3.03)",                                           MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2005, gprix_350,    gprix,    whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Grand Prix (CPU 3.50, display A3.03)",                                           MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2005, gprix_340,    gprix,    whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Grand Prix (CPU 3.40, display A3.03)",                                           MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2005, gprix_301,    gprix,    whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Grand Prix (CPU 3.01, display A3.01)",                                           MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2005, gprixf,       gprix,    whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Grand Prix (CPU 4.50, display F4.00, France)",                                   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2005, gprixf_400,   gprix,    whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Grand Prix (CPU 4.00, display F4.00, France)",                                   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2005, gprixf_352,   gprix,    whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Grand Prix (CPU 3.52, display F3.03, France)",                                   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2005, gprixf_350,   gprix,    whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Grand Prix (CPU 3.50, display F3.03, France)",                                   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2005, gprixf_340,   gprix,    whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Grand Prix (CPU 3.40, display F3.03, France)",                                   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2005, gprixf_301,   gprix,    whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Grand Prix (CPU 3.01, display F3.01, France)",                                   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2005, gprixg,       gprix,    whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Grand Prix (CPU 4.50, display G4.00, Germany)",                                  MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2005, gprixg_400,   gprix,    whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Grand Prix (CPU 4.00, display G4.00, Germany)",                                  MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2005, gprixg_352,   gprix,    whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Grand Prix (CPU 3.52, display G3.03, Germany)",                                  MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2005, gprixg_350,   gprix,    whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Grand Prix (CPU 3.50, display G3.03, Germany)",                                  MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2005, gprixg_340,   gprix,    whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Grand Prix (CPU 3.40, display G3.03, Germany)",                                  MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2005, gprixg_301,   gprix,    whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Grand Prix (CPU 3.01, display G3.01, Germany)",                                  MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2005, gprixi,       gprix,    whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Grand Prix (CPU 4.50, display I4.00, Italy)",                                    MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2005, gprixi_400,   gprix,    whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Grand Prix (CPU 4.00, display I4.00, Italy)",                                    MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2005, gprixi_352,   gprix,    whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Grand Prix (CPU 3.52, display I3.03, Italy)",                                    MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2005, gprixi_350,   gprix,    whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Grand Prix (CPU 3.50, display I3.03, Italy)",                                    MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2005, gprixi_340,   gprix,    whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Grand Prix (CPU 3.40, display I3.03, Italy)",                                    MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2005, gprixi_301,   gprix,    whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Grand Prix (CPU 3.01, display I3.01, Italy)",                                    MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2005, gprixl,       gprix,    whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Grand Prix (CPU 4.50, display L4.00, Spain)",                                    MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2005, gprixl_400,   gprix,    whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Grand Prix (CPU 4.00, display L4.00, Spain)",                                    MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2005, gprixl_352,   gprix,    whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Grand Prix (CPU 3.52, display L3.03, Spain)",                                    MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2005, gprixl_350,   gprix,    whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Grand Prix (CPU 3.50, display L3.03, Spain)",                                    MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2005, gprixl_340,   gprix,    whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Grand Prix (CPU 3.40, display L3.03, Spain)",                                    MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2005, gprixl_301,   gprix,    whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Grand Prix (CPU 3.01, display L3.01, Spain)",                                    MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1999, harl_a13,     0,        whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Sega",  "Harley Davidson (Sega, CPU 1.03, display A1.04)",                                MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1999, harl_a10,     harl_a13, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Sega",  "Harley Davidson (Sega, CPU 1.03, display A1.00)",                                MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1999, harl_u13,     harl_a13, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Sega",  "Harley Davidson (Sega, CPU 1.03, display A1.04, UK)",                            MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1999, harl_f13,     harl_a13, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Sega",  "Harley Davidson (Sega, CPU 1.03, display F1.04, France)",                        MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1999, harl_g13,     harl_a13, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Sega",  "Harley Davidson (Sega, CPU 1.03, display G1.04, Germany)",                       MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1999, harl_i13,     harl_a13, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Sega",  "Harley Davidson (Sega, CPU 1.03, display I1.04, Italy)",                         MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1999, harl_l13,     harl_a13, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Sega",  "Harley Davidson (Sega, CPU 1.03, display L1.04, Spain)",                         MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2004, harl_a40,     0,        whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Harley Davidson (Stern, CPU 4.00, display A4.00)",                               MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2004, harl_a30,     harl_a40, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Harley Davidson (Stern, CPU 3.00, display A3.00)",                               MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, harl_a18,     harl_a40, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Harley Davidson (Stern, CPU 1.08, display A1.05)",                               MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2004, harl_f40,     harl_a40, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Harley Davidson (Stern, CPU 4.00, display F4.00, France)",                       MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2004, harl_f30,     harl_a40, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Harley Davidson (Stern, CPU 3.00, display F3.00, France)",                       MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, harl_f18,     harl_a40, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Harley Davidson (Stern, CPU 1.08, display F1.05, France)",                       MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2004, harl_g40,     harl_a40, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Harley Davidson (Stern, CPU 4.00, display G4.00, Germany)",                      MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2004, harl_g30,     harl_a40, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Harley Davidson (Stern, CPU 3.00, display G3.00, Germany)",                      MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, harl_g18,     harl_a40, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Harley Davidson (Stern, CPU 1.08, display G1.05, Germany)",                      MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2004, harl_i40,     harl_a40, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Harley Davidson (Stern, CPU 4.00, display I4.00, Italy)",                        MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2004, harl_i30,     harl_a40, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Harley Davidson (Stern, CPU 3.00, display I3.00, Italy)",                        MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, harl_i18,     harl_a40, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Harley Davidson (Stern, CPU 1.08, display I1.05, Italy)",                        MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2004, harl_l40,     harl_a40, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Harley Davidson (Stern, CPU 4.00, display L4.00, Spain)",                        MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2004, harl_l30,     harl_a40, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Harley Davidson (Stern, CPU 3.00, display L3.00, Spain)",                        MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, harl_l18,     harl_a40, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Harley Davidson (Stern, CPU 1.08, display L1.05, Spain)",                        MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2001, hirolcas,     0,        whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "High Roller Casino (CPU 3.00, display A3.00)",                                   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2001, hironew,      hirolcas, whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "High Roller Casino (CPU 3.00, display A3.00, ARM7 sound board)",                 MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2001, hirolcat,     hirolcas, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "High Roller Casino (CPU 2.90, display A2.99)",                                   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2001, hirolcas_210, hirolcas, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "High Roller Casino (CPU 2.10, display A2.00)",                                   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2001, hirol_fr,     hirolcas, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "High Roller Casino (CPU 3.00, display F3.00, France)",                           MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2001, hirol_gr,     hirolcas, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "High Roller Casino (CPU 3.00, display G3.00, Germany)",                          MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2001, hirol_gr_210, hirolcas, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "High Roller Casino (CPU 2.10, display G2.01, Germany)",                          MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2001, hirol_it,     hirolcas, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "High Roller Casino (CPU 3.00, display I3.00, Italy)",                            MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1996, id4,          0,        whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Sega",  "Independence Day (CPU 2.02, display A2.00)",                                     MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1996, id4_201,      id4,      whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Sega",  "Independence Day (CPU 2.01, display A2.00)",                                     MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1996, id4f,         id4,      whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Sega",  "Independence Day (CPU 2.02, display F2.00, France)",                             MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1996, id4f_201,     id4,      whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Sega",  "Independence Day (CPU 2.01, display F2.00, France)",                             MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1998, lostspc,      0,        whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Sega",  "Lost in Space (CPU 1.01, display A1.02)",                                        MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1998, lostspc_d1,   lostspc,  whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Sega",  "Lost in Space (CPU 1.01, display A1.01)",                                        MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1998, lostspcf,     lostspc,  whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Sega",  "Lost in Space (CPU 1.01, display F1.02, France)",                                MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1998, lostspcg,     lostspc,  whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Sega",  "Lost in Space (CPU 1.01, display G1.02, Germany)",                               MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2001, monopolp,     0,        whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Monopoly (CPU 3.20, display A3.01)",                                             MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2002, mononew,      monopolp, whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Monopoly (CPU 3.20, display A3.01, ARM7 sound board)",                          MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2002, monop303,     monopolp, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Monopoly (CPU 3.03, display A3.01)",                                             MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2002, monop301,     monopolp, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Monopoly (CPU 3.01, display A3.01)",                                             MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2002, monop251,     monopolp, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Monopoly (CPU 2.51, display A2.06)",                                             MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2002, monop233,     monopolp, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Monopoly (CPU 2.33, display A2.03)",                                             MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2002, monopolf,     monopolp, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Monopoly (CPU 3.20, display F3.01, France)",                                     MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2002, monopolg,     monopolp, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Monopoly (CPU 3.20, display G3.01, Germany)",                                    MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2002, monopoli,     monopolp, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Monopoly (CPU 3.20, display I3.01, Italy)",                                      MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2002, monopoll,     monopolp, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Monopoly (CPU 3.20, display L3.01, Spain)",                                      MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2005, nascar,       0,        whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Nascar (CPU 4.50, display A4.00)",                                               MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2005, nascar_400,   nascar,   whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Nascar (CPU 4.00, display A4.00)",                                               MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2005, nascar_352,   nascar,   whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Nascar (CPU 3.52, display A3.03)",                                               MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2005, nascar_350,   nascar,   whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Nascar (CPU 3.50, display A3.03)",                                               MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2005, nascar_340,   nascar,   whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Nascar (CPU 3.40, display A3.03)",                                               MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2005, nascar_301,   nascar,   whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Nascar (CPU 3.01, display A3.01)",                                               MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2005, dalejr,       nascar,   whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Dale Jr. (CPU 5.00, display J5.00)",                                             MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2005, nascarl,      nascar,   whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Nascar (CPU 4.50, display L4.00, Spain)",                                        MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2005, nascarl_400,  nascar,   whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Nascar (CPU 4.00, display L4.00, Spain)",                                        MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2005, nascarl_352,  nascar,   whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Nascar (CPU 3.52, display L3.03, Spain)",                                        MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2005, nascarl_350,  nascar,   whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Nascar (CPU 3.50, display L3.03, Spain)",                                        MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2005, nascarl_340,  nascar,   whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Nascar (CPU 3.40, display L3.03, Spain)",                                        MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2005, nascarl_301,  nascar,   whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Nascar (CPU 3.01, display L3.01, Spain)",                                        MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2001, nfl,          0,        whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "NFL (CPU 1.01, display A1.02)",                                                  MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2002, playboys,     0,        whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Playboy (CPU 5.00, display A5.00)",                                              MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2002, playnew,      playboys, whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Playboy (CPU 5.00, display A5.00, ARM7 sound board)",                            MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2002, playboys_401, playboys, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Playboy (CPU 4.01, display A4.00)",                                              MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2002, playboys_303, playboys, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Playboy (CPU 3.03, display A3.00)",                                              MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2002, playboys_302, playboys, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Playboy (CPU 3.02, display A3.00)",                                              MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2002, playboys_300, playboys, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Playboy (CPU 3.00, display A3.00)",                                              MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2002, playboys_203, playboys, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Playboy (CPU 2.03, display A2.01)",                                              MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2002, playboyf,     playboys, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Playboy (CPU 5.00, display F5.00, France)",                                      MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2002, playboyf_401, playboys, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Playboy (CPU 4.01, display F4.00, France)",                                      MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2002, playboyf_303, playboys, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Playboy (CPU 3.03, display F3.00, France)",                                      MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2002, playboyf_302, playboys, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Playboy (CPU 3.02, display F3.00, France)",                                      MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2002, playboyf_300, playboys, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Playboy (CPU 3.00, display F3.00, France)",                                      MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2002, playboyf_203, playboys, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Playboy (CPU 2.03, display F2.01, France)",                                      MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2002, playboyg,     playboys, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Playboy (CPU 5.00, display G5.00, Germany)",                                     MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2002, playboyg_401, playboys, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Playboy (CPU 4.01, display G4.00, Germany)",                                     MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2002, playboyg_303, playboys, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Playboy (CPU 3.03, display G3.00, Germany)",                                     MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2002, playboyg_302, playboys, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Playboy (CPU 3.02, display G3.00, Germany)",                                     MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2002, playboyg_300, playboys, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Playboy (CPU 3.00, display G3.00, Germany)",                                     MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2002, playboyg_203, playboys, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Playboy (CPU 2.03, display G2.01, Germany)",                                     MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2002, playboyi,     playboys, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Playboy (CPU 5.00, display I5.00, Italy)",                                       MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2002, playboyi_401, playboys, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Playboy (CPU 4.01, display I4.00, Italy)",                                       MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2002, playboyi_303, playboys, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Playboy (CPU 3.03, display I3.00, Italy)",                                       MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2002, playboyi_302, playboys, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Playboy (CPU 3.02, display I3.00, Italy)",                                       MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2002, playboyi_300, playboys, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Playboy (CPU 3.00, display I3.00, Italy)",                                       MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2002, playboyi_203, playboys, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Playboy (CPU 2.03, display I2.01, Italy)",                                       MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2002, playboyl,     playboys, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Playboy (CPU 5.00, display L5.00, Spain)",                                       MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2002, playboyl_401, playboys, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Playboy (CPU 4.01, display L4.00, Spain)",                                       MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2002, playboyl_303, playboys, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Playboy (CPU 3.03, display L3.00, Spain)",                                       MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2002, playboyl_302, playboys, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Playboy (CPU 3.02, display L3.00, Spain)",                                       MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2002, playboyl_300, playboys, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Playboy (CPU 3.00, display L3.00, Spain)",                                       MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2002, playboyl_203, playboys, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Playboy (CPU 2.03, display L2.01, Spain)",                                       MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2004, ripleys,      0,        whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Ripley's Believe It or Not! (CPU 3.20, display A3.20)",                          MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2004, rip310,       ripleys,  whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Ripley's Believe It or Not! (CPU 3.10, display A3.20)",                          MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2004, rip302,       ripleys,  whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Ripley's Believe It or Not! (CPU 3.02, display A3.20)",                          MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2004, rip301,       ripleys,  whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Ripley's Believe It or Not! (CPU 3.01, display A3.20)",                          MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2004, rip300,       ripleys,  whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Ripley's Believe It or Not! (CPU 3.00, display A3.00)",                          MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2004, ripleysf,     ripleys,  whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Ripley's Believe It or Not! (CPU 3.20, display F3.01, France)",                  MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2004, ripf310,      ripleys,  whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Ripley's Believe It or Not! (CPU 3.10, display F3.01, France)",                  MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2004, ripf302,      ripleys,  whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Ripley's Believe It or Not! (CPU 3.02, display F3.01, France)",                  MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2004, ripf301,      ripleys,  whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Ripley's Believe It or Not! (CPU 3.01, display F3.01, France)",                  MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2004, ripf300,      ripleys,  whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Ripley's Believe It or Not! (CPU 3.00, display F3.00, France)",                  MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2004, ripleysg,     ripleys,  whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Ripley's Believe It or Not! (CPU 3.20, display G3.00, Germany)",                 MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2004, ripg310,      ripleys,  whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Ripley's Believe It or Not! (CPU 3.10, display G3.00, Germany)",                 MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2004, ripg302,      ripleys,  whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Ripley's Believe It or Not! (CPU 3.02, display G3.00, Germany)",                 MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2004, ripg301,      ripleys,  whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Ripley's Believe It or Not! (CPU 3.01, display G3.00, Germany)",                 MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2004, ripg300,      ripleys,  whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Ripley's Believe It or Not! (CPU 3.00, display G3.00, Germany)",                 MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2004, ripleysi,     ripleys,  whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Ripley's Believe It or Not! (CPU 3.20, display I3.00, Italy)",                   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2004, ripi310,      ripleys,  whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Ripley's Believe It or Not! (CPU 3.10, display I3.00, Italy)",                   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2004, ripi302,      ripleys,  whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Ripley's Believe It or Not! (CPU 3.02, display I3.00, Italy)",                   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2004, ripi301,      ripleys,  whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Ripley's Believe It or Not! (CPU 3.01, display I3.00, Italy)",                   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2004, ripi300,      ripleys,  whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Ripley's Believe It or Not! (CPU 3.00, display I3.00, Italy)",                   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2004, ripleysl,     ripleys,  whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Ripley's Believe It or Not! (CPU 3.20, display L3.01, Spain)",                   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2004, ripl310,      ripleys,  whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Ripley's Believe It or Not! (CPU 3.10, display L3.01, Spain)",                   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2004, ripl302,      ripleys,  whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Ripley's Believe It or Not! (CPU 3.02, display L3.01, Spain)",                   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2004, ripl301,      ripleys,  whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Ripley's Believe It or Not! (CPU 3.01, display L3.01, Spain)",                   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2004, ripl300,      ripleys,  whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Ripley's Believe It or Not! (CPU 3.00, display L3.00, Spain)",                   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2002, rctycn,       0,        whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "RollerCoaster Tycoon (CPU 7.02, display A7.01)",                                 MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2002, rctnew,       rctycn,   whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "RollerCoaster Tycoon (CPU 7.02, display A7.01, ARM7 sound board)",               MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2002, rctycn_701,   rctycn,   whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "RollerCoaster Tycoon (CPU 7.01, display A7.01)",                                 MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2002, rctycn_600,   rctycn,   whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "RollerCoaster Tycoon (CPU 6.00, display A6.00)",                                 MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2002, rctycn_400,   rctycn,   whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "RollerCoaster Tycoon (CPU 4.00, display A4.00)",                                 MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2002, rctycnf,      rctycn,   whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "RollerCoaster Tycoon (CPU 7.02, display F7.01, France)",                         MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2002, rctycnf_701,  rctycn,   whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "RollerCoaster Tycoon (CPU 7.01, display F7.00, France)",                         MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2002, rctycnf_600,  rctycn,   whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "RollerCoaster Tycoon (CPU 6.00, display F6.00, France)",                         MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2002, rctycnf_400,  rctycn,   whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "RollerCoaster Tycoon (CPU 4.00, display F4.00, France)",                         MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2002, rctycng,      rctycn,   whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "RollerCoaster Tycoon (CPU 7.02, display G7.01, Germany)",                        MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2002, rctycng_701,  rctycn,   whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "RollerCoaster Tycoon (CPU 7.01, display G7.00, Germany)",                        MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2002, rctycng_400,  rctycn,   whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "RollerCoaster Tycoon (CPU 4.00, display G4.00, Germany)",                        MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2002, rctycni,      rctycn,   whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "RollerCoaster Tycoon (CPU 7.02, display I7.01, Italy)",                          MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2002, rctycni_701,  rctycn,   whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "RollerCoaster Tycoon (CPU 7.01, display I7.00, Italy)",                          MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2002, rctycni_600,  rctycn,   whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "RollerCoaster Tycoon (CPU 6.00, display I6.00, Italy)",                          MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2002, rctycni_400,  rctycn,   whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "RollerCoaster Tycoon (CPU 4.00, display I4.00, Italy)",                          MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2002, rctycnl,      rctycn,   whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "RollerCoaster Tycoon (CPU 7.02, display L7.01, Spain)",                          MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2002, rctycnl_701,  rctycn,   whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "RollerCoaster Tycoon (CPU 7.01, display L7.00, Spain)",                          MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2002, rctycnl_600,  rctycn,   whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "RollerCoaster Tycoon (CPU 6.00, display L6.00, Spain)",                          MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2002, rctycnl_400,  rctycn,   whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "RollerCoaster Tycoon (CPU 4.00, display L4.00, Spain)",                          MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2000, shrkysht,     0,        whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Sharkey's Shootout (CPU 2.11, display A2.01)",                                   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2001, shrknew,      shrkysht, whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Sharkey's Shootout (CPU 2.11, display A2.01, ARM7 sound board)",                 MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2000, shrky_207,    shrkysht, goldcue,    whitestar, whitestar_state, empty_init, ROT0, "Stern", "Sharkey's Shootout (CPU 2.07, display A2.01)",                                   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2001, shrkyfr,      shrkysht, goldcue,    whitestar, whitestar_state, empty_init, ROT0, "Stern", "Sharkey's Shootout (CPU 2.11, display F2.01, France)",                           MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2001, shrkyfr_207,  shrkysht, goldcue,    whitestar, whitestar_state, empty_init, ROT0, "Stern", "Sharkey's Shootout (CPU 2.07, display F2.01, France)",                           MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2001, shrkygr,      shrkysht, goldcue,    whitestar, whitestar_state, empty_init, ROT0, "Stern", "Sharkey's Shootout (CPU 2.11, display G2.01, Germany)",                          MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2001, shrkygr_207,  shrkysht, goldcue,    whitestar, whitestar_state, empty_init, ROT0, "Stern", "Sharkey's Shootout (CPU 2.07, display G2.01, Germany)",                          MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2001, shrkyit,      shrkysht, goldcue,    whitestar, whitestar_state, empty_init, ROT0, "Stern", "Sharkey's Shootout (CPU 2.11, display I2.01, Italy)",                            MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2001, shrkyit_207,  shrkysht, goldcue,    whitestar, whitestar_state, empty_init, ROT0, "Stern", "Sharkey's Shootout (CPU 2.07, display I2.01, Italy)",                            MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1999, sprk_103,     0,        whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Sega",  "South Park (CPU 1.03, display A1.01)",                                           MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1999, sprk_096,     sprk_103, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Sega",  "South Park (CPU 0.96, display A1.01)",                                           MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1999, sprk_090,     sprk_103, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Sega",  "South Park (CPU 0.90, display A0.90)",                                           MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1997, spacejam,     0,        whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Sega",  "Space Jam (CPU 3.00, display A3.00)",                                            MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1997, spacejam_200, spacejam, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Sega",  "Space Jam (CPU 2.00, display A2.00)",                                            MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1997, spacejmf,     spacejam, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Sega",  "Space Jam (CPU 3.00, display F3.00, France)",                                    MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1997, spacejmg,     spacejam, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Sega",  "Space Jam (CPU 3.00, display G3.00, Germany)",                                   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1997, spacejmi,     spacejam, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Sega",  "Space Jam (CPU 3.00, display I3.00, Italy)",                                     MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1997, swtril43,     0,        whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Sega",  "Star Wars Trilogy (CPU 4.03, display A4.00)",                                    MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1997, swtril41,     swtril43, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Sega",  "Star Wars Trilogy (CPU 4.01, display A4.00)",                                    MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1997, swtril30,     swtril43, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Sega",  "Star Wars Trilogy (CPU 3.00, display A3.00)",                                    MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1997, startrp,      0,        whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Sega",  "Starship Troopers (CPU 2.01, display A2.00)",                                    MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1997, startrp2,     startrp,  whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Sega",  "Starship Troopers (CPU 2.00, display A2.00)",                                    MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2000, strikext,     0,        whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Striker Xtreme (CPU 1.02, display A1.03)",                                       MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1999, strknew,      strikext, whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Striker Xtreme (CPU 1.02, display A1.03, ARM7 sound board)",                     MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2000, strikext_100, strikext, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Striker Xtreme (CPU 1.00, display A1.03)",                                       MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2000, strxt_uk,     strikext, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Striker Xtreme (CPU 1.01, display A1.03, UK)",                                   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2000, strxt_fr,     strikext, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Striker Xtreme (CPU 1.02, display F1.03, France)",                               MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2000, strxt_gr,     strikext, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Striker Xtreme (CPU 1.03, display G1.03, Germany)",                              MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2000, strxt_it,     strikext, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Striker Xtreme (CPU 1.02, display I1.03, Italy)",                                MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2000, strxt_it_101, strikext, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Striker Xtreme (CPU 1.01, display I1.03, Italy)",                                MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2000, strxt_sp,     strikext, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Striker Xtreme (CPU 1.02, display L1.03, Spain)",                                MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, term3,        0,        whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Terminator 3: Rise of the Machines (CPU 4.00, display A4.00)",                   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, t3new,        term3,    whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "Terminator 3: Rise of the Machines (CPU 4.00, display A4.00, ARM7 sound board)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, term3_301,    term3,    whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Terminator 3: Rise of the Machines (CPU 3.01, display A3.00)",                   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, term3_205,    term3,    whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Terminator 3: Rise of the Machines (CPU 2.05, display A2.01)",                   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, term3f,       term3,    whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Terminator 3: Rise of the Machines (CPU 4.00, display F4.00, France)",           MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, term3f_301,   term3,    whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Terminator 3: Rise of the Machines (CPU 3.01, display F3.00, France)",           MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, term3f_205,   term3,    whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Terminator 3: Rise of the Machines (CPU 2.05, display F2.01, France)",           MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, term3g,       term3,    whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Terminator 3: Rise of the Machines (CPU 4.00, display G4.00, Germany)",          MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, term3g_301,   term3,    whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Terminator 3: Rise of the Machines (CPU 3.01, display G3.00, Germany)",          MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, term3i,       term3,    whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Terminator 3: Rise of the Machines (CPU 4.00, display I4.00, Italy)",            MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, term3i_301,   term3,    whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Terminator 3: Rise of the Machines (CPU 3.01, display I3.00, Italy)",            MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, term3i_205,   term3,    whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Terminator 3: Rise of the Machines (CPU 2.05, display I2.01, Italy)",            MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, term3l,       term3,    whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Terminator 3: Rise of the Machines (CPU 4.00, display L4.00, Spain)",            MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, term3l_301,   term3,    whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Terminator 3: Rise of the Machines (CPU 3.01, display L3.00, Spain)",            MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, term3l_205,   term3,    whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Terminator 3: Rise of the Machines (CPU 2.05, display L2.01, Spain)",            MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, lotr,         0,        whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Lord Of The Rings (CPU 10.00, display A10.00)",                              MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, lotr_le,      lotr,     whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Lord Of The Rings (CPU 10.02, display A10.00, Limited Edition)",             MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, lotr9,        lotr,     whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Lord Of The Rings (CPU 9.00, display A9.00)",                                MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, lotr8,        lotr,     whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Lord Of The Rings (CPU 8.00, display A8.00)",                                MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, lotr7,        lotr,     whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Lord Of The Rings (CPU 7.00, display A7.00)",                                MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, lotr6,        lotr,     whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Lord Of The Rings (CPU 6.00, display A6.00)",                                MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, lotr501,      lotr,     whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Lord Of The Rings (CPU 5.01, display A5.01)",                                MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, lotr5,        lotr,     whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Lord Of The Rings (CPU 5.00, display A5.00)",                                MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, lotr410,      lotr,     whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Lord Of The Rings (CPU 4.10, display A4.04)",                                MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, lotr401,      lotr,     whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Lord Of The Rings (CPU 4.01, display A4.03)",                                MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, lotr3,        lotr,     whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Lord Of The Rings (CPU 3.00, display A3.00)",                                MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, lotr_fr,      lotr,     whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Lord Of The Rings (CPU 10.00, display F10.00, France)",                      MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, lotr_fr9,     lotr,     whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Lord Of The Rings (CPU 9.00, display F9.00, France)",                        MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, lotr_fr8,     lotr,     whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Lord Of The Rings (CPU 8.00, display F8.00, France)",                        MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, lotr_fr7,     lotr,     whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Lord Of The Rings (CPU 7.00, display F7.00, France)",                        MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, lotr_fr6,     lotr,     whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Lord Of The Rings (CPU 6.00, display F6.00, France)",                        MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, lotr_fr501,   lotr,     whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Lord Of The Rings (CPU 5.01, display F5.01, France)",                        MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, lotr_fr5,     lotr,     whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Lord Of The Rings (CPU 5.00, display F5.00, France)",                        MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, lotr_fr410,   lotr,     whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Lord Of The Rings (CPU 4.10, display F4.04, France)",                        MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, lotr_fr401,   lotr,     whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Lord Of The Rings (CPU 4.01, display F4.03, France)",                        MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, lotr_gr,      lotr,     whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Lord Of The Rings (CPU 10.00, display G10.00, Germany)",                     MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, lotr_gr9,     lotr,     whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Lord Of The Rings (CPU 9.00, display G9.00, Germany)",                       MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, lotr_gr8,     lotr,     whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Lord Of The Rings (CPU 8.00, display G8.00, Germany)",                       MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, lotr_gr7,     lotr,     whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Lord Of The Rings (CPU 7.00, display G7.00, Germany)",                       MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, lotr_gr6,     lotr,     whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Lord Of The Rings (CPU 6.00, display G6.00, Germany)",                       MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, lotr_gr501,   lotr,     whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Lord Of The Rings (CPU 5.01, display G5.01, Germany)",                       MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, lotr_gr5,     lotr,     whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Lord Of The Rings (CPU 5.00, display G5.00, Germany)",                       MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, lotr_gr410,   lotr,     whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Lord Of The Rings (CPU 4.10, display G4.04, Germany)",                       MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, lotr_gr401,   lotr,     whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Lord Of The Rings (CPU 4.01, display G4.03, Germany)",                       MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, lotr_it,      lotr,     whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Lord Of The Rings (CPU 10.00, display I10.00, Italy)",                       MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, lotr_it9,     lotr,     whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Lord Of The Rings (CPU 9.00, display I9.00, Italy)",                         MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, lotr_it8,     lotr,     whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Lord Of The Rings (CPU 8.00, display I8.00, Italy)",                         MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, lotr_it7,     lotr,     whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Lord Of The Rings (CPU 7.00, display I7.00, Italy)",                         MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, lotr_it6,     lotr,     whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Lord Of The Rings (CPU 6.00, display I6.00, Italy)",                         MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, lotr_it501,   lotr,     whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Lord Of The Rings (CPU 5.01, display I5.01, Italy)",                         MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, lotr_it5,     lotr,     whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Lord Of The Rings (CPU 5.00, display I5.00, Italy)",                         MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, lotr_it410,   lotr,     whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Lord Of The Rings (CPU 4.10, display I4.04, Italy)",                         MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, lotr_it401,   lotr,     whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Lord Of The Rings (CPU 4.01, display I4.03, Italy)",                         MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, lotr_sp,      lotr,     whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Lord Of The Rings (CPU 10.00, display L10.00, Spain)",                       MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, lotr_sp9,     lotr,     whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Lord Of The Rings (CPU 9.00, display L9.00, Spain)",                         MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, lotr_sp8,     lotr,     whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Lord Of The Rings (CPU 8.00, display L8.00, Spain)",                         MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, lotr_sp7,     lotr,     whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Lord Of The Rings (CPU 7.00, display L7.00, Spain)",                         MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, lotr_sp6,     lotr,     whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Lord Of The Rings (CPU 6.00, display L6.00, Spain)",                         MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, lotr_sp501,   lotr,     whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Lord Of The Rings (CPU 5.01, display L5.01, Spain)",                         MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, lotr_sp5,     lotr,     whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Lord Of The Rings (CPU 5.00, display L5.00, Spain)",                         MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, lotr_sp401,   lotr,     whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Lord Of The Rings (CPU 4.01, display L4.03, Spain)",                         MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1997, jplstw22,     0,        whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Sega",  "The Lost World: Jurassic Park (pinball, CPU 2.02, display A2.01)",               MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1997, jplstw20,     jplstw22, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Sega",  "The Lost World: Jurassic Park (pinball, CPU 2.00, display A2.01)",               MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, simpprty,     0,        whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Simpsons Pinball Party (CPU 5.00, display A5.00)",                           MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, simpnew,      simpprty, whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Simpsons Pinball Party (CPU 5.00, display A5.00, ARM7 sound board)",         MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, simpprty_400, simpprty, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Simpsons Pinball Party (CPU 4.00, display A4.00)",                           MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, simpprty_300, simpprty, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Simpsons Pinball Party (CPU 3.00, display A3.00)",                           MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, simpprty_204, simpprty, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Simpsons Pinball Party (CPU 2.04, display A2.01)",                           MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, simpprtf,     simpprty, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Simpsons Pinball Party (CPU 5.00, display F5.00, France)",                   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, simpprtf_400, simpprty, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Simpsons Pinball Party (CPU 4.00, display F4.00, France)",                   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, simpprtf_300, simpprty, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Simpsons Pinball Party (CPU 3.00, display F3.00, France)",                   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, simpprtf_204, simpprty, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Simpsons Pinball Party (CPU 2.04, display F2.01, France)",                   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, simpprtg,     simpprty, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Simpsons Pinball Party (CPU 5.00, display G5.00, Germany)",                  MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, simpprtg_400, simpprty, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Simpsons Pinball Party (CPU 4.00, display G4.00, Germany)",                  MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, simpprti,     simpprty, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Simpsons Pinball Party (CPU 5.00, display I5.00, Italy)",                    MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, simpprti_400, simpprty, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Simpsons Pinball Party (CPU 4.00, display I4.00, Italy)",                    MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, simpprti_300, simpprty, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Simpsons Pinball Party (CPU 3.00, display I3.00, Italy)",                    MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, simpprti_204, simpprty, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Simpsons Pinball Party (CPU 2.04, display I2.01, Italy)",                    MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, simpprtl,     simpprty, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Simpsons Pinball Party (CPU 5.00, display L5.00, Spain)",                    MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, simpprtl_400, simpprty, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Simpsons Pinball Party (CPU 4.00, display L4.00, Spain)",                    MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, simpprtl_300, simpprty, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Simpsons Pinball Party (CPU 3.00, display L3.00, Spain)",                    MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2003, simpprtl_204, simpprty, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Simpsons Pinball Party (CPU 2.04, display L2.01, Spain)",                    MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2005, sopranos,     0,        whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Sopranos (CPU 5.00, display A5.00)",                                         MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2005, sopranos_400, sopranos, whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Sopranos (CPU 4.00, display A4.00)",                                         MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2005, sopranos_300, sopranos, whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Sopranos (CPU 3.00, display A3.00)",                                         MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2005, sopranos_204, sopranos, whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Sopranos (CPU 2.04, display A2.00)",                                         MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2005, sopranof,     sopranos, whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Sopranos (CPU 5.00, display F5.00, France)",                                 MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2005, sopranof_400, sopranos, whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Sopranos (CPU 4.00, display F4.00, France)",                                 MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2005, sopranof_300, sopranos, whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Sopranos (CPU 3.00, display F3.00, France)",                                 MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2005, sopranof_107, sopranos, whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Sopranos (CPU 1.07, display F1.00, France)",                                 MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2005, sopranog,     sopranos, whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Sopranos (CPU 5.00, display G5.00, Germany)",                                MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2005, sopranog_400, sopranos, whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Sopranos (CPU 4.00, display G4.00, Germany)",                                MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2005, sopranog_300, sopranos, whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Sopranos (CPU 3.00, display G3.00, Germany)",                                MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2005, sopranog_107, sopranos, whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Sopranos (CPU 1.07, display G1.00, Germany)",                                MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2005, sopranoi,     sopranos, whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Sopranos (CPU 5.00, display I5.00, Italy)",                                  MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2005, sopranoi_400, sopranos, whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Sopranos (CPU 4.00, display I4.00, Italy)",                                  MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2005, sopranoi_300, sopranos, whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Sopranos (CPU 3.00, display I3.00, Italy)",                                  MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2005, sopranoi_107, sopranos, whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Sopranos (CPU 1.07, display I1.00, Italy)",                                  MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2005, sopranol,     sopranos, whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Sopranos (CPU 5.00, display L5.00, Spain)",                                  MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2005, sopranol_400, sopranos, whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Sopranos (CPU 4.00, display L4.00, Spain)",                                  MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2005, sopranol_300, sopranos, whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Sopranos (CPU 3.00, display L3.00, Spain)",                                  MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2005, sopranol_107, sopranos, whitestarm, whitestar, whitestar_state, empty_init, ROT0, "Stern", "The Sopranos (CPU 1.07, display L1.00, Spain)",                                  MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1997, xfilesp,      0,        whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Sega",  "X-Files (CPU 3.03, display A3.00)",                                              MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1997, xfilespf,     xfilesp,  whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Sega",  "X-Files (CPU 3.03, display F3.00, France)",                                      MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1997, xfiles2,      xfilesp,  whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Sega",  "X-Files (CPU 2.04, display A2.01)",                                              MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1997, xfiles20,     xfilesp,  whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Sega",  "X-Files (CPU 2.00, display A2.00)",                                              MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1996, twst_405,     0,        whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Sega",  "Twister (CPU 4.05, display A4.00)",                                              MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1996, twst_404,     twst_405, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Sega",  "Twister (CPU 4.04, display A4.00)",                                              MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1996, twst_300,     twst_405, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Sega",  "Twister (CPU 3.00, display A3.01)",                                              MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1998, viprsega,     0,        whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Sega",  "Viper Night Drivin' (CPU 2.01, display A2.01)",                                  MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1998, viprsega_102, viprsega, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Sega",  "Viper Night Drivin' (CPU 1.02, display A1.00)",                                  MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )

// Not pinball
GAME(1998, ctchzdlx,     ctcheese, whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Sega",  "Cut The Cheese Deluxe (redemption, CPU 1.00, display A1.00)",                    MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2002, monopred,     0,        whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Stern", "Monopoly (coin dropper, CPU 4.01, display 4.00)",                                MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // No country code on display ROM
GAME(1998, titanic,      0,        whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Sega",  "Titanic (coin dropper, CPU 1.01, display A1.01)",                                MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1998, wackadoo,     0,        whitestar,  whitestar, whitestar_state, empty_init, ROT0, "Sega",  "Wack-A-Doodle-Doo (redemption, CPU 1.00, display A1.01)",                        MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
