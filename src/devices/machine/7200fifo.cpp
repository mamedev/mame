// license:BSD-3-Clause
// copyright-holders:hap
/**********************************************************************

    IDT7200 series 9-bit Asynchronous FIFO Emulation

    TODO:
    - retransmit (RT pin)
    - cascaded width expansion mode (when needed)

**********************************************************************/

#include "machine/7200fifo.h"


const device_type FIFO7200 = &device_creator<fifo7200_device>;

//-------------------------------------------------
//  fifo7200_device - constructor
//-------------------------------------------------

fifo7200_device::fifo7200_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, FIFO7200, "IDT7200 FIFO", tag, owner, clock, "fifo7200", __FILE__),
		m_ram_size(0), m_read_ptr(0), m_write_ptr(0), m_ef(0), m_ff(0), m_hf(0),
		m_ef_handler(*this),
		m_ff_handler(*this),
		m_hf_handler(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void fifo7200_device::device_start()
{
	assert(m_ram_size > 1 && ~m_ram_size & 1);
	m_buffer.resize(m_ram_size);

	// resolve callbacks
	m_ef_handler.resolve_safe();
	m_ff_handler.resolve_safe();
	m_hf_handler.resolve_safe();

	// state save
	save_item(NAME(m_buffer));
	save_item(NAME(m_read_ptr));
	save_item(NAME(m_write_ptr));
	save_item(NAME(m_ef));
	save_item(NAME(m_ff));
	save_item(NAME(m_hf));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void fifo7200_device::device_reset()
{
	// master reset
	m_buffer.clear();
	m_read_ptr = 0;
	m_write_ptr = 0;

	m_ef = 1;
	m_ff = 0;
	m_hf = 0;

	m_ef_handler(!m_ef);
	m_ff_handler(!m_ff);
	m_hf_handler(!m_hf);
}



void fifo7200_device::fifo_write(UINT16 data)
{
	if (m_ff)
	{
		logerror("IDT7200 %s fifo_write overflow!\n", tag());
		return;
	}

	m_buffer[m_write_ptr] = data & 0x1ff;
	m_write_ptr = (m_write_ptr + 1) % m_ram_size;

	// update flags
	if (m_ef)
	{
		m_ef = 0;
		m_ef_handler(!m_ef);
	}

	else if (m_read_ptr == m_write_ptr)
	{
		m_ff = 1;
		m_ff_handler(!m_ff);
	}

	else if (((m_read_ptr + 1 + m_ram_size / 2) % m_ram_size) == m_write_ptr)
	{
		m_hf = 1;
		m_hf_handler(!m_hf);
	}
}

UINT16 fifo7200_device::fifo_read()
{
	if (m_ef)
	{
		logerror("IDT7200 %s fifo_read underflow!\n", tag());
		return 0x1ff;
	}

	UINT16 ret = m_buffer[m_read_ptr];
	m_read_ptr = (m_read_ptr + 1) % m_ram_size;

	// update flags
	if (m_ff)
	{
		m_ff = 0;
		m_ff_handler(!m_ff);
	}

	else if (m_read_ptr == m_write_ptr)
	{
		m_ef = 1;
		m_ef_handler(!m_ef);
	}

	else if (((m_read_ptr + m_ram_size / 2) % m_ram_size) == m_write_ptr)
	{
		m_hf = 0;
		m_hf_handler(!m_hf);
	}

	return ret;
}
