// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***************************************************************************

    bcreader.c

    Generic barcode reader emulation.

    This device only provides the storage of the actual barcode, entered
    by the user via Internal UI, both as a raw strip of pixels (up to 95,
    for an EAN-13 barcode) and as an array of 0-9 digits.

    It is up to the driver to handle the serial transfer of the data to
    the emulated machine, depending on the used protocol

    E.g. Bandai Datach games directly read the raw pixel sequence of
    black/white bars;
    OTOH Barcode Battle (used by Barcode World for NES and a few SNES
    titles) sends the digits as sequences of 20 bytes (13 for the code,
    suitably padded for shorted codes, followed by a signature) and the
    actual serial transmission to the console is up to the slot device
    connected to the NES/SNES controller port (yet to be emulated, in this
    case)

    Note: we currently support the following barcode formats
    * UPC-A: 12 digits
    * EAN-13: 13 digits (extension of the former)
    * EAN-8: 8 digits (same encoding as UPC-A, but 4-digits blocks instead
      of 6-digits blocks)
    Notice that since EAN-13 is an extension of UPC-A, we just treat UPC-A
    as an EAN-13 code with leading '0'. If any barcode reader shall be found
    which supports the older format only, this shall be changed


    TODO: add support for UPC-E barcodes? these are 8 digits barcodes with 17
    black stripes (they are compressed UPC-A codes). Datach reader does not
    support these, so it is low priority


    TODO 2: verify barcode checksum in is_valid() and not only length, so
    that we can then use the actual last digit in the decode function below,
    rather than replacing it with the checksum value


    TODO 3: possibly the white spaces before the actual barcode (see the
    61 white pixels sent by read_pixel() before and after the code), shall
    be moved to the specific implementations to emulate different "sensitivity"
    of the readers? Bandai Datach seems to need at least 32 pixels...

***************************************************************************/

#include "emu.h"
#include "bcreader.h"

// device type definition
const device_type BARCODE_READER = &device_creator<barcode_reader_device>;

//-------------------------------------------------
//  barcode_reader_device - constructor
//-------------------------------------------------

barcode_reader_device::barcode_reader_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, BARCODE_READER, "Barcode Reader", tag, owner, clock, "bcreader", __FILE__)
	, m_byte_length(0)
	, m_pixel_length(0)
	, m_byte_count(0)
	, m_pixel_count(0)
	, m_new_code(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void barcode_reader_device::device_start()
{
	save_item(m_byte_data, "DATACH/m_byte_data");
	save_item(m_pixel_data, "DATACH/m_pixel_data");
	save_item(m_byte_length, "DATACH/m_byte_length");
	save_item(m_pixel_length, "DATACH/m_pixel_length");
	save_item(m_byte_count, "DATACH/m_byte_count");
	save_item(m_pixel_count, "DATACH/m_pixel_count");
	save_item(m_new_code, "DATACH/m_new_code");
}


//-------------------------------------------------
//  Barcode Decoding - convert the entered sequence
//  of digits into a sequence of B/W pixels (the
//  actual bars) - each digit corresponds to 7 pixels
//  0 is black, 1 is white
//-------------------------------------------------

// Left Odd
static const UINT8 bcread_data_LO[10][7] =
{
	{1, 1, 1, 0, 0, 1, 0}, {1, 1, 0, 0, 1, 1, 0},
	{1, 1, 0, 1, 1, 0, 0}, {1, 0, 0, 0, 0, 1, 0},
	{1, 0, 1, 1, 1, 0, 0}, {1, 0, 0, 1, 1, 1, 0},
	{1, 0, 1, 0, 0, 0, 0}, {1, 0, 0, 0, 1, 0, 0},
	{1, 0, 0, 1, 0, 0, 0}, {1, 1, 1, 0, 1, 0, 0}
};

// Left Even
static const UINT8 bcread_data_LE[10][7] =
{
	{1, 0, 1, 1, 0, 0, 0}, {1, 0, 0, 1, 1, 0, 0},
	{1, 1, 0, 0, 1, 0, 0}, {1, 0, 1, 1, 1, 1, 0},
	{1, 1, 0, 0, 0, 1, 0}, {1, 0, 0, 0, 1, 1, 0},
	{1, 1, 1, 1, 0, 1, 0}, {1, 1, 0, 1, 1, 1, 0},
	{1, 1, 1, 0, 1, 1, 0}, {1, 1, 0, 1, 0, 0, 0}
};

// Right Even
static const UINT8 bcread_data_RE[10][7] =
{
	{0, 0, 0, 1, 1, 0, 1}, {0, 0, 1, 1, 0, 0, 1},
	{0, 0, 1, 0, 0, 1, 1}, {0, 1, 1, 1, 1, 0, 1},
	{0, 1, 0, 0, 0, 1, 1}, {0, 1, 1, 0, 0, 0, 1},
	{0, 1, 0, 1, 1, 1, 1}, {0, 1, 1, 1, 0, 1, 1},
	{0, 1, 1, 0, 1, 1, 1}, {0, 0, 0, 1, 0, 1, 1}
};

// EAN-13 added an extra digit to determine
// the parity type of the first digits block
static const UINT8 bcread_parity_type[10][6] =
{
	{1, 1, 1, 1, 1, 1}, {1, 1, 0, 1, 0, 0},
	{1, 1, 0, 0, 1, 0}, {1, 1, 0, 0, 0, 1},
	{1, 0, 1, 1, 0, 0}, {1, 0, 0, 1, 1, 0},
	{1, 0, 0, 0, 1, 1}, {1, 0, 1, 0, 1, 0},
	{1, 0, 1, 0, 0, 1}, {1, 0, 0, 1, 0, 1}
};


void barcode_reader_device::decode(int len)
{
	int output = 0;
	int sum = 0;

	if (len == 13)
	{
		// UPC-A and EAN-13

		m_pixel_data[output++] = 0;
		m_pixel_data[output++] = 1;
		m_pixel_data[output++] = 0;

		for (int i = 1; i < 7; i++)
		{
			if (bcread_parity_type[m_byte_data[0]][i - 1])
			{
				for (int j = 0; j < 7; j++)
					m_pixel_data[output++] = bcread_data_LO[m_byte_data[i]][j];
			}
			else
			{
				for (int j = 0; j < 7; j++)
					m_pixel_data[output++] = bcread_data_LE[m_byte_data[i]][j];
			}
		}

		m_pixel_data[output++] = 1;
		m_pixel_data[output++] = 0;
		m_pixel_data[output++] = 1;
		m_pixel_data[output++] = 0;
		m_pixel_data[output++] = 1;

		for (int i = 7; i < 12; i++)
		{
			for (int j = 0; j < 7; j++)
				m_pixel_data[output++] = bcread_data_RE[m_byte_data[i]][j];
		}

		// ignore the last digit and compute it as checksum of the first 12
		for (int i = 0; i < 12; i++)
			sum += (i & 1) ? (m_byte_data[i] * 3) : (m_byte_data[i] * 1);
	}
	else if (len == 8)
	{
		// EAN-8 (same encoding as UPC-A, but only 4+4 digits, instead of 6+6)

		m_pixel_data[output++] = 0;
		m_pixel_data[output++] = 1;
		m_pixel_data[output++] = 0;

		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 7; j++)
				m_pixel_data[output++] = bcread_data_LO[m_byte_data[i]][j];
		}

		m_pixel_data[output++] = 1;
		m_pixel_data[output++] = 0;
		m_pixel_data[output++] = 1;
		m_pixel_data[output++] = 0;
		m_pixel_data[output++] = 1;

		for (int i = 4; i < 7; i++)
		{
			for (int j = 0; j < 7; j++)
				m_pixel_data[output++] = bcread_data_RE[m_byte_data[i]][j];
		}

		// ignore the last digit and compute it as checksum of the first 12
		for (int i = 0; i < 7; i++)
			sum += (i & 1) ? (m_byte_data[i] * 1) : (m_byte_data[i] * 3);
	}

	if (m_pixel_data)
	{
		sum = (10 - (sum % 10)) % 10;
		if (sum != m_byte_data[len - 1])
			logerror("WARNING: wrong checksum detected in the barcode! chksum %d last digit %d\n",
								sum, m_byte_data[len - 1]);

		for (int i = 0; i < 7; i++)
			m_pixel_data[output++] = bcread_data_RE[sum][i];

		m_pixel_data[output++] = 0;
		m_pixel_data[output++] = 1;
		m_pixel_data[output++] = 0;
	}

	m_byte_length = len;
	m_pixel_length = output;

//  printf("byte len %d - pixel len\n", m_byte_length, m_pixel_length);
}


//-------------------------------------------------
//  write_code - invoked by UI, stores the barcode
//  both as an array of digits and as a sequence
//  of B/W pixels (the actual bars)
//-------------------------------------------------

void barcode_reader_device::write_code(const char *barcode, int len)
{
	int shift = 0;
	int deconde_len = len;

	// len has already been checked to be one of the following: 8, 12 or 13

	if (deconde_len == 12)
	{
		// convert UPC-A to EAN-13
		shift = 1;
		deconde_len = 13;
		m_byte_data[0] = 0;
	}

	for (int i = 0; i < len; i++)
		m_byte_data[i+shift] = barcode[i] - '0';

	decode(deconde_len);

	m_new_code = 1;
}


//-------------------------------------------------
//  read_code - accessor for drivers which read
//  the codes by bytes
//-------------------------------------------------

UINT8 barcode_reader_device::read_code()
{
	if (m_new_code)
	{
		if (m_byte_count < m_byte_length)
		{
			UINT8 val = m_byte_data[m_byte_count];
			m_byte_count++;
			return val;
		}
		else
		{
			m_byte_count = 0;
			m_new_code = 0;
			return 0;
		}
	}

	// no pending transfer
	return 0;
}

//-------------------------------------------------
//  read_pixel - accessor for drivers which read
//  the codes by pixels
//-------------------------------------------------

int barcode_reader_device::read_pixel()
{
	if (m_new_code)
	{
		// start of card: 61 white pixels
		if (m_pixel_count < 61)
		{
			m_pixel_count++;
			return 1;
		}
		// barcode: approx 95 pixels for B/W bars
		else if (m_pixel_count < 61 + m_pixel_length)
		{
			// actual barcode starts here
			int val = m_pixel_data[m_pixel_count - 61];
			m_pixel_count++;
			return val;
		}
		// end of card: 61 white pixels
		else if (m_pixel_count < 61 + m_pixel_length + 61)
		{
			m_pixel_count++;
			return 1;
		}
		// finished scan, erase code
		else
		{
			m_pixel_count = 0;
			m_new_code = 0;
			return 0;
		}
	}

	// no pending transfer = black pixel = 0
	return 0;
}
