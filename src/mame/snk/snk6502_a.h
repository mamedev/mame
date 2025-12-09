// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Dan Boris
/*************************************************************************

    rokola hardware

*************************************************************************/
#ifndef MAME_SNK_SNK6502_A_H
#define MAME_SNK_SNK6502_A_H

#pragma once

#include "sound/discrete.h"
#include "sound/samples.h"
#include "sound/sn76477.h"

class snk6502_sound_device : public device_t, public device_sound_interface
{
public:
	snk6502_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	int music0_playing();

	void set_music_freq(int freq);
	void set_music_clock(double clock_time);
	void set_channel_base(int channel, int base, int mask = 0xff);
	void mute_channel(int channel);
	void unmute_channel(int channel);
	void set_sound0_stop_on_rollover(int value) { m_sound0_stop_on_rollover = value; }
	void reset_offset(int channel) { m_tone_channels[channel].offset = 0; }

	void speech_w(uint8_t data, const uint16_t *table, int start);

	void build_waveform(int channel, int mask);
	void sasuke_build_waveform(int mask);
	void satansat_build_waveform(int mask);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream) override;

private:
	static constexpr unsigned NUM_CHANNELS = 3;

	struct tone_t
	{
		int mute;
		int offset;
		int base;
		int mask;
		int32_t   sample_rate;
		int32_t   sample_step;
		int32_t   sample_cur;
		int16_t   form[16];
	};

	// internal state
	tone_t m_tone_channels[NUM_CHANNELS];
	int32_t m_tone_clock_expire;
	int32_t m_tone_clock;
	sound_stream * m_tone_stream;

	optional_device<samples_device> m_samples;
	required_memory_region m_rom;
	int m_sound0_stop_on_rollover;

	int m_hd38880_cmd;
	uint32_t m_hd38880_addr;
	int m_hd38880_data_bytes;
	double m_hd38880_speed;

	void validate_tone_channel(int channel);
};


class vanguard_sound_device : public device_t
{
public:
	vanguard_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void sound_w(offs_t offset, uint8_t data);
	void speech_w(uint8_t data);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	required_device<snk6502_sound_device> m_custom;
	required_device<sn76477_device> m_sn76477_2;
	required_device<samples_device> m_samples;

	uint8_t m_last_port1;
};


class fantasy_sound_device : public device_t
{
public:
	fantasy_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void sound_w(offs_t offset, uint8_t data);
	void speech_w(uint8_t data);

protected:
	fantasy_sound_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	required_device<snk6502_sound_device> m_custom;

private:
	required_device<discrete_sound_device> m_discrete;

	uint8_t m_last_port1;
};


class nibbler_sound_device : public fantasy_sound_device
{
public:
	nibbler_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};


class pballoon_sound_device : public fantasy_sound_device
{
public:
	pballoon_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
};


class sasuke_sound_device : public device_t
{
public:
	sasuke_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void sound_w(offs_t offset, uint8_t data);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	required_device<snk6502_sound_device> m_custom;
	required_device<samples_device> m_samples;

	uint8_t m_last_port1;
};


class satansat_sound_device : public device_t
{
public:
	satansat_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void sound_w(offs_t offset, uint8_t data);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	required_device<snk6502_sound_device> m_custom;
	required_device<samples_device> m_samples;

	uint8_t m_last_port1;
};


DECLARE_DEVICE_TYPE(SNK6502_SOUND,  snk6502_sound_device)

DECLARE_DEVICE_TYPE(VANGUARD_SOUND, vanguard_sound_device)
DECLARE_DEVICE_TYPE(FANTASY_SOUND,  fantasy_sound_device)
DECLARE_DEVICE_TYPE(NIBBLER_SOUND,  nibbler_sound_device)
DECLARE_DEVICE_TYPE(PBALLOON_SOUND, pballoon_sound_device)
DECLARE_DEVICE_TYPE(SASUKE_SOUND,   sasuke_sound_device)
DECLARE_DEVICE_TYPE(SATANSAT_SOUND, satansat_sound_device)

#endif // MAME_SNK_SNK6502_A_H
