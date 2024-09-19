// license:BSD-3-Clause
// copyright-holders:Robbbert
/******************************************************************************************************

Ultim809 homebrew computer by Matthew Sarnoff, 2011
http://www.msarnoff.org/6809/

Was developed during 2009-2011, but unknown if it was ever finished. Most of the claimed
features don't seem to be there or are inaccessible.

2020-06-04 Skeleton [Robbbert]

If someone knows more about the system, please update this source.


ToDo:
- Mirrors
- Video RAM has 2 banks of 0x4000 each. There's no way to select banks in the VDP code.
- Sound (hooked up, unable to test)
- FTDI connector
- Keyboard, PS/2 type from an older PC, connects to VIA PB7 and CA1.
- Joysticks (Sega gamepad, or Atari joystick only).
- Reset button, Run/Halt switch.
- Power LED, Bus status LEDs (2), User status LEDs (2).
- RTC type DS1307, connects to VIA PB0,PB1,PB6; xtal 32'768; battery CR2032.
- SD card slots and SPDI shift register (74595).
- Need software (some is supposed to exist; if found we could use a quickloader to get it in)

Status:
- It ran into the weeds at 0x100, so memory is patched there to jump to the sign-on screen.
- When it says to press INTERRUPT, press F1. May need multiple presses to get over random errors.
- Various commands starting with k are supposed to be valid, but nothing is acceptable.
- If an error occurs it locks up and you have to press F1 again.
- Even though it is suggested to use a dumb terminal, nothing ever shows on it.

****************************************************************************************************/

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/m6809/m6809.h"
#include "machine/6522via.h"
#include "machine/ins8250.h"
#include "sound/ay8910.h"
#include "video/tms9928a.h"

#include "speaker.h"


namespace {

class ultim809_state : public driver_device
{
public:
	ultim809_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_via(*this, "via")
		, m_crtc(*this, "crtc")
		, m_psg(*this, "psg")
		, m_uart(*this, "uart")
	{}

	void ultim809(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(nmi_button);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void mem_map(address_map &map) ATTR_COLD;
	std::unique_ptr<u8[]> m_ram;
	required_device<cpu_device> m_maincpu;
	required_device<via6522_device> m_via;
	required_device<tms9918a_device> m_crtc;
	required_device<ay8910_device> m_psg;
	required_device<ns16550_device> m_uart;
	u8 m_membank = 0U;
};

void ultim809_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	// main ram banks 0 and 1
	map(0x0000, 0x7fff).lrw8(NAME([this] (offs_t offset) { return m_ram[offset]; }), NAME([this] (offs_t offset, u8 data) { m_ram[offset] = data; } ));
	// main ram any bank
	map(0x8000, 0xbfff).lrw8(NAME([this] (offs_t offset) { return m_ram[offset | (m_membank << 14)]; }),
							 NAME([this] (offs_t offset, u8 data) { m_ram[offset | (m_membank << 14)] = data; } )); // u8
	// devices
	map(0xc000, 0xc00f).m(m_via, FUNC(via6522_device::map)); // u11
	map(0xc400, 0xc407).rw(m_uart, FUNC(ns16550_device::ins8250_r), FUNC(ns16550_device::ins8250_w));  // u16
	map(0xc800, 0xc800); //.r  chip enable 74595
	map(0xcc00, 0xcc00).rw(m_crtc, FUNC(tms9918a_device::vram_read), FUNC(tms9918a_device::vram_write));
	map(0xcc01, 0xcc01).rw(m_crtc, FUNC(tms9918a_device::register_read), FUNC(tms9918a_device::register_write));
	map(0xcc02, 0xcc03).rw(m_psg, FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_data_w));
	map(0xcc04, 0xcc04); //.r  select lower 16k of VRAM
	map(0xcc06, 0xcc06); //.r  clear gamepad pin 7
	map(0xcc0c, 0xcc0c); //.r  select upper 16k of VRAM
	map(0xcc0e, 0xcc0e); //.r  set gamepad pin 7
	map(0xd000, 0xd3ff); // expansion slot 1 (not used)
	map(0xd400, 0xd7ff); // expansion slot 2 (not used)
	map(0xd800, 0xdbff); // expansion slot 3 (not used)
	map(0xdc00, 0xdfff); // expansion slot 4 (not used)
	// rom
	map(0xe000, 0xffff).rom().region("maincpu", 0); // u9
}


static INPUT_PORTS_START( ultim809 )
	PORT_START("RESET")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Interrupt") PORT_CODE(KEYCODE_F1) PORT_CHANGED_MEMBER(DEVICE_SELF, ultim809_state, nmi_button, 0)
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(ultim809_state::nmi_button)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, newval ? ASSERT_LINE : CLEAR_LINE);
}

void ultim809_state::machine_start()
{
	m_ram = make_unique_clear<u8[]>(0x80000);
	save_pointer(NAME(m_ram), 0x80000);
	save_item(NAME(m_membank));
	// Send it to the sign-on instead of into the weeds
	m_ram[0x100] = 0x10;
	m_ram[0x101] = 0x3F;
}

static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_38400 )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_38400 )
DEVICE_INPUT_DEFAULTS_END

void ultim809_state::ultim809(machine_config &config)
{
	// basic machine hardware
	MC6809E(config, m_maincpu, 8000000 / 4);  // 68B09E
	m_maincpu->set_addrmap(AS_PROGRAM, &ultim809_state::mem_map);

	// video hardware
	TMS9918A(config, m_crtc, XTAL(10'738'635));
	m_crtc->set_screen("screen");
	m_crtc->set_vram_size(0x4000);    // actually 2 banks of 0x4000
	m_crtc->int_callback().set_inputline(m_maincpu, M6809_IRQ_LINE);
	SCREEN(config, "screen", SCREEN_TYPE_RASTER);

	// TBD: what type of VIA is this? It replaced a MC68B21P at some point in development.
	MOS6522(config, m_via, 8000000 / 4);
	// Memory banking: up to 32 banks with inbuilt U8, or replace it with external memory to get the full 4 MB
	m_via->writepa_handler().set([this] (u8 data) { m_membank = data & 0x1F; });   // memory banking
	//m_via->readpb_handler().set(FUNC(ultim809_state::portb_r));    // serial
	//m_via->writepb_handler().set(FUNC(ultim809_state::portb_w));   // serial
	m_via->irq_handler().set_inputline(m_maincpu, M6809_FIRQ_LINE);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	AY8910(config, m_psg, 8000000 / 4).add_route(ALL_OUTPUTS, "mono", 0.50);
	//m_psg->port_a_read_callback(FUNC(ultim809_state::...);  // joystick 1
	//m_psg->port_b_read_callback(FUNC(ultim809_state::...);  // joystick 2

	NS16550(config, m_uart, XTAL(1'843'200));
	m_uart->out_tx_callback().set("rs232", FUNC(rs232_port_device::write_txd));

	// there's no rs232 port, it uses FTDI, but we need to see what's going on
	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set(m_uart, FUNC(ins8250_uart_device::rx_w));
	rs232.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal)); // must be exactly here
}

/* ROM definition */
ROM_START( ultim809 )
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD( "ultim809.u9", 0x0000, 0x2000, CRC(b827aaf1) SHA1(64d9e94542d8ff13f64a4d787508eef7b64d4946) )
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT         COMPANY            FULLNAME     FLAGS
COMP( 2010, ultim809, 0,      0,      ultim809, ultim809, ultim809_state, empty_init, "Matthew Sarnoff", "Ultim809", MACHINE_IS_INCOMPLETE | MACHINE_IS_SKELETON | MACHINE_SUPPORTS_SAVE )
