// license:GPL-2.0+
// copyright-holders:Felipe Sanches
/*************************************************************************
 
  This is a driver for a gambling board with a yet unknown name.
  Driver by Felipe Sanches

  marywu
  The PCB is labeled with: WU- MARY-1A
  And there's a text string in the ROM that says: "Music by: SunKiss Chen"
  
  mary1s
  labeled as MARY-1/SUNRISE.
 
  U8 HM6116P?-??.
  U7 W78E52B-24.
  U14 U16 JFC 95101.
  U23 TOP 8279.
  U100 unpopulated for ym2413 or um3567.
  
  unpopulated led marked for rouletted circles instead of square.
  
  at back pcb 
  st m27c512? - duplicated 32x2 with same rom programs.
  
  unkwinw
  Product name: 小方玛丽板
  Product Code: square-mario-baord
  The board has a single marking J373.
  string: COPYRIGHT BY WIN WAY ELEC. CORP. CLEMENT CHANG, MUSIC by: SunKiss Chen 

  Mainboard
  18 x 16.5cm.
  3V6 BAT - backup battery to keep nvram
  U1 AT89C51 - internal rom.
  U2 HM6118LP-3
  U8 EPM7032SLC44-10N 
  U9 TOP 8279
  U10 U6295
  U11 MX 29F1615PC-10
  U12 w27c512 - eeprom
  Y1 10.7386
  at back
  MUSICBAR VER 201.
  
  Led Board
  3 x 2 leds
  6 x 24 leds for roulette
  29 x 20.5cm.

  the LED board says GHY1-PCB and BYP-020 
  U5 Two JFC 95101   
  unpopulated ld32 - led and ds14 ds15 2x  7 segment display 2 digit.
 
  Controls:
  mary1s:
  To reset nvram press K0 and k1, Will shown 09 then restart.
  
  unkwinw
  Press k0 and k2 to reset the nvram.
  
 TODO:
  * Figure out where exactly all devices are mapped to (the devices are
    2 sound chips, the 2kb SRAM, the 8bit DIP switches,
    31 LEDs, 13 modules of double-digit 7-seg displays and 4 push-buttons).
  * we may also have user inputs from the coin slot and from the
    cabinet buttons, for making bets.
  * Need proper layout for unkwinw.
**************************************************************************/

#include "emu.h"

#include "emu.h"
#include "cpu/mcs51/i80c51.h"
#include "cpu/mcs51/i80c52.h"
#include "machine/nvram.h"
#include "machine/i8279.h"
#include "sound/ay8910.h"
#include "sound/okim6295.h"
#include "speaker.h"

#include "marywu.lh"


namespace {

class marywu_state : public driver_device
{
public:
	marywu_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_digits(*this, "digit%u", 0U)
		, m_leds(*this, "led%u", 0U)
		, m_inputs(*this, { "KEYS1", "KEYS2", "DSW", "PUSHBUTTONS" })
		, m_oki(*this, "oki")

	{ }

	void marywu(machine_config &config);
    void mary1s(machine_config &config);
	void unkwinw(machine_config &config);
	void unkwinw_adpcm_bank(uint8_t data) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void display_7seg_data_w(uint8_t data);
	void multiplex_7seg_w(uint8_t data);
	void ay1_port_a_w(uint8_t data);
	void ay1_port_b_w(uint8_t data);
	void ay2_port_a_w(uint8_t data);
	void ay2_port_b_w(uint8_t data);
	uint8_t keyboard_r();
	void data_map(address_map &map) ATTR_COLD;
	void program_map(address_map &map) ATTR_COLD;
	void unkwinw_data_map(address_map &map) ATTR_COLD;
	void unkwinw_program_map(address_map &map) ATTR_COLD;

	uint8_t m_selected_7seg_module = 0;

	output_finder<32> m_digits;
	output_finder<30> m_leds;
	required_ioport_array<4> m_inputs;
	optional_device<okim6295_device> m_oki;

};

static INPUT_PORTS_START( marywu )
	PORT_START("KEYS1")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("BAR") PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("77") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Stars") PORT_CODE(KEYCODE_E)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Watermelon") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Bell") PORT_CODE(KEYCODE_T)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Mango") PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Orange") PORT_CODE(KEYCODE_U)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Apple") PORT_CODE(KEYCODE_I)
	
	PORT_START("KEYS2")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_COIN1)  PORT_IMPULSE(01) PORT_CODE(KEYCODE_5) // If IP_ACTIVE_LOW will cause error 30.
	PORT_BIT(0x40, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_O) // ??? 
	PORT_BIT(0x20, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_NAME("Up")PORT_CODE(KEYCODE_P) // Increase won number.
	PORT_BIT(0x10, IP_ACTIVE_LOW,  IPT_KEYPAD )PORT_NAME("Down") PORT_CODE(KEYCODE_A)  // Decrease won number.
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_S)  // ???
	PORT_BIT(0x04, IP_ACTIVE_LOW,  IPT_GAMBLE_HIGH ) PORT_NAME("Big") PORT_CODE(KEYCODE_D)
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_GAMBLE_LOW  ) PORT_NAME("Small") PORT_CODE(KEYCODE_F)
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_GAMBLE_DEAL ) PORT_NAME("Start") PORT_CODE(KEYCODE_G)

	PORT_START("DSW")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "DSW:1")
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "DSW:2")
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "DSW:3")
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "DSW:4")
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "DSW:5")
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "DSW:6")
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "DSW:7")
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "DSW:8")

	PORT_START("PUSHBUTTONS")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("K0") // K0
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("K1") // K1
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_NAME("K2") // K2
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_BUTTON4) PORT_NAME("K3") // K3
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1")
	PORT_BIT(0x10, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_NAME("P1.4") PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x20, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_NAME("P1.5") PORT_CODE(KEYCODE_X)
    PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("P1.6") PORT_CODE(KEYCODE_C) // If IP_ACTIVE_LOW Will cause Error 30 if press
    PORT_BIT(0x80, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_NAME("P1.7") PORT_CODE(KEYCODE_V) // If press during startup, it will cause error 76.
INPUT_PORTS_END

static INPUT_PORTS_START( unkwinw )
	PORT_START("KEYS1")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_W)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_E)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_R)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_T)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_U)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_I)
	
	
	PORT_START("KEYS2")
	PORT_BIT(0x80, IP_ACTIVE_LOW,  IPT_KEYPAD)  PORT_CODE(KEYCODE_O)
	PORT_BIT(0x40, IP_ACTIVE_LOW,  IPT_KEYPAD)  PORT_CODE(KEYCODE_P)
	PORT_BIT(0x20, IP_ACTIVE_LOW,  IPT_KEYPAD)  PORT_CODE(KEYCODE_A) 
	PORT_BIT(0x10, IP_ACTIVE_LOW,  IPT_KEYPAD)  PORT_CODE(KEYCODE_S)
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_KEYPAD)  PORT_CODE(KEYCODE_D)
	PORT_BIT(0x04, IP_ACTIVE_LOW,  IPT_KEYPAD)  PORT_CODE(KEYCODE_F)
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD)  PORT_CODE(KEYCODE_G)
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_GAMBLE_DEAL ) PORT_NAME("Start") PORT_CODE(KEYCODE_H)

	PORT_START("DSW")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "DSW:1")
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "DSW:2")
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "DSW:3")
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "DSW:4")
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "DSW:5")
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "DSW:6")
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "DSW:7")
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "DSW:8")

	PORT_START("PUSHBUTTONS")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("K0") // K0
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("K1") // K1
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_NAME("K2") // K2
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_BUTTON4) PORT_NAME("K3") // K3
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1")
	PORT_BIT(0x10, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_NAME("P1.4") PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x20, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_NAME("P1.5") PORT_CODE(KEYCODE_X)
    PORT_BIT(0x40, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_NAME("P1.6") PORT_CODE(KEYCODE_C)  //  will cause error 76.
    PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("P1.7") PORT_CODE(KEYCODE_V)  //  longer tone, if press, will sound normal. set to high.
INPUT_PORTS_END


void marywu_state::ay1_port_a_w(uint8_t data)
{
	for (uint8_t i = 0; i < 8; i++)
		m_leds[i] = BIT(data, i);
}

void marywu_state::ay1_port_b_w(uint8_t data)
{
	for (uint8_t i = 0; i < 8; i++)
		m_leds[i + 8] = BIT(data, i);
}

void marywu_state::ay2_port_a_w(uint8_t data)
{
	for (uint8_t i = 0; i < 8; i++)
		m_leds[i + 16] = BIT(data, i);
}

void marywu_state::ay2_port_b_w(uint8_t data)
{
	// we only have 30 LEDs. The last 2 bits in this port are unused.
	for (uint8_t i = 0; i < 6; i++)
		m_leds[i + 24] = BIT(data, i);
}

void marywu_state::multiplex_7seg_w(uint8_t data)
{
	m_selected_7seg_module = data;
}

uint8_t marywu_state::keyboard_r()
{
	switch (m_selected_7seg_module & 0x07)
	{
	case 0:
	case 1:
	case 2:
	case 3:
		return m_inputs[m_selected_7seg_module & 0x07]->read();
	default:
		return 0x00;
	}
}

void marywu_state::display_7seg_data_w(uint8_t data)
{
	static const uint8_t patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0, 0, 0, 0, 0, 0 }; // HEF4511BP (7 seg display driver)

	m_digits[2 * m_selected_7seg_module + 0] = patterns[data & 0x0f];
	m_digits[2 * m_selected_7seg_module + 1] = patterns[data >> 4];
}

void marywu_state::program_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
}

void marywu_state::data_map(address_map &map)
{
	map(0x8000, 0x87ff).mirror(0x0800).ram().share("nvram"); /* HM6116: 2kbytes of Static RAM */
	map(0x9000, 0x9001).mirror(0x0ffc).rw("ay1", FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_data_w));
	map(0x9002, 0x9003).mirror(0x0ffc).rw("ay2", FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_data_w));
	map(0xb000, 0xb001).mirror(0x0ffe).rw("i8279", FUNC(i8279_device::read), FUNC(i8279_device::write));
	map(0xf000, 0xf000).noprw();  /* TODO: Investigate this. There's something going on at this address range. Does not happen in mary1s. */
}

void marywu_state::unkwinw_program_map(address_map &map)
{
	map(0x0000, 0x0fff).rom().region("maincpu", 0);
	map(0x1000, 0xffff).rom().region("eeprom",  0x1000); 

}

void marywu_state::unkwinw_data_map(address_map &map)
{
	map(0x8000, 0x8001).rw("ay1", FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_data_w));
	map(0x8002, 0x8003).rw("ay2", FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_data_w));
    map(0x9000, 0x9000).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xa000, 0xa001).rw("i8279", FUNC(i8279_device::read), FUNC(i8279_device::write));
	map(0xb000, 0xb000).w(FUNC(marywu_state::unkwinw_adpcm_bank)); // 8 adpcm bank rom.
	map(0xf000, 0xf7ff).ram().share("nvram"); /* hm6116lp-3: 2kbytes of Static RAM */

}

void marywu_state::unkwinw_adpcm_bank(uint8_t data)
{
	
	m_oki->set_rom_bank(data & 0x07);


}

void marywu_state::machine_start()
{
	m_digits.resolve();
	m_leds.resolve();
}

void marywu_state::marywu(machine_config &config)
{
	// The 8279 (U23) and PSGs (U16 and U14) are clocked using the ALE output from the 8031.
	// This is normally 1/6th of the input clock frequency, but a pulse is skipped during external data memory accesses.
	// If the program accesses external data memory, sound generation and keyboard scanning will be affected.

	/* basic machine hardware */
	i80c31_device &maincpu(I80C31(config, "maincpu", XTAL(10'738'635))); //actual CPU is a Winbond w78c31b-24
	maincpu.set_addrmap(AS_PROGRAM, &marywu_state::program_map);
	maincpu.set_addrmap(AS_DATA, &marywu_state::data_map);
	//TODO: figure out what each bit is mapped to in the 80c31 ports P1 and P3
	maincpu.port_in_cb<1>().set_ioport("P1");

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* Keyboard & display interface */
	i8279_device &kbdc(I8279(config, "i8279", XTAL(10'738'635) / 6));
	kbdc.out_sl_callback().set(FUNC(marywu_state::multiplex_7seg_w));   // select  block of 7seg modules by multiplexing the SL scan lines
	kbdc.in_rl_callback().set(FUNC(marywu_state::keyboard_r));          // keyboard Return Lines
	kbdc.out_disp_callback().set(FUNC(marywu_state::display_7seg_data_w));

	/* Video */
	config.set_default_layout(layout_marywu);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
    
	ay8910_device &ay1(AY8910(config, "ay1", XTAL(10'738'635) / 6));
	ay1.add_route(ALL_OUTPUTS, "mono", 0.50);
	ay1.port_a_write_callback().set(FUNC(marywu_state::ay1_port_a_w));
	ay1.port_b_write_callback().set(FUNC(marywu_state::ay1_port_b_w));

	ay8910_device &ay2(AY8910(config, "ay2", XTAL(10'738'635) / 6));
	ay2.add_route(ALL_OUTPUTS, "mono", 0.50);
	ay2.port_a_write_callback().set(FUNC(marywu_state::ay2_port_a_w));
	ay2.port_b_write_callback().set(FUNC(marywu_state::ay2_port_b_w));
}

void marywu_state::mary1s(machine_config &config) 
{
	marywu(config);
	i80c52_device &maincpu(I80C52(config.replace(), "maincpu", XTAL(10'738'635))); // actual cpu is W78E52B-24. xtal jfc 10.7386 mhz
	maincpu.port_in_cb<1>().set_ioport("P1");
	maincpu.set_addrmap(AS_PROGRAM, &marywu_state::program_map);
    maincpu.set_addrmap(AS_DATA, &marywu_state::data_map);
}

void marywu_state::unkwinw(machine_config &config)
{
	marywu(config);
	i80c51_device &maincpu(I80C51(config.replace(), "maincpu", XTAL(10'738'635))); // actual cpu is at89c51
	OKIM6295(config, m_oki,  XTAL(10'738'635) / 4, okim6295_device::PIN7_LOW).add_route(ALL_OUTPUTS, "mono", 0.50);  // Clock frequency & pin 7 not verified
	maincpu.set_addrmap(AS_PROGRAM, &marywu_state::unkwinw_program_map);
	maincpu.set_addrmap(AS_DATA, &marywu_state::unkwinw_data_map);
	maincpu.port_in_cb<1>().set_ioport("P1");
}

ROM_START( marywu )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "marywu_sunkiss_chen.rom", 0x0000, 0x8000, CRC(11f67c7d) SHA1(9c1fd1a5cc6e2b0d675f0217aa8ff21c30609a0c) )
ROM_END

ROM_START( mary1s ) // actual cpu is W78E52B-24. xtal jfc 10.7386 mhz
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mary_sunrise.rom", 0x0000, 0x10000, CRC(746463A4) SHA1(065478223a809c75fe0302d1c85f129d94f503b4) )
ROM_END

ROM_START( unkwinw )

    ROM_REGION( 0x01000, "maincpu", 0 )
	ROM_LOAD( "at89c51.u1", 0x00000, 0x01000, CRC(67C00C5A) SHA1(f889522b6fff3f487183ff49e1144c9fe27cbb3e) ) // actual CPU is a Atmel at89c51. has the internal program code on it.
 		
	ROM_REGION( 0x10000, "eeprom", 0 ) 
	ROM_LOAD( "w27c512.u12", 0x0000, 0x10000, CRC(735147D8) SHA1(df2431f85224443eda4346a10183021f60d858a0) )
	
	ROM_REGION( 0x200000, "oki", 0 ) // 8 bank
	ROM_LOAD( "mx29f1615pc-10.u11", 0x000000, 0x200000, CRC(D8B7E688) SHA1(5e220f1cb963e0bc2ce37b297359f937fb097bf0) )
	
    ROM_REGION( 0x0800, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "epm7032.u8", 0x0000, 0x0800, NO_DUMP  ) // epm7032slc44-10n

ROM_END

} // anonymous namespace


//    YEAR  NAME    PARENT   MACHINE   INPUT   STATE           INIT         ROT   COMPANY               FULLNAME                                                   FLAGS
GAME( ????, marywu,  0,      marywu,   marywu,  marywu_state,  empty_init, ROT0, "<unknown>",          "unknown Labeled 'WU- MARY-1A' Music by: SunKiss Chen",    MACHINE_NOT_WORKING ) // Error 02
GAME( ????, mary1s,  0,      mary1s,   marywu,  marywu_state,  empty_init, ROT0, "<unknown>",          "unknown Labeled 'MARY-1/SUNRISE' Music by: SunKiss Chen", MACHINE_NOT_WORKING ) // Error 02

// Different Hardware.
GAME( ????, unkwinw, 0,      unkwinw,  unkwinw, marywu_state,  empty_init, ROT0,  "WIN WAY ELEC CORP", "Unknown Win way Gambling Board",                          MACHINE_NOT_WORKING ) // Error 02. Need Correct layout.
