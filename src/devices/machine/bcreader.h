// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/*********************************************************************

    bcreader.h

    Generic barcode reader emulation.

*********************************************************************/

#ifndef __BCREADER_H_
#define __BCREADER_H_

#define MCFG_BARCODE_READER_ADD( _tag ) \
	MCFG_DEVICE_ADD( _tag, BARCODE_READER, 0 )

#define MCFG_BARCODE_READER_REMOVE( _tag ) \
	MCFG_DEVICE_REMOVE( _tag )


// ======================> barcode_reader_device

class barcode_reader_device : public device_t
{
public:
	// construction/destruction
	barcode_reader_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	void write_code(const char *barcode, int len);
	int get_pending_code() { return m_new_code; }
	int get_byte_length() { return m_byte_length; }
	UINT8 read_code();
	int read_pixel();

	// TODO: add checksum validation!
	bool is_valid(int len) { return (len != 12 && len != 13 && len != 8) ? FALSE : TRUE; }
	void decode(int len);

protected:
	// device-level overrides
	virtual void device_start() override;

	UINT8 m_byte_data[13];
	UINT8 m_pixel_data[100];
	int m_byte_length;
	int m_pixel_length;
	int m_byte_count;
	int m_pixel_count;
	int m_new_code;
};


// device type definition
extern const device_type BARCODE_READER;

// device type iterator
typedef device_type_iterator<&device_creator<barcode_reader_device>, barcode_reader_device> barcode_reader_device_iterator;


#endif
