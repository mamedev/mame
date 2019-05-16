// license:BSD-3-Clause
// copyright-holders:Mike Harris
/***************************************************************************

    bally.cpp

    Functions to emulate the various Bally pinball sound boards.

***************************************************************************/

#include "emu.h"
#include "audio/bally.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(BALLY_AS3022,      bally_as3022_device,      "as3022",      "Bally AS3022 Sound Board")
DEFINE_DEVICE_TYPE(BALLY_SOUNDS_PLUS, bally_sounds_plus_device, "sounds_plus", "Bally Sounds Plus w/ Vocalizer Board")


//**************************************************************************
//  AS3022
//**************************************************************************

//**************************************************************************
//  IO ports
//**************************************************************************
static INPUT_PORTS_START(as3022)
        PORT_START("SW1")
        PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("SW1") PORT_CHANGED_MEMBER(DEVICE_SELF, bally_as3022_device, sw1, 0)
INPUT_PORTS_END

ioport_constructor bally_as3022_device::device_input_ports() const
{
        return INPUT_PORTS_NAME(as3022);
}

INPUT_CHANGED_MEMBER(bally_as3022_device::sw1)
{
        if (newval != oldval)
                m_cpu->set_input_line(INPUT_LINE_NMI, (newval ? ASSERT_LINE : CLEAR_LINE));
}

//-------------------------------------------------
//  sound_select - handle an external write to the board
//-------------------------------------------------

WRITE8_MEMBER(bally_as3022_device::sound_select)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(bally_as3022_device::sound_select_sync), this), data);
}


TIMER_CALLBACK_MEMBER(bally_as3022_device::sound_select_sync)
{
	m_sound_select = param;
}

//-------------------------------------------------
//
//  sound_int - handle an external sound interrupt to the board
//-------------------------------------------------

WRITE_LINE_MEMBER(bally_as3022_device::sound_int)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(bally_as3022_device::sound_int_sync), this), state);
}

TIMER_CALLBACK_MEMBER(bally_as3022_device::sound_int_sync)
{
	m_pia->ca1_w(param);
}


//-------------------------------------------------
//
//  irq_w - IRQ line state changes
//-------------------------------------------------

WRITE_LINE_MEMBER(bally_as3022_device::irq_w)
{
	int combined_state = m_pia->irq_a_state() | m_pia->irq_b_state();
	m_cpu->set_input_line(M6802_IRQ_LINE, combined_state ? ASSERT_LINE : CLEAR_LINE);
}


//-------------------------------------------------
//  CPU map, from schematics
//-------------------------------------------------

void bally_as3022_device::as3022_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0x1fff);  // A13-15 are unconnected
	map(0x0000, 0x007f).mirror(0x0f00).ram();
	map(0x0080, 0x0083).mirror(0x0f7c).rw("pia", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x1000, 0x1fff).rom();  // 4k RAM space, but could be jumpered for 2k
}


//-------------------------------------------------
// device_add_mconfig - add device configuration
//-------------------------------------------------

void bally_as3022_device::device_add_mconfig(machine_config &config)
{
	M6808(config, m_cpu, DERIVED_CLOCK(1, 1));
	m_cpu->set_addrmap(AS_PROGRAM, &bally_as3022_device::as3022_map);

	PIA6821(config, m_pia, 0);
	m_pia->readpa_handler().set(FUNC(bally_as3022_device::pia_porta_r));
	m_pia->writepa_handler().set(FUNC(bally_as3022_device::pia_porta_w));
	m_pia->writepb_handler().set(FUNC(bally_as3022_device::pia_portb_w));
	m_pia->cb2_handler().set(FUNC(bally_as3022_device::pia_cb2_w));
	m_pia->irqa_handler().set(FUNC(bally_as3022_device::irq_w));
	m_pia->irqb_handler().set(FUNC(bally_as3022_device::irq_w));

	AY8910(config, m_ay, DERIVED_CLOCK(1, 4));
	m_ay->port_a_read_callback().set(FUNC(bally_as3022_device::ay_io_r));
	m_ay->add_route(ALL_OUTPUTS, *this, 0.33, AUTO_ALLOC_INPUT, 0);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bally_as3022_device::device_start()
{
	// Set volumes to a sane default.
	m_ay->set_volume(0, 0);
	m_ay->set_volume(1, 0);
	m_ay->set_volume(2, 0);

	save_item(NAME(m_bc1));
        save_item(NAME(m_bdir));
        save_item(NAME(m_sound_select));
        save_item(NAME(m_ay_data));
}


//-------------------------------------------------
//  pia_porta_r - PIA port A reads
//-------------------------------------------------

READ8_MEMBER(bally_as3022_device::pia_porta_r)
{
	if (m_bc1 && !m_bdir)
	{
		m_ay_data = m_ay->data_r();
		return m_ay_data;
	}
	else
	{
		// Nothing is active on the bus, so return open bus.
		return 0xff;
	}
}


//-------------------------------------------------
//  pia_porta_w - PIA port A writes
//-------------------------------------------------

WRITE8_MEMBER(bally_as3022_device::pia_porta_w)
{
	if (m_bc1 && !m_bdir)
	{
		logerror("PIA port A bus contention!\n");
	}
	m_ay_data = data;
	update_sound_selects();
}


//-------------------------------------------------
//  pia_portb_w - PIA port B writes
//-------------------------------------------------

WRITE8_MEMBER(bally_as3022_device::pia_portb_w)
{
	m_bc1 = BIT(data, 0);
	m_bdir = BIT(data, 1);
	if (m_bc1 && !m_bdir)
	{
		m_ay_data = m_ay->data_r();
	}
	update_sound_selects();
}


//-------------------------------------------------
//  pia_cb2_w - PIA CB2 writes
//-------------------------------------------------
WRITE_LINE_MEMBER(bally_as3022_device::pia_cb2_w)
{
	// This pin is hooked up to the amp, and disables sounds when hi
	if (state)
	{
		m_ay->set_volume(0, 0);
		m_ay->set_volume(1, 0);
		m_ay->set_volume(2, 0);
	}
	else
	{
		m_ay->set_volume(0, 0xff);
		m_ay->set_volume(1, 0xff);
		m_ay->set_volume(2, 0xff);
	}
}


//-------------------------------------------------
//  ay_io_r - AY8912 IO A reads (B is unconnected)
//-------------------------------------------------

READ8_MEMBER(bally_as3022_device::ay_io_r)
{
	// The two high bits are unconnected, the others are inverted.
	return ~m_sound_select & 0x3f;
}


void bally_as3022_device::update_sound_selects()
{
	if (m_bc1 && m_bdir)
	{
		m_ay->address_w(m_ay_data);
	}
	else if (!m_bc1 && m_bdir)
	{
		m_ay->data_w(m_ay_data);
	}
}

//**************************************************************************
//  SOUNDS PLUS WITH VOCALIZER
//**************************************************************************

//-------------------------------------------------
// device_add_mconfig - add device configuration
//-------------------------------------------------

void bally_sounds_plus_device::device_add_mconfig(machine_config &config)
{
	bally_as3022_device::device_add_mconfig(config);

	m_cpu->set_addrmap(AS_PROGRAM, &bally_sounds_plus_device::sounds_plus_map);
	m_pia->writepb_handler().set(FUNC(bally_sounds_plus_device::vocalizer_pia_portb_w));

	MC3417(config, m_mc3417, 0);
	// A gain of 2.2 is a guess. It sounds about loud enough and doesn't clip.
	m_mc3417->add_route(ALL_OUTPUTS, *this, 2.2, AUTO_ALLOC_INPUT, 0);
}

//-------------------------------------------------
//  CPU map, from schematics
//-------------------------------------------------

void bally_sounds_plus_device::sounds_plus_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x007f).mirror(0x7f00).ram();
	map(0x0080, 0x0083).mirror(0x7f7c).rw("pia", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x8000, 0xffff).rom();
}

//-------------------------------------------------
//  pia_portb_w - PIA port B writes
//-------------------------------------------------

WRITE8_MEMBER(bally_sounds_plus_device::vocalizer_pia_portb_w)
{
	bool speech_clock = BIT(data, 6);
	bool speech_data = BIT(data, 7);
	m_mc3417->clock_w(speech_clock ? 1 : 0);
	m_mc3417->digit_w(speech_data ? 1 : 0);
	pia_portb_w(space, offset, data);
}
