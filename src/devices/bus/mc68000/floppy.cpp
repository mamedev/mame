// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    mc-68000-Computer Floppy Interface Card

    TODO:
    - Jumpers can change the meaning of some lines

    Notes:
    - Has two bus interfaces and supports up to 8 drives
    - Can either use the 1793 or 1797

***************************************************************************/

#include "emu.h"
#include "floppy.h"

#define VERBOSE 0

DEFINE_DEVICE_TYPE(MC68000_FLOPPY, mc68000_floppy_device, "mc68000_floppy", "mc-68000 Floppy Interface")

mc68000_floppy_device::mc68000_floppy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, MC68000_FLOPPY, tag, owner, clock),
	device_mc68000_sysbus_card_interface(mconfig, *this),
	m_fdc(*this, "fdc"),
	m_floppy(*this, "fdc:%u", 0U),
	m_latch(0x00)
{
}

// very flexible regarding drive support, we just add the most common ones
static void mc68000_bus1_floppies(device_slot_interface &device)
{
	device.option_add("35dd", FLOPPY_35_DD);
	device.option_add("525qd", FLOPPY_525_QD);
}

static void mc68000_bus2_floppies(device_slot_interface &device)
{
	device.option_add("8dsdd", FLOPPY_8_DSDD);
}

void mc68000_floppy_device::device_add_mconfig(machine_config &config)
{
	FD1793(config, m_fdc, 16_MHz_XTAL / 8);
	m_fdc->intrq_wr_callback().set(FUNC(mc68000_floppy_device::intrq_w));
	m_fdc->drq_wr_callback().set(FUNC(mc68000_floppy_device::drq_w));

	// floppy drive bus 1
	FLOPPY_CONNECTOR(config, m_floppy[0], mc68000_bus1_floppies, "525qd", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy[1], mc68000_bus1_floppies, "525qd", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy[2], mc68000_bus1_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy[3], mc68000_bus1_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats);

	// floppy drive bus 2
	FLOPPY_CONNECTOR(config, m_floppy[4], mc68000_bus2_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy[5], mc68000_bus2_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy[6], mc68000_bus2_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy[7], mc68000_bus2_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats);
}

void mc68000_floppy_device::device_start()
{
	// register for save states
	save_item(NAME(m_latch));
}

void mc68000_floppy_device::device_reset()
{
	m_latch = 0x00;
}

uint16_t mc68000_floppy_device::floppy_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t data = 0xffff;

	if (ACCESSING_BITS_8_15)
	{
		switch (offset & 0x07)
		{
		case 0:
		case 1:
		case 2:
		case 3:
			data &= (m_fdc->read(offset & 0x03) << 8) | 0xff;
			break;

		case 4:
			// 7-------  intrq from fdc
			// -6------  drq from fdc
			// --543210  write only

			data &= (m_latch << 8) | 0xff;
		}
	}

	return data & mem_mask;
}

void mc68000_floppy_device::floppy_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (VERBOSE)
		logerror("floppy_w: %06x = %04x & %04x\n", offset, data, mem_mask);

	if (ACCESSING_BITS_8_15)
	{
		data >>= 8;

		switch (offset & 0x07)
		{
		case 0:
		case 1:
		case 2:
		case 3:
			m_fdc->write(offset & 0x03, data);
			break;

		case 4:
			// 76------  read only
			// --5-----  side select
			// ---4----  force ready
			// ----3---  density select
			// -----2--  bus select
			// ------10  drive select

			floppy_image_device *floppy = nullptr;

			if (BIT(data, 2))
			{
				// bus 1, drives 0 to 3 at 1 MHz
				floppy = m_floppy[0 + (data & 0x03)]->get_device();
				m_fdc->set_clock_scale(0.5);
			}
			else
			{
				// bus 2, drives 4 to 7 at 2 MHz
				floppy = m_floppy[4 + (data & 0x03)]->get_device();
				m_fdc->set_clock_scale(1.0);
			}

			m_fdc->set_floppy(floppy);

			if (floppy)
			{
				// TODO: motor should only run for a few seconds after each access (for bus 1)
				floppy->mon_w(0);
				floppy->ss_w(BIT(data, 5));
			}

			m_fdc->dden_w(BIT(data, 3));
			m_fdc->set_force_ready(BIT(data, 4));
		}
	}
}

void mc68000_floppy_device::intrq_w(int state)
{
	m_latch &= ~(1 << 7);
	m_latch |= state << 7;
}

void mc68000_floppy_device::drq_w(int state)
{
	m_latch &= ~(1 << 6);
	m_latch |= state << 6;

	m_bus->irq6_w(state);
}
