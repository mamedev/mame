// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Cinemat/Leland driver

*************************************************************************/
#ifndef MAME_AUDIO_LELAND_H
#define MAME_AUDIO_LELAND_H

#pragma once

#include "cpu/i86/i186.h"
#include "machine/pit8253.h"
#include "sound/dac.h"
#include "sound/ym2151.h"


class leland_80186_sound_device : public device_t
{
public:
	leland_80186_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_WRITE16_MEMBER(peripheral_ctrl);
	DECLARE_WRITE8_MEMBER(leland_80186_control_w);
	DECLARE_WRITE8_MEMBER(ataxx_80186_control_w);
	DECLARE_READ16_MEMBER(peripheral_r);
	DECLARE_WRITE16_MEMBER(peripheral_w);
	DECLARE_WRITE8_MEMBER(leland_80186_command_lo_w);
	DECLARE_WRITE8_MEMBER(leland_80186_command_hi_w);
	DECLARE_READ8_MEMBER(leland_80186_response_r);
	DECLARE_WRITE16_MEMBER(dac_w);
	DECLARE_WRITE16_MEMBER(ataxx_dac_control);
	DECLARE_WRITE_LINE_MEMBER(i80186_tmr0_w);
	DECLARE_WRITE_LINE_MEMBER(i80186_tmr1_w);

	DECLARE_WRITE_LINE_MEMBER(pit0_2_w);
	DECLARE_WRITE_LINE_MEMBER(pit1_0_w);
	DECLARE_WRITE_LINE_MEMBER(pit1_1_w);
	DECLARE_WRITE_LINE_MEMBER(pit1_2_w);

protected:
	leland_80186_sound_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
	int m_type;

	enum {
		TYPE_LELAND,
		TYPE_REDLINE,
		TYPE_ATAXX,
		TYPE_WSF
	};

	required_device<dac_byte_interface> m_dac1;
	required_device<dac_byte_interface> m_dac2;
	required_device<dac_byte_interface> m_dac3;
	required_device<dac_byte_interface> m_dac4;
	optional_device<dac_byte_interface> m_dac5;
	optional_device<dac_byte_interface> m_dac6;
	optional_device<dac_byte_interface> m_dac7;
	optional_device<dac_byte_interface> m_dac8;
	optional_device<dac_word_interface> m_dac9;
	required_device<dac_byte_interface> m_dac1vol;
	required_device<dac_byte_interface> m_dac2vol;
	required_device<dac_byte_interface> m_dac3vol;
	required_device<dac_byte_interface> m_dac4vol;
	optional_device<dac_byte_interface> m_dac5vol;
	optional_device<dac_byte_interface> m_dac6vol;
	optional_device<dac_byte_interface> m_dac7vol;
	optional_device<dac_byte_interface> m_dac8vol;

private:
	void command_lo_sync(void *ptr, int param);
	void delayed_response_r(void *ptr, int param);
	void set_clock_line(int which, int state) { m_clock_active = state ? (m_clock_active | (1<<which)) : (m_clock_active & ~(1<<which)); }

	// internal state
	i80186_cpu_device *m_audiocpu;
	uint16_t m_peripheral;
	uint8_t m_last_control;
	uint8_t m_clock_active;
	uint8_t m_clock_tick;
	uint16_t m_sound_command;
	uint16_t m_sound_response;
	uint32_t m_ext_start;
	uint32_t m_ext_stop;
	uint8_t m_ext_active;
	uint8_t* m_ext_base;

	required_device<pit8254_device> m_pit0;
	optional_device<pit8254_device> m_pit1;
	optional_device<pit8254_device> m_pit2;
	optional_device<ym2151_device> m_ymsnd;
};


class redline_80186_sound_device : public leland_80186_sound_device
{
public:
	redline_80186_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	DECLARE_WRITE16_MEMBER(redline_dac_w);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
};


class ataxx_80186_sound_device : public leland_80186_sound_device
{
public:
	ataxx_80186_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
};


class wsf_80186_sound_device : public leland_80186_sound_device
{
public:
	wsf_80186_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
};


DECLARE_DEVICE_TYPE(LELAND_80186, leland_80186_sound_device)
DECLARE_DEVICE_TYPE(REDLINE_80186, redline_80186_sound_device)
DECLARE_DEVICE_TYPE(ATAXX_80186, ataxx_80186_sound_device)
DECLARE_DEVICE_TYPE(WSF_80186, wsf_80186_sound_device)

#endif // MAME_AUDIO_LELAND_H
