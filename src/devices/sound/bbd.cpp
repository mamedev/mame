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

bbd_device_base::bbd_device_base(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, device_type type) :
	device_t(mconfig, type, tag, owner, clock),
	device_sound_interface(mconfig, *this)
{
}

void bbd_device_base::set_bucket_count(int buckets)
{
	m_buffer.resize(buckets);
}

void bbd_device_base::device_start()
{
	m_stream = stream_alloc(1, 1, SAMPLE_RATE_OUTPUT_ADAPTIVE, STREAM_SYNCHRONOUS);
	save_item(NAME(m_buffer));
	save_item(NAME(m_curpos));


}

void bbd_device_base::device_reset()
{
	std::fill(m_buffer.begin(), m_buffer.end(), 0);
	m_curpos = 0;
}

void bbd_device_base::tick()
{
	u32 nextpos = m_curpos + 1;
	if(nextpos == m_buffer.size())
		nextpos = 0;
	m_buffer[nextpos] = m_buffer[m_curpos];
	m_curpos = nextpos;
}

void bbd_device_base::sound_stream_update(sound_stream &stream)
{
	u32 nextpos = m_curpos + 1;
	if(nextpos == m_buffer.size())
		nextpos = 0;
	stream.put(0, 0, m_buffer[nextpos]);
	m_buffer[m_curpos] = stream.get(0, 0);
}


//**************************************************************************
//  MN3004
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(MN3004, mn3004_device, "mn3004", "MN3004 BBD")

mn3004_device::mn3004_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	bbd_device_base(mconfig, tag, owner, clock, MN3004)
{
	set_bucket_count(512);
}


//**************************************************************************
//  MN3005
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(MN3005, mn3005_device, "mn3005", "MN3005 BBD")

mn3005_device::mn3005_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	bbd_device_base(mconfig, tag, owner, clock, MN3005)
{
	set_bucket_count(4096);
}


//**************************************************************************
//  MN3006
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(MN3006, mn3006_device, "mn3006", "MN3006 BBD")

mn3006_device::mn3006_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	bbd_device_base(mconfig, tag, owner, clock, MN3006)
{
	set_bucket_count(128);
}


//**************************************************************************
//  MN3204P
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(MN3204P, mn3204p_device, "mn3204p", "MN3204P BBD")

mn3204p_device::mn3204p_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	bbd_device_base(mconfig, tag, owner, clock, MN3204P)
{
	set_bucket_count(512);
}


//**************************************************************************
//  MN3207P
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(MN3207, mn3207_device, "mn3207", "MN3207 BBD")

mn3207_device::mn3207_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	bbd_device_base(mconfig, tag, owner, clock, MN3207)
{
	set_bucket_count(1024);
}
