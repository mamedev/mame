// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
/**********************************************************************
    Spectrum Next Copper
Refs:
    https://gitlab.com/thesmog358/tbblue/-/raw/master/docs/extra-hw/copper/COPPER-v0.1c.TXT
**********************************************************************/

#include "emu.h"
#include "specnext_copper.h"

#include <cassert>


#define LOG_CTRL  (1U << 1)
#define LOG_DATA  (1U << 2)

#define VERBOSE ( LOG_GENERAL /*| LOG_CTRL | LOG_DATA*/ )
#include "logmacro.h"

#define LOGCTRL(...) LOGMASKED(LOG_CTRL, __VA_ARGS__)
#define LOGDATA(...) LOGMASKED(LOG_DATA, __VA_ARGS__)


// device type definition
DEFINE_DEVICE_TYPE(SPECNEXT_COPPER, specnext_copper_device, "specnext_copper", "Spectrum Next Copper")


specnext_copper_device::specnext_copper_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SPECNEXT_COPPER, tag, owner, clock)
	, m_timer(nullptr)
	, m_frame_timer(nullptr)
	, m_listram(*this, "listram", 0x400 * 2, ENDIANNESS_LITTLE)
	, m_out_nextreg_cb(*this)
	, m_in_until_pos_cb(*this)
{
}

void specnext_copper_device::copper_en_w(u8 data)
{
	if (m_copper_en != data)
	{
		m_copper_en = data;
		if ((m_copper_en == 0b01) || (m_copper_en == 0b11))
			m_copper_list_addr = 0;
		m_copper_dout = 0;

		switch(m_copper_en)
		{
		case 0b00:
			LOGCTRL("STOP\n");
			m_timer->reset();
			m_frame_timer->reset();
			break;
		case 0b01:
			LOGCTRL("RESET\n");
			m_timer->adjust(m_in_until_pos_cb(0x0000));
			break;
		case 0b10:
			LOGCTRL("START\n");
			m_timer->adjust(attotime::zero);
			break;
		case 0b11:
			LOGCTRL("FRAME\n");
			m_frame_timer->adjust(m_in_until_pos_cb(0x0000));
			break;
		}
	}
}

void specnext_copper_device::data_w(u16 addr, u8 data)
{
	LOGDATA("data(W) %03x %02x\n", addr, data);
	assert(addr < 0x800);
	m_listram[addr] = data;
}

TIMER_CALLBACK_MEMBER(specnext_copper_device::timer_callback)
{
	if (m_copper_dout == 0)
		m_copper_list_data = (m_listram[m_copper_list_addr << 1] << 8) | m_listram[(m_copper_list_addr << 1) | 1];

	if (m_copper_dout == 1) // if we are on MOVE, clear the output for the next cycle
	{
		m_out_nextreg_cb((m_copper_list_data >> 8) & 0x7f, m_copper_list_data & 0xff);
		m_copper_dout = 0;
	}
	else if (BIT(m_copper_list_data, 15) == 1)  // command WAIT
	{
		m_timer->adjust(m_in_until_pos_cb(0x8000 | m_copper_list_data));
		++m_copper_list_addr;
		m_copper_dout = 0;
		return;
	}
	else // command MOVE
	{
		if (BIT(m_copper_list_data, 8, 7) != 0) // dont generate the write pulse if its a NOP (MOVE 0,0)
			m_copper_dout = 1;
		++m_copper_list_addr;
	};

	m_copper_list_addr %= 0x400;
	m_timer->adjust(attotime::from_hz(clock()));
}

TIMER_CALLBACK_MEMBER(specnext_copper_device::frame_timer_callback)
{
	if (m_copper_en == 0b11)
	{
		m_copper_list_addr = 0;
		m_copper_dout = 0;
		m_frame_timer->adjust(m_in_until_pos_cb(0x0000));
		m_timer->adjust(attotime::zero);
	}
	else
		m_frame_timer->reset();
}


void specnext_copper_device::device_start()
{
	m_timer = timer_alloc(FUNC(specnext_copper_device::timer_callback), this);
	m_frame_timer = timer_alloc(FUNC(specnext_copper_device::frame_timer_callback), this);

	m_in_until_pos_cb.resolve_safe(attotime::zero);

	save_item(NAME(m_copper_en));
	save_item(NAME(m_copper_list_addr));
	save_item(NAME(m_copper_list_data));
	save_item(NAME(m_copper_dout));
}

void specnext_copper_device::device_reset()
{
	m_timer->reset();
	m_frame_timer->reset();
	memset(m_listram, 0, sizeof(m_listram));

	m_copper_en = 0b00;
	m_copper_list_addr = 0x0000;
	m_copper_list_data = 0x00;
	m_copper_dout = 0;
}
