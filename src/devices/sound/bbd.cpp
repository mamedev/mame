// license:BSD-3-Clause
// copyright-holders:Aaron Giles
#include "emu.h"
#include "bbd.h"


//**************************************************************************
//  BBD DEVICE BASE
//**************************************************************************

//-------------------------------------------------
//  bbd_device_base - constructor
//-------------------------------------------------

template<int Entries, int Outputs>
bbd_device_base<Entries, Outputs>::bbd_device_base(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, device_type type) :
	device_t(mconfig, type, tag, owner, clock),
	device_sound_interface(mconfig, *this),
	m_stream(nullptr),
	m_curpos(0)
{
	std::fill_n(&m_buffer[0], Entries, 0);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

template<int Entries, int Outputs>
void bbd_device_base<Entries, Outputs>::device_start()
{
	m_stream = stream_alloc(1, Outputs, sample_rate());
	save_item(NAME(m_buffer));
}


//-------------------------------------------------
//  device_clock_changed - handle a clock change
//-------------------------------------------------

template<int Entries, int Outputs>
void bbd_device_base<Entries, Outputs>::device_clock_changed()
{
	m_stream->set_sample_rate(sample_rate());
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

template<int Entries, int Outputs>
void bbd_device_base<Entries, Outputs>::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
{
	// BBDs that I've seen so far typically have 2 outputs, with the first outputting
	// sample n-1 and the second outputting sampe n; if chips with more outputs
	// or other taps turn up, this logic will need to be made more flexible
	for (int sampindex = 0; sampindex < outputs[0].samples(); sampindex++)
	{
		for (int outnum = 0; outnum < Outputs; outnum++)
			outputs[outnum].put(sampindex, m_buffer[(m_curpos + (Outputs - 1) - outnum) % Entries]);
		m_buffer[m_curpos] = inputs[0].get(sampindex);
		m_curpos = (m_curpos + 1) % Entries;
	}
}


//**************************************************************************
//  MN3004
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(MN3004, mn3004_device, "mn3004", "MN3004 BBD")

mn3004_device::mn3004_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	bbd_device_base(mconfig, tag, owner, clock, MN3004)
{
}


//**************************************************************************
//  MN3005
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(MN3005, mn3005_device, "mn3005", "MN3005 BBD")

mn3005_device::mn3005_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	bbd_device_base(mconfig, tag, owner, clock, MN3005)
{
}


//**************************************************************************
//  MN3006
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(MN3006, mn3006_device, "mn3006", "MN3006 BBD")

mn3006_device::mn3006_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	bbd_device_base(mconfig, tag, owner, clock, MN3006)
{
}


//**************************************************************************
//  MN3204P
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(MN3204P, mn3204p_device, "mn3204p", "MN3204P BBD")

mn3204p_device::mn3204p_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	bbd_device_base(mconfig, tag, owner, clock, MN3204P)
{
}
