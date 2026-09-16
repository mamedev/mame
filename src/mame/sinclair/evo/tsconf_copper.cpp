// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
/**********************************************************************
    TS-Configuration Copper
**********************************************************************/

#include "emu.h"
#include "tsconf_copper.h"


#define LOG_CTRL  (1U << 1)
#define LOG_DATA  (1U << 2)

//#define VERBOSE ( LOG_GENERAL /*| LOG_CTRL | LOG_DATA*/ )
#include "logmacro.h"

#define LOGCTRL(...) LOGMASKED(LOG_CTRL, __VA_ARGS__)
#define LOGDATA(...) LOGMASKED(LOG_DATA, __VA_ARGS__)


// device type definition
DEFINE_DEVICE_TYPE(TSCONF_COPPER, tsconf_copper_device, "tsconf_copper", "TS-Configuration Copper")


tsconf_copper_device::tsconf_copper_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, TSCONF_COPPER, tag, owner, clock)
	, m_timer(nullptr)
	, m_cl_data(*this, "cl_data", 0x100, ENDIANNESS_LITTLE)
	, m_out_wreg_cb(*this)
	, m_in_dma_ready_cb(*this, 0)
	, m_in_until_pos_cb(*this)
{
}

void tsconf_copper_device::copper_en_w(u8 data)
{
	m_en = data != 0xff;
	m_pc = data;

	if (m_en)
	{
		LOGCTRL("START\n");
		m_timer->adjust(attotime::zero);
	}
	else
	{
		LOGCTRL("STOP\n");
		m_timer->reset();
	}
}

void tsconf_copper_device::cl_data_w(u16 addr, u8 data)
{
	addr &= 0x1ff;
	LOGDATA("data(W) %02x %04x\n", addr, data);
	m_cl_data[addr >> 1] = (addr & 1)
		? (m_cl_data[addr >> 1] & 0x00ff) | (data << 8)
		: (m_cl_data[addr >> 1] & 0xff00) | data;
}

void tsconf_copper_device::dma_ready_w(int status)
{
	if(m_en && (status & 1) && (m_cl_data[m_pc] & 0xffff) == 0b1111'1011'1111'0001)
	{
		++m_pc;
		m_timer->adjust(attotime::from_hz(clock()), 0);
	}
}

TIMER_CALLBACK_MEMBER(tsconf_copper_device::timer_callback)
{
	const u16 data = m_cl_data[m_pc];
	if (BIT(data, 12, 4) == 0b1111) // all instructions except WREG
	{
		switch (BIT(data, 8, 4))
		{
			case 0b0110: //SETA
				m_a = data & 0xff;
				++m_pc;
				break;

			case 0b1000: //SETB
			case 0b1001:
				m_b = data & 0x1ff;
				++m_pc;
				break;

			case 0b0111: // WAITX
				if (param == 0b1111'0111)
					++m_pc;
				else
				{
					const u8 x = data & 0xff;
					if (x < 224)
						m_timer->adjust(m_in_until_pos_cb(0x8000 | x), 0b1111'0111);
					else
						m_timer->reset();
					return;
				}
				break;

			case 0b1011: // WAITY
				if (BIT(data, 4, 4) == 0b1111) // WAIT event
				{
					const u8 event = data & 0x0f;
					if (event == param)
						++m_pc;
					else
					{
						switch (event)
						{
							case 1: // DMA
								m_timer->reset();
								return;
								break;
							case 2: // frame
								m_timer->adjust(m_in_until_pos_cb(0b00 << 14), 2);
								return;
								break;
							case 3: // line start
								m_timer->adjust(m_in_until_pos_cb(0b11 << 14), 3);
								return;
								break;
							default:
								++m_pc;
								break;

						}
					}
					break;
				}
				[[fallthrough]];
			case 0b1010:
				if (param == 0b1111'1010)
					++m_pc;
				else
				{
					const u16 y = data & 0x1ff;
					if (y < 320)
						m_timer->adjust(m_in_until_pos_cb(0x4000 | y), 0b1111'1010);
					else
						m_timer->reset();
					return;
				}
				break;

			case 0b1100: // DJNZA
				--m_a;
				m_pc = m_a ? data & 0xff : (m_pc + 1);
				break;

			case 0b1101: // DJNZB
				--m_b;
				m_pc = m_b ? data & 0xff : (m_pc + 1);
				break;

			case 0b1110: // CALL
				m_ret_pc = m_pc + 1;
				m_pc = data & 0xff;
				break;

			case 0b1111: // JUMP
				{
					const u8 to = data & 0xff;
					if (to == m_pc) // HALT
					{
						m_en = false;
						m_timer->reset();
						return;
					}

					m_pc = (to == 0xff) ? m_ret_pc : to;
				}
				break;

			case 0b0101: // SIGNAL
				++m_pc; // TODO signal logic
				break;

			default: // reserved instructions
				++m_pc;
				break;
		}
	}
	else
	{
		m_out_wreg_cb((data & 0xff00) | 0xaf, data & 0x00ff);
		++m_pc;
	}

	m_timer->adjust(attotime::from_hz(clock()), 0);
}


void tsconf_copper_device::device_start()
{
	m_timer = timer_alloc(FUNC(tsconf_copper_device::timer_callback), this);

	m_in_until_pos_cb.resolve_safe(attotime::zero);

	save_item(NAME(m_en));
	save_item(NAME(m_pc));
	save_item(NAME(m_ret_pc));
	save_item(NAME(m_a));
	save_item(NAME(m_b));
}

void tsconf_copper_device::device_reset()
{
	m_timer->reset();

	m_en = false;
	m_pc = 0;
	m_ret_pc = 0;
	m_a = 0;
	m_b = 0;
}
