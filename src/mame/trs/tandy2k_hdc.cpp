// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Tandy 2000 hard disk controller emulation

    See tandy2k_hdc.h.

*********************************************************************/

#include "emu.h"
#include "tandy2k_hdc.h"

#define LOG_REGS    (1U << 1)
#define LOG_DATA    (1U << 2)

#define VERBOSE (0)

#include "logmacro.h"

#define LOGREGS(...) LOGMASKED(LOG_REGS, __VA_ARGS__)
#define LOGDATA(...) LOGMASKED(LOG_DATA, __VA_ARGS__)


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

// SDH register fields
enum : uint8_t
{
	SDH_HEAD  = 0x07,   // head select
	SDH_DRIVE = 0x18    // drive select
};


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(TANDY2K_HDC, tandy2k_hdc_device, "tandy2k_hdc", "Tandy 2000 hard disk controller")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void tandy2k_hdc_device::device_add_mconfig(machine_config &config)
{
	WD1010(config, m_hdc, 10_MHz_XTAL / 2);
	m_hdc->out_intrq_callback().set(FUNC(tandy2k_hdc_device::intrq_w));
	m_hdc->out_bcr_callback().set(FUNC(tandy2k_hdc_device::bcr_w));
	m_hdc->in_data_callback().set(FUNC(tandy2k_hdc_device::buf_in));
	m_hdc->out_data_callback().set(FUNC(tandy2k_hdc_device::buf_out));

	// Tandon TM502
	HARDDISK(config, "hdc:0", "st506_hdd");
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  tandy2k_hdc_device - constructor
//-------------------------------------------------

tandy2k_hdc_device::tandy2k_hdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, TANDY2K_HDC, tag, owner, clock),
	m_hdc(*this, "hdc"),
	m_hdd(*this, "hdc:0"),
	m_write_intrq(*this),
	m_ptr(0)
{
	std::fill(std::begin(m_buf), std::end(m_buf), 0);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tandy2k_hdc_device::device_start()
{
	// register for save states
	save_item(NAME(m_buf));
	save_item(NAME(m_ptr));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tandy2k_hdc_device::device_reset()
{
	mr_w();
}


//-------------------------------------------------
//  mr_w - master reset
//-------------------------------------------------

void tandy2k_hdc_device::mr_w()
{
	m_ptr = 0;

	m_hdc->brdy_w(0);
	m_hdc->drdy_w(m_hdd->exists() ? 1 : 0);
	m_hdc->sc_w(1);
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  read - host interface
//-------------------------------------------------

uint8_t tandy2k_hdc_device::read(offs_t offset)
{
	if (offset == 0)
	{
		// host empties the sector buffer
		uint8_t const data = m_buf[m_ptr & (BUFFER_SIZE - 1)];

		if (!machine().side_effects_disabled())
		{
			LOGDATA("BUF RD %03x: %02x\n", m_ptr, data);

			if ((m_ptr++ & (BUFFER_SIZE - 1)) == (BUFFER_SIZE - 1))
				m_hdc->brdy_w(1); // the sector has been transferred to the host
		}

		return data;
	}

	return m_hdc->read(offset);
}


//-------------------------------------------------
//  write - host interface
//-------------------------------------------------

void tandy2k_hdc_device::write(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0:
		// host fills the sector buffer
		LOGDATA("BUF WR %03x: %02x\n", m_ptr, data);

		m_buf[m_ptr & (BUFFER_SIZE - 1)] = data;

		if ((m_ptr++ & (BUFFER_SIZE - 1)) == (BUFFER_SIZE - 1))
			m_hdc->brdy_w(1); // the sector is ready for the controller
		break;

	case 6:
		// head select is latched outside the WD1010
		m_hdc->head_w(data & SDH_HEAD);

		// only one drive is fitted, so the second one never reports ready
		m_hdc->drdy_w((((data & SDH_DRIVE) >> 3) == 0 && m_hdd->exists()) ? 1 : 0);

		m_hdc->write(offset, data);
		break;

	case 7:
		// a command starts with an empty buffer
		m_hdc->brdy_w(0);
		m_ptr = 0;
		m_hdc->write(offset, data);
		break;

	default:
		m_hdc->write(offset, data);
		break;
	}
}


//-------------------------------------------------
//  reset_r/reset_w - controller master reset
//-------------------------------------------------

uint8_t tandy2k_hdc_device::reset_r()
{
	LOGREGS("RESET\n");

	if (!machine().side_effects_disabled())
		mr_w();

	return 0xff;
}

void tandy2k_hdc_device::reset_w(uint8_t data)
{
	LOGREGS("RESET %02x\n", data);

	mr_w();
}


//-------------------------------------------------
//  buf_in/buf_out - controller interface
//-------------------------------------------------

uint8_t tandy2k_hdc_device::buf_in()
{
	uint8_t const data = m_buf[m_ptr & (BUFFER_SIZE - 1)];

	m_ptr++;

	return data;
}

void tandy2k_hdc_device::buf_out(uint8_t data)
{
	m_buf[m_ptr & (BUFFER_SIZE - 1)] = data;

	m_ptr++;
}


//-------------------------------------------------
//  intrq_w - controller interrupt
//-------------------------------------------------

void tandy2k_hdc_device::intrq_w(int state)
{
	m_write_intrq(state);
}


//-------------------------------------------------
//  bcr_w - buffer counter reset
//-------------------------------------------------

void tandy2k_hdc_device::bcr_w(int state)
{
	if (state)
		m_ptr = 0;
}
