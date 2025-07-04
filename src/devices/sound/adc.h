// license:BSD-3-Clause
// copyright-holders: Olivier Galibert

// ADCs, unsigned or signed two-complement

#ifndef MAME_SOUND_ADC_H
#define MAME_SOUND_ADC_H

#pragma once

class zn449_device : public device_t, public device_sound_interface
{
public:
	zn449_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	u8 read();

protected:
	sound_stream *m_stream;
	u8 m_current_value;

	virtual void device_start() override ATTR_COLD;
	virtual void sound_stream_update(sound_stream &stream) override;
};

class adc10_device : public device_t, public device_sound_interface
{
public:
	adc10_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	u16 read();

protected:
	sound_stream *m_stream;
	u16 m_current_value;

	virtual void device_start() override ATTR_COLD;
	virtual void sound_stream_update(sound_stream &stream) override;
};

DECLARE_DEVICE_TYPE(ZN449, zn449_device);
DECLARE_DEVICE_TYPE(ADC10, adc10_device);

#endif // MAME_SOUND_ADC_H
