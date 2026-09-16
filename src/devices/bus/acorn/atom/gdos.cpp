// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    GDOS - F.A.C.C. / Gerrit Hillebrand

    https://site.acornatom.nl/hardware/storage/gdos/

**********************************************************************/

#include "emu.h"
#include "gdos.h"

#include "formats/atom_dsk.h"
#include "imagedev/floppy.h"
#include "machine/6821pia.h"
#include "machine/wd_fdc.h"


namespace {

class atom_gdos_device : public device_t, public device_acorn_bus_interface
{
public:
	atom_gdos_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, ATOM_GDOS, tag, owner, clock)
		, device_acorn_bus_interface(mconfig, *this)
		, m_dos_rom(*this, "dos_rom")
		, m_pia(*this, "pia")
		, m_fdc(*this, "fdc")
		, m_floppies(*this, "fdc:%u", 0)
		, m_floppy(nullptr)
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

private:
	required_memory_region m_dos_rom;
	required_device<pia6821_device> m_pia;
	required_device<fd1793_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppies;
	floppy_image_device *m_floppy;

	uint8_t m_control;

	uint8_t pia_porta_r(offs_t offset);
	void pia_porta_w(offs_t offset, uint8_t data);

	void fdc_intrq_w(int state);
	void motor_w(int state);
};


//-------------------------------------------------
//  FLOPPY_FORMATS( floppy_formats )
//-------------------------------------------------

void atom_gdos_device::floppy_formats(format_registration &fr)
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
//  ROM( gdos )
//-------------------------------------------------

ROM_START( gdos )
	ROM_REGION(0x1000, "dos_rom", 0)
	ROM_SYSTEM_BIOS(0, "gdos166", "GDOS 1.66 910930")
	ROMX_LOAD("gdos166.rom", 0x0000, 0x1000, CRC(935407cb) SHA1(b8ded5f6ca78664b30e608631e95a5d2b0b3c1fc), ROM_BIOS(0))
ROM_END

const tiny_rom_entry *atom_gdos_device::device_rom_region() const
{
	return ROM_NAME( gdos );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void atom_gdos_device::device_add_mconfig(machine_config &config)
{
	PIA6821(config, m_pia);
	m_pia->readpa_handler().set(FUNC(atom_gdos_device::pia_porta_r));
	m_pia->writepa_handler().set(FUNC(atom_gdos_device::pia_porta_w));

	FD1793(config, m_fdc, 8_MHz_XTAL / 8);
	m_fdc->intrq_wr_callback().set(FUNC(atom_gdos_device::fdc_intrq_w));
	m_fdc->hld_wr_callback().set(FUNC(atom_gdos_device::motor_w));
	m_fdc->set_force_ready(true);

	FLOPPY_CONNECTOR(config, m_floppies[0], atom_floppies, "525qd", atom_gdos_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppies[1], atom_floppies, nullptr, atom_gdos_device::floppy_formats).enable_sound(true);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void atom_gdos_device::device_start()
{
	save_item(NAME(m_control));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void atom_gdos_device::device_reset()
{
	address_space &space = m_bus->memspace();

	space.install_readwrite_handler(0xbc10, 0xbc13, emu::rw_delegate(*m_fdc, FUNC(fd1793_device::read)), emu::rw_delegate(*m_fdc, FUNC(fd1793_device::write)));
	space.install_readwrite_handler(0xbc14, 0xbc17, emu::rw_delegate(*m_pia, FUNC(pia6821_device::read)), emu::rw_delegate(*m_pia, FUNC(pia6821_device::write)));
	space.install_rom(0xe000, 0xefff, m_dos_rom->base());
}

//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t atom_gdos_device::pia_porta_r(offs_t offset)
{
	m_control &= 0x3f;

	// bit 6: drive ready
	if (m_floppy)
		m_control |= m_floppy->ready_r() << 6;

	// bit 7: drq
	m_control |= m_fdc->drq_r() << 7;

	return m_control;
}

void atom_gdos_device::pia_porta_w(offs_t offset, uint8_t data)
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

void atom_gdos_device::motor_w(int state)
{
	if (m_floppies[0]->get_device()) m_floppies[0]->get_device()->mon_w(!state);
	if (m_floppies[1]->get_device()) m_floppies[1]->get_device()->mon_w(!state);
}

void atom_gdos_device::fdc_intrq_w(int state)
{
	m_bus->nmi_w(state);
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(ATOM_GDOS, device_acorn_bus_interface, atom_gdos_device, "atom_gdos", "Atom GDOS")
