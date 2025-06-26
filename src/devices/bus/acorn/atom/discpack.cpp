// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn Atom Disc Pack

**********************************************************************/

#include "emu.h"
#include "discpack.h"

#include "imagedev/floppy.h"
#include "machine/i8271.h"

#include "formats/atom_dsk.h"
#include "formats/acorn_dsk.h"


namespace {

class atom_discpack_device : public device_t, public device_acorn_bus_interface
{
public:
	atom_discpack_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, ATOM_DISCPACK, tag, owner, clock)
		, device_acorn_bus_interface(mconfig, *this)
		, m_dos_rom(*this, "dos_rom")
		, m_ram(*this, "ram", 0xc00, ENDIANNESS_LITTLE)
		, m_fdc(*this, "i8271")
		, m_floppy(*this, "i8271:%u", 0)
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
	void fdc_intrq_w(int state);
	void motor_w(int state);
	void side_w(int state);

	required_memory_region m_dos_rom;
	memory_share_creator<uint8_t> m_ram;
	required_device<i8271_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;
};

//-------------------------------------------------
//  FLOPPY_FORMATS( floppy_formats )
//-------------------------------------------------

void atom_discpack_device::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_ATOM_FORMAT);
}

static void atom_floppies(device_slot_interface &device)
{
	device.option_add("525sssd", FLOPPY_525_SSSD);
	device.option_add("525sd", FLOPPY_525_SD);
	device.option_add("525qd", FLOPPY_525_QD);
}

//-------------------------------------------------
//  ROM( discpack )
//-------------------------------------------------

ROM_START( discpack )
	ROM_REGION(0x1000, "dos_rom", 0)
	ROM_LOAD("dosrom.ic15", 0x0000, 0x1000, CRC(c431a9b7) SHA1(71ea0a4b8d9c3caf9718fc7cc279f4306a23b39c))
ROM_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void atom_discpack_device::device_add_mconfig(machine_config &config)
{
	I8271(config, m_fdc, 4_MHz_XTAL);
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


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void atom_discpack_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void atom_discpack_device::device_reset()
{
	address_space &space = m_bus->memspace();
	uint16_t m_blk0 = m_bus->blk0() << 12;

	space.install_device(m_blk0 | 0x0a00, m_blk0 | 0x0a03, *m_fdc, &i8271_device::map);
	space.install_readwrite_handler(m_blk0 | 0x0a04, m_blk0 | 0x0a04, 0, 0x078, 0, emu::rw_delegate(*m_fdc, FUNC(i8271_device::data_r)), emu::rw_delegate(*m_fdc, FUNC(i8271_device::data_w)));
	space.install_ram(0x2000, 0x27ff, m_ram);
	space.install_ram(0x3c00, 0x3fff, m_ram + 0x800);
	space.install_rom(0xe000, 0xefff, m_dos_rom->base());
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

void atom_discpack_device::motor_w(int state)
{
	if (m_floppy[0]->get_device()) m_floppy[0]->get_device()->mon_w(!state);
	if (m_floppy[1]->get_device()) m_floppy[1]->get_device()->mon_w(!state);
	m_fdc->ready_w(!state);
}

void atom_discpack_device::side_w(int state)
{
	if (m_floppy[0]->get_device()) m_floppy[0]->get_device()->ss_w(state);
	if (m_floppy[1]->get_device()) m_floppy[1]->get_device()->ss_w(state);
}

void atom_discpack_device::fdc_intrq_w(int state)
{
	m_bus->nmi_w(state);
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(ATOM_DISCPACK, device_acorn_bus_interface, atom_discpack_device, "atom_discpack", "Atom Disc Pack")
