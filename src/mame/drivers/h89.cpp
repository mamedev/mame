// license:BSD-3-Clause
// copyright-holders:Sandro Ronco, Mark Garlanger
/***************************************************************************

        Heathkit H89

        12/05/2009 Skeleton driver.

    Monitor Commands:
    B Boot
    C Convert (number)
    G Go (address)
    I In (address)
    O Out (address,data)
    R Radix (H/O)
    S Substitute (address)
    T Test Memory
    V View

****************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "machine/ins8250.h"
#include "machine/timer.h"

#define RS232_TAG "rs232"

class h89_state : public driver_device
{
public:
	h89_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{
	}

	void h89(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;

	DECLARE_WRITE8_MEMBER( port_f2_w );

	uint8_t m_port_f2;
	virtual void machine_reset() override;
	TIMER_DEVICE_CALLBACK_MEMBER(h89_irq_timer);
	void h89_io(address_map &map);
	void h89_mem(address_map &map);
};


void h89_state::h89_mem(address_map &map)
{
	map.unmap_value_high();
	// Bank 0 - At startup has the format defined below, but software could swap it for RAM (Later H-89s and
	//          Early ones with the Org-0 modification),
	//          TODO - define the RAM so it can swap in/out under program control.
	map(0x0000, 0x0fff).rom();   // Page 0-4 - System ROM (at most 4k(MTR-90), early versions(MTR-88, MTR-89) only had 2k)
	map(0x1000, 0x13ff).ram();   // Page 5 - Floppy Disk RAM (Write-protectable)
	map(0x1400, 0x1fff).rom();   // Page 6-7 - Floppy Disk ROM

	// Banks 1-7
	map(0x2000, 0xffff).ram();
}

void h89_state::h89_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
//  AM_RANGE(0x78, 0x7b)    expansion 1    // Options - Cassette I/O (only uses 0x78 - 0x79) Requires MTR-88 ROM
										   //         - H37 5-1/4" Soft-sectored Controller MTR-90 ROM
										   //         - H47 Dual 8" Drives - Requires MTR-89 or MTR-90 ROM
										   //         - H67 8" Hard disk + 8" Floppy Drives - MTR-90 ROM
//  AM_RANGE(0x7c, 0x7f)    expansion 2    // Options - 5-1/4" Hard-sectored Controller (works with ALL ROMs)
										   //         - H47 Dual 8" Drives - Requires MTR-89 or MTR-90 ROM
										   //         - H67 8" Hard disk + 8" Floppy Drives - MTR-90 ROM

//  AM_RANGE(0xd0, 0xd7)    8250 UART DCE
//  AM_RANGE(0xd8, 0xdf)    8250 UART DTE - MODEM
//  AM_RANGE(0xe0, 0xe7)    8250 UART DCE - LP
	map(0xe8, 0xef).rw("ins8250", FUNC(ins8250_device::ins8250_r), FUNC(ins8250_device::ins8250_w)); // 8250 UART console - this
																								 // connects internally to a Terminal board
																								 // that is also used in the H19. Ideally,
																								 // the H19 code could be connected and ran
																								 // as a separate thread.
//  AM_RANGE(0xf0, 0xf1)        // ports defined on the H8 - on the H89, access to these addresses causes a NMI
	map(0xf2, 0xf2).w(FUNC(h89_state::port_f2_w)).portr("SW501");
//  AM_RANGE(0xf3, 0xf3)        // ports defined on the H8 - on the H89, access to these addresses causes a NMI
}

/* Input ports */
static INPUT_PORTS_START( h89 )

		// Settings with the MTR-88 ROM (#444-40)
//  PORT_START("SW501")
//  PORT_DIPNAME( 0x1f, 0x00, "Unused" )  PORT_DIPLOCATION("S1:1,S1:2,S1:3,S1:4,S1:5")
//  PORT_DIPNAME( 0x20, 0x20, "Perform memory test at start" )  PORT_DIPLOCATION("S1:6")
//  PORT_DIPSETTING( 0x20, DEF_STR( Off ) )
//  PORT_DIPSETTING( 0x00, DEF_STR( On ) )
//  PORT_DIPNAME( 0xc0, 0x00, "Console Baud rate" )  PORT_DIPLOCATION("S1:7")
//  PORT_DIPSETTING( 0x00, "9600" )
//  PORT_DIPSETTING( 0x40, "19200" )
//  PORT_DIPSETTING( 0x80, "38400" )
//  PORT_DIPSETTING( 0xc0, "57600" )

		// Settings with the MTR-89 ROM (#444-62)
//  PORT_START("SW501")
//  PORT_DIPNAME( 0x03, 0x00, "Expansion 1" )  PORT_DIPLOCATION("S1:1,S1:2")
//  PORT_DIPSETTING( 0x00, "H-88-1" )
//  PORT_DIPSETTING( 0x01, "H/Z-47" )
//  PORT_DIPSETTING( 0x02, "Undefined" )
//  PORT_DIPSETTING( 0x03, "Undefined" )
//  PORT_DIPNAME( 0x0c, 0x00, "Expansion 2" )  PORT_DIPLOCATION("S1:3,S1:4")
//  PORT_DIPSETTING( 0x00, "Unused" )
//  PORT_DIPSETTING( 0x04, "H/Z-47" )
//  PORT_DIPSETTING( 0x08, "Undefined" )
//  PORT_DIPSETTING( 0x0c, "Undefined" )
//  PORT_DIPNAME( 0x10, 0x00, "Boot from" )  PORT_DIPLOCATION("S1:5")
//  PORT_DIPSETTING( 0x00, "Expansion 1" )
//  PORT_DIPSETTING( 0x10, "Expansion 2" )
//  PORT_DIPNAME( 0x20, 0x20, "Perform memory test at start" )  PORT_DIPLOCATION("S1:6")
//  PORT_DIPSETTING( 0x20, DEF_STR( Off ) )
//  PORT_DIPSETTING( 0x00, DEF_STR( On ) )
//  PORT_DIPNAME( 0x40, 0x00, "Console Baud rate" )  PORT_DIPLOCATION("S1:7")
//  PORT_DIPSETTING( 0x00, "9600" )
//  PORT_DIPSETTING( 0x40, "19200" )
//  PORT_DIPNAME( 0x80, 0x00, "Boot mode" )  PORT_DIPLOCATION("S1:8")
//  PORT_DIPSETTING( 0x00, DEF_STR( Normal ) )
//  PORT_DIPSETTING( 0x80, "Auto" )

		// Settings with the MTR-90 ROM (#444-84 or 444-142)
	PORT_START("SW501")
	PORT_DIPNAME( 0x03, 0x00, "Expansion 1" )  PORT_DIPLOCATION("S1:1,S1:2")
	PORT_DIPSETTING( 0x00, "H-88-1" )
	PORT_DIPSETTING( 0x01, "H/Z-47" )
	PORT_DIPSETTING( 0x02, "Z-67" )
	PORT_DIPSETTING( 0x03, "Undefined" )
	PORT_DIPNAME( 0x0c, 0x00, "Expansion 2" )  PORT_DIPLOCATION("S1:3,S1:4")
	PORT_DIPSETTING( 0x00, "H-89-37" )
	PORT_DIPSETTING( 0x04, "H/Z-47" )
	PORT_DIPSETTING( 0x08, "Z-67" )
	PORT_DIPSETTING( 0x0c, "Undefined" )
	PORT_DIPNAME( 0x10, 0x00, "Boot from" )  PORT_DIPLOCATION("S1:5")
	PORT_DIPSETTING( 0x00, "Expansion 1" )
	PORT_DIPSETTING( 0x10, "Expansion 2" )
	PORT_DIPNAME( 0x20, 0x20, "Perform memory test at start" )  PORT_DIPLOCATION("S1:6")
	PORT_DIPSETTING( 0x20, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Console Baud rate" )  PORT_DIPLOCATION("S1:7")
	PORT_DIPSETTING( 0x00, "9600" )
	PORT_DIPSETTING( 0x40, "19200" )
	PORT_DIPNAME( 0x80, 0x00, "Boot mode" )  PORT_DIPLOCATION("S1:8")
	PORT_DIPSETTING( 0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING( 0x80, "Auto" )
INPUT_PORTS_END


void h89_state::machine_reset()
{
}

TIMER_DEVICE_CALLBACK_MEMBER(h89_state::h89_irq_timer)
{
	if (m_port_f2 & 0x02)
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0xcf); // Z80
}

WRITE8_MEMBER( h89_state::port_f2_w )
{
	// Bit 0 - Single-step
	// Bit 1 - Enable timer interrupt (2mSec Clock)
	m_port_f2 = data;
}

static DEVICE_INPUT_DEFAULTS_START( terminal )
	// TODO - baud rate should be controlled by SW501 setting
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_STARTBITS", 0xff, RS232_STARTBITS_1 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END


void h89_state::h89(machine_config & config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(12'288'000) / 6);
	m_maincpu->set_addrmap(AS_PROGRAM, &h89_state::h89_mem);
	m_maincpu->set_addrmap(AS_IO, &h89_state::h89_io);

	ins8250_device &uart(INS8250(config, "ins8250", XTAL(1'843'200)));
	uart.out_tx_callback().set(RS232_TAG, FUNC(rs232_port_device::write_txd));

	rs232_port_device &rs232(RS232_PORT(config, RS232_TAG, default_rs232_devices, "terminal"));
	rs232.rxd_handler().set("ins8250", FUNC(ins8250_uart_device::rx_w));
	rs232.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));

	TIMER(config, "irq_timer", 0).configure_periodic(timer_device::expired_delegate(FUNC(h89_state::h89_irq_timer), this), attotime::from_hz(100));
}

/* ROM definition */
ROM_START( h89 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "2732_444-142_mtr90.rom", 0x0000, 0x1000, CRC(c4ff47c5) SHA1(d6f3d71ff270a663003ec18a3ed1fa49f627123a))
	ROM_LOAD( "2716_444-19_h17.rom", 0x1800, 0x0800, CRC(26e80ae3) SHA1(0c0ee95d7cb1a760f924769e10c0db1678f2435c))

	ROM_REGION( 0x10000, "otherroms", ROMREGION_ERASEFF )
	ROM_LOAD( "2732_444-84_mtr84.rom", 0x0000, 0x1000, CRC(c98e5f4c) SHA1(03347206dca145ff69ca08435db822b70ce106af))
	ROM_LOAD( "2732_mms84a_magnoliamms.bin", 0x0000, 0x1000, CRC(5563f42a) SHA1(1b74cafca8213d5c083f16d8a848933ab56eb43b))
ROM_END

/* Driver */

/*    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS      INIT        COMPANY      FULLNAME        FLAGS */
COMP( 1979, h89,  0,      0,      h89,     h89,   h89_state, empty_init, "Heath Inc", "Heathkit H89", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
