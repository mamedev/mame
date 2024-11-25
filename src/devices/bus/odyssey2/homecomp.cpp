// license:BSD-3-Clause
// copyright-holders:hap
/******************************************************************************

Videopac+ C7420 Home Computer Module emulation

This module only works on the Videopac+ G7400, although the cartridge can still
be inserted into the Odyssey 2/G7000.

Hardware notes:
- Z80 @ 3.547MHz
- 2*8KB ROM, 16KB RAM(2*TMS4416, 2 unpopulated locations)
- optional data recorder

The RAM can be expanded to 32KB by simply adding two more TMS4416. To enable it,
enter command CLEAR 50,-2

TODO:
- lots of unacknowledged writes to latch 1, probably harmless
- cassette data saved from MAME can be loaded fine, but other WAVs can't, even
  when they are good quality, maybe a filter on the data input?

******************************************************************************/

#include "emu.h"
#include "homecomp.h"

#include "cpu/z80/z80.h"
#include "imagedev/cassette.h"
#include "machine/gen_latch.h"

#include "speaker.h"

namespace {

//-------------------------------------------------
//  initialization
//-------------------------------------------------

class o2_homecomp_device : public device_t, public device_o2_cart_interface
{
public:
	o2_homecomp_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual void cart_init() override;

	virtual u8 read_rom04(offs_t offset) override { return m_rom[offset]; }
	virtual u8 read_rom0c(offs_t offset) override { return m_rom[offset + 0x400]; }

	virtual void write_p1(u8 data) override;
	virtual void io_write(offs_t offset, u8 data) override;
	virtual u8 io_read(offs_t offset) override;
	virtual int t0_read() override { return m_latch[0]->pending_r(); }

private:
	required_device<cpu_device> m_maincpu;
	required_device_array<generic_latch_8_device, 2> m_latch;
	required_device<cassette_image_device> m_cass;

	std::unique_ptr<u8[]> m_ram;
	u8 m_control = 0;
	bool m_installed = false;

	void internal_io_w(offs_t offset, u8 data);
	u8 internal_io_r(offs_t offset);
	u8 internal_rom_r(offs_t offset) { return m_exrom[offset]; }

	void homecomp_io(address_map &map) ATTR_COLD;
	void homecomp_mem(address_map &map) ATTR_COLD;
};

o2_homecomp_device::o2_homecomp_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, O2_ROM_HOMECOMP, tag, owner, clock),
	device_o2_cart_interface(mconfig, *this),
	m_maincpu(*this, "maincpu"),
	m_latch(*this, "latch%u", 0),
	m_cass(*this, "cassette")
{ }

void o2_homecomp_device::device_start()
{
	save_item(NAME(m_control));
	save_item(NAME(m_installed));

	// allocate maximum RAM beforehand
	m_ram = std::make_unique<u8[]>(0x8000);
	save_pointer(NAME(m_ram), 0x8000);
}

void o2_homecomp_device::device_reset()
{
	if (!m_installed)
	{
		// install RAM
		u32 ramsize = ioport("RAM")->read() ? 0x8000 : 0x4000;
		m_maincpu->space(AS_PROGRAM).install_ram(0x8000, ramsize - 1 + 0x8000, m_ram.get());

		m_installed = true;
	}
}

void o2_homecomp_device::cart_init()
{
	if (m_rom_size != 0x800 || m_exrom_size != 0x4000)
		fatalerror("o2_homecomp_device: Wrong ROM region size\n");
}


//-------------------------------------------------
//  mapper specific handlers
//-------------------------------------------------

void o2_homecomp_device::write_p1(u8 data)
{
	// P10: Z80 INT pin
	// P10+P11: Z80 RESET pin
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, (data & 1) ? CLEAR_LINE : ASSERT_LINE);
	m_maincpu->set_input_line(INPUT_LINE_RESET, (data & 3) ? CLEAR_LINE : ASSERT_LINE);

	// P11: must be low to access latch 0
	// P14: must be low to access latch 1
	m_control = data;
}

u8 o2_homecomp_device::io_read(offs_t offset)
{
	if ((offset & 0xa0) == 0xa0 && ~m_control & 2)
		return m_latch[0]->read();
	else
		return 0xff;
}

void o2_homecomp_device::io_write(offs_t offset, u8 data)
{
	if (offset & 0x80 && ~m_control & 0x10)
		m_latch[1]->write(data);
}


//-------------------------------------------------
//  internal i/o
//-------------------------------------------------

u8 o2_homecomp_device::internal_io_r(offs_t offset)
{
	u8 data = 0;

	// A7: input latch
	if (~offset & 0x80)
		data |= m_latch[1]->read();

	// A6: other i/o
	if (~offset & 0x40)
	{
		// d0: latch 0 status
		data |= m_latch[0]->pending_r() ^ 1;

		if (m_cass->is_playing() && m_cass->motor_on())
		{
			// d6: cass clock
			// d7: cass data
			double level = m_cass->input();
			if (level > 0.04)
				data |= 0xc0;
			else if (level < -0.04)
				data |= 0x80;
		}
	}

	return data;
}

void o2_homecomp_device::internal_io_w(offs_t offset, u8 data)
{
	// A7: output latch
	if (~offset & 0x80)
		m_latch[0]->write(data);

	// A6: other i/o
	if (~offset & 0x40)
	{
		// d7: cass remote control (also with data)
		m_cass->set_motor((data & 0x81) ? 1 : 0);

		// d0: cass data
		// d1: cass clock
		double level = 0.0;
		if (data & 1)
			level = (data & 2) ? 0.8 : -0.8;
		m_cass->output(level);
	}
}

void o2_homecomp_device::homecomp_mem(address_map &map)
{
	map(0x0000, 0x3fff).r(FUNC(o2_homecomp_device::internal_rom_r));
}

void o2_homecomp_device::homecomp_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0xff).rw(FUNC(o2_homecomp_device::internal_io_r), FUNC(o2_homecomp_device::internal_io_w));
}


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( homecomp )
	PORT_START("RAM")
	PORT_CONFNAME( 0x01, 0x00, "RAM Size" )
	PORT_CONFSETTING( 0x00, "16KB" )
	PORT_CONFSETTING( 0x01, "32KB" ) // unofficial
INPUT_PORTS_END

ioport_constructor o2_homecomp_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(homecomp);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void o2_homecomp_device::device_add_mconfig(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 3.547_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &o2_homecomp_device::homecomp_mem);
	m_maincpu->set_addrmap(AS_IO, &o2_homecomp_device::homecomp_io);

	GENERIC_LATCH_8(config, m_latch[0]);
	GENERIC_LATCH_8(config, m_latch[1]);

	// cassette
	CASSETTE(config, m_cass);
	m_cass->set_default_state(CASSETTE_STOPPED | CASSETTE_SPEAKER_MUTED | CASSETTE_MOTOR_DISABLED);
	SPEAKER(config, "cass_output").front_center(); // on data recorder
	m_cass->add_route(ALL_OUTPUTS, "cass_output", 0.05);
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(O2_ROM_HOMECOMP, device_o2_cart_interface, o2_homecomp_device, "o2_homecomp", "Videopac+ C7420 Cartridge")
