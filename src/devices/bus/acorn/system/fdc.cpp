// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn Floppy Disc Controller Board

    Part No. 200,004

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Acorn_fdc.html

**********************************************************************/

#include "emu.h"
#include "fdc.h"

#include "formats/acorn_dsk.h"
#include "imagedev/floppy.h"
#include "machine/i8271.h"


namespace {

class acorn_fdc_device : public device_t, public device_acorn_bus_interface
{
public:
	acorn_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, ACORN_FDC, tag, owner, clock)
		, device_acorn_bus_interface(mconfig, *this)
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

private:
	required_device<i8271_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;

	void bus_nmi_w(int state);
	void motor_w(int state);
	void side_w(int state);
};

//-------------------------------------------------
//  FLOPPY_FORMATS( floppy_formats )
//-------------------------------------------------

void acorn_fdc_device::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_ACORN_SSD_FORMAT);
}

static void acorn_floppies(device_slot_interface &device)
{
	device.option_add("525sssd", FLOPPY_525_SSSD);
	device.option_add("525sd", FLOPPY_525_SD);
	device.option_add("525qd", FLOPPY_525_QD);
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void acorn_fdc_device::device_add_mconfig(machine_config &config)
{
	I8271(config, m_fdc, 4_MHz_XTAL);
	m_fdc->intrq_wr_callback().set(FUNC(acorn_fdc_device::bus_nmi_w));
	m_fdc->hdl_wr_callback().set(FUNC(acorn_fdc_device::motor_w));
	m_fdc->opt_wr_callback().set(FUNC(acorn_fdc_device::side_w));

	FLOPPY_CONNECTOR(config, m_floppy[0], acorn_floppies, "525qd", acorn_fdc_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[1], acorn_floppies, "525qd", acorn_fdc_device::floppy_formats).enable_sound(true);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void acorn_fdc_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void acorn_fdc_device::device_reset()
{
	address_space &space = m_bus->memspace();
	uint16_t m_blk0 = m_bus->blk0() << 12;

	space.install_device(m_blk0 | 0x0a00, m_blk0 | 0x0a03, *m_fdc, &i8271_device::map);
	space.install_readwrite_handler(m_blk0 | 0x0a04, m_blk0 | 0x0a04, 0, 0x1f8, 0, emu::rw_delegate(*m_fdc, FUNC(i8271_device::data_r)), emu::rw_delegate(*m_fdc, FUNC(i8271_device::data_w)));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

void acorn_fdc_device::motor_w(int state)
{
	if (m_floppy[0]->get_device()) m_floppy[0]->get_device()->mon_w(!state);
	if (m_floppy[1]->get_device()) m_floppy[1]->get_device()->mon_w(!state);
	m_fdc->ready_w(!state);
}

void acorn_fdc_device::side_w(int state)
{
	if (m_floppy[0]->get_device()) m_floppy[0]->get_device()->ss_w(state);
	if (m_floppy[1]->get_device()) m_floppy[1]->get_device()->ss_w(state);
}

void acorn_fdc_device::bus_nmi_w(int state)
{
	m_bus->nmi_w(state);
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(ACORN_FDC, device_acorn_bus_interface, acorn_fdc_device, "acorn_fdc", "Acorn Floppy Disc Controller Board")
