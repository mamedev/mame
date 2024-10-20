// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/*********************************************************************

    bcreader.h

    Generic barcode reader emulation.

*********************************************************************/

#ifndef MAME_DEVICES_MACHINE_BCREADER_H
#define MAME_DEVICES_MACHINE_BCREADER_H

#pragma once

class barcode_reader_device : public device_t
{
public:
	// construction/destruction
	barcode_reader_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void write_code(const char *barcode, int len);
	int get_pending_code() { return m_new_code; }
	int get_byte_length() { return m_byte_length; }
	uint8_t read_code();
	int read_pixel();

	// TODO: add checksum validation!
	bool is_valid(int len) { return (len != 12 && len != 13 && len != 8) ? false : true; }
	void decode(int len);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	uint8_t m_byte_data[13];
	uint8_t m_pixel_data[100];
	int m_byte_length;
	int m_pixel_length;
	int m_byte_count;
	int m_pixel_count;
	int m_new_code;
};


// device type definition
DECLARE_DEVICE_TYPE(BARCODE_READER, barcode_reader_device)

// device type iterator
typedef device_type_enumerator<barcode_reader_device> barcode_reader_device_enumerator;

#endif // MAME_DEVICES_MACHINE_BCREADER_H
