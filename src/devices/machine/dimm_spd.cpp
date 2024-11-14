// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    DIMM serial presence detect (SPD) readback device
    by R. Belmont

    Each DIMM contains a small EEPROM with information about the capacity and
    timings of the module.  The EEPROM speaks a version of I2C called SMBus.
*/


#include "emu.h"
#include "dimm_spd.h"

#define LOG_DATAOUT (1U << 1)

#define VERBOSE (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(DIMM_SPD, dimm_spd_device, "dimm_spd", "DIMM Serial Presence Detect")

dimm_spd_device::dimm_spd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, DIMM_SPD, tag, owner, clock),
	i2c_hle_interface(mconfig, *this, 0)    // address will be overridden by set_address as before
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dimm_spd_device::device_start()
{
	std::fill(std::begin(m_data), std::end(m_data), 0);

	m_data[0] = 128;    // # of bytes in EEPROM
	m_data[1] = 8;      // log2 of EEPROM size
	m_data[2] = 4;      // 4 is SDRAM, 7 is DDR SDRAM, 8 is DDR2, 11 is DDR3, 12 is DDR4
	m_data[3] = 12;     // # of rows
	m_data[4] = 8;      // # of columns
	m_data[5] = 1;      // # of banks (12/8/1 = 32 MiB)
	m_data[6] = 64;     // data bus width low byte
	m_data[7] = 0;      // data bus width high byte
	m_data[11] = 0;     // non-ECC (1=parity, 2=ECC)
	m_data[62] = 0x12;  // SPD version 1.2
}

void dimm_spd_device::set_dimm_size(dimm_size_t size)
{
	m_size = size;

	switch (size)
	{
		case SIZE_4_MIB:
			m_data[3] = 12; // # of rows
			m_data[4] = 5;  // # of columns
			m_data[5] = 1;  // # of banks
			break;

		case SIZE_8_MIB:
			m_data[3] = 12;
			m_data[4] = 6;
			m_data[5] = 1;
			break;

		case SIZE_16_MIB:
			m_data[3] = 12;
			m_data[4] = 7;
			m_data[5] = 1;
			break;

		case SIZE_32_MIB:
			m_data[3] = 12;
			m_data[4] = 8;
			m_data[5] = 1;
			break;

		case SIZE_64_MIB:
			m_data[3] = 12;
			m_data[4] = 9;
			m_data[5] = 1;
			break;

		case SIZE_128_MIB:
			m_data[3] = 12;
			m_data[4] = 10;
			m_data[5] = 1;
			break;

		case SIZE_256_MIB:
			m_data[3] = 12;
			m_data[4] = 10;
			m_data[5] = 2;
			break;

		case SIZE_SLOT_EMPTY:
			break;
	}
}

u8 dimm_spd_device::read_data(u16 offset)
{
	// i2c_hle_interface auto-increments the address but doesn't wrap it around
	if (offset >= m_size)
	{
		m_data_offset = 0;
	}

	return m_data[offset];
}
