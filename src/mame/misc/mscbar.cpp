// license::BSD-3-Clause
// copyright-holders:flama12333
/*************************************************************************
 mscbar
  Product name: 小方玛丽板
  Product Code: square-mario-baord
  The board has a single marking J373.
  String: COPYRIGHT BY WIN WAY ELEC. CORP. CLEMENT CHANG, MUSIC by: SunKiss Chen

  Mainboard
  Size 18 x 16.5cm.
  3V6 BAT - backup battery to keep nvram
  U1 AT89C51-24PC - internal rom.
  U2 HM6118LP-3
  U8 EPM7032SLC44-10N
  U9 TOP 8279
  U10 U6295
  U11 MX 29F1615PC-10
  U12 W27C512-45Z - eeprom
  Y1 10.7386 xtal mhz
  
  At back.
  MUSICBAR VER 201.

  Led Board
  size 29 x 20.5cm.

  The LED board says GHY1-PCB and BYP-020
  U5 Two JFC 95101
  
  02 - led - Middle - double up
  06 - led - Bottom
  24 - led - roulette
  09 - 7 segment display - Bottom -  bet.
  04 - 7 segment display - Top left - win.
  04 - 7 segment display - Top right - credits.
  01 - 7 segment display - Middle - double up.
  
  Unpopulated 
  ld32 - led  
  ds14 ds15 2x  7 segment display 2 digit.

  Todo:
  Need layout.
  Meter In and Out.
  Verify Hooper hook up.
  Verify P3 Led output.
**************************************************************************/

#include "emu.h"

#include "cpu/mcs51/i80c51.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "machine/i8279.h"
#include "sound/ay8910.h"
#include "sound/okim6295.h"
#include "speaker.h"
#include "mscbar.lh"


namespace {

class mscbar_state : public driver_device
{
public:
	mscbar_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_digits(*this, "digit%u", 0U)
		, m_leds(*this, "led%u", 0U)
		, m_inputs(*this, { "KEYS1", "KEYS2", "DSW", "PUSHBUTTONS" })
		, m_oki(*this, "oki")
		, m_hopper(*this, "hopper")

	{ }

	void mscbar(machine_config &config);
	void mscbar_adpcm_bank(uint8_t data) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void display_7seg_data_w(uint8_t data);
	void multiplex_7seg_w(uint8_t data);
	void ay1_port_a_w(uint8_t data);
	void ay1_port_b_w(uint8_t data);
	void ay2_port_a_w(uint8_t data);
	void ay2_port_b_w(uint8_t data);
	void p3_port_w(uint8_t data);
	void p1_port_w(uint8_t data);

	uint8_t keyboard_r();
	void mscbar_data_map(address_map &map) ATTR_COLD;
	void mscbar_program_map(address_map &map) ATTR_COLD;

	uint8_t m_selected_7seg_module = 0;
    
    output_finder<32> m_digits;
	output_finder<34> m_leds;
	required_ioport_array<4> m_inputs;
	optional_device<okim6295_device> m_oki;
	required_device<hopper_device> m_hopper;

};

static INPUT_PORTS_START( mscbar )
	PORT_START("KEYS1")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Bet 4")PORT_CODE(KEYCODE_F)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Bet 3")PORT_CODE(KEYCODE_D)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Bet 2")PORT_CODE(KEYCODE_S)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Bet 1")PORT_CODE(KEYCODE_A)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Bet 6")PORT_CODE(KEYCODE_H)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Bet 7")PORT_CODE(KEYCODE_J)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Bet 8")PORT_CODE(KEYCODE_K)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Bet 9") PORT_CODE(KEYCODE_L)

    PORT_START("KEYS2")
	PORT_BIT(0x80, IP_ACTIVE_LOW,  IPT_GAMBLE_TAKE)  PORT_NAME("Credits") PORT_CODE(KEYCODE_Q) 
	PORT_BIT(0x40, IP_ACTIVE_LOW,  IPT_KEYPAD)  PORT_NAME("Bonus") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x20, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_T) // ???
	PORT_BIT(0x10, IP_ACTIVE_LOW,  IPT_KEYPAD)  PORT_NAME("Bet 5") PORT_CODE(KEYCODE_G)
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_GAMBLE_PAYOUT ) PORT_NAME("Payout") 
	PORT_BIT(0x04, IP_ACTIVE_LOW,  IPT_GAMBLE_HIGH)  PORT_CODE(KEYCODE_E)
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_GAMBLE_LOW)   PORT_CODE(KEYCODE_R)
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_START1) PORT_NAME("Start / Take Score")

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
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT) PORT_NAME("Clear Credits") PORT_CODE(KEYCODE_C)  //  will cause error 76.
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(hopper_device::line_r))  // For Hopper.

	PORT_START("P3")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(8)  // Coin
    INPUT_PORTS_END

void mscbar_state::ay1_port_a_w(uint8_t data)
{   
	for (uint8_t i = 0; i < 8; i++)
		m_leds[i] = BIT(~data, i);
}

void mscbar_state::ay1_port_b_w(uint8_t data)
{   
	for (uint8_t i = 0; i < 8; i++)
		m_leds[i + 8] = BIT(~data, i);
}

void mscbar_state::ay2_port_a_w(uint8_t data)
{   
	for (uint8_t i = 0; i < 8; i++)
		m_leds[i + 16] = BIT(~data, i);
}

void mscbar_state::ay2_port_b_w(uint8_t data)
{
    for (uint8_t i = 0; i < 6; i++)
    m_leds[i + 24] = BIT(~data, i);
    m_leds[30] = BIT(~data, 6); // Unused - only used during led test - K1
	m_leds[31] = BIT(~data, 7); // Unused - only used during led test - K1. after start the roulette will start to blink. special bonus led?
}

void mscbar_state::p1_port_w(uint8_t data)
{
 m_hopper->motor_w(BIT(data, 3));
}

void mscbar_state::p3_port_w(uint8_t data)  // bit 3 and 5 are used.
{
	m_leds[33] = BIT(data, 3);
    m_leds[32] = BIT(data, 5);
}
	
void mscbar_state::multiplex_7seg_w(uint8_t data)
{

	m_selected_7seg_module = data;
}

uint8_t mscbar_state::keyboard_r()
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

void mscbar_state::display_7seg_data_w(uint8_t data)
{
	static const uint8_t patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0, 0, 0, 0, 0, 0 }; // code from marywu.

	m_digits[2 * m_selected_7seg_module + 0] = patterns[data & 0x0f];
	m_digits[2 * m_selected_7seg_module + 1] = patterns[data >> 4];
}

void mscbar_state::mscbar_adpcm_bank(uint8_t data)
{
		m_oki->set_rom_bank(data & 0x07);
}

void mscbar_state::mscbar_program_map(address_map &map)
{
	map(0x0000, 0x0fff).rom().region("maincpu", 0);
	map(0x1000, 0xffff).rom().region("eeprom",  0x1000);
}

void mscbar_state::mscbar_data_map(address_map &map)
{
	map(0x8000, 0x8001).w("ay1", FUNC(ay8910_device::address_data_w));
	map(0x8002, 0x8003).w("ay2", FUNC(ay8910_device::address_data_w)); 
	map(0x9000, 0x9000).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xa000, 0xa001).rw("i8279", FUNC(i8279_device::read), FUNC(i8279_device::write));
	map(0xb000, 0xb000).w(FUNC(mscbar_state::mscbar_adpcm_bank)); /* adpcm bank */
	map(0xf000, 0xf7ff).ram().share("nvram"); /* HM6116LP-3: 2kb of Static RAM */
}

void mscbar_state::mscbar(machine_config &config)
{
	i80c51_device &maincpu(I80C51(config, "maincpu", XTAL(10'738'635))); // actual cpu is at89c51
	maincpu.set_addrmap(AS_PROGRAM, &mscbar_state::mscbar_program_map);
	maincpu.set_addrmap(AS_DATA, &mscbar_state::mscbar_data_map);
	maincpu.port_in_cb<1>().set_ioport("P1");
    maincpu.port_out_cb<1>().set(FUNC(mscbar_state::p1_port_w));
    maincpu.port_out_cb<3>().set(FUNC(mscbar_state::p3_port_w));
	maincpu.port_in_cb<3>().set_ioport("P3");

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
    HOPPER(config, m_hopper, attotime::from_msec(100));
	
	/* Video */
	config.set_default_layout(layout_mscbar);

	/* Keyboard & display interface */
	i8279_device &kbdc(I8279(config, "i8279", XTAL(10'738'635) / 6));
	kbdc.out_sl_callback().set(FUNC(mscbar_state::multiplex_7seg_w));   // select  block of 7seg modules by multiplexing the SL scan lines
	kbdc.in_rl_callback().set(FUNC(mscbar_state::keyboard_r));          // keyboard Return Lines
	kbdc.out_disp_callback().set(FUNC(mscbar_state::display_7seg_data_w));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	ay8910_device &ay1(AY8910(config, "ay1", XTAL(10'738'635) / 6));
	ay1.add_route(ALL_OUTPUTS, "mono", 0.50);
	ay1.port_a_write_callback().set(FUNC(mscbar_state::ay1_port_a_w));
	ay1.port_b_write_callback().set(FUNC(mscbar_state::ay1_port_b_w));

	ay8910_device &ay2(AY8910(config, "ay2", XTAL(10'738'635) / 6));
	ay2.add_route(ALL_OUTPUTS, "mono", 0.50);
	ay2.port_a_write_callback().set(FUNC(mscbar_state::ay2_port_a_w));
	ay2.port_b_write_callback().set(FUNC(mscbar_state::ay2_port_b_w));
   
    OKIM6295(config, m_oki,  XTAL(10'738'635) / 4, okim6295_device::PIN7_LOW).add_route(ALL_OUTPUTS, "mono", 1.00);  // Clock frequency & pin 7 not verified
}

void mscbar_state::machine_start()
{
	m_digits.resolve();
	m_leds.resolve();

}

ROM_START( mscbar )

	ROM_REGION( 0x01000, "maincpu", 0 )
	ROM_LOAD( "at89c51-24pc.u1", 0x00000, 0x01000, CRC(67C00C5A) SHA1(f889522b6fff3f487183ff49e1144c9fe27cbb3e) ) // actual CPU is a ATMEL AT89C51-24PC. Unprotected. Has the internal program code on it.

	ROM_REGION( 0x10000, "eeprom", 0 )
	ROM_LOAD( "w27c512.u12", 0x0000, 0x10000, CRC(735147D8) SHA1(df2431f85224443eda4346a10183021f60d858a0) )

	ROM_REGION( 0x200000, "oki", 0 ) // adpcm rom - 8 bank
	ROM_LOAD( "mx29f1615pc-10.u11", 0x000000, 0x200000, CRC(D8B7E688) SHA1(5e220f1cb963e0bc2ce37b297359f937fb097bf0) )

	ROM_REGION( 0x0800, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "epm7032slc44-10n.u8", 0x0000, 0x0800, NO_DUMP  )
ROM_END

} // anonymous namespace


//    YEAR  NAME    PARENT   MACHINE   INPUT   STATE           INIT         ROT   COMPANY               FULLNAME                                FLAGS
GAME( 20??, mscbar, 0,       mscbar,   mscbar,  mscbar_state,  empty_init, ROT0,  "WIN WAY ELEC CORP", "unknown Labeled 'MUSICBAR VER 201'",    MACHINE_NOT_WORKING ) // Error 02
