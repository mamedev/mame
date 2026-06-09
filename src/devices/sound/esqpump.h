// license:BSD-3-Clause
// copyright-holders:Christian Brunschen
#ifndef MAME_SOUND_ESQPUMP_H
#define MAME_SOUND_ESQPUMP_H

#pragma once

#include "sound/es5506.h"
#include "cpu/es5510/es5510.h"

namespace sound::esqpump {

struct stats;

}

class esq_5505_5510_pump_device : public device_t, public device_sound_interface
{
public:
	esq_5505_5510_pump_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_esp(T &&tag) { m_esp.set_tag(std::forward<T>(tag)); }
	void set_esp_halted(bool esp_halted);
	bool get_esp_halted() {
		return m_esp_halted;
	}

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_clock_changed() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream) override;

private:
	// internal state:
	// sound stream
	sound_stream *m_stream;

	// ESP signal processor
	required_device<es5510_device> m_esp;

	// Is the ESP halted by the CPU?
	bool m_esp_halted;

	// Statitics and other tracked information.
	std::unique_ptr<sound::esqpump::stats> m_stats;
};

DECLARE_DEVICE_TYPE(ESQ_5505_5510_PUMP, esq_5505_5510_pump_device)

#endif // MAME_SOUND_ESQPUMP_H
