// license:BSD-3-Clause
// copyright-holders:smf
#include "emu.h"
#include "latch.h"

DEFINE_DEVICE_TYPE(OUTPUT_LATCH, output_latch_device, "output_latch", "Output Latch")

output_latch_device::output_latch_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, OUTPUT_LATCH, tag, owner, clock)
	, m_bit_handlers{ { *this }, { *this }, { *this }, { *this }, { *this }, { *this }, { *this }, { *this } }
	, m_bits{ -1, -1, -1, -1, -1, -1, -1, -1 }
	, m_resolved(false)
{
}

void output_latch_device::device_start()
{
	save_item(NAME(m_bits));
}

void output_latch_device::write(uint8_t data)
{
	if (!m_resolved)
	{
		// HACK: move to device_config_complete() when devcb supports that
		for (devcb_write_line &handler : m_bit_handlers)
			handler.resolve_safe();

		m_resolved = true;
	}

	for (unsigned i = 0; 8 > i; ++i)
	{
		int const bit = BIT(data, i);
		if (bit != m_bits[i])
		{
			m_bits[i] = bit;
			if (!m_bit_handlers[i].isnull())
				m_bit_handlers[i](bit);
		}
	}
}
