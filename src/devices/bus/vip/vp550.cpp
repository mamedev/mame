// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    RCA VIP Super Sound System VP550 emulation

**********************************************************************/

/*

    TODO:

    - VP551

*/

#include "vp550.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0


#define CDP1863_A_TAG   "u1"
#define CDP1863_B_TAG   "u2"


enum
{
	CHANNEL_A = 0,
	CHANNEL_B
};


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type VP550 = &device_creator<vp550_device>;


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( vp550 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( vp550 )
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_CDP1863_ADD(CDP1863_A_TAG, 0, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	MCFG_CDP1863_ADD(CDP1863_B_TAG, 0, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor vp550_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( vp550 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vp550_device - constructor
//-------------------------------------------------

vp550_device::vp550_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, VP550, "VP550", tag, owner, clock, "vp550", __FILE__),
	device_vip_expansion_card_interface(mconfig, *this),
	m_pfg_a(*this, CDP1863_A_TAG),
	m_pfg_b(*this, CDP1863_B_TAG), m_sync_timer(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vp550_device::device_start()
{
	// allocate timers
	m_sync_timer = timer_alloc();
	m_sync_timer->adjust(attotime::from_hz(50), 0, attotime::from_hz(50));
	m_sync_timer->enable(0);
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void vp550_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (LOG) logerror("VP550 '%s' Interrupt\n", tag().c_str());

	m_slot->interrupt_w(ASSERT_LINE);
}


//-------------------------------------------------
//  vip_program_w - program write
//-------------------------------------------------

void vp550_device::vip_program_w(address_space &space, offs_t offset, UINT8 data, int cdef, int *minh)
{
	if (BIT(offset, 15))
	{
		*minh = 1;

		switch (offset & 0x03)
		{
		case 1: m_pfg_a->str_w(data); break;
		case 2: m_pfg_b->str_w(data); break;
		case 3: octave_w(space, offset, data); break;
		}

		switch ((offset >> 4) & 0x03)
		{
		case 1: vlmna_w(space, offset, data); break;
		case 2: vlmnb_w(space, offset, data); break;
		case 3: sync_w(space, offset, data); break;
		}
	}
}


//-------------------------------------------------
//  vip_sc_w - status code write
//-------------------------------------------------

void vp550_device::vip_sc_w(int data)
{
	if (BIT(data, 1))
	{
		if (LOG) logerror("VP550 '%s' Clear Interrupt\n", tag().c_str());

		m_slot->interrupt_w(CLEAR_LINE);
	}
}


//-------------------------------------------------
//  vip_q_w - Q write
//-------------------------------------------------

void vp550_device::vip_q_w(int state)
{
	m_pfg_a->oe_w(state);
	m_pfg_b->oe_w(state);
}


//-------------------------------------------------
//  vip_run_w - RUN write
//-------------------------------------------------

void vp550_device::vip_run_w(int state)
{
	if (!state)
	{
		m_pfg_a->reset();
		m_pfg_b->reset();
	}
}


//-------------------------------------------------
//  octave_w - octave select write
//-------------------------------------------------

WRITE8_MEMBER( vp550_device::octave_w )
{
	int channel = (data >> 2) & 0x03;
	int clock2 = 0;

	if (data & 0x10)
	{
		switch (data & 0x03)
		{
		case 0: clock2 = m_slot->clock() / 8; break;
		case 1: clock2 = m_slot->clock() / 4; break;
		case 2: clock2 = m_slot->clock() / 2; break;
		case 3: clock2 = m_slot->clock();     break;
		}
	}

	switch (channel)
	{
	case CHANNEL_A: m_pfg_a->set_clk2(clock2); break;
	case CHANNEL_B: m_pfg_b->set_clk2(clock2); break;
	}

	if (LOG) logerror("VP550 '%s' Clock %c: %u Hz\n", tag().c_str(), 'A' + channel, clock2);
}


//-------------------------------------------------
//  vlmna_w - channel A amplitude write
//-------------------------------------------------

WRITE8_MEMBER( vp550_device::vlmna_w )
{
	if (LOG) logerror("VP550 '%s' A Volume: %u\n", tag().c_str(), data & 0x0f);

	float gain = (data & 0x0f) * 0.0666;

	m_pfg_a->set_output_gain(0, gain);
}


//-------------------------------------------------
//  vlmnb_w - channel B amplitude write
//-------------------------------------------------

WRITE8_MEMBER( vp550_device::vlmnb_w )
{
	if (LOG) logerror("VP550 '%s' B Volume: %u\n", tag().c_str(), data & 0x0f);

	float gain = (data & 0x0f) * 0.0666;

	m_pfg_b->set_output_gain(0, gain);
}


//-------------------------------------------------
//  sync_w - interrupt enable write
//-------------------------------------------------

WRITE8_MEMBER( vp550_device::sync_w )
{
	if (LOG) logerror("VP550 '%s' Interrupt Enable: %u\n", tag().c_str(), BIT(data, 0));

	m_sync_timer->enable(BIT(data, 0));
}
