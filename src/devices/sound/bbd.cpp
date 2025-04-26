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
	m_curpos(0),
	m_cv_handler(*this),
	m_next_bbdtime(attotime::zero)
{
	std::fill_n(&m_buffer[0], Entries, 0);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

template<int Entries, int Outputs>
void bbd_device_base<Entries, Outputs>::device_start()
{
	m_cv_handler.resolve();

	if (m_cv_handler.isnull())
		m_stream = stream_alloc(1, Outputs, sample_rate());
	else
		m_stream = stream_alloc(1, Outputs, SAMPLE_RATE_OUTPUT_ADAPTIVE, STREAM_DISABLE_INPUT_RESAMPLING);

	save_item(NAME(m_buffer));
	save_item(NAME(m_curpos));
	save_item(NAME(m_next_bbdtime));
}


//-------------------------------------------------
//  device_clock_changed - handle a clock change
//-------------------------------------------------

template<int Entries, int Outputs>
void bbd_device_base<Entries, Outputs>::device_clock_changed()
{
	if (m_cv_handler.isnull())
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
	if (m_cv_handler.isnull())
	{
		for (int sampindex = 0; sampindex < outputs[0].samples(); sampindex++)
		{
			for (int outnum = 0; outnum < Outputs; outnum++)
				outputs[outnum].put(sampindex, m_buffer[(m_curpos + (Outputs - 1) - outnum) % Entries]);
			m_buffer[m_curpos] = inputs[0].get(sampindex);
			m_curpos = (m_curpos + 1) % Entries;
		}
	}
	else
	{
		read_stream_view in(inputs[0], m_next_bbdtime);
		attotime intime = in.start_time();
		attotime outtime = outputs[0].start_time();
		int inpos = 0;

		// loop until all outputs generated
		for (int sampindex = 0; sampindex < outputs[0].samples(); sampindex++)
		{
			// we need to process some more BBD input
			while (outtime >= m_next_bbdtime)
			{
				// find the input sample that overlaps our start time
				while (intime + in.sample_period() < m_next_bbdtime)
				{
					inpos++;
					intime += in.sample_period();
				}

				// copy that to the buffer
				m_buffer[m_curpos] = in.get(inpos);
				m_curpos = (m_curpos + 1) % std::size(m_buffer);

				// advance the end time of this BBD sample
				m_next_bbdtime += attotime(0, m_cv_handler(m_next_bbdtime));
			}

			// copy the most recently-generated BBD data
			for (int outnum = 0; outnum < Outputs; outnum++)
				outputs[outnum].put(sampindex, outputval(outnum - (Outputs - 1)));
			outtime += outputs[0].sample_period();
		}
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


//**************************************************************************
//  MN3207P
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(MN3207, mn3207_device, "mn3207", "MN3207 BBD")

mn3207_device::mn3207_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	bbd_device_base(mconfig, tag, owner, clock, MN3207)
{
}
