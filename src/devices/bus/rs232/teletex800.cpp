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

namespace {

ROM_START( teletex800 )
	ROM_REGION( 0x1000, "z80", 0 )
	ROM_LOAD( "ix44_ver1.1.u57", 0x0000, 0x1000, CRC(5c11b89c) SHA1(4911332709a8dcda12e72bcdf7a0acd58d65cbfd) )
ROM_END

static const z80_daisy_config z80_daisy_chain[] =
{
	{ nullptr }
};

static void printer_devices(device_slot_interface &device)
{
	device.option_add("printer", SERIAL_PRINTER);
}

static INPUT_PORTS_START( teletex800 )
INPUT_PORTS_END

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
		m_pia_cp(*this, "pia_cp")
	{
	}

protected:
	virtual const tiny_rom_entry *device_rom_region() const override
	{
		return ROM_NAME( teletex800 );
	}

	virtual void device_add_mconfig(machine_config &config) override
	{
		// main board
		Z80(config, m_maincpu, XTAL(4'915'200));
		m_maincpu->set_daisy_config(z80_daisy_chain);
		m_maincpu->set_addrmap(AS_PROGRAM, &teletex_800_device::program_map);
		m_maincpu->set_addrmap(AS_IO, &teletex_800_device::io_map);

		Z80CTC(config, m_ctc, XTAL(4'915'200));
		Z80SIO(config, m_sio, XTAL(4'915'200));
		ACIA6850(config, m_acia);
		PIA6821(config, m_pia);

		RS232_PORT(config, "printer", printer_devices, "printer");

		// control panel
		PIA6821(config, m_pia_cp);
	}

	virtual ioport_constructor device_input_ports() const override
	{
		return INPUT_PORTS_NAME( teletex800 );
	}

	virtual void device_start() override
	{
	}

private:
	required_device<z80_device> m_maincpu;
	required_device<z80ctc_device> m_ctc;
	required_device<z80sio_device> m_sio;
	required_device<acia6850_device> m_acia;
	required_device<pia6821_device> m_pia;
	required_device<pia6821_device> m_pia_cp;

	void program_map(address_map &map)
	{
		map(0x0000, 0x0fff).rom().region("z80", 0);
	}

	void io_map(address_map &map)
	{
	}
};

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(TELETEX_800, device_rs232_port_interface, teletex_800_device, "teletex800", "Luxor Teletex 800")
