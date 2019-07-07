// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Synertek Systems Corp. SYM-1

***************************************************************************/

#include "emu.h"

#include "cpu/m6502/m6502.h"
#include "bus/rs232/rs232.h"
#include "machine/6522via.h"
#include "machine/74145.h"
#include "machine/input_merger.h"
#include "machine/mos6530n.h"
#include "machine/ram.h"
#include "sound/spkrdev.h"

#include "speaker.h"

#include "sym1.lh"


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
		: driver_device(mconfig, type, tag),
		m_ram_1k(*this, "ram_1k"),
		m_ram_2k(*this, "ram_2k"),
		m_ram_3k(*this, "ram_3k"),
		m_monitor(*this, "monitor"),
		m_riot_ram(*this, "riot_ram"),
		m_maincpu(*this, "maincpu"),
		m_ram(*this, RAM_TAG),
		m_ttl74145(*this, "ttl74145"),
		m_crt(*this, "crt"),
		m_tty(*this, "tty"),
		m_row(*this, "ROW-%u", 0),
		m_wp(*this, "WP"),
		m_digits(*this, "digit%u", 0U)
		{ }

	void sym1(machine_config &config);

	void init_sym1();

private:
	required_shared_ptr<uint8_t> m_ram_1k;
	required_shared_ptr<uint8_t> m_ram_2k;
	required_shared_ptr<uint8_t> m_ram_3k;
	required_shared_ptr<uint8_t> m_monitor;
	required_shared_ptr<uint8_t> m_riot_ram;
	uint8_t m_riot_port_a;
	uint8_t m_riot_port_b;
	emu_timer *m_led_update;
	virtual void machine_reset() override;
	virtual void machine_start() override { m_digits.resolve(); }
	TIMER_CALLBACK_MEMBER(led_refresh);
	DECLARE_WRITE_LINE_MEMBER(sym1_74145_output_0_w);
	DECLARE_WRITE_LINE_MEMBER(sym1_74145_output_1_w);
	DECLARE_WRITE_LINE_MEMBER(sym1_74145_output_2_w);
	DECLARE_WRITE_LINE_MEMBER(sym1_74145_output_3_w);
	DECLARE_WRITE_LINE_MEMBER(sym1_74145_output_4_w);
	DECLARE_WRITE_LINE_MEMBER(sym1_74145_output_5_w);
	DECLARE_READ8_MEMBER(riot_a_r);
	DECLARE_READ8_MEMBER(riot_b_r);
	DECLARE_WRITE8_MEMBER(riot_a_w);
	DECLARE_WRITE8_MEMBER(riot_b_w);
	DECLARE_WRITE8_MEMBER(via3_a_w);

	void sym1_map(address_map &map);

	required_device<m6502_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<ttl74145_device> m_ttl74145;
	required_device<rs232_port_device> m_crt;
	required_device<rs232_port_device> m_tty;
	required_ioport_array<4> m_row;
	required_ioport m_wp;
	output_finder<6> m_digits;
};


//**************************************************************************
//  KEYBOARD INPUT & LED OUTPUT
//**************************************************************************

WRITE_LINE_MEMBER( sym1_state::sym1_74145_output_0_w ) { if (state) m_led_update->adjust(LED_REFRESH_DELAY, 0); }
WRITE_LINE_MEMBER( sym1_state::sym1_74145_output_1_w ) { if (state) m_led_update->adjust(LED_REFRESH_DELAY, 1); }
WRITE_LINE_MEMBER( sym1_state::sym1_74145_output_2_w ) { if (state) m_led_update->adjust(LED_REFRESH_DELAY, 2); }
WRITE_LINE_MEMBER( sym1_state::sym1_74145_output_3_w ) { if (state) m_led_update->adjust(LED_REFRESH_DELAY, 3); }
WRITE_LINE_MEMBER( sym1_state::sym1_74145_output_4_w ) { if (state) m_led_update->adjust(LED_REFRESH_DELAY, 4); }
WRITE_LINE_MEMBER( sym1_state::sym1_74145_output_5_w ) { if (state) m_led_update->adjust(LED_REFRESH_DELAY, 5); }

TIMER_CALLBACK_MEMBER(sym1_state::led_refresh)
{
	m_digits[param] = m_riot_port_a;
}

READ8_MEMBER(sym1_state::riot_a_r)
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

READ8_MEMBER(sym1_state::riot_b_r)
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

WRITE8_MEMBER(sym1_state::riot_a_w)
{
	logerror("%x: riot_a_w 0x%02x\n", m_maincpu->pc(), data);

	// save for later use
	m_riot_port_a = data;
}

WRITE8_MEMBER(sym1_state::riot_b_w)
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

/*
    PA0: Write protect R6532 RAM
    PA1: Write protect RAM 0x400-0x7ff
    PA2: Write protect RAM 0x800-0xbff
    PA3: Write protect RAM 0xc00-0xfff
 */
WRITE8_MEMBER( sym1_state::via3_a_w )
{
	address_space &cpu0space = m_maincpu->space( AS_PROGRAM );

	logerror("SYM1 VIA2 W 0x%02x\n", data);

	if ((m_wp->read() & 0x01) && !(data & 0x01)) {
		cpu0space.nop_write(0xa600, 0xa67f);
	} else {
		cpu0space.install_write_bank(0xa600, 0xa67f, "bank5");
	}
	if ((m_wp->read() & 0x02) && !(data & 0x02)) {
		cpu0space.nop_write(0x0400, 0x07ff);
	} else {
		cpu0space.install_write_bank(0x0400, 0x07ff, "bank2");
	}
	if ((m_wp->read() & 0x04) && !(data & 0x04)) {
		cpu0space.nop_write(0x0800, 0x0bff);
	} else {
		cpu0space.install_write_bank(0x0800, 0x0bff, "bank3");
	}
	if ((m_wp->read() & 0x08) && !(data & 0x08)) {
		cpu0space.nop_write(0x0c00, 0x0fff);
	} else {
		cpu0space.install_write_bank(0x0c00, 0x0fff, "bank4");
	}
}

void sym1_state::init_sym1()
{
	// wipe expansion memory banks that are not installed
	if (m_ram->size() < 4*1024)
	{
		m_maincpu->space(AS_PROGRAM).nop_readwrite(m_ram->size(), 0x0fff);
	}

	// allocate a timer to refresh the led display
	m_led_update = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(sym1_state::led_refresh), this));
}

void sym1_state::machine_reset()
{
	// make 0xf800 to 0xffff point to the last half of the monitor ROM
	// so that the CPU can find its reset vectors
	m_maincpu->space(AS_PROGRAM).install_read_bank(0xf800, 0xffff, "bank1");
	m_maincpu->space(AS_PROGRAM).nop_write(0xf800, 0xffff);
	membank("bank1")->set_base(m_monitor + 0x800);
	m_maincpu->reset();
}


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void sym1_state::sym1_map(address_map &map)
{
	map(0x0000, 0x03ff).ram(); // U12/U13 RAM
	map(0x0400, 0x07ff).bankrw("bank2").share("ram_1k");
	map(0x0800, 0x0bff).bankrw("bank3").share("ram_2k");
	map(0x0c00, 0x0fff).bankrw("bank4").share("ram_3k");
	map(0x8000, 0x8fff).rom().share("monitor"); // U20 Monitor ROM
	map(0xa000, 0xa00f).m("via1", FUNC(via6522_device::map));  // U25 VIA #1
	map(0xa400, 0xa41f).m("riot", FUNC(mos6532_new_device::io_map));  // U27 RIOT
	map(0xa600, 0xa67f).bankrw("bank5").share("riot_ram");   // U27 RIOT RAM
	map(0xa800, 0xa80f).m("via2", FUNC(via6522_device::map));  // U28 VIA #2
	map(0xac00, 0xac0f).m("via3", FUNC(via6522_device::map));  // U29 VIA #3
	map(0xb000, 0xefff).rom();
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
	mos6532_new_device &riot(MOS6532_NEW(config, "riot", SYM1_CLOCK));
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
	// lines 7-9 not connected

	VIA6522(config, "via1", SYM1_CLOCK).irq_handler().set("mainirq", FUNC(input_merger_device::in_w<0>));
	VIA6522(config, "via2", SYM1_CLOCK).irq_handler().set("mainirq", FUNC(input_merger_device::in_w<1>));

	via6522_device &via3(VIA6522(config, "via3", SYM1_CLOCK));
	via3.writepa_handler().set(FUNC(sym1_state::via3_a_w));
	via3.irq_handler().set("mainirq", FUNC(input_merger_device::in_w<2>));

	input_merger_device &merger(INPUT_MERGER_ANY_HIGH(config, "mainirq", 0)); // wire-or connection
	merger.output_handler().set_inputline(m_maincpu, M6502_IRQ_LINE);

	RS232_PORT(config, "crt", default_rs232_devices, nullptr);
	RS232_PORT(config, "tty", default_rs232_devices, nullptr); // actually a 20 mA current loop; 110 bps assumed

	// internal ram
	RAM(config, m_ram);
	m_ram->set_default_size("4K");
	m_ram->set_extra_options("1K,2K,3K");
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


//**************************************************************************
//  GAME DRIVERS
//**************************************************************************

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT       COMPANY                   FULLNAME          FLAGS
COMP( 1978, sym1, 0,      0,      sym1,    sym1,  sym1_state, init_sym1, "Synertek Systems Corp.", "SYM-1/SY-VIM-1", 0 )
