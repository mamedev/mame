// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Nascom NASBUS Floppy Disc Controller

***************************************************************************/

#include "emu.h"
#include "floppy.h"
#include "formats/nascom_dsk.h"

//**************************************************************************
//  CONSTANTS/MACROS
//**************************************************************************

#define VERBOSE 1
#define VERBOSE_STATUS 0


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(NASCOM_FDC, nascom_fdc_device, "nascom_fdc", "Nascom Floppy Disc Controller")

FLOPPY_FORMATS_MEMBER( nascom_fdc_device::floppy_formats )
	FLOPPY_NASCOM_FORMAT
FLOPPY_FORMATS_END

static void nascom_floppies(device_slot_interface &device)
{
	device.option_add("55e", TEAC_FD_55E);
	device.option_add("55f", TEAC_FD_55F);
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void nascom_fdc_device::device_add_mconfig(machine_config &config)
{
	FD1793(config, m_fdc, 16_MHz_XTAL / 4 / 4);

	FLOPPY_CONNECTOR(config, m_floppy0, nascom_floppies, "55f", nascom_fdc_device::floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy1, nascom_floppies, "55f", nascom_fdc_device::floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy2, nascom_floppies, nullptr, nascom_fdc_device::floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy3, nascom_floppies, nullptr, nascom_fdc_device::floppy_formats);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  wordpro_device - constructor
//-------------------------------------------------

nascom_fdc_device::nascom_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, NASCOM_FDC, tag, owner, clock),
	device_nasbus_card_interface(mconfig, *this),
	m_fdc(*this, "fd1793"),
	m_floppy0(*this, "fd1793:0"),
	m_floppy1(*this, "fd1793:1"),
	m_floppy2(*this, "fd1793:2"),
	m_floppy3(*this, "fd1793:3"),
	m_floppy(nullptr),
	m_motor(nullptr),
	m_select(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nascom_fdc_device::device_start()
{
	// timer to turn off the drive motor line
	m_motor = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(nascom_fdc_device::motor_off), this));

	save_item(NAME(m_select));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void nascom_fdc_device::device_reset()
{
	io_space().install_readwrite_handler(0xe0, 0xe3, read8sm_delegate(*m_fdc, FUNC(fd1793_device::read)), write8sm_delegate(*m_fdc, FUNC(fd1793_device::write)));
	io_space().install_readwrite_handler(0xe4, 0xe4, read8_delegate(*this, FUNC(nascom_fdc_device::select_r)), write8_delegate(*this, FUNC(nascom_fdc_device::select_w)));
	io_space().install_read_handler(0xe5, 0xe5, read8_delegate(*this, FUNC(nascom_fdc_device::status_r)));
}

//-------------------------------------------------
//  device_reset_after_children - device-specific reset after children
//-------------------------------------------------

void nascom_fdc_device::device_reset_after_children()
{
	// sanity check
	if (m_floppy0->get_device() && m_floppy1->get_device())
		if (m_floppy0->get_device()->get_sides() != m_floppy1->get_device()->get_sides())
			fatalerror("Floppy drive 0 and 1 need to be of the same type.\n");

	if (m_floppy2->get_device() && m_floppy3->get_device())
		if (m_floppy2->get_device()->get_sides() != m_floppy3->get_device()->get_sides())
			fatalerror("Floppy drive 2 and 3 need to be of the same type.\n");
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

TIMER_CALLBACK_MEMBER( nascom_fdc_device::motor_off )
{
	if (m_floppy0->get_device())
		m_floppy0->get_device()->mon_w(1);

	if (m_floppy1->get_device())
		m_floppy1->get_device()->mon_w(1);

	if (m_floppy2->get_device())
		m_floppy2->get_device()->mon_w(1);

	if (m_floppy3->get_device())
		m_floppy3->get_device()->mon_w(1);
}

READ8_MEMBER( nascom_fdc_device::select_r )
{
	m_select |= (0x80 | 0x20);

	// double sided drive for drive 0/1?
	if (m_floppy0->get_device() && (m_floppy0->get_device()->get_sides() == 2))
		m_select &= ~0x20;

	if (m_floppy1->get_device() && (m_floppy1->get_device()->get_sides() == 2))
		m_select &= ~0x20;

	// double sided drive for drive 2/3?
	if (m_floppy2->get_device() && (m_floppy2->get_device()->get_sides() == 2))
		m_select &= ~0x80;

	if (m_floppy3->get_device() && (m_floppy3->get_device()->get_sides() == 2))
		m_select &= ~0x80;

	if (VERBOSE)
		logerror("nascom_fdc_device::select_r: 0x%02x\n", m_select);

	return m_select;
}

WRITE8_MEMBER( nascom_fdc_device::select_w )
{
	if (VERBOSE)
		logerror("nascom_fdc_device::select_w: 0x%02x\n", data);

	m_floppy = nullptr;

	if (BIT(data, 0)) m_floppy = m_floppy0->get_device();
	if (BIT(data, 1)) m_floppy = m_floppy1->get_device();
	if (BIT(data, 2)) m_floppy = m_floppy2->get_device();
	if (BIT(data, 3)) m_floppy = m_floppy3->get_device();

	m_fdc->set_floppy(m_floppy);

	if (m_floppy)
	{
		m_floppy->ss_w(BIT(data, 4));
		m_floppy->mon_w(!BIT(data, 5));

		// motor gets turned off again after 10 seconds
		m_motor->adjust(attotime::from_seconds(10), 0);
	}

	m_fdc->dden_w(BIT(data, 6));

	m_select = data;
}

READ8_MEMBER( nascom_fdc_device::status_r )
{
	uint8_t data = 0;

	data |= m_fdc->intrq_r() << 0;

	// if a floppy is selected, get its ready state, otherwise just set non-ready
	if (m_floppy)
		data |= m_floppy->ready_r() << 1;
	else
		data |= 1 << 1;

	data |= m_fdc->drq_r()   << 7;

	if (VERBOSE_STATUS)
		logerror("nascom_fdc_device::status_r: 0x%02x\n", data);

	return data;
}
