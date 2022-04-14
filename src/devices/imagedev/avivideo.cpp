// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*********************************************************************

    avivideo.cpp

    Image device for AVI video.

*********************************************************************/

#include "emu.h"
#include "avivideo.h"
#include "aviio.h"

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// device type definition
DEFINE_DEVICE_TYPE(IMAGE_AVIVIDEO, avivideo_image_device, "avivideo_image", "AVI Video Image")

//-------------------------------------------------
//  avivideo_image_device - constructor
//-------------------------------------------------

avivideo_image_device::avivideo_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, IMAGE_AVIVIDEO, tag, owner, clock),
	device_image_interface(mconfig, *this),
	m_frame(nullptr),
	m_avi(nullptr),
	m_frame_timer(nullptr),
	m_frame_count(0),
	m_frame_num(0)
{
}

//-------------------------------------------------
//  avivideo_image_device - destructor
//-------------------------------------------------

avivideo_image_device::~avivideo_image_device()
{
	call_unload();
}


void avivideo_image_device::device_start()
{
	m_frame_timer = timer_alloc(TIMER_FRAME);
	m_frame_timer->adjust(attotime::never);

	save_item(NAME(m_frame_count));
	save_item(NAME(m_frame_num));
}

void avivideo_image_device::device_reset()
{
	m_frame_num = 0;
}

void avivideo_image_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	if (id == TIMER_FRAME)
	{
		if (m_avi != nullptr)
		{
			avi_file::error avierr = m_avi->read_uncompressed_video_frame(m_frame_num, *m_frame);
			if (avierr != avi_file::error::NONE)
			{
				m_frame_timer->adjust(attotime::never);
				return;
			}
			m_frame_num++;
			if (m_frame_num >= m_frame_count)
			{
				m_frame_num = 0;
			}
		}
		else
		{
			m_frame_timer->adjust(attotime::never);
		}
	}
}

image_init_result avivideo_image_device::call_load()
{
	m_frame = new bitmap_argb32;
	avi_file::error avierr = avi_file::open(filename(), m_avi);
	if (avierr != avi_file::error::NONE)
	{
		delete m_frame;
		m_frame = nullptr;
		return image_init_result::FAIL;
	}

	const avi_file::movie_info &aviinfo = m_avi->get_movie_info();
	float frame_rate = (float)aviinfo.video_timescale / (float)aviinfo.video_sampletime;
	attotime frame_time = attotime::from_hz((int)round(frame_rate));
	m_frame_timer->adjust(frame_time, 0, frame_time);
	m_frame_count = aviinfo.video_numsamples;
	m_frame_num = 0;
	return image_init_result::PASS;
}

void avivideo_image_device::call_unload()
{
	if (m_frame)
	{
		delete m_frame;
		m_frame = nullptr;
	}
	if (m_avi)
	{
		m_avi.release();
		m_avi = nullptr;
	}
}
