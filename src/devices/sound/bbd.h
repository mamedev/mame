// license:BSD-3-Clause
// copyright-holders:Aaron Giles
#ifndef MAME_SOUND_BBD_H
#define MAME_SOUND_BBD_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bbd_device_base

class bbd_device_base : public device_t, public device_sound_interface
{
public:
	void tick();

protected:
	// internal constructor
	bbd_device_base(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, device_type type);

	void set_bucket_count(int buckets);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream) override;

protected:
	// override to convert clock to sample rate
	sound_stream::sample_t outputval(s32 index) const { return m_buffer[(m_curpos - index) % std::size(m_buffer)]; }
	virtual u32 sample_rate() const { return clock(); }

	sound_stream *          m_stream;
	u32                     m_curpos;
	std::vector<sound_stream::sample_t> m_buffer;
};


// ======================> mn3004_device

class mn3004_device : public bbd_device_base
{
public:
	mn3004_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

DECLARE_DEVICE_TYPE(MN3004, mn3004_device)


// ======================> mn3005_device

class mn3005_device : public bbd_device_base
{
public:
	mn3005_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

DECLARE_DEVICE_TYPE(MN3005, mn3005_device)


// ======================> mn3006_device

class mn3006_device : public bbd_device_base
{
public:
	mn3006_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

DECLARE_DEVICE_TYPE(MN3006, mn3006_device)


// ======================> mn3204p_device

class mn3204p_device : public bbd_device_base
{
public:
	mn3204p_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

DECLARE_DEVICE_TYPE(MN3204P, mn3204p_device)


// ======================> mn3207_device

class mn3207_device : public bbd_device_base
{
public:
	mn3207_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

DECLARE_DEVICE_TYPE(MN3207, mn3207_device)

#endif // MAME_SOUND_BBD_H
