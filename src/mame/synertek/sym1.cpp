// license:GPL-2.0+
// copyright-holders:Dirk Best
/**********************************************************************************************

Synertek Systems Corp. SYM-1

Using the cassette (bios 0 only)
- To save: set tape to record, press F2, type id-start-end (e.g. 3-200-210), press enter.
- To load: press F1, type the id, press enter.

TODO:
- Digits should go blank during cassette save
- How to use cassette with -bios 1 ??
- TTY/CRT interfaces not working
- You can't show the TTY/CRT terminal screen and the SYM-1 screen at the same time
- Need software (SW-list is set up, but there's nothing for it)

***********************************************************************************************/

#include "emu.h"

#include "cpu/m6502/m6502.h"
#include "bus/rs232/rs232.h"
#include "imagedev/cassette.h"
#include "machine/6522via.h"
#include "machine/74145.h"
#include "machine/input_merger.h"
#include "machine/mos6530.h"
#include "machine/ram.h"
#include "machine/timer.h"
#include "sound/spkrdev.h"

#include "softlist_dev.h"
#include "speaker.h"

#include "sym1.lh"


namespace {

//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

// SYM-1 main (and only) oscillator Y1
#define SYM1_CLOCK  XTAL(1'000'000)

#define LED_REFRESH_DELAY   attotime::from_usec(70)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class sym1_state : public driver_device
{
public:
	sym1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ram(*this, RAM_TAG)
		, m_banks(*this, "bank%u", 0U)
		, m_ttl74145(*this, "ttl74145")
		, m_via1(*this, "via1")
		, m_crt(*this, "crt")
		, m_tty(*this, "tty")
		, m_cass(*this, "cassette")
		, m_row(*this, "ROW-%u", 0U)
		, m_wp(*this, "WP")
		, m_digits(*this, "digit%u", 0U)
		{ }

	void sym1(machine_config &config);

	void init_sym1();

private:
	uint8_t m_riot_port_a;
	uint8_t m_riot_port_b;
	emu_timer *m_led_update;
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override { m_digits.resolve(); }
	TIMER_CALLBACK_MEMBER(led_refresh);
	TIMER_DEVICE_CALLBACK_MEMBER(cass_r);
	void sym1_74145_output_0_w(int state);
	void sym1_74145_output_1_w(int state);
	void sym1_74145_output_2_w(int state);
	void sym1_74145_output_3_w(int state);
	void sym1_74145_output_4_w(int state);
	void sym1_74145_output_5_w(int state);
	void sym1_74145_output_7_w(int state);
	void via1_ca2_w(int state);
	void via1_cb2_w(int state);
	uint8_t riot_a_r();
	uint8_t riot_b_r();
	void riot_a_w(uint8_t data);
	void riot_b_w(uint8_t data);
	void via3_a_w(uint8_t data);

	std::unique_ptr<u8[]> m_riot_ram;
	std::unique_ptr<u8[]> m_dummy_ram;
	bool m_cb2 = false;
	void sym1_map(address_map &map) ATTR_COLD;

	required_device<m6502_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_memory_bank_array<10> m_banks;
	required_device<ttl74145_device> m_ttl74145;
	required_device<via6522_device> m_via1;
	required_device<rs232_port_device> m_crt;
	required_device<rs232_port_device> m_tty;
	required_device<cassette_image_device> m_cass;
	required_ioport_array<4> m_row;
	required_ioport m_wp;
	output_finder<6> m_digits;
};


//**************************************************************************
//  KEYBOARD INPUT & LED OUTPUT
//**************************************************************************

void sym1_state::sym1_74145_output_0_w(int state) { if (state) m_led_update->adjust(LED_REFRESH_DELAY, 0); }
void sym1_state::sym1_74145_output_1_w(int state) { if (state) m_led_update->adjust(LED_REFRESH_DELAY, 1); }
void sym1_state::sym1_74145_output_2_w(int state) { if (state) m_led_update->adjust(LED_REFRESH_DELAY, 2); }
void sym1_state::sym1_74145_output_3_w(int state) { if (state) m_led_update->adjust(LED_REFRESH_DELAY, 3); }
void sym1_state::sym1_74145_output_4_w(int state) { if (state) m_led_update->adjust(LED_REFRESH_DELAY, 4); }
void sym1_state::sym1_74145_output_5_w(int state) { if (state) m_led_update->adjust(LED_REFRESH_DELAY, 5); }
void sym1_state::sym1_74145_output_7_w(int state) { m_cass->output( state ? -1.0 : +1.0); }

void sym1_state::via1_cb2_w(int state)
{
	m_cb2 = state;
	m_cass->change_state(state ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);
}

TIMER_CALLBACK_MEMBER(sym1_state::led_refresh)
{
	m_digits[param] = m_riot_port_a;
}

TIMER_DEVICE_CALLBACK_MEMBER(sym1_state::cass_r)
{
	if (!m_cb2)
		return;
	bool cass_ws = (m_cass->input() > +0.03) ? 1 : 0;
	m_via1->write_pb6(cass_ws);
}

uint8_t sym1_state::riot_a_r()
{
	int data = 0x7f;

	// scan keypad rows
	if (!(m_riot_port_a & 0x80)) data &= m_row[0]->read();
	if (!(m_riot_port_b & 0x01)) data &= m_row[1]->read();
	if (!(m_riot_port_b & 0x02)) data &= m_row[2]->read();
	if (!(m_riot_port_b & 0x04)) data &= m_row[3]->read();

	// determine column
	if ( ((m_riot_port_a ^ 0xff) & (m_row[0]->read() ^ 0xff)) & 0x7f )
		data &= ~0x80;

	return data;
}

uint8_t sym1_state::riot_b_r()
{
	int data = 0x3f;

	// determine column
	if ( ((m_riot_port_a ^ 0xff) & (m_row[1]->read() ^ 0xff)) & 0x7f )
		data &= ~0x01;

	if ( ((m_riot_port_a ^ 0xff) & (m_row[2]->read() ^ 0xff)) & 0x3f )
		data &= ~0x02;

	if ( ((m_riot_port_a ^ 0xff) & (m_row[3]->read() ^ 0xff)) & 0x1f )
		data &= ~0x04;

	// PB6 in from TTY keyboard
	data |= m_tty->rxd_r() << 6;

	// PB7 in from RS-232 CRT
	data |= m_crt->rxd_r() << 7;

	data &= ~0x80; // else hangs 8b02

	return data;
}

void sym1_state::riot_a_w(uint8_t data)
{
	logerror("%x: riot_a_w 0x%02x\n", m_maincpu->pc(), data);

	// save for later use
	m_riot_port_a = data;
}

void sym1_state::riot_b_w(uint8_t data)
{
	logerror("%x: riot_b_w 0x%02x\n", m_maincpu->pc(), data);

	// save for later use
	m_riot_port_b = data;

	// first 4 pins are connected to the 74145
	m_ttl74145->write(data & 0x0f);

	// PB4 out to RS-232 CRT
	m_crt->write_txd(BIT(data, 4));

	// PB5 out to TTY printer
	m_tty->write_txd(BIT(data, 5));
}

//**************************************************************************
//  INPUT PORTS
//**************************************************************************

static INPUT_PORTS_START( sym1 )
	PORT_START("ROW-0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0     USR 0") PORT_CODE(KEYCODE_0)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4     USR 4") PORT_CODE(KEYCODE_4)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8     JUMP")  PORT_CODE(KEYCODE_8)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C     CALC")  PORT_CODE(KEYCODE_C)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CR    S DBL") PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("GO    LD P")  PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LD 2  LD 1")  PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("ROW-1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1     USR 1") PORT_CODE(KEYCODE_1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5     USR 5") PORT_CODE(KEYCODE_5)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9     VER")   PORT_CODE(KEYCODE_9)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D     DEP")   PORT_CODE(KEYCODE_D)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("-     +")     PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("REG   SAV P") PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SAV 2 SAV 1") PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ROW-2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2     USR 2") PORT_CODE(KEYCODE_2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6     USR 6") PORT_CODE(KEYCODE_6)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A     ASCII") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E     EXEC")  PORT_CODE(KEYCODE_E)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\xE2\x86\x92     \xE2\x86\x90") PORT_CODE(KEYCODE_RIGHT) PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("MEM   WP")    PORT_CODE(KEYCODE_F5)
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ROW-3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3     USR 3") PORT_CODE(KEYCODE_3)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7     USR 7") PORT_CODE(KEYCODE_7)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B     B MOV") PORT_CODE(KEYCODE_B)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F     FILL")  PORT_CODE(KEYCODE_F)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SHIFT")       PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN4")           /* IN4 */
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DEBUG OFF") PORT_CODE(KEYCODE_F6)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DEBUG ON")  PORT_CODE(KEYCODE_F7)

	PORT_START("WP")
	PORT_DIPNAME(0x01, 0x01, "6532 RAM WP")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x01, DEF_STR(On))
	PORT_DIPNAME(0x02, 0x02, "1K RAM WP")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x02, DEF_STR(On))
	PORT_DIPNAME(0x04, 0x04, "2K RAM WP")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x04, DEF_STR(On))
	PORT_DIPNAME(0x08, 0x08, "3K RAM WP")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x08, DEF_STR(On))
INPUT_PORTS_END


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

// CA2 will be forced high when the VIA is reset, causing the ROM to be switched in
// When the bios clears POR, FF80-FFFF becomes a mirror of A600-A67F
void sym1_state::via1_ca2_w(int state)
{
	m_banks[8]->set_entry(state);
}

/*
    PA0: Write protect R6532 RAM
    PA1: Write protect RAM 0x400-0x7ff
    PA2: Write protect RAM 0x800-0xbff
    PA3: Write protect RAM 0xc00-0xfff
 */
void sym1_state::via3_a_w(uint8_t data)
{
	logerror("SYM1 VIA2 W 0x%02x\n", data);

	u8 sw = m_wp->read();

	// apply or remove write-protection as directed
	for (u8 i = 0; i < 4; i++)
	{
		// considered readonly if DIP is on AND databit is low; OR if memory not fitted
		if ((BIT(sw, i) && !BIT(data, i)) || m_banks[i*2]->entry() )
			m_banks[i*2+1]->set_entry(1);  // readonly
		else
			m_banks[i*2+1]->set_entry(0);  // readwrite
		//printf("Bank %d has entry %d\n",i*2+1,m_banks[i*2+1]->entry());
	}

	// POR write is same as bank1
	m_banks[9]->set_entry(m_banks[1]->entry());
}

void sym1_state::init_sym1()
{
	// m_ram 000-3FF not allocated to anything, so we use it as a dummy write area
	u8 *const m = memregion("maincpu")->base()+0x8f80;
	m_riot_ram = make_unique_clear<u8[]>(0x80);
	m_dummy_ram = make_unique_clear<u8[]>(0x400); // dummy read area, preset to FF
	u8 *w = m_riot_ram.get();
	u8 *x = m_dummy_ram.get();
	std::memset(x, 0xff, 0x400);
	u8 *r = m_ram->pointer();
	// RAM 400-7FF
	m_banks[2]->configure_entry(0, r+0x400);    // ram exist, readable
	m_banks[2]->configure_entry(1, x);          // ram not present
	m_banks[3]->configure_entry(0, r+0x400);    // ram exist, writable
	m_banks[3]->configure_entry(1, r);          // ram not present or readonly
	// RAM 800-BFF
	m_banks[4]->configure_entry(0, r+0x800);
	m_banks[4]->configure_entry(1, x);
	m_banks[5]->configure_entry(0, r+0x800);
	m_banks[5]->configure_entry(1, r);
	// RAM C00-FFF
	m_banks[6]->configure_entry(0, r+0xc00);
	m_banks[6]->configure_entry(1, x);
	m_banks[7]->configure_entry(0, r+0xc00);
	m_banks[7]->configure_entry(1, r);
	// RIOT RAM A600-A67F
	m_banks[0]->configure_entry(0, w);          // riot ram, readable
	m_banks[0]->configure_entry(1, w);
	m_banks[1]->configure_entry(0, w);          // riot ram, writable
	m_banks[1]->configure_entry(1, r);          // riot ram, readonly
	// POR
	m_banks[8]->configure_entry(0, w);          // point at riot-ram
	m_banks[8]->configure_entry(1, m);          // point at rom
	m_banks[9]->configure_entry(0, w);          // riot ram, writable
	m_banks[9]->configure_entry(1, r);          // riot ram, readonly

	for (auto &bank : m_banks)
		bank->set_entry(1);

	// allocate a timer to refresh the led display
	m_led_update = timer_alloc(FUNC(sym1_state::led_refresh), this);
}

void sym1_state::machine_reset()
{
	// Enable extra ram if it is fitted
	switch (m_ram->size())
	{
		case 4*1024:
			m_banks[6]->set_entry(0);
			[[fallthrough]];
		case 3*1024:
			m_banks[4]->set_entry(0);
			[[fallthrough]];
		case 2*1024:
			m_banks[2]->set_entry(0);
			[[fallthrough]];
		default:
			m_banks[0]->set_entry(0);
			break;
	}

	// Enable POR
	via1_ca2_w(1);
}


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void sym1_state::sym1_map(address_map &map)
{
	map(0x0000, 0x03ff).ram(); // U12/U13 RAM
	map(0x0400, 0x07ff).bankr("bank2").bankw("bank3");  // U14/U15 OPT RAM
	map(0x0800, 0x0bff).bankr("bank4").bankw("bank5");  // U16/U17 OPT RAM
	map(0x0c00, 0x0fff).bankr("bank6").bankw("bank7");  // U18/U19 OPT RAM
	map(0x8000, 0x8fff).rom(); // U20 Monitor ROM
	map(0xa000, 0xa00f).m("via1", FUNC(via6522_device::map));  // U25 VIA #1
	map(0xa400, 0xa41f).m("riot", FUNC(mos6532_device::io_map));  // U27 RIOT
	map(0xa600, 0xa67f).bankr("bank0").bankw("bank1");   // U27 RIOT RAM
	map(0xa800, 0xa80f).m("via2", FUNC(via6522_device::map));  // U28 VIA #2
	map(0xac00, 0xac0f).m("via3", FUNC(via6522_device::map));  // U29 VIA #3
	map(0xb000, 0xefff).rom();
	map(0xff80, 0xffff).bankr("bank8").bankw("bank9");   // POR
}


//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

void sym1_state::sym1(machine_config &config)
{
	// basic machine hardware
	M6502(config, m_maincpu, SYM1_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &sym1_state::sym1_map);

	config.set_default_layout(layout_sym1);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, "mono", 1.0);

	// devices
	mos6532_device &riot(MOS6532(config, "riot", SYM1_CLOCK));
	riot.pa_rd_callback().set(FUNC(sym1_state::riot_a_r));
	riot.pa_wr_callback().set(FUNC(sym1_state::riot_a_w));
	riot.pb_rd_callback().set(FUNC(sym1_state::riot_b_r));
	riot.pb_wr_callback().set(FUNC(sym1_state::riot_b_w));

	TTL74145(config, m_ttl74145, 0);
	m_ttl74145->output_line_callback<0>().set(FUNC(sym1_state::sym1_74145_output_0_w));
	m_ttl74145->output_line_callback<1>().set(FUNC(sym1_state::sym1_74145_output_1_w));
	m_ttl74145->output_line_callback<2>().set(FUNC(sym1_state::sym1_74145_output_2_w));
	m_ttl74145->output_line_callback<3>().set(FUNC(sym1_state::sym1_74145_output_3_w));
	m_ttl74145->output_line_callback<4>().set(FUNC(sym1_state::sym1_74145_output_4_w));
	m_ttl74145->output_line_callback<5>().set(FUNC(sym1_state::sym1_74145_output_5_w));
	m_ttl74145->output_line_callback<6>().set("speaker", FUNC(speaker_sound_device::level_w));
	m_ttl74145->output_line_callback<7>().set(FUNC(sym1_state::sym1_74145_output_7_w));
	// lines 7-9 not connected

	MOS6522(config, m_via1, SYM1_CLOCK);
	m_via1->irq_handler().set("mainirq", FUNC(input_merger_device::in_w<0>));
	m_via1->ca2_handler().set(FUNC(sym1_state::via1_ca2_w));
	m_via1->cb2_handler().set(FUNC(sym1_state::via1_cb2_w));

	MOS6522(config, "via2", SYM1_CLOCK).irq_handler().set("mainirq", FUNC(input_merger_device::in_w<1>));

	via6522_device &via3(MOS6522(config, "via3", SYM1_CLOCK));
	via3.writepa_handler().set(FUNC(sym1_state::via3_a_w));
	via3.irq_handler().set("mainirq", FUNC(input_merger_device::in_w<2>));

	input_merger_device &merger(INPUT_MERGER_ANY_HIGH(config, "mainirq", 0)); // wire-or connection
	merger.output_handler().set_inputline(m_maincpu, M6502_IRQ_LINE);

	RS232_PORT(config, "crt", default_rs232_devices, nullptr);
	RS232_PORT(config, "tty", default_rs232_devices, nullptr); // actually a 20 mA current loop; 110 bps assumed

	/* cassette */
	CASSETTE(config, m_cass);
	m_cass->set_default_state(CASSETTE_PLAY | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);
	m_cass->set_interface("sym1_cass");
	TIMER(config, "cass_r").configure_periodic(FUNC(sym1_state::cass_r), attotime::from_hz(40000));

	// internal ram
	RAM(config, m_ram);
	m_ram->set_default_size("4K");
	m_ram->set_extra_options("1K,2K,3K");

	SOFTWARE_LIST(config, "cass_list").set_original("sym1_cass");
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( sym1 )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_SYSTEM_BIOS(0, "ver10",  "Version 1.0")
	ROMX_LOAD("symon1_0.bin", 0x8000, 0x1000, CRC(97928583) SHA1(6ac52c54adb7a086d51bc7f6d55dd30ab3a0a331), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "ver11",  "Version 1.1")
	ROMX_LOAD("symon1_1.bin", 0x8000, 0x1000, CRC(7a4b1e12) SHA1(cebdf815105592658cfb7af262f2101d2aeab786), ROM_BIOS(1))
	ROM_LOAD("rae_b000", 0xb000, 0x1000, CRC(f6429326) SHA1(6f2f10649b54f54217bb35c8c453b5d05434bd86) )
	ROM_LOAD("bas_c000", 0xc000, 0x1000, CRC(c168fe70) SHA1(7447a5e229140cbbde4cf90886966a5d93aa24e1) )
	ROM_LOAD("bas_d000", 0xd000, 0x1000, CRC(8375a978) SHA1(240301bf8bb8ddb99b65a585f17895e1ad872631) )
	ROM_LOAD("rae_e000", 0xe000, 0x1000, CRC(2255444b) SHA1(c7dd812962c2e2edd2faa7055e9cce4e769c0388) )
ROM_END

} // anonymous namespace


//**************************************************************************
//  GAME DRIVERS
//**************************************************************************

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT       COMPANY                   FULLNAME          FLAGS
COMP( 1978, sym1, 0,      0,      sym1,    sym1,  sym1_state, init_sym1, "Synertek Systems Corp.", "SYM-1/SY-VIM-1", 0 )
