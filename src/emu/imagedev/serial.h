/****************************************************************************

    serial.h

    Code for handling serial port image devices

****************************************************************************/

#ifndef __SERIAL_H__
#define __SERIAL_H__

#include "diimage.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> serial_data_stream
struct serial_data_stream
{
	/* pointer to buffer */
	UINT8 *pData;
	/* length of buffer */
	unsigned long DataLength;

	/* bit offset within current byte */
	unsigned long BitCount;
	/* byte offset within data */
	unsigned long ByteCount;
};

// ======================> serial_image_interface

struct serial_image_interface
{
	int m_baud_rate;
	int m_data_bits;
	int m_stop_bits;
	int m_parity;
	bool m_transmit_on_start;
	const char *m_tag_connected;
};

// ======================> serial_image_device

class serial_image_device : public device_t,
							public serial_image_interface,
							public device_serial_interface,
							public device_image_interface
{
public:
	// construction/destruction
	serial_image_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~serial_image_device();

	// image-level overrides
	virtual bool call_load();
	virtual void call_unload();

	virtual iodevice_t image_type() const { return IO_SERIAL; }

	virtual bool is_readable()  const { return 1; }
	virtual bool is_writeable() const { return 1; }
	virtual bool is_creatable() const { return 1; }
	virtual bool must_be_loaded() const { return 0; }
	virtual bool is_reset_on_load() const { return 0; }
	virtual const char *image_interface() const { return NULL; }
	virtual const char *file_extensions() const { return ""; }
	virtual const option_guide *create_option_guide() const { return NULL; }

	// specific implementation
	void timer_callback();
	virtual void input_callback(UINT8 state);
	void sent_char();
	void set_transmit_state(int state);
	int load_internal(unsigned char **ptr, int *pDataSize);

	// data stream
	int data_stream_get_data_bit_from_data_byte(serial_data_stream *stream);
	void data_stream_reset(serial_data_stream *stream);
	void data_stream_free(serial_data_stream *stream);
	void data_stream_init(serial_data_stream *stream, unsigned char *pData, unsigned long DataLength)   ;
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();

private:
	/* transmit data bit-stream */
	serial_data_stream m_transmit;
	/* receive data bit-stream */
	serial_data_stream m_receive;

	int m_transmit_state;

	/* baud rate timer */
	emu_timer   *m_timer;
};


// device type definition
extern const device_type SERIAL;


#define MCFG_SERIAL_ADD(_tag,_config) \
	MCFG_DEVICE_ADD(_tag, SERIAL, 0) \
	MCFG_DEVICE_CONFIG(_config)
#endif /* __SERIAL_H__ */
