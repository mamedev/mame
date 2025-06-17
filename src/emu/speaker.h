// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    speaker.h

    Speaker output sound device.
    Microphone input sound device.

    They have (x, y, z) coordinates in 3D space:
    * Observer is at position (0, 0, 0)
    * Positive x is to the right of the observer
    * Negative x is to the left of the observer
    * Positive y is above the observer
    * Negative y is below the observer
    * Positive z is in front of the observer
    * Negative z is behind the observer

***************************************************************************/

#ifndef MAME_EMU_SPEAKER_H
#define MAME_EMU_SPEAKER_H

#pragma once


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DECLARE_DEVICE_TYPE(SPEAKER, speaker_device)
DECLARE_DEVICE_TYPE(MICROPHONE, microphone_device)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class sound_io_device : public device_t, public device_sound_interface
{
public:

	virtual ~sound_io_device();

	// configuration helpers
	sound_io_device &set_position(u32 channel, double x, double y, double z);
	sound_io_device &set_position(u32 channel, const osd::channel_position &pos);
	sound_io_device &front_center(u32 channel = 0)        { return set_position(channel, osd::channel_position::FC); }
	sound_io_device &front_left(u32 channel = 0)          { return set_position(channel, osd::channel_position::FL); }
	sound_io_device &front_right(u32 channel = 0)         { return set_position(channel, osd::channel_position::FR); }
	sound_io_device &rear_center(u32 channel = 0)         { return set_position(channel, osd::channel_position::RC); }
	sound_io_device &rear_left(u32 channel = 0)           { return set_position(channel, osd::channel_position::RL); }
	sound_io_device &rear_right(u32 channel = 0)          { return set_position(channel, osd::channel_position::RR); }
	sound_io_device &headrest_center(u32 channel = 0)     { return set_position(channel, osd::channel_position::HC); }
	sound_io_device &headrest_left(u32 channel = 0)       { return set_position(channel, osd::channel_position::HL); }
	sound_io_device &headrest_right(u32 channel = 0)      { return set_position(channel, osd::channel_position::RC); }
	sound_io_device &backrest(u32 channel = 0)            { return set_position(channel, osd::channel_position::BACKREST); }
	sound_io_device &unknown(u32 channel = 0)             { return set_position(channel, osd::channel_position::UNKNOWN); }
	sound_io_device &map_on_request_only(u32 channel = 0) { return set_position(channel, osd::channel_position::ONREQ);   }
	sound_io_device &lfe(u32 channel = 0)                 { return set_position(channel, osd::channel_position::LFE);     }

	sound_io_device &front()                              { return front_left(0).front_right(1); }
	sound_io_device &rear()                               { return rear_left(0).rear_right(1); }
	sound_io_device &corners()                            { return front_left(0).front_right(1).rear_left(2).rear_right(3); }

	int channels() const { return m_positions.size(); }
	const osd::channel_position &get_position(u32 channel) const { return m_positions[channel]; }

	virtual bool is_output() const = 0;
	void set_id(int id) { m_id = id; }
	int get_id() const { return m_id; }

	sound_stream *stream() const { return m_stream; }

protected:
	// configuration state
	std::vector<osd::channel_position> m_positions;
	sound_stream *m_stream;
	int m_id;

	sound_io_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, double x, double y, double z)
		: sound_io_device(mconfig, type, tag, owner, 1)
	{
		set_position(0, x, y, z);
	}
	sound_io_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 channels); // Collides with clock, but not important
};

class speaker_device : public sound_io_device
{
public:
	// construction/destruction
	speaker_device(const machine_config &mconfig, const char *tag, device_t *owner, double x, double y, double z)
		: sound_io_device(mconfig, SPEAKER, tag, owner, x, y, z) {}
	speaker_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 channels = 1)
		: sound_io_device(mconfig, SPEAKER, tag, owner, channels) {}

	virtual ~speaker_device();

	virtual bool is_output() const override { return true; }

protected:

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	virtual void sound_stream_update(sound_stream &stream) override;
};

class microphone_device : public sound_io_device
{
public:
	// construction/destruction
	microphone_device(const machine_config &mconfig, const char *tag, device_t *owner, double x, double y, double z)
		: sound_io_device(mconfig, MICROPHONE, tag, owner, x, y, z) {}
	microphone_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 channels = 1)
		: sound_io_device(mconfig, MICROPHONE, tag, owner, channels) {}

	virtual ~microphone_device();

	virtual bool is_output() const override { return false; }

protected:

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	virtual void sound_stream_update(sound_stream &stream) override;
};

using speaker_device_enumerator = device_type_enumerator<speaker_device>;
using microphone_device_enumerator = device_type_enumerator<microphone_device>;


#endif // MAME_EMU_SPEAKER_H
