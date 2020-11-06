// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn Atom Disc Pack

**********************************************************************/


#include "emu.h"
#include "discpack.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ATOM_DISCPACK, atom_discpack_device, "atom_discpack", "Acorn Atom Disc Pack")

//-------------------------------------------------
//  MACHINE_DRIVER( discpack )
//-------------------------------------------------

FLOPPY_FORMATS_MEMBER(atom_discpack_device::floppy_formats )
	FLOPPY_ACORN_SSD_FORMAT
FLOPPY_FORMATS_END

static void atom_floppies(device_slot_interface &device)
{
	device.option_add("525sssd", FLOPPY_525_SSSD);
	device.option_add("525sd", FLOPPY_525_SD);
	device.option_add("525qd", FLOPPY_525_QD);
}

ROM_START( discpack )
	ROM_REGION(0x1000, "dos_rom", 0)
	ROM_LOAD("dosrom.ic15", 0x0000, 0x1000, CRC(c431a9b7) SHA1(71ea0a4b8d9c3caf9718fc7cc279f4306a23b39c))
ROM_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void atom_discpack_device::device_add_mconfig(machine_config &config)
{
	I8271(config, m_fdc, 4_MHz_XTAL / 2);
	m_fdc->intrq_wr_callback().set(FUNC(atom_discpack_device::fdc_intrq_w));
	m_fdc->hdl_wr_callback().set(FUNC(atom_discpack_device::motor_w));
	m_fdc->opt_wr_callback().set(FUNC(atom_discpack_device::side_w));
	FLOPPY_CONNECTOR(config, m_floppy[0], atom_floppies, "525sssd", atom_discpack_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[1], atom_floppies, nullptr, atom_discpack_device::floppy_formats).enable_sound(true);
}


const tiny_rom_entry *atom_discpack_device::device_rom_region() const
{
	return ROM_NAME( discpack );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  atom_discpack_device - constructor
//-------------------------------------------------

atom_discpack_device::atom_discpack_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ATOM_DISCPACK, tag, owner, clock)
	, device_acorn_bus_interface(mconfig, *this)
	, m_dos_rom(*this, "dos_rom")
	, m_fdc(*this, "i8271")
	, m_floppy(*this, "i8271:%u", 0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void atom_discpack_device::device_start()
{
	address_space &space = m_bus->memspace();

	space.install_device(0x0a00, 0x0a03, *m_fdc, &i8271_device::map);
	space.install_readwrite_handler(0x0a04, 0x0a04, 0, 0x1f8, 0, read8smo_delegate(*m_fdc, FUNC(i8271_device::data_r)), write8smo_delegate(*m_fdc, FUNC(i8271_device::data_w)));
	space.install_ram(0x2000, 0x27ff, m_ram);
	space.install_ram(0x3c00, 0x3fff, m_ram+0x800);
	space.install_rom(0xe000, 0xefff, m_dos_rom->base());

	save_item(NAME(m_ram));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

WRITE_LINE_MEMBER(atom_discpack_device::motor_w)
{
	if (m_floppy[0]->get_device()) m_floppy[0]->get_device()->mon_w(!state);
	if (m_floppy[1]->get_device()) m_floppy[1]->get_device()->mon_w(!state);
	m_fdc->ready_w(!state);
}

WRITE_LINE_MEMBER(atom_discpack_device::side_w)
{
	if (m_floppy[0]->get_device()) m_floppy[0]->get_device()->ss_w(state);
	if (m_floppy[1]->get_device()) m_floppy[1]->get_device()->ss_w(state);
}

WRITE_LINE_MEMBER(atom_discpack_device::fdc_intrq_w)
{
	m_bus->nmi_w(state);
}
