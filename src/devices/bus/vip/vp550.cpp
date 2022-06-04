// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    RCA VIP Super Sound System VP550 emulation

    TODO: Implement VP551 variant

**********************************************************************/

#include "emu.h"
#include "vp550.h"
#include "speaker.h"

#define VERBOSE	(0)
#include "logmacro.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(VP550, vp550_device, "vp550", "VP-550 Super Sound")


//-------------------------------------------------
//  device_add_mconfig
//-------------------------------------------------

void vp550_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "mono").front_center();

	CDP1863(config, m_pfg[0], 0);
	m_pfg[0]->set_clock2(0);
	m_pfg[0]->add_route(ALL_OUTPUTS, "mono", 1.0);

	CDP1863(config, m_pfg[1], 0);
	m_pfg[1]->set_clock2(0);
	m_pfg[1]->add_route(ALL_OUTPUTS, "mono", 1.0);
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vp550_device - constructor
//-------------------------------------------------

vp550_device::vp550_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VP550, tag, owner, clock),
	device_vip_expansion_card_interface(mconfig, *this),
	m_pfg(*this, "u%u", 1U),
	m_sync_timer(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vp550_device::device_start()
{
	// allocate timers
	m_sync_timer = timer_alloc(FUNC(vp550_device::sync_tick), this);
	m_sync_timer->adjust(attotime::from_hz(50), 0, attotime::from_hz(50));
	m_sync_timer->enable(0);
}


//-------------------------------------------------
//  sync_timer - fire an interrupt on host
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(vp550_device::sync_tick)
{
	LOG("VP550 '%s' Interrupt\n", tag());

	m_slot->interrupt_w(ASSERT_LINE);
}


//-------------------------------------------------
//  vip_program_w - program write
//-------------------------------------------------

void vp550_device::vip_program_w(offs_t offset, uint8_t data, int cdef, int *minh)
{
	if (BIT(offset, 15))
	{
		*minh = 1;

		switch (offset & 0x03)
		{
		case 1: m_pfg[0]->str_w(data); break;
		case 2: m_pfg[1]->str_w(data); break;
		case 3: octave_w(data); break;
		}

		switch ((offset >> 4) & 0x03)
		{
		case 1: vlmna_w(data); break;
		case 2: vlmnb_w(data); break;
		case 3: sync_w(data); break;
		}
	}
}


//-------------------------------------------------
//  vip_sc_w - status code write
//-------------------------------------------------

void vp550_device::vip_sc_w(int n, int sc)
{
	if (BIT(sc, 1))
	{
		LOG("VP550 '%s' Clear Interrupt\n", tag());

		m_slot->interrupt_w(CLEAR_LINE);
	}
}


//-------------------------------------------------
//  vip_q_w - Q write
//-------------------------------------------------

void vp550_device::vip_q_w(int state)
{
	m_pfg[0]->oe_w(state);
	m_pfg[1]->oe_w(state);
}


//-------------------------------------------------
//  vip_run_w - RUN write
//-------------------------------------------------

void vp550_device::vip_run_w(int state)
{
	if (!state)
	{
		m_pfg[0]->reset();
		m_pfg[1]->reset();
	}
}


//-------------------------------------------------
//  octave_w - octave select write
//-------------------------------------------------

void vp550_device::octave_w(uint8_t data)
{
	int channel = (data >> 2) & 0x03;
	int clock2 = 0;

	if (BIT(data, 4))
	{
		clock2 = m_slot->clock() >> (3 - (data & 0x03));
	}

	m_pfg[BIT(data, 2)]->set_clk2(clock2);

	LOG("VP550 '%s' Clock %c: %u Hz\n", tag(), 'A' + channel, clock2);
}


//-------------------------------------------------
//  vlmna_w - channel A amplitude write
//-------------------------------------------------

void vp550_device::vlmna_w(uint8_t data)
{
	LOG("VP550 '%s' A Volume: %u\n", tag(), data & 0x0f);

	float gain = (data & 0x0f) / 15.0f;
	m_pfg[0]->set_output_gain(0, gain);
}


//-------------------------------------------------
//  vlmnb_w - channel B amplitude write
//-------------------------------------------------

void vp550_device::vlmnb_w(uint8_t data)
{
	LOG("VP550 '%s' B Volume: %u\n", tag(), data & 0x0f);

	float gain = (data & 0x0f) / 15.0f;
	m_pfg[1]->set_output_gain(0, gain);
}


//-------------------------------------------------
//  sync_w - interrupt enable write
//-------------------------------------------------

void vp550_device::sync_w(uint8_t data)
{
	LOG("VP550 '%s' Interrupt Enable: %u\n", tag(), BIT(data, 0));

	m_sync_timer->enable(BIT(data, 0));
}
