// license:BSD-3-Clause
// copyright-holders:Curt Coder

#include "emu.h"
#include "printer.h"
#include "teletex800.h"
#include "cpu/z80/z80.h"
#include "machine/6821pia.h"
#include "machine/6850acia.h"
#include "machine/z80ctc.h"
#include "machine/z80daisy.h"
#include "machine/z80sio.h"
#include "teletex800.lh"

namespace {

class teletex_800_device : public device_t, public device_rs232_port_interface
{
public:
	teletex_800_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock) :
		device_t(mconfig, TELETEX_800, tag, owner, clock),
		device_rs232_port_interface(mconfig, *this),
		m_maincpu(*this, "z80"),
		m_ctc(*this, "ctc"),
		m_sio(*this, "sio"),
		m_acia(*this, "acia"),
		m_pia(*this, "pia"),
		m_pia_cp(*this, "pia_cp"),
		m_bat_led(*this, "bat_led"),
		m_pr_led(*this, "pr_led"),
		m_telex_led(*this, "telex_led"),
		m_mem_led(*this, "mem_led"),
		m_obs_led(*this, "obs_led"),
		m_write_led(*this, "write_led"),
		m_log_led(*this, "log_led"),
		m_queue_led(*this, "queue_led"),
		m_all_led(*this, "all_led"),
		m_time_led(*this, "time_led"),
		m_date_led(*this, "date_led"),
		m_year_led(*this, "year_led"),
		m_rx_digits(*this, "rx_digit%u", 0U),
		m_tx_digits(*this, "tx_digit%u", 0U)
	{
	}

protected:
	virtual const tiny_rom_entry *device_rom_region() const override
	{
		return ROM_NAME( teletex800 );
	}

	virtual void device_add_mconfig(machine_config &config) override
	{
		config.set_default_layout(layout_teletex800);

		// main board
		Z80(config, m_maincpu, XTAL(4'915'200));
		m_maincpu->set_daisy_config(z80_daisy_chain);
		m_maincpu->set_addrmap(AS_PROGRAM, &teletex_800_device::program_map);
		m_maincpu->set_addrmap(AS_IO, &teletex_800_device::io_map);

		Z80CTC(config, m_ctc, XTAL(4'915'200));
		Z80SIO(config, m_sio, XTAL(4'915'200));
		ACIA6850(config, m_acia);
		PIA6821(config, m_pia);

		RS232_PORT(config, "pr", printer_devices, "printer");

		// control panel
		PIA6821(config, m_pia_cp);
	}

	virtual ioport_constructor device_input_ports() const override
	{
		return INPUT_PORTS_NAME( teletex800 );
	}

	virtual void device_start() override
	{
		m_bat_led.resolve();
		m_pr_led.resolve();
		m_telex_led.resolve();
		m_mem_led.resolve();
		m_obs_led.resolve();
		m_write_led.resolve();
		m_log_led.resolve();
		m_queue_led.resolve();
		m_all_led.resolve();
		m_time_led.resolve();
		m_date_led.resolve();
		m_year_led.resolve();
		m_rx_digits.resolve();
		m_tx_digits.resolve();
	}

	virtual void device_reset() override
	{
	}

private:
	required_device<z80_device> m_maincpu;
	required_device<z80ctc_device> m_ctc;
	required_device<z80sio_device> m_sio;
	required_device<acia6850_device> m_acia;
	required_device<pia6821_device> m_pia;
	required_device<pia6821_device> m_pia_cp;

	output_finder<> m_bat_led;
	output_finder<> m_pr_led;
	output_finder<> m_telex_led;
	output_finder<> m_mem_led;
	output_finder<> m_obs_led;
	output_finder<> m_write_led;
	output_finder<> m_log_led;
	output_finder<> m_queue_led;
	output_finder<> m_all_led;
	output_finder<> m_time_led;
	output_finder<> m_date_led;
	output_finder<> m_year_led;
	output_finder<2> m_rx_digits;
	output_finder<2> m_tx_digits;

	void program_map(address_map &map)
	{
		map(0x0000, 0x0fff).rom().region("z80", 0);
	}

	void io_map(address_map &map)
	{
	}

	constexpr static const z80_daisy_config z80_daisy_chain[] =
	{
		{ nullptr }
	};

	static void printer_devices(device_slot_interface &device)
	{
		device.option_add("printer", SERIAL_PRINTER);
	}

	INPUT_CHANGED_MEMBER( write ) { };
	INPUT_CHANGED_MEMBER( all ) { };
	INPUT_CHANGED_MEMBER( clock ) { };

	static INPUT_PORTS_START( teletex800 )
		PORT_START("BTN")
		PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("SKRIV") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(teletex_800_device::write), 0)
		PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("ALLA") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(teletex_800_device::all), 0)
		PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("KLOCK") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(teletex_800_device::clock), 0)
	INPUT_PORTS_END

	constexpr ROM_START( teletex800 )
		ROM_REGION( 0x1000, "z80", 0 )
		ROM_LOAD( "ix44_ver1.1.u57", 0x0000, 0x1000, CRC(5c11b89c) SHA1(4911332709a8dcda12e72bcdf7a0acd58d65cbfd) )
	ROM_END
};

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(TELETEX_800, device_rs232_port_interface, teletex_800_device, "teletex800", "Luxor Teletex 800")
