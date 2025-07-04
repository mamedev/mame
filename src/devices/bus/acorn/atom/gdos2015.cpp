// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Atom GDOS 2015 Mk 1.00 2015-06-16

    Original designs 1987 Dutch Atom User Group

**********************************************************************/

#include "emu.h"
#include "gdos2015.h"

#include "formats/atom_dsk.h"
#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"


namespace {

class atom_gdos2015_device : public device_t, public device_acorn_bus_interface
{
public:
	atom_gdos2015_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, ATOM_GDOS2015, tag, owner, clock)
		, device_acorn_bus_interface(mconfig, *this)
		, m_dos_rom(*this, "dos_rom")
		, m_ram(*this, "ram", 0x8000, ENDIANNESS_LITTLE)
		, m_fdc(*this, "fdc")
		, m_floppies(*this, "fdc:%u", 0)
		, m_floppy(nullptr)
		, m_links(*this, "LINKS")
		, m_control(0)
	{
	}

	static void floppy_formats(format_registration &fr);

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	required_memory_region m_dos_rom;
	memory_share_creator<uint8_t> m_ram;
	required_device<wd1772_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppies;
	floppy_image_device *m_floppy;
	required_ioport m_links;

	uint8_t m_control;

	uint8_t control_r(offs_t offset);
	void control_w(offs_t offset, uint8_t data);

	void fdc_intrq_w(int state);
};


//-------------------------------------------------
//  INPUT_PORTS( gdos2015 )
//-------------------------------------------------

INPUT_PORTS_START(gdos2015)
	PORT_START("LINKS")
	PORT_CONFNAME(0x01, 0x01, "JP1: ROM enable/disable")
	PORT_CONFSETTING(0x00, "ROM disabled")
	PORT_CONFSETTING(0x01, "ROM enabled")

	PORT_CONFNAME(0x02, 0x00, "JP2: I/O address select")
	PORT_CONFSETTING(0x00, "$EFF0-$EFFF")
	PORT_CONFSETTING(0x02, "$BC10-$BC1F")

	PORT_CONFNAME(0x04, 0x04, "JP3: RAM3C RAM bank enable")
	PORT_CONFSETTING(0x00, "Disable RAM $3C00-$7FFF")
	PORT_CONFSETTING(0x04, "Enable RAM $3C00-$7FFF")

	PORT_CONFNAME(0x08, 0x00, "JP3: RAM29 RAM bank enable")
	PORT_CONFSETTING(0x00, "Disable RAM $2900-$3BFF")
	PORT_CONFSETTING(0x08, "Enable RAM $2900-$3BFF")

	PORT_CONFNAME(0x10, 0x10, "JP3: RAM04 RAM bank enable")
	PORT_CONFSETTING(0x00, "Disable RAM $0400-$27FF")
	PORT_CONFSETTING(0x10, "Enable RAM $0400-$27FF")

	PORT_CONFNAME(0xe0, 0x40, "Onboard ROM bank")
	PORT_CONFSETTING(0x00, "Acorn DOS I/O at $BC10")
	PORT_CONFSETTING(0x20, "GDOS I/O at $BC10")
	PORT_CONFSETTING(0x40, "Acorn DOS I/O at $EFF0")
	PORT_CONFSETTING(0x60, "GDOS I/O at $EFF0")
	PORT_CONFSETTING(0x80, "Hardware/RAM test (LINK#E000/LINK#E003)")
INPUT_PORTS_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor atom_gdos2015_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(gdos2015);
}


//-------------------------------------------------
//  FLOPPY_FORMATS( floppy_formats )
//-------------------------------------------------

void atom_gdos2015_device::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_ATOM_FORMAT);
}

static void atom_floppies(device_slot_interface &device)
{
	device.option_add("525sd", FLOPPY_525_SD);
	device.option_add("525qd", FLOPPY_525_QD);
}

//-------------------------------------------------
//  ROM( gdos2015 )
//-------------------------------------------------

ROM_START( gdos2015 )
	ROM_REGION(0x10000, "dos_rom", 0)
	ROM_LOAD("gdos2015_15.ic8", 0x0000, 0x10000, CRC(fc790657) SHA1(ff776eff6aadaf1a7f6d856cd6d1f1f59ac97112))
ROM_END

const tiny_rom_entry *atom_gdos2015_device::device_rom_region() const
{
	return ROM_NAME( gdos2015 );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void atom_gdos2015_device::device_add_mconfig(machine_config &config)
{
	WD1772(config, m_fdc, 8_MHz_XTAL);
	m_fdc->intrq_wr_callback().set(FUNC(atom_gdos2015_device::fdc_intrq_w));

	FLOPPY_CONNECTOR(config, m_floppies[0], atom_floppies, "525qd", atom_gdos2015_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppies[1], atom_floppies, nullptr, atom_gdos2015_device::floppy_formats).enable_sound(true);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void atom_gdos2015_device::device_start()
{
	save_item(NAME(m_control));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void atom_gdos2015_device::device_reset()
{
	address_space &space = m_bus->memspace();

	// install ROM
	if (m_links->read() & 0x01)
	{
		offs_t offset = (m_links->read() & 0xe0) << 7;
		space.install_rom(0xe000, 0xefff, m_dos_rom->base() + offset);
	}

	// install IO
	if (m_links->read() & 0x02)
	{
		space.install_readwrite_handler(0xbc10, 0xbc13, emu::rw_delegate(*m_fdc, FUNC(wd1772_device::read)), emu::rw_delegate(*m_fdc, FUNC(wd1772_device::write)));
		space.install_readwrite_handler(0xbc14, 0xbc14, emu::rw_delegate(*this, FUNC(atom_gdos2015_device::control_r)), emu::rw_delegate(*this, FUNC(atom_gdos2015_device::control_w)));
	}
	else
	{
		space.install_readwrite_handler(0xeff0, 0xeff3, emu::rw_delegate(*m_fdc, FUNC(wd1772_device::read)), emu::rw_delegate(*m_fdc, FUNC(wd1772_device::write)));
		space.install_readwrite_handler(0xeff4, 0xeff4, emu::rw_delegate(*this, FUNC(atom_gdos2015_device::control_r)), emu::rw_delegate(*this, FUNC(atom_gdos2015_device::control_w)));
	}

	// install RAM
	if (m_links->read() & 0x04)
	{
		space.install_ram(0x3c00, 0x7fff, m_ram + 0x3c00);
	}

	if (m_links->read() & 0x08)
	{
		space.install_ram(0x2900, 0x3bff, m_ram + 0x2900);
	}

	if (m_links->read() & 0x10)
	{
		space.install_ram(0x0400, 0x27ff, m_ram + 0x0400);
	}
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t atom_gdos2015_device::control_r(offs_t offset)
{
	m_control &= 0x3f;

	// bit 6: index pulse
	if (m_floppy)
		m_control |= m_floppy->idx_r() << 6;

	// bit 7: drq
	m_control |= m_fdc->drq_r() << 7;

	return m_control;
}

void atom_gdos2015_device::control_w(offs_t offset, uint8_t data)
{
	m_control = data;

	// bit 1, 2: drive select
	if (BIT(data, 1)) m_floppy = m_floppies[0]->get_device();
	if (BIT(data, 2)) m_floppy = m_floppies[1]->get_device();
	m_fdc->set_floppy(m_floppy);

	// bit 0: side select
	if (m_floppy)
		m_floppy->ss_w(BIT(data, 0));

	// bit 3: density
	m_fdc->dden_w(BIT(data, 3));

	// bit 4: reset
	m_fdc->mr_w(BIT(data, 4));

	// bit 5: ¬40/80 track
}

void atom_gdos2015_device::fdc_intrq_w(int state)
{
	m_bus->nmi_w(state);
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(ATOM_GDOS2015, device_acorn_bus_interface, atom_gdos2015_device, "atom_gdos2015", "Atom GDOS-2015")
