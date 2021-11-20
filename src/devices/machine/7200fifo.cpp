// license:BSD-3-Clause
// copyright-holders:hap
/**********************************************************************

    IDT7200 series 9-bit Asynchronous FIFO Emulation

    TODO:
    - retransmit (RT pin)
    - cascaded width expansion mode (when needed)

**********************************************************************/

#include "emu.h"
#include "machine/7200fifo.h"


DEFINE_DEVICE_TYPE(IDT7200, idt7200_device, "idt7200", "IDT7200 FIFO (256x9)")
DEFINE_DEVICE_TYPE(IDT7201, idt7201_device, "idt7201", "IDT7201 FIFO (512x9)")
DEFINE_DEVICE_TYPE(IDT7202, idt7202_device, "idt7202", "IDT7202 FIFO (1024x9)")

//-------------------------------------------------
//  fifo7200_device - constructor
//-------------------------------------------------

fifo7200_device::fifo7200_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, int size)
	: device_t(mconfig, type, tag, owner, (uint32_t)0),
		m_ram_size(size),
		m_read_ptr(0), m_write_ptr(0), m_ef(0), m_ff(0), m_hf(0),
		m_ef_handler(*this),
		m_ff_handler(*this),
		m_hf_handler(*this)
{
}


//-------------------------------------------------
//  idt7200_device - constructor
//-------------------------------------------------

idt7200_device::idt7200_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	 : fifo7200_device(mconfig, IDT7200, tag, owner, 0x100)
{
}


//-------------------------------------------------
//  idt7201_device - constructor
//-------------------------------------------------

idt7201_device::idt7201_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	 : fifo7200_device(mconfig, IDT7201, tag, owner, 0x200)
{
}


//-------------------------------------------------
//  idt7202_device - constructor
//-------------------------------------------------

idt7202_device::idt7202_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	 : fifo7200_device(mconfig, IDT7202, tag, owner, 0x400)
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
	std::fill(m_buffer.begin(), m_buffer.end(), 0);
	m_read_ptr = 0;
	m_write_ptr = 0;

	m_ef = 1;
	m_ff = 0;
	m_hf = 0;

	m_ef_handler(!m_ef);
	m_ff_handler(!m_ff);
	m_hf_handler(!m_hf);
}



void fifo7200_device::fifo_write(uint16_t data)
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

	else if (!m_hf && (fifo_used() >= m_ram_size / 2))
	{
		m_hf = 1;
		m_hf_handler(!m_hf);
	}
}

uint16_t fifo7200_device::fifo_read()
{
	if (m_ef)
	{
		if (!machine().side_effects_disabled())
			logerror("IDT7200 %s fifo_read underflow!\n", tag());
		return 0x1ff;
	}

	uint16_t ret = m_buffer[m_read_ptr];
	if (!machine().side_effects_disabled())
	{
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

		else if (m_hf && (fifo_used() < m_ram_size / 2))
		{
			m_hf = 0;
			m_hf_handler(!m_hf);
		}
	}
	return ret;
}

int fifo7200_device::fifo_used()
{
	int ret = m_ram_size;

	if (!m_ff)
	{
		if (m_write_ptr >= m_read_ptr)
		{
			ret = m_write_ptr - m_read_ptr;
		}
		else
		{
			ret = m_ram_size + m_write_ptr - m_read_ptr;
		}
	}

	return ret;
}
