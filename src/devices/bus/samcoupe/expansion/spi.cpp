// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    S.P.I. SAM Parallel Interface for SAM Coupe

***************************************************************************/

#include "emu.h"
#include "spi.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SAM_SPI, sam_spi_device, "spi", "S.P.I. SAM Parallel Interface")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void sam_spi_device::device_add_mconfig(machine_config &config)
{
	OUTPUT_LATCH(config, m_data_out);

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set(FUNC(sam_spi_device::centronics_busy_w));
	m_centronics->set_output_latch(*m_data_out);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sambus_device - constructor
//-------------------------------------------------

sam_spi_device::sam_spi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SAM_SPI, tag, owner, clock),
	device_samcoupe_expansion_interface(mconfig, *this),
	m_data_out(*this, "data_out"),
	m_centronics(*this, "centronics"),
	m_print(0),
	m_busy(0),
	m_mode(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sam_spi_device::device_start()
{
	// register for savestates
	save_item(NAME(m_print));
	save_item(NAME(m_busy));
	save_item(NAME(m_mode));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

WRITE_LINE_MEMBER( sam_spi_device::centronics_busy_w )
{
	m_busy = state;
}

void sam_spi_device::print_w(int state)
{
	m_print = state;
}

uint8_t sam_spi_device::iorq_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (m_print)
	{
		switch (offset & 0x07)
		{
			case 0x01:
				data = 0xfe | m_busy;
				break;

			case 0x02:
				logerror("Warning: Data read unsupported\n");
				data = 0xff;
				break;
		}
	}

	return data;
}

void sam_spi_device::iorq_w(offs_t offset, uint8_t data)
{
	if (m_print)
	{
		switch (offset & 0x07)
		{
			case 0x00:
				m_data_out->write(data);
				break;

			case 0x01:
				m_centronics->write_strobe(BIT(data, 0));
				break;

			case 0x02:
				m_mode = BIT(data, 0); // 0 = output (default), 1 = input
				break;
		}
	}
}
