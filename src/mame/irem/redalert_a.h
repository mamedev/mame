// license:BSD-3-Clause
// copyright-holders:Mike Balfour
#ifndef MAME_IREM_REDALERT_A_H
#define MAME_IREM_REDALERT_A_H

#pragma once

#include "cpu/i8085/i8085.h"
#include "machine/6821pia.h"
#include "machine/gen_latch.h"
#include "sound/ay8910.h"


DECLARE_DEVICE_TYPE(IREM_M37B_AUDIO,        irem_m37b_audio_device)
DECLARE_DEVICE_TYPE(PANTHER_AUDIO,          panther_audio_device)
DECLARE_DEVICE_TYPE(IREM_M37B_UE17B_AUDIO,  irem_m37b_ue17b_audio_device)
DECLARE_DEVICE_TYPE(DEMONEYE_AUDIO,         demoneye_audio_device)


class irem_m37b_audio_device : public device_t
{
public:
	static constexpr feature_type imperfect_features() { return feature::SOUND; } // missing analog effects

	irem_m37b_audio_device(const machine_config &config, const char *tag, device_t *owner, uint32_t clock = 0U);

	int sound_status_r();

	void audio_command_w(uint8_t data);

protected:
	irem_m37b_audio_device(const machine_config &config, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	void analog_w(uint8_t data);
	void ay8910_w(uint8_t data);
	uint8_t ay8910_latch_1_r();
	void ay8910_latch_2_w(uint8_t data);

	void redalert_audio_map(address_map &map) ATTR_COLD;

	required_device<cpu_device>             m_audiocpu;
	required_device<ay8910_device>          m_ay8910;
	required_device<generic_latch_8_device> m_soundlatch;

private:
	TIMER_CALLBACK_MEMBER(audio_irq_on);
	TIMER_CALLBACK_MEMBER(audio_irq_off);

	emu_timer *m_audio_irq_on_timer = nullptr;
	emu_timer *m_audio_irq_off_timer = nullptr;

	uint8_t m_sound_hs = 0;
	uint8_t m_ay8910_latch_1 = 0;
	uint8_t m_ay8910_latch_2 = 0;
};


class panther_audio_device : public irem_m37b_audio_device
{
public:
	panther_audio_device(const machine_config &config, const char *tag, device_t *owner, uint32_t clock = 0U);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	void panther_audio_map(address_map &map) ATTR_COLD;
};


class irem_m37b_ue17b_audio_device : public irem_m37b_audio_device
{
public:
	irem_m37b_ue17b_audio_device(const machine_config &config, const char *tag, device_t *owner, uint32_t clock = 0U);

	void voice_command_w(uint8_t data);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	void redalert_voice_map(address_map &map) ATTR_COLD;

	required_device<i8085a_cpu_device>      m_voicecpu;
	required_device<generic_latch_8_device> m_soundlatch2;
};


class demoneye_audio_device : public device_t
{
public:
	static constexpr feature_type imperfect_features() { return feature::SOUND; }

	demoneye_audio_device(const machine_config &config, const char *tag, device_t *owner, uint32_t clock = 0U);

	void audio_command_w(uint8_t data);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	TIMER_CALLBACK_MEMBER(audio_irq_on);
	TIMER_CALLBACK_MEMBER(audio_irq_off);

	void ay8910_latch_1_w(uint8_t data);
	uint8_t ay8910_latch_2_r();
	void ay8910_data_w(uint8_t data);

	void demoneye_audio_map(address_map &map) ATTR_COLD;

	required_device<cpu_device>             m_audiocpu;
	required_device_array<ay8910_device, 2> m_ay;
	required_device<pia6821_device>         m_sndpia;
	required_device<generic_latch_8_device> m_soundlatch;

	emu_timer *m_audio_irq_on_timer = nullptr;
	emu_timer *m_audio_irq_off_timer = nullptr;

	uint8_t m_ay8910_latch_1 = 0;
	uint8_t m_ay8910_latch_2 = 0;
};

#endif // MAME_IREM_REDALERT_A_H
