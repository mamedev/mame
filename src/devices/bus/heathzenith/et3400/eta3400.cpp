// license:BSD-3-Clause
// copyright-holders:Robbbert, Mark Garlanger
/***************************************************************************

  Heathkit ETA-3400 Memory I/O Accessory (595-2170 and 595-2271 manuals)

  Provides Tiny Basic, a Terminal monitor, a Serial Interface, a Cassette
  interface, and 1k to 4k of RAM.

  The ROMs are U105 (Monitor), U106 (Tiny Basic), both type NMOS2316E, and
  U108 (address decoder PROM). U108 decodes A10-A13, with A14 and A15 ANDed
  together, so the accessory also appears at 4000 and 8000:
    0000-0FFF  RAM in 1K pairs of 2114s (one pair standard, three optional)
    1000-13FF  PIA (serial and cassette)
    1400-1BFF  Terminal monitor ROM
    1C00-23FF  Tiny Basic ROM
  The accessory's RAM overlays the trainer's own, so both hold the same data.

  An original ET-3400 needs the modification kit (597-1954) to use it: a
  4 MHz crystal so the CPU runs at 1 MHz, and the bus connector. The
  ET-3400A only needs the connector.

  - Navigating:
      LED to Monitor: D1400
      Monitor to Basic: G 1C00
      Monitor to LED: G FC00
      Basic to Monitor: BYE
  - All commands in Basic and Monitor are UPPERCASE only.
  - Terminal is defaulted to 9600 baud, 7 bits, 2 stop bits.

  - Using Tiny Basic: press RESET ('.') on the trainer, then DO (D) 1400.
    The terminal shows "MON>". Type G 1C00 and the Basic prompt ":" appears.
    For example:
      :10 PRINT "HELLO",6*7
      :20 END
      :LIST
      :RUN
      HELLO  42
      :BYE
    Without the END line, RUN finishes with an error ("!75 AT 0").
  - Things to test:
      Card RAM: from the keypad, AUTO 0300 and a byte, RESET, then EXAM 0300
        shows the byte. 0400 needs the 2K or larger RAM setting.
      ROM and its mirror: EXAM 1400 and EXAM 5400 both show 0F.
      Both sides together: enter a program on the keypad (see et3400.cpp),
        then type G 0000 at the terminal's MON> prompt to run it.

****************************************************************************/

#include "emu.h"
#include "eta3400.h"

#include "bus/rs232/rs232.h"
#include "imagedev/cassette.h"
#include "machine/6821pia.h"

#include "speaker.h"

#include "eta3400.lh"


namespace {

class eta3400_device : public device_t, public device_et3400_expansion_card_interface
{
public:
	eta3400_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void reset_w(int state) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	// the accessory decodes A14 and A15 together, so it repeats at 4000 and 8000
	static constexpr offs_t BASES[] = { 0x0000, 0x4000, 0x8000 };
	static constexpr u32 MAX_RAM = 0x1000;

	u8 pia_ar();
	void pia_aw(u8 data);
	u8 pia_br();
	void pia_bw(u8 data);

	required_device<pia6821_device> m_pia;
	required_device<rs232_port_device> m_rs232;
	required_device<cassette_image_device> m_cass;
	required_region_ptr<u8> m_rom;
	required_ioport m_baud;
	required_ioport m_config;

	std::unique_ptr<u8[]> m_ram;
};


eta3400_device::eta3400_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ETA3400, tag, owner, clock)
	, device_et3400_expansion_card_interface(mconfig, *this)
	, m_pia(*this, "pia")
	, m_rs232(*this, "rs232")
	, m_cass(*this, "cassette")
	, m_rom(*this, "roms")
	, m_baud(*this, "BAUD")
	, m_config(*this, "CONFIG")
{
}

void eta3400_device::device_start()
{
	m_ram = make_unique_clear<u8[]>(MAX_RAM);
	save_pointer(NAME(m_ram), MAX_RAM);

	for (offs_t base : BASES)
	{
		space().install_readwrite_handler(base + 0x1000, base + 0x1003, 0, 0x03fc, 0,
				emu::rw_delegate(*m_pia, FUNC(pia6821_device::read)), emu::rw_delegate(*m_pia, FUNC(pia6821_device::write)));
		space().install_rom(base + 0x1400, base + 0x23ff, &m_rom[0]);
	}
}

void eta3400_device::device_reset()
{
	// 1K standard, more with ETA-3400-1 chip sets; the size is read at reset so a change applies then
	u32 const size = 0x400 * ((m_config->read() & 0x03) + 1);

	for (offs_t base : BASES)
	{
		space().unmap_readwrite(base, base + MAX_RAM - 1);
		space().install_ram(base, base + size - 1, &m_ram[0]);
	}
}

void eta3400_device::reset_w(int state)
{
	// the PIA also uses the reset line; the device has no reset input, so reset it directly
	if (!state)
		m_pia->reset();
}

// d1,2,3 = Baud rate
// d4 = gnd
// d7 = rs232 in
u8 eta3400_device::pia_ar()
{
	return m_baud->read() | (m_rs232->rxd_r() << 7);
}

// d0 = rs232 out
void eta3400_device::pia_aw(u8 data)
{
	m_rs232->write_txd(BIT(data, 0));
}

// d7 = cass in
u8 eta3400_device::pia_br()
{
	return (m_cass->input() > +0.04) << 7;
}

// d0 = cass out
void eta3400_device::pia_bw(u8 data)
{
	m_cass->output(BIT(data, 0) ? -1.0 : +1.0);
}


static INPUT_PORTS_START( eta3400 )
	PORT_START("BAUD")
	PORT_DIPNAME( 0x0E, 0x02, "Baud Rate" )
	PORT_DIPSETTING(    0x0E, "110" )
	PORT_DIPSETTING(    0x00, "300" )
	PORT_DIPSETTING(    0x0A, "600" )
	PORT_DIPSETTING(    0x08, "1200" )
	PORT_DIPSETTING(    0x06, "2400" )
	PORT_DIPSETTING(    0x04, "4800" )
	PORT_DIPSETTING(    0x02, "9600" )

	PORT_START("CONFIG")
	PORT_CONFNAME( 0x03, 0x00, "RAM (applies on reset)" )
	PORT_CONFSETTING(    0x00, "1K" )
	PORT_CONFSETTING(    0x01, "2K" )
	PORT_CONFSETTING(    0x02, "3K" )
	PORT_CONFSETTING(    0x03, "4K" )
INPUT_PORTS_END

ioport_constructor eta3400_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( eta3400 );
}


static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_7 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_2 )
DEVICE_INPUT_DEFAULTS_END

void eta3400_device::device_add_mconfig(machine_config &config)
{
	// adds a view with the trainer's display and keypad beside the terminal
	config.set_default_layout(layout_eta3400);

	PIA6821(config, m_pia);
	m_pia->writepa_handler().set(FUNC(eta3400_device::pia_aw));
	m_pia->writepb_handler().set(FUNC(eta3400_device::pia_bw));
	m_pia->readpa_handler().set(FUNC(eta3400_device::pia_ar));
	m_pia->readpb_handler().set(FUNC(eta3400_device::pia_br));

	RS232_PORT(config, m_rs232, default_rs232_devices, "terminal").set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));

	SPEAKER(config, "mono").front_center();

	CASSETTE(config, m_cass);
	m_cass->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);
}


ROM_START( eta3400 )
	ROM_REGION( 0x1000, "roms", 0 )
	ROM_LOAD( "monitor.u105", 0x0000, 0x0800, CRC(e4142682) SHA1(785966018dd6eb097ed9bd5c7def2354ab4347db) )
	ROM_LOAD( "basic.u106",   0x0800, 0x0800, CRC(bbd6a801) SHA1(088da24bd4d923d4f196b993154c538835d10605) )

	// address decoder, see the memory map in the notes at the top
	ROM_REGION( 0x0020, "decoder", 0 )
	ROM_LOAD( "prom.u108",    0x0000, 0x0020, CRC(273025c3) SHA1(136c1cdce2a4a796c1c46e8ea4f798cdee4b549b) )
ROM_END

const tiny_rom_entry *eta3400_device::device_rom_region() const
{
	return ROM_NAME( eta3400 );
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(ETA3400, device_et3400_expansion_card_interface, eta3400_device, "eta3400", "Heathkit ETA-3400 Memory I/O Accessory")
