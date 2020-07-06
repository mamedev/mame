// license:BSD-3-Clause
// copyright-holders:Aaron Giles
#ifndef MAME_AUDIO_CINEMAT_H
#define MAME_AUDIO_CINEMAT_H

#pragma once

#include "machine/74259.h"
#include "machine/netlist.h"
#include "netlist/nl_setup.h"
#include "sound/samples.h"

// log to cinemat.csv for nltool playback/analysis
#define ENABLE_NETLIST_LOGGING		(1)


class cinemat_audio_device : public device_t
{
public:
	cinemat_audio_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, u8 inputs_mask);

	void configure_latch_inputs(ls259_device &latch, u8 mask);

protected:
	virtual void device_start() override;
	virtual void device_stop() override;

	template<int _Index> DECLARE_WRITE_LINE_MEMBER(sound_w) { input_set(_Index, state); }

	// only used for sample hacks
	u64 framenum() { return machine().scheduler().time().as_ticks(38); }

	bool rising_edge(u16 newval, u16 oldval, int bit) const { u16 mask = 1 << bit; return ((oldval & mask) == 0 && (newval & mask) != 0); }
	bool falling_edge(u16 newval, u16 oldval, int bit) const { u16 mask = 1 << bit; return ((oldval & mask) != 0 && (newval & mask) == 0); }

	void shiftreg_clock(int bit) { m_shiftreg_accum = (m_shiftreg_accum >> 1) | (bit << 7); }
	void shiftreg16_clock(int bit) { m_shiftreg16_accum = (m_shiftreg16_accum >> 1) | (bit << 15); }

	void input_set(int bit, int state);
	void shiftreg_latch();
	void shiftreg16_latch();
	void shiftreg_set(int bit, int val);

	virtual void inputs_changed(u8 newvals, u8 oldvals);
	virtual void shiftreg_changed(u8 newvals, u8 oldvals);
	virtual void shiftreg16_changed(u16 newvals, u16 oldvals);

	virtual u8 shiftreg_swizzle(u8 rawvals);

	u16 shiftreg16_accum() const { return m_shiftreg16_accum; }

	optional_device<samples_device> m_samples;
	optional_device_array<netlist_mame_logic_input_device, 8> m_out_input;
	optional_device_array<netlist_mame_logic_input_device, 8> m_shiftreg_input;
	optional_device_array<netlist_mame_logic_input_device, 16> m_shiftreg16_input;

private:
	template<typename _Type>
	void log_changes(_Type newvals, _Type oldvals, const char *name_prefix, _Type mask = _Type(~0))
	{
	#if ENABLE_NETLIST_LOGGING
		if (m_logfile != nullptr)
		{
			attotime time = machine().scheduler().time();
			for (int bit = 0; bit < 8 * sizeof(_Type); bit++)
				if (((mask >> bit) & 1) != 0)
					if ((((newvals ^ oldvals) >> bit) & 1) != 0)
						fprintf(m_logfile, "%s,%s_%u.IN,%d\n", time.as_string(), name_prefix, bit, (newvals >> bit) & 1);
		}
	#endif
	}

	u8 m_inputs = 0xff;
	u8 m_inputs_mask = 0xff;
	u8 m_shiftreg = 0xff;
	u8 m_shiftreg_accum = 0xff;
	u16 m_shiftreg16 = 0xffff;
	u16 m_shiftreg16_accum = 0xffff;

#if ENABLE_NETLIST_LOGGING
	FILE *m_logfile = nullptr;
#endif

};


class spacewar_audio_device : public cinemat_audio_device
{
public:
	spacewar_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
};


class barrier_audio_device : public cinemat_audio_device
{
public:
	barrier_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
};


class speedfrk_audio_device : public cinemat_audio_device
{
public:
	speedfrk_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
};


class starhawk_audio_device : public cinemat_audio_device
{
public:
	starhawk_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
};


class sundance_audio_device : public cinemat_audio_device
{
public:
	sundance_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
};


class tailg_audio_device : public cinemat_audio_device
{
public:
	tailg_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
};


class warrior_audio_device : public cinemat_audio_device
{
public:
	warrior_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override;

	virtual void inputs_changed(u8 newvals, u8 oldvals) override;
};


class armora_audio_device : public cinemat_audio_device
{
public:
	armora_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override;

	virtual void inputs_changed(u8 newvals, u8 oldvals) override;
	virtual void shiftreg_changed(u8 newvals, u8 oldvals) override;
};


class ripoff_audio_device : public cinemat_audio_device
{
public:
	ripoff_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override;

	virtual void inputs_changed(u8 newvals, u8 oldvals) override;
	virtual void shiftreg_changed(u8 newvals, u8 oldvals) override;
};


class starcas_audio_device : public cinemat_audio_device
{
public:
	starcas_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
};


class solarq_audio_device : public cinemat_audio_device
{
public:
	solarq_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;

	virtual void inputs_changed(u8 newvals, u8 oldvals) override;
	virtual void shiftreg_changed(u8 newvals, u8 oldvals) override;
	virtual void shiftreg16_changed(u16 newvals, u16 oldvals) override;

private:
	u64 m_last_frame = 0;
	float m_current_volume = 0;
	float m_target_volume = 0;
};


class boxingb_audio_device : public cinemat_audio_device
{
public:
	boxingb_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override;

	virtual void inputs_changed(u8 newvals, u8 oldvals) override;
	virtual void shiftreg_changed(u8 newvals, u8 oldvals) override;
	virtual void shiftreg16_changed(u16 newvals, u16 oldvals) override;
};


class wotw_audio_device : public cinemat_audio_device
{
public:
	wotw_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
};


DECLARE_DEVICE_TYPE(SPACE_WARS_AUDIO, spacewar_audio_device)
DECLARE_DEVICE_TYPE(BARRIER_AUDIO, barrier_audio_device)
DECLARE_DEVICE_TYPE(STAR_HAWK_AUDIO, starhawk_audio_device)
DECLARE_DEVICE_TYPE(SUNDANCE_AUDIO, sundance_audio_device)
DECLARE_DEVICE_TYPE(SPEED_FREAK_AUDIO, speedfrk_audio_device)
DECLARE_DEVICE_TYPE(TAIL_GUNNER_AUDIO, tailg_audio_device)
DECLARE_DEVICE_TYPE(WARRIOR_AUDIO, warrior_audio_device)
DECLARE_DEVICE_TYPE(ARMOR_ATTACK_AUDIO, armora_audio_device)
DECLARE_DEVICE_TYPE(RIP_OFF_AUDIO, ripoff_audio_device)
DECLARE_DEVICE_TYPE(SOLAR_QUEST_AUDIO, solarq_audio_device)
DECLARE_DEVICE_TYPE(BOXING_BUGS_AUDIO, boxingb_audio_device)
DECLARE_DEVICE_TYPE(STAR_CASTLE_AUDIO, starcas_audio_device)
DECLARE_DEVICE_TYPE(WAR_OF_THE_WORLDS_AUDIO, wotw_audio_device)

#endif // MAME_AUDIO_CINEMAT_H
